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
#include "engines/wintermute/Ad/AdResponse.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BFontStorage.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/utils.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdResponse, false)

//////////////////////////////////////////////////////////////////////////
CAdResponse::CAdResponse(CBGame *inGame): CBObject(inGame) {
	_text = NULL;
	_textOrig = NULL;
	_icon = _iconHover = _iconPressed = NULL;
	_font = NULL;
	_iD = 0;
	_responseType = RESPONSE_ALWAYS;
}


//////////////////////////////////////////////////////////////////////////
CAdResponse::~CAdResponse() {
	delete[] _text;
	delete[] _textOrig;
	delete _icon;
	delete _iconHover;
	delete _iconPressed;
	_text = NULL;
	_textOrig = NULL;
	_icon = NULL;
	_iconHover = NULL;
	_iconPressed = NULL;
	if (_font) Game->_fontStorage->RemoveFont(_font);
}


//////////////////////////////////////////////////////////////////////////
void CAdResponse::SetText(const char *Text) {
	CBUtils::SetString(&_text, Text);
	CBUtils::SetString(&_textOrig, Text);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponse::SetIcon(const char *Filename) {
	delete _icon;
	_icon = new CBSprite(Game);
	if (!_icon || FAILED(_icon->LoadFile(Filename))) {
		Game->LOG(0, "CAdResponse::SetIcon failed for file '%s'", Filename);
		delete _icon;
		_icon = NULL;
		return E_FAIL;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponse::SetFont(const char *Filename) {
	if (_font) Game->_fontStorage->RemoveFont(_font);
	_font = Game->_fontStorage->AddFont(Filename);
	if (!_font) {
		Game->LOG(0, "CAdResponse::SetFont failed for file '%s'", Filename);
		return E_FAIL;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponse::SetIconHover(const char *Filename) {
	delete _iconHover;
	_iconHover = new CBSprite(Game);
	if (!_iconHover || FAILED(_iconHover->LoadFile(Filename))) {
		Game->LOG(0, "CAdResponse::SetIconHover failed for file '%s'", Filename);
		delete _iconHover;
		_iconHover = NULL;
		return E_FAIL;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponse::SetIconPressed(const char *Filename) {
	delete _iconPressed;
	_iconPressed = new CBSprite(Game);
	if (!_iconPressed || FAILED(_iconPressed->LoadFile(Filename))) {
		Game->LOG(0, "CAdResponse::SetIconPressed failed for file '%s'", Filename);
		delete _iconPressed;
		_iconPressed = NULL;
		return E_FAIL;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponse::Persist(CBPersistMgr *PersistMgr) {

	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_icon));
	PersistMgr->Transfer(TMEMBER(_iconHover));
	PersistMgr->Transfer(TMEMBER(_iconPressed));
	PersistMgr->Transfer(TMEMBER(_iD));
	PersistMgr->Transfer(TMEMBER(_text));
	PersistMgr->Transfer(TMEMBER(_textOrig));
	PersistMgr->Transfer(TMEMBER_INT(_responseType));
	PersistMgr->Transfer(TMEMBER(_font));

	return S_OK;
}

} // end of namespace WinterMute
