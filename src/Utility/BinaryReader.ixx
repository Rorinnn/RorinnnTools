module;

export module RnTools:BinaryReader;
import std;

export namespace RnTools
{
class BinaryReader
{
  public:
    BinaryReader(const std::uint8_t* PData, std::size_t Size, std::size_t Offset = 0);

    std::size_t GetOffset() const;
    std::size_t GetSize() const;
    std::size_t GetRemaining() const;
    bool        IsEnd() const;
    bool        IsValid() const;

    bool SetOffset(std::size_t NewOffset);

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

    bool ReadBytes(void* PBuffer, std::size_t Count);
    bool ReadString(std::string& Value);

  private:
    bool CanRead(std::size_t Count) const;

    const std::uint8_t* PData  = nullptr;
    std::size_t         Size   = 0;
    std::size_t         Offset = 0;
};
} // namespace RnTools
