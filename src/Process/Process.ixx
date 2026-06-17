module;

#include <Windows.h>

export module RorinnnTools:Process;

export namespace RorinnnTools
{

DWORD GetProcessIdByName(const wchar_t* Name);
bool  IsCorrectTargetArchitecture(HANDLE Process);
bool  EnableDebugPrivilege();

} // namespace RorinnnTools
