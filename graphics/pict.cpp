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
 */

#include "common/debug.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/textconsole.h"

#include "graphics/jpeg.h"
#include "graphics/pict.h"
#include "graphics/surface.h"

namespace Graphics {

// The PICT code is based off of the QuickDraw specs:
// http://developer.apple.com/legacy/mac/library/documentation/mac/QuickDraw/QuickDraw-461.html
// http://developer.apple.com/legacy/mac/library/documentation/mac/QuickDraw/QuickDraw-269.html

PictDecoder::PictDecoder(PixelFormat pixelFormat) {
	_jpeg = new JPEG();
	_pixelFormat = pixelFormat;
}

PictDecoder::~PictDecoder() {
	delete _jpeg;
}

Surface *PictDecoder::decodeImage(Common::SeekableReadStream *stream, byte *palette) {
	assert(stream);

	_outputSurface = 0;

	uint16 fileSize = stream->readUint16BE();

	// If we have no file size here, we probably have a PICT from a file
	// and not a resource. The other two bytes are the fileSize which we
	// don't actually need (and already read if from a resource).
	if (!fileSize)
		stream->seek(512 + 2);

	_imageRect.top = stream->readUint16BE();
	_imageRect.left = stream->readUint16BE();
	_imageRect.bottom = stream->readUint16BE();
	_imageRect.right = stream->readUint16BE();
	_imageRect.debugPrint(0, "PICT Rect:");
	_isPaletted = false;

	// NOTE: This is only a subset of the full PICT format.
	//     - Only V2 Images Supported
	//     - CompressedQuickTime (JPEG) compressed data is supported
	//     - DirectBitsRect/PackBitsRect compressed data is supported
	for (uint32 opNum = 0; !stream->eos() && !stream->err() && stream->pos() < stream->size(); opNum++) {
		uint16 opcode = stream->readUint16BE();
		debug(2, "Found PICT opcode %04x", opcode);

		if (opNum == 0 && opcode != 0x0011)
			error ("Cannot find PICT version opcode");
		else if (opNum == 1 && opcode != 0x0C00)
			error ("Cannot find PICT header opcode");

		if (opcode == 0x0000) {        // Nop
			stream->readUint16BE(); // Unknown
		} else if (opcode == 0x0001) { // Clip
			// Ignore
			uint16 clipSize = stream->readUint16BE();
			stream->seek(clipSize - 2, SEEK_CUR);
		} else if (opcode == 0x0007) { // PnSize
			// Ignore
			stream->readUint16BE();
			stream->readUint16BE();
		} else if (opcode == 0x0011) { // VersionOp
			uint16 version = stream->readUint16BE();
			if (version != 0x02FF)
				error ("Unknown PICT version");
		} else if (opcode == 0x001E) { // DefHilite
			// Ignore, Contains no Data
		} else if (opcode == 0x0098) { // PackBitsRect
			decodeDirectBitsRect(stream, true);
			_isPaletted = true;
		} else if (opcode == 0x009A) { // DirectBitsRect
			decodeDirectBitsRect(stream, false);
		} else if (opcode == 0x00A1) { // LongComment
			stream->readUint16BE();
			uint16 dataSize = stream->readUint16BE();
			stream->seek(dataSize, SEEK_CUR);
		} else if (opcode == 0x00FF) { // OpEndPic
			stream->readUint16BE();
			break;
		} else if (opcode == 0x0C00) { // HeaderOp
			/* uint16 version = */ stream->readUint16BE();
			stream->readUint16BE(); // Reserved
			/* uint32 hRes = */ stream->readUint32BE();
			/* uint32 vRes = */ stream->readUint32BE();
			Common::Rect origResRect;
			origResRect.top = stream->readUint16BE();
			origResRect.left = stream->readUint16BE();
			origResRect.bottom = stream->readUint16BE();
			origResRect.right = stream->readUint16BE();
			stream->readUint32BE(); // Reserved
		} else if (opcode == 0x8200) { // CompressedQuickTime
			decodeCompressedQuickTime(stream);
			break;
		} else {
			warning("Unknown PICT opcode %04x", opcode);
		}
	}

	// If we got a palette throughout this nonsense, go and grab it
	if (palette && _isPaletted)
		memcpy(palette, _palette, 256 * 3);

	return _outputSurface;
}

PictDecoder::PixMap PictDecoder::readPixMap(Common::SeekableReadStream *stream, bool hasBaseAddr) {
	PixMap pixMap;
	pixMap.baseAddr = hasBaseAddr ? stream->readUint32BE() : 0;
	pixMap.rowBytes = stream->readUint16BE() & 0x3fff;
	pixMap.bounds.top = stream->readUint16BE();
	pixMap.bounds.left = stream->readUint16BE();
	pixMap.bounds.bottom = stream->readUint16BE();
	pixMap.bounds.right = stream->readUint16BE();
	pixMap.pmVersion = stream->readUint16BE();
	pixMap.packType = stream->readUint16BE();
	pixMap.packSize = stream->readUint32BE();
	pixMap.hRes = stream->readUint32BE();
	pixMap.vRes = stream->readUint32BE();
	pixMap.pixelType = stream->readUint16BE();
	pixMap.pixelSize = stream->readUint16BE();
	pixMap.cmpCount = stream->readUint16BE();
	pixMap.cmpSize = stream->readUint16BE();
	pixMap.planeBytes = stream->readUint32BE();
	pixMap.pmTable = stream->readUint32BE();
	pixMap.pmReserved = stream->readUint32BE();
	return pixMap;
}

struct DirectBitsRectData {
	PictDecoder::PixMap pixMap;
	Common::Rect srcRect;
	Common::Rect dstRect;
	uint16 mode;
};

void PictDecoder::decodeDirectBitsRect(Common::SeekableReadStream *stream, bool hasPalette) {
	static const PixelFormat directBitsFormat16 = PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);

