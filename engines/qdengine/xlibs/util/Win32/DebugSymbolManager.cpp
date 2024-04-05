#include "stdafx.h"
#include "DebugSymbolManager.h"
#include <DbgHelp.h>

#pragma comment (lib, "Dbghelp.lib")

DebugSymbolManager* debugSymbolManager = 0;

void DebugSymbolManager::create()
{
	if(!debugSymbolManager){
		static DebugSymbolManager instance(GetCurrentProcess());
		debugSymbolManager = &instance;
	}
}

DebugSymbolManager::DebugSymbolManager(HANDLE hProcess)
{
	hProcess_ = hProcess;

	DWORD symOptions = SymGetOptions();
	symOptions |= SYMOPT_LOAD_LINES;
	symOptions &= ~SYMOPT_UNDNAME;
	symOptions &= ~SYMOPT_DEFERRED_LOADS;
	SymSetOptions(symOptions);

	if(!SymInitialize(hProcess_, NULL, TRUE))
		hProcess_ = 0;
}

DebugSymbolManager::~DebugSymbolManager()
{
	if(hProcess_)
		SymCleanup(hProcess_);
}

bool DebugSymbolManager::getProcName(void* proc, std::string& name)
{
	name.clear();
	if(!hProcess_)
		return false;
	
	SYMBOL_INFO_PACKAGE pack;
	pack.si.SizeOfStruct = sizeof(SYMBOL_INFO);
	pack.si.MaxNameLen = MAX_SYM_NAME;
	
	DWORD64 dwDisplacement;
	if(!SymFromAddr(hProcess_, getProcAddr(proc), &dwDisplacement, &pack.si))
		return false;

	name = pack.si.Name;
	return true;
}