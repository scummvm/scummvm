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

#include "engines/wintermute/Ad/AdEntity.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/UI/UIEntity.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUIEntity, false)

//////////////////////////////////////////////////////////////////////////
CUIEntity::CUIEntity(CBGame *inGame): CUIObject(inGame) {
	_type = UI_CUSTOM;
	_entity = NULL;
}


//////////////////////////////////////////////////////////////////////////
CUIEntity::~CUIEntity() {
	if (_entity) Game->unregisterObject(_entity);
	_entity = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::loadFile(const char *filename) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CUIEntity::LoadFile failed for file '%s'", filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (FAILED(ret = loadBuffer(buffer, true))) Game->LOG(0, "Error parsing ENTITY container file '%s'", filename);


	delete [] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(ENTITY_CONTAINER)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(DISABLED)
TOKEN_DEF(VISIBLE)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(NAME)
TOKEN_DEF(ENTITY)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ENTITY_CONTAINER)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(DISABLED)
	TOKEN_TABLE(VISIBLE)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(ENTITY)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	CBParser parser(Game);

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_ENTITY_CONTAINER) {
			Game->LOG(0, "'ENTITY_CONTAINER' keyword expected.");
			return E_FAIL;
		}
		buffer = params;
	}

	while (cmd > 0 && (cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_X:
			parser.scanStr((char *)params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.scanStr((char *)params, "%d", &_posY);
			break;

		case TOKEN_DISABLED:
			parser.scanStr((char *)params, "%b", &_disable);
			break;

		case TOKEN_VISIBLE:
			parser.scanStr((char *)params, "%b", &_visible);
			break;

		case TOKEN_ENTITY:
			if (FAILED(setEntity((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in ENTITY_CONTAINER definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading ENTITY_CONTAINER definition");
		return E_FAIL;
	}

	correctSize();

	if (Game->_editorMode) {
		_width = 50;
		_height = 50;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::saveAsText(CBDynBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "ENTITY_CONTAINER\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);

	buffer->putTextIndent(indent + 2, "\n");

	buffer->putTextIndent(indent + 2, "X=%d\n", _posX);
	buffer->putTextIndent(indent + 2, "Y=%d\n", _posY);

	buffer->putTextIndent(indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");

	if (_entity && _entity->_filename)
		buffer->putTextIndent(indent + 2, "ENTITY=\"%s\"\n", _entity->_filename);

	buffer->putTextIndent(indent + 2, "\n");

	// scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	CBBase::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::setEntity(const char *filename) {
	if (_entity) Game->unregisterObject(_entity);
	_entity = new CAdEntity(Game);
	if (!_entity || FAILED(_entity->loadFile(filename))) {
		delete _entity;
		_entity = NULL;
		return E_FAIL;
	} else {
		_entity->_nonIntMouseEvents = true;
		_entity->_sceneIndependent = true;
		_entity->makeFreezable(false);
		Game->registerObject(_entity);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::display(int OffsetX, int OffsetY) {
	if (!_visible) return S_OK;

	if (_entity) {
		_entity->_posX = OffsetX + _posX;
		_entity->_posY = OffsetY + _posY;
		if (_entity->_scale < 0) _entity->_zoomable = false;
		_entity->_shadowable = false;

		_entity->update();

		bool OrigReg = _entity->_registrable;

		if (_entity->_registrable && _disable) _entity->_registrable = false;

		_entity->display();
		_entity->_registrable = OrigReg;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// GetEntity
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetEntity") == 0) {
		stack->correctParams(0);

		if (_entity) stack->pushNative(_entity, true);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetEntity") == 0) {
		stack->correctParams(1);

		const char *filename = stack->pop()->getString();

		if (SUCCEEDED(setEntity(filename)))
			stack->pushBool(true);
		else
			stack->pushBool(false);

		return S_OK;
	}

	else return CUIObject::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIEntity::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("entity container");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Freezable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Freezable") == 0) {
		if (_entity) _scValue->setBool(_entity->_freezable);
		else _scValue->setBool(false);
		return _scValue;
	}

	else return CUIObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Freezable
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Freezable") == 0) {
		if (_entity) _entity->makeFreezable(value->getBool());
		return S_OK;
	} else return CUIObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CUIEntity::scToString() {
	return "[entity container]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::persist(CBPersistMgr *persistMgr) {

	CUIObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_entity));
	return S_OK;
}

} // end of namespace WinterMute
