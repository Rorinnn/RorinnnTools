// CallStackSpoof.cpp — 用户态调用栈伪造实现

#include "Stealth/CallStackSpoof.hpp"

#include <Psapi.h>

#include <algorithm>
#include <cstring>
#include <random>
#include <vector>

namespace RorinnnTools::Stealth
{

// ShellCode 模板
#pragma pack(push, 1)
struct SpoofShellcodeTemplate
{
    uint8_t  MovR11Imm1Op[2]; // 0x49, 0xBB          mov r11, imm64
    uint64_t FirstXorKey;     // (运行时填充)
    uint8_t  Pad1[56];        // xor / push / sub / lea / movsq / call / pop / add
    uint8_t  MovR11Imm2Op[2];
    uint64_t SecondXorKey;
    uint8_t  Pad2[5]; // xor [rsp], r11; ret
};
#pragma pack(pop)
static_assert(sizeof(SpoofShellcodeTemplate) == 81, "Shellcode template size mismatch");

static const uint8_t kSpoofShellcode[] = {
    // 1. 加密返回地址
    0x49,
    0xBB,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // mov     r11, imm64
    0x4C,
    0x31,
    0x1C,
    0x24, // xor     [rsp], r11

    // 2. 保存非易失性寄存器
    0x56,
    0x57, // push rsi / push rdi

    // 3. 抬高栈空间 0x300
    0x48,
    0x81,
    0xEC,
    0x00,
    0x03,
    0x00,
    0x00, // sub     rsp, 300h

    // 4. 源/目的
    0x48,
    0x8D,
    0xB4,
    0x24,
    0x40,
    0x03,
    0x00,
    0x00, // lea     rsi, [rsp + 340h]
    0x48,
    0x8D,
    0x7C,
    0x24,
    0x20, // lea     rdi, [rsp + 20h]

    // 5. 拷贝栈参数
    0x4C,
    0x8B,
    0xD1, // mov r10, rcx
    0xB9,
    0x40,
    0x00,
    0x00,
    0x00, // mov ecx, 40h
    0xF3,
    0x48,
    0xA5, // rep movsq
    0x49,
    0x8B,
    0xCA, // mov rcx, r10

    // 6. 调用真实目标
    0xFF,
    0x94,
    0x24,
    0x38,
    0x03,
    0x00,
    0x00, // call [rsp + 338h]

    // 7. 还原栈与寄存器
    0x48,
    0x81,
    0xC4,
    0x00,
    0x03,
    0x00,
    0x00, // add rsp, 300h
    0x5F,
    0x5E, // pop rdi / pop rsi

    // 8. 解密返回地址
    0x49,
    0xBB,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // mov r11, imm64
    0x4C,
    0x31,
    0x1C,
    0x24, // xor [rsp], r11

    // 9. 回到原始调用方
    0xC3,
};
static_assert(sizeof(kSpoofShellcode) == sizeof(SpoofShellcodeTemplate), "Shellcode size mismatch");

// 黑名单
static bool IsForbiddenHostName(const char* BaseName)
{
    static const char* const kBlacklist[] = {
        // 系统核心
        "ntdll.dll",
        "kernel32.dll",
        "kernelbase.dll",
        "user32.dll",
        "advapi32.dll",
        "gdi32.dll",
        "gdi32full.dll",
        "combase.dll",
        "ole32.dll",
        "oleaut32.dll",
        "rpcrt4.dll",
        "sechost.dll",
        "shell32.dll",
        "shlwapi.dll",
        "win32u.dll",
        "imm32.dll",
        "bcrypt.dll",
        "bcryptprimitives.dll",
        "ncrypt.dll",
        "cryptbase.dll",
        "crypt32.dll",
        "wintrust.dll",
        // C/C++ 运行时
        "msvcrt.dll",
        "ucrtbase.dll",
        "vcruntime140.dll",
        "vcruntime140_1.dll",
        "msvcp140.dll",
        "msvcp140_1.dll",
        "msvcp140_2.dll",
        // 图形/DirectX (反作弊做 vptr/vtable 检查常选这些)
        "d3d11.dll",
        "d3d11sdklayers.dll",
        "d3dcompiler_47.dll",
        "dxgi.dll",
        "dxgidebug.dll",
        "dxcore.dll",
        "d3d9.dll",
        "d3d10.dll",
        "d3d10_1.dll",
        "d3d12.dll",
        "d3d12core.dll",
        "opengl32.dll",
        "vulkan-1.dll",
        "dcomp.dll",
        "dwmapi.dll",
        // 网络
        "ws2_32.dll",
        "mswsock.dll",
        "wininet.dll",
        "winhttp.dll",
        "iphlpapi.dll",
        // 诊断
        "dbghelp.dll",
        "dbgcore.dll",
        "version.dll",
        "psapi.dll",
    };
    for (const char* Name : kBlacklist)
    {
        if (_stricmp(BaseName, Name) == 0) return true;
    }
    return false;
}

// 工具
static HMODULE GetSelfModuleHandle()
{
    HMODULE Self = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCSTR>(&GetSelfModuleHandle),
                       &Self);
    return Self;
}

