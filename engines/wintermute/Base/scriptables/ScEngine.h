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
			_timestamp = CBPlatform::getTime();
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
			_lines.removeAll();
		}

		Common::String _filename;
		CBArray<int, int> _lines;
	};




public:
	ERRORCODE dbgSendScripts(IWmeDebugClient *client);

	CBArray<CScBreakpoint *, CScBreakpoint *> _breakpoints;
	ERRORCODE addBreakpoint(const char *scriptFilename, int line);
	ERRORCODE removeBreakpoint(const char *scriptFilename, int line);
	ERRORCODE refreshScriptBreakpoints();
	ERRORCODE refreshScriptBreakpoints(CScScript *script);
	ERRORCODE saveBreakpoints();
	ERRORCODE loadBreakpoints();

	ERRORCODE clearGlobals(bool includingNatives = false);
	ERRORCODE tickUnbreakable();
	ERRORCODE removeFinishedScripts();
	bool isValidScript(CScScript *script);

	CScScript *_currentScript;
	ERRORCODE resumeAll();
	ERRORCODE pauseAll();
	void editorCleanup();
	ERRORCODE resetObject(CBObject *Object);
	ERRORCODE resetScript(CScScript *script);
	ERRORCODE emptyScriptCache();
	byte *getCompiledScript(const char *filename, uint32 *outSize, bool ignoreCache = false);
	DECLARE_PERSISTENT(CScEngine, CBBase)
	ERRORCODE cleanup();
	int getNumScripts(int *running = NULL, int *waiting = NULL, int *persistent = NULL);
	ERRORCODE tick();
	CScValue *_globals;
	CScScript *runScript(const char *filename, CBScriptHolder *owner = NULL);
	static const bool _compilerAvailable = false;

	CScEngine(CBGame *inGame);
	virtual ~CScEngine();
	static byte *loadFile(void *data, char *filename, uint32 *size);
	static void closeFile(void *data, byte *buffer);
	static void parseElement(void *data, int line, int type, void *elementData);

	CBArray<CScScript *, CScScript *> _scripts;

	void enableProfiling();
	void disableProfiling();
	bool getIsProfiling() {
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
