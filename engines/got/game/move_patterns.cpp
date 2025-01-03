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
#include "got/game/back.h"
#include "got/game/boss1.h"
#include "got/game/move.h"
#include "got/game/object.h"
#include "got/gfx/panel.h"
#include "got/game/script.h"
#include "got/game/special_tile.h"
#include "got/events.h"
#include "got/vars.h"

namespace Got {

#define TILE_SOLID   80
#define TILE_FLY     140
#define TILE_SPECIAL 200

static const int bomb_x[] = { 0,-16,32,-32,32,-16,-16, 32,-16 };
static const int bomb_y[] = { 0,-16,16,  0,16,-32, 32,-32, 32 };
static const int8 rotate_pat[] = { 0,3,1,2 };

extern int (*dialog_func[]) (ACTOR *actr);

int check_special_move1(int x, int y, ACTOR *actr);

int  check_move0(int x, int y, ACTOR *actr);
int  check_move1(int x, int y, ACTOR *actr);
int  check_move2(int x, int y, ACTOR *actr);
int  check_move3(int x, int y, ACTOR *actr);
int  check_move4(int x, int y, ACTOR *actr);

int movement_zero(ACTOR *actr);
int movement_one(ACTOR *actr);
int movement_two(ACTOR *actr);
int movement_three(ACTOR *actr);
int movement_four(ACTOR *actr);
int movement_five(ACTOR *actr);
int movement_six(ACTOR *actr);
int movement_seven(ACTOR *actr);
int movement_eight(ACTOR *actr);
int movement_nine(ACTOR *actr);
int movement_ten(ACTOR *actr);
int movement_eleven(ACTOR *actr);
int movement_twelve(ACTOR *actr);         // horz straight until bump
int movement_thirteen(ACTOR *actr);       // pause-seek (mushroom)
int movement_fourteen(ACTOR *actr);       // move-bump-stop (boulder)
int movement_fifteen(ACTOR *actr);        // no move, no frame cycle
int movement_sixteen(ACTOR *actr);        // tornado 1
int movement_seventeen(ACTOR *actr);      // tornado 2
int movement_eighteen(ACTOR *actr);       // random-seek-bite-run
int movement_nineteen(ACTOR *actr);       // tornado 2
int movement_twenty(ACTOR *actr);         // tornado 2
int movement_twentyone(ACTOR *actr);      // eyeball
int movement_twentytwo(ACTOR *actr);      // spear
int movement_twentythree(ACTOR *actr);    // spinball cw
int movement_twentyfour(ACTOR *actr);     // spinball ccw
int movement_twentyfive(ACTOR *actr);     //
int movement_twentysix(ACTOR *actr);      //
int movement_twentyseven(ACTOR *actr);    //
int movement_twentyeight(ACTOR *actr);    // tree boss
int movement_twentynine(ACTOR *actr);     // horz or vert (pass_val)
int movement_thirty(ACTOR *actr);         // vert straight
int movement_thirtyone(ACTOR *actr);      // drop (stalagtite)
int movement_thirtytwo(ACTOR *actr);      // bomb 1
int movement_thirtythree(ACTOR *actr);    // bomb 2
int movement_thirtyfour(ACTOR *actr);     // gun (4-dir)
int movement_thirtyfive(ACTOR *actr);     // gun (4-dir)
int movement_thirtysix(ACTOR *actr);      // acid drop
int movement_thirtyseven(ACTOR *actr);    // 4-way rnd,rnd len
int movement_thirtyeight(ACTOR *actr);    // timed darting
int movement_thirtynine(ACTOR *actr);     // troll 1
int movement_forty(ACTOR *actr);          // troll 2

int (*const movement_func[]) (ACTOR *actr) = {
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
	movement_forty
};

int special_movement_one(ACTOR *actr);
int special_movement_two(ACTOR *actr);
int special_movement_three(ACTOR *actr);
int special_movement_four(ACTOR *actr);
int special_movement_five(ACTOR *actr);
int special_movement_six(ACTOR *actr);
int special_movement_seven(ACTOR *actr);
int special_movement_eight(ACTOR *actr);
int special_movement_nine(ACTOR *actr);
int special_movement_ten(ACTOR *actr);

int (*special_movement_func[])(ACTOR *actr) = {
	NULL,
	special_movement_one,
	special_movement_two,
	special_movement_three,
	special_movement_four,
	special_movement_five,
	special_movement_six,
	special_movement_seven,
	special_movement_eight,
	special_movement_nine,
	special_movement_ten
};


// check thor move
int check_move0(int x, int y, ACTOR *actr) {
	int x1, x2, y1, y2;
	int x3, x4, y3, y4;
	int i, ti;
	ACTOR *act;
	byte icn1, icn2, icn3, icn4;

	_G(thor_icon1) = 0;
	_G(thor_icon2) = 0;
	_G(thor_icon3) = 0;
	_G(thor_icon4) = 0;

	if (x < 0) {
		if (_G(current_level) > 0) {
			_G(new_level) = _G(current_level) - 1;
			actr->x = 304;
			actr->last_x[0] = 304;
			actr->last_x[1] = 304;
			actr->show = 0;
			actr->move_count = 0;
			set_thor_vars();
			return 1;
		} else return 0;
	}
	if (x > 306) {
		if (_G(current_level) < 119) {
			_G(new_level) = _G(current_level) + 1;
			actr->x = 0;
			actr->last_x[0] = 0;
			actr->last_x[1] = 0;
			actr->show = 0;
			actr->move_count = 0;
			set_thor_vars();
			return 1;
		} else return 0;
	}
	if (y < 0) {
		if (_G(current_level) > 9) {
			_G(new_level) = _G(current_level) - 10;
			actr->y = 175;
			actr->last_y[0] = 175;
			actr->show = 0;
			actr->last_y[1] = 175;
			actr->move_count = 0;
			set_thor_vars();
			return 1;
		} else return 0;
	}
	if (y > 175) {
		if (_G(current_level) < 110) {
			_G(new_level) = _G(current_level) + 10;
			actr->y = 0;
			actr->last_y[0] = 0;
			actr->last_y[1] = 0;
			actr->show = 0;
			actr->move_count = 0;
			set_thor_vars();
			return 1;
		} else return 0;
	}
	x1 = (x + 1) >> 4;
	y1 = (y + 8) >> 4;
	if (_G(thor)->dir > 1) x2 = (x + 12) >> 4;
	else x2 = (x + 10) >> 4;
	y2 = (y + 15) >> 4;

	_G(slip_flag) = 0;

	// Check for cheat flying mode
	if (!actr->flying) {
		icn1 = _G(scrn).icon[y1][x1];
		icn2 = _G(scrn).icon[y2][x1];
		icn3 = _G(scrn).icon[y1][x2];
		icn4 = _G(scrn).icon[y2][x2];
		ti = 0;

		if (icn1 < TILE_FLY) {
			_G(thor_icon1) = 1; ti = 1;
		}
		if (icn2 < TILE_FLY) {
			_G(thor_icon2) = 1; ti = 1;
		}
		if (icn3 < TILE_FLY) {
			_G(thor_icon3) = 1; ti = 1;
		}
		if (icn4 < TILE_FLY) {
			_G(thor_icon4) = 1; ti = 1;
		}
		if (ti) return 0;

		if (icn1 > TILE_SPECIAL) {
			if (!special_tile_thor(y1, x1, icn1)) return 0;
			icn2 = _G(scrn).icon[y2][x1];
			icn3 = _G(scrn).icon[y1][x2];
			icn4 = _G(scrn).icon[y2][x2];
		}
		if (icn2 > TILE_SPECIAL) {
			if (!special_tile_thor(y2, x1, icn2)) return 0;
			icn3 = _G(scrn).icon[y1][x2];
			icn4 = _G(scrn).icon[y2][x2];
		}
		if (icn3 > TILE_SPECIAL) {
			if (!special_tile_thor(y1, x2, icn3)) return 0;
			icn4 = _G(scrn).icon[y2][x2];
		}
		if (icn4 > TILE_SPECIAL) if (!special_tile_thor(y2, x2, icn4)) return 0;
	}

	if (!_G(slip_flag)) {
		_G(slipping) = 0;
		_G(slip_cnt) = 0;
	}
	if (_G(slip_flag) && !_G(slipping))
		_G(slip_cnt)++;
	if (_G(slip_cnt) > 8)
		_G(slipping) = 1;
	_G(slip_flag) = 0;


	x1 = x + 1;
	y1 = y + 8;
	if (_G(thor)->dir > 1) x2 = x + 12;
	else x2 = x + 12;
	y2 = y + 15;

	_G(thor_special_flag) = 0;
	for (i = 3; i < MAX_ACTORS; i++) {   //max_actors
		act = &_G(actor)[i];
		if (act->solid & 128) continue;
		if (!act->used) continue;
		x3 = act->x + 1;
		if ((ABS(x3 - x1)) > 16) continue;
		y3 = act->y + 1;
		if ((ABS(y3 - y1)) > 16) continue;
		x4 = act->x + act->size_x - 1;
		y4 = act->y + act->size_y - 1;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) {
			if (act->func_num > 0) { //255=explosion
				if (act->func_num == 255) return 0;
				act->temp1 = x;
				act->temp2 = y;
				_G(thor_special_flag) = 1;
				return special_movement_func[act->func_num](act);
			} else {
				_G(thor_special_flag) = 0;
				thor_damaged(act);
				if (act->solid < 2) {
					if (!act->vunerable && (!(act->type & 1))) play_sound(PUNCH1, 0);
					if (!_G(hammer)->used && _G(key_flag)[key_fire])
						actor_damaged(act, _G(hammer)->strength);
					else
						actor_damaged(act, _G(thor)->strength);
				}
			}
			return 1;
		}
	}
	actr->x = x;
	actr->y = y;
	return 1;
}

