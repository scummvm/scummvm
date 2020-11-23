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

#include "ags/shared/ac/route_finder_impl.h"

//include <string.h>
//include <math.h>

#include "ags/shared/ac/common.h"   // quit()
#include "ags/shared/ac/movelist.h"     // MoveList
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/debug/out.h"

#include "ags/shared/route_finder_jps.inl"

namespace AGS3 {

extern MoveList *mls;

using AGS::Shared::Bitmap;

// #define DEBUG_PATHFINDER

namespace AGS {
namespace Engine {
namespace RouteFinder {

#define MAKE_INTCOORD(x,y) (((unsigned short)x << 16) | ((unsigned short)y))

static const int MAXNAVPOINTS = MAXNEEDSTAGES;
static int navpoints[MAXNAVPOINTS];
static int num_navpoints;
static fixed move_speed_x, move_speed_y;
static Navigation nav;
static Bitmap *wallscreen;
static int lastcx, lastcy;

void init_pathfinder() {
}

void shutdown_pathfinder() {
}

void set_wallscreen(Bitmap *wallscreen_) {
	wallscreen = wallscreen_;
}

static void sync_nav_wallscreen() {
	// FIXME: this is dumb, but...
	nav.Resize(wallscreen->GetWidth(), wallscreen->GetHeight());

	for (int y = 0; y < wallscreen->GetHeight(); y++)
		nav.SetMapRow(y, wallscreen->GetScanLine(y));
}

int can_see_from(int x1, int y1, int x2, int y2) {
	lastcx = x1;
	lastcy = y1;

	if ((x1 == x2) && (y1 == y2))
		return 1;

	sync_nav_wallscreen();

	return !nav.TraceLine(x1, y1, x2, y2, lastcx, lastcy);
}

void get_lastcpos(int &lastcx_, int &lastcy_) {
	lastcx_ = lastcx;
	lastcy_ = lastcy;
}

// new routing using JPS
static int find_route_jps(int fromx, int fromy, int destx, int desty) {
	sync_nav_wallscreen();

	static std::vector<int> path, cpath;
	path.clear();
	cpath.clear();

	if (nav.NavigateRefined(fromx, fromy, destx, desty, path, cpath) == Navigation::NAV_UNREACHABLE)
		return 0;

	num_navpoints = 0;

	// new behavior: cut path if too complex rather than abort with error message
	int count = std::min<int>((int)cpath.size(), MAXNAVPOINTS);

	for (int i = 0; i < count; i++) {
		int x, y;
		nav.UnpackSquare(cpath[i], x, y);

		navpoints[num_navpoints++] = MAKE_INTCOORD(x, y);
	}

	return 1;
}

void set_route_move_speed(int speed_x, int speed_y) {
	// negative move speeds like -2 get converted to 1/2
	if (speed_x < 0) {
		move_speed_x = itofix(1) / (-speed_x);
	} else {
		move_speed_x = itofix(speed_x);
	}

	if (speed_y < 0) {
		move_speed_y = itofix(1) / (-speed_y);
	} else {
		move_speed_y = itofix(speed_y);
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

	fixed useMoveSpeed;

	if (move_speed_x == move_speed_y) {
		useMoveSpeed = move_speed_x;
	} else {
		// different X and Y move speeds
		// the X proportion of the movement is (x / (x + y))
		fixed xproportion = fixdiv(xdist, (xdist + ydist));

		if (move_speed_x > move_speed_y) {
			// speed = y + ((1 - xproportion) * (x - y))
			useMoveSpeed = move_speed_y + fixmul(xproportion, move_speed_x - move_speed_y);
		} else {
			// speed = x + (xproportion * (y - x))
			useMoveSpeed = move_speed_x + fixmul(itofix(1) - xproportion, move_speed_y - move_speed_x);
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

	wallscreen = onscreen;

	num_navpoints = 0;

	if (ignore_walls || can_see_from(srcx, srcy, xx, yy)) {
		num_navpoints = 2;
		navpoints[0] = MAKE_INTCOORD(srcx, srcy);
		navpoints[1] = MAKE_INTCOORD(xx, yy);
	} else {
		if ((nocross == 0) && (wallscreen->GetPixel(xx, yy) == 0))
			return 0; // clicked on a wall

		find_route_jps(srcx, srcy, xx, yy);
	}

	if (!num_navpoints)
		return 0;

	// FIXME: really necessary?
	if (num_navpoints == 1)
		navpoints[num_navpoints++] = navpoints[0];

	assert(num_navpoints <= MAXNAVPOINTS);

#ifdef DEBUG_PATHFINDER
	AGS::Shared::Debug::Printf("Route from %d,%d to %d,%d - %d stages", srcx, srcy, xx, yy, num_navpoints);
#endif

	int mlist = movlst;
	mls[mlist].numstage = num_navpoints;
	memcpy(&mls[mlist].pos[0], &navpoints[0], sizeof(int) * num_navpoints);
#ifdef DEBUG_PATHFINDER
	AGS::Shared::Debug::Printf("stages: %d\n", num_navpoints);
#endif

	for (i = 0; i < num_navpoints - 1; i++)
		calculate_move_stage(&mls[mlist], i);

	mls[mlist].fromx = srcx;
	mls[mlist].fromy = srcy;
	mls[mlist].onstage = 0;
	mls[mlist].onpart = 0;
	mls[mlist].doneflag = 0;
	mls[mlist].lastx = -1;
	mls[mlist].lasty = -1;
	return mlist;
}


} // namespace RouteFinder
} // namespace Engine
} // namespace AGS
} // namespace AGS3
