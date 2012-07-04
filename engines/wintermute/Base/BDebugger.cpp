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

#include "engines/wintermute/Base/BDebugger.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBDebugger::CBDebugger(CBGame *inGame) : CBBase(inGame) {
	_enabled = false;
}

//////////////////////////////////////////////////////////////////////////
CBDebugger::~CBDebugger(void) {
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::Initialize() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::Shutdown() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onGameInit() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onGameShutdown() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onGameTick() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onLog(unsigned int errorCode, const char *text) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptInit(CScScript *script) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptEventThreadInit(CScScript *script, CScScript *parentScript, const char *name) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptMethodThreadInit(CScScript *script, CScScript *parentScript, const char *name) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptShutdown(CScScript *script) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptChangeLine(CScScript *Script, int Line) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptChangeScope(CScScript *Script, CScValue *Scope) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptShutdownScope(CScScript *Script, CScValue *Scope) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onVariableInit(EWmeDebuggerVariableType type, CScScript *script, CScValue *scope, CScValue *var, const char *variableName) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onVariableChangeValue(CScValue *Var, CScValue *value) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptHitBreakpoint(CScScript *script) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IWmeDebugServer interface implementation
bool CBDebugger::AttachClient(IWmeDebugClient *Client) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::DetachClient(IWmeDebugClient *Client) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBDebugger::QueryData(IWmeDebugClient *Client) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
int CBDebugger::GetPropInt(const char *PropName) {
	return 0;
}

//////////////////////////////////////////////////////////////////////////
double CBDebugger::GetPropFloat(const char *PropName) {
	return 0.0;
}

//////////////////////////////////////////////////////////////////////////
const char *CBDebugger::GetPropString(const char *PropName) {
	return "";
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::GetPropBool(const char *PropName) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::SetProp(const char *PropName, int PropValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::SetProp(const char *PropName, double PropValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::SetProp(const char *PropName, const char *PropValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::SetProp(const char *PropName, bool PropValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::ResolveFilename(const char *RelativeFilename, char *AbsFilenameBuf, int AbsBufSize) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::AddBreakpoint(const char *ScriptFilename, int Line) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::RemoveBreakpoint(const char *ScriptFilename, int Line) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::ContinueExecution() {
	return false;
}

} // end of namespace WinterMute
