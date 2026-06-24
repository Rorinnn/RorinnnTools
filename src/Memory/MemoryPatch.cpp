// MemoryPatch.cpp — 可恢复内存补丁工具

module;

module RorinnnTools;
import std;

namespace RorinnnTools::Memory
{
static std::vector<PatternByte> ToPatternBytes(std::span<const std::uint8_t> Bytes)
{
    std::vector<PatternByte> Pattern;
    Pattern.reserve(Bytes.size());
    for (std::uint8_t Byte : Bytes)
        Pattern.push_back({Byte, false});
    return Pattern;
}

static std::vector<std::uint8_t> BuildPatchBytes(std::span<const PatternByte> PatchBytes, std::span<const std::uint8_t> OriginalBytes)
{
    std::vector<std::uint8_t> Bytes;
    Bytes.reserve(PatchBytes.size());
    for (std::size_t Index = 0; Index < PatchBytes.size(); Index++)
        Bytes.push_back(PatchBytes[Index].Wildcard ? OriginalBytes[Index] : PatchBytes[Index].Value);
    return Bytes;
}

MemoryPatch::MemoryPatch(std::uintptr_t Target, std::span<const std::uint8_t> PatchBytes) : Target(Target), PatchBytes(ToPatternBytes(PatchBytes))
{
    if (Target && !PatchBytes.empty())
        OriginalBytes.resize(PatchBytes.size());
}

MemoryPatch::MemoryPatch(std::uintptr_t Target, std::string_view PatchPattern) : Target(Target)
{
    if (Target && ParsePattern(PatchPattern, PatchBytes))
        OriginalBytes.resize(PatchBytes.size());
}

MemoryPatch::MemoryPatch(SigScanner& Scanner, std::string_view Signature, std::string_view PatchPattern)
{
    Target = Scanner.ScanText(Signature);
    if (Target && ParsePattern(PatchPattern, PatchBytes))
        OriginalBytes.resize(PatchBytes.size());
}

MemoryPatch::MemoryPatch(MemoryPatch&& Other) noexcept
{
    *this = std::move(Other);
}

MemoryPatch& MemoryPatch::operator=(MemoryPatch&& Other) noexcept
{
    if (this == &Other)
        return *this;

    Restore();

    Target        = Other.Target;
    OriginalBytes = std::move(Other.OriginalBytes);
    PatchBytes    = std::move(Other.PatchBytes);
    Applied       = Other.Applied;

    Other.Target  = 0;
    Other.Applied = false;
    return *this;
}

MemoryPatch::~MemoryPatch()
{
    Restore();
}

bool MemoryPatch::Apply()
{
    if (!IsValid())
        return false;
    if (Applied)
        return true;
    if (!ReadBytes(Target, OriginalBytes.data(), OriginalBytes.size()))
        return false;

    const std::vector<std::uint8_t> Bytes = BuildPatchBytes(PatchBytes, OriginalBytes);
    if (!WriteBytes(Target, Bytes.data(), Bytes.size()))
        return false;

    Applied = true;
    return true;
}

bool MemoryPatch::Restore()
{
    if (!Applied)
        return true;
    if (!WriteBytes(Target, OriginalBytes.data(), OriginalBytes.size()))
        return false;

    Applied = false;
    return true;
}

bool MemoryPatch::IsValid() const
{
    return Target && !PatchBytes.empty() && PatchBytes.size() == OriginalBytes.size();
}

bool MemoryPatch::IsApplied() const
{
    return Applied;
}

std::uintptr_t MemoryPatch::GetTarget() const
{
    return Target;
}

std::size_t MemoryPatch::GetSize() const
{
    return PatchBytes.size();
}
} // namespace RorinnnTools::Memory
