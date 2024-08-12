#pragma once
#include "common.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class DxWindow
{
public:
	typedef std::function<void()> OverlayFunctionPtr;
	struct SOverlay
	{
		bool bIsShown;
		OverlayFunctionPtr Menu;
		OverlayFunctionPtr Shroud;
		OverlayFunctionPtr Hud;
	};

public:
	void SetWindowStyle(LONG style);
	void ClickThrough(bool bIsClickThrough);
	void SetWindowFocus(HWND window);
	ImVec2 GetScreenSize();
	ImVec2 GetCloneWindowSize();
	ImVec2 GetCloneWindowPos();
	HWND GetWindowHandle();
	ID3D11Device* GetD3DDevice();
	IDXGISwapChain* GetSwapChain();
	ID3D11DeviceContext* GetDeviceContext();
	ID3D11RenderTargetView* GetRTV();

public:
	void Update(SOverlay bind);
	void CloneUpdate(HWND wndw);

public:
	explicit DxWindow();
	~DxWindow();

private:
	HWND m_hwnd{ 0 };
	WNDCLASSEX m_wc{ };
	ID3D11Device* m_pd3dDevice{ nullptr };
	IDXGISwapChain* m_pSwapChain{ nullptr };
	ID3D11DeviceContext* m_pd3dDeviceContext{ nullptr };
	ID3D11RenderTargetView* m_mainRenderTargetView{ nullptr };
	ImVec2 m_szScreen{ 0.0f, 0.0f };
	ImVec2 m_szClone{ 0.0f, 0.0f };
	ImVec2 m_posClone{ 0.0f, 0.0f };

private:
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();

private:
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}; inline std::unique_ptr<DxWindow> g_dxWindow;