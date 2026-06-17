// ManualMapShellcode.cpp — 远程手动映射入口

module;

#include <Windows.h>

module RorinnnTools;

namespace RorinnnTools
{

void __stdcall ManualMapShellcode(ManualMappingData* Data)
{
    if (!Data)
    {
        return;
    }

    BYTE* Base           = Data->Base;
    auto* OptionalHeader = &reinterpret_cast<IMAGE_NT_HEADERS*>(
                                Base + reinterpret_cast<IMAGE_DOS_HEADER*>(reinterpret_cast<uintptr_t>(Base))->e_lfanew)
                                ->OptionalHeader;

    auto LoadLibraryA   = Data->LoadLibraryA;
    auto GetProcAddress = Data->GetProcAddress;
#ifdef _WIN64
    auto RtlAddFunctionTable = Data->RtlAddFunctionTable;
#endif
    auto DllEntryPoint = reinterpret_cast<DllEntryPoint_t>(Base + OptionalHeader->AddressOfEntryPoint);

    BYTE* LocationDelta = Base - OptionalHeader->ImageBase;
    if (LocationDelta && OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
    {
        auto* RelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
            Base + OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        const auto* RelocEnd = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
            reinterpret_cast<uintptr_t>(RelocData) +
            OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);

        while (RelocData < RelocEnd && RelocData->SizeOfBlock)
        {
            const UINT EntryCount   = (RelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            WORD*      RelativeInfo = reinterpret_cast<WORD*>(RelocData + 1);

            for (UINT i = 0; i != EntryCount; ++i, ++RelativeInfo)
            {
#ifdef _WIN64
                const bool RelocMatch = ((*RelativeInfo >> 0x0C) == IMAGE_REL_BASED_DIR64);
#else
                const bool RelocMatch = ((*RelativeInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW);
#endif
                if (RelocMatch)
                {
                    UINT_PTR* Patch =
                        reinterpret_cast<UINT_PTR*>(Base + RelocData->VirtualAddress + ((*RelativeInfo) & 0xFFF));
                    *Patch += reinterpret_cast<UINT_PTR>(LocationDelta);
                }
            }

            RelocData =
                reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(RelocData) + RelocData->SizeOfBlock);
        }
    }

    if (OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
    {
        auto* ImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
            Base + OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

        while (ImportDescr->Name)
        {
            char*     ModuleName = reinterpret_cast<char*>(Base + ImportDescr->Name);
            HINSTANCE Module     = LoadLibraryA(ModuleName);

            ULONG_PTR* ThunkRef = reinterpret_cast<ULONG_PTR*>(Base + ImportDescr->OriginalFirstThunk);
            ULONG_PTR* FuncRef  = reinterpret_cast<ULONG_PTR*>(Base + ImportDescr->FirstThunk);
            if (!ThunkRef)
            {
                ThunkRef = FuncRef;
            }

            for (; *ThunkRef; ++ThunkRef, ++FuncRef)
            {
                if (IMAGE_SNAP_BY_ORDINAL(*ThunkRef))
                {
                    *FuncRef = reinterpret_cast<ULONG_PTR>(
                        GetProcAddress(Module, reinterpret_cast<char*>(*ThunkRef & 0xFFFF)));
                }
                else
                {
                    auto* Import = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(Base + (*ThunkRef));
                    *FuncRef     = reinterpret_cast<ULONG_PTR>(GetProcAddress(Module, Import->Name));
                }
            }

            ++ImportDescr;
        }
    }

    if (OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
    {
        auto* Tls = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(
            Base + OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        auto* Callback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(Tls->AddressOfCallBacks);
        for (; Callback && *Callback; ++Callback)
        {
            (*Callback)(Base, DLL_PROCESS_ATTACH, nullptr);
        }
    }

    bool ExceptionSupportFailed = false;
#ifdef _WIN64
    if (Data->SEHSupport)
    {
        auto Exception = OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
        if (Exception.Size)
        {
            if (!RtlAddFunctionTable(reinterpret_cast<IMAGE_RUNTIME_FUNCTION_ENTRY*>(Base + Exception.VirtualAddress),
                                     Exception.Size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY),
                                     reinterpret_cast<DWORD64>(Base)))
            {
                ExceptionSupportFailed = true;
            }
        }
    }
#endif

    DllEntryPoint(Base, Data->Reason, Data->Reserved);
    Data->Module = ExceptionSupportFailed ? reinterpret_cast<HINSTANCE>(0x505050) : reinterpret_cast<HINSTANCE>(Base);
}

} // namespace RorinnnTools
