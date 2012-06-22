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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/Ad/AdSceneState.h"
#include "engines/wintermute/Ad/AdNodeState.h"
#include "engines/wintermute/PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdSceneState, false)

//////////////////////////////////////////////////////////////////////////
CAdSceneState::CAdSceneState(CBGame *inGame): CBBase(inGame) {
	_filename = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdSceneState::~CAdSceneState() {
	delete[] _filename;
	_filename = NULL;

	for (int i = 0; i < _nodeStates.GetSize(); i++) delete _nodeStates[i];
	_nodeStates.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSceneState::Persist(CBPersistMgr *persistMgr) {
	persistMgr->transfer(TMEMBER(_filename));
	_nodeStates.Persist(persistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdSceneState::SetFilename(const char *Filename) {
	delete[] _filename;
	_filename = new char [strlen(Filename) + 1];
	if (_filename) strcpy(_filename, Filename);
}


//////////////////////////////////////////////////////////////////////////
CAdNodeState *CAdSceneState::GetNodeState(char *Name, bool Saving) {
	for (int i = 0; i < _nodeStates.GetSize(); i++) {
		if (scumm_stricmp(_nodeStates[i]->_name, Name) == 0) return _nodeStates[i];
	}

	if (Saving) {
		CAdNodeState *ret = new CAdNodeState(Game);
		ret->SetName(Name);
		_nodeStates.Add(ret);

		return ret;
	} else return NULL;
}

} // end of namespace WinterMute
