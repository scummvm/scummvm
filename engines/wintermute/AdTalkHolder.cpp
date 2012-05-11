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

#define FORBIDDEN_SYMBOL_EXCEPTION_rand
#include "dcgf.h"
#include "AdTalkHolder.h"
#include "BDynBuffer.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "BGame.h"
#include "BSprite.h"
#include "PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdTalkHolder, false)

//////////////////////////////////////////////////////////////////////////
CAdTalkHolder::CAdTalkHolder(CBGame *inGame): CAdObject(inGame) {
	_sprite = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdTalkHolder::~CAdTalkHolder() {
	delete _sprite;
	_sprite = NULL;

	int i;
	for (i = 0; i < _talkSprites.GetSize(); i++) delete _talkSprites[i];
	_talkSprites.RemoveAll();

	for (i = 0; i < _talkSpritesEx.GetSize(); i++) delete _talkSpritesEx[i];
	_talkSpritesEx.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////
CBSprite *CAdTalkHolder::GetTalkStance(const char *Stance) {
	CBSprite *ret = NULL;


	// forced stance?
	if (_forcedTalkAnimName && !_forcedTalkAnimUsed) {
		_forcedTalkAnimUsed = true;
		delete _animSprite;
		_animSprite = new CBSprite(Game, this);
		if (_animSprite) {
			HRESULT res = _animSprite->LoadFile(_forcedTalkAnimName);
			if (FAILED(res)) {
				Game->LOG(res, "CAdTalkHolder::GetTalkStance: error loading talk sprite (object:\"%s\" sprite:\"%s\")", _name, _forcedTalkAnimName);
				delete _animSprite;
				_animSprite = NULL;
			} else return _animSprite;
		}
	}


	if (Stance != NULL) {
		// search special talk stances
		for (int i = 0; i < _talkSpritesEx.GetSize(); i++) {
			if (scumm_stricmp(_talkSpritesEx[i]->_name, Stance) == 0) {
				ret = _talkSpritesEx[i];
				break;
			}
		}
		if (ret == NULL) {
			// serach generic talk stances
			for (int i = 0; i < _talkSprites.GetSize(); i++) {
				if (scumm_stricmp(_talkSprites[i]->_name, Stance) == 0) {
					ret = _talkSprites[i];
					break;
				}
			}
		}
	}

	// not a valid stance? get a random one
	if (ret == NULL) {
		if (_talkSprites.GetSize() < 1) ret = _sprite;
		else {
			// TODO: remember last
			int rnd = rand() % _talkSprites.GetSize();
			ret = _talkSprites[rnd];
		}
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSprite
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetSprite") == 0) {
		Stack->CorrectParams(1);

		CScValue *Val = Stack->Pop();

		bool SetCurrent = false;
		if (_currentSprite && _currentSprite == _sprite) SetCurrent = true;

		delete _sprite;
		_sprite = NULL;

		if (Val->IsNULL()) {
			_sprite = NULL;
			if (SetCurrent) _currentSprite = NULL;
			Stack->PushBool(true);
		} else {
			const char *Filename = Val->GetString();
			CBSprite *spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->LoadFile(Filename))) {
				Script->RuntimeError("SetSprite method failed for file '%s'", Filename);
				Stack->PushBool(false);
			} else {
				_sprite = spr;
				if (SetCurrent) _currentSprite = _sprite;
				Stack->PushBool(true);
			}
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSprite") == 0) {
		Stack->CorrectParams(0);

		if (!_sprite || !_sprite->_filename) Stack->PushNULL();
		else Stack->PushString(_sprite->_filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSpriteObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSpriteObject") == 0) {
		Stack->CorrectParams(0);

		if (!_sprite) Stack->PushNULL();
		else Stack->PushNative(_sprite, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddTalkSprite") == 0) {
		Stack->CorrectParams(2);

		const char *Filename = Stack->Pop()->GetString();
		bool Ex = Stack->Pop()->GetBool();

		CBSprite *spr = new CBSprite(Game, this);
		if (!spr || FAILED(spr->LoadFile(Filename))) {
			Stack->PushBool(false);
			Script->RuntimeError("AddTalkSprite method failed for file '%s'", Filename);
		} else {
			if (Ex) _talkSpritesEx.Add(spr);
			else _talkSprites.Add(spr);
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RemoveTalkSprite") == 0) {
		Stack->CorrectParams(2);

		const char *Filename = Stack->Pop()->GetString();
		bool Ex = Stack->Pop()->GetBool();
		int i;

		bool SetCurrent = false;
		bool SetTemp2 = false;

		if (Ex) {
			for (i = 0; i < _talkSpritesEx.GetSize(); i++) {
				if (scumm_stricmp(_talkSpritesEx[i]->_filename, Filename) == 0) {
					if (_currentSprite == _talkSpritesEx[i]) SetCurrent = true;
					if (_tempSprite2 == _talkSpritesEx[i]) SetTemp2 = true;
					delete _talkSpritesEx[i];
					_talkSpritesEx.RemoveAt(i);
					break;
				}
			}
		} else {
			for (i = 0; i < _talkSprites.GetSize(); i++) {
				if (scumm_stricmp(_talkSprites[i]->_filename, Filename) == 0) {
					if (_currentSprite == _talkSprites[i]) SetCurrent = true;
					if (_tempSprite2 == _talkSprites[i]) SetTemp2 = true;
					delete _talkSprites[i];
					_talkSprites.RemoveAt(i);
					break;
				}
			}

		}

		Stack->PushBool(true);
		if (SetCurrent) _currentSprite = _sprite;
		if (SetTemp2) _tempSprite2 = _sprite;

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetTalkSprite") == 0) {
		Stack->CorrectParams(2);

		const char *Filename = Stack->Pop()->GetString();
		bool Ex = Stack->Pop()->GetBool();
		bool SetCurrent = false;
		bool SetTemp2 = false;

		CBSprite *spr = new CBSprite(Game, this);
		if (!spr || FAILED(spr->LoadFile(Filename))) {
			Stack->PushBool(false);
			Script->RuntimeError("SetTalkSprite method failed for file '%s'", Filename);
		} else {

			// delete current
			int i;
			if (Ex) {
				for (i = 0; i < _talkSpritesEx.GetSize(); i++) {
					if (_talkSpritesEx[i] == _currentSprite) SetCurrent = true;
					if (_talkSpritesEx[i] == _tempSprite2) SetTemp2 = true;
					delete _talkSpritesEx[i];
				}
				_talkSpritesEx.RemoveAll();
			} else {
				for (i = 0; i < _talkSprites.GetSize(); i++) {
					if (_talkSprites[i] == _currentSprite) SetCurrent = true;
					if (_talkSprites[i] == _tempSprite2) SetTemp2 = true;
					delete _talkSprites[i];
				}
				_talkSprites.RemoveAll();
			}

			// set new
			if (Ex) _talkSpritesEx.Add(spr);
			else _talkSprites.Add(spr);
			Stack->PushBool(true);

			if (SetCurrent) _currentSprite = spr;
			if (SetTemp2) _tempSprite2 = spr;
		}
		return S_OK;
	}

	else return CAdObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdTalkHolder::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("talk-holder");
		return _scValue;
	}

	else return CAdObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::ScSetProperty(const char *Name, CScValue *Value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////
	if(strcmp(Name, "Item")==0){
	    SetItem(Value->GetString());
	    return S_OK;
	}

	else*/ return CAdObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CAdTalkHolder::ScToString() {
	return "[talk-holder object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	int i;
	for (i = 0; i < _talkSprites.GetSize(); i++) {
		if (_talkSprites[i]->_filename)
			Buffer->PutTextIndent(Indent + 2, "TALK=\"%s\"\n", _talkSprites[i]->_filename);
	}

	for (i = 0; i < _talkSpritesEx.GetSize(); i++) {
		if (_talkSpritesEx[i]->_filename)
			Buffer->PutTextIndent(Indent + 2, "TALK_SPECIAL=\"%s\"\n", _talkSpritesEx[i]->_filename);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::Persist(CBPersistMgr *PersistMgr) {
	CAdObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_sprite));
	_talkSprites.Persist(PersistMgr);
	_talkSpritesEx.Persist(PersistMgr);

	return S_OK;
}

} // end of namespace WinterMute
