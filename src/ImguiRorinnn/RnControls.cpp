// RnControls.cpp — ImguiRorinnn 常用控件

module;

#include <imgui_internal.h>

#ifdef _WIN32
#include <Windows.h>
#include <shellapi.h>
#endif

module RnTools;
import std;

namespace RnTools::ImguiRorinnn
{
namespace
{
static int  g_ManagedWindowFrame           = -1;
static int  g_VisibleManagedWindowCount    = 0;
static bool g_LastPanelChildContentVisible = false;

struct TableFrame
{
    ImDrawList* DrawList;
    ImVec2      Pos;
    ImVec2      Min;
    ImVec2      Max;
    float       Rounding;
    int         ColorCount;
    int         VarCount;
};

static std::vector<TableFrame> g_TableFrames;

static void EnsureManagedWindowFrame()
{
    const int Frame = ImGui::GetFrameCount();
    if (g_ManagedWindowFrame != Frame)
    {
        g_ManagedWindowFrame        = Frame;
        g_VisibleManagedWindowCount = 0;
    }
}

static ImVec4 StatusColor(StatusKind Kind)
{
    const ColorTokens& C = Colors();
    switch (Kind)
    {
        case StatusKind::Success:
            return C.Success;
        case StatusKind::Warning:
            return C.Warning;
        case StatusKind::Danger:
            return C.Danger;
        case StatusKind::Neutral:
        default:
            return C.TextMuted;
    }
}

static ImVec4 BadgeColor(BadgeVariant Variant)
{
    const ColorTokens& C = Colors();
    switch (Variant)
    {
        case BadgeVariant::Accent:
            return C.Accent;
        case BadgeVariant::Success:
            return C.Success;
        case BadgeVariant::Warning:
            return C.Warning;
        case BadgeVariant::Danger:
            return C.Danger;
        case BadgeVariant::Neutral:
        default:
            return C.TextMuted;
    }
}

static ImVec4 InfoColor(InfoSeverity Severity)
{
    const ColorTokens& C = Colors();
    switch (Severity)
    {
        case InfoSeverity::Success:
            return C.Success;
        case InfoSeverity::Warning:
            return C.Warning;
        case InfoSeverity::Danger:
            return C.Danger;
        case InfoSeverity::Info:
        default:
            return C.Accent;
    }
}

static const char* InfoMark(InfoSeverity Severity)
{
    switch (Severity)
    {
        case InfoSeverity::Success:
            return "OK";
        case InfoSeverity::Warning:
            return "!";
        case InfoSeverity::Danger:
            return "X";
        case InfoSeverity::Info:
        default:
            return "i";
    }
}

static void PushButtonColors(ButtonVariant Variant, StyleColorScope& ColorScope)
{
    const ColorTokens& C = Colors();
    switch (Variant)
    {
        case ButtonVariant::Primary:
            ColorScope.Push(ImGuiCol_Button, C.Accent);
            ColorScope.Push(ImGuiCol_ButtonHovered, C.AccentHover);
            ColorScope.Push(ImGuiCol_ButtonActive, C.AccentActive);
            ColorScope.Push(ImGuiCol_Text, C.AccentText);
            break;
        case ButtonVariant::Ghost:
            ColorScope.Push(ImGuiCol_Button, WithAlpha(C.Surface, 0.0f));
            ColorScope.Push(ImGuiCol_ButtonHovered, WithAlpha(C.SurfaceHover, 0.78f));
            ColorScope.Push(ImGuiCol_ButtonActive, WithAlpha(C.SurfaceActive, 0.88f));
            ColorScope.Push(ImGuiCol_Text, C.Text);
            break;
        case ButtonVariant::Danger:
            ColorScope.Push(ImGuiCol_Button, C.Danger);
            ColorScope.Push(ImGuiCol_ButtonHovered, Blend(C.Danger, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.12f));
            ColorScope.Push(ImGuiCol_ButtonActive, Blend(C.Danger, ImVec4(0.0f, 0.0f, 0.0f, 1.0f), 0.16f));
            ColorScope.Push(ImGuiCol_Text, C.AccentText);
            break;
        case ButtonVariant::Secondary:
        default:
            ColorScope.Push(ImGuiCol_Button, C.Surface);
            ColorScope.Push(ImGuiCol_ButtonHovered, C.SurfaceHover);
            ColorScope.Push(ImGuiCol_ButtonActive, C.SurfaceActive);
            ColorScope.Push(ImGuiCol_Text, C.Text);
            break;
    }
}

static float CalcButtonAutoWidth(const char* Label)
{
    const char*       TextValue = Label ? Label : "";
    const ImVec2      TextSize  = ImGui::CalcTextSize(TextValue, nullptr, true);
    const SizeTokens& S         = Sizes();
    return TextSize.x + S.FramePadding.x * 2.0f + 8.0f;
}

static bool HasVisibleLabel(const char* Label)
{
    if (!Label)
        return false;
    return Label[0] != '#' || Label[1] != '#';
}

static ImFont* ResolveIconFont(bool FixedWidth)
{
    const FontSet& FontSetValue = Fonts();
    ImFont*        Font         = FixedWidth ? FontSetValue.IconFixedWidth : FontSetValue.Icon;
    return Font ? Font : ImGui::GetFont();
}

static void ShowTooltip(const char* Tooltip)
{
    if (Tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        ImGui::SetTooltip("%s", Tooltip);
}

static ImGuiID ChildAnimId(ImGuiID Id, const char* Name)
{
    return ImHashStr(Name, 0, Id);
}

static bool IsCurrentWindowInteractive()
{
    ImGuiWindow* Window        = ImGui::GetCurrentWindow();
    ImGuiWindow* HoveredWindow = GImGui ? GImGui->HoveredWindow : nullptr;
    return Window && HoveredWindow && ImGui::IsWindowChildOf(HoveredWindow, Window->RootWindow, true);
}

static bool IsRectInteractive(const ImRect& Bounds)
{
    return IsCurrentWindowInteractive() && ImGui::IsMouseHoveringRect(Bounds.Min, Bounds.Max);
}

static void DrawInputTextState(ImGuiID Id, const ImVec2& FrameMin, const ImVec2& FrameMax)
{
    const bool  Focused = ImGui::IsItemActive() || ImGui::IsItemFocused();
    const float FocusT  = AnimateBool(ChildAnimId(Id, "InputFocus"), Focused, 18.0f);
    if (FocusT <= 0.01f)
        return;

    const ColorTokens& C        = Colors();
    const SizeTokens&  S        = Sizes();
    const float        Expand   = 1.0f;
    ImDrawList*        DrawList = ImGui::GetWindowDrawList();

    DrawList->AddRect(ImVec2(FrameMin.x - Expand, FrameMin.y - Expand),
                      ImVec2(FrameMax.x + Expand, FrameMax.y + Expand),
                      ToU32(WithAlpha(C.Accent, 0.36f + FocusT * 0.42f)),
                      S.ControlRounding + Expand,
                      0,
                      std::max(1.0f, S.BorderWidth + FocusT));

    ImGuiInputTextState* State = ImGui::GetInputTextState(Id);
    if (!Focused || !State)
        return;

    const char* TextValue = State->GetText();
    const int   CursorPos = std::clamp(State->GetCursorPos(), 0, State->TextLen);
    const char* CursorEnd = TextValue + CursorPos;
    const float TextWidth = ImGui::CalcTextSize(TextValue, CursorEnd).x;
    const float CursorX   = FrameMin.x + S.FramePadding.x + TextWidth - State->Scroll.x;
    if (CursorX < FrameMin.x + S.FramePadding.x * 0.5f || CursorX > FrameMax.x - S.FramePadding.x * 0.5f)
        return;

    const float CursorTop    = FrameMin.y + S.FramePadding.y;
    const float CursorBottom = FrameMax.y - S.FramePadding.y;
    const float Blink        = (std::sin((float)ImGui::GetTime() * 7.5f) + 1.0f) * 0.5f;
    const ImU32 ShadowColor  = ToU32(WithAlpha(C.Surface, 0.80f));
    const ImU32 CursorColor  = ToU32(WithAlpha(C.Accent, 0.72f + Blink * 0.28f));
    DrawList->AddLine(ImVec2(CursorX + 1.0f, CursorTop), ImVec2(CursorX + 1.0f, CursorBottom), ShadowColor, 2.0f);
    DrawList->AddLine(
        ImVec2(CursorX, CursorTop), ImVec2(CursorX, CursorBottom), CursorColor, 2.0f);
}

} // namespace

void Text(const char* Value)
{
    ImGui::TextUnformatted(Value ? Value : "");
}

void TextMuted(const char* Value)
{
    ImGui::PushStyleColor(ImGuiCol_Text, Colors().TextMuted);
    ImGui::TextUnformatted(Value ? Value : "");
    ImGui::PopStyleColor();
}

void TextDisabled(const char* Value)
{
    ImGui::PushStyleColor(ImGuiCol_Text, Colors().TextDisabled);
    ImGui::TextUnformatted(Value ? Value : "");
    ImGui::PopStyleColor();
}

void Heading(const char* Value)
{
    ImGui::PushStyleColor(ImGuiCol_Text, Colors().Text);
    ImGui::TextUnformatted(Value ? Value : "");
    ImGui::PopStyleColor();
}

bool Hyperlink(const char* Label, const char* Url)
{
    ImGuiWindow* Window = ImGui::GetCurrentWindow();
    if (Window->SkipItems)
        return false;

    const ColorTokens& C         = Colors();
    const ImGuiID      Id        = ImGui::GetID(Label);
    const char*        TextValue = Label ? Label : "";
    const ImVec2       Pos       = ImGui::GetCursorScreenPos();
    const ImVec2       TextSize  = ImGui::CalcTextSize(TextValue);
    const ImRect       Bounds(Pos, ImVec2(Pos.x + TextSize.x, Pos.y + TextSize.y + 2.0f));

    ImGui::ItemSize(Bounds);
    if (!ImGui::ItemAdd(Bounds, Id))
        return false;

    bool         Hovered   = false;
    bool         Held      = false;
    const bool   Pressed   = ImGui::ButtonBehavior(Bounds, Id, &Hovered, &Held);
    const float  HoverT    = AnimateBool(ChildAnimId(Id, "HyperlinkHover"), Hovered, 22.0f);
    const ImVec4 TextColor = Blend(C.Accent, C.AccentHover, Held ? 1.0f : HoverT);

    Window->DrawList->AddText(Pos, ToU32(TextColor), TextValue);
    if (HoverT > 0.01f)
        Window->DrawList->AddLine(ImVec2(Pos.x, Bounds.Max.y - 1.0f),
                                  ImVec2(Pos.x + TextSize.x, Bounds.Max.y - 1.0f),
                                  ToU32(WithMultipliedAlpha(TextColor, HoverT)),
                                  Sizes().BorderWidth);

    if (Pressed && Url && Url[0])
    {
#ifdef _WIN32
        ShellExecuteA(nullptr, "open", Url, nullptr, nullptr, SW_SHOWNORMAL);
#endif
    }
    return Pressed;
}

void StatusText(const char* Value, StatusKind Kind)
{
    const ColorTokens& C        = Colors();
    const SizeTokens&  S        = Sizes();
    const ImVec2       TextSize = ImGui::CalcTextSize(Value ? Value : "");
    const ImVec2       Start    = ImGui::GetCursorScreenPos();
    const ImVec2       Size(TextSize.x + S.FramePadding.x * 1.55f, TextSize.y + S.CompactFramePadding.y * 1.55f);
    const ImVec4       Color = StatusColor(Kind);

    ImGui::Dummy(Size);
    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    DrawList->AddRectFilled(
        Start, ImVec2(Start.x + Size.x, Start.y + Size.y), ToU32(WithAlpha(Color, 0.16f)), S.SmallRounding);
    DrawList->AddText(ImVec2(Start.x + S.FramePadding.x * 0.75f, Start.y + S.CompactFramePadding.y * 0.75f),
                      ToU32(Blend(Color, C.Text, 0.20f)),
                      Value ? Value : "");
}

void Badge(const char* Label, BadgeVariant Variant)
{
    const ColorTokens& C         = Colors();
    const SizeTokens&  S         = Sizes();
    const char*        TextValue = Label ? Label : "";
    const ImVec2       TextSize  = ImGui::CalcTextSize(TextValue);
    const ImVec2       Pos       = ImGui::GetCursorScreenPos();
    const ImVec2       Size(TextSize.x + S.FramePadding.x * 1.45f, TextSize.y + S.CompactFramePadding.y * 1.30f);
    const ImVec4       Color = BadgeColor(Variant);

    ImGui::Dummy(Size);
    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    DrawList->AddRectFilled(Pos, ImVec2(Pos.x + Size.x, Pos.y + Size.y), ToU32(WithAlpha(Color, 0.15f)), Size.y * 0.5f);
    DrawList->AddRect(
        Pos, ImVec2(Pos.x + Size.x, Pos.y + Size.y), ToU32(WithAlpha(Color, 0.35f)), Size.y * 0.5f, 0, S.BorderWidth);
    DrawList->AddText(ImVec2(Pos.x + S.FramePadding.x * 0.72f, Pos.y + S.CompactFramePadding.y * 0.65f),
                      ToU32(Blend(Color, C.Text, 0.18f)),
                      TextValue);
}

void BadgeDot(BadgeVariant Variant, float Radius)
{
    const float  ActualRadius = Radius > 0.0f ? Radius : 4.0f;
    const ImVec2 Pos          = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(ActualRadius * 2.0f, ActualRadius * 2.0f));
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(Pos.x + ActualRadius, Pos.y + ActualRadius), ActualRadius, ToU32(BadgeColor(Variant)));
}

