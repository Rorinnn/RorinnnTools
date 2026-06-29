module;

#include <imgui.h>

module RnTools;

namespace RnTools::ImguiRorinnn
{

ImU32 ArgbToImColor(std::uint32_t Color)
{
    return IM_COL32((Color >> 16) & 0xFFu, (Color >> 8) & 0xFFu, Color & 0xFFu, (Color >> 24) & 0xFFu);
}

ImVec4 ArgbToImVec4(std::uint32_t Color)
{
    return ImVec4((float)((Color >> 16) & 0xFFu) / 255.0f,
                  (float)((Color >> 8) & 0xFFu) / 255.0f,
                  (float)(Color & 0xFFu) / 255.0f,
                  (float)((Color >> 24) & 0xFFu) / 255.0f);
}

std::uint32_t ImVec4ToArgb(const ImVec4& Color)
{
    const float R = std::clamp(Color.x, 0.0f, 1.0f);
    const float G = std::clamp(Color.y, 0.0f, 1.0f);
    const float B = std::clamp(Color.z, 0.0f, 1.0f);
    const float A = std::clamp(Color.w, 0.0f, 1.0f);
    return ((std::uint32_t)(A * 255.0f + 0.5f) << 24) | ((std::uint32_t)(R * 255.0f + 0.5f) << 16) |
           ((std::uint32_t)(G * 255.0f + 0.5f) << 8) | (std::uint32_t)(B * 255.0f + 0.5f);
}

bool ColorEditArgb(const char* PId, std::uint32_t& Color)
{
    ImVec4 ColorValue = ArgbToImVec4(Color);
    const ImVec2 ButtonSize(24.0f, 24.0f);
    bool Changed = false;
    ImGui::ColorButton(PId, ColorValue, ImGuiColorEditFlags_AlphaPreview, ButtonSize);
    if (ImGui::IsItemClicked())
        ImGui::OpenPopup(PId);
    if (ImGui::BeginPopup(PId))
    {
        Changed = ImGui::ColorPicker4("##ColorPicker", &ColorValue.x, ImGuiColorEditFlags_AlphaBar);
        ImGui::EndPopup();
    }
    if (!Changed)
        return false;

    Color = ImVec4ToArgb(ColorValue);
    return true;
}

} // namespace RnTools::ImguiRorinnn

