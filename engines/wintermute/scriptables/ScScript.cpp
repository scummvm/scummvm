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
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/scriptables/ScEngine.h"
#include "engines/wintermute/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CScScript, false)

//////////////////////////////////////////////////////////////////////////
CScScript::CScScript(CBGame *inGame, CScEngine *Engine): CBBase(inGame) {
	m_Buffer = NULL;
	m_BufferSize = m_IP = 0;
	m_Filename = NULL;
	m_CurrentLine = 0;

	m_Symbols = NULL;
	m_NumSymbols = 0;

	m_Engine = Engine;

	m_Globals = NULL;

	m_ScopeStack = NULL;
	m_CallStack  = NULL;
	m_ThisStack  = NULL;
	m_Stack      = NULL;

	m_Operand    = NULL;
	m_Reg1       = NULL;

	m_Functions = NULL;
	m_NumFunctions = 0;

	m_Methods = NULL;
	m_NumMethods = 0;

	m_Events = NULL;
	m_NumEvents = 0;

	m_Externals = NULL;
	m_NumExternals = 0;

	m_State = SCRIPT_FINISHED;
	m_OrigState = SCRIPT_FINISHED;

	m_WaitObject = NULL;
	m_WaitTime = 0;
	m_WaitFrozen = false;
	m_WaitScript = NULL;

	m_TimeSlice = 0;

	m_Thread = false;
	m_MethodThread = false;
	m_ThreadEvent = NULL;

	m_Freezable = true;
	m_Owner = NULL;

	m_Unbreakable = false;
	m_ParentScript = NULL;

	m_TracingMode = false;
}


//////////////////////////////////////////////////////////////////////////
CScScript::~CScScript() {
	Cleanup();
}



