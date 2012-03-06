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

#include "BDebugger.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBDebugger::CBDebugger(CBGame *inGame) : CBBase(inGame) {
	m_Enabled = false;
}

//////////////////////////////////////////////////////////////////////////
CBDebugger::~CBDebugger(void) {
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::Initialize() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::Shutdown() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnGameInit() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnGameShutdown() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnGameTick() {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnLog(unsigned int ErrorCode, const char *Text) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnScriptInit(CScScript *Script) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnScriptEventThreadInit(CScScript *Script, CScScript *ParentScript, const char *Name) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnScriptMethodThreadInit(CScScript *Script, CScScript *ParentScript, const char *Name) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnScriptShutdown(CScScript *Script) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnScriptChangeLine(CScScript *Script, int Line) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnScriptChangeScope(CScScript *Script, CScValue *Scope) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnScriptShutdownScope(CScScript *Script, CScValue *Scope) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnVariableInit(EWmeDebuggerVariableType Type, CScScript *Script, CScValue *Scope, CScValue *Var, const char *VariableName) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnVariableChangeValue(CScValue *Var, CScValue *Value) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBDebugger::OnScriptHitBreakpoint(CScScript *Script) {
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IWmeDebugServer interface implementation
bool CBDebugger::AttachClient(IWmeDebugClient *Client) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::DetachClient(IWmeDebugClient *Client) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBDebugger::QueryData(IWmeDebugClient *Client) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
int CBDebugger::GetPropInt(const char *PropName) {
	return 0;
}

//////////////////////////////////////////////////////////////////////////
double CBDebugger::GetPropFloat(const char *PropName) {
	return 0.0;
}

//////////////////////////////////////////////////////////////////////////
const char *CBDebugger::GetPropString(const char *PropName) {
	return "";
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::GetPropBool(const char *PropName) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::SetProp(const char *PropName, int PropValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::SetProp(const char *PropName, double PropValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::SetProp(const char *PropName, const char *PropValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::SetProp(const char *PropName, bool PropValue) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::ResolveFilename(const char *RelativeFilename, char *AbsFilenameBuf, int AbsBufSize) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::AddBreakpoint(const char *ScriptFilename, int Line) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::RemoveBreakpoint(const char *ScriptFilename, int Line) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBDebugger::ContinueExecution() {
	return false;
}

} // end of namespace WinterMute
