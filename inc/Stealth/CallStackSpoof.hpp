#pragma once

// CallStackSpoof.hpp — 用户态调用栈伪造接口

#include <Windows.h>

#include <cstddef>
#include <cstdint>

namespace RorinnnTools::Stealth
{

class CallStackSpoof
{
  public:
    CallStackSpoof() = default;

    // 在随机合法宿主模块内寄生 trampoline. 幂等; 失败返回 false.
    // XorKey 为 0 时返回 false.
    bool Init(uint64_t XorKey);

    // Trampoline 地址. 未初始化或 Init 失败时为 0.
    uint64_t GetTrampoline() const
    {
        return m_Trampoline;
    }

    bool IsReady() const
    {
        return m_Trampoline != 0;
    }

    // 通过 trampoline 调用目标函数. 未初始化时会直接调用 Func, 不做栈伪造.
    //
    // trampoline ABI (见实现):
    //   RCX = A1, RDX = A2, R8 = A3, R9 = A4
    //   [rsp+0x28] = 目标函数指针
    //   [rsp+0x30..] = 第 5 参及以上 (最多拷贝 64 个 qword)
    template <typename RetT = uint64_t,
              typename... Args,
              typename T1 = uint64_t,
              typename T2 = uint64_t,
              typename T3 = uint64_t,
              typename T4 = uint64_t>
    RetT Invoke(void* Func, T1 A1 = {}, T2 A2 = {}, T3 A3 = {}, T4 A4 = {}, Args... Rest) const
    {
        if (m_Trampoline == 0)
        {
            return reinterpret_cast<RetT (*)(T1, T2, T3, T4, Args...)>(Func)(A1, A2, A3, A4, Rest...);
        }
        using TrampolineFn = RetT (*)(T1, T2, T3, T4, void*, Args...);
        return reinterpret_cast<TrampolineFn>(m_Trampoline)(A1, A2, A3, A4, Func, Rest...);
    }

  private:
    uint64_t m_Trampoline = 0;
};

template <class Ret, class... Args> Ret SpoofRetType(Ret (*)(Args...));

} // namespace RorinnnTools::Stealth

#define CALL_STACK_SPOOF(SpoofInst, Func, ...)                                                               \
    (SpoofInst).Invoke<decltype(::RorinnnTools::Stealth::SpoofRetType(Func))>(reinterpret_cast<void*>(Func), \
                                                                              __VA_ARGS__)
