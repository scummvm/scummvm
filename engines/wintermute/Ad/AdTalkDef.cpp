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
#include "engines/wintermute/Ad/AdTalkDef.h"
#include "engines/wintermute/Ad/AdTalkNode.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Ad/AdSpriteSet.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/utils/utils.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdTalkDef, false)

//////////////////////////////////////////////////////////////////////////
CAdTalkDef::CAdTalkDef(CBGame *inGame): CBObject(inGame) {
	_defaultSpriteFilename = NULL;
	_defaultSprite = NULL;

	_defaultSpriteSetFilename = NULL;
	_defaultSpriteSet = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdTalkDef::~CAdTalkDef() {
	for (int i = 0; i < _nodes.GetSize(); i++) delete _nodes[i];
	_nodes.RemoveAll();

	delete[] _defaultSpriteFilename;
	delete _defaultSprite;
	_defaultSpriteFilename = NULL;
	_defaultSprite = NULL;

	delete[] _defaultSpriteSetFilename;
	delete _defaultSpriteSet;
	_defaultSpriteSetFilename = NULL;
	_defaultSpriteSet = NULL;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdTalkDef::loadFile(const char *filename) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CAdTalkDef::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	ERRORCODE ret;

	CBUtils::setString(&_filename, filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) Game->LOG(0, "Error parsing TALK file '%s'", filename);

	delete [] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(TALK)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(ACTION)
TOKEN_DEF(DEFAULT_SPRITESET_FILE)
TOKEN_DEF(DEFAULT_SPRITESET)
TOKEN_DEF(DEFAULT_SPRITE)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdTalkDef::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(TALK)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(ACTION)
	TOKEN_TABLE(DEFAULT_SPRITESET_FILE)
	TOKEN_TABLE(DEFAULT_SPRITESET)
	TOKEN_TABLE(DEFAULT_SPRITE)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_TALK) {
			Game->LOG(0, "'TALK' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_ACTION: {
			CAdTalkNode *Node = new CAdTalkNode(Game);
			if (Node && DID_SUCCEED(Node->loadBuffer(params, false))) _nodes.Add(Node);
			else {
				delete Node;
				Node = NULL;
				cmd = PARSERR_GENERIC;
			}
		}
		break;

		case TOKEN_DEFAULT_SPRITE:
			CBUtils::setString(&_defaultSpriteFilename, (char *)params);
			break;

		case TOKEN_DEFAULT_SPRITESET_FILE:
			CBUtils::setString(&_defaultSpriteSetFilename, (char *)params);
			break;

		case TOKEN_DEFAULT_SPRITESET: {
			delete _defaultSpriteSet;
			_defaultSpriteSet = new CAdSpriteSet(Game);
			if (!_defaultSpriteSet || DID_FAIL(_defaultSpriteSet->loadBuffer(params, false))) {
				delete _defaultSpriteSet;
				_defaultSpriteSet = NULL;
				cmd = PARSERR_GENERIC;
			}
		}
		break;


		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in TALK definition");
		return STATUS_FAILED;
	}

	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading TALK definition");
		return STATUS_FAILED;
	}

	delete _defaultSprite;
	delete _defaultSpriteSet;
	_defaultSprite = NULL;
	_defaultSpriteSet = NULL;

	if (_defaultSpriteFilename) {
		_defaultSprite = new CBSprite(Game);
		if (!_defaultSprite || DID_FAIL(_defaultSprite->loadFile(_defaultSpriteFilename))) return STATUS_FAILED;
	}

	if (_defaultSpriteSetFilename) {
		_defaultSpriteSet = new CAdSpriteSet(Game);
		if (!_defaultSpriteSet || DID_FAIL(_defaultSpriteSet->loadFile(_defaultSpriteSetFilename))) return STATUS_FAILED;
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdTalkDef::persist(CBPersistMgr *persistMgr) {

	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_defaultSprite));
	persistMgr->transfer(TMEMBER(_defaultSpriteFilename));
	persistMgr->transfer(TMEMBER(_defaultSpriteSet));
	persistMgr->transfer(TMEMBER(_defaultSpriteSetFilename));

	_nodes.persist(persistMgr);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdTalkDef::saveAsText(CBDynBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "TALK {\n");
	if (_defaultSpriteFilename) buffer->putTextIndent(indent + 2, "DEFAULT_SPRITE=\"%s\"\n", _defaultSpriteFilename);

	if (_defaultSpriteSetFilename) buffer->putTextIndent(indent + 2, "DEFAULT_SPRITESET_FILE=\"%s\"\n", _defaultSpriteSetFilename);
	else if (_defaultSpriteSet) _defaultSpriteSet->saveAsText(buffer, indent + 2);

	for (int i = 0; i < _nodes.GetSize(); i++) {
		_nodes[i]->saveAsText(buffer, indent + 2);
		buffer->putTextIndent(indent, "\n");
	}
	CBBase::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdTalkDef::loadDefaultSprite() {
	if (_defaultSpriteFilename && !_defaultSprite) {
		_defaultSprite = new CBSprite(Game);
		if (!_defaultSprite || DID_FAIL(_defaultSprite->loadFile(_defaultSpriteFilename))) {
			delete _defaultSprite;
			_defaultSprite = NULL;
			return STATUS_FAILED;
		} else return STATUS_OK;
	} else if (_defaultSpriteSetFilename && !_defaultSpriteSet) {
		_defaultSpriteSet = new CAdSpriteSet(Game);
		if (!_defaultSpriteSet || DID_FAIL(_defaultSpriteSet->loadFile(_defaultSpriteSetFilename))) {
			delete _defaultSpriteSet;
			_defaultSpriteSet = NULL;
			return STATUS_FAILED;
		} else return STATUS_OK;
	} else return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
CBSprite *CAdTalkDef::getDefaultSprite(TDirection dir) {
	loadDefaultSprite();
	if (_defaultSprite) return _defaultSprite;
	else if (_defaultSpriteSet) return _defaultSpriteSet->getSprite(dir);
	else return NULL;
}

} // end of namespace WinterMute
