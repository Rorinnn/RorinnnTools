module;

export module RorinnnTools:Text;
import std;

export namespace RorinnnTools::Text
{
struct TextSlice
{
    const char* Start  = nullptr;
    std::size_t Length = 0;

    bool             IsEmpty() const { return Length == 0; }
    std::string      ToString() const { return Start ? std::string(Start, Length) : std::string(); }
    std::string_view View() const { return Start ? std::string_view(Start, Length) : std::string_view(); }
};

inline bool StartsWith(std::string_view Text, std::string_view Prefix)
{
    return Text.size() >= Prefix.size() && Text.substr(0, Prefix.size()) == Prefix;
}

inline bool EndsWith(std::string_view Text, std::string_view Suffix)
{
    return Text.size() >= Suffix.size() && Text.substr(Text.size() - Suffix.size()) == Suffix;
}

inline bool Equals(TextSlice Slice, std::string_view Text)
{
    return Slice.View() == Text;
}

inline bool ContainsInsensitive(std::string_view Text, std::string_view Pattern)
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

inline void SplitLines(std::string_view Text, std::vector<TextSlice>& Lines, bool KeepEmpty = false)
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

inline void SplitFields(std::string_view Text, char Separator, std::vector<TextSlice>& Fields)
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

inline std::uint32_t ParseUInt32(TextSlice Slice)
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

inline float ParseFloat(TextSlice Slice)
{
    const std::string Text = Slice.ToString();
    return Text.empty() ? 0.0f : static_cast<float>(std::strtod(Text.c_str(), nullptr));
}

inline void AppendJsonString(std::string& Json, std::string_view Text)
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
} // namespace RorinnnTools::Text
