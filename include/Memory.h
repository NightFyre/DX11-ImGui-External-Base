#pragma once
#include "common.h"

class Memory
{
public:
	template<typename T>
	T Read(const __int64 addr) noexcept
	{
		T result{};
		ReadProcessMemory(m_hProc, (void*)addr, &result, sizeof(T), nullptr);
		return result;
	}

	template<typename T>
	T Read(const __int64 addr, SIZE_T szCount) noexcept
	{
		T result{};
		ReadProcessMemory(m_hProc, (void*)addr, &result, szCount, nullptr);
		return result;
	}

	template<typename T>
	T ReadChain(const __int64 addr, const unsigned int offsets[], SIZE_T szCount)
	{
		__int64 temp = addr;
		for (int i = 0; i < szCount; i++)
		{
			temp = Read<long long>(temp);
			temp += offsets[i];
		}

		return Read<T>(temp);
	}

	template<typename T>
	T Write(__int64 addr, const T& value) noexcept { WriteProcessMemory(m_hProc, (void*)addr, &value, sizeof(T), nullptr); }

public:
	DWORD GetPID();
	HANDLE GetHandle();
	HWND GetHWND();
	__int64	GetModuleBase();
	bool ResolveProcessByName(const wchar_t* pName);
	bool ResolveProcessByWindow(const char* pName);
	std::string ReadString(const __int64 addr, SIZE_T szCount);	//	ATTN: max string size = MAX_PATH. Check if string is valid using std::string::empty.
	static bool IsValidPtr(__int64 addr);
	static BOOL CALLBACK GetProcWindows(HWND handle, LPARAM lParam);

public:
	explicit Memory();
	Memory(const wchar_t* procName);
	Memory(const char* windowTitle);
	~Memory();

private:
	DWORD m_PID;
	HWND m_hWnd;
	HANDLE m_hProc;
	long long m_modBase;
	bool m_bAttached{ false };
	
	struct EnumWindowData
	{
		unsigned int procId;
		HWND hwnd;
	};

}; inline std::unique_ptr<Memory> g_Memory;