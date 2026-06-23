// VehHook.cpp — VEH 异常 Hook 与跳板管理

module;

#include <Windows.h>
#include <TlHelp32.h>

module RorinnnTools;
import std;

namespace RorinnnTools::Hook
{
namespace
{
#pragma region 运行时状态

constexpr std::size_t HardwareBreakpointCount = 4;
constexpr std::size_t AbsoluteJumpSize        = 14;

struct VehHookRecord
{
    int                       Token             = 0;
    void*                     TargetAddress     = nullptr;
    void*                     RedirectAddress   = nullptr;
    VehHookType               Type              = VehHookType::Int3;
    std::uint8_t              OriginalByte      = 0;
    void*                     TrampolineAddress = nullptr;
    std::size_t               TrampolineSize    = 0;
    std::vector<std::uint8_t> TrampolineBytes   = {};
    std::unordered_set<DWORD> TraceThreadIds    = {};
    VehHookCallback           Callback          = {};
};

struct VehHookDispatch
{
    VehHookType     Type         = VehHookType::Int3;
    int             Token        = 0;
    void*           Destination  = nullptr;
    VehHookCallback Callback     = {};
    bool            TraceStart   = false;
    bool            TraceStep    = false;
    bool            ContinueHere = false;
};

std::mutex                 HookMutex;
std::vector<VehHookRecord> HookRecords;
PVOID                      ExceptionHandlerHandle = nullptr;

#pragma endregion

#pragma region 类型判断

static bool IsJumpType(VehHookType Type)
{
    return Type == VehHookType::Int3Jump || Type == VehHookType::HardwareJump;
}

static bool IsHardwareType(VehHookType Type)
{
    return Type == VehHookType::HardwareBreakpoint || Type == VehHookType::HardwareTrace ||
           Type == VehHookType::HardwareJump;
}

static bool IsInt3Type(VehHookType Type)
{
    return Type == VehHookType::Int3 || Type == VehHookType::Int3Trace || Type == VehHookType::Int3Jump;
}

static bool IsKnownType(VehHookType Type)
{
    switch (Type)
    {
        case VehHookType::Int3:
        case VehHookType::Int3Trace:
        case VehHookType::Int3Jump:
        case VehHookType::HardwareBreakpoint:
        case VehHookType::HardwareTrace:
        case VehHookType::HardwareJump:
            return true;
        default:
            return false;
    }
}

#pragma endregion

#pragma region 内存工具

static bool ReadMemoryBytes(void* PAddress, void* PBuffer, std::size_t Size)
{
    if (!PAddress || !PBuffer || Size == 0) return false;

    __try
    {
        std::memcpy(PBuffer, PAddress, Size);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

static bool WriteMemoryBytes(void* PAddress, const void* PData, std::size_t Size)
{
    if (!PAddress || !PData || Size == 0) return false;

    DWORD OldProtection = 0;
    if (!VirtualProtect(PAddress, Size, PAGE_EXECUTE_READWRITE, &OldProtection))
    {
        return false;
    }

    bool Written = false;
    __try
    {
        std::memcpy(PAddress, PData, Size);
        FlushInstructionCache(GetCurrentProcess(), PAddress, Size);
        Written = true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Written = false;
    }

    DWORD UnusedProtection = 0;
    VirtualProtect(PAddress, Size, OldProtection, &UnusedProtection);
    return Written;
}

static void WriteAbsoluteJumpBytes(std::uint8_t* PBuffer, void* PTargetAddress)
{
    PBuffer[0] = 0xFF;
    PBuffer[1] = 0x25;
    PBuffer[2] = 0x00;
    PBuffer[3] = 0x00;
    PBuffer[4] = 0x00;
    PBuffer[5] = 0x00;

    const std::uint64_t TargetAddress = reinterpret_cast<std::uint64_t>(PTargetAddress);
    std::memcpy(PBuffer + 6, &TargetAddress, sizeof(TargetAddress));
}

static VehHookStatus BuildTrampoline(VehHookRecord& Record)
{
    const std::size_t PrefixSize = Record.TrampolineBytes.size();
    const std::size_t TotalSize  = PrefixSize + AbsoluteJumpSize;
    if (TotalSize <= AbsoluteJumpSize && !Record.RedirectAddress) return VehHookStatus::InvalidArgument;

    void* TrampolineAddress = VirtualAlloc(nullptr, TotalSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!TrampolineAddress)
    {
        return VehHookStatus::AllocateFailed;
    }

    std::uint8_t* TrampolineBytes = static_cast<std::uint8_t*>(TrampolineAddress);
    if (PrefixSize > 0)
    {
        std::memcpy(TrampolineBytes, Record.TrampolineBytes.data(), PrefixSize);
    }
    WriteAbsoluteJumpBytes(TrampolineBytes + PrefixSize, Record.RedirectAddress);
    FlushInstructionCache(GetCurrentProcess(), TrampolineAddress, TotalSize);

    Record.TrampolineAddress = TrampolineAddress;
    Record.TrampolineSize    = TotalSize;
    return VehHookStatus::Ok;
}

static void FreeTrampoline(VehHookRecord& Record)
{
    if (!Record.TrampolineAddress) return;

    VirtualFree(Record.TrampolineAddress, 0, MEM_RELEASE);
    Record.TrampolineAddress = nullptr;
    Record.TrampolineSize    = 0;
}

static VehHookStatus InstallInt3Hook(VehHookRecord& Record)
{
    if (!ReadMemoryBytes(Record.TargetAddress, &Record.OriginalByte, sizeof(Record.OriginalByte)))
    {
        return VehHookStatus::ReadFailed;
    }

    constexpr std::uint8_t Int3Opcode = 0xCC;
    if (!WriteMemoryBytes(Record.TargetAddress, &Int3Opcode, sizeof(Int3Opcode)))
    {
        return VehHookStatus::WriteFailed;
    }

    return VehHookStatus::Ok;
}

static VehHookStatus RemoveInt3Hook(const VehHookRecord& Record)
{
    if (!WriteMemoryBytes(Record.TargetAddress, &Record.OriginalByte, sizeof(Record.OriginalByte)))
    {
        return VehHookStatus::WriteFailed;
    }

    return VehHookStatus::Ok;
}

#pragma endregion

#pragma region 硬件断点

static std::uint64_t GetDebugAddress(const CONTEXT& Context, std::size_t Slot)
{
    switch (Slot)
    {
        case 0:
            return Context.Dr0;
        case 1:
            return Context.Dr1;
        case 2:
            return Context.Dr2;
        case 3:
            return Context.Dr3;
        default:
            return 0;
    }
}

static void SetDebugAddress(CONTEXT& Context, std::size_t Slot, std::uint64_t Address)
{
    switch (Slot)
    {
        case 0:
            Context.Dr0 = Address;
            break;
        case 1:
            Context.Dr1 = Address;
            break;
        case 2:
            Context.Dr2 = Address;
            break;
        case 3:
            Context.Dr3 = Address;
            break;
        default:
            break;
    }
}

static bool IsHardwareSlotEnabled(const CONTEXT& Context, std::size_t Slot)
{
    const std::uint64_t EnableBit = 1ull << (Slot * 2);
    return (Context.Dr7 & EnableBit) != 0;
}

static void ConfigureHardwareSlot(CONTEXT& Context, std::size_t Slot, std::uint64_t Address)
{
    const std::uint64_t LocalEnableBit  = 1ull << (Slot * 2);
    const std::uint64_t GlobalEnableBit = 1ull << (Slot * 2 + 1);
    const std::uint64_t ControlMask     = 0xFull << (16 + Slot * 4);

    SetDebugAddress(Context, Slot, Address);
    Context.Dr7 |= LocalEnableBit;
    Context.Dr7 &= ~GlobalEnableBit;
    Context.Dr7 &= ~ControlMask;
}

static void ClearHardwareSlot(CONTEXT& Context, std::size_t Slot)
{
    const std::uint64_t LocalEnableBit  = 1ull << (Slot * 2);
    const std::uint64_t GlobalEnableBit = 1ull << (Slot * 2 + 1);
    const std::uint64_t ControlMask     = 0xFull << (16 + Slot * 4);

    SetDebugAddress(Context, Slot, 0);
    Context.Dr7 &= ~LocalEnableBit;
    Context.Dr7 &= ~GlobalEnableBit;
    Context.Dr7 &= ~ControlMask;
}

static VehHookStatus EnableHardwareBreakpoint(CONTEXT& Context, void* PTargetAddress)
{
    const std::uint64_t TargetAddress = reinterpret_cast<std::uint64_t>(PTargetAddress);

    for (std::size_t i = 0; i < HardwareBreakpointCount; ++i)
    {
        if (GetDebugAddress(Context, i) == TargetAddress)
        {
            ConfigureHardwareSlot(Context, i, TargetAddress);
            return VehHookStatus::Ok;
        }
    }

    for (std::size_t i = 0; i < HardwareBreakpointCount; ++i)
    {
        if (!IsHardwareSlotEnabled(Context, i) || GetDebugAddress(Context, i) == 0)
        {
            ConfigureHardwareSlot(Context, i, TargetAddress);
            return VehHookStatus::Ok;
        }
    }

    return VehHookStatus::HardwareBreakpointLimit;
}

static VehHookStatus DisableHardwareBreakpoint(CONTEXT& Context, void* PTargetAddress)
{
    const std::uint64_t TargetAddress = reinterpret_cast<std::uint64_t>(PTargetAddress);

    for (std::size_t i = 0; i < HardwareBreakpointCount; ++i)
    {
        if (GetDebugAddress(Context, i) == TargetAddress)
        {
            ClearHardwareSlot(Context, i);
        }
    }

    return VehHookStatus::Ok;
}

static VehHookStatus ApplyHardwareBreakpointToThread(DWORD ThreadId, void* PTargetAddress, bool Enable)
{
    HANDLE Thread = OpenThread(
        THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, ThreadId);
    if (!Thread)
    {
        return GetLastError() == ERROR_INVALID_PARAMETER ? VehHookStatus::Ok : VehHookStatus::ThreadOpenFailed;
    }

    const bool ShouldSuspend = ThreadId != GetCurrentThreadId();
    if (ShouldSuspend && SuspendThread(Thread) == static_cast<DWORD>(-1))
    {
        CloseHandle(Thread);
        return VehHookStatus::ThreadContextFailed;
    }

    CONTEXT Context      = {};
    Context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    VehHookStatus Status = VehHookStatus::Ok;
    if (!GetThreadContext(Thread, &Context))
    {
        Status = VehHookStatus::ThreadContextFailed;
    }
    else
    {
        Status = Enable ? EnableHardwareBreakpoint(Context, PTargetAddress)
                        : DisableHardwareBreakpoint(Context, PTargetAddress);
    }

    if (Status == VehHookStatus::Ok && !SetThreadContext(Thread, &Context))
    {
        Status = VehHookStatus::ThreadContextFailed;
    }

    if (ShouldSuspend)
    {
        ResumeThread(Thread);
    }
    CloseHandle(Thread);
    return Status;
}

static VehHookStatus ApplyHardwareBreakpointToThreads(const VehHookRecord& Record, bool Enable)
{
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (Snapshot == INVALID_HANDLE_VALUE)
    {
        return VehHookStatus::ThreadSnapshotFailed;
    }

    THREADENTRY32 Entry = {};
    Entry.dwSize        = sizeof(Entry);

    if (!Thread32First(Snapshot, &Entry))
    {
        CloseHandle(Snapshot);
        return VehHookStatus::ThreadSnapshotFailed;
    }

    VehHookStatus FirstError = VehHookStatus::Ok;
    const DWORD   ProcessId  = GetCurrentProcessId();
    do
    {
        if (Entry.th32OwnerProcessID != ProcessId) continue;

        const VehHookStatus Status = ApplyHardwareBreakpointToThread(Entry.th32ThreadID, Record.TargetAddress, Enable);
        if (Status != VehHookStatus::Ok && FirstError == VehHookStatus::Ok)
        {
            FirstError = Status;
        }
    } while (Thread32Next(Snapshot, &Entry));

    CloseHandle(Snapshot);
    return FirstError;
}

static std::size_t CountHardwareHooks()
{
    return static_cast<std::size_t>(std::count_if(HookRecords.begin(),
                                                  HookRecords.end(),
                                                  [](const VehHookRecord& Record)
                                                  { return IsHardwareType(Record.Type); }));
}

static VehHookStatus InstallHookRecord(VehHookRecord& Record)
{
    VehHookStatus Status = VehHookStatus::Ok;

    if (IsJumpType(Record.Type))
    {
        Status = BuildTrampoline(Record);
        if (Status != VehHookStatus::Ok) return Status;
    }

    if (IsInt3Type(Record.Type))
    {
        Status = InstallInt3Hook(Record);
    }
    else if (IsHardwareType(Record.Type))
    {
        if (CountHardwareHooks() >= HardwareBreakpointCount)
        {
            Status = VehHookStatus::HardwareBreakpointLimit;
        }
        else
        {
            Status = ApplyHardwareBreakpointToThreads(Record, true);
            if (Status != VehHookStatus::Ok)
            {
                ApplyHardwareBreakpointToThreads(Record, false);
            }
        }
    }
    else
    {
        Status = VehHookStatus::TypeInvalid;
    }

    if (Status != VehHookStatus::Ok)
    {
        FreeTrampoline(Record);
    }
    return Status;
}

static VehHookStatus RemoveHookRecord(VehHookRecord& Record)
{
    VehHookStatus Status = VehHookStatus::Ok;
    if (IsInt3Type(Record.Type))
    {
        Status = RemoveInt3Hook(Record);
    }
    else if (IsHardwareType(Record.Type))
    {
        Status = ApplyHardwareBreakpointToThreads(Record, false);
    }
    else
    {
        Status = VehHookStatus::TypeInvalid;
    }

    if (Status == VehHookStatus::Ok)
    {
        FreeTrampoline(Record);
    }
    return Status;
}

#pragma endregion

#pragma region 异常分发

static bool TryBuildDispatch(PEXCEPTION_POINTERS PExceptionInfo, VehHookDispatch& Dispatch)
{
    if (!PExceptionInfo || !PExceptionInfo->ExceptionRecord || !PExceptionInfo->ContextRecord)
    {
        return false;
    }

    const DWORD ExceptionCode   = PExceptionInfo->ExceptionRecord->ExceptionCode;
    void*       Address         = PExceptionInfo->ExceptionRecord->ExceptionAddress;
    const DWORD CurrentThreadId = GetCurrentThreadId();

    std::lock_guard<std::mutex> Guard(HookMutex);
    if (ExceptionCode == EXCEPTION_SINGLE_STEP)
    {
        for (VehHookRecord& Record : HookRecords)
        {
            if (Record.Type != VehHookType::Int3Trace) continue;

            const auto FoundThread = Record.TraceThreadIds.find(CurrentThreadId);
            if (FoundThread == Record.TraceThreadIds.end()) continue;

            Record.TraceThreadIds.erase(FoundThread);
            if (Record.TraceThreadIds.empty())
            {
                constexpr std::uint8_t Int3Opcode = 0xCC;
                WriteMemoryBytes(Record.TargetAddress, &Int3Opcode, sizeof(Int3Opcode));
            }

            Dispatch.Type      = Record.Type;
            Dispatch.Token     = Record.Token;
            Dispatch.TraceStep = true;
            return true;
        }
    }

    for (VehHookRecord& Record : HookRecords)
    {
        if (Record.TargetAddress != Address) continue;

        const bool TraceMatch      = ExceptionCode == EXCEPTION_BREAKPOINT && Record.Type == VehHookType::Int3Trace;
        const bool BreakpointMatch = ExceptionCode == EXCEPTION_BREAKPOINT &&
                                     (Record.Type == VehHookType::Int3 || Record.Type == VehHookType::Int3Jump);
        const bool HardwareMatch = ExceptionCode == EXCEPTION_SINGLE_STEP && IsHardwareType(Record.Type);
        if (!TraceMatch && !BreakpointMatch && !HardwareMatch) continue;

        if (TraceMatch)
        {
            if (!WriteMemoryBytes(Record.TargetAddress, &Record.OriginalByte, sizeof(Record.OriginalByte)))
            {
                return false;
            }

            Record.TraceThreadIds.insert(CurrentThreadId);
        }

        Dispatch.Type         = Record.Type;
        Dispatch.Token        = Record.Token;
        Dispatch.Destination  = TraceMatch
                                    ? Record.TargetAddress
                                    : (IsJumpType(Record.Type) ? Record.TrampolineAddress : Record.RedirectAddress);
        Dispatch.Callback     = Record.Callback;
        Dispatch.TraceStart   = TraceMatch;
        Dispatch.ContinueHere = Record.Type == VehHookType::HardwareTrace;
        if (Dispatch.ContinueHere)
        {
            Dispatch.Destination = Address;
        }
        return Dispatch.Destination != nullptr;
    }

    return false;
}

static void SetInstructionPointer(CONTEXT* PContext, void* PAddress)
{
#ifdef _WIN64
    PContext->Rip = reinterpret_cast<DWORD64>(PAddress);
#else
    PContext->Eip = reinterpret_cast<DWORD>(PAddress);
#endif
}

static void EnableSingleStep(CONTEXT* PContext)
{
    PContext->EFlags |= 0x100;
}

static void EnableResumeFlag(CONTEXT* PContext)
{
    PContext->EFlags |= 0x10000;
}

static bool IsTraceStepPending(int Token, DWORD ThreadId)
{
    std::lock_guard<std::mutex> Guard(HookMutex);
    const auto                  Found = std::find_if(
        HookRecords.begin(), HookRecords.end(), [&](const VehHookRecord& Record)
        { return Record.Token == Token; });
    if (Found == HookRecords.end())
    {
        return false;
    }

    return Found->Type == VehHookType::Int3Trace && Found->TraceThreadIds.find(ThreadId) != Found->TraceThreadIds.end();
}

static LONG NTAPI VehExceptionHandler(PEXCEPTION_POINTERS PExceptionInfo)
{
    VehHookDispatch Dispatch = {};
    if (!TryBuildDispatch(PExceptionInfo, Dispatch))
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    if (Dispatch.TraceStep)
    {
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    if (Dispatch.Callback)
    {
        Dispatch.Callback(Dispatch.Type, PExceptionInfo);
    }

    if (IsHardwareType(Dispatch.Type))
    {
        PExceptionInfo->ContextRecord->Dr6 = 0;
    }

    if (Dispatch.ContinueHere)
    {
        EnableResumeFlag(PExceptionInfo->ContextRecord);
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    SetInstructionPointer(PExceptionInfo->ContextRecord, Dispatch.Destination);
    if (Dispatch.TraceStart && IsTraceStepPending(Dispatch.Token, GetCurrentThreadId()))
    {
        EnableSingleStep(PExceptionInfo->ContextRecord);
    }
    return EXCEPTION_CONTINUE_EXECUTION;
}

static VehHookStatus ValidateOptions(const VehHookOptions& Options)
{
    if (!IsKnownType(Options.Type)) return VehHookStatus::TypeInvalid;
    if (!Options.TargetAddress) return VehHookStatus::InvalidArgument;
    if (Options.Type != VehHookType::Int3Trace && Options.Type != VehHookType::HardwareTrace &&
        !Options.RedirectAddress)
    {
        return VehHookStatus::InvalidArgument;
    }
    if (Options.TrampolineSize > 0 && !Options.TrampolineBytes) return VehHookStatus::InvalidArgument;
    if (!IsJumpType(Options.Type) && Options.TrampolineSize > 0) return VehHookStatus::InvalidArgument;

    return VehHookStatus::Ok;
}

#pragma endregion
} // namespace

#pragma region 公开接口

VehHookStatus InstallVehHookHandler()
{
    std::lock_guard<std::mutex> Guard(HookMutex);
    if (ExceptionHandlerHandle)
    {
        return VehHookStatus::AlreadyInstalled;
    }

    ExceptionHandlerHandle = AddVectoredExceptionHandler(1, VehExceptionHandler);
    return ExceptionHandlerHandle ? VehHookStatus::Ok : VehHookStatus::HandlerInstallFailed;
}

VehHookStatus UninstallVehHookHandler()
{
    const VehHookStatus RemoveStatus = RemoveAllVehHooks();
    if (RemoveStatus != VehHookStatus::Ok)
    {
        return RemoveStatus;
    }

    std::lock_guard<std::mutex> Guard(HookMutex);
    if (!ExceptionHandlerHandle)
    {
        return VehHookStatus::NotInstalled;
    }

    if (!RemoveVectoredExceptionHandler(ExceptionHandlerHandle))
    {
        return VehHookStatus::HandlerInstallFailed;
    }

    ExceptionHandlerHandle = nullptr;
    return VehHookStatus::Ok;
}

bool IsVehHookHandlerInstalled()
{
    std::lock_guard<std::mutex> Guard(HookMutex);
    return ExceptionHandlerHandle != nullptr;
}

VehHookStatus AddVehHook(const VehHookOptions& Options)
{
    std::lock_guard<std::mutex> Guard(HookMutex);
    if (!ExceptionHandlerHandle)
    {
        return VehHookStatus::NotInstalled;
    }

    const VehHookStatus ValidateStatus = ValidateOptions(Options);
    if (ValidateStatus != VehHookStatus::Ok)
    {
        return ValidateStatus;
    }

    const auto Exists = std::find_if(HookRecords.begin(),
                                     HookRecords.end(),
                                     [&](const VehHookRecord& Record)
                                     { return Record.Token == Options.Token; });
    if (Exists != HookRecords.end())
    {
        return VehHookStatus::DuplicateToken;
    }

    const auto SameTarget =
        std::find_if(HookRecords.begin(),
                     HookRecords.end(),
                     [&](const VehHookRecord& Record)
                     { return Record.TargetAddress == Options.TargetAddress; });
    if (SameTarget != HookRecords.end())
    {
        return VehHookStatus::DuplicateTarget;
    }

    VehHookRecord Record   = {};
    Record.Token           = Options.Token;
    Record.TargetAddress   = Options.TargetAddress;
    Record.RedirectAddress = Options.RedirectAddress;
    Record.Type            = Options.Type;
    Record.Callback        = Options.Callback;
    if (Options.TrampolineSize > 0)
    {
        Record.TrampolineBytes.assign(Options.TrampolineBytes, Options.TrampolineBytes + Options.TrampolineSize);
    }

    const VehHookStatus InstallStatus = InstallHookRecord(Record);
    if (InstallStatus != VehHookStatus::Ok)
    {
        return InstallStatus;
    }

    HookRecords.push_back(std::move(Record));
    return VehHookStatus::Ok;
}

VehHookStatus RemoveVehHook(int Token)
{
    std::lock_guard<std::mutex> Guard(HookMutex);

    const auto Found = std::find_if(
        HookRecords.begin(), HookRecords.end(), [&](const VehHookRecord& Record)
        { return Record.Token == Token; });
    if (Found == HookRecords.end())
    {
        return VehHookStatus::TokenNotFound;
    }

    VehHookStatus Status = RemoveHookRecord(*Found);
    if (Status == VehHookStatus::Ok)
    {
        HookRecords.erase(Found);
    }

    return Status;
}

VehHookStatus RemoveAllVehHooks()
{
    std::lock_guard<std::mutex> Guard(HookMutex);

    VehHookStatus              FirstError = VehHookStatus::Ok;
    std::vector<VehHookRecord> FailedRecords;
    FailedRecords.reserve(HookRecords.size());

    for (VehHookRecord& Record : HookRecords)
    {
        const VehHookStatus Status = RemoveHookRecord(Record);
        if (Status != VehHookStatus::Ok && FirstError == VehHookStatus::Ok)
        {
            FirstError = Status;
        }
        if (Status != VehHookStatus::Ok)
        {
            FailedRecords.push_back(std::move(Record));
        }
    }

    HookRecords = std::move(FailedRecords);
    return FirstError;
}

VehHookStatus RefreshHardwareVehHooks()
{
    std::lock_guard<std::mutex> Guard(HookMutex);

    VehHookStatus FirstError = VehHookStatus::Ok;
    for (const VehHookRecord& Record : HookRecords)
    {
        if (!IsHardwareType(Record.Type)) continue;

        const VehHookStatus Status = ApplyHardwareBreakpointToThreads(Record, true);
        if (Status != VehHookStatus::Ok && FirstError == VehHookStatus::Ok)
        {
            FirstError = Status;
        }
    }
    return FirstError;
}

std::size_t GetVehHookCount()
{
    std::lock_guard<std::mutex> Guard(HookMutex);
    return HookRecords.size();
}

const char* GetVehHookStatusName(VehHookStatus Status)
{
    switch (Status)
    {
        case VehHookStatus::Ok:
            return "Ok";
        case VehHookStatus::AlreadyInstalled:
            return "AlreadyInstalled";
        case VehHookStatus::NotInstalled:
            return "NotInstalled";
        case VehHookStatus::HandlerInstallFailed:
            return "HandlerInstallFailed";
        case VehHookStatus::InvalidArgument:
            return "InvalidArgument";
        case VehHookStatus::DuplicateToken:
            return "DuplicateToken";
        case VehHookStatus::DuplicateTarget:
            return "DuplicateTarget";
        case VehHookStatus::TokenNotFound:
            return "TokenNotFound";
        case VehHookStatus::TypeInvalid:
            return "TypeInvalid";
        case VehHookStatus::ReadFailed:
            return "ReadFailed";
        case VehHookStatus::WriteFailed:
            return "WriteFailed";
        case VehHookStatus::AllocateFailed:
            return "AllocateFailed";
        case VehHookStatus::ThreadSnapshotFailed:
            return "ThreadSnapshotFailed";
        case VehHookStatus::ThreadOpenFailed:
            return "ThreadOpenFailed";
        case VehHookStatus::ThreadContextFailed:
            return "ThreadContextFailed";
        case VehHookStatus::HardwareBreakpointLimit:
            return "HardwareBreakpointLimit";
        default:
            return "Unknown";
    }
}

#pragma endregion
} // namespace RorinnnTools::Hook
