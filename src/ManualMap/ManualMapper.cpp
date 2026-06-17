// ManualMapper.cpp — 手动映射注入实现

module;

#include <Windows.h>

#include <cstdlib>
#include <cstring>

module RorinnnTools;

namespace RorinnnTools
{
namespace
{
#ifdef _WIN64
constexpr WORD CurrentArch = IMAGE_FILE_MACHINE_AMD64;
#else
constexpr WORD CurrentArch = IMAGE_FILE_MACHINE_I386;
#endif

static void ReleaseRemoteMemory(HANDLE Process, void* Address)
{
    if (!Address) return;
    VirtualFreeEx(Process, Address, 0, MEM_RELEASE);
}
} // namespace

bool ManualMapDll(HANDLE Process,
                  BYTE*  SourceData,
                  SIZE_T FileSize,
                  bool   ClearHeader,
                  bool   ClearNonNeededSections,
                  bool   AdjustProtections,
                  bool   SEHExceptionSupport,
                  DWORD  Reason,
                  LPVOID Reserved)
{
    if (!Process || !SourceData || FileSize < 0x1000) return false;

    if (reinterpret_cast<IMAGE_DOS_HEADER*>(SourceData)->e_magic != IMAGE_DOS_SIGNATURE)
    {
        return false;
    }

    auto* NtHeader =
        reinterpret_cast<IMAGE_NT_HEADERS*>(SourceData + reinterpret_cast<IMAGE_DOS_HEADER*>(SourceData)->e_lfanew);
    IMAGE_OPTIONAL_HEADER* OptionalHeader = &NtHeader->OptionalHeader;
    IMAGE_FILE_HEADER*     FileHeader     = &NtHeader->FileHeader;

    if (FileHeader->Machine != CurrentArch)
    {
        return false;
    }

    BYTE* TargetBase = reinterpret_cast<BYTE*>(
        VirtualAllocEx(Process, nullptr, OptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!TargetBase)
    {
        return false;
    }

    DWORD OldProtect = 0;
    VirtualProtectEx(Process, TargetBase, OptionalHeader->SizeOfImage, PAGE_EXECUTE_READWRITE, &OldProtect);

    ManualMappingData Data{};
    Data.LoadLibraryA   = LoadLibraryA;
    Data.GetProcAddress = GetProcAddress;
#ifdef _WIN64
    Data.RtlAddFunctionTable = reinterpret_cast<RtlAddFunctionTable_t>(RtlAddFunctionTable);
#else
    SEHExceptionSupport = false;
#endif
    Data.Base       = TargetBase;
    Data.Reason     = Reason;
    Data.Reserved   = Reserved;
    Data.SEHSupport = SEHExceptionSupport;

    if (!WriteProcessMemory(Process, TargetBase, SourceData, 0x1000, nullptr))
    {
        ReleaseRemoteMemory(Process, TargetBase);
        return false;
    }

    IMAGE_SECTION_HEADER* SectionHeader = IMAGE_FIRST_SECTION(NtHeader);
    for (UINT i = 0; i != FileHeader->NumberOfSections; ++i, ++SectionHeader)
    {
        if (!SectionHeader->SizeOfRawData) continue;

        if (!WriteProcessMemory(Process,
                                TargetBase + SectionHeader->VirtualAddress,
                                SourceData + SectionHeader->PointerToRawData,
                                SectionHeader->SizeOfRawData,
                                nullptr))
        {
            ReleaseRemoteMemory(Process, TargetBase);
            return false;
        }
    }

    BYTE* MappingDataAlloc = reinterpret_cast<BYTE*>(
        VirtualAllocEx(Process, nullptr, sizeof(ManualMappingData), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!MappingDataAlloc)
    {
        ReleaseRemoteMemory(Process, TargetBase);
        return false;
    }

    if (!WriteProcessMemory(Process, MappingDataAlloc, &Data, sizeof(ManualMappingData), nullptr))
    {
        ReleaseRemoteMemory(Process, TargetBase);
        ReleaseRemoteMemory(Process, MappingDataAlloc);
        return false;
    }

    void* ShellcodeAlloc = VirtualAllocEx(Process, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!ShellcodeAlloc)
    {
        ReleaseRemoteMemory(Process, TargetBase);
        ReleaseRemoteMemory(Process, MappingDataAlloc);
        return false;
    }

    if (!WriteProcessMemory(Process, ShellcodeAlloc, ManualMapShellcode, 0x1000, nullptr))
    {
        ReleaseRemoteMemory(Process, TargetBase);
        ReleaseRemoteMemory(Process, MappingDataAlloc);
        ReleaseRemoteMemory(Process, ShellcodeAlloc);
        return false;
    }

    HANDLE Thread = CreateRemoteThread(
        Process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ShellcodeAlloc), MappingDataAlloc, 0, nullptr);
    if (!Thread)
    {
        ReleaseRemoteMemory(Process, TargetBase);
        ReleaseRemoteMemory(Process, MappingDataAlloc);
        ReleaseRemoteMemory(Process, ShellcodeAlloc);
        return false;
    }
    CloseHandle(Thread);

    HINSTANCE Check = nullptr;
    while (!Check)
    {
        DWORD ExitCode = 0;
        GetExitCodeProcess(Process, &ExitCode);
        if (ExitCode != STILL_ACTIVE)
        {
            return false;
        }

        ManualMappingData CheckedData{};
        ReadProcessMemory(Process, MappingDataAlloc, &CheckedData, sizeof(CheckedData), nullptr);
        Check = CheckedData.Module;

        if (Check == reinterpret_cast<HINSTANCE>(0x404040))
        {
            ReleaseRemoteMemory(Process, TargetBase);
            ReleaseRemoteMemory(Process, MappingDataAlloc);
            ReleaseRemoteMemory(Process, ShellcodeAlloc);
            return false;
        }

        Sleep(10);
    }

    BYTE* EmptyBuffer = reinterpret_cast<BYTE*>(std::malloc(1024 * 1024 * 20));
    if (!EmptyBuffer)
    {
        return false;
    }
    std::memset(EmptyBuffer, 0, 1024 * 1024 * 20);

    if (ClearHeader)
    {
        WriteProcessMemory(Process, TargetBase, EmptyBuffer, 0x1000, nullptr);
    }

    if (ClearNonNeededSections)
    {
        SectionHeader = IMAGE_FIRST_SECTION(NtHeader);
        for (UINT i = 0; i != FileHeader->NumberOfSections; ++i, ++SectionHeader)
        {
            if (!SectionHeader->Misc.VirtualSize) continue;

            const bool ShouldClear =
                (!SEHExceptionSupport && std::strcmp(reinterpret_cast<char*>(SectionHeader->Name), ".pdata") == 0) ||
                std::strcmp(reinterpret_cast<char*>(SectionHeader->Name), ".rsrc") == 0 ||
                std::strcmp(reinterpret_cast<char*>(SectionHeader->Name), ".reloc") == 0;
            if (!ShouldClear) continue;

            WriteProcessMemory(Process,
                               TargetBase + SectionHeader->VirtualAddress,
                               EmptyBuffer,
                               SectionHeader->Misc.VirtualSize,
                               nullptr);
        }
    }

    if (AdjustProtections)
    {
        SectionHeader = IMAGE_FIRST_SECTION(NtHeader);
        for (UINT i = 0; i != FileHeader->NumberOfSections; ++i, ++SectionHeader)
        {
            if (!SectionHeader->Misc.VirtualSize) continue;

            DWORD NewProtect = PAGE_READONLY;
            if ((SectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE) > 0)
            {
                NewProtect = PAGE_READWRITE;
            }
            else if ((SectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) > 0)
            {
                NewProtect = PAGE_EXECUTE_READ;
            }

            DWORD Old = 0;
            VirtualProtectEx(
                Process, TargetBase + SectionHeader->VirtualAddress, SectionHeader->Misc.VirtualSize, NewProtect, &Old);
        }

        DWORD Old = 0;
        VirtualProtectEx(Process, TargetBase, IMAGE_FIRST_SECTION(NtHeader)->VirtualAddress, PAGE_READONLY, &Old);
    }

    WriteProcessMemory(Process, ShellcodeAlloc, EmptyBuffer, 0x1000, nullptr);
    ReleaseRemoteMemory(Process, ShellcodeAlloc);
    ReleaseRemoteMemory(Process, MappingDataAlloc);

    std::free(EmptyBuffer);
    return true;
}

} // namespace RorinnnTools
