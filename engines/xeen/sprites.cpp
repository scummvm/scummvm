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

SpriteResource::SpriteResource() {
	_filesize = 0;
	_data = nullptr;
	_scaledWidth = _scaledHeight = 0;
	Common::fill(&_lineDist[0], &_lineDist[SCREEN_WIDTH], false);
}

SpriteResource::SpriteResource(const Common::String &filename) {
	_data = nullptr;
	_scaledWidth = _scaledHeight = 0;
	Common::fill(&_lineDist[0], &_lineDist[SCREEN_WIDTH], false);
	load(filename);
}

SpriteResource::~SpriteResource() {
	clear();
}

/**
 * Copy operator for duplicating a sprite resource
 */
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

/**
 * Load a sprite resource from a given file
 */
void SpriteResource::load(const Common::String &filename) {
	File f(filename);
	load(f);
}

/**
 * Load a sprite resource from a given file and archive
 */
void SpriteResource::load(const Common::String &filename, Common::Archive &archive) {
	File f(filename, archive);
	load(f);
}

/**
 * Load a sprite resource from a stream
 */
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

/**
 * Clears the sprite resource
 */
void SpriteResource::clear() {
	delete[] _data;
	_data = nullptr;
	_filesize = 0;
}

/**
 * Draws a frame using data at a specific offset in the sprite resource
 */
void SpriteResource::drawOffset(XSurface &dest, uint16 offset, const Common::Point &pt, 
		const Common::Rect &bounds, int flags, int scale) {
	// Get cell header
	Common::MemoryReadStream f(_data, _filesize);
	f.seek(offset);
	int xOffset = f.readUint16LE();
	int width = f.readUint16LE();
	int yOffset = f.readUint16LE();
	int height = f.readUint16LE();

	// TODO: I don't currently know the algorithm the original used for scaling.
	// This is a best fit estimate that every increment of the scale field
	// reduces the size of a sprite by approximately 6.6%
	int newScale = MAX(100.0 - 6.6 * scale, 0.0);
	if (newScale == 0)
		return;
	setupScaling(newScale, xOffset + width, yOffset + height);

	Common::Point destPos = pt;
	destPos.x += (xOffset + width - _scaledWidth) / 2;

	bool flipped = (flags & SPRFLAG_HORIZ_FLIPPED) != 0;
	int xInc = flipped ? -1 : 1;

	if (flags & SPRFLAG_RESIZE) {
		if (dest.w < (xOffset + width) || dest.h < (yOffset + height))
			dest.create(xOffset + width, yOffset + height);
	}
	// The pattern steps used in the pattern command
	const int patternSteps[] = { 0, 1, 1, 1, 2, 2, 3, 3, 0, -1, -1, -1, -2, -2, -3, -3 };

	// Main loop
	int opr1, opr2;
	int32 pos;
	int yIndex = yOffset;
	int yPos = destPos.y + getScaledValue(yOffset);
	for (int yCtr = 0, byteCount = 0; yCtr < height; ++yCtr, ++yIndex, byteCount = 0) {
		// The number of bytes in this scan line
		int lineLength = f.readByte();

		if (lineLength == 0) {
			// Skip the specified number of scan lines
			int numLines = f.readByte();
			for (int idx = 0; idx < numLines; ++idx, ++yIndex, ++yCtr) {
				if (_lineDist[yIndex])
					++yPos;
			}
		} else if (destPos.y < bounds.top || yPos >= bounds.bottom 
				|| !_lineDist[yIndex]) {
			// Skip over the bytes of the line
			f.skip(lineLength);
		} else {
			const byte *lineStartP = (const byte *)dest.getBasePtr(bounds.left, yPos);
			const byte *lineEndP = (const byte *)dest.getBasePtr(bounds.right, yPos);

			// Skip the transparent pixels at the beginning of the scan line
			int xPos = f.readByte() + xOffset; ++byteCount;
			int xAmt = getScaledValue(flipped ? xOffset + width - xPos : xPos);
			int xIndex = 0;

			byte *destP = (byte *)dest.getBasePtr(destPos.x + xAmt, yPos);
			++yPos;

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
						byte b = f.readByte();
						++byteCount;

						if (_lineDist[xIndex++]) {
							if (destP >= lineStartP && destP < lineEndP)
								*destP = b;
							destP += xInc;
						}
					}
					break;

				case 2:   // The following byte is an index into the color table, draw it len + 3 times.
					opr1 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 3; ++i, ++xPos) {
						if (_lineDist[xIndex++]) {
							if (destP >= lineStartP && destP < lineEndP)
								*destP = opr1;
							destP += xInc;
						}
					}
					break;

				case 3:   // Stream copy command.
					opr1 = f.readUint16LE(); byteCount += 2;
					pos = f.pos();
					f.seek(-opr1, SEEK_CUR);

					for (int i = 0; i < len + 4; ++i, ++xPos) {
						byte b = f.readByte();
						if (_lineDist[xIndex++]) {
							if (destP >= lineStartP && destP < lineEndP)
								*destP = b;
							destP += xInc;
						}
					}

					f.seek(pos, SEEK_SET);
					break;

				case 4:   // The following two bytes are indexes into the color table, draw the pair len + 2 times.
					opr1 = f.readByte(); ++byteCount;
					opr2 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 2; ++i, xPos += 2) {
						if (destP < lineStartP || destP >= (lineEndP - 1)) {
							if (_lineDist[xIndex++])
								destP += xInc;
							if (_lineDist[xIndex++])
								destP += xInc;
						} else {
							if (_lineDist[xIndex++]) {
								*destP = opr1;
								destP += xInc;
							}
							if (_lineDist[xIndex++]) {
								*destP = opr2;
								destP += xInc;
							}
						}
					}
					break;

				case 5:   // Skip len + 1 pixels filling them with the transparent color.
					for (int idx = 0; idx < (len + 1); ++idx) {
						if (_lineDist[xIndex++])
							destP += xInc;
						++xPos;
					}
					break;

				case 6:   // Pattern command.
				case 7:
					// The pattern command has a different opcode format
					len = opcode & 0x07;
					cmd = (opcode >> 2) & 0x0E;

					opr1 = f.readByte(); ++byteCount;
					for (int i = 0; i < len + 3; ++i, ++xPos) {
						if (_lineDist[xIndex++]) {
							if (destP >= lineStartP && destP < lineEndP)
								*destP = opr1;
							destP += xInc;
						}
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
	
	Common::Rect r(Common::Rect(
		destPos.x + getScaledValue(xOffset), destPos.y + getScaledValue(yOffset),
		destPos.x + getScaledValue(xOffset + width), destPos.y + getScaledValue(yOffset + height)));
	r.clip(Common::Rect(0, 0, dest.w, dest.h));
	if (!r.isEmpty())
		dest.addDirtyRect(r);
}

