// ProcessMemory.cpp — 当前进程内存读写工具

module;

#include <Windows.h>

module RorinnnTools;
import std;

namespace RorinnnTools::Memory
{
bool ReadBytes(std::uintptr_t Ptr, void* PBuffer, std::size_t Size)
{
    if (Size == 0)
        return true;
    if (!Ptr || !PBuffer)
        return false;

    SIZE_T BytesRead = 0;
    return ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(Ptr), PBuffer, Size, &BytesRead) && BytesRead == Size;
}

bool WriteBytes(std::uintptr_t Ptr, const void* PBuffer, std::size_t Size)
{
    if (Size == 0)
        return true;
    if (!Ptr || !PBuffer)
        return false;

    SIZE_T BytesWritten = 0;
    return WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(Ptr), PBuffer, Size, &BytesWritten) && BytesWritten == Size;
}

bool ReadPtr(std::uintptr_t Ptr, std::uintptr_t& Value)
{
    return ReadValue(Ptr, Value);
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
