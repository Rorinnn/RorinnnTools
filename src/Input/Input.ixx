module;

#include <Windows.h>

#include <cstddef>

export module RorinnnTools:Input;

export namespace RorinnnTools::Input
{

bool TapVirtualKey(WORD VirtualKey, DWORD PressMs = 20);
bool TapVirtualKeyChord(const WORD* PVirtualKeys, size_t Count, DWORD PressMs = 20);

} // namespace RorinnnTools::Input
