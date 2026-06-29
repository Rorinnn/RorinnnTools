module;

#include <Windows.h>

export module RnTools:ManualMap;

namespace RnTools
{

using LoadLibraryA_t   = HINSTANCE(WINAPI*)(const char* FileName);
using GetProcAddress_t = FARPROC(WINAPI*)(HMODULE Module, LPCSTR ProcName);
using DllEntryPoint_t  = BOOL(WINAPI*)(void* Dll, DWORD Reason, void* Reserved);

#ifdef _WIN64
using RtlAddFunctionTable_t = BOOL(WINAPIV*)(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD64 BaseAddress);
#endif

struct ManualMappingData
{
    LoadLibraryA_t   LoadLibraryA;
    GetProcAddress_t GetProcAddress;
#ifdef _WIN64
    RtlAddFunctionTable_t RtlAddFunctionTable;
#endif
    BYTE*     Base;
    HINSTANCE Module;
    DWORD     Reason;
    LPVOID    Reserved;
    BOOL      SEHSupport;
};

void __stdcall ManualMapShellcode(ManualMappingData* Data);

} // namespace RnTools

export namespace RnTools
{

bool ManualMapDll(HANDLE Process,
                  BYTE*  SourceData,
                  SIZE_T FileSize,
                  bool   ClearHeader            = true,
                  bool   ClearNonNeededSections = true,
                  bool   AdjustProtections      = true,
                  bool   SEHExceptionSupport    = true,
                  DWORD  Reason                 = DLL_PROCESS_ATTACH,
                  LPVOID Reserved               = nullptr);

} // namespace RnTools
