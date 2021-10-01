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

//=============================================================================
//
// New jump point search (JPS) A* pathfinder by Martin Sedlak.
//
//=============================================================================

#include "ags/engine/ac/route_finder_impl.h"
#include "ags/lib/std/math.h"
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

#define MAKE_INTCOORD(x,y) (((unsigned short)x << 16) | ((unsigned short)y))

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
	int count = std::min<int>((int)cpath.size(), MAXNAVPOINTS);

	for (int i = 0; i < count; i++) {
		int x, y;
		_GP(nav).UnpackSquare(cpath[i], x, y);

		_G(navpoints)[_G(num_navpoints)++] = MAKE_INTCOORD(x, y);
	}

	return 1;
}

void set_route_move_speed(int speed_x, int speed_y) {
	// negative move speeds like -2 get converted to 1/2
	if (speed_x < 0) {
		_G(move_speed_x) = itofix(1) / (-speed_x);
	} else {
		_G(move_speed_x) = itofix(speed_x);
	}

	if (speed_y < 0) {
		_G(move_speed_y) = itofix(1) / (-speed_y);
	} else {
		_G(move_speed_y) = itofix(speed_y);
	}
}

// Calculates the X and Y per game loop, for this stage of the
// movelist
void calculate_move_stage(MoveList *mlsp, int aaa) {
	// work out the x & y per move. First, opp/adj=tan, so work out the angle
	if (mlsp->pos[aaa] == mlsp->pos[aaa + 1]) {
		mlsp->xpermove[aaa] = 0;
		mlsp->ypermove[aaa] = 0;
		return;
	}

	short ourx = (mlsp->pos[aaa] >> 16) & 0x000ffff;
	short oury = (mlsp->pos[aaa] & 0x000ffff);
	short destx = ((mlsp->pos[aaa + 1] >> 16) & 0x000ffff);
	short desty = (mlsp->pos[aaa + 1] & 0x000ffff);

	// Special case for vertical and horizontal movements
	if (ourx == destx) {
		mlsp->xpermove[aaa] = 0;
		mlsp->ypermove[aaa] = _G(move_speed_y);
		if (desty < oury)
			mlsp->ypermove[aaa] = -mlsp->ypermove[aaa];

		return;
	}

	if (oury == desty) {
		mlsp->xpermove[aaa] = _G(move_speed_x);
		mlsp->ypermove[aaa] = 0;
		if (destx < ourx)
			mlsp->xpermove[aaa] = -mlsp->xpermove[aaa];

		return;
	}

	fixed xdist = itofix(abs(ourx - destx));
	fixed ydist = itofix(abs(oury - desty));

	fixed useMoveSpeed;

	if (_G(move_speed_x) == _G(move_speed_y)) {
		useMoveSpeed = _G(move_speed_x);
	} else {
		// different X and Y move speeds
		// the X proportion of the movement is (x / (x + y))
		fixed xproportion = fixdiv(xdist, (xdist + ydist));

		if (_G(move_speed_x) > _G(move_speed_y)) {
			// speed = y + ((1 - xproportion) * (x - y))
			useMoveSpeed = _G(move_speed_y) + fixmul(xproportion, _G(move_speed_x) - _G(move_speed_y));
		} else {
			// speed = x + (xproportion * (y - x))
			useMoveSpeed = _G(move_speed_x) + fixmul(itofix(1) - xproportion, _G(move_speed_y) - _G(move_speed_x));
		}
	}

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


int find_route(short srcx, short srcy, short xx, short yy, Bitmap *onscreen, int movlst, int nocross, int ignore_walls) {
	int i;

	_G(wallscreen) = onscreen;

	_G(num_navpoints) = 0;

	if (ignore_walls || can_see_from(srcx, srcy, xx, yy)) {
		_G(num_navpoints) = 2;
		_G(navpoints)[0] = MAKE_INTCOORD(srcx, srcy);
		_G(navpoints)[1] = MAKE_INTCOORD(xx, yy);
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

	int mlist = movlst;
	_G(mls)[mlist].numstage = _G(num_navpoints);
	memcpy(&_G(mls)[mlist].pos[0], &_G(navpoints)[0], sizeof(int) * _G(num_navpoints));
#ifdef DEBUG_PATHFINDER
	AGS::Shared::Debug::Printf("stages: %d\n", _G(num_navpoints));
#endif

	for (i = 0; i < _G(num_navpoints) - 1; i++)
		calculate_move_stage(&_G(mls)[mlist], i);

	_G(mls)[mlist].fromx = srcx;
	_G(mls)[mlist].fromy = srcy;
	_G(mls)[mlist].onstage = 0;
	_G(mls)[mlist].onpart = 0;
	_G(mls)[mlist].doneflag = 0;
	_G(mls)[mlist].lastx = -1;
	_G(mls)[mlist].lasty = -1;
	return mlist;
}


} // namespace RouteFinder
} // namespace Engine
} // namespace AGS
} // namespace AGS3
