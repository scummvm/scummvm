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

#include "ultima/shared/std/string.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Std {

const char *const endl = "\n";

Common::String to_uppercase(const Common::String &s) {
	Common::String str = s;
	Common::String::iterator X;
	for (X = str.begin(); X != str.end(); ++X)
		*X = toupper(*X);

	return str;
}

string::string(size_t n, char c) : Common::String() {
	ensureCapacity(n, false);
	for (size_t idx = 0; idx < n; ++idx)
		(*this) += c;
}

void string::resize(size_t count) {
	if (count == 0)
		clear();
	else if (count < size())
		*this = string(_str, _str + count);
	else {
		while (size() < count)
			*this += ' ';
	}
}

} // End of namespace Std
} // End of namespace Ultima
