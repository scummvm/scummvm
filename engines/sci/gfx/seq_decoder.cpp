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

#include "common/archive.h"
#include "sci/gfx/seq_decoder.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

SeqDecoder::~SeqDecoder() {
	closeFile();
}

bool SeqDecoder::loadFile(Common::String fileName) {
	closeFile();

	_fileStream = SearchMan.createReadStreamForMember(fileName);
	if (!_fileStream)
		return false;

	_frameCount = _fileStream->readUint16LE();
	int paletteSize = _fileStream->readUint32LE();

	byte *paletteData = new byte[paletteSize];
	_fileStream->read(paletteData, paletteSize);
	_palette = gfxr_read_pal11(-1, paletteData, paletteSize);
	delete[] paletteData;

	_currentFrame = 0;

	return true;
}

void SeqDecoder::closeFile() {
	if (!_fileStream)
		return;

	delete _fileStream;
	_fileStream = 0;

	delete _palette;
	_palette = 0;
}

#define WRITE_TO_BUFFER(n) \
	if (writeRow * width + writeCol + (n) > width * height) { \
		warning("SEQ player: writing out of bounds, aborting"); \
		return false; \
	} \
	if (litPos + (n) > litSize) { \
		warning("SEQ player: reading out of bounds, aborting"); \
	} \
	memcpy(dest + writeRow * width + writeCol, litData + litPos, n);

bool SeqDecoder::decodeFrame(byte *rleData, int rleSize, byte *litData, int litSize, byte *dest, int width, int height, int colorKey) {
	int writeRow = 0;
	int writeCol = 0;
	int litPos = 0;
	int rlePos = 0;

	memset(dest, colorKey, width * height);

	while (rlePos < rleSize) {
		int op = rleData[rlePos++];

		if ((op & 0xc0) == 0xc0) {
			op &= 0x3f;
			if (op == 0) {
				// Go to next line in target buffer
				writeRow++;
				writeCol = 0;
			} else {
				// Skip bytes on current line
				writeCol += op;
			}
		} else if (op & 0x80) {
			op &= 0x3f;
			if (op == 0) {
				// Copy remainder of current line
				int rem = width - writeCol;

				WRITE_TO_BUFFER(rem);
				writeRow++;
				writeCol = 0;
				litPos += rem;
			} else {
				// Copy bytes
				WRITE_TO_BUFFER(op);
				writeCol += op;
				litPos += op;
			}
		} else {
			uint16 count = ((op & 7) << 8) | rleData[rlePos++];

			switch (op >> 3) {
			case 2:
				// Skip bytes
				writeCol += count;
				break;
			case 3:
				// Copy bytes
				WRITE_TO_BUFFER(count);
				writeCol += count;
				litPos += count;
				break;
			case 6: {
				// Copy rows
				if (count == 0)
					count = height - writeRow;

				for (int i = 0; i < count; i++) {
					WRITE_TO_BUFFER(width);
					litPos += width;
					writeRow++;
				}
				break;
			}
			case 7:
				// Skip rows
				if (count == 0)
					count = height - writeRow;

				writeRow += count;
				break;
			default:
				warning("Unsupported operation %i encountered", op >> 3);
				return false;
			}
		}
	}

	return true;
}

gfx_pixmap_t *SeqDecoder::getFrame(bool &hasNext) {
	int frameWidth = _fileStream->readUint16LE();
	int frameHeight = _fileStream->readUint16LE();
	int frameLeft = _fileStream->readUint16LE();
	int frameTop = _fileStream->readUint16LE();
	int colorKey = _fileStream->readByte();
	int type = _fileStream->readByte();
	_fileStream->seek(2, SEEK_CUR);
	uint16 bytes = _fileStream->readUint16LE();
	_fileStream->seek(2, SEEK_CUR);
	uint16 rle_bytes = _fileStream->readUint16LE();
	_fileStream->seek(6, SEEK_CUR);
	uint32 offset = _fileStream->readUint32LE();

	_fileStream->seek(offset);
	gfx_pixmap_t *pixmap = gfx_new_pixmap(frameWidth, frameHeight, 0, 0, 0);

	assert(pixmap);

	gfx_pixmap_alloc_index_data(pixmap);

	if (type == 0)
		_fileStream->read(pixmap->index_data, bytes);
	else {
		byte *buf = new byte[bytes];
		_fileStream->read(buf, bytes);
		decodeFrame(buf, rle_bytes, buf + rle_bytes, bytes - rle_bytes, pixmap->index_data, frameWidth, frameHeight, colorKey);
	}

	pixmap->xoffset = frameLeft;
	pixmap->yoffset = frameTop;
	pixmap->color_key = colorKey;
	pixmap->palette = _palette->getref();

	hasNext = ++_currentFrame < _frameCount;

	return pixmap;
}

} // End of namespace Sci
