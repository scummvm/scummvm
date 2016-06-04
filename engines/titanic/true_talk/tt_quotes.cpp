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
#include "titanic/titanic.h"

namespace Titanic {

TTquotes::TTquotes() {
	Common::fill(&_array[0], &_array[256], 0);
	_dataP = nullptr;
	_dataSize = 0;
	_field544 = 0;
}

TTquotes::~TTquotes() {
	delete[] _dataP;
}

void TTquotes::load(const CString &name) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource("TEXT/JRQuotes.txt");
	size_t size = r->readUint32LE();

	_dataSize = _field544 = size;
	_dataP = new char[size + 0x10];

	for (int idx = 0; idx < 256; ++idx)
		_array[idx] = r->readUint32LE();

	for (int charIdx = 0; charIdx < 26; ++charIdx) {
		TTquotesLetter &letter = _alphabet[charIdx];
		int count = r->readUint32LE();

		// Load the list of entries for the given letter
		letter._entries.resize(count);
		for (int idx = 0; idx < count; ++idx) {
			letter._entries[idx]._val1 = r->readByte();
			letter._entries[idx]._val2 = r->readByte();
			letter._entries[idx]._strP = _dataP + r->readUint32LE();
		}
	}

	// Read in buffer and then decode it
	r->read((byte *)_dataP, _dataSize);
	for (size_t idx = 0; idx < _dataSize; idx += 4)
		WRITE_LE_UINT32((byte *)_dataP + idx, READ_LE_UINT32((byte *)_dataP + idx) ^ 0xA55A5AA5);

	delete r;
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
	const TTquotesLetter &letter = _alphabet[index];
	if (letter._entries.empty())
		// No entries for the letter, so exit immediately
		return 0;

	int maxSize = size + 4;
	bool letterFlag = index != 25;

	for (uint idx = 0; idx < letter._entries.size(); ++idx) {
		const TTquotesEntry &entry = letter._entries[idx];
		if (entry._val2 > maxSize)
			continue;

		const char *srcP = startP;
		const char *destP = entry._strP;
		int srcIndex = 0, destIndex = 0;
		if (*destP) {
			do {
				if (!srcP[srcIndex]) {
					break;
				} else if (srcP[srcIndex] == '*') {
					++srcIndex;
				} else if (destP[destIndex] == '-') {
					++destIndex;
					if (srcP[srcIndex] == ' ')
						++srcIndex;
				} else if (srcP[srcIndex] != destP[destIndex]) {
					break;
				} else {
					++destIndex;
					++srcIndex;
				}
			} while (destP[destIndex]);

			if (!destP[destIndex] && (srcP[srcIndex] <= '*' ||
					(srcP[srcIndex] == 's' && srcP[srcIndex + 1] <= '*')))
				return entry._val1;
		}
	}

	return 0;
}

} // End of namespace Titanic
