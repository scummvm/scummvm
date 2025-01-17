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

#include "got/game/move_patterns.h"
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/boss1.h"
#include "got/game/boss2.h"
#include "got/game/boss3.h"
#include "got/game/move.h"
#include "got/game/object.h"
#include "got/game/script.h"
#include "got/game/special_tile.h"
#include "got/vars.h"

namespace Got {

#define TILE_SOLID 80
#define TILE_FLY 140
#define TILE_SPECIAL 200

int check_special_move1(int x, int y, Actor *actr);

int check_move0(int x, int y, Actor *actr);
int check_move1(int x, int y, Actor *actr);
//int check_move2(int x, int y, Actor *actr);
//int check_move3(int x, int y, Actor *actr);
//int check_move4(int x, int y, Actor *actr);

int movement_zero(Actor *actr);
int movement_one(Actor *actr);
int movement_two(Actor *actr);
int movement_three(Actor *actr);
int movement_four(Actor *actr);
int movement_five(Actor *actr);
int movement_six(Actor *actr);
int movement_seven(Actor *actr);
int movement_eight(Actor *actr);
int movement_nine(Actor *actr);
int movement_ten(Actor *actr);
int movement_eleven(Actor *actr);
int movement_twelve(Actor *actr);      // Horz straight until bump
int movement_thirteen(Actor *actr);    // Pause-seek (mushroom)
int movement_fourteen(Actor *actr);    // Move-bump-stop (boulder)
int movement_fifteen(Actor *actr);     // No move, no frame cycle
int movement_sixteen(Actor *actr);     // Tornado 1
int movement_seventeen(Actor *actr);   // Tornado 2
int movement_eighteen(Actor *actr);    // Random-seek-bite-run
int movement_nineteen(Actor *actr);    // Tornado 2
int movement_twenty(Actor *actr);      // Tornado 2
int movement_twentyone(Actor *actr);   // Eyeball
int movement_twentytwo(Actor *actr);   // Spear
int movement_twentythree(Actor *actr); // Spinball cw
int movement_twentyfour(Actor *actr);  // Spinball ccw
int movement_twentyfive(Actor *actr);  //
int movement_twentysix(Actor *actr);   //
int movement_twentyseven(Actor *actr); //
int movement_twentyeight(Actor *actr); // Tree boss
int movement_twentynine(Actor *actr);  // Horz or vert (pass_val)
int movement_thirty(Actor *actr);      // Vert straight
int movement_thirtyone(Actor *actr);   // Drop (stalagtite)
int movement_thirtytwo(Actor *actr);   // Bomb 1
int movement_thirtythree(Actor *actr); // Bomb 2
int movement_thirtyfour(Actor *actr);  // Gun (4-dir)
int movement_thirtyfive(Actor *actr);  // Gun (4-dir)
int movement_thirtysix(Actor *actr);   // Acid drop
int movement_thirtyseven(Actor *actr); // 4-way rnd,rnd len
int movement_thirtyeight(Actor *actr); // Timed darting
int movement_thirtynine(Actor *actr);  // Troll 1
int movement_forty(Actor *actr);       // Troll 2

int (*const movement_func[])(Actor *actr) = {
	movement_zero,
	movement_one,
	movement_two,
	movement_three,
	movement_four,
	movement_five,
	movement_six,
	movement_seven,
	movement_eight,
	movement_nine,
	movement_ten,
	movement_eleven,
	movement_twelve,
	movement_thirteen,
	movement_fourteen,
	movement_fifteen,
	movement_sixteen,
	movement_seventeen,
	movement_eighteen,
	movement_nineteen,
	movement_twenty,
	movement_twentyone,
	movement_twentytwo,
	movement_twentythree,
	movement_twentyfour,
	movement_twentyfive,
	movement_twentysix,
	movement_twentyseven,
	movement_twentyeight,
	movement_twentynine,
	movement_thirty,
	movement_thirtyone,
	movement_thirtytwo,
	movement_thirtythree,
	movement_thirtyfour,
	movement_thirtyfive,
	movement_thirtysix,
	movement_thirtyseven,
	movement_thirtyeight,
	movement_thirtynine,
	movement_forty};

int special_movement_one(Actor *actr);
int special_movement_two(Actor *actr);
int special_movement_three(Actor *actr);
int special_movement_four(Actor *actr);
int special_movement_five(Actor *actr);
int special_movement_six(Actor *actr);
int special_movement_seven(Actor *actr);
int special_movement_eight(Actor *actr);
int special_movement_nine(Actor *actr);
int special_movement_ten(Actor *actr);
int special_movement_eleven(Actor *actr);

int (*special_movement_func[])(Actor *actr) = {
	nullptr,
	special_movement_one,
	special_movement_two,
	special_movement_three,
	special_movement_four,
	special_movement_five,
	special_movement_six,
	special_movement_seven,
	special_movement_eight,
	special_movement_nine,
	special_movement_ten,
	special_movement_eleven};

// Check Thor move
int check_move0(int x, int y, Actor *actr) {
	_G(thor_icon1) = 0;
	_G(thor_icon2) = 0;
	_G(thor_icon3) = 0;
	_G(thor_icon4) = 0;

	if (x < 0) {
		if (_G(current_level) > 0) {
			_G(new_level) = _G(current_level) - 1;
			actr->_x = 304;
			actr->_lastX[0] = 304;
			actr->_lastX[1] = 304;
			actr->_show = 0;
			actr->_moveCount = 0;
			set_thor_vars();
			return 1;
		}

		return 0;
	}
	
	if (x > 306) {
		if (_G(current_level) < 119) {
			_G(new_level) = _G(current_level) + 1;
			actr->_x = 0;
			actr->_lastX[0] = 0;
			actr->_lastX[1] = 0;
			actr->_show = 0;
			actr->_moveCount = 0;
			set_thor_vars();
			return 1;
		}

		return 0;
	}
	
	if (y < 0) {
		if (_G(current_level) > 9) {
			_G(new_level) = _G(current_level) - 10;
			actr->_y = 175;
			actr->_lastY[0] = 175;
			actr->_show = 0;
			actr->_lastY[1] = 175;
			actr->_moveCount = 0;
			set_thor_vars();
			return 1;
		}

		return 0;
	}
	
	if (y > 175) {
		if (_G(current_level) < 110) {
			_G(new_level) = _G(current_level) + 10;
			actr->_y = 0;
			actr->_lastY[0] = 0;
			actr->_lastY[1] = 0;
			actr->_show = 0;
			actr->_moveCount = 0;
			set_thor_vars();
			return 1;
		}

		return 0;
	}
	
	int x1 = (x + 1) >> 4;
	int y1 = (y + 8) >> 4;
	int x2 = (_G(thor)->_dir > 1) ? (x + 12) >> 4 : (x + 10) >> 4;
	int y2 = (y + 15) >> 4;

	_G(slip_flag) = false;

	// Check for cheat flying mode
	if (!actr->_flying) {
		byte icn1 = _G(scrn).icon[y1][x1];
		byte icn2 = _G(scrn).icon[y2][x1];
		byte icn3 = _G(scrn).icon[y1][x2];
		byte icn4 = _G(scrn).icon[y2][x2];
		int ti = 0;

		if (icn1 < TILE_FLY) {
			_G(thor_icon1) = 1;
			ti = 1;
		}
		
		if (icn2 < TILE_FLY) {
			_G(thor_icon2) = 1;
			ti = 1;
		}
		
		if (icn3 < TILE_FLY) {
			_G(thor_icon3) = 1;
			ti = 1;
		}
		
		if (icn4 < TILE_FLY) {
			_G(thor_icon4) = 1;
			ti = 1;
		}
		
		if (ti)
			return 0;

		if (icn1 > TILE_SPECIAL) {
			if (!special_tile_thor(y1, x1, icn1))
				return 0;
			icn2 = _G(scrn).icon[y2][x1];
			icn3 = _G(scrn).icon[y1][x2];
			icn4 = _G(scrn).icon[y2][x2];
		}

		if (icn2 > TILE_SPECIAL) {
			if (!special_tile_thor(y2, x1, icn2))
				return 0;
			icn3 = _G(scrn).icon[y1][x2];
			icn4 = _G(scrn).icon[y2][x2];
		}

		if (icn3 > TILE_SPECIAL) {
			if (!special_tile_thor(y1, x2, icn3))
				return 0;
			icn4 = _G(scrn).icon[y2][x2];
		}

		if (icn4 > TILE_SPECIAL && !special_tile_thor(y2, x2, icn4))
			return 0;
	}

	if (!_G(slip_flag)) {
		_G(slipping) = false;
		_G(slip_cnt) = 0;
	}
	
	if (_G(slip_flag) && !_G(slipping))
		_G(slip_cnt++);
	
	if (_G(slip_cnt) > 8)
		_G(slipping) = true;
	
	_G(slip_flag) = false;

	x1 = x + 1;
	y1 = y + 8;
	
	x2 = x + 12;	
	y2 = y + 15;

	_G(thor_special_flag) = false;
	for (int i = 3; i < MAX_ACTORS; i++) {
		Actor *act = &_G(actor[i]);
		if ((act->_solid & 128) || !act->_active)
			continue;

		int x3 = act->_x + 1;
		int y3 = act->_y + 1;
		
		if (ABS(x3 - x1) > 16 || ABS(y3 - y1) > 16)
			continue;
		
		int x4 = act->_x + act->_sizeX - 1;
		int y4 = act->_y + act->_sizeY - 1;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) {
			if (act->_funcNum > 0) { // 255=explosion
				if (act->_funcNum == 255)
					return 0;
				act->_temp1 = x;
				act->_temp2 = y;
				_G(thor_special_flag) = true;
				
				return special_movement_func[act->_funcNum](act);
			}

			_G(thor_special_flag) = false;
			thor_damaged(act);
			if (act->_solid < 2) {
				if (!act->_vulnerableCountdown && (!(act->_type & 1)))
					play_sound(PUNCH1, false);

				if (!_G(hammer)->_active && _G(key_flag[key_fire]))
					actor_damaged(act, _G(hammer)->_hitStrength);
				else
					actor_damaged(act, _G(thor)->_hitStrength);
			}
			return 1;
		}
	}
	actr->_x = x;
	actr->_y = y;
	return 1;
}

