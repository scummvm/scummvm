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

#include "common/scummsys.h"
#include "common/archive.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "xeen/xeen.h"
#include "xeen/sprites.h"

namespace Xeen {

SpriteResource::SpriteResource() {
	_filesize = 0;
	_data = nullptr;
}

SpriteResource::SpriteResource(const Common::String &filename) {
	_data = nullptr;
	load(filename);
}

void SpriteResource::load(const Common::String &filename) {
	// Open the resource
	File f(filename);

	// Read in a copy of the file
	_filesize = f.size();
	delete[] _data;
	_data = new byte[_filesize];
	f.read(_data, _filesize);

	// Read in the index
	f.seek(0);
	int count = f.readUint16LE();
	_index.resize(count);

	for (int i = 0; i < count; ++i) {
		_index[i]._offset1 = f.readUint16LE();
		_index[i]._offset2 = f.readUint16LE();
	}
}

SpriteResource::~SpriteResource() {
	delete[] _data;
}

int SpriteResource::size() const {
	return _index.size();
}

void SpriteResource::drawOffset(XSurface &dest, uint16 offset, const Common::Point &destPos) const {
	// Get cell header
	Common::MemoryReadStream f(_data, _filesize);
	f.seek(offset);
	int xOffset = f.readUint16LE();
	int width = f.readUint16LE();
	int yOffset = f.readUint16LE();
	int height = f.readUint16LE();

	if (dest.w < (xOffset + width) || dest.h < (yOffset + height))
		dest.create(xOffset + width, yOffset + height);

	// The pattern steps used in the pattern command
	const int patternSteps[] = { 0, 1, 1, 1, 2, 2, 3, 3, 0, -1, -1, -1, -2, -2, -3, -3 };

	// Main loop
	int opr1, opr2;
	int32 pos;
	for (int yPos = yOffset, byteCount = 0; yPos < height + yOffset; yPos++, byteCount = 0) {
		// The number of bytes in this scan line
		int lineLength = f.readByte();

		if (lineLength == 0) {
			// Skip the specified number of scan lines
			yPos += f.readByte();
		} else {
			// Skip the transparent pixels at the beginning of the scan line
			int xPos = f.readByte() + xOffset; ++byteCount;
			byte *destP = (byte *)dest.getBasePtr(destPos.x + xPos, destPos.y + yPos);

			while (byteCount < lineLength) {
				// The next byte is an opcode that determines what 
				// operators are to follow and how to interpret them.
				int opcode = f.readByte(); ++byteCount;

				// Decode the opcode
				int len = opcode & 0x1F;
				int cmd = (opcode & 0xE0) >> 5;

				switch (cmd) {
				case 0:   // The following len + 1 bytes are stored as indexes into the color table.
				case 1:   // The following len + 33 bytes are stored as indexes into the color table.
					for (int i = 0; i < opcode + 1; ++i, ++xPos) {
						*destP++ = f.readByte(); ++byteCount;
					}
					break;

				case 2:   // The following byte is an index into the color table, draw it len + 3 times.
					opr1 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 3; ++i, ++xPos)
						*destP++ = opr1;
					break;

				case 3:   // Stream copy command.
					opr1 = f.readUint16LE(); byteCount += 2;
					pos = f.pos();
					f.seek(-opr1, SEEK_CUR);

					for (int i = 0; i < len + 4; ++i, ++xPos)
						*destP++ = f.readByte();

					f.seek(pos, SEEK_SET);
					break;

				case 4:   // The following two bytes are indexes into the color table, draw the pair len + 2 times.
					opr1 = f.readByte(); ++byteCount;
					opr2 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 2; ++i, xPos += 2) {
						*destP++ = opr1;
						*destP++ = opr2;
					}
					break;

				case 5:   // Skip len + 1 pixels filling them with the transparent color.
					xPos += len + 1;
					destP += len + 1;
					break;

				case 6:   // Pattern command.
				case 7:
					// The pattern command has a different opcode format
					len = opcode & 0x07;
					cmd = (opcode >> 2) & 0x0E;

					opr1 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 3; ++i, ++xPos) {
						*destP++ = opr1;
						opr1 += patternSteps[cmd + (i % 2)];
					}
					break;
				default:
					break;
				}
			}

			assert(byteCount == lineLength);
		}
	}

	dest.addDirtyRect(Common::Rect(destPos.x + xOffset, destPos.y + yOffset,
		destPos.x + xOffset + width, destPos.y + yOffset + height));
}

void SpriteResource::draw(XSurface &dest, int frame, const Common::Point &destPos) const {
	drawOffset(dest, _index[frame]._offset1, destPos);
	if (_index[frame]._offset2)
		drawOffset(dest, _index[frame]._offset2, destPos);
}

void SpriteResource::draw(XSurface &dest, int frame) const {
	draw(dest, frame, Common::Point());
}

} // End of namespace Xeen
