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
	m_Sprite = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdTalkHolder::~CAdTalkHolder() {
	delete m_Sprite;
	m_Sprite = NULL;

	int i;
	for (i = 0; i < m_TalkSprites.GetSize(); i++) delete m_TalkSprites[i];
	m_TalkSprites.RemoveAll();

	for (i = 0; i < m_TalkSpritesEx.GetSize(); i++) delete m_TalkSpritesEx[i];
	m_TalkSpritesEx.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////
CBSprite *CAdTalkHolder::GetTalkStance(char *Stance) {
	CBSprite *ret = NULL;


	// forced stance?
	if (m_ForcedTalkAnimName && !m_ForcedTalkAnimUsed) {
		m_ForcedTalkAnimUsed = true;
		delete m_AnimSprite;
		m_AnimSprite = new CBSprite(Game, this);
		if (m_AnimSprite) {
			HRESULT res = m_AnimSprite->LoadFile(m_ForcedTalkAnimName);
			if (FAILED(res)) {
				Game->LOG(res, "CAdTalkHolder::GetTalkStance: error loading talk sprite (object:\"%s\" sprite:\"%s\")", m_Name, m_ForcedTalkAnimName);
				delete m_AnimSprite;
				m_AnimSprite = NULL;
			} else return m_AnimSprite;
		}
	}


	if (Stance != NULL) {
		// search special talk stances
		for (int i = 0; i < m_TalkSpritesEx.GetSize(); i++) {
			if (scumm_stricmp(m_TalkSpritesEx[i]->m_Name, Stance) == 0) {
				ret = m_TalkSpritesEx[i];
				break;
			}
		}
		if (ret == NULL) {
			// serach generic talk stances
			for (int i = 0; i < m_TalkSprites.GetSize(); i++) {
				if (scumm_stricmp(m_TalkSprites[i]->m_Name, Stance) == 0) {
					ret = m_TalkSprites[i];
					break;
				}
			}
		}
	}

	// not a valid stance? get a random one
	if (ret == NULL) {
		if (m_TalkSprites.GetSize() < 1) ret = m_Sprite;
		else {
			// TODO: remember last
			int rnd = rand() % m_TalkSprites.GetSize();
			ret = m_TalkSprites[rnd];
		}
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSprite
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetSprite") == 0) {
		Stack->CorrectParams(1);

		CScValue *Val = Stack->Pop();

		bool SetCurrent = false;
		if (m_CurrentSprite && m_CurrentSprite == m_Sprite) SetCurrent = true;

		delete m_Sprite;
		m_Sprite = NULL;

		if (Val->IsNULL()) {
			m_Sprite = NULL;
			if (SetCurrent) m_CurrentSprite = NULL;
			Stack->PushBool(true);
		} else {
			char *Filename = Val->GetString();
			CBSprite *spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->LoadFile(Filename))) {
				Script->RuntimeError("SetSprite method failed for file '%s'", Filename);
				Stack->PushBool(false);
			} else {
				m_Sprite = spr;
				if (SetCurrent) m_CurrentSprite = m_Sprite;
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

		if (!m_Sprite || !m_Sprite->m_Filename) Stack->PushNULL();
		else Stack->PushString(m_Sprite->m_Filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSpriteObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSpriteObject") == 0) {
		Stack->CorrectParams(0);

		if (!m_Sprite) Stack->PushNULL();
		else Stack->PushNative(m_Sprite, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddTalkSprite") == 0) {
		Stack->CorrectParams(2);

		char *Filename = Stack->Pop()->GetString();
		bool Ex = Stack->Pop()->GetBool();

		CBSprite *spr = new CBSprite(Game, this);
		if (!spr || FAILED(spr->LoadFile(Filename))) {
			Stack->PushBool(false);
			Script->RuntimeError("AddTalkSprite method failed for file '%s'", Filename);
		} else {
			if (Ex) m_TalkSpritesEx.Add(spr);
			else m_TalkSprites.Add(spr);
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RemoveTalkSprite") == 0) {
		Stack->CorrectParams(2);

		char *Filename = Stack->Pop()->GetString();
		bool Ex = Stack->Pop()->GetBool();
		int i;

		bool SetCurrent = false;
		bool SetTemp2 = false;

		if (Ex) {
			for (i = 0; i < m_TalkSpritesEx.GetSize(); i++) {
				if (scumm_stricmp(m_TalkSpritesEx[i]->m_Filename, Filename) == 0) {
					if (m_CurrentSprite == m_TalkSpritesEx[i]) SetCurrent = true;
					if (m_TempSprite2 == m_TalkSpritesEx[i]) SetTemp2 = true;
					delete m_TalkSpritesEx[i];
					m_TalkSpritesEx.RemoveAt(i);
					break;
				}
			}
		} else {
			for (i = 0; i < m_TalkSprites.GetSize(); i++) {
				if (scumm_stricmp(m_TalkSprites[i]->m_Filename, Filename) == 0) {
					if (m_CurrentSprite == m_TalkSprites[i]) SetCurrent = true;
					if (m_TempSprite2 == m_TalkSprites[i]) SetTemp2 = true;
					delete m_TalkSprites[i];
					m_TalkSprites.RemoveAt(i);
					break;
				}
			}

		}

		Stack->PushBool(true);
		if (SetCurrent) m_CurrentSprite = m_Sprite;
		if (SetTemp2) m_TempSprite2 = m_Sprite;

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetTalkSprite") == 0) {
		Stack->CorrectParams(2);

		char *Filename = Stack->Pop()->GetString();
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
				for (i = 0; i < m_TalkSpritesEx.GetSize(); i++) {
					if (m_TalkSpritesEx[i] == m_CurrentSprite) SetCurrent = true;
					if (m_TalkSpritesEx[i] == m_TempSprite2) SetTemp2 = true;
					delete m_TalkSpritesEx[i];
				}
				m_TalkSpritesEx.RemoveAll();
			} else {
				for (i = 0; i < m_TalkSprites.GetSize(); i++) {
					if (m_TalkSprites[i] == m_CurrentSprite) SetCurrent = true;
					if (m_TalkSprites[i] == m_TempSprite2) SetTemp2 = true;
					delete m_TalkSprites[i];
				}
				m_TalkSprites.RemoveAll();
			}

			// set new
			if (Ex) m_TalkSpritesEx.Add(spr);
			else m_TalkSprites.Add(spr);
			Stack->PushBool(true);

			if (SetCurrent) m_CurrentSprite = spr;
			if (SetTemp2) m_TempSprite2 = spr;
		}
		return S_OK;
	}

	else return CAdObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdTalkHolder::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("talk-holder");
		return m_ScValue;
	}

	else return CAdObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::ScSetProperty(char *Name, CScValue *Value) {
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
	for (i = 0; i < m_TalkSprites.GetSize(); i++) {
		if (m_TalkSprites[i]->m_Filename)
			Buffer->PutTextIndent(Indent + 2, "TALK=\"%s\"\n", m_TalkSprites[i]->m_Filename);
	}

	for (i = 0; i < m_TalkSpritesEx.GetSize(); i++) {
		if (m_TalkSpritesEx[i]->m_Filename)
			Buffer->PutTextIndent(Indent + 2, "TALK_SPECIAL=\"%s\"\n", m_TalkSpritesEx[i]->m_Filename);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkHolder::Persist(CBPersistMgr *PersistMgr) {
	CAdObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Sprite));
	m_TalkSprites.Persist(PersistMgr);
	m_TalkSpritesEx.Persist(PersistMgr);

	return S_OK;
}

} // end of namespace WinterMute
