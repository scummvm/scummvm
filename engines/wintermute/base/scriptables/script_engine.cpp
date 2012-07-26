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

#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_ext_math.h"
#include "engines/wintermute/base/base_registry.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/utils/utils.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(ScEngine, true)

#define COMPILER_DLL "dcscomp.dll"
//////////////////////////////////////////////////////////////////////////
ScEngine::ScEngine(BaseGame *inGame): BaseClass(inGame) {
	_gameRef->LOG(0, "Initializing scripting engine...");

	if (_compilerAvailable) {
		_gameRef->LOG(0, "  Script compiler bound successfuly");
	} else {
		_gameRef->LOG(0, "  Script compiler is NOT available");
	}

	_globals = new ScValue(_gameRef);


	// register 'Game' as global variable
	if (!_globals->propExists("Game")) {
		ScValue val(_gameRef);
		val.setNative(_gameRef,  true);
		_globals->setProp("Game", &val);
	}

	// register 'Math' as global variable
	if (!_globals->propExists("Math")) {
		ScValue val(_gameRef);
		val.setNative(_gameRef->_mathClass, true);
		_globals->setProp("Math", &val);
	}

	// prepare script cache
	for (int i = 0; i < MAX_CACHED_SCRIPTS; i++) {
		_cachedScripts[i] = NULL;
	}

	_currentScript = NULL;

	_isProfiling = false;
	_profilingStartTime = 0;

	//EnableProfiling();
}


//////////////////////////////////////////////////////////////////////////
ScEngine::~ScEngine() {
	_gameRef->LOG(0, "Shutting down scripting engine");
	saveBreakpoints();

	disableProfiling();

	cleanup();

	for (int i = 0; i < _breakpoints.getSize(); i++) {
		delete _breakpoints[i];
		_breakpoints[i] = NULL;
	}
	_breakpoints.clear();
}


//////////////////////////////////////////////////////////////////////////
bool ScEngine::cleanup() {
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (!_scripts[i]->_thread && _scripts[i]->_owner) {
			_scripts[i]->_owner->removeScript(_scripts[i]);
		}
		delete _scripts[i];
		_scripts.remove_at(i);
		i--;
	}

	_scripts.clear();

	delete _globals;
	_globals = NULL;

	emptyScriptCache();

	_currentScript = NULL; // ref only

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
byte *ScEngine::loadFile(void *data, char *filename, uint32 *size) {
	BaseGame *gameRef = (BaseGame *)data;
	return gameRef->_fileManager->readWholeFile(filename, size);
}


//////////////////////////////////////////////////////////////////////////
void ScEngine::closeFile(void *data, byte *buffer) {
	delete[] buffer;
}


//////////////////////////////////////////////////////////////////////////
void ScEngine::parseElement(void *data, int line, int type, void *elementData) {
}


//////////////////////////////////////////////////////////////////////////
ScScript *ScEngine::runScript(const char *filename, BaseScriptHolder *owner) {
	byte *compBuffer;
	uint32 compSize;

	// get script from cache
	compBuffer = getCompiledScript(filename, &compSize);
	if (!compBuffer) {
		return NULL;
	}

	// add new script
	ScScript *script = new ScScript(_gameRef, this);
	bool ret = script->create(filename, compBuffer, compSize, owner);
	if (DID_FAIL(ret)) {
		_gameRef->LOG(ret, "Error running script '%s'...", filename);
		delete script;
		return NULL;
	} else {
		// publish the "self" pseudo-variable
		ScValue val(_gameRef);
		if (owner) {
			val.setNative(owner, true);
		} else {
			val.setNULL();
		}

		script->_globals->setProp("self", &val);
		script->_globals->setProp("this", &val);

		_scripts.add(script);
		_gameRef->getDebugMgr()->onScriptInit(script);

		return script;
	}
}


