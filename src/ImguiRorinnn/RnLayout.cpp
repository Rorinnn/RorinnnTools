// RnLayout.cpp — ImguiRorinnn 布局组件

module;

#include <imgui_internal.h>

module RnTools;
import std;

namespace RnTools::ImguiRorinnn
{
namespace
{
static bool g_LastPanelContentVisible = false;
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
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

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
        ChildFlags |= ImGuiChildFlags_Borders;
    if (Options.FitHeight)
        ChildFlags |= ImGuiChildFlags_AutoResizeY;

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

void BeginIndented(float Width)
{
    ImGui::Indent(Width);
}

void EndIndented(float Width)
{
    ImGui::Unindent(Width);
}

} // namespace RnTools::ImguiRorinnn
