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

#include "graphics/imagedec.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "common/file.h"

namespace Graphics {
//
// BMP Decoder
//
class BMPDecoder : public ImageDecoder {
public:
	BMPDecoder() {}
	virtual ~BMPDecoder() {}

	bool decodeable(Common::SeekableReadStream &stream);
	Surface *decodeImage(Common::SeekableReadStream &stream, const PixelFormat &format);

	struct BitmapHeader {
		uint16 type;
		uint32 size;
		uint16 res1;
		uint16 res2;
		uint32 imageOffset;
	};

	struct InfoHeader {
		uint32 size;
		uint32 width;
		uint32 height;
		uint16 planes;
		uint16 bitsPerPixel;
		uint32 compression;
		uint32 imageSize;
		uint32 pixelsPerMeterX;
		uint32 pixelsPerMeterY;
		uint32 colorsUsed;
		uint32 colorsImportant;
	};
};

bool BMPDecoder::decodeable(Common::SeekableReadStream &stream) {
	BitmapHeader header;
	stream.seek(0);
	header.type = stream.readUint16BE();
	header.size = stream.readUint32LE();

	// TODO: maybe improve this detection
	if (header.size == 0 || header.type != 'BM')
		return false;

	return true;
}

Surface *BMPDecoder::decodeImage(Common::SeekableReadStream &stream, const PixelFormat &format) {
	if (!decodeable(stream)) {
		return 0;
	}

	BitmapHeader header;
	InfoHeader info;

	stream.seek(0);
	header.type = stream.readUint16BE();
	header.size = stream.readUint32LE();
	header.res1 = stream.readUint16LE();
	header.res2 = stream.readUint16LE();
	header.imageOffset = stream.readUint32LE();

	if (header.size == 0 || header.type != 'BM') {
		stream.seek(0);
		return 0;
	}

	info.size = stream.readUint32LE();
	info.width = stream.readUint32LE();
	info.height = stream.readUint32LE();
	info.planes = stream.readUint16LE();
	info.bitsPerPixel = stream.readUint16LE();
	info.compression = stream.readUint32LE();
	info.imageSize = stream.readUint32LE();
	info.pixelsPerMeterX = stream.readUint32LE();
	info.pixelsPerMeterY = stream.readUint32LE();
	info.colorsUsed = stream.readUint32LE();
	info.colorsImportant = stream.readUint32LE();

	stream.seek(header.imageOffset);

	if (info.bitsPerPixel != 24) {
		stream.seek(0);
		return 0;
	}

	uint8 r = 0, g = 0, b = 0;
	Surface *newSurf = new Surface;
	assert(newSurf);
	newSurf->create(info.width, info.height, format);
	assert(newSurf->pixels);
	OverlayColor *curPixel = (OverlayColor*)newSurf->pixels + (newSurf->h-1) * newSurf->w;
	int pitchAdd = info.width % 4;
	for (int i = 0; i < newSurf->h; ++i) {
		for (int i2 = 0; i2 < newSurf->w; ++i2) {
			b = stream.readByte();
			g = stream.readByte();
			r = stream.readByte();
			*curPixel = format.RGBToColor(r, g, b);
			++curPixel;
		}
		stream.seek(pitchAdd, SEEK_CUR);
		curPixel -= newSurf->w*2;
	}

	stream.seek(0);
	return newSurf;
}

#pragma mark -

Surface *ImageDecoder::loadFile(const Common::String &name, const PixelFormat &format) {
	Surface *newSurf = 0;

	Common::File imageFile;
	if (imageFile.open(name)) {
		newSurf = loadFile(imageFile, format);
	}

	return newSurf;
}

Surface *ImageDecoder::loadFile(Common::SeekableReadStream &stream, const PixelFormat &format) {
	// TODO: implement support for bzipped memory

	// FIXME: this is not a very nice solution but it should work
	// for the moment, we should use a different way to get all
	// decoders
	static BMPDecoder bmpDecoder;
	static ImageDecoder *decoderList[] = {
		&bmpDecoder,			// for uncompressed .BMP files
		0
	};

	ImageDecoder *decoder = 0;
	for (int i = 0; decoderList[i] != 0; ++i) {
		if (decoderList[i]->decodeable(stream)) {
			decoder = decoderList[i];
			break;
		}
	}

	if (!decoder)
		return 0;

	return decoder->decodeImage(stream, format);
}
} // End of namespace Graphics
