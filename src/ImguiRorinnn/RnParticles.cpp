// RnParticles.cpp — ImguiRorinnn 粒子效果

module;

#include <imgui.h>

module RnTools;
import std;

namespace RnTools::ImguiRorinnn
{
namespace
{
static std::uint32_t Hash(std::uint32_t Value)
{
    Value ^= Value >> 16;
    Value *= 0x7FEB352Du;
    Value ^= Value >> 15;
    Value *= 0x846CA68Bu;
    Value ^= Value >> 16;
    return Value;
}

static float UnitFloat(std::uint32_t Value)
{
    return static_cast<float>(Hash(Value) & 0x00FFFFFFu) / 16777215.0f;
}

static float Lerp(float A, float B, float T)
{
    return A + (B - A) * T;
}

static float Wrap(float Value, float Size)
{
    if (Size <= 0.0f)
        return 0.0f;

    float Result = std::fmod(Value, Size);
    if (Result < 0.0f)
        Result += Size;
    return Result;
}

static void DrawSnowCrystal(
    ImDrawList* DrawList, const ImVec2& Center, float Radius, float Rotation, ImU32 Color, float Thickness)
{
    DrawList->AddCircleFilled(Center, std::max(0.7f, Radius * 0.16f), Color, 8);
    for (int Branch = 0; Branch < 6; Branch++)
    {
        const float  Angle = Rotation + static_cast<float>(Branch) * 1.0471976f;
        const float  CosA  = std::cos(Angle);
        const float  SinA  = std::sin(Angle);
        const ImVec2 End(Center.x + CosA * Radius, Center.y + SinA * Radius);
        DrawList->AddLine(Center, End, Color, Thickness);

        if (Radius < 2.4f)
            continue;

        const float     ForkDistance = Radius * 0.58f;
        const float     ForkLength   = Radius * 0.28f;
        const ImVec2    ForkBase(Center.x + CosA * ForkDistance, Center.y + SinA * ForkDistance);
        constexpr float ForkSigns[] = {-1.0f, 1.0f};
        for (float ForkSign : ForkSigns)
        {
            const float  ForkAngle = Angle + ForkSign * 0.72f;
            const ImVec2 ForkEnd(ForkBase.x + std::cos(ForkAngle) * ForkLength,
                                 ForkBase.y + std::sin(ForkAngle) * ForkLength);
            DrawList->AddLine(ForkBase, ForkEnd, Color, std::max(1.0f, Thickness * 0.78f));
        }
    }
}
} // namespace

void DrawSnowflakes(const char* Id, const ImVec2& Min, const ImVec2& Max, const SnowflakeOptions& Options)
{
    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    if (!DrawList || Max.x <= Min.x || Max.y <= Min.y || Options.Count <= 0)
        return;

    const ImGuiID BaseId    = ImGui::GetID(Id ? Id : "Snowflakes");
    const float   Width     = Max.x - Min.x;
    const float   Height    = Max.y - Min.y;
    const float   Time      = static_cast<float>(ImGui::GetTime());
    const float   MinRadius = std::max(0.2f, Options.MinRadius);
    const float   MaxRadius = std::max(MinRadius, Options.MaxRadius);
    const float   MinSpeed  = std::max(0.1f, Options.MinSpeed);
    const float   MaxSpeed  = std::max(MinSpeed, Options.MaxSpeed);

    DrawList->PushClipRect(Min, Max, true);
    for (int Index = 0; Index < Options.Count; Index++)
    {
        const std::uint32_t Seed         = Hash(static_cast<std::uint32_t>(BaseId) ^ static_cast<std::uint32_t>(Index * 977u + 17u));
        const float         Radius       = Lerp(MinRadius, MaxRadius, UnitFloat(Seed + 1u));
        const float         Speed        = Lerp(MinSpeed, MaxSpeed, UnitFloat(Seed + 2u));
        const float         Phase        = UnitFloat(Seed + 3u) * 6.2831853f;
        const float         Drift        = Lerp(Options.Drift * 0.35f, Options.Drift, UnitFloat(Seed + 4u));
        const float         Alpha        = Options.Color.w * Lerp(0.42f, 1.0f, UnitFloat(Seed + 5u));
        const float         TravelHeight = Height + MaxRadius * 8.0f;
        const float         BaseX        = Min.x + UnitFloat(Seed + 6u) * Width;
        const float         WrappedY     = Wrap(UnitFloat(Seed + 7u) * TravelHeight + Time * Speed, TravelHeight);
        const float         X            = BaseX + std::sin(Time * 0.75f + Phase) * Drift +
                        Time * Options.Wind * Lerp(0.10f, 0.55f, UnitFloat(Seed + 8u));
        const float Y        = Min.y - MaxRadius * 4.0f + WrappedY;
        const float WrappedX = Min.x + Wrap(X - Min.x, Width);

        ImVec4 Color          = Options.Color;
        Color.w               = std::clamp(Alpha, 0.0f, 1.0f);
        const ImU32 MainColor = ToU32(Color);
        const float Rotation  = Phase + Time * Lerp(0.15f, 0.65f, UnitFloat(Seed + 9u));
        DrawSnowCrystal(
            DrawList, ImVec2(WrappedX, Y), Radius * 1.9f, Rotation, MainColor, std::max(1.0f, Radius * 0.34f));

        if (Radius > 1.8f)
        {
            ImVec4 Glow  = Color;
            Glow.w      *= 0.14f;
            DrawList->AddCircleFilled(ImVec2(WrappedX, Y), Radius * 2.60f, ToU32(Glow), 16);
        }
    }
    DrawList->PopClipRect();
}

} // namespace RnTools::ImguiRorinnn
