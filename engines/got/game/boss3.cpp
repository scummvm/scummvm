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

static void set_boss(Actor *actr) {
	_G(actor[4])._nextFrame = actr->_nextFrame;
	_G(actor[5])._nextFrame = actr->_nextFrame;
	_G(actor[6])._nextFrame = actr->_nextFrame;

	_G(actor[4])._lastDir = actr->_dir;
	_G(actor[5])._lastDir = actr->_dir;
	_G(actor[6])._lastDir = actr->_dir;
	_G(actor[4])._dir = actr->_dir;
	_G(actor[5])._dir = actr->_dir;
	_G(actor[6])._dir = actr->_dir;

	_G(actor[4])._x = actr->_x + 16;
	_G(actor[4])._y = actr->_y;
	_G(actor[5])._x = actr->_x;
	_G(actor[5])._y = actr->_y + 16;
	_G(actor[6])._x = actr->_x + 16;
	_G(actor[6])._y = actr->_y + 16;
}

// Boss - Loki-2
static int boss_movement_one(Actor *actr) {
	int rx, ry, i, numPods = 0;
	int fcount;
	
	actr->_numMoves = 2;
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

	if (!actr->_temp1) {
		// Disappear
		actr->_dir = 1;
		actr->_frameCount = LFC;
		actr->_nextFrame = 0;
		actr->_temp1 = 1;
		actr->_i6 = 1;
		actr->_solid |= 128;
		_G(actor[4])._solid |= 128;
		_G(actor[5])._solid |= 128;
		_G(actor[6])._solid |= 128;
		play_sound(EXPLODE, true);
		goto done;
	}
	if (actr->_i6) {
		// Fade out
		fcount = actr->_frameCount - 1;
		if (fcount <= 0) {
			actr->_nextFrame++;
			if (actr->_nextFrame > 2) {
				actr->_i6 = 0;
				actr->_temp3 = 160;
			}
			actr->_frameCount = 3;
		} else
			actr->_frameCount = fcount;
		
		goto done1;
	}
	if (actr->_temp3 > 1) {
		actr->_temp3--;
		goto done1;
	}

	if (actr->_temp3) {
		for (i = 0; i < num_pods1; i++)
			if (_G(actor[19 + i])._active)
				goto done1;

		while (1) {
			rx = g_events->getRandomNumber(255) + 16;
			ry = g_events->getRandomNumber(143);
			if (!overlap(rx, ry, rx + 32, ry + 32, _G(thor_x1), _G(thor_y1),
						 _G(thor_x2), _G(thor_y2)))
				break;
		}

		actr->_x = rx;
		actr->_y = ry;
		actr->_frameCount = LFC;
		actr->_temp4 = 40;
		actr->_temp3 = 0;
		play_sound(EXPLODE, true);
		goto done1;
	}

	if (actr->_temp4) {
		// Fade in
		fcount = actr->_frameCount - 1;
		if (fcount <= 0) {
			actr->_nextFrame--;
			if (actr->_nextFrame > 254) {
				actr->_nextFrame = 0;
				actr->_dir = 0;
				actr->_temp4 = 0;
				actr->_temp5 = 80;
				actr->_solid &= 0x7f;
				_G(actor[4])._solid &= 0x7f;
				_G(actor[5])._solid &= 0x7f;
				_G(actor[6])._solid &= 0x7f;
			}

			actr->_frameCount = 3;
		} else
			actr->_frameCount = fcount;
		
		goto done1;
	}

	if (actr->_temp5) {
		// Shoot
		actr->_temp5--;
		if (actr->_temp5 == 20) {
			actr->_nextFrame = 3;
			goto done1;
		}
		
		if (!actr->_temp5) {
			if (_G(actor[4])._currNumShots < _G(actor[4])._numShotsAllowed) {
				actor_always_shoots(&_G(actor[4]), 0);
				const byte shot_actor = _G(actor[4])._shotActor;
				_G(actor[shot_actor])._numMoves = pod_speed;
				_G(actor[shot_actor])._x = actr->_x + 8;
				_G(actor[shot_actor])._y = actr->_y + 16;
				_G(actor[shot_actor])._temp5 = 0;
				for (i = 0; i < numPods; i++)
					_G(actor[20 + i]) = _G(actor[19]);

				num_pods1 = numPods;
				actr->_temp1 = 0;
			}
		}

		if (actr->_temp5 < 31)
			goto done1;
	}

done:
	fcount = actr->_frameCount - 1;
	if (fcount <= 0) {
		actr->_nextFrame++;
		if (actr->_nextFrame > 2)
			actr->_nextFrame = 0;
		actr->_frameCount = LFC;
	} else
		actr->_frameCount = fcount;
	
done1:
	set_boss(actr);
	return actr->_dir;
}

