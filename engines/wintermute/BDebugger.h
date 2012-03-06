/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __WmeBDebugger_H__
#define __WmeBDebugger_H__


#include "BBase.h"
#include "wme_debugger.h"

namespace WinterMute {
class CScScript;
class CScValue;
class CBDebugger : public CBBase, public IWmeDebugServer {
public:
	CBDebugger(CBGame *inGame);
	virtual ~CBDebugger(void);

	// initialization
	bool m_Enabled;
	HRESULT Initialize();
	HRESULT Shutdown();

	// internal interface
	HRESULT OnGameInit();
	HRESULT OnGameShutdown();
	HRESULT OnGameTick();
	HRESULT OnLog(unsigned int ErrorCode, const char *Text);
	HRESULT OnScriptInit(CScScript *Script);
	HRESULT OnScriptEventThreadInit(CScScript *Script, CScScript *ParentScript, const char *Name);
	HRESULT OnScriptMethodThreadInit(CScScript *Script, CScScript *ParentScript, const char *Name);

	HRESULT OnScriptShutdown(CScScript *Script);
	HRESULT OnScriptChangeLine(CScScript *Script, int Line);
	HRESULT OnScriptChangeScope(CScScript *Script, CScValue *Scope);
	HRESULT OnScriptShutdownScope(CScScript *Script, CScValue *Scope);
	HRESULT OnVariableInit(EWmeDebuggerVariableType Type, CScScript *Script, CScValue *Scope, CScValue *Var, const char *VariableName);
	HRESULT OnVariableChangeValue(CScValue *Var, CScValue *Value);

	HRESULT OnScriptHitBreakpoint(CScScript *Script);

	// IWmeDebugServer interface
	virtual bool AttachClient(IWmeDebugClient *Client);
	virtual bool DetachClient(IWmeDebugClient *Client);
	virtual bool QueryData(IWmeDebugClient *Client);

	virtual int GetPropInt(const char *PropName);
	virtual double GetPropFloat(const char *PropName);
	virtual const char *GetPropString(const char *PropName);
	virtual bool GetPropBool(const char *PropName);

	virtual bool SetProp(const char *PropName, int PropValue);
	virtual bool SetProp(const char *PropName, double PropValue);
	virtual bool SetProp(const char *PropName, const char *PropValue);
	virtual bool SetProp(const char *PropName, bool PropValue);

	virtual bool ResolveFilename(const char *RelativeFilename, char *AbsFilenameBuf, int AbsBufSize);

	virtual bool AddBreakpoint(const char *ScriptFilename, int Line);
	virtual bool RemoveBreakpoint(const char *ScriptFilename, int Line);

	virtual bool ContinueExecution();
private:
};

} // end of namespace WinterMute

#endif
