// MachineId.cpp — Windows 机器码生成工具

#include "MachineId/MachineId.hpp"

#include <Windows.h>
#include <bcrypt.h>
#include <comdef.h>
#include <wbemidl.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

namespace RorinnnTools
{
namespace
{

struct WmiQuery
{
    const wchar_t* Query        = nullptr;
    const wchar_t* Property     = nullptr;
    bool           UseFirstOnly = true;
};

struct DiskInfo
{
    std::string SerialNumber;
    uint64_t    Size = 0;
};

class ComScope
{
  public:
    ComScope()
    {
        Result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(Result)) return;

        SecurityResult = CoInitializeSecurity(nullptr,
                                              -1,
                                              nullptr,
                                              nullptr,
                                              RPC_C_AUTHN_LEVEL_DEFAULT,
                                              RPC_C_IMP_LEVEL_IMPERSONATE,
                                              nullptr,
                                              EOAC_NONE,
                                              nullptr);
    }

    ~ComScope()
    {
        if (SUCCEEDED(Result))
        {
            CoUninitialize();
        }
    }

    bool IsReady() const
    {
        return SUCCEEDED(Result) && (SUCCEEDED(SecurityResult) || SecurityResult == RPC_E_TOO_LATE);
    }

  private:
    HRESULT Result         = E_FAIL;
    HRESULT SecurityResult = E_FAIL;
};

class ComRelease
{
  public:
    explicit ComRelease(IUnknown* Value = nullptr) : Value(Value) {}

    ~ComRelease()
    {
        if (Value)
        {
            Value->Release();
        }
    }

    ComRelease(const ComRelease&)            = delete;
    ComRelease& operator=(const ComRelease&) = delete;

    IUnknown** Out()
    {
        return &Value;
    }

    template <typename T> T* As() const
    {
        return reinterpret_cast<T*>(Value);
    }