int check_move1(int x, int y, ACTOR *actr) {   //check hammer move
	int  x1, x2, y1, y2, i;
	int  x3, y3, x4, y4;
	int  icn, f;
	byte icn1, icn2, icn3, icn4;

	ACTOR *act;

	if (x < 0 || x>306 || y < 0 || y>177) return 0;

	x1 = (x + 1) >> 4;
	y1 = (y + 6) >> 4;
	x2 = (x + 10) >> 4;
	y2 = (y + 10) >> 4;

	//check for solid or fly over
	icn = TILE_FLY;
	if (actr->flying) icn = TILE_SOLID;

	icn1 = _G(scrn).icon[y1][x1];
	icn2 = _G(scrn).icon[y2][x1];
	icn3 = _G(scrn).icon[y1][x2];
	icn4 = _G(scrn).icon[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn) {
		if (actr->actor_num == 1 && actr->move == 2) play_sound(CLANG, 0);
		return 0;
	}

	if (icn1 > TILE_SPECIAL) if (!special_tile(actr, y1, x1, icn1)) return 0;
	if (icn2 > TILE_SPECIAL) if (!special_tile(actr, y2, x1, icn2)) return 0;
	if (icn3 > TILE_SPECIAL) if (!special_tile(actr, y1, x2, icn3)) return 0;
	if (icn4 > TILE_SPECIAL) if (!special_tile(actr, y2, x2, icn4)) return 0;

	x1 = x + 1;
	y1 = y + 1;
	x2 = x + 10;
	y2 = y + 10;

	f = 0;
	for (i = 3; i < MAX_ACTORS; i++) {       //max_actors
		act = &_G(actor)[i];
		if (!act->used) continue;
		if (act->type == 3) continue;  //continue
		x3 = act->x;
		if ((ABS(x3 - x1)) > 16) continue;
		y3 = act->y;
		if ((ABS(y3 - y1)) > 16) continue;
		x4 = act->x + act->size_x - 1;
		y4 = act->y + act->size_y - 1;

		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) {
			if (_G(boss_active)) {
				switch (_G(boss_active)) {
				case 1:
					check_boss1_hit(act, x1, y1, x2, y2, i);
					break;
				}
			} else {
				if (act->solid == 2 && (actr->move == 16 || actr->move == 17)) return 0;
				actor_damaged(act, actr->strength);
			}
			f++;
		}
	}
	if (f && actr->move == 2) return 0;
	actr->x = x;
	actr->y = y;
	return 1;
}

int check_move2(int x, int y, ACTOR *actr) {   //check enemy move
	int x1, x2, y1, y2, i;
	int x3, y3, x4, y4;
	int icn;
	byte icn1, icn2, icn3, icn4;

	ACTOR *act;

	if (actr->actor_num < 3) return check_move1(x, y, actr);

	if (x<0 || x>(319 - actr->size_x) || y < 0 || y>175) return 0;

	x1 = (x + 1) >> 4;
	if (!actr->func_num) y1 = (y + (actr->size_y / 2)) >> 4;
	else y1 = (y + 1) >> 4;
	x2 = ((x + actr->size_x) - 1) >> 4;
	y2 = ((y + actr->size_y) - 1) >> 4;

	//check for solid or fly over

	icn = TILE_FLY;
	if (actr->flying) icn = TILE_SOLID;


	icn1 = _G(scrn).icon[y1][x1];
	icn2 = _G(scrn).icon[y2][x1];
	icn3 = _G(scrn).icon[y1][x2];
	icn4 = _G(scrn).icon[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn) return 0;

	if (icn1 > TILE_SPECIAL) if (!special_tile(actr, y1, x1, icn1)) return 0;
	if (icn2 > TILE_SPECIAL) if (!special_tile(actr, y2, x1, icn2)) return 0;
	if (icn3 > TILE_SPECIAL) if (!special_tile(actr, y1, x2, icn3)) return 0;
	if (icn4 > TILE_SPECIAL) if (!special_tile(actr, y2, x2, icn4)) return 0;

	x1 = x + 1;
	y1 = y + 1;
	x2 = (x + actr->size_x) - 1;
	y2 = (y + actr->size_y) - 1;

	for (i = 0; i < MAX_ACTORS; i++) {   //max_actors
		act = &_G(actor)[i];
		if (act->actor_num == actr->actor_num) continue;
		if (act->actor_num == 1) continue;
		if (!act->used) continue;
		if (act->type == 3) continue;   //shot
		if (i == 0) {
			if (overlap(x1, y1, x2, y2, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
				thor_damaged(actr);
				return 0;
			}
		} else {
			x3 = act->x;
			if ((ABS(x3 - x1)) > 16) continue;
			y3 = act->y;
			if ((ABS(y3 - y1)) > 16) continue;
			x4 = act->x + act->size_x;
			y4 = act->y + act->size_y;
			if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) {
				if (actr->move == 38) {
					if (act->func_num == 4)
						_G(switch_flag) = 1;
					else if (act->func_num == 7)
						_G(switch_flag) = 2;
				}
				return 0;
			}
		}
	}
	actr->x = x;
	actr->y = y;
	return 1;
}

