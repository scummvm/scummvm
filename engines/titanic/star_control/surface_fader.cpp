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

#include "titanic/star_control/surface_fader.h"
#include "titanic/star_control/surface_area.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/video_surface.h"
#include "common/system.h"
#include "graphics/pixelformat.h"

namespace Titanic {


CSurfaceFader::CSurfaceFader() : _index(-1), _count(32), _fadeIn(false), _videoSurface(nullptr) {
	_dataP = new byte[_count];

	for (int idx = 0; idx < _count; ++idx)
		_dataP[idx] = (byte)(pow((double)idx / (double)_count, 1.299999952316284)
			* (double)_count + 0.5);
}

CSurfaceFader::~CSurfaceFader() {
	delete _videoSurface;
	delete[] _dataP;
}

void CSurfaceFader::reset() {
	_index = 0;
}

bool CSurfaceFader::setupSurface(CScreenManager *screenManager, CVideoSurface *srcSurface) {
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

CVideoSurface *CSurfaceFader::draw(CScreenManager *screenManager, CVideoSurface *srcSurface) {
	if (_index < 0 || _index >= _count)
		return srcSurface;

	// On the first iteration, set up a temporary surface
	if (_index == 0 && !setupSurface(screenManager, srcSurface))
		return nullptr;

	srcSurface->lock();
	_videoSurface->lock();
	CSurfaceArea srcSurfaceArea(srcSurface);
	CSurfaceArea destSurfaceArea(_videoSurface);

	// Copy the surface with fading
	step(srcSurfaceArea, destSurfaceArea);

	srcSurface->unlock();
	_videoSurface->unlock();

	++_index;
	return _videoSurface;
}

void CSurfaceFader::step(CSurfaceArea &srcSurface, CSurfaceArea &destSurface) {
	const uint16 *srcPixelP = (const uint16 *)srcSurface._pixelsPtr;
	uint16 *destPixelP = (uint16 *)destSurface._pixelsPtr;

	// Currently we only support 2 bytes per pixel surfaces
	assert(srcSurface._bpp == 2);

	byte dataVal = _dataP[_index];
	double fraction = (double)dataVal / ((double)(_count - 1));
	if (!_fadeIn)
		// For fade outs, reverse the percentage visibility
		fraction = 1.0 - fraction;

	// Iterate through the pixels
	byte r, g, b;
	Graphics::PixelFormat format = g_system->getScreenFormat();

	for (int yp = 0; yp < srcSurface._height; ++yp) {
		for (int xp = 0; xp < srcSurface._width; ++xp, ++srcPixelP, ++destPixelP) {
			format.colorToRGB(*srcPixelP, r, g, b);
			r = (byte)((double)r * fraction);
			g = (byte)((double)g * fraction);
			b = (byte)((double)b * fraction);
			*destPixelP = format.RGBToColor(r, g, b);
		}
	}
}

void CSurfaceFader::setFadeIn(bool fadeIn) {
	_fadeIn = fadeIn;
}

} // End of namespace Titanic
