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

#ifndef WINTERMUTE_BSCRIPTHOLDER_H
#define WINTERMUTE_BSCRIPTHOLDER_H

#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/Base/BScriptable.h"

namespace WinterMute {

class CBScriptHolder : public CBScriptable {
public:
	DECLARE_PERSISTENT(CBScriptHolder, CBScriptable)

	CBScriptHolder(CBGame *inGame);
	virtual ~CBScriptHolder();
	virtual CScScript *invokeMethodThread(const char *MethodName);
	virtual void MakeFreezable(bool Freezable);
	bool CanHandleEvent(const char *EventName);
	virtual bool canHandleMethod(const char *EventMethod);
	HRESULT Cleanup();
	HRESULT RemoveScript(CScScript *Script);
	HRESULT AddScript(const char *Filename);
	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent);
	virtual HRESULT Listen(CBScriptHolder *param1, uint32 param2);
	HRESULT ApplyEvent(const char *EventName, bool Unbreakable = false);
	void SetFilename(const char *Filename);
	HRESULT ParseProperty(byte  *Buffer, bool Complete = true);
	char *_filename;
	bool _freezable;
	bool _ready;

	CBArray<CScScript *, CScScript *> _scripts;
	// scripting interface
	virtual CScValue *scGetProperty(const char *Name);
	virtual HRESULT scSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT scCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual const char *scToString();
	virtual void scDebuggerDesc(char *Buf, int BufSize);
	// IWmeObject
public:
	virtual bool SendEvent(const char *EventName);
};

} // end of namespace WinterMute

#endif
