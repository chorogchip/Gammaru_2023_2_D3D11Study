#pragma comment (lib, "D3D11.lib")
#pragma comment (lib, "D3DCompiler.lib")
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_HARDWARE;
D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pImmediateContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;

bool InitD3D(HWND hwnd)
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(hwnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    hr = D3D11CreateDeviceAndSwapChain(
        NULL, g_driverType, NULL, createDeviceFlags, &featureLevel, 1,
        D3D11_SDK_VERSION, &sd,
        &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

    if (FAILED(hr))
        return false;

    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer(
        0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return false;

    hr = g_pd3dDevice->CreateRenderTargetView(
        pBackBuffer, NULL, &g_pRenderTargetView);

    pBackBuffer->Release();

    if (FAILED(hr))
        return false;

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

    return true;
}

void ClearD3D()
{
    if (g_pImmediateContext != NULL) g_pImmediateContext->ClearState();

    if (g_pRenderTargetView != NULL) g_pRenderTargetView->Release();
    if (g_pSwapChain != NULL) g_pSwapChain->Release();
    if (g_pImmediateContext != NULL) g_pImmediateContext->Release();
    if (g_pd3dDevice != NULL) g_pd3dDevice->Release();
}

void Render()
{
    float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);

    g_pSwapChain->Present(0, 0);
}
