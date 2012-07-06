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
#include "engines/wintermute/Base/BScriptable.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/BPersistMgr.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBScriptable, false)

//////////////////////////////////////////////////////////////////////////
CBScriptable::CBScriptable(CBGame *inGame, bool noValue, bool persistable): CBNamedObject(inGame) {
	_refCount = 0;

	if (noValue) _scValue = NULL;
	else _scValue = new CScValue(Game);

	_persistable = persistable;

	_scProp = NULL;
}


//////////////////////////////////////////////////////////////////////////
CBScriptable::~CBScriptable() {
	//if(_refCount>0) Game->LOG(0, "Warning: Destroying object, _refCount=%d", _refCount);
	delete _scValue;
	delete _scProp;
	_scValue = NULL;
	_scProp = NULL;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptable::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	/*
	stack->correctParams(0);
	stack->pushNULL();
	script->RuntimeError("Call to undefined method '%s'.", name);

	return S_OK;
	*/
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBScriptable::scGetProperty(const char *name) {
	if (!_scProp) _scProp = new CScValue(Game);
	if (_scProp) return _scProp->getProp(name);
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptable::scSetProperty(const char *name, CScValue *value) {
	if (!_scProp) _scProp = new CScValue(Game);
	if (_scProp) return _scProp->setProp(name, value);
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
const char *CBScriptable::scToString() {
	return "[native object]";
}

//////////////////////////////////////////////////////////////////////////
void *CBScriptable::scToMemBuffer() {
	return (void *)NULL;
}


//////////////////////////////////////////////////////////////////////////
int CBScriptable::scToInt() {
	return 0;
}


//////////////////////////////////////////////////////////////////////////
double CBScriptable::scToFloat() {
	return 0.0f;
}


//////////////////////////////////////////////////////////////////////////
bool CBScriptable::scToBool() {
	return false;
}


//////////////////////////////////////////////////////////////////////////
void CBScriptable::scSetString(const char *val) {
}


//////////////////////////////////////////////////////////////////////////
void CBScriptable::scSetInt(int val) {
}


//////////////////////////////////////////////////////////////////////////
void CBScriptable::scSetFloat(double val) {
}


//////////////////////////////////////////////////////////////////////////
void CBScriptable::scSetBool(bool val) {
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptable::persist(CBPersistMgr *persistMgr) {
	persistMgr->transfer(TMEMBER(Game));
	persistMgr->transfer(TMEMBER(_refCount));
	persistMgr->transfer(TMEMBER(_scProp));
	persistMgr->transfer(TMEMBER(_scValue));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CBScriptable::scCompare(CBScriptable *Val) {
	if (this < Val) return -1;
	else if (this > Val) return 1;
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
void CBScriptable::scDebuggerDesc(char *buf, int bufSize) {
	strcpy(buf, scToString());
}

//////////////////////////////////////////////////////////////////////////
bool CBScriptable::canHandleMethod(const char *eventMethod) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
CScScript *CBScriptable::invokeMethodThread(const char *methodName) {
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// IWmeDebugObject
//////////////////////////////////////////////////////////////////////////
const char *CBScriptable::dbgGetNativeClass() {
	return getClassName();
}

//////////////////////////////////////////////////////////////////////////
IWmeDebugProp *CBScriptable::dbgGetProperty(const char *name) {
	return scGetProperty(name);
}

} // end of namespace WinterMute
