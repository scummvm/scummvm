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

#include "AdInventory.h"
#include "ADGame.h"
#include "AdItem.h"
#include "PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdInventory, false)

//////////////////////////////////////////////////////////////////////////
CAdInventory::CAdInventory(CBGame *inGame): CBObject(inGame) {
	m_ScrollOffset = 0;
}


//////////////////////////////////////////////////////////////////////////
CAdInventory::~CAdInventory() {
	m_TakenItems.RemoveAll(); // ref only
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventory::InsertItem(char *Name, char *InsertAfter) {
	if (Name == NULL) return E_FAIL;

	CAdItem *item = ((CAdGame *)Game)->GetItemByName(Name);
	if (item == NULL) return E_FAIL;

	int InsertIndex = -1;
	for (int i = 0; i < m_TakenItems.GetSize(); i++) {
		if (scumm_stricmp(m_TakenItems[i]->m_Name, Name) == 0) {
			m_TakenItems.RemoveAt(i);
			i--;
			continue;
		}
		if (InsertAfter && scumm_stricmp(m_TakenItems[i]->m_Name, InsertAfter) == 0) InsertIndex = i + 1;
	}


	if (InsertIndex == -1) m_TakenItems.Add(item);
	else m_TakenItems.InsertAt(InsertIndex, item);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventory::RemoveItem(char *Name) {
	if (Name == NULL) return E_FAIL;

	for (int i = 0; i < m_TakenItems.GetSize(); i++) {
		if (scumm_stricmp(m_TakenItems[i]->m_Name, Name) == 0) {
			if (((CAdGame *)Game)->m_SelectedItem == m_TakenItems[i])((CAdGame *)Game)->m_SelectedItem = NULL;
			m_TakenItems.RemoveAt(i);
			return S_OK;
		}
	}

	return E_FAIL;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventory::RemoveItem(CAdItem *Item) {
	if (Item == NULL) return E_FAIL;

	for (int i = 0; i < m_TakenItems.GetSize(); i++) {
		if (m_TakenItems[i] == Item) {
			if (((CAdGame *)Game)->m_SelectedItem == m_TakenItems[i])((CAdGame *)Game)->m_SelectedItem = NULL;
			m_TakenItems.RemoveAt(i);
			return S_OK;
		}
	}

	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventory::Persist(CBPersistMgr *PersistMgr) {

	CBObject::Persist(PersistMgr);

	m_TakenItems.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_ScrollOffset));

	return S_OK;
}
    
} // end of namespace WinterMute
