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

#include "engines/wintermute/AdEntity.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/UIEntity.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BDynBuffer.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUIEntity, false)

//////////////////////////////////////////////////////////////////////////
CUIEntity::CUIEntity(CBGame *inGame): CUIObject(inGame) {
	m_Type = UI_CUSTOM;
	m_Entity = NULL;
}


//////////////////////////////////////////////////////////////////////////
CUIEntity::~CUIEntity() {
	if (m_Entity) Game->UnregisterObject(m_Entity);
	m_Entity = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUIEntity::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

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
			parser.ScanStr((char *)params, "%d", &m_PosX);
			break;

		case TOKEN_Y:
			parser.ScanStr((char *)params, "%d", &m_PosY);
			break;

		case TOKEN_DISABLED:
			parser.ScanStr((char *)params, "%b", &m_Disable);
			break;

		case TOKEN_VISIBLE:
			parser.ScanStr((char *)params, "%b", &m_Visible);
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

	if (Game->m_EditorMode) {
		m_Width = 50;
		m_Height = 50;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "ENTITY_CONTAINER\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", m_Name);

	Buffer->PutTextIndent(Indent + 2, "\n");

	Buffer->PutTextIndent(Indent + 2, "X=%d\n", m_PosX);
	Buffer->PutTextIndent(Indent + 2, "Y=%d\n", m_PosY);

	Buffer->PutTextIndent(Indent + 2, "DISABLED=%s\n", m_Disable ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "VISIBLE=%s\n", m_Visible ? "TRUE" : "FALSE");

	if (m_Entity && m_Entity->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "ENTITY=\"%s\"\n", m_Entity->m_Filename);

	Buffer->PutTextIndent(Indent + 2, "\n");

	// scripts
	for (int i = 0; i < m_Scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", m_Scripts[i]->m_Filename);
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::SetEntity(char *Filename) {
	if (m_Entity) Game->UnregisterObject(m_Entity);
	m_Entity = new CAdEntity(Game);
	if (!m_Entity || FAILED(m_Entity->LoadFile(Filename))) {
		delete m_Entity;
		m_Entity = NULL;
		return E_FAIL;
	} else {
		m_Entity->m_NonIntMouseEvents = true;
		m_Entity->m_SceneIndependent = true;
		m_Entity->MakeFreezable(false);
		Game->RegisterObject(m_Entity);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::Display(int OffsetX, int OffsetY) {
	if (!m_Visible) return S_OK;

	if (m_Entity) {
		m_Entity->m_PosX = OffsetX + m_PosX;
		m_Entity->m_PosY = OffsetY + m_PosY;
		if (m_Entity->m_Scale < 0) m_Entity->m_Zoomable = false;
		m_Entity->m_Shadowable = false;

		m_Entity->Update();

		bool OrigReg = m_Entity->m_Registrable;

		if (m_Entity->m_Registrable && m_Disable) m_Entity->m_Registrable = false;

		m_Entity->Display();
		m_Entity->m_Registrable = OrigReg;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GetEntity
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetEntity") == 0) {
		Stack->CorrectParams(0);

		if (m_Entity) Stack->PushNative(m_Entity, true);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetEntity") == 0) {
		Stack->CorrectParams(1);

		char *Filename = Stack->Pop()->GetString();

		if (SUCCEEDED(SetEntity(Filename)))
			Stack->PushBool(true);
		else
			Stack->PushBool(false);

		return S_OK;
	}

	else return CUIObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIEntity::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("entity container");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Freezable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Freezable") == 0) {
		if (m_Entity) m_ScValue->SetBool(m_Entity->m_Freezable);
		else m_ScValue->SetBool(false);
		return m_ScValue;
	}

	else return CUIObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Freezable
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Freezable") == 0) {
		if (m_Entity) m_Entity->MakeFreezable(Value->GetBool());
		return S_OK;
	} else return CUIObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CUIEntity::ScToString() {
	return "[entity container]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEntity::Persist(CBPersistMgr *PersistMgr) {

	CUIObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Entity));
	return S_OK;
}

} // end of namespace WinterMute
