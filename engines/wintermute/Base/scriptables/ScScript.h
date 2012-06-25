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

#ifndef WINTERMUTE_SCSCRIPT_H
#define WINTERMUTE_SCSCRIPT_H


#include "engines/wintermute/Base/BBase.h"
#include "engines/wintermute/dcscript.h"   // Added by ClassView
#include "engines/wintermute/coll_templ.h"

#include "engines/wintermute/wme_debugger.h"

namespace WinterMute {
class CBScriptHolder;
class CBObject;
class CScEngine;
class CScStack;
class CScScript : public CBBase, public IWmeDebugScript {
public:
	HRESULT DbgSendScript(IWmeDebugClient *Client);
	HRESULT DbgSendVariables(IWmeDebugClient *Client);

	CBArray<int, int> _breakpoints;
	bool _tracingMode;

	CScScript *_parentScript;
	bool _unbreakable;
	HRESULT FinishThreads();
	HRESULT CopyParameters(CScStack *Stack);

	void AfterLoad();

#ifdef __WIN32__
	static uint32 Call_cdecl(const void *args, size_t sz, uint32 func, bool *StackCorrupt);
	static uint32 Call_stdcall(const void *args, size_t sz, uint32 func, bool *StackCorrupt);
	static uint32 GetST0(void);
	static double GetST0Double(void);
#endif

	CScValue *_operand;
	CScValue *_reg1;
	bool _freezable;
	HRESULT Resume();
	HRESULT Pause();
	bool CanHandleEvent(const char *EventName);
	bool canHandleMethod(const char *MethodName);
	HRESULT CreateThread(CScScript *Original, uint32 InitIP, const char *EventName);
	HRESULT CreateMethodThread(CScScript *Original, const char *MethodName);
	CScScript *InvokeEventHandler(const char *EventName, bool Unbreakable = false);
	uint32 _timeSlice;
	DECLARE_PERSISTENT(CScScript, CBBase)
	void RuntimeError(LPCSTR fmt, ...);
	HRESULT Run();
	HRESULT Finish(bool IncludingThreads = false);
	HRESULT Sleep(uint32 Duration);
	HRESULT WaitForExclusive(CBObject *Object);
	HRESULT WaitFor(CBObject *Object);
	uint32 _waitTime;
	bool _waitFrozen;
	CBObject *_waitObject;
	CScScript *_waitScript;
	TScriptState _state;
	TScriptState _origState;
	CScValue *GetVar(char *Name);
	uint32 GetFuncPos(const char *Name);
	uint32 GetEventPos(const char *Name);
	uint32 GetMethodPos(const char *Name);
	typedef struct {
		uint32 magic;
		uint32 version;
		uint32 code_start;
		uint32 func_table;
		uint32 symbol_table;
		uint32 event_table;
		uint32 externals_table;
		uint32 method_table;
	} TScriptHeader;


	typedef struct {
		char *name;
		uint32 pos;
	} TFunctionPos;

	typedef struct {
		char *name;
		uint32 pos;
	} TMethodPos;

	typedef struct {
		char *name;
		uint32 pos;
	} TEventPos;

	typedef struct {
		char *name;
		char *dll_name;
		TCallType call_type;
		TExternalType returns;
		int nu_params;
		TExternalType *params;
	} TExternalFunction;


	CScStack *_callStack;
	CScStack *_thisStack;
	CScStack *_scopeStack;
	CScStack *_stack;
	CScValue *_globals;
	CScEngine *_engine;
	int _currentLine;
	HRESULT ExecuteInstruction();
	char *GetString();
	uint32 GetDWORD();
	double GetFloat();
	void Cleanup();
	HRESULT Create(const char *Filename, byte *Buffer, uint32 Size, CBScriptHolder *Owner);
	uint32 _iP;
	uint32 _bufferSize;
	byte *_buffer;
	CScScript(CBGame *inGame, CScEngine *Engine);
	virtual ~CScScript();
	char *_filename;
	char **_symbols;
	int _numSymbols;
	TFunctionPos *_functions;
	TMethodPos *_methods;
	TEventPos *_events;
	int _numExternals;
	TExternalFunction *_externals;
	int _numFunctions;
	int _numMethods;
	int _numEvents;
	bool _thread;
	bool _methodThread;
	char *_threadEvent;
	CBScriptHolder *_owner;
	CScScript::TExternalFunction *GetExternal(char *Name);
	HRESULT ExternalCall(CScStack *Stack, CScStack *ThisStack, CScScript::TExternalFunction *Function);
private:
	HRESULT InitScript();
	HRESULT InitTables();


// IWmeDebugScript interface implementation
public:
	virtual int DbgGetLine();
	virtual const char *DbgGetFilename();
	virtual TScriptState DbgGetState();
	virtual int DbgGetNumBreakpoints();
	virtual int DbgGetBreakpoint(int Index);

	virtual bool DbgSetTracingMode(bool IsTracing);
	virtual bool DbgGetTracingMode();
};

} // end of namespace WinterMute

#endif
