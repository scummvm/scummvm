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

#include "got/game/shot_movement.h"
#include "got/events.h"
#include "got/game/move.h"
#include "got/game/move_patterns.h"
#include "got/game/object.h"
#include "got/vars.h"

namespace Got {

int shotMovementNone(Actor *actor);
int shotMovementOne(Actor *actor);
int shotMovementTwo(Actor *actor);
int shotMovementThree(Actor *actor);
int shotMovementFour(Actor *actor);
int shotMovementFive(Actor *actor);
int shotMovementSix(Actor *actor);
int shotMovementSeven(Actor *actor);
int shotMovementEight(Actor *actor);
int shotMovementNine(Actor *actor);
int shotMovementTen(Actor *actor);
int shotMovementEleven(Actor *actor);
int shotMovementTwelve(Actor *actor);
int shotMovementThirteen(Actor *actor);

int (*const shotMovementFunc[])(Actor *actor) = {
	shotMovementNone,
	shotMovementOne,
	shotMovementTwo,
	shotMovementThree,
	shotMovementFour,
	shotMovementFive,
	shotMovementSix,
	shotMovementSeven,
	shotMovementEight,
	shotMovementNine,
	shotMovementTen,
	shotMovementEleven,
	shotMovementTwelve,
	shotMovementThirteen
};

void next_shot_frame(Actor *actor) {
	if (actor->_directions == 4 && actor->_framesPerDirection == 1) {
		actor->_nextFrame = actor->_lastDir;
		actor->_dir = 0;
	} else {
		const int fcount = actor->_frameCount - 1;

		if (fcount <= 0) {
			actor->_nextFrame++;
			if (actor->_nextFrame > 3)
				actor->_nextFrame = 0;

			actor->_frameCount = actor->_frameSpeed;
		} else
			actor->_frameCount = fcount;
	}
}

// Boss - snake
int shotMovementNone(Actor *actor) {
	actor->_temp3--;
	if (!actor->_temp3) {
		actorDestroyed(actor);
		if (_G(actor[actor->_creator])._currNumShots)
			_G(actor[actor->_creator])._currNumShots--;
	}

	next_shot_frame(actor);
	if (actor->_directions == 1)
		return 0;

	return actor->_lastDir;
}

int shotMovementOne(Actor *actor) {
	int x1 = 0, y1 = 0;

	const int d = actor->_lastDir;

	switch (d) {
	case 0:
		x1 = actor->_x;
		y1 = actor->_y - 2;
		break;
		
	case 1:
		x1 = actor->_x;
		y1 = actor->_y + 2;
		break;
		
	case 2:
		x1 = actor->_x - 2;
		y1 = actor->_y;
		break;
		
	case 3:
		x1 = actor->_x + 2;
		y1 = actor->_y;
		break;
		
	default:
		break;
	}

	if (!checkMove3(x1, y1, actor)) {
		actorDestroyed(actor);
		if (_G(actor[actor->_creator])._currNumShots)
			_G(actor[actor->_creator])._currNumShots--;
	} else {
		next_shot_frame(actor);
		actor->_lastDir = d;
	}

	if (actor->_directions == 1)
		return 0;
	if (actor->_directions == 4 && actor->_framesPerDirection == 1)
		return 0;

	return d;
}

int shotMovementTwo(Actor *actor) {
	int x1 = 0, y1 = 0;

	const int d = actor->_lastDir;

	switch (d) {
	case 0:
		x1 = actor->_x;
		y1 = actor->_y - 2;
		break;
		
	case 1:
		x1 = actor->_x;
		y1 = actor->_y + 2;
		break;
		
	case 2:
		x1 = actor->_x - 2;
		y1 = actor->_y;
		break;
		
	case 3:
		x1 = actor->_x + 2;
		y1 = actor->_y;
		break;
		
	default:
		break;
	}
	if (!checkMove4(x1, y1, actor)) {
		actorDestroyed(actor);
		if (_G(actor[actor->_creator])._currNumShots)
			_G(actor[actor->_creator])._currNumShots--;
	} else {
		next_shot_frame(actor);
		actor->_lastDir = d;
	}
	if (actor->_directions == 1)
		return 0;
	if (actor->_directions == 4 && actor->_framesPerDirection == 1)
		return 0;
	return d;
}

// Serpent fire
int shotMovementThree(Actor *actor) {
	const int d = actor->_lastDir;

	actor->_x -= 2;
	actor->_temp3--;
	actor->_temp4--;

	if (overlap(_G(thor_x1) - 1, _G(thor_y1) - 1, _G(thor_x2) + 1, _G(thor_y2) + 1,
				actor->_x, actor->_y, actor->_x + 15, actor->_y + 15)) {
		actor->_moveType = 0;
		actor->_speed = 6;
		thorDamaged(actor);
		actor->_x += 2;
	}
	if (!actor->_temp4) {
		actor->_temp4 = actor->_temp5;
		actor->_speed++;
		if (actor->_speed > 6)
			actor->_moveType = 0;
	}
	if (!actor->_temp3) {
		actorDestroyed(actor);
		if (_G(actor[actor->_creator])._currNumShots)
			_G(actor[actor->_creator])._currNumShots--;
	} else {
		next_shot_frame(actor);
		actor->_lastDir = d;
	}
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Wraith balls
int shotMovementFour(Actor *actor) {
	if (actor->_temp1) {
		actor->_temp1--;
		if (!actor->_temp1) {
			actorDestroyed(actor);
			_G(apple_drop++);
			if (_G(apple_drop) == 4) {
				if (dropObject(actor, 5))
					_G(apple_drop) = 0;
				else
					_G(apple_drop) = 3;
			} else
				dropObject(actor, 3);
			return 0;
		}
	}
	if (overlap(_G(thor)->_x - 1, _G(thor)->_y - 1, _G(thor_x2) + 1, _G(thor_y2) + 1,
				actor->_x, actor->_y, actor->_x + 15, actor->_y + 15)) {
		thorDamaged(actor);
		actorDestroyed(actor);
		return 0;
	}

	int x1 = actor->_x;
	int y1 = actor->_y;
	int yd = 0;
	int xd = 0;
	int d = actor->_lastDir;

	if ((x1 > (_G(thor_x1)) + 1))
		xd = -2;
	else if ((x1 < (_G(thor_x1)) - 1))
		xd = 2;

	if (actor->_actorNum == 1) {
		if (y1 < (_G(thor_y1) - 6))
			yd = 2;
		else if (y1 > (_G(thor_y1) - 6))
			yd = -2;
	} else {
		if (y1 < (_G(thor_real_y1) - 1))
			yd = 2;
		else if (y1 > (_G(thor_real_y1) + 1))
			yd = -2;
	}

	if (xd && yd) {
		if (xd == -2 && ABS(yd) == 2)
			d = 2;
		else if (xd == 2 && ABS(yd) == 2)
			d = 3;
		
		x1 += xd;
		y1 += yd;
		if (checkMove3(x1, y1, actor)) {
			nextFrame(actor);
			actor->_lastDir = d;
			if (actor->_directions == 1)
				return 0;
			return d;
		}
	} else if (xd == 0 && yd == 2)
		d = 1;
	else if (xd == 0 && yd == -2)
		d = 0;
	else if (xd == 2 && yd == 0)
		d = 3;
	else if (xd == -2 && yd == 0)
		d = 2;

	x1 = actor->_x;
	y1 = actor->_y;
	actor->_toggle ^= 1;

	if (actor->_toggle) {
		if (xd) {
			x1 += xd;
			if (checkMove3(x1, y1, actor)) {
				if (xd > 0)
					d = 3;
				else
					d = 2;
				nextFrame(actor);
				actor->_lastDir = d;
				if (actor->_directions == 1)
					return 0;
				return d;
			}
			x1 = actor->_x;
		}
		if (yd) {
			y1 += yd;
			if (checkMove3(x1, y1, actor)) {
				if (yd > 0)
					d = 1;
				else
					d = 0;
				nextFrame(actor);
				actor->_lastDir = d;
				if (actor->_directions == 1)
					return 0;
				return d;
			}
		}
	} else {
		if (yd) {
			y1 += yd;
			if (checkMove3(x1, y1, actor)) {
				if (yd > 0)
					d = 1;
				else
					d = 0;
				nextFrame(actor);
				actor->_lastDir = d;
				if (actor->_directions == 1)
					return 0;
				return d;
			}
			y1 = actor->_y;
		}
		if (xd) {
			x1 += xd;
			if (checkMove3(x1, y1, actor)) {
				if (xd > 0)
					d = 3;
				else
					d = 2;
				nextFrame(actor);
				actor->_lastDir = d;
				if (actor->_directions == 1)
					return 0;
				return d;
			}
		}
	}
	checkMove3(actor->_x, actor->_y, actor);
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// No move, frame cycle
int shotMovementFive(Actor *actor) {
	next_shot_frame(actor);
	if (actor->_directions == 1)
		return 0;
	return actor->_lastDir;
}

// Wraith spots
int shotMovementSix(Actor *actor) {
	actor->_temp1--;
	if (!actor->_temp1) {
		actorDestroyed(actor);
		if (_G(actor[actor->_creator])._currNumShots)
			_G(actor[actor->_creator])._currNumShots--;
	} else
		next_shot_frame(actor);
	if (actor->_directions == 1)
		return 0;
	return actor->_lastDir;
}

#define YV actor->_i1
#define YC actor->_i2
#define BC actor->_i3
#define YD actor->_i4
#define XD actor->_i5
#define XC actor->_i6
#define IV 100
#define IC 50

// Skull drop
int shotMovementSeven(Actor *actor) {
	if (actor->_temp3) {
		actor->_temp3--;
		goto done;
	}
	if (overlap(actor->_x, actor->_y, actor->_x + actor->_sizeX, actor->_y + actor->_sizeY,
				_G(thor)->_x, _G(thor)->_y + 4, _G(thor)->_x + 15, _G(thor)->_y + 15))
		thorDamaged(actor);

	actor->_temp2++;
	if (actor->_temp2 > 2) {
		if (actor->_temp4)
			actor->_temp4--;
		actor->_temp2 = 0;
	}
	actor->_temp3 = actor->_temp4;

	actor->_y += 2;
	if (actor->_y > 160 - 36) {
		actor->_x += (4 - g_events->getRandomNumber(8));
		actor->_moveType = 8;
		YV = IV;
		YC = 0;
		BC = IC;
		YD = 0;
		XC = 3;
		if (actor->_x < 150)
			XD = 1;
		else
			XD = 0;
	}

done:
	next_shot_frame(actor);
	if (actor->_directions == 1)
		return 0;
	return actor->_lastDir;
}

// Skull bounce
int shotMovementEight(Actor *actor) {
	int x = actor->_x;
	int y = actor->_y;

	XC--;
	if (!XC) {
		XC = 3;
		if (XD)
			x += 2;
		else
			x -= 2;
	}
	YC += YV;
	if (YC > (IV - 1)) {
		if (!YD) {
			YV -= 8;
			YC -= IV;
			y -= 2;
		} else {
			YV += 8;
			YC -= IV;
			y += 2;
		}
	}
	if (YV < 0) {
		YV = 0;
		BC = 1;
	}
	if (YV > IV) {
		YV = IV;
		BC = 1;
	}
	BC--;
	if (!BC) {
		BC = IC;
		if (YD)
			YV = IV;
		YD ^= 1;
	}
	if (y > 164)
		y = 164;
	//   8       311
	if (x < 1 || x > (319 - actor->_sizeX)) {
		if (!actor->_dead)
			if (_G(actor[actor->_creator])._currNumShots)
				_G(actor[actor->_creator])._currNumShots--;
		actorDestroyed(actor);
	}
	if (overlap(actor->_x, actor->_y, actor->_x + actor->_sizeX, actor->_y + actor->_sizeY,
				_G(thor)->_x, _G(thor)->_y + 4, _G(thor)->_x + 15, _G(thor)->_y + 15))
		thorDamaged(actor);
	actor->_x = x;
	actor->_y = y;

	//done:
	next_shot_frame(actor);
	if (actor->_directions == 1)
		return 0;
	return actor->_lastDir;
}

// Skull explode
int shotMovementNine(Actor *actor) {
	actor->_nextFrame++;
	if (actor->_nextFrame == 3) {
		_G(actor[actor->_creator])._currNumShots--;
		actorDestroyed(actor);
		return 0;
	}
	if (actor->_directions == 1)
		return 0;
	return actor->_lastDir;
}

// Skull - stalagtites
int shotMovementTen(Actor *actor) {
	bool check = false;
	actor->_y += 2;

	if (overlap(actor->_x, actor->_y, actor->_x + actor->_sizeX, actor->_y + actor->_sizeY,
				_G(thor)->_x, _G(thor)->_y + 4, _G(thor)->_x + 15, _G(thor)->_y + 15)) {
		thorDamaged(actor);
		check = true;
	}
	if (actor->_y > 160 || check) {
		if (_G(actor[actor->_creator])._currNumShots)
			_G(actor[actor->_creator])._currNumShots--;
		actorDestroyed(actor);
	}

	return 0;
}

#undef YD
#undef XD
#define YA actor->_i1
#define XA actor->_i2
#define YD actor->_i3
#define XD actor->_i4
#define DIR actor->_i5
#define CNT actor->_i6

void calc_angle(const int x1, const int y1, const int x2, const int y2, Actor *actor) {
	if (x1 < x2) {
		XA = -2;
		XD = x2 - x1;
	} else if (x1 > x2) {
		XA = 2;
		XD = x1 - x2;
	} else {
		XA = 0;
		XD = 0;
	}

	if (y1 < y2) {
		YA = -2;
		YD = y2 - y1;
	} else if (y1 > y2) {
		YA = 2;
		YD = y1 - y2;
	} else {
		YA = 0;
		YD = 0;
	}

	if (YD >= XD)
		DIR = 1;
	else
		DIR = 0;
	CNT = 0;
}

// Angle throw
int shotMovementEleven(Actor *actor) {
	int x1 = actor->_x;
	int y1 = actor->_y;

	if (!actor->_temp1) {
		calc_angle(_G(thor_x1), _G(thor_real_y1), x1, y1, actor);
		actor->_temp1 = 1;
	}

	if (DIR) {
		y1 += YA;
		CNT += XD;
		if (CNT >= YD) {
			x1 += XA;
			CNT -= YD;
		}
	} else {
		x1 += XA;
		CNT += YD;
		if (CNT >= XD) {
			y1 += YA;
			CNT -= XD;
		}
	}

	if (!checkMove3(x1, y1, actor)) {
		if (_G(actor[actor->_creator])._currNumShots)
			_G(actor[actor->_creator])._currNumShots--;
		actorDestroyed(actor);
	} else
		nextFrame(actor);

	if (actor->_directions == 1)
		return 0;
	return actor->_lastDir;
}

// Angle throw Loki
int shotMovementTwelve(Actor *actor) {
	int x1 = actor->_x;
	int y1 = actor->_y;

	if (!actor->_temp5) {
		calc_angle(_G(thor_x1), _G(thor_real_y1), x1, y1, actor);
		actor->_temp5 = 1;
	}

	if (DIR) {
		y1 += YA;
		CNT += XD;
		if (CNT >= YD) {
			x1 += XA;
			CNT -= YD;
		}
	} else {
		x1 += XA;
		CNT += YD;
		if (CNT >= XD) {
			y1 += YA;
			CNT -= XD;
		}
	}

	if (x1 < 16 || x1 > 287 || y1 < 16 || y1 > 159) {
		calc_angle(g_events->getRandomNumber(319),
				   g_events->getRandomNumber(191), x1, y1, actor);
		actor->_moveType = 13;
		actor->_temp4 = 240;
		actor->_nextFrame = 2;
	} else {
		if (overlap(x1 + 2, y1 + 2, x1 + 14, y1 + 14, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
			thorDamaged(actor);
		}
		actor->_x = x1;
		actor->_y = y1;
	}

	const int fcount = actor->_frameCount - 1;
	if (fcount <= 0) {
		actor->_nextFrame++;
		if (actor->_nextFrame > 1)
			actor->_nextFrame = 0;
		actor->_frameCount = actor->_frameSpeed;
	} else
		actor->_frameCount = fcount;
	
	if (actor->_directions == 1)
		return 0;
	return actor->_lastDir;
}

// Angle throw Loki-2
int shotMovementThirteen(Actor *actor) {
	int x1 = actor->_x;
	int y1 = actor->_y;

	if (!actor->_temp5) {
		calc_angle(_G(thor_x1), _G(thor_real_y1), x1, y1, actor);
		actor->_temp5 = 1;
	}

	if (DIR) {
		y1 += YA;
		CNT += XD;
		if (CNT >= YD) {
			x1 += XA;
			CNT -= YD;
		}
	} else {
		x1 += XA;
		CNT += YD;
		if (CNT >= XD) {
			y1 += YA;
			CNT -= XD;
		}
	}

	if (actor->_temp4)
		actor->_temp4--;

	if (!actor->_temp4) {
		if (_G(actor[actor->_creator])._currNumShots)
			_G(actor[actor->_creator])._currNumShots--;
		actorDestroyed(actor);
		_G(apple_drop++);
		if (_G(apple_drop) > 4) {
			if (dropObject(actor, 5))
				_G(apple_drop) = 0;
			else
				_G(apple_drop) = 4;
		} else
			dropObject(actor, 4);
		return 0;
	}

	if (x1 < 16 || x1 > 287 || y1 < 16 || y1 > 159) {
		if (x1 < 16 || x1 > 287)
			XA = 0 - XA;
		else
			YA = 0 - YA;
	} else {
		if (overlap(x1 + 4, y1 + 4, x1 + 12, y1 + 12, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
			thorDamaged(actor);
		}
		actor->_x = x1;
		actor->_y = y1;
	}

	const int fcount = actor->_frameCount - 1;
	if (fcount <= 0) {
		actor->_nextFrame++;
		if (actor->_nextFrame > 3)
			actor->_nextFrame = 2;
		actor->_frameCount = actor->_frameSpeed;
	} else
		actor->_frameCount = fcount;
	
	if (actor->_directions == 1)
		return 0;

	return actor->_lastDir;
}

} // namespace Got
