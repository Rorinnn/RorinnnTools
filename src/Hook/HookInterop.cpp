// HookInterop.cpp — 常用 Hook 定位入口

module;

#include <Windows.h>

module RorinnnTools;
import std;

namespace RorinnnTools::Hook
{
static int Unprotect(void* Region)
{
    MEMORY_BASIC_INFORMATION Mbi;
    VirtualQuery((LPCVOID)Region, &Mbi, sizeof(Mbi));
    VirtualProtect(Mbi.BaseAddress, Mbi.RegionSize, PAGE_READWRITE, &Mbi.Protect);
    return Mbi.Protect;
}

static void Protect(void* Region, int Protection)
{
    MEMORY_BASIC_INFORMATION Mbi;
    VirtualQuery((LPCVOID)Region, &Mbi, sizeof(Mbi));
    VirtualProtect(Mbi.BaseAddress, Mbi.RegionSize, Protection, &Mbi.Protect);
}

static HookResult MakeHookResult(VehHookStatus Status, void* Address)
{
    return {Status, Address};
}

static bool NameEqualsAscii(const char* Left, const char* Right)
{
    if (!Left || !Right)
        return false;
    return _stricmp(Left, Right) == 0;
}

static void* FindImportThunk(HMODULE Module, const char* ImportedModuleName, const char* SymbolName)
{
    if (!Module || !ImportedModuleName || !SymbolName)
        return nullptr;

    const auto       Base = reinterpret_cast<std::uintptr_t>(Module);
    IMAGE_DOS_HEADER Dos  = {};
    if (!RorinnnTools::Memory::ReadValue(Base, Dos) || Dos.e_magic != IMAGE_DOS_SIGNATURE)
        return nullptr;

    IMAGE_NT_HEADERS64   Nt        = {};
    const std::uintptr_t NtAddress = Base + static_cast<std::uintptr_t>(Dos.e_lfanew);
    if (!RorinnnTools::Memory::ReadValue(NtAddress, Nt) || Nt.Signature != IMAGE_NT_SIGNATURE)
        return nullptr;

    const IMAGE_DATA_DIRECTORY& ImportDir = Nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!ImportDir.VirtualAddress || !ImportDir.Size)
        return nullptr;

    auto* Import = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(Base + ImportDir.VirtualAddress);
    for (; Import->Name; Import++)
    {
        const char* ModuleName = reinterpret_cast<const char*>(Base + Import->Name);
        if (!NameEqualsAscii(ModuleName, ImportedModuleName))
            continue;

        auto* OriginalThunk = reinterpret_cast<IMAGE_THUNK_DATA64*>(Base + Import->OriginalFirstThunk);
        auto* FirstThunk    = reinterpret_cast<IMAGE_THUNK_DATA64*>(Base + Import->FirstThunk);
        if (!OriginalThunk)
            OriginalThunk = FirstThunk;

        for (; OriginalThunk->u1.AddressOfData; OriginalThunk++, FirstThunk++)
        {
            if (IMAGE_SNAP_BY_ORDINAL64(OriginalThunk->u1.Ordinal))
                continue;

            auto* ImportByName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(Base + OriginalThunk->u1.AddressOfData);
            if (NameEqualsAscii(reinterpret_cast<const char*>(ImportByName->Name), SymbolName))
                return &FirstThunk->u1.Function;
        }
    }

    return nullptr;
}

static void* ReplaceSlot(void** Slot, void* RedirectAddress)
{
    void* Original = *Slot;

    int OriginalProtection = Unprotect(Slot);
    *Slot                  = RedirectAddress;
    Protect(Slot, OriginalProtection);

    return Original;
}

HookResult HookFromAddress(int Token, void* TargetAddress, void* RedirectAddress)
{
    VehHookStatus Status = InstallVehHookHandler();
    if (Status != VehHookStatus::Ok && Status != VehHookStatus::AlreadyInstalled)
        return MakeHookResult(Status, TargetAddress);

    VehHookOptions Options  = {};
    Options.Token           = Token;
    Options.TargetAddress   = TargetAddress;
    Options.RedirectAddress = RedirectAddress;
    Options.Type            = VehHookType::Int3Jump;

    Status = AddVehHook(Options);
    return MakeHookResult(Status, TargetAddress);
}

void* ReplaceFunctionPointer(void** Slot, void* RedirectAddress)
{
    return ReplaceSlot(Slot, RedirectAddress);
}

void* ReplaceVirtualTable(void* Instance, int Offset, void* RedirectAddress)
{
    void** Table = *reinterpret_cast<void***>(Instance);
    return ReplaceFunctionPointer(&Table[Offset], RedirectAddress);
}

HookResult HookFromSignature(int Token, RorinnnTools::Memory::SigScanner& Scanner, std::string_view Signature, void* RedirectAddress)
{
    void* Address = reinterpret_cast<void*>(Scanner.ScanText(Signature));
    return HookFromAddress(Token, Address, RedirectAddress);
}

HookResult HookFromSymbol(int Token, const wchar_t* ModuleName, const char* SymbolName, void* RedirectAddress)
{
    HMODULE Module = GetModuleHandleW(ModuleName);
    if (!Module)
        return MakeHookResult(VehHookStatus::InvalidArgument, nullptr);

    void* Address = reinterpret_cast<void*>(GetProcAddress(Module, SymbolName));
    return HookFromAddress(Token, Address, RedirectAddress);
}

HookResult HookFromImport(const wchar_t* ImportingModuleName, const char* ImportedModuleName, const char* SymbolName, void* RedirectAddress)
{
    HMODULE Module = GetModuleHandleW(ImportingModuleName);
    if (!Module)
        return MakeHookResult(VehHookStatus::InvalidArgument, nullptr);

    void* Slot = FindImportThunk(Module, ImportedModuleName, SymbolName);
    if (!Slot)
        return MakeHookResult(VehHookStatus::InvalidArgument, nullptr);

    void* Original = RorinnnTools::Hook::ReplaceFunctionPointer(reinterpret_cast<void**>(Slot), RedirectAddress);
    return MakeHookResult(Original ? VehHookStatus::Ok : VehHookStatus::WriteFailed, Original);
}

HookResult HookFromImportAddress(int Token, const wchar_t* ImportingModuleName, const char* ImportedModuleName, const char* SymbolName, void* RedirectAddress)
{
    HMODULE Module = GetModuleHandleW(ImportingModuleName);
    if (!Module)
        return MakeHookResult(VehHookStatus::InvalidArgument, nullptr);

    void* Slot = FindImportThunk(Module, ImportedModuleName, SymbolName);
    if (!Slot)
        return MakeHookResult(VehHookStatus::InvalidArgument, nullptr);

    void* Target = *reinterpret_cast<void**>(Slot);
    return HookFromAddress(Token, Target, RedirectAddress);
}

HookResult HookFromFunctionPointer(int Token, void** FunctionPointer, void* RedirectAddress)
{
    if (!FunctionPointer || !*FunctionPointer)
        return MakeHookResult(VehHookStatus::InvalidArgument, nullptr);
    return HookFromAddress(Token, *FunctionPointer, RedirectAddress);
}

HookResult HookFromVirtualTableAddress(int Token, void* Instance, int Offset, void* RedirectAddress)
{
    if (!Instance)
        return MakeHookResult(VehHookStatus::InvalidArgument, nullptr);

    void** Table  = *reinterpret_cast<void***>(Instance);
    void*  Target = Table[Offset];
    return HookFromAddress(Token, Target, RedirectAddress);
}
} // namespace RorinnnTools::Hook
