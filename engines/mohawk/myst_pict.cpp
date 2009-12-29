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

#include "mohawk/myst_pict.h"

namespace Mohawk {
	
// The PICT code is based off of the QuickDraw specs:
// http://developer.apple.com/documentation/mac/QuickDraw/QuickDraw-461.html

MystPICT::MystPICT(MystJPEG *jpegDecoder) {
	_jpegDecoder = jpegDecoder;
	_pixelFormat = g_system->getScreenFormat();
}

Graphics::Surface *MystPICT::decodeImage(Common::SeekableReadStream *stream) {
	// Skip initial 512 bytes (all 0's)
	stream->seek(512, SEEK_CUR);

	// Read in the first part of the header
	/* uint16 fileSize = */ stream->readUint16BE();
	
	_imageRect.top = stream->readUint16BE();
	_imageRect.left = stream->readUint16BE();
	_imageRect.bottom = stream->readUint16BE();
	_imageRect.right = stream->readUint16BE();
	_imageRect.debugPrint(0, "PICT Rect:");
	
	Graphics::Surface *image = new Graphics::Surface();
	image->create(_imageRect.width(), _imageRect.height(), _pixelFormat.bytesPerPixel);
	
	// NOTE: This is only a subset of the full PICT format.
	//     - Only V2 Images Supported
	//     - JPEG Chunks are Supported
	//     - DirectBitsRect Chunks are Supported
	for (uint32 opNum = 0; !stream->eos() && !stream->err() && stream->pos() < stream->size(); opNum++) {
		uint16 opcode = stream->readUint16BE();
		debug(2, "Found PICT opcode %04x", opcode);
		
		if (opNum == 0 && opcode != 0x0011)
			error ("Cannot find PICT version opcode");
		else if (opNum == 1 && opcode != 0x0C00)
			error ("Cannot find PICT header opcode");
		
		if (opcode == 0x0001) {		   // Clip
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
		} else if (opcode == 0x009A) { // DirectBitsRect
			decodeDirectBitsRect(stream, image);
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
			decodeCompressedQuickTime(stream, image);
			break;
		} else {
			error ("Unknown PICT opcode %04x", opcode);
		}
	}
	
	return image;
}

struct DirectBitsRectData {
	// PixMap
	struct {
		uint32 baseAddr;
		uint16 rowBytes;
		Common::Rect bounds;
		uint16 pmVersion;
		uint16 packType;
		uint32 packSize;
		uint32 hRes;
		uint32 vRes;
		uint16 pixelType;
		uint16 pixelSize;
		uint16 cmpCount;
		uint16 cmpSize;
		uint32 planeBytes;
		uint32 pmTable;
		uint32 pmReserved;
	} pixMap;
	Common::Rect srcRect;
	Common::Rect dstRect;
	uint16 mode;
};

void MystPICT::decodeDirectBitsRect(Common::SeekableReadStream *stream, Graphics::Surface *image) {
	static const Graphics::PixelFormat directBitsFormat = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);

	Graphics::Surface buffer;
	buffer.create(image->w, image->h, 2);

	DirectBitsRectData directBitsData;
	
	directBitsData.pixMap.baseAddr = stream->readUint32BE();
	directBitsData.pixMap.rowBytes = stream->readUint16BE();
	directBitsData.pixMap.bounds.top = stream->readUint16BE();
	directBitsData.pixMap.bounds.left = stream->readUint16BE();
	directBitsData.pixMap.bounds.bottom = stream->readUint16BE();
	directBitsData.pixMap.bounds.right = stream->readUint16BE();
	directBitsData.pixMap.pmVersion = stream->readUint16BE();
	directBitsData.pixMap.packType = stream->readUint16BE();
	directBitsData.pixMap.packSize = stream->readUint32BE();
	directBitsData.pixMap.hRes = stream->readUint32BE();
	directBitsData.pixMap.vRes = stream->readUint32BE();
	directBitsData.pixMap.pixelType = stream->readUint16BE();
	directBitsData.pixMap.pixelSize = stream->readUint16BE();
	directBitsData.pixMap.cmpCount = stream->readUint16BE();
	directBitsData.pixMap.cmpSize = stream->readUint16BE();
	directBitsData.pixMap.planeBytes = stream->readUint32BE();
	directBitsData.pixMap.pmTable = stream->readUint32BE();
	directBitsData.pixMap.pmReserved = stream->readUint32BE();
	directBitsData.srcRect.top = stream->readUint16BE();
	directBitsData.srcRect.left = stream->readUint16BE();
	directBitsData.srcRect.bottom = stream->readUint16BE();
	directBitsData.srcRect.right = stream->readUint16BE();
	directBitsData.dstRect.top = stream->readUint16BE();
	directBitsData.dstRect.left = stream->readUint16BE();
	directBitsData.dstRect.bottom = stream->readUint16BE();
	directBitsData.dstRect.right = stream->readUint16BE();
	directBitsData.mode = stream->readUint16BE();
	
