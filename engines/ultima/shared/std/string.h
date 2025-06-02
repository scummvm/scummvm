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

#ifndef ULTIMA_STD_STRING_H
#define ULTIMA_STD_STRING_H

#include "common/str.h"

namespace Ultima {
namespace Std {

class string final : public Common::String {
public:
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
	constexpr string() : Common::String() {}
	string(const char *str) : Common::String(str) {}
	string(const char *str, uint32 len) : Common::String(str, len) {}
	string(const char *beginP, const char *endP) : Common::String(beginP, endP) {}
	string(const String &str) : Common::String(str) {}
	explicit constexpr string(char c) : Common::String(c) {}
	string(size_t n, char c) : Common::String(n, c) {}

	reverse_iterator rbegin() {
		return reverse_iterator(this, (int)size() - 1);
	}
	reverse_iterator rend() {
		return reverse_iterator(this, -1);
	}
};

} // End of namespace Std
} // End of namespace Ultima

#endif
