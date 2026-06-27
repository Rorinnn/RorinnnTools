// RnWin32InputRouting.cpp — Win32 文本输入消息路由

module;

#include <Windows.h>
#include <imgui_internal.h>

module RorinnnTools;

namespace RorinnnTools::ImguiRorinnn
{
namespace
{
static bool IsKeyboardMessage(UINT Msg)
{
    switch (Msg)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            return true;
        default:
            return false;
    }
}

static bool IsTextInputMessage(UINT Msg)
{
    switch (Msg)
    {
        case WM_CHAR:
        case WM_SYSCHAR:
        case WM_DEADCHAR:
        case WM_SYSDEADCHAR:
        case WM_UNICHAR:
        case WM_IME_CHAR:
        case WM_IME_KEYDOWN:
        case WM_IME_KEYUP:
        case WM_IME_STARTCOMPOSITION:
        case WM_IME_COMPOSITION:
        case WM_IME_ENDCOMPOSITION:
        case WM_IME_NOTIFY:
        case WM_IME_SETCONTEXT:
        case WM_INPUTLANGCHANGE:
            return true;
        default:
            return false;
    }
}

static bool HasActiveImguiTextInput()
{
    if (!GImGui)
        return false;

    ImGuiContext& Context = *GImGui;
    return Context.ActiveId != 0 && Context.InputTextState.ID == Context.ActiveId;
}

static bool GameHasNativeTextCaret(HWND Hwnd)
{
    if (!Hwnd)
        return false;

    GUITHREADINFO GuiThreadInfo = {};
    GuiThreadInfo.cbSize        = sizeof(GuiThreadInfo);
    const DWORD ThreadId        = GetWindowThreadProcessId(Hwnd, nullptr);
    if (!ThreadId || !GetGUIThreadInfo(ThreadId, &GuiThreadInfo))
        return false;

    return GuiThreadInfo.hwndCaret != nullptr;
}
} // namespace

bool ShouldPreferGameTextInput(HWND Hwnd, UINT Msg, WPARAM WParam)
{
    if (HasActiveImguiTextInput())
        return false;
    if (IsTextInputMessage(Msg))
        return true;
    if (WParam == VK_PROCESSKEY && IsKeyboardMessage(Msg))
        return true;
    if (GameHasNativeTextCaret(Hwnd) && IsKeyboardMessage(Msg))
        return true;
    return false;
}

} // namespace RorinnnTools::ImguiRorinnn
