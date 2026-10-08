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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCUMM_INSANE_HIGHSCORES_H
#define SCUMM_INSANE_HIGHSCORES_H

#include "common/util.h"

namespace Scumm {

template<class Entry, class Score>
int findHighScorePosition(const Entry *entries, int count, Score score, bool beforeEqual = false) {
	int position = 0;
	while (position < count && (entries[position].score > score ||
		(!beforeEqual && entries[position].score == score)))
		++position;
	return position;
}

template<class Entry>
int insertHighScoreEntry(Entry *entries, int count, int capacity, int position, const Entry &entry) {
	assert(count >= 0 && count <= capacity && position >= 0 && position <= count);
	if (position >= capacity)
		return count;
	for (int i = MIN(count, capacity - 1); i > position; --i)
		entries[i] = entries[i - 1];
	entries[position] = entry;
	return MIN(count + 1, capacity);
}

} // End of namespace Scumm

#endif
