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

#include "got/game/move.h"
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/move_patterns.h"
#include "got/game/shot_movement.h"
#include "got/game/shot_pattern.h"
#include "got/game/status.h"
#include "got/vars.h"

namespace Got {

void nextFrame(Actor *actor) {
	const int fcount = actor->_frameCount - 1;

	if (fcount <= 0) {
		actor->_nextFrame++;

		if (actor->_nextFrame > 3)
			actor->_nextFrame = 0;

		actor->_frameCount = actor->_frameSpeed;
	} else
		actor->_frameCount = fcount;
}

bool pointWithin(int x, int y, int x1, int y1, int x2, int y2) {
	return ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2));
}

bool overlap(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
	if ((x1 >= x3) && (x1 <= x4) && (y1 >= y3) && (y1 <= y4))
		return true;
	if ((x2 >= x3) && (x2 <= x4) && (y2 >= y3) && (y2 <= y4))
		return true;
	if ((x1 >= x3) && (x1 <= x4) && (y2 >= y3) && (y2 <= y4))
		return true;
	if ((x2 >= x3) && (x2 <= x4) && (y1 >= y3) && (y1 <= y4))
		return true;
	if ((x3 >= x1) && (x3 <= x2) && (y3 >= y1) && (y3 <= y2))
		return true;
	if ((x4 >= x1) && (x4 <= x2) && (y4 >= y1) && (y4 <= y2))
		return true;
	if ((x3 >= x1) && (x3 <= x2) && (y4 >= y1) && (y4 <= y2))
		return true;
	if ((x4 >= x1) && (x4 <= x2) && (y3 >= y1) && (y3 <= y2))
		return true;
	return false;
}

int reverseDirection(Actor *actor) {
	if (actor->_dir == 1)
		return 0;
	if (actor->_dir == 2)
		return 3;
	if (actor->_dir == 3)
		return 2;
	return 1;
}

void thorShoots() {
	if (!_G(hammer)->_active && (!_G(hammer)->_dead) && (!_G(thor)->_shotCountdown)) {
		play_sound(SWISH, false);
		_G(thor)->_shotCountdown = 20;
		_G(hammer)->_active = true;
		_G(hammer)->_dir = _G(thor)->_dir;
		_G(hammer)->_lastDir = _G(thor)->_dir;
		_G(hammer)->_x = _G(thor)->_x;
		_G(hammer)->_y = _G(thor)->_y + 2;
		_G(hammer)->_moveType = 2;
		_G(hammer)->_nextFrame = 0;
		_G(hammer)->_lastX[0] = _G(hammer)->_x;
		_G(hammer)->_lastX[1] = _G(hammer)->_x;
		_G(hammer)->_lastY[0] = _G(hammer)->_y;
		_G(hammer)->_lastY[1] = _G(hammer)->_y;
	}
}

int killGoodGuy() {
	if (!_G(killgg_inform) && !_G(thunder_flag)) {
		odinSpeaks(2010, 0);
		_G(killgg_inform) = true;
	}

	add_score(-1000);
	return 0;
}

void actorDamaged(Actor *actor, int damage) {
	if (!_G(setup)._difficultyLevel)
		damage *= 2;
	else if (_G(setup)._difficultyLevel == 2)
		damage /= 2;

	if (!actor->_vulnerableCountdown && actor->_type != 3 && (actor->_solid & 0x7f) != 2) {
		actor->_vulnerableCountdown = STAMINA;
		if (damage >= actor->_health) {
			if (actor->_type != 4) {
				add_score(actor->_initHealth * 10);

			} else {
				killGoodGuy();
			}

			actorDestroyed(actor);
		} else {
			actor->_show = 10;
			actor->_health -= damage;
			actor->_moveCountdown += 8;
		}
	} else if (!actor->_vulnerableCountdown) {
		actor->_vulnerableCountdown = STAMINA;

		if (actor->_funcNum == 4) {
			switchIcons();
		}
		if (actor->_funcNum == 7) {
			rotateArrows();
		}
	}
}

