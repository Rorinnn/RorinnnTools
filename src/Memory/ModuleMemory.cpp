// ModuleMemory.cpp — 当前进程模块与 PE 节区查询工具

module;

#include <Windows.h>
#include <psapi.h>

module RnTools;
import std;

namespace RnTools::Memory
{
static std::size_t GetSectionSize(const IMAGE_SECTION_HEADER& Section)
{
    return Section.Misc.VirtualSize ? Section.Misc.VirtualSize : Section.SizeOfRawData;
}

static bool ReadNtHeaders(std::uintptr_t ModuleBase, IMAGE_DOS_HEADER& Dos, IMAGE_NT_HEADERS64& Nt)
{
    if (!ReadValue(ModuleBase, Dos) || Dos.e_magic != IMAGE_DOS_SIGNATURE)
        return false;

    const std::uintptr_t NtAddress = ModuleBase + static_cast<std::uintptr_t>(Dos.e_lfanew);
    return ReadValue(NtAddress, Nt) && Nt.Signature == IMAGE_NT_SIGNATURE;
}

static bool SectionNameEquals(const IMAGE_SECTION_HEADER& Section, std::string_view Name)
{
    char Buffer[9] = {};
    std::memcpy(Buffer, Section.Name, sizeof(Section.Name));
    return Name == Buffer;
}

bool GetModuleInfo(const wchar_t* ModuleName, ModuleInfo& Info)
{
    HMODULE Module = GetModuleHandleW(ModuleName);
    if (!Module)
        return false;

    return GetModuleInfo(reinterpret_cast<std::uintptr_t>(Module), Info);
}

bool GetModuleInfo(std::uintptr_t ModuleBase, ModuleInfo& Info)
{
    if (!ModuleBase)
        return false;

    MODULEINFO NativeInfo = {};
    if (!GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(ModuleBase), &NativeInfo, sizeof(NativeInfo)))
        return false;

    Info.Base = reinterpret_cast<std::uintptr_t>(NativeInfo.lpBaseOfDll);
    Info.Size = static_cast<std::size_t>(NativeInfo.SizeOfImage);
    return Info.Base && Info.Size;
}

bool GetModuleSection(std::uintptr_t ModuleBase, std::string_view SectionName, SectionInfo& Info)
{
    IMAGE_DOS_HEADER   Dos = {};
    IMAGE_NT_HEADERS64 Nt  = {};
    if (!ReadNtHeaders(ModuleBase, Dos, Nt))
        return false;

    const std::uintptr_t SectionAddress = ModuleBase + static_cast<std::uintptr_t>(Dos.e_lfanew) +
                                          offsetof(IMAGE_NT_HEADERS64, OptionalHeader) + Nt.FileHeader.SizeOfOptionalHeader;
    for (std::uint16_t Index = 0; Index < Nt.FileHeader.NumberOfSections; Index++)
    {
        IMAGE_SECTION_HEADER Section = {};
        if (!ReadValue(SectionAddress + Index * sizeof(Section), Section))
            return false;
        if (!SectionName.empty() && !SectionNameEquals(Section, SectionName))
            continue;

        Info = {};
        std::memcpy(Info.Name, Section.Name, sizeof(Section.Name));
        Info.Base            = ModuleBase + Section.VirtualAddress;
        Info.Size            = GetSectionSize(Section);
        Info.Characteristics = Section.Characteristics;
        return Info.Base && Info.Size;
    }

    return false;
}

bool GetModuleSection(const wchar_t* ModuleName, std::string_view SectionName, SectionInfo& Info)
{
    ModuleInfo Module = {};
    if (!GetModuleInfo(ModuleName, Module))
        return false;
    return GetModuleSection(Module.Base, SectionName, Info);
}
} // namespace RnTools::Memory
