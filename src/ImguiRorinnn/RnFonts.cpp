// RnFonts.cpp — ImguiRorinnn 字体加载

module;

#ifdef _WIN32
#include <Windows.h>
#endif

#include <imgui.h>

module RorinnnTools;

namespace RorinnnTools::ImguiRorinnn
{
namespace
{
static FontSet g_Fonts;

static ImFont* LoadWindowsTextFont(ImFontAtlas* Atlas, float TextSize, bool& UsedDefaultFont)
{
    UsedDefaultFont = false;
#ifdef _WIN32
    char FontPath[MAX_PATH] = {};
    ExpandEnvironmentStringsA("%SystemRoot%\\Fonts\\msyhbd.ttc", FontPath, MAX_PATH);
    if (GetFileAttributesA(FontPath) != INVALID_FILE_ATTRIBUTES)
    {
        return Atlas->AddFontFromFileTTF(FontPath, TextSize, nullptr, Atlas->GetGlyphRangesChineseSimplifiedCommon());
    }
#else
    (void)TextSize;
#endif
    UsedDefaultFont = true;
    return Atlas->AddFontDefault();
}

static ImFont* LoadIconFont(ImFontAtlas* Atlas, float IconSize, bool FixedWidth)
{
    ImFontConfig Config{};
    Config.FontDataOwnedByAtlas = false;
    Config.MergeMode            = false;
    Config.PixelSnapH           = true;
    Config.GlyphMinAdvanceX     = FixedWidth ? IconSize : 0.0f;

    static const ImWchar SolidRanges[] = {0xF000, 0xF8FF, 0};
    return Atlas->AddFontFromMemoryTTF((void*)Resources::FontAwesomeSolidData,
                                       (int)Resources::FontAwesomeSolidDataSize,
                                       IconSize,
                                       &Config,
                                       SolidRanges);
}

static bool LoadBrandIcons(ImFontAtlas* Atlas, float IconSize, bool FixedWidth)
{
    ImFontConfig Config{};
    Config.MergeMode            = true;
    Config.FontDataOwnedByAtlas = false;
    Config.PixelSnapH           = true;
    Config.GlyphMinAdvanceX     = FixedWidth ? IconSize : 0.0f;

    static const ImWchar BrandRanges[] = {0xF09B, 0xF09B, 0xF392, 0xF392, 0};
    return Atlas->AddFontFromMemoryTTF((void*)Resources::FontAwesomeBrandsData,
                                       (int)Resources::FontAwesomeBrandsDataSize,
                                       IconSize,
                                       &Config,
                                       BrandRanges) != nullptr;
}
} // namespace

bool LoadFonts(float TextSize, float IconSize)
{
    ImGuiIO&     Io    = ImGui::GetIO();
    ImFontAtlas* Atlas = Io.Fonts;
    if (!Atlas)
    {
        return false;
    }

    g_Fonts = {};
    Atlas->Clear();

    bool UsedDefaultFont = false;
    g_Fonts.Default      = LoadWindowsTextFont(Atlas, TextSize, UsedDefaultFont);
    if (!g_Fonts.Default)
    {
        return false;
    }
    g_Fonts.UsedDefaultTextFont = UsedDefaultFont;

    g_Fonts.Icon                = LoadIconFont(Atlas, IconSize, false);
    const bool BrandIconsLoaded = LoadBrandIcons(Atlas, IconSize, false);

    g_Fonts.IconFixedWidth           = LoadIconFont(Atlas, IconSize, true);
    const bool FixedBrandIconsLoaded = LoadBrandIcons(Atlas, IconSize, true);

    Io.FontDefault = g_Fonts.Default;
    return g_Fonts.Icon != nullptr && g_Fonts.IconFixedWidth != nullptr && BrandIconsLoaded && FixedBrandIconsLoaded;
}

const FontSet& Fonts()
{
    return g_Fonts;
}

} // namespace RorinnnTools::ImguiRorinnn
