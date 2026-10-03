// https://github.com/NightFyre/exMemory/tree/main

#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <memory>
#include <vector>
#include <string>

//	architecture type helpers
#ifdef _WIN64
typedef unsigned __int64  i64_t;
#else
typedef unsigned int i64_t;
#endif

//	fwd declare helpers
inline static std::string ToLower(const std::string& input);
inline static std::string ToUpper(const std::string& input);
inline static std::string ToString(const std::wstring& input);
inline static std::wstring ToWString(const std::string& input);

//	general process information
typedef struct PROCESSINFO64
{
	bool							bAttached;								//	set when attached to a process
	DWORD							dwAccessLevel{ 0 };						//	access rights to process ( if attached )
	HWND							hWnd{ 0 };								//	handle to process window
	HANDLE							hProc{ INVALID_HANDLE_VALUE };			//	handle to process		
	DWORD							dwPID{ 0 };								//	process id
	i64_t							dwModuleBase{ 0 };						//	module base address
	std::string						mProcName{ "" };						//	process name
	std::string						mProcPath{ "" };						//	process path
	std::string						mWndwTitle{ "" };						//	process window title
} PROCESSINFO32, procInfo_t;

//	general module information
typedef struct MODULEINFO64
{
	DWORD							dwPID{ 0 };							//	owning process id
	i64_t							dwModuleBase{ 0 };					//	module base address in process
	std::string						mModName{ "" };						//	module name
} MODULEINFO32, modInfo_t;

typedef struct WINDOWINFO64
{
	HWND hWnd{ nullptr };

	std::string mTitle;
	std::string mClassName;

	int mWidth{ 0 };
	int mHeight{ 0 };

	bool bVisible{ false };
	bool bOwned{ false };
	bool bToolWindow{ false };

	LONG64 GetArea() const
	{
		return static_cast<LONG64>(mWidth) * mHeight;
	}
}WINDOWINFO32, wndwInfo_t;

//	assembly opcode index for ripping an offset from an instruction in memory
enum class EASM : int
{
	ASM_MOV = 0,		//	mov rax,[proc.exe+offset]	; 0x48 0x8B 0x05 ?? ?? ?? ??
	ASM_LEA,			//	lea rax,[proc.exe+offset]	; 0x48 0x8D 0x05 ?? ?? ?? ??
	ASM_CMP,			//	cmp rax,[proc.exe+offset]	; 0x48 0x3B 0x05 ?? ?? ?? ??
	ASM_CALL,			//	call proc.exe+offset		; 0xE8 ?? ?? ?? ??
	ASM_NULL
};

//	section headers index
enum class ESECTIONHEADERS : int
{
	SECTION_TEXT = 0,		//	.text
	SECTION_DATA,			//	.data
	SECTION_RDATA,			//	.rdata
	SECTION_IMPORT,			//	IMPORTS TABLE
	SECTION_EXPORT,			//	EXPORTS TABLE
	SECTION_NULL
};

//	injection type index
enum class EINJECTION : int
{
	INJECT_LOADLIBRARY = 0,
	INJECT_MANUAL,
	INJECT_NULL
};

/*
*
*
*/
class exMemory
{
	/*//--------------------------\\
			CONSTRUCTORS
	*/
public:
	explicit inline exMemory() = default;	//	 default constructor | does nothing
	explicit inline exMemory(const std::string& name);	//	attaches to process with all access rights
	explicit inline exMemory(const std::string& name, const DWORD& dwAccess);	//	attaches to process with specified access rights
	inline ~exMemory() noexcept;	//	destructor | detaches from process if attached

	/*//--------------------------\\
			INSTANCE MEMBERS
	*/
public:
	bool						bAttached;	//	attached to a process
	double						mFrequency;	//	update frequency in ms

protected:
	procInfo_t					vmProcess;	//	attached process information
	std::vector<procInfo_t>		vmProcList;	//	active process list
	std::vector<modInfo_t>		vmModList;	//	module list for attached process

	/*//--------------------------\\
			INSTANCE METHODS
	*/
public:

	/* attempts to attach to a process by name
	* virtualized to allow for custom behavior in derived classes
	*/
	virtual inline bool Attach(const std::string& name, const DWORD& dwAccess = PROCESS_ALL_ACCESS);

	/* detaches from the attached process
	* virtualized to allow for custom behavior in derived classes
	*/
	virtual inline bool Detach();

	/* verifies attached process is active & updates processinfo structure when needed
	* virtualized to allow for custom behavior in derived classes
	*/
	virtual inline void update();


public:
	/* returns the process information structure
	* see: procInfo_t or PROCESSINFO64
	*/
	inline const procInfo_t& GetProcessInfo() const { return vmProcess; }

	/* returns an updated process list */
	inline const std::vector<procInfo_t>& GetProcessList() const { return vmProcList; }

	/* returns a list containing all modules in the attached process */
	inline const std::vector<modInfo_t>& GetModuleList() const { return vmModList; }


protected:

	/* helper method to determine if the current memory instance is attached to a process for handling various memory operations */
	inline const bool IsValidInstance() noexcept { return bAttached && vmProcess.bAttached && vmProcess.hProc != INVALID_HANDLE_VALUE; }


public:

	/* reads a memory into a buffer at the specified address in the attached process
	* returns true if all bytes were read
	*/
	inline bool ReadMemory(const i64_t& addr, void* buffer, const DWORD& szRead);

	/* attempts to write bytes in the attached process
	* returns true if all bytes were written successfully
	*/
	inline bool WriteMemory(const i64_t& addr, const void* buffer, const DWORD& szWrite);

