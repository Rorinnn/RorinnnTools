// D3D12Locator — D3D12 方法地址定位实现

module;

#include <Windows.h>
#include <d3d12.h>
#include <dxgi.h>

module RorinnnTools;

namespace RorinnnTools::Graphics
{
#pragma region 类型定义

using CreateDXGIFactory_t = HRESULT(WINAPI*)(REFIID Riid, void** PPFactory);

using D3D12CreateDevice_t = HRESULT(WINAPI*)(IUnknown*         Adapter,
                                             D3D_FEATURE_LEVEL MinimumFeatureLevel,
                                             REFIID            Riid,
                                             void**            PPDevice);

#pragma endregion

#pragma region 公开接口

LocateStatus LocateD3D12(D3D12Methods& Out)
{
    HMODULE DxgiModule = GetModuleHandleA("dxgi.dll");
    if (!DxgiModule) return LocateStatus::ModuleNotFound;

    HMODULE D3D12Module = GetModuleHandleA("d3d12.dll");
    if (!D3D12Module) return LocateStatus::ModuleNotFound;

    auto CreateDXGIFactoryFn = reinterpret_cast<CreateDXGIFactory_t>(GetProcAddress(DxgiModule, "CreateDXGIFactory"));
    if (!CreateDXGIFactoryFn) return LocateStatus::MethodNotFound;

    IDXGIFactory* Factory;
    HRESULT       HResult = CreateDXGIFactoryFn(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&Factory));
    if (HResult != S_OK) return LocateStatus::D3D12CreateDXGIFactoryFailed;
    auto FactoryGuard = detail::MakeScopeExit([&]() { Factory->Release(); });

    IDXGIAdapter* Adapter;
    HResult = Factory->EnumAdapters(0, &Adapter);
    if (HResult != S_OK) return LocateStatus::D3D12EnumAdaptersFailed;
    auto AdapterGuard = detail::MakeScopeExit([&]() { Adapter->Release(); });

    auto D3D12CreateDeviceFn = reinterpret_cast<D3D12CreateDevice_t>(GetProcAddress(D3D12Module, "D3D12CreateDevice"));
    if (!D3D12CreateDeviceFn) return LocateStatus::MethodNotFound;

    ID3D12Device* Device;
    HResult =
        D3D12CreateDeviceFn(Adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), reinterpret_cast<void**>(&Device));
    if (HResult != S_OK) return LocateStatus::D3D12CreateDeviceFailed;
    auto DeviceGuard = detail::MakeScopeExit([&]() { Device->Release(); });

    D3D12_COMMAND_QUEUE_DESC CommandQueueDesc{};
    CommandQueueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
    CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    ID3D12CommandQueue* CommandQueue;
    HResult = Device->CreateCommandQueue(
        &CommandQueueDesc, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&CommandQueue));
    if (HResult != S_OK) return LocateStatus::D3D12CreateCommandQueueFailed;
    auto CommandQueueGuard = detail::MakeScopeExit([&]() { CommandQueue->Release(); });

    ID3D12CommandAllocator* CommandAllocator;
    HResult = Device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&CommandAllocator));
    if (HResult != S_OK) return LocateStatus::D3D12CreateCommandAllocatorFailed;
    auto CommandAllocatorGuard = detail::MakeScopeExit([&]() { CommandAllocator->Release(); });

    ID3D12GraphicsCommandList* CommandList;
    HResult = Device->CreateCommandList(0,
                                        D3D12_COMMAND_LIST_TYPE_DIRECT,
                                        CommandAllocator,
                                        nullptr,
                                        __uuidof(ID3D12GraphicsCommandList),
                                        reinterpret_cast<void**>(&CommandList));
    if (HResult != S_OK) return LocateStatus::D3D12CreateCommandListFailed;
    auto CommandListGuard = detail::MakeScopeExit([&]() { CommandList->Release(); });

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
    SwapChainDesc.BufferCount                        = 2;
    SwapChainDesc.OutputWindow                       = Window.WindowHandle;
    SwapChainDesc.Windowed                           = TRUE;
    SwapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDesc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain* SwapChain;
    HResult = Factory->CreateSwapChain(CommandQueue, &SwapChainDesc, &SwapChain);
    if (HResult != S_OK) return LocateStatus::D3D12CreateSwapChainFailed;
    auto SwapChainGuard = detail::MakeScopeExit([&]() { SwapChain->Release(); });

    for (auto VTable = *reinterpret_cast<void***>(Device); VTable; VTable++)
    {
        void* Ptr = *VTable;
        if (!Ptr) break;

        Out.DeviceMethods.push_back({VTable, Ptr});
    }

    for (auto VTable = *reinterpret_cast<void***>(CommandQueue); VTable; VTable++)
    {
        void* Ptr = *VTable;
        if (!Ptr) break;

        Out.CommandQueueMethods.push_back({VTable, Ptr});
    }

    for (auto VTable = *reinterpret_cast<void***>(CommandAllocator); VTable; VTable++)
    {
        void* Ptr = *VTable;
        if (!Ptr) break;

        Out.CommandAllocatorMethods.push_back({VTable, Ptr});
    }

    for (auto VTable = *reinterpret_cast<void***>(CommandList); VTable; VTable++)
    {
        void* Ptr = *VTable;
        if (!Ptr) break;

        Out.CommandListMethods.push_back({VTable, Ptr});
    }

    for (auto VTable = *reinterpret_cast<void***>(SwapChain); VTable; VTable++)
    {
        void* Ptr = *VTable;
        if (!Ptr) break;

        Out.SwapChainMethods.push_back({VTable, Ptr});
    }

    return LocateStatus::Ok;
}

#pragma endregion
} // namespace RorinnnTools::Graphics
