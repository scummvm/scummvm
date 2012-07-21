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

#include "engines/wintermute/base/base_debugger.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
BaseDebugger::BaseDebugger(BaseGame *inGame) : BaseClass(inGame) {
	_enabled = false;
}

//////////////////////////////////////////////////////////////////////////
BaseDebugger::~BaseDebugger(void) {
}


//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::initialize() {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::shutdown() {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onGameInit() {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onGameShutdown() {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onGameTick() {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onLog(unsigned int errorCode, const char *text) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onScriptInit(ScScript *script) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onScriptEventThreadInit(ScScript *script, ScScript *parentScript, const char *name) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onScriptMethodThreadInit(ScScript *script, ScScript *parentScript, const char *name) {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onScriptShutdown(ScScript *script) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onScriptChangeLine(ScScript *script, int Line) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onScriptChangeScope(ScScript *script, ScValue *scope) {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onScriptShutdownScope(ScScript *script, ScValue *scope) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onVariableInit(EWmeDebuggerVariableType type, ScScript *script, ScValue *scope, ScValue *var, const char *variableName) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onVariableChangeValue(ScValue *var, ScValue *value) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::onScriptHitBreakpoint(ScScript *script) {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
// IWmeDebugServer interface implementation
bool BaseDebugger::attachClient(IWmeDebugClient *client) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::detachClient(IWmeDebugClient *client) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::queryData(IWmeDebugClient *client) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
int BaseDebugger::getPropInt(const char *propName) {
	return 0;
}

//////////////////////////////////////////////////////////////////////////
double BaseDebugger::getPropFloat(const char *propName) {
	return 0.0;
}

//////////////////////////////////////////////////////////////////////////
const char *BaseDebugger::getPropString(const char *propName) {
	return "";
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::getPropBool(const char *propName) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::setProp(const char *propName, int propValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::setProp(const char *propName, double propValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::setProp(const char *propName, const char *propValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::setProp(const char *propName, bool propValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::resolveFilename(const char *relativeFilename, char *absFilenameBuf, int absBufSize) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::addBreakpoint(const char *scriptFilename, int line) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::removeBreakpoint(const char *scriptFilename, int line) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseDebugger::continueExecution() {
	return false;
}

} // end of namespace WinterMute
