/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "image/cgbi.h"
#include "common/compression/deflate.h"
#include "common/crc.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/debug.h"

namespace Image {

CgBIDecoder::CgBIDecoder() : _surface(nullptr), _palette(0) {}

CgBIDecoder::~CgBIDecoder() {
	destroy();
}

void CgBIDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}
	_palette.clear();
}

bool CgBIDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();
	if (stream.readUint32BE() != MKTAG(0x89, 'P', 'N', 'G')) {
		debug(1, "Invalid file format");
		return false;
	}
	stream.seek(0, SEEK_SET);
	byte buffer[8];
	stream.read(buffer, 8);
	for (int i = 0; i < 8; i++)
		debug(1, "%X", buffer[i]);

	uint32 width = 0, height = 0;
	uint8 bitDepth = 0, colorType = 0;
	uint32 bpp = 0;

	bool inIdat = false;
	bool idatProcessed = false;
	Common::MemoryWriteStreamDynamic idatData(DisposeAfterUse::YES);

	while (true) {
		uint32 size = stream.readUint32BE();
		uint32 type = stream.readUint32BE();
		if (!size && type != MKTAG('I', 'E', 'N', 'D')) {
			debug(1, "End of file.");
			break;
		}
		if (type == MKTAG('C', 'g', 'B', 'I')) {
			stream.seek(size + 4, SEEK_CUR);
			continue;
		}
		if (type == MKTAG('I', 'H', 'D', 'R')) {
			uint8 *payloadBuffer = new uint8[size + 4];
			stream.read(payloadBuffer, size + 4);

			width = READ_BE_UINT32(payloadBuffer);
			height = READ_BE_UINT32(payloadBuffer + 4);
			bitDepth = payloadBuffer[8];
			colorType = payloadBuffer[9];

			switch (colorType) {
			case 0:
				bpp = bitDepth / 8; // greyscale
				break;
			case 2:
				bpp = 3 * (bitDepth / 8); // RGB
				break;
			case 3:
				bpp = 1; // palette
				break;
			case 4:
				bpp = 2 * (bitDepth / 8); // greyscale + alpha
				break;
			case 6:
				bpp = 4 * (bitDepth / 8); // RGBA
				break;
			}
			delete[] payloadBuffer;
			continue;
		}
		if (type == MKTAG('I', 'D', 'A', 'T')) {
			inIdat = true;
			uint8 *payloadBuffer = new uint8[size + 4];
			stream.read(payloadBuffer, size + 4);
			idatData.write(payloadBuffer, size);
			delete[] payloadBuffer;
			continue;
		}
		if (inIdat && !idatProcessed) {
			Common::MemoryReadStream *rawCompressed = new Common::MemoryReadStream(idatData.getData(), idatData.size(), DisposeAfterUse::NO);
			Common::SeekableReadStream *deCompressed = Common::wrapDeflateReadStream(rawCompressed, DisposeAfterUse::YES);
			uint32 decompressedSize = height * (1 + width * bpp);
			uint8 *filteredBuffer = new uint8[decompressedSize];
			deCompressed->read(filteredBuffer, decompressedSize);
			delete deCompressed;

			uint8 *pixelBuffer = new uint8[height * width * 4];
			unfilterScanlines(filteredBuffer, width, height, bpp);
			convertBGRAtoRGBA(filteredBuffer, pixelBuffer, width, height, bpp, colorType == 6);
			delete[] filteredBuffer;

			_surface = new Graphics::Surface();
			_surface->create(width, height, Graphics::PixelFormat::createFormatRGBA32());
			memcpy(_surface->getPixels(), pixelBuffer, width * height * 4);
			delete[] pixelBuffer;

			idatProcessed = true;
			break;
		}

		if (size > (uint32)(stream.size() - stream.pos())) {
			debug(1, "Corrupt chunk length, aborting!");
			return false;
		}
		uint8 *payloadBuffer = new uint8[size + 4];
		stream.read(payloadBuffer, size + 4);
		delete[] payloadBuffer;
		if (type == MKTAG('I', 'E', 'N', 'D'))
			break;
	}
	return idatProcessed;
}

void CgBIDecoder::unfilterScanline(uint8 *scanline, uint8 *prev, int scanlineLen, int bpp) {
	uint8 filter = scanline[0];
	uint8 *dst = scanline + 1;

	switch (filter) {
	case 0: // data is already raw
		break;
	case 1: // sub
		for (int i = 0; i < scanlineLen; i ++) {
			uint8 left = (i >= bpp) ? dst[i - bpp] : 0;
			dst[i] += left;
		}
		break;
	case 2: // up
		if (prev)
			for (int i = 0; i < scanlineLen; i++)
				dst[i] += prev[i];
		break;
	case 3: // average
		for (int i = 0; i < scanlineLen; i++) {
			int left = (i >= bpp) ? dst[i - bpp] : 0;
			int above = prev ? prev[i] : 0;
			dst[i] += (left + above) / 2;
		}
		break;
	case 4: // paeth
		for (int i = 0; i < scanlineLen; i++) {
			int a = (i >= bpp) ? dst[i - bpp] : 0;
			int b = prev ? prev[i] : 0;
			int c = (i >= bpp && prev) ? prev[i - bpp] : 0;
			int p = a + b - c;
			int pa = ABS(p - a);
			int pb = ABS(p - b);
			int pc = ABS(p - c);
			int pr = (pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c;
			dst[i] += pr;
		}
		break;
	default:
		break;
	}
}

void CgBIDecoder::unfilterScanlines(uint8 *filtered, uint32 width, uint32 height, uint32 bpp) {
	uint32 rowBytes = width * bpp;
	uint8 *prev = nullptr;

	for (uint32 y = 0; y < height; y++) {
		uint8 *scanline = filtered + (size_t)y * (1 + rowBytes);
		unfilterScanline(scanline, prev, rowBytes, bpp);
		prev = scanline + 1;
	}
}

void CgBIDecoder::convertBGRAtoRGBA(const byte *filtered, byte *out, uint32 width, uint32 height, uint32 bpp, bool hasAlpha) {
	uint32 rowBytes = width * bpp;
	for (uint32 y = 0; y < height; y++) {
		const byte *src = filtered + (size_t)y * (1 + rowBytes) + 1;
		byte *rowOut = out + (size_t)y * width * 4;

		for (uint32 x = 0; x < width; x++) {
			byte b = src[0];
			byte g = src[1];
			byte r = src[2];
			byte a;

			if (hasAlpha) {
				a = src[3];
				if (a > 0 && a < 255) {
					r = (byte)((uint32)r * 255 / a);
					g = (byte)((uint32)g * 255 / a);
					b = (byte)((uint32)b * 255 / a);
				}
				src += 4;
			} else {
				a = 255;
				src += 3;
			}
			rowOut[0] = r;
			rowOut[1] = g;
			rowOut[2] = b;
			rowOut[3] = a;
			rowOut += 4;
		}
	}
}

} // End of namespace Image
