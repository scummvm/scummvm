#pragma once

class DebugSymbolManager
{
public:
	static void create();
	~DebugSymbolManager();

	bool getProcName(void* proc, std::string& name);

protected:
	DebugSymbolManager(HANDLE hProcess);

private:
	HANDLE hProcess_;

private:
	DWORD64 getProcAddr(void* proc)
	{
		const char* mem = static_cast<const char*>(proc);
		if(char(mem[0]) != char(0xe9))
			return reinterpret_cast<DWORD64>(proc);

		int shift = *(int*)(mem + 1);
		return (DWORD64)(mem + 5 + shift);
	}
};

extern DebugSymbolManager* debugSymbolManager;