	/* reads a continguous string in at the specified address in the attached process
	* returns true if the string was successfully read
	*/
	inline bool ReadString(const i64_t& addr, std::string& string, const DWORD& szString = MAX_PATH);

	/* reads a chain of pointers in the attached process to find an address in memory
	* returns the address if found
	*/
	inline i64_t ReadPointerChain(const i64_t& addr, std::vector<unsigned int>& offsets, i64_t* lpResult);

	/* attempts to patch a sequence of bytes in the attached process
	* returns true if successful
	*/
	inline bool PatchMemory(const i64_t& addr, const void* buffer, const DWORD& szWrite);

	/* gets an address relative to the input named module base address */
	inline i64_t GetAddress(const unsigned int& offset, const std::string& modName = "");
	inline bool GetAddress(const unsigned int& offset, i64_t* lpResult, const std::string& modName = "");

	/* attempts to find a pattern in the attached process
	* returns the address of pattern if found
	*/
	inline i64_t FindPattern(const std::string& signature);
	inline i64_t FindPattern(const std::string& signature, int padding);
	inline i64_t FindPattern(const std::string& signature, int padding, EASM instruction);

	/* attempts to find a section header address in the attached process*/
	inline i64_t GetSectionHeader(const ESECTIONHEADERS& section, i64_t* lpResult);

	/* attempts to obtain the address of a function located in the atteched processes export directory */
	inline i64_t GetProcAddress(const std::string& fnName, i64_t* lpResult);

	/* attempts to inject a module from disk into the attached process */
	inline bool LoadLibraryInject(const std::string& dllPath);


public:

	/* template read memory with szRead parameter
	* NOTE: does not work with strings
	*/
	template<typename T>
	auto Read(i64_t addr, DWORD szRead) noexcept -> T
	{
		T result{};
		ReadMemory(addr, &result, szRead);
		return result;
	}

	/* template read memory
	* NOTE: does not work with strings
	*/
	template<typename T>
	auto Read(i64_t addr) noexcept -> T
	{
		T result{};
		ReadMemory(addr, &result, sizeof(T));
		return result;
	}

	/* template write memory with szPatch param */
	template<typename T>
	auto Write(i64_t addr, T patch, DWORD szPatch) noexcept -> bool { return WriteMemory(addr, &patch, szPatch); }

	/* template write memory */
	template<typename T>
	auto Write(i64_t addr, T patch) noexcept -> bool { return WriteMemory(addr, &patch, sizeof(T)); }


	/*//--------------------------\\
			STATIC METHODS
	*/

public:	//	methods for directly attaching to a process

	/* attempts to attach to the named process with desired access level and returns a process information structure */
	static inline bool AttachEx(const std::string& name, procInfo_t* lpProcess, const DWORD& dwDesiredAccess);

	/* detaches from the attached process by freeing any opened handles to free the process information structure */
	static inline bool DetachEx(procInfo_t& pInfo);


public:	//	methods for retrieving information on a process by name , are somewhat slow and should not be used constantly. consider caching information if needed.

	/* attempts to retrieve a process id by name
	* utilizes FindProcessEx which iterates through ALL processes information before again searching through the procInfo list to return a match ( if any )
	*/
	static inline bool GetProcID(const std::string& procName, DWORD* outPID);

	/* attempts to obtain the module base address for the specified process name
	* utilizes FindProcessEx which iterates through ALL processes information before again searching through the procInfo list to return a match ( if any )
	*/
	static inline bool GetModuleBaseAddress(const std::string& procName, i64_t* lpResult, const std::string& modName = "");

	/* attempts to obtain information on a process without opening a handle to it
	* utilizes FindProcessEx which iterates through ALL processes information before again searching through the procInfo list to return a match ( if any )
	*/
	static inline bool GetProcInfo(const std::string& name, procInfo_t* lpout);

	/* determines if the specified name exists in the active process directory
	* utilizes FindProcessEx which iterates through ALL processes information before again searching through the procInfo list to return a match ( if any )
	*/
	static inline bool IsProcessRunning(const std::string& name);


public:	//	methods for obtaining info on active processes

	/* obtains a list of all active processes on the machine that contains basic information on a process without requiring a handle
	* ref: https://learn.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
	*/
	static inline bool GetActiveProcessesEx(std::vector<procInfo_t>& procList);

	/* obtains a list of all modules loaded in the attached process */
	static inline bool GetProcessModulesEx(const DWORD& dwPID, std::vector< modInfo_t>& moduleList);

	/* obtains all top-level windows belonging to the specified process */
	static inline bool GetProcessWindowsEx(const DWORD& dwPID, std::vector<wndwInfo_t>& windowList);

	/* attempts to determine the primary window belonging to the specified process */
	static inline HWND GetProcessWindowEx(const DWORD& dwPID);

	/* gets info on a process by name , can be extended to attach to the process if found
	* utilizes GetActiveProcesses method which is somewhat slow as it obtains ALL processes before returning
	*/
	static inline bool FindProcessEx(const std::string& procName, procInfo_t* procInfo, const bool& bAttach, const DWORD& dwDesiredAccess);

	/* attempts to find a module by name located in the attached process and returns it's base address */
	static inline bool FindModuleEx(const std::string& procName, const std::string& modName, modInfo_t* lpResult);

public:	//	basic memory operations

	/* attempts to read memory at the specified address from the target process */
	static inline bool ReadMemoryEx(const HANDLE& hProc, const i64_t& addr, void* buffer, size_t szRead);

	/* attempts to write bytes to the specified address in memory from the target process */
	static inline bool WriteMemoryEx(const HANDLE& hProc, const i64_t& addr, LPVOID buffer, DWORD szWrite);

