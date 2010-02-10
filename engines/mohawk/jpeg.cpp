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

#include "mohawk/jpeg.h"

namespace Mohawk {

JPEGDecoder::JPEGDecoder(bool freeSurfaceAfterUse) : Graphics::Codec(), _freeSurfaceAfterUse(freeSurfaceAfterUse) {
	_jpeg = new Graphics::JPEG();
	_pixelFormat = g_system->getScreenFormat();
	_surface = NULL;
}

JPEGDecoder::~JPEGDecoder() {
	delete _jpeg;

	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

Graphics::Surface *JPEGDecoder::decodeImage(Common::SeekableReadStream* stream) {
	_jpeg->read(stream);
	Graphics::Surface *ySurface = _jpeg->getComponent(1);
	Graphics::Surface *uSurface = _jpeg->getComponent(2);
	Graphics::Surface *vSurface = _jpeg->getComponent(3);

	Graphics::Surface *destSurface = NULL;

	// If we should free the surface after use, use the internal _surface storage
	// (this should be used when using as a Codec, as the Codecs should free their
	// surfaces when deleting the Codec object). Otherwise, create a new Surface
	// as the destination.
	if (_freeSurfaceAfterUse) {
		if (!_surface) {
			_surface = new Graphics::Surface();
			_surface->create(ySurface->w, ySurface->h, _pixelFormat.bytesPerPixel);
		}
		destSurface = _surface;
	} else {
		destSurface = new Graphics::Surface();
		destSurface->create(ySurface->w, ySurface->h, _pixelFormat.bytesPerPixel);
	}

	assert(destSurface);

	for (uint16 i = 0; i < destSurface->h; i++) {
		for (uint16 j = 0; j < destSurface->w; j++) {
			byte r = 0, g = 0, b = 0;
			Graphics::YUV2RGB(*((byte *)ySurface->getBasePtr(j, i)), *((byte *)uSurface->getBasePtr(j, i)), *((byte *)vSurface->getBasePtr(j, i)), r, g, b);
			if (_pixelFormat.bytesPerPixel == 2)
				*((uint16 *)destSurface->getBasePtr(j, i)) = _pixelFormat.RGBToColor(r, g, b);
			else
				*((uint32 *)destSurface->getBasePtr(j, i)) = _pixelFormat.RGBToColor(r, g, b);
		}
	}

	return destSurface;
}

} // End of namespace Mohawk
