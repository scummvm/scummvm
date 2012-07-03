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

	/*
	#ifdef __WIN32__
	    char CompilerPath[MAX_PATH];
	    strcpy(CompilerPath, COMPILER_DLL);

	    _compilerDLL = ::LoadLibrary(CompilerPath);
	    if (_compilerDLL == NULL) {
	        char ModuleName[MAX_PATH];
	        ::GetModuleFileName(NULL, ModuleName, MAX_PATH);

	        // switch to exe's dir
	        char *ExeDir = CBUtils::GetPath(ModuleName);
	        sprintf(CompilerPath, "%s%s", ExeDir, COMPILER_DLL);
	        _compilerDLL = ::LoadLibrary(CompilerPath);

	        delete [] ExeDir;
	    }
	    if (_compilerDLL != NULL) {
	        // bind compiler's functionality
	        ExtCompileBuffer  = (DLL_COMPILE_BUFFER) ::GetProcAddress(_compilerDLL, "CompileBuffer");
	        ExtCompileFile    = (DLL_COMPILE_FILE)   ::GetProcAddress(_compilerDLL, "CompileFile");
	        ExtReleaseBuffer  = (DLL_RELEASE_BUFFER) ::GetProcAddress(_compilerDLL, "ReleaseBuffer");
	        ExtSetCallbacks   = (DLL_SET_CALLBACKS)  ::GetProcAddress(_compilerDLL, "SetCallbacks");
	        ExtDefineFunction = (DLL_DEFINE_FUNCTION)::GetProcAddress(_compilerDLL, "DefineFunction");
	        ExtDefineVariable = (DLL_DEFINE_VARIABLE)::GetProcAddress(_compilerDLL, "DefineVariable");

	        if (!ExtCompileBuffer || !ExtCompileFile || !ExtReleaseBuffer || !ExtSetCallbacks || !ExtDefineFunction || !ExtDefineVariable) {
	            _compilerAvailable = false;
	            ::FreeLibrary(_compilerDLL);
	            _compilerDLL = NULL;
	        } else {
	        */  /*
            // publish external methods to the compiler
            CALLBACKS c;
            c.Dll_AddError = AddError;
            c.Dll_CloseFile = CloseFile;
            c.Dll_LoadFile = LoadFile;
            ExtSetCallbacks(&c, Game);
            */
	/*
	            _compilerAvailable = true;
	        }
	    } else _compilerAvailable = false;
	#else
	*/
	_compilerAvailable = false;
	_compilerDLL = 0;
