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
#include "AdTalkDef.h"
#include "AdTalkNode.h"
#include "BParser.h"
#include "BGame.h"
#include "BDynBuffer.h"
#include "BSprite.h"
#include "AdSpriteSet.h"
#include "BFileManager.h"
#include "utils.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdTalkDef, false)

//////////////////////////////////////////////////////////////////////////
CAdTalkDef::CAdTalkDef(CBGame *inGame): CBObject(inGame) {
	m_DefaultSpriteFilename = NULL;
	m_DefaultSprite = NULL;

	m_DefaultSpriteSetFilename = NULL;
	m_DefaultSpriteSet = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdTalkDef::~CAdTalkDef() {
	for (int i = 0; i < m_Nodes.GetSize(); i++) delete m_Nodes[i];
	m_Nodes.RemoveAll();

	delete[] m_DefaultSpriteFilename;
	delete m_DefaultSprite;
	m_DefaultSpriteFilename = NULL;
	m_DefaultSprite = NULL;

	delete[] m_DefaultSpriteSetFilename;
	delete m_DefaultSpriteSet;
	m_DefaultSpriteSetFilename = NULL;
	m_DefaultSpriteSet = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkDef::LoadFile(const char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdTalkDef::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	CBUtils::SetString(&m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing TALK file '%s'", Filename);

	delete [] Buffer;

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
HRESULT CAdTalkDef::LoadBuffer(byte  *Buffer, bool Complete) {
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

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_TALK) {
			Game->LOG(0, "'TALK' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_ACTION: {
			CAdTalkNode *Node = new CAdTalkNode(Game);
			if (Node && SUCCEEDED(Node->LoadBuffer(params, false))) m_Nodes.Add(Node);
			else {
				delete Node;
				Node = NULL;
				cmd = PARSERR_GENERIC;
			}
		}
		break;

		case TOKEN_DEFAULT_SPRITE:
			CBUtils::SetString(&m_DefaultSpriteFilename, (char *)params);
			break;

		case TOKEN_DEFAULT_SPRITESET_FILE:
			CBUtils::SetString(&m_DefaultSpriteSetFilename, (char *)params);
			break;

		case TOKEN_DEFAULT_SPRITESET: {
			delete m_DefaultSpriteSet;
			m_DefaultSpriteSet = new CAdSpriteSet(Game);
			if (!m_DefaultSpriteSet || FAILED(m_DefaultSpriteSet->LoadBuffer(params, false))) {
				delete m_DefaultSpriteSet;
				m_DefaultSpriteSet = NULL;
				cmd = PARSERR_GENERIC;
			}
		}
		break;


		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in TALK definition");
		return E_FAIL;
	}

	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading TALK definition");
		return E_FAIL;
	}

	delete m_DefaultSprite;
	delete m_DefaultSpriteSet;
	m_DefaultSprite = NULL;
	m_DefaultSpriteSet = NULL;

	if (m_DefaultSpriteFilename) {
		m_DefaultSprite = new CBSprite(Game);
		if (!m_DefaultSprite || FAILED(m_DefaultSprite->LoadFile(m_DefaultSpriteFilename))) return E_FAIL;
	}

	if (m_DefaultSpriteSetFilename) {
		m_DefaultSpriteSet = new CAdSpriteSet(Game);
		if (!m_DefaultSpriteSet || FAILED(m_DefaultSpriteSet->LoadFile(m_DefaultSpriteSetFilename))) return E_FAIL;
	}


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkDef::Persist(CBPersistMgr *PersistMgr) {

	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_DefaultSprite));
	PersistMgr->Transfer(TMEMBER(m_DefaultSpriteFilename));
	PersistMgr->Transfer(TMEMBER(m_DefaultSpriteSet));
	PersistMgr->Transfer(TMEMBER(m_DefaultSpriteSetFilename));

	m_Nodes.Persist(PersistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkDef::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "TALK {\n");
	if (m_DefaultSpriteFilename) Buffer->PutTextIndent(Indent + 2, "DEFAULT_SPRITE=\"%s\"\n", m_DefaultSpriteFilename);

	if (m_DefaultSpriteSetFilename) Buffer->PutTextIndent(Indent + 2, "DEFAULT_SPRITESET_FILE=\"%s\"\n", m_DefaultSpriteSetFilename);
	else if (m_DefaultSpriteSet) m_DefaultSpriteSet->SaveAsText(Buffer, Indent + 2);

	for (int i = 0; i < m_Nodes.GetSize(); i++) {
		m_Nodes[i]->SaveAsText(Buffer, Indent + 2);
		Buffer->PutTextIndent(Indent, "\n");
	}
	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkDef::LoadDefaultSprite() {
	if (m_DefaultSpriteFilename && !m_DefaultSprite) {
		m_DefaultSprite = new CBSprite(Game);
		if (!m_DefaultSprite || FAILED(m_DefaultSprite->LoadFile(m_DefaultSpriteFilename))) {
			delete m_DefaultSprite;
			m_DefaultSprite = NULL;
			return E_FAIL;
		} else return S_OK;
	} else if (m_DefaultSpriteSetFilename && !m_DefaultSpriteSet) {
		m_DefaultSpriteSet = new CAdSpriteSet(Game);
		if (!m_DefaultSpriteSet || FAILED(m_DefaultSpriteSet->LoadFile(m_DefaultSpriteSetFilename))) {
			delete m_DefaultSpriteSet;
			m_DefaultSpriteSet = NULL;
			return E_FAIL;
		} else return S_OK;
	} else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CBSprite *CAdTalkDef::GetDefaultSprite(TDirection Dir) {
	LoadDefaultSprite();
	if (m_DefaultSprite) return m_DefaultSprite;
	else if (m_DefaultSpriteSet) return m_DefaultSpriteSet->GetSprite(Dir);
	else return NULL;
}

} // end of namespace WinterMute
