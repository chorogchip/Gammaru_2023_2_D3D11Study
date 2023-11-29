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

ID3D11Texture2D* g_pDepthStencil = NULL;
ID3D11DepthStencilView* g_pDSV = NULL;

ID3D11VertexShader* g_pVertexShader = NULL;
ID3D11PixelShader* g_pPixelShader = NULL;
ID3D11InputLayout* g_pInputLayout = NULL;
ID3D11Buffer* g_pVertexBuffer = NULL;

using namespace DirectX;
struct VertexStructure1 {
    XMFLOAT3 pos;
    XMFLOAT2 uv;
};

struct VSConstBufferPerObject {
    XMMATRIX world;
};

struct VSConstBufferPerFrame {
    XMMATRIX view;
};

struct VSConstBufferPerResize {
    XMMATRIX projection;
};

ID3D11Buffer* g_pVSConstBufferPerObject = NULL;
ID3D11Buffer* g_pVSConstBufferPerFrame = NULL;
ID3D11Buffer* g_pVSConstBufferPerResize = NULL;

float objPos[2][3] = { { 0.0f, 0.0f, 0.0f }, { 3.0f, 3.0f, 3.0f } };
float objPitchYawRoll[2][3];
float camPos[3] = { 1.0f, 2.0f, -7.0f };
float camPitchYawRoll[3] = { 0.3f, -0.5f, 0.0f };

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

    D3D11_TEXTURE2D_DESC descDepth;
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
    if (FAILED(hr))
        return false;

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDSV);
    if (FAILED(hr))
        return false;

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDSV);

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

    D3D11_INPUT_ELEMENT_DESC layouts[2];
    UINT numElements = ARRAYSIZE(layouts);
    layouts[0].SemanticName = "POSITION";
    layouts[0].SemanticIndex = 0;
    layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    layouts[0].InputSlot = 0;
    layouts[0].AlignedByteOffset = 0;
    layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layouts[0].InstanceDataStepRate = 0;

    layouts[1].SemanticName = "TEXCOORD";
    layouts[1].SemanticIndex = 0;
    layouts[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    layouts[1].InputSlot = 0;
    layouts[1].AlignedByteOffset = sizeof(XMFLOAT3);
    layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layouts[1].InstanceDataStepRate = 0;

    hr = g_pd3dDevice->CreateInputLayout(
        layouts, numElements,
        pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(),
        &g_pInputLayout);

    pVSBlob->Release();
    pPSBlob->Release();
    if (FAILED(hr))
        return false;

    g_pImmediateContext->IASetInputLayout(g_pInputLayout);


    VertexStructure1 vertices[] =
    {
        // front
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

        // back
        { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

        // left
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

        // right
        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

        // down
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

        // down
        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
    };
    UINT vertices_size = ARRAYSIZE(vertices);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return false;

    UINT stride = sizeof(VertexStructure1);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(
        0, 1, &g_pVertexBuffer, &stride, &offset);

    g_pImmediateContext->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(VSConstBufferPerObject);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;

    hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pVSConstBufferPerObject);
    if (FAILED(hr))
        return false;

    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pVSConstBufferPerObject);

    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VSConstBufferPerFrame);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pVSConstBufferPerFrame);
    if (FAILED(hr))
        return false;

    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pVSConstBufferPerFrame);

    VSConstBufferPerResize vsConstBufferPerResize;
    // fill data
    vsConstBufferPerResize.projection = XMMatrixPerspectiveFovLH(
        XM_PI / 3.0f, (float)width / (float)height, 0.01f, 1000.0f);

    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(VSConstBufferPerResize);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;

    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &vsConstBufferPerResize;

    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVSConstBufferPerResize);
    if (FAILED(hr))
        return false;

    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pVSConstBufferPerResize);

    return true;
}

void ClearD3D()
{
    if (g_pImmediateContext != NULL) g_pImmediateContext->ClearState();

    if (g_pVSConstBufferPerObject != NULL) g_pVSConstBufferPerObject->Release();
    if (g_pVSConstBufferPerResize != NULL) g_pVSConstBufferPerResize->Release();
    if (g_pVSConstBufferPerFrame != NULL) g_pVSConstBufferPerFrame->Release();

    if (g_pVertexBuffer != NULL) g_pVertexBuffer->Release();
    if (g_pInputLayout != NULL) g_pInputLayout->Release();
    if (g_pPixelShader != NULL) g_pPixelShader->Release();
    if (g_pVertexShader != NULL) g_pVertexShader->Release();

    if (g_pDSV != NULL) g_pDSV->Release();
    if (g_pDepthStencil != NULL) g_pDepthStencil->Release();

    if (g_pRenderTargetView != NULL) g_pRenderTargetView->Release();
    if (g_pSwapChain != NULL) g_pSwapChain->Release();
    if (g_pImmediateContext != NULL) g_pImmediateContext->Release();
    if (g_pd3dDevice != NULL) g_pd3dDevice->Release();
}

void Render()
{
    float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
    g_pImmediateContext->ClearDepthStencilView(g_pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // make matrices
    static long long t = 0;
    ++t;

    VSConstBufferPerFrame vsConstBufferPerFrame;
    vsConstBufferPerFrame.view = XMMatrixMultiply(
        XMMatrixTranslation(-camPos[0], -camPos[1], -camPos[2]),
        XMMatrixTranspose(XMMatrixRotationRollPitchYaw(
            camPitchYawRoll[0], camPitchYawRoll[1], camPitchYawRoll[2]))
    );

    D3D11_MAPPED_SUBRESOURCE resourceVSConstBuffer;
    ZeroMemory(&resourceVSConstBuffer, sizeof(resourceVSConstBuffer));

    g_pImmediateContext->Map(g_pVSConstBufferPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &resourceVSConstBuffer);
    memcpy(resourceVSConstBuffer.pData, &vsConstBufferPerFrame, sizeof(VSConstBufferPerFrame));
    g_pImmediateContext->Unmap(g_pVSConstBufferPerFrame, 0);

    g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);

    VSConstBufferPerObject vsConstBufferPerObject;
    for (int i = 0; i < 2; ++i) {
        vsConstBufferPerObject.world = XMMatrixMultiply(
            XMMatrixRotationRollPitchYaw(
                t / 1000.0f + objPitchYawRoll[i][0],
                t / 2000.0f + objPitchYawRoll[i][1],
                t / 3000.0f + objPitchYawRoll[i][2]),
            XMMatrixTranslation(objPos[i][0], objPos[i][1], objPos[i][2]));
        g_pImmediateContext->UpdateSubresource(
            g_pVSConstBufferPerObject, 0, NULL, &vsConstBufferPerObject, 0, 0);
        g_pImmediateContext->Draw(3 * 2 * 6, 0);
    }

    g_pSwapChain->Present(0, 0);
}
