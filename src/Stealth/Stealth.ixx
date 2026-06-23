module;

export module RorinnnTools:Stealth;
import std;

export namespace RorinnnTools::Stealth
{

class CallStackSpoof
{
  public:
    CallStackSpoof() = default;

    bool Init(std::uint64_t XorKey);

    std::uint64_t GetTrampoline() const
    {
        return m_Trampoline;
    }

    bool IsReady() const
    {
        return m_Trampoline != 0;
    }

    template <typename RetT = std::uint64_t,
              typename... Args,
              typename T1 = std::uint64_t,
              typename T2 = std::uint64_t,
              typename T3 = std::uint64_t,
              typename T4 = std::uint64_t>
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
    std::uint64_t m_Trampoline = 0;
};

template <class Ret, class... Args>
Ret SpoofRetType(Ret (*)(Args...));

} // namespace RorinnnTools::Stealth
