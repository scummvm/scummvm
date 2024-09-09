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

//=============================================================================
//
// New jump point search (JPS) A* pathfinder by Martin Sedlak.
//
//=============================================================================

#include "ags/engine/ac/route_finder_impl.h"
#include "ags/shared/ac/common.h"   // quit()
#include "ags/engine/ac/move_list.h"     // MoveList
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/ac/route_finder_jps.h"
#include "ags/globals.h"

namespace AGS3 {

using AGS::Shared::Bitmap;

namespace AGS {
namespace Engine {
namespace RouteFinder {

static const int MAXNAVPOINTS = MAXNEEDSTAGES;

void init_pathfinder() {
}

void shutdown_pathfinder() {
}

void set_wallscreen(Bitmap *wallscreen_) {
	_G(wallscreen) = wallscreen_;
}

static void sync_nav_wallscreen() {
	// FIXME: this is dumb, but...
	_GP(nav).Resize(_G(wallscreen)->GetWidth(), _G(wallscreen)->GetHeight());

	for (int y = 0; y < _G(wallscreen)->GetHeight(); y++)
		_GP(nav).SetMapRow(y, _G(wallscreen)->GetScanLine(y));
}

int can_see_from(int x1, int y1, int x2, int y2) {
	_G(lastcx) = x1;
	_G(lastcy) = y1;

	if ((x1 == x2) && (y1 == y2))
		return 1;

	sync_nav_wallscreen();

	return !_GP(nav).TraceLine(x1, y1, x2, y2, _G(lastcx), _G(lastcy));
}

void get_lastcpos(int &lastcx_, int &lastcy_) {
	lastcx_ = _G(lastcx);
	lastcy_ = _G(lastcy);
}

// new routing using JPS
static int find_route_jps(int fromx, int fromy, int destx, int desty) {
	sync_nav_wallscreen();

	std::vector<int> path, cpath;
	path.clear();
	cpath.clear();

	if (_GP(nav).NavigateRefined(fromx, fromy, destx, desty, path, cpath) == Navigation::NAV_UNREACHABLE)
		return 0;

	_G(num_navpoints) = 0;

	// new behavior: cut path if too complex rather than abort with error message
	int count = MIN<int>((int)cpath.size(), MAXNAVPOINTS);

	for (int i = 0; i < count; i++) {
		int x, y;
		_GP(nav).UnpackSquare(cpath[i], x, y);

		_G(navpoints)[_G(num_navpoints)++] = {x, y};
	}

	return 1;
}

inline fixed input_speed_to_fixed(int speed_val) {
	// negative move speeds like -2 get converted to 1/2
	if (speed_val < 0) {
		return itofix(1) / (-speed_val);
	} else {
		return itofix(speed_val);
	}
}

void set_route_move_speed(int speed_x, int speed_y) {
	_G(move_speed_x) = input_speed_to_fixed(speed_x);
	_G(move_speed_y) = input_speed_to_fixed(speed_y);
}

inline fixed calc_move_speed_at_angle(fixed speed_x, fixed speed_y, fixed xdist, fixed ydist) {
	fixed useMoveSpeed;
	if (speed_x == speed_y) {
		useMoveSpeed = speed_x;
	} else {
		// different X and Y move speeds
		// the X proportion of the movement is (x / (x + y))
		fixed xproportion = fixdiv(xdist, (xdist + ydist));

		if (speed_x > speed_y) {
			// speed = y + ((1 - xproportion) * (x - y))
			useMoveSpeed = speed_y + fixmul(xproportion, speed_x - speed_y);
		} else {
			// speed = x + (xproportion * (y - x))
			useMoveSpeed = speed_x + fixmul(itofix(1) - xproportion, speed_y - speed_x);
		}
	}
	return useMoveSpeed;
}

// Calculates the X and Y per game loop, for this stage of the movelist
void calculate_move_stage(MoveList *mlsp, int aaa, fixed move_speed_x, fixed move_speed_y) {
	// work out the x & y per move. First, opp/adj=tan, so work out the angle
	if (mlsp->pos[aaa] == mlsp->pos[aaa + 1]) {
		mlsp->xpermove[aaa] = 0;
		mlsp->ypermove[aaa] = 0;
		return;
	}

	short ourx = mlsp->pos[aaa].X;
	short oury = mlsp->pos[aaa].Y;
	short destx = mlsp->pos[aaa + 1].X;
	short desty = mlsp->pos[aaa + 1].Y;

	// Special case for vertical and horizontal movements
	if (ourx == destx) {
		mlsp->xpermove[aaa] = 0;
		mlsp->ypermove[aaa] = move_speed_y;
		if (desty < oury)
			mlsp->ypermove[aaa] = -mlsp->ypermove[aaa];

		return;
	}

	if (oury == desty) {
		mlsp->xpermove[aaa] = move_speed_x;
		mlsp->ypermove[aaa] = 0;
		if (destx < ourx)
			mlsp->xpermove[aaa] = -mlsp->xpermove[aaa];

		return;
	}

	fixed xdist = itofix(abs(ourx - destx));
	fixed ydist = itofix(abs(oury - desty));

	fixed useMoveSpeed = calc_move_speed_at_angle(move_speed_x, move_speed_y, xdist, ydist);

	fixed angl = fixatan(fixdiv(ydist, xdist));

	// now, since new opp=hyp*sin, work out the Y step size
	//fixed newymove = useMoveSpeed * fsin(angl);
	fixed newymove = fixmul(useMoveSpeed, fixsin(angl));

	// since adj=hyp*cos, work out X step size
	//fixed newxmove = useMoveSpeed * fcos(angl);
	fixed newxmove = fixmul(useMoveSpeed, fixcos(angl));

	if (destx < ourx)
		newxmove = -newxmove;
	if (desty < oury)
		newymove = -newymove;

	mlsp->xpermove[aaa] = newxmove;
	mlsp->ypermove[aaa] = newymove;
}

void recalculate_move_speeds(MoveList *mlsp, int old_speed_x, int old_speed_y, int new_speed_x, int new_speed_y) {
	const fixed old_movspeed_x = input_speed_to_fixed(old_speed_x);
	const fixed old_movspeed_y = input_speed_to_fixed(old_speed_y);
	const fixed new_movspeed_x = input_speed_to_fixed(new_speed_x);
	const fixed new_movspeed_y = input_speed_to_fixed(new_speed_y);
	// save current stage's step lengths, for later onpart's update
	const fixed old_stage_xpermove = mlsp->xpermove[mlsp->onstage];
	const fixed old_stage_ypermove = mlsp->ypermove[mlsp->onstage];

	for (int i = 0; (i < mlsp->numstage) && ((mlsp->xpermove[i] != 0) || (mlsp->ypermove[i] != 0)); ++i) {
		// First three cases where the speed is a plain factor, therefore
		// we may simply divide on old one and multiple on a new one
		if ((old_movspeed_x == old_movspeed_y) || // diagonal move at straight 45 degrees
			(mlsp->xpermove[i] == 0) ||           // straight vertical move
			(mlsp->ypermove[i] == 0))             // straight horizontal move
		{
			mlsp->xpermove[i] = fixdiv(fixmul(mlsp->xpermove[i], new_movspeed_x), old_movspeed_x);
			mlsp->ypermove[i] = fixdiv(fixmul(mlsp->ypermove[i], new_movspeed_y), old_movspeed_y);
		} else {
			// Move at angle has adjusted speed factor, which we must recalculate first
			short ourx = mlsp->pos[i].X;
			short oury = mlsp->pos[i].Y;
			short destx = mlsp->pos[i + 1].X;
			short desty = mlsp->pos[i + 1].Y;

			fixed xdist = itofix(abs(ourx - destx));
			fixed ydist = itofix(abs(oury - desty));
			fixed old_speed_at_angle = calc_move_speed_at_angle(old_movspeed_x, old_movspeed_y, xdist, ydist);
			fixed new_speed_at_angle = calc_move_speed_at_angle(new_movspeed_x, new_movspeed_y, xdist, ydist);

			mlsp->xpermove[i] = fixdiv(fixmul(mlsp->xpermove[i], new_speed_at_angle), old_speed_at_angle);
			mlsp->ypermove[i] = fixdiv(fixmul(mlsp->ypermove[i], new_speed_at_angle), old_speed_at_angle);
		}
	}

	// now adjust current passed stage fraction
	if (mlsp->onpart >= 0.f) {
		if (old_stage_xpermove != 0)
			mlsp->onpart = (mlsp->onpart * fixtof(old_stage_xpermove)) / fixtof(mlsp->xpermove[mlsp->onstage]);
		else
			mlsp->onpart = (mlsp->onpart * fixtof(old_stage_ypermove)) / fixtof(mlsp->ypermove[mlsp->onstage]);
	}
}

int find_route(short srcx, short srcy, short xx, short yy, int move_speed_x, int move_speed_y, Bitmap *onscreen, int move_id, int nocross, int ignore_walls) {

	_G(wallscreen) = onscreen;

	_G(num_navpoints) = 0;

	if (ignore_walls || can_see_from(srcx, srcy, xx, yy)) {
		_G(num_navpoints) = 2;
		_G(navpoints)[0] = {srcx, srcy};
		_G(navpoints)[1] = {xx, yy};
	} else {
		if ((nocross == 0) && (_G(wallscreen)->GetPixel(xx, yy) == 0))
			return 0; // clicked on a wall

		find_route_jps(srcx, srcy, xx, yy);
	}

	if (!_G(num_navpoints))
		return 0;

	// FIXME: really necessary?
	if (_G(num_navpoints) == 1)
		_G(navpoints)[_G(num_navpoints)++] = _G(navpoints)[0];

	assert(_G(num_navpoints) <= MAXNAVPOINTS);

#ifdef DEBUG_PATHFINDER
	AGS::Shared::Debug::Printf("Route from %d,%d to %d,%d - %d stages", srcx, srcy, xx, yy, _G(num_navpoints));
#endif

	MoveList mlist;
	mlist.numstage = _G(num_navpoints);
	memcpy(&mlist.pos[0], &_G(navpoints)[0], sizeof(Point) * _G(num_navpoints));
#ifdef DEBUG_PATHFINDER
	AGS::Shared::Debug::Printf("stages: %d\n", _G(num_navpoints));
#endif

	const fixed fix_speed_x = input_speed_to_fixed(move_speed_x);
	const fixed fix_speed_y = input_speed_to_fixed(move_speed_y);
	for (int i = 0; i < _G(num_navpoints) - 1; i++) {
		calculate_move_stage(&mlist, i, fix_speed_x, fix_speed_y);
	}

	mlist.from = {srcx, srcy};
	_GP(mls)[move_id] = mlist;
	return move_id;
}

bool add_waypoint_direct(MoveList *mlsp, short x, short y, int move_speed_x, int move_speed_y) {
	if (mlsp->numstage >= MAXNEEDSTAGES)
		return false;

	const fixed fix_speed_x = input_speed_to_fixed(move_speed_x);
	const fixed fix_speed_y = input_speed_to_fixed(move_speed_y);
	mlsp->pos[mlsp->numstage] = {x, y};
	calculate_move_stage(mlsp, mlsp->numstage - 1, fix_speed_x, fix_speed_y);
	mlsp->numstage++;
	return true;
}

} // namespace RouteFinder
} // namespace Engine
} // namespace AGS
} // namespace AGS3