	// Clear the palette
	memset(_palette, 0, sizeof(_palette));

	DirectBitsRectData directBitsData;
	directBitsData.pixMap = readPixMap(stream, !hasPalette);

	// Read in the palette if there is one present
	if (hasPalette) {
		// See http://developer.apple.com/legacy/mac/library/documentation/mac/QuickDraw/QuickDraw-267.html
		stream->readUint32BE(); // seed
		stream->readUint16BE(); // flags
		uint16 colorCount = stream->readUint16BE() + 1;

		for (uint32 i = 0; i < colorCount; i++) {
			stream->readUint16BE();
			_palette[i * 3] = stream->readUint16BE() >> 8;
			_palette[i * 3 + 1] = stream->readUint16BE() >> 8;
			_palette[i * 3 + 2] = stream->readUint16BE() >> 8;
		}
	}

	directBitsData.srcRect.top = stream->readUint16BE();
	directBitsData.srcRect.left = stream->readUint16BE();
	directBitsData.srcRect.bottom = stream->readUint16BE();
	directBitsData.srcRect.right = stream->readUint16BE();
	directBitsData.dstRect.top = stream->readUint16BE();
	directBitsData.dstRect.left = stream->readUint16BE();
	directBitsData.dstRect.bottom = stream->readUint16BE();
	directBitsData.dstRect.right = stream->readUint16BE();
	directBitsData.mode = stream->readUint16BE();

	uint16 width = directBitsData.srcRect.width();
	uint16 height = directBitsData.srcRect.height();

	byte bytesPerPixel = 0;

	if (directBitsData.pixMap.pixelSize <= 8)
		bytesPerPixel = 1;
	else if (directBitsData.pixMap.pixelSize == 32)
		bytesPerPixel = 3;
	else
		bytesPerPixel = directBitsData.pixMap.pixelSize / 8;

	_outputSurface = new Graphics::Surface();
	_outputSurface->create(width, height, (bytesPerPixel == 1) ? PixelFormat::createFormatCLUT8() : _pixelFormat);
	byte *buffer = new byte[width * height * bytesPerPixel];

	// Read in amount of data per row
	for (uint16 i = 0; i < directBitsData.pixMap.bounds.height(); i++) {
		// NOTE: Compression 0 is "default". The format in SCI games is packed when 0.
		// In the future, we may need to have something to set the  "default" packing
		// format, but this is good for now.

		if (directBitsData.pixMap.packType == 1 || directBitsData.pixMap.rowBytes < 8) { // Unpacked, Pad-Byte (on 24-bit)
			// TODO: Finish this. Hasn't been needed (yet).
			error("Unpacked DirectBitsRect data (padded)");
		} else if (directBitsData.pixMap.packType == 2) { // Unpacked, No Pad-Byte (on 24-bit)
			// TODO: Finish this. Hasn't been needed (yet).
			error("Unpacked DirectBitsRect data (not padded)");
		} else if (directBitsData.pixMap.packType == 0 || directBitsData.pixMap.packType > 2) { // Packed
			uint16 byteCount = (directBitsData.pixMap.rowBytes > 250) ? stream->readUint16BE() : stream->readByte();
			decodeDirectBitsLine(buffer + i * _outputSurface->w * bytesPerPixel, directBitsData.pixMap.rowBytes, stream->readStream(byteCount), directBitsData.pixMap.pixelSize, bytesPerPixel);
		}
	}