	/* attempts to read a string at the specified address in memory from the target process */
	static inline bool ReadStringEx(const HANDLE& hProc, const i64_t& addr, const size_t& szString, std::string* lpResult);

	/* attempts to return an address located in memory via chain of offsets */
	static inline bool ReadPointerChainEx(const HANDLE& hProc, const i64_t& addr, const std::vector<unsigned int>& offsets, i64_t* lpResult);

	/* attempts to patch a sequence of bytes in the target process */
	static inline bool PatchMemoryEx(const HANDLE& hProc, const i64_t& addr, const void* buffer, const DWORD& szWrite);

public:	//	advanced methods for obtaining information on a process which requires a handle

	/* attempts to find a module by name located in the attached process and returns it's base address */
	static inline bool GetModuleAddressEx(const HANDLE& hProc, const std::string& moduleName, i64_t* lpResult);

	/* attempts to return the address of a section header by index
	* ref: https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_nt_headers64
	* ref: https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_file_header
	* ref: https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_optional_header64
	* ref: https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_section_header
	*/
	static inline bool GetSectionHeaderAddressEx(const HANDLE& hProc, const std::string& moduleName, const ESECTIONHEADERS& section, i64_t* lpResult, size_t* szImage);
	static inline bool GetSectionHeaderAddressEx(const HANDLE& hProc, const i64_t& dwModule, const ESECTIONHEADERS& section, i64_t* lpResult, size_t* szImage);

	/* attempts to return an address located in memory via pattern scan. can be extended to extract bytes from an instruction
	* modifed version of -> https://www.unknowncheats.me/forum/3019469-post2.html
	*/
	static inline bool FindPatternEx(const HANDLE& hProc, const std::string& moduleName, const std::string& signature, i64_t* lpResult, int padding, EASM instruction);
	static inline bool FindPatternEx(const HANDLE& hProc, const i64_t& dwModule, const std::string& signature, i64_t* lpResult, int padding, EASM instruction);

	/* attempts to find an exported function by name and return the it's rva
	* https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_data_directory
	*/
	static inline bool GetProcAddressEx(const HANDLE& hProc, const std::string& moduleName, const std::string& fnName, i64_t* lpResult);
	static inline bool GetProcAddressEx(const HANDLE& hProc, const i64_t& dwModule, const std::string& fnName, i64_t* lpResult);


public:	//	injection operations 

	/* injects a module (from disk) into the target process using LoadLibrary */
	static inline bool LoadLibraryInjectorEx(const HANDLE& hProc, const std::string& dllPath);


public:	//	template methods

	/* template read memory with szRead parameter
	* NOTE: does not work with strings
	*/
	template<typename T>
	static auto ReadEx(const HANDLE& hProc, const i64_t& addr, DWORD szRead) noexcept -> T
	{
		T result{};
		ReadMemoryEx(hProc, addr, &result, szRead);
		return result;
	}

	/* template read memory
	* NOTE: does not work with strings
	*/
	template<typename T>
	static auto ReadEx(const HANDLE& hProc, const i64_t& addr) noexcept -> T
	{
		T result{};
		ReadMemoryEx(hProc, addr, &result, sizeof(T));
		return result;
	}

	/* template write memory with szPatch param */
	template<typename T>
	static auto WriteEx(const HANDLE& hProc, const i64_t& addr, T patch, DWORD szPatch) noexcept -> bool { return WriteMemoryEx(hProc, addr, &patch, szPatch); }

	/* template write memory */
	template<typename T>
	static auto WriteEx(const HANDLE& hProc, const i64_t& addr, T patch) noexcept -> bool { return WriteMemoryEx(hProc, addr, &patch, sizeof(T)); }



	/*//--------------------------\\
			TOOL METHODS
	*/
protected:
	struct EnumWindowData
	{
		unsigned int procId;
		HWND hwnd;
		LONG64 area{ 0 };
		std::vector<wndwInfo_t>* windowList{ 0 };
	};

	/* callback for EnumWindows to find the maine process window
	* ref: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enumwindows
	*/
	static inline BOOL CALLBACK EnumProcessWindowsEx(HWND handle, LPARAM lParam);
};


//-------------------------------------------------------------------------------------------------
//
//										CONSTRUCTORS
//
//-------------------------------------------------------------------------------------------------

exMemory::exMemory(const std::string& name)
{
	bAttached = exMemory::Attach(name, PROCESS_ALL_ACCESS);
}

exMemory::exMemory(const std::string& name, const DWORD& dwAccess)
{
	bAttached = exMemory::Attach(name, dwAccess);
}

exMemory::~exMemory()
{
	Detach();	//	close handles and free resources
}


//-------------------------------------------------------------------------------------------------
//
//										INSTANCE METHODS								
//
//-------------------------------------------------------------------------------------------------


bool exMemory::Attach(const std::string& name, const DWORD& dwAccess)
{
	procInfo_t proc;
	if (!AttachEx(name, &proc, dwAccess))
		return false;

	vmProcess = proc;

	return vmProcess.bAttached;
}

bool exMemory::Detach()
{
	return DetachEx(vmProcess);
}

void exMemory::update()
{
	const bool& bAttched = vmProcess.bAttached;	//	is instance attached to a process ?

	//	check if attached process is running
	if (!IsProcessRunning(vmProcess.mProcName))
	{
		Detach();	//	close handles and free resources if not already done ( safe to call multiple times if nothing is attached )
		return;
	}

	//	attached process is running, update process information
}


//-------------------------------------------------------------------------------------------------
//
//										INSTANCE METHODS	( MEMORY OPERATIONS )							
//
//-------------------------------------------------------------------------------------------------

