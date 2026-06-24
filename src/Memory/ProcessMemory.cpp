// ProcessMemory.cpp — 当前进程内存读写工具

module;

#include <Windows.h>
#include <psapi.h>

module RorinnnTools;
import std;

namespace RorinnnTools::Memory
{
static constexpr std::uintptr_t PageSize = 0x1000;

bool IsReadablePtr(std::uintptr_t Ptr)
{
    return Ptr != 0;
}

bool IsReadableRange(std::uintptr_t Ptr, std::size_t Size)
{
    std::uintptr_t Address = Ptr;
    if (!Address || Size == 0)
        return false;

    std::uintptr_t StartAddress  = Address;
    std::uintptr_t EndAddress    = Address + Size - 1;
    std::uintptr_t StartPageBase = StartAddress & ~(PageSize - 1);
    std::uintptr_t EndPageBase   = EndAddress & ~(PageSize - 1);

    for (std::uintptr_t PageBase = StartPageBase; PageBase <= EndPageBase; PageBase += PageSize)
    {
        PSAPI_WORKING_SET_EX_INFORMATION Info = {};
        Info.VirtualAddress                   = reinterpret_cast<PVOID>(PageBase);

        if (!QueryWorkingSetEx(GetCurrentProcess(), &Info, sizeof(Info)))
            return false;

        if (Info.VirtualAttributes.Valid == 0)
            return false;
    }
    return true;
}

bool ReadBytes(std::uintptr_t Ptr, void* PBuffer, std::size_t Size)
{
    if (!Ptr || !PBuffer || Size == 0)
        return false;

    if (!IsReadableRange(Ptr, Size))
        return false;

    __try
    {
        std::memcpy(PBuffer, (const void*)Ptr, Size);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool WriteBytes(std::uintptr_t Ptr, const void* PBuffer, std::size_t Size)
{
    if (!Ptr || !PBuffer || Size == 0)
        return false;

    DWORD OldProtect = 0;
    if (!VirtualProtect(reinterpret_cast<void*>(Ptr), Size, PAGE_EXECUTE_READWRITE, &OldProtect))
        return false;

    bool Written = false;
    __try
    {
        std::memcpy(reinterpret_cast<void*>(Ptr), PBuffer, Size);
        FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(Ptr), Size);
        Written = true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Written = false;
    }

    DWORD Ignored = 0;
    VirtualProtect(reinterpret_cast<void*>(Ptr), Size, OldProtect, &Ignored);
    return Written;
}

bool ReadPtr(std::uintptr_t Ptr, std::uintptr_t& Value)
{
    return ReadValue(Ptr, Value) && IsReadablePtr(Value);
}

bool ResolvePointerChain(std::uintptr_t Base, std::span<const std::ptrdiff_t> Offsets, std::uintptr_t& Address)
{
    if (!Base)
        return false;

    std::uintptr_t Current = Base;
    for (std::size_t Index = 0; Index < Offsets.size(); Index++)
    {
        Current += Offsets[Index];
        if (Index + 1 < Offsets.size() && !ReadPtr(Current, Current))
            return false;
    }

    Address = Current;
    return true;
}
} // namespace RorinnnTools::Memory
