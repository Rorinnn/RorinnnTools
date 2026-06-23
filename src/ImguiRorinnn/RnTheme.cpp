// RnTheme.cpp — ImguiRorinnn 主题令牌

module;

#include <imgui.h>

module RorinnnTools;
import std;

namespace RorinnnTools::ImguiRorinnn
{
namespace
{
static float Clamp01(float Value)
{
    return std::clamp(Value, 0.0f, 1.0f);
}

static ImVec4 Col(float R, float G, float B, float A = 1.0f)
{
    return ImVec4(R, G, B, A);
}

static void ApplyStyleFromTheme(const Theme& ThemeValue)
{
    if (!ImGui::GetCurrentContext())
    {
        return;
    }

    ImGuiStyle&        Style = ImGui::GetStyle();
    const ColorTokens& C     = ThemeValue.Colors;
    const SizeTokens&  S     = ThemeValue.Sizes;

    Style.WindowPadding       = S.WindowPadding;
    Style.WindowRounding      = S.WindowRounding;
    Style.WindowBorderSize    = 0.0f;
    Style.ChildRounding       = S.PanelRounding;
    Style.ChildBorderSize     = 0.0f;
    Style.PopupRounding       = S.PanelRounding;
    Style.PopupBorderSize     = S.BorderWidth;
    Style.FramePadding        = S.FramePadding;
    Style.FrameRounding       = S.ControlRounding;
    Style.FrameBorderSize     = 0.0f;
    Style.ItemSpacing         = ImVec2(S.ItemGap, S.ItemGap * 0.75f);
    Style.ItemInnerSpacing    = ImVec2(S.ItemGap, S.ItemGap * 0.70f);
    Style.CellPadding         = ImVec2(S.ItemGap, S.ItemGap * 0.65f);
    Style.IndentSpacing       = 18.0f;
    Style.ScrollbarSize       = 13.0f;
    Style.ScrollbarRounding   = S.ControlRounding;
    Style.GrabMinSize         = 9.0f;
    Style.GrabRounding        = S.ControlRounding;
    Style.TabRounding         = S.ControlRounding;
    Style.ButtonTextAlign     = ImVec2(0.5f, 0.5f);
    Style.SelectableTextAlign = ImVec2(0.0f, 0.5f);

    ImVec4* Colors                        = Style.Colors;
    Colors[ImGuiCol_Text]                 = C.Text;
    Colors[ImGuiCol_TextDisabled]         = C.TextDisabled;
    Colors[ImGuiCol_WindowBg]             = C.Background;
    Colors[ImGuiCol_ChildBg]              = C.Surface;
    Colors[ImGuiCol_PopupBg]              = C.Overlay;
    Colors[ImGuiCol_Border]               = C.Border;
    Colors[ImGuiCol_BorderShadow]         = WithAlpha(C.Background, 0.0f);
    Colors[ImGuiCol_FrameBg]              = C.Surface;
    Colors[ImGuiCol_FrameBgHovered]       = C.SurfaceHover;
    Colors[ImGuiCol_FrameBgActive]        = C.SurfaceActive;
    Colors[ImGuiCol_TitleBg]              = C.Background;
    Colors[ImGuiCol_TitleBgActive]        = C.Background;
    Colors[ImGuiCol_TitleBgCollapsed]     = C.Background;
    Colors[ImGuiCol_MenuBarBg]            = C.Surface;
    Colors[ImGuiCol_ScrollbarBg]          = WithAlpha(C.Background, 0.55f);
    Colors[ImGuiCol_ScrollbarGrab]        = WithAlpha(C.BorderStrong, 0.55f);
    Colors[ImGuiCol_ScrollbarGrabHovered] = WithAlpha(C.Accent, 0.58f);
    Colors[ImGuiCol_ScrollbarGrabActive]  = WithAlpha(C.Accent, 0.78f);
    Colors[ImGuiCol_CheckMark]            = C.AccentText;
    Colors[ImGuiCol_SliderGrab]           = C.Accent;
    Colors[ImGuiCol_SliderGrabActive]     = C.AccentHover;
    Colors[ImGuiCol_Button]               = C.Surface;
    Colors[ImGuiCol_ButtonHovered]        = C.SurfaceHover;
    Colors[ImGuiCol_ButtonActive]         = C.SurfaceActive;
    Colors[ImGuiCol_Header]               = WithAlpha(C.AccentText, 0.10f);
    Colors[ImGuiCol_HeaderHovered]        = WithAlpha(C.AccentText, 0.17f);
    Colors[ImGuiCol_HeaderActive]         = WithAlpha(C.AccentText, 0.22f);
    Colors[ImGuiCol_Separator]            = WithAlpha(C.Border, 0.0f);
    Colors[ImGuiCol_SeparatorHovered]     = WithAlpha(C.Border, 0.0f);
    Colors[ImGuiCol_SeparatorActive]      = WithAlpha(C.Border, 0.0f);
    Colors[ImGuiCol_ResizeGrip]           = WithAlpha(C.BorderStrong, 0.20f);
    Colors[ImGuiCol_ResizeGripHovered]    = WithAlpha(C.Accent, 0.34f);
    Colors[ImGuiCol_ResizeGripActive]     = WithAlpha(C.Accent, 0.52f);
    Colors[ImGuiCol_Tab]                  = C.Surface;
    Colors[ImGuiCol_TabHovered]           = WithAlpha(C.Accent, 0.34f);
    Colors[ImGuiCol_TabActive]            = C.SurfaceActive;
    Colors[ImGuiCol_TabUnfocused]         = C.Surface;
    Colors[ImGuiCol_TabUnfocusedActive]   = C.SurfaceHover;
    Colors[ImGuiCol_TableHeaderBg]        = WithAlpha(C.AccentText, 0.10f);
    Colors[ImGuiCol_TableRowBg]           = C.Surface;
    Colors[ImGuiCol_TableRowBgAlt]        = C.SurfaceHover;
    Colors[ImGuiCol_TextSelectedBg]       = WithAlpha(C.Accent, 0.34f);
    Colors[ImGuiCol_NavHighlight]         = WithAlpha(C.Accent, 0.42f);
    Colors[ImGuiCol_ModalWindowDimBg]     = WithAlpha(Color::Black, 0.46f);
}

static Theme g_BaseTheme = MakeTheme();
static Theme g_Theme     = g_BaseTheme;
} // namespace

Theme MakeTheme()
{
    Theme Result{};

    const ImVec4 AccentValue = Color::SoftWhite;

    Result.Colors.Accent        = AccentValue;
    Result.Colors.AccentHover   = Blend(AccentValue, Col(1.0f, 1.0f, 1.0f), 0.12f);
    Result.Colors.AccentActive  = Blend(AccentValue, Col(0.0f, 0.0f, 0.0f), 0.20f);
    Result.Colors.AccentText    = Col(0.98f, 0.99f, 1.0f);
    Result.Colors.Success       = Color::Green;
    Result.Colors.Warning       = Color::Orange;
    Result.Colors.Danger        = Color::Red;
    Result.Colors.Background    = WithAlpha(Color::DarkSlate, 0.38f);
    Result.Colors.Surface       = WithAlpha(Color::SoftWhite, 0.12f);
    Result.Colors.SurfaceHover  = WithAlpha(Color::White, 0.19f);
    Result.Colors.SurfaceActive = WithAlpha(Color::White, 0.26f);
    Result.Colors.Overlay       = WithAlpha(Color::DarkSlate, 0.64f);
    Result.Colors.Border        = WithAlpha(Color::SoftWhite, 0.13f);
    Result.Colors.BorderStrong  = WithAlpha(Color::SoftWhite, 0.22f);
    Result.Colors.Text          = WithAlpha(Color::White, 0.99f);
    Result.Colors.TextMuted     = WithAlpha(Color::SoftWhite, 0.82f);
    Result.Colors.TextDisabled  = WithAlpha(Color::LightGray, 0.48f);

    return Result;
}

void SetTheme(const Theme& NewTheme)
{
    g_BaseTheme = NewTheme;
    g_Theme     = g_BaseTheme;
}

void ApplyTheme(const Theme& NewTheme)
{
    SetTheme(NewTheme);
    ApplyStyleFromTheme(g_Theme);
}

const Theme& GetTheme()
{
    return g_BaseTheme;
}

const ColorTokens& Colors()
{
    return g_Theme.Colors;
}

const SizeTokens& Sizes()
{
    return g_Theme.Sizes;
}

ImVec4 WithAlpha(ImVec4 Color, float Alpha)
{
    Color.w = Clamp01(Alpha);
    return Color;
}

ImVec4 WithMultipliedAlpha(ImVec4 Color, float AlphaScale)
{
    Color.w = Clamp01(Color.w * AlphaScale);
    return Color;
}

ImVec4 Blend(ImVec4 A, ImVec4 B, float Amount)
{
    const float T = Clamp01(Amount);
    return ImVec4(A.x + (B.x - A.x) * T, A.y + (B.y - A.y) * T, A.z + (B.z - A.z) * T, A.w + (B.w - A.w) * T);
}

ImU32 ToU32(const ImVec4& Color)
{
    return ImGui::ColorConvertFloat4ToU32(Color);
}

} // namespace RorinnnTools::ImguiRorinnn
