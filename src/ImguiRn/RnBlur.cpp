// RnBlur.cpp — ImguiRn DX11 高斯模糊工具

module;

#include <d3d11.h>
#include <d3dcompiler.h>
#include <imgui.h>

module RnTools;
import std;

namespace RnTools::ImguiRn
{
namespace
{
struct BlurConstants
{
    float TexelSize[2];
    float Direction[2];
    float Radius;
    float Padding[3];
};

static constexpr char BlurShaderSource[] = R"(
cbuffer BlurConstants : register(b0)
{
    float2 TexelSize;
    float2 Direction;
    float Radius;
    float3 Padding;
};

Texture2D SourceTexture : register(t0);
SamplerState SourceSampler : register(s0);

struct VsOut
{
    float4 Position : SV_POSITION;
    float2 Uv : TEXCOORD0;
};

VsOut VsMain(uint VertexId : SV_VertexID)
{
    float2 Pos[3] =
    {
        float2(-1.0, -1.0),
        float2(-1.0,  3.0),
        float2( 3.0, -1.0)
    };
    float2 Uv[3] =
    {
        float2(0.0, 1.0),
        float2(0.0, -1.0),
        float2(2.0, 1.0)
    };

    VsOut Output;
    Output.Position = float4(Pos[VertexId], 0.0, 1.0);
    Output.Uv = Uv[VertexId];
    return Output;
}

float4 PsMain(VsOut Input) : SV_TARGET
{
    float2 Step = Direction * TexelSize * max(Radius, 0.01);
    float4 Color = SourceTexture.Sample(SourceSampler, Input.Uv) * 0.2270270270;
    Color += SourceTexture.Sample(SourceSampler, Input.Uv + Step * 1.3846153846) * 0.3162162162;
    Color += SourceTexture.Sample(SourceSampler, Input.Uv - Step * 1.3846153846) * 0.3162162162;
    Color += SourceTexture.Sample(SourceSampler, Input.Uv + Step * 3.2307692308) * 0.0702702703;
    Color += SourceTexture.Sample(SourceSampler, Input.Uv - Step * 3.2307692308) * 0.0702702703;
    return Color;
}
)";

template <typename T>
static void ReleaseObject(T*& Object)
{
    if (Object)
    {
        Object->Release();
        Object = nullptr;
    }
}
} // namespace

Dx11GaussianBlur::~Dx11GaussianBlur()
{
    Shutdown();
}

bool Dx11GaussianBlur::Initialize(ID3D11Device* Device, ID3D11DeviceContext* Context)
{
    Shutdown();
    if (!Device || !Context)
        return false;

    D3dDevice  = Device;
    D3dContext = Context;
    D3dDevice->AddRef();
    D3dContext->AddRef();

    if (!CreateShaders() || !CreateSampler())
    {
        Shutdown();
        return false;
    }
    return true;
}

void Dx11GaussianBlur::Shutdown()
{
    DestroyTarget(PingTarget);
    DestroyTarget(PongTarget);
    ReleaseObject(BlendState);
    ReleaseObject(SamplerState);
    ReleaseObject(ConstantBuffer);
    ReleaseObject(PixelShader);
    ReleaseObject(VertexShader);
    ReleaseObject(D3dContext);
    ReleaseObject(D3dDevice);
}

bool Dx11GaussianBlur::Resize(int Width, int Height)
{
    Width  = std::max(1, Width);
    Height = std::max(1, Height);
    if (PingTarget.Width == Width && PingTarget.Height == Height && PongTarget.Width == Width &&
        PongTarget.Height == Height)
        return true;

    DestroyTarget(PingTarget);
    DestroyTarget(PongTarget);
    return CreateTarget(PingTarget, Width, Height) && CreateTarget(PongTarget, Width, Height);
}

bool Dx11GaussianBlur::Blur(ID3D11ShaderResourceView* SourceView, float Radius, int PassCount)
{
    if (!D3dContext || !SourceView || !PingTarget.RenderTargetView || !PongTarget.RenderTargetView)
        return false;

    PassCount                               = std::max(1, PassCount);
    ID3D11ShaderResourceView* CurrentSource = SourceView;
    for (int PassIndex = 0; PassIndex < PassCount; PassIndex++)
    {
        DrawPass(CurrentSource, PingTarget.RenderTargetView, 1.0f, 0.0f, Radius);
        DrawPass(PingTarget.ShaderResourceView, PongTarget.RenderTargetView, 0.0f, 1.0f, Radius);
        CurrentSource = PongTarget.ShaderResourceView;
    }

    ID3D11ShaderResourceView* NullView = nullptr;
    D3dContext->PSSetShaderResources(0, 1, &NullView);
    return true;
}

