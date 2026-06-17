// ProcessMemory.cpp — 当前进程内存安全读取工具

module;

#include <Windows.h>
#include <psapi.h>

module RorinnnTools;

namespace RorinnnTools::Memory
{
static constexpr uintptr_t PageSize = 0x1000;

bool IsReadablePtr(uintptr_t Ptr)
{
    return Ptr != 0;
}

bool IsReadableRange(uintptr_t Ptr, size_t Size)
{
    uintptr_t Address = Ptr;
    if (!Address || Size == 0)
    {
        return false;
    }

    uintptr_t StartAddress  = Address;
    uintptr_t EndAddress    = Address + Size - 1;
    uintptr_t StartPageBase = StartAddress & ~(PageSize - 1);
    uintptr_t EndPageBase   = EndAddress & ~(PageSize - 1);

    for (uintptr_t PageBase = StartPageBase; PageBase <= EndPageBase; PageBase += PageSize)
    {
        PSAPI_WORKING_SET_EX_INFORMATION Info = {};
        Info.VirtualAddress                   = reinterpret_cast<PVOID>(PageBase);

        if (!QueryWorkingSetEx(GetCurrentProcess(), &Info, sizeof(Info)))
        {
            return false;
        }

        if (Info.VirtualAttributes.Valid == 0)
        {
            return false;
        }
    }
    return true;
}

bool ReadBytes(uintptr_t Ptr, void* PBuffer, size_t Size)
{
    if (!Ptr || !PBuffer || Size == 0)
    {
        return false;
    }

    if (!IsReadableRange(Ptr, Size))
    {
        return false;
    }

    __try
    {
        memcpy(PBuffer, (const void*)Ptr, Size);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool ReadPtr(uintptr_t Ptr, uintptr_t& Value)
{
    return ReadValue(Ptr, Value) && IsReadablePtr(Value);
}
} // namespace RorinnnTools::Memory