// Boss - Loki-1
int boss3_movement(Actor *actr) {
	int x1, y1, ox, oy;
	int fcount;


	if (actr->_temp2)
		actr->_temp2--;
	if (_G(boss_dead))
		return boss_die();
	check_boss_hit();

	if (!boss_mode)
		return boss_movement_one(actr);
	num_pods1 = 10;
	switch (_G(setup).skill) {
	case 0:
		actr->_numMoves = 3;
		actr->_speed = 2;
		break;
	case 1:
		actr->_numMoves = 2;
		actr->_speed = 1;
		break;
	case 2:
		actr->_numMoves = 5;
		actr->_speed = 2;
		break;
	}

	int d = actr->_lastDir;
	actr->_temp3++;

	int f = 0;
	if (actr->_temp4) {
		actr->_temp4--;
		if (!actr->_temp4) {
			actr->_temp3 = 0;
			_G(actor[3])._frameSpeed = 4;
			_G(actor[3])._dir = 0;
			_G(actor[3])._lastDir = 0;
			_G(actor[3])._nextFrame = 3;
			_G(actor[4])._dir = 0;
			_G(actor[4])._lastDir = 0;
			_G(actor[4])._nextFrame = 3;
		}
		goto skip_move;
	}

	if (actr->_edgeCounter)
		actr->_edgeCounter--;
	else
		goto new_dir;

	if (overlap(actr->_x + 2, actr->_y + 8, actr->_x + 30, actr->_y + 30, _G(thor)->_x, _G(thor)->_y + 4, _G(thor)->_x + 15, _G(thor)->_y + 15))
		thor_damaged(actr);

	ox = actr->_x;
	oy = actr->_y;
	switch (actr->_temp5) {
	case 0:
		x1 = _G(actor[3])._x;
		y1 = _G(actor[3])._y - 2;
		if (!check_move2(x1, y1, &_G(actor[3]))) {
			f = 1;
			break;
		}
		if (!check_move2(x1 + 16, y1, &_G(actor[4])))
			f = 1;
		actr->_y = oy - 2;
		break;
	case 1:
		x1 = _G(actor[5])._x;
		y1 = _G(actor[5])._y + 2;
		if (!check_move2(x1, y1, &_G(actor[5]))) {
			f = 1;
			break;
		}
		if (!check_move2(x1 + 16, y1, &_G(actor[6])))
			f = 1;
		actr->_y = oy + 2;
		break;
	case 2:
		x1 = _G(actor[3])._x - 2;
		y1 = _G(actor[3])._y;
		if (!check_move2(x1, y1, &_G(actor[3]))) {
			f = 1;
			break;
		}
		if (!check_move2(x1, y1 + 16, &_G(actor[5])))
			f = 1;
		actr->_x = ox - 2;
		break;
	case 3:
		x1 = _G(actor[4])._x + 2;
		y1 = _G(actor[4])._y;
		if (!check_move2(x1, y1, &_G(actor[4]))) {
			f = 1;
			break;
		}
		if (!check_move2(x1, y1 + 16, &_G(actor[6])))
			f = 1;
		actr->_x = ox + 2;
		break;
	case 4: //ul
		x1 = _G(actor[3])._x - 2;
		y1 = _G(actor[3])._y - 2;
		if (!check_move2(x1, y1, &_G(actor[3]))) {
			f = 1;
			break;
		}
		actr->_x = ox - 2;
		actr->_y = oy - 2;
		break;
	case 5:
		x1 = _G(actor[4])._x + 2;
		y1 = _G(actor[4])._y - 2;
		if (!check_move2(x1, y1, &_G(actor[4]))) {
			f = 1;
			break;
		}
		actr->_x = ox + 2;
		actr->_y = oy - 2;
		break;
	case 6:
		x1 = _G(actor[6])._x + 2;
		y1 = _G(actor[6])._y + 2;
		if (!check_move2(x1, y1, &_G(actor[6]))) {
			f = 1;
			break;
		}
		actr->_x = ox + 2;
		actr->_y = oy + 2;
		break;
	case 7:
		x1 = _G(actor[5])._x - 2;
		y1 = _G(actor[5])._y + 2;
		if (!check_move2(x1, y1, &_G(actor[5]))) {
			f = 1;
			break;
		}
		actr->_x = ox - 2;
		actr->_y = oy + 2;
		break;
	}
	fcount = actr->_frameCount - 1;
	if (fcount) {
		actr->_nextFrame++;
		if (actr->_nextFrame > 2)
			actr->_nextFrame = 0;
		actr->_frameCount = 30;
	} else
		actr->_frameCount = fcount;

skip_move:

	set_boss(actr);
	if (!f)
		goto done;

new_dir:
	if (actr->_temp3 < 120)
		goto new_dir1;
	
	_G(actor[3])._frameSpeed = 8;
	_G(actor[3])._nextFrame = 3;
	_G(actor[4])._nextFrame = 3;
	actr->_temp4 = 120;
	actor_always_shoots(actr, 0);
	_G(actor[actr->_shotActor])._x = actr->_x + 8;
	_G(actor[actr->_shotActor])._y = actr->_y - 8;
	_G(actor[actr->_shotActor])._temp1 = g_events->getRandomNumber(90, 189);
	_G(actor[actr->_shotActor])._temp5 = 30;
	_G(actor[actr->_shotActor])._speed = 2;
	play_sound(BOSS12, true);

new_dir1:
	actr->_temp5 = _G(rand1) % 8;
	actr->_edgeCounter = _G(rand2) + 60;

done:
	if (actr->_directions == 1)
		return 0;
	return d;
}

