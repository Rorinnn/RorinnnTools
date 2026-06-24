module;

export module RorinnnTools:Memory;
import std;

export namespace RorinnnTools::Memory
{

class SigScanner;

struct ModuleInfo
{
    std::uintptr_t Base = 0;
    std::size_t    Size = 0;
};

struct SectionInfo
{
    char           Name[9]         = {};
    std::uintptr_t Base            = 0;
    std::size_t    Size            = 0;
    std::uint32_t  Characteristics = 0;
};

struct PatternByte
{
    std::uint8_t Value    = 0;
    bool         Wildcard = false;
};

struct MemoryRange
{
    std::uintptr_t Start = 0;
    std::size_t    Size  = 0;

    bool           IsValid() const { return Start && Size; }
    std::uintptr_t End() const { return Start + Size; }
};

bool           ReadBytes(std::uintptr_t Ptr, void* PBuffer, std::size_t Size);
bool           WriteBytes(std::uintptr_t Ptr, const void* PBuffer, std::size_t Size);
bool           ReadPtr(std::uintptr_t Ptr, std::uintptr_t& Value);
bool           TryReadString(std::uintptr_t Ptr, std::size_t MaxLength, std::string& Value);
std::string    ReadString(std::uintptr_t Ptr, std::size_t MaxLength);
bool           TryReadCString(std::uintptr_t Ptr, std::string& Value, std::size_t ChunkSize = 256, std::size_t MaxLength = 64 * 1024);
std::string    ReadCString(std::uintptr_t Ptr, std::size_t ChunkSize = 256, std::size_t MaxLength = 64 * 1024);
bool           ResolvePointerChain(std::uintptr_t Base, std::span<const std::ptrdiff_t> Offsets, std::uintptr_t& Address);
bool           GetModuleInfo(const wchar_t* ModuleName, ModuleInfo& Info);
bool           GetModuleInfo(std::uintptr_t ModuleBase, ModuleInfo& Info);
bool           GetModuleSection(std::uintptr_t ModuleBase, std::string_view SectionName, SectionInfo& Info);
bool           GetModuleSection(const wchar_t* ModuleName, std::string_view SectionName, SectionInfo& Info);
bool           ParsePattern(std::string_view Pattern, std::vector<PatternByte>& Bytes);
std::uintptr_t FindPattern(std::uintptr_t Start, std::size_t Size, std::span<const PatternByte> Pattern);
std::uintptr_t FindPattern(std::uintptr_t Start, std::size_t Size, std::string_view Pattern);
std::uintptr_t ResolveRelativeAddress(std::uintptr_t InstructionAddress, std::size_t InstructionSize, std::size_t DisplacementOffset = 1);

template <typename T>
bool ReadValue(std::uintptr_t Ptr, T& Value);

template <typename T>
bool WriteValue(std::uintptr_t Ptr, const T& Value);

namespace StructBytes
{
template <typename T>
bool FromBytes(std::span<const std::uint8_t> Bytes, T& Value)
{
    if (Bytes.size() < sizeof(T))
        return false;
    std::memcpy(&Value, Bytes.data(), sizeof(T));
    return true;
}

template <typename T>
std::vector<std::uint8_t> ToBytes(const T& Value)
{
    std::vector<std::uint8_t> Bytes(sizeof(T));
    std::memcpy(Bytes.data(), &Value, sizeof(T));
    return Bytes;
}

template <typename T>
bool Read(std::uintptr_t Ptr, T& Value)
{
    return ReadValue(Ptr, Value);
}

template <typename T>
bool Write(std::uintptr_t Ptr, const T& Value)
{
    return WriteValue(Ptr, Value);
}
} // namespace StructBytes

namespace MemoryHelper
{
template <typename T>
T& Cast(std::uintptr_t Ptr)
{
    return *reinterpret_cast<T*>(Ptr);
}

template <typename T>
std::span<T> Cast(std::uintptr_t Ptr, std::size_t Count)
{
    return {reinterpret_cast<T*>(Ptr), Count};
}

template <typename T>
T Read(std::uintptr_t Ptr)
{
    T Value = {};
    std::memcpy(&Value, reinterpret_cast<const void*>(Ptr), sizeof(T));
    return Value;
}

inline std::vector<std::uint8_t> ReadRaw(std::uintptr_t Ptr, std::size_t Size)
{
    std::vector<std::uint8_t> Bytes(Size);
    if (Size)
        std::memcpy(Bytes.data(), reinterpret_cast<const void*>(Ptr), Size);
    return Bytes;
}

template <typename T>
void Write(std::uintptr_t Ptr, const T& Value)
{
    std::memcpy(reinterpret_cast<void*>(Ptr), &Value, sizeof(T));
}

inline void WriteRaw(std::uintptr_t Ptr, std::span<const std::uint8_t> Bytes)
{
    if (!Bytes.empty())
        std::memcpy(reinterpret_cast<void*>(Ptr), Bytes.data(), Bytes.size());
}

template <typename T>
bool TryRead(std::uintptr_t Ptr, T& Value)
{
    return ReadBytes(Ptr, &Value, sizeof(T));
}

inline bool TryReadRaw(std::uintptr_t Ptr, std::size_t Size, std::vector<std::uint8_t>& Bytes)
{
    Bytes.assign(Size, 0);
    if (!Size)
        return true;
    return ReadBytes(Ptr, Bytes.data(), Bytes.size());
}

template <typename T>
bool TryWrite(std::uintptr_t Ptr, const T& Value)
{
    return WriteBytes(Ptr, &Value, sizeof(T));
}

inline bool TryWriteRaw(std::uintptr_t Ptr, std::span<const std::uint8_t> Bytes)
{
    if (Bytes.empty())
        return true;
    return WriteBytes(Ptr, Bytes.data(), Bytes.size());
}

inline bool TryReadString(std::uintptr_t Ptr, std::size_t MaxLength, std::string& Value)
{
    return RorinnnTools::Memory::TryReadString(Ptr, MaxLength, Value);
}

inline std::string ReadString(std::uintptr_t Ptr, std::size_t MaxLength)
{
    return RorinnnTools::Memory::ReadString(Ptr, MaxLength);
}

inline bool TryReadCString(std::uintptr_t Ptr, std::string& Value, std::size_t ChunkSize = 256, std::size_t MaxLength = 64 * 1024)
{
    return RorinnnTools::Memory::TryReadCString(Ptr, Value, ChunkSize, MaxLength);
}

inline std::string ReadCString(std::uintptr_t Ptr, std::size_t ChunkSize = 256, std::size_t MaxLength = 64 * 1024)
{
    return RorinnnTools::Memory::ReadCString(Ptr, ChunkSize, MaxLength);
}
} // namespace MemoryHelper

class SigScanner
{
  public:
    explicit SigScanner(bool DoCopy = false);
    explicit SigScanner(const wchar_t* ModuleName, bool DoCopy = false);
    explicit SigScanner(std::uintptr_t ModuleBase, bool DoCopy = false);

