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
	cleanup(true);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSurfaceStorage::cleanup(bool warn) {
	for (int i = 0; i < _surfaces.getSize(); i++) {
		if (warn) Game->LOG(0, "CBSurfaceStorage warning: purging surface '%s', usage:%d", _surfaces[i]->_filename, _surfaces[i]->_referenceCount);
		delete _surfaces[i];
	}
	_surfaces.removeAll();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSurfaceStorage::initLoop() {
	if (Game->_smartCache && Game->_liveTimer - _lastCleanupTime >= Game->_surfaceGCCycleTime) {
		_lastCleanupTime = Game->_liveTimer;
		sortSurfaces();
		for (int i = 0; i < _surfaces.getSize(); i++) {
			if (_surfaces[i]->_lifeTime <= 0) break;

			if (_surfaces[i]->_lifeTime > 0 && _surfaces[i]->_valid && Game->_liveTimer - _surfaces[i]->_lastUsedTime >= _surfaces[i]->_lifeTime) {
				//Game->QuickMessageForm("Invalidating: %s", _surfaces[i]->_filename);
				_surfaces[i]->invalidate();
			}
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
ERRORCODE CBSurfaceStorage::removeSurface(CBSurface *surface) {
	for (int i = 0; i < _surfaces.getSize(); i++) {
		if (_surfaces[i] == surface) {
			_surfaces[i]->_referenceCount--;
			if (_surfaces[i]->_referenceCount <= 0) {
				delete _surfaces[i];
				_surfaces.removeAt(i);
			}
			break;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
CBSurface *CBSurfaceStorage::addSurface(const char *filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	for (int i = 0; i < _surfaces.getSize(); i++) {
		if (scumm_stricmp(_surfaces[i]->_filename, filename) == 0) {
			_surfaces[i]->_referenceCount++;
			return _surfaces[i];
		}
	}

	if (!Game->_fileManager->hasFile(filename)) {
		if (filename) Game->LOG(0, "Missing image: '%s'", filename);
		if (Game->_dEBUG_DebugMode)
			return addSurface("invalid_debug.bmp", defaultCK, ckRed, ckGreen, ckBlue, lifeTime, keepLoaded);
		else
			return addSurface("invalid.bmp", defaultCK, ckRed, ckGreen, ckBlue, lifeTime, keepLoaded);
	}

	CBSurface *surface;
	surface = new CBSurfaceSDL(Game);

	if (!surface) return NULL;

	if (DID_FAIL(surface->create(filename, defaultCK, ckRed, ckGreen, ckBlue, lifeTime, keepLoaded))) {
		delete surface;
		return NULL;
	} else {
		surface->_referenceCount = 1;
		_surfaces.add(surface);
		return surface;
	}
}


//////////////////////////////////////////////////////////////////////
ERRORCODE CBSurfaceStorage::restoreAll() {
	ERRORCODE ret;
	for (int i = 0; i < _surfaces.getSize(); i++) {
		ret = _surfaces[i]->restore();
		if (ret != STATUS_OK) {
			Game->LOG(0, "CBSurfaceStorage::RestoreAll failed");
			return ret;
		}
	}
	return STATUS_OK;
}


/*
//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSurfaceStorage::persist(CBPersistMgr *persistMgr)
{

    if(!persistMgr->_saving) cleanup(false);

    persistMgr->transfer(TMEMBER(Game));

    //_surfaces.persist(persistMgr);

    return STATUS_OK;
}
*/


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSurfaceStorage::sortSurfaces() {
	qsort(_surfaces.getData(), _surfaces.getSize(), sizeof(CBSurface *), surfaceSortCB);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int CBSurfaceStorage::surfaceSortCB(const void *arg1, const void *arg2) {
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
