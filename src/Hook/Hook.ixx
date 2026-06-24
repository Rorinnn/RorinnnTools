module;

#include <Windows.h>

export module RorinnnTools:Hook;
import std;
import :Memory;

export namespace RorinnnTools::Hook
{

enum class VehHookType
{
    Int3,
    Int3Trace,
    Int3Jump,
    HardwareBreakpoint,
    HardwareTrace,
    HardwareJump,
};

enum class VehHookStatus
{
    Ok,
    AlreadyInstalled,
    NotInstalled,
    HandlerInstallFailed,
    InvalidArgument,
    DuplicateToken,
    DuplicateTarget,
    TokenNotFound,
    TypeInvalid,
    ReadFailed,
    WriteFailed,
    AllocateFailed,
    ThreadSnapshotFailed,
    ThreadOpenFailed,
    ThreadContextFailed,
    HardwareBreakpointLimit,
};

using VehHookCallback = std::function<void(VehHookType Type, PEXCEPTION_POINTERS PExceptionInfo)>;

struct VehHookOptions
{
    int                 Token           = 0;
    void*               TargetAddress   = nullptr;
    void*               RedirectAddress = nullptr;
    VehHookType         Type            = VehHookType::Int3;
    VehHookCallback     Callback        = {};
    const std::uint8_t* TrampolineBytes = nullptr;
    std::size_t         TrampolineSize  = 0;
};

VehHookStatus InstallVehHookHandler();
VehHookStatus UninstallVehHookHandler();
bool          IsVehHookHandlerInstalled();

VehHookStatus AddVehHook(const VehHookOptions& Options);
VehHookStatus RemoveVehHook(int Token);
VehHookStatus RemoveAllVehHooks();
VehHookStatus RefreshHardwareVehHooks();
std::size_t   GetVehHookCount();

const char* GetVehHookStatusText(VehHookStatus Status);

struct HookResult
{
    VehHookStatus Status  = VehHookStatus::InvalidArgument;
    void*         Address = nullptr;

    bool Succeeded() const { return Status == VehHookStatus::Ok; }
};

HookResult HookFromAddress(int Token, void* TargetAddress, void* RedirectAddress);
HookResult HookFromSignature(int Token, RorinnnTools::Memory::SigScanner& Scanner, std::string_view Signature, void* RedirectAddress);
HookResult HookFromSymbol(int Token, const wchar_t* ModuleName, const char* SymbolName, void* RedirectAddress);
HookResult HookFromImport(const wchar_t* ImportingModuleName, const char* ImportedModuleName, const char* SymbolName, void* RedirectAddress);
HookResult HookFromImportAddress(int Token, const wchar_t* ImportingModuleName, const char* ImportedModuleName, const char* SymbolName, void* RedirectAddress);
HookResult HookFromFunctionPointer(int Token, void** FunctionPointer, void* RedirectAddress);
void*      ReplaceFunctionPointer(void** Slot, void* RedirectAddress);
void*      ReplaceVTableSlot(void* Instance, int Offset, void* RedirectAddress);

HookResult HookFromVTable(int Token, void* Instance, int Offset, void* RedirectAddress);

} // namespace RorinnnTools::Hook
