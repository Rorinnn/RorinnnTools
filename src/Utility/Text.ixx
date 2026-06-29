module;

export module RnTools:Text;
import std;

export namespace RnTools::Text
{
struct TextSlice
{
    const char* Start  = nullptr;
    std::size_t Length = 0;

    bool             IsEmpty() const;
    std::string      ToString() const;
    std::string_view View() const;
};

bool StartsWith(std::string_view Text, std::string_view Prefix);
bool EndsWith(std::string_view Text, std::string_view Suffix);
bool Equals(TextSlice Slice, std::string_view Text);
bool ContainsAsciiIgnoreCase(std::string_view Text, std::string_view Pattern);

std::string_view TrimView(std::string_view Text);
std::string      Trim(std::string_view Text);
bool             EndsWithNewline(std::string_view Text);

std::string ToUtf8(std::wstring_view Text);
std::string ToUtf8(const wchar_t* PText);

void SplitLines(std::string_view Text, std::vector<TextSlice>& Lines, bool KeepEmpty = false);
void SplitFields(std::string_view Text, char Separator, std::vector<TextSlice>& Fields);

std::uint32_t ParseUInt32(TextSlice Slice);
int           ParseInt32(TextSlice Slice);
float         ParseFloat(TextSlice Slice);
void          CopyCString(char* PBuffer, std::size_t BufferSize, const char* PText);

std::string FormatU64(std::uint64_t Value);
std::string FormatSize(std::size_t Value);
std::string FormatHex(std::uint64_t Value);
std::string FormatAddress(std::uintptr_t Address);
std::string FormatLocalTimeMs(std::uint64_t UnixMs);

void AppendJsonString(std::string& Json, std::string_view Text);
void AppendJsonName(std::string& Json, std::string_view Name);
void AppendJsonName(std::string& Json, bool& NeedsComma, std::string_view Name);
void AppendJsonField(std::string& Json, std::string_view Name, std::string_view Value, bool AddComma = true);
void AppendJsonField(std::string& Json, bool& NeedsComma, std::string_view Name, std::string_view Value);
void AppendJsonU64Field(std::string& Json, std::string_view Name, std::uint64_t Value, bool AddComma = true);
void AppendJsonU64Field(std::string& Json, bool& NeedsComma, std::string_view Name, std::uint64_t Value);
void AppendJsonBoolField(std::string& Json, std::string_view Name, bool Value, bool AddComma = true);
void AppendJsonBoolField(std::string& Json, bool& NeedsComma, std::string_view Name, bool Value);
void AppendJsonHexField(std::string& Json, std::string_view Name, std::uint64_t Value, bool AddComma = true);
void AppendJsonHexField(std::string& Json, bool& NeedsComma, std::string_view Name, std::uint64_t Value);
} // namespace RnTools::Text
