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

typedef byte *(*DLL_COMPILE_BUFFER)(byte *buffer, char *source, uint32 bufferSize, uint32 *compiledSize);
typedef byte *(*DLL_COMPILE_FILE)(char *filename, uint32 *compiledSize);
typedef void (*DLL_RELEASE_BUFFER)(unsigned char *buffer);
typedef void (*DLL_SET_CALLBACKS)(CALLBACKS *callbacks, void *Data);
typedef int (*DLL_DEFINE_FUNCTION)(const char *name); /* Was non-const, changed to silence warnings */
typedef int (*DLL_DEFINE_VARIABLE)(const char *name); /* Was non-const, changed to silence warnings */

typedef void (*COMPILE_ERROR_CALLBACK)(int line, char *text , void *data);
typedef void (*PARSE_ELEMENT_CALLBACK)(int line, int type, void *elementData, void *data);

#define MAX_CACHED_SCRIPTS 20
class CScScript;
class CScValue;
class CBObject;
class CBScriptHolder;
class CScEngine : public CBBase {
public:
	class CScCachedScript {
	public:
		CScCachedScript(const char *filename, byte *buffer, uint32 size) {
			_timestamp = CBPlatform::GetTime();
			_buffer = new byte[size];
			if (_buffer) memcpy(_buffer, buffer, size);
			_size = size;
			_filename = filename;
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
		CScBreakpoint(const char *filename) {
			_filename = filename;
		}

		~CScBreakpoint() {
			_lines.RemoveAll();
		}

		Common::String _filename;
		CBArray<int, int> _lines;
	};




public:
	HRESULT dbgSendScripts(IWmeDebugClient *client);

	CBArray<CScBreakpoint *, CScBreakpoint *> _breakpoints;
	HRESULT addBreakpoint(const char *scriptFilename, int line);
	HRESULT removeBreakpoint(const char *scriptFilename, int line);
	HRESULT refreshScriptBreakpoints();
	HRESULT refreshScriptBreakpoints(CScScript *script);
	HRESULT saveBreakpoints();
	HRESULT loadBreakpoints();

	HRESULT clearGlobals(bool includingNatives = false);
	HRESULT tickUnbreakable();
	HRESULT removeFinishedScripts();
	bool isValidScript(CScScript *script);
	void setCompileErrorCallback(COMPILE_ERROR_CALLBACK callback, void *data);
	void setParseElementCallback(PARSE_ELEMENT_CALLBACK callback, void *data);

	COMPILE_ERROR_CALLBACK _compileErrorCallback;
	void *_compileErrorCallbackData;

	PARSE_ELEMENT_CALLBACK _parseElementCallback;
	void *_parseElementCallbackData;

	HRESULT setFileToCompile(const char *filename);
	char *_fileToCompile;
	CScScript *_currentScript;
	HRESULT resumeAll();
	HRESULT pauseAll();
	void editorCleanup();
	HRESULT resetObject(CBObject *Object);
	HRESULT resetScript(CScScript *script);
	HRESULT emptyScriptCache();
	byte *getCompiledScript(const char *filename, uint32 *outSize, bool ignoreCache = false);
	DECLARE_PERSISTENT(CScEngine, CBBase)
	HRESULT cleanup();
	int getNumScripts(int *running = NULL, int *waiting = NULL, int *persistent = NULL);
	HRESULT tick();
	CScValue *_globals;
	CScScript *runScript(const char *filename, CBScriptHolder *owner = NULL);
	bool _compilerAvailable;
	HINSTANCE _compilerDLL;
	CScEngine(CBGame *inGame);
	virtual ~CScEngine();
	static void addError(void *data, int line, char *text);
	static byte *loadFile(void *data, char *filename, uint32 *size);
	static void closeFile(void *data, byte *buffer);
	static void parseElement(void *data, int line, int type, void *elementData);
	DLL_COMPILE_BUFFER  ExtCompileBuffer;
	DLL_COMPILE_FILE    ExtCompileFile;
	DLL_RELEASE_BUFFER  ExtReleaseBuffer;
	DLL_SET_CALLBACKS   ExtSetCallbacks;
	DLL_DEFINE_FUNCTION ExtDefineFunction;
	DLL_DEFINE_VARIABLE ExtDefineVariable;

	CBArray<CScScript *, CScScript *> _scripts;

	void enableProfiling();
	void disableProfiling();
	bool isProfiling() {
		return _isProfiling;
	}

	void addScriptTime(const char *filename, uint32 Time);
	void dumpStats();

private:

	CScCachedScript *_cachedScripts[MAX_CACHED_SCRIPTS];
	bool _isProfiling;
	uint32 _profilingStartTime;

	typedef Common::HashMap<Common::String, uint32> ScriptTimes;
	ScriptTimes _scriptTimes;

};

} // end of namespace WinterMute

#endif
