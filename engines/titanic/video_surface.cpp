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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/video_surface.h"
#include "titanic/image_decoders.h"
#include "titanic/screen_manager.h"

namespace Titanic {

int CVideoSurface::_videoSurfaceCounter = 0;

CVideoSurface::CVideoSurface(CScreenManager *screenManager) :
		_screenManager(screenManager), _pixels(nullptr),
		_field34(nullptr), _pendingLoad(false), _field3C(0), _field40(0),
		_field44(4), _field48(0), _field50(1) {
	_videoSurfaceNum = _videoSurfaceCounter++;
}


CVideoSurface::~CVideoSurface() {
	if (_ddSurface)
		_videoSurfaceCounter -= freeSurface();
	--_videoSurfaceCounter;
}


void CVideoSurface::setSurface(CScreenManager *screenManager, DirectDrawSurface *surface) {
	_screenManager = screenManager;
	_ddSurface = surface;
}

/*------------------------------------------------------------------------*/

OSVideoSurface::OSVideoSurface(CScreenManager *screenManager, DirectDrawSurface *surface) :
		CVideoSurface(screenManager) {
	_ddSurface = surface;
}

OSVideoSurface::OSVideoSurface(CScreenManager *screenManager, const CResourceKey &key, bool pendingLoad) :
		CVideoSurface(screenManager) {
	_ddSurface = nullptr;
	_pendingLoad = pendingLoad;
	
	if (_pendingLoad) {
		loadResource(key);
	} else {
		_resourceKey = key;
		load();
	}
}

void OSVideoSurface::loadResource(const CResourceKey &key) {
	_resourceKey = key;
	_pendingLoad = true;

	if (hasSurface())
		load();
}

void OSVideoSurface::loadTarga() {
	warning("TODO");
}

void OSVideoSurface::loadJPEG() {
	CString filename = _resourceKey.exists();
	CJPEGDecode decoder(filename);


	warning("TODO");
}

void OSVideoSurface::loadMovie() {
	warning("TODO");
}

bool OSVideoSurface::lock() {
	if (!loadIfReady())
		return false;

	++_lockCount;
	_pixels = (uint16 *)_ddSurface->lock(nullptr, 0);
	return true;
}

void OSVideoSurface::unlock() {
	if (_pixels)
		_ddSurface->unlock();
	_pixels = nullptr;
	--_lockCount;
}

bool OSVideoSurface::hasSurface() {
	return _ddSurface != nullptr;
}

int OSVideoSurface::getWidth() const {
	assert(_ddSurface);
	return _ddSurface->w;
}

int OSVideoSurface::getHeight() const {
	assert(_ddSurface);
	return _ddSurface->h;
}

int OSVideoSurface::getPitch() const {
	assert(_ddSurface);
	return _ddSurface->pitch;
}

void OSVideoSurface::resize(int width, int height) {
	freeSurface();

	_screenManager->resizeSurface(this, width, height);
	if (_ddSurface)
		_videoSurfaceCounter += _ddSurface->getSize();
}

bool OSVideoSurface::load() {
	if (!_resourceKey.scanForFile())
		return false;

	switch (_resourceKey.fileTypeSuffix()) {
	case FILETYPE_IMAGE:
		switch (_resourceKey.imageTypeSuffix()) {
		case IMAGETYPE_TARGA:
			loadTarga();
			break;
		case IMAGETYPE_JPEG:
			loadJPEG();
			break;
		default:
			break;
		}
		return true;

	case FILETYPE_MOVIE:
		loadMovie();
		return true;

	default:
		return false;
	}
}

bool OSVideoSurface::loadIfReady() {
	_videoSurfaceNum = _videoSurfaceCounter;

	if (hasSurface()) {
		return true;
	} else if (_pendingLoad) {
		_field50 = 1;
		load();
		return true;
	} else {
		return false;
	}
}

int OSVideoSurface::freeSurface() {
	if (!_ddSurface)
		return 0;
	int surfaceSize = _ddSurface->getSize();

	delete _field34;
	_field34 = nullptr;
	delete _ddSurface;
	_ddSurface = nullptr;

	return surfaceSize;
}

} // End of namespace Titanic
