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
	if (_entity) Game->UnregisterObject(_entity);
	_entity = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::LoadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUIEntity::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing ENTITY container file '%s'", Filename);


	delete [] Buffer;

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
HRESULT CUIEntity::LoadBuffer(byte  *Buffer, bool Complete) {
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

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_ENTITY_CONTAINER) {
			Game->LOG(0, "'ENTITY_CONTAINER' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while (cmd > 0 && (cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_X:
			parser.ScanStr((char *)params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.ScanStr((char *)params, "%d", &_posY);
			break;

		case TOKEN_DISABLED:
			parser.ScanStr((char *)params, "%b", &_disable);
			break;

		case TOKEN_VISIBLE:
			parser.ScanStr((char *)params, "%b", &_visible);
			break;

		case TOKEN_ENTITY:
			if (FAILED(SetEntity((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
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

	CorrectSize();

	if (Game->_editorMode) {
		_width = 50;
		_height = 50;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "ENTITY_CONTAINER\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);

	Buffer->PutTextIndent(Indent + 2, "\n");

	Buffer->PutTextIndent(Indent + 2, "X=%d\n", _posX);
	Buffer->PutTextIndent(Indent + 2, "Y=%d\n", _posY);

	Buffer->PutTextIndent(Indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");

	if (_entity && _entity->_filename)
		Buffer->PutTextIndent(Indent + 2, "ENTITY=\"%s\"\n", _entity->_filename);

	Buffer->PutTextIndent(Indent + 2, "\n");

	// scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::SetEntity(const char *Filename) {
	if (_entity) Game->UnregisterObject(_entity);
	_entity = new CAdEntity(Game);
	if (!_entity || FAILED(_entity->LoadFile(Filename))) {
		delete _entity;
		_entity = NULL;
		return E_FAIL;
	} else {
		_entity->_nonIntMouseEvents = true;
		_entity->_sceneIndependent = true;
		_entity->MakeFreezable(false);
		Game->RegisterObject(_entity);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::Display(int OffsetX, int OffsetY) {
	if (!_visible) return S_OK;

	if (_entity) {
		_entity->_posX = OffsetX + _posX;
		_entity->_posY = OffsetY + _posY;
		if (_entity->_scale < 0) _entity->_zoomable = false;
		_entity->_shadowable = false;

		_entity->Update();

		bool OrigReg = _entity->_registrable;

		if (_entity->_registrable && _disable) _entity->_registrable = false;

		_entity->Display();
		_entity->_registrable = OrigReg;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GetEntity
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetEntity") == 0) {
		Stack->CorrectParams(0);

		if (_entity) Stack->PushNative(_entity, true);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetEntity") == 0) {
		Stack->CorrectParams(1);

		const char *Filename = Stack->Pop()->GetString();

		if (SUCCEEDED(SetEntity(Filename)))
			Stack->PushBool(true);
		else
			Stack->PushBool(false);

		return S_OK;
	}

	else return CUIObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIEntity::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("entity container");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Freezable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Freezable") == 0) {
		if (_entity) _scValue->SetBool(_entity->_freezable);
		else _scValue->SetBool(false);
		return _scValue;
	}

	else return CUIObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::ScSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Freezable
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Freezable") == 0) {
		if (_entity) _entity->MakeFreezable(Value->GetBool());
		return S_OK;
	} else return CUIObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CUIEntity::ScToString() {
	return "[entity container]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::Persist(CBPersistMgr *PersistMgr) {

	CUIObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_entity));
	return S_OK;
}

} // end of namespace WinterMute
