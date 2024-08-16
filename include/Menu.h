#pragma once
#include "Memory.h"
#include "DxWindow.h"

class Menu
{
public:
	bool bRunning{ true };
	bool bShowMenu{ true };
	bool bShowDemoWindow{ false };
	bool bShowStyleEditor{ false };

public:
	void Draw();
	void MainMenu();
	void SHROUD();
	void HUD();

	DxWindow::SOverlay GetOverlay();
	void UpdateOverlayViewState(bool bState);


public:
	explicit Menu();
	~Menu() noexcept = default;

private:
	DxWindow::SOverlay elements;

}; inline std::unique_ptr<Menu> g_Menu;


#define IM_COL32_RED ImColor(1.0f, 0.0f, 0.0f, 1.0f)
#define IM_COL32_BLUE ImColor(0.0f, 1.0f, 0.0f, 1.0f)
#define IM_COL32_GREEN ImColor(0.0f, 1.0f, 0.0f, 1.0f)
#define IM_COL32_YELLOW ImColor(1.0f, 1.0f, 0.0f, 1.0f)
class GUI
{
public:
	static void TextCentered(const char* pText);
	static void TextCenteredf(const char* pText, ...);

public:
	static void DrawText_(ImVec2 pos, ImColor color, const char* pText, float fontSize);
	static void DrawTextf(ImVec2 pos, ImColor color, const char* pText, float fontSize, ...);
	static void DrawTextCentered(ImVec2 pos, ImColor color, const char* pText, float fontsize);
	static void DrawTextCenteredf(ImVec2 pos, ImColor color, const char* pText, float fontsize, ...);
};
