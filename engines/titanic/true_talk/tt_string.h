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

#ifndef TITANIC_TT_STRING_H
#define TITANIC_TT_STRING_H

#include "titanic/support/string.h"

namespace Titanic {

class SimpleFile;

struct TTstringData {
	CString _string;
	int _referenceCount;

	TTstringData() : _referenceCount(1) {}
	TTstringData(const char *str) : _string(str), _referenceCount(1) {}
	TTstringData(const CString &str) : _string(str), _referenceCount(1) {}
};

enum TTstringStatus {
	SS_VALID = 0, SS_1 = 1, SS_2 = 2, SS_3 = 3, SS_4 = 4,
	SS_5 = 5, SS_7 = 7, SS_8 = 8, SS_11 = 11, SS_13 = 13
};

class TTstring {
private:
	TTstringData *_data;
	TTstringStatus _status;
public:
	TTstring();
	TTstring(const char *str);
	TTstring(const CString &str);
	TTstring(const TTstring &str);
	virtual ~TTstring();

	void operator=(const TTstring &str);
	void operator=(const CString &str);
	void operator=(const char *str);
	TTstring &operator+=(const char *str);
	TTstring &operator+=(const TTstring &str);
	TTstring &operator+=(char c);
	bool operator==(const TTstring &str) const;
	bool operator==(const char *str) const;

	const char &operator[](int index) {
		return *(c_str() + index);
	}

	bool empty() const {
		return _data->_string.empty();
	}

	char firstChar() const {
		return _data->_string.firstChar();
	}

	char lastChar() const {
		return _data->_string.lastChar();
	}

	int size() const {
		return _data->_string.size();
	}

	void deleteLastChar() {
		_data->_string.deleteLastChar();
	}

	bool hasPrefix(const CString &str) const {
		return _data->_string.hasPrefix(str);
	}
	bool hasPrefix(const char *str) const {
		return _data->_string.hasPrefix(str);
	}
	bool hasSuffix(const CString &str) const {
		return _data->_string.hasSuffix(str);
	}
	bool hasSuffix(const char *str) const {
		return _data->_string.hasSuffix(str);
	}

	bool contains(const char *s) const {
		return _data->_string.contains(s);
	}

	/**
	 * Create a new copy of the string
	 */
	TTstring *copy() const {
		return new TTstring(c_str());
	}

	/**
	 * Returns true if the string is valid
	 */
	bool isValid() const {
		return _status == SS_VALID;
	}

	/**
	 * Get the status of the string
	 */
	TTstringStatus getStatus() const { return _status; }

	/**
	 * Get a char * pointer to the string data
	 */
	const char *c_str() const { return _data->_string.c_str(); }

	/**
	 * Automatic operator to convert to a const char *
	 */
	operator const char *() const { return c_str(); }

	/**
	 * Get a character at a specified index
	 */
	char charAt(int index) const { return *(c_str() + index); }

	/**
	 * Save the sring to a passed file
	 */
	void save(SimpleFile *file) const;

	/**
	 * Compare a substring within the string at the specified index
	 */
	bool compareAt(int index, const char *str) const {
		return !strncmp(c_str() + index, str, strlen(str));
	}

	/**
	 * Split off everything in the string until the first occurrence
	 * of any specified delimiter character
	 */
	TTstring tokenize(const char *delim);

	/**
	 * Delets a specififed number of characters from the start of the string
	 */
	int deletePrefix(int count);

	/**
	 * Delets a specififed number of characters from the end of the string
	 */
	int deleteSuffix(int count);

};

} // End of namespace Titanic

#endif /* TITANIC_TT_STRING_H */
