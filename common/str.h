/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include "common/scummsys.h"
#include "common/array.h"

#include <assert.h>
#include <string.h>

namespace Common {

class String {
protected:
	char	*_str;
	int 	_len;
	int 	*_refCount;
	int 	_capacity;

public:
	static const String emptyString;

	String() : _str(0), _len(0), _capacity(0) { _refCount = new int(1); }
	String(const char *str, int len = -1);
	String(const String &str);
	virtual ~String();

	String &operator  =(const char *str);
	String &operator  =(const String &str);
	String &operator  =(char c);
	String &operator +=(const char *str);
	String &operator +=(const String &str);
	String &operator +=(char c);

	bool operator ==(const String &x) const;
	bool operator ==(const char *x) const;
	bool operator !=(const String &x) const;
	bool operator !=(const char *x) const;
	bool operator <(const String &x) const;
	bool operator <=(const String &x) const;
	bool operator >(const String &x) const;
	bool operator >=(const String &x) const;

	bool hasSuffix(const char *x) const;
	bool hasPrefix(const char *x) const;

	const char *c_str() const		{ return _str ? _str : ""; }
	uint size() const				{ return _len; }

	bool isEmpty() const	{ return (_len == 0); }
	char lastChar() const	{ return (_len > 0) ? _str[_len-1] : 0; }

	char operator [](int idx) const {
		assert(_str && idx >= 0 && idx < _len);
		return _str[idx];
	}

	char &operator [](int idx) {
		assert(_str && idx >= 0 && idx < _len);
		return _str[idx];
	}

	void deleteLastChar();
	void deleteChar(int p);
	void clear();
	void insertChar(char c, int p);

	void toLowercase();
	void toUppercase();

public:
	typedef char *        iterator;
	typedef const char *  const_iterator;

	iterator		begin() {
		return _str;
	}

	iterator		end() {
		return begin() + size();
	}

	const_iterator	begin() const {
		return _str;
	}

	const_iterator	end() const {
		return begin() + size();
	}

protected:
	void ensureCapacity(int new_len, bool keep_old);
	void decRefCount();
};

// Append two strings to form a new (temp) string
String operator +(const String &x, const String &y);
String operator +(const char *x, const String &y);
String operator +(const String &x, const char *y);

// Some useful additional comparision operators for Strings
bool operator == (const char *x, const String &y);
bool operator != (const char *x, const String &y);

class StringList : public Array<String> {
public:
	void push_back(const char *str) {
		ensureCapacity(_size + 1);
		_data[_size++] = str;
	}

	void push_back(const String &str) {
		ensureCapacity(_size + 1);
		_data[_size++] = str;
	}
};

}	// End of namespace Common

#endif
