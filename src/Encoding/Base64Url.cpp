// Base64Url.cpp — Base64Url 编解码工具

module;

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

module RorinnnTools;

namespace RorinnnTools::Encoding
{

std::string Base64UrlEncode(const std::vector<uint8_t>& Bytes)
{
    static constexpr char Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    std::string Result;
    Result.reserve(((Bytes.size() + 2) / 3) * 4);

    size_t i = 0;
    while (i + 3 <= Bytes.size())
    {
        uint32_t Value = (static_cast<uint32_t>(Bytes[i]) << 16) | (static_cast<uint32_t>(Bytes[i + 1]) << 8) |
                         static_cast<uint32_t>(Bytes[i + 2]);
        Result.push_back(Alphabet[(Value >> 18) & 0x3F]);
        Result.push_back(Alphabet[(Value >> 12) & 0x3F]);
        Result.push_back(Alphabet[(Value >> 6) & 0x3F]);
        Result.push_back(Alphabet[Value & 0x3F]);
        i += 3;
    }

    size_t Remaining = Bytes.size() - i;
    if (Remaining == 1)
    {
        uint32_t Value = static_cast<uint32_t>(Bytes[i]) << 16;
        Result.push_back(Alphabet[(Value >> 18) & 0x3F]);
        Result.push_back(Alphabet[(Value >> 12) & 0x3F]);
    }
    else if (Remaining == 2)
    {
        uint32_t Value = (static_cast<uint32_t>(Bytes[i]) << 16) | (static_cast<uint32_t>(Bytes[i + 1]) << 8);
        Result.push_back(Alphabet[(Value >> 18) & 0x3F]);
        Result.push_back(Alphabet[(Value >> 12) & 0x3F]);
        Result.push_back(Alphabet[(Value >> 6) & 0x3F]);
    }

    return Result;
}

std::string Base64UrlEncode(std::string_view Text)
{
    std::vector<uint8_t> Bytes(Text.begin(), Text.end());
    return Base64UrlEncode(Bytes);
}

static int DecodeBase64UrlChar(char Char)
{
    if (Char >= 'A' && Char <= 'Z') return Char - 'A';
    if (Char >= 'a' && Char <= 'z') return Char - 'a' + 26;
    if (Char >= '0' && Char <= '9') return Char - '0' + 52;
    if (Char == '-') return 62;
    if (Char == '_') return 63;

    return -1;
}

bool Base64UrlDecode(std::string_view Text, std::vector<uint8_t>& Bytes)
{
    Bytes.clear();
    if (Text.size() % 4 == 1)
    {
        return false;
    }

    Bytes.reserve((Text.size() * 3) / 4);

    uint32_t Buffer = 0;
    int      Bits   = 0;

    for (char Char : Text)
    {
        int Value = DecodeBase64UrlChar(Char);
        if (Value < 0)
        {
            Bytes.clear();
            return false;
        }

        Buffer = (Buffer << 6) | static_cast<uint32_t>(Value);
        Bits += 6;

        if (Bits >= 8)
        {
            Bits -= 8;
            Bytes.push_back(static_cast<uint8_t>((Buffer >> Bits) & 0xFF));
        }
    }

    return true;
}

} // namespace RorinnnTools::Encoding
