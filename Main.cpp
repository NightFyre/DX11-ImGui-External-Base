#include "console.h"
#include "Memory.h"
#include "Menu.h"

int main()
{
	//	Initialize Console Window
	///	print to console with printf or use Console class functions with the g_Console pointer
	g_Console = std::make_unique<Console>("Dx11 External Base", true);

	//	Initialize Memory Class
	///	default attaches to notepad.exe. 
	g_Memory = std::make_unique<Memory>(L"notepad.exe");
	
	//	Initialize DxWindow
	///	creates a directx window that spans the entire client screen
	///	additionally if the process was resolved in the previous step, a clone of the main window will have been created. Controls are available to target another window.
	g_dxWindow = std::make_unique<DxWindow>();
	
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
			g_Menu->UpdateOverlayViewState(g_Menu->bShowMenu);
			switch (g_Menu->bShowMenu)
			{
				case(true): g_dxWindow->SetWindowFocus(g_dxWindow->GetWindowHandle()); break;
				case(false): g_dxWindow->SetWindowFocus(g_Memory->GetHWND()); break;
			}

			LastTick = GetTickCount64();
		}

		g_dxWindow->CloneUpdate(g_Memory->GetHWND());
		g_dxWindow->Update(g_Menu->GetOverlay());

		std::this_thread::sleep_for(1ms);
		std::this_thread::yield();
	}

	return EXIT_SUCCESS;
}