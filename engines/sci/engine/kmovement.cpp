/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"
#include "sci/graphics/animate.h"

namespace Sci {

/**
 * Compute "velocity" vector (xStep,yStep)=(vx,vy) for a jump from (0,0) to
 * (dx,dy), with gravity constant gy. The gravity is assumed to be non-negative.
 *
 * If this was ordinary continuous physics, we would compute the desired
 * (floating point!) velocity vector (vx,vy) as follows, under the assumption
 * that vx and vy are linearly correlated by a constant c, i.e., vy = c * vx:
 *    dx = t * vx
 *    dy = t * vy + gy * t^2 / 2
 * => dy = c * dx + gy * (dx/vx)^2 / 2
 * => |vx| = sqrt( gy * dx^2 / (2 * (dy - c * dx)) )
 * Here, the sign of vx must be chosen equal to the sign of dx, obviously.
 *
 * This square root only makes sense in our context if the denominator is
 * positive, or equivalently, (dy - c * dx) must be positive. For simplicity
 * and by symmetry along the x-axis, we assume dx to be positive for all
 * computations, and only adjust for its sign in the end. Switching the sign of
 * c appropriately, we set tmp := (dy + c * dx) and compute c so that this term
 * becomes positive.
 *
 * Remark #1: If the jump is straight up, i.e. dx == 0, then we should not
 * assume the above linear correlation vy = c * vx of the velocities (as vx
 * will be 0, but vy shouldn't be, unless we drop down).
 *
 * Remark #2: We are actually in a discrete setup. The motion is computed
 * iteratively: each iteration, we add vx and vy to the position, then add gy
 * to vy. So the real formula is the following (where t ideally is close to an int):
 *
 *   dx = t * vx
 *   dy = t * vy + gy * t*(t-1) / 2
 *
 * But the solution resulting from that is a lot more complicated, so we use
 * the above approximation instead.
 *
 * Still, what we compute in the end is of course not a real velocity anymore,
 * but an integer approximation, used in an iterative stepping algorithm.
 */
reg_t kSetJump(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	// Input data
	reg_t object = argv[0];
	int dx = argv[1].toSint16();
	int dy = argv[2].toSint16();
	int gy = argv[3].toSint16();

	// Derived data
	int c;
	int tmp;
	int vx = 0;  // x velocity
	int vy = 0;  // y velocity

	int dxWasNegative = (dx < 0);
	dx = abs(dx);

	assert(gy >= 0);

	if (dx == 0) {
		// Upward jump. Value of c doesn't really matter
		c = 1;
	} else {
		// Compute a suitable value for c respectively tmp.
		// The important thing to consider here is that we want the resulting
		// *discrete* x/y velocities to be not-too-big integers, for a smooth
		// curve (i.e. we could just set vx=dx, vy=dy, and be done, but that
		// is hardly what you would call a parabolic jump, would ya? ;-).
		//
		// So, we make sure that 2.0*tmp will be bigger than dx (that way,
		// we ensure vx will be less than sqrt(gy * dx)).
		if (dx + dy < 0) {
			// dy is negative and |dy| > |dx|
			c = (2 * abs(dy)) / dx;
			//tmp = abs(dy);  // ALMOST the resulting value, except for obvious rounding issues
		} else {
			// dy is either positive, or |dy| <= |dx|
			c = (dx * 3 / 2 - dy) / dx;

			// We force c to be strictly positive
			if (c < 1)
				c = 1;

			//tmp = dx * 3 / 2;  // ALMOST the resulting value, except for obvious rounding issues

			// FIXME: Where is the 3 coming from? Maybe they hard/coded, by "accident", that usually gy=3 ?
			// Then this choice of scalar will make t equal to roughly sqrt(dx)
		}
	}
	// POST: c >= 1
	tmp = c * dx + dy;
	// POST: (dx != 0)  ==>  abs(tmp) > abs(dx)
	// POST: (dx != 0)  ==>  abs(tmp) ~>=~ abs(dy)

	debugC(2, kDebugLevelBresen, "c: %d, tmp: %d", c, tmp);

	// Compute x step
	if (tmp != 0)
		vx = (int)(dx * sqrt(gy / (2.0 * tmp)));
	else
		vx = 0;

	// Restore the left/right direction: dx and vx should have the same sign.
	if (dxWasNegative)
		vx = -vx;

	if ((dy < 0) && (vx == 0)) {
		// Special case: If this was a jump (almost) straight upward, i.e. dy < 0 (upward),
		// and vx == 0 (i.e. no horizontal movement, at least not after rounding), then we
		// compute vy directly.
		// For this, we drop the assumption on the linear correlation of vx and vy (obviously).

		// FIXME: This choice of vy makes t roughly (2+sqrt(2))/gy * sqrt(dy);
		// so if gy==3, then t is roughly sqrt(dy)...
		vy = (int)sqrt((double)gy * abs(2 * dy)) + 1;
	} else {
		// As stated above, the vertical direction is correlated to the horizontal by the
		// (non-zero) factor c.
		// Strictly speaking, we should probably be using the value of vx *before* rounding
		// it to an integer... Ah well
		vy = c * vx;
	}

	// Always force vy to be upwards
	vy = -abs(vy);

	debugC(2, kDebugLevelBresen, "SetJump for object at %04x:%04x", PRINT_REG(object));
	debugC(2, kDebugLevelBresen, "xStep: %d, yStep: %d", vx, vy);

	writeSelectorValue(segMan, object, SELECTOR(xStep), vx);
	writeSelectorValue(segMan, object, SELECTOR(yStep), vy);

	return s->r_acc;
}

#define _K_BRESEN_AXIS_X 0
#define _K_BRESEN_AXIS_Y 1

static void initialize_bresen(SegManager *segMan, int argc, reg_t *argv, reg_t mover, int step_factor, int deltax, int deltay) {
	reg_t client = readSelector(segMan, mover, SELECTOR(client));
	int stepx = (int16)readSelectorValue(segMan, client, SELECTOR(xStep)) * step_factor;
	int stepy = (int16)readSelectorValue(segMan, client, SELECTOR(yStep)) * step_factor;
	int numsteps_x = stepx ? (abs(deltax) + stepx - 1) / stepx : 0;
	int numsteps_y = stepy ? (abs(deltay) + stepy - 1) / stepy : 0;
	int bdi, i1;
	int numsteps;
	int deltax_step;
	int deltay_step;

	if (numsteps_x > numsteps_y) {
		numsteps = numsteps_x;
		deltax_step = (deltax < 0) ? -stepx : stepx;
		deltay_step = numsteps ? deltay / numsteps : deltay;
	} else { // numsteps_x <= numsteps_y
		numsteps = numsteps_y;
		deltay_step = (deltay < 0) ? -stepy : stepy;
		deltax_step = numsteps ? deltax / numsteps : deltax;
	}

/*	if (abs(deltax) > abs(deltay)) {*/ // Bresenham on y
	if (numsteps_y < numsteps_x) {

		writeSelectorValue(segMan, mover, SELECTOR(b_xAxis), _K_BRESEN_AXIS_Y);
		writeSelectorValue(segMan, mover, SELECTOR(b_incr), (deltay < 0) ? -1 : 1);
		//i1 = 2 * (abs(deltay) - abs(deltay_step * numsteps)) * abs(deltax_step);
		//bdi = -abs(deltax);
		i1 = 2 * (abs(deltay) - abs(deltay_step * (numsteps - 1))) * abs(deltax_step);
		bdi = -abs(deltax);
	} else { // Bresenham on x
		writeSelectorValue(segMan, mover, SELECTOR(b_xAxis), _K_BRESEN_AXIS_X);
		writeSelectorValue(segMan, mover, SELECTOR(b_incr), (deltax < 0) ? -1 : 1);
		//i1= 2 * (abs(deltax) - abs(deltax_step * numsteps)) * abs(deltay_step);
		//bdi = -abs(deltay);
		i1 = 2 * (abs(deltax) - abs(deltax_step * (numsteps - 1))) * abs(deltay_step);
		bdi = -abs(deltay);

	}

	writeSelectorValue(segMan, mover, SELECTOR(dx), deltax_step);
	writeSelectorValue(segMan, mover, SELECTOR(dy), deltay_step);

	debugC(2, kDebugLevelBresen, "Init bresen for mover %04x:%04x: d=(%d,%d)", PRINT_REG(mover), deltax, deltay);
	debugC(2, kDebugLevelBresen, "    steps=%d, mv=(%d, %d), i1= %d, i2=%d",
	          numsteps, deltax_step, deltay_step, i1, bdi*2);

	//writeSelectorValue(segMan, mover, SELECTOR(b_movCnt), numsteps); // Needed for HQ1/Ogre?
	writeSelectorValue(segMan, mover, SELECTOR(b_di), bdi);
	writeSelectorValue(segMan, mover, SELECTOR(b_i1), i1);
	writeSelectorValue(segMan, mover, SELECTOR(b_i2), bdi * 2);
}

reg_t kInitBresen(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t mover = argv[0];
	reg_t client = readSelector(segMan, mover, SELECTOR(client));

	int deltax = (int16)readSelectorValue(segMan, mover, SELECTOR(x)) - (int16)readSelectorValue(segMan, client, SELECTOR(x));
	int deltay = (int16)readSelectorValue(segMan, mover, SELECTOR(y)) - (int16)readSelectorValue(segMan, client, SELECTOR(y));
	int step_factor = (argc < 1) ? argv[1].toUint16() : 1;

	initialize_bresen(s->_segMan, argc, argv, mover, step_factor, deltax, deltay);

	return s->r_acc;
}

#define MOVING_ON_X (((axis == _K_BRESEN_AXIS_X)&&bi1) || dx)
#define MOVING_ON_Y (((axis == _K_BRESEN_AXIS_Y)&&bi1) || dy)

reg_t kDoBresen(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t mover = argv[0];
	reg_t client = readSelector(segMan, mover, SELECTOR(client));

	int x = (int16)readSelectorValue(segMan, client, SELECTOR(x));
	int y = (int16)readSelectorValue(segMan, client, SELECTOR(y));
	int oldx, oldy, destx, desty, dx, dy, bdi, bi1, bi2, movcnt, bdelta, axis;
	uint16 signal = readSelectorValue(segMan, client, SELECTOR(signal));
	int completed = 0;
	int max_movcnt = readSelectorValue(segMan, client, SELECTOR(moveSpeed));

	if (getSciVersion() > SCI_VERSION_01)
		signal &= ~kSignalHitObstacle;

	writeSelector(segMan, client, SELECTOR(signal), make_reg(0, signal)); // This is a NOP for SCI0
	oldx = x;
	oldy = y;
	destx = (int16)readSelectorValue(segMan, mover, SELECTOR(x));
	desty = (int16)readSelectorValue(segMan, mover, SELECTOR(y));
	dx = (int16)readSelectorValue(segMan, mover, SELECTOR(dx));
	dy = (int16)readSelectorValue(segMan, mover, SELECTOR(dy));
	bdi = (int16)readSelectorValue(segMan, mover, SELECTOR(b_di));
	bi1 = (int16)readSelectorValue(segMan, mover, SELECTOR(b_i1));
	bi2 = (int16)readSelectorValue(segMan, mover, SELECTOR(b_i2));
	movcnt = readSelectorValue(segMan, mover, SELECTOR(b_movCnt));
	bdelta = (int16)readSelectorValue(segMan, mover, SELECTOR(b_incr));
	axis = (int16)readSelectorValue(segMan, mover, SELECTOR(b_xAxis));

	if ((getSciVersion() >= SCI_VERSION_1_MIDDLE)) {
		// Introduced inbetween SCI1MIDDLE, lsl5 demo doesn't have it, longbow demo has
		if (SELECTOR(xLast) != -1) {
			// save last position into mover
			writeSelectorValue(segMan, mover, SELECTOR(xLast), x);
			writeSelectorValue(segMan, mover, SELECTOR(yLast), y);
		}
	}

	//printf("movecnt %d, move speed %d\n", movcnt, max_movcnt);

	if (g_sci->_features->handleMoveCount()) {
		if (max_movcnt > movcnt) {
			++movcnt;
			writeSelectorValue(segMan, mover, SELECTOR(b_movCnt), movcnt); // Needed for HQ1/Ogre?
			return NULL_REG;
		} else {
			movcnt = 0;
			writeSelectorValue(segMan, mover, SELECTOR(b_movCnt), movcnt); // Needed for HQ1/Ogre?
		}
	}

	if ((bdi += bi1) > 0) {
		bdi += bi2;

		if (axis == _K_BRESEN_AXIS_X)
			dx += bdelta;
		else
			dy += bdelta;
	}

	writeSelectorValue(segMan, mover, SELECTOR(b_di), bdi);

	x += dx;
	y += dy;

	if ((MOVING_ON_X && (((x < destx) && (oldx >= destx)) // Moving left, exceeded?
	            || ((x > destx) && (oldx <= destx)) // Moving right, exceeded?
	            || ((x == destx) && (abs(dx) > abs(dy))) // Moving fast, reached?
	            // Treat this last case specially- when doing sub-pixel movements
	            // on the other axis, we could still be far away from the destination
				)) || (MOVING_ON_Y && (((y < desty) && (oldy >= desty)) /* Moving upwards, exceeded? */
	                || ((y > desty) && (oldy <= desty)) /* Moving downwards, exceeded? */
	                || ((y == desty) && (abs(dy) >= abs(dx))) /* Moving fast, reached? */
				))) {
		// Whew... in short: If we have reached or passed our target position
		x = destx;
		y = desty;
		completed = 1;

		debugC(2, kDebugLevelBresen, "Finished mover %04x:%04x", PRINT_REG(mover));
	}

	writeSelectorValue(segMan, client, SELECTOR(x), x);
	writeSelectorValue(segMan, client, SELECTOR(y), y);

	debugC(2, kDebugLevelBresen, "New data: (x,y)=(%d,%d), di=%d", x, y, bdi);

	bool collision = false;
	reg_t cantBeHere = NULL_REG;

	if (SELECTOR(cantBeHere) != -1) {
		invokeSelector(s, client, SELECTOR(cantBeHere), argc, argv);
		if (!s->r_acc.isNull())
			collision = true;
		cantBeHere = s->r_acc;
	} else {
		invokeSelector(s, client, SELECTOR(canBeHere), argc, argv);
		if (s->r_acc.isNull())
			collision = true;
	}

	if (collision) {
		signal = readSelectorValue(segMan, client, SELECTOR(signal));

		writeSelectorValue(segMan, client, SELECTOR(x), oldx);
		writeSelectorValue(segMan, client, SELECTOR(y), oldy);
		writeSelectorValue(segMan, client, SELECTOR(signal), (signal | kSignalHitObstacle));

		debugC(2, kDebugLevelBresen, "Finished mover %04x:%04x by collision", PRINT_REG(mover));
		// we shall not set completed in this case, sierra sci also doesn't do it
		//  if we set call .moveDone in those cases qfg1 vga gate at the castle and lsl1 casino door will not work
	}

	if ((getSciVersion() >= SCI_VERSION_1_EGA))
		if (completed)
			invokeSelector(s, mover, SELECTOR(moveDone), argc, argv);

	if (SELECTOR(cantBeHere) != -1)
		return cantBeHere;
	return make_reg(0, completed);
}

extern void _k_dirloop(reg_t obj, uint16 angle, EngineState *s, int argc, reg_t *argv);

int getAngle(int xrel, int yrel) {
	if ((xrel == 0) && (yrel == 0))
		return 0;
	else {
		int val = (int)(180.0 / PI * atan2((double)xrel, (double) - yrel));
		if (val < 0)
			val += 360;

		// Take care of OB1 differences between SSCI and
		// FSCI. SCI games sometimes check for equality with
		// "round" angles
		if (val % 45 == 44)
			val++;
		else if (val % 45 == 1)
			val--;

		return val;
	}
}

reg_t kDoAvoider(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t avoider = argv[0];
	reg_t client, looper, mover;
	int angle;
	int dx, dy;
	int destx, desty;

	s->r_acc = SIGNAL_REG;

	if (!s->_segMan->isHeapObject(avoider)) {
		error("DoAvoider() where avoider %04x:%04x is not an object", PRINT_REG(avoider));
		return NULL_REG;
	}

	client = readSelector(segMan, avoider, SELECTOR(client));

	if (!s->_segMan->isHeapObject(client)) {
		error("DoAvoider() where client %04x:%04x is not an object", PRINT_REG(client));
		return NULL_REG;
	}

	looper = readSelector(segMan, client, SELECTOR(looper));
	mover = readSelector(segMan, client, SELECTOR(mover));

	if (!s->_segMan->isHeapObject(mover)) {
		if (mover.segment) {
			error("DoAvoider() where mover %04x:%04x is not an object", PRINT_REG(mover));
		}
		return s->r_acc;
	}

	destx = readSelectorValue(segMan, mover, SELECTOR(x));
	desty = readSelectorValue(segMan, mover, SELECTOR(y));

	debugC(2, kDebugLevelBresen, "Doing avoider %04x:%04x (dest=%d,%d)", PRINT_REG(avoider), destx, desty);

	invokeSelector(s, mover, SELECTOR(doit), argc, argv);

	mover = readSelector(segMan, client, SELECTOR(mover));
	if (!mover.segment) // Mover has been disposed?
		return s->r_acc; // Return gracefully.

	invokeSelector(s, client, SELECTOR(isBlocked), argc, argv);

	dx = destx - readSelectorValue(segMan, client, SELECTOR(x));
	dy = desty - readSelectorValue(segMan, client, SELECTOR(y));
	angle = getAngle(dx, dy);

	debugC(2, kDebugLevelBresen, "Movement (%d,%d), angle %d is %sblocked", dx, dy, angle, (s->r_acc.offset) ? " " : "not ");

	if (s->r_acc.offset) { // isBlocked() returned non-zero
		int rotation = (rand() & 1) ? 45 : (360 - 45); // Clockwise/counterclockwise
		int oldx = readSelectorValue(segMan, client, SELECTOR(x));
		int oldy = readSelectorValue(segMan, client, SELECTOR(y));
		int xstep = readSelectorValue(segMan, client, SELECTOR(xStep));
		int ystep = readSelectorValue(segMan, client, SELECTOR(yStep));
		int moves;

		debugC(2, kDebugLevelBresen, " avoider %04x:%04x", PRINT_REG(avoider));

		for (moves = 0; moves < 8; moves++) {
			int move_x = (int)(sin(angle * PI / 180.0) * (xstep));
			int move_y = (int)(-cos(angle * PI / 180.0) * (ystep));

			writeSelectorValue(segMan, client, SELECTOR(x), oldx + move_x);
			writeSelectorValue(segMan, client, SELECTOR(y), oldy + move_y);

			debugC(2, kDebugLevelBresen, "Pos (%d,%d): Trying angle %d; delta=(%d,%d)", oldx, oldy, angle, move_x, move_y);

			invokeSelector(s, client, SELECTOR(canBeHere), argc, argv);

			writeSelectorValue(segMan, client, SELECTOR(x), oldx);
			writeSelectorValue(segMan, client, SELECTOR(y), oldy);

			if (s->r_acc.offset) { // We can be here
				debugC(2, kDebugLevelBresen, "Success");
				writeSelectorValue(segMan, client, SELECTOR(heading), angle);

				return make_reg(0, angle);
			}

			angle += rotation;

			if (angle > 360)
				angle -= 360;
		}

		error("DoAvoider failed for avoider %04x:%04x", PRINT_REG(avoider));
	} else {
		int heading = readSelectorValue(segMan, client, SELECTOR(heading));

		if (heading == -1)
			return s->r_acc; // No change

		writeSelectorValue(segMan, client, SELECTOR(heading), angle);

		s->r_acc = make_reg(0, angle);

		reg_t params[2] = { make_reg(0, angle), client };

		if (looper.segment) {
			invokeSelector(s, looper, SELECTOR(doit), argc, argv, 2, params);
			return s->r_acc;
		} else {
			// No looper? Fall back to DirLoop
			_k_dirloop(client, (uint16)angle, s, argc, argv);
		}
	}

	return s->r_acc;
}

} // End of namespace Sci
