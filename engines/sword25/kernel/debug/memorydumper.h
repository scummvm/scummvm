// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef	BS_MEMORYDUMPER_H
#define BS_MEMORYDUMPER_H


// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <string>

#include "kernel/common.h"


// -----------------------------------------------------------------------------
// Class declaration
// -----------------------------------------------------------------------------

class BS_MemoryDumper
{
public:
	BS_MemoryDumper();
	~BS_MemoryDumper();

	bool WriteDump(_EXCEPTION_POINTERS * ExceptionInfoPtr, std::string & Filename);

private:
	typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
											 CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
											 CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
											 CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
	MINIDUMPWRITEDUMP m_MiniDumpWriteDump;
	HMODULE m_DbghelpDLL;
};

#endif
