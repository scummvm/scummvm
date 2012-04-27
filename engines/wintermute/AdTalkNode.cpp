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
#include "AdTalkNode.h"
#include "BParser.h"
#include "BDynBuffer.h"
#include "BGame.h"
#include "BSprite.h"
#include "AdSpriteSet.h"
#include "utils.h"
namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdTalkNode, false)

//////////////////////////////////////////////////////////////////////////
CAdTalkNode::CAdTalkNode(CBGame *inGame): CBBase(inGame) {
	_sprite = NULL;
	_spriteFilename = NULL;
	_spriteSet = NULL;
	_spriteSetFilename = NULL;
	_comment = NULL;

	_startTime = _endTime = 0;
	_playToEnd = false;
	_preCache = false;
}


//////////////////////////////////////////////////////////////////////////
CAdTalkNode::~CAdTalkNode() {
	delete[] _spriteFilename;
	delete _sprite;
	delete[] _spriteSetFilename;
	delete _spriteSet;
	delete _comment;
	_spriteFilename = NULL;
	_sprite = NULL;
	_spriteSetFilename = NULL;
	_spriteSet = NULL;
	_comment = NULL;
}



TOKEN_DEF_START
TOKEN_DEF(ACTION)
TOKEN_DEF(SPRITESET_FILE)
TOKEN_DEF(SPRITESET)
TOKEN_DEF(SPRITE)
TOKEN_DEF(START_TIME)
TOKEN_DEF(END_TIME)
TOKEN_DEF(COMMENT)
TOKEN_DEF(PRECACHE)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkNode::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ACTION)
	TOKEN_TABLE(SPRITESET_FILE)
	TOKEN_TABLE(SPRITESET)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(START_TIME)
	TOKEN_TABLE(END_TIME)
	TOKEN_TABLE(COMMENT)
	TOKEN_TABLE(PRECACHE)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_ACTION) {
			Game->LOG(0, "'ACTION' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	_endTime = 0;
	_playToEnd = false;
	_preCache = false;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_SPRITE:
			CBUtils::SetString(&_spriteFilename, (char *)params);
			break;

		case TOKEN_SPRITESET_FILE:
			CBUtils::SetString(&_spriteSetFilename, (char *)params);
			break;

		case TOKEN_SPRITESET: {
			delete _spriteSet;
			_spriteSet = new CAdSpriteSet(Game);
			if (!_spriteSet || FAILED(_spriteSet->LoadBuffer(params, false))) {
				delete _spriteSet;
				_spriteSet = NULL;
				cmd = PARSERR_GENERIC;
			}
		}
		break;

		case TOKEN_START_TIME:
			parser.ScanStr((char *)params, "%d", &_startTime);
			break;

		case TOKEN_END_TIME:
			parser.ScanStr((char *)params, "%d", &_endTime);
			break;

		case TOKEN_PRECACHE:
			parser.ScanStr((char *)params, "%b", &_preCache);
			break;

		case TOKEN_COMMENT:
			if (Game->_editorMode) CBUtils::SetString(&_comment, (char *)params);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in ACTION definition");
		return E_FAIL;
	}

	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading ACTION definition");
		return E_FAIL;
	}

	if (_endTime == 0) _playToEnd = true;
	else _playToEnd = false;

	if (_preCache && _spriteFilename) {
		delete _sprite;
		_sprite = new CBSprite(Game);
		if (!_sprite || FAILED(_sprite->LoadFile(_spriteFilename))) return E_FAIL;
	}

	if (_preCache && _spriteSetFilename) {
		delete _spriteSet;
		_spriteSet = new CAdSpriteSet(Game);
		if (!_spriteSet || FAILED(_spriteSet->LoadFile(_spriteSetFilename))) return E_FAIL;
	}


	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkNode::Persist(CBPersistMgr *PersistMgr) {
	PersistMgr->Transfer(TMEMBER(_comment));
	PersistMgr->Transfer(TMEMBER(_startTime));
	PersistMgr->Transfer(TMEMBER(_endTime));
	PersistMgr->Transfer(TMEMBER(_playToEnd));
	PersistMgr->Transfer(TMEMBER(_sprite));
	PersistMgr->Transfer(TMEMBER(_spriteFilename));
	PersistMgr->Transfer(TMEMBER(_spriteSet));
	PersistMgr->Transfer(TMEMBER(_spriteSetFilename));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkNode::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "ACTION {\n");
	if (_comment) Buffer->PutTextIndent(Indent + 2, "COMMENT=\"%s\"\n", _comment);
	Buffer->PutTextIndent(Indent + 2, "START_TIME=%d\n", _startTime);
	if (!_playToEnd) Buffer->PutTextIndent(Indent + 2, "END_TIME=%d\n", _endTime);
	if (_spriteFilename) Buffer->PutTextIndent(Indent + 2, "SPRITE=\"%s\"\n", _spriteFilename);
	if (_spriteSetFilename) Buffer->PutTextIndent(Indent + 2, "SPRITESET_FILE=\"%s\"\n", _spriteSetFilename);
	else if (_spriteSet) _spriteSet->SaveAsText(Buffer, Indent + 2);
	if (_preCache) Buffer->PutTextIndent(Indent + 2, "PRECACHE=\"%s\"\n", _preCache ? "TRUE" : "FALSE");

	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkNode::LoadSprite() {
	if (_spriteFilename && !_sprite) {
		_sprite = new CBSprite(Game);
		if (!_sprite || FAILED(_sprite->LoadFile(_spriteFilename))) {
			delete _sprite;
			_sprite = NULL;
			return E_FAIL;
		} else return S_OK;
	}

	else if (_spriteSetFilename && !_spriteSet) {
		_spriteSet = new CAdSpriteSet(Game);
		if (!_spriteSet || FAILED(_spriteSet->LoadFile(_spriteSetFilename))) {
			delete _spriteSet;
			_spriteSet = NULL;
			return E_FAIL;
		} else return S_OK;
	}

	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdTalkNode::IsInTimeInterval(uint32 Time, TDirection Dir) {
	if (Time >= _startTime) {
		if (_playToEnd) {
			if ((_spriteFilename && _sprite == NULL) || (_sprite && _sprite->_finished == false)) return true;
			else if ((_spriteSetFilename && _spriteSet == NULL) || (_spriteSet && _spriteSet->GetSprite(Dir) && _spriteSet->GetSprite(Dir)->_finished == false)) return true;
			else return false;
		} else return _endTime >= Time;
	} else return false;
}


//////////////////////////////////////////////////////////////////////////
CBSprite *CAdTalkNode::GetSprite(TDirection Dir) {
	LoadSprite();
	if (_sprite) return _sprite;
	else if (_spriteSet) return _spriteSet->GetSprite(Dir);
	else return NULL;
}

} // end of namespace WinterMute
