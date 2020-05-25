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

#ifndef ULTIMA_STD_STRING_H
#define ULTIMA_STD_STRING_H

#include "common/str.h"

namespace Ultima {
namespace Std {

class string : public Common::String {
public:
	typedef size_t size_type;

	struct reverse_iterator {
	private:
		string *_owner;
		int _index;
	public:
		reverse_iterator(string *owner, int index) : _owner(owner), _index(index) {}
		reverse_iterator() : _owner(0), _index(-1) {}

		char &operator*() const { return (*_owner)[_index]; }

		reverse_iterator &operator++() {
			--_index;
			return *this;
		}
		reverse_iterator operator++(int) {
			reverse_iterator tmp(_owner, _index);
			++(*this);
			return tmp;
		}

		bool operator==(const reverse_iterator &rhs) {
			return _owner == rhs._owner && _index == rhs._index;
		}
		bool operator!=(const reverse_iterator &rhs) {
			return !operator==(rhs);
		}
	};
public:
	string() : Common::String() {}
	string(const char *str) : Common::String(str) {}
	string(const char *str, uint32 len) : Common::String(str, len) {}
	string(const char *beginP, const char *endP) : Common::String(beginP, endP) {}
	string(const String &str) : Common::String(str) {}
	string(char c) : Common::String(c) {}
	string(size_t n, char c);
	virtual ~string() {}

	size_t length() const { return size(); }

	/**
	 * Assign a new string
	 */
	void assign(const char *s, size_t count = npos) {
		if (count == npos)
			*this = s;
		else
			*this = string(s, count);
	}
	void assign(const string &s, size_t count = npos) {
		if (count == npos)
			*this = s;
		else
			*this = string(s.c_str(), count);
	}

	/**
	 * String square brackets allows modifying characters
	 */
	char &operator[](size_t idx) {
		assert(idx < _size);
		return _str[idx];
	}

	/**
	 * Square brackets for const strings simply returns characters
	 */
	char operator[](size_t idx) const {
		return Common::String::operator[](idx);
	}

	/**
	 * Get a character at an index
	 */
	char at(size_t idx) { return operator[](idx); }

	virtual int Compare(const string &s) const {
		return compareTo(s);
	}

	/**
	 * Resizes a string
	 */
	void resize(size_t count);

	void insert(size_t pos, size_t n, char c) {
		for (uint idx = 0; idx < n; ++idx)
			insertChar(c, pos);
	}

	/**
	 * Append a given character a number of times to the string
	 */
	void append(size_t n, char c) {
		string str(n, c);
		*this += str;
	}

	/**
	 * Append another string to this one
	 */
	void append(const string &str, size_t theSize = npos) {
		if (theSize == npos)
			*this += str;
		else
			*this += Common::String(str.c_str(), theSize);
	}

	/**
	 * Append a character to the string
	 */
	void push_back(char c) {
		*this += c;
	}

	reverse_iterator rbegin() {
		return reverse_iterator(this, (int)size() - 1);
	}
	reverse_iterator rend() {
		return reverse_iterator(this, -1);
	}

	bool operator==(const string &x) const {
		return !Compare(x);
	}

	bool operator==(const char *x) const {
		return !Compare(x);
	}

	bool operator!=(const string &x) const {
		return Compare(x) != 0;
	}

	bool operator !=(const char *x) const {
		return Compare(x) != 0;
	}
};

extern const char *const endl;

extern Common::String to_uppercase(const Common::String &s);

} // End of namespace Std
} // End of namespace Ultima

#endif
