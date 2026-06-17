// D3D11Locator — D3D11 方法地址定位实现

module;

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

module RorinnnTools;

namespace RorinnnTools::Graphics
{
#pragma region 类型定义

using CreateDXGIFactory_t = HRESULT(WINAPI*)(REFIID Riid, void** PPFactory);

using D3D11CreateDeviceAndSwapChain_t = HRESULT(WINAPI*)(IDXGIAdapter*               Adapter,
                                                         D3D_DRIVER_TYPE             DriverType,
                                                         HMODULE                     Software,
                                                         UINT                        Flags,
                                                         const D3D_FEATURE_LEVEL*    PFeatureLevels,
                                                         UINT                        FeatureLevels,
                                                         UINT                        SDKVersion,
                                                         const DXGI_SWAP_CHAIN_DESC* PSwapChainDesc,
                                                         IDXGISwapChain**            PPSwapChain,
                                                         ID3D11Device**              PPDevice,
                                                         D3D_FEATURE_LEVEL*          PFeatureLevel,
                                                         ID3D11DeviceContext**       PPImmediateContext);

#pragma endregion

#pragma region 公开接口

LocateStatus LocateD3D11(D3D11Methods& Out)
{
    HMODULE DxgiModule = GetModuleHandleA("dxgi.dll");
    if (!DxgiModule) return LocateStatus::ModuleNotFound;

    HMODULE D3D11Module = GetModuleHandleA("d3d11.dll");
    if (!D3D11Module) return LocateStatus::ModuleNotFound;

    auto CreateDXGIFactoryFn = reinterpret_cast<CreateDXGIFactory_t>(GetProcAddress(DxgiModule, "CreateDXGIFactory"));
    if (!CreateDXGIFactoryFn) return LocateStatus::MethodNotFound;

    IDXGIFactory* Factory;
    HRESULT       HResult = CreateDXGIFactoryFn(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&Factory));
    if (HResult != S_OK) return LocateStatus::D3D11CreateDXGIFactoryFailed;
    auto FactoryGuard = detail::MakeScopeExit([&]() { Factory->Release(); });

    IDXGIAdapter* Adapter;
    HResult = Factory->EnumAdapters(0, &Adapter);
    if (HResult != S_OK) return LocateStatus::D3D11EnumAdaptersFailed;
    auto AdapterGuard = detail::MakeScopeExit([&]() { Adapter->Release(); });

    auto D3D11CreateDeviceAndSwapChainFn =
        reinterpret_cast<D3D11CreateDeviceAndSwapChain_t>(GetProcAddress(D3D11Module, "D3D11CreateDeviceAndSwapChain"));
    if (!D3D11CreateDeviceAndSwapChainFn) return LocateStatus::MethodNotFound;

    detail::DummyWin32Window Window{};
    detail::CreateDummyWin32Window(Window);
    auto WindowGuard = detail::MakeScopeExit([&]() { detail::DestroyDummyWin32Window(Window); });

    DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
    SwapChainDesc.BufferDesc.Width                   = 100;
    SwapChainDesc.BufferDesc.Height                  = 100;
    SwapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.SampleDesc.Count                   = 1;
    SwapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount                        = 1;
    SwapChainDesc.OutputWindow                       = Window.WindowHandle;
    SwapChainDesc.Windowed                           = TRUE;
    SwapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
    SwapChainDesc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    const D3D_FEATURE_LEVEL FeatureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    IDXGISwapChain*      SwapChain;
    ID3D11Device*        Device;
    ID3D11DeviceContext* Context;
    D3D_FEATURE_LEVEL    FeatureLevel;

    HResult = D3D11CreateDeviceAndSwapChainFn(Adapter,
                                              D3D_DRIVER_TYPE_UNKNOWN,
                                              nullptr,
                                              0,
                                              FeatureLevels,
                                              ARRAYSIZE(FeatureLevels),
                                              D3D11_SDK_VERSION,
                                              &SwapChainDesc,
                                              &SwapChain,
                                              &Device,
                                              &FeatureLevel,
                                              &Context);
    if (HResult != S_OK) return LocateStatus::D3D11CreateDeviceAndSwapChainFailed;
    auto CreatedObjectsGuard = detail::MakeScopeExit(
        [&]()
        {
            SwapChain->Release();
            Device->Release();
            Context->Release();
        });

    for (auto VTable = *reinterpret_cast<void***>(SwapChain); VTable; VTable++)
    {
        void* Ptr = *VTable;
        if (!Ptr) break;

        Out.SwapChainMethods.push_back({VTable, Ptr});
    }

    for (auto VTable = *reinterpret_cast<void***>(Device); VTable; VTable++)
    {
        void* Ptr = *VTable;
        if (!Ptr) break;

        Out.DeviceMethods.push_back({VTable, Ptr});
    }

    for (auto VTable = *reinterpret_cast<void***>(Context); VTable; VTable++)
    {
        void* Ptr = *VTable;
        if (!Ptr) break;

        Out.ContextMethods.push_back({VTable, Ptr});
    }

    return LocateStatus::Ok;
}

#pragma endregion
} // namespace RorinnnTools::Graphics
