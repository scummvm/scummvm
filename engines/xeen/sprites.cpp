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
#include "xeen/screen.h"
#include "xeen/sprites.h"

namespace Xeen {

#define SCENE_CLIP_LEFT 8
#define SCENE_CLIP_RIGHT 223

SpriteResource::SpriteResource() {
	_filesize = 0;
	_data = nullptr;
	_scaledWidth = _scaledHeight = 0;
}

SpriteResource::SpriteResource(const Common::String &filename) {
	_data = nullptr;
	_scaledWidth = _scaledHeight = 0;
	load(filename);
}

SpriteResource::~SpriteResource() {
	clear();
}

SpriteResource &SpriteResource::operator=(const SpriteResource &src) {
	delete[] _data;
	_index.clear();

	_filesize = src._filesize;
	_data = new byte[_filesize];
	Common::copy(src._data, src._data + _filesize, _data);

	_index.resize(src._index.size());
	for (uint i = 0; i < src._index.size(); ++i)
		_index[i] = src._index[i];

	return *this;
}

void SpriteResource::load(const Common::String &filename) {
	File f(filename);
	load(f);
}

void SpriteResource::load(const Common::String &filename, int ccMode) {
	File f(filename, ccMode);
	load(f);
}

void SpriteResource::load(Common::SeekableReadStream &f) {
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

void SpriteResource::clear() {
	delete[] _data;
	_data = nullptr;
	_filesize = 0;
}

void SpriteResource::drawOffset(XSurface &dest, uint16 offset, const Common::Point &pt,
		const Common::Rect &clipRect, uint flags, int scale) {
	static const uint SCALE_TABLE[] = {
		0xFFFF, 0xFFEF, 0xEFEF, 0xEFEE, 0xEEEE, 0xEEAE, 0xAEAE, 0xAEAA,
		0xAAAA, 0xAA8A, 0x8A8A, 0x8A88, 0x8888, 0x8880, 0x8080, 0x8000
	};
	static const int PATTERN_STEPS[] = { 0, 1, 1, 1, 2, 2, 3, 3, 0, -1, -1, -1, -2, -2, -3, -3 };

	assert((scale & SCALE_MASK) < 16);
	uint16 scaleMask = SCALE_TABLE[scale & SCALE_MASK];
	uint16 scaleMaskX = scaleMask, scaleMaskY = scaleMask;
	bool flipped = (flags & SPRFLAG_HORIZ_FLIPPED) != 0;
	int xInc = flipped ? -1 : 1;
	bool enlarge = (scale & SCALE_ENLARGE) != 0;

	// Get cell header
	Common::MemoryReadStream f(_data, _filesize);
	f.seek(offset);
	int xOffset = f.readUint16LE();
	int width = f.readUint16LE();
	int yOffset = f.readUint16LE();
	int height = f.readUint16LE();

	// Figure out drawing x, y
	Common::Point destPos;
	destPos.x = pt.x + getScaledVal(xOffset, scaleMaskX);
	destPos.x += (width - getScaledVal(width, scaleMaskX)) / 2;

	destPos.y = pt.y + getScaledVal(yOffset, scaleMaskY);

	// If the flags allow the dest surface to be resized, ensure dest surface is big enough
	Common::Rect bounds = clipRect;
	if (flags & SPRFLAG_RESIZE) {
		if (dest.w < (xOffset + width) || dest.h < (yOffset + height))
			dest.create(xOffset + width, yOffset + height);
		bounds = Common::Rect(0, 0, dest.w, dest.h);
	}

	uint16 scaleMaskXCopy = scaleMaskX;
	Common::Rect drawBounds;
	drawBounds.left = SCREEN_WIDTH;
	drawBounds.top = SCREEN_HEIGHT;
	drawBounds.right = drawBounds.bottom = 0;

	// Main loop
	for (int yCtr = height; yCtr > 0; --yCtr) {
		// The number of bytes in this scan line
		int lineLength = f.readByte();

		if (lineLength == 0) {
			// Skip the specified number of scan lines
			int numLines = f.readByte();
			destPos.y += getScaledVal(numLines, scaleMaskY);
			yCtr -= numLines;
			continue;
		}

		// Roll the scale mask
		uint bit = (scaleMaskY >> 15) & 1;
		scaleMaskY = ((scaleMaskY & 0x7fff) << 1) + bit;

		if (!bit) {
			// Not a line to be drawn due to scaling down
			f.skip(lineLength);
		} else if (destPos.y < bounds.top || destPos.y >= bounds.bottom) {
			// Skip over the bytes of the line
			f.skip(lineLength);
			destPos.y++;
		} else {
			scaleMaskX = scaleMaskXCopy;
			xOffset = f.readByte();

			// Initialize the array to hold the temporary data for the line. We do this to make it simpler
			// to handle both deciding which pixels to draw in a scaled image, as well as when images
			// have been horizontally flipped. Note that we allocate an extra line for before and after our
			// work line, just in case the sprite is screwed up and overruns the line
			int tempLine[SCREEN_WIDTH * 3];
			Common::fill(&tempLine[SCREEN_WIDTH], &tempLine[SCREEN_WIDTH * 3], -1);
			int *lineP = flipped ? &tempLine[SCREEN_WIDTH + width - 1 - xOffset] : &tempLine[SCREEN_WIDTH + xOffset];

			// Build up the line
			int byteCount, opr1, opr2;
			int32 pos;
			for (byteCount = 1; byteCount < lineLength; ) {
				// The next byte is an opcode that determines what operators are to follow and how to interpret them.
				int opcode = f.readByte(); ++byteCount;

				// Decode the opcode
				int len = opcode & 0x1F;
				int cmd = (opcode & 0xE0) >> 5;

				switch (cmd) {
				case 0:   // The following len + 1 bytes are stored as indexes into the color table.
				case 1:   // The following len + 33 bytes are stored as indexes into the color table.
					for (int i = 0; i < opcode + 1; ++i, ++byteCount) {
						byte b = f.readByte();
						*lineP = b;
						lineP += xInc;
					}
					break;

				case 2:   // The following byte is an index into the color table, draw it len + 3 times.
					opr1 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 3; ++i) {
						*lineP = opr1;
						lineP += xInc;
					}
					break;

				case 3:   // Stream copy command.
					opr1 = f.readUint16LE(); byteCount += 2;
					pos = f.pos();
					f.seek(-opr1, SEEK_CUR);

					for (int i = 0; i < len + 4; ++i) {
						*lineP = f.readByte();
						lineP += xInc;
					}

					f.seek(pos, SEEK_SET);
					break;

				case 4:   // The following two bytes are indexes into the color table, draw the pair len + 2 times.
					opr1 = f.readByte(); ++byteCount;
					opr2 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 2; ++i) {
						*lineP = opr1;
						lineP += xInc;
						*lineP = opr2;
						lineP += xInc;
					}
					break;

				case 5:   // Skip len + 1 pixels
					lineP += (len + 1) * xInc;
					break;

				case 6:   // Pattern command.
				case 7:
					// The pattern command has a different opcode format
					len = opcode & 0x07;
					cmd = (opcode >> 2) & 0x0E;

					opr1 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 3; ++i) {
						*lineP = opr1;
						lineP += xInc;
						opr1 += PATTERN_STEPS[cmd + (i % 2)];
					}
					break;

				default:
					break;
				}
			}
			assert(byteCount == lineLength);

			drawBounds.top = MIN(drawBounds.top, destPos.y);
			drawBounds.bottom = MAX((int)drawBounds.bottom, destPos.y + 1);

			// Handle drawing out the line
			byte *destP = (byte *)dest.getBasePtr(destPos.x, destPos.y);
			int16 xp = destPos.x;
			lineP = &tempLine[SCREEN_WIDTH];

			for (int xCtr = 0; xCtr < width; ++xCtr, ++lineP) {
				bit = (scaleMaskX >> 15) & 1;
				scaleMaskX = ((scaleMaskX & 0x7fff) << 1) + bit;

				if (bit) {
					// Check whether there's a pixel to write, and we're within the allowable bounds. Note that for
					// the SPRFLAG_SCENE_CLIPPED or when enlarging, we also have an extra horizontal bounds check
					if (*lineP != -1 && xp >= bounds.left && xp < bounds.right &&
							((!(flags & SPRFLAG_SCENE_CLIPPED) && !enlarge) || (xp >= SCENE_CLIP_LEFT && xp < SCENE_CLIP_RIGHT))) {
						drawBounds.left = MIN(drawBounds.left, xp);
						drawBounds.right = MAX((int)drawBounds.right, xp + 1);
						*destP = (byte)*lineP;
						if (enlarge)
							*(destP + SCREEN_WIDTH) = (byte)*lineP;
					}

					++destP;
					++xp;
				}
			}

			++destPos.y;
			if (enlarge)
				++destPos.y;
		}
	}

	if (drawBounds.isValidRect()) {
		drawBounds.clip(Common::Rect(0, 0, dest.w, dest.h));
		if (!drawBounds.isEmpty())
			dest.addDirtyRect(drawBounds);
	}
}

