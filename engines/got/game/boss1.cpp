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

int boss1Movement(Actor *actor) {
	bool f = false;
	
	if (_G(boss_dead))
		return boss1_dead();

	int d = actor->_lastDir;
	int x1 = actor->_x;
	int y1 = actor->_y;

	if (actor->_edgeCounter) {
		actor->_edgeCounter--;
		goto done;
	}

	if (overlap(actor->_x + 2, actor->_y + 8, actor->_x + 30, actor->_y + 30,
				_G(thor)->_x, _G(thor)->_y + 8, _G(thor)->_x + 15, _G(thor)->_y + 15)) {
		thorDamaged(actor);
	}

	if (actor->_temp3) { //start striking
		actor->_temp3--;
		if (!actor->_temp3)
			play_sound(BOSS11, false);

		if (_G(hourglass_flag))
			actor->_numMoves = 3;
		else
			actor->_numMoves = 6;

		goto done0;
	}

	// Strike
	if (actor->_temp1) {
		actor->_temp1--;
		if (actor->_x < (_G(thor_x1) + 12))
			actor->_temp1 = 0;
		actor->_temp2 = 1;
		d = 2;
		actor->_x -= 2;

		if (overlap(actor->_x + 2, actor->_y + 8, actor->_x + 32, actor->_y + 30,
					_G(thor)->_x, _G(thor)->_y + 8, _G(thor)->_x + 15, _G(thor)->_y + 15)) {
			actor->_temp1 = 0;
			goto done1;
		}

		actor->_nextFrame = 3;
		actor->_numMoves = _G(setup)._difficultyLevel + 2;
		goto done1;
	}

	if (actor->_temp2) { // Retreating
		if (actor->_x < 256) {
			d = 3;
			actor->_x += 2;
			if (overlap(actor->_x + 2, actor->_y + 8, actor->_x + 32, actor->_y + 30,
						_G(thor)->_x, _G(thor)->_y + 8, _G(thor)->_x + 15, _G(thor)->_y + 15)) {
				goto done1;
			}
			actor->_numMoves = _G(setup)._difficultyLevel + 1;
			goto done0;
		}

		actor->_temp2 = 0;
	}

	if (actor->_x > _G(thor_x1) && ABS((_G(thor_y1)) - (actor->_y + 20)) < 8) {
		actor->_temp3 = 75;
		actor->_temp1 = 130;
		actor->_temp2 = 0;
	}
	
	if (actor->_counter) {
		actor->_counter--;
		switch (d) {
		case 1:
		case 3:
			x1 = _G(actor[5])._x;
			y1 = _G(actor[5])._y;
			y1 += 2;

			if (!checkMove2(x1, y1, &_G(actor[5])))
				f = true;
			else {
				actor->_x = _G(actor[5])._x;
				actor->_y = _G(actor[5])._y - 16;
			}
			break;
		case 0:
		case 2:
			y1 -= 2;
			if (!checkMove2(x1, y1, actor))
				f = true;
			break;

		default:
			break;
		}
	} else
		f = true;

	if (f) {
		actor->_counter = g_events->getRandomNumber(10, 99);
		d = g_events->getRandomNumber(1);
		actor->_edgeCounter = 20;
	}

done:
	if (d > 1)
		d -= 2;

done0:
	nextFrame(actor);
	if (actor->_nextFrame == 3)
		actor->_nextFrame = 0;

done1:
	actor->_lastDir = d;

	_G(actor[4])._nextFrame = actor->_nextFrame;
	_G(actor[5])._nextFrame = actor->_nextFrame;
	_G(actor[6])._nextFrame = actor->_nextFrame;

	_G(actor[4])._lastDir = d;
	_G(actor[5])._lastDir = d;
	_G(actor[6])._lastDir = d;

	_G(actor[4])._x = actor->_x + 16;
	_G(actor[4])._y = actor->_y;
	_G(actor[5])._x = actor->_x;
	_G(actor[5])._y = actor->_y + 16;
	_G(actor[6])._x = actor->_x + 16;
	_G(actor[6])._y = actor->_y + 16;
	_G(actor[4])._numMoves = actor->_numMoves;
	_G(actor[5])._numMoves = actor->_numMoves;
	_G(actor[6])._numMoves = actor->_numMoves;

	if (actor->_directions == 1)
		return 0;
	return d;
}

