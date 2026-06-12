#pragma once

// GraphicsLocator — 图形 API 方法定位公共类型

#include <cstdint>

namespace RorinnnTools::Graphics
{
enum class LocateStatus : uint32_t
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

} // namespace RorinnnTools::Graphics
