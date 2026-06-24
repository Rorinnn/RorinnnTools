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

bool TryReadString(std::uintptr_t Ptr, std::size_t MaxLength, std::string& Value)
{
    Value.clear();
    if (!Ptr)
        return false;
    if (!MaxLength)
        return true;

    std::string Buffer(MaxLength, '\0');
    if (!ReadBytes(Ptr, Buffer.data(), Buffer.size()))
        return false;

    const std::size_t Length = std::char_traits<char>::length(Buffer.c_str());
    Value.assign(Buffer.data(), Length);
    return true;
}

std::string ReadString(std::uintptr_t Ptr, std::size_t MaxLength)
{
    std::string Value;
    TryReadString(Ptr, MaxLength, Value);
    return Value;
}

bool TryReadCString(std::uintptr_t Ptr, std::string& Value, std::size_t ChunkSize, std::size_t MaxLength)
{
    Value.clear();
    if (!Ptr)
        return false;
    if (!MaxLength)
        return true;
    if (!ChunkSize)
        ChunkSize = 256;

    std::vector<char> Buffer(ChunkSize);
    std::uintptr_t    Current = Ptr;
    while (Value.size() < MaxLength)
    {
        const std::size_t Remaining = MaxLength - Value.size();
        const std::size_t ToRead    = std::min(ChunkSize, Remaining);
        if (!ReadBytes(Current, Buffer.data(), ToRead))
            return false;

        const auto End = std::find(Buffer.begin(), Buffer.begin() + static_cast<std::ptrdiff_t>(ToRead), '\0');
        Value.append(Buffer.data(), static_cast<std::size_t>(End - Buffer.begin()));
        if (End != Buffer.begin() + static_cast<std::ptrdiff_t>(ToRead))
            return true;
        Current += ToRead;
    }

    return false;
}

std::string ReadCString(std::uintptr_t Ptr, std::size_t ChunkSize, std::size_t MaxLength)
{
    std::string Value;
    TryReadCString(Ptr, Value, ChunkSize, MaxLength);
    return Value;
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
