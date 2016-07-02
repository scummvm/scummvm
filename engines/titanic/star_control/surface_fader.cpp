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
#include "common/system.h"
#include "graphics/pixelformat.h"

namespace Titanic {

CSurfaceFader::CSurfaceFader() : CSurfaceFaderBase() {
	_dataP = new byte[_count];

	for (int idx = 0; idx < _count; ++idx) {
		// TODO: Setup data bytes
	}
}

CSurfaceFader::~CSurfaceFader() {
	delete[] _dataP;
}

void CSurfaceFader::copySurface(CSurfaceArea &srcSurface, CSurfaceArea &destSurface) {
	const uint16 *srcPixelP = srcSurface._pixelsPtr;
	uint16 *destPixelP = destSurface._pixelsPtr;

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
