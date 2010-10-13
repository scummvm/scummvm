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

inline uint GetHash(const Common::String &Str) {
	uint Result = 0;

	for (uint i = 0; i < Str.size(); i++)
		Result = ((Result << 5) - Result) + Str[i];

	return Result;
}

inline bool ToInt(const Common::String &Str, int &Result) {
	Common::String::const_iterator Iter = Str.begin();

	// Skip whitespaces
	while (*Iter && (*Iter == ' ' || *Iter == '\t')) {
		++Iter;
	}
	if (Iter == Str.end()) return false;

	// Read sign, if available
	bool IsNegative = false;
	if (*Iter == '-') {
		IsNegative = true;
		++Iter;
	} else if (*Iter == '+')
		++Iter;

	// Skip whitespaces
	while (*Iter && (*Iter == ' ' || *Iter == '\t')) {
		++Iter;
	}
	if (Iter == Str.end()) return false;

	// Convert string to integer
	Result = 0;
	while (Iter != Str.end()) {
		if (*Iter < '0' || *Iter > '9') {
			while (*Iter && (*Iter == ' ' || *Iter == '\t')) {
				++Iter;
			}
			if (Iter != Str.end()) return false;
			break;
		}
		Result = (Result * 10) + (*Iter - '0');
		++Iter;
	}

	if (IsNegative) Result = -Result;

	return true;
}

inline bool ToBool(const Common::String &Str, bool &Result) {
	if (Str == "true" || Str == "TRUE") {
		Result = true;
		return true;
	} else if (Str == "false" || Str == "FALSE") {
		Result = false;
		return true;
	}

	return false;
}

inline void ToLower(Common::String &Str) {
	Str.toLowercase();
}

} // End of namespace BS_String

#endif
