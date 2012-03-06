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
	m_Sprite = NULL;
	m_SpriteFilename = NULL;
	m_SpriteSet = NULL;
	m_SpriteSetFilename = NULL;
	m_Comment = NULL;

	m_StartTime = m_EndTime = 0;
	m_PlayToEnd = false;
	m_PreCache = false;
}


//////////////////////////////////////////////////////////////////////////
CAdTalkNode::~CAdTalkNode() {
	delete[] m_SpriteFilename;
	delete m_Sprite;
	delete[] m_SpriteSetFilename;
	delete m_SpriteSet;
	delete m_Comment;
	m_SpriteFilename = NULL;
	m_Sprite = NULL;
	m_SpriteSetFilename = NULL;
	m_SpriteSet = NULL;
	m_Comment = NULL;
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

	m_EndTime = 0;
	m_PlayToEnd = false;
	m_PreCache = false;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_SPRITE:
			CBUtils::SetString(&m_SpriteFilename, (char *)params);
			break;

		case TOKEN_SPRITESET_FILE:
			CBUtils::SetString(&m_SpriteSetFilename, (char *)params);
			break;

		case TOKEN_SPRITESET: {
			delete m_SpriteSet;
			m_SpriteSet = new CAdSpriteSet(Game);
			if (!m_SpriteSet || FAILED(m_SpriteSet->LoadBuffer(params, false))) {
				delete m_SpriteSet;
				m_SpriteSet = NULL;
				cmd = PARSERR_GENERIC;
			}
		}
		break;

		case TOKEN_START_TIME:
			parser.ScanStr((char *)params, "%d", &m_StartTime);
			break;

		case TOKEN_END_TIME:
			parser.ScanStr((char *)params, "%d", &m_EndTime);
			break;

		case TOKEN_PRECACHE:
			parser.ScanStr((char *)params, "%b", &m_PreCache);
			break;

		case TOKEN_COMMENT:
			if (Game->m_EditorMode) CBUtils::SetString(&m_Comment, (char *)params);
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

	if (m_EndTime == 0) m_PlayToEnd = true;
	else m_PlayToEnd = false;

	if (m_PreCache && m_SpriteFilename) {
		delete m_Sprite;
		m_Sprite = new CBSprite(Game);
		if (!m_Sprite || FAILED(m_Sprite->LoadFile(m_SpriteFilename))) return E_FAIL;
	}

	if (m_PreCache && m_SpriteSetFilename) {
		delete m_SpriteSet;
		m_SpriteSet = new CAdSpriteSet(Game);
		if (!m_SpriteSet || FAILED(m_SpriteSet->LoadFile(m_SpriteSetFilename))) return E_FAIL;
	}


	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkNode::Persist(CBPersistMgr *PersistMgr) {
	PersistMgr->Transfer(TMEMBER(m_Comment));
	PersistMgr->Transfer(TMEMBER(m_StartTime));
	PersistMgr->Transfer(TMEMBER(m_EndTime));
	PersistMgr->Transfer(TMEMBER(m_PlayToEnd));
	PersistMgr->Transfer(TMEMBER(m_Sprite));
	PersistMgr->Transfer(TMEMBER(m_SpriteFilename));
	PersistMgr->Transfer(TMEMBER(m_SpriteSet));
	PersistMgr->Transfer(TMEMBER(m_SpriteSetFilename));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkNode::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "ACTION {\n");
	if (m_Comment) Buffer->PutTextIndent(Indent + 2, "COMMENT=\"%s\"\n", m_Comment);
	Buffer->PutTextIndent(Indent + 2, "START_TIME=%d\n", m_StartTime);
	if (!m_PlayToEnd) Buffer->PutTextIndent(Indent + 2, "END_TIME=%d\n", m_EndTime);
	if (m_SpriteFilename) Buffer->PutTextIndent(Indent + 2, "SPRITE=\"%s\"\n", m_SpriteFilename);
	if (m_SpriteSetFilename) Buffer->PutTextIndent(Indent + 2, "SPRITESET_FILE=\"%s\"\n", m_SpriteSetFilename);
	else if (m_SpriteSet) m_SpriteSet->SaveAsText(Buffer, Indent + 2);
	if (m_PreCache) Buffer->PutTextIndent(Indent + 2, "PRECACHE=\"%s\"\n", m_PreCache ? "TRUE" : "FALSE");

	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdTalkNode::LoadSprite() {
	if (m_SpriteFilename && !m_Sprite) {
		m_Sprite = new CBSprite(Game);
		if (!m_Sprite || FAILED(m_Sprite->LoadFile(m_SpriteFilename))) {
			delete m_Sprite;
			m_Sprite = NULL;
			return E_FAIL;
		} else return S_OK;
	}

	else if (m_SpriteSetFilename && !m_SpriteSet) {
		m_SpriteSet = new CAdSpriteSet(Game);
		if (!m_SpriteSet || FAILED(m_SpriteSet->LoadFile(m_SpriteSetFilename))) {
			delete m_SpriteSet;
			m_SpriteSet = NULL;
			return E_FAIL;
		} else return S_OK;
	}

	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdTalkNode::IsInTimeInterval(uint32 Time, TDirection Dir) {
	if (Time >= m_StartTime) {
		if (m_PlayToEnd) {
			if ((m_SpriteFilename && m_Sprite == NULL) || (m_Sprite && m_Sprite->m_Finished == false)) return true;
			else if ((m_SpriteSetFilename && m_SpriteSet == NULL) || (m_SpriteSet && m_SpriteSet->GetSprite(Dir) && m_SpriteSet->GetSprite(Dir)->m_Finished == false)) return true;
			else return false;
		} else return m_EndTime >= Time;
	} else return false;
}


//////////////////////////////////////////////////////////////////////////
CBSprite *CAdTalkNode::GetSprite(TDirection Dir) {
	LoadSprite();
	if (m_Sprite) return m_Sprite;
	else if (m_SpriteSet) return m_SpriteSet->GetSprite(Dir);
	else return NULL;
}

} // end of namespace WinterMute
