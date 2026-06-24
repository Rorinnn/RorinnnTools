// PatternScanner.cpp — 当前进程签名扫描工具

module;

#include <Windows.h>

module RorinnnTools;
import std;

namespace RorinnnTools::Memory
{
static bool IsHexDigit(char Ch)
{
    return (Ch >= '0' && Ch <= '9') || (Ch >= 'a' && Ch <= 'f') || (Ch >= 'A' && Ch <= 'F');
}

static std::uint8_t HexValue(char Ch)
{
    if (Ch >= '0' && Ch <= '9')
        return static_cast<std::uint8_t>(Ch - '0');
    if (Ch >= 'a' && Ch <= 'f')
        return static_cast<std::uint8_t>(Ch - 'a' + 10);
    return static_cast<std::uint8_t>(Ch - 'A' + 10);
}

bool ParsePattern(std::string_view Pattern, std::vector<PatternByte>& Bytes)
{
    Bytes.clear();

    for (std::size_t Index = 0; Index < Pattern.size();)
    {
        if (std::isspace(static_cast<unsigned char>(Pattern[Index])))
        {
            Index++;
            continue;
        }

        if (Pattern[Index] == '?' || Pattern[Index] == '*')
        {
            const char WildcardChar = Pattern[Index];
            Bytes.push_back({0, true});
            Index++;
            if (Index < Pattern.size() && Pattern[Index] == WildcardChar)
                Index++;
            continue;
        }

        if (Index + 1 >= Pattern.size() || !IsHexDigit(Pattern[Index]) || !IsHexDigit(Pattern[Index + 1]))
            return false;

        const std::uint8_t Value = static_cast<std::uint8_t>((HexValue(Pattern[Index]) << 4) | HexValue(Pattern[Index + 1]));
        Bytes.push_back({Value, false});
        Index += 2;
    }

    return !Bytes.empty();
}

std::uintptr_t FindPattern(std::uintptr_t Start, std::size_t Size, std::span<const PatternByte> Pattern)
{
    if (!Start || Size == 0 || Pattern.empty() || Pattern.size() > Size)
        return 0;

    const auto* Bytes = reinterpret_cast<const std::uint8_t*>(Start);
    for (std::size_t Offset = 0; Offset <= Size - Pattern.size(); Offset++)
    {
        bool Match = true;
        for (std::size_t Index = 0; Index < Pattern.size(); Index++)
        {
            if (!Pattern[Index].Wildcard && Bytes[Offset + Index] != Pattern[Index].Value)
            {
                Match = false;
                break;
            }
        }
        if (Match)
            return Start + Offset;
    }

    return 0;
}

std::uintptr_t FindPattern(std::uintptr_t Start, std::size_t Size, std::string_view Pattern)
{
    std::vector<PatternByte> Bytes;
    if (!ParsePattern(Pattern, Bytes))
        return 0;
    return FindPattern(Start, Size, Bytes);
}

std::uintptr_t ResolveRelativeAddress(std::uintptr_t InstructionAddress, std::size_t InstructionSize, std::size_t DisplacementOffset)
{
    std::int32_t Displacement = 0;
    if (!ReadValue(InstructionAddress + DisplacementOffset, Displacement))
        return 0;
    return InstructionAddress + InstructionSize + Displacement;
}
} // namespace RorinnnTools::Memory