bool exMemory::ReadMemory(const i64_t& addr, void* buffer, const DWORD& szRead)
{
	if (!IsValidInstance())
		return false;

	return ReadMemoryEx(vmProcess.hProc, addr, buffer, szRead);
}

bool exMemory::ReadString(const i64_t& addr, std::string& string, const DWORD& szString)
{
	if (!IsValidInstance())
		return false;

	return ReadStringEx(vmProcess.hProc, addr, szString, &string);
}

bool exMemory::WriteMemory(const i64_t& addr, const void* buffer, const DWORD& szWrite)
{
	if (!IsValidInstance())
		return false;

	return WriteMemoryEx(vmProcess.hProc, addr, LPVOID(buffer), szWrite);
}

bool exMemory::PatchMemory(const i64_t& addr, const void* buffer, const DWORD& szWrite)
{
	if (!IsValidInstance())
		return false;

	return PatchMemoryEx(vmProcess.hProc, addr, buffer, szWrite);
}

i64_t exMemory::ReadPointerChain(const i64_t& addr, std::vector<unsigned int>& offsets, i64_t* lpResult)
{
	if (!IsValidInstance())
		return 0;

	if (!ReadPointerChainEx(vmProcess.hProc, addr, offsets, lpResult))
		return 0;

	return *lpResult;
}

i64_t exMemory::GetAddress(const unsigned int& offset, const std::string& modName)
{
	i64_t result = 0;
	if (!GetAddress(offset, &result, modName))
		return 0;

	return result;
}

bool exMemory::GetAddress(const unsigned int& offset, i64_t* lpResult, const std::string& modName)
{
	i64_t result = 0;
	if (!IsValidInstance())
		return 0;

	if (modName.empty())
		result = vmProcess.dwModuleBase + offset;
	else if (!GetModuleAddressEx(vmProcess.hProc, modName, lpResult))
		return false;

	*lpResult = result;

	return result > 0;
}

i64_t exMemory::FindPattern(const std::string& signature)
{
	if (!IsValidInstance())
		return 0;

	i64_t result = 0;
	if (!FindPatternEx(vmProcess.hProc, vmProcess.dwModuleBase, signature, &result, 0, EASM::ASM_NULL))
		return 0;

	return result;
}

i64_t exMemory::FindPattern(const std::string& signature, int padding)
{
	if (!IsValidInstance())
		return 0;

	i64_t result = 0;
	if (!FindPatternEx(vmProcess.hProc, vmProcess.dwModuleBase, signature, &result, padding, EASM::ASM_NULL))
		return 0;

	return result;
}

i64_t exMemory::FindPattern(const std::string& signature, int padding, EASM instruction)
{
	if (!IsValidInstance())
		return 0;

	i64_t result = 0;
	if (!FindPatternEx(vmProcess.hProc, vmProcess.dwModuleBase, signature, &result, padding, instruction))
		return 0;

	return result;
}

i64_t exMemory::GetSectionHeader(const ESECTIONHEADERS& section, i64_t* lpResult)
{
	if (!IsValidInstance())
		return 0;

	if (GetSectionHeaderAddressEx(vmProcess.hProc, vmProcess.dwModuleBase, section, lpResult, nullptr))
		return 0;

	return *lpResult;
}

i64_t exMemory::GetProcAddress(const std::string& fnName, i64_t* lpResult)
{
	if (!IsValidInstance())
		return 0;

	if (!GetProcAddressEx(vmProcess.hProc, vmProcess.dwModuleBase, fnName, lpResult))
		return 0;

	return *lpResult;
}

bool exMemory::LoadLibraryInject(const std::string& dllPath)
{
	if (!IsValidInstance())
		return false;

	return LoadLibraryInjectorEx(vmProcess.hProc, dllPath);
}


//-------------------------------------------------------------------------------------------------
//
// 									STATIC METHODS
//
//-------------------------------------------------------------------------------------------------

bool exMemory::AttachEx(const std::string& name, procInfo_t* lpProcess, const DWORD& dwDesiredAccess)
{
	return FindProcessEx(name, lpProcess, true, dwDesiredAccess);
}

bool exMemory::DetachEx(procInfo_t& pInfo)
{
	bool result{ true };

	if (pInfo.bAttached && pInfo.hProc != INVALID_HANDLE_VALUE)
		CloseHandle(pInfo.hProc);	//	close handle to process

	pInfo = procInfo_t();	//	clear process information

	return result;
}


//-------------------------------------------------------------------------------------------------
//
// 									STATIC METHODS ( PROCESS INFORMATION )
//
//-------------------------------------------------------------------------------------------------

bool exMemory::GetProcID(const std::string& procName, DWORD* outPID)
{
	procInfo_t proc;
	if (!GetProcInfo(procName, &proc))
		return false;

	*outPID = proc.dwPID;

	return proc.dwPID > 0;
}

bool exMemory::GetModuleBaseAddress(const std::string& procName, i64_t* lpResult, const std::string& modName)
{
	if (!modName.empty())
	{
		modInfo_t mod;
		if (!FindModuleEx(procName, modName, &mod))
			return false;

		*lpResult = mod.dwModuleBase;

		return mod.dwModuleBase > 0;
	}

	procInfo_t proc;
	if (!GetProcInfo(procName, &proc))
		return false;

	*lpResult = proc.dwModuleBase;

	return proc.dwModuleBase > 0;
}

bool exMemory::GetProcInfo(const std::string& name, procInfo_t* lpResult)
{
	return FindProcessEx(name, lpResult, false, NULL);
}

bool exMemory::IsProcessRunning(const std::string& name)
{
	return FindProcessEx(name, nullptr, false, NULL);
}


