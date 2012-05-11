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

#ifndef WINTERMUTE_BSCRIPTABLE_H
#define WINTERMUTE_BSCRIPTABLE_H


#include "engines/wintermute/BNamedObject.h"
#include "engines/wintermute/wme_debugger.h"
#include "engines/wintermute/persistent.h"

namespace WinterMute {

class CScValue;
class CScStack;
class CScScript;

class CBScriptable : public CBNamedObject, public IWmeDebugObject {
public:
	virtual CScScript *InvokeMethodThread(const char *MethodName);
	DECLARE_PERSISTENT(CBScriptable, CBNamedObject)

	CBScriptable(CBGame *inGame, bool NoValue = false, bool Persistable = true);
	virtual ~CBScriptable();

	// high level scripting interface
	virtual bool CanHandleMethod(char *EventMethod);
	virtual HRESULT ScSetProperty(const char *Name, CScValue *Value);
	virtual CScValue *ScGetProperty(const char *Name);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual char *ScToString();
	virtual void *ScToMemBuffer();
	virtual int ScToInt();
	virtual double ScToFloat();
	virtual bool ScToBool();
	virtual void ScSetString(const char *Val);
	virtual void ScSetInt(int Val);
	virtual void ScSetFloat(double Val);
	virtual void ScSetBool(bool Val);
	virtual int ScCompare(CBScriptable *Val);
	virtual void ScDebuggerDesc(char *Buf, int BufSize);
	int _refCount;
	CScValue *_scValue;
	CScValue *_scProp;

public:
	// IWmeDebugObject
	const char *DbgGetNativeClass();
	IWmeDebugProp *DbgGetProperty(const char *Name);

};

} // end of namespace WinterMute

#endif
