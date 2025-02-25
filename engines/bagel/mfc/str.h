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

#ifndef BAGEL_HODJNPODJ_MFC_STR_H
#define BAGEL_HODJNPODJ_MFC_STR_H

#include "common/str.h"

namespace Bagel {
namespace MFC {

class CString : public Common::String {
public:
	constexpr CString() : Common::String() {}
	CString(const char *str) : Common::String(str) {}
	CString(const char *str, uint32 len) : Common::String(str, len) {}
	CString(const char *beginP, const char *endP) : Common::String(beginP, endP) {}
	CString(const Common::String &str) : Common::String(str) {}
	explicit constexpr CString(value_type c) : Common::String(c) {}

	bool IsEmpty() const {
		return empty();
	}
	operator LPCSTR() const {
		return c_str();
	}

	CString operator+(const char *str) const {
		Common::String temp = Common::String(c_str()) + str;
		return CString(temp);
	}
};

} // namespace MFC
} // namespace Bagel

#endif
