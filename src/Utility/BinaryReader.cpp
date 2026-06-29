module;

module RnTools;
import std;

namespace RnTools
{
BinaryReader::BinaryReader(const std::uint8_t* PData, std::size_t Size, std::size_t Offset) : PData(PData), Size(Size), Offset(Offset) {}

std::size_t BinaryReader::GetOffset() const
{
    return Offset;
}

std::size_t BinaryReader::GetSize() const
{
    return Size;
}

std::size_t BinaryReader::GetRemaining() const
{
    return Offset <= Size ? Size - Offset : 0;
}

bool BinaryReader::IsEnd() const
{
    return Offset == Size;
}

bool BinaryReader::IsValid() const
{
    return PData || Size == 0;
}

bool BinaryReader::SetOffset(std::size_t NewOffset)
{
    if (NewOffset > Size)
        return false;
    Offset = NewOffset;
    return true;
}

bool BinaryReader::ReadBytes(void* PBuffer, std::size_t Count)
{
    if (!PBuffer && Count)
        return false;
    if (!CanRead(Count))
        return false;

    if (Count)
        std::memcpy(PBuffer, PData + Offset, Count);
    Offset += Count;
    return true;
}

bool BinaryReader::ReadString(std::string& Value)
{
    std::uint32_t     Length      = 0;
    const std::size_t StartOffset = Offset;
    if (!Read(Length) || !CanRead(Length))
    {
        Offset = StartOffset;
        return false;
    }

    Value.assign(reinterpret_cast<const char*>(PData + Offset), Length);
    Offset += Length;
    return true;
}

bool BinaryReader::CanRead(std::size_t Count) const
{
    return IsValid() && Offset <= Size && Count <= Size - Offset;
}
} // namespace RnTools
