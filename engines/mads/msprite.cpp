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
#include "engines/util.h"
#include "graphics/palette.h"
#include "mads/mads.h"
#include "mads/msurface.h"
#include "mads/msprite.h"

namespace MADS {

enum {
	kEndOfLine   = 0,
	kEndOfSprite = 1,
	kMarker = 2
};

MSprite::MSprite(): MSurface() {
	_encoding = 0;
}

MSprite::MSprite(Common::SeekableReadStream *source, const Common::Point &offset, 
		int widthVal, int heightVal, bool decodeRle, uint8 encodingVal)
		: MSurface(widthVal, heightVal), 
		_encoding(encodingVal), _offset(offset) {

	// Load the sprite data
	loadSprite(source);
}

MSprite::~MSprite() {
}


// TODO: The sprite outlines (pixel value 0xFD) are not shown
void MSprite::loadSprite(Common::SeekableReadStream *source) {
	byte *outp, *lineStart;
	bool newLine = false;

	outp = getData();
	lineStart = getData();

	while (1) {
		byte cmd1, cmd2, count, pixel;

		if (newLine) {
			outp = lineStart + getWidth();
			lineStart = outp;
			newLine = false;
		}

		cmd1 = source->readByte();

		if (cmd1 == 0xFC)
			break;
		else if (cmd1 == 0xFF)
			newLine = true;
		else if (cmd1 == 0xFD) {
			while (!newLine) {
				count = source->readByte();
				if (count == 0xFF) {
					newLine = true;
				} else {
					pixel = source->readByte();
					while (count--)
						*outp++ = (pixel == 0xFD) ? 0 : pixel;
				}
			}
		} else {
			while (!newLine) {
				cmd2 = source->readByte();
				if (cmd2 == 0xFF) {
					newLine = true;
				} else if (cmd2 == 0xFE) {
					count = source->readByte();
					pixel = source->readByte();
					while (count--)
						*outp++ = (pixel == 0xFD) ? 0 : pixel;
				} else {
					*outp++ = (cmd2 == 0xFD) ? 0 : cmd2;
				}
			}
		}
	}
}

} // End of namespace MADS