void BadgeNumber(int Value, BadgeVariant Variant)
{
    char Buffer[24] = {};
    if (Value > 99)
        std::snprintf(Buffer, sizeof(Buffer), "99+");
    else
        std::snprintf(Buffer, sizeof(Buffer), "%d", Value);
    Badge(Buffer, Variant);
}

bool InfoBar(const char* Message, InfoSeverity Severity, bool* Open, const char* ActionLabel)
{
    if (Open && !*Open)
        return false;

    ImGuiWindow* Window = ImGui::GetCurrentWindow();
    if (Window->SkipItems)
        return false;

    const ColorTokens& C         = Colors();
    const SizeTokens&  S         = Sizes();
    const char*        TextValue = Message ? Message : "";
    const ImVec4       Accent    = InfoColor(Severity);
    const ImVec2       Pos       = ImGui::GetCursorScreenPos();
    const float        Width     = ImGui::GetContentRegionAvail().x;
    const float        Height    = S.ControlHeight + S.PanelPadding.y * 1.25f;
    const ImRect       Bounds(Pos, ImVec2(Pos.x + Width, Pos.y + Height));
    const ImGuiID      Id = ImGui::GetID(TextValue);

    ImGui::ItemSize(Bounds);
    if (!ImGui::ItemAdd(Bounds, Id))
        return false;

    Window->DrawList->AddRectFilled(Bounds.Min, Bounds.Max, ToU32(WithAlpha(Accent, 0.13f)), S.ControlRounding);
    Window->DrawList->AddRect(
        Bounds.Min, Bounds.Max, ToU32(WithAlpha(Accent, 0.32f)), S.ControlRounding, 0, S.BorderWidth);

    const float CenterY    = Bounds.Min.y + Height * 0.5f;
    const float MarkRadius = 8.0f;
    float       CursorX    = Bounds.Min.x + S.PanelPadding.x + MarkRadius;
    Window->DrawList->AddCircleFilled(ImVec2(CursorX, CenterY), MarkRadius, ToU32(Accent));

    const char*  Mark     = InfoMark(Severity);
    const ImVec2 MarkSize = ImGui::CalcTextSize(Mark);
    Window->DrawList->AddText(
        ImVec2(CursorX - MarkSize.x * 0.5f, CenterY - MarkSize.y * 0.5f), ToU32(C.AccentText), Mark);

    CursorX                  += MarkRadius + S.ItemGap;
    const ImVec2 MessageSize  = ImGui::CalcTextSize(TextValue);
    Window->DrawList->AddText(ImVec2(CursorX, CenterY - MessageSize.y * 0.5f), ToU32(C.Text), TextValue);

    bool ActionClicked = false;
    if (ActionLabel && ActionLabel[0])
    {
        const ImVec2 ActionSize = ImGui::CalcTextSize(ActionLabel);
        const ImRect ActionBounds(ImVec2(Bounds.Max.x - S.PanelPadding.x - ActionSize.x - S.FramePadding.x,
                                         CenterY - ActionSize.y * 0.5f - S.CompactFramePadding.y),
                                  ImVec2(Bounds.Max.x - S.PanelPadding.x + S.FramePadding.x,
                                         CenterY + ActionSize.y * 0.5f + S.CompactFramePadding.y));
        const bool   Hovered = IsRectInteractive(ActionBounds);
        ActionClicked        = Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        Window->DrawList->AddText(ImVec2(ActionBounds.Min.x + S.FramePadding.x, CenterY - ActionSize.y * 0.5f),
                                  ToU32(Hovered ? C.AccentHover : C.Accent),
                                  ActionLabel);
    }

    return ActionClicked;
}

