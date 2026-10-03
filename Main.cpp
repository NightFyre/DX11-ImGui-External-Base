#include "console.h"
#include "Memory.h"
#include "Menu.h"

int main()
{
	//	Initialize Console Window
	///	print to console with printf or use Console class functions with the g_Console pointer
	g_Console = std::make_unique<Console>("Dx11 External Base", true);

	//	Initialize DxWindow
	///	creates a directx window that spans the entire client screen
	///	additionally if the process was resolved in the previous step, a clone of the main window will have been created. Controls are available to target another window.
	g_dxWindow = std::make_unique<DxWindow>();
	if (!g_dxWindow->Init())
		EXIT_FAILURE;

	g_dxWindow->UpdateClone(g_Memory.GetProcessInfo().hWnd);
	g_dxWindow->SetWindowFocus(g_Memory.GetProcessInfo().hWnd);
	
	//	Initialize Menu
	///	
	g_Menu = std::make_unique<Menu>();

	static int LastTick = 0;
	while (g_Menu->bRunning)
	{
		bool bTimer = GetTickCount64() - LastTick > 500;
		if (GetAsyncKeyState(VK_RCONTROL) & 0x8000 && bTimer)
		{
			g_Menu->bShowMenu ^= 1;
			g_Menu->SetVisible(g_Menu->bShowMenu);
			switch (g_Menu->bShowMenu)
			{
				case(true): g_dxWindow->SetWindowFocus(g_dxWindow->GetWindowHandle()); break;
				case(false): g_dxWindow->SetWindowFocus(g_Memory.GetProcessInfo().hWnd); break;
			}

			LastTick = GetTickCount64();
		}

		g_Memory.update();


		g_dxWindow->UpdateClone(g_Memory.GetProcessInfo().hWnd);
		g_dxWindow->Tick(g_Menu->GetOverlay());

		std::this_thread::sleep_for(1ms);
		std::this_thread::yield();
	}

	return EXIT_SUCCESS;
}