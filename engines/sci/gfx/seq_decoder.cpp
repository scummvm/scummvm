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
#include "sci/resource.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_palette.h"

namespace Sci {

SeqDecoder::~SeqDecoder() {
	closeFile();
}

bool SeqDecoder::loadFile(Common::String fileName, ResourceManager *resMan, SciGuiScreen *screen) {
	closeFile();

	_fileStream = SearchMan.createReadStreamForMember(fileName);
	if (!_fileStream)
		return false;

	_frameCount = _fileStream->readUint16LE();
	int paletteSize = _fileStream->readUint32LE();

	byte *paletteData = new byte[paletteSize];
	_fileStream->read(paletteData, paletteSize);
	GuiPalette seqPalette;
	SciGuiPalette *pal = new SciGuiPalette(resMan, screen);
	pal->createFromData(paletteData, &seqPalette);
	pal->set(&seqPalette, 2);
	delete pal;
	delete[] paletteData;

	_currentFrame = 0;

	return true;
}

void SeqDecoder::closeFile() {
	if (!_fileStream)
		return;

	delete _fileStream;
	_fileStream = 0;
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

SeqFrame *SeqDecoder::getFrame(bool &hasNext) {
	int16 frameWidth = _fileStream->readUint16LE();
	int16 frameHeight = _fileStream->readUint16LE();
	int16 frameLeft = _fileStream->readUint16LE();
	int16 frameTop = _fileStream->readUint16LE();
	byte colorKey = _fileStream->readByte();
	byte type = _fileStream->readByte();
	_fileStream->skip(2);
	uint16 bytes = _fileStream->readUint16LE();
	_fileStream->skip(2);
	uint16 rle_bytes = _fileStream->readUint16LE();
	_fileStream->skip(6);
	uint32 offset = _fileStream->readUint32LE();

	_fileStream->seek(offset);
	SeqFrame *frame = new SeqFrame();
	frame->frameRect = Common::Rect(frameLeft, frameTop, frameLeft + frameWidth, frameTop + frameHeight);
	frame->data = new byte[frameWidth * frameHeight];
	frame->colorKey = colorKey;

	if (type == 0)
		_fileStream->read(frame->data, bytes);
	else {
		byte *buf = new byte[bytes];
		_fileStream->read(buf, bytes);
		decodeFrame(buf, rle_bytes, buf + rle_bytes, bytes - rle_bytes, frame->data, frameWidth, frameHeight, colorKey);
	}

	hasNext = ++_currentFrame < _frameCount;

	return frame;
}

} // End of namespace Sci
