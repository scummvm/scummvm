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

#include "got/game/shot_pattern.h"
#include "got/game/move.h"
#include "got/vars.h"

namespace Got {

int shot_pattern_none(Actor *actr);
int shot_pattern_one(Actor *actr);
int shot_pattern_two(Actor *actr);
int shot_pattern_three(Actor *actr);
int shot_pattern_four(Actor *actr);
int shot_pattern_five(Actor *actr);
int shot_pattern_six(Actor *actr);
int shot_pattern_seven(Actor *actr);
int shot_pattern_eight(Actor *actr);

int (*const shot_pattern_func[])(Actor *actr) = {
	shot_pattern_none,
	shot_pattern_one,
	shot_pattern_two,
	shot_pattern_three,
	shot_pattern_four,
	shot_pattern_five,
	shot_pattern_six,
	shot_pattern_seven,
	shot_pattern_eight,
};

// No shooting
int shot_pattern_none(Actor *actr) {
	return 0;
}

// Uni-directional seek
int shot_pattern_one(Actor *actr) {
	switch (actr->_lastDir) {
	case 0:
		if (ABS(_G(thor_x1) - actr->_x) < 8) {
			if (actr->_y > _G(thor_real_y1)) { //r
				if (actorShoots(actr, 0))
					return 1;
			}
		}
		break;
	case 1:
		if (ABS(_G(thor_x1) - actr->_x) < 8) {
			if (actr->_y < _G(thor_real_y1)) { //r
				if (actorShoots(actr, 1))
					return 1;
			}
		}
		break;
	case 2:
		if (ABS(_G(thor_real_y1) - actr->_y) < 8) { //r
			if (actr->_x > _G(thor_x1)) {
				if (actorShoots(actr, 2))
					return 1;
			}
		}
		break;
	case 3:
		if (ABS(_G(thor_real_y1) - actr->_y) < 8) { //r
			if (actr->_x < _G(thor_x1)) {
				if (actorShoots(actr, 3))
					return 1;
			}
		}
		break;
	}

	return 0;
}

// Omni directional
int shot_pattern_two(Actor *actr) {
	if (ABS(_G(thor)->_x - actr->_x) < 8) {
		if (_G(thor)->_y > actr->_y)
			actorShoots(actr, 1);
		else if (_G(thor)->_y < actr->_y)
			actorShoots(actr, 0);
		else
			return 0;
	} else if (ABS(_G(thor)->_y - actr->_y) < 8) {
		if (_G(thor)->_x > actr->_x)
			actorShoots(actr, 3);
		else if (_G(thor)->_x < actr->_x)
			actorShoots(actr, 2);
	} else {
		return 0;
	}

	return 1;
}

// Uni directional (backwards)
int shot_pattern_three(Actor *actr) {
	int ld = actr->_lastDir;
	if (shot_pattern_one(actr)) {
		actr->_lastDir = reverseDirection(actr);
		return 1;
	}

	actr->_lastDir = reverseDirection(actr);
	shot_pattern_one(actr);
	actr->_lastDir = ld;

	return 1;
}

// Omni-directional not solid shot
int shot_pattern_four(Actor *actr) {
	if (ABS(_G(thor)->_x - actr->_x) < 8) {
		if (_G(thor)->_y > actr->_y)
			actorAlwaysShoots(actr, 1);
		else if (_G(thor)->_y < actr->_y)
			actorAlwaysShoots(actr, 0);
		else
			return 0;
	} else if (ABS(_G(thor)->_y - actr->_y) < 8) {
		if (_G(thor)->_x > actr->_x)
			actorAlwaysShoots(actr, 3);
		else if (_G(thor)->_x < actr->_x)
			actorAlwaysShoots(actr, 2);
	} else {
		return 0;
	}

	return 1;
}

// Boss - snake
int shot_pattern_five(Actor *actr) {
	if (_G(rand1) < 15 && (actr->_temp1 == 0) && (actr->_temp2 == 0)) {
		actr->_y += 16;
		actr->_numShotsAllowed = 3 + _G(setup)._difficultyLevel;
		actorShoots(actr, 2);
		play_sound(BOSS12, false);

		int num = actr->_shotActor;
		actr->_shotCountdown = 50;
		_G(actor[num])._temp3 = 120;
		_G(actor[num])._temp4 = 5 + (_G(rand2) % 17);
		_G(actor[num])._temp5 = _G(actor[num])._temp4;
		actr->_y -= 16;
		return 1;
	}

	return 0;
}

// 4 surrounding squares
int shot_pattern_six(Actor *actr) {
	int pos = ((actr->_x) / 16) + (((actr->_y) / 16) * 20);

	if (_G(thor_pos) == pos - 20)
		actorShoots(actr, 0);
	else if (_G(thor_pos) == pos + 20)
		actorShoots(actr, 1);
	else if (_G(thor_pos) == pos - 1)
		actorShoots(actr, 2);
	else if (_G(thor_pos) == pos + 1)
		actorShoots(actr, 3);
	else
		return 0;

	actr->_frameSequence[3] = 3;
	actr->_nextFrame = 3;
	return 1;
}

// none
int shot_pattern_seven(Actor *actr) {
	return 0;
}

// random
int shot_pattern_eight(Actor *actr) {
	if (!actr->_i2) {
		actr->_i1 = actr->_funcPass;
		actr->_i2 = 1;
	}

	if (actr->_i1) {
		actr->_i1--;
	} else if (_G(rand1) < 10) {
		actr->_i1 = actr->_funcPass;
		actr->_i2 = _G(thor_real_y1);
		actorShoots(actr, 0);
		return 1;
	}

	return 0;
}

} // namespace Got