void HelpMarker(const char* TextValue)
{
    ImFont* IconFont = ResolveIconFont(true);
    ImGui::PushFont(IconFont);
    ImGui::TextDisabled("%s", ToIconString(Icon::CircleQuestion));
    ImGui::PopFont();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && TextValue)
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(TextValue);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool Button(const char* Label, const ImVec2& Size, ButtonVariant Variant)
{
    StyleColorScope ColorScope;
    StyleVarScope   VarScope;
    PushButtonColors(Variant, ColorScope);
    VarScope.Push(ImGuiStyleVar_FrameRounding, Sizes().ControlRounding);
    VarScope.Push(ImGuiStyleVar_FramePadding, Sizes().FramePadding);

    ImVec2 ActualSize = Size;
    if (ActualSize.x <= 0.0f)
        ActualSize.x = CalcButtonAutoWidth(Label);
    if (ActualSize.y <= 0.0f)
        ActualSize.y = Sizes().ControlHeight;
    return ImGui::Button(Label, ActualSize);
}

bool PrimaryButton(const char* Label, const ImVec2& Size)
{
    return Button(Label, Size, ButtonVariant::Primary);
}

bool GhostButton(const char* Label, const ImVec2& Size)
{
    return Button(Label, Size, ButtonVariant::Ghost);
}

bool DangerButton(const char* Label, const ImVec2& Size)
{
    return Button(Label, Size, ButtonVariant::Danger);
}

