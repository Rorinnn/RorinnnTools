module;

module RnTools;
import std;

namespace RnTools::Memory
{
bool MemoryRange::IsValid() const
{
    return Start && Size;
}

std::uintptr_t MemoryRange::End() const
{
    return Start + Size;
}

namespace MemoryHelper
{
std::vector<std::uint8_t> ReadRaw(std::uintptr_t Ptr, std::size_t Size)
{
    std::vector<std::uint8_t> Bytes(Size);
    if (Size)
        std::memcpy(Bytes.data(), reinterpret_cast<const void*>(Ptr), Size);
    return Bytes;
}

void WriteRaw(std::uintptr_t Ptr, std::span<const std::uint8_t> Bytes)
{
    if (!Bytes.empty())
        std::memcpy(reinterpret_cast<void*>(Ptr), Bytes.data(), Bytes.size());
}

bool TryReadRaw(std::uintptr_t Ptr, std::size_t Size, std::vector<std::uint8_t>& Bytes)
{
    Bytes.assign(Size, 0);
    if (!Size)
        return true;
    return ReadBytes(Ptr, Bytes.data(), Bytes.size());
}

bool TryWriteRaw(std::uintptr_t Ptr, std::span<const std::uint8_t> Bytes)
{
    if (Bytes.empty())
        return true;
    return WriteBytes(Ptr, Bytes.data(), Bytes.size());
}

bool TryReadString(std::uintptr_t Ptr, std::size_t MaxLength, std::string& Value)
{
    return RnTools::Memory::TryReadString(Ptr, MaxLength, Value);
}

std::string ReadString(std::uintptr_t Ptr, std::size_t MaxLength)
{
    return RnTools::Memory::ReadString(Ptr, MaxLength);
}

bool TryReadCString(std::uintptr_t Ptr, std::string& Value, std::size_t ChunkSize, std::size_t MaxLength)
{
    return RnTools::Memory::TryReadCString(Ptr, Value, ChunkSize, MaxLength);
}

std::string ReadCString(std::uintptr_t Ptr, std::size_t ChunkSize, std::size_t MaxLength)
{
    return RnTools::Memory::ReadCString(Ptr, ChunkSize, MaxLength);
}
} // namespace MemoryHelper
} // namespace RnTools::Memory
