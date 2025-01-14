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

int shot_movement_none(ACTOR *actr);
int shot_movement_one(ACTOR *actr);
int shot_movement_two(ACTOR *actr);
int shot_movement_three(ACTOR *actr);
int shot_movement_four(ACTOR *actr);
int shot_movement_five(ACTOR *actr);
int shot_movement_six(ACTOR *actr);
int shot_movement_seven(ACTOR *actr);
int shot_movement_eight(ACTOR *actr);
int shot_movement_nine(ACTOR *actr);
int shot_movement_ten(ACTOR *actr);
int shot_movement_eleven(ACTOR *actr);
int shot_movement_twelve(ACTOR *actr);
int shot_movement_thirteen(ACTOR *actr);

int (*const shot_movement_func[])(ACTOR *actr) = {
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

void next_shot_frame(ACTOR *actr) {
	if (actr->directions == 4 && actr->frames == 1) {
		actr->next = actr->last_dir;
		actr->dir = 0;
	} else {
		const int fcount = actr->frame_count - 1;

		if (fcount <= 0) {
			actr->next++;
			if (actr->next > 3)
				actr->next = 0;

			actr->frame_count = actr->frame_speed;
		} else
			actr->frame_count = fcount;
	}
}

// Boss - snake
int shot_movement_none(ACTOR *actr) {
	actr->temp3--;
	if (!actr->temp3) {
		actor_destroyed(actr);
		if (_G(actor[actr->creator]).num_shots)
			_G(actor[actr->creator]).num_shots--;
	}

	next_shot_frame(actr);
	if (actr->directions == 1)
		return 0;

	return actr->last_dir;
}

int shot_movement_one(ACTOR *actr) {
	int x1 = 0, y1 = 0;

	int d = actr->last_dir;

	switch (d) {
	case 0:
		x1 = actr->x;
		y1 = actr->y - 2;
		break;
	case 1:
		x1 = actr->x;
		y1 = actr->y + 2;
		break;
	case 2:
		x1 = actr->x - 2;
		y1 = actr->y;
		break;
	case 3:
		x1 = actr->x + 2;
		y1 = actr->y;
		break;
	}

	if (!check_move3(x1, y1, actr)) {
		actor_destroyed(actr);
		if (_G(actor[actr->creator]).num_shots)
			_G(actor[actr->creator]).num_shots--;
	} else {
		next_shot_frame(actr);
		actr->last_dir = d;
	}

	if (actr->directions == 1)
		return 0;
	if (actr->directions == 4 && actr->frames == 1)
		return 0;

	return d;
}

int shot_movement_two(ACTOR *actr) {
	int x1 = 0, y1 = 0;

	int d = actr->last_dir;

	switch (d) {
	case 0:
		x1 = actr->x;
		y1 = actr->y - 2;
		break;
	case 1:
		x1 = actr->x;
		y1 = actr->y + 2;
		break;
	case 2:
		x1 = actr->x - 2;
		y1 = actr->y;
		break;
	case 3:
		x1 = actr->x + 2;
		y1 = actr->y;
		break;
	}
	if (!check_move4(x1, y1, actr)) {
		actor_destroyed(actr);
		if (_G(actor[actr->creator]).num_shots)
			_G(actor[actr->creator]).num_shots--;
	} else {
		next_shot_frame(actr);
		actr->last_dir = d;
	}
	if (actr->directions == 1)
		return 0;
	if (actr->directions == 4 && actr->frames == 1)
		return 0;
	return d;
}

// Serpent fire
int shot_movement_three(ACTOR *actr) {
	int d = actr->last_dir;

	actr->x -= 2;
	actr->temp3--;
	actr->temp4--;

	if (overlap(_G(thor_x1) - 1, _G(thor_y1) - 1, _G(thor_x2) + 1, _G(thor_y2) + 1,
				actr->x, actr->y, actr->x + 15, actr->y + 15)) {
		actr->move = 0;
		actr->speed = 6;
		thor_damaged(actr);
		actr->x += 2;
	}
	if (!actr->temp4) {
		actr->temp4 = actr->temp5;
		actr->speed++;
		if (actr->speed > 6)
			actr->move = 0;
	}
	if (!actr->temp3) {
		actor_destroyed(actr);
		if (_G(actor[actr->creator]).num_shots)
			_G(actor[actr->creator]).num_shots--;
	} else {
		next_shot_frame(actr);
		actr->last_dir = d;
	}
	if (actr->directions == 1)
		return 0;
	return d;
}

// Wraith balls
int shot_movement_four(ACTOR *actr) {
	if (actr->temp1) {
		actr->temp1--;
		if (!actr->temp1) {
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
	if (overlap(_G(thor)->x - 1, _G(thor)->y - 1, _G(thor_x2) + 1, _G(thor_y2) + 1,
				actr->x, actr->y, actr->x + 15, actr->y + 15)) {
		thor_damaged(actr);
		actor_destroyed(actr);
		return 0;
	}

	int x1 = actr->x;
	int y1 = actr->y;
	int yd = 0;
	int xd = 0;
	int d = actr->last_dir;

	if ((x1 > (_G(thor_x1)) + 1))
		xd = -2;
	else if ((x1 < (_G(thor_x1)) - 1))
		xd = 2;

	if (actr->actor_num == 1) {
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
			actr->last_dir = d;
			if (actr->directions == 1)
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

	x1 = actr->x;
	y1 = actr->y;
	actr->toggle ^= 1;

	if (actr->toggle) {
		if (xd) {
			x1 += xd;
			if (check_move3(x1, y1, actr)) {
				if (xd > 0)
					d = 3;
				else
					d = 2;
				next_frame(actr);
				actr->last_dir = d;
				if (actr->directions == 1)
					return 0;
				return d;
			}
			x1 = actr->x;
		}
		if (yd) {
			y1 += yd;
			if (check_move3(x1, y1, actr)) {
				if (yd > 0)
					d = 1;
				else
					d = 0;
				next_frame(actr);
				actr->last_dir = d;
				if (actr->directions == 1)
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
				actr->last_dir = d;
				if (actr->directions == 1)
					return 0;
				return d;
			}
			y1 = actr->y;
		}
		if (xd) {
			x1 += xd;
			if (check_move3(x1, y1, actr)) {
				if (xd > 0)
					d = 3;
				else
					d = 2;
				next_frame(actr);
				actr->last_dir = d;
				if (actr->directions == 1)
					return 0;
				return d;
			}
		}
	}
	check_move3(actr->x, actr->y, actr);
	next_frame(actr);
	actr->last_dir = d;
	if (actr->directions == 1)
		return 0;
	return d;
}

// No move, frame cycle
int shot_movement_five(ACTOR *actr) {
	next_shot_frame(actr);
	if (actr->directions == 1)
		return 0;
	return actr->last_dir;
}

// Wraith spots
int shot_movement_six(ACTOR *actr) {
	actr->temp1--;
	if (!actr->temp1) {
		actor_destroyed(actr);
		if (_G(actor[actr->creator]).num_shots)
			_G(actor[actr->creator]).num_shots--;
	} else
		next_shot_frame(actr);
	if (actr->directions == 1)
		return 0;
	return actr->last_dir;
}

#define YV actr->i1
#define YC actr->i2
#define BC actr->i3
#define YD actr->i4
#define XD actr->i5
#define XC actr->i6
#define IV 100
#define IC 50

// Skull drop
int shot_movement_seven(ACTOR *actr) {
	if (actr->temp3) {
		actr->temp3--;
		goto done;
	}
	if (overlap(actr->x, actr->y, actr->x + actr->size_x, actr->y + actr->size_y,
				_G(thor)->x, _G(thor)->y + 4, _G(thor)->x + 15, _G(thor)->y + 15))
		thor_damaged(actr);

	actr->temp2++;
	if (actr->temp2 > 2) {
		if (actr->temp4)
			actr->temp4--;
		actr->temp2 = 0;
	}
	actr->temp3 = actr->temp4;

	actr->y += 2;
	if (actr->y > 160 - 36) {
		actr->x += (4 - g_events->getRandomNumber(8));
		actr->move = 8;
		YV = IV;
		YC = 0;
		BC = IC;
		YD = 0;
		XC = 3;
		if (actr->x < 150)
			XD = 1;
		else
			XD = 0;
	}

done:
	next_shot_frame(actr);
	if (actr->directions == 1)
		return 0;
	return actr->last_dir;
}

// Skull bounce
int shot_movement_eight(ACTOR *actr) {
	int x, y;

	x = actr->x;
	y = actr->y;

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
	if (x < 1 || x > (319 - actr->size_x)) {
		if (!actr->dead)
			if (_G(actor[actr->creator]).num_shots)
				_G(actor[actr->creator]).num_shots--;
		actor_destroyed(actr);
	}
	if (overlap(actr->x, actr->y, actr->x + actr->size_x, actr->y + actr->size_y,
				_G(thor)->x, _G(thor)->y + 4, _G(thor)->x + 15, _G(thor)->y + 15))
		thor_damaged(actr);
	actr->x = x;
	actr->y = y;

	//done:
	next_shot_frame(actr);
	if (actr->directions == 1)
		return 0;
	return actr->last_dir;
}

// Skull explode
int shot_movement_nine(ACTOR *actr) {
	actr->next++;
	if (actr->next == 3) {
		_G(actor[actr->creator]).num_shots--;
		actor_destroyed(actr);
		return 0;
	}
	if (actr->directions == 1)
		return 0;
	return actr->last_dir;
}

// Skull - stalagtites
int shot_movement_ten(ACTOR *actr) {
	int f = 0;
	actr->y += 2;

	if (overlap(actr->x, actr->y, actr->x + actr->size_x, actr->y + actr->size_y,
				_G(thor)->x, _G(thor)->y + 4, _G(thor)->x + 15, _G(thor)->y + 15)) {
		thor_damaged(actr);
		f = 1;
	}
	if ((actr->y > 160) || f) {
		if (_G(actor[actr->creator]).num_shots)
			_G(actor[actr->creator]).num_shots--;
		actor_destroyed(actr);
	}

	return 0;
}

#undef YD
#undef XD
#define YA actr->i1
#define XA actr->i2
#define YD actr->i3
#define XD actr->i4
#define DIR actr->i5
#define CNT actr->i6

void calc_angle(int x1, int y1, int x2, int y2, ACTOR *actr) {
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
int shot_movement_eleven(ACTOR *actr) {
	int x1 = actr->x;
	int y1 = actr->y;

	if (!actr->temp1) {
		calc_angle(_G(thor_x1), _G(thor_real_y1), x1, y1, actr);
		actr->temp1 = 1;
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
		if (_G(actor[actr->creator]).num_shots)
			_G(actor[actr->creator]).num_shots--;
		actor_destroyed(actr);
	} else
		next_frame(actr);

	if (actr->directions == 1)
		return 0;
	return actr->last_dir;
}

// Angle throw Loki
int shot_movement_twelve(ACTOR *actr) {
	int x1 = actr->x;
	int y1 = actr->y;

	if (!actr->temp5) {
		calc_angle(_G(thor_x1), _G(thor_real_y1), x1, y1, actr);
		actr->temp5 = 1;
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
		actr->move = 13;
		actr->temp4 = 240;
		actr->next = 2;
	} else {
		if (overlap(x1 + 2, y1 + 2, x1 + 14, y1 + 14, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
			thor_damaged(actr);
		}
		actr->x = x1;
		actr->y = y1;
	}

	int fcount = actr->frame_count - 1;
	if (fcount <= 0) {
		actr->next++;
		if (actr->next > 1)
			actr->next = 0;
		actr->frame_count = actr->frame_speed;
	} else
		actr->frame_count = fcount;
	
	if (actr->directions == 1)
		return 0;
	return actr->last_dir;
}

// Angle throw Loki-2
int shot_movement_thirteen(ACTOR *actr) {
	int x1 = actr->x;
	int y1 = actr->y;

	if (!actr->temp5) {
		calc_angle(_G(thor_x1), _G(thor_real_y1), x1, y1, actr);
		actr->temp5 = 1;
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

	if (actr->temp4)
		actr->temp4--;

	if (!actr->temp4) {
		if (_G(actor[actr->creator]).num_shots)
			_G(actor[actr->creator]).num_shots--;
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
		actr->x = x1;
		actr->y = y1;
	}

	int fcount = actr->frame_count - 1;
	if (fcount <= 0) {
		actr->next++;
		if (actr->next > 3)
			actr->next = 2;
		actr->frame_count = actr->frame_speed;
	} else
		actr->frame_count = fcount;
	
	if (actr->directions == 1)
		return 0;

	return actr->last_dir;
}

} // namespace Got