bool IconButton(const char* Id, const char* Icon, const char* Tooltip, const ImVec2& Size)
{
    IconButtonOptions Options{};
    Options.Tooltip    = Tooltip;
    Options.Size       = Size;
    const bool Pressed = IconActionButton(Id, Options);
    if (Icon && Icon[0])
    {
        const ImRect Bounds(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        const ImVec2 IconSize = ImGui::CalcTextSize(Icon, nullptr, true);
        const ImVec2 IconPos(Bounds.Min.x + (Bounds.GetWidth() - IconSize.x) * 0.5f,
                             Bounds.Min.y + (Bounds.GetHeight() - IconSize.y) * 0.5f);
        ImGui::GetWindowDrawList()->AddText(IconPos, ImGui::GetColorU32(ImGuiCol_Text), Icon);
    }
    return Pressed;
}

bool IconButton(const char* Id, Icon IconValue, const char* Tooltip, const ImVec2& Size)
{
    IconButtonOptions Options{};
    Options.IconValue = IconValue;
    Options.Tooltip   = Tooltip;
    Options.Size      = Size;
    return IconActionButton(Id, Options);
}

bool IconActionButton(const char* Id, const IconButtonOptions& Options)
{
    StyleColorScope ColorScope;
    StyleVarScope   VarScope;
    PushButtonColors(Options.Variant, ColorScope);
    if (Options.Active)
    {
        ColorScope.Push(ImGuiCol_Button, Colors().SurfaceActive);
        ColorScope.Push(ImGuiCol_Text, Colors().AccentText);
    }
    VarScope.Push(ImGuiStyleVar_FrameRounding, Sizes().ControlRounding);
    VarScope.Push(ImGuiStyleVar_FramePadding, Sizes().FramePadding);

    const ImVec2 ActualSize(Options.Size.x > 0.0f ? Options.Size.x : Sizes().IconButtonSize,
                            Options.Size.y > 0.0f ? Options.Size.y : Sizes().IconButtonSize);

    bool Pressed = false;
    {
        DisabledScope Disabled(!Options.Enabled);
        ImGui::PushID(Id);
        Pressed = ImGui::Button("", ActualSize);
        ImGui::PopID();
    }

    ShowTooltip(Options.Tooltip);

    const char* IconText = ToIconString(Options.IconValue);
    if (IconText[0])
    {
        ImFont*      IconFont = ResolveIconFont(Options.FixedWidth);
        ImDrawList*  DrawList = ImGui::GetWindowDrawList();
        const ImRect Bounds(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        const float  FontSize = ImGui::GetFontSize();
        const ImVec2 IconSize = IconFont->CalcTextSizeA(FontSize, FLT_MAX, 0.0f, IconText);
        const ImVec2 IconPos(Bounds.Min.x + (Bounds.GetWidth() - IconSize.x) * 0.5f,
                             Bounds.Min.y + (Bounds.GetHeight() - IconSize.y) * 0.5f);
        const ImU32  IconColor =
            Options.Enabled ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_TextDisabled);
        DrawList->AddText(IconFont, FontSize, IconPos, IconColor, IconText);
    }

    return Pressed;
}

bool SettingsButton(const char* Id, bool Active)
{
    IconButtonOptions Options{};
    Options.IconValue = Icon::Gear;
    Options.Tooltip   = "设置";
    Options.Active    = Active;
    return IconActionButton(Id, Options);
}

bool HelpButton(const char* Id, const char* Tooltip)
{
    IconButtonOptions Options{};
    Options.IconValue = Icon::CircleQuestion;
    Options.Tooltip   = Tooltip;
    return IconActionButton(Id, Options);
}

bool DiscordButton(const char* Id)
{
    IconButtonOptions Options{};
    Options.IconValue = Icon::Discord;
    Options.Tooltip   = "Discord";
    return IconActionButton(Id, Options);
}

bool IconButtonWithText(const char* Id, Icon IconValue, const char* Label, const ImVec2& Size, ButtonVariant Variant)
{
    StyleColorScope ColorScope;
    StyleVarScope   VarScope;
    PushButtonColors(Variant, ColorScope);
    VarScope.Push(ImGuiStyleVar_FrameRounding, Sizes().ControlRounding);
    VarScope.Push(ImGuiStyleVar_FramePadding, Sizes().FramePadding);

    const char*  IconText    = ToIconString(IconValue);
    ImFont*      IconFont    = ResolveIconFont(true);
    const float  FontSize    = ImGui::GetFontSize();
    const ImVec2 IconSize    = IconFont->CalcTextSizeA(FontSize, FLT_MAX, 0.0f, IconText);
    const ImVec2 TextSize    = ImGui::CalcTextSize(Label, nullptr, true);
    const float  IconPadding = 3.0f;
    ImVec2       ActualSize  = Size;
    if (ActualSize.x <= 0.0f)
        ActualSize.x = IconSize.x + TextSize.x + ImGui::GetStyle().FramePadding.x * 2.0f + IconPadding;
    if (ActualSize.y <= 0.0f)
        ActualSize.y = Sizes().ControlHeight;

    ImGui::PushID(Id);
    const bool Pressed = ImGui::Button("", ActualSize);
    ImGui::PopID();

    const ImRect Bounds(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    const ImVec2 ContentSize(IconSize.x + IconPadding + TextSize.x, ImMax(IconSize.y, TextSize.y));
    const float  StartX   = Bounds.Min.x + (Bounds.GetWidth() - ContentSize.x) * 0.5f;
    const float  CenterY  = Bounds.Min.y + Bounds.GetHeight() * 0.5f;
    ImDrawList*  DrawList = ImGui::GetWindowDrawList();
    DrawList->AddText(
        IconFont, FontSize, ImVec2(StartX, CenterY - IconSize.y * 0.5f), ImGui::GetColorU32(ImGuiCol_Text), IconText);
    DrawList->AddText(ImVec2(StartX + IconSize.x + IconPadding, CenterY - TextSize.y * 0.5f),
                      ImGui::GetColorU32(ImGuiCol_Text),
                      Label);

    return Pressed;
}

bool Checkbox(const char* Label, bool* Value)
{
    StyleColorScope ColorScope;
    ColorScope.Push(ImGuiCol_CheckMark, Colors().AccentText);
    ColorScope.Push(ImGuiCol_FrameBg, Colors().Surface);
    ColorScope.Push(ImGuiCol_FrameBgHovered, Colors().SurfaceHover);
    ColorScope.Push(ImGuiCol_FrameBgActive, Colors().Accent);
    return ImGui::Checkbox(Label, Value);
}

bool CheckboxPill(const char* Label, bool* Value)
{
    ImGuiWindow* Window = ImGui::GetCurrentWindow();
    if (Window->SkipItems)
        return false;

    const ColorTokens& C         = Colors();
    const SizeTokens&  S         = Sizes();
    const ImGuiID      Id        = ImGui::GetID(Label);
    const ImVec2       LabelSize = ImGui::CalcTextSize(Label, nullptr, true);
    const float        Height    = S.ControlHeight;
    const float        CheckSize = 18.0f;
    const float        PaddingX  = 9.0f;
    const float        TotalWidth = PaddingX * 2.0f + CheckSize + 6.0f + LabelSize.x;
    const ImVec2       Pos        = ImGui::GetCursorScreenPos();
    const ImRect       Bounds(Pos, ImVec2(Pos.x + TotalWidth, Pos.y + Height));

    ImGui::ItemSize(Bounds);
    if (!ImGui::ItemAdd(Bounds, Id))
        return false;

    bool       Hovered = false;
    bool       Held    = false;
    const bool Pressed = ImGui::ButtonBehavior(Bounds, Id, &Hovered, &Held);
    if (Pressed && Value)
        *Value = !*Value;

    const bool   On         = Value && *Value;
    const float  HoverT     = AnimateBool(ChildAnimId(Id, "CheckboxPillHover"), Hovered, 20.0f);
    const float  OnT        = SmoothValue(ChildAnimId(Id, "CheckboxPillOn"), On ? 1.0f : 0.0f, 18.0f, On ? 1.0f : 0.0f);
    const ImVec4 OffFill    = Blend(C.Surface, C.SurfaceHover, HoverT);
    const ImVec4 Fill       = Blend(OffFill, WithAlpha(C.Accent, 0.22f), OnT);
    const ImVec4 Border     = Blend(C.Border, WithAlpha(C.Accent, 0.52f), OnT);
    const ImVec4 TextColor  = Blend(C.TextMuted, C.Text, OnT);
    const float  Rounding   = S.ControlRounding;
    ImDrawList*  DrawList   = Window->DrawList;

    DrawList->AddRectFilled(Bounds.Min, Bounds.Max, ToU32(Fill), Rounding);
    DrawList->AddRect(Bounds.Min, Bounds.Max, ToU32(Border), Rounding, 0, S.BorderWidth);

    const ImVec2 CheckMin(Bounds.Min.x + PaddingX, Bounds.Min.y + (Height - CheckSize) * 0.5f);
    const ImVec2 CheckMax(CheckMin.x + CheckSize, CheckMin.y + CheckSize);
    DrawList->AddRectFilled(CheckMin, CheckMax, ToU32(On ? C.Accent : C.Surface), S.SmallRounding);
    DrawList->AddRect(CheckMin, CheckMax, ToU32(On ? WithAlpha(KnownColor::Black, 0.42f) : C.Border), S.SmallRounding, 0, S.BorderWidth);
    if (On)
        ImGui::RenderCheckMark(DrawList,
                               ImVec2(CheckMin.x + 3.0f, CheckMin.y + 3.0f),
                               ToU32(KnownColor::Black),
                               CheckSize - 6.0f);

    DrawList->AddText(ImVec2(CheckMax.x + 6.0f, Bounds.Min.y + (Height - LabelSize.y) * 0.5f), ToU32(TextColor), Label);
    return Pressed;
}

bool Toggle(const char* Label, bool* Value)
{
    ImGuiWindow* Window = ImGui::GetCurrentWindow();
    if (Window->SkipItems)
        return false;

    const ColorTokens& C          = Colors();
    const SizeTokens&  S          = Sizes();
    const ImGuiID      Id         = ImGui::GetID(Label);
    const float        Height     = S.SmallControlHeight;
    const float        Width      = Height * 1.82f;
    const ImVec2       LabelSize  = ImGui::CalcTextSize(Label, nullptr, true);
    const ImVec2       Pos        = ImGui::GetCursorScreenPos();
    const float        TotalWidth = Width + S.ItemGap + LabelSize.x;
    const ImRect       Bounds(Pos, ImVec2(Pos.x + TotalWidth, Pos.y + ImMax(Height, LabelSize.y)));

    ImGui::ItemSize(Bounds);
    if (!ImGui::ItemAdd(Bounds, Id))
        return false;

    bool       Hovered = false;
    bool       Held    = false;
    const bool Pressed = ImGui::ButtonBehavior(Bounds, Id, &Hovered, &Held);
    if (Pressed && Value)
        *Value = !*Value;

    const bool   On         = Value && *Value;
    const float  HoverT     = AnimateBool(ChildAnimId(Id, "ToggleHover"), Hovered, 20.0f);
    const float  OnT        = SmoothValue(ChildAnimId(Id, "ToggleOn"), On ? 1.0f : 0.0f, 18.0f, On ? 1.0f : 0.0f);
    const ImVec4 OffTrack   = Blend(C.Surface, C.SurfaceHover, HoverT);
    const ImVec4 TrackColor = Blend(OffTrack, C.Accent, OnT);
    const ImVec4 ThumbColor = On ? C.AccentText : C.TextMuted;
    const ImVec2 TrackMin(Pos.x, Pos.y + (Bounds.GetHeight() - Height) * 0.5f);
    const ImVec2 TrackMax(TrackMin.x + Width, TrackMin.y + Height);
    const float  Radius      = Height * 0.5f;
    const float  ThumbRadius = Height * 0.32f;
    const float  ThumbX      = TrackMin.x + Radius + (Width - Height) * OnT;
    const ImVec2 ThumbCenter(ThumbX, TrackMin.y + Radius);

    Window->DrawList->AddRectFilled(TrackMin, TrackMax, ToU32(TrackColor), Radius);
    Window->DrawList->AddRect(TrackMin, TrackMax, ToU32(C.Border), Radius, 0, S.BorderWidth);
    Window->DrawList->AddCircleFilled(ThumbCenter, ThumbRadius, ToU32(ThumbColor));
    Window->DrawList->AddText(
        ImVec2(TrackMax.x + S.ItemGap, Pos.y + (Bounds.GetHeight() - LabelSize.y) * 0.5f), ToU32(C.Text), Label);
    return Pressed;
}

bool SliderFloat(const char* Label, float* Value, float Min, float Max, const char* Format)
{
    StyleColorScope ColorScope;
    StyleVarScope   VarScope;
    ColorScope.Push(ImGuiCol_FrameBg, Colors().Surface);
    ColorScope.Push(ImGuiCol_FrameBgHovered, Colors().SurfaceHover);
    ColorScope.Push(ImGuiCol_FrameBgActive, Colors().SurfaceActive);
    ColorScope.Push(ImGuiCol_SliderGrab, Colors().Accent);
    ColorScope.Push(ImGuiCol_SliderGrabActive, Colors().AccentHover);
    VarScope.Push(ImGuiStyleVar_GrabRounding, Sizes().ControlRounding);
    return ImGui::SliderFloat(Label, Value, Min, Max, Format);
}

bool SliderInt(const char* Label, int* Value, int Min, int Max)
{
    StyleColorScope ColorScope;
    StyleVarScope   VarScope;
    ColorScope.Push(ImGuiCol_FrameBg, Colors().Surface);
    ColorScope.Push(ImGuiCol_FrameBgHovered, Colors().SurfaceHover);
    ColorScope.Push(ImGuiCol_FrameBgActive, Colors().SurfaceActive);
    ColorScope.Push(ImGuiCol_SliderGrab, Colors().Accent);
    ColorScope.Push(ImGuiCol_SliderGrabActive, Colors().AccentHover);
    VarScope.Push(ImGuiStyleVar_GrabRounding, Sizes().ControlRounding);
    return ImGui::SliderInt(Label, Value, Min, Max);
}

bool SliderIntMapped(const char* Label, int* Value, int Min, int Max, int DisplayMultiplier, const char* DisplayFormat)
{
    StyleColorScope ColorScope;
    StyleVarScope   VarScope;
    ColorScope.Push(ImGuiCol_FrameBg, Colors().Surface);
    ColorScope.Push(ImGuiCol_FrameBgHovered, Colors().SurfaceHover);
    ColorScope.Push(ImGuiCol_FrameBgActive, Colors().SurfaceActive);
    ColorScope.Push(ImGuiCol_SliderGrab, Colors().Accent);
    ColorScope.Push(ImGuiCol_SliderGrabActive, Colors().AccentHover);
    VarScope.Push(ImGuiStyleVar_GrabRounding, Sizes().ControlRounding);

    const int    Multiplier      = DisplayMultiplier == 0 ? 1 : DisplayMultiplier;
    bool         Changed         = ImGui::SliderInt(Label, Value, Min, Max, "");
    const ImVec2 ItemMin         = ImGui::GetItemRectMin();
    const ImVec2 ItemMax         = ImGui::GetItemRectMax();
    char         DisplayText[32] = {};
    ImFormatString(DisplayText, IM_ARRAYSIZE(DisplayText), DisplayFormat ? DisplayFormat : "%d", (*Value) * Multiplier);
    const ImVec2 TextSize = ImGui::CalcTextSize(DisplayText);
    ImGui::GetWindowDrawList()->AddText(ImVec2(ItemMin.x + (ItemMax.x - ItemMin.x - TextSize.x) * 0.5f,
                                               ItemMin.y + (ItemMax.y - ItemMin.y - TextSize.y) * 0.5f),
                                        ImGui::GetColorU32(ImGuiCol_Text),
                                        DisplayText);
    return Changed;
}

bool Combo(const char* Label, int* CurrentItem, const char* const Items[], int ItemCount)
{
    StyleColorScope ColorScope;
    StyleVarScope   VarScope;
    ColorScope.Push(ImGuiCol_FrameBg, Colors().Surface);
    ColorScope.Push(ImGuiCol_FrameBgHovered, Colors().SurfaceHover);
    ColorScope.Push(ImGuiCol_FrameBgActive, Colors().SurfaceActive);
    ColorScope.Push(ImGuiCol_PopupBg, Colors().Overlay);
    VarScope.Push(ImGuiStyleVar_FrameRounding, Sizes().ControlRounding);
    VarScope.Push(ImGuiStyleVar_PopupRounding, Sizes().PanelRounding);
    return ImGui::Combo(Label, CurrentItem, Items, ItemCount);
}

bool SegmentedControl(const char* Id, int* CurrentItem, const char* const Items[], int ItemCount, const ImVec2& Size)
{
    ImGuiWindow* Window = ImGui::GetCurrentWindow();
    if (Window->SkipItems || !CurrentItem || !Items || ItemCount <= 0)
        return false;

    ImGui::PushID(Id);
    const ColorTokens& C        = Colors();
    const SizeTokens&  S        = Sizes();
    const ImGuiID      WidgetId = ImGui::GetID("##SegmentedControl");
    const ImVec2       Pos      = ImGui::GetCursorScreenPos();
    const float        Width    = Size.x > 0.0f ? Size.x : ImGui::GetContentRegionAvail().x;
    const float        Height   = Size.y > 0.0f ? Size.y : S.ControlHeight;
    const ImRect       Bounds(Pos, ImVec2(Pos.x + Width, Pos.y + Height));
    const float        SegmentWidth   = Width / (float)ItemCount;
    const int          ClampedCurrent = std::clamp(*CurrentItem, 0, ItemCount - 1);

    ImGui::ItemSize(Bounds);
    if (!ImGui::ItemAdd(Bounds, WidgetId))
    {
        ImGui::PopID();
        return false;
    }

    bool Hovered = false;
    bool Held    = false;
    bool Changed = false;
    const bool Pressed = ImGui::ButtonBehavior(Bounds, WidgetId, &Hovered, &Held);
    int        HoveredIndex = -1;
    if (Hovered)
        HoveredIndex = std::clamp((int)((ImGui::GetIO().MousePos.x - Pos.x) / SegmentWidth), 0, ItemCount - 1);
    if (Pressed && HoveredIndex >= 0 && *CurrentItem != HoveredIndex)
    {
        *CurrentItem = HoveredIndex;
        Changed      = true;
    }

    Window->DrawList->AddRectFilled(Bounds.Min, Bounds.Max, ToU32(C.Surface), S.ControlRounding);
    Window->DrawList->AddRect(Bounds.Min, Bounds.Max, ToU32(C.Border), S.ControlRounding, 0, S.BorderWidth);

    const float AnimatedIndex =
        SmoothValue(ChildAnimId(WidgetId, "SegmentedIndex"), (float)ClampedCurrent, 18.0f, (float)ClampedCurrent);
    const float  Inset = 3.0f;
    const ImVec2 IndicatorMin(Pos.x + AnimatedIndex * SegmentWidth + Inset, Pos.y + Inset);
    const ImVec2 IndicatorMax(IndicatorMin.x + SegmentWidth - Inset * 2.0f, Pos.y + Height - Inset);
    Window->DrawList->AddRectFilled(
        IndicatorMin, IndicatorMax, ToU32(C.Accent), std::max(0.0f, S.ControlRounding - 2.0f));

    for (int Index = 0; Index < ItemCount; Index++)
    {
        const ImVec2 SegmentMin(Pos.x + SegmentWidth * (float)Index, Pos.y);
        const ImVec2 SegmentMax(SegmentMin.x + SegmentWidth, Pos.y + Height);
        if (HoveredIndex == Index && Index != ClampedCurrent)
            Window->DrawList->AddRectFilled(ImVec2(SegmentMin.x + Inset, SegmentMin.y + Inset),
                                            ImVec2(SegmentMax.x - Inset, SegmentMax.y - Inset),
                                            ToU32(WithAlpha(C.SurfaceHover, 0.72f)),
                                            std::max(0.0f, S.ControlRounding - 2.0f));

        const char*  TextValue = Items[Index] ? Items[Index] : "";
        const ImVec2 TextSize  = ImGui::CalcTextSize(TextValue);
        const float  SelectedT = std::clamp(1.0f - std::fabs(AnimatedIndex - (float)Index), 0.0f, 1.0f);
        const ImVec4 TextColor = Blend(C.Text, KnownColor::Black, EaseValue(Ease::OutCubic, SelectedT));
        Window->DrawList->AddText(
            ImVec2(SegmentMin.x + (SegmentWidth - TextSize.x) * 0.5f, Pos.y + (Height - TextSize.y) * 0.5f),
            ToU32(TextColor),
            TextValue);
    }
    ImGui::PopID();
    return Changed;
}

bool InputText(const char* Label, char* Buffer, std::size_t BufferSize, const char* Hint, ImGuiInputTextFlags Flags)
{
    StyleColorScope ColorScope;
    StyleVarScope   VarScope;
    const ImGuiID   Id = ImGui::GetID(Label);
    ColorScope.Push(ImGuiCol_FrameBg, Colors().Surface);
    ColorScope.Push(ImGuiCol_FrameBgHovered, Colors().SurfaceHover);
    ColorScope.Push(ImGuiCol_FrameBgActive, Colors().SurfaceActive);
    ColorScope.Push(ImGuiCol_Border, Colors().Border);
    ColorScope.Push(ImGuiCol_TextSelectedBg, WithAlpha(Colors().Accent, 0.34f));
    VarScope.Push(ImGuiStyleVar_FrameRounding, Sizes().ControlRounding);
    VarScope.Push(ImGuiStyleVar_FramePadding, Sizes().FramePadding);
    VarScope.Push(ImGuiStyleVar_FrameBorderSize, Sizes().BorderWidth);

    const float  InputWidth = ImMax(Sizes().InputMinWidth, ImGui::CalcItemWidth());
    const ImVec2 FrameMin   = ImGui::GetCursorScreenPos();
    ImGui::SetNextItemWidth(InputWidth);
    bool Changed = false;
    if (Hint && HasVisibleLabel(Label))
    {
        Changed = ImGui::InputTextWithHint(Label, Hint, Buffer, BufferSize, Flags);
        DrawInputTextState(Id, FrameMin, ImVec2(FrameMin.x + InputWidth, ImGui::GetItemRectMax().y));
        return Changed;
    }
    if (Hint)
    {
        Changed = ImGui::InputTextWithHint(Label, Hint, Buffer, BufferSize, Flags);
        DrawInputTextState(Id, FrameMin, ImVec2(FrameMin.x + InputWidth, ImGui::GetItemRectMax().y));
        return Changed;
    }
    Changed = ImGui::InputText(Label, Buffer, BufferSize, Flags);
    DrawInputTextState(Id, FrameMin, ImVec2(FrameMin.x + InputWidth, ImGui::GetItemRectMax().y));
    return Changed;
}

bool InputInt(const char* Label, int* Value)
{
    StyleVarScope VarScope(ImGuiStyleVar_FrameRounding, Sizes().ControlRounding);
    return ImGui::InputInt(Label, Value);
}

bool InputFloat(const char* Label, float* Value, const char* Format)
{
    StyleVarScope VarScope(ImGuiStyleVar_FrameRounding, Sizes().ControlRounding);
    return ImGui::InputFloat(Label, Value, 0.0f, 0.0f, Format);
}

void ProgressBar(const char* Id, float Fraction, const ImVec2& Size, const char* Overlay)
{
    const ColorTokens& C             = Colors();
    const SizeTokens&  S             = Sizes();
    const ImGuiID      WidgetId      = ImGui::GetID(Id);
    const ImVec2       Pos           = ImGui::GetCursorScreenPos();
    const float        Width         = Size.x > 0.0f ? Size.x : ImGui::GetContentRegionAvail().x;
    const float        Height        = Size.y > 0.0f ? Size.y : 8.0f;
    const ImVec2       TextSize      = Overlay && Overlay[0] ? ImGui::CalcTextSize(Overlay) : ImVec2(0.0f, 0.0f);
    const float        TotalHeight   = TextSize.y > 0.0f ? Height + 3.0f + TextSize.y : Height;
    const float        Value         = std::clamp(Fraction, 0.0f, 1.0f);
    const float        AnimatedValue = SmoothValue(WidgetId, Value, 16.0f, Value);

    ImGui::Dummy(ImVec2(Width, TotalHeight));
    ImDrawList*  DrawList = ImGui::GetWindowDrawList();
    const ImVec2 End(Pos.x + Width, Pos.y + Height);
    DrawList->AddRectFilled(Pos, End, ToU32(C.Surface), Height * 0.5f);
    if (AnimatedValue > 0.0f)
        DrawList->AddRectFilled(Pos, ImVec2(Pos.x + Width * AnimatedValue, End.y), ToU32(C.Accent), Height * 0.5f);
    DrawList->AddRect(Pos, End, ToU32(C.Border), Height * 0.5f, 0, S.BorderWidth);

    if (Overlay && Overlay[0])
        DrawList->AddText(
            ImVec2(Pos.x + (Width - TextSize.x) * 0.5f, Pos.y + Height + 3.0f), ToU32(C.TextMuted), Overlay);
}

void DrawTextCenteredX(ImDrawList* DrawList, float CenterX, float Y, ImU32 Color, std::string_view Text)
{
    if (!DrawList || Text.empty())
        return;

    const ImVec2 Size = ImGui::CalcTextSize(Text.data(), Text.data() + Text.size());
    DrawList->AddText(ImVec2(CenterX - Size.x * 0.5f, Y), Color, Text.data(), Text.data() + Text.size());
}

void IndeterminateProgressBar(const char* Id, const ImVec2& Size)
{
    const ColorTokens& C       = Colors();
    const SizeTokens&  S       = Sizes();
    const ImVec2       Pos     = ImGui::GetCursorScreenPos();
    const float        Width   = Size.x > 0.0f ? Size.x : ImGui::GetContentRegionAvail().x;
    const float        Height  = Size.y > 0.0f ? Size.y : 8.0f;
    const float        Time    = (float)ImGui::GetTime();
    const float        Segment = Width * 0.30f;
    const float        Travel  = Width + Segment * 2.0f;
    const float        StartX  = Pos.x - Segment + std::fmod(Time * Travel * 0.65f, Travel);
    const float        EndX    = std::min(Pos.x + Width, StartX + Segment);

    ImGui::PushID(Id);
    ImGui::Dummy(ImVec2(Width, Height));
    ImGui::PopID();

    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    DrawList->AddRectFilled(Pos, ImVec2(Pos.x + Width, Pos.y + Height), ToU32(C.Surface), Height * 0.5f);
    if (EndX > Pos.x && StartX < Pos.x + Width)
        DrawList->AddRectFilled(
            ImVec2(std::max(Pos.x, StartX), Pos.y), ImVec2(EndX, Pos.y + Height), ToU32(C.Accent), Height * 0.5f);
}

void ProgressRing(const char* Id, float Fraction, float Radius, float Thickness)
{
    const ColorTokens& C               = Colors();
    const SizeTokens&  S               = Sizes();
    const float        ActualRadius    = Radius > 0.0f ? Radius : 12.0f;
    const float        ActualThickness = Thickness > 0.0f ? Thickness : 2.5f;
    const ImVec2       Size(ActualRadius * 2.0f + ActualThickness * 2.0f, ActualRadius * 2.0f + ActualThickness * 2.0f);
    const ImVec2       Pos = ImGui::GetCursorScreenPos();
    const ImVec2       Center(Pos.x + Size.x * 0.5f, Pos.y + Size.y * 0.5f);
    const float        Value = SmoothValue(ImGui::GetID(Id), std::clamp(Fraction, 0.0f, 1.0f), 16.0f, Fraction);

    ImGui::PushID(Id);
    ImGui::Dummy(Size);
    ImGui::PopID();

    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    DrawList->PathClear();
    DrawList->PathArcTo(Center, ActualRadius, -IM_PI * 0.5f, IM_PI * 1.5f, 48);
    DrawList->PathStroke(ToU32(C.Surface), 0, ActualThickness);
    if (Value > 0.0f)
    {
        DrawList->PathClear();
        DrawList->PathArcTo(Center, ActualRadius, -IM_PI * 0.5f, -IM_PI * 0.5f + IM_PI * 2.0f * Value, 48);
        DrawList->PathStroke(ToU32(C.Accent), 0, ActualThickness);
    }
}

void Spinner(const char* Id, float Radius, float Thickness)
{
    const ColorTokens& C               = Colors();
    const SizeTokens&  S               = Sizes();
    const float        ActualRadius    = Radius > 0.0f ? Radius : 7.0f;
    const float        ActualThickness = Thickness > 0.0f ? Thickness : 2.0f;
    const ImVec2       Size(ActualRadius * 2.0f + ActualThickness * 2.0f, ActualRadius * 2.0f + ActualThickness * 2.0f);
    const ImVec2       Pos = ImGui::GetCursorScreenPos();
    const ImVec2       Center(Pos.x + Size.x * 0.5f, Pos.y + Size.y * 0.5f);
    const float        Time  = (float)ImGui::GetTime();
    const float        Start = Time * 6.0f;
    const float        End   = Start + IM_PI * 1.45f;

    ImGui::PushID(Id);
    ImGui::Dummy(Size);
    ImGui::PopID();

    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    DrawList->PathClear();
    DrawList->PathArcTo(Center, ActualRadius, Start, End, 32);
    DrawList->PathStroke(ToU32(C.Accent), 0, ActualThickness);
}

bool BeginTable(const char* Id, int ColumnCount, ImGuiTableFlags Flags, const ImVec2& OuterSize)
{
    const ColorTokens& C          = Colors();
    const SizeTokens&  S          = Sizes();
    int                ColorCount = 0;
    int                VarCount   = 0;
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, KnownColor::Transparent);
    ColorCount++;
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, KnownColor::Transparent);
    ColorCount++;
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, KnownColor::Transparent);
    ColorCount++;
    ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, KnownColor::Transparent);
    ColorCount++;
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight, WithAlpha(C.Border, 0.24f));
    ColorCount++;
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(ImMax(10.0f, S.ItemGap), S.ItemGap * 0.55f));
    VarCount++;

    const ImVec2 Pos      = ImGui::GetCursorScreenPos();
    ImDrawList*  DrawList = ImGui::GetWindowDrawList();
    DrawList->ChannelsSplit(2);
    DrawList->ChannelsSetCurrent(1);
    ImGuiTableFlags TableFlags = (Flags & ~ImGuiTableFlags_BordersOuter) | ImGuiTableFlags_PadOuterX;
    if (!ImGui::BeginTable(Id, ColumnCount, TableFlags, OuterSize))
    {
        DrawList->ChannelsMerge();
        ImGui::PopStyleVar(VarCount);
        ImGui::PopStyleColor(ColorCount);
        return false;
    }

    TableFrame Frame = {};
    Frame.DrawList   = DrawList;
    Frame.Pos        = Pos;
    Frame.Rounding   = S.ControlRounding;
    Frame.ColorCount = ColorCount;
    Frame.VarCount   = VarCount;
    g_TableFrames.push_back(Frame);
    return true;
}

