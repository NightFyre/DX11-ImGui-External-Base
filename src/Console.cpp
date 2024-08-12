#include "Console.h"

Console::Console()
{
	AllocConsole();
	m_Handle = GetStdHandle(STD_OUTPUT_HANDLE);
	m_Hwnd = GetConsoleWindow();
	freopen_s(&m_OutStream, "CONOUT$", "w", stdout);
	ShowWindow(m_Hwnd, m_bShow ? SW_SHOW : SW_HIDE);
}

Console::Console(const char* title, bool bShowWindow)
{
	AllocConsole();
	m_Handle = GetStdHandle(STD_OUTPUT_HANDLE);
	m_Hwnd = GetConsoleWindow();
	freopen_s(&m_OutStream, "CONOUT$", "w", stdout);


	m_bShow = bShowWindow;
	SetConsoleTitleA(title);
	ShowWindow(m_Hwnd, m_bShow ? SW_SHOW : SW_HIDE);
}

Console::~Console()
{
	if (m_OutStream)
		fclose(m_OutStream);

	FreeConsole();
}

HANDLE Console::GetHandle() { return this->m_Handle; }

HWND Console::GetWindowHandle() { return this->m_Hwnd; }

void Console::Log(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	vfprintf(m_OutStream, fmt, arg);
	va_end(arg);
}

void Console::LogColor(const char* fmt, EColors color, ...)
{
	SetConsoleTextAttribute(m_Handle, color);
	va_list arg;
	va_start(arg, color);
	vfprintf(m_OutStream, fmt, arg);
	va_end(arg);
	SetConsoleTextAttribute(m_Handle, EColor_DEFAULT);
}

void Console::SetConsoleVisibility(bool bShow)
{
	this->m_bShow = bShow;
	ShowWindow(m_Hwnd, bShow ? SW_SHOW : SW_HIDE);
}