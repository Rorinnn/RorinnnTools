#pragma once

// RnTheme.hpp — ImguiRorinnn 主题令牌

#include <imgui.h>

namespace RorinnnTools::ImguiRorinnn
{

namespace Color
{
inline const ImVec4 Transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
inline const ImVec4 Black       = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
inline const ImVec4 White       = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
inline const ImVec4 Slate       = ImVec4(0.10f, 0.12f, 0.15f, 1.0f);
inline const ImVec4 DarkSlate   = ImVec4(0.02f, 0.03f, 0.04f, 1.0f);
inline const ImVec4 Gray        = ImVec4(0.50f, 0.54f, 0.61f, 1.0f);
inline const ImVec4 LightGray   = ImVec4(0.72f, 0.74f, 0.78f, 1.0f);
inline const ImVec4 SoftWhite   = ImVec4(0.92f, 0.94f, 0.98f, 1.0f);
inline const ImVec4 Blue        = ImVec4(0.22f, 0.52f, 0.88f, 1.0f);
inline const ImVec4 Green       = ImVec4(0.20f, 0.62f, 0.42f, 1.0f);
inline const ImVec4 Orange      = ImVec4(0.90f, 0.48f, 0.18f, 1.0f);
inline const ImVec4 Purple      = ImVec4(0.54f, 0.42f, 0.86f, 1.0f);
inline const ImVec4 Rose        = ImVec4(0.84f, 0.32f, 0.48f, 1.0f);
inline const ImVec4 Red         = ImVec4(0.86f, 0.28f, 0.32f, 1.0f);
} // namespace Color

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

} // namespace RorinnnTools::ImguiRorinnn