void TableHeadersRow(const char* const Headers[], int HeaderCount)
{
    if (!Headers || HeaderCount <= 0)
        return;

    const ColorTokens& C = Colors();
    const SizeTokens&  S = Sizes();
    ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFrameHeight());
    for (int i = 0; i < HeaderCount; i++)
    {
        ImGui::TableSetColumnIndex(i);
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(C.TextMuted, "%s", Headers[i] ? Headers[i] : "");
    }

    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    ImVec2      Min      = ImGui::GetItemRectMin();
    ImVec2      Max      = ImGui::GetItemRectMax();
    if (HeaderCount > 0)
    {
        Min.x = ImGui::GetWindowPos().x + ImGui::GetStyle().WindowPadding.x;
        Max.x = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        DrawList->AddLine(ImVec2(Min.x, Max.y + S.ItemGap * 0.25f),
                          ImVec2(Max.x, Max.y + S.ItemGap * 0.25f),
                          ToU32(WithAlpha(C.Border, 0.32f)),
                          1.0f);
    }
}

void EndTable()
{
    TableFrame Frame    = {};
    bool       HasFrame = false;
    if (!g_TableFrames.empty())
    {
        Frame = g_TableFrames.back();
        g_TableFrames.pop_back();
        HasFrame = true;
    }

    ImGui::EndTable();

    if (!HasFrame)
        return;

    Frame.Min = ImGui::GetItemRectMin();
    Frame.Max = ImGui::GetItemRectMax();
    if (Frame.Max.x <= Frame.Min.x || Frame.Max.y <= Frame.Min.y)
    {
        Frame.Min   = Frame.Pos;
        Frame.Max   = ImGui::GetCursorScreenPos();
        Frame.Max.x = Frame.Min.x + ImGui::GetContentRegionAvail().x;
    }

    ImDrawList* DrawList = Frame.DrawList ? Frame.DrawList : ImGui::GetWindowDrawList();
    DrawList->ChannelsSetCurrent(0);
    DrawList->AddRectFilled(Frame.Min, Frame.Max, ToU32(Colors().Surface), Frame.Rounding, ImDrawFlags_RoundCornersAll);
    DrawList->ChannelsMerge();
    ImGui::PopStyleVar(Frame.VarCount);
    ImGui::PopStyleColor(Frame.ColorCount);
}

