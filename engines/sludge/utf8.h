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
#ifndef SLUDGE_UTF8_H
#define SLUDGE_UTF8_H

#include "common/system.h"
#include "common/ustr.h"

namespace Sludge {

class UTF8Converter {
private:
	static const uint32 offsetsFromUTF8[6];

	/**
	 * we assume one character in a Common::String is one byte
	 * but in this case it's actually an UTF-8 string
	 */
	Common::String _str;

	/**
	 * wrap a converted U32String
	 */
	Common::U32String _str32;

	/** A tool function for string conversion
	 * return next character, updating an index variable
	 */
	static uint32 nextchar(const char *s, int *i);

	/** A tool function for string conversion
	 * is this byte the start of a utf8 sequence?
	 */
	static inline bool isutf(char c) { return (((c)&0xC0)!=0x80); }

public:
	/** Construct a new empty string. */
	UTF8Converter() {};

	/** Construct a UTF8String with original char array to convert */
	UTF8Converter(const char *data);

	/** U32 character index to origin char offset */
	int getOriginOffset(int origIdx);

	/**
	 * set a char array to this UTF8String
	 */
	void setUTF8String(Common::String str);

	/**
	 * get converted U32String
	 */
	Common::U32String getU32String() const { return _str32; };

	/**
	 * get origin UTF8String
	 */
	Common::String getUTF8String() const { return _str; };

	/** Convert UTF8 String to UTF32 String
	 */
	static Common::U32String convertUtf8ToUtf32(const Common::String &str);
};

} // End of namespace Sludge

#endif