// 用户态 .text 对齐填充以 0xCC (INT3) 为主, 0x00 较少; 都视为 cave.
static bool IsPaddingByte(uint8_t V)
{
    return V == 0x00 || V == 0xCC;
}

static uint8_t* SearchCodeCaveInModule(HMODULE Module, size_t CaveSize)
{
    if (!Module || CaveSize == 0) return nullptr;

    auto* Base = reinterpret_cast<uint8_t*>(Module);
    auto* Dos  = reinterpret_cast<IMAGE_DOS_HEADER*>(Base);
    if (Dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;
    auto* Nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(Base + Dos->e_lfanew);
    if (Nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;

    auto* Section = IMAGE_FIRST_SECTION(Nt);
    for (uint16_t i = 0; i < Nt->FileHeader.NumberOfSections; ++i)
    {
        if (memcmp(Section[i].Name, ".text", 5) != 0) continue;

        DWORD Rva  = Section[i].VirtualAddress;
        DWORD Size = Section[i].Misc.VirtualSize;
        if (Size <= CaveSize) continue;

        uint8_t* SectionStart = Base + Rva;
        uint8_t* SearchEnd    = SectionStart + Size - CaveSize;

        for (uint8_t* P = SectionStart; P <= SearchEnd; ++P)
        {
            bool AllPadding = true;
            for (size_t k = 0; k < CaveSize; ++k)
            {
                if (!IsPaddingByte(P[k]))
                {
                    AllPadding  = false;
                    P          += k;
                    break;
                }
            }
            if (!AllPadding) continue;

            // ShellCode 不能跨页
            uintptr_t Start = reinterpret_cast<uintptr_t>(P);
            uintptr_t End   = Start + CaveSize - 1;
            if ((Start >> 12) != (End >> 12)) continue;
            return P;
        }
    }
    return nullptr;
}

static bool EnumProcessModulesGrowing(HANDLE Proc, std::vector<HMODULE>& Modules)
{
    Modules.assign(256, nullptr);
    for (int Attempt = 0; Attempt < 4; ++Attempt)
    {
        DWORD BufBytes = static_cast<DWORD>(Modules.size() * sizeof(HMODULE));
        DWORD Needed   = 0;
        if (!K32EnumProcessModules(Proc, Modules.data(), BufBytes, &Needed)) return false;
        if (Needed <= BufBytes)
        {
            Modules.resize(Needed / sizeof(HMODULE));
            return true;
        }
        Modules.assign(Needed / sizeof(HMODULE), nullptr);
    }
    return false;
}

static bool WriteShellcodeToCave(uint8_t* Dest, const uint8_t* Source, size_t Size)
{
    DWORD OldProtect = 0;
    if (!VirtualProtect(Dest, Size, PAGE_EXECUTE_READWRITE, &OldProtect)) return false;
    memcpy(Dest, Source, Size);
    DWORD Tmp = 0;
    VirtualProtect(Dest, Size, OldProtect, &Tmp);
    FlushInstructionCache(GetCurrentProcess(), Dest, Size);
    return true;
}

bool CallStackSpoof::Init(uint64_t XorKey)
{
    if (m_Trampoline != 0) return true;
    if (XorKey == 0) return false;

    HMODULE Self = GetSelfModuleHandle();

    HANDLE               Proc = GetCurrentProcess();
    std::vector<HMODULE> Modules;
    if (!EnumProcessModulesGrowing(Proc, Modules) || Modules.empty())
    {
        return false;
    }

    std::random_device Rd;
    std::mt19937       Gen(Rd());
    std::shuffle(Modules.begin(), Modules.end(), Gen);

    uint8_t* Slot = nullptr;
    for (HMODULE Mod : Modules)
    {
        if (Mod == Self) continue;
        char Name[MAX_PATH] = {};
        if (K32GetModuleBaseNameA(Proc, Mod, Name, MAX_PATH) == 0) continue;
        if (IsForbiddenHostName(Name)) continue;

        Slot = SearchCodeCaveInModule(Mod, sizeof(SpoofShellcodeTemplate));
        if (Slot)
        {
            break;
        }
    }

    if (!Slot)
    {
        return false;
    }

    uint8_t Buffer[sizeof(SpoofShellcodeTemplate)];
    memcpy(Buffer, kSpoofShellcode, sizeof(Buffer));
    auto* Tpl         = reinterpret_cast<SpoofShellcodeTemplate*>(Buffer);
    Tpl->FirstXorKey  = XorKey;
    Tpl->SecondXorKey = XorKey;

    if (!WriteShellcodeToCave(Slot, Buffer, sizeof(Buffer)))
    {
        return false;
    }

    m_Trampoline = reinterpret_cast<uint64_t>(Slot);
    return true;
}

} // namespace RorinnnTools::Stealth
