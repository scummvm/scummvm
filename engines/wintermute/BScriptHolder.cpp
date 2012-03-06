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

#include "dcgf.h"
#include "AdGame.h"
#include "BScriptHolder.h"
#include "BParser.h"
#include "ScValue.h"
#include "ScEngine.h"
#include "ScScript.h"
#include "ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBScriptHolder, false)

//////////////////////////////////////////////////////////////////////
CBScriptHolder::CBScriptHolder(CBGame *inGame): CBScriptable(inGame) {
	SetName("<unnamed>");

	m_Freezable = true;
	m_Filename = NULL;
}


//////////////////////////////////////////////////////////////////////
CBScriptHolder::~CBScriptHolder() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::Cleanup() {
	delete[] m_Filename;
	m_Filename = NULL;

	int i;

	for (i = 0; i < m_Scripts.GetSize(); i++) {
		m_Scripts[i]->Finish(true);
		m_Scripts[i]->m_Owner = NULL;
	}
	m_Scripts.RemoveAll();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////
void CBScriptHolder::SetFilename(char *Filename) {
	if (m_Filename != NULL) delete [] m_Filename;

	m_Filename = new char [strlen(Filename) + 1];
	if (m_Filename != NULL) strcpy(m_Filename, Filename);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::ApplyEvent(const char *EventName, bool Unbreakable) {
	int NumHandlers = 0;

	HRESULT ret = E_FAIL;
	for (int i = 0; i < m_Scripts.GetSize(); i++) {
		if (!m_Scripts[i]->m_Thread) {
			CScScript *handler = m_Scripts[i]->InvokeEventHandler(EventName, Unbreakable);
			if (handler) {
				//m_Scripts.Add(handler);
				NumHandlers++;
				ret = S_OK;
			}
		}
	}
	if (NumHandlers > 0 && Unbreakable) Game->m_ScEngine->TickUnbreakable();

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::Listen(CBScriptHolder *param1, uint32 param2) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// DEBUG_CrashMe
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "DEBUG_CrashMe") == 0) {
		Stack->CorrectParams(0);
		byte *p = 0;
		*p = 10;
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ApplyEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ApplyEvent") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		HRESULT ret;
		ret = ApplyEvent(val->GetString());

		if (SUCCEEDED(ret)) Stack->PushBool(true);
		else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CanHandleEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CanHandleEvent") == 0) {
		Stack->CorrectParams(1);
		Stack->PushBool(CanHandleEvent(Stack->Pop()->GetString()));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CanHandleMethod
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CanHandleMethod") == 0) {
		Stack->CorrectParams(1);
		Stack->PushBool(CanHandleMethod(Stack->Pop()->GetString()));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AttachScript
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AttachScript") == 0) {
		Stack->CorrectParams(1);
		Stack->PushBool(SUCCEEDED(AddScript(Stack->Pop()->GetString())));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DetachScript
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DetachScript") == 0) {
		Stack->CorrectParams(2);
		char *Filename = Stack->Pop()->GetString();
		bool KillThreads = Stack->Pop()->GetBool(false);
		bool ret = false;
		for (int i = 0; i < m_Scripts.GetSize(); i++) {
			if (scumm_stricmp(m_Scripts[i]->m_Filename, Filename) == 0) {
				m_Scripts[i]->Finish(KillThreads);
				ret = true;
				break;
			}
		}
		Stack->PushBool(ret);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsScriptRunning
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsScriptRunning") == 0) {
		Stack->CorrectParams(1);
		char *Filename = Stack->Pop()->GetString();
		bool ret = false;
		for (int i = 0; i < m_Scripts.GetSize(); i++) {
			if (scumm_stricmp(m_Scripts[i]->m_Filename, Filename) == 0 && m_Scripts[i]->m_State != SCRIPT_FINISHED && m_Scripts[i]->m_State != SCRIPT_ERROR) {
				ret = true;
				break;
			}
		}
		Stack->PushBool(ret);

		return S_OK;
	} else return CBScriptable::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBScriptHolder::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("script_holder");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Name") == 0) {
		m_ScValue->SetString(m_Name);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Filename (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Filename") == 0) {
		m_ScValue->SetString(m_Filename);
		return m_ScValue;
	}

	else return CBScriptable::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
		SetName(Value->GetString());
		return S_OK;
	} else return CBScriptable::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CBScriptHolder::ScToString() {
	return "[script_holder]";
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	return CBBase::SaveAsText(Buffer, Indent);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::Persist(CBPersistMgr *PersistMgr) {
	CBScriptable::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Filename));
	PersistMgr->Transfer(TMEMBER(m_Freezable));
	PersistMgr->Transfer(TMEMBER(m_Name));
	m_Scripts.Persist(PersistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::AddScript(char *Filename) {
	for (int i = 0; i < m_Scripts.GetSize(); i++) {
		if (scumm_stricmp(m_Scripts[i]->m_Filename, Filename) == 0) {
			if (m_Scripts[i]->m_State != SCRIPT_FINISHED) {
				Game->LOG(0, "CBScriptHolder::AddScript - trying to add script '%s' mutiple times (obj: '%s')", Filename, m_Name);
				return S_OK;
			}
		}
	}

	CScScript *scr =  Game->m_ScEngine->RunScript(Filename, this);
	if (!scr) {
		if (Game->m_EditorForceScripts) {
			// editor hack
			scr = new CScScript(Game, Game->m_ScEngine);
			scr->m_Filename = new char[strlen(Filename) + 1];
			strcpy(scr->m_Filename, Filename);
			scr->m_State = SCRIPT_ERROR;
			scr->m_Owner = this;
			m_Scripts.Add(scr);
			Game->m_ScEngine->m_Scripts.Add(scr);
			Game->GetDebugMgr()->OnScriptInit(scr);

			return S_OK;
		}
		return E_FAIL;
	} else {
		scr->m_Freezable = m_Freezable;
		m_Scripts.Add(scr);
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::RemoveScript(CScScript *Script) {
	for (int i = 0; i < m_Scripts.GetSize(); i++) {
		if (m_Scripts[i] == Script) {
			m_Scripts.RemoveAt(i);
			break;
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBScriptHolder::CanHandleEvent(char *EventName) {
	for (int i = 0; i < m_Scripts.GetSize(); i++) {
		if (!m_Scripts[i]->m_Thread && m_Scripts[i]->CanHandleEvent(EventName)) return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBScriptHolder::CanHandleMethod(char *MethodName) {
	for (int i = 0; i < m_Scripts.GetSize(); i++) {
		if (!m_Scripts[i]->m_Thread && m_Scripts[i]->CanHandleMethod(MethodName)) return true;
	}
	return false;
}


TOKEN_DEF_START
TOKEN_DEF(PROPERTY)
TOKEN_DEF(NAME)
TOKEN_DEF(VALUE)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::ParseProperty(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(VALUE)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_PROPERTY) {
			Game->LOG(0, "'PROPERTY' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	char *PropName = NULL;
	char *PropValue = NULL;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_NAME:
			delete[] PropName;
			PropName = new char[strlen((char *)params) + 1];
			if (PropName) strcpy(PropName, (char *)params);
			else cmd = PARSERR_GENERIC;
			break;

		case TOKEN_VALUE:
			delete[] PropValue;
			PropValue = new char[strlen((char *)params) + 1];
			if (PropValue) strcpy(PropValue, (char *)params);
			else cmd = PARSERR_GENERIC;
			break;
		}

	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		delete[] PropName;
		delete[] PropValue;
		PropName = NULL;
		PropValue = NULL;
		Game->LOG(0, "Syntax error in PROPERTY definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC || PropName == NULL || PropValue == NULL) {
		delete[] PropName;
		delete[] PropValue;
		PropName = NULL;
		PropValue = NULL;
		Game->LOG(0, "Error loading PROPERTY definition");
		return E_FAIL;
	}


	CScValue *val = new CScValue(Game);
	val->SetString(PropValue);
	ScSetProperty(PropName, val);

	delete val;
	delete[] PropName;
	delete[] PropValue;
	PropName = NULL;
	PropValue = NULL;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBScriptHolder::MakeFreezable(bool Freezable) {
	m_Freezable = Freezable;
	for (int i = 0; i < m_Scripts.GetSize(); i++)
		m_Scripts[i]->m_Freezable = Freezable;

}


//////////////////////////////////////////////////////////////////////////
CScScript *CBScriptHolder::InvokeMethodThread(char *MethodName) {
	for (int i = m_Scripts.GetSize() - 1; i >= 0; i--) {
		if (m_Scripts[i]->CanHandleMethod(MethodName)) {

			CScScript *thread = new CScScript(Game, m_Scripts[i]->m_Engine);
			if (thread) {
				HRESULT ret = thread->CreateMethodThread(m_Scripts[i], MethodName);
				if (SUCCEEDED(ret)) {
					m_Scripts[i]->m_Engine->m_Scripts.Add(thread);
					Game->GetDebugMgr()->OnScriptMethodThreadInit(thread, m_Scripts[i], MethodName);

					return thread;
				} else {
					delete thread;
				}
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
void CBScriptHolder::ScDebuggerDesc(char *Buf, int BufSize) {
	strcpy(Buf, ScToString());
	if (m_Name && strcmp(m_Name, "<unnamed>") != 0) {
		strcat(Buf, "  Name: ");
		strcat(Buf, m_Name);
	}
	if (m_Filename) {
		strcat(Buf, "  File: ");
		strcat(Buf, m_Filename);
	}
}


//////////////////////////////////////////////////////////////////////////
// IWmeObject
//////////////////////////////////////////////////////////////////////////
bool CBScriptHolder::SendEvent(const char *EventName) {
	return SUCCEEDED(ApplyEvent((char *)EventName));
}

} // end of namespace WinterMute