    bool SetModule(const wchar_t* ModuleName);
    bool SetModule(std::uintptr_t ModuleBase);
    bool IsValid() const;
    bool IsCopyEnabled() const;

    ModuleInfo  GetModule() const;
    MemoryRange GetModuleRange() const;
    MemoryRange GetTextRange() const;
    MemoryRange GetDataRange() const;
    MemoryRange GetRDataRange() const;

    std::uintptr_t              ScanRange(MemoryRange Range, std::string_view Pattern) const;
    std::vector<std::uintptr_t> ScanRangeAll(MemoryRange Range, std::string_view Pattern) const;
    std::uintptr_t              ScanModule(std::string_view Pattern) const;
    std::uintptr_t              ScanText(std::string_view Pattern) const;
    std::uintptr_t              ScanTextBranchTarget(std::string_view Pattern) const;
    std::uintptr_t              ScanData(std::string_view Pattern) const;
    std::uintptr_t              ScanRData(std::string_view Pattern) const;
    std::vector<std::uintptr_t> ScanAllText(std::string_view Pattern) const;

    bool           TryGetStaticAddress(std::string_view Pattern, std::uintptr_t& Address, std::size_t InstructionSize = 7, std::size_t DisplacementOffset = 3) const;
    std::uintptr_t GetStaticAddress(std::string_view Pattern, std::size_t InstructionSize = 7, std::size_t DisplacementOffset = 3) const;
    bool           TryGetCallTarget(std::string_view Pattern, std::uintptr_t& Address) const;
    std::uintptr_t GetCallTarget(std::string_view Pattern) const;

