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

#include "common/random.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/oscar.h"

namespace Hollywood {
namespace Runaway {
namespace {

// Oscar uses the same idle sequence in the wide view and at the doorway.
const byte kOscarFrames[] = {
	106, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
	106, 106, 106, 106, 106, 106, 106, 106,
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
	53, 54, 55, 56, 57, 58, 59, 60, 53, 54, 55, 56, 57, 58, 59, 60,
	61, 62, 63, 64, 65, 66, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42,
	106, 106, 106, 106, 106, 106, 106, 106, 106,
	71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106
};

} // End of anonymous namespace

void OscarAnimation::update(uint32 now, Common::RandomSource &random, RunawayAudio &audio) {
	if (_talking) {
		_tick = now;
		return;
	}
	if (now - _tick < 80) {
		return;
	}
	_tick = now;
	const uint last = _idle == 1 ? 62 : _idle == 2 ? 121 : _closeup ? 165 : 164;
	if (_step < last) {
		++_step;
		if (_closeup && _idle == 2 && _step == 80) {
			audio.playEffect(_chapter == 6 ? 64 : 80, 26, 1);
		}
	} else if (_idle == 3 && !random.getRandomNumber(_closeup ? 4 : 9)) {
		_idle = 1 + random.getRandomBit();
		_step = _idle == 1 ? 0 : 70;
	} else {
		_idle = 3;
		_step = 130;
	}
	if (_closeup && _idle == 3 && !audio.effectPlaying(1)) {
		audio.playEffect(_chapter == 6 ? 63 : 76, 153, 1, true);
	}
}

uint OscarAnimation::frame(bool speaking, byte speechFrame) const {
	if (_talking) {
		return 107 + (speaking ? speechFrame % 6 : 0);
	}
	const uint frame = kOscarFrames[_step];
	return !_closeup && frame == 106 ? 105 : frame;
}

void OscarAnimation::startIdle() {
	_talking = false;
	_idle = 3;
	_step = 130;
}

} // End of namespace Runaway
} // End of namespace Hollywood
