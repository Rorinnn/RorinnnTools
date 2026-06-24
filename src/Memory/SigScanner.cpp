// SigScanner.cpp — 模块节区范围管理与签名扫描工具

module;

#include <Windows.h>

module RorinnnTools;
import std;

namespace RorinnnTools::Memory
{
static MemoryRange ToRange(const ModuleInfo& Module)
{
    return {Module.Base, Module.Size};
}

static MemoryRange ToRange(const SectionInfo& Section)
{
    return {Section.Base, Section.Size};
}

static MemoryRange QuerySectionRange(std::uintptr_t ModuleBase, std::string_view SectionName)
{
    SectionInfo Section = {};
    if (!GetModuleSection(ModuleBase, SectionName, Section))
        return {};
    return ToRange(Section);
}

static bool IsRelativeBranch(std::uintptr_t Address)
{
    std::uint8_t OpCode = 0;
    return ReadValue(Address, OpCode) && (OpCode == 0xE8 || OpCode == 0xE9);
}

static std::uintptr_t ResolveRelativeAddressFrom(std::uintptr_t InstructionAddress, std::uintptr_t ReadAddress, std::size_t InstructionSize, std::size_t DisplacementOffset)
{
    std::int32_t Displacement = 0;
    if (!ReadValue(ReadAddress + DisplacementOffset, Displacement))
        return 0;
    return InstructionAddress + InstructionSize + Displacement;
}

SigScanner::SigScanner(bool DoCopy) : UseCopy(DoCopy)
{
    SetModule(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)));
}

SigScanner::SigScanner(const wchar_t* ModuleName, bool DoCopy) : UseCopy(DoCopy)
{
    SetModule(ModuleName);
}

SigScanner::SigScanner(std::uintptr_t ModuleBase, bool DoCopy) : UseCopy(DoCopy)
{
    SetModule(ModuleBase);
}

bool SigScanner::SetModule(const wchar_t* ModuleName)
{
    ModuleInfo Info = {};
    if (!GetModuleInfo(ModuleName, Info))
        return false;
    return SetModule(Info.Base);
}

bool SigScanner::SetModule(std::uintptr_t ModuleBase)
{
    ModuleInfo Info = {};
    if (!GetModuleInfo(ModuleBase, Info))
        return false;

    Module = Info;
    Text   = QuerySectionRange(Module.Base, ".text");
    Data   = QuerySectionRange(Module.Base, ".data");
    RData  = QuerySectionRange(Module.Base, ".rdata");
    return !UseCopy || CopyModule();
}

bool SigScanner::IsValid() const
{
    return Module.Base && Module.Size;
}

bool SigScanner::IsCopyEnabled() const
{
    return UseCopy;
}

ModuleInfo SigScanner::GetModule() const
{
    return Module;
}

MemoryRange SigScanner::GetModuleRange() const
{
    return ToRange(Module);
}

MemoryRange SigScanner::GetTextRange() const
{
    return Text;
}

MemoryRange SigScanner::GetDataRange() const
{
    return Data;
}

MemoryRange SigScanner::GetRDataRange() const
{
    return RData;
}

std::uintptr_t SigScanner::ScanRange(MemoryRange Range, std::string_view Pattern) const
{
    if (!Range.IsValid())
        return 0;

    const std::uintptr_t Found = FindPattern(TranslateToScanAddress(Range.Start), Range.Size, Pattern);
    return TranslateFromScanAddress(Found);
}

std::vector<std::uintptr_t> SigScanner::ScanRangeAll(MemoryRange Range, std::string_view Pattern) const
{
    std::vector<std::uintptr_t> Results;
    if (!Range.IsValid())
        return Results;

    std::vector<PatternByte> Bytes;
    if (!ParsePattern(Pattern, Bytes))
        return Results;

    std::uintptr_t Cursor = TranslateToScanAddress(Range.Start);
    std::size_t    Size   = Range.Size;
    while (Size >= Bytes.size())
    {
        const std::uintptr_t Found = FindPattern(Cursor, Size, Bytes);
        if (!Found)
            break;

        Results.push_back(TranslateFromScanAddress(Found));

        const std::uintptr_t Next = Found + 1;
        const std::size_t    Used = Next - Cursor;
        if (Used >= Size)
            break;

        Cursor  = Next;
        Size   -= Used;
    }

    return Results;
}

