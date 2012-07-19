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


#include "engines/wintermute/base/BNamedObject.h"
#include "engines/wintermute/wme_debugger.h"
#include "engines/wintermute/persistent.h"

namespace WinterMute {

class CScValue;
class CScStack;
class CScScript;

class CBScriptable : public CBNamedObject, public IWmeDebugObject {
public:
	virtual CScScript *invokeMethodThread(const char *methodName);
	DECLARE_PERSISTENT(CBScriptable, CBNamedObject)

	CBScriptable(CBGame *inGame, bool noValue = false, bool persistable = true);
	virtual ~CBScriptable();

	// high level scripting interface
	virtual bool canHandleMethod(const char *eventMethod);
	virtual bool scSetProperty(const char *name, CScValue *value);
	virtual CScValue *scGetProperty(const char *name);
	virtual bool scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();
	virtual void *scToMemBuffer();
	virtual int scToInt();
	virtual double scToFloat();
	virtual bool scToBool();
	virtual void scSetString(const char *val);
	virtual void scSetInt(int val);
	virtual void scSetFloat(double val);
	virtual void scSetBool(bool val);
	virtual int scCompare(CBScriptable *val);
	virtual void scDebuggerDesc(char *buf, int bufSize);
	int _refCount;
	CScValue *_scValue;
	CScValue *_scProp;

public:
	// IWmeDebugObject
	const char *dbgGetNativeClass();
	IWmeDebugProp *dbgGetProperty(const char *name);

};

// Implemented in their respective .cpp-files
CBScriptable *makeSXArray(CBGame *inGame, CScStack *stack);
CBScriptable *makeSXDate(CBGame *inGame, CScStack *stack);
CBScriptable *makeSXFile(CBGame *inGame, CScStack *stack);
CBScriptable *makeSXMath(CBGame *inGame);
CBScriptable *makeSXMemBuffer(CBGame *inGame, CScStack *stack);
CBScriptable *makeSXObject(CBGame *inGame, CScStack *stack);
CBScriptable *makeSXStore(CBGame *inGame);
CBScriptable *makeSXString(CBGame *inGame, CScStack *stack);

} // end of namespace WinterMute

#endif
