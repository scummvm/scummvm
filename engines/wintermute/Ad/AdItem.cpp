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
#include "engines/wintermute/Ad/AdItem.h"
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/Ad/AdSentence.h"
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
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
HRESULT CAdItem::loadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->readWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdItem::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = loadBuffer(Buffer, true))) Game->LOG(0, "Error parsing ITEM file '%s'", Filename);


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
HRESULT CAdItem::loadBuffer(byte  *Buffer, bool Complete) {
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
			if (FAILED(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_FONT:
			SetFont((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_IMAGE:
		case TOKEN_SPRITE:
			delete _sprite;
			_sprite = new CBSprite(Game, this);
			if (!_sprite || FAILED(_sprite->loadFile((char *)params, ((CAdGame *)Game)->_texItemLifeTime))) {
				delete _sprite;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_HOVER:
		case TOKEN_SPRITE_HOVER:
			delete _spriteHover;
			_spriteHover = new CBSprite(Game, this);
			if (!_spriteHover || FAILED(_spriteHover->loadFile((char *)params, ((CAdGame *)Game)->_texItemLifeTime))) {
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
			if (!spr || FAILED(spr->loadFile((char *)params, ((CAdGame *)Game)->_texTalkLifeTime))) cmd = PARSERR_GENERIC;
			else _talkSprites.Add(spr);
		}
		break;

		case TOKEN_TALK_SPECIAL: {
			CBSprite *spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->loadFile((char *)params, ((CAdGame *)Game)->_texTalkLifeTime))) cmd = PARSERR_GENERIC;
			else _talkSpritesEx.Add(spr);
		}
		break;

		case TOKEN_CURSOR:
			delete _cursorNormal;
			_cursorNormal = new CBSprite(Game);
			if (!_cursorNormal || FAILED(_cursorNormal->loadFile((char *)params, ((CAdGame *)Game)->_texItemLifeTime))) {
				delete _cursorNormal;
				_cursorNormal = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CURSOR_HOVER:
			delete _cursorHover;
			_cursorHover = new CBSprite(Game);
			if (!_cursorHover || FAILED(_cursorHover->loadFile((char *)params, ((CAdGame *)Game)->_texItemLifeTime))) {
				delete _cursorHover;
				_cursorHover = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CURSOR_COMBINED:
			parser.ScanStr((char *)params, "%b", &_cursorCombined);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.ScanStr((char *)params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.ScanStr((char *)params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
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
HRESULT CAdItem::update() {
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

	if (_sentence && _state != STATE_TALKING) _sentence->finish();

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
		_sentence->update();
		if (_sentence->_currentSprite) _tempSprite2 = _sentence->_currentSprite;

		bool TimeIsUp = (_sentence->_sound && _sentence->_soundStarted && (!_sentence->_sound->isPlaying() && !_sentence->_sound->isPaused())) || (!_sentence->_sound && _sentence->_duration <= Game->_timer - _sentence->_startTime);
		if (_tempSprite2 == NULL || _tempSprite2->_finished || (/*_tempSprite2->_looping &&*/ TimeIsUp)) {
			if (TimeIsUp) {
				_sentence->finish();
				_tempSprite2 = NULL;
				_state = STATE_READY;
			} else {
				_tempSprite2 = getTalkStance(_sentence->getNextStance());
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
	default:
	break;
	}
	_ready = (_state == STATE_READY);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::display(int X, int Y) {
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
			if (_amountString) Font->drawText((byte *)_amountString, AmountX, AmountY, Width, _amountAlign);
			else {
				char Str[256];
				sprintf(Str, "%d", _amount);
				Font->drawText((byte *)Str, AmountX, AmountY, Width, _amountAlign);
			}
		}
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetHoverSprite
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetHoverSprite") == 0) {
		stack->correctParams(1);

		bool SetCurrent = false;
		if (_currentSprite && _currentSprite == _spriteHover) SetCurrent = true;

		const char *Filename = stack->pop()->GetString();

		delete _spriteHover;
		_spriteHover = NULL;
		CBSprite *spr = new CBSprite(Game, this);
		if (!spr || FAILED(spr->loadFile(Filename))) {
			stack->pushBool(false);
			script->RuntimeError("Item.SetHoverSprite failed for file '%s'", Filename);
		} else {
			_spriteHover = spr;
			if (SetCurrent) _currentSprite = _spriteHover;
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHoverSprite") == 0) {
		stack->correctParams(0);

		if (!_spriteHover || !_spriteHover->_filename) stack->pushNULL();
		else stack->pushString(_spriteHover->_filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverSpriteObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHoverSpriteObject") == 0) {
		stack->correctParams(0);
		if (!_spriteHover) stack->pushNULL();
		else stack->pushNative(_spriteHover, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetNormalCursor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetNormalCursor") == 0) {
		stack->correctParams(1);

		const char *Filename = stack->pop()->GetString();

		delete _cursorNormal;
		_cursorNormal = NULL;
		CBSprite *spr = new CBSprite(Game);
		if (!spr || FAILED(spr->loadFile(Filename))) {
			stack->pushBool(false);
			script->RuntimeError("Item.SetNormalCursor failed for file '%s'", Filename);
		} else {
			_cursorNormal = spr;
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNormalCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetNormalCursor") == 0) {
		stack->correctParams(0);

		if (!_cursorNormal || !_cursorNormal->_filename) stack->pushNULL();
		else stack->pushString(_cursorNormal->_filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNormalCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetNormalCursorObject") == 0) {
		stack->correctParams(0);

		if (!_cursorNormal) stack->pushNULL();
		else stack->pushNative(_cursorNormal, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetHoverCursor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetHoverCursor") == 0) {
		stack->correctParams(1);

		const char *Filename = stack->pop()->GetString();

		delete _cursorHover;
		_cursorHover = NULL;
		CBSprite *spr = new CBSprite(Game);
		if (!spr || FAILED(spr->loadFile(Filename))) {
			stack->pushBool(false);
			script->RuntimeError("Item.SetHoverCursor failed for file '%s'", Filename);
		} else {
			_cursorHover = spr;
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHoverCursor") == 0) {
		stack->correctParams(0);

		if (!_cursorHover || !_cursorHover->_filename) stack->pushNULL();
		else stack->pushString(_cursorHover->_filename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHoverCursorObject") == 0) {
		stack->correctParams(0);

		if (!_cursorHover) stack->pushNULL();
		else stack->pushNative(_cursorHover, true);
		return S_OK;
	}

	else return CAdTalkHolder::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdItem::scGetProperty(const char *name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->SetString("item");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Name") == 0) {
		_scValue->SetString(_name);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisplayAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisplayAmount") == 0) {
		_scValue->SetBool(_displayAmount);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Amount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Amount") == 0) {
		_scValue->SetInt(_amount);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountOffsetX") == 0) {
		_scValue->SetInt(_amountOffsetX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountOffsetY") == 0) {
		_scValue->SetInt(_amountOffsetY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountAlign") == 0) {
		_scValue->SetInt(_amountAlign);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountString") == 0) {
		if (!_amountString) _scValue->SetNULL();
		else _scValue->SetString(_amountString);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorCombined
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorCombined") == 0) {
		_scValue->SetBool(_cursorCombined);
		return _scValue;
	}

	else return CAdTalkHolder::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisplayAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisplayAmount") == 0) {
		_displayAmount = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Amount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Amount") == 0) {
		_amount = value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountOffsetX") == 0) {
		_amountOffsetX = value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountOffsetY") == 0) {
		_amountOffsetY = value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountAlign") == 0) {
		_amountAlign = (TTextAlign)value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountString") == 0) {
		if (value->IsNULL()) {
			delete[] _amountString;
			_amountString = NULL;
		} else {
			CBUtils::SetString(&_amountString, value->GetString());
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorCombined
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorCombined") == 0) {
		_cursorCombined = value->GetBool();
		return S_OK;
	}

	else return CAdTalkHolder::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CAdItem::scToString() {
	return "[item]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdItem::persist(CBPersistMgr *persistMgr) {

	CAdTalkHolder::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_cursorCombined));
	persistMgr->transfer(TMEMBER(_cursorHover));
	persistMgr->transfer(TMEMBER(_cursorNormal));
	persistMgr->transfer(TMEMBER(_spriteHover));
	persistMgr->transfer(TMEMBER(_inInventory));
	persistMgr->transfer(TMEMBER(_displayAmount));
	persistMgr->transfer(TMEMBER(_amount));
	persistMgr->transfer(TMEMBER(_amountOffsetX));
	persistMgr->transfer(TMEMBER(_amountOffsetY));
	persistMgr->transfer(TMEMBER_INT(_amountAlign));
	persistMgr->transfer(TMEMBER(_amountString));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdItem::getExtendedFlag(const char *FlagName) {
	if (!FlagName) return false;
	else if (strcmp(FlagName, "usable") == 0) return true;
	else return CAdObject::getExtendedFlag(FlagName);
}

} // end of namespace WinterMute
