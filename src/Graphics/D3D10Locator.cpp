// D3D10Locator — D3D10 方法地址定位实现

#include "Graphics/D3D10Locator.hpp"

#include <Windows.h>
#include <d3d10.h>
#include <dxgi.h>

#include "GraphicsIntern.hpp"

namespace RorinnnTools::Graphics
{
#pragma region 类型定义

using CreateDXGIFactory_t = HRESULT(WINAPI*)(REFIID Riid, void** PPFactory);

using D3D10CreateDeviceAndSwapChain_t = HRESULT(WINAPI*)(IDXGIAdapter*         Adapter,
                                                         D3D10_DRIVER_TYPE     DriverType,
                                                         HMODULE               Software,
                                                         UINT                  Flags,
                                                         UINT                  SDKVersion,
                                                         DXGI_SWAP_CHAIN_DESC* SwapChainDesc,
                                                         IDXGISwapChain**      PPSwapChain,
                                                         ID3D10Device**        PPDevice);

#pragma endregion

#pragma region 公开接口

LocateStatus LocateD3D10(D3D10Methods& Out)
{
    HMODULE DxgiModule = GetModuleHandleA("dxgi.dll");
    if (!DxgiModule) return LocateStatus::ModuleNotFound;

    HMODULE D3D10Module = GetModuleHandleA("d3d10.dll");
    if (!D3D10Module) return LocateStatus::ModuleNotFound;

    auto CreateDXGIFactoryFn = reinterpret_cast<CreateDXGIFactory_t>(GetProcAddress(DxgiModule, "CreateDXGIFactory"));
    if (!CreateDXGIFactoryFn) return LocateStatus::MethodNotFound;

    IDXGIFactory* Factory = nullptr;
    HRESULT       HResult = CreateDXGIFactoryFn(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&Factory));
    if (HResult != S_OK) return LocateStatus::D3D10CreateDXGIFactoryFailed;
    auto FactoryGuard = detail::MakeScopeExit([&]() { Factory->Release(); });

    IDXGIAdapter* Adapter;
    HResult = Factory->EnumAdapters(0, &Adapter);
    if (HResult != S_OK) return LocateStatus::D3D10EnumAdaptersFailed;
    auto AdapterGuard = detail::MakeScopeExit([&]() { Adapter->Release(); });

    auto D3D10CreateDeviceAndSwapChainFn =
        reinterpret_cast<D3D10CreateDeviceAndSwapChain_t>(GetProcAddress(D3D10Module, "D3D10CreateDeviceAndSwapChain"));
    if (!D3D10CreateDeviceAndSwapChainFn) return LocateStatus::MethodNotFound;

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

    IDXGISwapChain* SwapChain;
    ID3D10Device*   Device;
    HResult = D3D10CreateDeviceAndSwapChainFn(
        Adapter, D3D10_DRIVER_TYPE_HARDWARE, nullptr, 0, D3D10_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device);
    if (HResult != S_OK) return LocateStatus::D3D10DeviceCreateFailed;
    auto CreatedObjectsGuard = detail::MakeScopeExit(
        [&]()
        {
            SwapChain->Release();
            Device->Release();
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

    return LocateStatus::Ok;
}

#pragma endregion
} // namespace RorinnnTools::Graphics
