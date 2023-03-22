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
Common::String NumberToString(T Number) {
	Common::String res;
	const T Num = Number;

	if (Number == 0) {
		return "0";
	}

	if (Number < 0)
		Number = -Number;

	while (Number > 0) {
		T d = Number % 10;
		res += ('0' + d);
		Number /= 10;
	}

	if (Num < 0)
		res += "-";

	// reverse string
	Common::String revStr;
	for (int i = res.size() - 1; i >= 0; i--)
		revStr += res[i];

	return revStr;
}

template<typename T>
T StringToNumber(char *Text) {
	T result = 0;
	char *num = Text;

	if (Text[0] == '-')
		num++;

	for (int i = 0; num[i]; i++)
		result = (result * 10) + (num[i] - '0');

	return Text[0] == '-' ? -result : result;
}

template<typename T>
T StringToNumber(const Common::String &Text) {
	return StringToNumber<T>(const_cast<char *>(Text.c_str()));
}

template<typename T>
void GetPoint(T &v, Common::String &coords) {
	int comma = coords.findFirstOf(',');

	v.x = StringToNumber<int>(coords.substr(0, comma));
	v.y = StringToNumber<int>(coords.substr(comma + 1));
}

} // End of namespace Crab

#endif // CRAB_NUMSTR_H
