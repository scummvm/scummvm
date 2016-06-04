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
#include "titanic/true_talk/tt_quotes.h"

namespace Titanic {

TTquotes::TTquotes() {
	Common::fill(&_array[0], &_array[256], 0);
	_dataP = nullptr;
	_field540 = 0;
	_field544 = 0;
}

TTquotes::~TTquotes() {
	for (int idx = 0; idx < 26; ++idx)
		delete _alphabet[idx]._dataP;

	delete _dataP;
}

void TTquotes::load(const CString &name) {
	// TODO
}

int TTquotes::read(const char *str) {
	if (!str || !*str)
		return 0;

	// Find start and end of string
	const char *startP = str, *endP = str;
	while (*endP)
		++endP;

	do {
		int result = read(startP, endP);
		if (result)
			return result;

		// Move to next following space or end of string
		while (*startP && *startP != ' ')
			++startP;
		// If it's a space, then move past it to start of next word
		while (*startP && *startP == ' ')
			++startP;

	} while (*startP);

	return 0;
}

int TTquotes::read(const char *startP, const char *endP) {
	int size = endP - startP;
	if (size < 3)
		return 0;

	uint index = MIN((uint)(*startP - 'a'), (uint)25);
	TTquotesEntry &entry = _alphabet[index];
	if (!entry._dataP || entry._field4 <= 0)
		return 0;

	// TODO
	return 0;
}

} // End of namespace Titanic
