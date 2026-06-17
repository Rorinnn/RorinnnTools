module;

#include <Windows.h>
#include <d3d11.h>
#include <imgui.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module RorinnnTools;

namespace RorinnnTools
{

using LoadLibraryA_t   = HINSTANCE(WINAPI*)(const char* FileName);
using GetProcAddress_t = FARPROC(WINAPI*)(HMODULE Module, LPCSTR ProcName);
using DllEntryPoint_t  = BOOL(WINAPI*)(void* Dll, DWORD Reason, void* Reserved);

#ifdef _WIN64
using RtlAddFunctionTable_t = BOOL(WINAPIV*)(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD64 BaseAddress);
#endif

struct ManualMappingData
{
    LoadLibraryA_t   LoadLibraryA;
    GetProcAddress_t GetProcAddress;
#ifdef _WIN64
    RtlAddFunctionTable_t RtlAddFunctionTable;
#endif
    BYTE*     Base;
    HINSTANCE Module;
    DWORD     Reason;
    LPVOID    Reserved;
    BOOL      SEHSupport;
};

void __stdcall ManualMapShellcode(ManualMappingData* Data);

} // namespace RorinnnTools

namespace RorinnnTools::Graphics::detail
{

struct DummyWin32Window
{
    WNDCLASSEXA WindowClass{};
    HWND        WindowHandle = nullptr;
};

void CreateDummyWin32Window(DummyWin32Window& Window);
void DestroyDummyWin32Window(DummyWin32Window& Window);

template <typename F> class ScopeExit
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

template <typename F> ScopeExit<F> MakeScopeExit(F Callback)
{
    return ScopeExit<F>(std::move(Callback));
}

} // namespace RorinnnTools::Graphics::detail

namespace RorinnnTools::ImguiRorinnn::Resources
{

extern const std::uint8_t FontAwesomeBrandsData[];
extern const std::size_t  FontAwesomeBrandsDataSize;
extern const std::uint8_t FontAwesomeSolidData[];
extern const std::size_t  FontAwesomeSolidDataSize;

} // namespace RorinnnTools::ImguiRorinnn::Resources

export namespace RorinnnTools
{

bool BuildMachineCode(std::string& MachineCode);

DWORD GetProcessIdByName(const wchar_t* Name);
bool  IsCorrectTargetArchitecture(HANDLE Process);
bool  EnableDebugPrivilege();

bool ManualMapDll(HANDLE Process,
                  BYTE*  SourceData,
                  SIZE_T FileSize,
                  bool   ClearHeader            = true,
                  bool   ClearNonNeededSections = true,
                  bool   AdjustProtections      = true,
                  bool   SEHExceptionSupport    = true,
                  DWORD  Reason                 = DLL_PROCESS_ATTACH,
                  LPVOID Reserved               = nullptr);

} // namespace RorinnnTools

export namespace RorinnnTools::Crypto
{

bool EncryptAes256Cbc(
    const uint8_t* PData, size_t DataSize, const uint8_t* PKey, size_t KeySize, std::vector<uint8_t>& Output);

bool DecryptAes256Cbc(const uint8_t*        PData,
                      size_t                DataSize,
                      const uint8_t*        PKey,
                      size_t                KeySize,
                      std::vector<uint8_t>& Output,
                      size_t                OriginalSize);

bool Sha256Bytes(std::string_view Text, std::vector<uint8_t>& Hash);
bool Sha256Hex(std::string_view Text, std::string& HashHex);

} // namespace RorinnnTools::Crypto

export namespace RorinnnTools::Encoding
{

std::string Base64UrlEncode(const std::vector<uint8_t>& Bytes);
std::string Base64UrlEncode(std::string_view Text);
bool        Base64UrlDecode(std::string_view Text, std::vector<uint8_t>& Bytes);
bool        DecodeBase64Zlib(std::string_view Text, std::string& Plain, std::string& ErrorText);

} // namespace RorinnnTools::Encoding

export namespace RorinnnTools::Graphics
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

} // namespace RorinnnTools::Graphics

