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

static const byte EXPLOSION[4][8] = {
	{126, 127, 128, 129, 130, 131, 132, 133},
	{146, 147, 148, 149, 150, 151, 152, 153},
	{166, 167, 168, 169, 170, 171, 172, 173},
	{186, 187, 188, 189, 190, 191, 192, 193}
};

static int bossMode;
static int numPods1;
static byte podSpeed;
static bool expf[4][8];
static byte expCounter;

static int bossDie();
static void boss3CheckHit();
static void bossChangeMode();

static void setBoss(Actor *actor) {
	_G(actor[4])._nextFrame = actor->_nextFrame;
	_G(actor[5])._nextFrame = actor->_nextFrame;
	_G(actor[6])._nextFrame = actor->_nextFrame;

	_G(actor[4])._lastDir = actor->_dir;
	_G(actor[5])._lastDir = actor->_dir;
	_G(actor[6])._lastDir = actor->_dir;
	_G(actor[4])._dir = actor->_dir;
	_G(actor[5])._dir = actor->_dir;
	_G(actor[6])._dir = actor->_dir;

	_G(actor[4])._x = actor->_x + 16;
	_G(actor[4])._y = actor->_y;
	_G(actor[5])._x = actor->_x;
	_G(actor[5])._y = actor->_y + 16;
	_G(actor[6])._x = actor->_x + 16;
	_G(actor[6])._y = actor->_y + 16;
}

// Boss - Loki-2
static int boss3Movement1(Actor *actor) {
	int rx, ry, i, numPods = 0;
	int fcount;
	
	actor->_numMoves = 2;
	podSpeed = 2;

	switch (_G(setup)._difficultyLevel) {
	case 0:
		numPods = 3;
		break;

	case 1:
		numPods = 5;
		break;

	case 2:
		numPods = 8;
		break;

	default:
		break;
	}

	if (!actor->_temp1) {
		// Disappear
		actor->_dir = 1;
		actor->_frameCount = LFC;
		actor->_nextFrame = 0;
		actor->_temp1 = 1;
		actor->_i6 = 1;
		actor->_solid |= 128;
		_G(actor[4])._solid |= 128;
		_G(actor[5])._solid |= 128;
		_G(actor[6])._solid |= 128;
		play_sound(EXPLODE, true);
		goto done;
	}
	if (actor->_i6) {
		// Fade out
		fcount = actor->_frameCount - 1;
		if (fcount <= 0) {
			actor->_nextFrame++;
			if (actor->_nextFrame > 2) {
				actor->_i6 = 0;
				actor->_temp3 = 160;
			}
			actor->_frameCount = 3;
		} else
			actor->_frameCount = fcount;
		
		goto done1;
	}
	if (actor->_temp3 > 1) {
		actor->_temp3--;
		goto done1;
	}

	if (actor->_temp3) {
		for (i = 0; i < numPods1; i++)
			if (_G(actor[19 + i])._active)
				goto done1;

		while (true) {
			rx = g_events->getRandomNumber(255) + 16;
			ry = g_events->getRandomNumber(143);
			if (!overlap(rx, ry, rx + 32, ry + 32, _G(thor_x1), _G(thor_y1),
						 _G(thor_x2), _G(thor_y2)))
				break;
		}

		actor->_x = rx;
		actor->_y = ry;
		actor->_frameCount = LFC;
		actor->_temp4 = 40;
		actor->_temp3 = 0;
		play_sound(EXPLODE, true);
		goto done1;
	}

	if (actor->_temp4) {
		// Fade in
		fcount = actor->_frameCount - 1;
		if (fcount <= 0) {
			actor->_nextFrame--;
			if (actor->_nextFrame > 254) {
				actor->_nextFrame = 0;
				actor->_dir = 0;
				actor->_temp4 = 0;
				actor->_temp5 = 80;
				actor->_solid &= 0x7f;
				_G(actor[4])._solid &= 0x7f;
				_G(actor[5])._solid &= 0x7f;
				_G(actor[6])._solid &= 0x7f;
			}

			actor->_frameCount = 3;
		} else
			actor->_frameCount = fcount;
		
		goto done1;
	}

	if (actor->_temp5) {
		// Shoot
		actor->_temp5--;
		if (actor->_temp5 == 20) {
			actor->_nextFrame = 3;
			goto done1;
		}
		
		if (!actor->_temp5) {
			if (_G(actor[4])._currNumShots < _G(actor[4])._numShotsAllowed) {
				actorAlwaysShoots(&_G(actor[4]), 0);
				const byte shot_actor = _G(actor[4])._shotActor;
				_G(actor[shot_actor])._numMoves = podSpeed;
				_G(actor[shot_actor])._x = actor->_x + 8;
				_G(actor[shot_actor])._y = actor->_y + 16;
				_G(actor[shot_actor])._temp5 = 0;
				for (i = 0; i < numPods; i++)
					_G(actor[20 + i]) = _G(actor[19]);

				numPods1 = numPods;
				actor->_temp1 = 0;
			}
		}

		if (actor->_temp5 < 31)
			goto done1;
	}

done:
	fcount = actor->_frameCount - 1;
	if (fcount <= 0) {
		actor->_nextFrame++;
		if (actor->_nextFrame > 2)
			actor->_nextFrame = 0;
		actor->_frameCount = LFC;
	} else
		actor->_frameCount = fcount;
	
done1:
	setBoss(actor);
	return actor->_dir;
}

