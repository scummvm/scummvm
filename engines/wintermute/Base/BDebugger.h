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

#ifndef WINTERMUTE_BDEBUGGER_H
#define WINTERMUTE_BDEBUGGER_H


#include "engines/wintermute/Base/BBase.h"
#include "engines/wintermute/wme_debugger.h"

// TODO: The entire debugger should possibly be removed

namespace WinterMute {
class CScScript;
class CScValue;
class CBDebugger : public CBBase, public IWmeDebugServer {
public:
	CBDebugger(CBGame *inGame);
	virtual ~CBDebugger(void);

	// initialization
	bool _enabled;
	bool initialize();
	bool shutdown();

	// internal interface
	bool onGameInit();
	bool onGameShutdown();
	bool onGameTick();
	bool onLog(unsigned int errorCode, const char *text);
	bool onScriptInit(CScScript *script);
	bool onScriptEventThreadInit(CScScript *script, CScScript *parentScript, const char *name);
	bool onScriptMethodThreadInit(CScScript *script, CScScript *parentScript, const char *name);

	bool onScriptShutdown(CScScript *script);
	bool onScriptChangeLine(CScScript *script, int line);
	bool onScriptChangeScope(CScScript *script, CScValue *scope);
	bool onScriptShutdownScope(CScScript *script, CScValue *scope);
	bool onVariableInit(EWmeDebuggerVariableType type, CScScript *script, CScValue *scope, CScValue *var, const char *variableName);
	bool onVariableChangeValue(CScValue *var, CScValue *value);

	bool onScriptHitBreakpoint(CScScript *script);

	// IWmeDebugServer interface
	virtual bool attachClient(IWmeDebugClient *client);
	virtual bool detachClient(IWmeDebugClient *client);
	virtual bool queryData(IWmeDebugClient *client);

	virtual int getPropInt(const char *propName);
	virtual double getPropFloat(const char *propName);
	virtual const char *getPropString(const char *propName);
	virtual bool getPropBool(const char *propName);

	virtual bool setProp(const char *propName, int propValue);
	virtual bool setProp(const char *propName, double propValue);
	virtual bool setProp(const char *propName, const char *propValue);
	virtual bool setProp(const char *propName, bool propValue);

	virtual bool resolveFilename(const char *relativeFilename, char *absFilenameBuf, int absBufSize);

	virtual bool addBreakpoint(const char *scriptFilename, int line);
	virtual bool removeBreakpoint(const char *scriptFilename, int line);

	virtual bool continueExecution();
private:
};

} // end of namespace WinterMute

#endif
