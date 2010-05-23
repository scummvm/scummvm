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

#ifndef GRAPHICS_MJPEG_H
#define GRAPHICS_MJPEG_H

#include "common/scummsys.h"
#include "common/stream.h"

#include "graphics/video/codecs/codec.h"
#include "graphics/jpeg.h"
#include "graphics/pixelformat.h"

namespace Graphics {

// Motion JPEG Decoder
// Basically a wrapper around JPEG which converts to RGB and also functions
// as a Codec.

class JPEGDecoder : public Codec {
public:
	JPEGDecoder();
	~JPEGDecoder();

	Surface *decodeImage(Common::SeekableReadStream *stream);
	PixelFormat getPixelFormat() const { return _pixelFormat; }

private:
	PixelFormat _pixelFormat;
	JPEG *_jpeg;
	Surface *_surface;
};

} // End of namespace Graphics

#endif