namespace RnTools::ImguiRorinnn::KnownColor
{
const ImVec4 ActiveBorder = ImVec4(0.705882f, 0.705882f, 0.705882f, 1.0f);
const ImVec4 ActiveCaption = ImVec4(0.6f, 0.705882f, 0.819608f, 1.0f);
const ImVec4 ActiveCaptionText = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 AppWorkspace = ImVec4(0.670588f, 0.670588f, 0.670588f, 1.0f);
const ImVec4 Control = ImVec4(0.941176f, 0.941176f, 0.941176f, 1.0f);
const ImVec4 ControlDark = ImVec4(0.627451f, 0.627451f, 0.627451f, 1.0f);
const ImVec4 ControlDarkDark = ImVec4(0.411765f, 0.411765f, 0.411765f, 1.0f);
const ImVec4 ControlLight = ImVec4(0.890196f, 0.890196f, 0.890196f, 1.0f);
const ImVec4 ControlLightLight = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 ControlText = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 Desktop = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 GrayText = ImVec4(0.427451f, 0.427451f, 0.427451f, 1.0f);
const ImVec4 Highlight = ImVec4(0.0f, 0.470588f, 0.843137f, 1.0f);
const ImVec4 HighlightText = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 HotTrack = ImVec4(0.0f, 0.4f, 0.8f, 1.0f);
const ImVec4 InactiveBorder = ImVec4(0.956863f, 0.968627f, 0.988235f, 1.0f);
const ImVec4 InactiveCaption = ImVec4(0.74902f, 0.803922f, 0.858824f, 1.0f);
const ImVec4 InactiveCaptionText = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 Info = ImVec4(1.0f, 1.0f, 0.882353f, 1.0f);
const ImVec4 InfoText = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 Menu = ImVec4(0.941176f, 0.941176f, 0.941176f, 1.0f);
const ImVec4 MenuText = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 ScrollBar = ImVec4(0.784314f, 0.784314f, 0.784314f, 1.0f);
const ImVec4 Window = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 WindowFrame = ImVec4(0.392157f, 0.392157f, 0.392157f, 1.0f);
const ImVec4 WindowText = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 Transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
const ImVec4 AliceBlue = ImVec4(0.941176f, 0.972549f, 1.0f, 1.0f);
const ImVec4 AntiqueWhite = ImVec4(0.980392f, 0.921569f, 0.843137f, 1.0f);
const ImVec4 Aqua = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 Aquamarine = ImVec4(0.498039f, 1.0f, 0.831373f, 1.0f);
const ImVec4 Azure = ImVec4(0.941176f, 1.0f, 1.0f, 1.0f);
const ImVec4 Beige = ImVec4(0.960784f, 0.960784f, 0.862745f, 1.0f);
const ImVec4 Bisque = ImVec4(1.0f, 0.894118f, 0.768627f, 1.0f);
const ImVec4 Black = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 BlanchedAlmond = ImVec4(1.0f, 0.921569f, 0.803922f, 1.0f);
const ImVec4 Blue = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
const ImVec4 BlueViolet = ImVec4(0.541176f, 0.168627f, 0.886275f, 1.0f);
const ImVec4 Brown = ImVec4(0.647059f, 0.164706f, 0.164706f, 1.0f);
const ImVec4 BurlyWood = ImVec4(0.870588f, 0.721569f, 0.529412f, 1.0f);
const ImVec4 CadetBlue = ImVec4(0.372549f, 0.619608f, 0.627451f, 1.0f);
const ImVec4 Chartreuse = ImVec4(0.498039f, 1.0f, 0.0f, 1.0f);
const ImVec4 Chocolate = ImVec4(0.823529f, 0.411765f, 0.117647f, 1.0f);
const ImVec4 Coral = ImVec4(1.0f, 0.498039f, 0.313725f, 1.0f);
const ImVec4 CornflowerBlue = ImVec4(0.392157f, 0.584314f, 0.929412f, 1.0f);
const ImVec4 Cornsilk = ImVec4(1.0f, 0.972549f, 0.862745f, 1.0f);
const ImVec4 Crimson = ImVec4(0.862745f, 0.078431f, 0.235294f, 1.0f);
const ImVec4 Cyan = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 DarkBlue = ImVec4(0.0f, 0.0f, 0.545098f, 1.0f);
const ImVec4 DarkCyan = ImVec4(0.0f, 0.545098f, 0.545098f, 1.0f);
const ImVec4 DarkGoldenrod = ImVec4(0.721569f, 0.52549f, 0.043137f, 1.0f);
const ImVec4 DarkGray = ImVec4(0.662745f, 0.662745f, 0.662745f, 1.0f);
const ImVec4 DarkGreen = ImVec4(0.0f, 0.392157f, 0.0f, 1.0f);
const ImVec4 DarkKhaki = ImVec4(0.741176f, 0.717647f, 0.419608f, 1.0f);
const ImVec4 DarkMagenta = ImVec4(0.545098f, 0.0f, 0.545098f, 1.0f);
const ImVec4 DarkOliveGreen = ImVec4(0.333333f, 0.419608f, 0.184314f, 1.0f);
const ImVec4 DarkOrange = ImVec4(1.0f, 0.54902f, 0.0f, 1.0f);
const ImVec4 DarkOrchid = ImVec4(0.6f, 0.196078f, 0.8f, 1.0f);
const ImVec4 DarkRed = ImVec4(0.545098f, 0.0f, 0.0f, 1.0f);
const ImVec4 DarkSalmon = ImVec4(0.913725f, 0.588235f, 0.478431f, 1.0f);
const ImVec4 DarkSeaGreen = ImVec4(0.560784f, 0.737255f, 0.560784f, 1.0f);
const ImVec4 DarkSlateBlue = ImVec4(0.282353f, 0.239216f, 0.545098f, 1.0f);
const ImVec4 DarkSlateGray = ImVec4(0.184314f, 0.309804f, 0.309804f, 1.0f);
const ImVec4 DarkTurquoise = ImVec4(0.0f, 0.807843f, 0.819608f, 1.0f);
const ImVec4 DarkViolet = ImVec4(0.580392f, 0.0f, 0.827451f, 1.0f);
const ImVec4 DeepPink = ImVec4(1.0f, 0.078431f, 0.576471f, 1.0f);
const ImVec4 DeepSkyBlue = ImVec4(0.0f, 0.74902f, 1.0f, 1.0f);
const ImVec4 DimGray = ImVec4(0.411765f, 0.411765f, 0.411765f, 1.0f);
const ImVec4 DodgerBlue = ImVec4(0.117647f, 0.564706f, 1.0f, 1.0f);
const ImVec4 Firebrick = ImVec4(0.698039f, 0.133333f, 0.133333f, 1.0f);
const ImVec4 FloralWhite = ImVec4(1.0f, 0.980392f, 0.941176f, 1.0f);
const ImVec4 ForestGreen = ImVec4(0.133333f, 0.545098f, 0.133333f, 1.0f);
const ImVec4 Fuchsia = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
const ImVec4 Gainsboro = ImVec4(0.862745f, 0.862745f, 0.862745f, 1.0f);
const ImVec4 GhostWhite = ImVec4(0.972549f, 0.972549f, 1.0f, 1.0f);
const ImVec4 Gold = ImVec4(1.0f, 0.843137f, 0.0f, 1.0f);
const ImVec4 Goldenrod = ImVec4(0.854902f, 0.647059f, 0.12549f, 1.0f);
const ImVec4 Gray = ImVec4(0.501961f, 0.501961f, 0.501961f, 1.0f);
const ImVec4 Green = ImVec4(0.0f, 0.501961f, 0.0f, 1.0f);
const ImVec4 GreenYellow = ImVec4(0.678431f, 1.0f, 0.184314f, 1.0f);
const ImVec4 Honeydew = ImVec4(0.941176f, 1.0f, 0.941176f, 1.0f);
const ImVec4 HotPink = ImVec4(1.0f, 0.411765f, 0.705882f, 1.0f);
const ImVec4 IndianRed = ImVec4(0.803922f, 0.360784f, 0.360784f, 1.0f);
const ImVec4 Indigo = ImVec4(0.294118f, 0.0f, 0.509804f, 1.0f);
const ImVec4 Ivory = ImVec4(1.0f, 1.0f, 0.941176f, 1.0f);
const ImVec4 Khaki = ImVec4(0.941176f, 0.901961f, 0.54902f, 1.0f);
const ImVec4 Lavender = ImVec4(0.901961f, 0.901961f, 0.980392f, 1.0f);
const ImVec4 LavenderBlush = ImVec4(1.0f, 0.941176f, 0.960784f, 1.0f);
const ImVec4 LawnGreen = ImVec4(0.486275f, 0.988235f, 0.0f, 1.0f);
const ImVec4 LemonChiffon = ImVec4(1.0f, 0.980392f, 0.803922f, 1.0f);
const ImVec4 LightBlue = ImVec4(0.678431f, 0.847059f, 0.901961f, 1.0f);
const ImVec4 LightCoral = ImVec4(0.941176f, 0.501961f, 0.501961f, 1.0f);
const ImVec4 LightCyan = ImVec4(0.878431f, 1.0f, 1.0f, 1.0f);
const ImVec4 LightGoldenrodYellow = ImVec4(0.980392f, 0.980392f, 0.823529f, 1.0f);
const ImVec4 LightGray = ImVec4(0.827451f, 0.827451f, 0.827451f, 1.0f);
const ImVec4 LightGreen = ImVec4(0.564706f, 0.933333f, 0.564706f, 1.0f);
const ImVec4 LightPink = ImVec4(1.0f, 0.713725f, 0.756863f, 1.0f);
const ImVec4 LightSalmon = ImVec4(1.0f, 0.627451f, 0.478431f, 1.0f);
const ImVec4 LightSeaGreen = ImVec4(0.12549f, 0.698039f, 0.666667f, 1.0f);
const ImVec4 LightSkyBlue = ImVec4(0.529412f, 0.807843f, 0.980392f, 1.0f);
const ImVec4 LightSlateGray = ImVec4(0.466667f, 0.533333f, 0.6f, 1.0f);
const ImVec4 LightSteelBlue = ImVec4(0.690196f, 0.768627f, 0.870588f, 1.0f);
const ImVec4 LightYellow = ImVec4(1.0f, 1.0f, 0.878431f, 1.0f);
const ImVec4 Lime = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
const ImVec4 LimeGreen = ImVec4(0.196078f, 0.803922f, 0.196078f, 1.0f);
const ImVec4 Linen = ImVec4(0.980392f, 0.941176f, 0.901961f, 1.0f);
const ImVec4 Magenta = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
const ImVec4 Maroon = ImVec4(0.501961f, 0.0f, 0.0f, 1.0f);
const ImVec4 MediumAquamarine = ImVec4(0.4f, 0.803922f, 0.666667f, 1.0f);
const ImVec4 MediumBlue = ImVec4(0.0f, 0.0f, 0.803922f, 1.0f);
const ImVec4 MediumOrchid = ImVec4(0.729412f, 0.333333f, 0.827451f, 1.0f);
const ImVec4 MediumPurple = ImVec4(0.576471f, 0.439216f, 0.858824f, 1.0f);
const ImVec4 MediumSeaGreen = ImVec4(0.235294f, 0.701961f, 0.443137f, 1.0f);
const ImVec4 MediumSlateBlue = ImVec4(0.482353f, 0.407843f, 0.933333f, 1.0f);
const ImVec4 MediumSpringGreen = ImVec4(0.0f, 0.980392f, 0.603922f, 1.0f);
const ImVec4 MediumTurquoise = ImVec4(0.282353f, 0.819608f, 0.8f, 1.0f);
const ImVec4 MediumVioletRed = ImVec4(0.780392f, 0.082353f, 0.521569f, 1.0f);
const ImVec4 MidnightBlue = ImVec4(0.098039f, 0.098039f, 0.439216f, 1.0f);
const ImVec4 MintCream = ImVec4(0.960784f, 1.0f, 0.980392f, 1.0f);
const ImVec4 MistyRose = ImVec4(1.0f, 0.894118f, 0.882353f, 1.0f);
const ImVec4 Moccasin = ImVec4(1.0f, 0.894118f, 0.709804f, 1.0f);
const ImVec4 NavajoWhite = ImVec4(1.0f, 0.870588f, 0.678431f, 1.0f);
const ImVec4 Navy = ImVec4(0.0f, 0.0f, 0.501961f, 1.0f);
const ImVec4 OldLace = ImVec4(0.992157f, 0.960784f, 0.901961f, 1.0f);
const ImVec4 Olive = ImVec4(0.501961f, 0.501961f, 0.0f, 1.0f);
const ImVec4 OliveDrab = ImVec4(0.419608f, 0.556863f, 0.137255f, 1.0f);
const ImVec4 Orange = ImVec4(1.0f, 0.647059f, 0.0f, 1.0f);
const ImVec4 OrangeRed = ImVec4(1.0f, 0.270588f, 0.0f, 1.0f);
const ImVec4 Orchid = ImVec4(0.854902f, 0.439216f, 0.839216f, 1.0f);
const ImVec4 PaleGoldenrod = ImVec4(0.933333f, 0.909804f, 0.666667f, 1.0f);
const ImVec4 PaleGreen = ImVec4(0.596078f, 0.984314f, 0.596078f, 1.0f);
const ImVec4 PaleTurquoise = ImVec4(0.686275f, 0.933333f, 0.933333f, 1.0f);
const ImVec4 PaleVioletRed = ImVec4(0.858824f, 0.439216f, 0.576471f, 1.0f);
const ImVec4 PapayaWhip = ImVec4(1.0f, 0.937255f, 0.835294f, 1.0f);
const ImVec4 PeachPuff = ImVec4(1.0f, 0.854902f, 0.72549f, 1.0f);
const ImVec4 Peru = ImVec4(0.803922f, 0.521569f, 0.247059f, 1.0f);
const ImVec4 Pink = ImVec4(1.0f, 0.752941f, 0.796078f, 1.0f);
const ImVec4 Plum = ImVec4(0.866667f, 0.627451f, 0.866667f, 1.0f);
const ImVec4 PowderBlue = ImVec4(0.690196f, 0.878431f, 0.901961f, 1.0f);
const ImVec4 Purple = ImVec4(0.501961f, 0.0f, 0.501961f, 1.0f);
const ImVec4 Red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 RosyBrown = ImVec4(0.737255f, 0.560784f, 0.560784f, 1.0f);
const ImVec4 RoyalBlue = ImVec4(0.254902f, 0.411765f, 0.882353f, 1.0f);
const ImVec4 SaddleBrown = ImVec4(0.545098f, 0.270588f, 0.07451f, 1.0f);
const ImVec4 Salmon = ImVec4(0.980392f, 0.501961f, 0.447059f, 1.0f);
const ImVec4 SandyBrown = ImVec4(0.956863f, 0.643137f, 0.376471f, 1.0f);
const ImVec4 SeaGreen = ImVec4(0.180392f, 0.545098f, 0.341176f, 1.0f);
const ImVec4 SeaShell = ImVec4(1.0f, 0.960784f, 0.933333f, 1.0f);
const ImVec4 Sienna = ImVec4(0.627451f, 0.321569f, 0.176471f, 1.0f);
const ImVec4 Silver = ImVec4(0.752941f, 0.752941f, 0.752941f, 1.0f);
const ImVec4 SkyBlue = ImVec4(0.529412f, 0.807843f, 0.921569f, 1.0f);
const ImVec4 SlateBlue = ImVec4(0.415686f, 0.352941f, 0.803922f, 1.0f);
const ImVec4 SlateGray = ImVec4(0.439216f, 0.501961f, 0.564706f, 1.0f);
const ImVec4 Snow = ImVec4(1.0f, 0.980392f, 0.980392f, 1.0f);
const ImVec4 SpringGreen = ImVec4(0.0f, 1.0f, 0.498039f, 1.0f);
const ImVec4 SteelBlue = ImVec4(0.27451f, 0.509804f, 0.705882f, 1.0f);
const ImVec4 Tan = ImVec4(0.823529f, 0.705882f, 0.54902f, 1.0f);
const ImVec4 Teal = ImVec4(0.0f, 0.501961f, 0.501961f, 1.0f);
const ImVec4 Thistle = ImVec4(0.847059f, 0.74902f, 0.847059f, 1.0f);
const ImVec4 Tomato = ImVec4(1.0f, 0.388235f, 0.278431f, 1.0f);
const ImVec4 Turquoise = ImVec4(0.25098f, 0.878431f, 0.815686f, 1.0f);
const ImVec4 Violet = ImVec4(0.933333f, 0.509804f, 0.933333f, 1.0f);
const ImVec4 Wheat = ImVec4(0.960784f, 0.870588f, 0.701961f, 1.0f);
const ImVec4 White = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 WhiteSmoke = ImVec4(0.960784f, 0.960784f, 0.960784f, 1.0f);
const ImVec4 Yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
const ImVec4 YellowGreen = ImVec4(0.603922f, 0.803922f, 0.196078f, 1.0f);
const ImVec4 ButtonFace = ImVec4(0.941176f, 0.941176f, 0.941176f, 1.0f);
const ImVec4 ButtonHighlight = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 ButtonShadow = ImVec4(0.627451f, 0.627451f, 0.627451f, 1.0f);
const ImVec4 GradientActiveCaption = ImVec4(0.72549f, 0.819608f, 0.917647f, 1.0f);
const ImVec4 GradientInactiveCaption = ImVec4(0.843137f, 0.894118f, 0.94902f, 1.0f);
const ImVec4 MenuBar = ImVec4(0.941176f, 0.941176f, 0.941176f, 1.0f);
const ImVec4 MenuHighlight = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
const ImVec4 RebeccaPurple = ImVec4(0.4f, 0.2f, 0.6f, 1.0f);
} // namespace RnTools::ImguiRorinnn::KnownColor
