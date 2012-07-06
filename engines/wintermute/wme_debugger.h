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

#ifndef WME_DEBUGGER_H
#define WME_DEBUGGER_H

#ifdef SetProp
#undef SetProp
#endif

#include "engines/wintermute/dcscript.h"

namespace WinterMute {

class IWmeDebugClient;
class IWmeDebugObject;

//////////////////////////////////////////////////////////////////////////
typedef enum {
    WME_DBGPROP_NULL = 0,
    WME_DBGPROP_INT = 1,
    WME_DBGPROP_FLOAT = 2,
    WME_DBGPROP_BOOL = 3,
    WME_DBGPROP_STRING = 4,
    WME_DBGPROP_OBJECT = 5,
    WME_DBGPROP_NATIVE = 6,
    WME_DBGPROP_UNKNOWN = 7
}
EWmeDebuggerPropType;

//////////////////////////////////////////////////////////////////////////
typedef enum {
    WME_DBGVAR_GLOBAL = 0,
    WME_DBGVAR_SCRIPT = 1,
    WME_DBGVAR_SCOPE = 2
}
EWmeDebuggerVariableType;

//////////////////////////////////////////////////////////////////////////
class IWmeDebugScript {
public:
	virtual ~IWmeDebugScript() {}
	virtual int dbgGetLine() = 0;
	virtual const char *dbgGetFilename() = 0;
	virtual TScriptState dbgGetState() = 0;

	virtual int dbgGetNumBreakpoints() = 0;
	virtual int dbgGetBreakpoint(int index) = 0;

	virtual bool dbgSetTracingMode(bool isTracing) = 0;
	virtual bool dbgGetTracingMode() = 0;
};

//////////////////////////////////////////////////////////////////////////
class IWmeDebugProp {
public:
	virtual EWmeDebuggerPropType dbgGetType() = 0;

	virtual ~IWmeDebugProp() {}
	// getters
	virtual int dbgGetValInt() = 0;
	virtual double dbgGetValFloat() = 0;
	virtual bool dbgGetValBool() = 0;
	virtual const char *dbgGetValString() = 0;
	virtual IWmeDebugObject *dbgGetValNative() = 0;

	// setters
	virtual bool dbgSetVal(int value) = 0;
	virtual bool dbgSetVal(double value) = 0;
	virtual bool dbgSetVal(bool value) = 0;
	virtual bool dbgSetVal(const char *value) = 0;
	virtual bool dbgSetVal() = 0;

	// properties
	virtual int dbgGetNumProperties() = 0;
	virtual bool dbgGetProperty(int index, const char **name, IWmeDebugProp **value) = 0;

	virtual bool dbgGetDescription(char *buf, int bufSize) = 0;
};

//////////////////////////////////////////////////////////////////////////
class IWmeDebugObject {
public:
	virtual ~IWmeDebugObject() {}
	virtual const char *dbgGetNativeClass() = 0;
	virtual IWmeDebugProp *dbgGetProperty(const char *name) = 0;
};

//////////////////////////////////////////////////////////////////////////
class IWmeDebugClient {
public:
	virtual ~IWmeDebugClient() {}
	virtual bool onGameInit() = 0;
	virtual bool onGameShutdown() = 0;

	virtual bool onGameTick() = 0;

	virtual bool onLog(unsigned int errorCode, const char *text) = 0;

	virtual bool onScriptInit(IWmeDebugScript *script) = 0;
	virtual bool onScriptEventThreadInit(IWmeDebugScript *script, IWmeDebugScript *ParentScript, const char *EventName) = 0;
	virtual bool onScriptMethodThreadInit(IWmeDebugScript *script, IWmeDebugScript *ParentScript, const char *MethodName) = 0;
	virtual bool onScriptShutdown(IWmeDebugScript *script) = 0;
	virtual bool onScriptChangeLine(IWmeDebugScript *script, int Line) = 0;
	virtual bool onScriptChangeScope(IWmeDebugScript *script, unsigned int scopeID) = 0;
	virtual bool onScriptShutdownScope(IWmeDebugScript *script, unsigned int scopeID) = 0;

	virtual bool onVariableInit(EWmeDebuggerVariableType Type, IWmeDebugScript *script, unsigned int scopeID, IWmeDebugProp *variable, const char *variableName) = 0;
	virtual bool onVariableChangeValue(IWmeDebugProp *variable, IWmeDebugProp *value) = 0;

	virtual bool onScriptHitBreakpoint(IWmeDebugScript *script, int line) = 0;
};

//////////////////////////////////////////////////////////////////////////
class IWmeDebugServer {
public:
	virtual ~IWmeDebugServer() {}
	virtual bool attachClient(IWmeDebugClient *client) = 0;
	virtual bool detachClient(IWmeDebugClient *client) = 0;

	virtual bool queryData(IWmeDebugClient *client) = 0;

	virtual int getPropInt(const char *propName) = 0;
	virtual double getPropFloat(const char *propName) = 0;
	virtual const char *getPropString(const char *propName) = 0;
	virtual bool getPropBool(const char *propName) = 0;

	virtual bool setProp(const char *propName, int propValue) = 0;
	virtual bool setProp(const char *propName, double propValue) = 0;
	virtual bool setProp(const char *propName, const char *propValue) = 0;
	virtual bool setProp(const char *propName, bool propValue) = 0;

	virtual bool resolveFilename(const char *relativeFilename, char *absFilenameBuf, int absBufSize) = 0;

	virtual bool addBreakpoint(const char *scriptFilename, int line) = 0;
	virtual bool removeBreakpoint(const char *scriptFilename, int line) = 0;

	virtual bool continueExecution() = 0;
};


typedef bool (*WMEDBG_INITIALIZE)(IWmeDebugServer *Server);
typedef bool (*WMEDBG_SHUTDOWN)(IWmeDebugServer *Server);

} // end of namespace WinterMute

#endif // WME_DEBUGGER_H
