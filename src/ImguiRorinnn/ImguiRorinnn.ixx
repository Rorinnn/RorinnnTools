module;

#include <Windows.h>
#include <d3d11.h>
#include <imgui.h>

export module RnTools:ImguiRorinnn;
import std;

export namespace RnTools::ImguiRorinnn
{

enum class Ease
{
    Linear,
    OutQuad,
    OutCubic,
    InOutCubic,
    OutBack,
};

enum class Icon : std::uint32_t
{
    None            = 0,
    Check           = 0xF00C,
    Xmark           = 0xF00D,
    CaretDown       = 0xF0D7,
    CaretRight      = 0xF0DA,
    Gear            = 0xF013,
    Wrench          = 0xF0AD,
    Bug             = 0xF188,
    Play            = 0xF04B,
    Stop            = 0xF04D,
    Minus           = 0xF068,
    RotateRight     = 0xF2F9,
    WindowRestore   = 0xF2D2,
    CircleQuestion  = 0xF059,
    CircleInfo      = 0xF05A,
    Copy            = 0xF0C5,
    Download        = 0xF019,
    Upload          = 0xF093,
    FolderOpen      = 0xF07C,
    FloppyDisk      = 0xF0C7,
    Trash           = 0xF1F8,
    MagnifyingGlass = 0xF002,
    Link            = 0xF0C1,
    Discord         = 0xF392,
    Github          = 0xF09B,
};

enum class ButtonVariant
{
    Secondary,
    Primary,
    Ghost,
    Danger,
};

enum class StatusKind
{
    Neutral,
    Success,
    Warning,
    Danger,
};

enum class BadgeVariant
{
    Neutral,
    Accent,
    Success,
    Warning,
    Danger,
};

enum class InfoSeverity
{
    Info,
    Success,
    Warning,
    Danger,
};

namespace KnownColor
{
extern const ImVec4 ActiveBorder;
extern const ImVec4 ActiveCaption;
extern const ImVec4 ActiveCaptionText;
extern const ImVec4 AppWorkspace;
extern const ImVec4 Control;
extern const ImVec4 ControlDark;
extern const ImVec4 ControlDarkDark;
extern const ImVec4 ControlLight;
extern const ImVec4 ControlLightLight;
extern const ImVec4 ControlText;
extern const ImVec4 Desktop;
extern const ImVec4 GrayText;
extern const ImVec4 Highlight;
extern const ImVec4 HighlightText;
extern const ImVec4 HotTrack;
extern const ImVec4 InactiveBorder;
extern const ImVec4 InactiveCaption;
extern const ImVec4 InactiveCaptionText;
extern const ImVec4 Info;
extern const ImVec4 InfoText;
extern const ImVec4 Menu;
extern const ImVec4 MenuText;
extern const ImVec4 ScrollBar;
extern const ImVec4 Window;
extern const ImVec4 WindowFrame;
extern const ImVec4 WindowText;
extern const ImVec4 Transparent;
extern const ImVec4 AliceBlue;
extern const ImVec4 AntiqueWhite;
extern const ImVec4 Aqua;
extern const ImVec4 Aquamarine;
extern const ImVec4 Azure;
extern const ImVec4 Beige;
extern const ImVec4 Bisque;
extern const ImVec4 Black;
extern const ImVec4 BlanchedAlmond;
extern const ImVec4 Blue;
extern const ImVec4 BlueViolet;
extern const ImVec4 Brown;
extern const ImVec4 BurlyWood;
extern const ImVec4 CadetBlue;
extern const ImVec4 Chartreuse;
extern const ImVec4 Chocolate;
extern const ImVec4 Coral;
extern const ImVec4 CornflowerBlue;
extern const ImVec4 Cornsilk;
extern const ImVec4 Crimson;
extern const ImVec4 Cyan;
extern const ImVec4 DarkBlue;
extern const ImVec4 DarkCyan;
extern const ImVec4 DarkGoldenrod;
extern const ImVec4 DarkGray;
extern const ImVec4 DarkGreen;
extern const ImVec4 DarkKhaki;
extern const ImVec4 DarkMagenta;
extern const ImVec4 DarkOliveGreen;
extern const ImVec4 DarkOrange;
extern const ImVec4 DarkOrchid;
extern const ImVec4 DarkRed;
extern const ImVec4 DarkSalmon;
extern const ImVec4 DarkSeaGreen;
extern const ImVec4 DarkSlateBlue;
extern const ImVec4 DarkSlateGray;
extern const ImVec4 DarkTurquoise;
extern const ImVec4 DarkViolet;
extern const ImVec4 DeepPink;
extern const ImVec4 DeepSkyBlue;
extern const ImVec4 DimGray;
extern const ImVec4 DodgerBlue;
extern const ImVec4 Firebrick;
extern const ImVec4 FloralWhite;
extern const ImVec4 ForestGreen;
extern const ImVec4 Fuchsia;
extern const ImVec4 Gainsboro;
extern const ImVec4 GhostWhite;
extern const ImVec4 Gold;
extern const ImVec4 Goldenrod;
extern const ImVec4 Gray;
extern const ImVec4 Green;
extern const ImVec4 GreenYellow;
extern const ImVec4 Honeydew;
extern const ImVec4 HotPink;
extern const ImVec4 IndianRed;
extern const ImVec4 Indigo;
extern const ImVec4 Ivory;
extern const ImVec4 Khaki;
extern const ImVec4 Lavender;
extern const ImVec4 LavenderBlush;
extern const ImVec4 LawnGreen;
extern const ImVec4 LemonChiffon;
extern const ImVec4 LightBlue;
extern const ImVec4 LightCoral;
extern const ImVec4 LightCyan;
extern const ImVec4 LightGoldenrodYellow;
extern const ImVec4 LightGray;
extern const ImVec4 LightGreen;
extern const ImVec4 LightPink;
extern const ImVec4 LightSalmon;
extern const ImVec4 LightSeaGreen;
extern const ImVec4 LightSkyBlue;
extern const ImVec4 LightSlateGray;
extern const ImVec4 LightSteelBlue;
extern const ImVec4 LightYellow;
extern const ImVec4 Lime;
extern const ImVec4 LimeGreen;
extern const ImVec4 Linen;
extern const ImVec4 Magenta;
extern const ImVec4 Maroon;
extern const ImVec4 MediumAquamarine;
extern const ImVec4 MediumBlue;
extern const ImVec4 MediumOrchid;
extern const ImVec4 MediumPurple;
extern const ImVec4 MediumSeaGreen;
extern const ImVec4 MediumSlateBlue;
extern const ImVec4 MediumSpringGreen;
extern const ImVec4 MediumTurquoise;
extern const ImVec4 MediumVioletRed;
extern const ImVec4 MidnightBlue;
extern const ImVec4 MintCream;
extern const ImVec4 MistyRose;
extern const ImVec4 Moccasin;
extern const ImVec4 NavajoWhite;
extern const ImVec4 Navy;
extern const ImVec4 OldLace;
extern const ImVec4 Olive;
extern const ImVec4 OliveDrab;
extern const ImVec4 Orange;
extern const ImVec4 OrangeRed;
extern const ImVec4 Orchid;
extern const ImVec4 PaleGoldenrod;
extern const ImVec4 PaleGreen;
extern const ImVec4 PaleTurquoise;
extern const ImVec4 PaleVioletRed;
extern const ImVec4 PapayaWhip;
extern const ImVec4 PeachPuff;
extern const ImVec4 Peru;
extern const ImVec4 Pink;
extern const ImVec4 Plum;
extern const ImVec4 PowderBlue;
extern const ImVec4 Purple;
extern const ImVec4 Red;
extern const ImVec4 RosyBrown;
extern const ImVec4 RoyalBlue;
extern const ImVec4 SaddleBrown;
extern const ImVec4 Salmon;
extern const ImVec4 SandyBrown;
extern const ImVec4 SeaGreen;
extern const ImVec4 SeaShell;
extern const ImVec4 Sienna;
extern const ImVec4 Silver;
extern const ImVec4 SkyBlue;
extern const ImVec4 SlateBlue;
extern const ImVec4 SlateGray;
extern const ImVec4 Snow;
extern const ImVec4 SpringGreen;
extern const ImVec4 SteelBlue;
extern const ImVec4 Tan;
extern const ImVec4 Teal;
extern const ImVec4 Thistle;
extern const ImVec4 Tomato;
extern const ImVec4 Turquoise;
extern const ImVec4 Violet;
extern const ImVec4 Wheat;
extern const ImVec4 White;
extern const ImVec4 WhiteSmoke;
extern const ImVec4 Yellow;
extern const ImVec4 YellowGreen;
extern const ImVec4 ButtonFace;
extern const ImVec4 ButtonHighlight;
extern const ImVec4 ButtonShadow;
extern const ImVec4 GradientActiveCaption;
extern const ImVec4 GradientInactiveCaption;
extern const ImVec4 MenuBar;
extern const ImVec4 MenuHighlight;
extern const ImVec4 RebeccaPurple;
} // namespace KnownColor

struct FontSet
{
    ImFont* Default             = nullptr;
    ImFont* Icon                = nullptr;
    ImFont* IconFixedWidth      = nullptr;
    bool    UsedDefaultTextFont = false;
};

struct ImageTexture
{
    ID3D11ShaderResourceView* View   = nullptr;
    int                       Width  = 0;
    int                       Height = 0;
};

struct Dx11BlurTarget
{
    ID3D11Texture2D*          Texture            = nullptr;
    ID3D11ShaderResourceView* ShaderResourceView = nullptr;
    ID3D11RenderTargetView*   RenderTargetView   = nullptr;
    int                       Width              = 0;
    int                       Height             = 0;
};

struct PanelOptions
{
    ImVec2 Size      = ImVec2(0.0f, 0.0f);
    bool   Border    = false;
    bool   FitHeight = false;
};

struct ModuleHeaderOptions
{
    const char* Description                 = nullptr;
    bool        DefaultOpen                 = false;
    bool        Enabled                     = true;
    bool*       Checked                     = nullptr;
    bool        HasBody                     = true;
    bool        DescriptionAtBottomWhenOpen = false;
    float       AnimationSpeed              = 18.0f;
};

struct VerticalSplitterOptions
{
    float Height    = 0.0f;
    float HitWidth  = 8.0f;
    float LineWidth = 2.0f;
};

struct SnowflakeOptions
{
    int    Count     = 72;
    float  MinRadius = 1.0f;
    float  MaxRadius = 3.4f;
    float  MinSpeed  = 18.0f;
    float  MaxSpeed  = 62.0f;
    float  Wind      = 12.0f;
    float  Drift     = 18.0f;
    ImVec4 Color     = ImVec4(1.0f, 1.0f, 1.0f, 0.72f);
};

struct ColorTokens
{
    ImVec4 Background{};
    ImVec4 Surface{};
    ImVec4 SurfaceHover{};
    ImVec4 SurfaceActive{};
    ImVec4 Overlay{};
    ImVec4 Border{};
    ImVec4 BorderStrong{};
    ImVec4 Text{};
    ImVec4 TextMuted{};
    ImVec4 TextDisabled{};
    ImVec4 Accent{};
    ImVec4 AccentHover{};
    ImVec4 AccentActive{};
    ImVec4 AccentText{};
    ImVec4 Success{};
    ImVec4 Warning{};
    ImVec4 Danger{};
};

struct SizeTokens
{
    float  WindowRounding      = 8.0f;
    float  PanelRounding       = 8.0f;
    float  ControlRounding     = 6.0f;
    float  SmallRounding       = 4.0f;
    float  BorderWidth         = 1.0f;
    float  ControlHeight       = 30.0f;
    float  SmallControlHeight  = 24.0f;
    float  IconButtonSize      = 28.0f;
    float  InputMinWidth       = 180.0f;
    float  ItemGap             = 8.0f;
    float  SectionGap          = 10.0f;
    ImVec2 WindowPadding       = ImVec2(12.0f, 10.0f);
    ImVec2 PanelPadding        = ImVec2(10.0f, 8.0f);
    ImVec2 FramePadding        = ImVec2(9.0f, 5.0f);
    ImVec2 CompactFramePadding = ImVec2(7.0f, 4.0f);
};

struct Theme
{
    ColorTokens Colors{};
    SizeTokens  Sizes{};
};

struct IconButtonOptions
{
    Icon          IconValue  = Icon::None;
    const char*   Tooltip    = nullptr;
    ImVec2        Size       = ImVec2(0.0f, 0.0f);
    ButtonVariant Variant    = ButtonVariant::Ghost;
    bool          Active     = false;
    bool          Enabled    = true;
    bool          FixedWidth = true;
};

struct WindowOptions
{
    bool*            Open                       = nullptr;
    bool             UseRightSideCollapseButton = true;
    const char*      CollapseTooltip            = "折叠";
    const char*      RestoreTooltip             = "还原";
    ImGuiWindowFlags Flags                      = 0;
    bool             CountAsManagedWindow       = true;
};

struct PanelChildOptions
{
    ImVec2           Size   = ImVec2(0.0f, 0.0f);
    bool             Border = true;
    ImGuiWindowFlags Flags  = 0;
};

class StyleColorScope
{
  public:
    StyleColorScope() = default;
    StyleColorScope(ImGuiCol Index, const ImVec4& Color);
    ~StyleColorScope();

