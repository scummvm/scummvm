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

#ifndef WINTERMUTE_BDEBUGGER_H
#define WINTERMUTE_BDEBUGGER_H


#include "engines/wintermute/Base/BBase.h"
#include "engines/wintermute/wme_debugger.h"

namespace WinterMute {
class CScScript;
class CScValue;
class CBDebugger : public CBBase, public IWmeDebugServer {
public:
	CBDebugger(CBGame *inGame);
	virtual ~CBDebugger(void);

	// initialization
	bool _enabled;
	HRESULT Initialize();
	HRESULT Shutdown();

	// internal interface
	HRESULT OnGameInit();
	HRESULT OnGameShutdown();
	HRESULT OnGameTick();
	HRESULT OnLog(unsigned int ErrorCode, const char *Text);
	HRESULT OnScriptInit(CScScript *script);
	HRESULT OnScriptEventThreadInit(CScScript *Script, CScScript *ParentScript, const char *name);
	HRESULT OnScriptMethodThreadInit(CScScript *Script, CScScript *ParentScript, const char *name);

	HRESULT OnScriptShutdown(CScScript *script);
	HRESULT OnScriptChangeLine(CScScript *Script, int Line);
	HRESULT OnScriptChangeScope(CScScript *Script, CScValue *Scope);
	HRESULT OnScriptShutdownScope(CScScript *Script, CScValue *Scope);
	HRESULT OnVariableInit(EWmeDebuggerVariableType Type, CScScript *script, CScValue *Scope, CScValue *Var, const char *VariableName);
	HRESULT OnVariableChangeValue(CScValue *Var, CScValue *value);

	HRESULT OnScriptHitBreakpoint(CScScript *script);

	// IWmeDebugServer interface
	virtual bool AttachClient(IWmeDebugClient *Client);
	virtual bool DetachClient(IWmeDebugClient *Client);
	virtual bool QueryData(IWmeDebugClient *Client);

	virtual int GetPropInt(const char *PropName);
	virtual double GetPropFloat(const char *PropName);
	virtual const char *GetPropString(const char *PropName);
	virtual bool GetPropBool(const char *PropName);

	virtual bool SetProp(const char *PropName, int PropValue);
	virtual bool SetProp(const char *PropName, double PropValue);
	virtual bool SetProp(const char *PropName, const char *PropValue);
	virtual bool SetProp(const char *PropName, bool PropValue);

	virtual bool ResolveFilename(const char *RelativeFilename, char *AbsFilenameBuf, int AbsBufSize);

	virtual bool AddBreakpoint(const char *ScriptFilename, int Line);
	virtual bool RemoveBreakpoint(const char *ScriptFilename, int Line);

	virtual bool ContinueExecution();
private:
};

} // end of namespace WinterMute

#endif