//////////////////////////////////////////////////////////////////////////
byte *ScEngine::getCompiledScript(const char *filename, uint32 *outSize, bool ignoreCache) {
	// is script in cache?
	if (!ignoreCache) {
		for (int i = 0; i < MAX_CACHED_SCRIPTS; i++) {
			if (_cachedScripts[i] && scumm_stricmp(_cachedScripts[i]->_filename.c_str(), filename) == 0) {
				_cachedScripts[i]->_timestamp = g_system->getMillis();
				*outSize = _cachedScripts[i]->_size;
				return _cachedScripts[i]->_buffer;
			}
		}
	}

	// nope, load it
	byte *compBuffer;
	uint32 compSize;

	uint32 size;

	byte *buffer = _gameRef->_fileManager->readWholeFile(filename, &size);
	if (!buffer) {
		_gameRef->LOG(0, "ScEngine::GetCompiledScript - error opening script '%s'", filename);
		return NULL;
	}

	// needs to be compiled?
	if (FROM_LE_32(*(uint32 *)buffer) == SCRIPT_MAGIC) {
		compBuffer = buffer;
		compSize = size;
	} else {
		if (!_compilerAvailable) {
			_gameRef->LOG(0, "ScEngine::GetCompiledScript - script '%s' needs to be compiled but compiler is not available", filename);
			delete[] buffer;
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
		uint32 minTime = g_system->getMillis();
		for (int i = 0; i < MAX_CACHED_SCRIPTS; i++) {
			if (_cachedScripts[i] == NULL) {
				index = i;
				break;
			} else if (_cachedScripts[i]->_timestamp <= minTime) {
				minTime = _cachedScripts[i]->_timestamp;
				index = i;
			}
		}

		if (_cachedScripts[index] != NULL) {
			delete _cachedScripts[index];
		}
		_cachedScripts[index] = cachedScript;

		ret = cachedScript->_buffer;
		*outSize = cachedScript->_size;
	}


	// cleanup
	delete[] buffer;

	return ret;
}



//////////////////////////////////////////////////////////////////////////
bool ScEngine::tick() {
	if (_scripts.getSize() == 0) {
		return STATUS_OK;
	}


	// resolve waiting scripts
	for (int i = 0; i < _scripts.getSize(); i++) {

		switch (_scripts[i]->_state) {
		case SCRIPT_WAITING: {
			/*
			bool obj_found=false;
			for(int j=0; j<_gameRef->_regObjects.getSize(); j++)
			{
			    if (_gameRef->_regObjects[j] == _scripts[i]->_waitObject)
			    {
			        if (_gameRef->_regObjects[j]->IsReady()) _scripts[i]->Run();
			        obj_found = true;
			        break;
			    }
			}
			if (!obj_found) _scripts[i]->finish(); // _waitObject no longer exists
			*/
			if (_gameRef->validObject(_scripts[i]->_waitObject)) {
				if (_scripts[i]->_waitObject->isReady()) {
					_scripts[i]->run();
				}
			} else {
				_scripts[i]->finish();
			}
			break;
		}

		case SCRIPT_SLEEPING: {
			if (_scripts[i]->_waitFrozen) {
				if (_scripts[i]->_waitTime <= g_system->getMillis()) {
					_scripts[i]->run();
				}
			} else {
				if (_scripts[i]->_waitTime <= _gameRef->_timer) {
					_scripts[i]->run();
				}
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
			//warning("ScEngine::Tick - Unhandled enum");
			break;
		} // switch
	} // for each script


	// execute scripts
	for (int i = 0; i < _scripts.getSize(); i++) {

		// skip paused scripts
		if (_scripts[i]->_state == SCRIPT_PAUSED) {
			continue;
		}

		// time sliced script
		if (_scripts[i]->_timeSlice > 0) {
			uint32 startTime = g_system->getMillis();
			while (_scripts[i]->_state == SCRIPT_RUNNING && g_system->getMillis() - startTime < _scripts[i]->_timeSlice) {
				_currentScript = _scripts[i];
				_scripts[i]->executeInstruction();
			}
			if (_isProfiling && _scripts[i]->_filename) {
				addScriptTime(_scripts[i]->_filename, g_system->getMillis() - startTime);
			}
		}

		// normal script
		else {
			uint32 startTime = 0;
			bool isProfiling = _isProfiling;
			if (isProfiling) {
				startTime = g_system->getMillis();
			}

			while (_scripts[i]->_state == SCRIPT_RUNNING) {
				_currentScript = _scripts[i];
				_scripts[i]->executeInstruction();
			}
			if (isProfiling && _scripts[i]->_filename) {
				addScriptTime(_scripts[i]->_filename, g_system->getMillis() - startTime);
			}
		}
		_currentScript = NULL;
	}

	removeFinishedScripts();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScEngine::tickUnbreakable() {
	// execute unbreakable scripts
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (!_scripts[i]->_unbreakable) {
			continue;
		}

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
bool ScEngine::removeFinishedScripts() {
	// remove finished scripts
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_FINISHED || _scripts[i]->_state == SCRIPT_ERROR) {
			if (!_scripts[i]->_thread && _scripts[i]->_owner) {
				_scripts[i]->_owner->removeScript(_scripts[i]);
			}
			_gameRef->getDebugMgr()->onScriptShutdown(_scripts[i]);
			delete _scripts[i];
			_scripts.remove_at(i);
			i--;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int ScEngine::getNumScripts(int *running, int *waiting, int *persistent) {
	int numRunning = 0, numWaiting = 0, numPersistent = 0, numTotal = 0;

	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_FINISHED) {
			continue;
		}
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
			warning("ScEngine::GetNumScripts - unhandled enum");
			break;
		}
		numTotal++;
	}
	if (running) {
		*running = numRunning;
	}
	if (waiting) {
		*waiting = numWaiting;
	}
	if (persistent) {
		*persistent = numPersistent;
	}

	return numTotal;
}


//////////////////////////////////////////////////////////////////////////
bool ScEngine::emptyScriptCache() {
	for (int i = 0; i < MAX_CACHED_SCRIPTS; i++) {
		if (_cachedScripts[i]) {
			delete _cachedScripts[i];
			_cachedScripts[i] = NULL;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScEngine::resetObject(BaseObject *Object) {
	// terminate all scripts waiting for this object
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_WAITING && _scripts[i]->_waitObject == Object) {
			if (!_gameRef->_compatKillMethodThreads) {
				resetScript(_scripts[i]);
			}

			bool isThread = _scripts[i]->_methodThread || _scripts[i]->_thread;
			_scripts[i]->finish(!isThread); // 1.9b1 - top-level script kills its threads as well
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool ScEngine::resetScript(ScScript *script) {
	// terminate all scripts waiting for this script
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_WAITING_SCRIPT && _scripts[i]->_waitScript == script) {
			_scripts[i]->finish();
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool ScEngine::persist(BasePersistenceManager *persistMgr) {
	if (!persistMgr->getIsSaving()) {
		cleanup();
	}

	persistMgr->transfer(TMEMBER(_gameRef));
	persistMgr->transfer(TMEMBER(_currentScript));
	persistMgr->transfer(TMEMBER(_globals));
	_scripts.persist(persistMgr);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void ScEngine::editorCleanup() {
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_owner == NULL && (_scripts[i]->_state == SCRIPT_FINISHED || _scripts[i]->_state == SCRIPT_ERROR)) {
			delete _scripts[i];
			_scripts.remove_at(i);
			i--;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
bool ScEngine::pauseAll() {
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i] != _currentScript) {
			_scripts[i]->pause();
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScEngine::resumeAll() {
	for (int i = 0; i < _scripts.getSize(); i++) {
		_scripts[i]->resume();
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScEngine::isValidScript(ScScript *script) {
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i] == script) {
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool ScEngine::clearGlobals(bool includingNatives) {
	_globals->CleanProps(includingNatives);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool ScEngine::dbgSendScripts(IWmeDebugClient *client) {
	// send global variables
	_globals->dbgSendVariables(client, WME_DBGVAR_GLOBAL, NULL, 0);

	// process normal scripts first
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_thread || _scripts[i]->_methodThread) {
			continue;
		}
		_scripts[i]->dbgSendScript(client);
	}

	// and threads later
	for (int i = 0; i < _scripts.getSize(); i++) {
		if (_scripts[i]->_thread || _scripts[i]->_methodThread) {
			_scripts[i]->dbgSendScript(client);
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool ScEngine::addBreakpoint(const char *scriptFilename, int line) {
	if (!_gameRef->getDebugMgr()->_enabled) {
		return STATUS_OK;
	}

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
		if (bp->_lines[i] == line) {
			return STATUS_OK;
		}
	}
	bp->_lines.add(line);

	// refresh changes
	refreshScriptBreakpoints();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool ScEngine::removeBreakpoint(const char *scriptFilename, int line) {
	if (!_gameRef->getDebugMgr()->_enabled) {
		return STATUS_OK;
	}

	for (int i = 0; i < _breakpoints.getSize(); i++) {
		if (scumm_stricmp(_breakpoints[i]->_filename.c_str(), scriptFilename) == 0) {
			for (int j = 0; j < _breakpoints[i]->_lines.getSize(); j++) {
				if (_breakpoints[i]->_lines[j] == line) {
					_breakpoints[i]->_lines.remove_at(j);
					if (_breakpoints[i]->_lines.getSize() == 0) {
						delete _breakpoints[i];
						_breakpoints.remove_at(i);
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
bool ScEngine::refreshScriptBreakpoints() {
	if (!_gameRef->getDebugMgr()->_enabled) {
		return STATUS_OK;
	}

	for (int i = 0; i < _scripts.getSize(); i++) {
		refreshScriptBreakpoints(_scripts[i]);
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool ScEngine::refreshScriptBreakpoints(ScScript *script) {
	if (!_gameRef->getDebugMgr()->_enabled) {
		return STATUS_OK;
	}

	if (!script || !script->_filename) {
		return STATUS_FAILED;
	}

	for (int i = 0; i < _breakpoints.getSize(); i++) {
		if (scumm_stricmp(_breakpoints[i]->_filename.c_str(), script->_filename) == 0) {
			script->_breakpoints.copy(_breakpoints[i]->_lines);
			return STATUS_OK;
		}
	}
	if (script->_breakpoints.getSize() > 0) {
		script->_breakpoints.clear();
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool ScEngine::saveBreakpoints() {
	if (!_gameRef->getDebugMgr()->_enabled) {
		return STATUS_OK;
	}


	char text[512];
	char key[100];

	int count = 0;
	for (int i = 0; i < _breakpoints.getSize(); i++) {
		for (int j = 0; j < _breakpoints[i]->_lines.getSize(); j++) {
			count++;
			sprintf(key, "Breakpoint%d", count);
			sprintf(text, "%s:%d", _breakpoints[i]->_filename.c_str(), _breakpoints[i]->_lines[j]);

			_gameRef->_registry->writeString("Debug", key, text);
		}
	}
	_gameRef->_registry->writeInt("Debug", "NumBreakpoints", count);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool ScEngine::loadBreakpoints() {
	if (!_gameRef->getDebugMgr()->_enabled) {
		return STATUS_OK;
	}

	char key[100];

	int count = _gameRef->_registry->readInt("Debug", "NumBreakpoints", 0);
	for (int i = 1; i <= count; i++) {
		/*  uint32 bufSize = 512; */
		sprintf(key, "Breakpoint%d", i);
		AnsiString breakpoint = _gameRef->_registry->readString("Debug", key, "");

		char *path = BaseUtils::strEntry(0, breakpoint.c_str(), ':');
		char *line = BaseUtils::strEntry(1, breakpoint.c_str(), ':');

		if (path != NULL && line != NULL) {
			addBreakpoint(path, atoi(line));
		}
		delete[] path;
		delete[] line;
		path = NULL;
		line = NULL;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void ScEngine::addScriptTime(const char *filename, uint32 time) {
	if (!_isProfiling) {
		return;
	}

	AnsiString fileName = filename;
	fileName.toLowercase();
	_scriptTimes[fileName] += time;
}


//////////////////////////////////////////////////////////////////////////
void ScEngine::enableProfiling() {
	if (_isProfiling) {
		return;
	}

	// destroy old data, if any
	_scriptTimes.clear();

	_profilingStartTime = g_system->getMillis();
	_isProfiling = true;
}


//////////////////////////////////////////////////////////////////////////
void ScEngine::disableProfiling() {
	if (!_isProfiling) {
		return;
	}

	dumpStats();
	_isProfiling = false;
}


//////////////////////////////////////////////////////////////////////////
void ScEngine::dumpStats() {
	error("DumpStats not ported to ScummVM yet");
	/*  uint32 totalTime = g_system->getMillis() - _profilingStartTime;

	    typedef std::vector <std::pair<uint32, std::string> > TimeVector;
	    TimeVector times;

	    ScriptTimes::iterator it;
	    for (it = _scriptTimes.begin(); it != _scriptTimes.end(); it++) {
	        times.push_back(std::pair<uint32, std::string> (it->_value, it->_key));
	    }
	    std::sort(times.begin(), times.end());


	    TimeVector::reverse_iterator tit;

	    _gameRef->LOG(0, "***** Script profiling information: *****");
	    _gameRef->LOG(0, "  %-40s %fs", "Total execution time", (float)totalTime / 1000);

	    for (tit = times.rbegin(); tit != times.rend(); tit++) {
	        _gameRef->LOG(0, "  %-40s %fs (%f%%)", tit->second.c_str(), (float)tit->first / 1000, (float)tit->first / (float)totalTime * 100);
	    }*/
}

} // end of namespace WinterMute
