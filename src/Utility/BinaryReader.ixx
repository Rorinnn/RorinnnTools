module;

export module RorinnnTools:BinaryReader;
import std;

export namespace RorinnnTools
{
class BinaryReader
{
  public:
    BinaryReader(const std::uint8_t* PData, std::size_t Size, std::size_t Offset = 0) : PData(PData), Size(Size), Offset(Offset) {}

    std::size_t GetOffset() const { return Offset; }
    std::size_t GetSize() const { return Size; }
    std::size_t GetRemaining() const { return Offset <= Size ? Size - Offset : 0; }
    bool        IsEnd() const { return Offset == Size; }
    bool        IsValid() const { return PData || Size == 0; }

    bool SetOffset(std::size_t NewOffset)
    {
        if (NewOffset > Size)
            return false;
        Offset = NewOffset;
        return true;
    }

    template <typename T>
    bool Read(T& Value)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        if (!CanRead(sizeof(T)))
            return false;

        std::memcpy(&Value, PData + Offset, sizeof(T));
        Offset += sizeof(T);
        return true;
    }

    bool ReadBytes(void* PBuffer, std::size_t Count)
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

    bool ReadString(std::string& Value)
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

  private:
    bool CanRead(std::size_t Count) const
    {
        return IsValid() && Offset <= Size && Count <= Size - Offset;
    }

    const std::uint8_t* PData  = nullptr;
    std::size_t         Size   = 0;
    std::size_t         Offset = 0;
};
} // namespace RorinnnTools
