#pragma once
#include "common.h"

class Console
{
public:
	enum EColors : int
		{
			EColor_dark_blue = 1,
			EColor_dark_green,
			EColor_dark_teal,
			EColor_dark_red,
			EColor_dark_pink,
			EColor_dark_yellow,
			EColor_dark_white,
			EColor_dark_gray,
			EColor_blue,
			EColor_green,
			EColor_teal,
			EColor_red,
			EColor_pink,
			EColor_yellow,
			EColor_white,
			EColor_DEFAULT = EColor_white
		};

public:
	void								SetConsoleVisibility(bool bShow);
	HANDLE								GetHandle();
	HWND								GetWindowHandle();
	void								Log(const char* fmt, ...);
	void								LogColor(const char* fmt, EColors color, ...);

public:
	explicit Console();
	Console(const char* title, bool bShowWindow = true);
	~Console();

private:
	HANDLE								m_Handle{};
	HWND								m_Hwnd{};
	FILE*								m_OutStream{};
	bool								m_bShow{ true };
}; inline std::unique_ptr<Console> g_Console;