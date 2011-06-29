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

#include "common/rect.h"
#include "common/textconsole.h"

#include "m4/globals.h"
#include "m4/graphics.h"
#include "m4/m4.h"
#include "m4/resource.h"
#include "m4/sprite.h"

namespace M4 {

enum {
	kEndOfLine   = 0,
	kEndOfSprite = 1,
	kMarker = 2
};

M4Sprite::M4Sprite(Common::SeekableReadStream* source, int xOfs, int yOfs, int widthVal, int heightVal, bool decodeRle, uint8 encodingVal)
	: M4Surface(widthVal, heightVal), encoding(encodingVal) {

	if (_vm->isM4()) {
		if (decodeRle) {
			loadRle(source);
		} else {
			// Raw sprite data, load directly
			byte *dst = getBasePtr();
			source->read(dst, widthVal * heightVal);
		}
	} else {
		loadMadsSprite(source);
	}

	x = xOffset = xOfs;
	y = yOffset = yOfs;

}

void M4Sprite::loadRle(Common::SeekableReadStream* rleData) {
	byte *dst = getBasePtr();
	while (1) {
		byte len = rleData->readByte();
		if (len == 0) {
			len = rleData->readByte();
			if (len <= kMarker) {
				if (len == kEndOfSprite)
					break;
			} else {
				while (len--) {
					*dst++ = rleData->readByte();
				}
			}
		} else {
			byte value = rleData->readByte();
			while (len--)
				*dst++ = value;
		}
	}
}

void M4Sprite::loadDeltaRle(Common::SeekableReadStream* rleData, int destX, int destY) {
	int lineNum = 0;
	byte *dst = getBasePtr(destX, destY);
	while (1) {
		byte len = rleData->readByte();
		if (len == 0) {
			len = rleData->readByte();
			if (len <= kMarker) {
				if (len == kEndOfLine) {
					dst = getBasePtr(destX, destY + lineNum);
					lineNum++;
				} else if (len == kEndOfSprite)
					break;
			} else {
				while (len--) {
					byte pixel = rleData->readByte();
					if (pixel == 0)
						dst++;
					else
						*dst++ = pixel;
					/* NOTE: The change below behaved differently than the old code,
					   so I put the old code back in again above.
					   If the pixel value is 0, nothing should be written to the
					   output buffer, since 0 means transparent. */
					//*dst++ = (pixel == 0xFD) ? 0 : pixel;
				}
			}
		} else {
			byte value = rleData->readByte();
			if (value == 0)
				dst += len;
			else
				while (len--)
					*dst++ = value;
		}
	}
}

// TODO: The sprite outlines (pixel value 0xFD) are not shown
void M4Sprite::loadMadsSprite(Common::SeekableReadStream* source) {
	bool spriteEnd = false;

	// Set entire sprite contents to transparent pixels
	fillRect(bounds(), TRANSPARENT_COLOR_INDEX);

	// Major line loop
	for (int yp = 0; yp < h; ++yp) {
		byte *destP = getBasePtr(0, yp);
		bool newLine = false;
		byte cmd = source->readByte();
		int x2 = 0;

		if (cmd == 0xfc) {
			// End of entire sprite
			spriteEnd = true;
			break;
		} else if (cmd == 0xff) {
			// The entire line is empty
			newLine = true;
		} else if (cmd == 0xFD) {
			// Lines contains only run lenghs of pixels
			while (x2 < w) {
				cmd = source->readByte();
				if (cmd == 0xff) {
					// End of line reached
					newLine = true;
					break;
				}

				byte v = source->readByte();
				while (cmd-- > 0) {
					if (x2 < w)
						*destP++ = (v == 0xFD) ? TRANSPARENT_COLOR_INDEX : v;
					++x2;
				}
			}
		} else {
			// Line intermixes run lengths with individual pixels
			while (x2 < w) {
				cmd = source->readByte();
				if (cmd == 0xff) {
					// End of line reached
					newLine = true;
					break;
				}

				if (cmd == 0xFE) {
					// Handle repeated sequence
					cmd = source->readByte();
					byte v = source->readByte();
					while (cmd-- > 0) {
						if (x2 < w) {
							*destP++ = (v == 0xFD) ? TRANSPARENT_COLOR_INDEX : v;
						}
						++x2;
					}
				} else {
					// Handle writing out single pixel
					*destP++ = (cmd == 0xFD) ? TRANSPARENT_COLOR_INDEX : cmd;
					++x2;
				}
			}
		}

		// Check if we need to scan forward to find the end of the line
		if (!newLine) {
			do {
				if (source->eos()) {
					warning("M4Sprite::loadMadsSprite: unexpected end of data");
					break;
				}
			} while (source->readByte() != 0xff);
		}
	}

	if (!spriteEnd) {
		byte v = source->readByte();
		assert(v == 0xFC);
	}
}

byte M4Sprite::getTransparencyIndex() const {
		return TRANSPARENT_COLOR_INDEX;
}

} // End of namespace M4
