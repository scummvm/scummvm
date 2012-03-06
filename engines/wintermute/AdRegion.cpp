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
#include "AdRegion.h"
#include "BParser.h"
#include "BDynBuffer.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "BGame.h"
#include "BFileManager.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdRegion, false)

//////////////////////////////////////////////////////////////////////////
CAdRegion::CAdRegion(CBGame *inGame): CBRegion(inGame) {
	m_Blocked = false;
	m_Decoration = false;
	m_Zoom = 0;
	m_Alpha = 0xFFFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
CAdRegion::~CAdRegion() {
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdRegion::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdRegion::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing REGION file '%s'", Filename);


	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(REGION)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(ACTIVE)
TOKEN_DEF(ZOOM)
TOKEN_DEF(SCALE)
TOKEN_DEF(BLOCKED)
TOKEN_DEF(DECORATION)
TOKEN_DEF(POINT)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(EDITOR_SELECTED_POINT)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdRegion::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(REGION)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(ACTIVE)
	TOKEN_TABLE(ZOOM)
	TOKEN_TABLE(SCALE)
	TOKEN_TABLE(BLOCKED)
	TOKEN_TABLE(DECORATION)
	TOKEN_TABLE(POINT)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(EDITOR_SELECTED_POINT)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_REGION) {
			Game->LOG(0, "'REGION' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	int i;

	for (i = 0; i < m_Points.GetSize(); i++) delete m_Points[i];
	m_Points.RemoveAll();

	int ar = 255, ag = 255, ab = 255, alpha = 255;

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

		case TOKEN_ACTIVE:
			parser.ScanStr((char *)params, "%b", &m_Active);
			break;

		case TOKEN_BLOCKED:
			parser.ScanStr((char *)params, "%b", &m_Blocked);
			break;

		case TOKEN_DECORATION:
			parser.ScanStr((char *)params, "%b", &m_Decoration);
			break;

		case TOKEN_ZOOM:
		case TOKEN_SCALE: {
			int i;
			parser.ScanStr((char *)params, "%d", &i);
			m_Zoom = (float)i;
		}
		break;

		case TOKEN_POINT: {
			int x, y;
			parser.ScanStr((char *)params, "%d,%d", &x, &y);
			m_Points.Add(new CBPoint(x, y));
		}
		break;

		case TOKEN_ALPHA_COLOR:
			parser.ScanStr((char *)params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.ScanStr((char *)params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_SELECTED:
			parser.ScanStr((char *)params, "%b", &m_EditorSelected);
			break;

		case TOKEN_EDITOR_SELECTED_POINT:
			parser.ScanStr((char *)params, "%d", &m_EditorSelectedPoint);
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
		Game->LOG(0, "Syntax error in REGION definition");
		return E_FAIL;
	}

	CreateRegion();

	m_Alpha = DRGBA(ar, ag, ab, alpha);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdRegion::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	/*
	    //////////////////////////////////////////////////////////////////////////
	    // SkipTo
	    //////////////////////////////////////////////////////////////////////////
	    if(strcmp(Name, "SkipTo")==0){
	        Stack->CorrectParams(2);
	        m_PosX = Stack->Pop()->GetInt();
	        m_PosY = Stack->Pop()->GetInt();
	        Stack->PushNULL();

	        return S_OK;
	    }

	    else*/ return CBRegion::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdRegion::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("ad region");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Name") == 0) {
		m_ScValue->SetString(m_Name);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Blocked
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Blocked") == 0) {
		m_ScValue->SetBool(m_Blocked);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Decoration
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Decoration") == 0) {
		m_ScValue->SetBool(m_Decoration);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scale") == 0) {
		m_ScValue->SetFloat(m_Zoom);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AlphaColor") == 0) {
		m_ScValue->SetInt((int)m_Alpha);
		return m_ScValue;
	}

	else return CBRegion::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdRegion::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
		SetName(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Blocked
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Blocked") == 0) {
		m_Blocked = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Decoration
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Decoration") == 0) {
		m_Decoration = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scale") == 0) {
		m_Zoom = Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AlphaColor") == 0) {
		m_Alpha = (uint32)Value->GetInt();
		return S_OK;
	}

	else return CBRegion::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CAdRegion::ScToString() {
	return "[ad region]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdRegion::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "REGION {\n");
	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", m_Name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());
	Buffer->PutTextIndent(Indent + 2, "BLOCKED=%s\n", m_Blocked ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "DECORATION=%s\n", m_Decoration ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "ACTIVE=%s\n", m_Active ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "SCALE=%d\n", (int)m_Zoom);
	Buffer->PutTextIndent(Indent + 2, "ALPHA_COLOR { %d,%d,%d }\n", D3DCOLGetR(m_Alpha), D3DCOLGetG(m_Alpha), D3DCOLGetB(m_Alpha));
	Buffer->PutTextIndent(Indent + 2, "ALPHA = %d\n", D3DCOLGetA(m_Alpha));
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SELECTED=%s\n", m_EditorSelected ? "TRUE" : "FALSE");

	int i;
	for (i = 0; i < m_Scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", m_Scripts[i]->m_Filename);
	}

	if (m_ScProp) m_ScProp->SaveAsText(Buffer, Indent + 2);

	for (i = 0; i < m_Points.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "POINT {%d,%d}\n", m_Points[i]->x, m_Points[i]->y);
	}

	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdRegion::Persist(CBPersistMgr *PersistMgr) {
	CBRegion::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Alpha));
	PersistMgr->Transfer(TMEMBER(m_Blocked));
	PersistMgr->Transfer(TMEMBER(m_Decoration));
	PersistMgr->Transfer(TMEMBER(m_Zoom));

	return S_OK;
}

} // end of namespace WinterMute
