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

#ifndef ULTIMA8_MISC_ISTRING_H
#define ULTIMA8_MISC_ISTRING_H

#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

class istring : public Std::string {
public:
	istring() : Std::string() {}
	istring(const char *str) : Std::string(str) {}
	istring(const char *str, uint32 len) : Std::string(str, len) {}
	istring(const char *beginP, const char *endP) : Std::string(beginP, endP) {}
	istring(const String &str) : Std::string(str) {}
	explicit istring(char c) : Std::string(c) {}
	istring(size_t n, char c) : Std::string(n, c) {}
	~istring() override {}

	int Compare(const string &s) const override {
		return compareToIgnoreCase(s);
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
