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
	_scrollOffset = 0;
}


//////////////////////////////////////////////////////////////////////////
CAdInventory::~CAdInventory() {
	_takenItems.RemoveAll(); // ref only
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventory::InsertItem(const char *Name, const char *InsertAfter) {
	if (Name == NULL) return E_FAIL;

	CAdItem *item = ((CAdGame *)Game)->GetItemByName(Name);
	if (item == NULL) return E_FAIL;

	int InsertIndex = -1;
	for (int i = 0; i < _takenItems.GetSize(); i++) {
		if (scumm_stricmp(_takenItems[i]->_name, Name) == 0) {
			_takenItems.RemoveAt(i);
			i--;
			continue;
		}
		if (InsertAfter && scumm_stricmp(_takenItems[i]->_name, InsertAfter) == 0) InsertIndex = i + 1;
	}


	if (InsertIndex == -1) _takenItems.Add(item);
	else _takenItems.InsertAt(InsertIndex, item);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventory::RemoveItem(const char *Name) {
	if (Name == NULL) return E_FAIL;

	for (int i = 0; i < _takenItems.GetSize(); i++) {
		if (scumm_stricmp(_takenItems[i]->_name, Name) == 0) {
			if (((CAdGame *)Game)->_selectedItem == _takenItems[i])((CAdGame *)Game)->_selectedItem = NULL;
			_takenItems.RemoveAt(i);
			return S_OK;
		}
	}

	return E_FAIL;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventory::RemoveItem(CAdItem *Item) {
	if (Item == NULL) return E_FAIL;

	for (int i = 0; i < _takenItems.GetSize(); i++) {
		if (_takenItems[i] == Item) {
			if (((CAdGame *)Game)->_selectedItem == _takenItems[i])((CAdGame *)Game)->_selectedItem = NULL;
			_takenItems.RemoveAt(i);
			return S_OK;
		}
	}

	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventory::Persist(CBPersistMgr *PersistMgr) {

	CBObject::Persist(PersistMgr);

	_takenItems.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_scrollOffset));

	return S_OK;
}
    
} // end of namespace WinterMute
