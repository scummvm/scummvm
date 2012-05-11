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
#include "engines/wintermute/AdGame.h"
#include "engines/wintermute/BScriptHolder.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScEngine.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBScriptHolder, false)

//////////////////////////////////////////////////////////////////////
CBScriptHolder::CBScriptHolder(CBGame *inGame): CBScriptable(inGame) {
	SetName("<unnamed>");

	_freezable = true;
	_filename = NULL;
}


//////////////////////////////////////////////////////////////////////
CBScriptHolder::~CBScriptHolder() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::Cleanup() {
	delete[] _filename;
	_filename = NULL;

	int i;

	for (i = 0; i < _scripts.GetSize(); i++) {
		_scripts[i]->Finish(true);
		_scripts[i]->_owner = NULL;
	}
	_scripts.RemoveAll();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////
void CBScriptHolder::SetFilename(const char *Filename) {
	if (_filename != NULL) delete [] _filename;

	_filename = new char [strlen(Filename) + 1];
	if (_filename != NULL) strcpy(_filename, Filename);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::ApplyEvent(const char *EventName, bool Unbreakable) {
	int NumHandlers = 0;

	HRESULT ret = E_FAIL;
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (!_scripts[i]->_thread) {
			CScScript *handler = _scripts[i]->InvokeEventHandler(EventName, Unbreakable);
			if (handler) {
				//_scripts.Add(handler);
				NumHandlers++;
				ret = S_OK;
			}
		}
	}
	if (NumHandlers > 0 && Unbreakable) Game->_scEngine->TickUnbreakable();

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::Listen(CBScriptHolder *param1, uint32 param2) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
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
		const char *Filename = Stack->Pop()->GetString();
		bool KillThreads = Stack->Pop()->GetBool(false);
		bool ret = false;
		for (int i = 0; i < _scripts.GetSize(); i++) {
			if (scumm_stricmp(_scripts[i]->_filename, Filename) == 0) {
				_scripts[i]->Finish(KillThreads);
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
		const char *Filename = Stack->Pop()->GetString();
		bool ret = false;
		for (int i = 0; i < _scripts.GetSize(); i++) {
			if (scumm_stricmp(_scripts[i]->_filename, Filename) == 0 && _scripts[i]->_state != SCRIPT_FINISHED && _scripts[i]->_state != SCRIPT_ERROR) {
				ret = true;
				break;
			}
		}
		Stack->PushBool(ret);

		return S_OK;
	} else return CBScriptable::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBScriptHolder::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("script_holder");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Name") == 0) {
		_scValue->SetString(_name);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Filename (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Filename") == 0) {
		_scValue->SetString(_filename);
		return _scValue;
	}

	else return CBScriptable::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::ScSetProperty(const char *Name, CScValue *Value) {
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

	PersistMgr->Transfer(TMEMBER(_filename));
	PersistMgr->Transfer(TMEMBER(_freezable));
	PersistMgr->Transfer(TMEMBER(_name));
	_scripts.Persist(PersistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::AddScript(const char *Filename) {
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (scumm_stricmp(_scripts[i]->_filename, Filename) == 0) {
			if (_scripts[i]->_state != SCRIPT_FINISHED) {
				Game->LOG(0, "CBScriptHolder::AddScript - trying to add script '%s' mutiple times (obj: '%s')", Filename, _name);
				return S_OK;
			}
		}
	}

	CScScript *scr =  Game->_scEngine->RunScript(Filename, this);
	if (!scr) {
		if (Game->_editorForceScripts) {
			// editor hack
			scr = new CScScript(Game, Game->_scEngine);
			scr->_filename = new char[strlen(Filename) + 1];
			strcpy(scr->_filename, Filename);
			scr->_state = SCRIPT_ERROR;
			scr->_owner = this;
			_scripts.Add(scr);
			Game->_scEngine->_scripts.Add(scr);
			Game->GetDebugMgr()->OnScriptInit(scr);

			return S_OK;
		}
		return E_FAIL;
	} else {
		scr->_freezable = _freezable;
		_scripts.Add(scr);
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBScriptHolder::RemoveScript(CScScript *Script) {
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (_scripts[i] == Script) {
			_scripts.RemoveAt(i);
			break;
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBScriptHolder::CanHandleEvent(const char *EventName) {
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (!_scripts[i]->_thread && _scripts[i]->CanHandleEvent(EventName)) return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBScriptHolder::CanHandleMethod(const char *MethodName) {
	for (int i = 0; i < _scripts.GetSize(); i++) {
		if (!_scripts[i]->_thread && _scripts[i]->CanHandleMethod(MethodName)) return true;
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
	_freezable = Freezable;
	for (int i = 0; i < _scripts.GetSize(); i++)
		_scripts[i]->_freezable = Freezable;

}


//////////////////////////////////////////////////////////////////////////
CScScript *CBScriptHolder::InvokeMethodThread(const char *MethodName) {
	for (int i = _scripts.GetSize() - 1; i >= 0; i--) {
		if (_scripts[i]->CanHandleMethod(MethodName)) {

			CScScript *thread = new CScScript(Game, _scripts[i]->_engine);
			if (thread) {
				HRESULT ret = thread->CreateMethodThread(_scripts[i], MethodName);
				if (SUCCEEDED(ret)) {
					_scripts[i]->_engine->_scripts.Add(thread);
					Game->GetDebugMgr()->OnScriptMethodThreadInit(thread, _scripts[i], MethodName);

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
	if (_name && strcmp(_name, "<unnamed>") != 0) {
		strcat(Buf, "  Name: ");
		strcat(Buf, _name);
	}
	if (_filename) {
		strcat(Buf, "  File: ");
		strcat(Buf, _filename);
	}
}


//////////////////////////////////////////////////////////////////////////
// IWmeObject
//////////////////////////////////////////////////////////////////////////
bool CBScriptHolder::SendEvent(const char *EventName) {
	return SUCCEEDED(ApplyEvent((char *)EventName));
}

} // end of namespace WinterMute
