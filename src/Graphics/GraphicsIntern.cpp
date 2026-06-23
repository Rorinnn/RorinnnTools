// GraphicsIntern — 图形方法定位内部工具实现

module;

#include <Windows.h>

module RorinnnTools;
import std;

namespace RorinnnTools::Graphics::detail
{
void CreateDummyWin32Window(DummyWin32Window& Window)
{
    Window.WindowClass               = {};
    Window.WindowClass.cbSize        = sizeof(Window.WindowClass);
    Window.WindowClass.lpfnWndProc   = DefWindowProcA;
    Window.WindowClass.hInstance     = GetModuleHandleA(nullptr);
    Window.WindowClass.lpszClassName = "RorinnnTools_Graphics_DummyWindow";

    RegisterClassExA(&Window.WindowClass);

    Window.WindowHandle = CreateWindowExA(0,
                                          Window.WindowClass.lpszClassName,
                                          "RorinnnTools Dummy Window",
                                          WS_OVERLAPPEDWINDOW,
                                          CW_USEDEFAULT,
                                          CW_USEDEFAULT,
                                          100,
                                          100,
                                          nullptr,
                                          nullptr,
                                          Window.WindowClass.hInstance,
                                          nullptr);
}

void DestroyDummyWin32Window(DummyWin32Window& Window)
{
    DestroyWindow(Window.WindowHandle);
    UnregisterClassA(Window.WindowClass.lpszClassName, Window.WindowClass.hInstance);
}

} // namespace RorinnnTools::Graphics::detail