  private:
    ModuleInfo                Module     = {};
    MemoryRange               Text       = {};
    MemoryRange               Data       = {};
    MemoryRange               RData      = {};
    bool                      UseCopy    = false;
    std::vector<std::uint8_t> ModuleCopy = {};

    bool           CopyModule();
    std::uintptr_t TranslateToScanAddress(std::uintptr_t Address) const;
    std::uintptr_t TranslateFromScanAddress(std::uintptr_t Address) const;
};

class MemoryPatch
{
  public:
    MemoryPatch() = default;
    MemoryPatch(std::uintptr_t Target, std::span<const std::uint8_t> PatchBytes);
    MemoryPatch(std::uintptr_t Target, std::string_view PatchPattern);
    MemoryPatch(SigScanner& Scanner, std::string_view Signature, std::string_view PatchPattern);
    MemoryPatch(const MemoryPatch&)            = delete;
    MemoryPatch& operator=(const MemoryPatch&) = delete;
    MemoryPatch(MemoryPatch&& Other) noexcept;
    MemoryPatch& operator=(MemoryPatch&& Other) noexcept;
    ~MemoryPatch();

    bool           Apply();
    bool           Restore();
    bool           IsValid() const;
    bool           IsApplied() const;
    std::uintptr_t GetTarget() const;
    std::size_t    GetSize() const;

  private:
    std::uintptr_t            Target = 0;
    std::vector<std::uint8_t> OriginalBytes;
    std::vector<PatternByte>  PatchBytes;
    bool                      Applied = false;
};

template <typename T>
class MemoryValuePatch
{
  public:
    MemoryValuePatch() = default;
    explicit MemoryValuePatch(std::uintptr_t PointerAddress) : PointerAddress(PointerAddress) {}

    bool Apply()
    {
        return CaptureOriginal();
    }

    bool Restore()
    {
        bool Ok = true;
        if (OriginalCaptured && !WriteValue(PointerAddress, OriginalValue))
            Ok = false;
        return Ok;
    }

    bool Set(const T& Value)
    {
        if (!CaptureOriginal())
            return false;
        return WriteValue(PointerAddress, Value);
    }

    bool Reset()
    {
        if (!OriginalCaptured)
            return true;
        return WriteValue(PointerAddress, OriginalValue);
    }

    std::uintptr_t GetPointerAddress() const { return PointerAddress; }
    bool           HasOriginalValue() const { return OriginalCaptured; }
    const T&       GetOriginalValue() const { return OriginalValue; }

  private:
    bool CaptureOriginal()
    {
        if (OriginalCaptured)
            return true;
        if (!ReadValue(PointerAddress, OriginalValue))
            return false;
        OriginalCaptured = true;
        return true;
    }

    std::uintptr_t PointerAddress   = 0;
    T              OriginalValue    = {};
    bool           OriginalCaptured = false;
};

template <typename T>
bool ReadValue(std::uintptr_t Ptr, T& Value)
{
    if (!ReadBytes(Ptr, &Value, sizeof(T)))
        return false;
    return true;
}

template <typename T>
bool WriteValue(std::uintptr_t Ptr, const T& Value)
{
    return WriteBytes(Ptr, &Value, sizeof(T));
}

template <typename T>
bool ReadPointerChainValue(std::uintptr_t Base, std::span<const std::ptrdiff_t> Offsets, T& Value)
{
    std::uintptr_t Address = 0;
    if (!ResolvePointerChain(Base, Offsets, Address))
        return false;
    return ReadValue(Address, Value);
}

} // namespace RorinnnTools::Memory