//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::InitScript() {
	TScriptHeader *Header = (TScriptHeader *)m_Buffer;
	if (Header->magic != SCRIPT_MAGIC) {
		Game->LOG(0, "File '%s' is not a valid compiled script", m_Filename);
		Cleanup();
		return E_FAIL;
	}

	if (Header->version > SCRIPT_VERSION) {
		Game->LOG(0, "Script '%s' has a wrong version %d.%d (expected %d.%d)", m_Filename, Header->version / 256, Header->version % 256, SCRIPT_VERSION / 256, SCRIPT_VERSION % 256);
		Cleanup();
		return E_FAIL;
	}

	InitTables();

	// init stacks
	m_ScopeStack = new CScStack(Game);
	m_CallStack  = new CScStack(Game);
	m_ThisStack  = new CScStack(Game);
	m_Stack      = new CScStack(Game);

	m_Operand    = new CScValue(Game);
	m_Reg1       = new CScValue(Game);


	// skip to the beginning
	m_IP = Header->code_start;
	m_CurrentLine = 0;

	// init breakpoints
	m_Engine->RefreshScriptBreakpoints(this);


	// ready to rumble...
	m_State = SCRIPT_RUNNING;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::InitTables() {
	uint32 OrigIP = m_IP;

	TScriptHeader *Header = (TScriptHeader *)m_Buffer;

	uint32 i;

	// load symbol table
	m_IP = Header->symbol_table;

	m_NumSymbols = GetDWORD();
	m_Symbols = new char*[m_NumSymbols];
	for (i = 0; i < m_NumSymbols; i++) {
		uint32 index = GetDWORD();
		m_Symbols[index] = GetString();
	}

	// load functions table
	m_IP = Header->func_table;

	m_NumFunctions = GetDWORD();
	m_Functions = new TFunctionPos[m_NumFunctions];
	for (i = 0; i < m_NumFunctions; i++) {
		m_Functions[i].pos = GetDWORD();
		m_Functions[i].name = GetString();
	}


	// load events table
	m_IP = Header->event_table;

	m_NumEvents = GetDWORD();
	m_Events = new TEventPos[m_NumEvents];
	for (i = 0; i < m_NumEvents; i++) {
		m_Events[i].pos = GetDWORD();
		m_Events[i].name = GetString();
	}


	// load externals
	if (Header->version >= 0x0101) {
		m_IP = Header->externals_table;

		m_NumExternals = GetDWORD();
		m_Externals = new TExternalFunction[m_NumExternals];
		for (i = 0; i < m_NumExternals; i++) {
			m_Externals[i].dll_name = GetString();
			m_Externals[i].name = GetString();
			m_Externals[i].call_type = (TCallType)GetDWORD();
			m_Externals[i].returns = (TExternalType)GetDWORD();
			m_Externals[i].num_params = GetDWORD();
			if (m_Externals[i].num_params > 0) {
				m_Externals[i].params = new TExternalType[m_Externals[i].num_params];
				for (int j = 0; j < m_Externals[i].num_params; j++) {
					m_Externals[i].params[j] = (TExternalType)GetDWORD();
				}
			}
		}
	}

	// load method table
	m_IP = Header->method_table;

	m_NumMethods = GetDWORD();
	m_Methods = new TMethodPos[m_NumMethods];
	for (i = 0; i < m_NumMethods; i++) {
		m_Methods[i].pos = GetDWORD();
		m_Methods[i].name = GetString();
	}


	m_IP = OrigIP;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Create(char *Filename, byte *Buffer, uint32 Size, CBScriptHolder *Owner) {
	Cleanup();

	m_Thread = false;
	m_MethodThread = false;

	delete[] m_ThreadEvent;
	m_ThreadEvent = NULL;

	m_Filename = new char[strlen(Filename) + 1];
	if (m_Filename) strcpy(m_Filename, Filename);

	m_Buffer = new byte [Size];
	if (!m_Buffer) return E_FAIL;

	memcpy(m_Buffer, Buffer, Size);

	m_BufferSize = Size;

	HRESULT res = InitScript();
	if (FAILED(res)) return res;

	// establish global variables table
	m_Globals = new CScValue(Game);

	m_Owner = Owner;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::CreateThread(CScScript *Original, uint32 InitIP, const char *EventName) {
	Cleanup();

	m_Thread = true;
	m_MethodThread = false;
	m_ThreadEvent = new char[strlen(EventName) + 1];
	if (m_ThreadEvent) strcpy(m_ThreadEvent, EventName);

	// copy filename
	m_Filename = new char[strlen(Original->m_Filename) + 1];
	if (m_Filename) strcpy(m_Filename, Original->m_Filename);

	// copy buffer
	m_Buffer = new byte [Original->m_BufferSize];
	if (!m_Buffer) return E_FAIL;

	memcpy(m_Buffer, Original->m_Buffer, Original->m_BufferSize);
	m_BufferSize = Original->m_BufferSize;

	// initialize
	HRESULT res = InitScript();
	if (FAILED(res)) return res;

	// copy globals
	m_Globals = Original->m_Globals;

	// skip to the beginning of the event
	m_IP = InitIP;

	m_TimeSlice = Original->m_TimeSlice;
	m_Freezable = Original->m_Freezable;
	m_Owner = Original->m_Owner;

	m_Engine = Original->m_Engine;
	m_ParentScript = Original;

	return S_OK;
}




//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::CreateMethodThread(CScScript *Original, const char *MethodName) {
	uint32 IP = Original->GetMethodPos(MethodName);
	if (IP == 0) return E_FAIL;

	Cleanup();

	m_Thread = true;
	m_MethodThread = true;
	m_ThreadEvent = new char[strlen(MethodName) + 1];
	if (m_ThreadEvent) strcpy(m_ThreadEvent, MethodName);

	// copy filename
	m_Filename = new char[strlen(Original->m_Filename) + 1];
	if (m_Filename) strcpy(m_Filename, Original->m_Filename);

	// copy buffer
	m_Buffer = new byte [Original->m_BufferSize];
	if (!m_Buffer) return E_FAIL;

	memcpy(m_Buffer, Original->m_Buffer, Original->m_BufferSize);
	m_BufferSize = Original->m_BufferSize;

	// initialize
	HRESULT res = InitScript();
	if (FAILED(res)) return res;

	// copy globals
	m_Globals = Original->m_Globals;

	// skip to the beginning of the event
	m_IP = IP;

	m_TimeSlice = Original->m_TimeSlice;
	m_Freezable = Original->m_Freezable;
	m_Owner = Original->m_Owner;

	m_Engine = Original->m_Engine;
	m_ParentScript = Original;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CScScript::Cleanup() {
	if (m_Buffer) delete [] m_Buffer;
	m_Buffer = NULL;

	if (m_Filename) delete [] m_Filename;
	m_Filename = NULL;

	if (m_Symbols) delete [] m_Symbols;
	m_Symbols = NULL;
	m_NumSymbols = 0;

	if (m_Globals && !m_Thread) delete m_Globals;
	m_Globals = NULL;

	if (m_ScopeStack) delete m_ScopeStack;
	m_ScopeStack = NULL;

	if (m_CallStack) delete m_CallStack;
	m_CallStack = NULL;

	if (m_ThisStack) delete m_ThisStack;
	m_ThisStack = NULL;

	if (m_Stack) delete m_Stack;
	m_Stack = NULL;

	if (m_Functions) delete [] m_Functions;
	m_Functions = NULL;
	m_NumFunctions = 0;

	if (m_Methods) delete [] m_Methods;
	m_Methods = NULL;
	m_NumMethods = 0;

	if (m_Events) delete [] m_Events;
	m_Events = NULL;
	m_NumEvents = 0;


	if (m_Externals) {
		for (int i = 0; i < m_NumExternals; i++) {
			if (m_Externals[i].num_params > 0) delete [] m_Externals[i].params;
		}
		delete [] m_Externals;
	}
	m_Externals = NULL;
	m_NumExternals = 0;

	delete m_Operand;
	delete m_Reg1;
	m_Operand = NULL;
	m_Reg1 = NULL;

	delete[] m_ThreadEvent;
	m_ThreadEvent = NULL;

	m_State = SCRIPT_FINISHED;

	m_WaitObject = NULL;
	m_WaitTime = 0;
	m_WaitFrozen = false;
	m_WaitScript = NULL;

	m_ParentScript = NULL; // ref only
}


//////////////////////////////////////////////////////////////////////////
uint32 CScScript::GetDWORD() {
	uint32 ret = *(uint32 *)(m_Buffer + m_IP);
	m_IP += sizeof(uint32);

	return ret;
}


//////////////////////////////////////////////////////////////////////////
double CScScript::GetFloat() {
	double ret = *(double *)(m_Buffer + m_IP);
	m_IP += sizeof(double);

	return ret;
}


//////////////////////////////////////////////////////////////////////////
char *CScScript::GetString() {
	char *ret = (char *)(m_Buffer + m_IP);
	while (*(char *)(m_Buffer + m_IP) != '\0') m_IP++;
	m_IP++; // string terminator

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::ExecuteInstruction() {
	HRESULT ret = S_OK;

	uint32 dw;
	char *str;

	//CScValue* op = new CScValue(Game);
	m_Operand->Cleanup();

	CScValue *op1;
	CScValue *op2;

	uint32 inst = GetDWORD();
	switch (inst) {

	case II_DEF_VAR:
		m_Operand->SetNULL();
		dw = GetDWORD();
		if (m_ScopeStack->m_SP < 0) {
			m_Globals->SetProp(m_Symbols[dw], m_Operand);
			if (Game->GetDebugMgr()->m_Enabled)
				Game->GetDebugMgr()->OnVariableInit(WME_DBGVAR_SCRIPT, this, NULL, m_Globals->GetProp(m_Symbols[dw]), m_Symbols[dw]);
		} else {
			m_ScopeStack->GetTop()->SetProp(m_Symbols[dw], m_Operand);
			if (Game->GetDebugMgr()->m_Enabled)
				Game->GetDebugMgr()->OnVariableInit(WME_DBGVAR_SCOPE, this, m_ScopeStack->GetTop(), m_ScopeStack->GetTop()->GetProp(m_Symbols[dw]), m_Symbols[dw]);
		}

		break;

	case II_DEF_GLOB_VAR:
	case II_DEF_CONST_VAR: {
		dw = GetDWORD();
		char *Temp = m_Symbols[dw]; // TODO delete
		// only create global var if it doesn't exist
		if (!m_Engine->m_Globals->PropExists(m_Symbols[dw])) {
			m_Operand->SetNULL();
			m_Engine->m_Globals->SetProp(m_Symbols[dw], m_Operand, false, inst == II_DEF_CONST_VAR);

			if (Game->GetDebugMgr()->m_Enabled)
				Game->GetDebugMgr()->OnVariableInit(WME_DBGVAR_GLOBAL, this, NULL, m_Engine->m_Globals->GetProp(m_Symbols[dw]), m_Symbols[dw]);
		}
		break;
	}

	case II_RET:
		if (m_ScopeStack->m_SP >= 0 && m_CallStack->m_SP >= 0) {
			Game->GetDebugMgr()->OnScriptShutdownScope(this, m_ScopeStack->GetTop());

			m_ScopeStack->Pop();
			m_IP = (uint32)m_CallStack->Pop()->GetInt();

			if (m_ScopeStack->m_SP < 0) Game->GetDebugMgr()->OnScriptChangeScope(this, NULL);
			else Game->GetDebugMgr()->OnScriptChangeScope(this, m_ScopeStack->GetTop());
		} else {
			if (m_Thread) {
				m_State = SCRIPT_THREAD_FINISHED;
			} else {
				if (m_NumEvents == 0 && m_NumMethods == 0) m_State = SCRIPT_FINISHED;
				else m_State = SCRIPT_PERSISTENT;
			}
		}

		break;

	case II_RET_EVENT:
		m_State = SCRIPT_FINISHED;
		break;


	case II_CALL:
		dw = GetDWORD();

		m_Operand->SetInt(m_IP);
		m_CallStack->Push(m_Operand);

		m_IP = dw;

		break;

	case II_CALL_BY_EXP: {
		// push var
		// push string
		str = m_Stack->Pop()->GetString();
		char *MethodName = new char[strlen(str) + 1];
		strcpy(MethodName, str);

		CScValue *var = m_Stack->Pop();
		if (var->m_Type == VAL_VARIABLE_REF) var = var->m_ValRef;

		HRESULT res = E_FAIL;
		bool TriedNative = false;

		// we are already calling this method, try native
		if (m_Thread && m_MethodThread && strcmp(MethodName, m_ThreadEvent) == 0 && var->m_Type == VAL_NATIVE && m_Owner == var->GetNative()) {
			TriedNative = true;
			res = var->m_ValNative->ScCallMethod(this, m_Stack, m_ThisStack, MethodName);
		}

		if (FAILED(res)) {
			if (var->IsNative() && var->GetNative()->CanHandleMethod(MethodName)) {
				if (!m_Unbreakable) {
					m_WaitScript = var->GetNative()->InvokeMethodThread(MethodName);
					if (!m_WaitScript) {
						m_Stack->CorrectParams(0);
						RuntimeError("Error invoking method '%s'.", MethodName);
						m_Stack->PushNULL();
					} else {
						m_State = SCRIPT_WAITING_SCRIPT;
						m_WaitScript->CopyParameters(m_Stack);
					}
				} else {
					// can call methods in unbreakable mode
					m_Stack->CorrectParams(0);
					RuntimeError("Cannot call method '%s'. Ignored.", MethodName);
					m_Stack->PushNULL();
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
			            ExternalCall(m_Stack, m_ThisStack, f);
			        }
			        else{
			            // not an internal nor external, try for native function
			            Game->ExternalCall(this, m_Stack, m_ThisStack, val->GetString());
			        }
			    }
			    else{
			        m_Operand->SetInt(m_IP);
			        m_CallStack->Push(m_Operand);
			        m_IP = dw;
			    }
			}
			*/
			else {
				res = E_FAIL;
				if (var->m_Type == VAL_NATIVE && !TriedNative) res = var->m_ValNative->ScCallMethod(this, m_Stack, m_ThisStack, MethodName);

				if (FAILED(res)) {
					m_Stack->CorrectParams(0);
					RuntimeError("Call to undefined method '%s'. Ignored.", MethodName);
					m_Stack->PushNULL();
				}
			}
		}
		delete [] MethodName;
	}
	break;

	case II_EXTERNAL_CALL: {
		uint32 SymbolIndex = GetDWORD();

		TExternalFunction *f = GetExternal(m_Symbols[SymbolIndex]);
		if (f) {
			ExternalCall(m_Stack, m_ThisStack, f);
		} else Game->ExternalCall(this, m_Stack, m_ThisStack, m_Symbols[SymbolIndex]);

		break;
	}
	case II_SCOPE:
		m_Operand->SetNULL();
		m_ScopeStack->Push(m_Operand);

		if (m_ScopeStack->m_SP < 0) Game->GetDebugMgr()->OnScriptChangeScope(this, NULL);
		else Game->GetDebugMgr()->OnScriptChangeScope(this, m_ScopeStack->GetTop());

		break;

	case II_CORRECT_STACK:
		dw = GetDWORD(); // params expected
		m_Stack->CorrectParams(dw);
		break;

	case II_CREATE_OBJECT:
		m_Operand->SetObject();
		m_Stack->Push(m_Operand);
		break;

	case II_POP_EMPTY:
		m_Stack->Pop();
		break;

	case II_PUSH_VAR: {
		CScValue *var = GetVar(m_Symbols[GetDWORD()]);
		if (false && /*var->m_Type==VAL_OBJECT ||*/ var->m_Type == VAL_NATIVE) {
			m_Operand->SetReference(var);
			m_Stack->Push(m_Operand);
		} else m_Stack->Push(var);
		break;
	}

	case II_PUSH_VAR_REF: {
		CScValue *var = GetVar(m_Symbols[GetDWORD()]);
		m_Operand->SetReference(var);
		m_Stack->Push(m_Operand);
		break;
	}

	case II_POP_VAR: {
		char *VarName = m_Symbols[GetDWORD()];
		CScValue *var = GetVar(VarName);
		if (var) {
			CScValue *val = m_Stack->Pop();
			if (!val) {
				RuntimeError("Script stack corruption detected. Please report this script at WME bug reports forum.");
				var->SetNULL();
			} else {
				if (val->GetType() == VAL_VARIABLE_REF) val = val->m_ValRef;
				if (val->m_Type == VAL_NATIVE) var->SetValue(val);
				else {
					var->Copy(val);
				}
			}

			if (Game->GetDebugMgr()->m_Enabled)
				Game->GetDebugMgr()->OnVariableChangeValue(var, val);
		}

		break;
	}

	case II_PUSH_VAR_THIS:
		m_Stack->Push(m_ThisStack->GetTop());
		break;

	case II_PUSH_INT:
		m_Stack->PushInt((int)GetDWORD());
		break;

	case II_PUSH_FLOAT:
		m_Stack->PushFloat(GetFloat());
		break;


	case II_PUSH_BOOL:
		m_Stack->PushBool(GetDWORD() != 0);

		break;

	case II_PUSH_STRING:
		m_Stack->PushString(GetString());
		break;

	case II_PUSH_NULL:
		m_Stack->PushNULL();
		break;

	case II_PUSH_THIS_FROM_STACK:
		m_Operand->SetReference(m_Stack->GetTop());
		m_ThisStack->Push(m_Operand);
		break;

	case II_PUSH_THIS:
		m_Operand->SetReference(GetVar(m_Symbols[GetDWORD()]));
		m_ThisStack->Push(m_Operand);
		break;

	case II_POP_THIS:
		m_ThisStack->Pop();
		break;

	case II_PUSH_BY_EXP: {
		str = m_Stack->Pop()->GetString();
		CScValue *val = m_Stack->Pop()->GetProp(str);
		if (val) m_Stack->Push(val);
		else m_Stack->PushNULL();

		break;
	}

	case II_POP_BY_EXP: {
		str = m_Stack->Pop()->GetString();
		CScValue *var = m_Stack->Pop();
		CScValue *val = m_Stack->Pop();

		if (val == NULL) {
			RuntimeError("Script stack corruption detected. Please report this script at WME bug reports forum.");
			var->SetNULL();
		} else var->SetProp(str, val);

		if (Game->GetDebugMgr()->m_Enabled)
			Game->GetDebugMgr()->OnVariableChangeValue(var, NULL);

		break;
	}

	case II_PUSH_REG1:
		m_Stack->Push(m_Reg1);
		break;

	case II_POP_REG1:
		m_Reg1->Copy(m_Stack->Pop());
		break;

	case II_JMP:
		m_IP = GetDWORD();
		break;

	case II_JMP_FALSE: {
		dw = GetDWORD();
		//if(!m_Stack->Pop()->GetBool()) m_IP = dw;
		CScValue *Val = m_Stack->Pop();
		if (!Val) {
			RuntimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
		} else {
			if (!Val->GetBool()) m_IP = dw;
		}
		break;
	}

	case II_ADD:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		if (op1->IsNULL() || op2->IsNULL()) m_Operand->SetNULL();
		else if (op1->GetType() == VAL_STRING || op2->GetType() == VAL_STRING) {
			str = new char [strlen(op1->GetString()) + strlen(op2->GetString()) + 1];
			strcpy(str, op1->GetString());
			strcat(str, op2->GetString());
			m_Operand->SetString(str);
			delete [] str;
		} else if (op1->GetType() == VAL_INT && op2->GetType() == VAL_INT)
			m_Operand->SetInt(op1->GetInt() + op2->GetInt());
		else m_Operand->SetFloat(op1->GetFloat() + op2->GetFloat());

		m_Stack->Push(m_Operand);

		break;

	case II_SUB:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		if (op1->IsNULL() || op2->IsNULL()) m_Operand->SetNULL();
		else if (op1->GetType() == VAL_INT && op2->GetType() == VAL_INT)
			m_Operand->SetInt(op1->GetInt() - op2->GetInt());
		else m_Operand->SetFloat(op1->GetFloat() - op2->GetFloat());

		m_Stack->Push(m_Operand);

		break;

	case II_MUL:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		if (op1->IsNULL() || op2->IsNULL()) m_Operand->SetNULL();
		else if (op1->GetType() == VAL_INT && op2->GetType() == VAL_INT)
			m_Operand->SetInt(op1->GetInt() * op2->GetInt());
		else m_Operand->SetFloat(op1->GetFloat() * op2->GetFloat());

		m_Stack->Push(m_Operand);

		break;

	case II_DIV:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		if (op2->GetFloat() == 0.0f) RuntimeError("Division by zero.");

		if (op1->IsNULL() || op2->IsNULL() || op2->GetFloat() == 0.0f) m_Operand->SetNULL();
		else m_Operand->SetFloat(op1->GetFloat() / op2->GetFloat());

		m_Stack->Push(m_Operand);

		break;

	case II_MODULO:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		if (op2->GetInt() == 0) RuntimeError("Division by zero.");

		if (op1->IsNULL() || op2->IsNULL() || op2->GetInt() == 0) m_Operand->SetNULL();
		else m_Operand->SetInt(op1->GetInt() % op2->GetInt());

		m_Stack->Push(m_Operand);

		break;

	case II_NOT:
		op1 = m_Stack->Pop();
		//if(op1->IsNULL()) m_Operand->SetNULL();
		if (op1->IsNULL()) m_Operand->SetBool(true);
		else m_Operand->SetBool(!op1->GetBool());
		m_Stack->Push(m_Operand);

		break;

	case II_AND:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();
		if (op1 == NULL || op2 == NULL) {
			RuntimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
			m_Operand->SetBool(false);
		} else {
			m_Operand->SetBool(op1->GetBool() && op2->GetBool());
		}
		m_Stack->Push(m_Operand);
		break;

	case II_OR:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();
		if (op1 == NULL || op2 == NULL) {
			RuntimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
			m_Operand->SetBool(false);
		} else {
			m_Operand->SetBool(op1->GetBool() || op2->GetBool());
		}
		m_Stack->Push(m_Operand);
		break;

	case II_CMP_EQ:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		/*
		if((op1->IsNULL() && !op2->IsNULL()) || (!op1->IsNULL() && op2->IsNULL())) m_Operand->SetBool(false);
		else if(op1->IsNative() && op2->IsNative()){
		    m_Operand->SetBool(op1->GetNative() == op2->GetNative());
		}
		else if(op1->GetType()==VAL_STRING || op2->GetType()==VAL_STRING){
		    m_Operand->SetBool(scumm_stricmp(op1->GetString(), op2->GetString())==0);
		}
		else if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    m_Operand->SetBool(op1->GetFloat() == op2->GetFloat());
		}
		else{
		    m_Operand->SetBool(op1->GetInt() == op2->GetInt());
		}
		*/

		m_Operand->SetBool(CScValue::Compare(op1, op2) == 0);
		m_Stack->Push(m_Operand);
		break;

	case II_CMP_NE:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		/*
		if((op1->IsNULL() && !op2->IsNULL()) || (!op1->IsNULL() && op2->IsNULL())) m_Operand->SetBool(true);
		else if(op1->IsNative() && op2->IsNative()){
		    m_Operand->SetBool(op1->GetNative() != op2->GetNative());
		}
		else if(op1->GetType()==VAL_STRING || op2->GetType()==VAL_STRING){
		    m_Operand->SetBool(scumm_stricmp(op1->GetString(), op2->GetString())!=0);
		}
		else if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    m_Operand->SetBool(op1->GetFloat() != op2->GetFloat());
		}
		else{
		    m_Operand->SetBool(op1->GetInt() != op2->GetInt());
		}
		*/

		m_Operand->SetBool(CScValue::Compare(op1, op2) != 0);
		m_Stack->Push(m_Operand);
		break;

	case II_CMP_L:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		/*
		if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    m_Operand->SetBool(op1->GetFloat() < op2->GetFloat());
		}
		else m_Operand->SetBool(op1->GetInt() < op2->GetInt());
		*/

		m_Operand->SetBool(CScValue::Compare(op1, op2) < 0);
		m_Stack->Push(m_Operand);
		break;

	case II_CMP_G:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		/*
		if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    m_Operand->SetBool(op1->GetFloat() > op2->GetFloat());
		}
		else m_Operand->SetBool(op1->GetInt() > op2->GetInt());
		*/

		m_Operand->SetBool(CScValue::Compare(op1, op2) > 0);
		m_Stack->Push(m_Operand);
		break;

	case II_CMP_LE:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		/*
		if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    m_Operand->SetBool(op1->GetFloat() <= op2->GetFloat());
		}
		else m_Operand->SetBool(op1->GetInt() <= op2->GetInt());
		*/

		m_Operand->SetBool(CScValue::Compare(op1, op2) <= 0);
		m_Stack->Push(m_Operand);
		break;

	case II_CMP_GE:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		/*
		if(op1->GetType()==VAL_FLOAT && op2->GetType()==VAL_FLOAT){
		    m_Operand->SetBool(op1->GetFloat() >= op2->GetFloat());
		}
		else m_Operand->SetBool(op1->GetInt() >= op2->GetInt());
		*/

		m_Operand->SetBool(CScValue::Compare(op1, op2) >= 0);
		m_Stack->Push(m_Operand);
		break;

	case II_CMP_STRICT_EQ:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		//m_Operand->SetBool(op1->GetType()==op2->GetType() && op1->GetFloat()==op2->GetFloat());
		m_Operand->SetBool(CScValue::CompareStrict(op1, op2) == 0);
		m_Stack->Push(m_Operand);

		break;

	case II_CMP_STRICT_NE:
		op2 = m_Stack->Pop();
		op1 = m_Stack->Pop();

		//m_Operand->SetBool(op1->GetType()!=op2->GetType() || op1->GetFloat()!=op2->GetFloat());
		m_Operand->SetBool(CScValue::CompareStrict(op1, op2) != 0);
		m_Stack->Push(m_Operand);
		break;

	case II_DBG_LINE: {
		int NewLine = GetDWORD();
		if (NewLine != m_CurrentLine) {
			m_CurrentLine = NewLine;
			if (Game->GetDebugMgr()->m_Enabled) {
				Game->GetDebugMgr()->OnScriptChangeLine(this, m_CurrentLine);
				for (int i = 0; i < m_Breakpoints.GetSize(); i++) {
					if (m_Breakpoints[i] == m_CurrentLine) {
						Game->GetDebugMgr()->OnScriptHitBreakpoint(this);
						Sleep(0);
						break;
					}
				}
				if (m_TracingMode) {
					Game->GetDebugMgr()->OnScriptHitBreakpoint(this);
					Sleep(0);
					break;
				}
			}
		}
		break;

	}
	default:
		Game->LOG(0, "Fatal: Invalid instruction %d ('%s', line %d, IP:0x%x)\n", inst, m_Filename, m_CurrentLine, m_IP - sizeof(uint32));
		m_State = SCRIPT_FINISHED;
		ret = E_FAIL;
	} // switch(instruction)

	//delete op;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
uint32 CScScript::GetFuncPos(const char *Name) {
	for (int i = 0; i < m_NumFunctions; i++) {
		if (strcmp(Name, m_Functions[i].name) == 0) return m_Functions[i].pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
uint32 CScScript::GetMethodPos(const char *Name) {
	for (int i = 0; i < m_NumMethods; i++) {
		if (strcmp(Name, m_Methods[i].name) == 0) return m_Methods[i].pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScScript::GetVar(char *Name) {
	CScValue *ret = NULL;

	// scope locals
	if (m_ScopeStack->m_SP >= 0) {
		if (m_ScopeStack->GetTop()->PropExists(Name)) ret = m_ScopeStack->GetTop()->GetProp(Name);
	}

	// script globals
	if (ret == NULL) {
		if (m_Globals->PropExists(Name)) ret = m_Globals->GetProp(Name);
	}

	// engine globals
	if (ret == NULL) {
		if (m_Engine->m_Globals->PropExists(Name)) ret = m_Engine->m_Globals->GetProp(Name);
	}

	if (ret == NULL) {
		//RuntimeError("Variable '%s' is inaccessible in the current block. Consider changing the script.", Name);
		Game->LOG(0, "Warning: variable '%s' is inaccessible in the current block. Consider changing the script (script:%s, line:%d)", Name, m_Filename, m_CurrentLine);
		CScValue *Val = new CScValue(Game);
		CScValue *Scope = m_ScopeStack->GetTop();
		if (Scope) {
			Scope->SetProp(Name, Val);
			ret = m_ScopeStack->GetTop()->GetProp(Name);
		} else {
			m_Globals->SetProp(Name, Val);
			ret = m_Globals->GetProp(Name);
		}
		delete Val;
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::WaitFor(CBObject *Object) {
	if (m_Unbreakable) {
		RuntimeError("Script cannot be interrupted.");
		return S_OK;
	}

	m_State = SCRIPT_WAITING;
	m_WaitObject = Object;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::WaitForExclusive(CBObject *Object) {
	m_Engine->ResetObject(Object);
	return WaitFor(Object);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Sleep(uint32 Duration) {
	if (m_Unbreakable) {
		RuntimeError("Script cannot be interrupted.");
		return S_OK;
	}

	m_State = SCRIPT_SLEEPING;
	if (Game->m_State == GAME_FROZEN) {
		m_WaitTime = CBPlatform::GetTime() + Duration;
		m_WaitFrozen = true;
	} else {
		m_WaitTime = Game->m_Timer + Duration;
		m_WaitFrozen = false;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Finish(bool IncludingThreads) {
	if (m_State != SCRIPT_FINISHED && IncludingThreads) {
		m_State = SCRIPT_FINISHED;
		FinishThreads();
	} else m_State = SCRIPT_FINISHED;


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Run() {
	m_State = SCRIPT_RUNNING;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////
void CScScript::RuntimeError(LPCSTR fmt, ...) {
	char buff[256];
	va_list va;

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	Game->LOG(0, "Runtime error. Script '%s', line %d", m_Filename, m_CurrentLine);
	Game->LOG(0, "  %s", buff);

	if (!Game->m_SuppressScriptErrors)
		Game->QuickMessage("Script runtime error. View log for details.");
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Persist(CBPersistMgr *PersistMgr) {

	PersistMgr->Transfer(TMEMBER(Game));

	// buffer
	if (PersistMgr->m_Saving) {
		if (m_State != SCRIPT_PERSISTENT && m_State != SCRIPT_FINISHED && m_State != SCRIPT_THREAD_FINISHED) {
			PersistMgr->Transfer(TMEMBER(m_BufferSize));
			PersistMgr->PutBytes(m_Buffer, m_BufferSize);
		} else {
			// don't save idle/finished scripts
			int bufferSize = 0;
			PersistMgr->Transfer(TMEMBER(bufferSize));
		}
	} else {
		PersistMgr->Transfer(TMEMBER(m_BufferSize));
		if (m_BufferSize > 0) {
			m_Buffer = new byte[m_BufferSize];
			PersistMgr->GetBytes(m_Buffer, m_BufferSize);
			InitTables();
		} else m_Buffer = NULL;
	}

	PersistMgr->Transfer(TMEMBER(m_CallStack));
	PersistMgr->Transfer(TMEMBER(m_CurrentLine));
	PersistMgr->Transfer(TMEMBER(m_Engine));
	PersistMgr->Transfer(TMEMBER(m_Filename));
	PersistMgr->Transfer(TMEMBER(m_Freezable));
	PersistMgr->Transfer(TMEMBER(m_Globals));
	PersistMgr->Transfer(TMEMBER(m_IP));
	PersistMgr->Transfer(TMEMBER(m_ScopeStack));
	PersistMgr->Transfer(TMEMBER(m_Stack));
	PersistMgr->Transfer(TMEMBER_INT(m_State));
	PersistMgr->Transfer(TMEMBER(m_Operand));
	PersistMgr->Transfer(TMEMBER_INT(m_OrigState));
	PersistMgr->Transfer(TMEMBER(m_Owner));
	PersistMgr->Transfer(TMEMBER(m_Reg1));
	PersistMgr->Transfer(TMEMBER(m_Thread));
	PersistMgr->Transfer(TMEMBER(m_ThreadEvent));
	PersistMgr->Transfer(TMEMBER(m_ThisStack));
	PersistMgr->Transfer(TMEMBER(m_TimeSlice));
	PersistMgr->Transfer(TMEMBER(m_WaitObject));
	PersistMgr->Transfer(TMEMBER(m_WaitScript));
	PersistMgr->Transfer(TMEMBER(m_WaitTime));
	PersistMgr->Transfer(TMEMBER(m_WaitFrozen));

	PersistMgr->Transfer(TMEMBER(m_MethodThread));
	PersistMgr->Transfer(TMEMBER(m_MethodThread));
	PersistMgr->Transfer(TMEMBER(m_Unbreakable));
	PersistMgr->Transfer(TMEMBER(m_ParentScript));

	if (!PersistMgr->m_Saving) m_TracingMode = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CScScript *CScScript::InvokeEventHandler(const char *EventName, bool Unbreakable) {
	//if(m_State!=SCRIPT_PERSISTENT) return NULL;

	uint32 pos = GetEventPos(EventName);
	if (!pos) return NULL;

	CScScript *thread = new CScScript(Game, m_Engine);
	if (thread) {
		HRESULT ret = thread->CreateThread(this, pos, EventName);
		if (SUCCEEDED(ret)) {
			thread->m_Unbreakable = Unbreakable;
			m_Engine->m_Scripts.Add(thread);
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
	for (int i = m_NumEvents - 1; i >= 0; i--) {
		if (scumm_stricmp(Name, m_Events[i].name) == 0) return m_Events[i].pos;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
bool CScScript::CanHandleEvent(char *EventName) {
	return GetEventPos(EventName) != 0;
}


//////////////////////////////////////////////////////////////////////////
bool CScScript::CanHandleMethod(char *MethodName) {
	return GetMethodPos(MethodName) != 0;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Pause() {
	if (m_State == SCRIPT_PAUSED) {
		Game->LOG(0, "Attempting to pause a paused script ('%s', line %d)", m_Filename, m_CurrentLine);
		return E_FAIL;
	}

	if (!m_Freezable || m_State == SCRIPT_PERSISTENT) return S_OK;

	m_OrigState = m_State;
	m_State = SCRIPT_PAUSED;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::Resume() {
	if (m_State != SCRIPT_PAUSED) return S_OK;

	m_State = m_OrigState;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CScScript::TExternalFunction *CScScript::GetExternal(char *Name) {
	for (int i = 0; i < m_NumExternals; i++) {
		if (strcmp(Name, m_Externals[i].name) == 0) return &m_Externals[i];
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
			Stack->CorrectParams(Function->num_params);
			CBDynBuffer *Buffer = new CBDynBuffer(Game, 20 * sizeof(uint32));

			for (i = 0; i < Function->num_params; i++) {
				CScValue *Val = Stack->Pop();
				switch (Function->params[i]) {
				case TYPE_BOOL:
					Buffer->PutDWORD((uint32)Val->GetBool());
					break;
				case TYPE_LONG:
					Buffer->PutDWORD(Val->GetInt());
					break;
				case TYPE_BYTE:
					Buffer->PutDWORD((byte )Val->GetInt());
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
				ret = Call_cdecl(Buffer->m_Buffer, Buffer->GetSize(), (uint32)pFunc, &StackCorrupted);
				break;
			default:
				ret = Call_stdcall(Buffer->m_Buffer, Buffer->GetSize(), (uint32)pFunc, &StackCorrupted);
			}
			delete Buffer;

			// return
			switch (Function->returns) {
			case TYPE_BOOL:
				Stack->PushBool((byte )ret != 0);
				break;
			case TYPE_LONG:
				Stack->PushInt(ret);
				break;
			case TYPE_BYTE:
				Stack->PushInt((byte )ret);
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
		m_Stack->Push(Stack->GetAt(i));
	}
	m_Stack->PushInt(NumParams);

	for (i = 0; i < NumParams; i++) Stack->Pop();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::FinishThreads() {
	for (int i = 0; i < m_Engine->m_Scripts.GetSize(); i++) {
		CScScript *Scr = m_Engine->m_Scripts[i];
		if (Scr->m_Thread && Scr->m_State != SCRIPT_FINISHED && Scr->m_Owner == m_Owner && scumm_stricmp(Scr->m_Filename, m_Filename) == 0)
			Scr->Finish(true);
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// IWmeDebugScript interface implementation
int CScScript::DbgGetLine() {
	return m_CurrentLine;
}

//////////////////////////////////////////////////////////////////////////
const char *CScScript::DbgGetFilename() {
	return m_Filename;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::DbgSendScript(IWmeDebugClient *Client) {
	if (m_MethodThread) Client->OnScriptMethodThreadInit(this, m_ParentScript, m_ThreadEvent);
	else if (m_Thread) Client->OnScriptEventThreadInit(this, m_ParentScript, m_ThreadEvent);
	else Client->OnScriptInit(this);

	return DbgSendVariables(Client);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScScript::DbgSendVariables(IWmeDebugClient *Client) {
	// send script globals
	m_Globals->DbgSendVariables(Client, WME_DBGVAR_SCRIPT, this, 0);

	// send scope variables
	if (m_ScopeStack->m_SP >= 0) {
		for (int i = 0; i <= m_ScopeStack->m_SP; i++) {
			CScValue *Scope = m_ScopeStack->GetAt(i);
			//Scope->DbgSendVariables(Client, WME_DBGVAR_SCOPE, this, (unsigned int)Scope);
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
TScriptState CScScript::DbgGetState() {
	return m_State;
}

//////////////////////////////////////////////////////////////////////////
int CScScript::DbgGetNumBreakpoints() {
	return m_Breakpoints.GetSize();
}

//////////////////////////////////////////////////////////////////////////
int CScScript::DbgGetBreakpoint(int Index) {
	if (Index >= 0 && Index < m_Breakpoints.GetSize()) return m_Breakpoints[Index];
	else return -1;
}

//////////////////////////////////////////////////////////////////////////
bool CScScript::DbgSetTracingMode(bool IsTracing) {
	m_TracingMode = IsTracing;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScScript::DbgGetTracingMode() {
	return m_TracingMode;
}


//////////////////////////////////////////////////////////////////////////
void CScScript::AfterLoad() {
	if (m_Buffer == NULL) {
		byte *buffer = m_Engine->GetCompiledScript(m_Filename, &m_BufferSize);
		if (!buffer) {
			Game->LOG(0, "Error reinitializing script '%s' after load. Script will be terminated.", m_Filename);
			m_State = SCRIPT_ERROR;
			return;
		}

		m_Buffer = new byte [m_BufferSize];
		memcpy(m_Buffer, buffer, m_BufferSize);

		InitTables();
	}
}

} // end of namespace WinterMute
