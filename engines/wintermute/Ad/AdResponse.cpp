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
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/utils/utils.h"

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
	if (_font) Game->_fontStorage->removeFont(_font);
}


//////////////////////////////////////////////////////////////////////////
void CAdResponse::setText(const char *text) {
	CBUtils::setString(&_text, text);
	CBUtils::setString(&_textOrig, text);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponse::setIcon(const char *filename) {
	delete _icon;
	_icon = new CBSprite(Game);
	if (!_icon || DID_FAIL(_icon->loadFile(filename))) {
		Game->LOG(0, "CAdResponse::setIcon failed for file '%s'", filename);
		delete _icon;
		_icon = NULL;
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponse::setFont(const char *filename) {
	if (_font) Game->_fontStorage->removeFont(_font);
	_font = Game->_fontStorage->addFont(filename);
	if (!_font) {
		Game->LOG(0, "CAdResponse::setFont failed for file '%s'", filename);
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponse::setIconHover(const char *filename) {
	delete _iconHover;
	_iconHover = new CBSprite(Game);
	if (!_iconHover || DID_FAIL(_iconHover->loadFile(filename))) {
		Game->LOG(0, "CAdResponse::setIconHover failed for file '%s'", filename);
		delete _iconHover;
		_iconHover = NULL;
		return STATUS_FAILED;
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponse::setIconPressed(const char *filename) {
	delete _iconPressed;
	_iconPressed = new CBSprite(Game);
	if (!_iconPressed || DID_FAIL(_iconPressed->loadFile(filename))) {
		Game->LOG(0, "CAdResponse::setIconPressed failed for file '%s'", filename);
		delete _iconPressed;
		_iconPressed = NULL;
		return STATUS_FAILED;
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponse::persist(CBPersistMgr *persistMgr) {

	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_icon));
	persistMgr->transfer(TMEMBER(_iconHover));
	persistMgr->transfer(TMEMBER(_iconPressed));
	persistMgr->transfer(TMEMBER(_iD));
	persistMgr->transfer(TMEMBER(_text));
	persistMgr->transfer(TMEMBER(_textOrig));
	persistMgr->transfer(TMEMBER_INT(_responseType));
	persistMgr->transfer(TMEMBER(_font));

	return STATUS_OK;
}

} // end of namespace WinterMute
