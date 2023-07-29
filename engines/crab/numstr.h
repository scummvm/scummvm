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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_NUMSTR_H
#define CRAB_NUMSTR_H
#include "crab/common_header.h"

namespace Crab {

template<typename T>
Common::String numberToString(T number) {
	return Common::String::format("%d", number);
}

template<typename T>
inline T stringToNumber(char *text) {
	int res = 0;
	if (sscanf(text, "%d", &res) > 0)
		return static_cast<T>(res); // static cast to deal with signed to unsigned conversions
	return 0;
}

template<>
inline float stringToNumber<float>(char *text) {
	float res = 0.0f;
	if (sscanf(text, "%f", &res) > 0)
		return res;
	return 0.0f;
}

template<>
inline double stringToNumber<double>(char *text) {
	double res = 0.0;
	if (sscanf(text, "%lf", &res) > 0)
		return res;
	return 0.0;
}

template<typename T>
T stringToNumber(const Common::String &text) {
	return stringToNumber<T>(const_cast<char *>(text.c_str()));
}

template<typename T>
void getPoint(T &v, Common::String &coords) {
	int comma = coords.findFirstOf(',');

	v._x = stringToNumber<int>(coords.substr(0, comma));
	v._y = stringToNumber<int>(coords.substr(comma + 1));
}

} // End of namespace Crab

#endif // CRAB_NUMSTR_H