std::uintptr_t SigScanner::ScanModule(std::string_view Pattern) const
{
    return ScanRange(GetModuleRange(), Pattern);
}

std::uintptr_t SigScanner::ScanText(std::string_view Pattern) const
{
    return ScanRange(Text, Pattern);
}

std::uintptr_t SigScanner::ScanTextTarget(std::string_view Pattern) const
{
    const std::uintptr_t Found = ScanText(Pattern);
    if (!Found)
        return 0;
    if (IsRelativeBranch(TranslateToScanAddress(Found)))
        return ResolveRelativeAddressFrom(Found, TranslateToScanAddress(Found), 5, 1);
    return Found;
}

std::uintptr_t SigScanner::ScanData(std::string_view Pattern) const
{
    return ScanRange(Data, Pattern);
}

std::uintptr_t SigScanner::ScanRData(std::string_view Pattern) const
{
    return ScanRange(RData, Pattern);
}

std::vector<std::uintptr_t> SigScanner::ScanAllText(std::string_view Pattern) const
{
    return ScanRangeAll(Text, Pattern);
}

bool SigScanner::TryGetStaticAddressFromSig(std::string_view Pattern, std::uintptr_t& Address, std::size_t InstructionSize, std::size_t DisplacementOffset) const
{
    const std::uintptr_t Found = ScanText(Pattern);
    if (!Found)
        return false;

    Address = ResolveRelativeAddressFrom(Found, TranslateToScanAddress(Found), InstructionSize, DisplacementOffset);
    return Address != 0;
}

std::uintptr_t SigScanner::GetStaticAddressFromSig(std::string_view Pattern, std::size_t InstructionSize, std::size_t DisplacementOffset) const
{
    std::uintptr_t Address = 0;
    TryGetStaticAddressFromSig(Pattern, Address, InstructionSize, DisplacementOffset);
    return Address;
}

bool SigScanner::TryGetCallTargetFromSig(std::string_view Pattern, std::uintptr_t& Address) const
{
    const std::uintptr_t Found = ScanText(Pattern);
    if (!Found || !IsRelativeBranch(TranslateToScanAddress(Found)))
        return false;

    Address = ResolveRelativeAddressFrom(Found, TranslateToScanAddress(Found), 5, 1);
    return Address != 0;
}

std::uintptr_t SigScanner::GetCallTargetFromSig(std::string_view Pattern) const
{
    std::uintptr_t Address = 0;
    TryGetCallTargetFromSig(Pattern, Address);
    return Address;
}

bool SigScanner::CopyModule()
{
    ModuleCopy.resize(Module.Size);
    if (!ReadBytes(Module.Base, ModuleCopy.data(), ModuleCopy.size()))
    {
        ModuleCopy.clear();
        return false;
    }
    return true;
}

std::uintptr_t SigScanner::TranslateToScanAddress(std::uintptr_t Address) const
{
    if (!UseCopy || ModuleCopy.empty() || Address < Module.Base || Address >= Module.Base + Module.Size)
        return Address;
    return reinterpret_cast<std::uintptr_t>(ModuleCopy.data()) + (Address - Module.Base);
}

std::uintptr_t SigScanner::TranslateFromScanAddress(std::uintptr_t Address) const
{
    if (!Address || !UseCopy || ModuleCopy.empty())
        return Address;

    const std::uintptr_t CopyBase = reinterpret_cast<std::uintptr_t>(ModuleCopy.data());
    if (Address < CopyBase || Address >= CopyBase + ModuleCopy.size())
        return Address;
    return Module.Base + (Address - CopyBase);
}
} // namespace RorinnnTools::Memory
