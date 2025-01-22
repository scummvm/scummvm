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

static bool expf[60];
static byte numSkulls;
static byte numSpikes;
static bool dropFlag;
static byte su[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static int boss2MovementExplode(Actor *actor);
static int boss2MovementShake(Actor *actor);
static int boss2Die();
static void bossSet(byte d, int x, int y);

int boss2Movement(Actor *actor) {
	switch (_G(setup)._difficultyLevel) {
	case 0:
		numSkulls = 3;
		numSpikes = 5;
		break;

	case 1:
		numSkulls = 6;
		numSpikes = 8;
		break;

	case 2:
		numSkulls = 9;
		numSpikes = 11;
		break;

	default:
		break;
	}
	if (_G(boss_dead))
		return boss2Die();

	if (actor->_i1) {
		if (actor->_i1 == 1)
			return boss2MovementExplode(actor);

		return boss2MovementShake(actor);
	}

	byte d = actor->_lastDir;
	int x = actor->_x;

	if (actor->_temp6)
		actor->_temp6--;

	if (!actor->_temp6) {
		bool f = false;
		dropFlag = false;

		if (actor->_temp5)
			actor->_temp5--;

		if (!actor->_temp5)
			f = true;
		else {
			if (d == 2) {
				if (x > 18)
					actor->_x -= 2;
				else
					f = true;
			} else if (d == 3) {
				if (x < 272)
					actor->_x += 2;
				else
					f = true;
			}
		}
		if (f) {
			actor->_temp5 = _G(rand1) + 60;
			if (d == 2)
				d = 3;
			else
				d = 2;
		}
	}

	const int count = actor->_frameCount - 1;
	
	if (count <= 0) {
		actor->_nextFrame++;
		if (actor->_nextFrame > 2)
			actor->_nextFrame = 0;
		actor->_frameCount = actor->_frameSpeed;
	} else
		actor->_frameCount = count;
	
	x = actor->_x;
	if (actor->_currNumShots < numSkulls && !dropFlag) {
		if (x == 48 || x == 112 || x == 176 || x == 240) {
			dropFlag = true;
			_G(actor[3])._temp6 = 40;

			actorAlwaysShoots(actor, 1);
			playSound(FALL, false);
			_G(actor[actor->_shotActor])._x = actor->_x + 12;
			_G(actor[actor->_shotActor])._y = actor->_y + 32;
			_G(actor[actor->_shotActor])._temp2 = 0;
			_G(actor[actor->_shotActor])._temp3 = 4;
			_G(actor[actor->_shotActor])._temp4 = 4;
		}
	}

	bossSet(d, x, actor->_y);

	if (actor->_directions == 1)
		return 0;

	return d;
}

static void bossSet(byte d, int x, int y) {
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

void boss2CheckHit(Actor *actor, int x1, int y1, int x2, int y2, int act_num) {
	if ((!_G(actor[3])._vulnerableCountdown)) {
		actorDamaged(&_G(actor[3]), _G(hammer)->_hitStrength);
		_G(actor[3])._health -= 10;
		if (_G(actor[3])._health == 50) {
			playSound(BOSS12, true);

			g_events->send("Game", GameMessage("PAUSE", 40));

			_G(actor[3])._i1 = 1;
			_G(actor[3])._i2 = 0;
			memset(expf, 0, 60);

			for (int rep = 7; rep < MAX_ACTORS; rep++) {
				if (_G(actor[rep])._active)
					actorDestroyed(&_G(actor[rep]));
			}
			_G(actor[3])._currNumShots = 0;
		} else
			playSound(BOSS13, true);
		
		_G(actor[3])._moveCountdown = 75;
		_G(actor[3])._vulnerableCountdown = 75;
		_G(actor[3])._nextFrame = 1;

		for (int rep = 4; rep < 7; rep++) {
			_G(actor[rep])._nextFrame = 1;
			_G(actor[rep])._moveCountdown = 50;
		}
		
		if (_G(actor[3])._health == 0) {
			_G(boss_dead) = true;
			for (int rep = 7; rep < MAX_ACTORS; rep++) {
				if (_G(actor[rep])._active)
					actorDestroyed(&_G(actor[rep]));
			}
		}
	}
}

void boss2SetupLevel() {
	setupBoss(2);
	_G(boss_active) = true;
	musicPause();
	playSound(BOSS11, true);
	_G(timer_cnt) = 0;

	dropFlag = false;
	Common::fill(su, su + 18, 0);

	g_events->send("Game", GameMessage("PAUSE", 40));
	musicPlay(7, true);
}

static int boss2Die() {
	_G(hourglass_flag) = 0;
	_G(thunder_flag) = 0;
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

		playSound(EXPLODE, true);
		_G(boss_dead) = true;
	}

	return _G(actor[3])._lastDir;
}

// Boss - skull (explode)
static int boss2MovementExplode(Actor *actor) {
	nextFrame(actor);
	_G(actor[4])._nextFrame = actor->_nextFrame;
	_G(actor[5])._nextFrame = actor->_nextFrame;
	_G(actor[6])._nextFrame = actor->_nextFrame;
	actor->_vulnerableCountdown = 20;
	
	if (actor->_currNumShots || _G(actor[5])._currNumShots)
		return 0;

	playSound(EXPLODE, true);
	actorAlwaysShoots(&_G(actor[5]), 0);
	const int an = _G(actor[5])._shotActor;
	_G(actor[an])._moveType = 9;

	int r = _G(rand1) % 60;
	while (expf[r]) {
		r++;
		if (r > 59)
			r = 0;
	}
	expf[r] = true;
	const int x = (EXPLOSION[r] % 20) * 16;
	const int y = (EXPLOSION[r] / 20) * 16;
	_G(actor[an])._x = x;
	_G(actor[an])._y = y;

	_G(scrn)._iconGrid[y / 16][x / 16] = _G(scrn)._backgroundColor;

	_G(actor[3])._i2++;
	if (_G(actor[3])._i2 > 59) {
		_G(actor[3])._i1 = 2;
		_G(actor[3])._i2 = 0;
		_G(actor[3])._numMoves = 3;
	}

	return 0;
}

// Boss - skull - shake
static int boss2MovementShake(Actor *actor) {
	int rep, an, hx;

	if (_G(hammer)->_active && _G(hammer)->_moveType != 5) {
		hx = _G(hammer)->_x;
		const int hy = _G(hammer)->_y;
		for (rep = 7; rep < 15; rep++) {
			if (!_G(actor[rep])._active)
				continue;
			
			if (overlap(hx + 1, hy + 1, hx + 10, hy + 10, _G(actor[rep])._x, _G(actor[rep])._y,
						_G(actor[rep])._x + _G(actor[rep])._sizeX - 1, _G(actor[rep])._y + _G(actor[rep])._sizeY - 1)) {
				_G(hammer)->_moveType = 5;
				_G(hammer)->_dir = reverseDirection(_G(hammer));
				break;
			}
		}
	}
	if (actor->_i4) {
		actor->_i4--;
		if (!actor->_i4)
			_G(thunder_flag) = 0;
	}
	if (!actor->_i2) {
		if (actor->_x < 144)
			actor->_x += 2;
		else if (actor->_x > 144)
			actor->_x -= 2;
		else {
			actor->_i2 = 1;
			actor->_i3 = 0;
		}
		goto done;
	}
	if (_G(actor[4])._currNumShots)
		goto done;

	if (!actor->_i3) {
		actor->_i3 = g_events->getRandomNumber(2, 3);
	}

	if (actor->_i3 == 2)
		actor->_x -= 2;
	else
		actor->_x += 2;

	if (actor->_x < 20 || actor->_x > 270) {
		_G(thunder_flag) = 100;
		actor->_i4 = 50;
		playSound(EXPLODE, true);
		actor->_i2 = 0;

		Common::fill(su, su + 18, 0);
		actorAlwaysShoots(&_G(actor[4]), 1);
		an = _G(actor[4])._shotActor;
		hx = (_G(thor)->_x / 16);
		_G(actor[an])._x = _G(thor)->_x; //hx*16;
		_G(actor[an])._y = g_events->getRandomNumber(15);

		su[hx] = 1;
		_G(actor[an])._nextFrame = g_events->getRandomNumber(3);
		for (rep = 0; rep < numSpikes; rep++) {
			while (true) {
				hx = g_events->getRandomNumber(17);
				if (!su[hx])
					break;
			}
			su[hx] = 1;
			actorAlwaysShoots(&_G(actor[4]), 1);
			an = _G(actor[4])._shotActor;
			_G(actor[an])._nextFrame = g_events->getRandomNumber(3);
			_G(actor[an])._x = 16 + hx * 16;
			_G(actor[an])._y = g_events->getRandomNumber(15);
		}
	}

done:
	nextFrame(actor);
	bossSet(actor->_dir, actor->_x, actor->_y);
	return 0;
}

void boss2ClosingSequence1() {
	musicPlay(6, true);
	odinSpeaks(1001, 0, "CLOSING");
}

void boss2ClosingSequence2() {
	_G(thor_info)._armor = 10;
	loadNewThor();
	_G(thor)->_dir = 1;
	_G(thor)->_nextFrame = 0;

	fillScore(20, "CLOSING");
}

void boss2ClosingSequence3() {
	fillHealth();
	fillMagic();
	odinSpeaks(1002, 0, "CLOSING");
}

void boss2ClosingSequence4() {
	for (int rep = 0; rep < 16; rep++)
		_G(scrn)._actorType[rep] = 0;

	_G(boss_dead) = false;
	_G(setup)._bossDead[1] = true;
	_G(game_over) = true;
	_G(boss_active) = false;
	_G(scrn)._music = 6;

	showLevel(BOSS_LEVEL2);

	playSound(ANGEL, true);
	placeTile(18, 10, 152);
	placeTile(19, 10, 202);
	actorVisible(1);
	actorVisible(2);
	_G(actor[7])._x = 288;
	_G(actor[7])._y = 160;
	_G(actor[8])._x = 304;
	_G(actor[8])._y = 160;

	Level lvl;
	lvl.load(BOSS_LEVEL2);
	lvl._iconGrid[6][18] = 152;
	lvl._iconGrid[6][19] = 202;
	lvl.save(BOSS_LEVEL2);
}

} // namespace Got