//-------------------------------------------------------------------------------------------------
//
// 									STATIC METHODS ( BASIC MEMORY OPERATIONS )
//
//-------------------------------------------------------------------------------------------------

bool exMemory::ReadMemoryEx(const HANDLE& hProc, const i64_t& addr, void* lpResult, size_t szRead)
{
	SIZE_T size_read{};
	return ReadProcessMemory(hProc, LPCVOID(addr), lpResult, szRead, &size_read) && szRead == size_read;
}

bool exMemory::WriteMemoryEx(const HANDLE& hProc, const i64_t& addr, LPVOID buffer, DWORD szWrite)
{
	SIZE_T size_write{};
	return WriteProcessMemory(hProc, LPVOID(addr), buffer, szWrite, &size_write) && szWrite == size_write;
}

bool exMemory::ReadStringEx(const HANDLE& hProc, const i64_t& addr, const size_t& szString, std::string* lpResult)
{
	size_t bytes_read{};
	char buf[MAX_PATH]{};
	if (!ReadMemoryEx(hProc, addr, buf, szString))
		return false;

	*lpResult = std::string(buf);

	return true;
}

bool exMemory::ReadPointerChainEx(const HANDLE& hProc, const i64_t& addr, const std::vector<unsigned int>& offsets, i64_t* lpResult)
{
	i64_t result = addr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		result = ReadEx<i64_t>(hProc, result);
		result += offsets[i];
	}

	*lpResult = result;

	return result > 0;
}

bool exMemory::PatchMemoryEx(const HANDLE& hProc, const i64_t& addr, const void* buffer, const DWORD& szWrite)
{
	//	store original protection & set new protection
	DWORD oldprotect;
	if (!VirtualProtectEx(hProc, LPVOID(addr), szWrite, PAGE_EXECUTE_READWRITE, &oldprotect))
		return false;

	bool result = WriteProcessMemory(hProc, LPVOID(addr), buffer, szWrite, nullptr);			//	write bytes to address
	VirtualProtectEx(hProc, LPVOID(addr), szWrite, oldprotect, &oldprotect);					//	restore memory protection
	return result;
}


//-------------------------------------------------------------------------------------------------
//
// 									STATIC METHODS ( PROCESS & MODULE ENUMERATION )
//
//-------------------------------------------------------------------------------------------------

bool exMemory::GetActiveProcessesEx(std::vector<procInfo_t>& list)
{
	//	snapshot processes
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return FALSE;

	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);
	if (!Process32Next(hSnap, &procEntry))
	{
		CloseHandle(hSnap);
		return FALSE;
	}

	//  iterate through all processes
	std::vector<procInfo_t> active_process_list;
	do
	{
		const DWORD procID = procEntry.th32ProcessID;
		if (!procID)
			continue;

		//	snapshot modules
		HANDLE modSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPPROCESS, procID);
		if (modSnap == INVALID_HANDLE_VALUE)
			continue;

		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (!Module32First(modSnap, &modEntry))
		{
			CloseHandle(modSnap);
			continue;
		}

		//	 iterate through all modules
		do
		{
			//	compare module names
			if (_wcsicmp(modEntry.szModule, procEntry.szExeFile))
				continue;

			//	module found
			procInfo_t proc;
			proc.mProcName = ToString(procEntry.szExeFile);      //  process name
			proc.mProcPath = ToString(modEntry.szExePath);       //  process path
			proc.dwPID = procID;											   //  process ID
			proc.dwModuleBase = i64_t(modEntry.modBaseAddr);                   //  module base address

			//  push back process to list
			active_process_list.push_back(proc);

			break;  //  get next process information

		} while (Module32Next(modSnap, &modEntry));

		CloseHandle(modSnap);

	} while (Process32Next(hSnap, &procEntry));

	CloseHandle(hSnap);

	list = active_process_list;

	return list.size() > 0;
}

bool exMemory::GetProcessModulesEx(const DWORD& dwPID, std::vector<modInfo_t>& list)
{
	//	snapshot modules
	HANDLE modSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPPROCESS, dwPID);
	if (modSnap == INVALID_HANDLE_VALUE)
		return false;

	MODULEENTRY32 modEntry;
	modEntry.dwSize = sizeof(modEntry);
	if (!Module32First(modSnap, &modEntry))
	{
		CloseHandle(modSnap);
		return false;
	}

	//	 iterate through all modules
	std::vector<modInfo_t> active_module_list;
	do
	{
		//	module found
		modInfo_t mod;
		mod.dwPID = dwPID;												   //  process ID
		mod.dwModuleBase = i64_t(modEntry.modBaseAddr);					   //  module base address
		mod.mModName = ToString(modEntry.szModule);		   //  module name

		//  push back module to list
		active_module_list.push_back(mod);

	} while (Module32Next(modSnap, &modEntry));

	CloseHandle(modSnap);

	list = active_module_list;

	return list.size() > 0;
}

HWND exMemory::GetProcessWindowEx(const DWORD& dwPID)
{
	std::vector<wndwInfo_t> windows;

	if (!GetProcessWindowsEx(dwPID, windows))
		return nullptr;

	const wndwInfo_t* best = nullptr;

	for (const auto& window : windows)
	{
		if (!window.bVisible)
			continue;

		if (window.bOwned)
			continue;

		if (window.bToolWindow)
			continue;

		if (window.mWidth <= 0 || window.mHeight <= 0)
			continue;

		if (!best || window.GetArea() > best->GetArea())
			best = &window;
	}

	return best ? best->hWnd : nullptr;
}

