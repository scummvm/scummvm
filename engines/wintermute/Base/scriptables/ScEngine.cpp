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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/scriptables/ScEngine.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/SXMath.h"
#include "engines/wintermute/Base/BRegistry.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BFileManager.h"


namespace WinterMute {

IMPLEMENT_PERSISTENT(CScEngine, true)

#define COMPILER_DLL "dcscomp.dll"
//////////////////////////////////////////////////////////////////////////
CScEngine::CScEngine(CBGame *inGame): CBBase(inGame) {
	Game->LOG(0, "Initializing scripting engine...");

	if (_compilerAvailable) Game->LOG(0, "  Script compiler bound successfuly");
	else Game->LOG(0, "  Script compiler is NOT available");

	_globals = new CScValue(Game);


	// register 'Game' as global variable
	if (!_globals->propExists("Game")) {
		CScValue val(Game);
		val.setNative(Game, true);
		_globals->setProp("Game", &val);
	}

	// register 'Math' as global variable
	if (!_globals->propExists("Math")) {
		CScValue val(Game);
		val.setNative(Game->_mathClass, true);
		_globals->setProp("Math", &val);
	}

	// prepare script cache
	for (int i = 0; i < MAX_CACHED_SCRIPTS; i++) _cachedScripts[i] = NULL;

	_currentScript = NULL;

	_fileToCompile = NULL;

	_compileErrorCallback = NULL;
	_compileErrorCallbackData = NULL;

	_parseElementCallback = NULL;
	_parseElementCallbackData = NULL;

	_isProfiling = false;
	_profilingStartTime = 0;

	//EnableProfiling();
}


//////////////////////////////////////////////////////////////////////////
CScEngine::~CScEngine() {
	Game->LOG(0, "Shutting down scripting engine");
	saveBreakpoints();

	disableProfiling();

	cleanup();

	for (int i = 0; i < _breakpoints.GetSize(); i++) {
		delete _breakpoints[i];
		_breakpoints[i] = NULL;
	}
	_breakpoints.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::cleanup() {
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (!_scripts[i]->_thread && _scripts[i]->_owner) _scripts[i]->_owner->removeScript(_scripts[i]);
		delete _scripts[i];
		_scripts.RemoveAt(i);
		i--;
	}

	_scripts.RemoveAll();

	delete _globals;
	_globals = NULL;

	emptyScriptCache();

	_currentScript = NULL; // ref only

	delete[] _fileToCompile;
	_fileToCompile = NULL;

	_compileErrorCallback = NULL;
	_compileErrorCallbackData = NULL;

	_parseElementCallback = NULL;
	_parseElementCallbackData = NULL;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
byte *CScEngine::loadFile(void *data, char *filename, uint32 *size) {
	CBGame *Game = (CBGame *)data;
	return Game->_fileManager->readWholeFile(filename, size);
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::closeFile(void *data, byte *buffer) {
	delete [] buffer;
}

//////////////////////////////////////////////////////////////////////////
void CScEngine::addError(void *data, int line, char *text) {
	CBGame *Game = (CBGame *)data;

	if (Game) {
		if (Game->_scEngine && Game->_scEngine->_fileToCompile)
			Game->LOG(0, "Compiling script '%s'...", Game->_scEngine->_fileToCompile);
		Game->LOG(0, "  Error@line %d: %s", line, text);


		// redirect to an engine's own callback
		if (Game->_scEngine && Game->_scEngine->_compileErrorCallback) {
			Game->_scEngine->_compileErrorCallback(line, text, Game->_scEngine->_compileErrorCallbackData);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::parseElement(void *data, int line, int type, void *elementData) {
	CBGame *Game = (CBGame *)data;

	if (Game) {
		// redirect to an engine's own callback
		if (Game->_scEngine && Game->_scEngine->_parseElementCallback) {
			Game->_scEngine->_parseElementCallback(line, type, elementData, Game->_scEngine->_compileErrorCallbackData);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
CScScript *CScEngine::runScript(const char *filename, CBScriptHolder *owner) {
	byte *compBuffer;
	uint32 compSize;

	// get script from cache
	compBuffer = getCompiledScript(filename, &compSize);
	if (!compBuffer) return NULL;

	// add new script
	CScScript *script = new CScScript(Game, this);
	HRESULT ret = script->create(filename, compBuffer, compSize, owner);
	if (FAILED(ret)) {
		Game->LOG(ret, "Error running script '%s'...", filename);
		delete script;
		return NULL;
	} else {
		// publish the "self" pseudo-variable
		CScValue val(Game);
		if (owner)val.setNative(owner, true);
		else val.setNULL();

		script->_globals->setProp("self", &val);
		script->_globals->setProp("this", &val);

		_scripts.Add(script);
		Game->getDebugMgr()->onScriptInit(script);

		return script;
	}
}


//////////////////////////////////////////////////////////////////////////
byte *CScEngine::getCompiledScript(const char *filename, uint32 *outSize, bool ignoreCache) {
	// is script in cache?
	if (!ignoreCache) {
		for (int i = 0; i < MAX_CACHED_SCRIPTS; i++) {
			if (_cachedScripts[i] && scumm_stricmp(_cachedScripts[i]->_filename.c_str(), filename) == 0) {
				_cachedScripts[i]->_timestamp = CBPlatform::GetTime();
				*outSize = _cachedScripts[i]->_size;
				return _cachedScripts[i]->_buffer;
			}
		}
	}

	// nope, load it
	byte *compBuffer;
	uint32 compSize;
	bool compiledNow = false;

	uint32 size;

	byte *buffer = Game->_fileManager->readWholeFile(filename, &size);
	if (!buffer) {
		Game->LOG(0, "CScEngine::GetCompiledScript - error opening script '%s'", filename);
		return NULL;
	}

	// needs to be compiled?
	if (FROM_LE_32(*(uint32 *)buffer) == SCRIPT_MAGIC) {
		compBuffer = buffer;
		compSize = size;
	} else {
		if (!_compilerAvailable) {
			Game->LOG(0, "CScEngine::GetCompiledScript - script '%s' needs to be compiled but compiler is not available", filename);
			delete [] buffer;
			return NULL;
		}
		// This code will never be called, since _compilerAvailable is const false.
		// It's only here in the event someone would want to reinclude the compiler.
		error("Script needs compilation, ScummVM does not contain a WME compiler");
	}

	byte *ret = NULL;

	// add script to cache
	CScCachedScript *cachedScript = new CScCachedScript(filename, compBuffer, compSize);
	if (cachedScript) {
		int index = 0;
		uint32 MinTime = CBPlatform::GetTime();
		for (int i = 0; i < MAX_CACHED_SCRIPTS; i++) {
			if (_cachedScripts[i] == NULL) {
				index = i;
				break;
			} else if (_cachedScripts[i]->_timestamp <= MinTime) {
				MinTime = _cachedScripts[i]->_timestamp;
				index = i;
			}
		}

		if (_cachedScripts[index] != NULL) delete _cachedScripts[index];
		_cachedScripts[index] = cachedScript;

		ret = cachedScript->_buffer;
		*outSize = cachedScript->_size;
	}


	// cleanup
	delete [] buffer;
	if (compiledNow) ExtReleaseBuffer(compBuffer);

	return ret;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::tick() {


	if (_scripts.GetSize() == 0) return S_OK;


	// resolve waiting scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {

		switch (_scripts[i]->_state) {
		case SCRIPT_WAITING: {
			/*
			bool obj_found=false;
			for(int j=0; j<Game->_regObjects.GetSize(); j++)
			{
			    if(Game->_regObjects[j] == _scripts[i]->_waitObject)
			    {
			        if(Game->_regObjects[j]->IsReady()) _scripts[i]->Run();
			        obj_found = true;
			        break;
			    }
			}
			if(!obj_found) _scripts[i]->finish(); // _waitObject no longer exists
			*/
			if (Game->validObject(_scripts[i]->_waitObject)) {
				if (_scripts[i]->_waitObject->isReady()) _scripts[i]->run();
			} else _scripts[i]->finish();
			break;
		}

		case SCRIPT_SLEEPING: {
			if (_scripts[i]->_waitFrozen) {
				if (_scripts[i]->_waitTime <= CBPlatform::GetTime()) _scripts[i]->run();
			} else {
				if (_scripts[i]->_waitTime <= Game->_timer) _scripts[i]->run();
			}
			break;
		}

		case SCRIPT_WAITING_SCRIPT: {
			if (!isValidScript(_scripts[i]->_waitScript) || _scripts[i]->_waitScript->_state == SCRIPT_ERROR) {
				// fake return value
				_scripts[i]->_stack->pushNULL();
				_scripts[i]->_waitScript = NULL;
				_scripts[i]->run();
			} else {
				if (_scripts[i]->_waitScript->_state == SCRIPT_THREAD_FINISHED) {
					// copy return value
					_scripts[i]->_stack->push(_scripts[i]->_waitScript->_stack->pop());
					_scripts[i]->run();
					_scripts[i]->_waitScript->finish();
					_scripts[i]->_waitScript = NULL;
				}
			}
			break;
		}
		default:
			//warning("CScEngine::Tick - Unhandled enum");
			break;
		} // switch
	} // for each script


	// execute scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {

		// skip paused scripts
		if (_scripts[i]->_state == SCRIPT_PAUSED) continue;

		// time sliced script
		if (_scripts[i]->_timeSlice > 0) {
			uint32 StartTime = CBPlatform::GetTime();
			while (_scripts[i]->_state == SCRIPT_RUNNING && CBPlatform::GetTime() - StartTime < _scripts[i]->_timeSlice) {
				_currentScript = _scripts[i];
				_scripts[i]->executeInstruction();
			}
			if (_isProfiling && _scripts[i]->_filename) addScriptTime(_scripts[i]->_filename, CBPlatform::GetTime() - StartTime);
		}

		// normal script
		else {
			uint32 StartTime = 0;
			bool isProfiling = _isProfiling;
			if (isProfiling) StartTime = CBPlatform::GetTime();

			while (_scripts[i]->_state == SCRIPT_RUNNING) {
				_currentScript = _scripts[i];
				_scripts[i]->executeInstruction();
			}
			if (isProfiling && _scripts[i]->_filename) addScriptTime(_scripts[i]->_filename, CBPlatform::GetTime() - StartTime);
		}
		_currentScript = NULL;
	}

	removeFinishedScripts();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::tickUnbreakable() {
	// execute unbreakable scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (!_scripts[i]->_unbreakable) continue;

		while (_scripts[i]->_state == SCRIPT_RUNNING) {
			_currentScript = _scripts[i];
			_scripts[i]->executeInstruction();
		}
		_scripts[i]->finish();
		_currentScript = NULL;
	}
	removeFinishedScripts();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::removeFinishedScripts() {
	// remove finished scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_FINISHED || _scripts[i]->_state == SCRIPT_ERROR) {
			if (!_scripts[i]->_thread && _scripts[i]->_owner) _scripts[i]->_owner->removeScript(_scripts[i]);
			Game->getDebugMgr()->onScriptShutdown(_scripts[i]);
			delete _scripts[i];
			_scripts.RemoveAt(i);
			i--;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CScEngine::getNumScripts(int *running, int *waiting, int *persistent) {
	int numRunning = 0, numWaiting = 0, numPersistent = 0, numTotal = 0;

	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_FINISHED) continue;
		switch (_scripts[i]->_state) {
		case SCRIPT_RUNNING:
		case SCRIPT_SLEEPING:
		case SCRIPT_PAUSED:
			numRunning++;
			break;
		case SCRIPT_WAITING:
			numWaiting++;
			break;
		case SCRIPT_PERSISTENT:
			numPersistent++;
			break;
		default:
			warning("CScEngine::GetNumScripts - unhandled enum");
			break;
		}
		numTotal++;
	}
	if (running) *running = numRunning;
	if (waiting) *waiting = numWaiting;
	if (persistent)  *persistent = numPersistent;

	return numTotal;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::emptyScriptCache() {
	for (int i = 0; i < MAX_CACHED_SCRIPTS; i++) {
		if (_cachedScripts[i]) {
			delete _cachedScripts[i];
			_cachedScripts[i] = NULL;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::resetObject(CBObject *Object) {
	// terminate all scripts waiting for this object
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_WAITING && _scripts[i]->_waitObject == Object) {
			if (!Game->_compatKillMethodThreads) resetScript(_scripts[i]);

			bool IsThread = _scripts[i]->_methodThread || _scripts[i]->_thread;
			_scripts[i]->finish(!IsThread); // 1.9b1 - top-level script kills its threads as well
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::resetScript(CScScript *script) {
	// terminate all scripts waiting for this script
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_WAITING_SCRIPT && _scripts[i]->_waitScript == script) {
			_scripts[i]->finish();
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::persist(CBPersistMgr *persistMgr) {
	if (!persistMgr->_saving) cleanup();

	persistMgr->transfer(TMEMBER(Game));
	persistMgr->transfer(TMEMBER(_currentScript));
	persistMgr->transfer(TMEMBER(_fileToCompile));
	persistMgr->transfer(TMEMBER(_globals));
	_scripts.persist(persistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::editorCleanup() {
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_owner == NULL && (_scripts[i]->_state == SCRIPT_FINISHED || _scripts[i]->_state == SCRIPT_ERROR)) {
			delete _scripts[i];
			_scripts.RemoveAt(i);
			i--;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::pauseAll() {
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i] != _currentScript) _scripts[i]->pause();
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::resumeAll() {
	for (int i = 0; i < _scripts.GetSize(); i++)
		_scripts[i]->resume();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::setFileToCompile(const char *filename) {
	delete[] _fileToCompile;
	_fileToCompile = new char[strlen(filename) + 1];
	if (_fileToCompile) {
		strcpy(_fileToCompile, filename);
		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::setCompileErrorCallback(COMPILE_ERROR_CALLBACK callback, void *data) {
	_compileErrorCallback = callback;
	_compileErrorCallbackData = data;
}

//////////////////////////////////////////////////////////////////////////
void CScEngine::setParseElementCallback(PARSE_ELEMENT_CALLBACK callback, void *data) {
	_parseElementCallback = callback;
	_parseElementCallbackData = data;
}


//////////////////////////////////////////////////////////////////////////
bool CScEngine::isValidScript(CScScript *script) {
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i] == script) return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::clearGlobals(bool includingNatives) {
	_globals->CleanProps(includingNatives);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::dbgSendScripts(IWmeDebugClient *client) {
	// send global variables
	_globals->dbgSendVariables(client, WME_DBGVAR_GLOBAL, NULL, 0);

	// process normal scripts first
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_thread || _scripts[i]->_methodThread) continue;
		_scripts[i]->dbgSendScript(client);
	}

	// and threads later
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_thread || _scripts[i]->_methodThread)
			_scripts[i]->dbgSendScript(client);
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::addBreakpoint(const char *scriptFilename, int line) {
	if (!Game->getDebugMgr()->_enabled) return S_OK;

	CScBreakpoint *bp = NULL;
	for (int i = 0; i < _breakpoints.GetSize(); i++) {
		if (scumm_stricmp(_breakpoints[i]->_filename.c_str(), scriptFilename) == 0) {
			bp = _breakpoints[i];
			break;
		}
	}
	if (bp == NULL) {
		bp = new CScBreakpoint(scriptFilename);
		_breakpoints.Add(bp);
	}

	for (int i = 0; i < bp->_lines.GetSize(); i++) {
		if (bp->_lines[i] == line) return S_OK;
	}
	bp->_lines.Add(line);

	// refresh changes
	refreshScriptBreakpoints();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::removeBreakpoint(const char *scriptFilename, int line) {
	if (!Game->getDebugMgr()->_enabled) return S_OK;

	for (int i = 0; i < _breakpoints.GetSize(); i++) {
		if (scumm_stricmp(_breakpoints[i]->_filename.c_str(), scriptFilename) == 0) {
			for (int j = 0; j < _breakpoints[i]->_lines.GetSize(); j++) {
				if (_breakpoints[i]->_lines[j] == line) {
					_breakpoints[i]->_lines.RemoveAt(j);
					if (_breakpoints[i]->_lines.GetSize() == 0) {
						delete _breakpoints[i];
						_breakpoints.RemoveAt(i);
					}
					// refresh changes
					refreshScriptBreakpoints();

					return S_OK;
				}
			}
			break;
		}
	}
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::refreshScriptBreakpoints() {
	if (!Game->getDebugMgr()->_enabled) return S_OK;

	for (int i = 0; i < _scripts.GetSize(); i++) {
		refreshScriptBreakpoints(_scripts[i]);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::refreshScriptBreakpoints(CScScript *script) {
	if (!Game->getDebugMgr()->_enabled) return S_OK;

	if (!script || !script->_filename) return E_FAIL;

	for (int i = 0; i < _breakpoints.GetSize(); i++) {
		if (scumm_stricmp(_breakpoints[i]->_filename.c_str(), script->_filename) == 0) {
			script->_breakpoints.Copy(_breakpoints[i]->_lines);
			return S_OK;
		}
	}
	if (script->_breakpoints.GetSize() > 0) script->_breakpoints.RemoveAll();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::saveBreakpoints() {
	if (!Game->getDebugMgr()->_enabled) return S_OK;


	char text[512];
	char key[100];

	int count = 0;
	for (int i = 0; i < _breakpoints.GetSize(); i++) {
		for (int j = 0; j < _breakpoints[i]->_lines.GetSize(); j++) {
			count++;
			sprintf(key, "Breakpoint%d", count);
			sprintf(text, "%s:%d", _breakpoints[i]->_filename.c_str(), _breakpoints[i]->_lines[j]);

			Game->_registry->writeString("Debug", key, text);
		}
	}
	Game->_registry->writeInt("Debug", "NumBreakpoints", count);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::loadBreakpoints() {
	if (!Game->getDebugMgr()->_enabled) return S_OK;

	char key[100];

	int count = Game->_registry->readInt("Debug", "NumBreakpoints", 0);
	for (int i = 1; i <= count; i++) {
		/*  uint32 BufSize = 512; */
		sprintf(key, "Breakpoint%d", i);
		AnsiString breakpoint = Game->_registry->readString("Debug", key, "");

		char *path = CBUtils::strEntry(0, breakpoint.c_str(), ':');
		char *line = CBUtils::strEntry(1, breakpoint.c_str(), ':');

		if (path != NULL && line != NULL) addBreakpoint(path, atoi(line));
		delete[] path;
		delete[] line;
		path = NULL;
		line = NULL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::addScriptTime(const char *filename, uint32 time) {
	if (!_isProfiling) return;

	AnsiString fileName = filename;
	StringUtil::toLowerCase(fileName);
	_scriptTimes[fileName] += time;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::enableProfiling() {
	if (_isProfiling) return;

	// destroy old data, if any
	_scriptTimes.clear();

	_profilingStartTime = CBPlatform::GetTime();
	_isProfiling = true;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::disableProfiling() {
	if (!_isProfiling) return;

	dumpStats();
	_isProfiling = false;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::dumpStats() {
	error("DumpStats not ported to ScummVM yet");
	/*  uint32 totalTime = CBPlatform::GetTime() - _profilingStartTime;

	    typedef std::vector <std::pair<uint32, std::string> > TimeVector;
	    TimeVector times;

	    ScriptTimes::iterator it;
	    for (it = _scriptTimes.begin(); it != _scriptTimes.end(); it++) {
	        times.push_back(std::pair<uint32, std::string> (it->_value, it->_key));
	    }
	    std::sort(times.begin(), times.end());


	    TimeVector::reverse_iterator tit;

	    Game->LOG(0, "***** Script profiling information: *****");
	    Game->LOG(0, "  %-40s %fs", "Total execution time", (float)totalTime / 1000);

	    for (tit = times.rbegin(); tit != times.rend(); tit++) {
	        Game->LOG(0, "  %-40s %fs (%f%%)", tit->second.c_str(), (float)tit->first / 1000, (float)tit->first / (float)totalTime * 100);
	    }*/
}

} // end of namespace WinterMute