// Check hammer move
int check_move1(int x, int y, Actor *actr) {
	if (x < 0 || x > 306 || y < 0 || y > 177)
		return 0;

	int x1 = (x + 1) >> 4;
	int y1 = (y + 6) >> 4;
	int x2 = (x + 10) >> 4;
	int y2 = (y + 10) >> 4;

	// Check for solid or fly over
	int icn = TILE_FLY;
	if (actr->_flying)
		icn = TILE_SOLID;

	byte icn1 = _G(scrn).icon[y1][x1];
	byte icn2 = _G(scrn).icon[y2][x1];
	byte icn3 = _G(scrn).icon[y1][x2];
	byte icn4 = _G(scrn).icon[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn) {
		if (actr->_actorNum == 1 && actr->_moveType == 2)
			play_sound(CLANG, false);

		return 0;
	}

	if (icn1 > TILE_SPECIAL && !special_tile(actr, y1, x1, icn1))
		return 0;

	if (icn2 > TILE_SPECIAL && !special_tile(actr, y2, x1, icn2))
		return 0;

	if (icn3 > TILE_SPECIAL && !special_tile(actr, y1, x2, icn3))
		return 0;

	if (icn4 > TILE_SPECIAL && !special_tile(actr, y2, x2, icn4))
		return 0;

	x1 = x + 1;
	y1 = y + 1;
	x2 = x + 10;
	y2 = y + 10;

	int f = 0;
	for (int i = 3; i < MAX_ACTORS; i++) {
		Actor *act = &_G(actor[i]);
		if (!act->_active || act->_type == 3)
			continue;
		
		int x3 = act->_x;
		int y3 = act->_y;

		if (ABS(x3 - x1) > 16 || ABS(y3 - y1) > 16)
			continue;
		
		int x4 = act->_x + act->_sizeX - 1;
		int y4 = act->_y + act->_sizeY - 1;

		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) {
			if (_G(boss_active) && !GAME3) {
				switch (_G(area)) {
				case 1:
					check_boss1_hit(act, x1, y1, x2, y2, i);
					break;
				case 2:
					check_boss2_hit(act, x1, y1, x2, y2, i);
					break;
				default:
					// Area 3 boss Loki isn't checked here
					break;
				}
			} else {
				if (act->_solid == 2 && (actr->_moveType == 16 || actr->_moveType == 17))
					return 0;
				actor_damaged(act, actr->_hitStrength);
			}
			f++;
		}
	}
	if (f && actr->_moveType == 2)
		return 0;

	actr->_x = x;
	actr->_y = y;
	return 1;
}

// Check enemy move
int check_move2(int x, int y, Actor *actr) {
	if (actr->_actorNum < 3)
		return check_move1(x, y, actr);

	if (x < 0 || x > (319 - actr->_sizeX) || y < 0 || y > 175)
		return 0;

	int x1 = (x + 1) >> 4;
	int y1;
	if (!actr->_funcNum)
		y1 = (y + (actr->_sizeY / 2)) >> 4;
	else
		y1 = (y + 1) >> 4;

	int x2 = ((x + actr->_sizeX) - 1) >> 4;
	int y2 = ((y + actr->_sizeY) - 1) >> 4;

	// Check for solid or fly over

	int icn = TILE_FLY;
	if (actr->_flying)
		icn = TILE_SOLID;

	byte icn1 = _G(scrn).icon[y1][x1];
	byte icn2 = _G(scrn).icon[y2][x1];
	byte icn3 = _G(scrn).icon[y1][x2];
	byte icn4 = _G(scrn).icon[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn)
		return 0;

	if (icn1 > TILE_SPECIAL)
		if (!special_tile(actr, y1, x1, icn1))
			return 0;
	if (icn2 > TILE_SPECIAL)
		if (!special_tile(actr, y2, x1, icn2))
			return 0;
	if (icn3 > TILE_SPECIAL)
		if (!special_tile(actr, y1, x2, icn3))
			return 0;
	if (icn4 > TILE_SPECIAL)
		if (!special_tile(actr, y2, x2, icn4))
			return 0;

	x1 = x + 1;
	y1 = y + 1;
	x2 = (x + actr->_sizeX) - 1;
	y2 = (y + actr->_sizeY) - 1;

	for (int i = 0; i < MAX_ACTORS; i++) {
		Actor *act = &_G(actor[i]);
		if (act->_actorNum == actr->_actorNum || act->_actorNum == 1 || !act->_active)
			continue;
		if (act->_type == 3)
			continue; // Shot

		if (i == 0) {
			if (overlap(x1, y1, x2, y2, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
				thor_damaged(actr);
				return 0;
			}
		} else {
			int x3 = act->_x;
			int y3 = act->_y;
			
			if (ABS(x3 - x1) > 16 || ABS(y3 - y1) > 16)
				continue;

			int x4 = act->_x + act->_sizeX;
			int y4 = act->_y + act->_sizeY;
			if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) {
				if (actr->_moveType == 38) {
					if (act->_funcNum == 4)
						_G(switch_flag) = 1;
					else if (act->_funcNum == 7)
						_G(switch_flag) = 2;
				}
				return 0;
			}
		}
	}
	actr->_x = x;
	actr->_y = y;
	return 1;
}

// Check enemy shot move
int check_move3(int x, int y, Actor *actr) {
	if (x < 0 || x > (319 - actr->_sizeX) || y < 0 || y > 175)
		return 0;

	int x1 = (x + 1) >> 4;
	int y1 = (y + (actr->_sizeY / 2)) >> 4;
	int x2 = ((x + actr->_sizeX) - 1) >> 4;
	int y2 = ((y + actr->_sizeY) - 1) >> 4;

	// Check for solid or fly over

	int icn = TILE_FLY;
	if (actr->_flying)
		icn = TILE_SOLID;

	byte icn1 = _G(scrn).icon[y1][x1];
	byte icn2 = _G(scrn).icon[y2][x1];
	byte icn3 = _G(scrn).icon[y1][x2];
	byte icn4 = _G(scrn).icon[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn)
		return 0;

	if (icn1 > TILE_SPECIAL && !special_tile(actr, y1, x1, icn1))
		return 0;
	if (icn2 > TILE_SPECIAL && !special_tile(actr, y2, x1, icn2))
		return 0;
	if (icn3 > TILE_SPECIAL && !special_tile(actr, y1, x2, icn3))
		return 0;
	if (icn4 > TILE_SPECIAL && !special_tile(actr, y2, x2, icn4))
		return 0;

	// Check for solid or fly over
	x1 = x + 1;
	y1 = y + 1;
	x2 = (x + actr->_sizeX) - 1;
	y2 = (y + actr->_sizeY) - 1;

	if (overlap(x1, y1, x2, y2, _G(thor_x1), _G(thor_real_y1), _G(thor_x2), _G(thor_y2))) {
		thor_damaged(actr);
		return 0;
	}
	for (int i = 3; i < MAX_ACTORS; i++) {
		if (i == actr->_actorNum)
			continue;

		Actor *act = &_G(actor[i]);

		if (!act->_active)
			continue;
		if (act->_solid < 2)
			continue;
		if (act->_type == 3)
			continue; // Shot
		if (act->_actorNum == actr->_creator)
			continue;

		int x3 = act->_x;
		int y3 = act->_y;

		if (ABS(x3 - x1) > 16 || ABS(y3 - y1) > 16)
			continue;

		int x4 = x3 + 15;
		int y4 = y3 + 15;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4))
			return 0;
	}
	actr->_x = x;
	actr->_y = y;
	return 1;
}

