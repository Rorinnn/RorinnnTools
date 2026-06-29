module;

#include <Windows.h>

export module RnTools:Graphics;
import std;

namespace RnTools::Graphics::detail
{

struct DummyWin32Window
{
    WNDCLASSEXA WindowClass{};
    HWND        WindowHandle = nullptr;
};

void CreateDummyWindow(DummyWin32Window& Window);
void DestroyDummyWindow(DummyWin32Window& Window);

template <typename F>
class ScopeExit
{
  public:
    explicit ScopeExit(F Callback) : Callback(std::move(Callback)) {}
    ScopeExit(const ScopeExit&)            = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;

    ~ScopeExit()
    {
        Callback();
    }

  private:
    F Callback;
};

template <typename F>
ScopeExit<F> MakeScopeExit(F Callback)
{
    return ScopeExit<F>(std::move(Callback));
}

} // namespace RnTools::Graphics::detail

export namespace RnTools::Graphics
{

enum class LocateStatus : std::uint32_t
{
    Ok               = 0,
    Unknown          = 1,
    ModuleNotFound   = 2,
    MethodNotFound   = 3,
    BackendErrorBase = 4,

    D3D9Direct3DCreate9Failed = BackendErrorBase,
    D3D9CreateDeviceFailed,

    D3D10CreateDXGIFactoryFailed = BackendErrorBase,
    D3D10EnumAdaptersFailed,
    D3D10DeviceCreateFailed,

    D3D11CreateDXGIFactoryFailed = BackendErrorBase,
    D3D11EnumAdaptersFailed,
    D3D11CreateDeviceAndSwapChainFailed,

    D3D12CreateDXGIFactoryFailed = BackendErrorBase,
    D3D12EnumAdaptersFailed,
    D3D12CreateDeviceFailed,
    D3D12CreateCommandQueueFailed,
    D3D12CreateCommandAllocatorFailed,
    D3D12CreateCommandListFailed,
    D3D12CreateSwapChainFailed,
};

struct VTableEntry
{
    void** Slot    = nullptr;
    void*  Address = nullptr;
};

struct D3D9Methods
{
    std::vector<VTableEntry> DeviceMethods;
};

struct D3D10Methods
{
    std::vector<VTableEntry> SwapChainMethods;
    std::vector<VTableEntry> DeviceMethods;
};

struct D3D11Methods
{
    std::vector<VTableEntry> SwapChainMethods;
    std::vector<VTableEntry> DeviceMethods;
    std::vector<VTableEntry> ContextMethods;
};

struct D3D12Methods
{
    std::vector<VTableEntry> DeviceMethods;
    std::vector<VTableEntry> CommandQueueMethods;
    std::vector<VTableEntry> CommandAllocatorMethods;
    std::vector<VTableEntry> CommandListMethods;
    std::vector<VTableEntry> SwapChainMethods;
};

struct OpenGLMethods
{
    std::unordered_map<std::string, void*> Methods;
};

struct VulkanMethods
{
    std::unordered_map<std::string, void*> Methods;
};

LocateStatus LocateD3D9(D3D9Methods& Out);
LocateStatus LocateD3D10(D3D10Methods& Out);
LocateStatus LocateD3D11(D3D11Methods& Out);
LocateStatus LocateD3D12(D3D12Methods& Out);
LocateStatus LocateOpenGL(OpenGLMethods& Out);
LocateStatus LocateVulkan(VulkanMethods& Out);

} // namespace RnTools::Graphics
