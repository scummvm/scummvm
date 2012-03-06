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
#include "persistent.h"
#include "AdSceneState.h"
#include "AdNodeState.h"
#include "PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdSceneState, false)

//////////////////////////////////////////////////////////////////////////
CAdSceneState::CAdSceneState(CBGame *inGame): CBBase(inGame) {
	m_Filename = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdSceneState::~CAdSceneState() {
	SAFE_DELETE_ARRAY(m_Filename);

	for (int i = 0; i < m_NodeStates.GetSize(); i++) delete m_NodeStates[i];
	m_NodeStates.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSceneState::Persist(CBPersistMgr *PersistMgr) {
	PersistMgr->Transfer(TMEMBER(m_Filename));
	m_NodeStates.Persist(PersistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdSceneState::SetFilename(char *Filename) {
	SAFE_DELETE_ARRAY(m_Filename);
	m_Filename = new char [strlen(Filename) + 1];
	if (m_Filename) strcpy(m_Filename, Filename);
}


//////////////////////////////////////////////////////////////////////////
CAdNodeState *CAdSceneState::GetNodeState(char *Name, bool Saving) {
	for (int i = 0; i < m_NodeStates.GetSize(); i++) {
		if (scumm_stricmp(m_NodeStates[i]->m_Name, Name) == 0) return m_NodeStates[i];
	}

	if (Saving) {
		CAdNodeState *ret = new CAdNodeState(Game);
		ret->SetName(Name);
		m_NodeStates.Add(ret);

		return ret;
	} else return NULL;
}

} // end of namespace WinterMute
