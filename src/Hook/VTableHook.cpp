// VTableHook.cpp — 虚表函数替换工具

module;

#include <Windows.h>

module RorinnnTools;

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

void* RorinnnTools::Hook::VTable::Hook(void* Instance, void* HookFn, int Offset)
{
    void** Table = *reinterpret_cast<void***>(Instance);
    return HookSlot(&Table[Offset], HookFn);
}

void* RorinnnTools::Hook::VTable::HookSlot(void** Slot, void* HookFn)
{
    void* Original = *Slot;

    int OriginalProtection = Unprotect(Slot);
    *Slot                  = HookFn;
    Protect(Slot, OriginalProtection);

    return Original;
}
