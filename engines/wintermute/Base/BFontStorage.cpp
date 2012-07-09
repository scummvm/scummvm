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
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(CBFontStorage, true)

//////////////////////////////////////////////////////////////////////////
CBFontStorage::CBFontStorage(CBGame *inGame): CBBase(inGame) {
}

//////////////////////////////////////////////////////////////////////////
CBFontStorage::~CBFontStorage() {
	cleanup(true);
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CBFontStorage::cleanup(bool warn) {
	for (int i = 0; i < _fonts.getSize(); i++) {
		if (warn) Game->LOG(0, "Removing orphan font '%s'", _fonts[i]->_filename);
		delete _fonts[i];
	}
	_fonts.removeAll();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CBFontStorage::initLoop() {
	for (int i = 0; i < _fonts.getSize(); i++) {
		_fonts[i]->initLoop();
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
CBFont *CBFontStorage::addFont(const char *filename) {
	if (!filename) return NULL;

	for (int i = 0; i < _fonts.getSize(); i++) {
		if (scumm_stricmp(_fonts[i]->_filename, filename) == 0) {
			_fonts[i]->_refCount++;
			return _fonts[i];
		}
	}

	/*
	CBFont* font = new CBFont(Game);
	if (!font) return NULL;

	if (DID_FAIL(font->loadFile(filename))) {
	    delete font;
	    return NULL;
	}
	else {
	    font->_refCount = 1;
	    _fonts.add(font);
	    return font;
	}
	*/
	CBFont *font = CBFont::createFromFile(Game, filename);
	if (font) {
		font->_refCount = 1;
		_fonts.add(font);
	}
	return font;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBFontStorage::removeFont(CBFont *font) {
	if (!font) return STATUS_FAILED;

	for (int i = 0; i < _fonts.getSize(); i++) {
		if (_fonts[i] == font) {
			_fonts[i]->_refCount--;
			if (_fonts[i]->_refCount <= 0) {
				delete _fonts[i];
				_fonts.removeAt(i);
			}
			break;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBFontStorage::persist(CBPersistMgr *persistMgr) {

	if (!persistMgr->_saving) cleanup(false);

	persistMgr->transfer(TMEMBER(Game));
	_fonts.persist(persistMgr);

	return STATUS_OK;
}

} // end of namespace WinterMute
