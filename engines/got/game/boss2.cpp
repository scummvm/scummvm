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

#include "got/game/boss2.h"
#include "got/game/back.h"
#include "got/game/init.h"
#include "got/game/move.h"
#include "got/game/move_patterns.h"
#include "got/game/status.h"
#include "got/gfx/image.h"
#include "got/gfx/panel.h"
#include "got/vars.h"
#include "got/events.h"
#include "got/sound.h"

namespace Got {
static const byte exp[] = {
	 61, 62, 65, 66, 69, 70, 73, 74, 77, 78,
	 81, 82, 85, 86, 89, 90, 93, 94, 97, 98,
	101,102,105,106,109,110,113,114,117,118,
	121,122,125,126,129,130,133,134,137,138,
	141,142,145,146,149,150,153,154,157,158,
	161,162,165,166,169,170,173,174,177,178
};
static byte expf[60];
static byte num_skulls;  // Hehe
static byte num_spikes;
static bool drop_flag;
static byte su[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

static int boss2a_movement(ACTOR *actr);
static int boss2b_movement(ACTOR *actr);
static int boss2_die();
static void boss_set(int d, int x, int y);

int boss2_movement(ACTOR *actr) {
	int d, f, x;

	switch (_G(setup).skill) {
	case 0:
		num_skulls = 3;
		num_spikes = 5;
		break;
	case 1:
		num_skulls = 6;
		num_spikes = 8;
		break;
	case 2:
		num_skulls = 9;
		num_spikes = 11;
		break;
	}
	if (_G(boss_dead))
		return boss2_die();

	if (actr->i1) {
		if (actr->i1 == 1) return boss2a_movement(actr);
		else return boss2b_movement(actr);
	}
	d = actr->last_dir;

	//if(overlap(actr->x+2,actr->y+8,actr->x+30,actr->y+30,
	//          _G(thor)->x,_G(thor)->y+4,_G(thor)->x+15,_G(thor)->y+15))
	//  thor_damaged(actr);

	x = actr->x;

	f = 0;
	if (actr->temp6) actr->temp6--;
	if (!actr->temp6) {
		drop_flag = false;
		if (actr->temp5) actr->temp5--;
		if (!actr->temp5) f = 1;
		else {
			if (d == 2) {
				if (x > 18) actr->x -= 2;
				else f = 1;
			} else if (d == 3) {
				if (x < 272) actr->x += 2;
				else f = 1;
			}
		}
		if (f) {
			actr->temp5 = _G(rand1) + 60;
			if (d == 2) d = 3;
			else d = 2;
		}
	}

	actr->frame_count--;
	if (actr->frame_count <= 0) {
		actr->next++;
		if (actr->next > 2) actr->next = 0;
		actr->frame_count = actr->frame_speed;
	}
	x = actr->x;
	if (actr->num_shots < num_skulls && !drop_flag) {
		if (x == 48 || x == 112 || x == 176 || x == 240) {
			drop_flag = true;
			_G(actor)[3].temp6 = 40;
			//    actr->next=3;
			actor_always_shoots(actr, 1);
			play_sound(FALL, 0);
			_G(actor)[actr->shot_actor].x = actr->x + 12;
			_G(actor)[actr->shot_actor].y = actr->y + 32;
			_G(actor)[actr->shot_actor].temp2 = 0;
			_G(actor)[actr->shot_actor].temp3 = 4;
			_G(actor)[actr->shot_actor].temp4 = 4;
		}
	}

	boss_set(d, x, actr->y);

	if (actr->directions == 1) return 0;
	return d;
}

static void boss_set(int d, int x, int y) {
	_G(actor)[4].next = _G(actor)[3].next;
	_G(actor)[5].next = _G(actor)[3].next;
	_G(actor)[6].next = _G(actor)[3].next;
	_G(actor)[3].last_dir = d;
	_G(actor)[4].last_dir = d;
	_G(actor)[5].last_dir = d;
	_G(actor)[6].last_dir = d;
	_G(actor)[4].x = x + 16;
	_G(actor)[4].y = y;
	_G(actor)[5].x = x;
	_G(actor)[5].y = y + 16;
	_G(actor)[6].x = x + 16;
	_G(actor)[6].y = y + 16;
}

void check_boss2_hit(ACTOR *actr, int x1, int y1, int x2, int y2, int act_num) {
	int rep;

	if ((!_G(actor)[3].vunerable)) {
		actor_damaged(&_G(actor)[3], _G(hammer)->strength);
		_G(actor)[3].health -= 10;
		if (_G(actor)[3].health == 50) {
			play_sound(BOSS12, 1);
#if 0
			timer_cnt = 0;
			while (timer_cnt < 120);
#else
			g_events->send("Game", GameMessage("PAUSE", 40));
#endif
			_G(actor)[3].i1 = 1;
			_G(actor)[3].i2 = 0;
			memset(expf, 0, 60);
			for (rep = 7; rep < MAX_ACTORS; rep++)
				if (_G(actor)[rep].used)
					actor_destroyed(&_G(actor)[rep]);
			_G(actor)[3].num_shots = 0;
		} else play_sound(BOSS13, 1);
		_G(actor)[3].speed_count = 75;
		boss_status(_G(actor)[3].health);
		_G(actor)[3].vunerable = 75;
		_G(actor)[3].next = 1;
		for (rep = 4; rep < 7; rep++) {
			_G(actor)[rep].next = 1;
			_G(actor)[rep].speed_count = 50;
		}
		if (_G(actor)[3].health == 0) {
			_G(boss_dead) = 1;
			for (rep = 7; rep < MAX_ACTORS; rep++)
				if (_G(actor)[rep].used)
					actor_destroyed(&_G(actor)[rep]);
		}
	}
	x1 = x1;
	y1 = y1;
	x2 = x2;
	y2 = y2;
	actr = actr;
	act_num = act_num;
}

void boss_level2() {
	setup_boss(2);
	_G(boss_active) = 1;
	boss_status(-1);
	music_pause();
	play_sound(BOSS11, 1);
	_G(timer_cnt) = 0;

	drop_flag = false;
	Common::fill(su, su + 18, 0);

	g_events->send("Game", GameMessage("PAUSE", 40));
	music_play(7, 1);
}

static int boss2_die() {
	int rep, n, x, y, r, x1, y1;

	_G(hourglass_flag) = 0;
	_G(thunder_flag) = 0;
	if (_G(boss_dead) == 1) {
		REPEAT(4) {
			x1 = _G(actor)[3 + rep].last_x[_G(pge)];
			y1 = _G(actor)[3 + rep].last_y[_G(pge)];
			x = _G(actor)[3 + rep].x;
			y = _G(actor)[3 + rep].y;
			n = _G(actor)[3 + rep].actor_num;
			r = _G(actor)[3 + rep].rating;

			_G(actor)[3 + rep] = _G(explosion);

			_G(actor)[3 + rep].actor_num = n;
			_G(actor)[3 + rep].rating = r;
			_G(actor)[3 + rep].x = x;
			_G(actor)[3 + rep].y = y;
			_G(actor)[3 + rep].last_x[_G(pge)] = x1;
			_G(actor)[3 + rep].last_x[_G(pge) ^ 1] = x;
			_G(actor)[3 + rep].last_y[_G(pge)] = y1;
			_G(actor)[3 + rep].last_y[_G(pge) ^ 1] = y;
			_G(actor)[3 + rep].used = 1;
			_G(actor)[3 + rep].vunerable = 255;
			_G(actor)[3 + rep].move = 6;
			_G(actor)[3 + rep].next = rep;
			_G(actor)[3 + rep].speed = g_events->getRandomNumber(6, 8);
			_G(actor)[3 + rep].num_shots = (10 - _G(actor)[3 + rep].speed) * 10;
			_G(actor)[3 + rep].speed_count = _G(actor)[3 + rep].speed;
		}

		play_sound(EXPLODE, 1);
		_G(boss_dead)++;
	}

	return _G(actor)[3].last_dir;
}

// Boss - skull (explode)
static int boss2a_movement(ACTOR *actr) {
	int an, x, y, r;

	next_frame(actr);
	_G(actor)[4].next = actr->next;
	_G(actor)[5].next = actr->next;
	_G(actor)[6].next = actr->next;
	actr->vunerable = 20;
	if (actr->num_shots) return 0;
	if (_G(actor)[5].num_shots) return 0;

	play_sound(EXPLODE, 1);
	actor_always_shoots(&_G(actor)[5], 0);
	an = _G(actor)[5].shot_actor;
	_G(actor)[an].move = 9;

	r = _G(rand1) % 60;
	while (expf[r]) {
		r++;
		if (r > 59) r = 0;
	}
	expf[r] = 1;
	x = (exp[r] % 20) * 16;
	y = (exp[r] / 20) * 16;
	_G(actor)[an].x = x;
	_G(actor)[an].y = y;

#if 0
	xfput(x, y, display_page, (char far *) (bg_pics + (_G(scrn).bg_color * 262)));
	xfput(x, y, draw_page, (char far *) (bg_pics + (_G(scrn).bg_color * 262)));
	xfput(x, y, PAGE2, (char far *) (bg_pics + (_G(scrn).bg_color * 262)));
#endif
	_G(scrn).icon[y / 16][x / 16] = _G(scrn).bg_color;

	_G(actor)[3].i2++;
	if (_G(actor)[3].i2 > 59) {
		_G(actor)[3].i1 = 2;
		_G(actor)[3].i2 = 0;
		_G(actor)[3].num_moves = 3;
	}

	return 0;
}

// Boss - skull - shake
static int boss2b_movement(ACTOR *actr) {
	int rep, an, hx, hy, d;

	if (_G(hammer)->used && _G(hammer)->move != 5) {
		hx = _G(hammer)->x;
		hy = _G(hammer)->y;
		for (rep = 7; rep < 15; rep++) {
			if (!_G(actor)[rep].used) continue;
			if (overlap(hx + 1, hy + 1, hx + 10, hy + 10, _G(actor)[rep].x, _G(actor)[rep].y,
				_G(actor)[rep].x + _G(actor)[rep].size_x - 1, _G(actor)[rep].y + _G(actor)[rep].size_y - 1)) {
				_G(hammer)->move = 5;
				d = reverse_direction(_G(hammer));
				_G(hammer)->dir = d;
				break;
			}
		}
	}
	if (actr->i4) {
		actr->i4--;
		if (!actr->i4) _G(thunder_flag) = 0;
	}
	if (!actr->i2) {
		if (actr->x < 144) actr->x += 2;
		else if (actr->x > 144) actr->x -= 2;
		else {
			actr->i2 = 1;
			actr->i3 = 0;
		}
		goto done;
	}
	if (_G(actor)[4].num_shots) goto done;

	if (!actr->i3) {
		actr->i3 = g_events->getRandomNumber(2, 3);
	}

	if (actr->i3 == 2) actr->x -= 2;
	else actr->x += 2;

	if (actr->x < 20 || actr->x>270) {
		_G(thunder_flag) = 100;
		actr->i4 = 50;
		play_sound(EXPLODE, 1);
		actr->i2 = 0;
		hx = _G(thor)->x;

		Common::fill(su, su + 18, 0);
		actor_always_shoots(&_G(actor)[4], 1);
		an = _G(actor)[4].shot_actor;
		hx = (_G(thor)->x / 16);
		_G(actor)[an].x = _G(thor)->x;  //hx*16;
		_G(actor)[an].y = g_events->getRandomNumber(15);

		su[hx] = 1;
		_G(actor)[an].next = g_events->getRandomNumber(3);
		REPEAT(num_spikes) {
			while (1) {
				hx = g_events->getRandomNumber(17);
				if (!su[hx]) break;
			}
			su[hx] = 1;
			actor_always_shoots(&_G(actor)[4], 1);
			an = _G(actor)[4].shot_actor;
			_G(actor)[an].next = g_events->getRandomNumber(3);
			_G(actor)[an].x = 16 + hx * 16;
			_G(actor)[an].y = g_events->getRandomNumber(15);
		}
	}

done:
	next_frame(actr);
	boss_set(actr->dir, actr->x, actr->y);
	return 0;
}

void closing_sequence2() {
	music_play(6, 1);
	odin_speaks(1001, 0, "CLOSING");
}

void closing_sequence2_2() {
	_G(thor_info).armor = 10;
	load_new_thor();
	_G(thor)->dir = 1;
	_G(thor)->next = 0;

	fill_score(20, "CLOSING");
}

void closing_sequence2_3() {
	fill_health();
	fill_magic();
	odin_speaks(1002, 0, "CLOSING");
}

void closing_sequence2_4() {
	LEVEL lvl;
	int rep;

	REPEAT(16) _G(scrn).actor_type[rep] = 0;
	_G(boss_dead) = 0;
	_G(setup).boss_dead[1] = 1;
	_G(game_over) = 1;
	_G(boss_active) = 0;
	_G(scrn).type = 6;
	//	_G(game_is_over) = 1;
	show_level(BOSS_LEVEL2);

	play_sound(ANGEL, 1);
	place_tile(18, 10, 152);
	place_tile(19, 10, 202);
	actor_visible(1);
	actor_visible(2);
	_G(actor)[7].x = 288;
	_G(actor)[7].y = 160;
	_G(actor)[8].x = 304;
	_G(actor)[8].y = 160;

	lvl.load(BOSS_LEVEL2);
	lvl.icon[6][18] = 152;
	lvl.icon[6][19] = 202;
	lvl.save(BOSS_LEVEL2);
}

} // namespace Got