void thorDamaged(Actor *actor) {
	actor->_hitThor = true;

	// If we're invincible, ignore any damage
	if (_G(cheats).freezeHealth)
		return;

	if (GAME3 && actor->_funcNum == 11) {
		if (actor->_talkCounter) {
			actor->_talkCounter--;
			return;
		}

		const int currType = actor->_type;
		actor->_type = 4;
		actorSpeaks(actor, 0, 0);
		actor->_type = currType;
		actor->_talkCounter = 30;
		return;
	}

	int damage = actor->_hitStrength;
	if (damage != 255) {
		if (!_G(setup)._difficultyLevel)
			damage /= 2;
		else if (_G(setup)._difficultyLevel == 2)
			damage *= 2;
	}
	if ((!_G(thor)->_vulnerableCountdown && !_G(shield_on)) || damage == 255) {
		if (damage >= _G(thor)->_health) {
			_G(thor)->_vulnerableCountdown = 40;
			_G(thor)->_show = 0;
			_G(thor)->_health = 0;
			_G(exit_flag) = 2;
			g_events->send(GameMessage("THOR_DIES"));
		} else if (damage) {
			_G(thor)->_vulnerableCountdown = 40;
			_G(sound).play_sound(OW, false);
			_G(thor)->_show = 10;
			_G(thor)->_health -= damage;
		}
	}
}

void actorDestroyed(Actor *actor) {
	if (actor->_actorNum > 2) {
		const int curPage = _G(pge);

		const int x = actor->_lastX[curPage ^ 1];
		const int y = actor->_lastY[curPage ^ 1];
		const int x1 = actor->_lastX[curPage];
		const int y1 = actor->_lastY[curPage];
		const int r = actor->_dropRating;
		const int n = actor->_actorNum;
		const int t = actor->_type;

		if (actor->_funcNum == 255)
			actor->copyFixedAndPics(_G(explosion));
		else
			actor->copyFixedAndPics(_G(sparkle));

		actor->_type = t;
		actor->_actorNum = n;
		actor->_dropRating = r;
		actor->_x = x;
		actor->_y = y;
		actor->_lastX[curPage] = x1;
		actor->_lastX[curPage ^ 1] = x;
		actor->_lastY[curPage] = y1;
		actor->_lastY[curPage ^ 1] = y;
		actor->_moveCountdown = actor->_speed;
		actor->_active = true;
		actor->_currNumShots = 3; // used to reverse explosion
		actor->_vulnerableCountdown = 255;
	} else {
		actor->_dead = 2;
		actor->_active = false;
	}
}

int _actor_shoots(Actor *actor, int dir) {
	const int t = actor->_shotType - 1;
	for (int i = MAX_ENEMIES + 3; i < MAX_ACTORS; i++) {
		if ((!_G(actor[i])._active) && (!_G(actor[i])._dead)) {
			Actor *act = &_G(actor[i]);
			*act = _G(shot[t]);
			int cx, cy;

			if (actor->_sizeY < act->_sizeY)
				cy = actor->_y - ((act->_sizeY - actor->_sizeY) / 2);
			else
				cy = actor->_y + ((actor->_sizeY - act->_sizeY) / 2);

			if (actor->_sizeX < act->_sizeX)
				cx = actor->_x - ((act->_sizeX - actor->_sizeX) / 2);
			else
				cx = actor->_x + ((actor->_sizeX - act->_sizeX) / 2);

			if (cy > 174)
				cy = 174;

			if (cx > 304)
				cx = 304;

			act->_x = cx;
			act->_y = cy;
			act->_lastDir = dir;
			act->_nextFrame = 0;
			act->_dir = dir;
			if (act->_directions == 1)
				act->_dir = 0;
			else if (act->_directions == 4 && act->_framesPerDirection == 1) {
				act->_nextFrame = dir;
				act->_dir = 0;
			}
			act->_frameCount = act->_frameSpeed;
			act->_moveCountdown = act->_speed;
			act->_lastX[0] = cx;
			act->_lastX[1] = cx;
			act->_lastX[0] = actor->_x;
			act->_lastX[1] = actor->_x;
			act->_lastY[0] = cy;
			act->_lastY[1] = cy;
			act->_active = true;
			act->_creator = actor->_actorNum;
			act->_moveCount = act->_numMoves;
			act->_dead = 0;
			actor->_shotActor = i;
			actor->_currNumShots++;
			actor->_shotCountdown = 20;
			_G(shot_ok) = false;
			return 1;
		}
	}
	return 0;
}

