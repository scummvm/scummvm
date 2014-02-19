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

MADSEngine *MSprite::_vm;

MSprite *MSprite::init(MSurface &s) {
	if (_vm->getGameFeatures() & GF_MADS) {
		return new MSpriteMADS(s);
	} else {
		return new MSpriteM4(s);
	}
}

MSprite *MSprite::init(Common::SeekableReadStream *source, const Common::Point &offset, 
		int widthVal, int heightVal, bool decodeRle, uint8 encodingVal) {

	if (_vm->getGameFeatures() & GF_MADS) {
		return new MSpriteMADS(source, offset, widthVal, heightVal, decodeRle, encodingVal);
	} else {
		return new MSpriteM4(source, offset, widthVal, heightVal, decodeRle, encodingVal);
	}
}

MSprite::MSprite(MSurface &s): _surface(s) {
	_encoding = 0;
}

MSprite::MSprite(Common::SeekableReadStream *source, const Common::Point &offset, 
		int widthVal, int heightVal, bool decodeRle, uint8 encodingVal)
		: _surface(*MSurface::init(widthVal, heightVal)), 
		_encoding(encodingVal), _offset(offset) {

	// Load the sprite data
	load(source, widthVal, heightVal, decodeRle);
}

MSprite::~MSprite() {
}

/*------------------------------------------------------------------------*/

void MSpriteMADS::load(Common::SeekableReadStream *stream, int widthVal, int heightVal,
		bool decodeRle) {
	loadSprite(stream);
}

// TODO: The sprite outlines (pixel value 0xFD) are not shown
void MSpriteMADS::loadSprite(Common::SeekableReadStream *source) {
	byte *outp, *lineStart;
	bool newLine = false;

	outp = _surface.getData();
	lineStart = _surface.getData();

	while (1) {
		byte cmd1, cmd2, count, pixel;

		if (newLine) {
			outp = lineStart + _surface.w;
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

/*------------------------------------------------------------------------*/

void MSpriteM4::load(Common::SeekableReadStream *stream, int widthVal, int heightVal,
		bool decodeRle) {
	if (decodeRle) {
		loadRle(stream);
	} else {
		// Raw sprite data, load directly
		byte *dst = _surface.getData();
		stream->read(dst, widthVal * heightVal);
	}
}

void MSpriteM4::loadRle(Common::SeekableReadStream* rleData) {
	byte *dst = _surface.getData();
	for (;;) {
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

void MSpriteM4::loadDeltaRle(Common::SeekableReadStream* rleData, int destX, int destY) {
	int lineNum = 0;
	byte *dst = _surface.getBasePtr(destX, destY);

	for (;;) {
		byte len = rleData->readByte();
		if (len == 0) {
			len = rleData->readByte();
			if (len <= kMarker) {
				if (len == kEndOfLine) {
					dst = _surface.getBasePtr(destX, destY + lineNum);
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

} // End of namespace MADS
