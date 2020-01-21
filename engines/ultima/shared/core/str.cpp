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

#include "ultima/shared/core/str.h"

namespace Ultima {
namespace Shared {

int String::indexOf(char c) const {
	const char *s = strchr(c_str(), c);
	return s ? s - c_str() : -1;
}

int String::indexOf(const String &chars) const {
	uint minIndex = size();

	for (uint idx = 0; idx < chars.size(); ++idx) {
		int charIndex = indexOf(chars[idx]);
		if (charIndex != -1 && charIndex < (int)minIndex)
			minIndex = charIndex;
	}
	
	return minIndex == size() ? -1 : minIndex;
}

StringArray String::split(char c) const {
	return split(String(c));
}

StringArray String::split(const String &chars) const {
	StringArray results;
	String temp = *this;
	int idx;

	// Iterate through the text
	while ((idx = temp.indexOf(chars)) != -1) {
		results.push_back(String(temp.c_str(), temp.c_str() + idx));
		temp = String(temp.c_str() + idx + 1);
	}

	if (!empty() && !temp.empty())
		results.push_back(temp);

	return results;
}

} // End of namespace Shared
} // End of namespace Ultima
