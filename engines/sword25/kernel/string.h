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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_STRING
#define SWORD25_STRING

#include "common/str.h"

namespace BS_String {

inline uint getHash(const Common::String &str) {
	uint result = 0;

	for (uint i = 0; i < str.size(); i++)
		result = ((result << 5) - result) + str[i];

	return result;
}

inline bool toInt(const Common::String &str, int &result) {
	Common::String::const_iterator iter = str.begin();

	// Skip whitespaces
	while (*iter && (*iter == ' ' || *iter == '\t')) {
		++iter;
	}
	if (iter == str.end())
		return false;

	// Read sign, if available
	bool isNegative = false;
	if (*iter == '-') {
		isNegative = true;
		++iter;
	} else if (*iter == '+')
		++iter;

	// Skip whitespaces
	while (*iter && (*iter == ' ' || *iter == '\t')) {
		++iter;
	}
	if (iter == str.end())
		return false;

	// Convert string to integer
	result = 0;
	while (iter != str.end()) {
		if (*iter < '0' || *iter > '9') {
			while (*iter && (*iter == ' ' || *iter == '\t')) {
				++iter;
			}
			if (iter != str.end())
				return false;
			break;
		}
		result = (result * 10) + (*iter - '0');
		++iter;
	}

	if (isNegative)
		result = -result;

	return true;
}

inline bool toBool(const Common::String &str, bool &result) {
	if (str == "true" || str == "TRUE") {
		result = true;
		return true;
	} else if (str == "false" || str == "FALSE") {
		result = false;
		return true;
	}

	return false;
}

inline void toLower(Common::String &str) {
	str.toLowercase();
}

} // End of namespace BS_String

#endif