export namespace RorinnnTools::Hook::VTable
{

void* HookSlot(void** Slot, void* HookFn);
void* Hook(void* Instance, void* HookFn, int Offset);

} // namespace RorinnnTools::Hook::VTable

export namespace RorinnnTools::Hook
{

enum class VehHookType
{
    Int3,
    Int3Trace,
    Int3Jump,
    HardwareBreakpoint,
    HardwareTrace,
    HardwareJump,
};

enum class VehHookStatus
{
    Ok,
    AlreadyInstalled,
    NotInstalled,
    HandlerInstallFailed,
    InvalidArgument,
    DuplicateToken,
    DuplicateTarget,
    TokenNotFound,
    TypeInvalid,
    ReadFailed,
    WriteFailed,
    AllocateFailed,
    ThreadSnapshotFailed,
    ThreadOpenFailed,
    ThreadContextFailed,
    HardwareBreakpointLimit,
};

using VehHookCallback = std::function<void(VehHookType Type, PEXCEPTION_POINTERS PExceptionInfo)>;

struct VehHookOptions
{
    int             Token           = 0;
    void*           TargetAddress   = nullptr;
    void*           RedirectAddress = nullptr;
    VehHookType     Type            = VehHookType::Int3;
    VehHookCallback Callback        = {};
    const uint8_t*  TrampolineBytes = nullptr;
    size_t          TrampolineSize  = 0;
};

VehHookStatus InstallVehHookHandler();
VehHookStatus UninstallVehHookHandler();
bool          IsVehHookHandlerInstalled();

VehHookStatus AddVehHook(const VehHookOptions& Options);
VehHookStatus RemoveVehHook(int Token);
VehHookStatus RemoveAllVehHooks();
VehHookStatus RefreshHardwareVehHooks();
size_t        GetVehHookCount();

const char* GetVehHookStatusName(VehHookStatus Status);

} // namespace RorinnnTools::Hook

export namespace RorinnnTools::Input
{

bool TapVirtualKey(WORD VirtualKey, DWORD PressMs = 20);
bool TapVirtualKeyChord(const WORD* PVirtualKeys, size_t Count, DWORD PressMs = 20);

} // namespace RorinnnTools::Input

export namespace RorinnnTools::Memory
{

bool ReadBytes(uintptr_t Ptr, void* PBuffer, size_t Size);
bool IsReadablePtr(uintptr_t Ptr);
bool IsReadableRange(uintptr_t Ptr, size_t Size);
bool ReadPtr(uintptr_t Ptr, uintptr_t& Value);

template <typename T> bool ReadValue(uintptr_t Ptr, T& Value)
{
    if (!ReadBytes(Ptr, &Value, sizeof(T)))
    {
        return false;
    }
    return true;
}

} // namespace RorinnnTools::Memory

export namespace RorinnnTools::Stealth
{

class CallStackSpoof
{
  public:
    CallStackSpoof() = default;

    bool Init(uint64_t XorKey);

    uint64_t GetTrampoline() const
    {
        return m_Trampoline;
    }

