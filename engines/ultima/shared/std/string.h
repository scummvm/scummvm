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
	constexpr string() : Common::String() {}
	string(const char *str) : Common::String(str) {}
	string(const char *str, uint32 len) : Common::String(str, len) {}
	string(const char *beginP, const char *endP) : Common::String(beginP, endP) {}
	string(const String &str) : Common::String(str) {}
	explicit constexpr string(char c) : Common::String(c) {}
	string(size_t n, char c) : Common::String(n, c) {}
};

} // End of namespace Std
} // End of namespace Ultima

#endif