void SpriteResource::draw(XSurface &dest, int frame, const Common::Point &destPos,
		uint flags, int scale) {
	draw(dest, frame, destPos, Common::Rect(0, 0, dest.w, dest.h), flags, scale);
}

void SpriteResource::draw(Window &dest, int frame, const Common::Point &destPos,
		uint flags, int scale) {
	draw(dest, frame, destPos, dest.getBounds(), flags, scale);
}

void SpriteResource::draw(int windowIndex, int frame, const Common::Point &destPos,
		uint flags, int scale) {
	Window &win = (*g_vm->_windows)[windowIndex];
	draw(win, frame, destPos, flags, scale);
}

void SpriteResource::draw(XSurface &dest, int frame, const Common::Point &destPos,
		const Common::Rect &bounds, uint flags, int scale) {

	// Sprites can consist of separate background & foreground
	drawOffset(dest, _index[frame]._offset1, destPos, bounds, flags, scale);
	if (_index[frame]._offset2)
		drawOffset(dest, _index[frame]._offset2, destPos, bounds, flags, scale);
}

void SpriteResource::draw(XSurface &dest, int frame) {
	draw(dest, frame, Common::Point());
}

void SpriteResource::draw(int windowIndex, int frame) {
	draw((*g_vm->_windows)[windowIndex], frame, Common::Point());
}

uint SpriteResource::getScaledVal(int xy, uint16 &scaleMask) {
	if (!xy)
		return 0;

	uint result = 0;
	for (int idx = 0; idx < xy; ++idx) {
		uint bit = (scaleMask >> 15) & 1;
		scaleMask = ((scaleMask & 0x7fff) << 1) + bit;
		result += bit;
	}

	return result;
}

} // End of namespace Xeen
