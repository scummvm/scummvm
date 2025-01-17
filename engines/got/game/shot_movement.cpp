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

int shot_movement_none(Actor *actr);
int shot_movement_one(Actor *actr);
int shot_movement_two(Actor *actr);
int shot_movement_three(Actor *actr);
int shot_movement_four(Actor *actr);
int shot_movement_five(Actor *actr);
int shot_movement_six(Actor *actr);
int shot_movement_seven(Actor *actr);
int shot_movement_eight(Actor *actr);
int shot_movement_nine(Actor *actr);
int shot_movement_ten(Actor *actr);
int shot_movement_eleven(Actor *actr);
int shot_movement_twelve(Actor *actr);
int shot_movement_thirteen(Actor *actr);

int (*const shot_movement_func[])(Actor *actr) = {
	shot_movement_none,
	shot_movement_one,
	shot_movement_two,
	shot_movement_three,
	shot_movement_four,
	shot_movement_five,
	shot_movement_six,
	shot_movement_seven,
	shot_movement_eight,
	shot_movement_nine,
	shot_movement_ten,
	shot_movement_eleven,
	shot_movement_twelve,
	shot_movement_thirteen};

void next_shot_frame(Actor *actr) {
	if (actr->_directions == 4 && actr->_framesPerDirection == 1) {
		actr->_nextFrame = actr->_lastDir;
		actr->_dir = 0;
	} else {
		const int fcount = actr->_frameCount - 1;

		if (fcount <= 0) {
			actr->_nextFrame++;
			if (actr->_nextFrame > 3)
				actr->_nextFrame = 0;

			actr->_frameCount = actr->_frameSpeed;
		} else
			actr->_frameCount = fcount;
	}
}

// Boss - snake
int shot_movement_none(Actor *actr) {
	actr->_temp3--;
	if (!actr->_temp3) {
		actor_destroyed(actr);
		if (_G(actor[actr->_creator])._currNumShots)
			_G(actor[actr->_creator])._currNumShots--;
	}

	next_shot_frame(actr);
	if (actr->_directions == 1)
		return 0;

	return actr->_lastDir;
}

int shot_movement_one(Actor *actr) {
	int x1 = 0, y1 = 0;

	int d = actr->_lastDir;

	switch (d) {
	case 0:
		x1 = actr->_x;
		y1 = actr->_y - 2;
		break;
	case 1:
		x1 = actr->_x;
		y1 = actr->_y + 2;
		break;
	case 2:
		x1 = actr->_x - 2;
		y1 = actr->_y;
		break;
	case 3:
		x1 = actr->_x + 2;
		y1 = actr->_y;
		break;
	}

	if (!check_move3(x1, y1, actr)) {
		actor_destroyed(actr);
		if (_G(actor[actr->_creator])._currNumShots)
			_G(actor[actr->_creator])._currNumShots--;
	} else {
		next_shot_frame(actr);
		actr->_lastDir = d;
	}

	if (actr->_directions == 1)
		return 0;
	if (actr->_directions == 4 && actr->_framesPerDirection == 1)
		return 0;

	return d;
}

int shot_movement_two(Actor *actr) {
	int x1 = 0, y1 = 0;

	int d = actr->_lastDir;

	switch (d) {
	case 0:
		x1 = actr->_x;
		y1 = actr->_y - 2;
		break;
	case 1:
		x1 = actr->_x;
		y1 = actr->_y + 2;
		break;
	case 2:
		x1 = actr->_x - 2;
		y1 = actr->_y;
		break;
	case 3:
		x1 = actr->_x + 2;
		y1 = actr->_y;
		break;
	}
	if (!check_move4(x1, y1, actr)) {
		actor_destroyed(actr);
		if (_G(actor[actr->_creator])._currNumShots)
			_G(actor[actr->_creator])._currNumShots--;
	} else {
		next_shot_frame(actr);
		actr->_lastDir = d;
	}
	if (actr->_directions == 1)
		return 0;
	if (actr->_directions == 4 && actr->_framesPerDirection == 1)
		return 0;
	return d;
}

