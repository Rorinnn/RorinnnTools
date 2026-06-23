module;

#include <Windows.h>

export module RorinnnTools:Input;
import std;

export namespace RorinnnTools::Input
{

bool TapVirtualKey(WORD VirtualKey, DWORD PressMs = 20);
bool TapVirtualKeyChord(const WORD* PVirtualKeys, std::size_t Count, DWORD PressMs = 20);

} // namespace RorinnnTools::Input
