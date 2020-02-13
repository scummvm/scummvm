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

#ifndef ULTIMA_STR_H
#define ULTIMA_STR_H

#include "common/str.h"
#include "common/array.h"

namespace Ultima {
namespace Shared {

class String;

typedef Common::Array<String> StringArray;

/**
 * Derived string class
 */
class String : public Common::String {
public:
	String() : Common::String() {}
	String(const char *str) : Common::String(str) {}
	String(const char *str, uint32 len) : Common::String(str, len) {}
	String(const char *beginP, const char *endP) : Common::String(beginP, endP) {}
	String(const String &str) : Common::String(str) {}
	String(const Common::String &str) : Common::String(str) {}
	explicit String(char c) : Common::String(c) {}

	/**
	 * Returns the index of a given character, or -1 if not present
	 */
	int indexOf(char c) const;

	/**
	 * Returns the earliest index of any character in a passed set of characters, or -1 if not present
	 */
	int indexOf(const String &chars) const;

	/**
	 * Splits up a text string by a given character
	 */
	StringArray split(char c) const;

	/**
	 * Splits up a text string by any of the characters in the passed string
	 */
	StringArray split(const String &chars) const;
};

} // End of namespace Shared

using Shared::String;

} // End of namespace Ultima

#endif