void Dx11GaussianBlur::DrawBlurredImage(const ImVec2& Size, const ImVec4& Tint)
{
    if (!PongTarget.ShaderResourceView)
    {
        ImGui::Dummy(Size);
        return;
    }
    ImGui::Image((ImTextureID)PongTarget.ShaderResourceView,
                 Size,
                 ImVec2(0.0f, 0.0f),
                 ImVec2(1.0f, 1.0f),
                 Tint,
                 ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
}

ID3D11ShaderResourceView* Dx11GaussianBlur::GetOutputView() const
{
    return PongTarget.ShaderResourceView;
}

int Dx11GaussianBlur::GetWidth() const
{
    return PongTarget.Width;
}

int Dx11GaussianBlur::GetHeight() const
{
    return PongTarget.Height;
}

bool Dx11GaussianBlur::CreateShaders()
{
    ID3DBlob* VertexBlob = nullptr;
    ID3DBlob* PixelBlob  = nullptr;
    ID3DBlob* ErrorBlob  = nullptr;

    HRESULT Result = D3DCompile(BlurShaderSource,
                                sizeof(BlurShaderSource) - 1,
                                nullptr,
                                nullptr,
                                nullptr,
                                "VsMain",
                                "vs_4_0",
                                0,
                                0,
                                &VertexBlob,
                                &ErrorBlob);
    ReleaseObject(ErrorBlob);
    if (FAILED(Result))
        return false;

    Result = D3DCompile(BlurShaderSource,
                        sizeof(BlurShaderSource) - 1,
                        nullptr,
                        nullptr,
                        nullptr,
                        "PsMain",
                        "ps_4_0",
                        0,
                        0,
                        &PixelBlob,
                        &ErrorBlob);
    ReleaseObject(ErrorBlob);
    if (FAILED(Result))
    {
        ReleaseObject(VertexBlob);
        return false;
    }

    Result = D3dDevice->CreateVertexShader(
        VertexBlob->GetBufferPointer(), VertexBlob->GetBufferSize(), nullptr, &VertexShader);
    if (SUCCEEDED(Result))
        Result = D3dDevice->CreatePixelShader(
            PixelBlob->GetBufferPointer(), PixelBlob->GetBufferSize(), nullptr, &PixelShader);
    ReleaseObject(VertexBlob);
    ReleaseObject(PixelBlob);
    if (FAILED(Result))
        return false;

    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.ByteWidth         = sizeof(BlurConstants);
    BufferDesc.Usage             = D3D11_USAGE_DYNAMIC;
    BufferDesc.BindFlags         = D3D11_BIND_CONSTANT_BUFFER;
    BufferDesc.CPUAccessFlags    = D3D11_CPU_ACCESS_WRITE;
    return SUCCEEDED(D3dDevice->CreateBuffer(&BufferDesc, nullptr, &ConstantBuffer));
}

bool Dx11GaussianBlur::CreateSampler()
{
    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter             = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.AddressU           = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV           = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW           = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.ComparisonFunc     = D3D11_COMPARISON_ALWAYS;
    SamplerDesc.MinLOD             = 0.0f;
    SamplerDesc.MaxLOD             = D3D11_FLOAT32_MAX;
    if (FAILED(D3dDevice->CreateSamplerState(&SamplerDesc, &SamplerState)))
        return false;

    D3D11_BLEND_DESC BlendDesc                      = {};
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    return SUCCEEDED(D3dDevice->CreateBlendState(&BlendDesc, &BlendState));
}

bool Dx11GaussianBlur::CreateTarget(Dx11BlurTarget& Target, int Width, int Height)
{
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width                = (UINT)Width;
    TextureDesc.Height               = (UINT)Height;
    TextureDesc.MipLevels            = 1;
    TextureDesc.ArraySize            = 1;
    TextureDesc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
    TextureDesc.SampleDesc.Count     = 1;
    TextureDesc.Usage                = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    if (FAILED(D3dDevice->CreateTexture2D(&TextureDesc, nullptr, &Target.Texture)))
        return false;
    if (FAILED(D3dDevice->CreateShaderResourceView(Target.Texture, nullptr, &Target.ShaderResourceView)) ||
        FAILED(D3dDevice->CreateRenderTargetView(Target.Texture, nullptr, &Target.RenderTargetView)))
    {
        DestroyTarget(Target);
        return false;
    }

    Target.Width  = Width;
    Target.Height = Height;
    return true;
}

void Dx11GaussianBlur::DestroyTarget(Dx11BlurTarget& Target)
{
    ReleaseObject(Target.RenderTargetView);
    ReleaseObject(Target.ShaderResourceView);
    ReleaseObject(Target.Texture);
    Target.Width  = 0;
    Target.Height = 0;
}

void Dx11GaussianBlur::DrawPass(ID3D11ShaderResourceView* SourceView,
                                ID3D11RenderTargetView*   TargetView,
                                float                     DirectionX,
                                float                     DirectionY,
                                float                     Radius)
{
    D3D11_VIEWPORT Viewport = {};
    Viewport.Width          = (float)PingTarget.Width;
    Viewport.Height         = (float)PingTarget.Height;
    Viewport.MinDepth       = 0.0f;
    Viewport.MaxDepth       = 1.0f;

    BlurConstants Constants = {};
    Constants.TexelSize[0]  = 1.0f / std::max(1, PingTarget.Width);
    Constants.TexelSize[1]  = 1.0f / std::max(1, PingTarget.Height);
    Constants.Direction[0]  = DirectionX;
    Constants.Direction[1]  = DirectionY;
    Constants.Radius        = std::max(0.01f, Radius);

    D3D11_MAPPED_SUBRESOURCE Mapped = {};
    if (SUCCEEDED(D3dContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
    {
        *(BlurConstants*)Mapped.pData = Constants;
        D3dContext->Unmap(ConstantBuffer, 0);
    }

    const float ClearColor[4] = {};
    D3dContext->ClearRenderTargetView(TargetView, ClearColor);
    D3dContext->OMSetRenderTargets(1, &TargetView, nullptr);
    D3dContext->OMSetBlendState(BlendState, nullptr, 0xFFFFFFFF);
    D3dContext->RSSetViewports(1, &Viewport);
    D3dContext->IASetInputLayout(nullptr);
    D3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3dContext->VSSetShader(VertexShader, nullptr, 0);
    D3dContext->PSSetShader(PixelShader, nullptr, 0);
    D3dContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);
    D3dContext->PSSetSamplers(0, 1, &SamplerState);
    D3dContext->PSSetShaderResources(0, 1, &SourceView);
    D3dContext->Draw(3, 0);
}

} // namespace RnTools::ImguiRn
