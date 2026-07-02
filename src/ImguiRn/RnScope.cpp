// RnScope.cpp — ImGui 样式作用域

module;

#include <imgui.h>

module RnTools;

namespace RnTools::ImguiRn
{

StyleColorScope::StyleColorScope(ImGuiCol Index, const ImVec4& Color)
{
    Push(Index, Color);
}

StyleColorScope::~StyleColorScope()
{
    Pop();
}

StyleColorScope::StyleColorScope(StyleColorScope&& Other) noexcept
{
    Count       = Other.Count;
    Other.Count = 0;
}

StyleColorScope& StyleColorScope::operator=(StyleColorScope&& Other) noexcept
{
    if (this == &Other)
        return *this;
    Pop();
    Count       = Other.Count;
    Other.Count = 0;
    return *this;
}

void StyleColorScope::Push(ImGuiCol Index, const ImVec4& Color)
{
    ImGui::PushStyleColor(Index, Color);
    Count++;
}

void StyleColorScope::Pop()
{
    if (Count <= 0)
        return;
    ImGui::PopStyleColor(Count);
    Count = 0;
}

StyleVarScope::StyleVarScope(ImGuiStyleVar Index, float Value)
{
    Push(Index, Value);
}

StyleVarScope::StyleVarScope(ImGuiStyleVar Index, const ImVec2& Value)
{
    Push(Index, Value);
}

StyleVarScope::~StyleVarScope()
{
    Pop();
}

StyleVarScope::StyleVarScope(StyleVarScope&& Other) noexcept
{
    Count       = Other.Count;
    Other.Count = 0;
}

StyleVarScope& StyleVarScope::operator=(StyleVarScope&& Other) noexcept
{
    if (this == &Other)
        return *this;
    Pop();
    Count       = Other.Count;
    Other.Count = 0;
    return *this;
}

void StyleVarScope::Push(ImGuiStyleVar Index, float Value)
{
    ImGui::PushStyleVar(Index, Value);
    Count++;
}

void StyleVarScope::Push(ImGuiStyleVar Index, const ImVec2& Value)
{
    ImGui::PushStyleVar(Index, Value);
    Count++;
}

void StyleVarScope::Pop()
{
    if (Count <= 0)
        return;
    ImGui::PopStyleVar(Count);
    Count = 0;
}

DisabledScope::DisabledScope(bool Disabled)
{
    if (!Disabled)
        return;
    Active = true;
    ImGui::BeginDisabled(true);
}

DisabledScope::~DisabledScope()
{
    if (!Active)
        return;
    ImGui::EndDisabled();
}

} // namespace RnTools::ImguiRn
