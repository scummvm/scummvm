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

	for (int i = 0; i < _breakpoints.getSize(); i++) {
		delete _breakpoints[i];
		_breakpoints[i] = NULL;
	}
	_breakpoints.removeAll();
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::cleanup() {
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (!_scripts[i]->_thread && _scripts[i]->_owner) _scripts[i]->_owner->removeScript(_scripts[i]);
		delete _scripts[i];
		_scripts.removeAt(i);
		i--;
	}

	_scripts.removeAll();

	delete _globals;
	_globals = NULL;

	emptyScriptCache();

	_currentScript = NULL; // ref only

	return STATUS_OK;
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
void CScEngine::parseElement(void *data, int line, int type, void *elementData) {
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
	ERRORCODE ret = script->create(filename, compBuffer, compSize, owner);
	if (DID_FAIL(ret)) {
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

		_scripts.add(script);
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
				_cachedScripts[i]->_timestamp = CBPlatform::getTime();
				*outSize = _cachedScripts[i]->_size;
				return _cachedScripts[i]->_buffer;
			}
		}
	}

	// nope, load it
	byte *compBuffer;
	uint32 compSize;

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
		uint32 MinTime = CBPlatform::getTime();
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

	return ret;
}



//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::tick() {
	if (_scripts.getSize() == 0)
		return STATUS_OK;


	// resolve waiting scripts
	for (int i = 0; i < _scripts.getSize(); i++) {

		switch (_scripts[i]->_state) {
		case SCRIPT_WAITING: {
			/*
			bool obj_found=false;
			for(int j=0; j<Game->_regObjects.getSize(); j++)
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
				if (_scripts[i]->_waitTime <= CBPlatform::getTime()) _scripts[i]->run();
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
	for (int i = 0; i < _scripts.getSize(); i++) {

		// skip paused scripts
		if (_scripts[i]->_state == SCRIPT_PAUSED) continue;

		// time sliced script
		if (_scripts[i]->_timeSlice > 0) {
			uint32 StartTime = CBPlatform::getTime();
			while (_scripts[i]->_state == SCRIPT_RUNNING && CBPlatform::getTime() - StartTime < _scripts[i]->_timeSlice) {
				_currentScript = _scripts[i];
				_scripts[i]->executeInstruction();
			}
			if (_isProfiling && _scripts[i]->_filename) addScriptTime(_scripts[i]->_filename, CBPlatform::getTime() - StartTime);
		}

		// normal script
		else {
			uint32 startTime = 0;
			bool isProfiling = _isProfiling;
			if (isProfiling) startTime = CBPlatform::getTime();

			while (_scripts[i]->_state == SCRIPT_RUNNING) {
				_currentScript = _scripts[i];
				_scripts[i]->executeInstruction();
			}
			if (isProfiling && _scripts[i]->_filename) addScriptTime(_scripts[i]->_filename, CBPlatform::getTime() - startTime);
		}
		_currentScript = NULL;
	}

	removeFinishedScripts();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::tickUnbreakable() {
	// execute unbreakable scripts
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (!_scripts[i]->_unbreakable) continue;

		while (_scripts[i]->_state == SCRIPT_RUNNING) {
			_currentScript = _scripts[i];
			_scripts[i]->executeInstruction();
		}
		_scripts[i]->finish();
		_currentScript = NULL;
	}
	removeFinishedScripts();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::removeFinishedScripts() {
	// remove finished scripts
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_FINISHED || _scripts[i]->_state == SCRIPT_ERROR) {
			if (!_scripts[i]->_thread && _scripts[i]->_owner) _scripts[i]->_owner->removeScript(_scripts[i]);
			Game->getDebugMgr()->onScriptShutdown(_scripts[i]);
			delete _scripts[i];
			_scripts.removeAt(i);
			i--;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int CScEngine::getNumScripts(int *running, int *waiting, int *persistent) {
	int numRunning = 0, numWaiting = 0, numPersistent = 0, numTotal = 0;

	for (int i = 0; i < _scripts.getSize(); i++) {
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
ERRORCODE CScEngine::emptyScriptCache() {
	for (int i = 0; i < MAX_CACHED_SCRIPTS; i++) {
		if (_cachedScripts[i]) {
			delete _cachedScripts[i];
			_cachedScripts[i] = NULL;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::resetObject(CBObject *Object) {
	// terminate all scripts waiting for this object
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_WAITING && _scripts[i]->_waitObject == Object) {
			if (!Game->_compatKillMethodThreads) resetScript(_scripts[i]);

			bool IsThread = _scripts[i]->_methodThread || _scripts[i]->_thread;
			_scripts[i]->finish(!IsThread); // 1.9b1 - top-level script kills its threads as well
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::resetScript(CScScript *script) {
	// terminate all scripts waiting for this script
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_WAITING_SCRIPT && _scripts[i]->_waitScript == script) {
			_scripts[i]->finish();
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::persist(CBPersistMgr *persistMgr) {
	if (!persistMgr->_saving) cleanup();

	persistMgr->transfer(TMEMBER(Game));
	persistMgr->transfer(TMEMBER(_currentScript));
	persistMgr->transfer(TMEMBER(_globals));
	_scripts.persist(persistMgr);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::editorCleanup() {
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_owner == NULL && (_scripts[i]->_state == SCRIPT_FINISHED || _scripts[i]->_state == SCRIPT_ERROR)) {
			delete _scripts[i];
			_scripts.removeAt(i);
			i--;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::pauseAll() {
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i] != _currentScript) _scripts[i]->pause();
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::resumeAll() {
	for (int i = 0; i < _scripts.getSize(); i++)
		_scripts[i]->resume();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CScEngine::isValidScript(CScScript *script) {
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i] == script) return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::clearGlobals(bool includingNatives) {
	_globals->CleanProps(includingNatives);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::dbgSendScripts(IWmeDebugClient *client) {
	// send global variables
	_globals->dbgSendVariables(client, WME_DBGVAR_GLOBAL, NULL, 0);

	// process normal scripts first
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_thread || _scripts[i]->_methodThread) continue;
		_scripts[i]->dbgSendScript(client);
	}

	// and threads later
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_thread || _scripts[i]->_methodThread)
			_scripts[i]->dbgSendScript(client);
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::addBreakpoint(const char *scriptFilename, int line) {
	if (!Game->getDebugMgr()->_enabled) return STATUS_OK;

	CScBreakpoint *bp = NULL;
	for (int i = 0; i < _breakpoints.getSize(); i++) {
		if (scumm_stricmp(_breakpoints[i]->_filename.c_str(), scriptFilename) == 0) {
			bp = _breakpoints[i];
			break;
		}
	}
	if (bp == NULL) {
		bp = new CScBreakpoint(scriptFilename);
		_breakpoints.add(bp);
	}

	for (int i = 0; i < bp->_lines.getSize(); i++) {
		if (bp->_lines[i] == line) return STATUS_OK;
	}
	bp->_lines.add(line);

	// refresh changes
	refreshScriptBreakpoints();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::removeBreakpoint(const char *scriptFilename, int line) {
	if (!Game->getDebugMgr()->_enabled) return STATUS_OK;

	for (int i = 0; i < _breakpoints.getSize(); i++) {
		if (scumm_stricmp(_breakpoints[i]->_filename.c_str(), scriptFilename) == 0) {
			for (int j = 0; j < _breakpoints[i]->_lines.getSize(); j++) {
				if (_breakpoints[i]->_lines[j] == line) {
					_breakpoints[i]->_lines.removeAt(j);
					if (_breakpoints[i]->_lines.getSize() == 0) {
						delete _breakpoints[i];
						_breakpoints.removeAt(i);
					}
					// refresh changes
					refreshScriptBreakpoints();

					return STATUS_OK;
				}
			}
			break;
		}
	}
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::refreshScriptBreakpoints() {
	if (!Game->getDebugMgr()->_enabled) return STATUS_OK;

	for (int i = 0; i < _scripts.getSize(); i++) {
		refreshScriptBreakpoints(_scripts[i]);
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::refreshScriptBreakpoints(CScScript *script) {
	if (!Game->getDebugMgr()->_enabled) return STATUS_OK;

	if (!script || !script->_filename) return STATUS_FAILED;

	for (int i = 0; i < _breakpoints.getSize(); i++) {
		if (scumm_stricmp(_breakpoints[i]->_filename.c_str(), script->_filename) == 0) {
			script->_breakpoints.copy(_breakpoints[i]->_lines);
			return STATUS_OK;
		}
	}
	if (script->_breakpoints.getSize() > 0) script->_breakpoints.removeAll();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::saveBreakpoints() {
	if (!Game->getDebugMgr()->_enabled) return STATUS_OK;


	char text[512];
	char key[100];

	int count = 0;
	for (int i = 0; i < _breakpoints.getSize(); i++) {
		for (int j = 0; j < _breakpoints[i]->_lines.getSize(); j++) {
			count++;
			sprintf(key, "Breakpoint%d", count);
			sprintf(text, "%s:%d", _breakpoints[i]->_filename.c_str(), _breakpoints[i]->_lines[j]);

			Game->_registry->writeString("Debug", key, text);
		}
	}
	Game->_registry->writeInt("Debug", "NumBreakpoints", count);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CScEngine::loadBreakpoints() {
	if (!Game->getDebugMgr()->_enabled) return STATUS_OK;

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

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::addScriptTime(const char *filename, uint32 time) {
	if (!_isProfiling) return;

	AnsiString fileName = filename;
	fileName.toLowercase();
	_scriptTimes[fileName] += time;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::enableProfiling() {
	if (_isProfiling) return;

	// destroy old data, if any
	_scriptTimes.clear();

	_profilingStartTime = CBPlatform::getTime();
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
	/*  uint32 totalTime = CBPlatform::getTime() - _profilingStartTime;

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