    StyleColorScope(const StyleColorScope&)            = delete;
    StyleColorScope& operator=(const StyleColorScope&) = delete;

    StyleColorScope(StyleColorScope&& Other) noexcept;
    StyleColorScope& operator=(StyleColorScope&& Other) noexcept;

    void Push(ImGuiCol Index, const ImVec4& Color);
    void Pop();

  private:
    int Count = 0;
};

class StyleVarScope
{
  public:
    StyleVarScope() = default;
    StyleVarScope(ImGuiStyleVar Index, float Value);
    StyleVarScope(ImGuiStyleVar Index, const ImVec2& Value);
    ~StyleVarScope();

    StyleVarScope(const StyleVarScope&)            = delete;
    StyleVarScope& operator=(const StyleVarScope&) = delete;

    StyleVarScope(StyleVarScope&& Other) noexcept;
    StyleVarScope& operator=(StyleVarScope&& Other) noexcept;

    void Push(ImGuiStyleVar Index, float Value);
    void Push(ImGuiStyleVar Index, const ImVec2& Value);
    void Pop();

  private:
    int Count = 0;
};

class DisabledScope
{
  public:
    explicit DisabledScope(bool Disabled);
    ~DisabledScope();

    DisabledScope(const DisabledScope&)            = delete;
    DisabledScope& operator=(const DisabledScope&) = delete;