    bool IsReady() const
    {
        return m_Trampoline != 0;
    }

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

export namespace RorinnnTools::ImguiRorinnn
{

enum class Ease
{
    Linear,
    OutQuad,
    OutCubic,
    InOutCubic,
    OutBack,
};

enum class Icon : std::uint32_t
{
    None            = 0,
    Check           = 0xF00C,
    Xmark           = 0xF00D,
    CaretDown       = 0xF0D7,
    CaretRight      = 0xF0DA,
    Gear            = 0xF013,
    Wrench          = 0xF0AD,
    Play            = 0xF04B,
    Stop            = 0xF04D,
    Minus           = 0xF068,
    RotateRight     = 0xF2F9,
    WindowRestore   = 0xF2D2,
    CircleQuestion  = 0xF059,
    CircleInfo      = 0xF05A,
    Copy            = 0xF0C5,
    Download        = 0xF019,
    Upload          = 0xF093,
    FolderOpen      = 0xF07C,
    FloppyDisk      = 0xF0C7,
    Trash           = 0xF1F8,
    MagnifyingGlass = 0xF002,
    Link            = 0xF0C1,
    Discord         = 0xF392,
    Github          = 0xF09B,
};

enum class ButtonVariant
{
    Secondary,
    Primary,
    Ghost,
    Danger,
};

enum class StatusKind
{
    Neutral,
    Success,
    Warning,
    Danger,
};

enum class BadgeVariant
{
    Neutral,
    Accent,
    Success,
    Warning,
    Danger,
};

enum class InfoSeverity
{
    Info,
    Success,
    Warning,
    Danger,
};

namespace Color
{
inline const ImVec4 Transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
inline const ImVec4 Black       = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
inline const ImVec4 White       = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
inline const ImVec4 Slate       = ImVec4(0.10f, 0.12f, 0.15f, 1.0f);
inline const ImVec4 DarkSlate   = ImVec4(0.02f, 0.03f, 0.04f, 1.0f);
inline const ImVec4 Gray        = ImVec4(0.50f, 0.54f, 0.61f, 1.0f);
inline const ImVec4 LightGray   = ImVec4(0.72f, 0.74f, 0.78f, 1.0f);
inline const ImVec4 SoftWhite   = ImVec4(0.92f, 0.94f, 0.98f, 1.0f);
inline const ImVec4 Blue        = ImVec4(0.22f, 0.52f, 0.88f, 1.0f);
inline const ImVec4 Green       = ImVec4(0.20f, 0.62f, 0.42f, 1.0f);
inline const ImVec4 Orange      = ImVec4(0.90f, 0.48f, 0.18f, 1.0f);
inline const ImVec4 Purple      = ImVec4(0.54f, 0.42f, 0.86f, 1.0f);
inline const ImVec4 Rose        = ImVec4(0.84f, 0.32f, 0.48f, 1.0f);
inline const ImVec4 Red         = ImVec4(0.86f, 0.28f, 0.32f, 1.0f);
} // namespace Color

struct FontSet
{
    ImFont* Default             = nullptr;
    ImFont* Icon                = nullptr;
    ImFont* IconFixedWidth      = nullptr;
    bool    UsedDefaultTextFont = false;
};

struct ImageTexture
{
    ID3D11ShaderResourceView* View   = nullptr;
    int                       Width  = 0;
    int                       Height = 0;
};

struct Dx11BlurTarget
{
    ID3D11Texture2D*          Texture            = nullptr;
    ID3D11ShaderResourceView* ShaderResourceView = nullptr;
    ID3D11RenderTargetView*   RenderTargetView   = nullptr;
    int                       Width              = 0;
    int                       Height             = 0;
};

struct PanelOptions
{
    ImVec2 Size      = ImVec2(0.0f, 0.0f);
    bool   Border    = false;
    bool   FitHeight = false;
};

struct ModuleHeaderOptions
{
    const char* Description                 = nullptr;
    bool        DefaultOpen                 = false;
    bool        Enabled                     = true;
    bool*       Checked                     = nullptr;
    bool        DescriptionAtBottomWhenOpen = false;
    float       AnimationSpeed              = 18.0f;
};

struct VerticalSplitterOptions
{
    float Height    = 0.0f;
    float HitWidth  = 8.0f;
    float LineWidth = 2.0f;
};

struct SnowflakeOptions
{
    int    Count     = 72;
    float  MinRadius = 1.0f;
    float  MaxRadius = 3.4f;
    float  MinSpeed  = 18.0f;
    float  MaxSpeed  = 62.0f;
    float  Wind      = 12.0f;
    float  Drift     = 18.0f;
    ImVec4 Color     = ImVec4(1.0f, 1.0f, 1.0f, 0.72f);
};

struct ColorTokens
{
    ImVec4 Background{};
    ImVec4 Surface{};
    ImVec4 SurfaceHover{};
    ImVec4 SurfaceActive{};
    ImVec4 Overlay{};
    ImVec4 Border{};
    ImVec4 BorderStrong{};
    ImVec4 Text{};
    ImVec4 TextMuted{};
    ImVec4 TextDisabled{};
    ImVec4 Accent{};
    ImVec4 AccentHover{};
    ImVec4 AccentActive{};
    ImVec4 AccentText{};
    ImVec4 Success{};
    ImVec4 Warning{};
    ImVec4 Danger{};
};

struct SizeTokens
{
    float  WindowRounding      = 8.0f;
    float  PanelRounding       = 8.0f;
    float  ControlRounding     = 6.0f;
    float  SmallRounding       = 4.0f;
    float  BorderWidth         = 1.0f;
    float  ControlHeight       = 30.0f;
    float  SmallControlHeight  = 24.0f;
    float  IconButtonSize      = 28.0f;
    float  InputMinWidth       = 180.0f;
    float  ItemGap             = 8.0f;
    float  SectionGap          = 10.0f;
    ImVec2 WindowPadding       = ImVec2(12.0f, 10.0f);
    ImVec2 PanelPadding        = ImVec2(10.0f, 8.0f);
    ImVec2 FramePadding        = ImVec2(9.0f, 5.0f);
    ImVec2 CompactFramePadding = ImVec2(7.0f, 4.0f);
};

struct Theme
{
    ColorTokens Colors{};
    SizeTokens  Sizes{};
};

struct IconButtonOptions
{
    Icon          IconValue  = Icon::None;
    const char*   Tooltip    = nullptr;
    ImVec2        Size       = ImVec2(0.0f, 0.0f);
    ButtonVariant Variant    = ButtonVariant::Ghost;
    bool          Active     = false;
    bool          Enabled    = true;
    bool          FixedWidth = true;
};

struct WindowOptions
{
    bool*            Open                       = nullptr;
    bool             UseRightSideCollapseButton = true;
    const char*      CollapseTooltip            = "折叠";
    const char*      RestoreTooltip             = "还原";
    ImGuiWindowFlags Flags                      = 0;
    bool             CountAsManagedWindow       = true;
};

struct PanelChildOptions
{
    ImVec2           Size   = ImVec2(0.0f, 0.0f);
    bool             Border = true;
    ImGuiWindowFlags Flags  = 0;
};

class StyleColorScope
{
  public:
    StyleColorScope() = default;
    StyleColorScope(ImGuiCol Index, const ImVec4& Color);
    ~StyleColorScope();

