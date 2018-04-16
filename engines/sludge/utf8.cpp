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
/*
 Basic UTF-8 manipulation routines
 by Jeff Bezanson
 placed in the public domain Fall 2005

 This code is designed to provide the utilities you need to manipulate
 UTF-8 as an internal string encoding. These functions do not perform the
 error checking normally needed when handling UTF-8 data, so if you happen
 to be from the Unicode Consortium you will want to flay me alive.
 I do this because error checking can be performed at the boundaries (I/O),
 with these routines reserved for higher performance on data known to be
 valid.
 */

#include "common/debug.h"

#include "sludge/utf8.h"

namespace Sludge {

const uint32 UTF8Converter::offsetsFromUTF8[6] = {
		0x00000000UL, 0x00003080UL,
		0x000E2080UL, 0x03C82080UL,
		0xFA082080UL, 0x82082080UL };

/* reads the next utf-8 sequence out of a string, updating an index */
uint32 UTF8Converter::nextchar(const char *s, int *i) {
	uint32 ch = 0;
	int sz = 0;

	do {
		ch <<= 6;
		ch += (byte)s[(*i)++];
		sz++;
	} while (s[*i] && !isutf(s[*i]));
	ch -= offsetsFromUTF8[sz - 1];

	return ch;
}

Common::U32String UTF8Converter::convertUtf8ToUtf32(const Common::String &str) {
	// we assume one character in a Common::String is one byte
	// but in this case it's actually an UTF-8 string
	// with up to 4 bytes per character. To work around this,
	// convert it to an U32String before any further operation
	Common::U32String u32str;
	int i = 0;
	while (i < (int)str.size()) {
		uint32 chr = nextchar(str.c_str(), &i);
		u32str += chr;
	}
	return u32str;
}

/* utf32 index => original byte offset */
int UTF8Converter::getOriginOffset(int origIdx) {
	uint offs = 0;
	while (origIdx > 0 && offs < _str.size()) {
		// increment if it's not the start of a utf8 sequence
		(void)(	(++offs < _str.size() && isutf(_str[offs])) ||
				(++offs < _str.size() && isutf(_str[offs])) ||
				(++offs < _str.size() && isutf(_str[offs])) ||
				++offs);
		origIdx--;
	}
	return offs;
}

/** Construct a UTF8String with original char array to convert */
UTF8Converter::UTF8Converter(const char *str) {
	setUTF8String(str);
}

/** set a utf8 string to convert */
void UTF8Converter::setUTF8String(Common::String str) {
	_str32.clear();
	_str32 = convertUtf8ToUtf32(str);
	_str.clear();
	_str = str;
}

} // End of namespace Sludge
