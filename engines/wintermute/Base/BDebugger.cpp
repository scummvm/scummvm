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
HRESULT CBDebugger::initialize() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::shutdown() {
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
HRESULT CBDebugger::onScriptChangeLine(CScScript *script, int Line) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptChangeScope(CScScript *script, CScValue *scope) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptShutdownScope(CScScript *script, CScValue *scope) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onVariableInit(EWmeDebuggerVariableType type, CScScript *script, CScValue *scope, CScValue *var, const char *variableName) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onVariableChangeValue(CScValue *var, CScValue *value) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::onScriptHitBreakpoint(CScScript *script) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IWmeDebugServer interface implementation
bool CBDebugger::attachClient(IWmeDebugClient *client) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::detachClient(IWmeDebugClient *client) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBDebugger::queryData(IWmeDebugClient *client) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
int CBDebugger::getPropInt(const char *propName) {
	return 0;
}

//////////////////////////////////////////////////////////////////////////
double CBDebugger::getPropFloat(const char *propName) {
	return 0.0;
}

//////////////////////////////////////////////////////////////////////////
const char *CBDebugger::getPropString(const char *propName) {
	return "";
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::getPropBool(const char *propName) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::setProp(const char *propName, int propValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::setProp(const char *propName, double propValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::setProp(const char *propName, const char *propValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::setProp(const char *propName, bool propValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::resolveFilename(const char *relativeFilename, char *absFilenameBuf, int absBufSize) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::addBreakpoint(const char *scriptFilename, int line) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::removeBreakpoint(const char *scriptFilename, int line) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::continueExecution() {
	return false;
}

} // end of namespace WinterMute
