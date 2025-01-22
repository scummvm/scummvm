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

int shotPatternNone(Actor *actor);
int shotPatternOne(Actor *actor);
int shotPatternTwo(Actor *actor);
int shotPatternThree(Actor *actor);
int shotPatternFour(Actor *actor);
int shotPatternFive(Actor *actor);
int shotPatternSix(Actor *actor);
int shotPatternSeven(Actor *actor);
int shotPatternEight(Actor *actor);

int (*const shotPatternFunc[])(Actor *actor) = {
	shotPatternNone,
	shotPatternOne,
	shotPatternTwo,
	shotPatternThree,
	shotPatternFour,
	shotPatternFive,
	shotPatternSix,
	shotPatternSeven,
	shotPatternEight,
};

// No shooting
int shotPatternNone(Actor *actor) {
	return 0;
}

// Uni-directional seek
int shotPatternOne(Actor *actor) {
	switch (actor->_lastDir) {
	case 0:
		if (ABS(_G(thor_x1) - actor->_x) < 8) {
			if (actor->_y > _G(thor_real_y1)) { //r
				if (actorShoots(actor, 0))
					return 1;
			}
		}
		break;
		
	case 1:
		if (ABS(_G(thor_x1) - actor->_x) < 8) {
			if (actor->_y < _G(thor_real_y1)) { //r
				if (actorShoots(actor, 1))
					return 1;
			}
		}
		break;
		
	case 2:
		if (ABS(_G(thor_real_y1) - actor->_y) < 8) { //r
			if (actor->_x > _G(thor_x1)) {
				if (actorShoots(actor, 2))
					return 1;
			}
		}
		break;
		
	case 3:
		if (ABS(_G(thor_real_y1) - actor->_y) < 8) { //r
			if (actor->_x < _G(thor_x1)) {
				if (actorShoots(actor, 3))
					return 1;
			}
		}
		break;

	default:
		break;
	}

	return 0;
}

// Omni directional
int shotPatternTwo(Actor *actor) {
	if (ABS(_G(thor)->_x - actor->_x) < 8) {
		if (_G(thor)->_y > actor->_y)
			actorShoots(actor, 1);
		else if (_G(thor)->_y < actor->_y)
			actorShoots(actor, 0);
		else
			return 0;
	} else if (ABS(_G(thor)->_y - actor->_y) < 8) {
		if (_G(thor)->_x > actor->_x)
			actorShoots(actor, 3);
		else if (_G(thor)->_x < actor->_x)
			actorShoots(actor, 2);
	} else {
		return 0;
	}

	return 1;
}

// Uni directional (backwards)
int shotPatternThree(Actor *actor) {
	const int oldDir = actor->_lastDir;
	if (shotPatternOne(actor)) {
		actor->_lastDir = reverseDirection(actor);
		return 1;
	}

	actor->_lastDir = reverseDirection(actor);
	shotPatternOne(actor);
	actor->_lastDir = oldDir;

	return 1;
}

// Omni-directional not solid shot
int shotPatternFour(Actor *actor) {
	if (ABS(_G(thor)->_x - actor->_x) < 8) {
		if (_G(thor)->_y > actor->_y)
			actorAlwaysShoots(actor, 1);
		else if (_G(thor)->_y < actor->_y)
			actorAlwaysShoots(actor, 0);
		else
			return 0;
	} else if (ABS(_G(thor)->_y - actor->_y) < 8) {
		if (_G(thor)->_x > actor->_x)
			actorAlwaysShoots(actor, 3);
		else if (_G(thor)->_x < actor->_x)
			actorAlwaysShoots(actor, 2);
	} else {
		return 0;
	}

	return 1;
}

// Boss - snake
int shotPatternFive(Actor *actor) {
	if (_G(rand1) < 15 && (actor->_temp1 == 0) && (actor->_temp2 == 0)) {
		actor->_y += 16;
		actor->_numShotsAllowed = 3 + _G(setup)._difficultyLevel;
		actorShoots(actor, 2);
		playSound(BOSS12, false);

		const int num = actor->_shotActor;
		actor->_shotCountdown = 50;
		_G(actor[num])._temp3 = 120;
		_G(actor[num])._temp4 = 5 + (_G(rand2) % 17);
		_G(actor[num])._temp5 = _G(actor[num])._temp4;
		actor->_y -= 16;
		return 1;
	}

	return 0;
}

// 4 surrounding squares
int shotPatternSix(Actor *actor) {
	const int pos = ((actor->_x) / 16) + (((actor->_y) / 16) * 20);

	if (_G(thor_pos) == pos - 20)
		actorShoots(actor, 0);
	else if (_G(thor_pos) == pos + 20)
		actorShoots(actor, 1);
	else if (_G(thor_pos) == pos - 1)
		actorShoots(actor, 2);
	else if (_G(thor_pos) == pos + 1)
		actorShoots(actor, 3);
	else
		return 0;

	actor->_frameSequence[3] = 3;
	actor->_nextFrame = 3;
	return 1;
}

// none
int shotPatternSeven(Actor *actor) {
	return 0;
}

// random
int shotPatternEight(Actor *actor) {
	if (!actor->_i2) {
		actor->_i1 = actor->_funcPass;
		actor->_i2 = 1;
	}

	if (actor->_i1) {
		actor->_i1--;
	} else if (_G(rand1) < 10) {
		actor->_i1 = actor->_funcPass;
		actor->_i2 = _G(thor_real_y1);
		actorShoots(actor, 0);
		return 1;
	}

	return 0;
}

} // namespace Got
