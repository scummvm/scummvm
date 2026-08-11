/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TITANIC_STRING_H
#define TITANIC_STRING_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/str.h"

namespace Titanic {

enum FileType {
	FILETYPE_UNKNOWN = 0, FILETYPE_IMAGE = 1, FILETYPE_MOVIE = 2,
	FILETYPE_WAV = 3, FILETYPE_DLG = 4
};

enum ImageType {
	IMAGETYPE_UNKNOWN = 0, IMAGETYPE_TARGA = 1, IMAGETYPE_JPEG = 2
};

class CString : public Common::String {
public:
	CString() : Common::String() {}
	CString(const char *str) : Common::String(str) {}
	CString(const char *str, uint32 len) : Common::String(str, len) {}
	CString(const char *beginP, const char *endP) : Common::String(beginP, endP) {}
	CString(const String &str) : Common::String(str) {}
	CString(char c, uint32 len);
	explicit CString(char c) : Common::String(c) {}
	explicit CString(int val);

	bool operator==(const CString &x) const;
	bool operator==(const char *x) const;
	bool operator!=(const CString &x) const;
	bool operator!=(const char *x) const;

	/**
	 * Returns the left n characters of the string
	 */
	CString left(uint count) const;

	/**
	 * Returns the right n characters of the string
	 */
	CString right(uint count) const;

	/**
	 * Returns a substring from within the string
	 */
	CString mid(uint start, uint count) const;

	/**
	 * Returns a substring from within the string
	 */
	CString mid(uint start) const;

	/**
	 * Returns a substring consisting of the entire string
	 * except for a specified number of characters at the end
	 */
	CString deleteRight(uint count) const;

	/**
	 * Returns the index of the first occurrence of a given character
	 */
	int indexOf(char c) const;

	/**
	 * Returns the index of the first occurrence of a given string
	 */
	int indexOf(const char *s) const;

	/**
	 * Returns the index of the last occurrence of a given character
	 */
	int lastIndexOf(char c) const;

	/**
	 * Returns true if the string contains a specified substring, ignoring case
	 */
	bool containsIgnoreCase(const CString &str) const;

	/**
	 * Returns the type of a filename based on it's extension
	 */
	FileType fileTypeSuffix() const;

	/**
	 * Returns the type of an image filename based on it's extension
	 */
	ImageType imageTypeSuffix() const;

	/**
	 * Parses the string as an integer and returns the value
	 */
	int readInt() const {
		return atoi(c_str());
	}

	/**
	 * Format a string
	 */
	static CString format(const char *fmt, ...);
};

typedef Common::Array<CString> StringArray;

} // End of namespace Titanic

#endif /* TITANIC_STRING_H */
