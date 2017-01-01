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

#include "titanic/star_control/surface_fader_base.h"

namespace Titanic {

CSurfaceFaderBase::CSurfaceFaderBase() : _index(-1), _count(32),
		_videoSurface(nullptr) {
}

CSurfaceFaderBase::~CSurfaceFaderBase() {
	delete _videoSurface;
}

void CSurfaceFaderBase::reset() {
	_index = 0;
}

bool CSurfaceFaderBase::setupSurface(CScreenManager *screenManager, CVideoSurface *srcSurface) {
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

CVideoSurface *CSurfaceFaderBase::fade(CScreenManager *screenManager, CVideoSurface *srcSurface) {
	if (_index == -1 || _index >= _count)
		return srcSurface;

	if (!_count && !setupSurface(screenManager, srcSurface))
		return nullptr;

	srcSurface->lock();
	_videoSurface->lock();
	CSurfaceArea srCSurfaceArea(srcSurface);
	CSurfaceArea destSurfaceObj(_videoSurface);

	// Copy the surface with fading
	copySurface(srCSurfaceArea, destSurfaceObj);

	srcSurface->unlock();
	_videoSurface->unlock();

	++_index;
	return _videoSurface;
}

} // End of namespace Titanic
