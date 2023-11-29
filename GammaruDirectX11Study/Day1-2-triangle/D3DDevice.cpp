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

ID3D11VertexShader* g_pVertexShader = NULL;
ID3D11PixelShader* g_pPixelShader = NULL;
ID3D11InputLayout* g_pInputLayout = NULL;
ID3D11Buffer* g_pVertexBuffer = NULL;

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

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* pErrorBlob = NULL;

    ID3DBlob* pVSBlob = NULL;
    hr = D3DCompileFromFile(
        L"Shader1.fx", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VS", "vs_4_0",
        dwShaderFlags, 0, &pVSBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob != NULL)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        MessageBox(NULL, L"Vertex Shader Compile Failed", L"Error", MB_OK);
        return false;
    }
    if (pErrorBlob != NULL)
    {
        pErrorBlob->Release();
        pErrorBlob = NULL;
    }

    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(),
        NULL, &g_pVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return false;
    }

    ID3DBlob* pPSBlob = NULL;
    hr = D3DCompileFromFile(
        L"Shader1.fx", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PS", "ps_4_0",
        dwShaderFlags, 0, &pPSBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob != NULL)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        MessageBox(NULL, L"Pixel Shader Compile Failed", L"Error", MB_OK);
        return false;
    }
    if (pErrorBlob != NULL)
    {
        pErrorBlob->Release();
        pErrorBlob = NULL;
    }

    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(),
        NULL, &g_pPixelShader);
    if (FAILED(hr))
    {
        pPSBlob->Release();
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC layouts[1];
    UINT numElements = ARRAYSIZE(layouts);
    layouts[0].SemanticName = "POSITION";
    layouts[0].SemanticIndex = 0;
    layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    layouts[0].InputSlot = 0;
    layouts[0].AlignedByteOffset = 0;
    layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layouts[0].InstanceDataStepRate = 0;

    hr = g_pd3dDevice->CreateInputLayout(
        layouts, numElements,
        pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(),
        &g_pInputLayout);

    pVSBlob->Release();
    pPSBlob->Release();
    if (FAILED(hr))
        return false;

    g_pImmediateContext->IASetInputLayout(g_pInputLayout);


    DirectX::XMFLOAT3 vertices[] =
    {
        DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f),
        DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f),
        DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f),
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(DirectX::XMFLOAT3) * 3;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return false;

    UINT stride = sizeof(DirectX::XMFLOAT3);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(
        0, 1, &g_pVertexBuffer, &stride, &offset);

    g_pImmediateContext->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


    return true;
}

void ClearD3D()
{
    if (g_pImmediateContext != NULL) g_pImmediateContext->ClearState();

    if (g_pVertexBuffer != NULL) g_pVertexBuffer->Release();
    if (g_pInputLayout != NULL) g_pInputLayout->Release();
    if (g_pPixelShader != NULL) g_pPixelShader->Release();
    if (g_pVertexShader != NULL) g_pVertexShader->Release();

    if (g_pRenderTargetView != NULL) g_pRenderTargetView->Release();
    if (g_pSwapChain != NULL) g_pSwapChain->Release();
    if (g_pImmediateContext != NULL) g_pImmediateContext->Release();
    if (g_pd3dDevice != NULL) g_pd3dDevice->Release();
}

void Render()
{
    float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);

    g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
    g_pImmediateContext->Draw(3, 0);

    g_pSwapChain->Present(0, 0);
}
