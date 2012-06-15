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
	HRESULT DbgSendVariables(IWmeDebugClient *Client, EWmeDebuggerVariableType Type, CScScript *Script, unsigned int ScopeID);

	static int Compare(CScValue *Val1, CScValue *Val2);
	static int CompareStrict(CScValue *Val1, CScValue *Val2);
	TValType GetTypeTolerant();
	void Cleanup(bool IgnoreNatives = false);
	DECLARE_PERSISTENT(CScValue, CBBase)

	bool _isConstVar;
	HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent);
	void SetValue(CScValue *Val);
	bool _persistent;
	bool PropExists(const char *Name);
	void Copy(CScValue *orig, bool CopyWhole = false);
	void SetStringVal(const char *Val);
	TValType GetType();
	bool GetBool(bool Default = false);
	int GetInt(int Default = 0);
	double GetFloat(double Default = 0.0f);
	const char *GetString();
	void *GetMemBuffer();
	CBScriptable *GetNative();
	HRESULT DeleteProp(const char *Name);
	void DeleteProps();
	void CleanProps(bool IncludingNatives);
	void SetBool(bool Val);
	void SetInt(int Val);
	void SetFloat(double Val);
	void SetString(const char *Val);
	void SetString(const Common::String &Val);
	void SetNULL();
	void SetNative(CBScriptable *Val, bool Persistent = false);
	void SetObject();
	void SetReference(CScValue *Val);
	bool IsNULL();
	bool IsNative();
	bool IsString();
	bool IsBool();
	bool IsFloat();
	bool IsInt();
	bool IsObject();
	HRESULT SetProp(const char *Name, CScValue *Val, bool CopyWhole = false, bool SetAsConst = false);
	CScValue *GetProp(const char *Name);
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

	bool SetProperty(const char *PropName, int Value);
	bool SetProperty(const char *PropName, const char *Value);
	bool SetProperty(const char *PropName, double Value);
	bool SetProperty(const char *PropName, bool Value);
	bool SetProperty(const char *PropName);


// IWmeDebugProp interface implementation
public:
	virtual EWmeDebuggerPropType DbgGetType();

	// getters
	virtual int DbgGetValInt();
	virtual double DbgGetValFloat();
	virtual bool DbgGetValBool();
	virtual const char *DbgGetValString();
	virtual IWmeDebugObject *DbgGetValNative();

	// setters
	virtual bool DbgSetVal(int Value);
	virtual bool DbgSetVal(double Value);
	virtual bool DbgSetVal(bool Value);
	virtual bool DbgSetVal(const char *Value);
	virtual bool DbgSetVal();

	// properties
	virtual int DbgGetNumProperties();
	virtual bool DbgGetProperty(int Index, const char **Name, IWmeDebugProp **Value);

	virtual bool DbgGetDescription(char *Buf, int BufSize);
};

} // end of namespace WinterMute

#endif
