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
	virtual int DbgGetLine() = 0;
	virtual const char *DbgGetFilename() = 0;
	virtual TScriptState DbgGetState() = 0;

	virtual int DbgGetNumBreakpoints() = 0;
	virtual int DbgGetBreakpoint(int Index) = 0;

	virtual bool DbgSetTracingMode(bool IsTracing) = 0;
	virtual bool DbgGetTracingMode() = 0;
};

//////////////////////////////////////////////////////////////////////////
class IWmeDebugProp {
public:
	virtual EWmeDebuggerPropType DbgGetType() = 0;

	virtual ~IWmeDebugProp() {}
	// getters
	virtual int DbgGetValInt() = 0;
	virtual double DbgGetValFloat() = 0;
	virtual bool DbgGetValBool() = 0;
	virtual const char *DbgGetValString() = 0;
	virtual IWmeDebugObject *DbgGetValNative() = 0;

	// setters
	virtual bool DbgSetVal(int Value) = 0;
	virtual bool DbgSetVal(double Value) = 0;
	virtual bool DbgSetVal(bool Value) = 0;
	virtual bool DbgSetVal(const char *Value) = 0;
	virtual bool DbgSetVal() = 0;

	// properties
	virtual int DbgGetNumProperties() = 0;
	virtual bool DbgGetProperty(int Index, const char **Name, IWmeDebugProp **Value) = 0;

	virtual bool DbgGetDescription(char *Buf, int BufSize) = 0;
};

//////////////////////////////////////////////////////////////////////////
class IWmeDebugObject {
public:
	virtual ~IWmeDebugObject() {}
	virtual const char *DbgGetNativeClass() = 0;
	virtual IWmeDebugProp *DbgGetProperty(const char *Name) = 0;
};

//////////////////////////////////////////////////////////////////////////
class IWmeDebugClient {
public:
	virtual ~IWmeDebugClient() {}
	virtual bool OnGameInit() = 0;
	virtual bool OnGameShutdown() = 0;

	virtual bool OnGameTick() = 0;

	virtual bool OnLog(unsigned int ErrorCode, const char *Text) = 0;

	virtual bool OnScriptInit(IWmeDebugScript *Script) = 0;
	virtual bool OnScriptEventThreadInit(IWmeDebugScript *Script, IWmeDebugScript *ParentScript, const char *EventName) = 0;
	virtual bool OnScriptMethodThreadInit(IWmeDebugScript *Script, IWmeDebugScript *ParentScript, const char *MethodName) = 0;
	virtual bool OnScriptShutdown(IWmeDebugScript *Script) = 0;
	virtual bool OnScriptChangeLine(IWmeDebugScript *Script, int Line) = 0;
	virtual bool OnScriptChangeScope(IWmeDebugScript *Script, unsigned int ScopeID) = 0;
	virtual bool OnScriptShutdownScope(IWmeDebugScript *Script, unsigned int ScopeID) = 0;

	virtual bool OnVariableInit(EWmeDebuggerVariableType Type, IWmeDebugScript *Script, unsigned int ScopeID, IWmeDebugProp *Variable, const char *VariableName) = 0;
	virtual bool OnVariableChangeValue(IWmeDebugProp *Variable, IWmeDebugProp *Value) = 0;

	virtual bool OnScriptHitBreakpoint(IWmeDebugScript *Script, int Line) = 0;
};

//////////////////////////////////////////////////////////////////////////
class IWmeDebugServer {
public:
	virtual ~IWmeDebugServer() {}
	virtual bool AttachClient(IWmeDebugClient *Client) = 0;
	virtual bool DetachClient(IWmeDebugClient *Client) = 0;

	virtual bool QueryData(IWmeDebugClient *Client) = 0;

	virtual int GetPropInt(const char *PropName) = 0;
	virtual double GetPropFloat(const char *PropName) = 0;
	virtual const char *GetPropString(const char *PropName) = 0;
	virtual bool GetPropBool(const char *PropName) = 0;

	virtual bool SetProp(const char *PropName, int PropValue) = 0;
	virtual bool SetProp(const char *PropName, double PropValue) = 0;
	virtual bool SetProp(const char *PropName, const char *PropValue) = 0;
	virtual bool SetProp(const char *PropName, bool PropValue) = 0;

	virtual bool ResolveFilename(const char *RelativeFilename, char *AbsFilenameBuf, int AbsBufSize) = 0;

	virtual bool AddBreakpoint(const char *ScriptFilename, int Line) = 0;
	virtual bool RemoveBreakpoint(const char *ScriptFilename, int Line) = 0;

	virtual bool ContinueExecution() = 0;
};


typedef bool (*WMEDBG_INITIALIZE)(IWmeDebugServer *Server);
typedef bool (*WMEDBG_SHUTDOWN)(IWmeDebugServer *Server);

} // end of namespace WinterMute

#endif // WME_DEBUGGER_H