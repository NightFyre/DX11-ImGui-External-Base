#pragma once
#include "libs/exMemory.hpp"

class Memory : public exMemory
{
public:
	explicit Memory();
	explicit Memory(const std::string& name);
	explicit Memory(const std::string& name, const DWORD& dwAccess);

public:
	virtual bool Attach(const std::string& name, const DWORD& dwAccess = PROCESS_ALL_ACCESS) override;
	virtual bool Detach() override;
	virtual void update() override;
};
inline Memory g_Memory = Memory("notepad.exe");
