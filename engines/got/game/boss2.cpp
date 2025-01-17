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
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/init.h"
#include "got/game/move.h"
#include "got/game/status.h"
#include "got/gfx/image.h"
#include "got/sound.h"
#include "got/vars.h"

namespace Got {

static const byte EXPLOSION[] = {
	61, 62, 65, 66, 69, 70, 73, 74, 77, 78,
	81, 82, 85, 86, 89, 90, 93, 94, 97, 98,
	101, 102, 105, 106, 109, 110, 113, 114, 117, 118,
	121, 122, 125, 126, 129, 130, 133, 134, 137, 138,
	141, 142, 145, 146, 149, 150, 153, 154, 157, 158,
	161, 162, 165, 166, 169, 170, 173, 174, 177, 178
};

static byte expf[60];
static byte num_skulls; // Hehe
static byte num_spikes;
static bool drop_flag;
static byte su[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static int boss2a_movement(Actor *actr);
static int boss2b_movement(Actor *actr);
static int boss2_die();
static void boss_set(int d, int x, int y);

int boss2_movement(Actor *actr) {
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

	if (actr->_i1) {
		if (actr->_i1 == 1)
			return boss2a_movement(actr);

		return boss2b_movement(actr);
	}

	int d = actr->_lastDir;
	int x = actr->_x;

	if (actr->_temp6)
		actr->_temp6--;

	if (!actr->_temp6) {
		bool f = false;
		drop_flag = false;

		if (actr->_temp5)
			actr->_temp5--;

		if (!actr->_temp5)
			f = true;
		else {
			if (d == 2) {
				if (x > 18)
					actr->_x -= 2;
				else
					f = true;
			} else if (d == 3) {
				if (x < 272)
					actr->_x += 2;
				else
					f = true;
			}
		}
		if (f) {
			actr->_temp5 = _G(rand1) + 60;
			if (d == 2)
				d = 3;
			else
				d = 2;
		}
	}

	const int count = actr->_frameCount - 1;
	
	if (count <= 0) {
		actr->_nextFrame++;
		if (actr->_nextFrame > 2)
			actr->_nextFrame = 0;
		actr->_frameCount = actr->_frameSpeed;
	} else
		actr->_frameCount = count;
	
	x = actr->_x;
	if (actr->_currNumShots < num_skulls && !drop_flag) {
		if (x == 48 || x == 112 || x == 176 || x == 240) {
			drop_flag = true;
			_G(actor[3])._temp6 = 40;

			actor_always_shoots(actr, 1);
			play_sound(FALL, false);
			_G(actor[actr->_shotActor])._x = actr->_x + 12;
			_G(actor[actr->_shotActor])._y = actr->_y + 32;
			_G(actor[actr->_shotActor])._temp2 = 0;
			_G(actor[actr->_shotActor])._temp3 = 4;
			_G(actor[actr->_shotActor])._temp4 = 4;
		}
	}

	boss_set(d, x, actr->_y);

	if (actr->_directions == 1)
		return 0;

	return d;
}

static void boss_set(int d, int x, int y) {
	_G(actor[4])._nextFrame = _G(actor[3])._nextFrame;
	_G(actor[5])._nextFrame = _G(actor[3])._nextFrame;
	_G(actor[6])._nextFrame = _G(actor[3])._nextFrame;
	_G(actor[3])._lastDir = d;
	_G(actor[4])._lastDir = d;
	_G(actor[5])._lastDir = d;
	_G(actor[6])._lastDir = d;
	_G(actor[4])._x = x + 16;
	_G(actor[4])._y = y;
	_G(actor[5])._x = x;
	_G(actor[5])._y = y + 16;
	_G(actor[6])._x = x + 16;
	_G(actor[6])._y = y + 16;
}

void check_boss2_hit(Actor *actr, int x1, int y1, int x2, int y2, int act_num) {
	if ((!_G(actor[3])._vulnerableCountdown)) {
		int rep;

		actor_damaged(&_G(actor[3]), _G(hammer)->_hitStrength);
		_G(actor[3])._health -= 10;
		if (_G(actor[3])._health == 50) {
			play_sound(BOSS12, 1);

			g_events->send("Game", GameMessage("PAUSE", 40));

			_G(actor[3])._i1 = 1;
			_G(actor[3])._i2 = 0;
			memset(expf, 0, 60);
			for (rep = 7; rep < MAX_ACTORS; rep++) {
				if (_G(actor[rep])._active)
					actor_destroyed(&_G(actor[rep]));
			}
			_G(actor[3])._currNumShots = 0;
		} else
			play_sound(BOSS13, true);
		
		_G(actor[3])._moveCountdown = 75;
		_G(actor[3])._vulnerableCountdown = 75;
		_G(actor[3])._nextFrame = 1;
		
		for (rep = 4; rep < 7; rep++) {
			_G(actor[rep])._nextFrame = 1;
			_G(actor[rep])._moveCountdown = 50;
		}
		
		if (_G(actor[3])._health == 0) {
			_G(boss_dead) = true;
			for (rep = 7; rep < MAX_ACTORS; rep++) {
				if (_G(actor[rep])._active)
					actor_destroyed(&_G(actor[rep]));
			}
		}
	}
}

void boss_level2() {
	setup_boss(2);
	_G(boss_active) = true;
	music_pause();
	play_sound(BOSS11, true);
	_G(timer_cnt) = 0;

	drop_flag = false;
	Common::fill(su, su + 18, 0);

	g_events->send("Game", GameMessage("PAUSE", 40));
	music_play(7, true);
}

static int boss2_die() {
	_G(hourglass_flag) = 0;
	_G(thunder_flag) = 0;
	if (_G(boss_dead)) {
		for (int rep = 0; rep < 4; rep++) {
			int x1 = _G(actor[3 + rep])._lastX[_G(pge)];
			int y1 = _G(actor[3 + rep])._lastY[_G(pge)];
			int x = _G(actor[3 + rep])._x;
			int y = _G(actor[3 + rep])._y;
			int n = _G(actor[3 + rep])._actorNum;
			int r = _G(actor[3 + rep])._dropRating;

			_G(actor[3 + rep]) = _G(explosion);

			_G(actor[3 + rep])._actorNum = n;
			_G(actor[3 + rep])._dropRating = r;
			_G(actor[3 + rep])._x = x;
			_G(actor[3 + rep])._y = y;
			_G(actor[3 + rep])._lastX[_G(pge)] = x1;
			_G(actor[3 + rep])._lastX[_G(pge) ^ 1] = x;
			_G(actor[3 + rep])._lastY[_G(pge)] = y1;
			_G(actor[3 + rep])._lastY[_G(pge) ^ 1] = y;
			_G(actor[3 + rep])._active = 1;
			_G(actor[3 + rep])._vulnerableCountdown = 255;
			_G(actor[3 + rep])._moveType = 6;
			_G(actor[3 + rep])._nextFrame = rep;
			_G(actor[3 + rep])._speed = g_events->getRandomNumber(6, 8);
			_G(actor[3 + rep])._currNumShots = (10 - _G(actor[3 + rep])._speed) * 10;
			_G(actor[3 + rep])._moveCountdown = _G(actor[3 + rep])._speed;
		}

		play_sound(EXPLODE, true);
		_G(boss_dead) = true;
	}

	return _G(actor[3])._lastDir;
}

// Boss - skull (explode)
static int boss2a_movement(Actor *actr) {
	next_frame(actr);
	_G(actor[4])._nextFrame = actr->_nextFrame;
	_G(actor[5])._nextFrame = actr->_nextFrame;
	_G(actor[6])._nextFrame = actr->_nextFrame;
	actr->_vulnerableCountdown = 20;
	
	if (actr->_currNumShots || _G(actor[5])._currNumShots)
		return 0;

	play_sound(EXPLODE, true);
	actor_always_shoots(&_G(actor[5]), 0);
	int an = _G(actor[5])._shotActor;
	_G(actor[an])._moveType = 9;

	int r = _G(rand1) % 60;
	while (expf[r]) {
		r++;
		if (r > 59)
			r = 0;
	}
	expf[r] = 1;
	int x = (EXPLOSION[r] % 20) * 16;
	int y = (EXPLOSION[r] / 20) * 16;
	_G(actor[an])._x = x;
	_G(actor[an])._y = y;

	_G(scrn).icon[y / 16][x / 16] = _G(scrn).bg_color;

	_G(actor[3])._i2++;
	if (_G(actor[3])._i2 > 59) {
		_G(actor[3])._i1 = 2;
		_G(actor[3])._i2 = 0;
		_G(actor[3])._numMoves = 3;
	}

	return 0;
}

// Boss - skull - shake
static int boss2b_movement(Actor *actr) {
	int rep, an, hx;

	if (_G(hammer)->_active && _G(hammer)->_moveType != 5) {
		hx = _G(hammer)->_x;
		int hy = _G(hammer)->_y;
		for (rep = 7; rep < 15; rep++) {
			if (!_G(actor[rep])._active)
				continue;
			
			if (overlap(hx + 1, hy + 1, hx + 10, hy + 10, _G(actor[rep])._x, _G(actor[rep])._y,
						_G(actor[rep])._x + _G(actor[rep])._sizeX - 1, _G(actor[rep])._y + _G(actor[rep])._sizeY - 1)) {
				_G(hammer)->_moveType = 5;
				int d = reverse_direction(_G(hammer));
				_G(hammer)->_dir = d;
				break;
			}
		}
	}
	if (actr->_i4) {
		actr->_i4--;
		if (!actr->_i4)
			_G(thunder_flag) = 0;
	}
	if (!actr->_i2) {
		if (actr->_x < 144)
			actr->_x += 2;
		else if (actr->_x > 144)
			actr->_x -= 2;
		else {
			actr->_i2 = 1;
			actr->_i3 = 0;
		}
		goto done;
	}
	if (_G(actor[4])._currNumShots)
		goto done;

	if (!actr->_i3) {
		actr->_i3 = g_events->getRandomNumber(2, 3);
	}

	if (actr->_i3 == 2)
		actr->_x -= 2;
	else
		actr->_x += 2;

	if (actr->_x < 20 || actr->_x > 270) {
		_G(thunder_flag) = 100;
		actr->_i4 = 50;
		play_sound(EXPLODE, true);
		actr->_i2 = 0;

		Common::fill(su, su + 18, 0);
		actor_always_shoots(&_G(actor[4]), 1);
		an = _G(actor[4])._shotActor;
		hx = (_G(thor)->_x / 16);
		_G(actor[an])._x = _G(thor)->_x; //hx*16;
		_G(actor[an])._y = g_events->getRandomNumber(15);

		su[hx] = 1;
		_G(actor[an])._nextFrame = g_events->getRandomNumber(3);
		for (rep = 0; rep < num_spikes; rep++) {
			while (1) {
				hx = g_events->getRandomNumber(17);
				if (!su[hx])
					break;
			}
			su[hx] = 1;
			actor_always_shoots(&_G(actor[4]), 1);
			an = _G(actor[4])._shotActor;
			_G(actor[an])._nextFrame = g_events->getRandomNumber(3);
			_G(actor[an])._x = 16 + hx * 16;
			_G(actor[an])._y = g_events->getRandomNumber(15);
		}
	}

done:
	next_frame(actr);
	boss_set(actr->_dir, actr->_x, actr->_y);
	return 0;
}

void closing_sequence2() {
	music_play(6, true);
	odin_speaks(1001, 0, "CLOSING");
}

void closing_sequence2_2() {
	_G(thor_info).armor = 10;
	load_new_thor();
	_G(thor)->_dir = 1;
	_G(thor)->_nextFrame = 0;

	fill_score(20, "CLOSING");
}

void closing_sequence2_3() {
	fill_health();
	fill_magic();
	odin_speaks(1002, 0, "CLOSING");
}

void closing_sequence2_4() {
	for (int rep = 0; rep < 16; rep++)
		_G(scrn).actor_type[rep] = 0;

	_G(boss_dead) = false;
	_G(setup).boss_dead[1] = 1;
	_G(game_over) = true;
	_G(boss_active) = false;
	_G(scrn).type = 6;

	show_level(BOSS_LEVEL2);

	play_sound(ANGEL, true);
	place_tile(18, 10, 152);
	place_tile(19, 10, 202);
	actor_visible(1);
	actor_visible(2);
	_G(actor[7])._x = 288;
	_G(actor[7])._y = 160;
	_G(actor[8])._x = 304;
	_G(actor[8])._y = 160;

	LEVEL lvl;
	lvl.load(BOSS_LEVEL2);
	lvl.icon[6][18] = 152;
	lvl.icon[6][19] = 202;
	lvl.save(BOSS_LEVEL2);
}

} // namespace Got
