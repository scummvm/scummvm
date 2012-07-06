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

#ifndef WINTERMUTE_SCVALUE_H
#define WINTERMUTE_SCVALUE_H


#include "engines/wintermute/Base/BBase.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/dcscript.h"   // Added by ClassView
#include "engines/wintermute/wme_debugger.h"
#include "common/str.h"

namespace WinterMute {

class CScScript;
class CBScriptable;

class CScValue : public CBBase, public IWmeDebugProp {
public:
	HRESULT DbgSendVariables(IWmeDebugClient *client, EWmeDebuggerVariableType type, CScScript *script, unsigned int scopeID);

	static int compare(CScValue *Val1, CScValue *Val2);
	static int compareStrict(CScValue *Val1, CScValue *Val2);
	TValType getTypeTolerant();
	void cleanup(bool IgnoreNatives = false);
	DECLARE_PERSISTENT(CScValue, CBBase)

	bool _isConstVar;
	HRESULT saveAsText(CBDynBuffer *buffer, int indent);
	void setValue(CScValue *Val);
	bool _persistent;
	bool propExists(const char *name);
	void copy(CScValue *orig, bool copyWhole = false);
	void setStringVal(const char *val);
	TValType getType();
	bool getBool(bool Default = false);
	int getInt(int Default = 0);
	double getFloat(double Default = 0.0f);
	const char *getString();
	void *getMemBuffer();
	CBScriptable *getNative();
	HRESULT deleteProp(const char *name);
	void deleteProps();
	void CleanProps(bool includingNatives);
	void setBool(bool val);
	void setInt(int val);
	void setFloat(double val);
	void setString(const char *val);
	void setString(const Common::String &val);
	void setNULL();
	void setNative(CBScriptable *val, bool persistent = false);
	void setObject();
	void setReference(CScValue *val);
	bool isNULL();
	bool isNative();
	bool isString();
	bool isBool();
	bool isFloat();
	bool isInt();
	bool isObject();
	HRESULT setProp(const char *name, CScValue *val, bool copyWhole = false, bool setAsConst = false);
	CScValue *getProp(const char *name);
	CBScriptable *_valNative;
	CScValue *_valRef;
protected:
	bool _valBool;
	int _valInt;
	double _valFloat;
	char *_valString;
public:
	TValType _type;
	CScValue(CBGame *inGame);
	CScValue(CBGame *inGame, bool Val);
	CScValue(CBGame *inGame, int Val);
	CScValue(CBGame *inGame, double Val);
	CScValue(CBGame *inGame, const char *Val);
	virtual ~CScValue();
	Common::HashMap<Common::String, CScValue *> _valObject;
	Common::HashMap<Common::String, CScValue *>::iterator _valIter;

	bool setProperty(const char *PropName, int Value);
	bool setProperty(const char *PropName, const char *Value);
	bool setProperty(const char *PropName, double Value);
	bool setProperty(const char *PropName, bool Value);
	bool setProperty(const char *PropName);


// IWmeDebugProp interface implementation
public:
	virtual EWmeDebuggerPropType dbgGetType();

	// getters
	virtual int dbgGetValInt();
	virtual double dbgGetValFloat();
	virtual bool dbgGetValBool();
	virtual const char *dbgGetValString();
	virtual IWmeDebugObject *dbgGetValNative();

	// setters
	virtual bool dbgSetVal(int Value);
	virtual bool dbgSetVal(double Value);
	virtual bool dbgSetVal(bool Value);
	virtual bool dbgSetVal(const char *Value);
	virtual bool dbgSetVal();

	// properties
	virtual int dbgGetNumProperties();
	virtual bool dbgGetProperty(int Index, const char **Name, IWmeDebugProp **Value);

	virtual bool dbgGetDescription(char *Buf, int BufSize);
};

} // end of namespace WinterMute

#endif
