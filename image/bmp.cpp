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

#include "image/bmp.h"

#include "common/stream.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "image/codecs/codec.h"

namespace Image {

BitmapDecoder::BitmapDecoder() {
	_surface = 0;
	_palette = 0;
	_paletteColorCount = 0;
	_codec = 0;
}

BitmapDecoder::~BitmapDecoder() {
	destroy();
}

void BitmapDecoder::destroy() {
	_surface = 0;

	delete[] _palette;
	_palette = 0;

	_paletteColorCount = 0;

	delete _codec;
	_codec = 0;
}

bool BitmapDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	if (stream.readByte() != 'B')
		return false;

	if (stream.readByte() != 'M')
		return false;

	/* uint32 fileSize = */ stream.readUint32LE();
	/* uint16 res1 = */ stream.readUint16LE();
	/* uint16 res2 = */ stream.readUint16LE();
	uint32 imageOffset = stream.readUint32LE();

	uint32 infoSize = stream.readUint32LE();
	if (infoSize != 40 && infoSize != 108) {
		warning("Only Windows v3 & v4 bitmaps are supported");
		return false;
	}

	uint32 width = stream.readUint32LE();
	int32 height = stream.readSint32LE();

	if (width == 0 || height == 0)
		return false;

	if (height < 0) {
		warning("Right-side up bitmaps not supported");
		return false;
	}

	/* uint16 planes = */ stream.readUint16LE();
	uint16 bitsPerPixel = stream.readUint16LE();

	if (bitsPerPixel != 8 && bitsPerPixel != 24 && bitsPerPixel != 32) {
		warning("%dbpp bitmaps not supported", bitsPerPixel);
		return false;
	}

	uint32 compression = stream.readUint32BE();
	uint32 imageSize = stream.readUint32LE();
	/* uint32 pixelsPerMeterX = */ stream.readUint32LE();
	/* uint32 pixelsPerMeterY = */ stream.readUint32LE();
	_paletteColorCount = stream.readUint32LE();
	/* uint32 colorsImportant = */ stream.readUint32LE();

	if (bitsPerPixel == 8) {
		if (_paletteColorCount == 0)
			_paletteColorCount = 256;

		// Read the palette
		_palette = new byte[_paletteColorCount * 3];
		for (uint16 i = 0; i < _paletteColorCount; i++) {
			_palette[i * 3 + 2] = stream.readByte();
			_palette[i * 3 + 1] = stream.readByte();
			_palette[i * 3 + 0] = stream.readByte();
			stream.readByte();
		}
	}

	// Create the codec (it will warn about unhandled compression)
	_codec = createBitmapCodec(compression, 0, width, height, bitsPerPixel);
	if (!_codec)
		return false;

	// If the image size is zero, set it to the rest of the stream.
	if (imageSize == 0)
		imageSize = stream.size() - imageOffset;

	// Grab the frame data
	Common::SeekableSubReadStream subStream(&stream, imageOffset, imageOffset + imageSize);

	// We only support raw bitmaps for now
	_surface = _codec->decodeFrame(subStream);

	return true;
}

bool writeBMP(Common::WriteStream &out, const Graphics::Surface &input) {
#ifdef SCUMM_LITTLE_ENDIAN
	const Graphics::PixelFormat requiredFormat_3byte(3, 8, 8, 8, 0, 16, 8, 0, 0);
#else
	const Graphics::PixelFormat requiredFormat_3byte(3, 8, 8, 8, 0, 0, 8, 16, 0);
#endif

	Graphics::Surface *tmp = NULL;
	const Graphics::Surface *surface;

	if (input.format == requiredFormat_3byte) {
		surface = &input;
	} else {
		surface = tmp = input.convertTo(requiredFormat_3byte);
	}

	int dstPitch = surface->w * 3;
	int extraDataLength = (dstPitch % 4) ? 4 - (dstPitch % 4) : 0;
	int padding = 0;

	out.writeByte('B');
	out.writeByte('M');
	out.writeUint32LE(surface->h * dstPitch + 54);
	out.writeUint32LE(0);
	out.writeUint32LE(54);
	out.writeUint32LE(40);
	out.writeUint32LE(surface->w);
	out.writeUint32LE(surface->h);
	out.writeUint16LE(1);
	out.writeUint16LE(24);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);


	for (uint y = surface->h; y-- > 0;) {
		out.write((const void *)surface->getBasePtr(0, y), dstPitch);
		out.write(&padding, extraDataLength);
	}

	// free tmp surface
	if (tmp) {
		tmp->free();
		delete tmp;
	}

	return true;
}

} // End of namespace Image