bool exMemory::GetProcessWindowsEx(const DWORD& dwPID, std::vector<wndwInfo_t>& windowList)
{
	windowList.clear();

	if (!dwPID)
		return false;

	EnumWindowData data{};
	data.procId = dwPID;
	data.windowList = &windowList;

	if (!EnumWindows(EnumProcessWindowsEx, reinterpret_cast<LPARAM>(&data)))
		return false;

	return !windowList.empty();

}

bool exMemory::FindProcessEx(const std::string& procName, procInfo_t* procInfo, const bool& bAttach, const DWORD& dwDesiredAccess)
{
	bool result = false;
	const auto& input = ToLower(procName);

	//	create process snapshot
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return FALSE;

	//	get first entry
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);
	if (!Process32Next(hSnap, &procEntry))
	{
		CloseHandle(hSnap);
		return FALSE;
	}

	//  iterate through all processes
	do
	{
		//	compare names
		if (ToLower(ToString(procEntry.szExeFile)) != input)
			continue;

		//	snapshot modules
		HANDLE modSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPPROCESS, procEntry.th32ProcessID);
		if (modSnap == INVALID_HANDLE_VALUE)
			break;

		// get first entry
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (!Module32First(modSnap, &modEntry))
		{
			CloseHandle(modSnap);
			break;
		}

		//	module found
		procInfo_t proc;
		proc.mProcName = ToString(procEntry.szExeFile);      //  process name
		proc.mProcPath = ToString(modEntry.szExePath);       //  process path
		proc.dwPID = procEntry.th32ProcessID;				//  process ID
		proc.dwModuleBase = i64_t(modEntry.modBaseAddr);	//  module base address
		proc.dwAccessLevel = dwDesiredAccess;				//  desired access level

		//  attempt to get main process window
		proc.hWnd = GetProcessWindowEx(proc.dwPID);

		//  Get window title
		char buffer[MAX_PATH];
		if (proc.hWnd && GetWindowTextA(proc.hWnd, buffer, MAX_PATH))
			proc.mWndwTitle = std::string(buffer);

		//  open handle to process
		if (bAttach && dwDesiredAccess > 0)
		{
			proc.hProc = OpenProcess(proc.dwAccessLevel, false, proc.dwPID);

			proc.bAttached = proc.hProc != INVALID_HANDLE_VALUE;
		}

		*procInfo = proc;

		result = true;

		CloseHandle(modSnap);

		break;
	} while (Process32Next(hSnap, &procEntry));

	CloseHandle(hSnap);

	return result;
}

bool exMemory::FindModuleEx(const std::string& procName, const std::string& modName, modInfo_t* lpResult)
{
	const auto& proc_cmp = ToLower(procName);
	const auto& mod_cmp = ToLower(modName);
	bool bFound{ false };
	modInfo_t modInfo;

	//	snapshot processes
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return FALSE;

	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);
	if (!Process32Next(hSnap, &procEntry))
	{
		CloseHandle(hSnap);
		return FALSE;
	}

	//  iterate through all processes
	do
	{
		//	compare process names
		if (ToLower(ToString(procEntry.szExeFile)) != proc_cmp)
			continue;

		//	snapshot modules
		HANDLE modSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPPROCESS, procEntry.th32ProcessID);
		if (modSnap == INVALID_HANDLE_VALUE)
			break;

		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (!Module32First(modSnap, &modEntry))
		{
			CloseHandle(modSnap);
			break;
		}

		//	 iterate through all modules
		do
		{
			//	compare module names
			if (ToLower(ToString(modEntry.szModule)) != mod_cmp)
				continue;

			//	module found
			bFound = true;

			//	get module properties
			modInfo.dwModuleBase = i64_t(modEntry.modBaseAddr);                   //  module base address
			modInfo.dwPID = procEntry.th32ProcessID;                              //  process ID
			modInfo.mModName = ToString(modEntry.szModule);		//  module name

			//	pass ref
			*lpResult = modInfo;

			break;

		} while (Module32Next(modSnap, &modEntry));

		CloseHandle(modSnap);

		break;

	} while (Process32Next(hSnap, &procEntry));

	CloseHandle(hSnap);

	return bFound;
}


//-------------------------------------------------------------------------------------------------
//
// 									STATIC METHODS ( ADVANCED MEMORY OPERATIONS )
//
//-------------------------------------------------------------------------------------------------

bool exMemory::GetModuleAddressEx(const HANDLE& hProc, const std::string& moduleName, i64_t* lpResult)
{
	DWORD cbNeeded;
	HMODULE modules[1024];
	if (!EnumProcessModulesEx(hProc, modules, sizeof(modules), &cbNeeded, LIST_MODULES_ALL))
		return false;

	const auto szModule = cbNeeded / sizeof(HMODULE);
	for (int i = 0; i < szModule; i++)
	{
		wchar_t modName[MAX_PATH];
		if (!GetModuleBaseName(hProc, modules[i], modName, sizeof(modName) / sizeof(wchar_t)))
			continue;

		if (ToLower(ToString(modName)) != moduleName)
			continue;

		*lpResult = reinterpret_cast<i64_t>(modules[i]);

		return true;
	}

	return false;
}

bool exMemory::GetSectionHeaderAddressEx(const HANDLE& hProc, const std::string& moduleName, const ESECTIONHEADERS& section, i64_t* lpResult, size_t* szImage)
{
	i64_t dwModuleBase = 0;
	if (!GetModuleAddressEx(hProc, moduleName, &dwModuleBase) || !dwModuleBase)
		return false;

	return GetSectionHeaderAddressEx(hProc, dwModuleBase, section, lpResult, szImage);
}