    StyleColorScope(const StyleColorScope&)            = delete;
    StyleColorScope& operator=(const StyleColorScope&) = delete;

    StyleColorScope(StyleColorScope&& Other) noexcept;
    StyleColorScope& operator=(StyleColorScope&& Other) noexcept;

    void Push(ImGuiCol Index, const ImVec4& Color);
    void Pop();

  private:
    int Count = 0;
};

class StyleVarScope
{
  public:
    StyleVarScope() = default;
    StyleVarScope(ImGuiStyleVar Index, float Value);
    StyleVarScope(ImGuiStyleVar Index, const ImVec2& Value);
    ~StyleVarScope();

    StyleVarScope(const StyleVarScope&)            = delete;
    StyleVarScope& operator=(const StyleVarScope&) = delete;

    StyleVarScope(StyleVarScope&& Other) noexcept;
    StyleVarScope& operator=(StyleVarScope&& Other) noexcept;

    void Push(ImGuiStyleVar Index, float Value);
    void Push(ImGuiStyleVar Index, const ImVec2& Value);
    void Pop();

  private:
    int Count = 0;
};

class DisabledScope
{
  public:
    explicit DisabledScope(bool Disabled);
    ~DisabledScope();

    DisabledScope(const DisabledScope&)            = delete;
    DisabledScope& operator=(const DisabledScope&) = delete;

  private:
    bool Active = false;
};

class Dx11GaussianBlur
{
  public:
    Dx11GaussianBlur() = default;
    ~Dx11GaussianBlur();

