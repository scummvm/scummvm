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
 * $URL$
 * $Id$
 */

#ifndef GRAPHICS_IMAGEDEC_H
#define GRAPHICS_IMAGEDEC_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

#include "graphics/surface.h"
#include "graphics/pixelformat.h"

namespace Graphics {

class ImageDecoder {
public:
	ImageDecoder() {}
	virtual ~ImageDecoder() {}

	static Surface *loadFile(const Common::String &name, const PixelFormat &format);
	static Surface *loadFile(Common::SeekableReadStream &stream, const PixelFormat &format);

	/**
	 * checks if the data can be decoded by this decoder
	 *
	 * @param stream memory read stream
	 * @return true if it can be decoded, otherwise false
	 */
	virtual bool decodeable(Common::SeekableReadStream &stream) = 0;

	/**
	 * decodes the data and returns an pointer to the resulting surface.
	 * Surface::free() must be called by the user also it must be deleted
	 * with delete;
	 *
	 * @param stream the memory stream which should be decoded
	 * @param format the pixel format used to generate the surface
	 * @return returns a new surface if the image could be decoded, otherwise 0
	 */
	virtual Surface *decodeImage(Common::SeekableReadStream &stream, const PixelFormat &format) = 0;
};
} // End of namespace Graphics

#endif

