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
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/scriptables/ScEngine.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CScScript, false)

//////////////////////////////////////////////////////////////////////////
CScScript::CScScript(CBGame *inGame, CScEngine *Engine): CBBase(inGame) {
	_buffer = NULL;
	_bufferSize = _iP = 0;
	_filename = NULL;
	_currentLine = 0;

	_symbols = NULL;
	_numSymbols = 0;

	_engine = Engine;

	_globals = NULL;

	_scopeStack = NULL;
	_callStack  = NULL;
	_thisStack  = NULL;
	_stack      = NULL;

	_operand    = NULL;
	_reg1       = NULL;

	_functions = NULL;
	_numFunctions = 0;

	_methods = NULL;
	_numMethods = 0;

	_events = NULL;
	_numEvents = 0;

	_externals = NULL;
	_numExternals = 0;

	_state = SCRIPT_FINISHED;
	_origState = SCRIPT_FINISHED;

	_waitObject = NULL;
	_waitTime = 0;
	_waitFrozen = false;
	_waitScript = NULL;

	_timeSlice = 0;

	_thread = false;
	_methodThread = false;
	_threadEvent = NULL;

	_freezable = true;
	_owner = NULL;

	_unbreakable = false;
	_parentScript = NULL;

	_tracingMode = false;
}


//////////////////////////////////////////////////////////////////////////
CScScript::~CScScript() {
	cleanup();
}



