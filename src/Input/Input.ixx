module;

#include <Windows.h>

export module RnTools:Input;
import std;

export namespace RnTools::Input
{

bool TapVirtualKey(WORD VirtualKey, DWORD PressMs = 20);
bool TapVirtualKeyChord(const WORD* PVirtualKeys, std::size_t Count, DWORD PressMs = 20);

} // namespace RnTools::Input