// Flying enemies
int check_move4(int x, int y, Actor *actr) {
	if (x < 0 || x > (319 - actr->_sizeX) || y < 0 || y > 175)
		return 0;
	if (overlap(x, y, x + actr->_sizeX - 1, y + actr->_sizeY - 1,
				_G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
		if (actr->_type == 3)
			thor_damaged(actr);
		return 0;
	}
	actr->_x = x;
	actr->_y = y;
	return 1;
}

#define THOR_PAD1 2
#define THOR_PAD2 4

int check_thor_move(int x, int y, Actor *actr) {
	if (check_move0(x, y, actr))
		return 1;
	if (_G(diag_flag) || _G(thor_special_flag))
		return 0;

	if (_G(thor_icon1) + _G(thor_icon2) + _G(thor_icon3) + _G(thor_icon4) > 1)
		return 0;

	switch (actr->_dir) {
	case 0:
		if (_G(thor_icon1)) {
			actr->_dir = 3;
			if (check_move0(x + THOR_PAD1, y + 2, actr)) {
				actr->_dir = 0;
				return 1;
			}
		} else if (_G(thor_icon3)) {
			actr->_dir = 2;
			if (check_move0(x - THOR_PAD1, y + 2, actr)) {
				actr->_dir = 0;
				return 1;
			}
		}
		actr->_dir = 0;
		break;
	case 1:
		if (_G(thor_icon2)) {
			actr->_dir = 3;
			if (check_move0(x + THOR_PAD1, y - 2, actr)) {
				actr->_dir = 1;
				return 1;
			}
		} else if (_G(thor_icon4)) {
			actr->_dir = 2;
			if (check_move0(x - THOR_PAD1, y - 2, actr)) {
				actr->_dir = 1;
				return 1;
			}
		}
		actr->_dir = 1;
		break;
	case 2:
		if (_G(thor_icon1)) {
			if (check_move0(x + 2, y + THOR_PAD1, actr))
				return 1;
		} else if (_G(thor_icon2)) {
			if (check_move0(x + 2, y - THOR_PAD1, actr))
				return 1;
		}
		break;
	case 3:
		if (_G(thor_icon3)) {
			if (check_move0(x - 2, y + THOR_PAD1, actr))
				return 1;
		} else if (_G(thor_icon4)) {
			if (check_move0(x - 2, y - THOR_PAD1, actr))
				return 1;
		}
		break;
	}

	return 0;
}

// Player control
int movement_zero(Actor *actr) {
	int d = actr->_dir;
	int od = d;

	set_thor_vars();

	if (_G(hammer)->_active && _G(hammer)->_moveType == 5) {
		if (overlap(_G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2),
					_G(hammer)->_x, _G(hammer)->_y, _G(hammer)->_x + 13, _G(hammer)->_y + 13)) {
			actor_destroyed(_G(hammer));
		}
	}
	int x = actr->_x;
	int y = actr->_y;
	_G(diag_flag) = false;
	if (actr->_moveCounter)
		actr->_moveCounter--;

	if (_G(slipping)) {
		if (_G(slip_cnt) == 8)
			play_sound(FALL, true);

		y += 2;
		_G(slip_cnt--);
		if (!_G(slip_cnt))
			_G(slipping) = false;

		check_thor_move(x, y, actr);
		_G(thor)->_moveCountdown = 4;
		return d;
	}

	if (_G(key_flag[key_up]) && _G(key_flag[key_left])) {
		d = 2;
		actr->_dir = d;
		_G(diag) = 1;
		_G(diag_flag) = true;
		if (check_thor_move(x - 2, y - 2, actr)) {
			next_frame(actr);
			return d;
		}
	} else if (_G(key_flag[key_up]) && _G(key_flag[key_right])) {
		d = 3;
		actr->_dir = d;
		_G(diag) = 2;
		_G(diag_flag) = true;
		if (check_thor_move(x + 2, y - 2, actr)) {
			next_frame(actr);
			return d;
		}
	} else if (_G(key_flag[key_down]) && _G(key_flag[key_left])) {
		d = 2;
		actr->_dir = d;
		_G(diag) = 4;
		_G(diag_flag) = true;
		if (check_thor_move(x - 2, y + 2, actr)) {
			next_frame(actr);
			return d;
		}
	} else if (_G(key_flag[key_down]) && _G(key_flag[key_right])) {
		d = 3;
		actr->_dir = d;
		_G(diag) = 3;
		_G(diag_flag) = true;
		if (check_thor_move(x + 2, y + 2, actr)) {
			next_frame(actr);
			return d;
		}
	}
	_G(diag) = 0;
	if (_G(key_flag[key_right])) {
		if (!_G(key_flag[key_left])) {
			d = 3;
			actr->_dir = d;
			if (check_thor_move(x + 2, y, actr)) {
				next_frame(actr);
				return d;
			}
		}
	}
	if (_G(key_flag[key_left])) {
		if (!_G(key_flag[key_right])) {
			d = 2;
			actr->_dir = d;
			if (check_thor_move(x - 2, y, actr)) {
				next_frame(actr);
				return d;
			}
		}
	}
	if (_G(key_flag[key_down])) {
		if (!_G(key_flag[key_up])) {
			d = 1;
			actr->_dir = d;
			if (check_thor_move(x, y + 2, actr)) {
				next_frame(actr);
				return d;
			}
		}
	}
	if (_G(key_flag[key_up])) {
		if (!_G(key_flag[key_down])) {
			d = 0;
			actr->_dir = d;
			if (check_thor_move(x, y - 2, actr)) {
				next_frame(actr);
				return d;
			}
		}
	}
	actr->_moveCounter = 5;
	actr->_nextFrame = 0;
	actr->_dir = od;
	return d;
}

int check_special_move1(int x, int y, Actor *actr) {
	int i;
	int x3, y3, x4, y4;

	Actor *act;

	if (actr->_actorNum < 3)
		return check_move1(x, y, actr);

	if (x < 0 || x > 304 || y < 0 || y > 176)
		return 0;

	int x1 = x >> 4;
	int y1 = y >> 4;
	int x2 = (x + 15) >> 4;
	int y2 = (y + 15) >> 4;

	// Check for solid or fly over

	int icn = TILE_FLY;
	if (actr->_flying)
		icn = TILE_SOLID;

	byte icn1 = _G(scrn).icon[y1][x1];
	byte icn2 = _G(scrn).icon[y2][x1];
	byte icn3 = _G(scrn).icon[y1][x2];
	byte icn4 = _G(scrn).icon[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn)
		return 0;

	if (icn1 > TILE_SPECIAL && !special_tile(actr, y1, x1, icn1))
		return 0;
	if (icn2 > TILE_SPECIAL && !special_tile(actr, y2, x1, icn2))
		return 0;
	if (icn3 > TILE_SPECIAL && !special_tile(actr, y1, x2, icn3))
		return 0;
	if (icn4 > TILE_SPECIAL && !special_tile(actr, y2, x2, icn4))
		return 0;

	x1 = x;
	y1 = y;
	x2 = (x + 15);
	y2 = (y + 15);

	for (i = 3; i < MAX_ACTORS; i++) {
		act = &_G(actor[i]);
		if (act->_actorNum == actr->_actorNum)
			continue;
		if (!act->_active)
			continue;
		if (act->_type == 3)
			continue; //shot
		x3 = act->_x;
		if ((ABS(x3 - x1)) > 16)
			continue;
		y3 = act->_y;
		if ((ABS(y3 - y1)) > 16)
			continue;
		x4 = act->_x + act->_sizeX;
		y4 = act->_y + 15;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4))
			return 0;
	}
	for (i = 3; i < MAX_ACTORS; i++) {
		act = &_G(actor[i]);
		if (act->_actorNum == actr->_actorNum)
			continue;
		if (!act->_active)
			continue;
		if (act->_type == 3)
			continue; // Shot
		x3 = act->_x;
		if ((ABS(x3 - x1)) > 16)
			continue;
		y3 = act->_y;
		if ((ABS(y3 - y1)) > 16)
			continue;
		x4 = act->_x + act->_sizeX;
		y4 = act->_y + act->_sizeY;
		if (overlap(_G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2), x3, y3, x4, y4))
			return 0;
	}
	actr->_x = x;
	actr->_y = y;
	return 1;
}

