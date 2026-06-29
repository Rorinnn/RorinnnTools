// MachineId.cpp — Windows 机器码生成工具

module;

#include <Windows.h>
#include <comdef.h>
#include <wbemidl.h>

#include <botan/base64.h>
#include <botan/hash.h>

module RnTools;
import std;

namespace RnTools
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
    std::string   SerialNumber;
    std::uint64_t Size = 0;
};

class ComScope
{
  public:
    ComScope()
    {
        Result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(Result))
            return;

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
            CoUninitialize();
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
            Value->Release();
    }

    ComRelease(const ComRelease&)            = delete;
    ComRelease& operator=(const ComRelease&) = delete;

    IUnknown** Out()
    {
        return &Value;
    }

    template <typename T>
    T* As() const
    {
        return reinterpret_cast<T*>(Value);
    }

  private:
    IUnknown* Value = nullptr;
};

static std::string VariantToString(const VARIANT& Value)
{
    if (Value.vt == VT_BSTR)
        return Text::Trim(Text::ToUtf8(Value.bstrVal));

    VARIANT TextValue{};
    VariantInit(&TextValue);
    if (SUCCEEDED(VariantChangeType(&TextValue, const_cast<VARIANT*>(&Value), 0, VT_BSTR)))
    {
        std::string Result = Text::Trim(Text::ToUtf8(TextValue.bstrVal));
        VariantClear(&TextValue);
        return Result;
    }

    return {};
}

static std::uint64_t VariantToUInt64(const VARIANT& Value)
{
    if (Value.vt == VT_I8 || Value.vt == VT_UI8)
        return static_cast<std::uint64_t>(Value.ullVal);

    std::string Text = VariantToString(Value);
    if (Text.empty())
        return 0;

    char*         End  = nullptr;
    std::uint64_t Size = std::strtoull(Text.c_str(), &End, 10);
    return End && *End == '\0' ? Size : 0;
}

static std::string NormalizeSerial(std::string Value)
{
    Value = Text::Trim(Value);
    std::replace(Value.begin(), Value.end(), ' ', '_');
    std::transform(Value.begin(),
                   Value.end(),
                   Value.begin(),
                   [](unsigned char Char)
                   { return static_cast<char>(std::toupper(Char)); });
    return Value;
}

static bool IsValidSerialNumber(const std::string& SerialNumber)
{
    const std::string Value = NormalizeSerial(SerialNumber);
    if (Value.size() < 3)
        return false;

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
        if (Value == Invalid)
            return false;

    return std::any_of(
        Value.begin(), Value.end(), [](unsigned char Char)
        { return std::isalnum(Char) != 0 && Char != '0'; });
}

static bool ConnectWmi(IWbemServices*& Services)
{
    Services = nullptr;

    ComRelease Locator;
    HRESULT    Result = CoCreateInstance(
        CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void**>(Locator.Out()));
    if (FAILED(Result))
        return false;

    Result = Locator.As<IWbemLocator>()->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &Services);
    if (FAILED(Result))
        return false;

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
    if (!Services || !Query)
        return Objects;

    IEnumWbemClassObject* Enumerator = nullptr;
    HRESULT               Result     = Services->ExecQuery(
        _bstr_t(L"WQL"), _bstr_t(Query), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &Enumerator);
    if (FAILED(Result) || !Enumerator)
        return Objects;

    while (true)
    {
        IWbemClassObject* Object   = nullptr;
        ULONG             Returned = 0;
        Result                     = Enumerator->Next(WBEM_INFINITE, 1, &Object, &Returned);
        if (FAILED(Result) || Returned == 0 || !Object)
            break;

        Objects.push_back(Object);
    }

    Enumerator->Release();
    return Objects;
}

static std::string ReadWmiString(IWbemClassObject* Object, const wchar_t* Property)
{
    if (!Object || !Property)
        return {};

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

static std::uint64_t ReadWmiUInt64(IWbemClassObject* Object, const wchar_t* Property)
{
    if (!Object || !Property)
        return 0;

    VARIANT Value{};
    VariantInit(&Value);
    HRESULT Result = Object->Get(Property, 0, &Value, nullptr, nullptr);
    if (FAILED(Result))
    {
        VariantClear(&Value);
        return 0;
    }

    std::uint64_t Number = VariantToUInt64(Value);
    VariantClear(&Value);
    return Number;
}

static std::string GetHardwareInfo(IWbemServices* Services, const WmiQuery& Query)
{
    std::vector<IWbemClassObject*> Objects = ExecuteWmiQuery(Services, Query.Query);
    if (Objects.empty())
        return "Unknown";

    if (Query.UseFirstOnly)
    {
        std::string Value = ReadWmiString(Objects.front(), Query.Property);
        for (IWbemClassObject* Object : Objects)
            Object->Release();

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

    std::uint64_t TotalSize = 0;
    for (const DiskInfo& Disk : ValidDisks)
        TotalSize += Disk.Size;

    std::vector<std::string> UniqueSerials;
    UniqueSerials.reserve(ValidDisks.size());
    for (const DiskInfo& Disk : ValidDisks)
        UniqueSerials.push_back(Disk.SerialNumber);

    std::sort(UniqueSerials.begin(), UniqueSerials.end());
    UniqueSerials.erase(std::unique(UniqueSerials.begin(), UniqueSerials.end()), UniqueSerials.end());

    std::string Serials;
    for (std::size_t i = 0; i < UniqueSerials.size(); i++)
    {
        if (i > 0)
            Serials += ",";
        Serials += UniqueSerials[i];
    }

    std::ostringstream Stream;
    Stream << Serials << "|Count:" << ValidDisks.size() << "|TotalSize:" << TotalSize;
    return Stream.str();
}

static bool BuildSha256Base64Url(std::string_view Text, std::string& Result)
{
    Result.clear();

    try
    {
        auto Hasher = Botan::HashFunction::create_or_throw("SHA-256");
        Hasher->update(Text);
        std::vector<std::uint8_t> Hash = Hasher->final_stdvec();

        Result = Botan::base64_encode(Hash.data(), Hash.size());
        for (char& Char : Result)
            if (Char == '+')
                Char = '-';
            else if (Char == '/')
                Char = '_';

        while (!Result.empty() && Result.back() == '=')
            Result.pop_back();

        return true;
    }
    catch (const std::exception&)
    {
        Result.clear();
        return false;
    }
}

} // namespace

bool BuildMachineId(std::string& MachineId)
{
    MachineId.clear();

    ComScope Com;
    if (!Com.IsReady())
        return false;

    IWbemServices* Services = nullptr;
    if (!ConnectWmi(Services))
        return false;

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
        Material += GetHardwareInfo(Services, Query);

    if (Material.empty() || Material == "UnknownUnknown|Count:0|TotalSize:0")
        return false;

    return BuildSha256Base64Url(Material, MachineId);
}

} // namespace RnTools