static void check_boss_hit() {
	int rep;

	if (_G(actor[3])._solid & 128) {
		for (rep = 3; rep < 7; rep++)
			_G(actor[rep])._magicHit = 0;

		return;
	}
	if (_G(actor[3])._magicHit || _G(actor[4])._magicHit || _G(actor[5])._magicHit || _G(actor[6])._magicHit) {
		if (!_G(actor[3])._temp2) {
			actor_damaged(&_G(actor[3]), 10);

			if (_G(cheat) && _G(key_flag[_Z]))
				_G(actor[3])._health -= 50;
			else
				_G(actor[3])._health -= 10;
			
			_G(actor[3])._moveCountdown = 50;

			_G(actor[3])._vulnerableCountdown = 50;
			play_sound(BOSS13, true);

			for (rep = 4; rep < 7; rep++) {
				_G(actor[rep])._magicHit = 0;
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

			if (_G(actor[3])._health == 50) {
				boss_change_mode();
				_G(actor[3])._temp1 = 0;
				_G(actor[3])._temp2 = 0;
				_G(actor[3])._temp3 = 0;
				_G(actor[3])._temp4 = 0;
				_G(actor[3])._temp5 = 0;
				_G(actor[3])._i6 = 0;
				_G(actor[3])._moveCountdown = 2;
			} else {
				_G(actor[3])._temp2 = 40;
			}
		}
		for (rep = 3; rep < 7; rep++)
			_G(actor[rep])._magicHit = 0;
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
	_G(thor)->_x = 152;
	_G(thor)->_y = 160;
	_G(thor)->_dir = 1;
}

void ending_screen() {
	for (int i = 3; i < MAX_ACTORS; i++)
		_G(actor[i])._moveType = 1;
	
	music_play(6, true);
	_G(timer_cnt) = 0;

	memset(expf, 0, 4 * 8);
	_G(endgame) = 1;

	_G(exprow) = 0;
	expcnt = 0;

	_G(actor[34]) = _G(explosion);
	_G(actor[34])._active = 0;
	_G(actor[34])._speed = 2;
	_G(actor[34])._moveCountdown = _G(actor[34])._speed;
	_G(actor[34])._currNumShots = 3; // Used to reverse explosion
	_G(actor[34])._vulnerableCountdown = 255;
	_G(actor[34])._i2 = 6;
}

// Explode
int endgame_one() {
	if (_G(actor[34])._i2) {
		_G(actor[34])._i2--;
		return 0;
	}

	_G(actor[34])._i2 = 6;
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
	_G(actor[34])._x = x;
	_G(actor[34])._y = y;
	_G(actor[34])._active = 1;
	_G(actor[34])._nextFrame = 0;
	_G(actor[34])._currNumShots = 3;

	_G(scrn).icon[y / 16][x / 16] = _G(scrn).bg_color;

	_G(endgame++);
	if (_G(endgame) > 32) {
		_G(actor[34])._active = 0;
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
	if (_G(actor[34])._i2) {
		_G(actor[34])._i2--;
		return 0;
	}
	_G(actor[34])._i2 = 6;
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
	_G(actor[34])._x = x;
	_G(actor[34])._y = y;
	_G(actor[34])._active = 1;
	_G(actor[34])._nextFrame = 0;
	_G(actor[34])._currNumShots = 3;

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
