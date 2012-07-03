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
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Ad/AdNodeState.h"
#include "engines/wintermute/Ad/AdEntity.h"
#include "engines/wintermute/Base/BStringTable.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdNodeState, false)


//////////////////////////////////////////////////////////////////////////
CAdNodeState::CAdNodeState(CBGame *inGame): CBBase(inGame) {
	_name = NULL;
	_active = false;
	for (int i = 0; i < 7; i++) _caption[i] = NULL;
	_alphaColor = 0;
	_filename = NULL;
	_cursor = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdNodeState::~CAdNodeState() {
	delete[] _name;
	delete[] _filename;
	delete[] _cursor;
	_name = NULL;
	_filename = NULL;
	_cursor = NULL;
	for (int i = 0; i < 7; i++) {
		delete[] _caption[i];
		_caption[i] = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdNodeState::setName(const char *name) {
	delete[] _name;
	_name = NULL;
	CBUtils::SetString(&_name, name);
}


//////////////////////////////////////////////////////////////////////////
void CAdNodeState::setFilename(const char *Filename) {
	delete[] _filename;
	_filename = NULL;
	CBUtils::SetString(&_filename, Filename);
}


//////////////////////////////////////////////////////////////////////////
void CAdNodeState::SetCursor(const char *Filename) {
	delete[] _cursor;
	_cursor = NULL;
	CBUtils::SetString(&_cursor, Filename);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdNodeState::persist(CBPersistMgr *persistMgr) {
	persistMgr->transfer(TMEMBER(Game));

	persistMgr->transfer(TMEMBER(_active));
	persistMgr->transfer(TMEMBER(_name));
	persistMgr->transfer(TMEMBER(_filename));
	persistMgr->transfer(TMEMBER(_cursor));
	persistMgr->transfer(TMEMBER(_alphaColor));
	for (int i = 0; i < 7; i++) persistMgr->transfer(TMEMBER(_caption[i]));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdNodeState::setCaption(const char *Caption, int Case) {
	if (Case == 0) Case = 1;
	if (Case < 1 || Case > 7) return;

	delete[] _caption[Case - 1];
	_caption[Case - 1] = new char[strlen(Caption) + 1];
	if (_caption[Case - 1]) {
		strcpy(_caption[Case - 1], Caption);
		Game->_stringTable->Expand(&_caption[Case - 1]);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CAdNodeState::getCaption(int Case) {
	if (Case == 0) Case = 1;
	if (Case < 1 || Case > 7 || _caption[Case - 1] == NULL) return "";
	else return _caption[Case - 1];
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdNodeState::TransferEntity(CAdEntity *Entity, bool IncludingSprites, bool Saving) {
	if (!Entity) return E_FAIL;

	// hack!
	if (this->Game != Entity->Game) this->Game = Entity->Game;

	if (Saving) {
		for (int i = 0; i < 7; i++) {
			if (Entity->_caption[i]) setCaption(Entity->_caption[i], i);
		}
		if (!Entity->_region && Entity->_sprite && Entity->_sprite->_filename) {
			if (IncludingSprites) setFilename(Entity->_sprite->_filename);
			else setFilename("");
		}
		if (Entity->_cursor && Entity->_cursor->_filename) SetCursor(Entity->_cursor->_filename);
		_alphaColor = Entity->_alphaColor;
		_active = Entity->_active;
	} else {
		for (int i = 0; i < 7; i++) {
			if (_caption[i]) Entity->setCaption(_caption[i], i);
		}
		if (_filename && !Entity->_region && IncludingSprites && strcmp(_filename, "") != 0) {
			if (!Entity->_sprite || !Entity->_sprite->_filename || scumm_stricmp(Entity->_sprite->_filename, _filename) != 0)
				Entity->setSprite(_filename);
		}
		if (_cursor) {
			if (!Entity->_cursor || !Entity->_cursor->_filename || scumm_stricmp(Entity->_cursor->_filename, _cursor) != 0)
				Entity->setCursor(_cursor);
		}

		Entity->_active = _active;
		Entity->_alphaColor = _alphaColor;
	}

	return S_OK;
}

} // end of namespace WinterMute
