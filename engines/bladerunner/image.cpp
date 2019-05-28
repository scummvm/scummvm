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

#include "bladerunner/image.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/decompress_lcw.h"

#include "common/rect.h"

namespace BladeRunner {

Image::Image(BladeRunnerEngine *vm) {
	_vm = vm;
}

Image::~Image() {
	_surface.free();
}

bool Image::open(const Common::String &name) {
	Common::SeekableReadStream *stream = _vm->getResourceStream(name);
	if (!stream) {
		warning("Image::open failed to open '%s'\n", name.c_str());
		return false;
	}

	char tag[4] = { 0 };
	stream->read(tag, 3);
	uint32 width  = stream->readUint32LE();
	uint32 height = stream->readUint32LE();

	// Enforce a reasonable limit
	assert(width < 8000 && height < 8000);

	uint32 bufSize = stream->size();
	uint8 *buf = new uint8[bufSize];
	stream->read(buf, bufSize);

	uint32 dataSize = 2 * width * height;
	void *data = malloc(dataSize);
	assert(data);

	if (strcmp(tag, "LZO") == 0) {
		warning("LZO image decompression is not implemented");
	} else if (strcmp(tag, "LCW") == 0) {
		decompress_lcw(buf, bufSize, (uint8 *)data, dataSize);
#ifdef SCUMM_BIG_ENDIAN
		// As the compression is working with 8-bit data, on big-endian architectures we have to switch order of bytes in uncompressed data
		uint8 *rawData = (uint8 *)data;
		for (size_t i = 0; i < dataSize - 1; i += 2) {
			SWAP(rawData[i], rawData[i + 1]);
		}
#endif
	}

	_surface.init(width, height, 2*width, data, gameDataPixelFormat());
	_surface.convertToInPlace(screenPixelFormat());

	delete[] buf;
	delete stream;

	return true;
}

void Image::copyToSurface(Graphics::Surface *dst) const {
	dst->copyRectToSurface(_surface, 0, 0, Common::Rect(_surface.w, _surface.h));
}

} // End of namespace BladeRunner