int check_move3(int x, int y, ACTOR *actr) {   //check enemy shot move
	int x1, x2, y1, y2;
	int x3, x4, y3, y4, i;
	byte icn1, icn2, icn3, icn4;
	ACTOR *act;

	int icn;

	if (x<0 || x>(319 - actr->size_x) || y < 0 || y>175) return 0;

	x1 = (x + 1) >> 4;
	y1 = (y + (actr->size_y / 2)) >> 4;
	x2 = ((x + actr->size_x) - 1) >> 4;
	y2 = ((y + actr->size_y) - 1) >> 4;

	//check for solid or fly over

	icn = TILE_FLY;
	if (actr->flying) icn = TILE_SOLID;

	icn1 = _G(scrn).icon[y1][x1];
	icn2 = _G(scrn).icon[y2][x1];
	icn3 = _G(scrn).icon[y1][x2];
	icn4 = _G(scrn).icon[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn) return 0;

	if (icn1 > TILE_SPECIAL) if (!special_tile(actr, y1, x1, icn1)) return 0;
	if (icn2 > TILE_SPECIAL) if (!special_tile(actr, y2, x1, icn2)) return 0;
	if (icn3 > TILE_SPECIAL) if (!special_tile(actr, y1, x2, icn3)) return 0;
	if (icn4 > TILE_SPECIAL) if (!special_tile(actr, y2, x2, icn4)) return 0;

	//check for solid or fly over
	x1 = x + 1;
	y1 = y + 1;
	x2 = (x + actr->size_x) - 1;
	y2 = (y + actr->size_y) - 1;

	if (overlap(x1, y1, x2, y2, _G(thor_x1), _G(thor_real_y1), _G(thor_x2), _G(thor_y2))) {
		thor_damaged(actr);
		return 0;
	}
	for (i = 3; i < MAX_ACTORS; i++) {   //max_actors
		if (i == actr->actor_num) continue;
		act = &_G(actor)[i];
		if (!act->used) continue;
		if (act->solid < 2) continue;
		if (act->type == 3) continue;   //shot
		if (act->actor_num == actr->creator) continue;
		x3 = act->x;
		if ((ABS(x3 - x1)) > 16) continue;
		y3 = act->y;
		if ((ABS(y3 - y1)) > 16) continue;
		x4 = x3 + 15;
		y4 = y3 + 15;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) return 0;
	}
	actr->x = x;
	actr->y = y;
	return 1;
}

int check_move4(int x, int y, ACTOR *actr) {   //flying enemies

	if (x<0 || x>(319 - actr->size_x) || y < 0 || y>175) return 0;
	if (overlap(x, y, x + actr->size_x - 1, y + actr->size_y - 1,
		_G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
		if (actr->type == 3) thor_damaged(actr);
		return 0;
	}
	actr->x = x;
	actr->y = y;
	return 1;
}

#define THOR_PAD1 2
#define THOR_PAD2 4

int  check_thor_move(int x, int y, ACTOR *actr) {
	if (check_move0(x, y, actr))
		return 1;
	if (_G(diag_flag) || _G(thor_special_flag))
		return 0;
	if ((_G(thor_icon1) + _G(thor_icon2) + _G(thor_icon3) + _G(thor_icon4)) > 1)
		return 0;

	switch (actr->dir) {
	case 0:
		if (_G(thor_icon1)) {
			actr->dir = 3;
			if (check_move0(x + THOR_PAD1, y + 2, actr)) {
				actr->dir = 0;
				return 1;
			}
		} else if (_G(thor_icon3)) {
			actr->dir = 2;
			if (check_move0(x - THOR_PAD1, y + 2, actr)) {
				actr->dir = 0;
				return 1;
			}
		}
		actr->dir = 0;
		break;
	case 1:
		if (_G(thor_icon2)) {
			actr->dir = 3;
			if (check_move0(x + THOR_PAD1, y - 2, actr)) {
				actr->dir = 1;
				return 1;
			}
		} else if (_G(thor_icon4)) {
			actr->dir = 2;
			if (check_move0(x - THOR_PAD1, y - 2, actr)) {
				actr->dir = 1;
				return 1;
			}
		}
		actr->dir = 1;
		break;
	case 2:
		if (_G(thor_icon1)) {
			if (check_move0(x + 2, y + THOR_PAD1, actr)) return 1;
		} else if (_G(thor_icon2)) {
			if (check_move0(x + 2, y - THOR_PAD1, actr)) return 1;
		}
		break;
	case 3:
		if (_G(thor_icon3)) {
			if (check_move0(x - 2, y + THOR_PAD1, actr)) return 1;
		} else if (_G(thor_icon4)) {
			if (check_move0(x - 2, y - THOR_PAD1, actr)) return 1;
		}
		break;
	}

	return 0;
}

int movement_zero(ACTOR *actr) {       //player control
	int d, x, y, od;
	d = actr->dir;
	od = d;

	set_thor_vars();

	if ((_G(hammer)->used) && (_G(hammer)->move == 5)) {
		if (overlap(_G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2),
			_G(hammer)->x, _G(hammer)->y, _G(hammer)->x + 13, _G(hammer)->y + 13)) {
			actor_destroyed(_G(hammer));
		}
	}
	x = actr->x;
	y = actr->y;
	_G(diag_flag) = 0;
	if (actr->move_counter) actr->move_counter--;

	if (_G(slipping)) {
		if (_G(slip_cnt) == 8) play_sound(FALL, 1);
		y += 2;
		_G(slip_cnt)--;
		if (!_G(slip_cnt))
			_G(slipping) = false;

		check_thor_move(x, y, actr);
		_G(thor)->speed_count = 4;
		return d;
	}

	if (_G(key_flag)[key_up] && _G(key_flag)[key_left]) {
		d = 2;
		actr->dir = d;
		_G(diag) = 1;
		_G(diag_flag) = 1;
		if (check_thor_move(x - 2, y - 2, actr)) {
			next_frame(actr);
			return d;
		}
	} else if (_G(key_flag)[key_up] && _G(key_flag)[key_right]) {
		d = 3;
		actr->dir = d;
		_G(diag) = 2;
		_G(diag_flag) = 1;
		if (check_thor_move(x + 2, y - 2, actr)) {
			next_frame(actr);
			return d;
		}
	} else if (_G(key_flag)[key_down] && _G(key_flag)[key_left]) {
		d = 2;
		actr->dir = d;
		_G(diag) = 4;
		_G(diag_flag) = 1;
		if (check_thor_move(x - 2, y + 2, actr)) {
			next_frame(actr);
			return d;
		}
	} else if (_G(key_flag)[key_down] && _G(key_flag)[key_right]) {
		d = 3;
		actr->dir = d;
		_G(diag) = 3;
		_G(diag_flag) = 1;
		if (check_thor_move(x + 2, y + 2, actr)) {
			next_frame(actr);
			return d;
		}
	}
	_G(diag) = 0;
	if (_G(key_flag)[key_right]) {
		if (!_G(key_flag)[key_left]) {
			d = 3;
			actr->dir = d;
			if (check_thor_move(x + 2, y, actr)) {
				next_frame(actr);
				return d;
			}
		}
	}
	if (_G(key_flag)[key_left]) {
		if (!_G(key_flag)[key_right]) {
			d = 2;
			actr->dir = d;
			if (check_thor_move(x - 2, y, actr)) {
				next_frame(actr);
				return d;
			}
		}
	}
	if (_G(key_flag)[key_down]) {
		if (!_G(key_flag)[key_up]) {
			d = 1;
			actr->dir = d;
			if (check_thor_move(x, y + 2, actr)) {
				next_frame(actr);
				return d;
			}
		}
	}
	if (_G(key_flag)[key_up]) {
		if (!_G(key_flag)[key_down]) {
			d = 0;
			actr->dir = d;
			if (check_thor_move(x, y - 2, actr)) {
				next_frame(actr);
				return d;
			}
		}
	}
	actr->move_counter = 5;
	actr->next = 0;
	actr->dir = od;
	return d;
}

