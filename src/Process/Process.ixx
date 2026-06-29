module;

#include <Windows.h>

export module RnTools:Process;

export namespace RnTools
{

DWORD GetProcessIdByName(const wchar_t* Name);
bool  IsCorrectTargetArchitecture(HANDLE Process);
bool  EnableDebugPrivilege();

} // namespace RnTools
