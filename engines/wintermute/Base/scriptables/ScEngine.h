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
#include "engines/wintermute/Base/BBase.h"
#include "engines/wintermute/wme_debugger.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/PlatformSDL.h"

namespace WinterMute {

typedef byte *(*DLL_COMPILE_BUFFER)(byte  *Buffer, char *Source, uint32 BufferSize, uint32 *CompiledSize);
typedef byte *(*DLL_COMPILE_FILE)(char *Filename, uint32 *CompiledSize);
typedef void (*DLL_RELEASE_BUFFER)(unsigned char *Buffer);
typedef void (*DLL_SET_CALLBACKS)(CALLBACKS *callbacks, void *Data);
typedef int (*DLL_DEFINE_FUNCTION)(const char *Name); /* Was non-const, changed to silence warnings */
typedef int (*DLL_DEFINE_VARIABLE)(const char *Name); /* Was non-const, changed to silence warnings */

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
		CScCachedScript(const char *Filename, byte *Buffer, uint32 Size) {
			_timestamp = CBPlatform::GetTime();
			_buffer = new byte[Size];
			if (_buffer) memcpy(_buffer, Buffer, Size);
			_size = Size;
			_filename = Filename;
		};

		~CScCachedScript() {
			if (_buffer) delete [] _buffer;
		};

		uint32 _timestamp;
		byte *_buffer;
		uint32 _size;
		Common::String _filename;
	};

	class CScBreakpoint {
	public:
		CScBreakpoint(const char *Filename) {
			_filename = Filename;
		}

		~CScBreakpoint() {
			_lines.RemoveAll();
		}

		Common::String _filename;
		CBArray<int, int> _lines;
	};




public:
	HRESULT DbgSendScripts(IWmeDebugClient *Client);

	CBArray<CScBreakpoint *, CScBreakpoint *> _breakpoints;
	HRESULT AddBreakpoint(const char *ScriptFilename, int Line);
	HRESULT RemoveBreakpoint(const char *ScriptFilename, int Line);
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

	COMPILE_ERROR_CALLBACK _compileErrorCallback;
	void *_compileErrorCallbackData;

	PARSE_ELEMENT_CALLBACK _parseElementCallback;
	void *_parseElementCallbackData;

	HRESULT SetFileToCompile(const char *Filename);
	char *_fileToCompile;
	CScScript *_currentScript;
	HRESULT ResumeAll();
	HRESULT PauseAll();
	void EditorCleanup();
	HRESULT ResetObject(CBObject *Object);
	HRESULT ResetScript(CScScript *Script);
	HRESULT EmptyScriptCache();
	byte *GetCompiledScript(const char *Filename, uint32 *OutSize, bool IgnoreCache = false);
	DECLARE_PERSISTENT(CScEngine, CBBase)
	HRESULT Cleanup();
	int GetNumScripts(int *Running = NULL, int *Waiting = NULL, int *Persistent = NULL);
	HRESULT Tick();
	CScValue *_globals;
	CScScript *RunScript(const char *Filename, CBScriptHolder *Owner = NULL);
	bool _compilerAvailable;
	HINSTANCE _compilerDLL;
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

	CBArray<CScScript *, CScScript *> _scripts;

	void EnableProfiling();
	void DisableProfiling();
	bool IsProfiling() {
		return _isProfiling;
	}

	void AddScriptTime(const char *Filename, uint32 Time);
	void DumpStats();

private:

	CScCachedScript *_cachedScripts[MAX_CACHED_SCRIPTS];
	bool _isProfiling;
	uint32 _profilingStartTime;

	typedef Common::HashMap<Common::String, uint32> ScriptTimes;
	ScriptTimes _scriptTimes;

};

} // end of namespace WinterMute

#endif
