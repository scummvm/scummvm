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
#include "AdResponse.h"
#include "BGame.h"
#include "BFontStorage.h"
#include "BSprite.h"
#include "utils.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdResponse, false)

//////////////////////////////////////////////////////////////////////////
CAdResponse::CAdResponse(CBGame *inGame): CBObject(inGame) {
	m_Text = NULL;
	m_TextOrig = NULL;
	m_Icon = m_IconHover = m_IconPressed = NULL;
	m_Font = NULL;
	m_ID = 0;
	m_ResponseType = RESPONSE_ALWAYS;
}


//////////////////////////////////////////////////////////////////////////
CAdResponse::~CAdResponse() {
	delete[] m_Text;
	delete[] m_TextOrig;
	delete m_Icon;
	delete m_IconHover;
	delete m_IconPressed;
	m_Text = NULL;
	m_TextOrig = NULL;
	m_Icon = NULL;
	m_IconHover = NULL;
	m_IconPressed = NULL;
	if (m_Font) Game->m_FontStorage->RemoveFont(m_Font);
}


//////////////////////////////////////////////////////////////////////////
void CAdResponse::SetText(char *Text) {
	CBUtils::SetString(&m_Text, Text);
	CBUtils::SetString(&m_TextOrig, Text);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponse::SetIcon(char *Filename) {
	delete m_Icon;
	m_Icon = new CBSprite(Game);
	if (!m_Icon || FAILED(m_Icon->LoadFile(Filename))) {
		Game->LOG(0, "CAdResponse::SetIcon failed for file '%s'", Filename);
		delete m_Icon;
		m_Icon = NULL;
		return E_FAIL;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponse::SetFont(char *Filename) {
	if (m_Font) Game->m_FontStorage->RemoveFont(m_Font);
	m_Font = Game->m_FontStorage->AddFont(Filename);
	if (!m_Font) {
		Game->LOG(0, "CAdResponse::SetFont failed for file '%s'", Filename);
		return E_FAIL;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponse::SetIconHover(char *Filename) {
	delete m_IconHover;
	m_IconHover = new CBSprite(Game);
	if (!m_IconHover || FAILED(m_IconHover->LoadFile(Filename))) {
		Game->LOG(0, "CAdResponse::SetIconHover failed for file '%s'", Filename);
		delete m_IconHover;
		m_IconHover = NULL;
		return E_FAIL;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponse::SetIconPressed(char *Filename) {
	delete m_IconPressed;
	m_IconPressed = new CBSprite(Game);
	if (!m_IconPressed || FAILED(m_IconPressed->LoadFile(Filename))) {
		Game->LOG(0, "CAdResponse::SetIconPressed failed for file '%s'", Filename);
		delete m_IconPressed;
		m_IconPressed = NULL;
		return E_FAIL;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponse::Persist(CBPersistMgr *PersistMgr) {

	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Icon));
	PersistMgr->Transfer(TMEMBER(m_IconHover));
	PersistMgr->Transfer(TMEMBER(m_IconPressed));
	PersistMgr->Transfer(TMEMBER(m_ID));
	PersistMgr->Transfer(TMEMBER(m_Text));
	PersistMgr->Transfer(TMEMBER(m_TextOrig));
	PersistMgr->Transfer(TMEMBER_INT(m_ResponseType));
	PersistMgr->Transfer(TMEMBER(m_Font));

	return S_OK;
}

} // end of namespace WinterMute
