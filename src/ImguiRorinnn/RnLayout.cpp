// RnLayout.cpp — ImguiRorinnn 布局组件

module;

#include <imgui_internal.h>

module RorinnnTools;
import std;

namespace RorinnnTools::ImguiRorinnn
{
namespace
{
struct ModuleFrame
{
    ImGuiStorage* Storage;
    ImGuiID       BodyHeightId;
    ImGuiWindow*  ScrollParent;
    const char*   Description;
    bool          ShowBottomDescription;
    bool          Enabled;
    bool          Active;
    float         OpenT;
    float         DescriptionHeight;
    float         DescriptionGap;
    ImVec2        DescriptionStart;
    float         DescriptionWidth;
};

static std::vector<ModuleFrame> g_ModuleFrames;
static bool                     g_LastPanelContentVisible = false;

static ImGuiID ChildId(ImGuiID Id, const char* Name)
{
    ImGui::PushID(Id);
    ImGuiID Child = ImGui::GetID(Name);
    ImGui::PopID();
    return Child;
}

static float Saturate(float Value)
{
    if (Value < 0.0f) return 0.0f;
    if (Value > 1.0f) return 1.0f;
    return Value;
}

static float ModuleDescriptionBottomPadding()
{
    const SizeTokens& S = Sizes();
    return ImMax(S.PanelPadding.y + 2.0f, 10.0f);
}

static float ModuleBottomDescriptionHeight(float DescriptionHeight, float OpenT)
{
    if (OpenT <= 0.001f)
    {
        return 0.0f;
    }

    const SizeTokens& S = Sizes();
    return (S.ItemGap + DescriptionHeight + ModuleDescriptionBottomPadding()) * Saturate(OpenT);
}

static ImFont* ModuleIconFont()
{
    ImFont* Font = Fonts().IconFixedWidth;
    return Font ? Font : ImGui::GetFont();
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

static void DrawCenteredIcon(
    ImDrawList* DrawList, ImFont* Font, Icon IconValue, const ImVec2& Center, float FontSize, ImU32 Color)
{
    const char* Text = ToIconString(IconValue);
    if (!Text[0])
    {
        return;
    }

    const ImVec2 TextSize = Font->CalcTextSizeA(FontSize, FLT_MAX, 0.0f, Text);
    DrawList->AddText(Font, FontSize, ImVec2(Center.x - TextSize.x * 0.5f, Center.y - TextSize.y * 0.5f), Color, Text);
}

static void DrawModuleArrow(
    ImDrawList* DrawList, const ImVec2& Center, float FontSize, float OpenT, const ImVec4& Color)
{
    ImFont*     Font   = ModuleIconFont();
    const float DownT  = Saturate(OpenT);
    const float RightT = 1.0f - DownT;

    if (RightT > 0.01f)
    {
        DrawCenteredIcon(DrawList, Font, Icon::CaretRight, Center, FontSize, ToU32(WithMultipliedAlpha(Color, RightT)));
    }
    if (DownT > 0.01f)
    {
        DrawCenteredIcon(DrawList, Font, Icon::CaretDown, Center, FontSize, ToU32(WithMultipliedAlpha(Color, DownT)));
    }
}

static bool DrawModuleCheckbox(const ImRect& Bounds, ImGuiID Id, bool* Value, bool Enabled)
{
    if (!Value)
    {
        return false;
    }

    bool Hovered = false;
    bool Pressed = false;
    if (Enabled)
    {
        Hovered = IsRectInteractive(Bounds);
        Pressed = Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        if (Pressed)
        {
            *Value = !*Value;
        }
    }

    const ColorTokens& C          = Colors();
    ImDrawList*        DrawList   = ImGui::GetWindowDrawList();
    const float        CheckT     = SmoothValue(ChildId(Id, "Check"), *Value ? 1.0f : 0.0f, 18.0f, *Value ? 1.0f : 0.0f);
    ImVec4             FrameColor = Hovered && Enabled ? C.SurfaceHover : C.Surface;
    if (*Value)
    {
        FrameColor = Blend(FrameColor, C.Accent, 0.78f);
    }
    if (!Enabled)
    {
        FrameColor = WithMultipliedAlpha(FrameColor, 0.52f);
    }

    const float Radius = 4.0f;
    DrawList->AddRectFilled(Bounds.Min, Bounds.Max, ToU32(FrameColor), Radius);
    DrawList->AddRect(Bounds.Min, Bounds.Max, ToU32(Enabled ? C.BorderStrong : C.Border), Radius, 0, 1.0f);

    if (CheckT > 0.01f)
    {
        const ImVec2 A(Bounds.Min.x + Bounds.GetWidth() * 0.24f, Bounds.Min.y + Bounds.GetHeight() * 0.54f);
        const ImVec2 B(Bounds.Min.x + Bounds.GetWidth() * 0.42f, Bounds.Min.y + Bounds.GetHeight() * 0.70f);
        const ImVec2 D(Bounds.Min.x + Bounds.GetWidth() * 0.78f, Bounds.Min.y + Bounds.GetHeight() * 0.30f);
        const ImU32  MarkColor = ToU32(WithMultipliedAlpha(C.AccentText, CheckT));
        DrawList->AddLine(A, B, MarkColor, 2.35f);
        DrawList->AddLine(B, D, MarkColor, 2.35f);
    }

    return Pressed;
}

static float MeasuredChildContentHeight()
{
    ImGuiWindow* Window = ImGui::GetCurrentWindow();
    if (!Window)
    {
        return 0.0f;
    }

    return ImMax(0.0f, Window->DC.CursorMaxPos.y - Window->DC.CursorStartPos.y);
}

static void DrawModuleBottomDescription(const ModuleFrame& Frame)
{
    if (!Frame.ShowBottomDescription || !Frame.Description || Frame.OpenT <= 0.001f)
    {
        return;
    }

    const SizeTokens&  S         = Sizes();
    const ColorTokens& C         = Colors();
    const ImVec4       TextColor = Frame.Enabled && Frame.Active ? C.TextMuted : C.TextDisabled;
    const float        Height    = ModuleBottomDescriptionHeight(Frame.DescriptionHeight, Frame.OpenT);
    const ImVec2       Start     = Frame.DescriptionStart;
    const float        TextWidth = ImMax(1.0f, Frame.DescriptionWidth - S.PanelPadding.x * 2.0f);

    ImDrawList*  DrawList = ImGui::GetWindowDrawList();
    const ImVec2 TextPos(Start.x + S.PanelPadding.x, Start.y + Frame.DescriptionGap * Frame.OpenT);
    const ImVec2 ClipMin = Start;
    const ImVec2 ClipMax(Start.x + Frame.DescriptionWidth, Start.y + Height);
    DrawList->PushClipRect(ClipMin, ClipMax, true);
    DrawList->AddText(ImGui::GetFont(),
                      ImGui::GetFontSize(),
                      TextPos,
                      ToU32(WithMultipliedAlpha(TextColor, Frame.OpenT)),
                      Frame.Description,
                      nullptr,
                      TextWidth);
    DrawList->PopClipRect();
}
} // namespace

void AddVerticalSpace(float Height)
{
    ImGui::Dummy(ImVec2(1.0f, Height));
}

void SameLineRight(float ItemWidth)
{
    const float CursorX     = ImGui::GetCursorPosX();
    const float RegionRight = ImGui::GetContentRegionAvail().x;
    const float TargetX     = CursorX + RegionRight - ItemWidth;
    if (TargetX > CursorX)
    {
        ImGui::SameLine();
        ImGui::SetCursorPosX(TargetX);
    }
}

ImVec2 CalcButtonSize(const char* Label, float MinWidth)
{
    const SizeTokens& S        = Sizes();
    const ImVec2      TextSize = ImGui::CalcTextSize(Label, nullptr, true);
    return ImVec2(ImMax(MinWidth, TextSize.x + S.FramePadding.x * 2.0f), S.ControlHeight);
}

ImVec2 CalcWindowContentSize(float PreferredWidth, float PreferredHeight)
{
    const ImGuiStyle& Style  = ImGui::GetStyle();
    const float       Width  = PreferredWidth + Style.WindowPadding.x * 2.0f;
    const float       Height = PreferredHeight + Style.WindowPadding.y * 2.0f;
    return ImVec2(Width, Height);
}

float GetVerticalSplitterHitWidth()
{
    return VerticalSplitterOptions{}.HitWidth;
}

bool DrawVerticalSplitter(const char* Id, float Height)
{
    VerticalSplitterOptions Options = {};
    Options.Height                  = Height;
    return DrawVerticalSplitter(Id, Options);
}

bool DrawVerticalSplitter(const char* Id, const VerticalSplitterOptions& Options)
{
    ImGui::InvisibleButton(Id, ImVec2(Options.HitWidth, Options.Height));
    const bool Hovered = ImGui::IsItemHovered();
    const bool Active  = ImGui::IsItemActive();
    if (Hovered || Active)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

    const ImVec2 HitMin    = ImGui::GetItemRectMin();
    const ImVec2 HitMax    = ImGui::GetItemRectMax();
    const float  CenterX   = (HitMin.x + HitMax.x) * 0.5f;
    const float  HalfLineW = Options.LineWidth * 0.5f;
    const ImVec2 LineMin(CenterX - HalfLineW, HitMin.y);
    const ImVec2 LineMax(CenterX + HalfLineW, HitMax.y);
    const ImU32  Color = ImGui::GetColorU32(Hovered || Active ? ImGuiCol_ButtonHovered : ImGuiCol_Border);
    ImGui::GetWindowDrawList()->AddRectFilled(LineMin, LineMax, Color);
    return Active && ImGui::IsMouseDragging(ImGuiMouseButton_Left);
}

bool BeginPanel(const char* Id, const PanelOptions& Options)
{
    const SizeTokens& S          = Sizes();
    ImGuiChildFlags   ChildFlags = 0;
    if (Options.Border)
    {
        ChildFlags |= ImGuiChildFlags_Borders;
    }
    if (Options.FitHeight)
    {
        ChildFlags |= ImGuiChildFlags_AutoResizeY;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, S.PanelRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, S.PanelPadding);
    g_LastPanelContentVisible = ImGui::BeginChild(Id, Options.Size, ChildFlags);
    return true;
}

void EndPanel()
{
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

bool IsPanelContentVisible()
{
    return g_LastPanelContentVisible;
}

bool BeginModule(const char* Id, const char* Name, const ModuleHeaderOptions& Options)
{
    const ColorTokens& C      = Colors();
    const SizeTokens&  S      = Sizes();
    ImGuiWindow*       Window = ImGui::GetCurrentWindow();
    if (Window->SkipItems) return false;

    ImGui::PushID(Id);

    const ImVec2  Start          = ImGui::GetCursorScreenPos();
    const float   Width          = ImGui::GetContentRegionAvail().x;
    const float   NameHeight     = ImGui::GetTextLineHeight();
    const bool    HasDescription = Options.Description && Options.Description[0];
    const ImGuiID StorageId      = ImGui::GetID("##Open");
    const ImGuiID BodyHeightId   = ImGui::GetID("##BodyHeight");
    ImGuiStorage* Storage        = ImGui::GetStateStorage();
    const bool    WasInitialized = Storage->GetInt(ImGui::GetID("##Initialized"), 0) != 0;
    if (!WasInitialized)
    {
        Storage->SetInt(ImGui::GetID("##Initialized"), 1);
        Storage->SetInt(StorageId, Options.DefaultOpen ? 1 : 0);
    }

    bool        Open = Storage->GetInt(StorageId, 0) != 0;
    const float OpenT =
        SmoothValue(ChildId(StorageId, "OpenT"), Open ? 1.0f : 0.0f, Options.AnimationSpeed, Open ? 1.0f : 0.0f);
    const float TopDescriptionT =
        HasDescription && Options.DescriptionAtBottomWhenOpen ? 1.0f - Saturate(OpenT) : (HasDescription ? 1.0f : 0.0f);
    const bool   TopDescription  = TopDescriptionT > 0.001f;
    const float  HeaderRowHeight = ImMax(NameHeight, 22.0f);
    const float  RowY            = Start.y + S.PanelPadding.y;
    const float  RowCenterY      = RowY + HeaderRowHeight * 0.5f;
    const float  ArrowSize       = 20.0f;
    const float  ArrowX          = Start.x + S.PanelPadding.x + ArrowSize * 0.5f;
    const float  CheckboxSize    = 20.0f;
    const float  CheckboxX       = Start.x + S.PanelPadding.x + ArrowSize + 8.0f;
    const ImRect CheckboxBounds(ImVec2(CheckboxX, RowCenterY - CheckboxSize * 0.5f),
                                ImVec2(CheckboxX + CheckboxSize, RowCenterY + CheckboxSize * 0.5f));
    const float  TextX =
        Options.Checked ? CheckboxBounds.Max.x + S.ItemGap : Start.x + S.PanelPadding.x + ArrowSize + 8.0f;
    const float DescriptionRight           = Start.x + Width - S.PanelPadding.x;
    const float TopDescriptionWidth        = ImMax(1.0f, DescriptionRight - TextX);
    const float BottomDescriptionTextWidth = ImMax(1.0f, Width - S.PanelPadding.x * 2.0f);
    const float TopDescriptionHeight =
        HasDescription ? ImGui::CalcTextSize(Options.Description, nullptr, true, TopDescriptionWidth).y : 0.0f;
    const float BottomDescriptionTextHeight =
        HasDescription ? ImGui::CalcTextSize(Options.Description, nullptr, true, BottomDescriptionTextWidth).y : 0.0f;
    const float TopDescriptionGap = 4.0f * TopDescriptionT;
    const float HeaderHeight      = S.PanelPadding.y * 2.0f + HeaderRowHeight +
                               (TopDescription ? TopDescriptionGap + TopDescriptionHeight * TopDescriptionT : 0.0f);
    const ImVec2 HeaderSize(Width, HeaderHeight);

    const ImRect HeaderBounds(Start, ImVec2(Start.x + HeaderSize.x, Start.y + HeaderSize.y));
    const bool   HeaderHovered = Options.Enabled && IsRectInteractive(HeaderBounds);
    const bool   HeaderClicked = HeaderHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

    const float BodySpacing        = S.ItemGap * Saturate(OpenT);
    const float LastBodyHeight     = Storage->GetFloat(BodyHeightId, 0.0f);
    float       AnimatedBodyHeight = LastBodyHeight * Saturate(OpenT);
    if (OpenT > 0.001f && AnimatedBodyHeight < 1.0f)
    {
        AnimatedBodyHeight = 1.0f;
    }

    const bool  BottomDescription = HasDescription && Options.DescriptionAtBottomWhenOpen && OpenT > 0.001f;
    const float BottomDescriptionHeight =
        BottomDescription ? ModuleBottomDescriptionHeight(BottomDescriptionTextHeight, OpenT) : 0.0f;
    const float  FrameHeight     = HeaderHeight + BodySpacing + AnimatedBodyHeight + BottomDescriptionHeight;
    const float  SafeFrameHeight = ImMax(FrameHeight, HeaderHeight);
    const ImVec4 Fill            = HeaderHovered ? C.SurfaceHover : C.Surface;
    const bool   ModuleActive    = !Options.Checked || *Options.Checked;

    ImGui::SetCursorScreenPos(Start);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, S.PanelRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Fill);
    ImGui::PushStyleColor(ImGuiCol_Border, C.Border);
    ImGui::BeginChild("##ModuleFrame",
                      ImVec2(Width, SafeFrameHeight),
                      ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    ImGui::SetCursorScreenPos(Start);
    const bool CheckboxPressed =
        DrawModuleCheckbox(CheckboxBounds, ImGui::GetID("##ModuleCheckbox"), Options.Checked, Options.Enabled);
    if (HeaderClicked && !CheckboxPressed)
    {
        Open = !Open;
        Storage->SetInt(StorageId, Open ? 1 : 0);
    }

    DrawModuleArrow(
        DrawList, ImVec2(ArrowX, RowCenterY), ArrowSize, OpenT, Options.Enabled ? C.TextMuted : C.TextDisabled);

    const float NameY = Start.y + S.PanelPadding.y;
    DrawList->AddText(ImVec2(TextX, NameY), ToU32(Options.Enabled && ModuleActive ? C.Text : C.TextDisabled), Name);

    if (TopDescription)
    {
        const ImVec2 DescriptionPos(TextX, NameY + NameHeight + 3.0f);
        const ImVec2 DescriptionClipMax(Start.x + Width, Start.y + HeaderHeight);
        DrawList->PushClipRect(Start, DescriptionClipMax, true);
        DrawList->AddText(
            ImGui::GetFont(),
            ImGui::GetFontSize(),
            DescriptionPos,
            ToU32(WithMultipliedAlpha(Options.Enabled && ModuleActive ? C.TextMuted : C.TextDisabled, TopDescriptionT)),
            Options.Description,
            nullptr,
            TopDescriptionWidth);
        DrawList->PopClipRect();
    }

    if (OpenT <= 0.001f || !Options.Enabled)
    {
        ImGui::EndChild();
        ImGui::PopID();
        return false;
    }

    ImGui::SetCursorScreenPos(ImVec2(Start.x, Start.y + HeaderHeight + BodySpacing));
    ImGui::Indent(15.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * Saturate(OpenT));
    ImGuiWindow* ScrollParent = ImGui::GetCurrentWindow()->ParentWindow;
    ImGui::BeginChild("##ModuleBody",
                      ImVec2(0.0f, AnimatedBodyHeight),
                      ImGuiChildFlags_None,
                      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
                          ImGuiWindowFlags_NoScrollWithMouse);
    g_ModuleFrames.push_back({Storage,
                              BodyHeightId,
                              ScrollParent,
                              Options.Description,
                              BottomDescription,
                              Options.Enabled,
                              ModuleActive,
                              Saturate(OpenT),
                              BottomDescriptionTextHeight,
                              S.ItemGap,
                              ImVec2(Start.x, Start.y + HeaderHeight + BodySpacing + AnimatedBodyHeight),
                              Width});
    return true;
}

void EndModule()
{
    ModuleFrame Frame = {};
    if (!g_ModuleFrames.empty())
    {
        Frame = g_ModuleFrames.back();
        g_ModuleFrames.pop_back();
    }

    const float BodyHeight  = MeasuredChildContentHeight();
    const bool  BodyHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    const float MouseWheel  = ImGui::GetIO().MouseWheel;
    ImGui::EndChild();
    if (Frame.Storage)
    {
        Frame.Storage->SetFloat(Frame.BodyHeightId, BodyHeight);
    }
    if (BodyHovered && MouseWheel != 0.0f && Frame.ScrollParent)
    {
        const float ScrollStep = ImGui::GetTextLineHeightWithSpacing() * 3.0f;
        ImGui::SetScrollY(Frame.ScrollParent, Frame.ScrollParent->Scroll.y - MouseWheel * ScrollStep);
    }
    ImGui::PopStyleVar();
    ImGui::Unindent(15.0f);
    DrawModuleBottomDescription(Frame);
    ImGui::EndChild();
    ImGui::PopID();
}

void BeginIndented(float Width)
{
    ImGui::Indent(Width);
}

void EndIndented(float Width)
{
    ImGui::Unindent(Width);
}

} // namespace RorinnnTools::ImguiRorinnn
