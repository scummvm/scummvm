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

namespace Titanic {

int CVideoSurface::_videoSurfaceCounter = 0;

CVideoSurface::CVideoSurface(CScreenManager *screenManager) :
		_screenManager(screenManager), _field2C(0),
		_field34(0), _field38(0), _field3C(0), _field40(0),
		_field44(4), _field48(0), _field50(1) {
	_videoSurfaceNum = _videoSurfaceCounter++;
}

void CVideoSurface::setSurface(CScreenManager *screenManager, DirectDrawSurface *surface) {
	_screenManager = screenManager;
	_surface = surface;
}

/*------------------------------------------------------------------------*/

OSVideoSurface::OSVideoSurface(CScreenManager *screenManager, DirectDrawSurface *surface) :
		CVideoSurface(screenManager) {
	_surface = surface;
}

OSVideoSurface::OSVideoSurface(CScreenManager *screenManager, const CResourceKey &key, bool flag) :
		CVideoSurface(screenManager) {
	_surface = nullptr;
	_field38 = flag;
	
	if (_field38) {
		proc8(key);
	} else {
		_resourceKey = key;
		proc43();
	}
}

void OSVideoSurface::proc8(const CResourceKey &key) {
	_resourceKey = key;
	_field38 = 1;

	if (hasSurface())
		proc43();
}

bool OSVideoSurface::hasSurface() {
	return _surface != nullptr;
}

void OSVideoSurface::proc43() {
	warning("TODO");
}

} // End of namespace Titanic
