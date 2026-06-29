module;

#include <Windows.h>

module RnTools;
import std;

namespace RnTools::Text
{
bool TextSlice::IsEmpty() const
{
    return Length == 0;
}

std::string TextSlice::ToString() const
{
    return Start ? std::string(Start, Length) : std::string();
}

std::string_view TextSlice::View() const
{
    return Start ? std::string_view(Start, Length) : std::string_view();
}

bool StartsWith(std::string_view Text, std::string_view Prefix)
{
    return Text.size() >= Prefix.size() && Text.substr(0, Prefix.size()) == Prefix;
}

bool EndsWith(std::string_view Text, std::string_view Suffix)
{
    return Text.size() >= Suffix.size() && Text.substr(Text.size() - Suffix.size()) == Suffix;
}

bool Equals(TextSlice Slice, std::string_view Text)
{
    return Slice.View() == Text;
}

bool ContainsAsciiIgnoreCase(std::string_view Text, std::string_view Pattern)
{
    if (Pattern.empty())
        return true;
    if (Text.size() < Pattern.size())
        return false;

    const std::size_t LastStart = Text.size() - Pattern.size();
    for (std::size_t Start = 0; Start <= LastStart; Start++)
    {
        std::size_t Index = 0;
        while (Index < Pattern.size() && std::tolower(static_cast<unsigned char>(Text[Start + Index])) ==
                                             std::tolower(static_cast<unsigned char>(Pattern[Index])))
            Index++;
        if (Index == Pattern.size())
            return true;
    }
    return false;
}

std::string_view TrimView(std::string_view Text)
{
    auto IsSpace = [](unsigned char Ch)
    { return std::isspace(Ch) != 0; };

    while (!Text.empty() && IsSpace(static_cast<unsigned char>(Text.front())))
        Text.remove_prefix(1);
    while (!Text.empty() && IsSpace(static_cast<unsigned char>(Text.back())))
        Text.remove_suffix(1);
    return Text;
}

std::string Trim(std::string_view Text)
{
    const std::string_view Trimmed = TrimView(Text);
    return std::string(Trimmed.data(), Trimmed.size());
}

bool EndsWithNewline(std::string_view Text)
{
    return Text.empty() || Text.back() == '\n' || Text.back() == '\r';
}

std::string ToUtf8(std::wstring_view Text)
{
    if (Text.empty())
        return {};

    const int Size = WideCharToMultiByte(CP_UTF8,
                                         0,
                                         Text.data(),
                                         static_cast<int>(Text.size()),
                                         nullptr,
                                         0,
                                         nullptr,
                                         nullptr);
    if (Size <= 0)
        return {};

    std::string Result(static_cast<std::size_t>(Size), '\0');
    WideCharToMultiByte(CP_UTF8,
                        0,
                        Text.data(),
                        static_cast<int>(Text.size()),
                        Result.data(),
                        Size,
                        nullptr,
                        nullptr);
    return Result;
}

std::string ToUtf8(const wchar_t* PText)
{
    return PText ? ToUtf8(std::wstring_view(PText)) : std::string();
}

void SplitLines(std::string_view Text, std::vector<TextSlice>& Lines, bool KeepEmpty)
{
    Lines.clear();
    const char* Data      = Text.data();
    std::size_t LineStart = 0;
    for (std::size_t Index = 0; Index <= Text.size(); Index++)
    {
        if (Index != Text.size() && Data[Index] != '\n')
            continue;

        std::size_t LineEnd = Index;
        if (LineEnd > LineStart && Data[LineEnd - 1] == '\r')
            LineEnd--;
        if (KeepEmpty || LineEnd > LineStart)
            Lines.push_back({Data + LineStart, LineEnd - LineStart});
        LineStart = Index + 1;
    }
}

void SplitFields(std::string_view Text, char Separator, std::vector<TextSlice>& Fields)
{
    Fields.clear();
    const char* Data       = Text.data();
    std::size_t FieldStart = 0;
    for (std::size_t Index = 0; Index <= Text.size(); Index++)
    {
        if (Index != Text.size() && Data[Index] != Separator)
            continue;
        Fields.push_back({Data + FieldStart, Index - FieldStart});
        FieldStart = Index + 1;
    }
}

std::uint32_t ParseUInt32(TextSlice Slice)
{
    std::uint32_t Value = 0;
    for (char Ch : Slice.View())
    {
        if (Ch < '0' || Ch > '9')
            break;
        Value = Value * 10u + static_cast<std::uint32_t>(Ch - '0');
    }
    return Value;
}

int ParseInt32(TextSlice Slice)
{
    const std::string Text = Slice.ToString();
    return Text.empty() ? 0 : static_cast<int>(std::strtol(Text.c_str(), nullptr, 10));
}

float ParseFloat(TextSlice Slice)
{
    const std::string Text = Slice.ToString();
    return Text.empty() ? 0.0f : static_cast<float>(std::strtod(Text.c_str(), nullptr));
}

void CopyCString(char* PBuffer, std::size_t BufferSize, const char* PText)
{
    if (!PBuffer || BufferSize == 0)
        return;

    const char* Source = PText ? PText : "";
    std::size_t Length = std::strlen(Source);
    if (Length >= BufferSize)
        Length = BufferSize - 1;
    std::memcpy(PBuffer, Source, Length);
    PBuffer[Length] = '\0';
}

std::string FormatU64(std::uint64_t Value)
{
    char Buffer[32] = {};
    std::snprintf(Buffer, sizeof(Buffer), "%llu", static_cast<unsigned long long>(Value));
    return Buffer;
}

std::string FormatSize(std::size_t Value)
{
    char Buffer[32] = {};
    std::snprintf(Buffer, sizeof(Buffer), "%zu", Value);
    return Buffer;
}

std::string FormatHex(std::uint64_t Value)
{
    char Buffer[32] = {};
    std::snprintf(Buffer, sizeof(Buffer), "0x%llX", static_cast<unsigned long long>(Value));
    return Buffer;
}

std::string FormatAddress(std::uintptr_t Address)
{
    return FormatHex(static_cast<std::uint64_t>(Address));
}

std::string FormatLocalTimeMs(std::uint64_t UnixMs)
{
    if (UnixMs == 0)
        return {};

    const std::uint64_t FileTimeValue = (UnixMs + 11644473600000ULL) * 10000ULL;
    FILETIME            FileTime      = {};
    FILETIME            LocalFileTime = {};
    SYSTEMTIME          Time          = {};
    FileTime.dwLowDateTime           = static_cast<DWORD>(FileTimeValue & 0xFFFFFFFFULL);
    FileTime.dwHighDateTime          = static_cast<DWORD>(FileTimeValue >> 32);
    if (!FileTimeToLocalFileTime(&FileTime, &LocalFileTime) || !FileTimeToSystemTime(&LocalFileTime, &Time))
        return FormatU64(UnixMs);

    char Buffer[32] = {};
    std::snprintf(Buffer,
                  sizeof(Buffer),
                  "%02u:%02u:%02u.%03u",
                  Time.wHour,
                  Time.wMinute,
                  Time.wSecond,
                  Time.wMilliseconds);
    return Buffer;
}

void AppendJsonString(std::string& Json, std::string_view Text)
{
    Json.push_back('"');
    for (char Ch : Text)
    {
        switch (Ch)
        {
            case '\\':
                Json += "\\\\";
                break;
            case '"':
                Json += "\\\"";
                break;
            case '\b':
                Json += "\\b";
                break;
            case '\f':
                Json += "\\f";
                break;
            case '\n':
                Json += "\\n";
                break;
            case '\r':
                Json += "\\r";
                break;
            case '\t':
                Json += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(Ch) < 0x20)
                {
                    char Buffer[8] = {};
                    std::snprintf(Buffer, sizeof(Buffer), "\\u%04X", static_cast<unsigned char>(Ch));
                    Json += Buffer;
                }
                else
                {
                    Json.push_back(Ch);
                }
                break;
        }
    }
    Json.push_back('"');
}

