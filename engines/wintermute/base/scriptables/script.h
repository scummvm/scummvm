/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_SCSCRIPT_H
#define WINTERMUTE_SCSCRIPT_H


#include "engines/wintermute/base/base.h"
#include "engines/wintermute/base/scriptables/dcscript.h"   // Added by ClassView
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/persistent.h"

namespace Wintermute {
class BaseScriptHolder;
class BaseObject;
class ScEngine;
class ScStack;
class ScValue;

class ScScript : public BaseClass {
public:
	BaseArray<int> _breakpoints;
	bool _tracingMode;

	ScScript *_parentScript;
	bool _unbreakable;
	bool finishThreads();
	bool copyParameters(ScStack *stack);

	ScValue *_operand;
	ScValue *_reg1;
	bool _freezable;
	bool resume();
	bool pause();
	bool canHandleEvent(const char *eventName) const;
	bool canHandleMethod(const char *methodName) const;
	bool createThread(ScScript *original, uint32 initIP, const char *eventName);
	bool createMethodThread(ScScript *original, const char *methodName);
	ScScript *invokeEventHandler(const char *eventName, bool unbreakable = false);
	uint32 _timeSlice;
	DECLARE_PERSISTENT(ScScript, BaseClass)
	void afterLoad();
	void runtimeError(const char *fmt, ...);
	bool run();
	bool finish(bool includingThreads = false);
	bool sleep(uint32 duration);
	bool waitForExclusive(BaseObject *object);
	bool waitFor(BaseObject *object);
	uint32 _waitTime;
	bool _waitFrozen;
	BaseObject *_waitObject;
	ScScript *_waitScript;
	TScriptState _state;
	TScriptState _origState;
	ScValue *getVar(char *name);
	uint32 getFuncPos(const char *name);
	uint32 getEventPos(const char *name) const;
	uint32 getMethodPos(const char *name) const;
	typedef struct {
		uint32 magic;
		uint32 version;
		uint32 codeStart;
		uint32 funcTable;
		uint32 symbolTable;
		uint32 eventTable;
		uint32 externalsTable;
		uint32 methodTable;
	} TScriptHeader;

	TScriptHeader _header{};

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
		int32 numParams;
		TExternalType *params;
	} TExternalFunction;


	ScStack *_callStack;
	ScStack *_thisStack;
	ScStack *_scopeStack;
	ScStack *_stack;
	ScValue *_globals;
	ScEngine *_engine;
	int32 _currentLine;
	virtual bool executeInstruction();
	char *getString();
	uint32 getDWORD();
	double getFloat();
	void cleanup();
	bool create(const char *filename, byte *buffer, uint32 size, BaseScriptHolder *owner);
	uint32 _iP;
	void readHeader();
	uint32 _bufferSize;
	byte *_buffer;
	Common::SeekableReadStream *_scriptStream;
	ScScript(BaseGame *inGame, ScEngine *engine);
	~ScScript() override;
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
	BaseScriptHolder *_owner;
	ScScript::TExternalFunction *getExternal(char *name);
	bool externalCall(ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function);
private:

	bool initScript();
	bool initTables();

	virtual void preInstHook(uint32 inst);
	virtual void postInstHook(uint32 inst);

#ifdef ENABLE_FOXTAIL
	TOpcodesType _opcodesType;
	void initOpcodesType();
	uint32 decodeAltOpcodes(uint32 inst);
#endif

	bool _enableFloatCompareWA{};
};

} // End of namespace Wintermute

#endif
