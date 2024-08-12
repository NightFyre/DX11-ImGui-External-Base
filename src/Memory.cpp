#include "Memory.h"

Memory::Memory() { m_hWnd = GetConsoleWindow(); }

Memory::Memory(const wchar_t* procName) { m_bAttached = ResolveProcessByName(procName); }

Memory::Memory(const char* windowTitle) { m_bAttached = ResolveProcessByWindow(windowTitle); }

Memory::~Memory()
{
	if (m_hProc != INVALID_HANDLE_VALUE)
		CloseHandle(m_hProc);

	if (m_hWnd != INVALID_HANDLE_VALUE)
		CloseHandle(m_hWnd);
}

DWORD Memory::GetPID() { return m_PID; }

HANDLE Memory::GetHandle() { return m_hProc; }

HWND Memory::GetHWND() { return m_hWnd; }

__int64 Memory::GetModuleBase() { return m_modBase; }

bool Memory::ResolveProcessByName(const wchar_t* pName)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);
	if (!Process32Next(hSnap, &procEntry))
	{
		CloseHandle(hSnap);
		return false;
	}

	do
	{
		if (_wcsicmp(procEntry.szExeFile, pName))
			continue;

		m_PID = procEntry.th32ProcessID;
		m_hProc = OpenProcess(PROCESS_ALL_ACCESS, false, procEntry.th32ProcessID);

		HANDLE modSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_PID);
		if (modSnap == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hSnap);
			return false;
		}

		MODULEENTRY32 me32;
		me32.dwSize = sizeof(me32);
		if (!Module32First(modSnap, &me32))
		{
			CloseHandle(modSnap);
			CloseHandle(hSnap);
			return false;
		}

		m_modBase = (long long)me32.modBaseAddr;

		CloseHandle(modSnap);
		CloseHandle(hSnap);
		
		EnumWindowData eDat;
		eDat.procId = m_PID;
		if (EnumWindows(GetProcWindows, reinterpret_cast<LPARAM>(&eDat)))
			m_hWnd = eDat.hwnd;
		
		return true;

	} while (Process32Next(hSnap, &procEntry));

	CloseHandle(hSnap);

	return false;
}

bool Memory::ResolveProcessByWindow(const char* windowName)
{
	m_hWnd = FindWindowExA(0, 0, 0, windowName);
	if (!m_hWnd || m_hWnd == INVALID_HANDLE_VALUE)
		return false;

	if (!GetWindowThreadProcessId(m_hWnd, &m_PID))
		return false;

	m_hProc = OpenProcess(PROCESS_ALL_ACCESS, false, m_PID);
	if (!m_hProc || m_hProc == INVALID_HANDLE_VALUE)
		return false;

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_PID);
	MODULEENTRY32 me32;
	me32.dwSize = sizeof(me32);
	Module32First(hSnap, &me32);
	m_modBase = (long long)me32.modBaseAddr;
	CloseHandle(hSnap);
}

bool Memory::IsValidPtr(__int64 addr) { return (addr < 0x1000 ? false : addr < 0x000F000000000000); }

std::string Memory::ReadString(const __int64 addr, const SIZE_T szCount)
{
	char result[MAX_PATH];
	
	if (!ReadProcessMemory(m_hProc, (void*)addr, &result, szCount, nullptr))
		return std::string();
	
	return std::string(result);
}

BOOL CALLBACK Memory::GetProcWindows(HWND window, LPARAM lParam)
{
	auto data = reinterpret_cast<EnumWindowData*>(lParam);

	DWORD windowPID;
	GetWindowThreadProcessId(window, &windowPID);

	bool isMainWindow = GetWindow(window, GW_OWNER) == (HWND)0 && IsWindowVisible(window);
	if (windowPID != data->procId || !isMainWindow)
		return true;

	data->hwnd = window;

	return true;
}