void LabelValue(const char* Label, const char* Value)
{
    TextMuted(Label);
    ImGui::SameLine();
    Text(Value);
}

void LabelValue(const char* Label, int Value)
{
    char Buffer[32]{};
    std::snprintf(Buffer, sizeof(Buffer), "%d", Value);
    LabelValue(Label, Buffer);
}

void LabelValue(const char* Label, float Value, const char* Format)
{
    char Buffer[64]{};
    std::snprintf(Buffer, sizeof(Buffer), Format, Value);
    LabelValue(Label, Buffer);
}

bool BeginWindow(const char* Name, const WindowOptions& Options)
{
    ImGuiWindowFlags Flags                       = Options.Flags;
    const bool       DrawRightSideCollapseButton = Options.UseRightSideCollapseButton &&
                                             !(Flags & ImGuiWindowFlags_NoCollapse) &&
                                             !(Flags & ImGuiWindowFlags_NoTitleBar);

    const bool CountAsManagedWindow = Options.CountAsManagedWindow && (!Options.Open || *Options.Open);
    EnsureManagedWindowFrame();
    if (CountAsManagedWindow)
        g_VisibleManagedWindowCount++;

    ImGuiDir PreviousWindowMenuButtonPosition = ImGuiDir_None;
    if (DrawRightSideCollapseButton)
    {
        ImGuiStyle& Style                = ImGui::GetStyle();
        PreviousWindowMenuButtonPosition = Style.WindowMenuButtonPosition;
        Style.WindowMenuButtonPosition   = ImGuiDir_Right;
    }

    const bool Open = ImGui::Begin(Name, Options.Open, Flags);
    if (PreviousWindowMenuButtonPosition != ImGuiDir_None)
        ImGui::GetStyle().WindowMenuButtonPosition = PreviousWindowMenuButtonPosition;
    return Open;
}

