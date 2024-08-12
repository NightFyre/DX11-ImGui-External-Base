#include "DxWindow.h"

MARGINS gMargin;
DxWindow::DxWindow()
{
    m_szScreen.x = GetSystemMetrics(SM_CXSCREEN);
    m_szScreen.y = GetSystemMetrics(SM_CYSCREEN);
    m_wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Dx11 External Base", nullptr };
    ::RegisterClassEx(&m_wc);
    m_hwnd = ::CreateWindowW(m_wc.lpszClassName, L"Dx11 External Base", WS_EX_TOPMOST | WS_POPUP, 0, 0, static_cast<int>(m_szScreen.x), static_cast<int>(m_szScreen.y), nullptr, nullptr, m_wc.hInstance, nullptr);
    SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    gMargin = { 0, 0, static_cast<int>(m_szScreen.x), static_cast<int>(m_szScreen.y) };
    DwmExtendFrameIntoClientArea(m_hwnd, &gMargin);

    if (!CreateDeviceD3D(m_hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
        return;
    }

    ::ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(m_hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
    io.IniFilename = NULL;                                      // Disable Ini File
    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_pd3dDevice, m_pd3dDeviceContext);
}

DxWindow::~DxWindow()
{
    CleanupDeviceD3D();
    DestroyWindow(m_hwnd);
    UnregisterClass(m_wc.lpszClassName, m_wc.hInstance);
}

bool DxWindow::CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();

    return true;
}

void DxWindow::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
    pBackBuffer->Release();
}

void DxWindow::CleanupDeviceD3D()
{
    CleanupRenderTarget();

    if (m_pSwapChain) 
    { 
        m_pSwapChain->Release();
        m_pSwapChain = nullptr; 
    }

    if (m_pd3dDeviceContext) 
    { 
        m_pd3dDeviceContext->Release(); 
        m_pd3dDeviceContext = nullptr;
    }

    if (m_pd3dDevice) 
    { 
        m_pd3dDevice->Release();
        m_pd3dDevice = nullptr; 
    }
}

void DxWindow::CleanupRenderTarget()
{
    if (m_mainRenderTargetView) 
    { 
        m_mainRenderTargetView->Release(); 
        m_mainRenderTargetView = nullptr;
    }
}

void DxWindow::SetWindowStyle(LONG flags) { SetWindowLong(m_hwnd, GWL_EXSTYLE, flags); }

void DxWindow::ClickThrough(bool bIsClickThrough)
{
    LONG flags = WS_EX_LAYERED;

    if (bIsClickThrough) 
      flags = WS_EX_LAYERED | WS_EX_TRANSPARENT;

    SetWindowStyle(flags);
}

void DxWindow::SetWindowFocus(HWND window)
{
    SetForegroundWindow(window);
    SetActiveWindow(window);
}

ImVec2 DxWindow::GetScreenSize() { return m_szScreen; }

ImVec2 DxWindow::GetCloneWindowSize() { return m_szClone; }

ImVec2 DxWindow::GetCloneWindowPos() { return m_posClone; }

HWND DxWindow::GetWindowHandle() { return m_hwnd; }

ID3D11Device* DxWindow::GetD3DDevice() { return m_pd3dDevice; }

IDXGISwapChain* DxWindow::GetSwapChain() { return m_pSwapChain; }

ID3D11DeviceContext* DxWindow::GetDeviceContext() { return m_pd3dDeviceContext; }

ID3D11RenderTargetView* DxWindow::GetRTV() { return m_mainRenderTargetView; }

void DxWindow::Update(SOverlay bind)
{
    static float clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };

    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    ::SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (bind.bIsShown)
    {
        bind.Shroud();
        bind.Menu();
    }
    else
        bind.Hud();

    ClickThrough(!bind.bIsShown);

    ImGui::Render();
    m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, NULL);
    m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, (float*)clearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    m_pSwapChain->Present(1, 0);
}

void DxWindow::CloneUpdate(HWND window)
{
    RECT clientRect;
    POINT topLeft{};
    POINT bottomRight{};
    GetClientRect(window, &clientRect);
    topLeft.x = clientRect.left;
    topLeft.y = clientRect.top;
    bottomRight.x = clientRect.right;
    bottomRight.y = clientRect.bottom;
    ClientToScreen(window, &topLeft);
    ClientToScreen(window, &bottomRight);
    m_posClone = { float(topLeft.x), float(topLeft.y) };
    m_szClone = { float((bottomRight.x - topLeft.x)), float((bottomRight.y - topLeft.y)) };
}

LRESULT WINAPI DxWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}