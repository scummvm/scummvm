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

#include "titanic/star_control/star_control_sub16.h"

namespace Titanic {

CStarControlSub16::CStarControlSub16() : _field4(-1), _field8(32),
		_videoSurface(nullptr) {
}

void CStarControlSub16::reset() {
	_field4 = 0;
}

bool CStarControlSub16::setupSurface(CScreenManager *screenManager, CVideoSurface *srcSurface) {
	int width = srcSurface->getWidth();
	int height = srcSurface->getHeight();

	if (_videoSurface) {
		if (width == _videoSurface->getWidth() && _videoSurface->getHeight())
			// Allocated surface already matches new size
			return true;

		// Different sizes, so delete old surface
		delete _videoSurface;
	}

	_videoSurface = screenManager->createSurface(width, height);
	return true;
}

CVideoSurface *CStarControlSub16::loadSurface(CScreenManager *screenManager, CVideoSurface *srcSurface) {
	if (_field4 < 0 || _field4 >= _field8)
		return srcSurface;

	if (!_field8 && !setupSurface(screenManager, srcSurface))
		return nullptr;

	srcSurface->lock();
	_videoSurface->lock();
	CSurfaceObj srcSurfaceObj(srcSurface);
	CSurfaceObj destSurfaceObj(_videoSurface);

	proc4(srcSurfaceObj, destSurfaceObj);

	srcSurface->unlock();
	_videoSurface->unlock();
	
	++_field4;
	return _videoSurface;
}


} // End of namespace Titanic