	if (bytesPerPixel == 1) {
		// Just copy to the image
		memcpy(_outputSurface->pixels, buffer, _outputSurface->w * _outputSurface->h);
	} else if (bytesPerPixel == 2) {
		// Convert from 16-bit to whatever surface we need
		for (uint16 y = 0; y < _outputSurface->h; y++) {
			for (uint16 x = 0; x < _outputSurface->w; x++) {
				byte r = 0, g = 0, b = 0;
				uint32 color = READ_BE_UINT16(buffer + (y * _outputSurface->w + x) * bytesPerPixel);
				directBitsFormat16.colorToRGB(color, r, g, b);
				if (_pixelFormat.bytesPerPixel == 2)
					*((uint16 *)_outputSurface->getBasePtr(x, y)) = _pixelFormat.RGBToColor(r, g, b);
				else
					*((uint32 *)_outputSurface->getBasePtr(x, y)) = _pixelFormat.RGBToColor(r, g, b);
			}
		}
	} else {
		// Convert from 24-bit (planar!) to whatever surface we need
		for (uint16 y = 0; y < _outputSurface->h; y++) {
			for (uint16 x = 0; x < _outputSurface->w; x++) {
				byte r = *(buffer + y * _outputSurface->w * 3 + x);
				byte g = *(buffer + y * _outputSurface->w * 3 + _outputSurface->w + x);
				byte b = *(buffer + y * _outputSurface->w * 3 + _outputSurface->w * 2 + x);
				if (_pixelFormat.bytesPerPixel == 2)
					*((uint16 *)_outputSurface->getBasePtr(x, y)) = _pixelFormat.RGBToColor(r, g, b);
				else
					*((uint32 *)_outputSurface->getBasePtr(x, y)) = _pixelFormat.RGBToColor(r, g, b);
			}
		}
	}

	delete[] buffer;
}

void PictDecoder::decodeDirectBitsLine(byte *out, uint32 length, Common::SeekableReadStream *data, byte bitsPerPixel, byte bytesPerPixel) {
	uint32 dataDecoded = 0;
	byte bytesPerDecode = (bytesPerPixel == 2) ? 2 : 1;

	while (data->pos() < data->size() && dataDecoded < length) {
		byte op = data->readByte();

		if (op & 0x80) {
			uint32 runSize = (op ^ 255) + 2;
			uint16 value = (bytesPerDecode == 2) ? data->readUint16BE() : data->readByte();

			for (uint32 i = 0; i < runSize; i++) {
				if (bytesPerDecode == 2) {
					WRITE_BE_UINT16(out, value);
					out += 2;
				} else {
					outputPixelBuffer(out, value, bitsPerPixel);
				}
			}
			dataDecoded += runSize * bytesPerDecode;
		} else {
			uint32 runSize = (op + 1) * bytesPerDecode;

			for (uint32 i = 0; i < runSize; i++)
				outputPixelBuffer(out, data->readByte(), bitsPerPixel);

			dataDecoded += runSize;
		}
	}

	// HACK: rowBytes is in 32-bit, but the data is 24-bit...
	if (bytesPerPixel == 3)
		dataDecoded += length / 4;

	if (length != dataDecoded)
		warning("Mismatched DirectBits read (%d/%d)", dataDecoded, length);

	delete data;
}

void PictDecoder::outputPixelBuffer(byte *&out, byte value, byte bitsPerPixel) {
	switch (bitsPerPixel) {
	case 1:
		for (int i = 7; i >= 0; i--)
			*out++ = (value >> i) & 1;
		break;
	case 2:
		for (int i = 6; i >= 0; i -= 2)
			*out++ = (value >> i) & 3;
		break;
	case 4:
		*out++ = (value >> 4) & 0xf;
		*out++ = value & 0xf;
		break;
	default:
		*out++ = value;
	}
}

// Compressed QuickTime details can be found here:
// http://developer.apple.com/legacy/mac/library/#documentation/QuickTime/Rm/CompressDecompress/ImageComprMgr/B-Chapter/2TheImageCompression.html
// http://developer.apple.com/legacy/mac/library/#documentation/QuickTime/Rm/CompressDecompress/ImageComprMgr/F-Chapter/6WorkingwiththeImage.html
// I'm just ignoring that because Myst ME uses none of that extra stuff. The offset is always the same.

void PictDecoder::decodeCompressedQuickTime(Common::SeekableReadStream *stream) {
	uint32 dataSize = stream->readUint32BE();
	uint32 startPos = stream->pos();

	Common::SeekableReadStream *jpegStream = new Common::SeekableSubReadStream(stream, stream->pos() + 156, stream->pos() + dataSize);

	if (!_jpeg->read(jpegStream))
		error("PictDecoder::decodeCompressedQuickTime(): Could not decode JPEG data");

	_outputSurface = _jpeg->getSurface(_pixelFormat);

	stream->seek(startPos + dataSize);
	delete jpegStream;
}

} // End of namespace Graphics
