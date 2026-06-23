// RnFonts.cpp — ImguiRorinnn 字体加载

module;

#ifdef _WIN32
#include <Windows.h>
#endif

#include <imgui.h>

module RorinnnTools;
import std;

extern "C"
{
    extern const std::uint8_t _binary_FontAwesomeBrands_bin_start[];
    extern const std::uint8_t _binary_FontAwesomeBrands_bin_end[];
    extern const std::uint8_t _binary_FontAwesomeSolid_bin_start[];
    extern const std::uint8_t _binary_FontAwesomeSolid_bin_end[];
}

namespace RorinnnTools::ImguiRorinnn
{
namespace
{
static FontSet g_Fonts;

static std::size_t GetEmbeddedResourceSize(const std::uint8_t* PStart, const std::uint8_t* PEnd)
{
    const auto Start = reinterpret_cast<std::uintptr_t>(PStart);
    const auto End   = reinterpret_cast<std::uintptr_t>(PEnd);
    return End >= Start ? End - Start : 0;
}

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
    const std::size_t SolidSize = GetEmbeddedResourceSize(_binary_FontAwesomeSolid_bin_start, _binary_FontAwesomeSolid_bin_end);
    if (SolidSize == 0 || SolidSize > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        return nullptr;
    }

    ImFontConfig Config{};
    Config.FontDataOwnedByAtlas = false;
    Config.MergeMode            = false;
    Config.PixelSnapH           = true;
    Config.GlyphMinAdvanceX     = FixedWidth ? IconSize : 0.0f;

    static const ImWchar SolidRanges[] = {0xF000, 0xF8FF, 0};
    return Atlas->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(_binary_FontAwesomeSolid_bin_start),
                                       static_cast<int>(SolidSize),
                                       IconSize,
                                       &Config,
                                       SolidRanges);
}

static bool LoadBrandIcons(ImFontAtlas* Atlas, float IconSize, bool FixedWidth)
{
    const std::size_t BrandsSize =
        GetEmbeddedResourceSize(_binary_FontAwesomeBrands_bin_start, _binary_FontAwesomeBrands_bin_end);
    if (BrandsSize == 0 || BrandsSize > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        return false;
    }

    ImFontConfig Config{};
    Config.MergeMode            = true;
    Config.FontDataOwnedByAtlas = false;
    Config.PixelSnapH           = true;
    Config.GlyphMinAdvanceX     = FixedWidth ? IconSize : 0.0f;

    static const ImWchar BrandRanges[] = {0xF09B, 0xF09B, 0xF392, 0xF392, 0};
    return Atlas->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(_binary_FontAwesomeBrands_bin_start),
                                       static_cast<int>(BrandsSize),
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
