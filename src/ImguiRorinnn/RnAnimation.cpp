// RnAnimation.cpp — ImguiRorinnn 即时模式动画工具

module;

#include <algorithm>
#include <cmath>
#include <unordered_map>

#include <imgui.h>

module RorinnnTools;

namespace RorinnnTools::ImguiRorinnn
{
namespace
{
static std::unordered_map<ImGuiID, float> g_SmoothedValues;

static float Clamp01(float Value)
{
    return std::clamp(Value, 0.0f, 1.0f);
}
} // namespace

float EaseValue(Ease Mode, float T)
{
    T = Clamp01(T);
    switch (Mode)
    {
        case Ease::OutQuad:
            return 1.0f - (1.0f - T) * (1.0f - T);
        case Ease::OutCubic:
            return 1.0f - std::pow(1.0f - T, 3.0f);
        case Ease::InOutCubic:
            return T < 0.5f ? 4.0f * T * T * T : 1.0f - std::pow(-2.0f * T + 2.0f, 3.0f) * 0.5f;
        case Ease::OutBack:
        {
            constexpr float C1 = 1.70158f;
            constexpr float C3 = C1 + 1.0f;
            return 1.0f + C3 * std::pow(T - 1.0f, 3.0f) + C1 * std::pow(T - 1.0f, 2.0f);
        }
        case Ease::Linear:
        default:
            return T;
    }
}

float GetFrameDeltaSeconds()
{
    ImGuiContext* Context = ImGui::GetCurrentContext();
    if (!Context)
    {
        return 1.0f / 60.0f;
    }

    return std::clamp(ImGui::GetIO().DeltaTime, 0.0f, 0.10f);
}

float SmoothValue(ImGuiID Id, float Target, float Speed, float InitialValue)
{
    float&      Value        = g_SmoothedValues.try_emplace(Id, InitialValue).first->second;
    const float DeltaSeconds = GetFrameDeltaSeconds();
    if (DeltaSeconds <= 0.0f || Speed <= 0.0f)
    {
        Value = Target;
        return Value;
    }

    const float Amount  = 1.0f - std::exp(-Speed * DeltaSeconds);
    Value              += (Target - Value) * Amount;
    if (std::fabs(Value - Target) <= 0.001f)
    {
        Value = Target;
    }
    return Value;
}

float SmoothValue(const char* Id, float Target, float Speed, float InitialValue)
{
    return SmoothValue(ImGui::GetID(Id), Target, Speed, InitialValue);
}

float AnimateBool(ImGuiID Id, bool Active, float Speed)
{
    return SmoothValue(Id, Active ? 1.0f : 0.0f, Speed, Active ? 1.0f : 0.0f);
}

float AnimateBool(const char* Id, bool Active, float Speed)
{
    return AnimateBool(ImGui::GetID(Id), Active, Speed);
}

void ClearAnimationState(ImGuiID Id)
{
    g_SmoothedValues.erase(Id);
}

void ClearAllAnimationStates()
{
    g_SmoothedValues.clear();
}

} // namespace RorinnnTools::ImguiRorinnn
