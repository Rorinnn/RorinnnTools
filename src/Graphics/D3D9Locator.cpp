// D3D9Locator — D3D9 方法地址定位实现

module;

#include <Windows.h>
#include <d3d9.h>

module RorinnnTools;

namespace RorinnnTools::Graphics
{
#pragma region 类型定义

using Direct3DCreate9_t = IDirect3D9*(WINAPI*)(UINT SDKVersion);

#pragma endregion

#pragma region 公开接口

LocateStatus LocateD3D9(D3D9Methods& Out)
{
    HMODULE D3D9Module = GetModuleHandleA("d3d9.dll");
    if (!D3D9Module) return LocateStatus::ModuleNotFound;

    auto Direct3DCreate9Fn = reinterpret_cast<Direct3DCreate9_t>(GetProcAddress(D3D9Module, "Direct3DCreate9"));
    if (!Direct3DCreate9Fn) return LocateStatus::MethodNotFound;

    IDirect3D9* Direct3D = Direct3DCreate9Fn(D3D_SDK_VERSION);
    if (!Direct3D) return LocateStatus::D3D9Direct3DCreate9Failed;
    auto Direct3DGuard = detail::MakeScopeExit([&]() { Direct3D->Release(); });

    detail::DummyWin32Window Window{};
    detail::CreateDummyWin32Window(Window);
    auto WindowGuard = detail::MakeScopeExit([&]() { detail::DestroyDummyWin32Window(Window); });

    D3DPRESENT_PARAMETERS PresentParameters{};
    PresentParameters.Windowed      = TRUE;
    PresentParameters.SwapEffect    = D3DSWAPEFFECT_DISCARD;
    PresentParameters.hDeviceWindow = Window.WindowHandle;

    IDirect3DDevice9* Device;
    HRESULT           HResult = Direct3D->CreateDevice(D3DADAPTER_DEFAULT,
                                             D3DDEVTYPE_HAL,
                                             Window.WindowHandle,
                                             D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                             &PresentParameters,
                                             &Device);
    if (HResult != S_OK) return LocateStatus::D3D9CreateDeviceFailed;
    auto DeviceGuard = detail::MakeScopeExit([&]() { Device->Release(); });

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
