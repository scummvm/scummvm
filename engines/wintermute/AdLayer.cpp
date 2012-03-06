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
#include "BGame.h"
#include "AdLayer.h"
#include "AdSceneNode.h"
#include "BParser.h"
#include "BDynBuffer.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "BFileManager.h"
#include "PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {
    
IMPLEMENT_PERSISTENT(CAdLayer, false)

//////////////////////////////////////////////////////////////////////////
CAdLayer::CAdLayer(CBGame *inGame): CBObject(inGame) {
	m_Main = false;
	m_Width = m_Height = 0;
	m_Active = true;
	m_CloseUp = false;
}


//////////////////////////////////////////////////////////////////////////
CAdLayer::~CAdLayer() {
	for (int i = 0; i < m_Nodes.GetSize(); i++)
		delete m_Nodes[i];
	m_Nodes.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdLayer::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing LAYER file '%s'", Filename);


	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(LAYER)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(WIDTH)
TOKEN_DEF(HEIGHT)
TOKEN_DEF(MAIN)
TOKEN_DEF(ENTITY)
TOKEN_DEF(REGION)
TOKEN_DEF(ACTIVE)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(CLOSE_UP)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(LAYER)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(WIDTH)
	TOKEN_TABLE(HEIGHT)
	TOKEN_TABLE(MAIN)
	TOKEN_TABLE(ENTITY)
	TOKEN_TABLE(REGION)
	TOKEN_TABLE(ACTIVE)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(CLOSE_UP)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_LAYER) {
			Game->LOG(0, "'LAYER' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_CAPTION:
			SetCaption((char *)params);
			break;

		case TOKEN_MAIN:
			parser.ScanStr((char *)params, "%b", &m_Main);
			break;

		case TOKEN_CLOSE_UP:
			parser.ScanStr((char *)params, "%b", &m_CloseUp);
			break;

		case TOKEN_WIDTH:
			parser.ScanStr((char *)params, "%d", &m_Width);
			break;

		case TOKEN_HEIGHT:
			parser.ScanStr((char *)params, "%d", &m_Height);
			break;

		case TOKEN_ACTIVE:
			parser.ScanStr((char *)params, "%b", &m_Active);
			break;

		case TOKEN_REGION: {
			CAdRegion *region = new CAdRegion(Game);
			CAdSceneNode *node = new CAdSceneNode(Game);
			if (!region || !node || FAILED(region->LoadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete region;
				delete node;
				region = NULL;
				node = NULL;
			} else {
				node->SetRegion(region);
				m_Nodes.Add(node);
			}
		}
		break;

		case TOKEN_ENTITY: {
			CAdEntity *entity = new CAdEntity(Game);
			CAdSceneNode *node = new CAdSceneNode(Game);
			if (entity) entity->m_Zoomable = false; // scene entites default to NOT zoom
			if (!entity || !node || FAILED(entity->LoadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete entity;
				delete node;
				entity = NULL;
				node = NULL;
			} else {
				node->SetEntity(entity);
				m_Nodes.Add(node);
			}
		}
		break;

		case TOKEN_EDITOR_SELECTED:
			parser.ScanStr((char *)params, "%b", &m_EditorSelected);
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_PROPERTY:
			ParseProperty(params, false);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in LAYER definition");
		return E_FAIL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GetNode
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetNode") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		int node = -1;

		if (val->m_Type == VAL_INT) node = val->GetInt();
		else { // get by name
			for (int i = 0; i < m_Nodes.GetSize(); i++) {
				if ((m_Nodes[i]->m_Type == OBJECT_ENTITY && scumm_stricmp(m_Nodes[i]->m_Entity->m_Name, val->GetString()) == 0) ||
				        (m_Nodes[i]->m_Type == OBJECT_REGION && scumm_stricmp(m_Nodes[i]->m_Region->m_Name, val->GetString()) == 0)) {
					node = i;
					break;
				}
			}
		}

		if (node < 0 || node >= m_Nodes.GetSize()) Stack->PushNULL();
		else {
			switch (m_Nodes[node]->m_Type) {
			case OBJECT_ENTITY:
				Stack->PushNative(m_Nodes[node]->m_Entity, true);
				break;
			case OBJECT_REGION:
				Stack->PushNative(m_Nodes[node]->m_Region, true);
				break;
			default:
				Stack->PushNULL();
			}
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddRegion / AddEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddRegion") == 0 || strcmp(Name, "AddEntity") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdSceneNode *Node = new CAdSceneNode(Game);
		if (strcmp(Name, "AddRegion") == 0) {
			CAdRegion *Region = new CAdRegion(Game);
			if (!Val->IsNULL()) Region->SetName(Val->GetString());
			Node->SetRegion(Region);
			Stack->PushNative(Region, true);
		} else {
			CAdEntity *Entity = new CAdEntity(Game);
			if (!Val->IsNULL()) Entity->SetName(Val->GetString());
			Node->SetEntity(Entity);
			Stack->PushNative(Entity, true);
		}
		m_Nodes.Add(Node);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertRegion / InsertEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InsertRegion") == 0 || strcmp(Name, "InsertEntity") == 0) {
		Stack->CorrectParams(2);
		int Index = Stack->Pop()->GetInt();
		CScValue *Val = Stack->Pop();

		CAdSceneNode *Node = new CAdSceneNode(Game);
		if (strcmp(Name, "InsertRegion") == 0) {
			CAdRegion *Region = new CAdRegion(Game);
			if (!Val->IsNULL()) Region->SetName(Val->GetString());
			Node->SetRegion(Region);
			Stack->PushNative(Region, true);
		} else {
			CAdEntity *Entity = new CAdEntity(Game);
			if (!Val->IsNULL()) Entity->SetName(Val->GetString());
			Node->SetEntity(Entity);
			Stack->PushNative(Entity, true);
		}
		if (Index < 0) Index = 0;
		if (Index <= m_Nodes.GetSize() - 1) m_Nodes.InsertAt(Index, Node);
		else m_Nodes.Add(Node);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteNode") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdSceneNode *ToDelete = NULL;
		if (Val->IsNative()) {
			CBScriptable *Temp = Val->GetNative();
			for (int i = 0; i < m_Nodes.GetSize(); i++) {
				if (m_Nodes[i]->m_Region == Temp || m_Nodes[i]->m_Entity == Temp) {
					ToDelete = m_Nodes[i];
					break;
				}
			}
		} else {
			int Index = Val->GetInt();
			if (Index >= 0 && Index < m_Nodes.GetSize()) {
				ToDelete = m_Nodes[Index];
			}
		}
		if (ToDelete == NULL) {
			Stack->PushBool(false);
			return S_OK;
		}

		for (int i = 0; i < m_Nodes.GetSize(); i++) {
			if (m_Nodes[i] == ToDelete) {
				delete m_Nodes[i];
				m_Nodes[i] = NULL;
				m_Nodes.RemoveAt(i);
				break;
			}
		}
		Stack->PushBool(true);
		return S_OK;
	}

	else return CBObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdLayer::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("layer");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumNodes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumNodes") == 0) {
		m_ScValue->SetInt(m_Nodes.GetSize());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Width") == 0) {
		m_ScValue->SetInt(m_Width);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		m_ScValue->SetInt(m_Height);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Main (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Main") == 0) {
		m_ScValue->SetBool(m_Main);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CloseUp
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CloseUp") == 0) {
		m_ScValue->SetBool(m_CloseUp);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Active") == 0) {
		m_ScValue->SetBool(m_Active);
		return m_ScValue;
	}

	else return CBObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
		SetName(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CloseUp
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CloseUp") == 0) {
		m_CloseUp = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Width") == 0) {
		m_Width = Value->GetInt();
		if (m_Width < 0) m_Width = 0;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		m_Height = Value->GetInt();
		if (m_Height < 0) m_Height = 0;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Active") == 0) {
		bool b = Value->GetBool();
		if (b == false && m_Main) {
			Game->LOG(0, "Warning: cannot deactivate scene's main layer");
		} else m_Active = b;
		return S_OK;
	}

	else return CBObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CAdLayer::ScToString() {
	return "[layer]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "LAYER {\n");
	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", m_Name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());
	Buffer->PutTextIndent(Indent + 2, "MAIN=%s\n", m_Main ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "WIDTH=%d\n", m_Width);
	Buffer->PutTextIndent(Indent + 2, "HEIGHT=%d\n", m_Height);
	Buffer->PutTextIndent(Indent + 2, "ACTIVE=%s\n", m_Active ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SELECTED=%s\n", m_EditorSelected ? "TRUE" : "FALSE");
	if (m_CloseUp)
		Buffer->PutTextIndent(Indent + 2, "CLOSE_UP=%s\n", m_CloseUp ? "TRUE" : "FALSE");

	int i;

	for (i = 0; i < m_Scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", m_Scripts[i]->m_Filename);
	}

	if (m_ScProp) m_ScProp->SaveAsText(Buffer, Indent + 2);

	for (i = 0; i < m_Nodes.GetSize(); i++) {
		switch (m_Nodes[i]->m_Type) {
		case OBJECT_ENTITY:
			m_Nodes[i]->m_Entity->SaveAsText(Buffer, Indent + 2);
			break;
		case OBJECT_REGION:
			m_Nodes[i]->m_Region->SaveAsText(Buffer, Indent + 2);
			break;
		}
	}

	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::Persist(CBPersistMgr *PersistMgr) {

	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Active));
	PersistMgr->Transfer(TMEMBER(m_CloseUp));
	PersistMgr->Transfer(TMEMBER(m_Height));
	PersistMgr->Transfer(TMEMBER(m_Main));
	m_Nodes.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_Width));

	return S_OK;
}

} // end of namespace WinterMute
