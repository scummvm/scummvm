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

#include "got/data/setup.h"
#include "common/algorithm.h"

namespace Got {

SetupFlags &SetupFlags::operator=(const Got::SetupFlags &src) {
	Common::copy(src._flags, src._flags + 8, _flags);
	return *this;
}

void SetupFlags::sync(Common::Serializer &s) {
	byte flags[8] = {};

	if (s.isSaving()) {
		for (int i = 0; i < 64; ++i) {
			if (_flags[i])
				flags[i / 8] = flags[i / 8] | (1 << (i % 8));
		}
		s.syncBytes(flags, 8);
	} else {
		s.syncBytes(flags, 8);
		for (int i = 0; i < 64; ++i)
			_flags[i] = (flags[i / 8] & (1 << (i % 8))) != 0;
	}
}

void Setup::sync(Common::Serializer &s) {
	// Sync the flags bit-fields
	SetupFlags::sync(s);

	s.syncBytes(_value, 16);
	s.syncAsByte(_filler1);
	s.syncAsByte(_game);
	s.syncAsByte(_areaNum);
	s.syncAsByte(_speakerSound);
	s.syncAsByte(_digitalSound);
	s.syncAsByte(_musicEnabled);
	s.syncAsByte(_slowMode);
	s.syncAsByte(_scrollFlag);
	for (int i = 0; i < 3 ; ++i)
		s.syncAsByte(_bossDead[i]);
	s.syncAsByte(_difficultyLevel);
	s.syncAsByte(_gameOver);
	s.syncBytes(_filler2, 19);
}

} // namespace Got
