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

#include "got/game/boss3.h"
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/init.h"
#include "got/game/move.h"
#include "got/game/move_patterns.h"
#include "got/game/status.h"
#include "got/sound.h"
#include "got/vars.h"

namespace Got {

#define LFC 10

static int boss_mode;
static int num_pods1;
static byte pod_speed;
static const byte EXPLOSION[4][8] = {
	{126, 127, 128, 129, 130, 131, 132, 133},
	{146, 147, 148, 149, 150, 151, 152, 153},
	{166, 167, 168, 169, 170, 171, 172, 173},
	{186, 187, 188, 189, 190, 191, 192, 193}};

static bool expf[4][8];
static byte expcnt;

static int boss_die();
static void check_boss_hit();
static void boss_change_mode();

static void set_boss(ACTOR *actr) {
	_G(actor[4]).next = actr->next;
	_G(actor[5]).next = actr->next;
	_G(actor[6]).next = actr->next;

	_G(actor[4]).last_dir = actr->dir;
	_G(actor[5]).last_dir = actr->dir;
	_G(actor[6]).last_dir = actr->dir;
	_G(actor[4]).dir = actr->dir;
	_G(actor[5]).dir = actr->dir;
	_G(actor[6]).dir = actr->dir;

	_G(actor[4]).x = actr->x + 16;
	_G(actor[4]).y = actr->y;
	_G(actor[5]).x = actr->x;
	_G(actor[5]).y = actr->y + 16;
	_G(actor[6]).x = actr->x + 16;
	_G(actor[6]).y = actr->y + 16;
}

// Boss - Loki-2
static int boss_movement_one(ACTOR *actr) {
	int rx, ry, i, numPods = 0;
	int fcount;
	
	actr->num_moves = 2;
	pod_speed = 2;

	switch (_G(setup).skill) {
	case 0:
		numPods = 3;
		break;
	case 1:
		numPods = 5;
		break;
	case 2:
		numPods = 8;
		break;
	}

	if (!actr->temp1) {
		// Disappear
		actr->dir = 1;
		actr->frame_count = LFC;
		actr->next = 0;
		actr->temp1 = 1;
		actr->i6 = 1;
		actr->solid |= 128;
		_G(actor[4]).solid |= 128;
		_G(actor[5]).solid |= 128;
		_G(actor[6]).solid |= 128;
		play_sound(EXPLODE, true);
		goto done;
	}
	if (actr->i6) {
		// Fade out
		fcount = actr->frame_count - 1;
		if (fcount <= 0) {
			actr->next++;
			if (actr->next > 2) {
				actr->i6 = 0;
				actr->temp3 = 160;
			}
			actr->frame_count = 3;
		} else
			actr->frame_count = fcount;
		
		goto done1;
	}
	if (actr->temp3 > 1) {
		actr->temp3--;
		goto done1;
	}

	if (actr->temp3) {
		for (i = 0; i < num_pods1; i++)
			if (_G(actor[19 + i]).used)
				goto done1;

		while (1) {
			rx = g_events->getRandomNumber(255) + 16;
			ry = g_events->getRandomNumber(143);
			if (!overlap(rx, ry, rx + 32, ry + 32, _G(thor_x1), _G(thor_y1),
						 _G(thor_x2), _G(thor_y2)))
				break;
		}

		actr->x = rx;
		actr->y = ry;
		actr->frame_count = LFC;
		actr->temp4 = 40;
		actr->temp3 = 0;
		play_sound(EXPLODE, true);
		goto done1;
	}

	if (actr->temp4) {
		// Fade in
		fcount = actr->frame_count - 1;
		if (fcount <= 0) {
			actr->next--;
			if (actr->next > 254) {
				actr->next = 0;
				actr->dir = 0;
				actr->temp4 = 0;
				actr->temp5 = 80;
				actr->solid &= 0x7f;
				_G(actor[4]).solid &= 0x7f;
				_G(actor[5]).solid &= 0x7f;
				_G(actor[6]).solid &= 0x7f;
			}

			actr->frame_count = 3;
		} else
			actr->frame_count = fcount;
		
		goto done1;
	}

	if (actr->temp5) {
		// Shoot
		actr->temp5--;
		if (actr->temp5 == 20) {
			actr->next = 3;
			goto done1;
		}
		
		if (!actr->temp5) {
			if (_G(actor[4]).num_shots < _G(actor[4]).shots_allowed) {
				actor_always_shoots(&_G(actor[4]), 0);
				const byte shot_actor = _G(actor[4]).shot_actor;
				_G(actor[shot_actor]).num_moves = pod_speed;
				_G(actor[shot_actor]).x = actr->x + 8;
				_G(actor[shot_actor]).y = actr->y + 16;
				_G(actor[shot_actor]).temp5 = 0;
				for (i = 0; i < numPods; i++)
					_G(actor[20 + i]) = _G(actor[19]);

				num_pods1 = numPods;
				actr->temp1 = 0;
			}
		}

		if (actr->temp5 < 31)
			goto done1;
	}

done:
	fcount = actr->frame_count - 1;
	if (fcount <= 0) {
		actr->next++;
		if (actr->next > 2)
			actr->next = 0;
		actr->frame_count = LFC;
	} else
		actr->frame_count = fcount;
	
done1:
	set_boss(actr);
	return actr->dir;
}

// Boss - Loki-1
int boss3_movement(ACTOR *actr) {
	int x1, y1, ox, oy;
	int fcount;


	if (actr->temp2)
		actr->temp2--;
	if (_G(boss_dead))
		return boss_die();
	check_boss_hit();

	if (!boss_mode)
		return boss_movement_one(actr);
	num_pods1 = 10;
	switch (_G(setup).skill) {
	case 0:
		actr->num_moves = 3;
		actr->speed = 2;
		break;
	case 1:
		actr->num_moves = 2;
		actr->speed = 1;
		break;
	case 2:
		actr->num_moves = 5;
		actr->speed = 2;
		break;
	}

	int d = actr->last_dir;
	actr->temp3++;

	int f = 0;
	if (actr->temp4) {
		actr->temp4--;
		if (!actr->temp4) {
			actr->temp3 = 0;
			_G(actor[3]).frame_speed = 4;
			_G(actor[3]).dir = 0;
			_G(actor[3]).last_dir = 0;
			_G(actor[3]).next = 3;
			_G(actor[4]).dir = 0;
			_G(actor[4]).last_dir = 0;
			_G(actor[4]).next = 3;
		}
		goto skip_move;
	}

	if (actr->edge_counter)
		actr->edge_counter--;
	else
		goto new_dir;

	if (overlap(actr->x + 2, actr->y + 8, actr->x + 30, actr->y + 30, _G(thor)->x, _G(thor)->y + 4, _G(thor)->x + 15, _G(thor)->y + 15))
		thor_damaged(actr);

	ox = actr->x;
	oy = actr->y;
	switch (actr->temp5) {
	case 0:
		x1 = _G(actor[3]).x;
		y1 = _G(actor[3]).y - 2;
		if (!check_move2(x1, y1, &_G(actor[3]))) {
			f = 1;
			break;
		}
		if (!check_move2(x1 + 16, y1, &_G(actor[4])))
			f = 1;
		actr->y = oy - 2;
		break;
	case 1:
		x1 = _G(actor[5]).x;
		y1 = _G(actor[5]).y + 2;
		if (!check_move2(x1, y1, &_G(actor[5]))) {
			f = 1;
			break;
		}
		if (!check_move2(x1 + 16, y1, &_G(actor[6])))
			f = 1;
		actr->y = oy + 2;
		break;
	case 2:
		x1 = _G(actor[3]).x - 2;
		y1 = _G(actor[3]).y;
		if (!check_move2(x1, y1, &_G(actor[3]))) {
			f = 1;
			break;
		}
		if (!check_move2(x1, y1 + 16, &_G(actor[5])))
			f = 1;
		actr->x = ox - 2;
		break;
	case 3:
		x1 = _G(actor[4]).x + 2;
		y1 = _G(actor[4]).y;
		if (!check_move2(x1, y1, &_G(actor[4]))) {
			f = 1;
			break;
		}
		if (!check_move2(x1, y1 + 16, &_G(actor[6])))
			f = 1;
		actr->x = ox + 2;
		break;
	case 4: //ul
		x1 = _G(actor[3]).x - 2;
		y1 = _G(actor[3]).y - 2;
		if (!check_move2(x1, y1, &_G(actor[3]))) {
			f = 1;
			break;
		}
		actr->x = ox - 2;
		actr->y = oy - 2;
		break;
	case 5:
		x1 = _G(actor[4]).x + 2;
		y1 = _G(actor[4]).y - 2;
		if (!check_move2(x1, y1, &_G(actor[4]))) {
			f = 1;
			break;
		}
		actr->x = ox + 2;
		actr->y = oy - 2;
		break;
	case 6:
		x1 = _G(actor[6]).x + 2;
		y1 = _G(actor[6]).y + 2;
		if (!check_move2(x1, y1, &_G(actor[6]))) {
			f = 1;
			break;
		}
		actr->x = ox + 2;
		actr->y = oy + 2;
		break;
	case 7:
		x1 = _G(actor[5]).x - 2;
		y1 = _G(actor[5]).y + 2;
		if (!check_move2(x1, y1, &_G(actor[5]))) {
			f = 1;
			break;
		}
		actr->x = ox - 2;
		actr->y = oy + 2;
		break;
	}
	fcount = actr->frame_count - 1;
	if (fcount) {
		actr->next++;
		if (actr->next > 2)
			actr->next = 0;
		actr->frame_count = 30;
	} else
		actr->frame_count = fcount;

skip_move:

	set_boss(actr);
	if (!f)
		goto done;

new_dir:
	if (actr->temp3 < 120)
		goto new_dir1;
	
	_G(actor[3]).frame_speed = 8;
	_G(actor[3]).next = 3;
	_G(actor[4]).next = 3;
	actr->temp4 = 120;
	actor_always_shoots(actr, 0);
	_G(actor[actr->shot_actor]).x = actr->x + 8;
	_G(actor[actr->shot_actor]).y = actr->y - 8;
	_G(actor[actr->shot_actor]).temp1 = g_events->getRandomNumber(90, 189);
	_G(actor[actr->shot_actor]).temp5 = 30;
	_G(actor[actr->shot_actor]).speed = 2;
	play_sound(BOSS12, true);

new_dir1:
	actr->temp5 = _G(rand1) % 8;
	actr->edge_counter = _G(rand2) + 60;

done:
	if (actr->directions == 1)
		return 0;
	return d;
}

static void check_boss_hit() {
	int rep;

	if (_G(actor[3]).solid & 128) {
		for (rep = 3; rep < 7; rep++)
			_G(actor[rep]).magic_hit = 0;

		return;
	}
	if (_G(actor[3]).magic_hit || _G(actor[4]).magic_hit || _G(actor[5]).magic_hit || _G(actor[6]).magic_hit) {
		if (!_G(actor[3]).temp2) {
			actor_damaged(&_G(actor[3]), 10);

			if (_G(cheat) && _G(key_flag[_Z]))
				_G(actor[3]).health -= 50;
			else
				_G(actor[3]).health -= 10;
			
			_G(actor[3]).speed_count = 50;

			_G(actor[3]).vunerable = 50;
			play_sound(BOSS13, true);

			for (rep = 4; rep < 7; rep++) {
				_G(actor[rep]).magic_hit = 0;
				_G(actor[rep]).next = 1;
				_G(actor[rep]).speed_count = 50;
			}

			if (_G(actor[3]).health == 0) {
				_G(boss_dead) = true;
				for (rep = 7; rep < MAX_ACTORS; rep++) {
					if (_G(actor[rep]).used)
						actor_destroyed(&_G(actor[rep]));
				}
			}

			if (_G(actor[3]).health == 50) {
				boss_change_mode();
				_G(actor[3]).temp1 = 0;
				_G(actor[3]).temp2 = 0;
				_G(actor[3]).temp3 = 0;
				_G(actor[3]).temp4 = 0;
				_G(actor[3]).temp5 = 0;
				_G(actor[3]).i6 = 0;
				_G(actor[3]).speed_count = 2;
			} else {
				_G(actor[3]).temp2 = 40;
			}
		}
		for (rep = 3; rep < 7; rep++)
			_G(actor[rep]).magic_hit = 0;
	}
}

static void boss_change_mode() {
	if (!_G(boss_intro2)) {
		Gfx::Pics loki("FACE18", 262);
		execute_script(1003, loki);
		_G(boss_intro2) = true;
	}
	boss_mode = 0;
}

void boss_level3() {
	setup_boss(3);
	_G(boss_active) = true;
	music_pause();
	play_sound(BOSS11, true);
	_G(timer_cnt) = 0;

	g_events->send("Game", GameMessage("PAUSE", 40));

	if (!_G(boss_intro1)) {
		Gfx::Pics loki("FACE18", 262);
		execute_script(1002, loki);
		_G(boss_intro1) = true;
	}

	music_play(7, true);
	_G(apple_drop) = 0;
	boss_mode = 1;
}

static int boss_die() {
	if (_G(boss_dead)) {
		for (int rep = 0; rep < 4; rep++) {
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
	}

	return _G(actor[3]).last_dir;
}

void closing_sequence3() {
	music_play(6, true);
	odin_speaks(1001, 0, "CLOSING");
}

void closing_sequence3_2() {
	fill_score(20, "CLOSING");
}

void closing_sequence3_3() {
	fill_health();
	fill_magic();

	for (int rep = 0; rep < 16; rep++)
		_G(scrn).actor_type[rep] = 0;

	_G(boss_dead) = false;
	_G(setup).boss_dead[2] = 1;
	_G(game_over) = true;
	_G(boss_active) = false;
	_G(scrn).type = 6;
	show_level(BOSS_LEVEL3);

	_G(exit_flag) = 0;
	music_pause();

	_G(new_level) = ENDING_SCREEN;
	_G(thor)->x = 152;
	_G(thor)->y = 160;
	_G(thor)->dir = 1;
}

void ending_screen() {
	for (int i = 3; i < MAX_ACTORS; i++)
		_G(actor[i]).move = 1;
	
	music_play(6, true);
	_G(timer_cnt) = 0;

	memset(expf, 0, 4 * 8);
	_G(endgame) = 1;

	_G(exprow) = 0;
	expcnt = 0;

	_G(actor[34]) = _G(explosion);
	_G(actor[34]).used = 0;
	_G(actor[34]).speed = 2;
	_G(actor[34]).speed_count = _G(actor[34]).speed;
	_G(actor[34]).num_shots = 3; // Used to reverse explosion
	_G(actor[34]).vunerable = 255;
	_G(actor[34]).i2 = 6;
}

// Explode
int endgame_one() {
	if (_G(actor[34]).i2) {
		_G(actor[34]).i2--;
		return 0;
	}

	_G(actor[34]).i2 = 6;
	play_sound(EXPLODE, true);

	int r = _G(rand1) % 32;
	while (expf[r / 8][r % 8]) {
		r++;
		if (r > 31)
			r = 0;
	}
	expf[r / 8][r % 8] = 1;
	int x = (EXPLOSION[r / 8][r % 8] % 20) * 16;
	int y = (EXPLOSION[r / 8][r % 8] / 20) * 16;
	_G(actor[34]).x = x;
	_G(actor[34]).y = y;
	_G(actor[34]).used = 1;
	_G(actor[34]).next = 0;
	_G(actor[34]).num_shots = 3;

	_G(scrn).icon[y / 16][x / 16] = _G(scrn).bg_color;

	_G(endgame++);
	if (_G(endgame) > 32) {
		_G(actor[34]).used = 0;
		_G(endgame) = 0;
	}
	return 1;
}

// Explode
int endgame_movement() {
	if (!_G(endgame))
		return 0;
	if (expcnt > 3) {
		endgame_one();
		return 0;
	}
	if (_G(actor[34]).i2) {
		_G(actor[34]).i2--;
		return 0;
	}
	_G(actor[34]).i2 = 6;
	play_sound(EXPLODE, true);

	int r = _G(rand1) % 8;
	while (expf[_G(exprow)][r]) {
		r++;
		if (r > 7)
			r = 0;
	}
	expf[_G(exprow)][r] = 1;
	int x = (EXPLOSION[_G(exprow)][r] % 20) * 16;
	int y = (EXPLOSION[_G(exprow)][r] / 20) * 16;
	_G(actor[34]).x = x;
	_G(actor[34]).y = y;
	_G(actor[34]).used = 1;
	_G(actor[34]).next = 0;
	_G(actor[34]).num_shots = 3;

	_G(scrn).icon[y / 16][x / 16] = _G(scrn).bg_color;
	_G(scrn).icon[(y / 16) - 4][x / 16] = _G(scrn).bg_color;

	_G(endgame++);
	if (_G(endgame) > 8) {
		_G(endgame) = 1;
		_G(exprow++);
		expcnt++;
		if (expcnt > 3) {
			memset(expf, 0, 32);
		}
	}

	return 1;
}

} // namespace Got