//#endif


	if (_compilerAvailable) Game->LOG(0, "  Script compiler bound successfuly");
	else Game->LOG(0, "  Script compiler is NOT available");

	_globals = new CScValue(Game);


	// register 'Game' as global variable
	if (!_globals->PropExists("Game")) {
		CScValue val(Game);
		val.SetNative(Game, true);
		_globals->SetProp("Game", &val);
	}

	// register 'Math' as global variable
	if (!_globals->PropExists("Math")) {
		CScValue val(Game);
		val.SetNative(Game->_mathClass, true);
		_globals->SetProp("Math", &val);
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
	SaveBreakpoints();

	DisableProfiling();
#ifdef __WIN32__
	if (_compilerAvailable && _compilerDLL) ::FreeLibrary(_compilerDLL);
#endif
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

	EmptyScriptCache();

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
byte *WINAPI CScEngine::loadFile(void *Data, char *Filename, uint32 *Size) {
	CBGame *Game = (CBGame *)Data;
	return Game->_fileManager->readWholeFile(Filename, Size);
}


//////////////////////////////////////////////////////////////////////////
void WINAPI CScEngine::CloseFile(void *Data, byte *Buffer) {
	delete [] Buffer;
}

//////////////////////////////////////////////////////////////////////////
void WINAPI CScEngine::AddError(void *Data, int Line, char *Text) {
	CBGame *Game = (CBGame *)Data;

	if (Game) {
		if (Game->_scEngine && Game->_scEngine->_fileToCompile)
			Game->LOG(0, "Compiling script '%s'...", Game->_scEngine->_fileToCompile);
		Game->LOG(0, "  Error@line %d: %s", Line, Text);


		// redirect to an engine's own callback
		if (Game->_scEngine && Game->_scEngine->_compileErrorCallback) {
			Game->_scEngine->_compileErrorCallback(Line, Text, Game->_scEngine->_compileErrorCallbackData);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void WINAPI CScEngine::ParseElement(void *Data, int Line, int Type, void *ElementData) {
	CBGame *Game = (CBGame *)Data;

	if (Game) {
		// redirect to an engine's own callback
		if (Game->_scEngine && Game->_scEngine->_parseElementCallback) {
			Game->_scEngine->_parseElementCallback(Line, Type, ElementData, Game->_scEngine->_compileErrorCallbackData);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
CScScript *CScEngine::RunScript(const char *Filename, CBScriptHolder *Owner) {
	byte *CompBuffer;
	uint32 CompSize;

	// get script from cache
	CompBuffer = GetCompiledScript(Filename, &CompSize);
	if (!CompBuffer) return NULL;

	// add new script
	CScScript *script = new CScScript(Game, this);
	HRESULT ret = script->Create(Filename, CompBuffer, CompSize, Owner);
	if (FAILED(ret)) {
		Game->LOG(ret, "Error running script '%s'...", Filename);
		delete script;
		return NULL;
	} else {
		// publish the "self" pseudo-variable
		CScValue val(Game);
		if (Owner)val.SetNative(Owner, true);
		else val.SetNULL();

		script->_globals->SetProp("self", &val);
		script->_globals->SetProp("this", &val);

		_scripts.Add(script);
		Game->GetDebugMgr()->OnScriptInit(script);

		return script;
	}
}


//////////////////////////////////////////////////////////////////////////
byte *CScEngine::GetCompiledScript(const char *Filename, uint32 *OutSize, bool IgnoreCache) {
	int i;

	// is script in cache?
	if (!IgnoreCache) {
		for (i = 0; i < MAX_CACHED_SCRIPTS; i++) {
			if (_cachedScripts[i] && scumm_stricmp(_cachedScripts[i]->_filename.c_str(), Filename) == 0) {
				_cachedScripts[i]->_timestamp = CBPlatform::GetTime();
				*OutSize = _cachedScripts[i]->_size;
				return _cachedScripts[i]->_buffer;
			}
		}
	}

	// nope, load it
	byte *CompBuffer;
	uint32 CompSize;
	bool CompiledNow = false;

	uint32 Size;

	byte *Buffer = Game->_fileManager->readWholeFile(Filename, &Size);
	if (!Buffer) {
		Game->LOG(0, "CScEngine::GetCompiledScript - error opening script '%s'", Filename);
		return NULL;
	}

	// needs to be compiled?
	if (FROM_LE_32(*(uint32 *)Buffer) == SCRIPT_MAGIC) {
		CompBuffer = Buffer;
		CompSize = Size;
	} else {
		if (!_compilerAvailable) {
			Game->LOG(0, "CScEngine::GetCompiledScript - script '%s' needs to be compiled but compiler is not available", Filename);
			delete [] Buffer;
			return NULL;
		}

		CompiledNow = true;

		// publish external methods to the compiler
		CALLBACKS c;
		c.Dll_AddError = AddError;
		c.Dll_CloseFile = CloseFile;
		c.Dll_LoadFile = loadFile;
		c.Dll_ParseElement = ParseElement;
		ExtSetCallbacks(&c, Game);

		// publish native interfaces
		Game->PublishNatives();

		// We have const char* everywhere but in the DLL-interfaces...
		char *tempFileName = new char[strlen(Filename) + 1];
		memcpy(tempFileName, Filename, strlen(Filename) + 1);

		SetFileToCompile(Filename);
		CompBuffer = ExtCompileFile(tempFileName, &CompSize);
		delete[] tempFileName;
		if (!CompBuffer) {
			Game->QuickMessage("Script compiler error. View log for details.");
			delete [] Buffer;
			return NULL;
		}
	}

	byte *ret = NULL;

	// add script to cache
	CScCachedScript *CachedScript = new CScCachedScript(Filename, CompBuffer, CompSize);
	if (CachedScript) {
		int index = 0;
		uint32 MinTime = CBPlatform::GetTime();
		for (i = 0; i < MAX_CACHED_SCRIPTS; i++) {
			if (_cachedScripts[i] == NULL) {
				index = i;
				break;
			} else if (_cachedScripts[i]->_timestamp <= MinTime) {
				MinTime = _cachedScripts[i]->_timestamp;
				index = i;
			}
		}

		if (_cachedScripts[index] != NULL) delete _cachedScripts[index];
		_cachedScripts[index] = CachedScript;

		ret = CachedScript->_buffer;
		*OutSize = CachedScript->_size;
	}


	// cleanup
	delete [] Buffer;
	if (CompiledNow) ExtReleaseBuffer(CompBuffer);

	return ret;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::Tick() {
	int i;

	if (_scripts.GetSize() == 0) return S_OK;


	// resolve waiting scripts
	for (i = 0; i < _scripts.GetSize(); i++) {

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
			if (Game->ValidObject(_scripts[i]->_waitObject)) {
				if (_scripts[i]->_waitObject->isReady()) _scripts[i]->Run();
			} else _scripts[i]->finish();
			break;
		}

		case SCRIPT_SLEEPING: {
			if (_scripts[i]->_waitFrozen) {
				if (_scripts[i]->_waitTime <= CBPlatform::GetTime()) _scripts[i]->Run();
			} else {
				if (_scripts[i]->_waitTime <= Game->_timer) _scripts[i]->Run();
			}
			break;
		}

		case SCRIPT_WAITING_SCRIPT: {
			if (!IsValidScript(_scripts[i]->_waitScript) || _scripts[i]->_waitScript->_state == SCRIPT_ERROR) {
				// fake return value
				_scripts[i]->_stack->PushNULL();
				_scripts[i]->_waitScript = NULL;
				_scripts[i]->Run();
			} else {
				if (_scripts[i]->_waitScript->_state == SCRIPT_THREAD_FINISHED) {
					// copy return value
					_scripts[i]->_stack->Push(_scripts[i]->_waitScript->_stack->Pop());
					_scripts[i]->Run();
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
	for (i = 0; i < _scripts.GetSize(); i++) {

		// skip paused scripts
		if (_scripts[i]->_state == SCRIPT_PAUSED) continue;

		// time sliced script
		if (_scripts[i]->_timeSlice > 0) {
			uint32 StartTime = CBPlatform::GetTime();
			while (_scripts[i]->_state == SCRIPT_RUNNING && CBPlatform::GetTime() - StartTime < _scripts[i]->_timeSlice) {
				_currentScript = _scripts[i];
				_scripts[i]->ExecuteInstruction();
			}
			if (_isProfiling && _scripts[i]->_filename) AddScriptTime(_scripts[i]->_filename, CBPlatform::GetTime() - StartTime);
		}

		// normal script
		else {
			uint32 StartTime = 0;
			bool isProfiling = _isProfiling;
			if (isProfiling) StartTime = CBPlatform::GetTime();

			while (_scripts[i]->_state == SCRIPT_RUNNING) {
				_currentScript = _scripts[i];
				_scripts[i]->ExecuteInstruction();
			}
			if (isProfiling && _scripts[i]->_filename) AddScriptTime(_scripts[i]->_filename, CBPlatform::GetTime() - StartTime);
		}
		_currentScript = NULL;
	}

	RemoveFinishedScripts();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::TickUnbreakable() {
	// execute unbreakable scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (!_scripts[i]->_unbreakable) continue;

		while (_scripts[i]->_state == SCRIPT_RUNNING) {
			_currentScript = _scripts[i];
			_scripts[i]->ExecuteInstruction();
		}
		_scripts[i]->finish();
		_currentScript = NULL;
	}
	RemoveFinishedScripts();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::RemoveFinishedScripts() {
	// remove finished scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_FINISHED || _scripts[i]->_state == SCRIPT_ERROR) {
			if (!_scripts[i]->_thread && _scripts[i]->_owner) _scripts[i]->_owner->removeScript(_scripts[i]);
			Game->GetDebugMgr()->OnScriptShutdown(_scripts[i]);
			delete _scripts[i];
			_scripts.RemoveAt(i);
			i--;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CScEngine::GetNumScripts(int *Running, int *Waiting, int *Persistent) {
	int running = 0, waiting = 0, persistent = 0, total = 0;

	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_FINISHED) continue;
		switch (_scripts[i]->_state) {
		case SCRIPT_RUNNING:
		case SCRIPT_SLEEPING:
		case SCRIPT_PAUSED:
			running++;
			break;
		case SCRIPT_WAITING:
			waiting++;
			break;
		case SCRIPT_PERSISTENT:
			persistent++;
			break;
		default:
			warning("CScEngine::GetNumScripts - unhandled enum");
			break;
		}
		total++;
	}
	if (Running) *Running = running;
	if (Waiting) *Waiting = waiting;
	if (Persistent)  *Persistent = persistent;

	return total;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::EmptyScriptCache() {
	for (int i = 0; i < MAX_CACHED_SCRIPTS; i++) {
		if (_cachedScripts[i]) {
			delete _cachedScripts[i];
			_cachedScripts[i] = NULL;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::ResetObject(CBObject *Object) {
	// terminate all scripts waiting for this object
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_state == SCRIPT_WAITING && _scripts[i]->_waitObject == Object) {
			if (!Game->_compatKillMethodThreads) ResetScript(_scripts[i]);

			bool IsThread = _scripts[i]->_methodThread || _scripts[i]->_thread;
			_scripts[i]->finish(!IsThread); // 1.9b1 - top-level script kills its threads as well
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::ResetScript(CScScript *script) {
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
HRESULT CScEngine::PauseAll() {
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i] != _currentScript) _scripts[i]->Pause();
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::ResumeAll() {
	for (int i = 0; i < _scripts.GetSize(); i++)
		_scripts[i]->Resume();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::SetFileToCompile(const char *Filename) {
	delete[] _fileToCompile;
	_fileToCompile = new char[strlen(Filename) + 1];
	if (_fileToCompile) {
		strcpy(_fileToCompile, Filename);
		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::SetCompileErrorCallback(COMPILE_ERROR_CALLBACK Callback, void *Data) {
	_compileErrorCallback = Callback;
	_compileErrorCallbackData = Data;
}

//////////////////////////////////////////////////////////////////////////
void CScEngine::SetParseElementCallback(PARSE_ELEMENT_CALLBACK Callback, void *Data) {
	_parseElementCallback = Callback;
	_parseElementCallbackData = Data;
}


//////////////////////////////////////////////////////////////////////////
bool CScEngine::IsValidScript(CScScript *script) {
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i] == script) return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::ClearGlobals(bool IncludingNatives) {
	_globals->CleanProps(IncludingNatives);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::DbgSendScripts(IWmeDebugClient *Client) {
	// send global variables
	_globals->DbgSendVariables(Client, WME_DBGVAR_GLOBAL, NULL, 0);

	// process normal scripts first
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_thread || _scripts[i]->_methodThread) continue;
		_scripts[i]->DbgSendScript(Client);
	}

	// and threads later
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i]->_thread || _scripts[i]->_methodThread)
			_scripts[i]->DbgSendScript(Client);
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::AddBreakpoint(const char *ScriptFilename, int Line) {
	if (!Game->GetDebugMgr()->_enabled) return S_OK;

	CScBreakpoint *Bp = NULL;
	for (int i = 0; i < _breakpoints.GetSize(); i++) {
		if (scumm_stricmp(_breakpoints[i]->_filename.c_str(), ScriptFilename) == 0) {
			Bp = _breakpoints[i];
			break;
		}
	}
	if (Bp == NULL) {
		Bp = new CScBreakpoint(ScriptFilename);
		_breakpoints.Add(Bp);
	}

	for (int i = 0; i < Bp->_lines.GetSize(); i++) {
		if (Bp->_lines[i] == Line) return S_OK;
	}
	Bp->_lines.Add(Line);

	// refresh changes
	RefreshScriptBreakpoints();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::RemoveBreakpoint(const char *ScriptFilename, int Line) {
	if (!Game->GetDebugMgr()->_enabled) return S_OK;

	for (int i = 0; i < _breakpoints.GetSize(); i++) {
		if (scumm_stricmp(_breakpoints[i]->_filename.c_str(), ScriptFilename) == 0) {
			for (int j = 0; j < _breakpoints[i]->_lines.GetSize(); j++) {
				if (_breakpoints[i]->_lines[j] == Line) {
					_breakpoints[i]->_lines.RemoveAt(j);
					if (_breakpoints[i]->_lines.GetSize() == 0) {
						delete _breakpoints[i];
						_breakpoints.RemoveAt(i);
					}
					// refresh changes
					RefreshScriptBreakpoints();

					return S_OK;
				}
			}
			break;
		}
	}
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::RefreshScriptBreakpoints() {
	if (!Game->GetDebugMgr()->_enabled) return S_OK;

	for (int i = 0; i < _scripts.GetSize(); i++) {
		RefreshScriptBreakpoints(_scripts[i]);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::RefreshScriptBreakpoints(CScScript *script) {
	if (!Game->GetDebugMgr()->_enabled) return S_OK;

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
HRESULT CScEngine::SaveBreakpoints() {
	if (!Game->GetDebugMgr()->_enabled) return S_OK;


	char Text[512];
	char Key[100];

	int Count = 0;
	for (int i = 0; i < _breakpoints.GetSize(); i++) {
		for (int j = 0; j < _breakpoints[i]->_lines.GetSize(); j++) {
			Count++;
			sprintf(Key, "Breakpoint%d", Count);
			sprintf(Text, "%s:%d", _breakpoints[i]->_filename.c_str(), _breakpoints[i]->_lines[j]);

			Game->_registry->WriteString("Debug", Key, Text);
		}
	}
	Game->_registry->WriteInt("Debug", "NumBreakpoints", Count);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScEngine::LoadBreakpoints() {
	if (!Game->GetDebugMgr()->_enabled) return S_OK;

	char Key[100];

	int Count = Game->_registry->ReadInt("Debug", "NumBreakpoints", 0);
	for (int i = 1; i <= Count; i++) {
		/*  uint32 BufSize = 512; */
		sprintf(Key, "Breakpoint%d", i);
		AnsiString breakpoint = Game->_registry->ReadString("Debug", Key, "");

		char *Path = CBUtils::StrEntry(0, breakpoint.c_str(), ':');
		char *Line = CBUtils::StrEntry(1, breakpoint.c_str(), ':');

		if (Path != NULL && Line != NULL) AddBreakpoint(Path, atoi(Line));
		delete[] Path;
		delete[] Line;
		Path = NULL;
		Line = NULL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::AddScriptTime(const char *Filename, uint32 Time) {
	if (!_isProfiling) return;

	AnsiString fileName = Filename;
	StringUtil::ToLowerCase(fileName);
	_scriptTimes[fileName] += Time;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::EnableProfiling() {
	if (_isProfiling) return;

	// destroy old data, if any
	_scriptTimes.clear();

	_profilingStartTime = CBPlatform::GetTime();
	_isProfiling = true;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::DisableProfiling() {
	if (!_isProfiling) return;

	DumpStats();
	_isProfiling = false;
}


//////////////////////////////////////////////////////////////////////////
void CScEngine::DumpStats() {
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