int check_special_move1(int x, int y, ACTOR *actr) {
	int x1, x2, y1, y2, i;
	int x3, y3, x4, y4;
	int icn;
	byte icn1, icn2, icn3, icn4;

	ACTOR *act;

	if (actr->actor_num < 3) return check_move1(x, y, actr);

	if (x < 0 || x>304 || y < 0 || y>176) return 0;

	x1 = x >> 4;
	y1 = y >> 4;
	x2 = (x + 15) >> 4;
	y2 = (y + 15) >> 4;

	//check for solid or fly over

	icn = TILE_FLY;
	if (actr->flying) icn = TILE_SOLID;

	icn1 = _G(scrn).icon[y1][x1];
	icn2 = _G(scrn).icon[y2][x1];
	icn3 = _G(scrn).icon[y1][x2];
	icn4 = _G(scrn).icon[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn) return 0;

	if (icn1 > TILE_SPECIAL) if (!special_tile(actr, y1, x1, icn1)) return 0;
	if (icn2 > TILE_SPECIAL) if (!special_tile(actr, y2, x1, icn2)) return 0;
	if (icn3 > TILE_SPECIAL) if (!special_tile(actr, y1, x2, icn3)) return 0;
	if (icn4 > TILE_SPECIAL) if (!special_tile(actr, y2, x2, icn4)) return 0;

	x1 = x;
	y1 = y;
	x2 = (x + 15);
	y2 = (y + 15);

	for (i = 3; i < MAX_ACTORS; i++) {   //max_actors
		act = &_G(actor)[i];
		if (act->actor_num == actr->actor_num) continue;
		if (!act->used) continue;
		if (act->type == 3) continue;   //shot
		x3 = act->x;
		if ((ABS(x3 - x1)) > 16) continue;
		y3 = act->y;
		if ((ABS(y3 - y1)) > 16) continue;
		x4 = act->x + act->size_x;
		y4 = act->y + 15;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) return 0;
	}
	for (i = 3; i < MAX_ACTORS; i++) {   //max_actors
		act = &_G(actor)[i];
		if (act->actor_num == actr->actor_num) continue;
		if (!act->used) continue;
		if (act->type == 3) continue;   //shot
		x3 = act->x;
		if ((ABS(x3 - x1)) > 16) continue;
		y3 = act->y;
		if ((ABS(y3 - y1)) > 16) continue;
		x4 = act->x + act->size_x;
		y4 = act->y + act->size_y;
		if (overlap(_G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2), x3, y3, x4, y4)) return 0;
	}
	actr->x = x;
	actr->y = y;
	return 1;
}

//*==========================================================================

int special_movement_one(ACTOR *actr) {   //block
	int d, x1, y1, sd;

	if (_G(diag_flag)) return 0;
	d = _G(thor)->dir;
	x1 = actr->x;
	y1 = actr->y;
	sd = actr->last_dir;
	actr->last_dir = d;

	switch (d) {
	case 0:
		y1 -= 2;
		if (!check_special_move1(x1, y1, actr)) {
			actr->last_dir = sd;
			return 0;
		}
		break;
	case 1:
		y1 += 2;
		if (!check_special_move1(x1, y1, actr)) {
			actr->last_dir = sd;
			return 0;
		}
		break;
	case 2:
		x1 -= 2;
		if (!check_special_move1(x1, y1, actr)) {
			actr->last_dir = sd;
			return 0;
		}
		break;
	case 3:
		x1 += 2;
		if (!check_special_move1(x1, y1, actr)) {
			actr->last_dir = sd;
			return 0;
		}
		break;
	}
	next_frame(actr);
	actr->last_dir = d;
	return 1;
}

int special_movement_two(ACTOR *actr) {   //angle
	int x1, y1, x2, y2, i;
	int x3, y3, x4, y4;
	ACTOR *act;

	x1 = actr->temp1;       //calc thor pos
	y1 = actr->temp2;
	x2 = x1 + 13;
	y2 = y1 + 14;

	for (i = 3; i < MAX_ACTORS; i++) {   //max_actors
		act = &_G(actor)[i];
		if (actr->actor_num == act->actor_num) continue;
		if (!act->used) continue;
		x3 = act->x;
		if ((ABS(x3 - x1)) > 16) continue;
		y3 = act->y;
		if ((ABS(y3 - y1)) > 16) continue;
		x4 = act->x + act->size_x;
		y4 = act->y + act->size_y;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) return 0;
	}
	if (!actr->func_pass) {
		if (_G(thor)->health < 150) {
			if (!sound_playing()) play_sound(ANGEL, 0);
			_G(thor)->health += 1;
			display_health();
		}
	} else {
		if (_G(thor_info).magic < 150) {
			if (!sound_playing()) play_sound(ANGEL, 0);
			_G(thor_info).magic += 1;
			display_magic();
		}
	}
	return 1;
}

int special_movement_three(ACTOR *actr) {  //yellow globe
	long lind;

	if (_G(thunder_flag)) return 0;

	lind = (long)_G(current_level);
	lind = lind * 1000;
	lind += (long)actr->actor_num;
	execute_script(lind, _G(odin));

	return 0;
}

int special_movement_four(ACTOR *actr) {   //peg switch

	if (actr->shot_cnt != 0) return 0;
	actr->shot_cnt = 30;
	actr = actr;
	_G(switch_flag) = 1;
	return 0;
}
void kill_enemies(int ix, int iy);
void remove_objects(int iy, int ix);

int special_movement_five(ACTOR *actr) {    //boulder roll
	int d;

	d = _G(thor)->dir;

	if (_G(diag_flag)) {
		switch (_G(diag)) {
		case 1:
			if (_G(thor_x1) < (actr->x + 15)) d = 0;
			else d = 2;
			break;
		case 2:
			if (_G(thor_x2) < actr->x) d = 3;
			else d = 0;
			break;
		case 3:
			if (_G(thor_x2) > (actr->x)) d = 1;
			else d = 3;
			break;
		case 4:
			if (_G(thor_x1) > (actr->x + 15)) d = 2;
			else d = 1;
			break;
		}
	}

	actr->last_dir = d;
	actr->move = 14;
	return 0;
}

int special_movement_six(ACTOR *actr) {

	thor_damaged(actr);
	return 0;
}

int special_movement_seven(ACTOR *actr) {

	if (actr->shot_cnt != 0) return 0;
	actr->shot_cnt = 30;
	actr = actr;
	_G(switch_flag) = 2;
	return 0;
}

int special_movement_eight(ACTOR *actr) {

	if (_G(thor)->dir < 2 || _G(diag_flag)) return 0;
	actr->last_dir = _G(thor)->dir;
	actr->move = 14;
	return 0;
}

int special_movement_nine(ACTOR *actr) {

	if (_G(thor)->dir > 1 || _G(diag_flag)) return 0;
	actr->last_dir = _G(thor)->dir;
	actr->move = 14;
	return 0;
}

int special_movement_ten(ACTOR *actr) {

	if (actr->temp6) {
		actr->temp6--;
		return 0;
	}
	if (_G(thunder_flag)) return 0;
	if (!actr->temp6) {
		actr->temp6 = 10;
		actor_speaks(actr, 0 - actr->pass_value, 0);
	}
	return 0;
}

int movement_one(ACTOR *actr) {    //no movement - frame cycle

	next_frame(actr);
	return actr->dir;
}

