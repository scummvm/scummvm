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

#ifndef GOT_DATA_HIGH_SCORE_H
#define GOT_DATA_HIGH_SCORE_H

#include "common/serializer.h"

namespace Got {

#define HIGH_SCORES_PER_AREA 7

struct HighScore {
	uint16 _unused = 0;
	uint32 _total = 0;
	char _name[32] = {};

	HighScore() {}
	HighScore(const char *name, uint32 total) : _total(total) {
		Common::strcpy_s(_name, name);
	}
	void sync(Common::Serializer &s);
};

struct HighScores {
private:
	void sync(Common::Serializer &s);

public:
	HighScore _scores[3][HIGH_SCORES_PER_AREA];

	void load();
	void save();
	void add(int area, const Common::String &name, uint total);
};

} // namespace Got

#endif