// Boss - Loki-1
int boss3Movement(Actor *actor) {
	int x1, y1, ox, oy;
	int fcount;


	if (actor->_temp2)
		actor->_temp2--;
	if (_G(boss_dead))
		return bossDie();
	boss3CheckHit();

	if (!bossMode)
		return boss3Movement1(actor);
	numPods1 = 10;
	switch (_G(setup)._difficultyLevel) {
	case 0:
		actor->_numMoves = 3;
		actor->_speed = 2;
		break;
	case 1:
		actor->_numMoves = 2;
		actor->_speed = 1;
		break;
	case 2:
		actor->_numMoves = 5;
		actor->_speed = 2;
		break;
	default:
		break;
	}

	int d = actor->_lastDir;
	actor->_temp3++;

	int f = 0;
	if (actor->_temp4) {
		actor->_temp4--;
		if (!actor->_temp4) {
			actor->_temp3 = 0;
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

	if (actor->_edgeCounter)
		actor->_edgeCounter--;
	else
		goto new_dir;

	if (overlap(actor->_x + 2, actor->_y + 8, actor->_x + 30, actor->_y + 30, _G(thor)->_x, _G(thor)->_y + 4, _G(thor)->_x + 15, _G(thor)->_y + 15))
		thorDamaged(actor);

	ox = actor->_x;
	oy = actor->_y;
	switch (actor->_temp5) {
	case 0:
		x1 = _G(actor[3])._x;
		y1 = _G(actor[3])._y - 2;
		if (!checkMove2(x1, y1, &_G(actor[3]))) {
			f = 1;
			break;
		}
		if (!checkMove2(x1 + 16, y1, &_G(actor[4])))
			f = 1;
		actor->_y = oy - 2;
		break;
	case 1:
		x1 = _G(actor[5])._x;
		y1 = _G(actor[5])._y + 2;
		if (!checkMove2(x1, y1, &_G(actor[5]))) {
			f = 1;
			break;
		}
		if (!checkMove2(x1 + 16, y1, &_G(actor[6])))
			f = 1;
		actor->_y = oy + 2;
		break;
	case 2:
		x1 = _G(actor[3])._x - 2;
		y1 = _G(actor[3])._y;
		if (!checkMove2(x1, y1, &_G(actor[3]))) {
			f = 1;
			break;
		}
		if (!checkMove2(x1, y1 + 16, &_G(actor[5])))
			f = 1;
		actor->_x = ox - 2;
		break;
	case 3:
		x1 = _G(actor[4])._x + 2;
		y1 = _G(actor[4])._y;
		if (!checkMove2(x1, y1, &_G(actor[4]))) {
			f = 1;
			break;
		}
		if (!checkMove2(x1, y1 + 16, &_G(actor[6])))
			f = 1;
		actor->_x = ox + 2;
		break;
	case 4: //ul
		x1 = _G(actor[3])._x - 2;
		y1 = _G(actor[3])._y - 2;
		if (!checkMove2(x1, y1, &_G(actor[3]))) {
			f = 1;
			break;
		}
		actor->_x = ox - 2;
		actor->_y = oy - 2;
		break;
	case 5:
		x1 = _G(actor[4])._x + 2;
		y1 = _G(actor[4])._y - 2;
		if (!checkMove2(x1, y1, &_G(actor[4]))) {
			f = 1;
			break;
		}
		actor->_x = ox + 2;
		actor->_y = oy - 2;
		break;
	case 6:
		x1 = _G(actor[6])._x + 2;
		y1 = _G(actor[6])._y + 2;
		if (!checkMove2(x1, y1, &_G(actor[6]))) {
			f = 1;
			break;
		}
		actor->_x = ox + 2;
		actor->_y = oy + 2;
		break;
	case 7:
		x1 = _G(actor[5])._x - 2;
		y1 = _G(actor[5])._y + 2;
		if (!checkMove2(x1, y1, &_G(actor[5]))) {
			f = 1;
			break;
		}
		actor->_x = ox - 2;
		actor->_y = oy + 2;
		break;

	default:
		break;
	}
	
	fcount = actor->_frameCount - 1;
	if (fcount) {
		actor->_nextFrame++;
		if (actor->_nextFrame > 2)
			actor->_nextFrame = 0;
		actor->_frameCount = 30;
	} else
		actor->_frameCount = fcount;

skip_move:

	setBoss(actor);
	if (!f)
		goto done;

new_dir:
	if (actor->_temp3 < 120)
		goto new_dir1;
	
	_G(actor[3])._frameSpeed = 8;
	_G(actor[3])._nextFrame = 3;
	_G(actor[4])._nextFrame = 3;
	actor->_temp4 = 120;
	actorAlwaysShoots(actor, 0);
	_G(actor[actor->_shotActor])._x = actor->_x + 8;
	_G(actor[actor->_shotActor])._y = actor->_y - 8;
	_G(actor[actor->_shotActor])._temp1 = g_events->getRandomNumber(90, 189);
	_G(actor[actor->_shotActor])._temp5 = 30;
	_G(actor[actor->_shotActor])._speed = 2;
	play_sound(BOSS12, true);

new_dir1:
	actor->_temp5 = _G(rand1) % 8;
	actor->_edgeCounter = _G(rand2) + 60;

done:
	if (actor->_directions == 1)
		return 0;
	return d;
}

static void boss3CheckHit() {
	if (_G(actor[3])._solid & 128) {
		for (int rep = 3; rep < 7; rep++)
			_G(actor[rep])._magicHit = 0;

		return;
	}
	if (_G(actor[3])._magicHit || _G(actor[4])._magicHit || _G(actor[5])._magicHit || _G(actor[6])._magicHit) {
		if (!_G(actor[3])._temp2) {
			actorDamaged(&_G(actor[3]), 10);

			if (_G(cheat) && _G(key_flag[_Z]))
				_G(actor[3])._health -= 50;
			else
				_G(actor[3])._health -= 10;
			
			_G(actor[3])._moveCountdown = 50;

			_G(actor[3])._vulnerableCountdown = 50;
			play_sound(BOSS13, true);

			for (int rep = 4; rep < 7; rep++) {
				_G(actor[rep])._magicHit = 0;
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

			if (_G(actor[3])._health == 50) {
				bossChangeMode();
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
		for (int rep = 3; rep < 7; rep++)
			_G(actor[rep])._magicHit = 0;
	}
}

static void bossChangeMode() {
	if (!_G(boss_intro2)) {
		Gfx::Pics loki("FACE18", 262);
		executeScript(1003, loki);
		_G(boss_intro2) = true;
	}
	
	bossMode = 0;
}

void boss3SetupLevel() {
	setupBoss(3);
	_G(boss_active) = true;
	music_pause();
	play_sound(BOSS11, true);
	_G(timer_cnt) = 0;

	g_events->send("Game", GameMessage("PAUSE", 40));

	if (!_G(boss_intro1)) {
		Gfx::Pics loki("FACE18", 262);
		executeScript(1002, loki);
		_G(boss_intro1) = true;
	}

	music_play(7, true);
	_G(apple_drop) = 0;
	bossMode = 1;
}

static int bossDie() {
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
	}

	return _G(actor[3])._lastDir;
}

void boss3ClosingSequence1() {
	music_play(6, true);
	odinSpeaks(1001, 0, "CLOSING");
}

void boss3ClosingSequence2() {
	fill_score(20, "CLOSING");
}

void boss3ClosingSequence3() {
	fill_health();
	fill_magic();

	for (int rep = 0; rep < 16; rep++)
		_G(scrn)._actorType[rep] = 0;

	_G(boss_dead) = false;
	_G(setup)._bossDead[2] = true;
	_G(game_over) = true;
	_G(boss_active) = false;
	_G(scrn)._music = 6;
	showLevel(BOSS_LEVEL3);

	_G(exit_flag) = 0;
	music_pause();

	_G(new_level) = ENDING_SCREEN;
	_G(thor)->_x = 152;
	_G(thor)->_y = 160;
	_G(thor)->_dir = 1;
}

void endingScreen() {
	for (int i = 3; i < MAX_ACTORS; i++)
		_G(actor[i])._moveType = 1;
	
	music_play(6, true);
	_G(timer_cnt) = 0;

	memset(expf, 0, 4 * 8);
	_G(endgame) = 1;

	_G(exprow) = 0;
	expCounter = 0;

	_G(actor[34]) = _G(explosion);
	_G(actor[34])._active = false;
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
	expf[r / 8][r % 8] = true;
	int x = (EXPLOSION[r / 8][r % 8] % 20) * 16;
	int y = (EXPLOSION[r / 8][r % 8] / 20) * 16;
	_G(actor[34])._x = x;
	_G(actor[34])._y = y;
	_G(actor[34])._active = true;
	_G(actor[34])._nextFrame = 0;
	_G(actor[34])._currNumShots = 3;

	_G(scrn)._iconGrid[y / 16][x / 16] = _G(scrn)._backgroundColor;

	_G(endgame++);
	if (_G(endgame) > 32) {
		_G(actor[34])._active = false;
		_G(endgame) = 0;
	}
	return 1;
}

// Explode
int endGameMovement() {
	if (!_G(endgame))
		return 0;
	if (expCounter > 3) {
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
	expf[_G(exprow)][r] = true;
	const int x = (EXPLOSION[_G(exprow)][r] % 20) * 16;
	const int y = (EXPLOSION[_G(exprow)][r] / 20) * 16;
	_G(actor[34])._x = x;
	_G(actor[34])._y = y;
	_G(actor[34])._active = true;
	_G(actor[34])._nextFrame = 0;
	_G(actor[34])._currNumShots = 3;

	_G(scrn)._iconGrid[y / 16][x / 16] = _G(scrn)._backgroundColor;
	_G(scrn)._iconGrid[(y / 16) - 4][x / 16] = _G(scrn)._backgroundColor;

	_G(endgame++);
	if (_G(endgame) > 8) {
		_G(endgame) = 1;
		_G(exprow++);
		expCounter++;
		if (expCounter > 3) {
			memset(expf, 0, 32);
		}
	}

	return 1;
}

} // namespace Got
