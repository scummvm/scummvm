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

int boss1_movement(Actor *actr) {
	int x1, y1;

	bool f = false;
	
	if (_G(boss_dead))
		return boss1_dead();

	int d = actr->_lastDir;
	if (actr->_edgeCounter) {
		actr->_edgeCounter--;
		goto done;
	}

	x1 = actr->_x;
	y1 = actr->_y;

	if (overlap(actr->_x + 2, actr->_y + 8, actr->_x + 30, actr->_y + 30,
				_G(thor)->_x, _G(thor)->_y + 8, _G(thor)->_x + 15, _G(thor)->_y + 15)) {
		thor_damaged(actr);
	}

	if (actr->_temp3) { //start striking
		actr->_temp3--;
		if (!actr->_temp3)
			play_sound(BOSS11, false);

		if (_G(hourglass_flag))
			actr->_numMoves = 3;
		else
			actr->_numMoves = 6;

		goto done0;
	}

	// Strike
	if (actr->_temp1) {
		actr->_temp1--;
		if (actr->_x < (_G(thor_x1) + 12))
			actr->_temp1 = 0;
		actr->_temp2 = 1;
		d = 2;
		actr->_x -= 2;

		if (overlap(actr->_x + 2, actr->_y + 8, actr->_x + 32, actr->_y + 30,
					_G(thor)->_x, _G(thor)->_y + 8, _G(thor)->_x + 15, _G(thor)->_y + 15)) {
			actr->_temp1 = 0;
			goto done1;
		}

		actr->_nextFrame = 3;
		actr->_numMoves = _G(setup).skill + 2;
		goto done1;
	}

	if (actr->_temp2) { // Retreating
		if (actr->_x < 256) {
			d = 3;
			actr->_x += 2;
			if (overlap(actr->_x + 2, actr->_y + 8, actr->_x + 32, actr->_y + 30,
						_G(thor)->_x, _G(thor)->_y + 8, _G(thor)->_x + 15, _G(thor)->_y + 15)) {
				goto done1;
			}
			actr->_numMoves = _G(setup).skill + 1;
			goto done0;
		}

		actr->_temp2 = 0;
	}

	if (actr->_x > _G(thor_x1) && ABS((_G(thor_y1)) - (actr->_y + 20)) < 8) {
		actr->_temp3 = 75;
		actr->_temp1 = 130;
		actr->_temp2 = 0;
	}
	
	if (actr->_counter) {
		actr->_counter--;
		switch (d) {
		case 1:
		case 3:
			x1 = _G(actor[5])._x;
			y1 = _G(actor[5])._y;
			y1 += 2;

			if (!check_move2(x1, y1, &_G(actor[5])))
				f = true;
			else {
				actr->_x = _G(actor[5])._x;
				actr->_y = _G(actor[5])._y - 16;
			}
			break;
		case 0:
		case 2:
			y1 -= 2;
			if (!check_move2(x1, y1, actr))
				f = true;
			break;

		default:
			break;
		}
	} else
		f = true;

	if (f) {
		actr->_counter = g_events->getRandomNumber(10, 99);
		d = g_events->getRandomNumber(1);
		actr->_edgeCounter = 20;
	}

done:
	if (d > 1)
		d -= 2;

done0:
	next_frame(actr);
	if (actr->_nextFrame == 3)
		actr->_nextFrame = 0;

done1:
	actr->_lastDir = d;

	_G(actor[4])._nextFrame = actr->_nextFrame;
	_G(actor[5])._nextFrame = actr->_nextFrame;
	_G(actor[6])._nextFrame = actr->_nextFrame;

	_G(actor[4])._lastDir = d;
	_G(actor[5])._lastDir = d;
	_G(actor[6])._lastDir = d;

	_G(actor[4])._x = actr->_x + 16;
	_G(actor[4])._y = actr->_y;
	_G(actor[5])._x = actr->_x;
	_G(actor[5])._y = actr->_y + 16;
	_G(actor[6])._x = actr->_x + 16;
	_G(actor[6])._y = actr->_y + 16;
	_G(actor[4])._numMoves = actr->_numMoves;
	_G(actor[5])._numMoves = actr->_numMoves;
	_G(actor[6])._numMoves = actr->_numMoves;

	if (actr->_directions == 1)
		return 0;
	return d;
}

void check_boss1_hit(Actor *actr, int x1, int y1, int x2, int y2, int act_num) {
	if (actr->_moveType == 15 && act_num == 4) {
		if ((!_G(actor[3])._vulnerableCountdown) && (_G(actor[3])._nextFrame != 3) &&
			overlap(x1, y1, x2, y2, actr->_x + 6, actr->_y + 4, actr->_x + 14, actr->_y + 20)) {
			actor_damaged(&_G(actor[3]), _G(hammer)->_hitStrength);
			if (_G(cheat) && _G(key_flag[_Z]))
				_G(actor[3])._health = 0;
			else
				_G(actor[3])._health -= 10;

			_G(actor[3])._moveCountdown = 50;
			_G(actor[3])._vulnerableCountdown = 100;
			play_sound(BOSS13, true);
			_G(actor[3])._nextFrame = 1;

			for (int rep = 4; rep < 7; rep++) {
				_G(actor[rep])._nextFrame = 1;
				_G(actor[rep])._moveCountdown = 50;
			}

			if (_G(actor[3])._health == 0)
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

		for (rep = 7; rep < MAX_ACTORS; rep++) {
			if (_G(actor[rep])._active)
				actor_destroyed(&_G(actor[rep]));
		}
	}

	return _G(actor[3])._lastDir;
}

void closing_sequence1() {
	_G(game_over) = true;
	music_play(4, true);
	odin_speaks(1001, 13, "CLOSING");
}

void closing_sequence1_2() {
	_G(thor_info).armor = 1;
	load_new_thor();
	_G(thor)->_dir = 1;
	_G(thor)->_nextFrame = 0;
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
