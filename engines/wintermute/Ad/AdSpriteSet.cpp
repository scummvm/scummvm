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
#include "engines/wintermute/Ad/AdSpriteSet.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFileManager.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdSpriteSet, false)

//////////////////////////////////////////////////////////////////////////
CAdSpriteSet::CAdSpriteSet(CBGame *inGame, CBObject *Owner): CBObject(inGame) {
	_owner = Owner;

	for (int i = 0; i < NUM_DIRECTIONS; i++)
		_sprites[i] = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdSpriteSet::~CAdSpriteSet() {
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		delete _sprites[i];
		_sprites[i] = NULL;
	}

	_owner = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSpriteSet::LoadFile(const char *Filename, int LifeTime, TSpriteCacheType CacheType) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdSpriteSet::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing SPRITESET file '%s'", Filename);

	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(SPRITESET)
TOKEN_DEF(NAME)
TOKEN_DEF(UP_LEFT)
TOKEN_DEF(DOWN_LEFT)
TOKEN_DEF(LEFT)
TOKEN_DEF(UP_RIGHT)
TOKEN_DEF(DOWN_RIGHT)
TOKEN_DEF(RIGHT)
TOKEN_DEF(UP)
TOKEN_DEF(DOWN)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdSpriteSet::LoadBuffer(byte  *Buffer, bool Complete, int LifeTime, TSpriteCacheType CacheType) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(SPRITESET)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(UP_LEFT)
	TOKEN_TABLE(DOWN_LEFT)
	TOKEN_TABLE(LEFT)
	TOKEN_TABLE(UP_RIGHT)
	TOKEN_TABLE(DOWN_RIGHT)
	TOKEN_TABLE(RIGHT)
	TOKEN_TABLE(UP)
	TOKEN_TABLE(DOWN)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_SPRITESET) {
			Game->LOG(0, "'SPRITESET' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	CBSprite *spr = NULL;
	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params, LifeTime, CacheType))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_LEFT:
			delete _sprites[DI_LEFT];
			_sprites[DI_LEFT] = NULL;
			spr = new CBSprite(Game, _owner);
			if (!spr || FAILED(spr->LoadFile((char *)params, LifeTime, CacheType))) cmd = PARSERR_GENERIC;
			else _sprites[DI_LEFT] = spr;
			break;

		case TOKEN_RIGHT:
			delete _sprites[DI_RIGHT];
			_sprites[DI_RIGHT] = NULL;
			spr = new CBSprite(Game, _owner);
			if (!spr || FAILED(spr->LoadFile((char *)params, LifeTime, CacheType))) cmd = PARSERR_GENERIC;
			else _sprites[DI_RIGHT] = spr;
			break;

		case TOKEN_UP:
			delete _sprites[DI_UP];
			_sprites[DI_UP] = NULL;
			spr = new CBSprite(Game, _owner);
			if (!spr || FAILED(spr->LoadFile((char *)params, LifeTime, CacheType))) cmd = PARSERR_GENERIC;
			else _sprites[DI_UP] = spr;
			break;

		case TOKEN_DOWN:
			delete _sprites[DI_DOWN];
			_sprites[DI_DOWN] = NULL;
			spr = new CBSprite(Game, _owner);
			if (!spr || FAILED(spr->LoadFile((char *)params, LifeTime, CacheType))) cmd = PARSERR_GENERIC;
			else _sprites[DI_DOWN] = spr;
			break;

		case TOKEN_UP_LEFT:
			delete _sprites[DI_UPLEFT];
			_sprites[DI_UPLEFT] = NULL;
			spr = new CBSprite(Game, _owner);
			if (!spr || FAILED(spr->LoadFile((char *)params, LifeTime, CacheType))) cmd = PARSERR_GENERIC;
			else _sprites[DI_UPLEFT] = spr;
			break;

		case TOKEN_UP_RIGHT:
			delete _sprites[DI_UPRIGHT];
			_sprites[DI_UPRIGHT] = NULL;
			spr = new CBSprite(Game, _owner);
			if (!spr || FAILED(spr->LoadFile((char *)params, LifeTime, CacheType))) cmd = PARSERR_GENERIC;
			else _sprites[DI_UPRIGHT] = spr;
			break;

		case TOKEN_DOWN_LEFT:
			delete _sprites[DI_DOWNLEFT];
			_sprites[DI_DOWNLEFT] = NULL;
			spr = new CBSprite(Game, _owner);
			if (!spr || FAILED(spr->LoadFile((char *)params, LifeTime, CacheType))) cmd = PARSERR_GENERIC;
			else _sprites[DI_DOWNLEFT] = spr;
			break;

		case TOKEN_DOWN_RIGHT:
			delete _sprites[DI_DOWNRIGHT];
			_sprites[DI_DOWNRIGHT] = NULL;
			spr = new CBSprite(Game, _owner);
			if (!spr || FAILED(spr->LoadFile((char *)params, LifeTime, CacheType))) cmd = PARSERR_GENERIC;
			else _sprites[DI_DOWNRIGHT] = spr;
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in SPRITESET definition");
		return E_FAIL;
	}

	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading SPRITESET definition");
		if (spr) delete spr;
		return E_FAIL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSpriteSet::persist(CBPersistMgr *persistMgr) {

	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_owner));
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		persistMgr->transfer("", &_sprites[i]);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CBSprite *CAdSpriteSet::GetSprite(TDirection Direction) {
	int Dir = (int)Direction;
	if (Dir < 0) Dir = 0;
	if (Dir >= NUM_DIRECTIONS) Dir = NUM_DIRECTIONS - 1;

	CBSprite *ret = NULL;

	// find nearest set sprite
	int i;
	int NumSteps = 0;
	for (i = Dir, NumSteps = 0; i >= 0; i--) {
		if (_sprites[i] != NULL) {
			ret = _sprites[i];
			NumSteps = Dir - i;
			break;
		}
	}

	for (i = Dir; i < NUM_DIRECTIONS; i++) {
		if (_sprites[i] != NULL) {
			if (ret == NULL || NumSteps > i - Dir) return _sprites[i];
			else return ret;
		}
	}

	return ret;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CAdSpriteSet::saveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "SPRITESET {\n");
	if (_name) Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		if (_sprites[i]) {
			switch (i) {
			case DI_UP:
				Buffer->PutTextIndent(Indent + 2, "UP=\"%s\"\n",         _sprites[i]->_filename);
				break;
			case DI_UPRIGHT:
				Buffer->PutTextIndent(Indent + 2, "UP_RIGHT=\"%s\"\n",   _sprites[i]->_filename);
				break;
			case DI_RIGHT:
				Buffer->PutTextIndent(Indent + 2, "RIGHT=\"%s\"\n",      _sprites[i]->_filename);
				break;
			case DI_DOWNRIGHT:
				Buffer->PutTextIndent(Indent + 2, "DOWN_RIGHT=\"%s\"\n", _sprites[i]->_filename);
				break;
			case DI_DOWN:
				Buffer->PutTextIndent(Indent + 2, "DOWN=\"%s\"\n",       _sprites[i]->_filename);
				break;
			case DI_DOWNLEFT:
				Buffer->PutTextIndent(Indent + 2, "DOWN_LEFT=\"%s\"\n",  _sprites[i]->_filename);
				break;
			case DI_LEFT:
				Buffer->PutTextIndent(Indent + 2, "LEFT=\"%s\"\n",       _sprites[i]->_filename);
				break;
			case DI_UPLEFT:
				Buffer->PutTextIndent(Indent + 2, "UP_LEFT=\"%s\"\n",    _sprites[i]->_filename);
				break;
			}
		}
	}

	CBBase::saveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CAdSpriteSet::ContainsSprite(CBSprite *Sprite) {
	if (!Sprite) return false;

	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		if (_sprites[i] == Sprite) return true;
	}
	return false;
}

} // end of namespace WinterMute
