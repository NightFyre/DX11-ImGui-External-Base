#include "Memory.h"

Memory::Memory() : exMemory()
{
	this->bAttached = Attach("notepad.exe", PROCESS_ALL_ACCESS);
}

Memory::Memory(const std::string& name) : exMemory(name)
{
	this->bAttached = Attach(name, PROCESS_ALL_ACCESS);
}

Memory::Memory(const std::string& name, const DWORD& dwAccess) : exMemory(name, dwAccess)
{
	this->bAttached = Attach(name, dwAccess);
}

bool Memory::Attach(const std::string& name, const DWORD& dwAccess)
{
	procInfo_t proc;
	if (!AttachEx(name, &proc, dwAccess))
		return false;

	this->vmProcess = proc;

	this->vmProcess.hWnd = GetProcessWindowEx(this->vmProcess.dwPID);

	return this->vmProcess.bAttached;
}

bool Memory::Detach()
{
	bool result = DetachEx(this->vmProcess);

	return result;
}

void Memory::update()
{
	if (!this->vmProcess.dwPID)
	{
		*this = Memory("notepad.exe");
	}

	if (!this->vmProcess.hWnd || !IsWindow(this->vmProcess.hWnd) || !IsWindowVisible(this->vmProcess.hWnd))
		this->vmProcess.hWnd = GetProcessWindowEx(this->vmProcess.dwPID);

	char buffer[MAX_PATH];
	if (this->vmProcess.hWnd && GetWindowTextA(this->vmProcess.hWnd, buffer, MAX_PATH))
		this->vmProcess.mWndwTitle = std::string(buffer);
}