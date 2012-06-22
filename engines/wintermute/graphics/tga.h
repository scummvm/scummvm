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
 */

/* Based on code from eos https://github.com/DrMcCoy/xoreos/
 * relicensed under GPLv2+ with permission from DrMcCoy and clone2727
 */

#ifndef WINTERMUTE_GRAPHICS_IMAGES_TGA_H
#define WINTERMUTE_GRAPHICS_IMAGES_TGA_H

#include "graphics/surface.h"
#include "graphics/decoders/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace WinterMute {

/** TarGa image. */
class TGA : public Graphics::ImageDecoder {
public:
	TGA();
	virtual ~TGA();
	virtual void destroy();
	virtual const Graphics::Surface *getSurface() const {
		return &_surface;
	};
	virtual bool loadStream(Common::SeekableReadStream &stream);
private:
	Graphics::PixelFormat _format;
	bool _hasAlpha;
	Graphics::Surface _surface;
	// Loading helpers
	bool readHeader(Common::SeekableReadStream &tga, byte &imageType, byte &pixelDepth);
	bool readData(Common::SeekableReadStream &tga, byte imageType, byte pixelDepth);
};

} // End of namespace Graphics

#endif // GRAPHICS_IMAGES_TGA_H
