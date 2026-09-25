#include "DirectXBackend.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <vector>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

struct Vertex {
    float x, y, z;
    float u, v;
    float r, g, b, a;
};

struct ConstantBufferData {
    XMFLOAT4X4 projection;
};

struct DXTexture {
    ID3D11Texture2D* texture = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;
    int width = 0;
    int height = 0;
};

struct DXState {
    HWND hwnd = nullptr;
    int width = 0;
    int height = 0;

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;

    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11PixelShader* pixelShader = nullptr;
    ID3D11PixelShader* texturePixelShader = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;

    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;
    ID3D11Buffer* constantBuffer = nullptr;
    ID3D11SamplerState* samplerState = nullptr;
    ID3D11RasterizerState* rasterizerState = nullptr;

    ID3D11BlendState* blendNone = nullptr;
    ID3D11BlendState* blendSrcOver = nullptr;
    ID3D11BlendState* blendAdditive = nullptr;
    ID3D11BlendState* blendMultiply = nullptr;

    int currentVertexBufferSize = 0;
    int currentIndexBufferSize = 0;
    DXTexture* defaultWhiteTexture = nullptr;
};

static const char* g_shaderCode = R"(
cbuffer MatrixBuffer : register(b0) {
    row_major matrix projection;
};

struct VS_INPUT {
    float3 pos : POSITION;
    float2 tex : TEXCOORD;
    float4 col : COLOR;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
    float4 col : COLOR;
};

PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), projection);
    output.tex = input.tex;
    output.col = input.col;
    return output;
}

float4 PSColor(PS_INPUT input) : SV_TARGET {
    return input.col;
}

Texture2D tex2D : register(t0);
SamplerState samplerState : register(s0);

// Helper: 4x4 = 16 Subpixel Super-Sampling Coverage calculation for Circles
float calculateCircleCoverage(float2 p, float2 dPdx, float2 dPdy, float maxR) {
    float cov = 0.0f;
    [unroll]
    for (int j = 0; j < 4; j++) {
        float sy = (float(j) + 0.5f) * 0.25f - 0.5f;
        [unroll]
        for (int i = 0; i < 4; i++) {
            float sx = (float(i) + 0.5f) * 0.25f - 0.5f;
            float2 subP = p + sx * dPdx + sy * dPdy;
            if (length(subP) <= maxR) {
                cov += 0.0625f;
            }
        }
    }
    return cov;
}

// 2D RoundRect SDF
float sdRoundRect(float2 p, float2 b, float r) {
    float2 q = abs(p) - (b - float2(r, r));
    return min(max(q.x, q.y), 0.0f) + length(max(q, 0.0f)) - r;
}

float calculateRoundRectCoverage(float2 p, float2 dPdx, float2 dPdy, float2 b, float r) {
    float cov = 0.0f;
    [unroll]
    for (int j = 0; j < 4; j++) {
        float sy = (float(j) + 0.5f) * 0.25f - 0.5f;
        [unroll]
        for (int i = 0; i < 4; i++) {
            float sx = (float(i) + 0.5f) * 0.25f - 0.5f;
            float2 subP = p + sx * dPdx + sy * dPdy;
            if (sdRoundRect(subP, b, r) <= 0.0f) {
                cov += 0.0625f;
            }
        }
    }
    return cov;
}

float calculateRoundRectStrokeCoverage(float2 p, float2 dPdx, float2 dPdy, float2 b, float r, float strokePx) {
    float covOuter = calculateRoundRectCoverage(p, dPdx, dPdy, b, r);
    float2 bInner = b - float2(strokePx, strokePx);
    float rInner = max(0.0f, r - strokePx);
    float covInner = calculateRoundRectCoverage(p, dPdx, dPdy, bInner, rInner);
    return clamp(covOuter - covInner, 0.0f, 1.0f);
}

float calculateStrokeCoverage(float2 p, float2 dPdx, float2 dPdy, float targetR, float halfStroke) {
    float cov = 0.0f;
    [unroll]
    for (int j = 0; j < 4; j++) {
        float sy = (float(j) + 0.5f) * 0.25f - 0.5f;
        [unroll]
        for (int i = 0; i < 4; i++) {
            float sx = (float(i) + 0.5f) * 0.25f - 0.5f;
            float2 subP = p + sx * dPdx + sy * dPdy;
            float r = length(subP);
            if (abs(r - targetR) <= halfStroke) {
                cov += 0.0625f;
            }
        }
    }
    return cov;
}

