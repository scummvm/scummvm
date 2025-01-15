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

#include "got/game/boss1.h"
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/init.h"
#include "got/game/move.h"
#include "got/game/move_patterns.h"
#include "got/game/status.h"
#include "got/gfx/image.h"
#include "got/sound.h"
#include "got/vars.h"

namespace Got {

static int boss1_dead();

int boss1_movement(ACTOR *actr) {
	int x1, y1, f;

	if (_G(boss_dead))
		return boss1_dead();

	int d = actr->last_dir;
	if (actr->edge_counter) {
		actr->edge_counter--;
		goto done;
	}

	x1 = actr->x;
	y1 = actr->y;

	if (overlap(actr->x + 2, actr->y + 8, actr->x + 30, actr->y + 30,
				_G(thor)->x, _G(thor)->y + 8, _G(thor)->x + 15, _G(thor)->y + 15)) {
		thor_damaged(actr);
	}

	if (actr->temp3) { //start striking
		actr->temp3--;
		if (!actr->temp3)
			play_sound(BOSS11, false);

		if (_G(hourglass_flag))
			actr->num_moves = 3;
		else
			actr->num_moves = 6;

		goto done0;
	}

	// Strike
	if (actr->temp1) {
		actr->temp1--;
		if (actr->x < (_G(thor_x1) + 12))
			actr->temp1 = 0;
		actr->temp2 = 1;
		d = 2;
		actr->x -= 2;

		if (overlap(actr->x + 2, actr->y + 8, actr->x + 32, actr->y + 30,
					_G(thor)->x, _G(thor)->y + 8, _G(thor)->x + 15, _G(thor)->y + 15)) {
			actr->temp1 = 0;
			goto done1;
		}

		actr->next = 3;
		actr->num_moves = _G(setup).skill + 2;
		goto done1;
	}

	if (actr->temp2) { // Retreating
		if (actr->x < 256) {
			d = 3;
			actr->x += 2;
			if (overlap(actr->x + 2, actr->y + 8, actr->x + 32, actr->y + 30,
						_G(thor)->x, _G(thor)->y + 8, _G(thor)->x + 15, _G(thor)->y + 15)) {
				goto done1;
			}
			actr->num_moves = _G(setup).skill + 1;
			goto done0;
		}

		actr->temp2 = 0;
	}

	if (actr->x > _G(thor_x1) && ABS((_G(thor_y1)) - (actr->y + 20)) < 8) {
		actr->temp3 = 75;
		actr->temp1 = 130;
		actr->temp2 = 0;
	}
	
	f = 0;
	if (actr->counter) {
		actr->counter--;
		switch (d) {
		case 1:
		case 3:
			x1 = _G(actor[5]).x;
			y1 = _G(actor[5]).y;
			y1 += 2;

			if (!check_move2(x1, y1, &_G(actor[5])))
				f = 1;
			else {
				actr->x = _G(actor[5]).x;
				actr->y = _G(actor[5]).y - 16;
			}
			break;
		case 0:
		case 2:
			y1 -= 2;
			if (!check_move2(x1, y1, actr))
				f = 1;
			break;

		default:
			break;
		}
	} else
		f = 1;

	if (f == 1) {
		actr->counter = g_events->getRandomNumber(10, 99);
		d = g_events->getRandomNumber(1);
		actr->edge_counter = 20;
	}

done:
	if (d > 1)
		d -= 2;

done0:
	next_frame(actr);
	if (actr->next == 3)
		actr->next = 0;

done1:
	actr->last_dir = d;

	_G(actor[4]).next = actr->next;
	_G(actor[5]).next = actr->next;
	_G(actor[6]).next = actr->next;

	_G(actor[4]).last_dir = d;
	_G(actor[5]).last_dir = d;
	_G(actor[6]).last_dir = d;

	_G(actor[4]).x = actr->x + 16;
	_G(actor[4]).y = actr->y;
	_G(actor[5]).x = actr->x;
	_G(actor[5]).y = actr->y + 16;
	_G(actor[6]).x = actr->x + 16;
	_G(actor[6]).y = actr->y + 16;
	_G(actor[4]).num_moves = actr->num_moves;
	_G(actor[5]).num_moves = actr->num_moves;
	_G(actor[6]).num_moves = actr->num_moves;

	if (actr->directions == 1)
		return 0;
	return d;
}

void check_boss1_hit(ACTOR *actr, int x1, int y1, int x2, int y2, int act_num) {
	if (actr->move == 15 && act_num == 4) {
		if ((!_G(actor[3]).vunerable) && (_G(actor[3]).next != 3) &&
			overlap(x1, y1, x2, y2, actr->x + 6, actr->y + 4, actr->x + 14, actr->y + 20)) {
			actor_damaged(&_G(actor[3]), _G(hammer)->strength);
			if (_G(cheat) && _G(key_flag[_Z]))
				_G(actor[3]).health = 0;
			else
				_G(actor[3]).health -= 10;

			_G(actor[3]).speed_count = 50;
			_G(actor[3]).vunerable = 100;
			play_sound(BOSS13, true);
			_G(actor[3]).next = 1;

			for (int rep = 4; rep < 7; rep++) {
				_G(actor[rep]).next = 1;
				_G(actor[rep]).speed_count = 50;
			}

			if (_G(actor[3]).health == 0)
				_G(boss_dead) = true;
		}
	}
}

void boss_level1() {
	setup_boss(1);
	_G(boss_active) = true;
	music_pause();
	play_sound(BOSS11, true);
	g_events->send("Game", GameMessage("PAUSE", 40));
	music_play(5, true);
}

static int boss1_dead() {
	_G(hourglass_flag) = 0;
	if (_G(boss_dead)) {
		int rep;

		for (rep = 0; rep < 4; rep++) {
			int x1 = _G(actor[3 + rep]).last_x[_G(pge)];
			int y1 = _G(actor[3 + rep]).last_y[_G(pge)];
			int x = _G(actor[3 + rep]).x;
			int y = _G(actor[3 + rep]).y;
			int n = _G(actor[3 + rep]).actor_num;
			int r = _G(actor[3 + rep]).rating;

			_G(actor[3 + rep]) = _G(explosion);
			_G(actor[3 + rep]).actor_num = n;
			_G(actor[3 + rep]).rating = r;
			_G(actor[3 + rep]).x = x;
			_G(actor[3 + rep]).y = y;
			_G(actor[3 + rep]).last_x[_G(pge)] = x1;
			_G(actor[3 + rep]).last_x[_G(pge) ^ 1] = x;
			_G(actor[3 + rep]).last_y[_G(pge)] = y1;
			_G(actor[3 + rep]).last_y[_G(pge) ^ 1] = y;
			_G(actor[3 + rep]).used = 1;
			_G(actor[3 + rep]).vunerable = 255;
			_G(actor[3 + rep]).move = 6;
			_G(actor[3 + rep]).next = rep;
			_G(actor[3 + rep]).speed = g_events->getRandomNumber(6, 8);
			_G(actor[3 + rep]).num_shots = (10 - _G(actor[3 + rep]).speed) * 10;
			_G(actor[3 + rep]).speed_count = _G(actor[3 + rep]).speed;
		}
		play_sound(EXPLODE, true);
		_G(boss_dead) = true;

		for (rep = 7; rep < MAX_ACTORS; rep++) {
			if (_G(actor[rep]).used)
				actor_destroyed(&_G(actor[rep]));
		}
	}

	return _G(actor[3]).last_dir;
}

void closing_sequence1() {
	_G(game_over) = true;
	music_play(4, true);
	odin_speaks(1001, 13, "CLOSING");
}

void closing_sequence1_2() {
	_G(thor_info).armor = 1;
	load_new_thor();
	_G(thor)->dir = 1;
	_G(thor)->next = 0;
	fill_score(20, "CLOSING");
}

void closing_sequence1_3() {
	fill_health();
	fill_magic();
	odin_speaks(1002, 0, "CLOSING");
}

void closing_sequence1_4() {
	for (int rep = 0; rep < 16; rep++)
		_G(scrn).actor_type[rep] = 0;

	_G(boss_dead) = false;
	_G(setup).boss_dead[0] = 1;
	_G(boss_active) = false;
	_G(scrn).type = 4;
	show_level(BOSS_LEVEL1);

	play_sound(ANGEL, true);
	place_tile(18, 6, 148);
	place_tile(19, 6, 202);
	actor_visible(1);
	actor_visible(2);

	LEVEL lvl;
	lvl.load(59);
	lvl.icon[6][18] = 148;
	lvl.icon[6][19] = 202;
	lvl.save(59);
}

} // namespace Got
