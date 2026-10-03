#include "DxWindow.h"

DxWindow::DxWindow() { }

DxWindow::~DxWindow() { }

bool DxWindow::Init()
{
    this->m_wc =
    {
        sizeof(WNDCLASSEX),
        CS_CLASSDC,
        WndProc,
        0L,
        0L,
        GetModuleHandle(nullptr),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        L"WC NightFyre Dx11 External Base",
        nullptr
    };
    if (!::RegisterClassEx(&this->m_wc))
        return false;

    this->m_hwnd = ::CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        this->m_wc.lpszClassName,
        L"NightFyre Dx11 External Base",
        WS_POPUP,
        0,
        0,
        1,
        1,
        nullptr,
        nullptr,
        this->m_wc.hInstance,
        this
    );

    if (!this->m_hwnd)
        return false;

    SetLayeredWindowAttributes(this->m_hwnd, 0, 255, LWA_ALPHA);

    MARGINS margins{ -1 };
    DwmExtendFrameIntoClientArea(this->m_hwnd, &margins);

    if (!CreateDeviceD3D(this->m_hwnd))
    {
        DestroyWindow(this->m_hwnd);
        this->m_hwnd = nullptr;
        CleanupDeviceD3D();
        ::UnregisterClassW(this->m_wc.lpszClassName, this->m_wc.hInstance);
        return false;
    }

    ::ShowWindow(this->m_hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(this->m_hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
    io.IniFilename = NULL;                                      // Disable Ini File

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowTitleAlign = ImVec2(.5f, .5f);                  // Center Align Window Title

    ImGui_ImplWin32_Init(this->m_hwnd);
    ImGui_ImplDX11_Init(this->m_pd3dDevice, this->m_pd3dDeviceContext);

    return true;
}

void DxWindow::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(this->m_hwnd);
    UnregisterClass(this->m_wc.lpszClassName, this->m_wc.hInstance);
}

void DxWindow::Tick(const SOverlay& bind)
{
    static float clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };

    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return;

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    if (this->m_pendingWidth != 0 && this->m_pendingHeight != 0 && this->m_pSwapChain)
    {
        CleanupRenderTarget();

        const HRESULT hr = this->m_pSwapChain->ResizeBuffers(
            0,
            this->m_pendingWidth,
            this->m_pendingHeight,
            DXGI_FORMAT_UNKNOWN,
            0
        );

        this->m_pendingWidth = 0;
        this->m_pendingHeight = 0;


        if (SUCCEEDED(hr))
            CreateRenderTarget();
    }

    if (!this->m_mainRenderTargetView || !this->m_ValidClone)
        return;

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
    this->m_pd3dDeviceContext->OMSetRenderTargets(1, &this->m_mainRenderTargetView, NULL);
    this->m_pd3dDeviceContext->ClearRenderTargetView(this->m_mainRenderTargetView, (float*)clearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    this->m_pSwapChain->Present(0, 0);
}

bool DxWindow::UpdateClone(HWND window)
{
    if (!window || !IsWindow(window))
    {
        this->m_hwndTarget = nullptr;
        this->m_posClone = ImVec2(0.0f, 0.0f);
        this->m_szClone = ImVec2(0.0f, 0.0f);
        this->m_ValidClone = false;

        ShowWindow(this->m_hwnd, SW_HIDE);

        return false; // @TODO: window is not a window
    }

    this->m_hwndTarget = window;

    RECT clientRect;
    if (!GetClientRect(window, &clientRect))
    {
        this->m_ValidClone = false;
        ShowWindow(m_hwnd, SW_HIDE);
        return false; // @TODO: failed to get client rect
    }

    const int width = clientRect.right - clientRect.left;
    const int height = clientRect.bottom - clientRect.top;

    if (width <= 0 || height <= 0)
    {
        this->m_ValidClone = false;
        ShowWindow(m_hwnd, SW_HIDE);
        return false; // @TODO: window is collapsed
    }

    POINT clientPos
    {
        clientRect.left,
        clientRect.top
    };

    if (!ClientToScreen(window, &clientPos))
    {
        this->m_ValidClone = false;
        ShowWindow(m_hwnd, SW_HIDE);
        return false; // @TODO: failed to get client rect
    }

    const ImVec2 newPos
    {
        static_cast<float>(clientPos.x),
        static_cast<float>(clientPos.y)
    };

    const ImVec2 newSize
    {
        static_cast<float>(width),
        static_cast<float>(height)
    };

    const bool changed =
        newPos.x != this->m_posClone.x ||
        newPos.y != this->m_posClone.y ||
        newSize.x != this->m_szClone.x ||
        newSize.y != this->m_szClone.y;

    const bool wasValid = this->m_ValidClone;

    this->m_posClone = newPos;
    this->m_szClone = newSize;
    this->m_ValidClone = true;

    if (!wasValid)
        ShowWindow(this->m_hwnd, SW_SHOWNA);

    if (!changed)
        return false; // @TODO: no change detected

    SetWindowPos(
        this->m_hwnd,
        HWND_TOPMOST,
        static_cast<int>(this->m_posClone.x),
        static_cast<int>(this->m_posClone.y),
        static_cast<int>(this->m_szClone.x),
        static_cast<int>(this->m_szClone.y),
        SWP_NOACTIVATE
    );

    return true; // @TODO: updated window , what was updated ?
}