void actorAlwaysShoots(Actor *actor, int dir) {
	_actor_shoots(actor, dir);
}

int actorShoots(Actor *actor, int dir) {
	const int cx = (actor->_x + (actor->_sizeX / 2)) >> 4;
	const int cy = ((actor->_y + actor->_sizeY) - 2) >> 4;

	const int tx = _G(thor)->_centerX;
	const int ty = _G(thor)->_centerY;

	int icn = 140;

	if (_G(shot[actor->_shotType - 1])._flying)
		icn = 80;

	switch (dir) {
	case 0:
		for (int i = ty + 1; i <= cy; i++) {
			if (_G(scrn)._iconGrid[i][cx] < icn)
				return 0;
		}
		break;
		
	case 1:
		for (int i = cy; i <= ty; i++) {
			if (_G(scrn)._iconGrid[i][cx] < icn)
				return 0;
		}
		break;
		
	case 2:
		for (int i = tx; i < cx; i++) {
			if (_G(scrn)._iconGrid[cy][i] < icn)
				return 0;
		}
		break;

	case 3:
		for (int i = cx; i < tx; i++) {
			if (_G(scrn)._iconGrid[cy][i] < icn)
				return 0;
		}
		break;

	default:
		break;
	}
	
	return _actor_shoots(actor, dir);
}

void moveActor(Actor *actor) {
	if (actor->_vulnerableCountdown != 0)
		actor->_vulnerableCountdown--;
	if (actor->_shotCountdown != 0)
		actor->_shotCountdown--;
	if (actor->_show != 0)
		actor->_show--;

	if (!actor->_shotCountdown && _G(shot_ok)) {
		if (actor->_numShotsAllowed) {
			if (actor->_currNumShots < actor->_numShotsAllowed) {
				shot_pattern_func[actor->_shotPattern](actor);
			}
		}
	}

	const int scount = actor->_moveCountdown - 1;
	if (scount <= 0) {
		if (!actor->_moveCounter)
			actor->_moveCountdown = actor->_speed;
		else
			actor->_moveCountdown = (actor->_speed << 1);
		
		int i;

		if (actor->_type == 3)
			i = shot_movement_func[actor->_moveType](actor);
		else
			i = _movementFunc[actor->_moveType](actor);
		
		if (actor->_directions == 2)
			i &= 1;
		
		if (i != actor->_dir)
			actor->_dir = i;

		if (actor->_moveType == 0 && _G(current_level) != _G(new_level) && _G(shield_on)) {
			_G(actor[2])._x = actor->_x - 2;
			if (_G(actor[2])._x < 0)
				_G(actor[2])._x = 0;
			_G(actor[2])._y = actor->_y;
			_G(actor[2])._lastX[0] = _G(actor[2])._x;
			_G(actor[2])._lastX[1] = _G(actor[2])._x;
			_G(actor[2])._lastY[0] = _G(actor[2])._y;
			_G(actor[2])._lastY[1] = _G(actor[2])._y;
		}
	} else
		actor->_moveCountdown = scount;

	actor->_x &= 0xfffe;
}

} // namespace Got