    bool Initialize(ID3D11Device* Device, ID3D11DeviceContext* Context);
    void Shutdown();
    bool Resize(int Width, int Height);
    bool Blur(ID3D11ShaderResourceView* SourceView, float Radius = 10.0f, int PassCount = 2);
    void DrawBlurredImage(const ImVec2& Size, const ImVec4& Tint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ID3D11ShaderResourceView* GetOutputView() const;
    int                       GetWidth() const;
    int                       GetHeight() const;

  private:
    bool CreateShaders();
    bool CreateSampler();
    bool CreateTarget(Dx11BlurTarget& Target, int Width, int Height);
    void DestroyTarget(Dx11BlurTarget& Target);
    void DrawPass(ID3D11ShaderResourceView* SourceView,
                  ID3D11RenderTargetView*   TargetView,
                  float                     DirectionX,
                  float                     DirectionY,
                  float                     Radius);

    ID3D11Device*        D3dDevice      = nullptr;
    ID3D11DeviceContext* D3dContext     = nullptr;
    ID3D11VertexShader*  VertexShader   = nullptr;
    ID3D11PixelShader*   PixelShader    = nullptr;
    ID3D11Buffer*        ConstantBuffer = nullptr;
    ID3D11SamplerState*  SamplerState   = nullptr;
    ID3D11BlendState*    BlendState     = nullptr;
    Dx11BlurTarget       PingTarget{};
    Dx11BlurTarget       PongTarget{};
};

float EaseValue(Ease Mode, float T);
float GetFrameDeltaSeconds();
float SmoothValue(ImGuiID Id, float Target, float Speed = 18.0f, float InitialValue = 0.0f);
float SmoothValue(const char* Id, float Target, float Speed = 18.0f, float InitialValue = 0.0f);
float AnimateBool(ImGuiID Id, bool Active, float Speed = 18.0f);
float AnimateBool(const char* Id, bool Active, float Speed = 18.0f);
void  ClearAnimationState(ImGuiID Id);
void  ClearAllAnimationStates();

const char* ToIconString(Icon Value);

bool           LoadFonts(float TextSize = 16.0f, float IconSize = 15.0f);
const FontSet& Fonts();

bool CreateDx11TextureFromMemory(ID3D11Device* Device, const void* Data, size_t Size, ImageTexture& Texture);
bool CreateDx11TextureFromFile(ID3D11Device* Device, const wchar_t* Path, ImageTexture& Texture);
void DestroyDx11Texture(ImageTexture& Texture);
void Image(const ImageTexture& Texture,
           const ImVec2&       Size,
           const ImVec4&       Tint   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
           const ImVec4&       Border = ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

void   AddVerticalSpace(float Height);
void   SameLineRight(float ItemWidth);
ImVec2 CalcButtonSize(const char* Label, float MinWidth = 0.0f);
ImVec2 CalcWindowContentSize(float PreferredWidth, float PreferredHeight);
float  GetVerticalSplitterHitWidth();
bool   DrawVerticalSplitter(const char* Id, float Height);
bool   DrawVerticalSplitter(const char* Id, const VerticalSplitterOptions& Options = {});

bool BeginPanel(const char* Id, const PanelOptions& Options = {});
void EndPanel();

bool IsPanelContentVisible();
bool BeginModule(const char* Id, const char* Name, const ModuleHeaderOptions& Options = {});
void EndModule();

void BeginIndented(float Width = 12.0f);
void EndIndented(float Width = 12.0f);

void DrawSnowflakes(const char* Id, const ImVec2& Min, const ImVec2& Max, const SnowflakeOptions& Options = {});

Theme MakeTheme();

void SetTheme(const Theme& NewTheme);
void ApplyTheme(const Theme& NewTheme);

const Theme&       GetTheme();
const ColorTokens& Colors();
const SizeTokens&  Sizes();

ImVec4 WithAlpha(ImVec4 Color, float Alpha);
ImVec4 WithMultipliedAlpha(ImVec4 Color, float AlphaScale);
ImVec4 Blend(ImVec4 A, ImVec4 B, float Amount);
ImU32  ToU32(const ImVec4& Color);

void Text(const char* Value);
void TextMuted(const char* Value);
void TextDisabled(const char* Value);
void Heading(const char* Value);
bool Hyperlink(const char* Label, const char* Url = nullptr);
void StatusText(const char* Value, StatusKind Kind = StatusKind::Neutral);
void Badge(const char* Label, BadgeVariant Variant = BadgeVariant::Neutral);
void BadgeDot(BadgeVariant Variant = BadgeVariant::Danger, float Radius = 0.0f);
void BadgeNumber(int Value, BadgeVariant Variant = BadgeVariant::Danger);
bool InfoBar(const char*  Message,
             InfoSeverity Severity    = InfoSeverity::Info,
             bool*        Open        = nullptr,
             const char*  ActionLabel = nullptr);
void HelpMarker(const char* Text);

bool Button(const char*   Label,
            const ImVec2& Size    = ImVec2(0.0f, 0.0f),
            ButtonVariant Variant = ButtonVariant::Secondary);
bool PrimaryButton(const char* Label, const ImVec2& Size = ImVec2(0.0f, 0.0f));
bool GhostButton(const char* Label, const ImVec2& Size = ImVec2(0.0f, 0.0f));
bool DangerButton(const char* Label, const ImVec2& Size = ImVec2(0.0f, 0.0f));
bool IconButton(const char*   Id,
                const char*   Icon,
                const char*   Tooltip = nullptr,
                const ImVec2& Size    = ImVec2(0.0f, 0.0f));
bool IconButton(const char* Id, Icon IconValue, const char* Tooltip = nullptr, const ImVec2& Size = ImVec2(0.0f, 0.0f));
bool IconActionButton(const char* Id, const IconButtonOptions& Options);
bool SettingsButton(const char* Id = "Settings", bool Active = false);
bool HelpButton(const char* Id, const char* Tooltip);
bool DiscordButton(const char* Id = "Discord");
bool IconButtonWithText(const char*   Id,
                        Icon          IconValue,
                        const char*   Label,
                        const ImVec2& Size    = ImVec2(0.0f, 0.0f),
                        ButtonVariant Variant = ButtonVariant::Secondary);

bool Checkbox(const char* Label, bool* Value);
bool Toggle(const char* Label, bool* Value);
bool SliderFloat(const char* Label, float* Value, float Min, float Max, const char* Format = "%.2f");
bool SliderInt(const char* Label, int* Value, int Min, int Max);
bool SliderIntMapped(
    const char* Label, int* Value, int Min, int Max, int DisplayMultiplier, const char* DisplayFormat = "%d");
bool Combo(const char* Label, int* CurrentItem, const char* const Items[], int ItemCount);
bool SegmentedControl(const char*       Id,
                      int*              CurrentItem,
                      const char* const Items[],
                      int               ItemCount,
                      const ImVec2&     Size = ImVec2(0.0f, 0.0f));
bool InputText(
    const char* Label, char* Buffer, size_t BufferSize, const char* Hint = nullptr, ImGuiInputTextFlags Flags = 0);
bool InputInt(const char* Label, int* Value);
bool InputFloat(const char* Label, float* Value, const char* Format = "%.3f");
void ProgressBar(const char*   Id,
                 float         Fraction,
                 const ImVec2& Size    = ImVec2(0.0f, 0.0f),
                 const char*   Overlay = nullptr);
void IndeterminateProgressBar(const char* Id, const ImVec2& Size = ImVec2(0.0f, 0.0f));
void ProgressRing(const char* Id, float Fraction, float Radius = 0.0f, float Thickness = 0.0f);
void Spinner(const char* Id, float Radius = 0.0f, float Thickness = 0.0f);
bool BeginTable(const char* Id, int ColumnCount, ImGuiTableFlags Flags = 0, const ImVec2& OuterSize = ImVec2(0.0f, 0.0f));
void TableHeadersRow(const char* const Headers[], int HeaderCount);
void EndTable();

void LabelValue(const char* Label, const char* Value);
void LabelValue(const char* Label, int Value);
void LabelValue(const char* Label, float Value, const char* Format = "%.2f");

bool BeginWindow(const char* Name, const WindowOptions& Options = {});
void EndWindow();
void BeginManagedWindowFrame();
bool HasVisibleManagedWindows();
void DrawTitleBarCollapseButton(const char* Id              = "RnTitleBarCollapseButton",
                                const char* CollapseTooltip = "折叠",
                                const char* RestoreTooltip  = "还原");
bool BeginPanelChild(const char* Id, const PanelChildOptions& Options = {});
void EndPanelChild();
bool IsPanelChildContentVisible();

} // namespace RorinnnTools::ImguiRorinnn