//*==========================================================================

// Block
int special_movement_one(Actor *actr) {
	if (_G(diag_flag))
		return 0;

	int d = _G(thor)->_dir;
	int x1 = actr->_x;
	int y1 = actr->_y;
	int sd = actr->_lastDir;
	actr->_lastDir = d;

	switch (d) {
	case 0:
		y1 -= 2;
		if (!check_special_move1(x1, y1, actr)) {
			actr->_lastDir = sd;
			return 0;
		}
		break;
	case 1:
		y1 += 2;
		if (!check_special_move1(x1, y1, actr)) {
			actr->_lastDir = sd;
			return 0;
		}
		break;
	case 2:
		x1 -= 2;
		if (!check_special_move1(x1, y1, actr)) {
			actr->_lastDir = sd;
			return 0;
		}
		break;
	case 3:
		x1 += 2;
		if (!check_special_move1(x1, y1, actr)) {
			actr->_lastDir = sd;
			return 0;
		}
		break;
	}
	next_frame(actr);
	actr->_lastDir = d;
	return 1;
}

// Angle
int special_movement_two(Actor *actr) {
	int x1 = actr->_temp1; // Calc thor pos
	int y1 = actr->_temp2;
	int x2 = x1 + 13;
	int y2 = y1 + 14;

	for (int i = 3; i < MAX_ACTORS; i++) {
		Actor *act = &_G(actor[i]);
		if (actr->_actorNum == act->_actorNum)
			continue;
		if (!act->_active)
			continue;
		int x3 = act->_x;
		if ((ABS(x3 - x1)) > 16)
			continue;
		int y3 = act->_y;
		if ((ABS(y3 - y1)) > 16)
			continue;
		int x4 = act->_x + act->_sizeX;
		int y4 = act->_y + act->_sizeY;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4))
			return 0;
	}
	if (!actr->_funcPass) {
		if (_G(thor)->_health < 150) {
			if (!sound_playing())
				play_sound(ANGEL, false);
			_G(thor)->_health += 1;
		}
	} else if (_G(thor_info).magic < 150) {
		if (!sound_playing())
			play_sound(ANGEL, false);
		_G(thor_info).magic += 1;
	}

	return 1;
}

// Yellow globe
int special_movement_three(Actor *actr) {
	if (_G(thunder_flag))
		return 0;

	long lind = (long)_G(current_level);
	lind *= 1000;
	lind += (long)actr->_actorNum;
	execute_script(lind, _G(odin));

	return 0;
}

// Peg switch
int special_movement_four(Actor *actr) {
	if (actr->_shotCountdown != 0)
		return 0;
	actr->_shotCountdown = 30;

	_G(switch_flag) = 1;
	return 0;
}

// Boulder roll
int special_movement_five(Actor *actr) {
	int d = _G(thor)->_dir;

	if (_G(diag_flag)) {
		switch (_G(diag)) {
		case 1:
			if (_G(thor_x1) < (actr->_x + 15))
				d = 0;
			else
				d = 2;
			break;
		case 2:
			if (_G(thor_x2) < actr->_x)
				d = 3;
			else
				d = 0;
			break;
		case 3:
			if (_G(thor_x2) > (actr->_x))
				d = 1;
			else
				d = 3;
			break;
		case 4:
			if (_G(thor_x1) > (actr->_x + 15))
				d = 2;
			else
				d = 1;
			break;
		}
	}

	actr->_lastDir = d;
	actr->_moveType = 14;
	return 0;
}

int special_movement_six(Actor *actr) {
	thor_damaged(actr);
	return 0;
}

int special_movement_seven(Actor *actr) {
	if (actr->_shotCountdown != 0)
		return 0;

	actr->_shotCountdown = 30;

	_G(switch_flag) = 2;
	return 0;
}

int special_movement_eight(Actor *actr) {
	if (_G(thor)->_dir < 2 || _G(diag_flag))
		return 0;

	actr->_lastDir = _G(thor)->_dir;
	actr->_moveType = 14;
	return 0;
}

int special_movement_nine(Actor *actr) {
	if (_G(thor)->_dir > 1 || _G(diag_flag))
		return 0;

	actr->_lastDir = _G(thor)->_dir;
	actr->_moveType = 14;
	return 0;
}

int special_movement_ten(Actor *actr) {
	byte &actor_ctr = GAME1 ? actr->_temp6 : actr->_talkCounter;

	if (actor_ctr) {
		actor_ctr--;
		return 0;
	}

	if (_G(thunder_flag))
		return 0;

	actor_ctr = 10;
	actor_speaks(actr, 0 - actr->_passValue, 0);
	return 0;
}

// Red guard
int special_movement_eleven(Actor *actr) {
	if (actr->_talkCounter) {
		actr->_talkCounter--;
		return 0;
	}

	const int oldType = actr->_type;
	actr->_type = 4;
	actor_speaks(actr, 0, 0);
	actr->_type = oldType;
	actr->_talkCounter = 10;

	return 0;
}

// No movement - frame cycle
int movement_one(Actor *actr) {
	next_frame(actr);
	return actr->_dir;
}

