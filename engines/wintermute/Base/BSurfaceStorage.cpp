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
#include "engines/wintermute/Base/BSurfaceStorage.h"
#include "engines/wintermute/Base/BSurfaceSDL.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

//IMPLEMENT_PERSISTENT(CBSurfaceStorage, true);

//////////////////////////////////////////////////////////////////////
CBSurfaceStorage::CBSurfaceStorage(CBGame *inGame): CBBase(inGame) {
	_lastCleanupTime = 0;
}


//////////////////////////////////////////////////////////////////////
CBSurfaceStorage::~CBSurfaceStorage() {
	Cleanup(true);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceStorage::Cleanup(bool Warn) {
	for (int i = 0; i < _surfaces.GetSize(); i++) {
		if (Warn) Game->LOG(0, "CBSurfaceStorage warning: purging surface '%s', usage:%d", _surfaces[i]->_filename, _surfaces[i]->_referenceCount);
		delete _surfaces[i];
	}
	_surfaces.RemoveAll();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceStorage::InitLoop() {
	if (Game->_smartCache && Game->_liveTimer - _lastCleanupTime >= Game->_surfaceGCCycleTime) {
		_lastCleanupTime = Game->_liveTimer;
		SortSurfaces();
		for (int i = 0; i < _surfaces.GetSize(); i++) {
			if (_surfaces[i]->_lifeTime <= 0) break;

			if (_surfaces[i]->_lifeTime > 0 && _surfaces[i]->_valid && Game->_liveTimer - _surfaces[i]->_lastUsedTime >= _surfaces[i]->_lifeTime) {
				//Game->QuickMessageForm("Invalidating: %s", _surfaces[i]->_filename);
				_surfaces[i]->invalidate();
			}
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceStorage::RemoveSurface(CBSurface *surface) {
	for (int i = 0; i < _surfaces.GetSize(); i++) {
		if (_surfaces[i] == surface) {
			_surfaces[i]->_referenceCount--;
			if (_surfaces[i]->_referenceCount <= 0) {
				delete _surfaces[i];
				_surfaces.RemoveAt(i);
			}
			break;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////
CBSurface *CBSurfaceStorage::AddSurface(const char *Filename, bool default_ck, byte ck_red, byte ck_green, byte ck_blue, int LifeTime, bool KeepLoaded) {
	for (int i = 0; i < _surfaces.GetSize(); i++) {
		if (scumm_stricmp(_surfaces[i]->_filename, Filename) == 0) {
			_surfaces[i]->_referenceCount++;
			return _surfaces[i];
		}
	}

	Common::SeekableReadStream *File = Game->_fileManager->OpenFile(Filename);
	if (!File) {
		if (Filename) Game->LOG(0, "Missing image: '%s'", Filename);
		if (Game->_dEBUG_DebugMode)
			return AddSurface("invalid_debug.bmp", default_ck, ck_red, ck_green, ck_blue, LifeTime, KeepLoaded);
		else
			return AddSurface("invalid.bmp", default_ck, ck_red, ck_green, ck_blue, LifeTime, KeepLoaded);
	} else Game->_fileManager->CloseFile(File);


	CBSurface *surface;
	surface = new CBSurfaceSDL(Game);


	if (!surface) return NULL;

	if (FAILED(surface->create(Filename, default_ck, ck_red, ck_green, ck_blue, LifeTime, KeepLoaded))) {
		delete surface;
		return NULL;
	} else {
		surface->_referenceCount = 1;
		_surfaces.Add(surface);
		return surface;
	}
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceStorage::RestoreAll() {
	HRESULT ret;
	for (int i = 0; i < _surfaces.GetSize(); i++) {
		ret = _surfaces[i]->restore();
		if (ret != S_OK) {
			Game->LOG(0, "CBSurfaceStorage::RestoreAll failed");
			return ret;
		}
	}
	return S_OK;
}


/*
//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceStorage::Persist(CBPersistMgr *persistMgr)
{

    if(!persistMgr->_saving) Cleanup(false);

    persistMgr->transfer(TMEMBER(Game));

    //_surfaces.Persist(persistMgr);

    return S_OK;
}
*/


//////////////////////////////////////////////////////////////////////////
HRESULT CBSurfaceStorage::SortSurfaces() {
	qsort(_surfaces.GetData(), _surfaces.GetSize(), sizeof(CBSurface *), SurfaceSortCB);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CBSurfaceStorage::SurfaceSortCB(const void *arg1, const void *arg2) {
	CBSurface *s1 = *((CBSurface **)arg1);
	CBSurface *s2 = *((CBSurface **)arg2);

	// sort by life time
	if (s1->_lifeTime <= 0 && s2->_lifeTime > 0) return 1;
	else if (s1->_lifeTime > 0 && s2->_lifeTime <= 0) return -1;


	// sort by validity
	if (s1->_valid && !s2->_valid) return -1;
	else if (!s1->_valid && s2->_valid) return 1;

	// sort by time
	else if (s1->_lastUsedTime > s2->_lastUsedTime) return 1;
	else if (s1->_lastUsedTime < s2->_lastUsedTime) return -1;
	else return 0;
}

} // end of namespace WinterMute