float4 PSTexture(PS_INPUT input) : SV_TARGET {
    float u = input.tex.x;
    float v = input.tex.y;

    // Mode -100: Solid Rectangle
    if (u < -80.0f) {
        return input.col;
    }

    // Mode -70: GPU Parametric Bézier Curve (Loop-Blinn)
    if (u < -65.0f) {
        float2 p = float2(u + 70.0f, v);
        float f = p.x * p.x - p.y;
        float2 grad = float2(ddx(f), ddy(f));
        float gLen = length(grad);
        if (gLen > 0.0f) {
            float dist = f / gLen;
            float alpha = clamp(0.5f - dist, 0.0f, 1.0f);
            if (alpha <= 0.0f) discard;
            return float4(input.col.rgb, input.col.a * alpha);
        } else {
            if (f > 0.0f) discard;
            return input.col;
        }
    }

    // Mode -60: Oval Fill WITHOUT AA (u in [-61, -59])
    if (u < -55.0f) {
        float2 p = float2(u + 60.0f, v);
        if (length(p) > 1.0f) discard;
        return input.col;
    }

    // Mode -50: Oval Outline WITHOUT AA (u in [-51, -49])
    if (u < -45.0f) {
        float2 p = float2(u + 50.0f, v);
        float2 dPdx = ddx(p);
        float2 dPdy = ddy(p);
        float pxSize = max(length(dPdx), length(dPdy));
        if (abs(length(p) - 1.0f) > 0.5f * pxSize) discard;
        return input.col;
    }

    // Mode -30: Oval Fill WITH 16x Subpixel AA (u in [-31, -29])
    if (u < -25.0f) {
        float2 p = float2(u + 30.0f, v);
        float2 dPdx = ddx(p);
        float2 dPdy = ddy(p);
        float alpha = calculateCircleCoverage(p, dPdx, dPdy, 1.0f);
        if (alpha <= 0.0f) discard;
        return float4(input.col.rgb, input.col.a * alpha);
    }

    // Mode -20: RoundRectangle Fill WITH 16x Subpixel AA (u in [-21, -19])
    if (u < -15.0f) {
        float2 p = float2(u + 20.0f, v);
        float2 dPdx = ddx(p);
        float2 dPdy = ddy(p);
        float alpha = calculateRoundRectCoverage(p, dPdx, dPdy, float2(1.0f, 1.0f), 0.35f);
        if (alpha <= 0.0f) discard;
        return float4(input.col.rgb, input.col.a * alpha);
    }

    // Mode -10: RoundRectangle Outline WITH 16x Subpixel AA (u in [-11, -9])
    if (u < -5.0f) {
        float2 p = float2(u + 10.0f, v);
        float2 dPdx = ddx(p);
        float2 dPdy = ddy(p);
        float pxSize = max(length(dPdx), length(dPdy));
        float alpha = calculateRoundRectStrokeCoverage(p, dPdx, dPdy, float2(1.0f, 1.0f), 0.368f, pxSize);
        if (alpha <= 0.0f) discard;
        return float4(input.col.rgb, input.col.a * alpha);
    }

    // Mode -2: Oval Outline WITH 16x Subpixel AA (u in [-3, -1])
    if (u < -0.5f) {
        float2 p = float2(u + 2.0f, v);
        float2 dPdx = ddx(p);
        float2 dPdy = ddy(p);
        float pxSize = max(length(dPdx), length(dPdy));
        float halfStroke = 0.5f * pxSize;
        float alpha = calculateStrokeCoverage(p, dPdx, dPdy, 1.0f, halfStroke);
        if (alpha <= 0.0f) discard;
        return float4(input.col.rgb, input.col.a * alpha);
    }

    // Standard Texture Mode: u >= 0.0
    return tex2D.Sample(samplerState, input.tex) * input.col;
}
)";