void DxWindow::ClickThrough(bool enabled)
{
    if (this->m_clickThrough == enabled)
        return;

    LONG_PTR style = GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE);

    if (enabled)
        style |= WS_EX_TRANSPARENT;
    else
        style &= ~WS_EX_TRANSPARENT;

    SetWindowLongPtrW(this->m_hwnd, GWL_EXSTYLE, style);

    this->m_clickThrough = enabled;
}

void DxWindow::SetWindowFocus(HWND window)
{
    if (!window || !IsWindow(window))
        return;

    if (IsIconic(this->m_hwndTarget))
        ShowWindow(this->m_hwndTarget, SW_RESTORE);

    SetForegroundWindow(window);
    SetActiveWindow(window);
}

void DxWindow::FocusOverlay()
{
    if (!this->m_hwnd)
        return;

    SetWindowFocus(this->m_hwnd);
}

void DxWindow::FocusTarget()
{
    if (!this->m_hwndTarget || !IsWindow(this->m_hwndTarget))
        return;

    if (IsIconic(this->m_hwndTarget))
        ShowWindow(this->m_hwndTarget, SW_RESTORE);

    SetWindowFocus(this->m_hwndTarget);
    SetFocus(this->m_hwndTarget);
}

ImVec2 DxWindow::GetCloneWindowSize() const { return this->m_szClone; }

ImVec2 DxWindow::GetCloneWindowPos() const { return this->m_posClone; }

HWND DxWindow::GetWindowHandle() const { return this->m_hwnd; }

ID3D11Device* DxWindow::GetD3DDevice() const { return this->m_pd3dDevice; }

IDXGISwapChain* DxWindow::GetSwapChain() const { return this->m_pSwapChain; }

ID3D11DeviceContext* DxWindow::GetDeviceContext() const { return this->m_pd3dDeviceContext; }

ID3D11RenderTargetView* DxWindow::GetRTV() const { return this->m_mainRenderTargetView; }

bool DxWindow::CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;

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
    const HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &this->m_pSwapChain,
        &this->m_pd3dDevice,
        &featureLevel,
        &this->m_pd3dDeviceContext
    );

    if (FAILED(hr))
        return false;

    return CreateRenderTarget();
}

bool DxWindow::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    HRESULT hr = this->m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (FAILED(hr))
        return false;

    hr = this->m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &this->m_mainRenderTargetView);

    pBackBuffer->Release();

    return SUCCEEDED(hr);
}

void DxWindow::CleanupDeviceD3D()
{
    CleanupRenderTarget();

    if (this->m_pSwapChain)
    {
        this->m_pSwapChain->Release();
        this->m_pSwapChain = nullptr;
    }

    if (this->m_pd3dDeviceContext)
    {
        this->m_pd3dDeviceContext->Release();
        this->m_pd3dDeviceContext = nullptr;
    }

    if (this->m_pd3dDevice)
    {
        this->m_pd3dDevice->Release();
        this->m_pd3dDevice = nullptr;
    }
}

void DxWindow::CleanupRenderTarget()
{
    if (this->m_mainRenderTargetView)
    {
        this->m_mainRenderTargetView->Release();
        this->m_mainRenderTargetView = nullptr;
    }
}

LRESULT WINAPI DxWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DxWindow* window = reinterpret_cast<DxWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    if (msg == WM_NCCREATE)
    {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);

        window = static_cast<DxWindow*>(create->lpCreateParams);

        SetWindowLongPtrW(
            hWnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(window)
        );
    }

    if (ImGui::GetCurrentContext() && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
    {
        if (window && wParam != SIZE_MINIMIZED)
        {
            window->m_pendingWidth = LOWORD(lParam);
            window->m_pendingHeight = HIWORD(lParam);
        }

        return 0;
    }

    case WM_SYSCOMMAND:
    {
        if ((wParam & 0xFFF0) == SC_KEYMENU)
            return 0;

        break;
    }

    case WM_DESTROY:
    {
        ::PostQuitMessage(0);
        return 0;
    }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}