void SpriteResource::draw(XSurface &dest, int frame, const Common::Point &destPos,
		int flags, int scale) {
	draw(dest, frame, destPos, Common::Rect(0, 0, dest.w, dest.h), flags, scale);
}

void SpriteResource::draw(Window &dest, int frame, const Common::Point &destPos,
		int flags, int scale) {
	draw(dest, frame, destPos, dest.getBounds(), flags, scale);
}

/**
 * Draw the sprite onto the given surface
 */
void SpriteResource::draw(XSurface &dest, int frame, const Common::Point &destPos, 
		const Common::Rect &bounds, int flags, int scale) {
	// TODO: TO test when I find sprites using scale values and flags
	assert(scale != 0x8000); 
	assert(scale >= 0);
//	assert((flags & SPRFLAG_2000) == 0);

	drawOffset(dest, _index[frame]._offset1, destPos, bounds, flags, scale);
	if (_index[frame]._offset2)
		drawOffset(dest, _index[frame]._offset2, destPos, bounds, flags, scale);
}

/**
 * Draw the sprite onto the given surface
 */
void SpriteResource::draw(XSurface &dest, int frame) {
	draw(dest, frame, Common::Point());
}

void SpriteResource::setupScaling(int scale, int frameWidth, int frameHeight) {
	int highestDim = MAX(frameWidth, frameHeight);
	int distCtr = 0;
	int distIndex = 0;
	_scaledWidth = _scaledHeight = 0;

	do {
		distCtr += scale;
		if (distCtr < 100) {
			_lineDist[distIndex] = false;
		} else {
			_lineDist[distIndex] = true;
			distCtr -= 100;

			if (distIndex < frameWidth)
				++_scaledWidth;

			if (distIndex < frameHeight)
				++_scaledHeight;
		}
	} while (++distIndex < highestDim);
}

/**
 * Returns a scaled value based on a passed in x or y distance
 */
int SpriteResource::getScaledValue(int xy) {
	int newVal = 0;
	for (int idx = 0; idx < xy; ++idx) {
		if (_lineDist[idx])
			++newVal;
	}

	return newVal;
}

} // End of namespace Xeen
