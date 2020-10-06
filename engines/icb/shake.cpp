/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/icb/common/px_common.h"
#include "engines/icb/icb.h"
#include "engines/icb/debug.h"
#include "engines/icb/shake.h"
#include "engines/icb/fn_routines.h"

namespace ICB {

#define MAX_SHAKE_LOOP (1000)

int32 shakeX = 0;
int32 shakeY = 0;

int32 nextShakeX = 0;
int32 nextShakeY = 0;

int32 rangeX = 0;
int32 rangeY = 0;

int GetShakeX() { return shakeX; }

int GetShakeY() {
	return (shakeY << 1);
}

int GetNextShakeX() { return nextShakeX; }

int GetNextShakeY() {
	return nextShakeY << 1;
}

mcodeFunctionReturnCodes fn_set_screen_offset(int32 &, int32 *params) {
	int32 x, y;

	x = params[0];
	y = params[1];

	shakeX = nextShakeX = x;
	shakeY = nextShakeY = y;

	return IR_CONT;
}

// fn_shake_screen(cycles,rx,ry,sx,sy);
mcodeFunctionReturnCodes fn_shake_screen(int32 &, int32 *params) {
	int32 sx, sy;

	static int32 cycles = 0;
	static bool8 shaking = FALSE8;

	// these are okay...
	rangeX = params[1];
	rangeY = params[2];

	if (rangeX > GET_SHAKE_RANGE_MAX_X)
		rangeX = GET_SHAKE_RANGE_MAX_X;
	if (rangeY > GET_SHAKE_RANGE_MAX_Y)
		rangeY = GET_SHAKE_RANGE_MAX_Y;

	sx = params[3];
	sy = params[4];

	if (sx > (2 * GET_SHAKE_RANGE_MAX_X))
		sx = 2 * GET_SHAKE_RANGE_MAX_X;
	if (sy > (2 * GET_SHAKE_RANGE_MAX_Y))
		sy = 2 * GET_SHAKE_RANGE_MAX_Y;

	// if not shaking then set up the shake...
	if (!shaking) {
		shaking = TRUE8;
		cycles = params[0];
		if (cycles < 0)
			Fatal_error("cycles < 0 in fn_shake_screen???");
	}

	// set actual shake to next shake...
	shakeX = nextShakeX;
	shakeY = nextShakeY;

	// if we have some cycles to go then shake and return IR_RPEAT
	if (cycles > 0) {
		int32 rx = rangeX;
		int32 ry = rangeY;

		// one less cycle to go...
		cycles--;

		int randX, randY;
		int x, y;

		// Check that the current screen shake position is acceptable within the new shake range
		if (((shakeX + sx) < -rx) || ((shakeX - sx) > rx)) {
			shakeX = 0;
		}
		if (((shakeY + sy) < -ry) || ((shakeY - sy) > ry)) {
			shakeY = 0;
		}

		// keep going till we find something that is acceptable...
		int loop = 0;
		// Want to make: -sx, -sx+1, .... 0, 1, 2, sx-1, sx
		int sxrange = 2 * sx + 1;
		// Want to make: -sy, -sy+1, .... 0, 1, 2, sy-1, sy
		int syrange = 2 * sy + 1;
		do {
			// work out steps...
			randX = (g_icb->getRandomSource()->getRandomNumber(sxrange - 1)) - sx;
			randY = (g_icb->getRandomSource()->getRandomNumber(syrange - 1)) - sy;

			x = shakeX + randX;
			y = shakeY + randY;
			loop++;
			if (loop > MAX_SHAKE_LOOP)
				break;
		} while ((x > rx) || (x < -rx) || (y > ry) || (y < -ry));

		if (loop > MAX_SHAKE_LOOP) {
			x = shakeX;
			y = shakeY;
		}

		// set the next lot to be this new value then...
		nextShakeX = x;
		nextShakeY = y;

		return IR_REPEAT;
	}

	// otherwise we have finished so set shaking to zero, zero the shake and quit
	nextShakeX = 0;
	nextShakeY = 0;

	if ((nextShakeX != shakeX) || (nextShakeY != shakeY))
		return IR_REPEAT;

	shaking = FALSE8;
	return IR_CONT;
}

} // End of namespace ICB