static void CleanupRenderTarget(DXState* state) {
    if (state->rtv) {
        state->rtv->Release();
        state->rtv = nullptr;
    }
}

static void CreateRenderTarget(DXState* state) {
    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT hr = state->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (SUCCEEDED(hr) && backBuffer) {
        state->device->CreateRenderTargetView(backBuffer, nullptr, &state->rtv);
        backBuffer->Release();
    }
}

extern "C" {

FASTDX_API int64_t fastdx_create(int64_t hwnd, int32_t w, int32_t h) {
    DXState* state = new DXState();
    state->hwnd = (HWND)hwnd;
    state->width = w;
    state->height = h;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Width = w;
    scd.BufferDesc.Height = h;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = state->hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featureLevels, 2, D3D11_SDK_VERSION, &scd,
        &state->swapChain, &state->device, &featureLevel, &state->context
    );

    if (FAILED(hr)) {
        delete state;
        return 0;
    }

    CreateRenderTarget(state);

    // Compile Shaders
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psColorBlob = nullptr;
    ID3DBlob* psTexBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hrVS = D3DCompile(g_shaderCode, strlen(g_shaderCode), nullptr, nullptr, nullptr, "VSMain", "vs_4_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hrVS) && errorBlob) {
        fprintf(stderr, "[FastDirectX] Vertex Shader Compile Error: %s\n", (char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
        errorBlob = nullptr;
    }
    HRESULT hrPSCol = D3DCompile(g_shaderCode, strlen(g_shaderCode), nullptr, nullptr, nullptr, "PSColor", "ps_4_0", 0, 0, &psColorBlob, &errorBlob);
    if (FAILED(hrPSCol) && errorBlob) {
        fprintf(stderr, "[FastDirectX] PSColor Shader Compile Error: %s\n", (char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
        errorBlob = nullptr;
    }
    HRESULT hrPSTex = D3DCompile(g_shaderCode, strlen(g_shaderCode), nullptr, nullptr, nullptr, "PSTexture", "ps_4_0", 0, 0, &psTexBlob, &errorBlob);
    if (FAILED(hrPSTex) && errorBlob) {
        fprintf(stderr, "[FastDirectX] PSTexture Shader Compile Error: %s\n", (char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
        errorBlob = nullptr;
    }

    if (vsBlob) {
        state->device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &state->vertexShader);
    }
    if (psColorBlob) {
        state->device->CreatePixelShader(psColorBlob->GetBufferPointer(), psColorBlob->GetBufferSize(), nullptr, &state->pixelShader);
    }
    if (psTexBlob) {
        state->device->CreatePixelShader(psTexBlob->GetBufferPointer(), psTexBlob->GetBufferSize(), nullptr, &state->texturePixelShader);
    }

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(Vertex, u),          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, r),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (vsBlob) {
        state->device->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &state->inputLayout);
        vsBlob->Release();
    }
    if (psColorBlob) psColorBlob->Release();
    if (psTexBlob) psTexBlob->Release();

    // Constant buffer
    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(ConstantBufferData);
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    state->device->CreateBuffer(&cbd, nullptr, &state->constantBuffer);

    // Sampler state
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    state->device->CreateSamplerState(&sampDesc, &state->samplerState);

    // Rasterizer state (disable culling for 2D quad batches)
    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = FALSE;
    rastDesc.DepthClipEnable = FALSE;
    state->device->CreateRasterizerState(&rastDesc, &state->rasterizerState);

    // Blend States
    D3D11_BLEND_DESC bdesc = {};
    bdesc.RenderTarget[0].BlendEnable = TRUE;
    bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bdesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bdesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bdesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bdesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bdesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    state->device->CreateBlendState(&bdesc, &state->blendSrcOver);

    bdesc.RenderTarget[0].BlendEnable = FALSE;
    state->device->CreateBlendState(&bdesc, &state->blendNone);

    uint32_t whitePixel = 0xFFFFFFFF;
    state->defaultWhiteTexture = (DXTexture*)fastdx_create_texture((int64_t)state, 1, 1, &whitePixel);

    return (int64_t)state;
}

FASTDX_API void fastdx_resize(int64_t handle, int32_t w, int32_t h) {
    DXState* state = (DXState*)handle;
    if (!state || !state->swapChain || w <= 0 || h <= 0) return;

    if (state->context) {
        ID3D11RenderTargetView* nullRtv[1] = { nullptr };
        state->context->OMSetRenderTargets(1, nullRtv, nullptr);
        state->context->Flush();
    }
    CleanupRenderTarget(state);
    HRESULT hr = state->swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(hr)) {
        CreateRenderTarget(state);
        state->width = w;
        state->height = h;
        if (state->context) {
            state->context->Flush();
        }
    }
}

FASTDX_API void fastdx_begin_frame(int64_t handle) {
    DXState* state = (DXState*)handle;
    if (!state || !state->context) return;
    state->context->OMSetRenderTargets(1, &state->rtv, nullptr);
}

FASTDX_API void fastdx_clear(int64_t handle, float r, float g, float b, float a) {
    DXState* state = (DXState*)handle;
    if (!state || !state->rtv || !state->context) return;
    float color[4] = { r, g, b, a };
    state->context->ClearRenderTargetView(state->rtv, color);
}

FASTDX_API void fastdx_set_viewport(int64_t handle, int32_t x, int32_t y, int32_t w, int32_t h) {
    DXState* state = (DXState*)handle;
    if (!state || !state->context) return;
    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = (float)x;
    vp.TopLeftY = (float)y;
    vp.Width = (float)w;
    vp.Height = (float)h;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    state->context->RSSetViewports(1, &vp);
}

FASTDX_API void fastdx_set_projection(int64_t handle, const float* matrix16) {
    DXState* state = (DXState*)handle;
    if (!state || !state->constantBuffer || !matrix16 || !state->context) return;

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(state->context->Map(state->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        ConstantBufferData* cb = (ConstantBufferData*)mapped.pData;
        memcpy(&cb->projection, matrix16, 16 * sizeof(float));
        state->context->Unmap(state->constantBuffer, 0);
    }

    state->context->VSSetConstantBuffers(0, 1, &state->constantBuffer);
}

FASTDX_API void fastdx_set_blend_mode(int64_t handle, int32_t mode) {
    DXState* state = (DXState*)handle;
    if (!state || !state->context) return;
    ID3D11BlendState* bs = (mode == 0) ? state->blendNone : state->blendSrcOver;
    state->context->OMSetBlendState(bs, nullptr, 0xFFFFFFFF);
}

FASTDX_API void fastdx_draw_triangles(int64_t handle,
                                     const float* vbData, int32_t vertexCount,
                                     const int32_t* ibData, int32_t indexCount,
                                     int64_t textureHandle) {

    DXState* state = (DXState*)handle;
    if (!state || vertexCount <= 0 || indexCount <= 0 || !vbData || !ibData || !state->context) return;

    int vbBytes = vertexCount * sizeof(Vertex);
    int ibBytes = indexCount * sizeof(uint32_t);

    // Manage Dynamic Vertex Buffer
    if (!state->vertexBuffer || state->currentVertexBufferSize < vbBytes) {
        if (state->vertexBuffer) state->vertexBuffer->Release();
        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth = vbBytes + 4096;
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        state->device->CreateBuffer(&bd, nullptr, &state->vertexBuffer);
        state->currentVertexBufferSize = bd.ByteWidth;
    }

    // Manage Dynamic Index Buffer
    if (!state->indexBuffer || state->currentIndexBufferSize < ibBytes) {
        if (state->indexBuffer) state->indexBuffer->Release();
        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth = ibBytes + 4096;
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        state->device->CreateBuffer(&bd, nullptr, &state->indexBuffer);
        state->currentIndexBufferSize = bd.ByteWidth;
    }

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(state->context->Map(state->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, vbData, vbBytes);
        state->context->Unmap(state->vertexBuffer, 0);
    }

    if (SUCCEEDED(state->context->Map(state->indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, ibData, ibBytes);
        state->context->Unmap(state->indexBuffer, 0);
    }

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    state->context->IASetVertexBuffers(0, 1, &state->vertexBuffer, &stride, &offset);
    state->context->IASetIndexBuffer(state->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    state->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    state->context->IASetInputLayout(state->inputLayout);
    if (state->rasterizerState) {
        state->context->RSSetState(state->rasterizerState);
    }

    state->context->VSSetShader(state->vertexShader, nullptr, 0);

    DXTexture* tex = textureHandle ? (DXTexture*)textureHandle : state->defaultWhiteTexture;
    state->context->PSSetShader(state->texturePixelShader, nullptr, 0);
    if (tex && tex->srv) {
        state->context->PSSetShaderResources(0, 1, &tex->srv);
        state->context->PSSetSamplers(0, 1, &state->samplerState);
    }

    state->context->DrawIndexed(indexCount, 0, 0);
}

FASTDX_API int64_t fastdx_create_texture(int64_t handle, int32_t w, int32_t h, const void* pixels) {
    DXState* state = (DXState*)handle;
    if (!state || !state->device || w <= 0 || h <= 0) return 0;

    DXTexture* tex = new DXTexture();
    tex->width = w;
    tex->height = h;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels;
    initData.SysMemPitch = w * 4;

    HRESULT hr = state->device->CreateTexture2D(&desc, pixels ? &initData : nullptr, &tex->texture);
    if (FAILED(hr)) {
        delete tex;
        return 0;
    }

    hr = state->device->CreateShaderResourceView(tex->texture, nullptr, &tex->srv);
    if (FAILED(hr)) {
        if (tex->texture) tex->texture->Release();
        delete tex;
        return 0;
    }

    return (int64_t)tex;
}

FASTDX_API void fastdx_update_texture(int64_t handle, int64_t texHandle, const void* pixels) {
    DXState* state = (DXState*)handle;
    DXTexture* tex = (DXTexture*)texHandle;
    if (!state || !tex || !tex->texture || !pixels || !state->context) return;

    state->context->UpdateSubresource(tex->texture, 0, nullptr, pixels, tex->width * 4, 0);
}

FASTDX_API void fastdx_destroy_texture(int64_t handle, int64_t texHandle) {
    DXTexture* tex = (DXTexture*)texHandle;
    if (tex) {
        if (tex->srv) tex->srv->Release();
        if (tex->texture) tex->texture->Release();
        delete tex;
    }
}

FASTDX_API void fastdx_end_frame(int64_t handle) {
    // End Frame marker
}

FASTDX_API void fastdx_present(int64_t handle) {
    DXState* state = (DXState*)handle;
    if (state && state->swapChain) {
        state->swapChain->Present(1, 0);
    }
}

FASTDX_API void fastdx_destroy(int64_t handle) {
    DXState* state = (DXState*)handle;
    if (!state) return;

    if (state->context) {
        ID3D11RenderTargetView* nullRtv[1] = { nullptr };
        state->context->OMSetRenderTargets(1, nullRtv, nullptr);
        state->context->ClearState();
        state->context->Flush();
    }

    CleanupRenderTarget(state);
    if (state->defaultWhiteTexture) {
        fastdx_destroy_texture(handle, (int64_t)state->defaultWhiteTexture);
        state->defaultWhiteTexture = nullptr;
    }
    if (state->vertexBuffer) state->vertexBuffer->Release();
    if (state->indexBuffer) state->indexBuffer->Release();
    if (state->constantBuffer) state->constantBuffer->Release();
    if (state->samplerState) state->samplerState->Release();
    if (state->rasterizerState) state->rasterizerState->Release();
    if (state->blendNone) state->blendNone->Release();
    if (state->blendSrcOver) state->blendSrcOver->Release();
    if (state->blendAdditive) state->blendAdditive->Release();
    if (state->blendMultiply) state->blendMultiply->Release();
    if (state->inputLayout) state->inputLayout->Release();
    if (state->vertexShader) state->vertexShader->Release();
    if (state->pixelShader) state->pixelShader->Release();
    if (state->texturePixelShader) state->texturePixelShader->Release();
    if (state->swapChain) {
        state->swapChain->SetFullscreenState(FALSE, nullptr);
        state->swapChain->Release();
    }
    if (state->context) state->context->Release();
    if (state->device) state->device->Release();

    delete state;
}

}
