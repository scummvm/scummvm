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

#include "image/gif.h"

#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_GIF
#define STBI_NO_STDIO
#include "image/stb_image.h"

namespace Image {

GIFDecoder::GIFDecoder() : _outputSurface(0) {
}

GIFDecoder::~GIFDecoder() {
	destroy();
}

static Graphics::PixelFormat getByteOrderRgbaPixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#else
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#endif
}

bool GIFDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();
	const int32 size = stream.size();
	if (size == -1) {
		return false;
	}
	uint8 *buf = (uint8 *)malloc(size);
	if ((uint32)size != stream.read(buf, size)) {
		return false;
	}
	int w = 0;
	int h = 0;
	int n = 0;
	int z = 0;
	int *delays = 0;

	const Graphics::PixelFormat pixelFormat = getByteOrderRgbaPixelFormat();
	assert(pixelFormat.bytesPerPixel == STBI_rgb_alpha);
	uint8 *data = stbi_load_gif_from_memory(buf, size, &delays, &w, &h, &z, &n, STBI_rgb_alpha);
	if (data == 0) {
		return false;
	}

	if (n != pixelFormat.bytesPerPixel) {
		stbi_image_free(data);
		warning("GIF has more unsupported color depth: %i", n);
		return false;
	}

	if (z > 1) {
		stbi_image_free(data);
		warning("GIF has more than 1 frame: %i", z);
		return false;
	}
	_outputSurface = new Graphics::Surface();
	_outputSurface->create(w, h, pixelFormat);
	memcpy(_outputSurface->getPixels(), data, w * h * n);
	stbi_image_free(data);
	return true;
}

void GIFDecoder::destroy() {
	if (_outputSurface) {
		_outputSurface->free();
		delete _outputSurface;
		_outputSurface = 0;
	}
}

} // End of namespace Image
