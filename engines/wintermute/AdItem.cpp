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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/AdItem.h"
#include "engines/wintermute/AdGame.h"
#include "engines/wintermute/AdSentence.h"
#include "engines/wintermute/BFontStorage.h"
#include "engines/wintermute/BFont.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BSound.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdItem, false)

//////////////////////////////////////////////////////////////////////////
CAdItem::CAdItem(CBGame *inGame): CAdTalkHolder(inGame) {
	m_SpriteHover = NULL;
	m_CursorNormal = m_CursorHover = NULL;

	m_CursorCombined = true;
	m_InInventory = false;

	m_DisplayAmount = false;
	m_Amount = 0;
	m_AmountOffsetX = 0;
	m_AmountOffsetY = 0;
	m_AmountAlign = TAL_RIGHT;
	m_AmountString = NULL;

	m_State = STATE_READY;

	m_Movable = false;
}


//////////////////////////////////////////////////////////////////////////
CAdItem::~CAdItem() {
	delete m_SpriteHover;
	delete m_CursorNormal;
	delete m_CursorHover;
	m_SpriteHover = NULL;
	m_CursorNormal = NULL;
	m_CursorHover = NULL;

	delete[] m_AmountString;
	m_AmountString = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdItem::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing ITEM file '%s'", Filename);


	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(ITEM)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(CURSOR_HOVER)
TOKEN_DEF(CURSOR_COMBINED)
TOKEN_DEF(CURSOR)
TOKEN_DEF(NAME)
TOKEN_DEF(IMAGE_HOVER)
TOKEN_DEF(IMAGE)
TOKEN_DEF(EVENTS)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(FONT)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(TALK_SPECIAL)
TOKEN_DEF(TALK)
TOKEN_DEF(SPRITE_HOVER)
TOKEN_DEF(SPRITE)
TOKEN_DEF(DISPLAY_AMOUNT)
TOKEN_DEF(AMOUNT_OFFSET_X)
TOKEN_DEF(AMOUNT_OFFSET_Y)
TOKEN_DEF(AMOUNT_ALIGN)
TOKEN_DEF(AMOUNT_STRING)
TOKEN_DEF(AMOUNT)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ITEM)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(CURSOR_HOVER)
	TOKEN_TABLE(CURSOR_COMBINED)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(IMAGE_HOVER)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(TALK_SPECIAL)
	TOKEN_TABLE(TALK)
	TOKEN_TABLE(SPRITE_HOVER)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(DISPLAY_AMOUNT)
	TOKEN_TABLE(AMOUNT_OFFSET_X)
	TOKEN_TABLE(AMOUNT_OFFSET_Y)
	TOKEN_TABLE(AMOUNT_ALIGN)
	TOKEN_TABLE(AMOUNT_STRING)
	TOKEN_TABLE(AMOUNT)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_ITEM) {
			Game->LOG(0, "'ITEM' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	int ar = 0, ag = 0, ab = 0, alpha = 255;
	while (cmd > 0 && (cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_FONT:
			SetFont((char *)params);
			break;

		case TOKEN_CAPTION:
			SetCaption((char *)params);
			break;

		case TOKEN_IMAGE:
		case TOKEN_SPRITE:
			delete m_Sprite;
			m_Sprite = new CBSprite(Game, this);
			if (!m_Sprite || FAILED(m_Sprite->LoadFile((char *)params, ((CAdGame *)Game)->m_TexItemLifeTime))) {
				delete m_Sprite;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_HOVER:
		case TOKEN_SPRITE_HOVER:
			delete m_SpriteHover;
			m_SpriteHover = new CBSprite(Game, this);
			if (!m_SpriteHover || FAILED(m_SpriteHover->LoadFile((char *)params, ((CAdGame *)Game)->m_TexItemLifeTime))) {
				delete m_SpriteHover;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_AMOUNT:
			parser.ScanStr((char *)params, "%d", &m_Amount);
			break;

		case TOKEN_DISPLAY_AMOUNT:
			parser.ScanStr((char *)params, "%b", &m_DisplayAmount);
			break;

		case TOKEN_AMOUNT_OFFSET_X:
			parser.ScanStr((char *)params, "%d", &m_AmountOffsetX);
			break;

		case TOKEN_AMOUNT_OFFSET_Y:
			parser.ScanStr((char *)params, "%d", &m_AmountOffsetY);
			break;

		case TOKEN_AMOUNT_ALIGN:
			if (scumm_stricmp((char *)params, "left") == 0) m_AmountAlign = TAL_LEFT;
			else if (scumm_stricmp((char *)params, "right") == 0) m_AmountAlign = TAL_RIGHT;
			else m_AmountAlign = TAL_CENTER;
			break;

		case TOKEN_AMOUNT_STRING:
			CBUtils::SetString(&m_AmountString, (char *)params);
			break;

		case TOKEN_TALK: {
			CBSprite *spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->LoadFile((char *)params, ((CAdGame *)Game)->m_TexTalkLifeTime))) cmd = PARSERR_GENERIC;
			else m_TalkSprites.Add(spr);
		}
		break;

		case TOKEN_TALK_SPECIAL: {
			CBSprite *spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->LoadFile((char *)params, ((CAdGame *)Game)->m_TexTalkLifeTime))) cmd = PARSERR_GENERIC;
			else m_TalkSpritesEx.Add(spr);
		}
		break;

		case TOKEN_CURSOR:
			delete m_CursorNormal;
			m_CursorNormal = new CBSprite(Game);
			if (!m_CursorNormal || FAILED(m_CursorNormal->LoadFile((char *)params, ((CAdGame *)Game)->m_TexItemLifeTime))) {
				delete m_CursorNormal;
				m_CursorNormal = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CURSOR_HOVER:
			delete m_CursorHover;
			m_CursorHover = new CBSprite(Game);
			if (!m_CursorHover || FAILED(m_CursorHover->LoadFile((char *)params, ((CAdGame *)Game)->m_TexItemLifeTime))) {
				delete m_CursorHover;
				m_CursorHover = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CURSOR_COMBINED:
			parser.ScanStr((char *)params, "%b", &m_CursorCombined);
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_PROPERTY:
			ParseProperty(params, false);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.ScanStr((char *)params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.ScanStr((char *)params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in ITEM definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading ITEM definition");
		return E_FAIL;
	}

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	m_AlphaColor = DRGBA(ar, ag, ab, alpha);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::Update() {
	m_CurrentSprite = NULL;

	if (m_State == STATE_READY && m_AnimSprite) {
		delete m_AnimSprite;
		m_AnimSprite = NULL;
	}

	// finished playing animation?
	if (m_State == STATE_PLAYING_ANIM && m_AnimSprite != NULL && m_AnimSprite->m_Finished) {
		m_State = STATE_READY;
		m_CurrentSprite = m_AnimSprite;
	}

	if (m_Sentence && m_State != STATE_TALKING) m_Sentence->Finish();

	// default: stand animation
	if (!m_CurrentSprite) m_CurrentSprite = m_Sprite;

	switch (m_State) {
		//////////////////////////////////////////////////////////////////////////
	case STATE_PLAYING_ANIM:
		m_CurrentSprite = m_AnimSprite;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_READY:
		if (!m_AnimSprite) {
			if (Game->m_ActiveObject == this && m_SpriteHover) m_CurrentSprite = m_SpriteHover;
			else m_CurrentSprite = m_Sprite;
		}
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_TALKING: {
		m_Sentence->Update();
		if (m_Sentence->m_CurrentSprite) m_TempSprite2 = m_Sentence->m_CurrentSprite;

		bool TimeIsUp = (m_Sentence->m_Sound && m_Sentence->m_SoundStarted && (!m_Sentence->m_Sound->IsPlaying() && !m_Sentence->m_Sound->IsPaused())) || (!m_Sentence->m_Sound && m_Sentence->m_Duration <= Game->m_Timer - m_Sentence->m_StartTime);
		if (m_TempSprite2 == NULL || m_TempSprite2->m_Finished || (/*m_TempSprite2->m_Looping &&*/ TimeIsUp)) {
			if (TimeIsUp) {
				m_Sentence->Finish();
				m_TempSprite2 = NULL;
				m_State = STATE_READY;
			} else {
				m_TempSprite2 = GetTalkStance(m_Sentence->GetNextStance());
				if (m_TempSprite2) {
					m_TempSprite2->Reset();
					m_CurrentSprite = m_TempSprite2;
				}
				((CAdGame *)Game)->AddSentence(m_Sentence);
			}
		} else {
			m_CurrentSprite = m_TempSprite2;
			((CAdGame *)Game)->AddSentence(m_Sentence);
		}
	}
	break;
	}
	m_Ready = (m_State == STATE_READY);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::Display(int X, int Y) {
	int Width = 0;
	if (m_CurrentSprite) {
		RECT rc;
		m_CurrentSprite->GetBoundingRect(&rc, 0, 0);
		Width = rc.right - rc.left;
	}

	m_PosX = X + Width / 2;
	m_PosY = Y;

	HRESULT ret;
	if (m_CurrentSprite) ret = m_CurrentSprite->Draw(X, Y, this, 100, 100, m_AlphaColor);
	else ret = S_OK;

	if (m_DisplayAmount) {
		int AmountX = X;
		int AmountY = Y + m_AmountOffsetY;

		if (m_AmountAlign == TAL_RIGHT) {
			Width -= m_AmountOffsetX;
			AmountX -= m_AmountOffsetX;
		}
		AmountX += m_AmountOffsetX;

		CBFont *Font = m_Font ? m_Font : Game->m_SystemFont;
		if (Font) {
			if (m_AmountString) Font->DrawText((byte  *)m_AmountString, AmountX, AmountY, Width, m_AmountAlign);
			else {
				char Str[256];
				sprintf(Str, "%d", m_Amount);
				Font->DrawText((byte  *)Str, AmountX, AmountY, Width, m_AmountAlign);
			}
		}
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetHoverSprite
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetHoverSprite") == 0) {
		Stack->CorrectParams(1);

		bool SetCurrent = false;
		if (m_CurrentSprite && m_CurrentSprite == m_SpriteHover) SetCurrent = true;

		char *Filename = Stack->Pop()->GetString();

		delete m_SpriteHover;
		m_SpriteHover = NULL;
		CBSprite *spr = new CBSprite(Game, this);
		if (!spr || FAILED(spr->LoadFile(Filename))) {
			Stack->PushBool(false);
			Script->RuntimeError("Item.SetHoverSprite failed for file '%s'", Filename);
		} else {
			m_SpriteHover = spr;
			if (SetCurrent) m_CurrentSprite = m_SpriteHover;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverSprite") == 0) {
		Stack->CorrectParams(0);

		if (!m_SpriteHover || !m_SpriteHover->m_Filename) Stack->PushNULL();
		else Stack->PushString(m_SpriteHover->m_Filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverSpriteObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverSpriteObject") == 0) {
		Stack->CorrectParams(0);
		if (!m_SpriteHover) Stack->PushNULL();
		else Stack->PushNative(m_SpriteHover, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetNormalCursor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetNormalCursor") == 0) {
		Stack->CorrectParams(1);

		char *Filename = Stack->Pop()->GetString();

		delete m_CursorNormal;
		m_CursorNormal = NULL;
		CBSprite *spr = new CBSprite(Game);
		if (!spr || FAILED(spr->LoadFile(Filename))) {
			Stack->PushBool(false);
			Script->RuntimeError("Item.SetNormalCursor failed for file '%s'", Filename);
		} else {
			m_CursorNormal = spr;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNormalCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetNormalCursor") == 0) {
		Stack->CorrectParams(0);

		if (!m_CursorNormal || !m_CursorNormal->m_Filename) Stack->PushNULL();
		else Stack->PushString(m_CursorNormal->m_Filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNormalCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetNormalCursorObject") == 0) {
		Stack->CorrectParams(0);

		if (!m_CursorNormal) Stack->PushNULL();
		else Stack->PushNative(m_CursorNormal, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetHoverCursor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetHoverCursor") == 0) {
		Stack->CorrectParams(1);

		char *Filename = Stack->Pop()->GetString();

		delete m_CursorHover;
		m_CursorHover = NULL;
		CBSprite *spr = new CBSprite(Game);
		if (!spr || FAILED(spr->LoadFile(Filename))) {
			Stack->PushBool(false);
			Script->RuntimeError("Item.SetHoverCursor failed for file '%s'", Filename);
		} else {
			m_CursorHover = spr;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverCursor") == 0) {
		Stack->CorrectParams(0);

		if (!m_CursorHover || !m_CursorHover->m_Filename) Stack->PushNULL();
		else Stack->PushString(m_CursorHover->m_Filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverCursorObject") == 0) {
		Stack->CorrectParams(0);

		if (!m_CursorHover) Stack->PushNULL();
		else Stack->PushNative(m_CursorHover, true);
		return S_OK;
	}

	else return CAdTalkHolder::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdItem::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("item");
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
	// DisplayAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DisplayAmount") == 0) {
		m_ScValue->SetBool(m_DisplayAmount);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Amount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Amount") == 0) {
		m_ScValue->SetInt(m_Amount);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountOffsetX") == 0) {
		m_ScValue->SetInt(m_AmountOffsetX);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountOffsetY") == 0) {
		m_ScValue->SetInt(m_AmountOffsetY);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountAlign") == 0) {
		m_ScValue->SetInt(m_AmountAlign);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountString") == 0) {
		if (!m_AmountString) m_ScValue->SetNULL();
		else m_ScValue->SetString(m_AmountString);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorCombined
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CursorCombined") == 0) {
		m_ScValue->SetBool(m_CursorCombined);
		return m_ScValue;
	}

	else return CAdTalkHolder::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
		SetName(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisplayAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DisplayAmount") == 0) {
		m_DisplayAmount = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Amount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Amount") == 0) {
		m_Amount = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountOffsetX") == 0) {
		m_AmountOffsetX = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountOffsetY") == 0) {
		m_AmountOffsetY = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountAlign") == 0) {
		m_AmountAlign = (TTextAlign)Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountString") == 0) {
		if (Value->IsNULL()) SAFE_DELETE_ARRAY(m_AmountString);
		else CBUtils::SetString(&m_AmountString, Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorCombined
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CursorCombined") == 0) {
		m_CursorCombined = Value->GetBool();
		return S_OK;
	}

	else return CAdTalkHolder::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CAdItem::ScToString() {
	return "[item]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::Persist(CBPersistMgr *PersistMgr) {

	CAdTalkHolder::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_CursorCombined));
	PersistMgr->Transfer(TMEMBER(m_CursorHover));
	PersistMgr->Transfer(TMEMBER(m_CursorNormal));
	PersistMgr->Transfer(TMEMBER(m_SpriteHover));
	PersistMgr->Transfer(TMEMBER(m_InInventory));
	PersistMgr->Transfer(TMEMBER(m_DisplayAmount));
	PersistMgr->Transfer(TMEMBER(m_Amount));
	PersistMgr->Transfer(TMEMBER(m_AmountOffsetX));
	PersistMgr->Transfer(TMEMBER(m_AmountOffsetY));
	PersistMgr->Transfer(TMEMBER_INT(m_AmountAlign));
	PersistMgr->Transfer(TMEMBER(m_AmountString));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdItem::GetExtendedFlag(char *FlagName) {
	if (!FlagName) return false;
	else if (strcmp(FlagName, "usable") == 0) return true;
	else return CAdObject::GetExtendedFlag(FlagName);
}

} // end of namespace WinterMute
