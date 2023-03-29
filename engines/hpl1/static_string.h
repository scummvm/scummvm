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

#include "common/str.h"
#include "common/system.h"

namespace Hpl1 {

class StaticString {
public:
	constexpr StaticString(const char *str) : _str(str) {
	}

	const char *c_str() const {
		return _str;
	}

	operator Common::String() {
		return _str;
	}

private:
	const char *_str = "";
};

inline bool operator==(const StaticString &lhs, const StaticString &rhs) {
	return scumm_stricmp(lhs.c_str(), rhs.c_str()) == 0;
}

inline bool operator<(const StaticString &lhs, const StaticString &rhs) {
	return scumm_stricmp(lhs.c_str(), rhs.c_str()) < 0;
}

} // namespace Hpl1