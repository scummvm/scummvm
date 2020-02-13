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

#ifndef ARCHETYPE_STRING
#define ARCHETYPE_STRING

#include "common/str.h"

namespace Glk {
namespace Archetype {

class String : public Common::String {
public:
	String() : Common::String() {}
	String(const char *str) : Common::String(str) {}
	String(const char *str, uint32 len) : Common::String(str, len) {}
	String(const char *beginP, const char *endP) : Common::String(beginP, endP) {}
	String(const Common::String &str) : Common::String(str) {}
	explicit String(char c) : Common::String(c) {}

	String &operator=(const char *str) {
		Common::String::operator=(str);
		return *this;
	}
	String &operator=(const Common::String &str) {
		Common::String::operator=(str);
		return *this;
	}
	String &operator=(char c) {
		Common::String::operator=(c);
		return *this;
	}
	String &operator+=(const char *str) {
		Common::String::operator+=(str);
		return *this;
	}
	String &operator+=(const Common::String &str) {
		Common::String::operator+=(str);
		return *this;
	}
	String &operator+=(char c) {
		Common::String::operator+=(c);
		return *this;
	}

	static String format(const char *fmt, ...) GCC_PRINTF(1, 2);

	static String vformat(const char *fmt, va_list args);

	/**
	 * Returns the index of a character within this string
	 */
	int indexOf(char c) const;

	/**
	 * Returns the index of a substring within this string
	 */
	int indexOf(const String &substr) const;

	/**
	 * Returns the index of a substring within this string starting at a given index
	 */
	int indexOf(const String &substr, int start) const;

	/**
	 * Returns the last index of a character in a string, or -1 if it isn't present
	 */
	int lastIndexOf(char c) const;

	/**
	 * Trims spaces(and tabs and newlines) off the ends of a given string
	 */
	void trim();

	/**
	 * Gets a substring of the string
	 */
	String substring(int index, int count) const {
		return String(c_str() + index, c_str() + index + count);
	}

	/**
	 * Converts a string to a value
	 * @param code		Optional returns non-value of character index 		
	 */
	int val(int *code);

	/**
	 * Returns a given number of chracters from the start of a string
	 */
	String left(size_t count) const;

	/**
	 * Returns a given number of characters from the end of a string
	 */
	String right(size_t count) const;

	/**
	 * Returns a substring of another string
	 */
	String mid(size_t start) const;
	String mid(size_t start, size_t count) const;

	/**
	 * Delete a range within a string
	 */
	void del(size_t start, size_t count);
};

// Append two strings to form a new (temp) string
String operator+(const String &x, const String &y);

String operator+(const char *x, const String &y);
String operator+(const String &x, const char *y);

String operator+(const String &x, char y);
String operator+(char x, const String &y);

// Some useful additional comparison operators for Strings
bool operator==(const char *x, const String &y);
bool operator!=(const char *x, const String &y);

typedef String *StringPtr;
typedef String ShortStringType;

} // End of namespace Archetype
} // End of namespace Glk

#endif
