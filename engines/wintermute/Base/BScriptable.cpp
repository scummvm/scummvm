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
CBScriptable::CBScriptable(CBGame *inGame, bool NoValue, bool Persistable): CBNamedObject(inGame) {
	_refCount = 0;

	if (NoValue) _scValue = NULL;
	else _scValue = new CScValue(Game);

	_persistable = Persistable;

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
HRESULT CBScriptable::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	/*
	Stack->CorrectParams(0);
	Stack->PushNULL();
	Script->RuntimeError("Call to undefined method '%s'.", Name);

	return S_OK;
	*/
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBScriptable::ScGetProperty(const char *Name) {
	if (!_scProp) _scProp = new CScValue(Game);
	if (_scProp) return _scProp->GetProp(Name);
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptable::ScSetProperty(const char *Name, CScValue *Value) {
	if (!_scProp) _scProp = new CScValue(Game);
	if (_scProp) return _scProp->SetProp(Name, Value);
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
const char *CBScriptable::ScToString() {
	return "[native object]";
}

//////////////////////////////////////////////////////////////////////////
void *CBScriptable::ScToMemBuffer() {
	return (void *)NULL;
}


//////////////////////////////////////////////////////////////////////////
int CBScriptable::ScToInt() {
	return 0;
}


//////////////////////////////////////////////////////////////////////////
double CBScriptable::ScToFloat() {
	return 0.0f;
}


//////////////////////////////////////////////////////////////////////////
bool CBScriptable::ScToBool() {
	return false;
}


//////////////////////////////////////////////////////////////////////////
void CBScriptable::ScSetString(const char *Val) {
}


//////////////////////////////////////////////////////////////////////////
void CBScriptable::ScSetInt(int Val) {
}


//////////////////////////////////////////////////////////////////////////
void CBScriptable::ScSetFloat(double Val) {
}


//////////////////////////////////////////////////////////////////////////
void CBScriptable::ScSetBool(bool Val) {
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
int CBScriptable::ScCompare(CBScriptable *Val) {
	if (this < Val) return -1;
	else if (this > Val) return 1;
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
void CBScriptable::ScDebuggerDesc(char *Buf, int BufSize) {
	strcpy(Buf, ScToString());
}

//////////////////////////////////////////////////////////////////////////
bool CBScriptable::CanHandleMethod(const char *EventMethod) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
CScScript *CBScriptable::InvokeMethodThread(const char *MethodName) {
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// IWmeDebugObject
//////////////////////////////////////////////////////////////////////////
const char *CBScriptable::DbgGetNativeClass() {
	return GetClassName();
}

//////////////////////////////////////////////////////////////////////////
IWmeDebugProp *CBScriptable::DbgGetProperty(const char *Name) {
	return ScGetProperty((char *)Name);
}

} // end of namespace WinterMute
