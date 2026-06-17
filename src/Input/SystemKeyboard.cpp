// SystemKeyboard.cpp — Windows 系统键盘输入封装

module;

#include <Windows.h>

module RorinnnTools;

namespace RorinnnTools::Input
{
static constexpr size_t MaxChordKeys = 8;

static bool SendVirtualKey(WORD VirtualKey, bool Down)
{
    if (!VirtualKey)
    {
        return false;
    }

    INPUT InputValue  = {};
    InputValue.type   = INPUT_KEYBOARD;
    InputValue.ki.wVk = VirtualKey;
    if (!Down)
    {
        InputValue.ki.dwFlags = KEYEVENTF_KEYUP;
    }

    return SendInput(1, &InputValue, sizeof(InputValue)) == 1;
}

bool TapVirtualKey(WORD VirtualKey, DWORD PressMs)
{
    return TapVirtualKeyChord(&VirtualKey, 1, PressMs);
}

bool TapVirtualKeyChord(const WORD* PVirtualKeys, size_t Count, DWORD PressMs)
{
    if (!PVirtualKeys || Count == 0 || Count > MaxChordKeys)
    {
        return false;
    }

    for (size_t i = 0; i < Count; i++)
    {
        if (!PVirtualKeys[i])
        {
            return false;
        }
    }

    for (size_t i = 0; i < Count; i++)
    {
        if (!SendVirtualKey(PVirtualKeys[i], true))
        {
            for (size_t j = i; j > 0; j--)
            {
                SendVirtualKey(PVirtualKeys[j - 1], false);
            }
            return false;
        }
    }

    if (PressMs > 0)
    {
        Sleep(PressMs);
    }

    bool Released = true;
    for (size_t i = Count; i > 0; i--)
    {
        Released = SendVirtualKey(PVirtualKeys[i - 1], false) && Released;
    }
    return Released;
}
} // namespace RorinnnTools::Input