	if (directBitsData.pixMap.pixelSize != 16)
		error("Unhandled directBitsRect bitsPerPixel %d", directBitsData.pixMap.pixelSize);
	
	// Read in amount of data per row
	for (uint16 i = 0; i < directBitsData.pixMap.bounds.height(); i++) {
		if (directBitsData.pixMap.packType == 1 || directBitsData.pixMap.rowBytes < 8) { // Unpacked, Pad-Byte
			error("Pack Type = %d, Row Bytes = %d", directBitsData.pixMap.packType, directBitsData.pixMap.rowBytes);
			// TODO
		} else if (directBitsData.pixMap.packType == 2) { // Unpacked, No Pad-Byte
			error("Pack Type = 2");
			// TODO
		} else if (directBitsData.pixMap.packType > 2) { // Packed
			uint16 byteCount = (directBitsData.pixMap.rowBytes > 250) ? stream->readUint16BE() : stream->readByte();
			decodeDirectBitsLine((byte *)buffer.getBasePtr(0, i), directBitsData.pixMap.rowBytes, stream->readStream(byteCount));
		}
	}
	
	// Convert from 16-bit to whatever surface we need
	for (uint16 y = 0; y < buffer.h; y++) {
		for (uint16 x = 0; x < buffer.w; x++) {
			byte r = 0, g = 0, b = 0;
			uint16 color = READ_BE_UINT16(buffer.getBasePtr(x, y));
			directBitsFormat.colorToRGB(color, r, g, b);
			*((uint16 *)image->getBasePtr(x, y)) = _pixelFormat.RGBToColor(r, g, b);
		}
	}
}

void MystPICT::decodeDirectBitsLine(byte *out, uint32 length, Common::SeekableReadStream *data) {
	uint32 dataDecoded = 0;
	while (data->pos() < data->size() && dataDecoded < length) {
		byte op = data->readByte();
		
		if (op & 0x80) {
			uint32 runSize = (op ^ 255) + 2;
			byte value1 = data->readByte();
			byte value2 = data->readByte();
			for (uint32 i = 0; i < runSize; i++) {
				*out++ = value1;
				*out++ = value2;
			}
			dataDecoded += runSize * 2;
		} else {
			uint32 runSize = (op + 1) * 2;
			for (uint32 i = 0; i < runSize; i++)
				*out++ = data->readByte();
			dataDecoded += runSize;
		}
	}
	
	if (length != dataDecoded)
		warning("Mismatched DirectBits read");
	delete data;
}
	
// Compressed QuickTime details can be found here: 
// http://developer.apple.com/documentation/QuickTime/Rm/CompressDecompress/ImageComprMgr/B-Chapter/2TheImageCompression.html
// http://developer.apple.com/documentation/QuickTime/Rm/CompressDecompress/ImageComprMgr/F-Chapter/6WorkingwiththeImage.html
// I'm just ignoring that because Myst ME uses none of that extra stuff. The offset is always the same.

void MystPICT::decodeCompressedQuickTime(Common::SeekableReadStream *stream, Graphics::Surface *image) {
	uint32 dataSize = stream->readUint32BE();
	uint32 startPos = stream->pos();
	
	Graphics::Surface *jpegImage = _jpegDecoder->decodeImage(new Common::SeekableSubReadStream(stream, stream->pos() + 156, stream->pos() + dataSize));
	stream->seek(startPos + dataSize);
	
	image->copyFrom(*jpegImage);
	
	jpegImage->free();
	delete jpegImage;
}

} // End of namespace Mohawk
