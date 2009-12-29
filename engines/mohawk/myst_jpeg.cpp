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
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"
#include "graphics/conversion.h" // For YUV2RGB

#include "mohawk/myst_jpeg.h"

namespace Mohawk {
	
MystJPEG::MystJPEG() {
	_jpeg = new Graphics::JPEG();
	_pixelFormat = g_system->getScreenFormat();
	
	// We're going to have to dither if we're running in 8bpp.
	// We'll take RGBA8888 for best color performance in this case.
	if (_pixelFormat.bytesPerPixel == 1)
		_pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
}

Graphics::Surface *MystJPEG::decodeImage(Common::SeekableReadStream* stream) {
	_jpeg->read(stream);
	Graphics::Surface *ySurface = _jpeg->getComponent(1);
	Graphics::Surface *uSurface = _jpeg->getComponent(2);
	Graphics::Surface *vSurface = _jpeg->getComponent(3);
	
	Graphics::Surface *finalSurface = new Graphics::Surface();
	finalSurface->create(ySurface->w, ySurface->h, _pixelFormat.bytesPerPixel);
	
	for (uint16 i = 0; i < finalSurface->h; i++) {
		for (uint16 j = 0; j < finalSurface->w; j++) {
			byte r = 0, g = 0, b = 0;
			Graphics::YUV2RGB(*((byte *)ySurface->getBasePtr(j, i)), *((byte *)uSurface->getBasePtr(j, i)), *((byte *)vSurface->getBasePtr(j, i)), r, g, b);
			if (_pixelFormat.bytesPerPixel == 2)
				*((uint16 *)finalSurface->getBasePtr(j, i)) = _pixelFormat.RGBToColor(r, g, b);
			else
				*((uint32 *)finalSurface->getBasePtr(j, i)) = _pixelFormat.RGBToColor(r, g, b);
		}
	}
	
	return finalSurface;
}

} // End of namespace Mohawk
