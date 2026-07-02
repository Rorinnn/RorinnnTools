// RnImage.cpp — ImguiRn DX11 图片资源工具

module;

#include <d3d11.h>
#include <imgui.h>
#include <wincodec.h>
#include <wrl/client.h>

module RnTools;
import std;

namespace RnTools::ImguiRn
{
namespace
{
using Microsoft::WRL::ComPtr;

static bool EnsureComInitialized()
{
    HRESULT Result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    return SUCCEEDED(Result) || Result == RPC_E_CHANGED_MODE;
}

static bool CreateWicFactory(ComPtr<IWICImagingFactory>& Factory)
{
    if (!EnsureComInitialized())
        return false;

    return SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&Factory)));
}

static bool CreateTextureFromBitmap(ID3D11Device* Device, IWICBitmapSource* BitmapSource, ImageTexture& Texture)
{
    if (!Device || !BitmapSource)
        return false;

    UINT Width  = 0;
    UINT Height = 0;
    if (FAILED(BitmapSource->GetSize(&Width, &Height)) || Width == 0 || Height == 0)
        return false;
    if (Width > static_cast<UINT>((std::numeric_limits<int>::max)()) ||
        Height > static_cast<UINT>((std::numeric_limits<int>::max)()) || Width > (std::numeric_limits<UINT>::max)() / 4)
        return false;

    const UINT Stride = Width * 4;
    if (Height > (std::numeric_limits<UINT>::max)() / Stride)
        return false;

    const UINT                BufferSize = Stride * Height;
    std::vector<std::uint8_t> Pixels(BufferSize);
    if (FAILED(BitmapSource->CopyPixels(nullptr, Stride, BufferSize, Pixels.data())))
        return false;

    D3D11_TEXTURE2D_DESC Desc = {};
    Desc.Width                = Width;
    Desc.Height               = Height;
    Desc.MipLevels            = 1;
    Desc.ArraySize            = 1;
    Desc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
    Desc.SampleDesc.Count     = 1;
    Desc.Usage                = D3D11_USAGE_DEFAULT;
    Desc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA InitialData = {};
    InitialData.pSysMem                = Pixels.data();
    InitialData.SysMemPitch            = Stride;

    ComPtr<ID3D11Texture2D> Texture2D;
    HRESULT                 Result = Device->CreateTexture2D(&Desc, &InitialData, &Texture2D);
    if (FAILED(Result))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = {};
    ViewDesc.Format                          = Desc.Format;
    ViewDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
    ViewDesc.Texture2D.MipLevels             = 1;

    ID3D11ShaderResourceView* View = nullptr;
    if (FAILED(Device->CreateShaderResourceView(Texture2D.Get(), &ViewDesc, &View)))
        return false;

    DestroyDx11Texture(Texture);
    Texture.View   = View;
    Texture.Width  = (int)Width;
    Texture.Height = (int)Height;
    return true;
}

static bool CreateTextureFromDecoder(ID3D11Device* Device, IWICBitmapDecoder* Decoder, ImageTexture& Texture)
{
    ComPtr<IWICImagingFactory> Factory;
    if (!CreateWicFactory(Factory) || !Decoder)
        return false;

    ComPtr<IWICBitmapFrameDecode> Frame;
    if (FAILED(Decoder->GetFrame(0, &Frame)))
        return false;

    ComPtr<IWICFormatConverter> Converter;
    if (FAILED(Factory->CreateFormatConverter(&Converter)))
        return false;

    if (FAILED(Converter->Initialize(Frame.Get(),
                                     GUID_WICPixelFormat32bppRGBA,
                                     WICBitmapDitherTypeNone,
                                     nullptr,
                                     0.0,
                                     WICBitmapPaletteTypeCustom)))
        return false;

    return CreateTextureFromBitmap(Device, Converter.Get(), Texture);
}
} // namespace

bool CreateDx11TextureFromMemory(ID3D11Device* Device, const void* Data, std::size_t Size, ImageTexture& Texture)
{
    if (!Device || !Data || Size == 0 || Size > std::numeric_limits<std::uint32_t>::max())
        return false;

    ComPtr<IWICImagingFactory> Factory;
    if (!CreateWicFactory(Factory))
        return false;

    ComPtr<IWICStream> Stream;
    if (FAILED(Factory->CreateStream(&Stream)))
        return false;

    if (FAILED(Stream->InitializeFromMemory((WICInProcPointer)Data, (DWORD)Size)))
        return false;

    ComPtr<IWICBitmapDecoder> Decoder;
    if (FAILED(Factory->CreateDecoderFromStream(Stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &Decoder)))
        return false;

    return CreateTextureFromDecoder(Device, Decoder.Get(), Texture);
}

bool CreateDx11TextureFromFile(ID3D11Device* Device, const wchar_t* Path, ImageTexture& Texture)
{
    if (!Device || !Path || !Path[0])
        return false;

    ComPtr<IWICImagingFactory> Factory;
    if (!CreateWicFactory(Factory))
        return false;

    ComPtr<IWICBitmapDecoder> Decoder;
    if (FAILED(Factory->CreateDecoderFromFilename(Path, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &Decoder)))
        return false;

    return CreateTextureFromDecoder(Device, Decoder.Get(), Texture);
}

void DestroyDx11Texture(ImageTexture& Texture)
{
    if (Texture.View)
        Texture.View->Release();
    Texture = {};
}

ImVec2 FitImageSize(int ImageWidth, int ImageHeight, const ImVec2& Bounds)
{
    if (ImageWidth <= 0 || ImageHeight <= 0)
        return ImVec2(1.0f, 1.0f);

    const float MaxWidth  = (std::max)(1.0f, Bounds.x);
    const float MaxHeight = (std::max)(1.0f, Bounds.y);
    const float Scale     = (std::min)(MaxWidth / static_cast<float>(ImageWidth),
                                   MaxHeight / static_cast<float>(ImageHeight));
    return ImVec2((std::max)(1.0f, static_cast<float>(ImageWidth) * Scale),
                  (std::max)(1.0f, static_cast<float>(ImageHeight) * Scale));
}

void Image(const ImageTexture& Texture, const ImVec2& Size, const ImVec4& Tint, const ImVec4& Border)
{
    if (!Texture.View)
    {
        ImGui::Dummy(Size);
        return;
    }

    ImGui::Image((ImTextureID)Texture.View, Size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), Tint, Border);
}

} // namespace RnTools::ImguiRn
