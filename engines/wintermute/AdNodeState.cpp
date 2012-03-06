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
#include "AdNodeState.h"
#include "AdEntity.h"
#include "BStringTable.h"
#include "BSprite.h"
#include "utils.h"
#include "PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdNodeState, false)


//////////////////////////////////////////////////////////////////////////
CAdNodeState::CAdNodeState(CBGame *inGame): CBBase(inGame) {
	m_Name = NULL;
	m_Active = false;
	for (int i = 0; i < 7; i++) m_Caption[i] = NULL;
	m_AlphaColor = 0;
	m_Filename = NULL;
	m_Cursor = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdNodeState::~CAdNodeState() {
	delete[] m_Name;
	delete[] m_Filename;
	delete[] m_Cursor;
	m_Name = NULL;
	m_Filename = NULL;
	m_Cursor = NULL;
	for (int i = 0; i < 7; i++) {
		delete[] m_Caption[i];
		m_Caption[i] = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdNodeState::SetName(char *Name) {
	delete[] m_Name;
	m_Name = NULL;
	CBUtils::SetString(&m_Name, Name);
}


//////////////////////////////////////////////////////////////////////////
void CAdNodeState::SetFilename(char *Filename) {
	delete[] m_Filename;
	m_Filename = NULL;
	CBUtils::SetString(&m_Filename, Filename);
}


//////////////////////////////////////////////////////////////////////////
void CAdNodeState::SetCursor(char *Filename) {
	delete[] m_Cursor;
	m_Cursor = NULL;
	CBUtils::SetString(&m_Cursor, Filename);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdNodeState::Persist(CBPersistMgr *PersistMgr) {
	PersistMgr->Transfer(TMEMBER(Game));

	PersistMgr->Transfer(TMEMBER(m_Active));
	PersistMgr->Transfer(TMEMBER(m_Name));
	PersistMgr->Transfer(TMEMBER(m_Filename));
	PersistMgr->Transfer(TMEMBER(m_Cursor));
	PersistMgr->Transfer(TMEMBER(m_AlphaColor));
	for (int i = 0; i < 7; i++) PersistMgr->Transfer(TMEMBER(m_Caption[i]));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdNodeState::SetCaption(char *Caption, int Case) {
	if (Case == 0) Case = 1;
	if (Case < 1 || Case > 7) return;

	delete[] m_Caption[Case - 1];
	m_Caption[Case - 1] = new char[strlen(Caption) + 1];
	if (m_Caption[Case - 1]) {
		strcpy(m_Caption[Case - 1], Caption);
		Game->m_StringTable->Expand(&m_Caption[Case - 1]);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CAdNodeState::GetCaption(int Case) {
	if (Case == 0) Case = 1;
	if (Case < 1 || Case > 7 || m_Caption[Case - 1] == NULL) return "";
	else return m_Caption[Case - 1];
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdNodeState::TransferEntity(CAdEntity *Entity, bool IncludingSprites, bool Saving) {
	if (!Entity) return E_FAIL;

	// hack!
	if (this->Game != Entity->Game) this->Game = Entity->Game;

	if (Saving) {
		for (int i = 0; i < 7; i++) {
			if (Entity->m_Caption[i]) SetCaption(Entity->m_Caption[i], i);
		}
		if (!Entity->m_Region && Entity->m_Sprite && Entity->m_Sprite->m_Filename) {
			if (IncludingSprites) SetFilename(Entity->m_Sprite->m_Filename);
			else SetFilename("");
		}
		if (Entity->m_Cursor && Entity->m_Cursor->m_Filename) SetCursor(Entity->m_Cursor->m_Filename);
		m_AlphaColor = Entity->m_AlphaColor;
		m_Active = Entity->m_Active;
	} else {
		for (int i = 0; i < 7; i++) {
			if (m_Caption[i]) Entity->SetCaption(m_Caption[i], i);
		}
		if (m_Filename && !Entity->m_Region && IncludingSprites && strcmp(m_Filename, "") != 0) {
			if (!Entity->m_Sprite || !Entity->m_Sprite->m_Filename || scumm_stricmp(Entity->m_Sprite->m_Filename, m_Filename) != 0)
				Entity->SetSprite(m_Filename);
		}
		if (m_Cursor) {
			if (!Entity->m_Cursor || !Entity->m_Cursor->m_Filename || scumm_stricmp(Entity->m_Cursor->m_Filename, m_Cursor) != 0)
				Entity->SetCursor(m_Cursor);
		}

		Entity->m_Active = m_Active;
		Entity->m_AlphaColor = m_AlphaColor;
	}

	return S_OK;
}

} // end of namespace WinterMute