  private:
    bool Active = false;
};

class Dx11GaussianBlur
{
  public:
    Dx11GaussianBlur() = default;
    ~Dx11GaussianBlur();

    bool Initialize(ID3D11Device* Device, ID3D11DeviceContext* Context);
    void Shutdown();
    bool Resize(int Width, int Height);
    bool Blur(ID3D11ShaderResourceView* SourceView, float Radius = 10.0f, int PassCount = 2);
    void DrawBlurredImage(const ImVec2& Size, const ImVec4& Tint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ID3D11ShaderResourceView* GetOutputView() const;
    int                       GetWidth() const;
    int                       GetHeight() const;

  private:
    bool CreateShaders();
    bool CreateSampler();
    bool CreateTarget(Dx11BlurTarget& Target, int Width, int Height);
    void DestroyTarget(Dx11BlurTarget& Target);
    void DrawPass(ID3D11ShaderResourceView* SourceView,
                  ID3D11RenderTargetView*   TargetView,
                  float                     DirectionX,
                  float                     DirectionY,
                  float                     Radius);

    ID3D11Device*        D3dDevice      = nullptr;
    ID3D11DeviceContext* D3dContext     = nullptr;
    ID3D11VertexShader*  VertexShader   = nullptr;
    ID3D11PixelShader*   PixelShader    = nullptr;
    ID3D11Buffer*        ConstantBuffer = nullptr;
    ID3D11SamplerState*  SamplerState   = nullptr;
    ID3D11BlendState*    BlendState     = nullptr;
    Dx11BlurTarget       PingTarget{};
    Dx11BlurTarget       PongTarget{};
};

float EaseValue(Ease Mode, float T);
float GetFrameDeltaSeconds();
float SmoothValue(ImGuiID Id, float Target, float Speed = 18.0f, float InitialValue = 0.0f);
float SmoothValue(const char* Id, float Target, float Speed = 18.0f, float InitialValue = 0.0f);
float AnimateBool(ImGuiID Id, bool Active, float Speed = 18.0f);
float AnimateBool(const char* Id, bool Active, float Speed = 18.0f);
void  ClearAnimationState(ImGuiID Id);
void  ClearAllAnimationStates();

const char* ToIconString(Icon Value);

bool           LoadFonts(float TextSize = 16.0f, float IconSize = 15.0f);
const FontSet& Fonts();

bool   CreateDx11TextureFromMemory(ID3D11Device* Device, const void* Data, std::size_t Size, ImageTexture& Texture);
bool   CreateDx11TextureFromFile(ID3D11Device* Device, const wchar_t* Path, ImageTexture& Texture);
void   DestroyDx11Texture(ImageTexture& Texture);
ImVec2 FitImageSize(int ImageWidth, int ImageHeight, const ImVec2& Bounds);
void   Image(const ImageTexture& Texture,
             const ImVec2&       Size,
             const ImVec4&       Tint   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
             const ImVec4&       Border = ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

void   AddVerticalSpace(float Height);
void   SameLineRight(float ItemWidth);
ImVec2 CalcButtonSize(const char* Label, float MinWidth = 0.0f);
ImVec2 CalcWindowContentSize(float PreferredWidth, float PreferredHeight);
float  GetVerticalSplitterHitWidth();
bool   DrawVerticalSplitter(const char* Id, float Height);
bool   DrawVerticalSplitter(const char* Id, const VerticalSplitterOptions& Options = {});

bool BeginPanel(const char* Id, const PanelOptions& Options = {});
void EndPanel();

bool IsPanelContentVisible();
bool BeginModule(const char* Id, const char* Name, const ModuleHeaderOptions& Options = {});
void EndModule();

void BeginIndented(float Width = 12.0f);
void EndIndented(float Width = 12.0f);

void DrawSnowflakes(const char* Id, const ImVec2& Min, const ImVec2& Max, const SnowflakeOptions& Options = {});

Theme MakeTheme();

void SetTheme(const Theme& NewTheme);
void ApplyTheme(const Theme& NewTheme);

const Theme&       GetTheme();
const ColorTokens& Colors();
const SizeTokens&  Sizes();

ImVec4 WithAlpha(ImVec4 Color, float Alpha);
ImVec4 WithMultipliedAlpha(ImVec4 Color, float AlphaScale);
ImVec4 Blend(ImVec4 A, ImVec4 B, float Amount);
ImU32  ToU32(const ImVec4& Color);

void Text(const char* Value);
void TextMuted(const char* Value);
void TextDisabled(const char* Value);
void Heading(const char* Value);
bool Hyperlink(const char* Label, const char* Url = nullptr);
void StatusText(const char* Value, StatusKind Kind = StatusKind::Neutral);
void Badge(const char* Label, BadgeVariant Variant = BadgeVariant::Neutral);
void BadgeDot(BadgeVariant Variant = BadgeVariant::Danger, float Radius = 0.0f);
void BadgeNumber(int Value, BadgeVariant Variant = BadgeVariant::Danger);
bool InfoBar(const char*  Message,
             InfoSeverity Severity    = InfoSeverity::Info,
             bool*        Open        = nullptr,
             const char*  ActionLabel = nullptr);
void HelpMarker(const char* Text);

bool Button(const char*   Label,
            const ImVec2& Size    = ImVec2(0.0f, 0.0f),
            ButtonVariant Variant = ButtonVariant::Secondary);
bool PrimaryButton(const char* Label, const ImVec2& Size = ImVec2(0.0f, 0.0f));
bool GhostButton(const char* Label, const ImVec2& Size = ImVec2(0.0f, 0.0f));
bool DangerButton(const char* Label, const ImVec2& Size = ImVec2(0.0f, 0.0f));
bool IconButton(const char*   Id,
                const char*   Icon,
                const char*   Tooltip = nullptr,
                const ImVec2& Size    = ImVec2(0.0f, 0.0f));
bool IconButton(const char* Id, Icon IconValue, const char* Tooltip = nullptr, const ImVec2& Size = ImVec2(0.0f, 0.0f));
bool IconActionButton(const char* Id, const IconButtonOptions& Options);
bool SettingsButton(const char* Id = "Settings", bool Active = false);
bool HelpButton(const char* Id, const char* Tooltip);
bool DiscordButton(const char* Id = "Discord");
bool IconButtonWithText(const char*   Id,
                        Icon          IconValue,
                        const char*   Label,
                        const ImVec2& Size    = ImVec2(0.0f, 0.0f),
                        ButtonVariant Variant = ButtonVariant::Secondary);

bool Checkbox(const char* Label, bool* Value);
bool CheckboxPill(const char* Label, bool* Value);
bool Toggle(const char* Label, bool* Value);
bool SliderFloat(const char* Label, float* Value, float Min, float Max, const char* Format = "%.2f");
bool SliderInt(const char* Label, int* Value, int Min, int Max);
bool SliderIntMapped(
    const char* Label, int* Value, int Min, int Max, int DisplayMultiplier, const char* DisplayFormat = "%d");
bool Combo(const char* Label, int* CurrentItem, const char* const Items[], int ItemCount);
bool SegmentedControl(const char*       Id,
                      int*              CurrentItem,
                      const char* const Items[],
                      int               ItemCount,
                      const ImVec2&     Size = ImVec2(0.0f, 0.0f));
ImU32 ArgbToImColor(std::uint32_t Color);
ImVec4 ArgbToImVec4(std::uint32_t Color);
std::uint32_t ImVec4ToArgb(const ImVec4& Color);
bool ColorEditArgb(const char* PId, std::uint32_t& Color, const ImVec2& Size = ImVec2(24.0f, 24.0f));
bool InputText(
    const char* Label, char* Buffer, std::size_t BufferSize, const char* Hint = nullptr, ImGuiInputTextFlags Flags = 0);
bool InputInt(const char* Label, int* Value);
bool InputFloat(const char* Label, float* Value, const char* Format = "%.3f");
void ProgressBar(const char*   Id,
                 float         Fraction,
                 const ImVec2& Size    = ImVec2(0.0f, 0.0f),
                 const char*   Overlay = nullptr);
void DrawTextCenteredX(ImDrawList* DrawList, float CenterX, float Y, ImU32 Color, std::string_view Text);
void IndeterminateProgressBar(const char* Id, const ImVec2& Size = ImVec2(0.0f, 0.0f));
void ProgressRing(const char* Id, float Fraction, float Radius = 0.0f, float Thickness = 0.0f);
void Spinner(const char* Id, float Radius = 0.0f, float Thickness = 0.0f);
bool BeginTable(const char* Id, int ColumnCount, ImGuiTableFlags Flags = 0, const ImVec2& OuterSize = ImVec2(0.0f, 0.0f));
void TableHeadersRow(const char* const Headers[], int HeaderCount);
void EndTable();

inline float CalcTableColumnWidth(std::initializer_list<std::string_view> Texts)
{
    float Width = 0.0f;
    for (std::string_view Text : Texts)
        Width = (std::max)(Width, ImGui::CalcTextSize(Text.data(), Text.data() + Text.size()).x);

    const ImGuiStyle& Style = ImGui::GetStyle();
    return std::ceil(Width + Style.CellPadding.x * 2.0f);
}

template <class Range, class Formatter>
float CalcTableColumnWidth(std::string_view Header, const Range& Rows, Formatter&& Format)
{
    float Width = ImGui::CalcTextSize(Header.data(), Header.data() + Header.size()).x;
    for (const auto& Row : Rows)
    {
        const std::string Text = std::forward<Formatter>(Format)(Row);
        Width                  = (std::max)(Width, ImGui::CalcTextSize(Text.c_str()).x);
    }

    const ImGuiStyle& Style = ImGui::GetStyle();
    return std::ceil(Width + Style.CellPadding.x * 2.0f);
}

void LabelValue(const char* Label, const char* Value);
void LabelValue(const char* Label, int Value);
void LabelValue(const char* Label, float Value, const char* Format = "%.2f");

bool BeginWindow(const char* Name, const WindowOptions& Options = {});
void EndWindow();
void BeginManagedWindowFrame();
bool HasVisibleManagedWindows();
bool ShouldPreferGameTextInput(HWND Hwnd, UINT Msg, WPARAM WParam);
void DrawTitleBarCollapseButton(const char* Id              = "RnTitleBarCollapseButton",
                                const char* CollapseTooltip = "折叠",
                                const char* RestoreTooltip  = "还原");
bool BeginPanelChild(const char* Id, const PanelChildOptions& Options = {});
void EndPanelChild();
bool IsPanelChildContentVisible();

} // namespace RnTools::ImguiRorinnn
