// ProcessTools.cpp — 进程查询与权限工具

module;

#include <Windows.h>
#include <TlHelp32.h>

module RorinnnTools;

namespace RorinnnTools
{

DWORD GetProcessIdByName(const wchar_t* Name)
{
    if (!Name || Name[0] == L'\0') return 0;

    PROCESSENTRY32W Entry{};
    Entry.dwSize = sizeof(Entry);

    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Snapshot == INVALID_HANDLE_VALUE) return 0;

    DWORD Result = 0;
    if (Process32FirstW(Snapshot, &Entry))
    {
        do
        {
            if (_wcsicmp(Entry.szExeFile, Name) == 0)
            {
                Result = Entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(Snapshot, &Entry));
    }

    CloseHandle(Snapshot);
    return Result;
}

bool IsCorrectTargetArchitecture(HANDLE Process)
{
    BOOL TargetWow64 = FALSE;
    if (!IsWow64Process(Process, &TargetWow64)) return false;

    BOOL HostWow64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &HostWow64);
    return TargetWow64 == HostWow64;
}

bool EnableDebugPrivilege()
{
    TOKEN_PRIVILEGES Privileges{};
    HANDLE           Token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &Token))
    {
        return false;
    }

    Privileges.PrivilegeCount           = 1;
    Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    const bool Ok = LookupPrivilegeValueW(nullptr, L"SeDebugPrivilege", &Privileges.Privileges[0].Luid) &&
                    AdjustTokenPrivileges(Token, FALSE, &Privileges, 0, nullptr, nullptr);
    CloseHandle(Token);
    return Ok;
}

} // namespace RorinnnTools
