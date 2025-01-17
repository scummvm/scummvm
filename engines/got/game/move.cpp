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

void next_frame(Actor *actr) {
	const int fcount = actr->_frameCount - 1;

	if (fcount <= 0) {
		actr->_nextFrame++;

		if (actr->_nextFrame > 3)
			actr->_nextFrame = 0;

		actr->_frameCount = actr->_frameSpeed;
	} else
		actr->_frameCount = fcount;
}

bool point_within(int x, int y, int x1, int y1, int x2, int y2) {
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

int reverse_direction(Actor *actr) {
	if (actr->_dir == 1)
		return 0;
	if (actr->_dir == 2)
		return 3;
	if (actr->_dir == 3)
		return 2;
	return 1;
}

void thor_shoots() {
	if ((_G(hammer)->_active != 1) && (!_G(hammer)->_dead) && (!_G(thor)->_shotCountdown)) {
		play_sound(SWISH, false);
		_G(thor)->_shotCountdown = 20;
		_G(hammer)->_active = 1;
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

int kill_good_guy(void) {
	if (!_G(killgg_inform) && !_G(thunder_flag)) {
		odin_speaks(2010, 0);
		_G(killgg_inform) = true;
	}

	add_score(-1000);
	return 0;
}

void actor_damaged(Actor *actr, int damage) {
	if (!_G(setup).skill)
		damage *= 2;
	else if (_G(setup).skill == 2)
		damage /= 2;

	if (!actr->_vulnerableCountdown && actr->_type != 3 && (actr->_solid & 0x7f) != 2) {
		actr->_vulnerableCountdown = STAMINA;
		if (damage >= actr->_health) {
			if (actr->_type != 4) {
				add_score(actr->_initHealth * 10);

			} else {
				kill_good_guy();
			}

			actor_destroyed(actr);
		} else {
			actr->_show = 10;
			actr->_health -= damage;
			actr->_moveCountdown += 8;
		}
	} else if (!actr->_vulnerableCountdown) {
		actr->_vulnerableCountdown = STAMINA;

		if (actr->_funcNum == 4) {
			switch_icons();
		}
		if (actr->_funcNum == 7) {
			rotate_arrows();
		}
	}
}

void thor_damaged(Actor *actr) {
	actr->_hitThor = 1;

	// If we're invincible, ignore any damage
	if (_G(cheats).freezeHealth)
		return;

	if (GAME3 && actr->_funcNum == 11) {
		if (actr->_talkCounter) {
			actr->_talkCounter--;
			return;
		}

		int t = actr->_type;
		actr->_type = 4;
		actor_speaks(actr, 0, 0);
		actr->_type = t;
		actr->_talkCounter = 30;
		return;
	}

	int damage = actr->_hitStrength;
	if (damage != 255) {
		if (!_G(setup).skill)
			damage /= 2;
		else if (_G(setup).skill == 2)
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
			_G(sound).play_sound(OW, 0);
			_G(thor)->_show = 10;
			_G(thor)->_health -= damage;
		}
	}
}

void actor_destroyed(Actor *actr) {
	if (actr->_actorNum > 2) {
		int pge = _G(pge);

		int x = actr->_lastX[pge ^ 1];
		int y = actr->_lastY[pge ^ 1];
		int x1 = actr->_lastX[pge];
		int y1 = actr->_lastY[pge];
		int r = actr->_dropRating;
		int n = actr->_actorNum;
		int t = actr->_type;

		if (actr->_funcNum == 255)
			actr->copyFixedAndPics(_G(explosion));
		else
			actr->copyFixedAndPics(_G(sparkle));

		actr->_type = t;
		actr->_actorNum = n;
		actr->_dropRating = r;
		actr->_x = x;
		actr->_y = y;
		actr->_lastX[pge] = x1;
		actr->_lastX[pge ^ 1] = x;
		actr->_lastY[pge] = y1;
		actr->_lastY[pge ^ 1] = y;
		actr->_moveCountdown = actr->_speed;
		actr->_active = 1;
		actr->_currNumShots = 3; // used to reverse explosion
		actr->_vulnerableCountdown = 255;
	} else {
		actr->_dead = 2;
		actr->_active = 0;
	}
}

int _actor_shoots(Actor *actr, int dir) {
	int t = actr->_shotType - 1;
	for (int i = MAX_ENEMIES + 3; i < MAX_ACTORS; i++) {
		if ((!_G(actor[i])._active) && (!_G(actor[i])._dead)) {
			Actor *act = &_G(actor[i]);
			*act = _G(shot[t]);
			int cx, cy;

			if (actr->_sizeY < act->_sizeY)
				cy = actr->_y - ((act->_sizeY - actr->_sizeY) / 2);
			else
				cy = actr->_y + ((actr->_sizeY - act->_sizeY) / 2);

			if (actr->_sizeX < act->_sizeX)
				cx = actr->_x - ((act->_sizeX - actr->_sizeX) / 2);
			else
				cx = actr->_x + ((actr->_sizeX - act->_sizeX) / 2);

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
			act->_lastX[0] = actr->_x;
			act->_lastX[1] = actr->_x;
			act->_lastY[0] = cy;
			act->_lastY[1] = cy;
			act->_active = 1;
			act->_creator = actr->_actorNum;
			act->_moveCount = act->_numMoves;
			act->_dead = 0;
			actr->_shotActor = i;
			actr->_currNumShots++;
			actr->_shotCountdown = 20;
			_G(shot_ok) = false;
			return 1;
		}
	}
	return 0;
}

void actor_always_shoots(Actor *actr, int dir) {
	_actor_shoots(actr, dir);
}

int actor_shoots(Actor *actr, int dir) {
	int i;

	int cx = (actr->_x + (actr->_sizeX / 2)) >> 4;
	int cy = ((actr->_y + actr->_sizeY) - 2) >> 4;

	int tx = _G(thor)->_centerX;
	int ty = _G(thor)->_centerY;

	int icn = 140;

	if (_G(shot[actr->_shotType - 1])._flying)
		icn = 80;

	switch (dir) {
	case 0:
		for (i = ty + 1; i <= cy; i++) {
			if (_G(scrn).icon[i][cx] < icn)
				return 0;
		}
		break;
	case 1:
		for (i = cy; i <= ty; i++) {
			if (_G(scrn).icon[i][cx] < icn)
				return 0;
		}
		break;
	case 2:
		for (i = tx; i < cx; i++) {
			if (_G(scrn).icon[cy][i] < icn)
				return 0;
		}
		break;
	case 3:
		for (i = cx; i < tx; i++) {
			if (_G(scrn).icon[cy][i] < icn)
				return 0;
		}
		break;
	}
	return _actor_shoots(actr, dir);
}

void move_actor(Actor *actr) {
	if (actr->_vulnerableCountdown != 0)
		actr->_vulnerableCountdown--;
	if (actr->_shotCountdown != 0)
		actr->_shotCountdown--;
	if (actr->_show != 0)
		actr->_show--;

	if (!actr->_shotCountdown && _G(shot_ok)) {
		if (actr->_numShotsAllowed) {
			if (actr->_currNumShots < actr->_numShotsAllowed) {
				shot_pattern_func[actr->_shotPattern](actr);
			}
		}
	}

	const int scount = actr->_moveCountdown - 1;
	if (scount <= 0) {
		if (!actr->_moveCounter)
			actr->_moveCountdown = actr->_speed;
		else
			actr->_moveCountdown = (actr->_speed << 1);
		
		int i;

		if (actr->_type == 3)
			i = shot_movement_func[actr->_moveType](actr);
		else
			i = movement_func[actr->_moveType](actr);
		
		if (actr->_directions == 2)
			i &= 1;
		
		if (i != actr->_dir)
			actr->_dir = i;

		if (actr->_moveType == 0 && _G(current_level) != _G(new_level) && _G(shield_on)) {
			_G(actor[2])._x = actr->_x - 2;
			if (_G(actor[2])._x < 0)
				_G(actor[2])._x = 0;
			_G(actor[2])._y = actr->_y;
			_G(actor[2])._lastX[0] = _G(actor[2])._x;
			_G(actor[2])._lastX[1] = _G(actor[2])._x;
			_G(actor[2])._lastY[0] = _G(actor[2])._y;
			_G(actor[2])._lastY[1] = _G(actor[2])._y;
		}
	} else
		actr->_moveCountdown = scount;

	actr->_x &= 0xfffe;
}

} // namespace Got
