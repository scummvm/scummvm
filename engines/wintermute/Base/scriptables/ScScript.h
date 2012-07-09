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
	ERRORCODE dbgSendScript(IWmeDebugClient *client);
	ERRORCODE dbgSendVariables(IWmeDebugClient *client);

	CBArray<int, int> _breakpoints;
	bool _tracingMode;

	CScScript *_parentScript;
	bool _unbreakable;
	ERRORCODE finishThreads();
	ERRORCODE copyParameters(CScStack *stack);

	void afterLoad();

#ifdef __WIN32__
	static uint32 Call_cdecl(const void *args, size_t sz, uint32 func, bool *StackCorrupt);
	static uint32 Call_stdcall(const void *args, size_t sz, uint32 func, bool *StackCorrupt);
	static uint32 GetST0(void);
	static double GetST0Double(void);
#endif

	CScValue *_operand;
	CScValue *_reg1;
	bool _freezable;
	ERRORCODE resume();
	ERRORCODE pause();
	bool canHandleEvent(const char *eventName);
	bool canHandleMethod(const char *methodName);
	ERRORCODE createThread(CScScript *original, uint32 initIP, const char *eventName);
	ERRORCODE createMethodThread(CScScript *original, const char *methodName);
	CScScript *invokeEventHandler(const char *eventName, bool unbreakable = false);
	uint32 _timeSlice;
	DECLARE_PERSISTENT(CScScript, CBBase)
	void runtimeError(LPCSTR fmt, ...);
	ERRORCODE run();
	ERRORCODE finish(bool includingThreads = false);
	ERRORCODE sleep(uint32 duration);
	ERRORCODE waitForExclusive(CBObject *object);
	ERRORCODE waitFor(CBObject *object);
	uint32 _waitTime;
	bool _waitFrozen;
	CBObject *_waitObject;
	CScScript *_waitScript;
	TScriptState _state;
	TScriptState _origState;
	CScValue *getVar(char *name);
	uint32 getFuncPos(const char *name);
	uint32 getEventPos(const char *name);
	uint32 getMethodPos(const char *name);
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

	TScriptHeader _header;

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
	ERRORCODE executeInstruction();
	char *getString();
	uint32 getDWORD();
	double getFloat();
	void cleanup();
	ERRORCODE create(const char *filename, byte *buffer, uint32 size, CBScriptHolder *owner);
	uint32 _iP;
private:
	void readHeader();
	uint32 _bufferSize;
	byte *_buffer;
public:
	Common::SeekableReadStream *_scriptStream;
	CScScript(CBGame *inGame, CScEngine *Engine);
	virtual ~CScScript();
	char *_filename;
	char **_symbols;
	uint32 _numSymbols;
	TFunctionPos *_functions;
	TMethodPos *_methods;
	TEventPos *_events;
	uint32 _numExternals;
	TExternalFunction *_externals;
	uint32 _numFunctions;
	uint32 _numMethods;
	uint32 _numEvents;
	bool _thread;
	bool _methodThread;
	char *_threadEvent;
	CBScriptHolder *_owner;
	CScScript::TExternalFunction *getExternal(char *name);
	ERRORCODE externalCall(CScStack *stack, CScStack *thisStack, CScScript::TExternalFunction *function);
private:
	ERRORCODE initScript();
	ERRORCODE initTables();


// IWmeDebugScript interface implementation
public:
	virtual int dbgGetLine();
	virtual const char *dbgGetFilename();
	virtual TScriptState dbgGetState();
	virtual int dbgGetNumBreakpoints();
	virtual int dbgGetBreakpoint(int Index);

	virtual bool dbgSetTracingMode(bool IsTracing);
	virtual bool dbgGetTracingMode();
};

} // end of namespace WinterMute

#endif
