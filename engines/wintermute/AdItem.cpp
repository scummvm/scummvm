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
	_spriteHover = NULL;
	_cursorNormal = _cursorHover = NULL;

	_cursorCombined = true;
	_inInventory = false;

	_displayAmount = false;
	_amount = 0;
	_amountOffsetX = 0;
	_amountOffsetY = 0;
	_amountAlign = TAL_RIGHT;
	_amountString = NULL;

	_state = STATE_READY;

	_movable = false;
}


//////////////////////////////////////////////////////////////////////////
CAdItem::~CAdItem() {
	delete _spriteHover;
	delete _cursorNormal;
	delete _cursorHover;
	_spriteHover = NULL;
	_cursorNormal = NULL;
	_cursorHover = NULL;

	delete[] _amountString;
	_amountString = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::LoadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdItem::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

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
			delete _sprite;
			_sprite = new CBSprite(Game, this);
			if (!_sprite || FAILED(_sprite->LoadFile((char *)params, ((CAdGame *)Game)->_texItemLifeTime))) {
				delete _sprite;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_HOVER:
		case TOKEN_SPRITE_HOVER:
			delete _spriteHover;
			_spriteHover = new CBSprite(Game, this);
			if (!_spriteHover || FAILED(_spriteHover->LoadFile((char *)params, ((CAdGame *)Game)->_texItemLifeTime))) {
				delete _spriteHover;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_AMOUNT:
			parser.ScanStr((char *)params, "%d", &_amount);
			break;

		case TOKEN_DISPLAY_AMOUNT:
			parser.ScanStr((char *)params, "%b", &_displayAmount);
			break;

		case TOKEN_AMOUNT_OFFSET_X:
			parser.ScanStr((char *)params, "%d", &_amountOffsetX);
			break;

		case TOKEN_AMOUNT_OFFSET_Y:
			parser.ScanStr((char *)params, "%d", &_amountOffsetY);
			break;

		case TOKEN_AMOUNT_ALIGN:
			if (scumm_stricmp((char *)params, "left") == 0) _amountAlign = TAL_LEFT;
			else if (scumm_stricmp((char *)params, "right") == 0) _amountAlign = TAL_RIGHT;
			else _amountAlign = TAL_CENTER;
			break;

		case TOKEN_AMOUNT_STRING:
			CBUtils::SetString(&_amountString, (char *)params);
			break;

		case TOKEN_TALK: {
			CBSprite *spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->LoadFile((char *)params, ((CAdGame *)Game)->_texTalkLifeTime))) cmd = PARSERR_GENERIC;
			else _talkSprites.Add(spr);
		}
		break;

		case TOKEN_TALK_SPECIAL: {
			CBSprite *spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->LoadFile((char *)params, ((CAdGame *)Game)->_texTalkLifeTime))) cmd = PARSERR_GENERIC;
			else _talkSpritesEx.Add(spr);
		}
		break;

		case TOKEN_CURSOR:
			delete _cursorNormal;
			_cursorNormal = new CBSprite(Game);
			if (!_cursorNormal || FAILED(_cursorNormal->LoadFile((char *)params, ((CAdGame *)Game)->_texItemLifeTime))) {
				delete _cursorNormal;
				_cursorNormal = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CURSOR_HOVER:
			delete _cursorHover;
			_cursorHover = new CBSprite(Game);
			if (!_cursorHover || FAILED(_cursorHover->LoadFile((char *)params, ((CAdGame *)Game)->_texItemLifeTime))) {
				delete _cursorHover;
				_cursorHover = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CURSOR_COMBINED:
			parser.ScanStr((char *)params, "%b", &_cursorCombined);
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
	_alphaColor = DRGBA(ar, ag, ab, alpha);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::Update() {
	_currentSprite = NULL;

	if (_state == STATE_READY && _animSprite) {
		delete _animSprite;
		_animSprite = NULL;
	}

	// finished playing animation?
	if (_state == STATE_PLAYING_ANIM && _animSprite != NULL && _animSprite->_finished) {
		_state = STATE_READY;
		_currentSprite = _animSprite;
	}

	if (_sentence && _state != STATE_TALKING) _sentence->Finish();

	// default: stand animation
	if (!_currentSprite) _currentSprite = _sprite;

	switch (_state) {
		//////////////////////////////////////////////////////////////////////////
	case STATE_PLAYING_ANIM:
		_currentSprite = _animSprite;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_READY:
		if (!_animSprite) {
			if (Game->_activeObject == this && _spriteHover) _currentSprite = _spriteHover;
			else _currentSprite = _sprite;
		}
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_TALKING: {
		_sentence->Update();
		if (_sentence->_currentSprite) _tempSprite2 = _sentence->_currentSprite;

		bool TimeIsUp = (_sentence->_sound && _sentence->_soundStarted && (!_sentence->_sound->IsPlaying() && !_sentence->_sound->IsPaused())) || (!_sentence->_sound && _sentence->_duration <= Game->_timer - _sentence->_startTime);
		if (_tempSprite2 == NULL || _tempSprite2->_finished || (/*_tempSprite2->_looping &&*/ TimeIsUp)) {
			if (TimeIsUp) {
				_sentence->Finish();
				_tempSprite2 = NULL;
				_state = STATE_READY;
			} else {
				_tempSprite2 = GetTalkStance(_sentence->GetNextStance());
				if (_tempSprite2) {
					_tempSprite2->Reset();
					_currentSprite = _tempSprite2;
				}
				((CAdGame *)Game)->AddSentence(_sentence);
			}
		} else {
			_currentSprite = _tempSprite2;
			((CAdGame *)Game)->AddSentence(_sentence);
		}
	}
	break;
	}
	_ready = (_state == STATE_READY);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::Display(int X, int Y) {
	int Width = 0;
	if (_currentSprite) {
		RECT rc;
		_currentSprite->GetBoundingRect(&rc, 0, 0);
		Width = rc.right - rc.left;
	}

	_posX = X + Width / 2;
	_posY = Y;

	HRESULT ret;
	if (_currentSprite) ret = _currentSprite->Draw(X, Y, this, 100, 100, _alphaColor);
	else ret = S_OK;

	if (_displayAmount) {
		int AmountX = X;
		int AmountY = Y + _amountOffsetY;

		if (_amountAlign == TAL_RIGHT) {
			Width -= _amountOffsetX;
			AmountX -= _amountOffsetX;
		}
		AmountX += _amountOffsetX;

		CBFont *Font = _font ? _font : Game->_systemFont;
		if (Font) {
			if (_amountString) Font->DrawText((byte *)_amountString, AmountX, AmountY, Width, _amountAlign);
			else {
				char Str[256];
				sprintf(Str, "%d", _amount);
				Font->DrawText((byte *)Str, AmountX, AmountY, Width, _amountAlign);
			}
		}
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetHoverSprite
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetHoverSprite") == 0) {
		Stack->CorrectParams(1);

		bool SetCurrent = false;
		if (_currentSprite && _currentSprite == _spriteHover) SetCurrent = true;

		const char *Filename = Stack->Pop()->GetString();

		delete _spriteHover;
		_spriteHover = NULL;
		CBSprite *spr = new CBSprite(Game, this);
		if (!spr || FAILED(spr->LoadFile(Filename))) {
			Stack->PushBool(false);
			Script->RuntimeError("Item.SetHoverSprite failed for file '%s'", Filename);
		} else {
			_spriteHover = spr;
			if (SetCurrent) _currentSprite = _spriteHover;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverSprite") == 0) {
		Stack->CorrectParams(0);

		if (!_spriteHover || !_spriteHover->_filename) Stack->PushNULL();
		else Stack->PushString(_spriteHover->_filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverSpriteObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverSpriteObject") == 0) {
		Stack->CorrectParams(0);
		if (!_spriteHover) Stack->PushNULL();
		else Stack->PushNative(_spriteHover, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetNormalCursor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetNormalCursor") == 0) {
		Stack->CorrectParams(1);

		const char *Filename = Stack->Pop()->GetString();

		delete _cursorNormal;
		_cursorNormal = NULL;
		CBSprite *spr = new CBSprite(Game);
		if (!spr || FAILED(spr->LoadFile(Filename))) {
			Stack->PushBool(false);
			Script->RuntimeError("Item.SetNormalCursor failed for file '%s'", Filename);
		} else {
			_cursorNormal = spr;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNormalCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetNormalCursor") == 0) {
		Stack->CorrectParams(0);

		if (!_cursorNormal || !_cursorNormal->_filename) Stack->PushNULL();
		else Stack->PushString(_cursorNormal->_filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNormalCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetNormalCursorObject") == 0) {
		Stack->CorrectParams(0);

		if (!_cursorNormal) Stack->PushNULL();
		else Stack->PushNative(_cursorNormal, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetHoverCursor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetHoverCursor") == 0) {
		Stack->CorrectParams(1);

		const char *Filename = Stack->Pop()->GetString();

		delete _cursorHover;
		_cursorHover = NULL;
		CBSprite *spr = new CBSprite(Game);
		if (!spr || FAILED(spr->LoadFile(Filename))) {
			Stack->PushBool(false);
			Script->RuntimeError("Item.SetHoverCursor failed for file '%s'", Filename);
		} else {
			_cursorHover = spr;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverCursor") == 0) {
		Stack->CorrectParams(0);

		if (!_cursorHover || !_cursorHover->_filename) Stack->PushNULL();
		else Stack->PushString(_cursorHover->_filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverCursorObject") == 0) {
		Stack->CorrectParams(0);

		if (!_cursorHover) Stack->PushNULL();
		else Stack->PushNative(_cursorHover, true);
		return S_OK;
	}

	else return CAdTalkHolder::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdItem::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("item");
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
	// DisplayAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DisplayAmount") == 0) {
		_scValue->SetBool(_displayAmount);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Amount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Amount") == 0) {
		_scValue->SetInt(_amount);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountOffsetX") == 0) {
		_scValue->SetInt(_amountOffsetX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountOffsetY") == 0) {
		_scValue->SetInt(_amountOffsetY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountAlign") == 0) {
		_scValue->SetInt(_amountAlign);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountString") == 0) {
		if (!_amountString) _scValue->SetNULL();
		else _scValue->SetString(_amountString);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorCombined
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CursorCombined") == 0) {
		_scValue->SetBool(_cursorCombined);
		return _scValue;
	}

	else return CAdTalkHolder::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::ScSetProperty(const char *Name, CScValue *Value) {
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
		_displayAmount = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Amount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Amount") == 0) {
		_amount = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountOffsetX") == 0) {
		_amountOffsetX = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountOffsetY") == 0) {
		_amountOffsetY = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountAlign") == 0) {
		_amountAlign = (TTextAlign)Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AmountString") == 0) {
		if (Value->IsNULL()) {
			delete[] _amountString;
			_amountString = NULL;
		} else {
			CBUtils::SetString(&_amountString, Value->GetString());
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorCombined
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CursorCombined") == 0) {
		_cursorCombined = Value->GetBool();
		return S_OK;
	}

	else return CAdTalkHolder::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CAdItem::ScToString() {
	return "[item]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::Persist(CBPersistMgr *PersistMgr) {

	CAdTalkHolder::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_cursorCombined));
	PersistMgr->Transfer(TMEMBER(_cursorHover));
	PersistMgr->Transfer(TMEMBER(_cursorNormal));
	PersistMgr->Transfer(TMEMBER(_spriteHover));
	PersistMgr->Transfer(TMEMBER(_inInventory));
	PersistMgr->Transfer(TMEMBER(_displayAmount));
	PersistMgr->Transfer(TMEMBER(_amount));
	PersistMgr->Transfer(TMEMBER(_amountOffsetX));
	PersistMgr->Transfer(TMEMBER(_amountOffsetY));
	PersistMgr->Transfer(TMEMBER_INT(_amountAlign));
	PersistMgr->Transfer(TMEMBER(_amountString));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdItem::GetExtendedFlag(const char *FlagName) {
	if (!FlagName) return false;
	else if (strcmp(FlagName, "usable") == 0) return true;
	else return CAdObject::GetExtendedFlag(FlagName);
}

} // end of namespace WinterMute
