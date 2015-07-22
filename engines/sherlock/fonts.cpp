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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/platform.h"
#include "sherlock/fonts.h"
#include "sherlock/image_file.h"
#include "sherlock/surface.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

SherlockEngine *Fonts::_vm;
ImageFile *Fonts::_font;
int Fonts::_fontNumber;
int Fonts::_fontHeight;
int Fonts::_widestChar;
uint16 Fonts::_charCount;
byte Fonts::_yOffsets[255];

void Fonts::setVm(SherlockEngine *vm) {
	_vm = vm;
	_font = nullptr;
	_charCount = 0;
}

void Fonts::free() {
	delete _font;
}

void Fonts::setFont(int fontNum) {
	_fontNumber = fontNum;

	// Discard previous font
	delete _font;

	Common::String fontFilename;

	if (_vm->getPlatform() != Common::kPlatform3DO) {
		// PC
		// use FONT[number].VGS, which is a regular sherlock graphic file
		fontFilename = Common::String::format("FONT%d.VGS", fontNum + 1);

		// load font data
		_font = new ImageFile(fontFilename);
	} else {
		// 3DO
		switch (fontNum) {
		case 0:
		case 1:
			fontFilename = "helvetica14.font";
			break;
		case 2:
			fontFilename = "darts.font";
			break;
		default:
			error("setFont(): unsupported 3DO font number");
		}

		// load font data
		_font = new ImageFile3DO(fontFilename, kImageFile3DOType_Font);
	}

	_charCount = _font->size();
		
	// Iterate through the frames to find the widest and tallest font characters
	_fontHeight = _widestChar = 0;
	for (uint idx = 0; idx < _charCount; ++idx) {
		_fontHeight = MAX((uint16)_fontHeight, (*_font)[idx]._frame.h);
		_widestChar = MAX((uint16)_widestChar, (*_font)[idx]._frame.w);
	}

	// Initialize the Y offset table for the extended character set
	for (int idx = 0; idx < 255; ++idx) {
		_yOffsets[idx] = 0;

		if (IS_ROSE_TATTOO) {
			if ((idx >= 129 && idx < 135) || (idx >= 136 && idx < 143) || (idx >= 147 && idx < 155) || 
					(idx >= 156 && idx < 165))
				_yOffsets[idx] = 1;
			else if ((idx >= 143 && idx < 146) || idx == 165)
				_yOffsets[idx] = 2;
		}
	}
}

inline byte Fonts::translateChar(byte c) {
	switch (c) {
	case ' ':
		return 0; // translate to first actual character
	case 225:
		// This was done in the German interpreter
		// happens when talking to the kid in the 2nd room
		return 135; // special handling for 0xE1
	default:
		if (c >= 0x80) { // German SH1 version did this
			c--;
		}
		// Spanish SH1 did this (reverse engineered code)
		//if ((c >= 0xA0) && (c <= 0xAD) || (c == 0x82)) {
		//	c--;
		//}
		assert(c > 32); // anything above space is allowed
		return c - 33;
	}
}

void Fonts::writeString(Surface *surface, const Common::String &str,
		const Common::Point &pt, int overrideColor) {
	Common::Point charPos = pt;

	if (!_font)
		return;

	for (const char *curCharPtr = str.c_str(); *curCharPtr; ++curCharPtr) {
		byte curChar = *curCharPtr;

		if (curChar == ' ') {
			charPos.x += 5; // hardcoded space
			continue;
		}
		curChar = translateChar(curChar);

		assert(curChar < _charCount);
		ImageFrame &frame = (*_font)[curChar];
		surface->transBlitFrom(frame, Common::Point(charPos.x, charPos.y + _yOffsets[curChar]), false, overrideColor);
		charPos.x += frame._frame.w + 1;
	}
}

int Fonts::stringWidth(const Common::String &str) {
	int width = 0;

	if (!_font)
		return 0;

	for (const char *c = str.c_str(); *c; ++c)
		width += charWidth(*c);

	return width;
}

int Fonts::stringHeight(const Common::String &str) {
	int height = 0;

	if (!_font)
		return 0;

	for (const char *c = str.c_str(); *c; ++c)
		height = MAX(height, charHeight(*c));

	return height;
}

int Fonts::charWidth(unsigned char c) {
	byte curChar;

	if (!_font)
		return 0;

	if (c == ' ') {
		return 5; // hardcoded space
	}
	curChar = translateChar(c);

	if (curChar < _charCount)
		return (*_font)[curChar]._frame.w + 1;
	return 0;
}

int Fonts::charHeight(unsigned char c) {
	byte curChar;

	if (!_font)
		return 0;

	// Space is supposed to be handled like the first actual character (which is decimal 33)
	curChar = translateChar(c);

	assert(curChar < _charCount);
	const ImageFrame &img = (*_font)[curChar];
	return img._height + img._offset.y + 1;
}

} // End of namespace Sherlock