void AppendJsonName(std::string& Json, std::string_view Name)
{
    AppendJsonString(Json, Name);
    Json.push_back(':');
}

void AppendJsonName(std::string& Json, bool& NeedsComma, std::string_view Name)
{
    if (NeedsComma)
        Json.push_back(',');
    AppendJsonName(Json, Name);
    NeedsComma = true;
}

void AppendJsonField(std::string& Json, std::string_view Name, std::string_view Value, bool AddComma)
{
    AppendJsonName(Json, Name);
    AppendJsonString(Json, Value);
    if (AddComma)
        Json.push_back(',');
}

void AppendJsonField(std::string& Json, bool& NeedsComma, std::string_view Name, std::string_view Value)
{
    AppendJsonName(Json, NeedsComma, Name);
    AppendJsonString(Json, Value);
}

void AppendJsonU64Field(std::string& Json, std::string_view Name, std::uint64_t Value, bool AddComma)
{
    AppendJsonName(Json, Name);
    Json += FormatU64(Value);
    if (AddComma)
        Json.push_back(',');
}

void AppendJsonU64Field(std::string& Json, bool& NeedsComma, std::string_view Name, std::uint64_t Value)
{
    AppendJsonName(Json, NeedsComma, Name);
    Json += FormatU64(Value);
}

void AppendJsonBoolField(std::string& Json, std::string_view Name, bool Value, bool AddComma)
{
    AppendJsonName(Json, Name);
    Json += Value ? "true" : "false";
    if (AddComma)
        Json.push_back(',');
}

void AppendJsonBoolField(std::string& Json, bool& NeedsComma, std::string_view Name, bool Value)
{
    AppendJsonName(Json, NeedsComma, Name);
    Json += Value ? "true" : "false";
}

void AppendJsonHexField(std::string& Json, std::string_view Name, std::uint64_t Value, bool AddComma)
{
    AppendJsonField(Json, Name, FormatHex(Value), AddComma);
}

void AppendJsonHexField(std::string& Json, bool& NeedsComma, std::string_view Name, std::uint64_t Value)
{
    AppendJsonField(Json, NeedsComma, Name, FormatHex(Value));
}
} // namespace RnTools::Text
