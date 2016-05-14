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

enum TTstringStatus { SS_VALID = 0, SS_5 = 5, SS_7 = 7, SS_8 = 8, SS_11 = 11, SS_13 = 13 };

class TTstring {
private:
	TTstringData *_data;
	TTstringStatus _status;
public:
	TTstring();
	TTstring(const char *str);
	TTstring(const CString &str);
	TTstring(TTstring &str);
	virtual ~TTstring();

	void operator=(const TTstring &str);
	void operator=(const CString &str);
	void operator=(const char *str);
	TTstring &operator+=(const char *str);
	TTstring &operator+=(const TTstring &str);
	TTstring &operator+=(char c);
	bool empty() const;
	char firstChar() const;
	char lastChar() const;

	/**
	 * Create a new copy of the string
	 */
	TTstring *copy() const;

	/**
	 * Returns true if the string is valid
	 */
	bool isValid() const;

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
};

} // End of namespace Titanic

#endif /* TITANIC_TT_STRING_H */
