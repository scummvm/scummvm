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
#include "common/memstream.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CScScript, false)

//////////////////////////////////////////////////////////////////////////
CScScript::CScScript(CBGame *inGame, CScEngine *Engine): CBBase(inGame) {
	_buffer = NULL;
	_bufferSize = _iP = 0;
	_scriptStream = NULL;
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

void CScScript::readHeader() {
	uint32 oldPos = _scriptStream->pos();
	_scriptStream->seek(0);
	_header.magic = _scriptStream->readUint32LE();
	_header.version = _scriptStream->readUint32LE();
	_header.code_start = _scriptStream->readUint32LE();
	_header.func_table = _scriptStream->readUint32LE();
	_header.symbol_table = _scriptStream->readUint32LE();
	_header.event_table = _scriptStream->readUint32LE();
	_header.externals_table = _scriptStream->readUint32LE();
	_header.method_table = _scriptStream->readUint32LE();
	_scriptStream->seek(oldPos);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::initScript() {
	if (!_scriptStream) {
		_scriptStream = new Common::MemoryReadStream(_buffer, _bufferSize);
	}
	readHeader();

	if (_header.magic != SCRIPT_MAGIC) {
		Game->LOG(0, "File '%s' is not a valid compiled script", _filename);
		cleanup();
		return E_FAIL;
	}

	if (_header.version > SCRIPT_VERSION) {
		Game->LOG(0, "Script '%s' has a wrong version %d.%d (expected %d.%d)", _filename, _header.version / 256, _header.version % 256, SCRIPT_VERSION / 256, SCRIPT_VERSION % 256);
		cleanup();
		return E_FAIL;
	}

	initTables();

	// init stacks
	_scopeStack = new CScStack(Game);
	_callStack  = new CScStack(Game);
	_thisStack  = new CScStack(Game);
	_stack      = new CScStack(Game);

	_operand    = new CScValue(Game);
	_reg1       = new CScValue(Game);


	// skip to the beginning
	_iP = _header.code_start;
	_scriptStream->seek(_iP);
	_currentLine = 0;

	// init breakpoints
	_engine->refreshScriptBreakpoints(this);


	// ready to rumble...
	_state = SCRIPT_RUNNING;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::initTables() {
	uint32 OrigIP = _iP;

	readHeader();
	// load symbol table
	_iP = _header.symbol_table;

	_numSymbols = getDWORD();
	_symbols = new char*[_numSymbols];
	for (uint32 i = 0; i < _numSymbols; i++) {
		uint32 index = getDWORD();
		_symbols[index] = getString();
	}

	// load functions table
	_iP = _header.func_table;

	_numFunctions = getDWORD();
	_functions = new TFunctionPos[_numFunctions];
	for (uint32 i = 0; i < _numFunctions; i++) {
		_functions[i].pos = getDWORD();
		_functions[i].name = getString();
	}


	// load events table
	_iP = _header.event_table;

	_numEvents = getDWORD();
	_events = new TEventPos[_numEvents];
	for (uint32 i = 0; i < _numEvents; i++) {
		_events[i].pos = getDWORD();
		_events[i].name = getString();
	}


	// load externals
	if (_header.version >= 0x0101) {
		_iP = _header.externals_table;

		_numExternals = getDWORD();
		_externals = new TExternalFunction[_numExternals];
		for (uint32 i = 0; i < _numExternals; i++) {
			_externals[i].dll_name = getString();
			_externals[i].name = getString();
			_externals[i].call_type = (TCallType)getDWORD();
			_externals[i].returns = (TExternalType)getDWORD();
			_externals[i].nu_params = getDWORD();
			if (_externals[i].nu_params > 0) {
				_externals[i].params = new TExternalType[_externals[i].nu_params];
				for (int j = 0; j < _externals[i].nu_params; j++) {
					_externals[i].params[j] = (TExternalType)getDWORD();
				}
			}
		}
	}

	// load method table
	_iP = _header.method_table;

	_numMethods = getDWORD();
	_methods = new TMethodPos[_numMethods];
	for (uint32 i = 0; i < _numMethods; i++) {
		_methods[i].pos = getDWORD();
		_methods[i].name = getString();
	}


	_iP = OrigIP;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::create(const char *filename, byte *buffer, uint32 size, CBScriptHolder *owner) {
	cleanup();

	_thread = false;
	_methodThread = false;

	delete[] _threadEvent;
	_threadEvent = NULL;

	_filename = new char[strlen(filename) + 1];
	if (_filename) strcpy(_filename, filename);

	_buffer = new byte [size];
	if (!_buffer) return E_FAIL;

	memcpy(_buffer, buffer, size);

	_bufferSize = size;

	HRESULT res = initScript();
	if (FAILED(res)) return res;

	// establish global variables table
	_globals = new CScValue(Game);

	_owner = owner;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::createThread(CScScript *original, uint32 initIP, const char *eventName) {
	cleanup();

	_thread = true;
	_methodThread = false;
	_threadEvent = new char[strlen(eventName) + 1];
	if (_threadEvent) strcpy(_threadEvent, eventName);

	// copy filename
	_filename = new char[strlen(original->_filename) + 1];
	if (_filename) strcpy(_filename, original->_filename);

	// copy buffer
	_buffer = new byte [original->_bufferSize];
	if (!_buffer) return E_FAIL;

	memcpy(_buffer, original->_buffer, original->_bufferSize);
	_bufferSize = original->_bufferSize;

	// initialize
	HRESULT res = initScript();
	if (FAILED(res)) return res;

	// copy globals
	_globals = original->_globals;

	// skip to the beginning of the event
	_iP = initIP;
	_scriptStream->seek(_iP);

	_timeSlice = original->_timeSlice;
	_freezable = original->_freezable;
	_owner = original->_owner;

	_engine = original->_engine;
	_parentScript = original;

	return S_OK;
}




//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::createMethodThread(CScScript *original, const char *methodName) {
	uint32 ip = original->getMethodPos(methodName);
	if (ip == 0) return E_FAIL;

	cleanup();

	_thread = true;
	_methodThread = true;
	_threadEvent = new char[strlen(methodName) + 1];
	if (_threadEvent) strcpy(_threadEvent, methodName);

	// copy filename
	_filename = new char[strlen(original->_filename) + 1];
	if (_filename) strcpy(_filename, original->_filename);

	// copy buffer
	_buffer = new byte [original->_bufferSize];
	if (!_buffer) return E_FAIL;

	memcpy(_buffer, original->_buffer, original->_bufferSize);
	_bufferSize = original->_bufferSize;

	// initialize
	HRESULT res = initScript();
	if (FAILED(res)) return res;

	// copy globals
	_globals = original->_globals;

	// skip to the beginning of the event
	_iP = ip;

	_timeSlice = original->_timeSlice;
	_freezable = original->_freezable;
	_owner = original->_owner;

	_engine = original->_engine;
	_parentScript = original;

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

	delete _scopeStack;
	_scopeStack = NULL;

	delete _callStack;
	_callStack = NULL;

	delete _thisStack;
	_thisStack = NULL;

	delete _stack;
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
	
	delete _scriptStream;
}


//////////////////////////////////////////////////////////////////////////
uint32 CScScript::getDWORD() {
	_scriptStream->seek(_iP);
	uint32 ret = _scriptStream->readUint32LE();
	_iP += sizeof(uint32);
//	assert(oldRet == ret);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
double CScScript::getFloat() {
	_scriptStream->seek((int32)_iP);
	byte buffer[8];
	_scriptStream->read(buffer, 8);

#ifdef SCUMM_BIG_ENDIAN
	// TODO: For lack of a READ_LE_UINT64
	SWAP(buffer[0], buffer[7]);
	SWAP(buffer[1], buffer[6]);
	SWAP(buffer[2], buffer[5]);
	SWAP(buffer[3], buffer[4]);
#endif

	double ret = *(double *)(buffer);
	_iP += 8; // Hardcode the double-size used originally.
	return ret;
}


//////////////////////////////////////////////////////////////////////////
char *CScScript::getString() {
	char *ret = (char *)(_buffer + _iP);
	while (*(char *)(_buffer + _iP) != '\0') _iP++;
	_iP++; // string terminator
	_scriptStream->seek(_iP);

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::executeInstruction() {
	HRESULT ret = S_OK;

	uint32 dw;
	const char *str = NULL;

	//CScValue* op = new CScValue(Game);
	_operand->cleanup();

	CScValue *op1;
	CScValue *op2;

	uint32 inst = getDWORD();
	switch (inst) {

	case II_DEF_VAR:
		_operand->setNULL();
		dw = getDWORD();
		if (_scopeStack->_sP < 0) {
			_globals->setProp(_symbols[dw], _operand);
			if (Game->getDebugMgr()->_enabled)
				Game->getDebugMgr()->onVariableInit(WME_DBGVAR_SCRIPT, this, NULL, _globals->getProp(_symbols[dw]), _symbols[dw]);
		} else {
			_scopeStack->getTop()->setProp(_symbols[dw], _operand);
			if (Game->getDebugMgr()->_enabled)
				Game->getDebugMgr()->onVariableInit(WME_DBGVAR_SCOPE, this, _scopeStack->getTop(), _scopeStack->getTop()->getProp(_symbols[dw]), _symbols[dw]);
		}

		break;

	case II_DEF_GLOB_VAR:
	case II_DEF_CONST_VAR: {
		dw = getDWORD();
		/*      char *Temp = _symbols[dw]; // TODO delete */
		// only create global var if it doesn't exist
		if (!_engine->_globals->propExists(_symbols[dw])) {
			_operand->setNULL();
			_engine->_globals->setProp(_symbols[dw], _operand, false, inst == II_DEF_CONST_VAR);

			if (Game->getDebugMgr()->_enabled)
				Game->getDebugMgr()->onVariableInit(WME_DBGVAR_GLOBAL, this, NULL, _engine->_globals->getProp(_symbols[dw]), _symbols[dw]);
		}
		break;
	}

	case II_RET:
		if (_scopeStack->_sP >= 0 && _callStack->_sP >= 0) {
			Game->getDebugMgr()->onScriptShutdownScope(this, _scopeStack->getTop());

			_scopeStack->pop();
			_iP = (uint32)_callStack->pop()->getInt();

			if (_scopeStack->_sP < 0) Game->getDebugMgr()->onScriptChangeScope(this, NULL);
			else Game->getDebugMgr()->onScriptChangeScope(this, _scopeStack->getTop());
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
		dw = getDWORD();

		_operand->setInt(_iP);
		_callStack->push(_operand);

		_iP = dw;

		break;

	case II_CALL_BY_EXP: {
		// push var
		// push string
		str = _stack->pop()->getString();
		char *MethodName = new char[strlen(str) + 1];
		strcpy(MethodName, str);

		CScValue *var = _stack->pop();
		if (var->_type == VAL_VARIABLE_REF) var = var->_valRef;

		HRESULT res = E_FAIL;
		bool TriedNative = false;

		// we are already calling this method, try native
		if (_thread && _methodThread && strcmp(MethodName, _threadEvent) == 0 && var->_type == VAL_NATIVE && _owner == var->getNative()) {
			TriedNative = true;
			res = var->_valNative->scCallMethod(this, _stack, _thisStack, MethodName);
		}

		if (FAILED(res)) {
			if (var->isNative() && var->getNative()->canHandleMethod(MethodName)) {
				if (!_unbreakable) {
					_waitScript = var->getNative()->invokeMethodThread(MethodName);
					if (!_waitScript) {
						_stack->correctParams(0);
						runtimeError("Error invoking method '%s'.", MethodName);
						_stack->pushNULL();
					} else {
						_state = SCRIPT_WAITING_SCRIPT;
						_waitScript->copyParameters(_stack);
					}
				} else {
					// can call methods in unbreakable mode
					_stack->correctParams(0);
					runtimeError("Cannot call method '%s'. Ignored.", MethodName);
					_stack->pushNULL();
				}
				delete [] MethodName;
				break;
			}
			/*
			CScValue* val = var->getProp(MethodName);
			if(val){
			    dw = GetFuncPos(val->getString());
			    if(dw==0){
			        TExternalFunction* f = GetExternal(val->getString());
			        if(f){
			            ExternalCall(_stack, _thisStack, f);
			        }
			        else{
			            // not an internal nor external, try for native function
			            Game->ExternalCall(this, _stack, _thisStack, val->getString());
			        }
			    }
			    else{
			        _operand->setInt(_iP);
			        _callStack->Push(_operand);
			        _iP = dw;
			    }
			}
			*/
			else {
				res = E_FAIL;
				if (var->_type == VAL_NATIVE && !TriedNative) res = var->_valNative->scCallMethod(this, _stack, _thisStack, MethodName);

				if (FAILED(res)) {
					_stack->correctParams(0);
					runtimeError("Call to undefined method '%s'. Ignored.", MethodName);
					_stack->pushNULL();
				}
			}
		}
		delete [] MethodName;
	}
	break;

	case II_EXTERNAL_CALL: {
		uint32 SymbolIndex = getDWORD();

		TExternalFunction *f = getExternal(_symbols[SymbolIndex]);
		if (f) {
			externalCall(_stack, _thisStack, f);
		} else Game->ExternalCall(this, _stack, _thisStack, _symbols[SymbolIndex]);

		break;
	}
	case II_SCOPE:
		_operand->setNULL();
		_scopeStack->push(_operand);

		if (_scopeStack->_sP < 0) Game->getDebugMgr()->onScriptChangeScope(this, NULL);
		else Game->getDebugMgr()->onScriptChangeScope(this, _scopeStack->getTop());

		break;

	case II_CORRECT_STACK:
		dw = getDWORD(); // params expected
		_stack->correctParams(dw);
		break;

	case II_CREATE_OBJECT:
		_operand->setObject();
		_stack->push(_operand);
		break;

	case II_POP_EMPTY:
		_stack->pop();
		break;

	case II_PUSH_VAR: {
		CScValue *var = getVar(_symbols[getDWORD()]);
		if (false && /*var->_type==VAL_OBJECT ||*/ var->_type == VAL_NATIVE) {
			_operand->setReference(var);
			_stack->push(_operand);
		} else _stack->push(var);
		break;
	}

	case II_PUSH_VAR_REF: {
		CScValue *var = getVar(_symbols[getDWORD()]);
		_operand->setReference(var);
		_stack->push(_operand);
		break;
	}

	case II_POP_VAR: {
		char *VarName = _symbols[getDWORD()];
		CScValue *var = getVar(VarName);
		if (var) {
			CScValue *val = _stack->pop();
			if (!val) {
				runtimeError("Script stack corruption detected. Please report this script at WME bug reports forum.");
				var->setNULL();
			} else {
				if (val->getType() == VAL_VARIABLE_REF) val = val->_valRef;
				if (val->_type == VAL_NATIVE) var->setValue(val);
				else {
					var->copy(val);
				}
			}

			if (Game->getDebugMgr()->_enabled)
				Game->getDebugMgr()->onVariableChangeValue(var, val);
		}

		break;
	}

	case II_PUSH_VAR_THIS:
		_stack->push(_thisStack->getTop());
		break;

	case II_PUSH_INT:
		_stack->pushInt((int)getDWORD());
		break;

	case II_PUSH_FLOAT:
		_stack->pushFloat(getFloat());
		break;


	case II_PUSH_BOOL:
		_stack->pushBool(getDWORD() != 0);

		break;

	case II_PUSH_STRING:
		_stack->pushString(getString());
		break;

	case II_PUSH_NULL:
		_stack->pushNULL();
		break;

	case II_PUSH_THIS_FROM_STACK:
		_operand->setReference(_stack->getTop());
		_thisStack->push(_operand);
		break;

	case II_PUSH_THIS:
		_operand->setReference(getVar(_symbols[getDWORD()]));
		_thisStack->push(_operand);
		break;

	case II_POP_THIS:
		_thisStack->pop();
		break;

	case II_PUSH_BY_EXP: {
		str = _stack->pop()->getString();
		CScValue *val = _stack->pop()->getProp(str);
		if (val) _stack->push(val);
		else _stack->pushNULL();

		break;
	}

	case II_POP_BY_EXP: {
		str = _stack->pop()->getString();
		CScValue *var = _stack->pop();
		CScValue *val = _stack->pop();

		if (val == NULL) {
			runtimeError("Script stack corruption detected. Please report this script at WME bug reports forum.");
			var->setNULL();
		} else var->setProp(str, val);

		if (Game->getDebugMgr()->_enabled)
			Game->getDebugMgr()->onVariableChangeValue(var, NULL);

		break;
	}

	case II_PUSH_REG1:
		_stack->push(_reg1);
		break;

	case II_POP_REG1:
		_reg1->copy(_stack->pop());
		break;

	case II_JMP:
		_iP = getDWORD();
		break;

	case II_JMP_FALSE: {
		dw = getDWORD();
		//if(!_stack->pop()->getBool()) _iP = dw;
		CScValue *val = _stack->pop();
		if (!val) {
			runtimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
		} else {
			if (!val->getBool()) _iP = dw;
		}
		break;
	}

	case II_ADD:
		op2 = _stack->pop();
		op1 = _stack->pop();

		if (op1->isNULL() || op2->isNULL())
			_operand->setNULL();
		else if (op1->getType() == VAL_STRING || op2->getType() == VAL_STRING) {
			char *tempStr = new char [strlen(op1->getString()) + strlen(op2->getString()) + 1];
			strcpy(tempStr, op1->getString());
			strcat(tempStr, op2->getString());
			_operand->setString(tempStr);
			delete [] tempStr;
		} else if (op1->getType() == VAL_INT && op2->getType() == VAL_INT)
			_operand->setInt(op1->getInt() + op2->getInt());
		else _operand->setFloat(op1->getFloat() + op2->getFloat());

		_stack->push(_operand);

		break;

	case II_SUB:
		op2 = _stack->pop();
		op1 = _stack->pop();

		if (op1->isNULL() || op2->isNULL())
			_operand->setNULL();
		else if (op1->getType() == VAL_INT && op2->getType() == VAL_INT)
			_operand->setInt(op1->getInt() - op2->getInt());
		else _operand->setFloat(op1->getFloat() - op2->getFloat());

		_stack->push(_operand);

		break;

	case II_MUL:
		op2 = _stack->pop();
		op1 = _stack->pop();

		if (op1->isNULL() || op2->isNULL()) _operand->setNULL();
		else if (op1->getType() == VAL_INT && op2->getType() == VAL_INT)
			_operand->setInt(op1->getInt() * op2->getInt());
		else _operand->setFloat(op1->getFloat() * op2->getFloat());

		_stack->push(_operand);

		break;

	case II_DIV:
		op2 = _stack->pop();
		op1 = _stack->pop();

		if (op2->getFloat() == 0.0f)
			runtimeError("Division by zero.");

		if (op1->isNULL() || op2->isNULL() || op2->getFloat() == 0.0f) _operand->setNULL();
		else _operand->setFloat(op1->getFloat() / op2->getFloat());

		_stack->push(_operand);

		break;

	case II_MODULO:
		op2 = _stack->pop();
		op1 = _stack->pop();

		if (op2->getInt() == 0)
			runtimeError("Division by zero.");

		if (op1->isNULL() || op2->isNULL() || op2->getInt() == 0)
			_operand->setNULL();
		else _operand->setInt(op1->getInt() % op2->getInt());

		_stack->push(_operand);

		break;

	case II_NOT:
		op1 = _stack->pop();
		//if(op1->isNULL()) _operand->setNULL();
		if (op1->isNULL()) _operand->setBool(true);
		else _operand->setBool(!op1->getBool());
		_stack->push(_operand);

		break;

	case II_AND:
		op2 = _stack->pop();
		op1 = _stack->pop();
		if (op1 == NULL || op2 == NULL) {
			runtimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
			_operand->setBool(false);
		} else {
			_operand->setBool(op1->getBool() && op2->getBool());
		}
		_stack->push(_operand);
		break;

	case II_OR:
		op2 = _stack->pop();
		op1 = _stack->pop();
		if (op1 == NULL || op2 == NULL) {
			runtimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
			_operand->setBool(false);
		} else {
			_operand->setBool(op1->getBool() || op2->getBool());
		}
		_stack->push(_operand);
		break;

	case II_CMP_EQ:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if((op1->isNULL() && !op2->isNULL()) || (!op1->isNULL() && op2->isNULL())) _operand->setBool(false);
		else if(op1->isNative() && op2->isNative()){
		    _operand->setBool(op1->getNative() == op2->getNative());
		}
		else if(op1->getType()==VAL_STRING || op2->getType()==VAL_STRING){
		    _operand->setBool(scumm_stricmp(op1->getString(), op2->getString())==0);
		}
		else if(op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() == op2->getFloat());
		}
		else{
		    _operand->setBool(op1->getInt() == op2->getInt());
		}
		*/

		_operand->setBool(CScValue::compare(op1, op2) == 0);
		_stack->push(_operand);
		break;

	case II_CMP_NE:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if((op1->isNULL() && !op2->isNULL()) || (!op1->isNULL() && op2->isNULL())) _operand->setBool(true);
		else if(op1->isNative() && op2->isNative()){
		    _operand->setBool(op1->getNative() != op2->getNative());
		}
		else if(op1->getType()==VAL_STRING || op2->getType()==VAL_STRING){
		    _operand->setBool(scumm_stricmp(op1->getString(), op2->getString())!=0);
		}
		else if(op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() != op2->getFloat());
		}
		else{
		    _operand->setBool(op1->getInt() != op2->getInt());
		}
		*/

		_operand->setBool(CScValue::compare(op1, op2) != 0);
		_stack->push(_operand);
		break;

	case II_CMP_L:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if(op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() < op2->getFloat());
		}
		else _operand->setBool(op1->getInt() < op2->getInt());
		*/

		_operand->setBool(CScValue::compare(op1, op2) < 0);
		_stack->push(_operand);
		break;

	case II_CMP_G:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if(op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() > op2->getFloat());
		}
		else _operand->setBool(op1->getInt() > op2->getInt());
		*/

		_operand->setBool(CScValue::compare(op1, op2) > 0);
		_stack->push(_operand);
		break;

	case II_CMP_LE:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if(op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() <= op2->getFloat());
		}
		else _operand->setBool(op1->getInt() <= op2->getInt());
		*/

		_operand->setBool(CScValue::compare(op1, op2) <= 0);
		_stack->push(_operand);
		break;

	case II_CMP_GE:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if(op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() >= op2->getFloat());
		}
		else _operand->setBool(op1->getInt() >= op2->getInt());
		*/

		_operand->setBool(CScValue::compare(op1, op2) >= 0);
		_stack->push(_operand);
		break;

	case II_CMP_STRICT_EQ:
		op2 = _stack->pop();
		op1 = _stack->pop();

		//_operand->setBool(op1->getType()==op2->getType() && op1->getFloat()==op2->getFloat());
		_operand->setBool(CScValue::compareStrict(op1, op2) == 0);
		_stack->push(_operand);

		break;

	case II_CMP_STRICT_NE:
		op2 = _stack->pop();
		op1 = _stack->pop();

		//_operand->setBool(op1->getType()!=op2->getType() || op1->getFloat()!=op2->getFloat());
		_operand->setBool(CScValue::compareStrict(op1, op2) != 0);
		_stack->push(_operand);
		break;

	case II_DBG_LINE: {
		int newLine = getDWORD();
		if (newLine != _currentLine) {
			_currentLine = newLine;
			if (Game->getDebugMgr()->_enabled) {
				Game->getDebugMgr()->onScriptChangeLine(this, _currentLine);
				for (int i = 0; i < _breakpoints.GetSize(); i++) {
					if (_breakpoints[i] == _currentLine) {
						Game->getDebugMgr()->onScriptHitBreakpoint(this);
						sleep(0);
						break;
					}
				}
				if (_tracingMode) {
					Game->getDebugMgr()->onScriptHitBreakpoint(this);
					sleep(0);
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
uint32 CScScript::getFuncPos(const char *name) {
	for (int i = 0; i < _numFunctions; i++) {
		if (strcmp(name, _functions[i].name) == 0)
			return _functions[i].pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
uint32 CScScript::getMethodPos(const char *name) {
	for (int i = 0; i < _numMethods; i++) {
		if (strcmp(name, _methods[i].name) == 0)
			return _methods[i].pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScScript::getVar(char *name) {
	CScValue *ret = NULL;

	// scope locals
	if (_scopeStack->_sP >= 0) {
		if (_scopeStack->getTop()->propExists(name))
			ret = _scopeStack->getTop()->getProp(name);
	}

	// script globals
	if (ret == NULL) {
		if (_globals->propExists(name))
			ret = _globals->getProp(name);
	}

	// engine globals
	if (ret == NULL) {
		if (_engine->_globals->propExists(name))
			ret = _engine->_globals->getProp(name);
	}

	if (ret == NULL) {
		//RuntimeError("Variable '%s' is inaccessible in the current block. Consider changing the script.", name);
		Game->LOG(0, "Warning: variable '%s' is inaccessible in the current block. Consider changing the script (script:%s, line:%d)", name, _filename, _currentLine);
		CScValue *val = new CScValue(Game);
		CScValue *scope = _scopeStack->getTop();
		if (scope) {
			scope->setProp(name, val);
			ret = _scopeStack->getTop()->getProp(name);
		} else {
			_globals->setProp(name, val);
			ret = _globals->getProp(name);
		}
		delete val;
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::waitFor(CBObject *object) {
	if (_unbreakable) {
		runtimeError("Script cannot be interrupted.");
		return S_OK;
	}

	_state = SCRIPT_WAITING;
	_waitObject = object;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::waitForExclusive(CBObject *object) {
	_engine->resetObject(object);
	return waitFor(object);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::sleep(uint32 duration) {
	if (_unbreakable) {
		runtimeError("Script cannot be interrupted.");
		return S_OK;
	}

	_state = SCRIPT_SLEEPING;
	if (Game->_state == GAME_FROZEN) {
		_waitTime = CBPlatform::GetTime() + duration;
		_waitFrozen = true;
	} else {
		_waitTime = Game->_timer + duration;
		_waitFrozen = false;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::finish(bool includingThreads) {
	if (_state != SCRIPT_FINISHED && includingThreads) {
		_state = SCRIPT_FINISHED;
		finishThreads();
	} else _state = SCRIPT_FINISHED;


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::run() {
	_state = SCRIPT_RUNNING;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////
void CScScript::runtimeError(LPCSTR fmt, ...) {
	char buff[256];
	va_list va;

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	Game->LOG(0, "Runtime error. Script '%s', line %d", _filename, _currentLine);
	Game->LOG(0, "  %s", buff);

	if (!Game->_suppressScriptErrors)
		Game->quickMessage("Script runtime error. View log for details.");
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
			_scriptStream = new Common::MemoryReadStream(_buffer, _bufferSize);
			initTables();
		} else {
			_buffer = NULL;
			_scriptStream = NULL;	
		}	
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
CScScript *CScScript::invokeEventHandler(const char *eventName, bool unbreakable) {
	//if(_state!=SCRIPT_PERSISTENT) return NULL;

	uint32 pos = getEventPos(eventName);
	if (!pos) return NULL;

	CScScript *thread = new CScScript(Game, _engine);
	if (thread) {
		HRESULT ret = thread->createThread(this, pos, eventName);
		if (SUCCEEDED(ret)) {
			thread->_unbreakable = unbreakable;
			_engine->_scripts.Add(thread);
			Game->getDebugMgr()->onScriptEventThreadInit(thread, this, eventName);
			return thread;
		} else {
			delete thread;
			return NULL;
		}
	} else return NULL;

}


//////////////////////////////////////////////////////////////////////////
uint32 CScScript::getEventPos(const char *name) {
	for (int i = _numEvents - 1; i >= 0; i--) {
		if (scumm_stricmp(name, _events[i].name) == 0) return _events[i].pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
bool CScScript::canHandleEvent(const char *eventName) {
	return getEventPos(eventName) != 0;
}


//////////////////////////////////////////////////////////////////////////
bool CScScript::canHandleMethod(const char *methodName) {
	return getMethodPos(methodName) != 0;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::pause() {
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
HRESULT CScScript::resume() {
	if (_state != SCRIPT_PAUSED) return S_OK;

	_state = _origState;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CScScript::TExternalFunction *CScScript::getExternal(char *name) {
	for (int i = 0; i < _numExternals; i++) {
		if (strcmp(name, _externals[i].name) == 0)
			return &_externals[i];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::externalCall(CScStack *stack, CScStack *thisStack, CScScript::TExternalFunction *function) {

#ifndef __WIN32__

	Game->LOG(0, "External functions are not supported on this platform.");
	stack->correctParams(0);
	stack->pushNULL();
	return E_FAIL;

#else

	bool Success = false;
	HMODULE hDll = LoadLibrary(Function->dll_name);

	if (hDll) {
		FARPROC pFunc = GetProcAddress(hDll, Function->name);
		if (pFunc) {
			int i;
			Success = true;
			stack->correctParams(Function->nu_params);
			CBDynBuffer *Buffer = new CBDynBuffer(Game, 20 * sizeof(uint32));

			for (i = 0; i < Function->nu_params; i++) {
				CScValue *Val = stack->pop();
				switch (Function->params[i]) {
				case TYPE_BOOL:
					buffer->PutDWORD((uint32)Val->getBool());
					break;
				case TYPE_LONG:
					buffer->PutDWORD(Val->getInt());
					break;
				case TYPE_BYTE:
					buffer->PutDWORD((byte)Val->getInt());
					break;
				case TYPE_STRING:
					if (Val->isNULL()) buffer->PutDWORD(0);
					else buffer->PutDWORD((uint32)Val->getString());
					break;
				case TYPE_MEMBUFFER:
					if (Val->isNULL()) buffer->PutDWORD(0);
					else buffer->PutDWORD((uint32)Val->getMemBuffer());
					break;
				case TYPE_FLOAT: {
					float f = Val->getFloat();
					buffer->PutDWORD(*((uint32 *)&f));
					break;
				}
				case TYPE_DOUBLE: {
					double d = Val->getFloat();
					uint32 *pd = (uint32 *)&d;

					buffer->PutDWORD(pd[0]);
					buffer->PutDWORD(pd[1]);
					break;
				}
				}
			}

			// call
			uint32 ret;
			bool StackCorrupted = false;
			switch (Function->call_type) {
			case CALL_CDECL:
				ret = Call_cdecl(buffer->_buffer, buffer->GetSize(), (uint32)pFunc, &StackCorrupted);
				break;
			default:
				ret = Call_stdcall(buffer->_buffer, buffer->GetSize(), (uint32)pFunc, &StackCorrupted);
			}
			delete Buffer;

			// return
			switch (Function->returns) {
			case TYPE_BOOL:
				stack->pushBool((byte)ret != 0);
				break;
			case TYPE_LONG:
				stack->pushInt(ret);
				break;
			case TYPE_BYTE:
				stack->pushInt((byte)ret);
				break;
				break;
			case TYPE_STRING:
				stack->pushString((char *)ret);
				break;
			case TYPE_MEMBUFFER: {
				CSXMemBuffer *Buf = new CSXMemBuffer(Game, (void *)ret);
				stack->pushNative(Buf, false);
			}
			break;
			case TYPE_FLOAT: {
				uint32 dw = GetST0();
				stack->pushFloat(*((float *)&dw));
				break;
			}
			case TYPE_DOUBLE:
				stack->pushFloat(GetST0Double());
				break;

			default:
				stack->pushNULL();
			}

			if (StackCorrupted) RuntimeError("Warning: Stack corrupted after calling '%s' in '%s'\n         Check parameters and/or calling convention.", Function->name, Function->dll_name);
		} else RuntimeError("Exported function '%s' not found in '%s'", Function->name, Function->dll_name);
	} else RuntimeError("Error loading DLL '%s'", Function->dll_name);

	if (!Success) {
		stack->correctParams(0);
		stack->pushNULL();
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
HRESULT CScScript::copyParameters(CScStack *stack) {
	int i;
	int NumParams = stack->pop()->getInt();
	for (i = NumParams - 1; i >= 0; i--) {
		_stack->push(stack->getAt(i));
	}
	_stack->pushInt(NumParams);

	for (i = 0; i < NumParams; i++) stack->pop();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::finishThreads() {
	for (int i = 0; i < _engine->_scripts.GetSize(); i++) {
		CScScript *scr = _engine->_scripts[i];
		if (scr->_thread && scr->_state != SCRIPT_FINISHED && scr->_owner == _owner && scumm_stricmp(scr->_filename, _filename) == 0)
			scr->finish(true);
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// IWmeDebugScript interface implementation
int CScScript::dbgGetLine() {
	return _currentLine;
}

//////////////////////////////////////////////////////////////////////////
const char *CScScript::dbgGetFilename() {
	return _filename;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::dbgSendScript(IWmeDebugClient *client) {
	if (_methodThread) client->onScriptMethodThreadInit(this, _parentScript, _threadEvent);
	else if (_thread) client->onScriptEventThreadInit(this, _parentScript, _threadEvent);
	else client->onScriptInit(this);

	return dbgSendVariables(client);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::dbgSendVariables(IWmeDebugClient *client) {
	// send script globals
	_globals->dbgSendVariables(client, WME_DBGVAR_SCRIPT, this, 0);

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
TScriptState CScScript::dbgGetState() {
	return _state;
}

//////////////////////////////////////////////////////////////////////////
int CScScript::dbgGetNumBreakpoints() {
	return _breakpoints.GetSize();
}

//////////////////////////////////////////////////////////////////////////
int CScScript::dbgGetBreakpoint(int index) {
	if (index >= 0 && index < _breakpoints.GetSize()) return _breakpoints[index];
	else return -1;
}

//////////////////////////////////////////////////////////////////////////
bool CScScript::dbgSetTracingMode(bool isTracing) {
	_tracingMode = isTracing;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScScript::dbgGetTracingMode() {
	return _tracingMode;
}


//////////////////////////////////////////////////////////////////////////
void CScScript::afterLoad() {
	if (_buffer == NULL) {
		byte *buffer = _engine->getCompiledScript(_filename, &_bufferSize);
		if (!buffer) {
			Game->LOG(0, "Error reinitializing script '%s' after load. Script will be terminated.", _filename);
			_state = SCRIPT_ERROR;
			return;
		}

		_buffer = new byte [_bufferSize];
		memcpy(_buffer, buffer, _bufferSize);

		delete _scriptStream;
		_scriptStream = new Common::MemoryReadStream(_buffer, _bufferSize);

		initTables();
	}
}

} // end of namespace WinterMute