// Hammer only
int movement_two(Actor *actr) {
	int d = actr->_lastDir;
	int x1 = actr->_x;
	int y1 = actr->_y;

	switch (d) {
	case 0:
		y1 -= 2;
		break;
	case 1:
		y1 += 2;
		break;
	case 2:
		x1 -= 2;
		break;
	case 3:
		x1 += 2;
		break;
	}
	if (!check_move2(x1, y1, actr)) {
		if (actr->_actorNum == 1) {
			_G(hammer)->_moveType = 5;
			d = reverse_direction(_G(hammer));
			_G(hammer)->_dir = d;
		}
		if (actr->_actorNum == 2) {
			actr->_active = 0;
			actr->_dead = 2;
			_G(lightning_used) = false;
			_G(tornado_used) = false;
		}
	}
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Walk-bump-random turn
int movement_three(Actor *actr) {
	int d = actr->_lastDir;
	int x1 = actr->_x;
	int y1 = actr->_y;

	switch (d) {
	case 0:
		y1 -= 2;
		if (!check_move2(x1, y1, actr)) {
			d = g_events->getRandomNumber(3);
		}
		break;
	case 1:
		y1 += 2;
		if (!check_move2(x1, y1, actr)) {
			d = g_events->getRandomNumber(3);
		}
		break;
	case 2:
		x1 -= 2;
		if (!check_move2(x1, y1, actr)) {
			d = g_events->getRandomNumber(3);
		}
		break;
	case 3:
		x1 += 2;
		if (!check_move2(x1, y1, actr)) {
			d = g_events->getRandomNumber(3);
		}
		break;
	}
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Simple tracking
int movement_four(Actor *actr) {
	int d = actr->_lastDir;

	int x1 = actr->_x;
	int y1 = actr->_y;

	int f = 0;
	if (x1 > _G(thor_x1) - 1) {
		x1 -= 2;
		d = 2;
		f = 1;
	} else if (x1 < _G(thor_x1) - 1) {
		x1 += 2;
		d = 3;
		f = 1;
	}

	if (f)
		f = check_move2(x1, y1, actr);

	if (!f) {
		if (y1 < (_G(thor_real_y1))) {
			d = (_G(thor_real_y1)) - y1;
			if (d > 2)
				d = 2;
			y1 += d;
			d = 1;
			f = 1;
		} else if (y1 > (_G(thor_real_y1))) {
			d = y1 - (_G(thor_real_y1));
			if (d > 2)
				d = 2;
			y1 -= d;
			d = 0;
			f = 1;
		}
		if (f)
			f = check_move2(actr->_x, y1, actr);
		if (!f)
			check_move2(actr->_x, actr->_y, actr);
	}
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

int movement_five(Actor *actr) {
	int x1 = actr->_x;
	int y1 = actr->_y;
	int xd = 0;
	int yd = 0;
	int d = actr->_lastDir;

	if (x1 > (_G(thor)->_x + 1))
		xd = -2; //+1
	else if (x1 < (_G(thor)->_x - 1))
		xd = 2;

	if (actr->_actorNum == 1) {
		if (y1 < (_G(thor_y1) - 6))
			yd = 2;
		else if (y1 > (_G(thor_y1) - 6))
			yd = -2;
	} else {
		if (y1 < (_G(thor_real_y1) - 1))
			yd = 2;
		else if (y1 > (_G(thor_real_y1) + 1))
			yd = -2;
	}

	if (xd && yd) {
		if (xd == -2 && yd == -2)
			d = 2;
		else if (xd == -2 && yd == 2)
			d = 2;
		else if (xd == 2 && yd == -2)
			d = 3;
		else if (xd == 2 && yd == 2)
			d = 3;
		x1 += xd;
		y1 += yd;
		if (check_move2(x1, y1, actr)) {
			next_frame(actr);
			actr->_lastDir = d;
			if (actr->_directions == 1)
				return 0;
			return d;
		}
	} else {
		if (xd == 0 && yd == 2)
			d = 1;
		else if (xd == 0 && yd == -2)
			d = 0;
		else if (xd == 2 && yd == 0)
			d = 3;
		else if (xd == -2 && yd == 0)
			d = 2;
	}
	x1 = actr->_x;
	y1 = actr->_y;
	actr->_toggle ^= 1;

	if (actr->_toggle) {
		if (xd) {
			x1 += xd;
			if (check_move2(x1, y1, actr)) {
				if (xd > 0)
					d = 3;
				else
					d = 2;
				next_frame(actr);
				actr->_lastDir = d;
				if (actr->_directions == 1)
					return 0;
				return d;
			}
			x1 = actr->_x;
		}
		if (yd) {
			y1 += yd;
			if (check_move2(x1, y1, actr)) {
				if (yd > 0)
					d = 1;
				else
					d = 0;
				next_frame(actr);
				actr->_lastDir = d;
				if (actr->_directions == 1)
					return 0;
				return d;
			}
		}
	} else {
		if (yd) {
			y1 += yd;
			if (check_move2(x1, y1, actr)) {
				if (yd > 0)
					d = 1;
				else
					d = 0;
				next_frame(actr);
				actr->_lastDir = d;
				if (actr->_directions == 1)
					return 0;
				return d;
			}
			y1 = actr->_y;
		}
		if (xd) {
			x1 += xd;
			if (check_move2(x1, y1, actr)) {
				if (xd > 0)
					d = 3;
				else
					d = 2;
				next_frame(actr);
				actr->_lastDir = d;
				if (actr->_directions == 1)
					return 0;
				return d;
			}
		}
	}
	check_move2(actr->_x, actr->_y, actr);
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Explosion only
int movement_six(Actor *actr) {
	if (actr->_currNumShots > 0) {
		actr->_nextFrame++;
		if (actr->_nextFrame > 2) {
			actr->_nextFrame = 0;
			if (_G(boss_dead))
				play_sound(EXPLODE, false);
		}
		actr->_currNumShots--;
	} else {
		actr->_dead = 2;
		actr->_active = 0;
		if (!_G(boss_dead) && !_G(endgame)) {
			if (actr->_type == 2)
				drop_object(actr);
		}
	}

	next_frame(actr);
	return 0;
}

// Walk-bump-random turn (pause also)
int movement_seven(Actor *actr) {
	if (actr->_nextFrame == 0 && actr->_frameCount == actr->_frameSpeed) {
		actr->_moveCountdown = 12;
		actr->_lastDir = g_events->getRandomNumber(3);
	}
	return movement_three(actr);
}

// Follow thor
int movement_eight(Actor *actr) {
	if (_G(thor)->_x > 0)
		actr->_x = _G(thor)->_x - 1;
	else
		actr->_x = _G(thor)->_x;
	actr->_y = _G(thor)->_y;
	next_frame(actr);
	return 0;
}

// 4-way straight (random length) change
int movement_nine(Actor *actr) {
	int d = actr->_lastDir;
	int x1 = actr->_x;
	int y1 = actr->_y;

	int f = 0;
	if (actr->_counter) {
		actr->_counter--;
		switch (d) {
		case 0:
			y1 -= 2;
			if (!check_move4(x1, y1, actr))
				f = 1;
			break;
		case 1:
			y1 += 2;
			if (!check_move4(x1, y1, actr))
				f = 1;
			break;
		case 2:
			x1 -= 2;
			if (!check_move4(x1, y1, actr))
				f = 1;
			break;
		case 3:
			x1 += 2;
			if (!check_move4(x1, y1, actr))
				f = 1;
			break;
		}
	} else
		f = 1;

	if (f == 1) {
		actr->_counter = g_events->getRandomNumber(10, 99);
		d = g_events->getRandomNumber(3);
	}
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Vert straight (random length) change
int movement_ten(Actor *actr) {
	int lastDir = actr->_lastDir;
	int x1 = actr->_x;
	int y1 = actr->_y;

	bool setRandomDirFl = false;
	if (actr->_counter) {
		if (actr->_passValue != 1)
			actr->_counter--;
		switch (lastDir) {
		case 0:
		case 2:
			y1 -= 2;
			if (!check_move2(x1, y1, actr))
				setRandomDirFl = true;
			break;
		case 1:
		case 3:
			y1 += 2;
			if (!check_move2(x1, y1, actr))
				setRandomDirFl = true;
			break;

		default:
			break;
		}
	} else
		setRandomDirFl = true;

	if (setRandomDirFl) {
		actr->_counter = g_events->getRandomNumber(10, 99);
		lastDir = g_events->getRandomNumber(1);
	}

	if (lastDir > 1)
		lastDir -= 2;

	next_frame(actr);
	actr->_lastDir = lastDir;
	if (actr->_directions == 1)
		return 0;
	return lastDir;
}

// Horz only (bats)
int movement_eleven(Actor *actr) {
	int d = actr->_lastDir;

	switch (d) {
	case 0:
		if (check_move2(actr->_x - 2, actr->_y - 2, actr))
			break;
		d = 1;
		if (check_move2(actr->_x - 2, actr->_y + 2, actr))
			break;
		d = 2;
		break;
	case 1:
		if (check_move2(actr->_x - 2, actr->_y + 2, actr))
			break;
		d = 0;
		if (check_move2(actr->_x - 2, actr->_y - 2, actr))
			break;
		d = 3;
		break;
	case 2:
		if (check_move2(actr->_x + 2, actr->_y - 2, actr))
			break;
		d = 3;
		if (check_move2(actr->_x + 2, actr->_y + 2, actr))
			break;
		d = 0;
		break;
	case 3:
		if (check_move2(actr->_x + 2, actr->_y + 2, actr))
			break;
		d = 2;
		if (check_move2(actr->_x + 2, actr->_y - 2, actr))
			break;
		d = 1;
		break;
	}

	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Horz straight until bump
int movement_twelve(Actor *actr) {
	int d = actr->_lastDir;

	switch (d) {
	case 0:
	case 2:
		if (check_move2(actr->_x - 2, actr->_y, actr))
			break;
		d = 3;
		break;
	case 1:
	case 3:
		if (check_move2(actr->_x + 2, actr->_y, actr))
			break;
		d = 2;
		break;
	}
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Pause-seek (mushroom)
int movement_thirteen(Actor *actr) {
	int d = actr->_lastDir;

	if (actr->_counter == 0 && actr->_unpauseCountdown == 0) {
		actr->_unpauseCountdown = 60;
		return d;
	}
	if (actr->_unpauseCountdown > 0) {
		actr->_unpauseCountdown--;
		if (!actr->_unpauseCountdown)
			actr->_counter = 60;
		actr->_vulnerableCountdown = 5;
		actr->_hitStrength = 0;
		return d;
	}
	if (actr->_counter > 0) {
		actr->_counter--;
		if (!actr->_counter)
			actr->_unpauseCountdown = 60;
		actr->_hitStrength = 10;
		return movement_five(actr);
	}
	return d;
}

// Move-bump-stop (boulder)
int movement_fourteen(Actor *actr) {
	int d = actr->_lastDir;
	actr->_dir = d;
	int x1 = actr->_x;
	int y1 = actr->_y;

	switch (d) {
	case 0:
		y1 -= 2;
		if (!check_move2(x1, y1, actr)) {
			actr->_moveType = 15;
			return 0;
		}
		break;
	case 1:
		y1 += 2;
		if (!check_move2(x1, y1, actr)) {
			actr->_moveType = 15;
			return 0;
		}
		break;
	case 2:
		x1 -= 2;
		if (!check_move2(x1, y1, actr)) {
			actr->_moveType = 15;
			return 0;
		}
		break;
	case 3:
		x1 += 2;
		if (!check_move2(x1, y1, actr)) {
			actr->_moveType = 15;
			return 0;
		}
		break;
	}
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// No movement - no frame cycle
int movement_fifteen(Actor *actr) {
	return actr->_dir;
}

// Tornado 1
int movement_sixteen(Actor *actr) {
	int d = actr->_lastDir;
	int x1 = actr->_x;
	int y1 = actr->_y;

	switch (d) {
	case 0:
		y1 -= 2;
		break;
	case 1:
		y1 += 2;
		break;
	case 2:
		x1 -= 2;
		break;
	case 3:
		x1 += 2;
		break;
	}
	if (!check_move1(x1, y1, actr)) {
		actr->_moveType = 17;
		d = g_events->getRandomNumber(3);
	}

	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Tornado 2
int movement_seventeen(Actor *actr) {
	int d = actr->_lastDir;

	switch (d) {
	case 0:
		if (check_move1(actr->_x - 2, actr->_y - 2, actr))
			break;
		d = 1;
		if (check_move1(actr->_x - 2, actr->_y + 2, actr))
			break;
		d = 2;
		break;
	case 1:
		if (check_move1(actr->_x - 2, actr->_y + 2, actr))
			break;
		d = 0;
		if (check_move1(actr->_x - 2, actr->_y - 2, actr))
			break;
		d = 3;
		break;
	case 2:
		if (check_move1(actr->_x + 2, actr->_y - 2, actr))
			break;
		d = 3;
		if (check_move1(actr->_x + 2, actr->_y + 2, actr))
			break;
		d = 0;
		break;
	case 3:
		if (check_move1(actr->_x + 2, actr->_y + 2, actr))
			break;
		d = 2;
		if (check_move1(actr->_x + 2, actr->_y - 2, actr))
			break;
		d = 1;
		break;
	}
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// No movement - frame cycle
int movement_eighteen(Actor *actr) {
	int d;

	if (actr->_temp5) {
		actr->_temp5--;
		if (!actr->_temp5)
			actr->_numMoves = 1;
	}
	if (actr->_temp1) {
		d = movement_five(actr);
		actr->_rand--;
		if (actr->_hitThor || !actr->_rand) {
			if (actr->_hitThor) {
				actr->_temp5 = 50;
				actr->_numMoves = 2;
				actr->_hitThor = 0;
				actr->_dir = d;
				d = reverse_direction(actr);
			}
			actr->_temp1 = 0;
			actr->_rand = g_events->getRandomNumber(50, 149);
		}
	} else {
		d = movement_three(actr);
		actr->_rand--;
		if (!actr->_rand) {
			actr->_temp5 = 0;
			actr->_temp1 = 1;
			actr->_rand = g_events->getRandomNumber(50, 149);
		}
		if (actr->_hitThor) {
			actr->_temp5 = 50;
			actr->_numMoves = 2;
			actr->_hitThor = 0;
			actr->_dir = d;
			d = reverse_direction(actr);
		}
	}
	next_frame(actr);
	return d;
}

// No movement - frame cycle
int movement_nineteen(Actor *actr) {
	return movement_seven(actr);
}

int movement_twenty(Actor *actr) {
	if (GAME1)
		// Boss - snake
		return boss1_movement(actr);

	return movement_one(actr);
}

// No movement - frame cycle
int movement_twentyone(Actor *actr) {
	return movement_three(actr);
}

// Spear
int movement_twentytwo(Actor *actr) {
	int d = actr->_lastDir;
	if (actr->_directions == 1)
		d = 0;

redo:

	switch (actr->_temp2) {
	case 0:
		if (bgtile(actr->_x, actr->_y) >= TILE_SOLID)
			actr->_nextFrame = 1;
		else {
			actr->_temp2 = 6;
			actr->_temp1 = 1;
			goto redo;
		}
		actr->_temp2++;
		break;
	case 1:
		actr->_nextFrame = 2;
		actr->_temp2++;
		break;
	case 2:
		actr->_nextFrame = 3;
		actr->_hitStrength = 255;
		actr->_temp2++;
		actr->_temp1 = 10;
		break;
	case 3:
		check_move2(actr->_x, actr->_y, actr);
		actr->_temp1--;
		if (actr->_temp1)
			break;
		actr->_temp2++;
		actr->_nextFrame = 2;
		break;
	case 4:
		actr->_hitStrength = 0;
		actr->_temp2++;
		actr->_nextFrame = 1;
		break;
	case 5:
		actr->_temp2++;
		actr->_nextFrame = 0;
		actr->_temp1 = 10;
		break;
	case 6:
		actr->_temp1--;
		if (actr->_temp1)
			break;
		actr->_temp2 = 0;
		actr->_nextFrame = 0;
		switch (d) {
		case 0:
			actr->_x += 16;
			actr->_y += 16;
			d = 3;
			if (bgtile(actr->_x, actr->_y) < TILE_SOLID)
				goto redo;
			break;
		case 1:
			actr->_x -= 16;
			actr->_y -= 16;
			d = 2;
			if (bgtile(actr->_x, actr->_y) < TILE_SOLID)
				goto redo;
			break;
		case 2:
			actr->_x += 16;
			actr->_y -= 16;
			d = 0;
			if (bgtile(actr->_x, actr->_y) < TILE_SOLID)
				goto redo;
			break;
		case 3:
			actr->_x -= 16;
			actr->_y += 16;
			d = 1;
			if (bgtile(actr->_x, actr->_y) < TILE_SOLID)
				goto redo;
			break;
		}
		actr->_dir = d;
		actr->_lastDir = d;
		break;
	}
	return d;
}

// Spinball counter-clockwise
int movement_twentythree(Actor *actr) {
	int d = actr->_lastDir;
	next_frame(actr);
	if (actr->_passValue & 2)
		actr->_numMoves = 2;

	switch (d) {
	case 0:
		if (bgtile(actr->_x - 2, actr->_y) >= TILE_FLY &&
			bgtile(actr->_x - 2, actr->_y + actr->_sizeY - 1) >= TILE_FLY) {
			d = 2;
			actr->_x -= 2;
		} else {
			if (bgtile(actr->_x, actr->_y - 2) < TILE_FLY ||
				bgtile(actr->_x + actr->_sizeX - 1, actr->_y - 2) < TILE_FLY) {
				if (bgtile(actr->_x + actr->_sizeX + 1, actr->_y) >= TILE_FLY &&
					bgtile(actr->_x + actr->_sizeX + 1, actr->_y + actr->_sizeY - 1) >= TILE_FLY) {
					d = 3;
					actr->_x += 2;
				} else {
					d = 1;
					break;
				}
			} else
				actr->_y -= 2;
		}
		break;
	case 1:
		if (bgtile(actr->_x + actr->_sizeX + 1, actr->_y) >= TILE_FLY &&
			bgtile(actr->_x + actr->_sizeX + 1, actr->_y + actr->_sizeY - 1) >= TILE_FLY) {
			d = 3;
			actr->_x += 2;
		} else {
			if (bgtile(actr->_x, actr->_y + actr->_sizeY + 1) < TILE_FLY ||
				bgtile(actr->_x + actr->_sizeX - 1, actr->_y + actr->_sizeY + 1) < TILE_FLY) {
				if (bgtile(actr->_x - 2, actr->_y) >= TILE_FLY &&
					bgtile(actr->_x - 2, actr->_y + actr->_sizeY - 1) >= TILE_FLY) {
					d = 2;
					actr->_x -= 2;
				} else {
					d = 0;
					break;
				}
			} else
				actr->_y += 2;
		}
		break;
	case 2:
		if (bgtile(actr->_x, actr->_y + actr->_sizeY + 1) >= TILE_FLY &&
			bgtile(actr->_x + actr->_sizeX - 1, actr->_y + actr->_sizeY + 1) >= TILE_FLY) {
			d = 1;
			actr->_y += 2;
		} else {
			if (bgtile(actr->_x - 2, actr->_y) < TILE_FLY ||
				bgtile(actr->_x - 2, actr->_y + actr->_sizeY - 1) < TILE_FLY) {
				if (bgtile(actr->_x, actr->_y - 2) >= TILE_FLY &&
					bgtile(actr->_x + actr->_sizeX - 1, actr->_y - 2) >= TILE_FLY) {
					d = 0;
					actr->_y -= 2;
				} else {
					d = 3;
					break;
				}
			} else
				actr->_x -= 2;
		}
		break;
	case 3:
		if (bgtile(actr->_x, actr->_y - 2) >= TILE_FLY &&
			bgtile(actr->_x + actr->_sizeX - 1, actr->_y - 2) >= TILE_FLY) {
			d = 0;
			actr->_y -= 2;
		} else {
			if (bgtile(actr->_x + actr->_sizeX + 1, actr->_y) < TILE_FLY ||
				bgtile(actr->_x + actr->_sizeX + 1, actr->_y + actr->_sizeY - 1) < TILE_FLY) {
				if (bgtile(actr->_x, actr->_y + actr->_sizeY + 1) >= TILE_FLY &&
					bgtile(actr->_x + actr->_sizeX - 1, actr->_y + actr->_sizeY + 1) >= TILE_FLY) {
					d = 1;
					actr->_y += 2;
				} else {
					d = 2;
					break;
				}
			} else
				actr->_x += 2;
		}
		break;
	}
	check_move2(actr->_x, actr->_y, actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Spinball  clockwise
int movement_twentyfour(Actor *actr) {
	int d = actr->_lastDir;
	next_frame(actr);
	if (actr->_passValue & 2)
		actr->_numMoves = 2;

	switch (d) {
	case 0:
		if (bgtile(actr->_x + actr->_sizeX + 1, actr->_y) >= TILE_FLY &&
			bgtile(actr->_x + actr->_sizeX + 1, actr->_y + actr->_sizeY - 1) >= TILE_FLY) {
			d = 3;
			actr->_x += 2;
		} else {
			if (bgtile(actr->_x, actr->_y - 2) < TILE_FLY ||
				bgtile(actr->_x + actr->_sizeX - 1, actr->_y - 2) < TILE_FLY) {
				if (bgtile(actr->_x - 2, actr->_y) >= TILE_FLY &&
					bgtile(actr->_x - 2, actr->_y + actr->_sizeY - 1) >= TILE_FLY) {
					d = 2;
					actr->_x -= 2;
				} else {
					d = 1;
					break;
				}
			} else
				actr->_y -= 2;
		}
		break;
	case 1:
		if (bgtile(actr->_x - 2, actr->_y) >= TILE_FLY &&
			bgtile(actr->_x - 2, actr->_y + actr->_sizeY - 1) >= TILE_FLY) {
			d = 2;
			actr->_x -= 2;
		} else {
			if (bgtile(actr->_x, actr->_y + actr->_sizeY + 1) < TILE_FLY ||
				bgtile(actr->_x + actr->_sizeX - 1, actr->_y + actr->_sizeY + 1) < TILE_FLY) {
				if (bgtile(actr->_x + actr->_sizeX + 1, actr->_y) >= TILE_FLY &&
					bgtile(actr->_x + actr->_sizeX + 1, actr->_y + actr->_sizeY - 1) >= TILE_FLY) {
					d = 3;
					actr->_x += 2;
				} else {
					d = 0;
					break;
				}
			} else
				actr->_y += 2;
		}
		break;
	case 2:
		if (bgtile(actr->_x, actr->_y - 2) >= TILE_FLY &&
			bgtile(actr->_x + actr->_sizeX - 1, actr->_y - 2) >= TILE_FLY) {
			d = 0;
			actr->_y -= 2;
		} else {
			if (bgtile(actr->_x - 2, actr->_y) < TILE_FLY ||
				bgtile(actr->_x - 2, actr->_y + actr->_sizeY - 1) < TILE_FLY) {
				if (bgtile(actr->_x, actr->_y + actr->_sizeY + 1) >= TILE_FLY &&
					bgtile(actr->_x + actr->_sizeX - 1, actr->_y + actr->_sizeY + 1) >= TILE_FLY) {
					d = 1;
					actr->_y += 2;
				} else {
					d = 3;
					break;
				}
			} else
				actr->_x -= 2;
		}
		break;
	case 3:
		if (bgtile(actr->_x, actr->_y + actr->_sizeY + 1) >= TILE_FLY &&
			bgtile(actr->_x + actr->_sizeX - 1, actr->_y + actr->_sizeY + 1) >= TILE_FLY) {
			d = 1;
			actr->_y += 2;
		} else {
			if (bgtile(actr->_x + actr->_sizeX + 1, actr->_y) < TILE_FLY ||
				bgtile(actr->_x + actr->_sizeX + 1, actr->_y + actr->_sizeY - 1) < TILE_FLY) {
				if (bgtile(actr->_x, actr->_y - 2) >= TILE_FLY &&
					bgtile(actr->_x + actr->_sizeX - 1, actr->_y - 2) >= TILE_FLY) {
					d = 0;
					actr->_y -= 2;
				} else {
					d = 2;
					break;
				}
			} else
				actr->_x += 2;
		}
		break;
	}
	check_move2(actr->_x, actr->_y, actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Acid puddle
int movement_twentyfive(Actor *actr) {
	if (actr->_temp2) {
		actr->_temp2--;
		return movement_one(actr);
	}
	if (!actr->_temp1) {
		actr->_lastDir = g_events->getRandomNumber(3);
		int i = 4;
		while (i--) {
			int ret = movement_three(actr);
			if (ret)
				return ret;
			actr->_lastDir++;
			if (actr->_lastDir > 3)
				actr->_lastDir = 0;
		}
		actr->_temp1 = 16;
	}
	actr->_temp1--;
	return movement_three(actr);
}

int movement_twentysix(Actor *actr) {
	if (GAME2)
		return boss2_movement(actr);
	if (GAME3)
		return boss3_movement(actr);

	return movement_one(actr);
}

int movement_twentyseven(Actor *actr) {
	if (GAME2)
		return boss2_movement(actr);

	return movement_one(actr);
}

void set_thor_vars() {
	_G(thor_x1) = _G(thor)->_x + 1;
	_G(thor_y1) = _G(thor)->_y + 8;

	_G(thor_real_y1) = _G(thor)->_y;
	_G(thor_x2) = (_G(thor)->_x + 12);
	_G(thor_y2) = _G(thor)->_y + 15;
}

// Fish
int movement_twentyeight(Actor *actr) {
	int d = actr->_lastDir;
	int x1 = actr->_x;
	int y1 = actr->_y;
	int ret;
	
	if (actr->_i1)
		actr->_i1--;
	else {
		if (!actr->_temp3) {
			if (!actr->_nextFrame) {
				actr->_frameCount = 1;
				actr->_frameSpeed = 4;
			}

			next_frame(actr);
			if (actr->_nextFrame == 3) {
				if (actr->_currNumShots < actr->_numShotsAllowed)
					actor_shoots(actr, 0);
				actr->_temp3 = 1;
			}
		} else {
			const int fcount = actr->_frameCount - 1;
			if (fcount <= 0) {
				actr->_nextFrame--;
				actr->_frameCount = actr->_frameSpeed;
				if (!actr->_nextFrame) {
					actr->_temp3 = 0;
					actr->_frameSpeed = 4;
					actr->_i1 = g_events->getRandomNumber(60, 159);
				}
			} else
				actr->_frameCount = fcount;
		}
		goto done;
	}
	switch (actr->_temp2) {
	case 0:
		y1 -= 2;
		break;
	case 1:
		y1 += 2;
		break;
	case 2:
		x1 -= 2;
		break;
	case 3:
		x1 += 2;
		break;
	}
	
	ret = bgtile(x1, y1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113)
		goto chg_dir;
	ret = bgtile((x1 + actr->_sizeX) - 1, y1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113)
		goto chg_dir;
	ret = bgtile(x1, (y1 + actr->_sizeY) - 1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113)
		goto chg_dir;
	ret = bgtile((x1 + actr->_sizeX) - 1, (y1 + actr->_sizeY) - 1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113)
		goto chg_dir;

	actr->_x = x1;
	actr->_y = y1;

	goto done;

chg_dir:
	actr->_temp2 = _G(rand1) % 4;

done:
	if (actr->_nextFrame) {
		x1 = actr->_x;
		y1 = actr->_y;
		actr->_solid = 1;
		check_move2(x1, y1, actr);
		actr->_x = x1;
		actr->_y = y1;
	} else
		actr->_solid = 2;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Horz or vert (pass_val)
int movement_twentynine(Actor *actr) {
	if (!actr->_passValue)
		return movement_thirty(actr);

	return movement_twelve(actr);
}

// Vert straight
int movement_thirty(Actor *actr) {
	int d = actr->_lastDir;
	int x1 = actr->_x;
	int y1 = actr->_y;

	switch (d) {
	case 0:
		y1 -= 2;
		break;
	case 1:
		y1 += 2;
		break;
	}
	if (!check_move2(x1, y1, actr))
		d ^= 1;

	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Drop (stalagtite)
int movement_thirtyone(Actor *actr) {
	int d = actr->_lastDir;
	int x1 = actr->_x;
	int y1 = actr->_y;

	if (actr->_temp1) {
		y1 += 2;
		if (!check_move2(x1, y1, actr))
			actor_destroyed(actr);
	} else if (_G(thor_y1) > y1 && ABS(x1 - _G(thor_x1)) < 16) {
		int cx = (actr->_x + (actr->_sizeX / 2)) >> 4;
		int cy = ((actr->_y + actr->_sizeY) - 2) >> 4;
		int ty = _G(thor)->_centerY;
		for (int i = cy; i <= ty; i++)
			if (_G(scrn).icon[i][cx] < TILE_SOLID)
				goto done;
		actr->_numMoves = actr->_passValue + 1;
		actr->_temp1 = 1;
	}

done:
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Unused
int movement_thirtytwo(Actor *actr) {
	return 0;
}

// Unused
int movement_thirtythree(Actor *actr) {
	return 0;
}

// Unused
int movement_thirtyfour(Actor *actr) {
	return 0;
}

// Gun (single)
int movement_thirtyfive(Actor *actr) {
	actr->_nextFrame = actr->_lastDir;
	return actr->_dir;
}

// Acid drop
int movement_thirtysix(Actor *actr) {
	actr->_speed = actr->_passValue;
	next_frame(actr);
	if (actr->_nextFrame == 0 && actr->_frameCount == actr->_frameSpeed) {
		actor_always_shoots(actr, 1);
		_G(actor[actr->_shotActor])._x -= 2;
	}
	return 0;
}

// 4-way straight (random length) change
int movement_thirtyseven(Actor *actr) {
	int d = actr->_lastDir;
	int x1 = actr->_x;
	int y1 = actr->_y;

	int f = 0;
	if (actr->_counter) {
		actr->_counter--;
		switch (d) {
		case 0:
			y1 -= 2;
			if (!check_move2(x1, y1, actr))
				f = 1;
			break;
		case 1:
			y1 += 2;
			if (!check_move2(x1, y1, actr))
				f = 1;
			break;
		case 2:
			x1 -= 2;
			if (!check_move2(x1, y1, actr))
				f = 1;
			break;
		case 3:
			x1 += 2;
			if (!check_move2(x1, y1, actr))
				f = 1;
			break;
		}
	} else
		f = 1;

	if (f == 1) {
		actr->_counter = g_events->getRandomNumber(10, 99);
		d = g_events->getRandomNumber(3);
	}
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Timed darting
#define TIMER actr->_i1
#define INIT_DIR actr->_temp1
#define OTHER_DIR actr->_temp2
#define FLAG actr->_temp3
#define OX actr->_i2
#define OY actr->_i3
#define CNT actr->_i4
int movement_thirtyeight(Actor *actr) {
	int d = actr->_lastDir;
	int x1 = actr->_x;
	int y1 = actr->_y;

	if (!FLAG) {
		FLAG = 1;
		if (actr->_passValue)
			TIMER = actr->_passValue * 15;
		else
			TIMER = g_events->getRandomNumber(5, 364);
		INIT_DIR = actr->_lastDir;
		OX = x1;
		OY = y1;
		CNT = 0;
		switch (INIT_DIR) {
		case 0:
			OTHER_DIR = 1;
			break;
		case 1:
			OTHER_DIR = 0;
			break;
		case 2:
			OTHER_DIR = 3;
			break;
		case 3:
			OTHER_DIR = 2;
			break;
		}
	}
	if (TIMER) {
		TIMER--;
		goto done;
	}

	switch (d) {
	case 0:
		y1 -= 2;
		break;
	case 1:
		y1 += 2;
		break;
	case 2:
		x1 -= 2;
		break;
	case 3:
		x1 += 2;
		break;
	}

	switch (FLAG) {
	case 1:
		if (!check_move2(x1, y1, actr)) {
			if (CNT) {
				d = OTHER_DIR;
				actr->_lastDir = d;
				FLAG = 2;
			} else {
				actr->_nextFrame = 0;
				FLAG = 0;
				goto done;
			}
		} else
			CNT++;
		break;
	case 2:
		check_move2(x1, y1, actr);
		if (x1 == OX && y1 == OY) {
			FLAG = 0;
			d = INIT_DIR;
			actr->_lastDir = d;
			actr->_nextFrame = 0;
			goto done;
		}
	}
	next_frame(actr);

done:
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Troll 1
int movement_thirtynine(Actor *actr) {
	if (_G(setup).skill == 0) {
		actr->_speed = 3;
		actr->_numMoves = 1;
	} else if (_G(setup).skill == 1) {
		actr->_speed = 2;
		actr->_numMoves = 1;
	} else if (_G(setup).skill == 2) {
		actr->_speed = 1;
		actr->_numMoves = 1;
	}

	if (actr->_passValue < 5)
		return movement_forty(actr);
	if (actr->_passValue == 10) {
		if (overlap(actr->_x + 1, actr->_y + 1, actr->_x + actr->_sizeX - 1,
					actr->_y + actr->_sizeY - 1, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
			actr->_hitStrength = 255;
			thor_damaged(actr);
		}
		return actr->_dir;
	}
	if (actr->_actorNum != 3)
		return actr->_dir;

	if (actr->_i1) {
		actr->_i1--;
		actr->_x -= 2;
		check_move2(actr->_x, actr->_y, actr);
		_G(actor[4])._x -= 2;
		_G(actor[5])._x -= 2;
		_G(actor[6])._x -= 2;
	}
	next_frame(actr);
	if (actr->_nextFrame == 3)
		actr->_nextFrame = 0;
	_G(actor[4])._nextFrame = _G(actor[3])._nextFrame;
	_G(actor[5])._nextFrame = 0;
	_G(actor[6])._nextFrame = 0;
	return actr->_dir;
}

// Troll 2
int movement_forty(Actor *actr) {
	if (overlap(actr->_x + 1, actr->_y + 1, actr->_x + actr->_sizeX + 3,
				actr->_y + actr->_sizeY - 1, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
		actr->_hitStrength = 150;
		thor_damaged(actr);
	}
	int a = 5 + (actr->_passValue * 4);
	const int x1 = actr->_x;
	int d = actr->_lastDir;

	if (actr->_lastDir == 2) {
		if (bgtile(x1 - 2, actr->_y) >= TILE_SOLID) {
			_G(actor[a]._x) -= 2;
			_G(actor[a - 1])._x -= 2;
			_G(actor[a - 2])._x -= 2;
			_G(actor[a + 1])._x -= 2;
		} else
			d = 3;
	} else if (bgtile(_G(actor[a + 1])._x + 14, _G(actor[a + 1])._y) >= TILE_SOLID) {
		_G(actor[a])._x += 2;
		_G(actor[a - 1])._x += 2;
		_G(actor[a - 2])._x += 2;
		_G(actor[a + 1])._x += 2;
	} else
		d = 2;

	if (actr->_nextFrame == 3 && !actr->_currNumShots && actr->_frameCount == actr->_frameSpeed) {
		actor_always_shoots(actr, 1);
		_G(actor[actr->_shotActor])._x += 6;
	}

	next_frame(actr);
	_G(actor[a - 2])._nextFrame = actr->_nextFrame;
	_G(actor[a - 1])._nextFrame = actr->_nextFrame;
	_G(actor[a + 1])._nextFrame = actr->_nextFrame;
	_G(actor[a - 2])._lastDir = d;
	_G(actor[a - 1])._lastDir = d;
	_G(actor[a + 1])._lastDir = d;
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

} // namespace Got
