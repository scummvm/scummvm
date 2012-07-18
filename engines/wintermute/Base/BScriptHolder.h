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
	virtual CScScript *invokeMethodThread(const char *methodName);
	virtual void makeFreezable(bool freezable);
	bool canHandleEvent(const char *eventName);
	virtual bool canHandleMethod(const char *eventMethod);
	bool cleanup();
	bool removeScript(CScScript *script);
	bool addScript(const char *filename);
	virtual bool saveAsText(CBDynBuffer *buffer, int indent);
	virtual bool listen(CBScriptHolder *param1, uint32 param2);
	bool applyEvent(const char *eventName, bool unbreakable = false);
	void setFilename(const char *filename);
	bool parseProperty(byte *buffer, bool complete = true);
	char *_filename;
	bool _freezable;
	bool _ready;

	CBArray<CScScript *, CScScript *> _scripts;
	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual bool scSetProperty(const char *name, CScValue *value);
	virtual bool scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();
	virtual void scDebuggerDesc(char *buf, int bufSize);
	// IWmeObject
public:
	virtual bool sendEvent(const char *eventName);
};

} // end of namespace WinterMute

#endif