void boss1CheckHit(const Actor *actor, int x1, int y1, int x2, int y2, int act_num) {
	if (actor->_moveType == 15 && act_num == 4) {
		if ((!_G(actor[3])._vulnerableCountdown) && (_G(actor[3])._nextFrame != 3) &&
			overlap(x1, y1, x2, y2, actor->_x + 6, actor->_y + 4, actor->_x + 14, actor->_y + 20)) {
			actorDamaged(&_G(actor[3]), _G(hammer)->_hitStrength);
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

void boss1SetupLevel() {
	setupBoss(1);
	_G(boss_active) = true;
	music_pause();
	play_sound(BOSS11, true);
	g_events->send("Game", GameMessage("PAUSE", 40));
	music_play(5, true);
}

static int boss1_dead() {
	_G(hourglass_flag) = 0;
	if (_G(boss_dead)) {
		for (int rep = 0; rep < 4; rep++) {
			const int x1 = _G(actor[3 + rep])._lastX[_G(pge)];
			const int y1 = _G(actor[3 + rep])._lastY[_G(pge)];
			const int x = _G(actor[3 + rep])._x;
			const int y = _G(actor[3 + rep])._y;
			const int n = _G(actor[3 + rep])._actorNum;
			const int r = _G(actor[3 + rep])._dropRating;

			_G(actor[3 + rep]) = _G(explosion);
			_G(actor[3 + rep])._actorNum = n;
			_G(actor[3 + rep])._dropRating = r;
			_G(actor[3 + rep])._x = x;
			_G(actor[3 + rep])._y = y;
			_G(actor[3 + rep])._lastX[_G(pge)] = x1;
			_G(actor[3 + rep])._lastX[_G(pge) ^ 1] = x;
			_G(actor[3 + rep])._lastY[_G(pge)] = y1;
			_G(actor[3 + rep])._lastY[_G(pge) ^ 1] = y;
			_G(actor[3 + rep])._active = true;
			_G(actor[3 + rep])._vulnerableCountdown = 255;
			_G(actor[3 + rep])._moveType = 6;
			_G(actor[3 + rep])._nextFrame = rep;
			_G(actor[3 + rep])._speed = g_events->getRandomNumber(6, 8);
			_G(actor[3 + rep])._currNumShots = (10 - _G(actor[3 + rep])._speed) * 10;
			_G(actor[3 + rep])._moveCountdown = _G(actor[3 + rep])._speed;
		}
		play_sound(EXPLODE, true);
		_G(boss_dead) = true;

		for (int rep = 7; rep < MAX_ACTORS; rep++) {
			if (_G(actor[rep])._active)
				actorDestroyed(&_G(actor[rep]));
		}
	}

	return _G(actor[3])._lastDir;
}

void boss1ClosingSequence1() {
	_G(game_over) = true;
	music_play(4, true);
	odinSpeaks(1001, 13, "CLOSING");
}

void boss1ClosingSequence2() {
	_G(thor_info)._armor = 1;
	load_new_thor();
	_G(thor)->_dir = 1;
	_G(thor)->_nextFrame = 0;
	fill_score(20, "CLOSING");
}

void boss1ClosingSequence3() {
	fill_health();
	fill_magic();
	odinSpeaks(1002, 0, "CLOSING");
}

void boss1ClosingSequence4() {
	for (int rep = 0; rep < 16; rep++)
		_G(scrn)._actorType[rep] = 0;

	_G(boss_dead) = false;
	_G(setup)._bossDead[0] = true;
	_G(boss_active) = false;
	_G(scrn)._music = 4;
	showLevel(BOSS_LEVEL1);

	play_sound(ANGEL, true);
	placeTile(18, 6, 148);
	placeTile(19, 6, 202);
	actor_visible(1);
	actor_visible(2);

	Level lvl;
	lvl.load(59);
	lvl._iconGrid[6][18] = 148;
	lvl._iconGrid[6][19] = 202;
	lvl.save(59);
}

} // namespace Got