//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::InitScript() {
	TScriptHeader *Header = (TScriptHeader *)_buffer;
	if (Header->magic != SCRIPT_MAGIC) {
		Game->LOG(0, "File '%s' is not a valid compiled script", _filename);
		cleanup();
		return E_FAIL;
	}

	if (Header->version > SCRIPT_VERSION) {
		Game->LOG(0, "Script '%s' has a wrong version %d.%d (expected %d.%d)", _filename, Header->version / 256, Header->version % 256, SCRIPT_VERSION / 256, SCRIPT_VERSION % 256);
		cleanup();
		return E_FAIL;
	}

	InitTables();

	// init stacks
	_scopeStack = new CScStack(Game);
	_callStack  = new CScStack(Game);
	_thisStack  = new CScStack(Game);
	_stack      = new CScStack(Game);

	_operand    = new CScValue(Game);
	_reg1       = new CScValue(Game);


	// skip to the beginning
	_iP = Header->code_start;
	_currentLine = 0;

	// init breakpoints
	_engine->RefreshScriptBreakpoints(this);


	// ready to rumble...
	_state = SCRIPT_RUNNING;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::InitTables() {
	uint32 OrigIP = _iP;

	TScriptHeader *Header = (TScriptHeader *)_buffer;

	int32 i;

	// load symbol table
	_iP = Header->symbol_table;

	_numSymbols = GetDWORD();
	_symbols = new char*[_numSymbols];
	for (i = 0; i < _numSymbols; i++) {
		uint32 index = GetDWORD();
		_symbols[index] = GetString();
	}

	// load functions table
	_iP = Header->func_table;

	_numFunctions = GetDWORD();
	_functions = new TFunctionPos[_numFunctions];
	for (i = 0; i < _numFunctions; i++) {
		_functions[i].pos = GetDWORD();
		_functions[i].name = GetString();
	}


	// load events table
	_iP = Header->event_table;

	_numEvents = GetDWORD();
	_events = new TEventPos[_numEvents];
	for (i = 0; i < _numEvents; i++) {
		_events[i].pos = GetDWORD();
		_events[i].name = GetString();
	}


	// load externals
	if (Header->version >= 0x0101) {
		_iP = Header->externals_table;

		_numExternals = GetDWORD();
		_externals = new TExternalFunction[_numExternals];
		for (i = 0; i < _numExternals; i++) {
			_externals[i].dll_name = GetString();
			_externals[i].name = GetString();
			_externals[i].call_type = (TCallType)GetDWORD();
			_externals[i].returns = (TExternalType)GetDWORD();
			_externals[i].nu_params = GetDWORD();
			if (_externals[i].nu_params > 0) {
				_externals[i].params = new TExternalType[_externals[i].nu_params];
				for (int j = 0; j < _externals[i].nu_params; j++) {
					_externals[i].params[j] = (TExternalType)GetDWORD();
				}
			}
		}
	}

	// load method table
	_iP = Header->method_table;

	_numMethods = GetDWORD();
	_methods = new TMethodPos[_numMethods];
	for (i = 0; i < _numMethods; i++) {
		_methods[i].pos = GetDWORD();
		_methods[i].name = GetString();
	}


	_iP = OrigIP;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Create(const char *Filename, byte *Buffer, uint32 Size, CBScriptHolder *Owner) {
	cleanup();

	_thread = false;
	_methodThread = false;

	delete[] _threadEvent;
	_threadEvent = NULL;

	_filename = new char[strlen(Filename) + 1];
	if (_filename) strcpy(_filename, Filename);

	_buffer = new byte [Size];
	if (!_buffer) return E_FAIL;

	memcpy(_buffer, Buffer, Size);

	_bufferSize = Size;

	HRESULT res = InitScript();
	if (FAILED(res)) return res;

	// establish global variables table
	_globals = new CScValue(Game);

	_owner = Owner;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::CreateThread(CScScript *Original, uint32 InitIP, const char *EventName) {
	cleanup();

	_thread = true;
	_methodThread = false;
	_threadEvent = new char[strlen(EventName) + 1];
	if (_threadEvent) strcpy(_threadEvent, EventName);

	// copy filename
	_filename = new char[strlen(Original->_filename) + 1];
	if (_filename) strcpy(_filename, Original->_filename);

	// copy buffer
	_buffer = new byte [Original->_bufferSize];
	if (!_buffer) return E_FAIL;

	memcpy(_buffer, Original->_buffer, Original->_bufferSize);
	_bufferSize = Original->_bufferSize;

	// initialize
	HRESULT res = InitScript();
	if (FAILED(res)) return res;

	// copy globals
	_globals = Original->_globals;

	// skip to the beginning of the event
	_iP = InitIP;

	_timeSlice = Original->_timeSlice;
	_freezable = Original->_freezable;
	_owner = Original->_owner;

	_engine = Original->_engine;
	_parentScript = Original;

	return S_OK;
}




//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::CreateMethodThread(CScScript *Original, const char *MethodName) {
	uint32 IP = Original->GetMethodPos(MethodName);
	if (IP == 0) return E_FAIL;

	cleanup();

	_thread = true;
	_methodThread = true;
	_threadEvent = new char[strlen(MethodName) + 1];
	if (_threadEvent) strcpy(_threadEvent, MethodName);

	// copy filename
	_filename = new char[strlen(Original->_filename) + 1];
	if (_filename) strcpy(_filename, Original->_filename);

	// copy buffer
	_buffer = new byte [Original->_bufferSize];
	if (!_buffer) return E_FAIL;

	memcpy(_buffer, Original->_buffer, Original->_bufferSize);
	_bufferSize = Original->_bufferSize;

	// initialize
	HRESULT res = InitScript();
	if (FAILED(res)) return res;

	// copy globals
	_globals = Original->_globals;

	// skip to the beginning of the event
	_iP = IP;

	_timeSlice = Original->_timeSlice;
	_freezable = Original->_freezable;
	_owner = Original->_owner;

	_engine = Original->_engine;
	_parentScript = Original;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CScScript::cleanup() {
	if (_buffer) delete [] _buffer;
	_buffer = NULL;

	if (_filename) delete [] _filename;
	_filename = NULL;

	if (_symbols) delete [] _symbols;
	_symbols = NULL;
	_numSymbols = 0;

	if (_globals && !_thread) delete _globals;
	_globals = NULL;

	if (_scopeStack) delete _scopeStack;
	_scopeStack = NULL;

	if (_callStack) delete _callStack;
	_callStack = NULL;

	if (_thisStack) delete _thisStack;
	_thisStack = NULL;

	if (_stack) delete _stack;
	_stack = NULL;

	if (_functions) delete [] _functions;
	_functions = NULL;
	_numFunctions = 0;

	if (_methods) delete [] _methods;
	_methods = NULL;
	_numMethods = 0;

	if (_events) delete [] _events;
	_events = NULL;
	_numEvents = 0;


	if (_externals) {
		for (int i = 0; i < _numExternals; i++) {
			if (_externals[i].nu_params > 0) delete [] _externals[i].params;
		}
		delete [] _externals;
	}
	_externals = NULL;
	_numExternals = 0;

	delete _operand;
	delete _reg1;
	_operand = NULL;
	_reg1 = NULL;

	delete[] _threadEvent;
	_threadEvent = NULL;

	_state = SCRIPT_FINISHED;

	_waitObject = NULL;
	_waitTime = 0;
	_waitFrozen = false;
	_waitScript = NULL;

	_parentScript = NULL; // ref only
}


//////////////////////////////////////////////////////////////////////////
uint32 CScScript::GetDWORD() {
	uint32 ret = *(uint32 *)(_buffer + _iP);
	_iP += sizeof(uint32);

	return ret;
}


//////////////////////////////////////////////////////////////////////////
double CScScript::GetFloat() {
	double ret = *(double *)(_buffer + _iP);
	_iP += sizeof(double);

	return ret;
}


//////////////////////////////////////////////////////////////////////////
char *CScScript::GetString() {
	char *ret = (char *)(_buffer + _iP);
	while (*(char *)(_buffer + _iP) != '\0') _iP++;
	_iP++; // string terminator

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::ExecuteInstruction() {
	HRESULT ret = S_OK;

	uint32 dw;
	const char *str = NULL;

	//CScValue* op = new CScValue(Game);
	_operand->cleanup();

	CScValue *op1;
	CScValue *op2;

	uint32 inst = GetDWORD();
	switch (inst) {

	case II_DEF_VAR:
		_operand->SetNULL();
		dw = GetDWORD();
		if (_scopeStack->_sP < 0) {
			_globals->SetProp(_symbols[dw], _operand);
			if (Game->GetDebugMgr()->_enabled)
				Game->GetDebugMgr()->OnVariableInit(WME_DBGVAR_SCRIPT, this, NULL, _globals->GetProp(_symbols[dw]), _symbols[dw]);
		} else {
			_scopeStack->GetTop()->SetProp(_symbols[dw], _operand);
			if (Game->GetDebugMgr()->_enabled)
				Game->GetDebugMgr()->OnVariableInit(WME_DBGVAR_SCOPE, this, _scopeStack->GetTop(), _scopeStack->GetTop()->GetProp(_symbols[dw]), _symbols[dw]);
		}

		break;

	case II_DEF_GLOB_VAR:
	case II_DEF_CONST_VAR: {
		dw = GetDWORD();
		/*      char *Temp = _symbols[dw]; // TODO delete */
		// only create global var if it doesn't exist
		if (!_engine->_globals->PropExists(_symbols[dw])) {
			_operand->SetNULL();
			_engine->_globals->SetProp(_symbols[dw], _operand, false, inst == II_DEF_CONST_VAR);

			if (Game->GetDebugMgr()->_enabled)
				Game->GetDebugMgr()->OnVariableInit(WME_DBGVAR_GLOBAL, this, NULL, _engine->_globals->GetProp(_symbols[dw]), _symbols[dw]);
		}
		break;
	}

	case II_RET:
		if (_scopeStack->_sP >= 0 && _callStack->_sP >= 0) {
			Game->GetDebugMgr()->OnScriptShutdownScope(this, _scopeStack->GetTop());

			_scopeStack->Pop();
			_iP = (uint32)_callStack->Pop()->GetInt();

			if (_scopeStack->_sP < 0) Game->GetDebugMgr()->OnScriptChangeScope(this, NULL);
			else Game->GetDebugMgr()->OnScriptChangeScope(this, _scopeStack->GetTop());
		} else {
			if (_thread) {
				_state = SCRIPT_THREAD_FINISHED;
			} else {
				if (_numEvents == 0 && _numMethods == 0) _state = SCRIPT_FINISHED;
				else _state = SCRIPT_PERSISTENT;
			}
		}

		break;

	case II_RET_EVENT:
		_state = SCRIPT_FINISHED;
		break;


	case II_CALL:
		dw = GetDWORD();

		_operand->SetInt(_iP);
		_callStack->Push(_operand);

		_iP = dw;

		break;

	case II_CALL_BY_EXP: {
		// push var
		// push string
		str = _stack->Pop()->GetString();
		char *MethodName = new char[strlen(str) + 1];
		strcpy(MethodName, str);

		CScValue *var = _stack->Pop();
		if (var->_type == VAL_VARIABLE_REF) var = var->_valRef;

		HRESULT res = E_FAIL;
		bool TriedNative = false;

		// we are already calling this method, try native
		if (_thread && _methodThread && strcmp(MethodName, _threadEvent) == 0 && var->_type == VAL_NATIVE && _owner == var->GetNative()) {
			TriedNative = true;
			res = var->_valNative->scCallMethod(this, _stack, _thisStack, MethodName);
		}

		if (FAILED(res)) {
			if (var->IsNative() && var->GetNative()->canHandleMethod(MethodName)) {
				if (!_unbreakable) {
					_waitScript = var->GetNative()->invokeMethodThread(MethodName);
					if (!_waitScript) {
						_stack->CorrectParams(0);
						RuntimeError("Error invoking method '%s'.", MethodName);
						_stack->PushNULL();
					} else {
						_state = SCRIPT_WAITING_SCRIPT;
						_waitScript->CopyParameters(_stack);
					}
				} else {
					// can call methods in unbreakable mode
					_stack->CorrectParams(0);
					RuntimeError("Cannot call method '%s'. Ignored.", MethodName);
					_stack->PushNULL();
				}
				delete [] MethodName;
				break;
			}
			/*
			CScValue* val = var->GetProp(MethodName);
			if(val){
			    dw = GetFuncPos(val->GetString());
			    if(dw==0){
			        TExternalFunction* f = GetExternal(val->GetString());
			        if(f){
			            ExternalCall(_stack, _thisStack, f);
			        }
			        else{
			            // not an internal nor external, try for native function
			            Game->ExternalCall(this, _stack, _thisStack, val->GetString());
			        }
			    }
			    else{
			        _operand->SetInt(_iP);
			        _callStack->Push(_operand);
			        _iP = dw;
			    }
			}
			*/
			else {
				res = E_FAIL;
				if (var->_type == VAL_NATIVE && !TriedNative) res = var->_valNative->scCallMethod(this, _stack, _thisStack, MethodName);

				if (FAILED(res)) {
					_stack->CorrectParams(0);
					RuntimeError("Call to undefined method '%s'. Ignored.", MethodName);
					_stack->PushNULL();
				}
			}
		}
		delete [] MethodName;
	}
	break;

	case II_EXTERNAL_CALL: {
		uint32 SymbolIndex = GetDWORD();

		TExternalFunction *f = GetExternal(_symbols[SymbolIndex]);
		if (f) {
			ExternalCall(_stack, _thisStack, f);
		} else Game->ExternalCall(this, _stack, _thisStack, _symbols[SymbolIndex]);

		break;
	}
	case II_SCOPE:
		_operand->SetNULL();
		_scopeStack->Push(_operand);

		if (_scopeStack->_sP < 0) Game->GetDebugMgr()->OnScriptChangeScope(this, NULL);
		else Game->GetDebugMgr()->OnScriptChangeScope(this, _scopeStack->GetTop());

		break;

	case II_CORRECT_STACK:
		dw = GetDWORD(); // params expected
		_stack->CorrectParams(dw);
		break;

	case II_CREATE_OBJECT:
		_operand->SetObject();
		_stack->Push(_operand);
		break;

	case II_POP_EMPTY:
		_stack->Pop();
		break;

	case II_PUSH_VAR: {
		CScValue *var = GetVar(_symbols[GetDWORD()]);
		if (false && /*var->_type==VAL_OBJECT ||*/ var->_type == VAL_NATIVE) {
			_operand->SetReference(var);
			_stack->Push(_operand);
		} else _stack->Push(var);
		break;
	}

	case II_PUSH_VAR_REF: {
		CScValue *var = GetVar(_symbols[GetDWORD()]);
		_operand->SetReference(var);
		_stack->Push(_operand);
		break;
	}

	case II_POP_VAR: {
		char *VarName = _symbols[GetDWORD()];
		CScValue *var = GetVar(VarName);
		if (var) {
			CScValue *val = _stack->Pop();
			if (!val) {
				RuntimeError("Script stack corruption detected. Please report this script at WME bug reports forum.");
				var->SetNULL();
			} else {
				if (val->GetType() == VAL_VARIABLE_REF) val = val->_valRef;
				if (val->_type == VAL_NATIVE) var->SetValue(val);
				else {
					var->Copy(val);
				}
			}

			if (Game->GetDebugMgr()->_enabled)
				Game->GetDebugMgr()->OnVariableChangeValue(var, val);
		}

		break;
	}

	case II_PUSH_VAR_THIS:
		_stack->Push(_thisStack->GetTop());
		break;

	case II_PUSH_INT:
		_stack->PushInt((int)GetDWORD());
		break;

	case II_PUSH_FLOAT:
		_stack->PushFloat(GetFloat());
		break;


	case II_PUSH_BOOL:
		_stack->PushBool(GetDWORD() != 0);

		break;

	case II_PUSH_STRING:
		_stack->PushString(GetString());
		break;

	case II_PUSH_NULL:
		_stack->PushNULL();
		break;

	case II_PUSH_THIS_FROM_STACK:
		_operand->SetReference(_stack->GetTop());
		_thisStack->Push(_operand);
		break;

	case II_PUSH_THIS:
		_operand->SetReference(GetVar(_symbols[GetDWORD()]));
		_thisStack->Push(_operand);
		break;

	case II_POP_THIS:
		_thisStack->Pop();
		break;

	case II_PUSH_BY_EXP: {
		str = _stack->Pop()->GetString();
		CScValue *val = _stack->Pop()->GetProp(str);
		if (val) _stack->Push(val);
		else _stack->PushNULL();

		break;
	}

	case II_POP_BY_EXP: {
		str = _stack->Pop()->GetString();
		CScValue *var = _stack->Pop();
		CScValue *val = _stack->Pop();

		if (val == NULL) {
			RuntimeError("Script stack corruption detected. Please report this script at WME bug reports forum.");
			var->SetNULL();
		} else var->SetProp(str, val);

		if (Game->GetDebugMgr()->_enabled)
			Game->GetDebugMgr()->OnVariableChangeValue(var, NULL);

		break;
	}

	case II_PUSH_REG1:
		_stack->Push(_reg1);
		break;

	case II_POP_REG1:
		_reg1->Copy(_stack->Pop());
		break;

	case II_JMP:
		_iP = GetDWORD();
		break;

	case II_JMP_FALSE: {
		dw = GetDWORD();
		//if(!_stack->Pop()->GetBool()) _iP = dw;
		CScValue *Val = _stack->Pop();
		if (!Val) {
			RuntimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
		} else {
			if (!Val->GetBool()) _iP = dw;
		}
		break;
	}

	case II_ADD:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		if (op1->IsNULL() || op2->IsNULL()) _operand->SetNULL();
		else if (op1->GetType() == VAL_STRING || op2->GetType() == VAL_STRING) {
			char *tempStr = new char [strlen(op1->GetString()) + strlen(op2->GetString()) + 1];
			strcpy(tempStr, op1->GetString());
			strcat(tempStr, op2->GetString());
			_operand->SetString(tempStr);
			delete [] tempStr;
		} else if (op1->GetType() == VAL_INT && op2->GetType() == VAL_INT)
			_operand->SetInt(op1->GetInt() + op2->GetInt());
		else _operand->SetFloat(op1->GetFloat() + op2->GetFloat());

		_stack->Push(_operand);

		break;

	case II_SUB:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		if (op1->IsNULL() || op2->IsNULL()) _operand->SetNULL();
		else if (op1->GetType() == VAL_INT && op2->GetType() == VAL_INT)
			_operand->SetInt(op1->GetInt() - op2->GetInt());
		else _operand->SetFloat(op1->GetFloat() - op2->GetFloat());

		_stack->Push(_operand);

		break;

	case II_MUL:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		if (op1->IsNULL() || op2->IsNULL()) _operand->SetNULL();
		else if (op1->GetType() == VAL_INT && op2->GetType() == VAL_INT)
			_operand->SetInt(op1->GetInt() * op2->GetInt());
		else _operand->SetFloat(op1->GetFloat() * op2->GetFloat());

		_stack->Push(_operand);

		break;

	case II_DIV:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		if (op2->GetFloat() == 0.0f) RuntimeError("Division by zero.");

		if (op1->IsNULL() || op2->IsNULL() || op2->GetFloat() == 0.0f) _operand->SetNULL();
		else _operand->SetFloat(op1->GetFloat() / op2->GetFloat());

		_stack->Push(_operand);

		break;

	case II_MODULO:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		if (op2->GetInt() == 0) RuntimeError("Division by zero.");

		if (op1->IsNULL() || op2->IsNULL() || op2->GetInt() == 0) _operand->SetNULL();
		else _operand->SetInt(op1->GetInt() % op2->GetInt());

		_stack->Push(_operand);

		break;

	case II_NOT:
		op1 = _stack->Pop();
		//if(op1->IsNULL()) _operand->SetNULL();
		if (op1->IsNULL()) _operand->SetBool(true);
		else _operand->SetBool(!op1->GetBool());
		_stack->Push(_operand);

		break;

	case II_AND:
		op2 = _stack->Pop();
		op1 = _stack->Pop();
		if (op1 == NULL || op2 == NULL) {
			RuntimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
			_operand->SetBool(false);
		} else {
			_operand->SetBool(op1->GetBool() && op2->GetBool());
		}
		_stack->Push(_operand);
		break;

	case II_OR:
		op2 = _stack->Pop();
		op1 = _stack->Pop();
		if (op1 == NULL || op2 == NULL) {
			RuntimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
			_operand->SetBool(false);
		} else {
			_operand->SetBool(op1->GetBool() || op2->GetBool());
		}
		_stack->Push(_operand);
		break;

	case II_CMP_EQ:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		/*
		if((op1->IsNULL() && !op2->IsNULL()) || (!op1->IsNULL() && op2->IsNULL())) _operand->SetBool(false);
		else if(op1->IsNative() && op2->IsNative()){
		    _operand->SetBool(op1->GetNative() == op2->GetNative());
		}
		else if(op1->GetType()==VAL_STRING || op2->GetType()==VAL_STRING){
		    _operand->SetBool(scumm_stricmp(op1->GetString(), op2->GetString())==0);
		}
		else if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    _operand->SetBool(op1->GetFloat() == op2->GetFloat());
		}
		else{
		    _operand->SetBool(op1->GetInt() == op2->GetInt());
		}
		*/

		_operand->SetBool(CScValue::Compare(op1, op2) == 0);
		_stack->Push(_operand);
		break;

	case II_CMP_NE:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		/*
		if((op1->IsNULL() && !op2->IsNULL()) || (!op1->IsNULL() && op2->IsNULL())) _operand->SetBool(true);
		else if(op1->IsNative() && op2->IsNative()){
		    _operand->SetBool(op1->GetNative() != op2->GetNative());
		}
		else if(op1->GetType()==VAL_STRING || op2->GetType()==VAL_STRING){
		    _operand->SetBool(scumm_stricmp(op1->GetString(), op2->GetString())!=0);
		}
		else if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    _operand->SetBool(op1->GetFloat() != op2->GetFloat());
		}
		else{
		    _operand->SetBool(op1->GetInt() != op2->GetInt());
		}
		*/

		_operand->SetBool(CScValue::Compare(op1, op2) != 0);
		_stack->Push(_operand);
		break;

	case II_CMP_L:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		/*
		if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    _operand->SetBool(op1->GetFloat() < op2->GetFloat());
		}
		else _operand->SetBool(op1->GetInt() < op2->GetInt());
		*/

		_operand->SetBool(CScValue::Compare(op1, op2) < 0);
		_stack->Push(_operand);
		break;

	case II_CMP_G:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		/*
		if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    _operand->SetBool(op1->GetFloat() > op2->GetFloat());
		}
		else _operand->SetBool(op1->GetInt() > op2->GetInt());
		*/

		_operand->SetBool(CScValue::Compare(op1, op2) > 0);
		_stack->Push(_operand);
		break;

	case II_CMP_LE:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		/*
		if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    _operand->SetBool(op1->GetFloat() <= op2->GetFloat());
		}
		else _operand->SetBool(op1->GetInt() <= op2->GetInt());
		*/

		_operand->SetBool(CScValue::Compare(op1, op2) <= 0);
		_stack->Push(_operand);
		break;

	case II_CMP_GE:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		/*
		if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    _operand->SetBool(op1->GetFloat() >= op2->GetFloat());
		}
		else _operand->SetBool(op1->GetInt() >= op2->GetInt());
		*/

		_operand->SetBool(CScValue::Compare(op1, op2) >= 0);
		_stack->Push(_operand);
		break;

	case II_CMP_STRICT_EQ:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		//_operand->SetBool(op1->GetType()==op2->GetType() && op1->GetFloat()==op2->GetFloat());
		_operand->SetBool(CScValue::CompareStrict(op1, op2) == 0);
		_stack->Push(_operand);

		break;

	case II_CMP_STRICT_NE:
		op2 = _stack->Pop();
		op1 = _stack->Pop();

		//_operand->SetBool(op1->GetType()!=op2->GetType() || op1->GetFloat()!=op2->GetFloat());
		_operand->SetBool(CScValue::CompareStrict(op1, op2) != 0);
		_stack->Push(_operand);
		break;

	case II_DBG_LINE: {
		int NewLine = GetDWORD();
		if (NewLine != _currentLine) {
			_currentLine = NewLine;
			if (Game->GetDebugMgr()->_enabled) {
				Game->GetDebugMgr()->OnScriptChangeLine(this, _currentLine);
				for (int i = 0; i < _breakpoints.GetSize(); i++) {
					if (_breakpoints[i] == _currentLine) {
						Game->GetDebugMgr()->OnScriptHitBreakpoint(this);
						Sleep(0);
						break;
					}
				}
				if (_tracingMode) {
					Game->GetDebugMgr()->OnScriptHitBreakpoint(this);
					Sleep(0);
					break;
				}
			}
		}
		break;

	}
	default:
		Game->LOG(0, "Fatal: Invalid instruction %d ('%s', line %d, IP:0x%x)\n", inst, _filename, _currentLine, _iP - sizeof(uint32));
		_state = SCRIPT_FINISHED;
		ret = E_FAIL;
	} // switch(instruction)

	//delete op;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
uint32 CScScript::GetFuncPos(const char *Name) {
	for (int i = 0; i < _numFunctions; i++) {
		if (strcmp(Name, _functions[i].name) == 0) return _functions[i].pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
uint32 CScScript::GetMethodPos(const char *Name) {
	for (int i = 0; i < _numMethods; i++) {
		if (strcmp(Name, _methods[i].name) == 0) return _methods[i].pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScScript::GetVar(char *Name) {
	CScValue *ret = NULL;

	// scope locals
	if (_scopeStack->_sP >= 0) {
		if (_scopeStack->GetTop()->PropExists(Name)) ret = _scopeStack->GetTop()->GetProp(Name);
	}

	// script globals
	if (ret == NULL) {
		if (_globals->PropExists(Name)) ret = _globals->GetProp(Name);
	}

	// engine globals
	if (ret == NULL) {
		if (_engine->_globals->PropExists(Name)) ret = _engine->_globals->GetProp(Name);
	}

	if (ret == NULL) {
		//RuntimeError("Variable '%s' is inaccessible in the current block. Consider changing the script.", Name);
		Game->LOG(0, "Warning: variable '%s' is inaccessible in the current block. Consider changing the script (script:%s, line:%d)", Name, _filename, _currentLine);
		CScValue *Val = new CScValue(Game);
		CScValue *Scope = _scopeStack->GetTop();
		if (Scope) {
			Scope->SetProp(Name, Val);
			ret = _scopeStack->GetTop()->GetProp(Name);
		} else {
			_globals->SetProp(Name, Val);
			ret = _globals->GetProp(Name);
		}
		delete Val;
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::WaitFor(CBObject *Object) {
	if (_unbreakable) {
		RuntimeError("Script cannot be interrupted.");
		return S_OK;
	}

	_state = SCRIPT_WAITING;
	_waitObject = Object;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::WaitForExclusive(CBObject *Object) {
	_engine->ResetObject(Object);
	return WaitFor(Object);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Sleep(uint32 Duration) {
	if (_unbreakable) {
		RuntimeError("Script cannot be interrupted.");
		return S_OK;
	}

	_state = SCRIPT_SLEEPING;
	if (Game->_state == GAME_FROZEN) {
		_waitTime = CBPlatform::GetTime() + Duration;
		_waitFrozen = true;
	} else {
		_waitTime = Game->_timer + Duration;
		_waitFrozen = false;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::finish(bool IncludingThreads) {
	if (_state != SCRIPT_FINISHED && IncludingThreads) {
		_state = SCRIPT_FINISHED;
		FinishThreads();
	} else _state = SCRIPT_FINISHED;


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Run() {
	_state = SCRIPT_RUNNING;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////
void CScScript::RuntimeError(LPCSTR fmt, ...) {
	char buff[256];
	va_list va;

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	Game->LOG(0, "Runtime error. Script '%s', line %d", _filename, _currentLine);
	Game->LOG(0, "  %s", buff);

	if (!Game->_suppressScriptErrors)
		Game->QuickMessage("Script runtime error. View log for details.");
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::persist(CBPersistMgr *persistMgr) {

	persistMgr->transfer(TMEMBER(Game));

	// buffer
	if (persistMgr->_saving) {
		if (_state != SCRIPT_PERSISTENT && _state != SCRIPT_FINISHED && _state != SCRIPT_THREAD_FINISHED) {
			persistMgr->transfer(TMEMBER(_bufferSize));
			persistMgr->putBytes(_buffer, _bufferSize);
		} else {
			// don't save idle/finished scripts
			int bufferSize = 0;
			persistMgr->transfer(TMEMBER(bufferSize));
		}
	} else {
		persistMgr->transfer(TMEMBER(_bufferSize));
		if (_bufferSize > 0) {
			_buffer = new byte[_bufferSize];
			persistMgr->getBytes(_buffer, _bufferSize);
			InitTables();
		} else _buffer = NULL;
	}

	persistMgr->transfer(TMEMBER(_callStack));
	persistMgr->transfer(TMEMBER(_currentLine));
	persistMgr->transfer(TMEMBER(_engine));
	persistMgr->transfer(TMEMBER(_filename));
	persistMgr->transfer(TMEMBER(_freezable));
	persistMgr->transfer(TMEMBER(_globals));
	persistMgr->transfer(TMEMBER(_iP));
	persistMgr->transfer(TMEMBER(_scopeStack));
	persistMgr->transfer(TMEMBER(_stack));
	persistMgr->transfer(TMEMBER_INT(_state));
	persistMgr->transfer(TMEMBER(_operand));
	persistMgr->transfer(TMEMBER_INT(_origState));
	persistMgr->transfer(TMEMBER(_owner));
	persistMgr->transfer(TMEMBER(_reg1));
	persistMgr->transfer(TMEMBER(_thread));
	persistMgr->transfer(TMEMBER(_threadEvent));
	persistMgr->transfer(TMEMBER(_thisStack));
	persistMgr->transfer(TMEMBER(_timeSlice));
	persistMgr->transfer(TMEMBER(_waitObject));
	persistMgr->transfer(TMEMBER(_waitScript));
	persistMgr->transfer(TMEMBER(_waitTime));
	persistMgr->transfer(TMEMBER(_waitFrozen));

	persistMgr->transfer(TMEMBER(_methodThread));
	persistMgr->transfer(TMEMBER(_methodThread));
	persistMgr->transfer(TMEMBER(_unbreakable));
	persistMgr->transfer(TMEMBER(_parentScript));

	if (!persistMgr->_saving) _tracingMode = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CScScript *CScScript::InvokeEventHandler(const char *EventName, bool Unbreakable) {
	//if(_state!=SCRIPT_PERSISTENT) return NULL;

	uint32 pos = GetEventPos(EventName);
	if (!pos) return NULL;

	CScScript *thread = new CScScript(Game, _engine);
	if (thread) {
		HRESULT ret = thread->CreateThread(this, pos, EventName);
		if (SUCCEEDED(ret)) {
			thread->_unbreakable = Unbreakable;
			_engine->_scripts.Add(thread);
			Game->GetDebugMgr()->OnScriptEventThreadInit(thread, this, EventName);
			return thread;
		} else {
			delete thread;
			return NULL;
		}
	} else return NULL;

}


//////////////////////////////////////////////////////////////////////////
uint32 CScScript::GetEventPos(const char *Name) {
	for (int i = _numEvents - 1; i >= 0; i--) {
		if (scumm_stricmp(Name, _events[i].name) == 0) return _events[i].pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
bool CScScript::canHandleEvent(const char *EventName) {
	return GetEventPos(EventName) != 0;
}


//////////////////////////////////////////////////////////////////////////
bool CScScript::canHandleMethod(const char *MethodName) {
	return GetMethodPos(MethodName) != 0;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Pause() {
	if (_state == SCRIPT_PAUSED) {
		Game->LOG(0, "Attempting to pause a paused script ('%s', line %d)", _filename, _currentLine);
		return E_FAIL;
	}

	if (!_freezable || _state == SCRIPT_PERSISTENT) return S_OK;

	_origState = _state;
	_state = SCRIPT_PAUSED;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Resume() {
	if (_state != SCRIPT_PAUSED) return S_OK;

	_state = _origState;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CScScript::TExternalFunction *CScScript::GetExternal(char *Name) {
	for (int i = 0; i < _numExternals; i++) {
		if (strcmp(Name, _externals[i].name) == 0) return &_externals[i];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::ExternalCall(CScStack *Stack, CScStack *ThisStack, CScScript::TExternalFunction *Function) {

#ifndef __WIN32__

	Game->LOG(0, "External functions are not supported on this platform.");
	Stack->CorrectParams(0);
	Stack->PushNULL();
	return E_FAIL;

#else

	bool Success = false;
	HMODULE hDll = LoadLibrary(Function->dll_name);

	if (hDll) {
		FARPROC pFunc = GetProcAddress(hDll, Function->name);
		if (pFunc) {
			int i;
			Success = true;
			Stack->CorrectParams(Function->nu_params);
			CBDynBuffer *Buffer = new CBDynBuffer(Game, 20 * sizeof(uint32));

			for (i = 0; i < Function->nu_params; i++) {
				CScValue *Val = Stack->Pop();
				switch (Function->params[i]) {
				case TYPE_BOOL:
					Buffer->PutDWORD((uint32)Val->GetBool());
					break;
				case TYPE_LONG:
					Buffer->PutDWORD(Val->GetInt());
					break;
				case TYPE_BYTE:
					Buffer->PutDWORD((byte)Val->GetInt());
					break;
				case TYPE_STRING:
					if (Val->IsNULL()) Buffer->PutDWORD(0);
					else Buffer->PutDWORD((uint32)Val->GetString());
					break;
				case TYPE_MEMBUFFER:
					if (Val->IsNULL()) Buffer->PutDWORD(0);
					else Buffer->PutDWORD((uint32)Val->GetMemBuffer());
					break;
				case TYPE_FLOAT: {
					float f = Val->GetFloat();
					Buffer->PutDWORD(*((uint32 *)&f));
					break;
				}
				case TYPE_DOUBLE: {
					double d = Val->GetFloat();
					uint32 *pd = (uint32 *)&d;

					Buffer->PutDWORD(pd[0]);
					Buffer->PutDWORD(pd[1]);
					break;
				}
				}
			}

			// call
			uint32 ret;
			bool StackCorrupted = false;
			switch (Function->call_type) {
			case CALL_CDECL:
				ret = Call_cdecl(Buffer->_buffer, Buffer->GetSize(), (uint32)pFunc, &StackCorrupted);
				break;
			default:
				ret = Call_stdcall(Buffer->_buffer, Buffer->GetSize(), (uint32)pFunc, &StackCorrupted);
			}
			delete Buffer;

			// return
			switch (Function->returns) {
			case TYPE_BOOL:
				Stack->PushBool((byte)ret != 0);
				break;
			case TYPE_LONG:
				Stack->PushInt(ret);
				break;
			case TYPE_BYTE:
				Stack->PushInt((byte)ret);
				break;
				break;
			case TYPE_STRING:
				Stack->PushString((char *)ret);
				break;
			case TYPE_MEMBUFFER: {
				CSXMemBuffer *Buf = new CSXMemBuffer(Game, (void *)ret);
				Stack->PushNative(Buf, false);
			}
			break;
			case TYPE_FLOAT: {
				uint32 dw = GetST0();
				Stack->PushFloat(*((float *)&dw));
				break;
			}
			case TYPE_DOUBLE:
				Stack->PushFloat(GetST0Double());
				break;

			default:
				Stack->PushNULL();
			}

			if (StackCorrupted) RuntimeError("Warning: Stack corrupted after calling '%s' in '%s'\n         Check parameters and/or calling convention.", Function->name, Function->dll_name);
		} else RuntimeError("Exported function '%s' not found in '%s'", Function->name, Function->dll_name);
	} else RuntimeError("Error loading DLL '%s'", Function->dll_name);

	if (!Success) {
		Stack->CorrectParams(0);
		Stack->PushNULL();
	}

	if (hDll) FreeLibrary(hDll);

	return Success ? S_OK : E_FAIL;
#endif
}

#ifdef __WIN32__
//////////////////////////////////////////////////////////////////////////
uint32 CScScript::Call_cdecl(const void *args, size_t sz, uint32 func, bool *StackCorrupt) {
	uint32 rc;               // here's our return value...
	uint32 OrigESP;
	bool StkCorrupt = false;
	__asm {
		mov   OrigESP, esp
		mov   ecx, sz       // get size of buffer
		mov   esi, args     // get buffer
		sub   esp, ecx      // allocate stack space
		mov   edi, esp      // start of destination stack frame
		shr   ecx, 2        // make it dwords
		rep   movsd         // copy params to real stack
		call  [func]        // call the function
		mov   rc,  eax      // save the return value
		add   esp, sz       // restore the stack pointer
		cmp   esp, OrigESP
		jz finish
		mov   esp, OrigESP
		mov   StkCorrupt, 1
		finish:
	}

	if (StackCorrupt) *StackCorrupt = StkCorrupt;
	return rc;
}



//////////////////////////////////////////////////////////////////////////
uint32 CScScript::Call_stdcall(const void *args, size_t sz, uint32 func, bool *StackCorrupt) {
	uint32 rc;               // here's our return value...
	uint32 OrigESP;
	bool StkCorrupt = false;

	__asm {
		mov   OrigESP, esp
		mov   ecx, sz       // get size of buffer
		mov   esi, args     // get buffer
		sub   esp, ecx      // allocate stack space
		mov   edi, esp      // start of destination stack frame
		shr   ecx, 2        // make it dwords
		rep   movsd         // copy it
		call  [func]        // call the function
		mov   rc,  eax      // save the return value
		cmp   esp, OrigESP
		jz finish
		mov   esp, OrigESP
		mov   StkCorrupt, 1
		finish:
	}

	if (StackCorrupt) *StackCorrupt = StkCorrupt;
	return rc;
}


//////////////////////////////////////////////////////////////////////////
__declspec(naked) uint32 CScScript::GetST0(void) {
	uint32 f;                // temp var
	__asm {
		fstp uint32 ptr [f]      // pop ST0 into f
		mov eax, uint32 ptr [f]  // copy into eax
		ret                     // done
	}
}


//////////////////////////////////////////////////////////////////////////
double CScScript::GetST0Double(void) {
	double d;               // temp var
	__asm {
		fstp qword ptr [d]  // get ST0 into d
	}
	return d;
}
#endif


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::CopyParameters(CScStack *Stack) {
	int i;
	int NumParams = Stack->Pop()->GetInt();
	for (i = NumParams - 1; i >= 0; i--) {
		_stack->Push(Stack->GetAt(i));
	}
	_stack->PushInt(NumParams);

	for (i = 0; i < NumParams; i++) Stack->Pop();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::FinishThreads() {
	for (int i = 0; i < _engine->_scripts.GetSize(); i++) {
		CScScript *Scr = _engine->_scripts[i];
		if (Scr->_thread && Scr->_state != SCRIPT_FINISHED && Scr->_owner == _owner && scumm_stricmp(Scr->_filename, _filename) == 0)
			Scr->finish(true);
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// IWmeDebugScript interface implementation
int CScScript::DbgGetLine() {
	return _currentLine;
}

//////////////////////////////////////////////////////////////////////////
const char *CScScript::DbgGetFilename() {
	return _filename;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::DbgSendScript(IWmeDebugClient *Client) {
	if (_methodThread) Client->OnScriptMethodThreadInit(this, _parentScript, _threadEvent);
	else if (_thread) Client->OnScriptEventThreadInit(this, _parentScript, _threadEvent);
	else Client->OnScriptInit(this);

	return DbgSendVariables(Client);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::DbgSendVariables(IWmeDebugClient *Client) {
	// send script globals
	_globals->DbgSendVariables(Client, WME_DBGVAR_SCRIPT, this, 0);

	// send scope variables
	if (_scopeStack->_sP >= 0) {
		for (int i = 0; i <= _scopeStack->_sP; i++) {
			//  CScValue *Scope = _scopeStack->GetAt(i);
			//Scope->DbgSendVariables(Client, WME_DBGVAR_SCOPE, this, (unsigned int)Scope);
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
TScriptState CScScript::DbgGetState() {
	return _state;
}

//////////////////////////////////////////////////////////////////////////
int CScScript::DbgGetNumBreakpoints() {
	return _breakpoints.GetSize();
}

//////////////////////////////////////////////////////////////////////////
int CScScript::DbgGetBreakpoint(int Index) {
	if (Index >= 0 && Index < _breakpoints.GetSize()) return _breakpoints[Index];
	else return -1;
}

//////////////////////////////////////////////////////////////////////////
bool CScScript::DbgSetTracingMode(bool IsTracing) {
	_tracingMode = IsTracing;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScScript::DbgGetTracingMode() {
	return _tracingMode;
}


//////////////////////////////////////////////////////////////////////////
void CScScript::AfterLoad() {
	if (_buffer == NULL) {
		byte *buffer = _engine->GetCompiledScript(_filename, &_bufferSize);
		if (!buffer) {
			Game->LOG(0, "Error reinitializing script '%s' after load. Script will be terminated.", _filename);
			_state = SCRIPT_ERROR;
			return;
		}

		_buffer = new byte [_bufferSize];
		memcpy(_buffer, buffer, _bufferSize);

		InitTables();
	}
}

} // end of namespace WinterMute
