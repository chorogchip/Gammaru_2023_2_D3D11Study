#include <Windows.h>
#include <cmath>
#include <DirectXMath.h>

bool InitD3D(HWND hwnd);
void ClearD3D();
void Render();

extern float camPos[3];
extern float camPitchYawRoll[3];
const float PI = 3.1415926536f;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        switch ((char)wParam)
        {
        case 'W':
        {
            auto mat = DirectX::XMMatrixRotationRollPitchYaw(
                camPitchYawRoll[0], camPitchYawRoll[1], camPitchYawRoll[2]);
            camPos[0] += mat.r[2].m128_f32[0];
            camPos[1] += mat.r[2].m128_f32[1];
            camPos[2] += mat.r[2].m128_f32[2];
            break;
        }
        case 'S':
        {
            auto mat = DirectX::XMMatrixRotationRollPitchYaw(
                camPitchYawRoll[0], camPitchYawRoll[1], camPitchYawRoll[2]);
            camPos[0] -= mat.r[2].m128_f32[0];
            camPos[1] -= mat.r[2].m128_f32[1];
            camPos[2] -= mat.r[2].m128_f32[2];
            break;
        }
        case 'A':
        {
            auto mat = DirectX::XMMatrixRotationRollPitchYaw(
                camPitchYawRoll[0], camPitchYawRoll[1], camPitchYawRoll[2]);
            camPos[0] -= mat.r[0].m128_f32[0];
            camPos[1] -= mat.r[0].m128_f32[1];
            camPos[2] -= mat.r[0].m128_f32[2];
            break;
        }
        case 'D':
        {
            auto mat = DirectX::XMMatrixRotationRollPitchYaw(
                camPitchYawRoll[0], camPitchYawRoll[1], camPitchYawRoll[2]);
            camPos[0] += mat.r[0].m128_f32[0];
            camPos[1] += mat.r[0].m128_f32[1];
            camPos[2] += mat.r[0].m128_f32[2];
            break;
        }
        case VK_SPACE:
        {
            auto mat = DirectX::XMMatrixRotationRollPitchYaw(
                camPitchYawRoll[0], camPitchYawRoll[1], camPitchYawRoll[2]);
            camPos[0] += mat.r[1].m128_f32[0];
            camPos[1] += mat.r[1].m128_f32[1];
            camPos[2] += mat.r[1].m128_f32[2];
            break;
        }
        case VK_SHIFT:
        {
            auto mat = DirectX::XMMatrixRotationRollPitchYaw(
                camPitchYawRoll[0], camPitchYawRoll[1], camPitchYawRoll[2]);
            camPos[0] -= mat.r[1].m128_f32[0];
            camPos[1] -= mat.r[1].m128_f32[1];
            camPos[2] -= mat.r[1].m128_f32[2];
            break;
        }
        case 'Q':
            camPitchYawRoll[1] = std::remainder(
                camPitchYawRoll[1] - 0.1f, 2.0f * PI);
            break;
        case 'E':
            camPitchYawRoll[1] = std::remainder(
                camPitchYawRoll[1] + 0.1f, 2.0f * PI);
            break;
        case 'R':
            camPitchYawRoll[0] = max(-0.5f * PI, min(0.5f * PI,
                camPitchYawRoll[0] - 0.1f));
            break;
        case 'F':
            camPitchYawRoll[0] = max(-0.5f * PI, min(0.5f * PI,
                camPitchYawRoll[0] + 0.1f));
            break;
        }
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";

    if (!RegisterClass(&wc))
        return FALSE;

    HWND hwnd = CreateWindow(
        L"WindowClass", L"Title",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        960, 540,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
        return FALSE;

    if (!InitD3D(hwnd))
        return FALSE;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (msg.message != WM_QUIT)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Render();
    }

    ClearD3D();

    return (int)msg.wParam;
}