int movement_two(ACTOR *actr) {     //hammer only
	int d, x1, y1;

	d = actr->last_dir;
	x1 = actr->x;
	y1 = actr->y;

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
		if (actr->actor_num == 1) {
			_G(hammer)->move = 5;
			d = reverse_direction(_G(hammer));
			_G(hammer)->dir = d;
		}
		if (actr->actor_num == 2) {
			actr->used = 0;
			actr->dead = 2;
			_G(lightning_used) = 0;
			_G(tornado_used) = 0;
		}
	}
	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_three(ACTOR *actr) {         //walk-bump-random turn
	int d, x1, y1;

	d = actr->last_dir;
	x1 = actr->x;
	y1 = actr->y;

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
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_four(ACTOR *actr) {         //simple tracking
	int d, x1, y1, f;

	d = actr->last_dir;

	x1 = actr->x;
	y1 = actr->y;

	f = 0;
	if (x1 > _G(thor_x1) - 1) {
		x1 -= 2; d = 2; f = 1;
	} else if (x1 < _G(thor_x1) - 1) {
		x1 += 2; d = 3; f = 1;
	}

	if (f) f = check_move2(x1, y1, actr);

	if (!f) {
		if (y1 < (_G(thor_real_y1))) {
			d = (_G(thor_real_y1))-y1;
			if (d > 2) d = 2;
			y1 += d;
			d = 1;
			f = 1;
		} else if (y1 > (_G(thor_real_y1))) {
			d = y1 - (_G(thor_real_y1));
			if (d > 2) d = 2;
			y1 -= d;
			d = 0;
			f = 1;
		}
		if (f) f = check_move2(actr->x, y1, actr);
		if (!f) check_move2(actr->x, actr->y, actr);
	}
	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_five(ACTOR *actr) {
	int x1, y1, xd, yd, d;

	x1 = actr->x;
	y1 = actr->y;
	xd = yd = 0;
	d = actr->last_dir;

	if (x1 > (_G(thor)->x + 1)) xd = -2;  //+1
	else if (x1 < (_G(thor)->x - 1)) xd = 2;

	if (actr->actor_num == 1) {
		if (y1 < (_G(thor_y1) - 6)) yd = 2;
		else if (y1 > (_G(thor_y1) - 6)) yd = -2;
	} else {
		if (y1 < (_G(thor_real_y1) - 1)) yd = 2;
		else if (y1 > (_G(thor_real_y1) + 1)) yd = -2;
	}

	if (xd && yd) {
		if (xd == -2 && yd == -2) d = 2;
		else if (xd == -2 && yd == 2) d = 2;
		else if (xd == 2 && yd == -2) d = 3;
		else if (xd == 2 && yd == 2) d = 3;
		x1 += xd;
		y1 += yd;
		if (check_move2(x1, y1, actr)) {
			next_frame(actr);
			actr->last_dir = d;
			if (actr->directions == 1) return 0;
			return d;
		}
	} else {
		if (xd == 0 && yd == 2) d = 1;
		else if (xd == 0 && yd == -2) d = 0;
		else if (xd == 2 && yd == 0) d = 3;
		else if (xd == -2 && yd == 0) d = 2;
	}
	x1 = actr->x;
	y1 = actr->y;
	actr->toggle ^= 1;

	if (actr->toggle) {
		if (xd) {
			x1 += xd;
			if (check_move2(x1, y1, actr)) {
				if (xd > 0) d = 3;
				else d = 2;
				next_frame(actr);
				actr->last_dir = d;
				if (actr->directions == 1) return 0;
				return d;
			}
			x1 = actr->x;
		}
		if (yd) {
			y1 += yd;
			if (check_move2(x1, y1, actr)) {
				if (yd > 0) d = 1;
				else d = 0;
				next_frame(actr);
				actr->last_dir = d;
				if (actr->directions == 1) return 0;
				return d;
			}
		}
		y1 = actr->y;
	} else {
		if (yd) {
			y1 += yd;
			if (check_move2(x1, y1, actr)) {
				if (yd > 0) d = 1;
				else d = 0;
				next_frame(actr);
				actr->last_dir = d;
				if (actr->directions == 1) return 0;
				return d;
			}
			y1 = actr->y;
		}
		if (xd) {
			x1 += xd;
			if (check_move2(x1, y1, actr)) {
				if (xd > 0) d = 3;
				else d = 2;
				next_frame(actr);
				actr->last_dir = d;
				if (actr->directions == 1) return 0;
				return d;
			}
		}
	}
	check_move2(actr->x, actr->y, actr);
	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_six(ACTOR *actr) {   //explosion only

	if (actr->num_shots > 0) {
		actr->next++;
		if (actr->next > 2) {
			actr->next = 0;
			if (_G(boss_dead)) play_sound(EXPLODE, 0);
		}
		actr->num_shots--;
	} else {
		actr->dead = 2;
		actr->used = 0;
		if (!_G(boss_dead)) if (actr->type == 2)
			drop_object(actr);

	}
	next_frame(actr);
	return 0;
}

int movement_seven(ACTOR *actr) {         //walk-bump-random turn (pause also)

	if (actr->next == 0 && actr->frame_count == actr->frame_speed) {
		actr->speed_count = 12;
		actr->last_dir = g_events->getRandomNumber(3);
	}
	return movement_three(actr);
}

int movement_eight(ACTOR *actr) {         //follow thor

	if (_G(thor)->x > 0) actr->x = _G(thor)->x - 1;
	else actr->x = _G(thor)->x;
	actr->y = _G(thor)->y;
	next_frame(actr);
	return 0;
}

int movement_nine(ACTOR *actr) {         //4-way straight (random length) change
	int d, x1, y1, f;

	d = actr->last_dir;
	x1 = actr->x;
	y1 = actr->y;

	f = 0;
	if (actr->counter) {
		actr->counter--;
		switch (d) {
		case 0:
			y1 -= 2;
			if (!check_move4(x1, y1, actr)) f = 1;
			break;
		case 1:
			y1 += 2;
			if (!check_move4(x1, y1, actr)) f = 1;
			break;
		case 2:
			x1 -= 2;
			if (!check_move4(x1, y1, actr)) f = 1;
			break;
		case 3:
			x1 += 2;
			if (!check_move4(x1, y1, actr)) f = 1;
			break;
		}
	} else f = 1;

	if (f == 1) {
		actr->counter = g_events->getRandomNumber(10, 99);
		d = g_events->getRandomNumber(3);
	}
	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_ten(ACTOR *actr) {         //vert straight (random length) change
	int d, x1, y1, f;

	d = actr->last_dir;
	x1 = actr->x;
	y1 = actr->y;

	f = 0;
	if (actr->counter) {
		if (actr->pass_value != 1) actr->counter--;
		switch (d) {
		case 0:
			y1 -= 2;
			if (!check_move2(x1, y1, actr)) f = 1;
			break;
		case 1:
			y1 += 2;
			if (!check_move2(x1, y1, actr)) f = 1;
			break;
		case 2:
			y1 -= 2;
			if (!check_move2(x1, y1, actr)) f = 1;
			break;
		case 3:
			y1 += 2;
			if (!check_move2(x1, y1, actr)) f = 1;
			break;
		}
	} else f = 1;

	if (f == 1) {
		actr->counter = g_events->getRandomNumber(10, 99);
		d = g_events->getRandomNumber(1);
	}

	if (d > 1)
		d -= 2;

	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_eleven(ACTOR *actr) {         //horz only (bats)
	int d;

	d = actr->last_dir;

	switch (d) {
	case 0:
		if (check_move2(actr->x - 2, actr->y - 2, actr)) break;
		d = 1;
		if (check_move2(actr->x - 2, actr->y + 2, actr)) break;
		d = 2;
		break;
	case 1:
		if (check_move2(actr->x - 2, actr->y + 2, actr)) break;
		d = 0;
		if (check_move2(actr->x - 2, actr->y - 2, actr)) break;
		d = 3;
		break;
	case 2:
		if (check_move2(actr->x + 2, actr->y - 2, actr)) break;
		d = 3;
		if (check_move2(actr->x + 2, actr->y + 2, actr)) break;
		d = 0;
		break;
	case 3:
		if (check_move2(actr->x + 2, actr->y + 2, actr)) break;
		d = 2;
		if (check_move2(actr->x + 2, actr->y - 2, actr)) break;
		d = 1;
		break;
	}

	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_twelve(ACTOR *actr) {         //horz straight until bump
	int d;

	d = actr->last_dir;

	switch (d) {
	case 0:
	case 2:
		if (check_move2(actr->x - 2, actr->y, actr)) break;
		d = 3;
		break;
	case 1:
	case 3:
		if (check_move2(actr->x + 2, actr->y, actr)) break;
		d = 2;
		break;
	}
	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_thirteen(ACTOR *actr) {         //pause-seek (mushroom)
	int d;

	d = actr->last_dir;

	if (actr->counter == 0 && actr->pause == 0) {
		actr->pause = 60;
		return d;
	}
	if (actr->pause > 0) {
		actr->pause--;
		if (!actr->pause) actr->counter = 60;
		actr->vunerable = 5;
		actr->strength = 0;
		return d;
	}
	if (actr->counter > 0) {
		actr->counter--;
		if (!actr->counter) actr->pause = 60;
		actr->strength = 10;
		return movement_five(actr);
	}
	return d;
}

int movement_fourteen(ACTOR *actr) {         //move-bump-stop (boulder)
	int d, x1, y1;

	d = actr->last_dir;
	actr->dir = d;
	x1 = actr->x;
	y1 = actr->y;

	switch (d) {
	case 0:
		y1 -= 2;
		if (!check_move2(x1, y1, actr)) {
			actr->move = 15;
			return 0;
		}
		break;
	case 1:
		y1 += 2;
		if (!check_move2(x1, y1, actr)) {
			actr->move = 15;
			return 0;
		}
		break;
	case 2:
		x1 -= 2;
		if (!check_move2(x1, y1, actr)) {
			actr->move = 15;
			return 0;
		}
		break;
	case 3:
		x1 += 2;
		if (!check_move2(x1, y1, actr)) {
			actr->move = 15;
			return 0;
		}
		break;
	}
	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_fifteen(ACTOR *actr) {    //no movement - no frame cycle

	return actr->dir;
}

int movement_sixteen(ACTOR *actr) {     //tornado 1
	int d, x1, y1;

	d = actr->last_dir;
	x1 = actr->x;
	y1 = actr->y;

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
		actr->move = 17;
		d = g_events->getRandomNumber(3);
	}

	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_seventeen(ACTOR *actr) {         //tornado 2
	int d;

	d = actr->last_dir;

	switch (d) {
	case 0:
		if (check_move1(actr->x - 2, actr->y - 2, actr)) break;
		d = 1;
		if (check_move1(actr->x - 2, actr->y + 2, actr)) break;
		d = 2;
		break;
	case 1:
		if (check_move1(actr->x - 2, actr->y + 2, actr)) break;
		d = 0;
		if (check_move1(actr->x - 2, actr->y - 2, actr)) break;
		d = 3;
		break;
	case 2:
		if (check_move1(actr->x + 2, actr->y - 2, actr)) break;
		d = 3;
		if (check_move1(actr->x + 2, actr->y + 2, actr)) break;
		d = 0;
		break;
	case 3:
		if (check_move1(actr->x + 2, actr->y + 2, actr)) break;
		d = 2;
		if (check_move1(actr->x + 2, actr->y - 2, actr)) break;
		d = 1;
		break;
	}
	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_eighteen(ACTOR *actr) {    //no movement - frame cycle
	int d;

	if (actr->temp5) {
		actr->temp5--;
		if (!actr->temp5) actr->num_moves = 1;
	}
	if (actr->temp1) {
		d = movement_five(actr);
		actr->rand--;
		if (actr->hit_thor || !actr->rand) {
			if (actr->hit_thor) {
				actr->temp5 = 50;
				actr->num_moves = 2;
				actr->hit_thor = 0;
				actr->dir = d;
				d = reverse_direction(actr);
			}
			actr->temp1 = 0;
			actr->rand = g_events->getRandomNumber(50, 149);
		}
	} else {
		d = movement_three(actr);
		actr->rand--;
		if (!actr->rand) {
			actr->temp5 = 0;
			actr->temp1 = 1;
			actr->rand = g_events->getRandomNumber(50, 149);
		}
		if (actr->hit_thor) {
			actr->temp5 = 50;
			actr->num_moves = 2;
			actr->hit_thor = 0;
			actr->dir = d;
			d = reverse_direction(actr);
		}
	}
	next_frame(actr);
	return d;
}

int movement_nineteen(ACTOR *actr) {    //no movement - frame cycle

	return movement_seven(actr);
}

int movement_twenty(ACTOR *actr) {    //boss - snake

	return boss1_movement(actr);
}

int movement_twentyone(ACTOR *actr) {    //no movement - frame cycle

	return movement_three(actr);
}

int movement_twentytwo(ACTOR *actr) {    //spear
	int d;

	d = actr->last_dir;
	if (actr->directions == 1) d = 0;

redo:

	switch (actr->temp2) {
	case 0:
		if (bgtile(actr->x, actr->y) >= TILE_SOLID) actr->next = 1;
		else {
			actr->temp2 = 6;
			actr->temp1 = 1;
			goto redo;
		}
		actr->temp2++;
		break;
	case 1:
		actr->next = 2;
		actr->temp2++;
		break;
	case 2:
		actr->next = 3;
		actr->strength = 255;
		actr->temp2++;
		actr->temp1 = 10;
		break;
	case 3:
		check_move2(actr->x, actr->y, actr);
		actr->temp1--;
		if (actr->temp1) break;
		actr->temp2++;
		actr->next = 2;
		break;
	case 4:
		actr->strength = 0;
		actr->temp2++;
		actr->next = 1;
		break;
	case 5:
		actr->temp2++;
		actr->next = 0;
		actr->temp1 = 10;
		break;
	case 6:
		actr->temp1--;
		if (actr->temp1) break;
		actr->temp2 = 0;
		actr->next = 0;
		switch (d) {
		case 0:
			actr->x += 16;
			actr->y += 16;
			d = 3;
			if (bgtile(actr->x, actr->y) < TILE_SOLID) goto redo;
			break;
		case 1:
			actr->x -= 16;
			actr->y -= 16;
			d = 2;
			if (bgtile(actr->x, actr->y) < TILE_SOLID) goto redo;
			break;
		case 2:
			actr->x += 16;
			actr->y -= 16;
			d = 0;
			if (bgtile(actr->x, actr->y) < TILE_SOLID) goto redo;
			break;
		case 3:
			actr->x -= 16;
			actr->y += 16;
			d = 1;
			if (bgtile(actr->x, actr->y) < TILE_SOLID) goto redo;
			break;
		}
		actr->dir = d;
		actr->last_dir = d;
		break;
	}
	return d;
}

int movement_twentythree(ACTOR *actr) {    //spinball counter-clockwise
	int d;

	d = actr->last_dir;
	next_frame(actr);
	if (actr->pass_value & 2) actr->num_moves = 2;

	switch (d) {
	case 0:
		if (bgtile(actr->x - 2, actr->y) >= TILE_FLY &&
			bgtile(actr->x - 2, actr->y + actr->size_y - 1) >= TILE_FLY) {
			d = 2;
			actr->x -= 2;
		} else {
			if (bgtile(actr->x, actr->y - 2) < TILE_FLY ||
				bgtile(actr->x + actr->size_x - 1, actr->y - 2) < TILE_FLY) {
				if (bgtile(actr->x + actr->size_x + 1, actr->y) >= TILE_FLY &&
					bgtile(actr->x + actr->size_x + 1, actr->y + actr->size_y - 1) >= TILE_FLY) {
					d = 3;
					actr->x += 2;
				} else {
					d = 1;
					break;
				}
			} else actr->y -= 2;
		}
		break;
	case 1:
		if (bgtile(actr->x + actr->size_x + 1, actr->y) >= TILE_FLY &&
			bgtile(actr->x + actr->size_x + 1, actr->y + actr->size_y - 1) >= TILE_FLY) {
			d = 3;
			actr->x += 2;
		} else {
			if (bgtile(actr->x, actr->y + actr->size_y + 1) < TILE_FLY ||
				bgtile(actr->x + actr->size_x - 1, actr->y + actr->size_y + 1) < TILE_FLY) {
				if (bgtile(actr->x - 2, actr->y) >= TILE_FLY &&
					bgtile(actr->x - 2, actr->y + actr->size_y - 1) >= TILE_FLY) {
					d = 2;
					actr->x -= 2;
				} else {
					d = 0;
					break;
				}
			} else actr->y += 2;
		}
		break;
	case 2:
		if (bgtile(actr->x, actr->y + actr->size_y + 1) >= TILE_FLY &&
			bgtile(actr->x + actr->size_x - 1, actr->y + actr->size_y + 1) >= TILE_FLY) {
			d = 1;
			actr->y += 2;
		} else {
			if (bgtile(actr->x - 2, actr->y) < TILE_FLY ||
				bgtile(actr->x - 2, actr->y + actr->size_y - 1) < TILE_FLY) {
				if (bgtile(actr->x, actr->y - 2) >= TILE_FLY &&
					bgtile(actr->x + actr->size_x - 1, actr->y - 2) >= TILE_FLY) {
					d = 0;
					actr->y -= 2;
				} else {
					d = 3;
					break;
				}
			} else actr->x -= 2;
		}
		break;
	case 3:
		if (bgtile(actr->x, actr->y - 2) >= TILE_FLY &&
			bgtile(actr->x + actr->size_x - 1, actr->y - 2) >= TILE_FLY) {
			d = 0;
			actr->y -= 2;
		} else {
			if (bgtile(actr->x + actr->size_x + 1, actr->y) < TILE_FLY ||
				bgtile(actr->x + actr->size_x + 1, actr->y + actr->size_y - 1) < TILE_FLY) {
				if (bgtile(actr->x, actr->y + actr->size_y + 1) >= TILE_FLY &&
					bgtile(actr->x + actr->size_x - 1, actr->y + actr->size_y + 1) >= TILE_FLY) {
					d = 1;
					actr->y += 2;
				} else {
					d = 2;
					break;
				}
			} else actr->x += 2;
		}
		break;
	}
	check_move2(actr->x, actr->y, actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_twentyfour(ACTOR *actr) {    //spinball  clockwise
	int d;

	d = actr->last_dir;
	next_frame(actr);
	if (actr->pass_value & 2) actr->num_moves = 2;

	switch (d) {
	case 0:
		if (bgtile(actr->x + actr->size_x + 1, actr->y) >= TILE_FLY &&
			bgtile(actr->x + actr->size_x + 1, actr->y + actr->size_y - 1) >= TILE_FLY) {
			d = 3;
			actr->x += 2;
		} else {
			if (bgtile(actr->x, actr->y - 2) < TILE_FLY ||
				bgtile(actr->x + actr->size_x - 1, actr->y - 2) < TILE_FLY) {
				if (bgtile(actr->x - 2, actr->y) >= TILE_FLY &&
					bgtile(actr->x - 2, actr->y + actr->size_y - 1) >= TILE_FLY) {
					d = 2;
					actr->x -= 2;
				} else {
					d = 1;
					break;
				}
			} else actr->y -= 2;
		}
		break;
	case 1:
		if (bgtile(actr->x - 2, actr->y) >= TILE_FLY &&
			bgtile(actr->x - 2, actr->y + actr->size_y - 1) >= TILE_FLY) {
			d = 2;
			actr->x -= 2;
		} else {
			if (bgtile(actr->x, actr->y + actr->size_y + 1) < TILE_FLY ||
				bgtile(actr->x + actr->size_x - 1, actr->y + actr->size_y + 1) < TILE_FLY) {
				if (bgtile(actr->x + actr->size_x + 1, actr->y) >= TILE_FLY &&
					bgtile(actr->x + actr->size_x + 1, actr->y + actr->size_y - 1) >= TILE_FLY) {
					d = 3;
					actr->x += 2;
				} else {
					d = 0;
					break;
				}
			} else actr->y += 2;
		}
		break;
	case 2:
		if (bgtile(actr->x, actr->y - 2) >= TILE_FLY &&
			bgtile(actr->x + actr->size_x - 1, actr->y - 2) >= TILE_FLY) {
			d = 0;
			actr->y -= 2;
		} else {
			if (bgtile(actr->x - 2, actr->y) < TILE_FLY ||
				bgtile(actr->x - 2, actr->y + actr->size_y - 1) < TILE_FLY) {
				if (bgtile(actr->x, actr->y + actr->size_y + 1) >= TILE_FLY &&
					bgtile(actr->x + actr->size_x - 1, actr->y + actr->size_y + 1) >= TILE_FLY) {
					d = 1;
					actr->y += 2;
				} else {
					d = 3;
					break;
				}
			} else actr->x -= 2;
		}
		break;
	case 3:
		if (bgtile(actr->x, actr->y + actr->size_y + 1) >= TILE_FLY &&
			bgtile(actr->x + actr->size_x - 1, actr->y + actr->size_y + 1) >= TILE_FLY) {
			d = 1;
			actr->y += 2;
		} else {
			if (bgtile(actr->x + actr->size_x + 1, actr->y) < TILE_FLY ||
				bgtile(actr->x + actr->size_x + 1, actr->y + actr->size_y - 1) < TILE_FLY) {
				if (bgtile(actr->x, actr->y - 2) >= TILE_FLY &&
					bgtile(actr->x + actr->size_x - 1, actr->y - 2) >= TILE_FLY) {
					d = 0;
					actr->y -= 2;
				} else {
					d = 2;
					break;
				}
			} else actr->x += 2;
		}
		break;
	}
	check_move2(actr->x, actr->y, actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_twentyfive(ACTOR *actr) {         //acid puddle
	int i, ret;

	if (actr->temp2) {
		actr->temp2--;
		return movement_one(actr);
	}
	if (!actr->temp1) {
		//  actr->temp2=12;
		actr->last_dir = g_events->getRandomNumber(3);
		i = 4;
		while (i--) {
			ret = movement_three(actr);
			if (ret) return ret;
			actr->last_dir++;
			if (actr->last_dir > 3) actr->last_dir = 0;
		}
		actr->temp1 = 16;
	}
	actr->temp1--;
	return movement_three(actr);
}

int movement_twentysix(ACTOR *actr) {         //

	return movement_one(actr);
}

int movement_twentyseven(ACTOR *actr) {         //

	return movement_one(actr);
}

void set_thor_vars() {
	_G(thor_x1) = _G(thor)->x + 1;
	_G(thor_y1) = _G(thor)->y + 8;
	_G(thor_real_y1) = _G(thor)->y;
	if (_G(thor)->dir > 1)
		_G(thor_x2) = (_G(thor)->x + 12);
	else
		_G(thor_x2) = (_G(thor)->x + 12);

	_G(thor_y2) = _G(thor)->y + 15;
}

int movement_twentyeight(ACTOR *actr) {         //fish
	int d, x1, y1, ret;

	d = actr->last_dir;
	x1 = actr->x;
	y1 = actr->y;

	if (actr->i1) actr->i1--;
	else {
		if (!actr->temp3) {
			if (!actr->next) {
				actr->frame_count = 1;
				actr->frame_speed = 4;
			}

			next_frame(actr);
			if (actr->next == 3) {
				if (actr->num_shots < actr->shots_allowed)
					actor_shoots(actr, 0);
				actr->temp3 = 1;
			}
		} else {
			actr->frame_count--;
			if (actr->frame_count <= 0) {
				actr->next--;
				actr->frame_count = actr->frame_speed;
				if (!actr->next) {
					actr->temp3 = 0;
					actr->frame_speed = 4;
					actr->i1 = g_events->getRandomNumber(60, 159);
				}
				//      else actr->solid=1;
			}
		}
		goto done;
	}
	switch (actr->temp2) {
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
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113) goto chg_dir;
	ret = bgtile((x1 + actr->size_x) - 1, y1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113) goto chg_dir;
	ret = bgtile(x1, (y1 + actr->size_y) - 1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113) goto chg_dir;
	ret = bgtile((x1 + actr->size_x) - 1, (y1 + actr->size_y) - 1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113) goto chg_dir;

	actr->x = x1;
	actr->y = y1;

	goto done;

chg_dir:
	actr->temp2 = _G(rand1) % 4;

done:
	if (actr->next) {
		x1 = actr->x;
		y1 = actr->y;
		actr->solid = 1;
		check_move2(x1, y1, actr);
		actr->x = x1;
		actr->y = y1;
	} else actr->solid = 2;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_twentynine(ACTOR *actr) {  //horz or vert (pass_val)
	if (!actr->pass_value) return movement_thirty(actr);
	else return movement_twelve(actr);
}

int movement_thirty(ACTOR *actr) {         //vert straight
	int d, x1, y1;

	d = actr->last_dir;
	x1 = actr->x;
	y1 = actr->y;

	switch (d) {
	case 0:
		y1 -= 2;
		break;
	case 1:
		y1 += 2;
		break;
	}
	if (!check_move2(x1, y1, actr)) d ^= 1;

	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_thirtyone(ACTOR *actr) {         //drop (stalagtite)
	int d, x1, y1, cx, cy, ty, i;

	d = actr->last_dir;
	x1 = actr->x;
	y1 = actr->y;

	if (actr->temp1) {
		y1 += 2;
		if (!check_move2(x1, y1, actr)) actor_destroyed(actr);
	} else {
		if (_G(thor_y1) > y1) {
			if (ABS(x1 - _G(thor_x1)) < 16) {
				cx = (actr->x + (actr->size_x / 2)) >> 4;
				cy = ((actr->y + actr->size_y) - 2) >> 4;
				ty = _G(thor)->center_y;
				for (i = cy; i <= ty; i++) if (_G(scrn).icon[i][cx] < TILE_SOLID) goto done;
				actr->num_moves = actr->pass_value + 1;
				actr->temp1 = 1;
			}
		}
	}

done:
	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_thirtytwo(ACTOR *actr) {      //unused

	actr = actr;
	return 0;
}

int movement_thirtythree(ACTOR *actr) {    //unused

	actr = actr;
	return 0;
}

int movement_thirtyfour(ACTOR *actr) {    //unused

	actr = actr;
	return 0;
}

int movement_thirtyfive(ACTOR *actr) {    //gun (single)

	actr->next = actr->last_dir;
	return actr->dir;
}

int movement_thirtysix(ACTOR *actr) {    //acid drop

	actr->speed = actr->pass_value;
	next_frame(actr);
	if (actr->next == 0 && actr->frame_count == actr->frame_speed) {
		actor_always_shoots(actr, 1);
		_G(actor)[actr->shot_actor].x -= 2;
	}
	return 0;
}

//4-way straight (random length) change
int movement_thirtyseven(ACTOR *actr) {
	int d, x1, y1, f;

	d = actr->last_dir;
	x1 = actr->x;
	y1 = actr->y;

	f = 0;
	if (actr->counter) {
		actr->counter--;
		switch (d) {
		case 0:
			y1 -= 2;
			if (!check_move2(x1, y1, actr)) f = 1;
			break;
		case 1:
			y1 += 2;
			if (!check_move2(x1, y1, actr)) f = 1;
			break;
		case 2:
			x1 -= 2;
			if (!check_move2(x1, y1, actr)) f = 1;
			break;
		case 3:
			x1 += 2;
			if (!check_move2(x1, y1, actr)) f = 1;
			break;
		}
	} else f = 1;

	if (f == 1) {
		actr->counter = g_events->getRandomNumber(10, 99);
		d = g_events->getRandomNumber(3);
	}
	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

//timed darting
#define TIMER     actr->i1
#define INIT_DIR  actr->temp1
#define OTHER_DIR actr->temp2
#define FLAG      actr->temp3
#define OX        actr->i2
#define OY        actr->i3
#define CNT       actr->i4
int movement_thirtyeight(ACTOR *actr) {
	int d, x1, y1;

	d = actr->last_dir;
	x1 = actr->x;
	y1 = actr->y;


	if (!FLAG) {
		FLAG = 1;
		if (actr->pass_value) TIMER = actr->pass_value * 15;
		else TIMER = g_events->getRandomNumber(5, 364);
		INIT_DIR = actr->last_dir;
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
		TIMER--; goto done;
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
				actr->last_dir = d;
				FLAG = 2;
			} else {
				actr->next = 0;
				FLAG = 0;
				goto done;
			}
		} else CNT++;
		break;
	case 2:
		check_move2(x1, y1, actr);
		if (x1 == OX && y1 == OY) {
			FLAG = 0;
			d = INIT_DIR;
			actr->last_dir = d;
			actr->next = 0;
			goto done;
		}
	}
	next_frame(actr);

done:
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

int movement_thirtynine(ACTOR *actr) {    //troll 1

	if (_G(setup).skill == 0) {
		actr->speed = 3; actr->num_moves = 1;
	} else if (_G(setup).skill == 1) {
		actr->speed = 2; actr->num_moves = 1;
	} else if (_G(setup).skill == 2) {
		actr->speed = 1; actr->num_moves = 1;
	}


	if (actr->pass_value < 5) return movement_forty(actr);
	if (actr->pass_value == 10) {
		if (overlap(actr->x + 1, actr->y + 1, actr->x + actr->size_x - 1,
			actr->y + actr->size_y - 1, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
			actr->strength = 255;
			thor_damaged(actr);
		}
		return actr->dir;
	}
	if (actr->actor_num != 3) return actr->dir;

	if (actr->i1) {
		actr->i1--;
		actr->x -= 2;
		check_move2(actr->x, actr->y, actr);
		_G(actor)[4].x -= 2;
		_G(actor)[5].x -= 2;
		_G(actor)[6].x -= 2;
	}
	next_frame(actr);
	if (actr->next == 3) actr->next = 0;
	_G(actor)[4].next = _G(actor)[3].next;
	_G(actor)[5].next = 0;
	_G(actor)[6].next = 0;
	return actr->dir;
}

int movement_forty(ACTOR *actr) {    //troll 2
	int x1, x2, a, d, f;

	if (overlap(actr->x + 1, actr->y + 1, actr->x + actr->size_x + 3,
		actr->y + actr->size_y - 1, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
		actr->strength = 150;
		thor_damaged(actr);
	}
	a = 5 + (actr->pass_value * 4);
	x1 = actr->x;
	x2 = _G(actor)[a + 1].x;
	d = actr->last_dir;

	if (actr->last_dir == 2) {
		f = 1;
		if (bgtile(x1 - 2, actr->y) >= TILE_SOLID) {
			_G(actor)[a].x -= 2;
			_G(actor)[a - 1].x -= 2;
			_G(actor)[a - 2].x -= 2;
			_G(actor)[a + 1].x -= 2;
		} else f = 0;
		if (!f)
			d = 3;
	} else {
		f = 1;
		if (bgtile(_G(actor)[a + 1].x + 14, _G(actor)[a + 1].y) >= TILE_SOLID) {
			_G(actor)[a].x += 2;
			_G(actor)[a - 1].x += 2;
			_G(actor)[a - 2].x += 2;
			_G(actor)[a + 1].x += 2;
		} else f = 0;
		if (!f) d = 2;
	}
	if (!f) {
		actr->x = x1;
		_G(actor)[a + 1].x = x2;
	}

	if (actr->next == 3 && !actr->num_shots && actr->frame_count == actr->frame_speed) {
		actor_always_shoots(actr, 1);
		_G(actor)[actr->shot_actor].x += 6;
	}

	next_frame(actr);
	_G(actor)[a - 2].next = actr->next;
	_G(actor)[a - 1].next = actr->next;
	_G(actor)[a + 1].next = actr->next;
	_G(actor)[a - 2].last_dir = d;
	_G(actor)[a - 1].last_dir = d;
	_G(actor)[a + 1].last_dir = d;
	actr->last_dir = d;
	if (actr->directions == 1) return 0;
	return d;
}

} // namespace Got
