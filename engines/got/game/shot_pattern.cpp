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

int shot_pattern_none(ACTOR *actr);
int shot_pattern_one(ACTOR *actr);
int shot_pattern_two(ACTOR *actr);
int shot_pattern_three(ACTOR *actr);
int shot_pattern_four(ACTOR *actr);
int shot_pattern_five(ACTOR *actr);
int shot_pattern_six(ACTOR *actr);
int shot_pattern_seven(ACTOR *actr);
int shot_pattern_eight(ACTOR *actr);

int (*const shot_pattern_func[])(ACTOR *actr) = {
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
int shot_pattern_none(ACTOR *actr) {
	return 0;
}

// Uni-directional seek
int shot_pattern_one(ACTOR *actr) {
	switch (actr->last_dir) {
	case 0:
		if (ABS(_G(thor_x1) - actr->x) < 8) {
			if (actr->y > _G(thor_real_y1)) { //r
				if (actor_shoots(actr, 0))
					return 1;
			}
		}
		break;
	case 1:
		if (ABS(_G(thor_x1) - actr->x) < 8) {
			if (actr->y < _G(thor_real_y1)) { //r
				if (actor_shoots(actr, 1))
					return 1;
			}
		}
		break;
	case 2:
		if (ABS(_G(thor_real_y1) - actr->y) < 8) { //r
			if (actr->x > _G(thor_x1)) {
				if (actor_shoots(actr, 2))
					return 1;
			}
		}
		break;
	case 3:
		if (ABS(_G(thor_real_y1) - actr->y) < 8) { //r
			if (actr->x < _G(thor_x1)) {
				if (actor_shoots(actr, 3))
					return 1;
			}
		}
		break;
	}

	return 0;
}

// Omni directional
int shot_pattern_two(ACTOR *actr) {
	if (ABS(_G(thor)->x - actr->x) < 8) {
		if (_G(thor)->y > actr->y)
			actor_shoots(actr, 1);
		else if (_G(thor)->y < actr->y)
			actor_shoots(actr, 0);
		else
			return 0;
	} else if (ABS(_G(thor)->y - actr->y) < 8) {
		if (_G(thor)->x > actr->x)
			actor_shoots(actr, 3);
		else if (_G(thor)->x < actr->x)
			actor_shoots(actr, 2);
	} else {
		return 0;
	}

	return 1;
}

// Uni directional (backwards)
int shot_pattern_three(ACTOR *actr) {
	int ld = actr->last_dir;
	if (shot_pattern_one(actr)) {
		actr->last_dir = reverse_direction(actr);
		return 1;
	}

	actr->last_dir = reverse_direction(actr);
	shot_pattern_one(actr);
	actr->last_dir = ld;

	return 1;
}

// Omni-directional not solid shot
int shot_pattern_four(ACTOR *actr) {
	if (ABS(_G(thor)->x - actr->x) < 8) {
		if (_G(thor)->y > actr->y)
			actor_always_shoots(actr, 1);
		else if (_G(thor)->y < actr->y)
			actor_always_shoots(actr, 0);
		else
			return 0;
	} else if (ABS(_G(thor)->y - actr->y) < 8) {
		if (_G(thor)->x > actr->x)
			actor_always_shoots(actr, 3);
		else if (_G(thor)->x < actr->x)
			actor_always_shoots(actr, 2);
	} else {
		return 0;
	}

	return 1;
}

// Boss - snake
int shot_pattern_five(ACTOR *actr) {
	if (_G(rand1) < 15 && (actr->temp1 == 0) && (actr->temp2 == 0)) {
		actr->y += 16;
		actr->shots_allowed = 3 + _G(setup).skill;
		actor_shoots(actr, 2);
		play_sound(BOSS12, false);

		int num = actr->shot_actor;
		actr->shot_cnt = 50;
		_G(actor[num]).temp3 = 120;
		_G(actor[num]).temp4 = 5 + (_G(rand2) % 17);
		_G(actor[num]).temp5 = _G(actor[num]).temp4;
		actr->y -= 16;
		return 1;
	}

	return 0;
}

// 4 surrounding squares
int shot_pattern_six(ACTOR *actr) {
	int pos = ((actr->x) / 16) + (((actr->y) / 16) * 20);

	if (_G(thor_pos) == pos - 20)
		actor_shoots(actr, 0);
	else if (_G(thor_pos) == pos + 20)
		actor_shoots(actr, 1);
	else if (_G(thor_pos) == pos - 1)
		actor_shoots(actr, 2);
	else if (_G(thor_pos) == pos + 1)
		actor_shoots(actr, 3);
	else
		return 0;

	actr->frame_sequence[3] = 3;
	actr->next = 3;
	return 1;
}

// none
int shot_pattern_seven(ACTOR *actr) {
	return 0;
}

// random
int shot_pattern_eight(ACTOR *actr) {
	if (!actr->i2) {
		actr->i1 = actr->func_pass;
		actr->i2 = 1;
	}

	if (actr->i1) {
		actr->i1--;
	} else if (_G(rand1) < 10) {
		actr->i1 = actr->func_pass;
		actr->i2 = _G(thor_real_y1);
		actor_shoots(actr, 0);
		return 1;
	}

	return 0;
}

} // namespace Got