bool exMemory::GetSectionHeaderAddressEx(const HANDLE& hProc, const i64_t& dwModule, const ESECTIONHEADERS& section, i64_t* lpResult, size_t* szImage)
{
	//	get segment title
	std::string segment;
	switch (section)
	{
	case ESECTIONHEADERS::SECTION_TEXT: { segment = ".text"; break; }
	case ESECTIONHEADERS::SECTION_DATA: { segment = ".data"; break; }
	case ESECTIONHEADERS::SECTION_RDATA: { segment = ".rdata"; break; }
	case ESECTIONHEADERS::SECTION_IMPORT: { segment = ".idata"; break; }
	case ESECTIONHEADERS::SECTION_EXPORT: { segment = ".edata"; break; }
	default: return false;
	}
	if (segment.empty())	//	segment title not captured ?? 
		return false;

	//	get dos header
	const auto& image_dos_header = ReadEx<IMAGE_DOS_HEADER>(hProc, dwModule);
	if (image_dos_header.e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	//	get nt headers
	const auto& e_lfanew = dwModule + image_dos_header.e_lfanew;
	const auto& image_nt_headers = ReadEx<IMAGE_NT_HEADERS>(hProc, e_lfanew);
	if (image_nt_headers.Signature != IMAGE_NT_SIGNATURE)
		return false;

	//	Get section
	size_t section_size = 0;
	i64_t section_base = 0;
	const auto& image_section_header = e_lfanew + sizeof(IMAGE_NT_HEADERS);
	IMAGE_SECTION_HEADER section_headers_base = ReadEx<IMAGE_SECTION_HEADER>(hProc, image_section_header);
	for (int i = 0; i < image_nt_headers.FileHeader.NumberOfSections; ++i)
	{
		if (strncmp(reinterpret_cast<const char*>(section_headers_base.Name), segment.c_str(), segment.size()) != 0)
		{
			section_headers_base = ReadEx<IMAGE_SECTION_HEADER>(hProc, image_section_header + (sizeof(IMAGE_SECTION_HEADER) * i));
			continue;
		}

		section_base = dwModule + section_headers_base.VirtualAddress;
		section_size = section_headers_base.SizeOfRawData;
		break;
	}
	if (!section_base)
		return false;

	//	pass result
	*lpResult = section_base;
	*szImage = section_size;

	return true;
}

bool exMemory::FindPatternEx(const HANDLE& hProc, const std::string& moduleName, const std::string& signature, i64_t* lpResult, int padding, EASM instruction)
{
	i64_t dwModuleBase = 0;
	if (!GetModuleAddressEx(hProc, moduleName, &dwModuleBase) || !dwModuleBase)
		return false;

	return FindPatternEx(hProc, dwModuleBase, signature, lpResult, padding, instruction);
}

bool exMemory::FindPatternEx(const HANDLE& hProc, const i64_t& dwModule, const std::string& signature, i64_t* lpResult, int padding, EASM instruction)
{
	static auto pattern_to_byte = [](const char* pattern)
	{
		const auto start = const_cast<char*>(pattern);
		const auto end = const_cast<char*>(pattern) + strlen(pattern);

		auto bytes = std::vector<int>{};
		for (auto current = start; current < end; ++current)
		{
			if (*current == '?')
			{
				++current;
				bytes.push_back(-1);
			}
			else
			{
				bytes.push_back(strtoul(current, &current, 16));
			}
		}
		return bytes;
	};

	i64_t result = 0;

	//	Get .text segment
	i64_t section_base = 0;
	size_t section_size = 0;
	if (!GetSectionHeaderAddressEx(hProc, dwModule, ESECTIONHEADERS::SECTION_TEXT, &section_base, &section_size))
		return false;

	//	get pattern
	const auto pattern_bytes = pattern_to_byte(signature.c_str());
	const auto cbSize = pattern_bytes.size();
	const auto cbData = pattern_bytes.data();

	//	read section
	std::vector<unsigned __int8> scan_bytes(section_size);
	if (!ReadMemoryEx(hProc, section_base, scan_bytes.data(), scan_bytes.size()))
		return false;

	//	iterate through buffer & compare with pattern
	for (auto i = 0ul; i < section_size - cbSize; ++i)
	{
		bool found = true;
		for (auto j = 0ul; j < cbSize; ++j)
		{
			if (scan_bytes[i + j] != cbData[j] && cbData[j] != -1)
			{
				found = false;
				break;
			}
		}

		if (!found)
			continue;

		//	set result address
		auto address = section_base + i;

		//	apply optional padding
		address += padding;

		//	rip offset from instruction
		switch (instruction)
		{
		case EASM::ASM_NULL:
		{
			//	just return the address
			result = address;
			break;
		}
		case EASM::ASM_MOV: //	mov rax,[proc.exe+offset]	; 0x48 0x8B 0x05 ?? ?? ?? ??
		{
			const auto offset = ReadEx<int>(hProc, address + 3);
			result = (address + offset) + 7;	// 7 = sizeof instruction
			break;
		}
		case EASM::ASM_CALL: //	call proc.exe+offset		; 0xE8 ?? ?? ?? ??
		{
			const auto offset = ReadEx<int>(hProc, address + 1);
			result = (address + offset) + 5; 	// 5 = sizeof instruction
			break;
		}
		case EASM::ASM_LEA: //	lea rax,[proc.exe+offset]	; 0x48 0x8D 0x05 ?? ?? ?? ??
		{
			const auto offset = ReadEx<int>(hProc, address + 3);
			result = (address + offset) + 7;	// 7 = sizeof instruction
			break;
		}
		case EASM::ASM_CMP: //	cmp rax,[proc.exe+offset]	; 0x48 0x3B 0x05 ?? ?? ?? ??
		{
			const auto offset = ReadEx<int>(hProc, address + 2);
			result = (address + offset) + 6;	// 6 = sizeof instruction
			break;
		}

		default:
			return false;
		}

		break;
	}


	*lpResult = result;

	return result > 0;
}

bool exMemory::GetProcAddressEx(const HANDLE& hProc, const std::string& moduleName, const std::string& fnName, i64_t* lpResult)
{
	i64_t dwModuleBase = 0;
	if (!GetModuleAddressEx(hProc, moduleName, &dwModuleBase) || !dwModuleBase)
		return false;

	return GetProcAddressEx(hProc, dwModuleBase, fnName, lpResult);
}

bool exMemory::GetProcAddressEx(const HANDLE& hProc, const i64_t& dwModule, const std::string& fnName, i64_t* lpResult)
{
	const auto& fnNameLower = ToLower(fnName);

	//	get image doe header
	const auto& image_dos_header = ReadEx<IMAGE_DOS_HEADER>(hProc, dwModule);
	if (image_dos_header.e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	//	get nt headers
	const auto& image_nt_headers = ReadEx<IMAGE_NT_HEADERS>(hProc, dwModule + image_dos_header.e_lfanew);
	if (image_nt_headers.Signature != IMAGE_NT_SIGNATURE
		|| image_nt_headers.OptionalHeader.NumberOfRvaAndSizes <= 0)
		return false;

	//	get export directory
	const auto& export_directory_va = image_nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + dwModule;
	const auto& export_directory = ReadEx<IMAGE_EXPORT_DIRECTORY>(hProc, export_directory_va);
	if (!export_directory.AddressOfNames || !export_directory.AddressOfFunctions || !export_directory.AddressOfNameOrdinals)
		return false;

	//	get address of *
	const auto& names_va = dwModule + export_directory.AddressOfNames;
	const auto& functions_va = dwModule + export_directory.AddressOfFunctions;
	const auto& ordinals_va = dwModule + export_directory.AddressOfNameOrdinals;
	for (int i = 0; i < export_directory.NumberOfNames; i++)
	{
		//	get address of name
		const auto& name_rva = ReadEx<DWORD>(hProc, names_va + (i * 0x4));
		const auto& name_va = name_rva + dwModule;

		//	read & compare name with input string
		std::string cmp;
		if (!ReadStringEx(hProc, name_va, MAX_PATH, &cmp))
			continue;

		//	compare strings
		if (fnNameLower != ToLower(cmp))
			continue;

		//	get function address
		const auto& name_ordinal = ReadEx<short>(hProc, ordinals_va + (i * 0x2));				//	get ordinal at the current index
		const auto& function_rva = ReadEx<DWORD>(hProc, functions_va + (name_ordinal * 0x4));	//	get function va from the ordinal index of the functions array

		//	pass result
		*lpResult = i64_t(function_rva + dwModule);

		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
//
// 									STATIC METHODS ( INJECTION OPERATIONS )
//
//-------------------------------------------------------------------------------------------------

bool exMemory::LoadLibraryInjectorEx(const HANDLE& hProc, const std::string& dllPath)
{
	//  allocate memory
	void* addr = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!addr)
		return false;

	//  write to memory
	if (!WriteProcessMemory(hProc, addr, dllPath.c_str(), dllPath.size() + 1, 0))
	{
		VirtualFreeEx(hProc, addr, 0, MEM_RELEASE);
		return false;
	}

	//  create thread
	HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, addr, 0, 0);
	if (!hThread)
	{
		VirtualFreeEx(hProc, addr, 0, MEM_RELEASE);
		return false;
	}

	//  close handle and return result
	CloseHandle(hThread);
	return true;
}


//-------------------------------------------------------------------------------------------------
//
// 									PRIVATE METHODS
//
//-------------------------------------------------------------------------------------------------

BOOL CALLBACK exMemory::EnumProcessWindowsEx(HWND window, LPARAM lParam)
{
	auto* data = reinterpret_cast<EnumWindowData*>(lParam);
	if (!data || !data->windowList)
		return FALSE;

	DWORD windowPID = 0;
	GetWindowThreadProcessId(window, &windowPID);
	if (windowPID != data->procId)
		return TRUE;

	wndwInfo_t info{};
	info.hWnd = window;
	info.bVisible = IsWindowVisible(window);
	info.bOwned = GetWindow(window, GW_OWNER) != nullptr;

	const LONG_PTR exStyle = GetWindowLongPtrW(window, GWL_EXSTYLE);
	info.bToolWindow = (exStyle & WS_EX_TOOLWINDOW) != 0;

	RECT clientRect{};
	if (GetClientRect(window, &clientRect))
	{
		info.mWidth = clientRect.right - clientRect.left;
		info.mHeight = clientRect.bottom - clientRect.top;
	}

	char buffer[MAX_PATH]{};
	if (GetWindowTextA(window, buffer, MAX_PATH))
		info.mTitle = buffer;
	memset(buffer, 0, sizeof(buffer));

	if (GetClassNameA(window, buffer, MAX_PATH))
		info.mClassName = buffer;

	data->windowList->push_back(std::move(info));

	return TRUE;
}


//-------------------------------------------------------------------------------------------------
//
// 									HELPER METHODS
//
//-------------------------------------------------------------------------------------------------

std::string ToLower(const std::string& input)
{
	std::string result;
	for (auto c : input)
		result += tolower(c);
	return result;
};

std::string ToUpper(const std::string& input)
{
	std::string result;
	for (auto c : input)
		result += toupper(c);
	return result;
};

std::string ToString(const std::wstring& input) { return std::string(input.begin(), input.end()); }

std::wstring ToWString(const std::string& input) { return std::wstring(input.begin(), input.end()); }