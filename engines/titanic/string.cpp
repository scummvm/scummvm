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

#include "common/algorithm.h"
#include "titanic/string.h"

namespace Titanic {

CString CString::left(uint count) const {
	return (count > size()) ? *this : CString(c_str(), c_str() + count);
}

CString CString::right(uint count) const {
	return (count > size()) ? *this : CString(c_str() - count, c_str());
}

CString CString::mid(uint start, uint count) const {
	if (start >= size())
		return CString();
	else
		return CString(c_str() + start, MIN(count, size() - start));
}

int CString::indexOf(char c) {
	const char *charP = strchr(c_str(), c);
	return charP ? charP - c_str() : -1;
}

} // End of namespace Titanic
