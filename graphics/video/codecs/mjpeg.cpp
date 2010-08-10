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

#include "graphics/video/codecs/mjpeg.h"

namespace Graphics {

JPEGDecoder::JPEGDecoder() : Codec() {
	_jpeg = new JPEG();
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

Surface *JPEGDecoder::decodeImage(Common::SeekableReadStream* stream) {
	_jpeg->read(stream);
	Surface *ySurface = _jpeg->getComponent(1);
	Surface *uSurface = _jpeg->getComponent(2);
	Surface *vSurface = _jpeg->getComponent(3);

	if (!_surface) {
		_surface = new Surface();
		_surface->create(ySurface->w, ySurface->h, _pixelFormat.bytesPerPixel);
	}

	for (uint16 i = 0; i < _surface->h; i++) {
		for (uint16 j = 0; j < _surface->w; j++) {
			byte r = 0, g = 0, b = 0;
			YUV2RGB(*((byte *)ySurface->getBasePtr(j, i)), *((byte *)uSurface->getBasePtr(j, i)), *((byte *)vSurface->getBasePtr(j, i)), r, g, b);
			if (_pixelFormat.bytesPerPixel == 2)
				*((uint16 *)_surface->getBasePtr(j, i)) = _pixelFormat.RGBToColor(r, g, b);
			else
				*((uint32 *)_surface->getBasePtr(j, i)) = _pixelFormat.RGBToColor(r, g, b);
		}
	}

	return _surface;
}

} // End of namespace Graphics
