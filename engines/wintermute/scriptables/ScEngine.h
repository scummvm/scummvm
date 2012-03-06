/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_SCENGINE_H
#define WINTERMUTE_SCENGINE_H

#include "engines/wintermute/persistent.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/BBase.h"
#include "engines/wintermute/wme_debugger.h"
#include "engines/wintermute/utils.h"
#include "engines/wintermute/PlatformSDL.h"

namespace WinterMute {

typedef byte *(*DLL_COMPILE_BUFFER)(byte  *Buffer, char *Source, uint32 BufferSize, uint32 *CompiledSize);
typedef byte *(*DLL_COMPILE_FILE)(char *Filename, uint32 *CompiledSize);
typedef void (*DLL_RELEASE_BUFFER)(unsigned char *Buffer);
typedef void (*DLL_SET_CALLBACKS)(CALLBACKS *callbacks, void *Data);
typedef int (*DLL_DEFINE_FUNCTION)(char *Name);
typedef int (*DLL_DEFINE_VARIABLE)(char *Name);

typedef void (*COMPILE_ERROR_CALLBACK)(int Line, char *Text , void *Data);
typedef void (*PARSE_ELEMENT_CALLBACK)(int Line, int Type, void *ElementData, void *Data);

#define MAX_CACHED_SCRIPTS 20
class CScScript;
class CScValue;
class CBObject;
class CBScriptHolder;
class CScEngine : public CBBase {
public:
	class CScCachedScript {
	public:
		CScCachedScript(char *Filename, byte *Buffer, uint32 Size) {
			m_Timestamp = CBPlatform::GetTime();
			m_Buffer = new byte[Size];
			if (m_Buffer) memcpy(m_Buffer, Buffer, Size);
			m_Size = Size;
			m_Filename = new char[strlen(Filename) + 1];
			if (m_Filename) strcpy(m_Filename, Filename);
		};

		~CScCachedScript() {
			if (m_Buffer) delete [] m_Buffer;
			if (m_Filename) delete [] m_Filename;
		};

		uint32 m_Timestamp;
		byte *m_Buffer;
		uint32 m_Size;
		char *m_Filename;
	};

	class CScBreakpoint {
	public:
		CScBreakpoint(const char *Filename) {
			m_Filename = NULL;
			CBUtils::SetString(&m_Filename, Filename);
		}

		~CScBreakpoint() {
			if (m_Filename) delete [] m_Filename;
			m_Lines.RemoveAll();
		}

		char *m_Filename;
		CBArray<int, int> m_Lines;
	};




public:
	HRESULT DbgSendScripts(IWmeDebugClient *Client);

	CBArray<CScBreakpoint *, CScBreakpoint *> m_Breakpoints;
	HRESULT AddBreakpoint(char *ScriptFilename, int Line);
	HRESULT RemoveBreakpoint(char *ScriptFilename, int Line);
	HRESULT RefreshScriptBreakpoints();
	HRESULT RefreshScriptBreakpoints(CScScript *Script);
	HRESULT SaveBreakpoints();
	HRESULT LoadBreakpoints();

	HRESULT ClearGlobals(bool IncludingNatives = false);
	HRESULT TickUnbreakable();
	HRESULT RemoveFinishedScripts();
	bool IsValidScript(CScScript *Script);
	void SetCompileErrorCallback(COMPILE_ERROR_CALLBACK Callback, void *Data);
	void SetParseElementCallback(PARSE_ELEMENT_CALLBACK Callback, void *Data);

	COMPILE_ERROR_CALLBACK m_CompileErrorCallback;
	void *m_CompileErrorCallbackData;

	PARSE_ELEMENT_CALLBACK m_ParseElementCallback;
	void *m_ParseElementCallbackData;

	HRESULT SetFileToCompile(char *Filename);
	char *m_FileToCompile;
	CScScript *m_CurrentScript;
	HRESULT ResumeAll();
	HRESULT PauseAll();
	void EditorCleanup();
	HRESULT ResetObject(CBObject *Object);
	HRESULT ResetScript(CScScript *Script);
	HRESULT EmptyScriptCache();
	byte *GetCompiledScript(char *Filename, uint32 *OutSize, bool IgnoreCache = false);
	DECLARE_PERSISTENT(CScEngine, CBBase)
	HRESULT Cleanup();
	int GetNumScripts(int *Running = NULL, int *Waiting = NULL, int *Persistent = NULL);
	HRESULT Tick();
	CScValue *m_Globals;
	CScScript *RunScript(char *Filename, CBScriptHolder *Owner = NULL);
	bool m_CompilerAvailable;
	HINSTANCE m_CompilerDLL;
	CScEngine(CBGame *inGame);
	virtual ~CScEngine();
	static void WINAPI AddError(void *Data, int Line, char *Text);
	static byte *WINAPI LoadFile(void *Data, char *Filename, uint32 *Size);
	static void  WINAPI CloseFile(void *Data, byte *Buffer);
	static void WINAPI ParseElement(void *Data, int Line, int Type, void *ElementData);
	DLL_COMPILE_BUFFER  ExtCompileBuffer;
	DLL_COMPILE_FILE    ExtCompileFile;
	DLL_RELEASE_BUFFER  ExtReleaseBuffer;
	DLL_SET_CALLBACKS   ExtSetCallbacks;
	DLL_DEFINE_FUNCTION ExtDefineFunction;
	DLL_DEFINE_VARIABLE ExtDefineVariable;

	CBArray<CScScript *, CScScript *> m_Scripts;

	void EnableProfiling();
	void DisableProfiling();
	bool IsProfiling() {
		return m_IsProfiling;
	}

	void AddScriptTime(const char *Filename, uint32 Time);
	void DumpStats();

private:

	CScCachedScript *m_CachedScripts[MAX_CACHED_SCRIPTS];
	bool m_IsProfiling;
	uint32 m_ProfilingStartTime;

	typedef std::map<std::string, uint32> ScriptTimes;
	ScriptTimes m_ScriptTimes;

};

} // end of namespace WinterMute

#endif