// Serpent fire
int shot_movement_three(Actor *actr) {
	int d = actr->_lastDir;

	actr->_x -= 2;
	actr->_temp3--;
	actr->_temp4--;

	if (overlap(_G(thor_x1) - 1, _G(thor_y1) - 1, _G(thor_x2) + 1, _G(thor_y2) + 1,
				actr->_x, actr->_y, actr->_x + 15, actr->_y + 15)) {
		actr->_moveType = 0;
		actr->_speed = 6;
		thor_damaged(actr);
		actr->_x += 2;
	}
	if (!actr->_temp4) {
		actr->_temp4 = actr->_temp5;
		actr->_speed++;
		if (actr->_speed > 6)
			actr->_moveType = 0;
	}
	if (!actr->_temp3) {
		actor_destroyed(actr);
		if (_G(actor[actr->_creator])._currNumShots)
			_G(actor[actr->_creator])._currNumShots--;
	} else {
		next_shot_frame(actr);
		actr->_lastDir = d;
	}
	if (actr->_directions == 1)
		return 0;
	return d;
}

// Wraith balls
int shot_movement_four(Actor *actr) {
	if (actr->_temp1) {
		actr->_temp1--;
		if (!actr->_temp1) {
			actor_destroyed(actr);
			_G(apple_drop++);
			if (_G(apple_drop) == 4) {
				if (_drop_obj(actr, 5))
					_G(apple_drop) = 0;
				else
					_G(apple_drop) = 3;
			} else
				_drop_obj(actr, 3);
			return 0;
		}
	}
	if (overlap(_G(thor)->_x - 1, _G(thor)->_y - 1, _G(thor_x2) + 1, _G(thor_y2) + 1,
				actr->_x, actr->_y, actr->_x + 15, actr->_y + 15)) {
		thor_damaged(actr);
		actor_destroyed(actr);
		return 0;
	}

	int x1 = actr->_x;
	int y1 = actr->_y;
	int yd = 0;
	int xd = 0;
	int d = actr->_lastDir;

	if ((x1 > (_G(thor_x1)) + 1))
		xd = -2;
	else if ((x1 < (_G(thor_x1)) - 1))
		xd = 2;

	if (actr->_actorNum == 1) {
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
		if (xd == -2 && yd == -2)
			d = 2;
		else if (xd == -2 && yd == 2)
			d = 2;
		else if (xd == 2 && yd == -2)
			d = 3;
		else if (xd == 2 && yd == 2)
			d = 3;
		x1 += xd;
		y1 += yd;
		if (check_move3(x1, y1, actr)) {
			next_frame(actr);
			actr->_lastDir = d;
			if (actr->_directions == 1)
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

	x1 = actr->_x;
	y1 = actr->_y;
	actr->_toggle ^= 1;

	if (actr->_toggle) {
		if (xd) {
			x1 += xd;
			if (check_move3(x1, y1, actr)) {
				if (xd > 0)
					d = 3;
				else
					d = 2;
				next_frame(actr);
				actr->_lastDir = d;
				if (actr->_directions == 1)
					return 0;
				return d;
			}
			x1 = actr->_x;
		}
		if (yd) {
			y1 += yd;
			if (check_move3(x1, y1, actr)) {
				if (yd > 0)
					d = 1;
				else
					d = 0;
				next_frame(actr);
				actr->_lastDir = d;
				if (actr->_directions == 1)
					return 0;
				return d;
			}
		}
	} else {
		if (yd) {
			y1 += yd;
			if (check_move3(x1, y1, actr)) {
				if (yd > 0)
					d = 1;
				else
					d = 0;
				next_frame(actr);
				actr->_lastDir = d;
				if (actr->_directions == 1)
					return 0;
				return d;
			}
			y1 = actr->_y;
		}
		if (xd) {
			x1 += xd;
			if (check_move3(x1, y1, actr)) {
				if (xd > 0)
					d = 3;
				else
					d = 2;
				next_frame(actr);
				actr->_lastDir = d;
				if (actr->_directions == 1)
					return 0;
				return d;
			}
		}
	}
	check_move3(actr->_x, actr->_y, actr);
	next_frame(actr);
	actr->_lastDir = d;
	if (actr->_directions == 1)
		return 0;
	return d;
}

// No move, frame cycle
int shot_movement_five(Actor *actr) {
	next_shot_frame(actr);
	if (actr->_directions == 1)
		return 0;
	return actr->_lastDir;
}

// Wraith spots
int shot_movement_six(Actor *actr) {
	actr->_temp1--;
	if (!actr->_temp1) {
		actor_destroyed(actr);
		if (_G(actor[actr->_creator])._currNumShots)
			_G(actor[actr->_creator])._currNumShots--;
	} else
		next_shot_frame(actr);
	if (actr->_directions == 1)
		return 0;
	return actr->_lastDir;
}

#define YV actr->_i1
#define YC actr->_i2
#define BC actr->_i3
#define YD actr->_i4
#define XD actr->_i5
#define XC actr->_i6
#define IV 100
#define IC 50

// Skull drop
int shot_movement_seven(Actor *actr) {
	if (actr->_temp3) {
		actr->_temp3--;
		goto done;
	}
	if (overlap(actr->_x, actr->_y, actr->_x + actr->_sizeX, actr->_y + actr->_sizeY,
				_G(thor)->_x, _G(thor)->_y + 4, _G(thor)->_x + 15, _G(thor)->_y + 15))
		thor_damaged(actr);

	actr->_temp2++;
	if (actr->_temp2 > 2) {
		if (actr->_temp4)
			actr->_temp4--;
		actr->_temp2 = 0;
	}
	actr->_temp3 = actr->_temp4;

	actr->_y += 2;
	if (actr->_y > 160 - 36) {
		actr->_x += (4 - g_events->getRandomNumber(8));
		actr->_moveType = 8;
		YV = IV;
		YC = 0;
		BC = IC;
		YD = 0;
		XC = 3;
		if (actr->_x < 150)
			XD = 1;
		else
			XD = 0;
	}

done:
	next_shot_frame(actr);
	if (actr->_directions == 1)
		return 0;
	return actr->_lastDir;
}

// Skull bounce
int shot_movement_eight(Actor *actr) {
	int x, y;

	x = actr->_x;
	y = actr->_y;

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
	if (x < 1 || x > (319 - actr->_sizeX)) {
		if (!actr->_dead)
			if (_G(actor[actr->_creator])._currNumShots)
				_G(actor[actr->_creator])._currNumShots--;
		actor_destroyed(actr);
	}
	if (overlap(actr->_x, actr->_y, actr->_x + actr->_sizeX, actr->_y + actr->_sizeY,
				_G(thor)->_x, _G(thor)->_y + 4, _G(thor)->_x + 15, _G(thor)->_y + 15))
		thor_damaged(actr);
	actr->_x = x;
	actr->_y = y;

	//done:
	next_shot_frame(actr);
	if (actr->_directions == 1)
		return 0;
	return actr->_lastDir;
}

// Skull explode
int shot_movement_nine(Actor *actr) {
	actr->_nextFrame++;
	if (actr->_nextFrame == 3) {
		_G(actor[actr->_creator])._currNumShots--;
		actor_destroyed(actr);
		return 0;
	}
	if (actr->_directions == 1)
		return 0;
	return actr->_lastDir;
}

// Skull - stalagtites
int shot_movement_ten(Actor *actr) {
	int f = 0;
	actr->_y += 2;

	if (overlap(actr->_x, actr->_y, actr->_x + actr->_sizeX, actr->_y + actr->_sizeY,
				_G(thor)->_x, _G(thor)->_y + 4, _G(thor)->_x + 15, _G(thor)->_y + 15)) {
		thor_damaged(actr);
		f = 1;
	}
	if ((actr->_y > 160) || f) {
		if (_G(actor[actr->_creator])._currNumShots)
			_G(actor[actr->_creator])._currNumShots--;
		actor_destroyed(actr);
	}

	return 0;
}

#undef YD
#undef XD
#define YA actr->_i1
#define XA actr->_i2
#define YD actr->_i3
#define XD actr->_i4
#define DIR actr->_i5
#define CNT actr->_i6

void calc_angle(int x1, int y1, int x2, int y2, Actor *actr) {
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
int shot_movement_eleven(Actor *actr) {
	int x1 = actr->_x;
	int y1 = actr->_y;

	if (!actr->_temp1) {
		calc_angle(_G(thor_x1), _G(thor_real_y1), x1, y1, actr);
		actr->_temp1 = 1;
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

	if (!check_move3(x1, y1, actr)) {
		if (_G(actor[actr->_creator])._currNumShots)
			_G(actor[actr->_creator])._currNumShots--;
		actor_destroyed(actr);
	} else
		next_frame(actr);

	if (actr->_directions == 1)
		return 0;
	return actr->_lastDir;
}

// Angle throw Loki
int shot_movement_twelve(Actor *actr) {
	int x1 = actr->_x;
	int y1 = actr->_y;

	if (!actr->_temp5) {
		calc_angle(_G(thor_x1), _G(thor_real_y1), x1, y1, actr);
		actr->_temp5 = 1;
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
				   g_events->getRandomNumber(191), x1, y1, actr);
		actr->_moveType = 13;
		actr->_temp4 = 240;
		actr->_nextFrame = 2;
	} else {
		if (overlap(x1 + 2, y1 + 2, x1 + 14, y1 + 14, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
			thor_damaged(actr);
		}
		actr->_x = x1;
		actr->_y = y1;
	}

	int fcount = actr->_frameCount - 1;
	if (fcount <= 0) {
		actr->_nextFrame++;
		if (actr->_nextFrame > 1)
			actr->_nextFrame = 0;
		actr->_frameCount = actr->_frameSpeed;
	} else
		actr->_frameCount = fcount;
	
	if (actr->_directions == 1)
		return 0;
	return actr->_lastDir;
}

// Angle throw Loki-2
int shot_movement_thirteen(Actor *actr) {
	int x1 = actr->_x;
	int y1 = actr->_y;

	if (!actr->_temp5) {
		calc_angle(_G(thor_x1), _G(thor_real_y1), x1, y1, actr);
		actr->_temp5 = 1;
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

	if (actr->_temp4)
		actr->_temp4--;

	if (!actr->_temp4) {
		if (_G(actor[actr->_creator])._currNumShots)
			_G(actor[actr->_creator])._currNumShots--;
		actor_destroyed(actr);
		_G(apple_drop++);
		if (_G(apple_drop) > 4) {
			if (_drop_obj(actr, 5))
				_G(apple_drop) = 0;
			else
				_G(apple_drop) = 4;
		} else
			_drop_obj(actr, 4);
		return 0;
	}

	if (x1 < 16 || x1 > 287 || y1 < 16 || y1 > 159) {
		if (x1 < 16 || x1 > 287)
			XA = 0 - XA;
		else
			YA = 0 - YA;
	} else {
		if (overlap(x1 + 4, y1 + 4, x1 + 12, y1 + 12, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
			thor_damaged(actr);
		}
		actr->_x = x1;
		actr->_y = y1;
	}

	int fcount = actr->_frameCount - 1;
	if (fcount <= 0) {
		actr->_nextFrame++;
		if (actr->_nextFrame > 3)
			actr->_nextFrame = 2;
		actr->_frameCount = actr->_frameSpeed;
	} else
		actr->_frameCount = fcount;
	
	if (actr->_directions == 1)
		return 0;

	return actr->_lastDir;
}

} // namespace Got