  private:
    IUnknown* Value = nullptr;
};

static std::string Trim(std::string Value)
{
    auto IsSpace = [](unsigned char Char) { return std::isspace(Char) != 0; };
    Value.erase(Value.begin(), std::find_if_not(Value.begin(), Value.end(), IsSpace));
    Value.erase(std::find_if_not(Value.rbegin(), Value.rend(), IsSpace).base(), Value.end());
    return Value;
}

static std::string ToUtf8(const wchar_t* Text)
{
    if (!Text || Text[0] == L'\0') return {};

    int Size = WideCharToMultiByte(CP_UTF8, 0, Text, -1, nullptr, 0, nullptr, nullptr);
    if (Size <= 1) return {};

    std::string Result(static_cast<size_t>(Size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, Text, -1, Result.data(), Size, nullptr, nullptr);
    return Result;
}

static std::string VariantToString(const VARIANT& Value)
{
    if (Value.vt == VT_BSTR)
    {
        return Trim(ToUtf8(Value.bstrVal));
    }

    VARIANT TextValue{};
    VariantInit(&TextValue);
    if (SUCCEEDED(VariantChangeType(&TextValue, const_cast<VARIANT*>(&Value), 0, VT_BSTR)))
    {
        std::string Result = Trim(ToUtf8(TextValue.bstrVal));
        VariantClear(&TextValue);
        return Result;
    }

    return {};
}

static uint64_t VariantToUInt64(const VARIANT& Value)
{
    if (Value.vt == VT_I8 || Value.vt == VT_UI8)
    {
        return static_cast<uint64_t>(Value.ullVal);
    }

    std::string Text = VariantToString(Value);
    if (Text.empty()) return 0;

    char*    End  = nullptr;
    uint64_t Size = std::strtoull(Text.c_str(), &End, 10);
    return End && *End == '\0' ? Size : 0;
}

static std::string NormalizeSerial(std::string Value)
{
    Value = Trim(std::move(Value));
    std::replace(Value.begin(), Value.end(), ' ', '_');
    std::transform(Value.begin(),
                   Value.end(),
                   Value.begin(),
                   [](unsigned char Char) { return static_cast<char>(std::toupper(Char)); });
    return Value;
}

static bool IsValidSerialNumber(const std::string& SerialNumber)
{
    const std::string Value = NormalizeSerial(SerialNumber);
    if (Value.size() < 3) return false;

    static const char* InvalidValues[] = {
        "0000_0000_0000_0000_0000_0000_0000_0000",
        "0000_0000_0000_0001",
        "NONE",
        "TO_BE_FILLED_BY_O.E.M.",
        "DEFAULT_STRING",
        "SYSTEM_SERIAL_NUMBER",
        "VM",
        "UNKNOWN",
    };

    for (const char* Invalid : InvalidValues)
    {
        if (Value == Invalid) return false;
    }

    return std::any_of(
        Value.begin(), Value.end(), [](unsigned char Char) { return std::isalnum(Char) != 0 && Char != '0'; });
}

static bool ConnectWmi(IWbemServices*& Services)
{
    Services = nullptr;

    ComRelease Locator;
    HRESULT    Result = CoCreateInstance(
        CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void**>(Locator.Out()));
    if (FAILED(Result))
    {
        return false;
    }

    Result = Locator.As<IWbemLocator>()->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &Services);
    if (FAILED(Result))
    {
        return false;
    }

    Result = CoSetProxyBlanket(Services,
                               RPC_C_AUTHN_WINNT,
                               RPC_C_AUTHZ_NONE,
                               nullptr,
                               RPC_C_AUTHN_LEVEL_CALL,
                               RPC_C_IMP_LEVEL_IMPERSONATE,
                               nullptr,
                               EOAC_NONE);
    if (FAILED(Result))
    {
        Services->Release();
        Services = nullptr;
        return false;
    }

    return true;
}

static std::vector<IWbemClassObject*> ExecuteWmiQuery(IWbemServices* Services, const wchar_t* Query)
{
    std::vector<IWbemClassObject*> Objects;
    if (!Services || !Query) return Objects;

    IEnumWbemClassObject* Enumerator = nullptr;
    HRESULT               Result     = Services->ExecQuery(
        _bstr_t(L"WQL"), _bstr_t(Query), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &Enumerator);
    if (FAILED(Result) || !Enumerator)
    {
        return Objects;
    }

    while (true)
    {
        IWbemClassObject* Object   = nullptr;
        ULONG             Returned = 0;
        Result                     = Enumerator->Next(WBEM_INFINITE, 1, &Object, &Returned);
        if (FAILED(Result) || Returned == 0 || !Object)
        {
            break;
        }

        Objects.push_back(Object);
    }

    Enumerator->Release();
    return Objects;
}

static std::string ReadWmiString(IWbemClassObject* Object, const wchar_t* Property)
{
    if (!Object || !Property) return {};

    VARIANT Value{};
    VariantInit(&Value);
    HRESULT Result = Object->Get(Property, 0, &Value, nullptr, nullptr);
    if (FAILED(Result))
    {
        VariantClear(&Value);
        return {};
    }

    std::string Text = VariantToString(Value);
    VariantClear(&Value);
    return Text;
}

static uint64_t ReadWmiUInt64(IWbemClassObject* Object, const wchar_t* Property)
{
    if (!Object || !Property) return 0;

    VARIANT Value{};
    VariantInit(&Value);
    HRESULT Result = Object->Get(Property, 0, &Value, nullptr, nullptr);
    if (FAILED(Result))
    {
        VariantClear(&Value);
        return 0;
    }

    uint64_t Number = VariantToUInt64(Value);
    VariantClear(&Value);
    return Number;
}

static std::string GetHardwareInfo(IWbemServices* Services, const WmiQuery& Query)
{
    std::vector<IWbemClassObject*> Objects = ExecuteWmiQuery(Services, Query.Query);
    if (Objects.empty()) return "Unknown";

    if (Query.UseFirstOnly)
    {
        std::string Value = ReadWmiString(Objects.front(), Query.Property);
        for (IWbemClassObject* Object : Objects)
        {
            Object->Release();
        }

        return IsValidSerialNumber(Value) ? NormalizeSerial(Value) : "Unknown";
    }

    std::vector<DiskInfo> ValidDisks;
    for (IWbemClassObject* Object : Objects)
    {
        std::string Serial = ReadWmiString(Object, Query.Property);
        if (IsValidSerialNumber(Serial))
        {
            DiskInfo Info{};
            Info.SerialNumber = NormalizeSerial(Serial);
            Info.Size         = ReadWmiUInt64(Object, L"Size");
            ValidDisks.push_back(std::move(Info));
        }

        Object->Release();
    }

    uint64_t TotalSize = 0;
    for (const DiskInfo& Disk : ValidDisks)
    {
        TotalSize += Disk.Size;
    }

    std::vector<std::string> UniqueSerials;
    UniqueSerials.reserve(ValidDisks.size());
    for (const DiskInfo& Disk : ValidDisks)
    {
        UniqueSerials.push_back(Disk.SerialNumber);
    }

    std::sort(UniqueSerials.begin(), UniqueSerials.end());
    UniqueSerials.erase(std::unique(UniqueSerials.begin(), UniqueSerials.end()), UniqueSerials.end());

    std::string Serials;
    for (size_t i = 0; i < UniqueSerials.size(); i++)
    {
        if (i > 0) Serials += ",";
        Serials += UniqueSerials[i];
    }

    std::ostringstream Stream;
    Stream << Serials << "|Count:" << ValidDisks.size() << "|TotalSize:" << TotalSize;
    return Stream.str();
}

static bool Sha256Bytes(std::string_view Text, std::vector<uint8_t>& Hash)
{
    Hash.clear();

    BCRYPT_ALG_HANDLE  AlgHandle  = nullptr;
    BCRYPT_HASH_HANDLE HashHandle = nullptr;

    NTSTATUS Status = BCryptOpenAlgorithmProvider(&AlgHandle, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (Status < 0)
    {
        return false;
    }

    Status = BCryptCreateHash(AlgHandle, &HashHandle, nullptr, 0, nullptr, 0, 0);
    if (Status < 0)
    {
        BCryptCloseAlgorithmProvider(AlgHandle, 0);
        return false;
    }

    Status = BCryptHashData(
        HashHandle, reinterpret_cast<PUCHAR>(const_cast<char*>(Text.data())), static_cast<ULONG>(Text.size()), 0);
    if (Status < 0)
    {
        BCryptDestroyHash(HashHandle);
        BCryptCloseAlgorithmProvider(AlgHandle, 0);
        return false;
    }

    Hash.resize(32);
    Status = BCryptFinishHash(HashHandle, Hash.data(), static_cast<ULONG>(Hash.size()), 0);

    BCryptDestroyHash(HashHandle);
    BCryptCloseAlgorithmProvider(AlgHandle, 0);

    if (Status < 0)
    {
        Hash.clear();
        return false;
    }

    return true;
}

static std::string Base64UrlEncode(const std::vector<uint8_t>& Bytes)
{
    static constexpr char Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    std::string Result;
    Result.reserve(((Bytes.size() + 2) / 3) * 4);

    size_t i = 0;
    while (i + 3 <= Bytes.size())
    {
        uint32_t Value = (static_cast<uint32_t>(Bytes[i]) << 16) | (static_cast<uint32_t>(Bytes[i + 1]) << 8) |
                         static_cast<uint32_t>(Bytes[i + 2]);
        Result.push_back(Alphabet[(Value >> 18) & 0x3F]);
        Result.push_back(Alphabet[(Value >> 12) & 0x3F]);
        Result.push_back(Alphabet[(Value >> 6) & 0x3F]);
        Result.push_back(Alphabet[Value & 0x3F]);
        i += 3;
    }

    size_t Remaining = Bytes.size() - i;
    if (Remaining == 1)
    {
        uint32_t Value = static_cast<uint32_t>(Bytes[i]) << 16;
        Result.push_back(Alphabet[(Value >> 18) & 0x3F]);
        Result.push_back(Alphabet[(Value >> 12) & 0x3F]);
    }
    else if (Remaining == 2)
    {
        uint32_t Value = (static_cast<uint32_t>(Bytes[i]) << 16) | (static_cast<uint32_t>(Bytes[i + 1]) << 8);
        Result.push_back(Alphabet[(Value >> 18) & 0x3F]);
        Result.push_back(Alphabet[(Value >> 12) & 0x3F]);
        Result.push_back(Alphabet[(Value >> 6) & 0x3F]);
    }

    return Result;
}

} // namespace

bool BuildMachineCode(std::string& MachineCode)
{
    MachineCode.clear();

    ComScope Com;
    if (!Com.IsReady())
    {
        return false;
    }

    IWbemServices* Services = nullptr;
    if (!ConnectWmi(Services))
    {
        return false;
    }

    ComRelease     ServicesScope(Services);
    const WmiQuery Queries[] = {
        {L"SELECT SerialNumber FROM Win32_BaseBoard", L"SerialNumber", true},
        {L"SELECT ProcessorId FROM Win32_Processor", L"ProcessorId", true},
        {L"SELECT SerialNumber, Size, Model FROM Win32_DiskDrive WHERE MediaType='Fixed hard disk media'",
         L"SerialNumber",
         false},
    };

    std::string Material;
    for (const WmiQuery& Query : Queries)
    {
        Material += GetHardwareInfo(Services, Query);
    }

    if (Material.empty() || Material == "UnknownUnknown|Count:0|TotalSize:0")
    {
        return false;
    }

    std::vector<uint8_t> Hash;
    if (!Sha256Bytes(Material, Hash))
    {
        return false;
    }

    MachineCode = Base64UrlEncode(Hash);
    return true;
}

} // namespace RorinnnTools
