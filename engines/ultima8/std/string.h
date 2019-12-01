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

#ifndef ULTIMA8_STD_STRING_H
#define ULTIMA8_STD_STRING_H

#include "common/str.h"

namespace Ultima8 {
namespace std {

class string : public Common::String {
public:
	typedef size_t size_type;
	static const size_type npos = (size_type) - 1;
public:
	string() : Common::String() {}
	string(const char *str) : Common::String(str) {}
	string(const char *str, uint32 len) : Common::String(str, len) {}
	string(const char *beginP, const char *endP) : Common::String(beginP, endP) {}
	string(const String &str) : Common::String(str) {}
	explicit string(char c) : Common::String(c) {}
	string(size_t n, char c);

	/**
	 * Finds the index of a character in the string
	 */
	size_t find(char c, size_t pos = 0) const;

	/**
	 * Does a find for the passed string
	 */
	size_t find(const char *s) const;

	/**
	 * Does a reverse find for the passed string
	 */
	size_t rfind(const char *s) const;

	/**
	 * Does a reverse find for a passed character
	 */
	size_t rfind(char c, size_t pos = npos) const;

	/**
	 * Find first character in the string that's not the specified character
	 */
	size_t find_first_not_of(char c) const;

	/**
	 * Find the last character in the string that's not the specified character
	 */
	size_t find_last_not_of(char c) const;

	/**
	 * Return a substring of this string
	 */
	string substr(size_t pos = 0, size_t len = npos) const;

	/**
	 * Erases len number of characters from pos
	 */
	string &erase(size_t pos = 0, size_t len = npos);
};

extern const char *const endl;

} // End of namespace std
} // End of namespace Ultima8

#endif