void EndWindow()
{
    ImGui::End();
}

void BeginManagedWindowFrame()
{
    g_ManagedWindowFrame        = ImGui::GetFrameCount();
    g_VisibleManagedWindowCount = 0;
}

bool HasVisibleManagedWindows()
{
    return g_VisibleManagedWindowCount > 0;
}

void DrawTitleBarCollapseButton(const char* Id, const char* CollapseTooltip, const char* RestoreTooltip)
{
    ImGuiWindow* Window = ImGui::GetCurrentWindow();
    if (!Window || Window->Hidden)
        return;

    const ImGuiStyle& Style        = ImGui::GetStyle();
    const ImRect      TitleBarRect = Window->TitleBarRect();
    if (TitleBarRect.GetHeight() <= 0.0f || TitleBarRect.GetWidth() <= 0.0f)
        return;

    const float ButtonSize   = ImMax(1.0f, TitleBarRect.GetHeight() - Style.FramePadding.y * 2.0f);
    float       RightPadding = Style.FramePadding.x;
    if (Window->HasCloseButton)
        RightPadding += ButtonSize + Style.ItemInnerSpacing.x;

    const ImVec2 ButtonMin(TitleBarRect.Max.x - RightPadding - ButtonSize, TitleBarRect.Min.y + Style.FramePadding.y);
    if (ButtonMin.x <= TitleBarRect.Min.x)
        return;

    ImDrawList* DrawList = Window->DrawList;
    DrawList->PushClipRect(TitleBarRect.Min, TitleBarRect.Max, false);
    ImGui::PushID(Id);
    const ImGuiID ButtonId = Window->GetID("##TitleBarCollapse");
    const bool    Pressed  = ImGui::CollapseButton(ButtonId, ButtonMin);
    ImGui::PopID();
    DrawList->PopClipRect();

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        ImGui::SetTooltip("%s", Window->Collapsed ? RestoreTooltip : CollapseTooltip);
    if (Pressed)
        ImGui::SetWindowCollapsed(Window, !Window->Collapsed, ImGuiCond_Always);
}

bool BeginPanelChild(const char* Id, const PanelChildOptions& Options)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Border));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, Options.Border ? ImGui::GetStyle().FrameBorderSize : 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().FrameRounding);

    g_LastPanelChildContentVisible = ImGui::BeginChild(Id, Options.Size, Options.Border, Options.Flags);
    return true;
}

void EndPanelChild()
{
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

bool IsPanelChildContentVisible()
{
    return g_LastPanelChildContentVisible;
}

} // namespace RnTools::ImguiRorinnn
