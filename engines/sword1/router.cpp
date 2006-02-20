/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "sword1/router.h"
#include "common/util.h"
#include "common/scummsys.h"
#include "sword1/swordres.h"
#include "sword1/sworddefs.h"
#include "sword1/objectman.h"
#include "sword1/resman.h"

namespace Sword1 {

/****************************************************************************
 *    JROUTER.C				polygon router with modular walks
 *       							using a tree of modules
 *       							21 july 94
 *    3  november 94
 *    System currently works by scanning grid data and coming up with a	ROUTE
 *    as a series of way points(nodes), the smoothest eight directional PATH
 * 		through these nodes is then found, and a WALK created to fit the PATH.
 *
 *		Two funtions are called by the user, RouteFinder creates a route as a
 *		module list, HardWalk creates an animation list from the module list.
 *		The split is only provided to allow the possibility of turning the
 *		autorouter over two game cycles.
 ****************************************************************************
 *
 *    Routine timings on osborne 486
 *
 *		Read floor resource (file already loaded)	 112 pixels
 *
 *		Read mega resource (file already loaded)	 112 pixels
 *
 *
 *
 ****************************************************************************
 *
 *    Modified 12 Oct 95
 *
 *		Target Points within 1 pixel of a line are ignored ???
 *
 *		Modules split into  Points within 1 pixel of a line are ignored ???
 *
 ****************************************************************************/

#define		NO_DIRECTIONS					8
#define		SLOW_IN								3
#define		SLOW_OUT							7
#define		ROUTE_END_FLAG				255
//#define		PLOT_PATHS						1
#undef		PLOT_PATHS

Router::Router(ObjectMan *pObjMan, ResMan *pResMan) {
	_objMan = pObjMan;
	_resMan = pResMan;
	_numExtraBars = _numExtraNodes = 0;
	nnodes = nbars = 0;
	_playerTargetX = _playerTargetY = _playerTargetDir = _playerTargetStance = 0;
	diagonalx = diagonaly = 0;
}

/*
 *    CODE
 */

int32 Router::routeFinder(int32 id, Object *megaObject, int32 x, int32 y, int32 dir)
{
/****************************************************************************
 *    RouteFinder.C				polygon router with modular walks
 *    		   							21 august 94
 *    3  november 94
 *		RouteFinder creates a list of modules that enables HardWalk to create
 *		an animation list.
 *
 *    RouteFinder currently works by scanning grid data and coming up with a ROUTE
 *    as a series of way points(nodes), the smoothest eight directional PATH
 * 		through these nodes is then found, this information is made available to
 *		HardWalk for a WALK to be created to fit the PATH.
 *
 *    30 november 94 return values modified
 *
 *    return 	0 = failed to find a route
 *
 *    				1 = found a route
 *
 *    				2 = mega already at target
 *
 ****************************************************************************/

	int32		routeFlag = 0;
	int32		solidFlag = 0;

	megaId = id;

	LoadWalkResources(megaObject, x, y, dir);

	framesPerStep = nWalkFrames/2;
	framesPerChar = nWalkFrames * NO_DIRECTIONS;

	// offset pointers added Oct 30 95 JPS
	standFrames = framesPerChar;
	turnFramesLeft = standFrames;
	turnFramesRight = standFrames;
	walkFramesLeft = 0;
	walkFramesRight = 0;
	slowInFrames = 0;
	slowOutFrames = 0;

	if (megaId == GEORGE)
	{
		turnFramesLeft = 3 * framesPerChar + NO_DIRECTIONS + 2 * SLOW_IN + 4 * SLOW_OUT;
		turnFramesRight = 3 * framesPerChar + NO_DIRECTIONS + 2 * SLOW_IN + 4 * SLOW_OUT + NO_DIRECTIONS;
		walkFramesLeft = framesPerChar + NO_DIRECTIONS;
		walkFramesRight = 2 * framesPerChar + NO_DIRECTIONS;
		slowInFrames = 3 * framesPerChar + NO_DIRECTIONS;
		slowOutFrames = 3 * framesPerChar + NO_DIRECTIONS + 2 * SLOW_IN;
	}
	else if (megaId == NICO)
	{
		turnFramesLeft = framesPerChar + NO_DIRECTIONS;
		turnFramesRight = framesPerChar + 2 * NO_DIRECTIONS;
		walkFramesLeft = 0;
		walkFramesRight = 0;
		slowInFrames = 0;
		slowOutFrames = 0;
	}

// **************************************************************************
// All route data now loaded start finding a route
// **************************************************************************
// **************************************************************************
// Check if we can get a route through the floor 		changed 12 Oct95 JPS
// **************************************************************************

	routeFlag = GetRoute();

	if (routeFlag == 2)  //special case for zero length route
	{
		if (targetDir >7)// if target direction specified as any
		{
			targetDir = startDir;
		}
		// just a turn on the spot is required set an end module for the route let the animator deal with it
		// modularPath is normally set by ExtractRoute
		modularPath[0].dir = startDir;
 		modularPath[0].num = 0;
 		modularPath[0].x = startX;
 		modularPath[0].y = startY;
		modularPath[1].dir = targetDir;
 		modularPath[1].num = 0;
 		modularPath[1].x = startX;
 		modularPath[1].y = startY;
 		modularPath[2].dir = 9;
 		modularPath[2].num = ROUTE_END_FLAG;

 		SlidyWalkAnimator(megaObject->o_route);
 		routeFlag = 2;
	}
	else if (routeFlag == 1) // a normal route
	{
		SmoothestPath();//Converts the route to an exact path
																			// The Route had waypoints and direction options
																			// The Path is an exact set of lines in 8 directions that reach the target.
																			// The path is in module format, but steps taken in each direction are not accurate
		// if target dir = 8 then the walk isn't linked to an anim so
		// we can create a route without sliding and miss the exact target
		if (targetDir == NO_DIRECTIONS)
		{
			SolidPath();
			solidFlag = SolidWalkAnimator(megaObject->o_route);
		}

		if (!solidFlag)
		{
			SlidyPath();
			SlidyWalkAnimator(megaObject->o_route);
		}
	}
	else // Route didn't reach target so assume point was off the floor
	{
//		routeFlag = 0;
	}
	return routeFlag;	// send back null route
}

// ****************************************************************************
// * GET A ROUTE
// ****************************************************************************

int32 Router::GetRoute()
{
 /****************************************************************************
  *    GetRoute.C				extract a path from walk grid
  *    		   							12 october 94
  *
  *    GetRoute currently works by scanning grid data and coming up with a ROUTE
  *    as a series of way points(nodes).
	*		 static	_routeData			route[O_ROUTE_SIZE];
  *
  *    return 	0 = failed to find a route
  *
  *    				1 = found a route
  *
  *    				2 = mega already at target
  *
  *    				3 = failed to find a route because target was on a line
  *
  ****************************************************************************/
	int32		routeGot = 0;
	int32		level;
	int32		changed;

	if ((startX == targetX) && (startY == targetY))
		routeGot = 2;

	else	// 'else' added by JEL (23jan96) otherwise 'routeGot' affected even when already set to '2' above - causing some 'turns' to walk downwards on the spot
		routeGot = CheckTarget(targetX,targetY);// returns 3 if target on a line ( +- 1 pixel )


	if (routeGot == 0) //still looking for a route check if target is within a pixel of a line
	{
		// scan through the nodes linking each node to its nearest neighbour until no more nodes change
		// This is the routine that finds a route using Scan()
		level = 1;
		do
		{
			changed = Scan(level);
			level =level + 1;
		}
		while (changed == 1);

		// Check to see if the route reached the target
		if (node[nnodes].dist < 9999)
	  {
			routeGot = 1;
			ExtractRoute();	// it did so extract the route as nodes and the directions to go between each node
											// route.X,route.Y and route.Dir now hold all the route infomation with the target dir or route continuation
		}
	}

	return routeGot;
}

// ****************************************************************************
// * THE SLIDY PATH ROUTINES
// ****************************************************************************

int32 Router::SmoothestPath()
{
/*
 *	This is the second big part of the route finder and the the only bit that tries to be clever
 *	(the other bits are clever).
 *  This part of the autorouter creates a list of modules from a set of lines running across the screen
 *	The task is complicated by two things;
 *	Firstly in chosing a route through the maze of nodes	the routine tries to minimise	the amount of each
 *	individual turn avoiding 90 degree and greater turns (where possible) and reduces the total nuber of
 *	turns (subject to two 45 degree turns being better than one 90 degree turn).
 *  Secondly when walking in a given direction the number of steps required to reach the end of that run
 *	is not calculated accurately. This is because I was unable to derive a function to relate number of
 *	steps taken between two points to the shrunken step size
 *
 */
	int32 p;
	int32	dirS;
	int32	dirD;
	int32	dS;
	int32	dD;
	int32	dSS;
	int32	dSD;
	int32	dDS;
	int32	dDD;
	int32	i;
	int32	steps;
	int32	option;
	int32	options;
	int32 lastDir;
	int32 nextDirS;
	int32 nextDirD;
	int32 tempturns[4];
	int32 turns[4];
	int32 turntable[NO_DIRECTIONS] = { 0, 1, 3, 5, 7, 5, 3, 1 };

//	targetDir;// no warnings

	// route.X route.Y and route.Dir start at far end
	smoothPath[0].x = startX;
	smoothPath[0].y = startY;
	smoothPath[0].dir = startDir;
	smoothPath[0].num = 0;
	p = 0;
	lastDir = startDir;
	// for each section of the route
	do
	{
		dirS = route[p].dirS;
		dirD = route[p].dirD;
		nextDirS = route[p+1].dirS;
		nextDirD = route[p+1].dirD;

		// Check directions into and out of a pair of nodes
		// going in
		dS = dirS - lastDir;
		if ( dS < 0)
			dS = dS + NO_DIRECTIONS;

		dD = dirD - lastDir;
		if ( dD < 0)
			dD = dD + NO_DIRECTIONS;

		// coming out
		dSS = dirS - nextDirS;
		if ( dSS < 0)
			dSS = dSS + NO_DIRECTIONS;

		dDD = dirD - nextDirD;
		if ( dDD < 0)
			dDD = dDD + NO_DIRECTIONS;

		dSD = dirS - nextDirD;
		if ( dSD < 0)
			dSD = dSD + NO_DIRECTIONS;

		dDS = dirD - nextDirS;
		if ( dDS < 0)
			dDS = dDS + NO_DIRECTIONS;

		// Determine the amount of turning involved in each possible path
		dS = turntable[dS];
		dD = turntable[dD];
		dSS = turntable[dSS];
		dDD = turntable[dDD];
		dSD = turntable[dSD];
		dDS = turntable[dDS];
		// get the best path out ie assume next section uses best direction
		if (dSD < dSS)
			dSS = dSD;
		if (dDS < dDD)
			dDD = dDS;

		// Rate each option. Split routes look crap so weight against
		// them
		tempturns[0] = dS + dSS + 3;
		turns[0] = 0;
		tempturns[1] = dS + dDD;
		turns[1] = 1;
		tempturns[2] = dD + dSS;
		turns[2] = 2;
		tempturns[3] = dD + dDD + 3;
		turns[3] = 3;

		// set up turns as a sorted	array of the turn values
		for (i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				if (tempturns[j] > tempturns[j + 1]) {
					SWAP(turns[j], turns[j + 1]);
					SWAP(tempturns[j], tempturns[j + 1]);
				}
			}
		}

		// best option matched in order of the priority we would like to see on the screen
		// but each option must be checked to see if it can be walked

		options = NewCheck(1, route[p].x, route[p].y, route[p + 1].x, route[p + 1].y);

		if (options == 0)
		{
			/*Tdebug("BestTurns fail %d %d %d %d",route[p].x, route[p].y, route[p + 1].x, route[p + 1].y);
			Tdebug("BestTurns fail %d %d %d %d",turns[0],turns[1],turns[2],options);
			Go_dos("BestTurns failed");*/
			error("BestTurns failed");
		}
		i = 0;
		steps = 0;
		do
		{
			option = 1 << turns[i];
			if (option & options)
				steps = SmoothCheck(turns[i],p,dirS,dirD);
			i = i + 1;
		}
		while ((steps == 0) && (i < 4));

#ifdef PLOT_PATHS	// plot the best path
		if (steps != 0)
		{
			i = 0;
			do
			{
				RouteLine(smoothPath[i].x, smoothPath[i].y, smoothPath[i+1].x, smoothPath[i+1].y, 228);
				i = i + 1;
			}
			while (i < steps);
		}
#endif

		if (steps == 0)
		{
			/*Tdebug("BestTurns failed %d %d %d %d",route[p].x, route[p].y, route[p + 1].x, route[p + 1].y);
			Tdebug("BestTurns failed %d %d %d %d",turns[0],turns[1],turns[2],options);
			Go_dos("BestTurns failed");*/
			error("BestTurns failed");
		}
		// route.X route.Y route.dir and bestTurns start at far end
		p = p + 1;


	}
	while (p < (routeLength));
	// best turns will end heading as near as possible to target dir rest is down to anim for now
	smoothPath[steps].dir = 9;
	smoothPath[steps].num = ROUTE_END_FLAG;
		return 1;
}




int32 Router::SmoothCheck(int32 best, int32 p, int32 dirS, int32 dirD)
/****************************************************************************
 *	Slip sliding away
 *  This path checker checks to see if a walk that exactly follows the path
 *  would be valid. This should be inherently true for atleast one of the turn
 *	options.
 *  No longer checks the data it only creates the smoothPath array JPS
 ****************************************************************************/
{
	static	int32  k;
	int32 tempK;
	int32 x;
	int32 y;
	int32 x2;
	int32 y2;
	int32 dx;
	int32 dy;
	int32 dsx;
	int32 dsy;
	int32 ddx;
	int32 ddy;
	int32 dirX;
	int32 dirY;
	int32	ss0;
	int32	ss1;
	int32	ss2;
	int32	sd0;
	int32	sd1;
	int32	sd2;

	if (p == 0)
	{
		k = 1;
	}
	tempK = 0;
	x = route[p].x;
	y = route[p].y;
	x2 = route[p + 1].x;
	y2 = route[p + 1].y;
	dx = x2 - x;
	dy = y2 - y;
	dirX = 1;
	dirY = 1;
	if (dx < 0)
	{
		dx = -dx;
		dirX = -1;
	}

	if (dy < 0)
	{
		dy = -dy;
		dirY = -1;
	}

// set up sd0-ss2 to reflect possible movement in each direction
	if ((dirS == 0)	|| (dirS == 4))// vert and diag
	{
		ddx = dx;
		ddy = (dx*diagonaly)/diagonalx;
		dsy = dy - ddy;
		ddx = ddx * dirX;
		ddy = ddy * dirY;
		dsy = dsy * dirY;
		dsx = 0;

		sd0 = (ddx + modX[dirD]/2)/ modX[dirD];
		ss0 = (dsy + modY[dirS]/2) / modY[dirS];
		sd1 = sd0/2;
		ss1 = ss0/2;
		sd2 = sd0 - sd1;
		ss2 = ss0 - ss1;
	}
	else
	{
		ddy = dy;
		ddx = (dy*diagonalx)/diagonaly;
		dsx = dx - ddx;
		ddy = ddy * dirY;
		ddx = ddx * dirX;
		dsx = dsx * dirX;
		dsy = 0;

		sd0 = (ddy + modY[dirD]/2)/ modY[dirD];
		ss0 = (dsx + modX[dirS]/2)/ modX[dirS];
		sd1 = sd0/2;
		ss1 = ss0/2;
		sd2 = sd0 - sd1;
		ss2 = ss0 - ss1;
	}

	if (best == 0) //halfsquare, diagonal,	halfsquare
	{
		smoothPath[k].x = x+dsx/2;
		smoothPath[k].y = y+dsy/2;
		smoothPath[k].dir = dirS;
		smoothPath[k].num = ss1;
		k = k + 1;
		smoothPath[k].x = x+dsx/2+ddx;
		smoothPath[k].y = y+dsy/2+ddy;
		smoothPath[k].dir = dirD;
		smoothPath[k].num = sd0;
		k = k + 1;
		smoothPath[k].x = x+dsx+ddx;
		smoothPath[k].y = y+dsy+ddy;
		smoothPath[k].dir = dirS;
		smoothPath[k].num = ss2;
		k = k + 1;
		tempK = k;
	}
	else if (best == 1) //square, diagonal
	{
		smoothPath[k].x = x+dsx;
		smoothPath[k].y = y+dsy;
		smoothPath[k].dir = dirS;
		smoothPath[k].num = ss0;
		k = k + 1;
		smoothPath[k].x = x2;
		smoothPath[k].y = y2;
		smoothPath[k].dir = dirD;
		smoothPath[k].num = sd0;
		k = k + 1;
		tempK = k;
	}
	else if (best == 2) //diagonal square
	{
		smoothPath[k].x = x+ddx;
		smoothPath[k].y = y+ddy;
		smoothPath[k].dir = dirD;
		smoothPath[k].num = sd0;
		k = k + 1;
		smoothPath[k].x = x2;
		smoothPath[k].y = y2;
		smoothPath[k].dir = dirS;
		smoothPath[k].num = ss0;
		k = k + 1;
		tempK = k;
	}
	else //halfdiagonal, square, halfdiagonal
	{
		smoothPath[k].x = x+ddx/2;
		smoothPath[k].y = y+ddy/2;
		smoothPath[k].dir = dirD;
		smoothPath[k].num = sd1;
		k = k + 1;
		smoothPath[k].x = x+dsx+ddx/2;
		smoothPath[k].y = y+dsy+ddy/2;
		smoothPath[k].dir = dirS;
		smoothPath[k].num = ss0;
		k = k + 1;
		smoothPath[k].x = x2;
		smoothPath[k].y = y2;
		smoothPath[k].dir = dirD;
		smoothPath[k].num = sd2;
		k = k + 1;
		tempK = k;
	}

	return tempK;
}

int32 Router::SlidyPath()
{
/****************************************************************************
 *  SlidyPath creates a path based on part steps with no sliding to get
 *	as near as possible to the target without any sliding this routine is
 *	currently unused, but is intended for use when just clicking about.
 *
 *	produce a module list from the line data
 *
 ****************************************************************************/
	int32 smooth;
	int32 slidy;
	int32 scale;
	int32 stepX;
	int32 stepY;
	int32 deltaX;
	int32 deltaY;

	// strip out the short sections
	slidy = 1;
	smooth = 1;
	modularPath[0].x = smoothPath[0].x;
	modularPath[0].y = smoothPath[0].y;
	modularPath[0].dir = smoothPath[0].dir;
	modularPath[0].num = 0;

	while (smoothPath[smooth].num < ROUTE_END_FLAG)
	{
		scale = scaleA * smoothPath[smooth].y + scaleB;
		deltaX = smoothPath[smooth].x - modularPath[slidy-1].x;
		deltaY = smoothPath[smooth].y - modularPath[slidy-1].y;
		stepX = modX[smoothPath[smooth].dir];
		stepY = modY[smoothPath[smooth].dir];
		stepX = stepX * scale;
		stepY = stepY * scale;
		stepX = stepX >> 19;// quarter a step minimum
		stepY = stepY >> 19;
		if ((ABS(deltaX)>=ABS(stepX)) &&	(ABS(deltaY)>=ABS(stepY)))
		{
	 		modularPath[slidy].x = smoothPath[smooth].x;
			modularPath[slidy].y = smoothPath[smooth].y;
			modularPath[slidy].dir = smoothPath[smooth].dir;
			modularPath[slidy].num = 1;
			slidy += 1;
		}
		smooth += 1;
	}
	// in	case the last bit had no steps
	if (slidy > 1)
	{
		modularPath[slidy-1].x = smoothPath[smooth-1].x;
		modularPath[slidy-1].y = smoothPath[smooth-1].y;
	}
	// set up the end of the walk
	modularPath[slidy].x = smoothPath[smooth-1].x;
	modularPath[slidy].y = smoothPath[smooth-1].y;
	modularPath[slidy].dir = targetDir;
	modularPath[slidy].num = 0;
	slidy += 1;
	modularPath[slidy].x = smoothPath[smooth-1].x;
	modularPath[slidy].y = smoothPath[smooth-1].y;
	modularPath[slidy].dir = 9;
	modularPath[slidy].num = ROUTE_END_FLAG;
	return 1;

}

void Router::SlidyWalkAnimator(WalkData *walkAnim)
/****************************************************************************
 *  Skidding every where HardWalk creates an animation that exactly fits the
 *	smoothPath and uses foot slipping to fit whole steps into the route
 *	Parameters: georgeg,mouseg
 *	Returns:		rout
 *
 *	produce a module list from the line data
 *
 ****************************************************************************/
{

	static int32 left = 0;
	int32 p;
	int32	lastDir;
	int32	lastRealDir;
	int32	currentDir;
	int32	turnDir;
	int32 scale;
	int32 step;
	int32 module;
	int32 moduleEnd;
	int32 moduleX;
	int32 moduleY;
	int32 module16X = 0;
	int32 module16Y = 0;
	int32 stepX;
	int32 stepY;
	int32 errorX;
	int32 errorY;
	int32 lastErrorX;
	int32 lastErrorY;
	int32 lastCount;
	int32 stepCount;
	int32 frameCount;
	int32 frames;
	int32 frame;

	// start at the begining for a change
	p = 0;
	lastDir = modularPath[0].dir;
	currentDir = modularPath[1].dir;
	if (currentDir == NO_DIRECTIONS)
	{
		currentDir = lastDir;
	}
	moduleX = startX;
	moduleY = startY;
	module16X = moduleX << 16;
	module16Y = moduleY << 16;
	stepCount = 0;

	//****************************************************************************
	// SLIDY
	// START THE WALK WITH THE FIRST STANDFRAME THIS MAY CAUSE A DELAY
	// BUT IT STOPS THE PLAYER MOVING FOR COLLISIONS ARE DETECTED
	//****************************************************************************
	module =	framesPerChar + lastDir;
	walkAnim[stepCount].frame = module;
	walkAnim[stepCount].step = 0;
	walkAnim[stepCount].dir = lastDir;
	walkAnim[stepCount].x = moduleX;
	walkAnim[stepCount].y = moduleY;
	stepCount += 1;

	//****************************************************************************
	// SLIDY
	// TURN TO START THE WALK
	//****************************************************************************
	// rotate if we need to
	if (lastDir != currentDir)
	{
		// get the direction to turn
		turnDir = currentDir - lastDir;
		if ( turnDir < 0)
				turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to new walk direction
		// for george and nico put in a head turn at the start
		if ((megaId == GEORGE) || (megaId == NICO))
		{
			if ( turnDir < 0)	// new frames for turn frames	29oct95jps
			{
				module =	turnFramesLeft + lastDir;
			}
			else
			{
				module =	turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}

		// rotate till were facing new dir then go back 45 degrees
		while (lastDir != currentDir)
		{
			lastDir += turnDir;
			if ( turnDir < 0)	// new frames for turn frames	29oct95jps
			{
				if ( lastDir < 0)
						lastDir += NO_DIRECTIONS;
				module =	turnFramesLeft + lastDir;
			}
			else
			{
				if ( lastDir > 7)
						lastDir -= NO_DIRECTIONS;
				module =	turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
		// the back 45 degrees bit
		stepCount -= 1;// step back one because new head turn for george takes us past the new dir
	}
	// his head is in the right direction
	lastRealDir = currentDir;

	//****************************************************************************
	// SLIDY
	// THE WALK
	//****************************************************************************

	if (left == 0)
		left = framesPerStep;
	else
		left = 0;

	lastCount = stepCount;
	lastDir = 99;// this ensures that we don't put in turn frames for the start
	currentDir = 99;// this ensures that we don't put in turn frames for the start
	do
	{
		while (modularPath[p].num == 0)
		{
			p = p + 1;
			if (currentDir != 99)
				lastRealDir = currentDir;
			lastDir = currentDir;
			lastCount = stepCount;
		}
		//calculate	average amount to lose in each step on the way to the next node
		currentDir = modularPath[p].dir;
		if (currentDir < NO_DIRECTIONS)
		{
			module =	currentDir * framesPerStep * 2 + left;
			if (left == 0)
				left = framesPerStep;
			else
				left = 0;
			moduleEnd = module + framesPerStep;
			step = 0;
			scale = (scaleA * moduleY + scaleB);
			do
			{
				module16X += _dx[module]*scale;
				module16Y += _dy[module]*scale;
				moduleX = module16X >> 16;
				moduleY = module16Y >> 16;
				walkAnim[stepCount].frame = module;
				walkAnim[stepCount].step = step;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
				step += 1;
				module += 1;
			}
			while ( module < moduleEnd) ;
			stepX = modX[modularPath[p].dir];
			stepY = modY[modularPath[p].dir];
			errorX = modularPath[p].x -	moduleX;
			errorX = errorX * stepX;
			errorY = modularPath[p].y -	moduleY;
			errorY = errorY * stepY;
			if ((errorX < 0) || (errorY < 0))
			{
				modularPath[p].num = 0;	// the end of the path
				// okay those last steps took us past our target but do we want to scoot or moonwalk
				frames = stepCount - lastCount;
				errorX = modularPath[p].x - walkAnim[stepCount-1].x;
				errorY = modularPath[p].y - walkAnim[stepCount-1].y;

				if (frames > framesPerStep)
				{
					lastErrorX = modularPath[p].x - walkAnim[stepCount-7].x;
					lastErrorY = modularPath[p].y - walkAnim[stepCount-7].y;
					if (stepX==0)
					{
						if (3*ABS(lastErrorY) < ABS(errorY)) //the last stop was closest
						{
							stepCount -= framesPerStep;
							if (left == 0)
						 		left = framesPerStep;
							else
							 	left = 0;
						}
					}
					else
					{
						if (3*ABS(lastErrorX) < ABS(errorX)) //the last stop was closest
						{
							stepCount -= framesPerStep;
							if (left == 0)
						 		left = framesPerStep;
							else
							 	left = 0;
						}
					}
				}
				errorX = modularPath[p].x - walkAnim[stepCount-1].x;
				errorY = modularPath[p].y - walkAnim[stepCount-1].y;
				// okay we've reached the end but we still have an error
				if (errorX != 0)
				{
					frameCount = 0;
					frames = stepCount - lastCount;
					do
					{
						frameCount += 1;
						walkAnim[lastCount + frameCount - 1].x +=	errorX*frameCount/frames;
					}
					while (frameCount<frames);
				}
				if (errorY != 0)
				{
					frameCount = 0;
					frames = stepCount - lastCount;
					do
					{
						frameCount += 1;
						walkAnim[lastCount + frameCount-1].y +=	errorY*frameCount/frames;
					}
					while (frameCount<frames);
				}
				// Now is the time to put in the turn frames for the last turn
				if (frames < framesPerStep)
					currentDir = 99;// this ensures that we don't put in turn frames for this walk or the next
				if (currentDir != 99)
					lastRealDir = currentDir;
				// check each turn condition in turn
				if (((lastDir != 99) && (currentDir != 99)) && (megaId == GEORGE)) // only for george
				{
					lastDir = currentDir - lastDir;//1 and -7 going right -1 and 7 going left
					if (((lastDir == -1) || (lastDir == 7)) || ((lastDir == -2) || (lastDir == 6)))
					{
						// turn at the end of the last walk
						frame = lastCount - framesPerStep;
						do
						{
							walkAnim[frame].frame += 104;//turning left
							frame += 1;
						}
						while (frame < lastCount );
					}
					if (((lastDir == 1) || (lastDir == -7)) || ((lastDir == 2) || (lastDir == -6)))
					{
						// turn at the end of the current walk
						frame = lastCount - framesPerStep;
						do
						{
							walkAnim[frame].frame += 200; //was 60 now 116
							frame += 1;
						}
						while (frame < lastCount );
					}
					lastDir = currentDir;
				}
				// all turns checked

				lastCount = stepCount;
				moduleX = walkAnim[stepCount-1].x;
				moduleY =	walkAnim[stepCount-1].y;
				module16X = moduleX << 16;
				module16Y = moduleY << 16;
			}
		}
	}
	while (modularPath[p].dir < NO_DIRECTIONS);



	if (lastRealDir == 99)
	{
		error("SlidyWalkAnimatorlast direction error\n");
	}
	//****************************************************************************
	// SLIDY
	// TURNS TO END THE WALK ?
	//****************************************************************************

	// We've done the walk now put in any turns at the end


	if (targetDir == NO_DIRECTIONS)	// stand in the last direction
	{
		module =	standFrames + lastRealDir;
		targetDir =	lastRealDir;
		walkAnim[stepCount].frame = module;
		walkAnim[stepCount].step = 0;
		walkAnim[stepCount].dir = lastRealDir;
		walkAnim[stepCount].x = moduleX;
		walkAnim[stepCount].y = moduleY;
		stepCount += 1;
	}
	if (targetDir == 9)
	{
		if (stepCount == 0)
		{
			module =	framesPerChar + lastRealDir;
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
	}
	else if (targetDir != lastRealDir) // rotate to targetDir
	{
		// rotate to target direction
		turnDir = targetDir - lastRealDir;
		if ( turnDir < 0)
			turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to target direction
		// for george and nico put in a head turn at the start
		if ((megaId == GEORGE) || (megaId == NICO))
		{
			if ( turnDir < 0)	// new frames for turn frames	29oct95jps
			{
				module =	turnFramesLeft + lastDir;
			}
			else
			{
				module =	turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}

		// rotate if we need to
		while (lastRealDir != targetDir)
		{
			lastRealDir += turnDir;
			if ( turnDir < 0)	// new frames for turn frames	29oct95jps
			{
				if ( lastRealDir < 0)
						lastRealDir += NO_DIRECTIONS;
				module =	turnFramesLeft + lastRealDir;
			}
			else
			{
				if ( lastRealDir > 7)
						lastRealDir -= NO_DIRECTIONS;
				module =	turnFramesRight + lastRealDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
		module =	standFrames + lastRealDir;
		walkAnim[stepCount-1].frame = module;
	}
	else // just stand at the end
	{
		module =	standFrames + lastRealDir;
		walkAnim[stepCount].frame = module;
		walkAnim[stepCount].step = 0;
		walkAnim[stepCount].dir = lastRealDir;
		walkAnim[stepCount].x = moduleX;
		walkAnim[stepCount].y = moduleY;
		stepCount += 1;
	}

	walkAnim[stepCount].frame = 512;
	stepCount += 1;
	walkAnim[stepCount].frame = 512;
	stepCount += 1;
	walkAnim[stepCount].frame = 512;
//	Tdebug("RouteFinder RouteSize is %d", stepCount);
	return;
}

// ****************************************************************************
// * THE SOLID PATH ROUTINES
// ****************************************************************************

int32 Router::SolidPath()
{
/****************************************************************************
 *  SolidPath creates a path based on whole steps with no sliding to get
 *	as near as possible to the target without any sliding this routine is
 *	currently unused, but is intended for use when just clicking about.
 *
 *	produce a module list from the line data
 *
 ****************************************************************************/
	int32 smooth;
	int32 solid;
	int32 scale;
	int32 stepX;
	int32 stepY;
	int32 deltaX;
	int32 deltaY;

	// strip out the short sections
	solid = 1;
	smooth = 1;
	modularPath[0].x = smoothPath[0].x;
	modularPath[0].y = smoothPath[0].y;
	modularPath[0].dir = smoothPath[0].dir;
	modularPath[0].num = 0;

	do
	{
		scale = scaleA * smoothPath[smooth].y + scaleB;
		deltaX = smoothPath[smooth].x - modularPath[solid-1].x;
		deltaY = smoothPath[smooth].y - modularPath[solid-1].y;
		stepX = modX[smoothPath[smooth].dir];
		stepY = modY[smoothPath[smooth].dir];
		stepX = stepX * scale;
		stepY = stepY * scale;
		stepX = stepX >> 16;
		stepY = stepY >> 16;
		if ((ABS(deltaX)>=ABS(stepX)) &&	(ABS(deltaY)>=ABS(stepY)))
		{
			modularPath[solid].x = smoothPath[smooth].x;
			modularPath[solid].y = smoothPath[smooth].y;
			modularPath[solid].dir = smoothPath[smooth].dir;
			modularPath[solid].num = 1;
			solid += 1;
		}
		smooth += 1;
	}
	while (smoothPath[smooth].num < ROUTE_END_FLAG);
	// in	case the last bit had no steps
	if (solid == 1) //there were no paths so put in a dummy end
	{
		solid = 2;
		modularPath[1].dir = smoothPath[0].dir;
		modularPath[1].num = 0;
	}
	modularPath[solid-1].x = smoothPath[smooth-1].x;
	modularPath[solid-1].y = smoothPath[smooth-1].y;
	// set up the end of the walk
	modularPath[solid].x = smoothPath[smooth-1].x;
	modularPath[solid].y = smoothPath[smooth-1].y;
	modularPath[solid].dir = 9;
	modularPath[solid].num = ROUTE_END_FLAG;
	return 1;

}

int32 Router::SolidWalkAnimator(WalkData *walkAnim)
{
/****************************************************************************
 *  SolidWalk creates an animation based on whole steps with no sliding to get
 *	as near as possible to the target without any sliding this routine is
 *	is intended for use when just clicking about.
 *
 *	produce a module list from the line data
 *
 *	returns 0 if solid route not found
 ****************************************************************************/
	int32 p;
	int32 i;
	int32 left;
	int32	lastDir;
	int32	currentDir;
	int32	turnDir;
	int32 scale;
	int32 step;
	int32 module;
	int32 moduleX;
	int32 moduleY;
	int32 module16X;
	int32 module16Y;
	int32 errorX;
	int32 errorY;
	int32 moduleEnd;
	int32 slowStart;
	int32 stepCount;
	int32 lastCount;
	int32 frame;

	// start at the begining for a change
	lastDir = modularPath[0].dir;
	p = 1;
	currentDir = modularPath[1].dir;
	module =	framesPerChar + lastDir;
	moduleX = startX;
	moduleY = startY;
	module16X = moduleX << 16;
	module16Y = moduleY << 16;
	slowStart = 0;
	stepCount = 0;

	//****************************************************************************
	// SOLID
	// START THE WALK WITH THE FIRST STANDFRAME THIS MAY CAUSE A DELAY
	// BUT IT STOPS THE PLAYER MOVING FOR COLLISIONS ARE DETECTED
	//****************************************************************************
	walkAnim[stepCount].frame = module;
	walkAnim[stepCount].step = 0;
	walkAnim[stepCount].dir = lastDir;
	walkAnim[stepCount].x = moduleX;
	walkAnim[stepCount].y = moduleY;
	stepCount += 1;

	//****************************************************************************
	// SOLID
	// TURN TO START THE WALK
	//****************************************************************************
	// rotate if we need to
	if (lastDir != currentDir)
	{
		// get the direction to turn
		turnDir = currentDir - lastDir;
		if ( turnDir < 0)
				turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to new walk direction
		// for george and nico put in a head turn at the start
		if ((megaId == GEORGE) || (megaId == NICO))
		{
			if ( turnDir < 0)	// new frames for turn frames	29oct95jps
			{
				module =	turnFramesLeft + lastDir;
			}
			else
			{
				module =	turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}

		// rotate till were facing new dir then go back 45 degrees
		while (lastDir != currentDir)
		{
			lastDir += turnDir;
			if ( turnDir < 0)	// new frames for turn frames	29oct95jps
			{
				if ( lastDir < 0)
					lastDir += NO_DIRECTIONS;
				module =	turnFramesLeft + lastDir;
			}
			else
			{
				if ( lastDir > 7)
					lastDir -= NO_DIRECTIONS;
				module =	turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
		// the back 45 degrees bit
		stepCount -= 1;// step back one because new head turn for george takes us past the new dir
	}

	//****************************************************************************
	// SOLID
	// THE SLOW IN
	//****************************************************************************

	// do start frames if its george and left or right
	if (megaId == GEORGE)
	{
		if (modularPath[1].num > 0)
		{
			if (currentDir == 2) // only for george
			{
				slowStart = 1;
				walkAnim[stepCount].frame = 296;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
				walkAnim[stepCount].frame = 297;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
				walkAnim[stepCount].frame = 298;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
			}
			else if (currentDir == 6) // only for george
			{
				slowStart = 1;
				walkAnim[stepCount].frame = 299;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
				walkAnim[stepCount].frame = 300;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
				walkAnim[stepCount].frame = 301;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
			}
		}
	}
	//****************************************************************************
	// SOLID
	// THE WALK
	//****************************************************************************

	if (currentDir > 4)
		left = framesPerStep;
	else
		left = 0;

	lastCount = stepCount;
	lastDir = 99;// this ensures that we don't put in turn frames for the start
	currentDir = 99;// this ensures that we don't put in turn frames for the start

	do
	{
		while (modularPath[p].num > 0)
		{
			currentDir = modularPath[p].dir;
			if (currentDir< NO_DIRECTIONS)
			{

				module =	currentDir * framesPerStep * 2 + left;
				if (left == 0)
					left = framesPerStep;
				else
					left = 0;
				moduleEnd = module + framesPerStep;
				step = 0;
				scale = (scaleA * moduleY + scaleB);
				do
				{
					module16X += _dx[module]*scale;
					module16Y += _dy[module]*scale;
					moduleX = module16X >> 16;
					moduleY = module16Y >> 16;
					walkAnim[stepCount].frame = module;
					walkAnim[stepCount].step = step;
					walkAnim[stepCount].dir = currentDir;
					walkAnim[stepCount].x = moduleX;
					walkAnim[stepCount].y = moduleY;
					stepCount += 1;
					module += 1;
					step += 1;
				}
				while ( module < moduleEnd) ;
				errorX = modularPath[p].x -	moduleX;
				errorX = errorX * modX[modularPath[p].dir];
				errorY = modularPath[p].y -	moduleY;
				errorY = errorY * modY[modularPath[p].dir];
				if ((errorX < 0) || (errorY < 0))
				{
					modularPath[p].num = 0;
					stepCount -= framesPerStep;
					if (left == 0)
						left = framesPerStep;
					else
						left = 0;
					// Okay this is the end of a section
					moduleX = walkAnim[stepCount-1].x;
					moduleY =	walkAnim[stepCount-1].y;
					module16X = moduleX << 16;
					module16Y = moduleY << 16;
					modularPath[p].x =moduleX;
					modularPath[p].y =moduleY;
					// Now is the time to put in the turn frames for the last turn
					if ((stepCount - lastCount) < framesPerStep)// no step taken
					{
						currentDir = 99;// this ensures that we don't put in turn frames for this walk or the next
						if (slowStart == 1)// clean up if a slow in but no walk
						{
							stepCount -= 3;
							lastCount -= 3;
							slowStart = 0;
						}
					}
					// check each turn condition in turn
					if (((lastDir != 99) && (currentDir != 99)) && (megaId == GEORGE)) // only for george
					{
						lastDir = currentDir - lastDir;//1 and -7 going right -1 and 7 going left
						if (((lastDir == -1) || (lastDir == 7)) || ((lastDir == -2) || (lastDir == 6)))
						{
							// turn at the end of the last walk
							frame = lastCount - framesPerStep;
							do
							{
								walkAnim[frame].frame += 104;//turning left
								frame += 1;
							}
							while (frame < lastCount );
						}
						if (((lastDir == 1) || (lastDir == -7)) || ((lastDir == 2) || (lastDir == -6)))
						{
							// turn at the end of the current walk
							frame = lastCount - framesPerStep;
							do
							{
								walkAnim[frame].frame += 200; //was 60 now 116
								frame += 1;
							}
							while (frame < lastCount );
						}
					}
					// all turns checked
					lastCount = stepCount;
				}
			}
		}
		p = p + 1;
		lastDir = currentDir;
		slowStart = 0; //can only be valid first time round
	}
	while (modularPath[p].dir < NO_DIRECTIONS);

	//****************************************************************************
	// SOLID
	// THE SLOW OUT
	//****************************************************************************

	if ((currentDir == 2) && (megaId == GEORGE)) // only for george
	{
		// place stop frames here
		// slowdown at the end of the last walk
		frame = lastCount - framesPerStep;
		if (walkAnim[frame].frame == 24)
		{
			do
			{
				walkAnim[frame].frame += 278;//stopping right
				frame += 1;
			}
			while (frame < lastCount );
			walkAnim[stepCount].frame = 308;
			walkAnim[stepCount].step = 7;
			walkAnim[stepCount].dir = currentDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
		else if (walkAnim[frame].frame == 30)
		{
			do
			{
				walkAnim[frame].frame += 279;//stopping right
				frame += 1;
			}
			while (frame < lastCount );
			walkAnim[stepCount].frame = 315;
			walkAnim[stepCount].step = 7;
			walkAnim[stepCount].dir = currentDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
	}
	else if ((currentDir == 6) && (megaId == GEORGE)) // only for george
	{
		// place stop frames here
		// slowdown at the end of the last walk
		frame = lastCount - framesPerStep;
		if (walkAnim[frame].frame == 72)
		{
			do
			{
				walkAnim[frame].frame += 244;//stopping left
				frame += 1;
			}
			while (frame < lastCount );
			walkAnim[stepCount].frame = 322;
			walkAnim[stepCount].step = 7;
			walkAnim[stepCount].dir = currentDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
		else if (walkAnim[frame].frame == 78)
		{
			do
			{
				walkAnim[frame].frame += 245;//stopping left
				frame += 1;
			}
			while (frame < lastCount );
			walkAnim[stepCount].frame = 329;
			walkAnim[stepCount].step = 7;
			walkAnim[stepCount].dir = currentDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
	}
	module =	framesPerChar + modularPath[p-1].dir;
	walkAnim[stepCount].frame = module;
	walkAnim[stepCount].step = 0;
	walkAnim[stepCount].dir = modularPath[p-1].dir;
	walkAnim[stepCount].x = moduleX;
	walkAnim[stepCount].y = moduleY;
	stepCount += 1;

	walkAnim[stepCount].frame = 512;
	stepCount += 1;
	walkAnim[stepCount].frame = 512;
	stepCount += 1;
	walkAnim[stepCount].frame = 512;

	//****************************************************************************
	// SOLID
	// NO END TURNS
	//****************************************************************************

//	Tdebug("RouteFinder RouteSize is %d", stepCount);
//	now check the route
	i = 0;
	do
	{
		if (!Check(modularPath[i].x, modularPath[i].y, modularPath[i+1].x, modularPath[i+1].y))
			p=0;
#ifdef PLOT_PATHS
		RouteLine(modularPath[i].x, modularPath[i].y, modularPath[i+1].x, modularPath[i+1].y, 227);
#endif
		i += 1;
	}
	while (i<p-1);
	if (p != 0)
	{
		targetDir =	modularPath[p-1].dir;
	}
	if (p != 0)
	{
		if (CheckTarget(moduleX,moduleY) == 3)// new target on a line
		{
			p = 0;
			//Tdebug("Solid walk target was on a line %d %d", moduleX, moduleY);
		}
	}

	return p;
}

// ****************************************************************************
// * THE SCAN ROUTINES
// ****************************************************************************

int32 Router::Scan(int32 level)
/******************************************************************************
 * Called successively from RouteFinder	until no more changes take place in the
 * grid array	ie he best path has been found
 *
 * Scans through every point in the node array and checks if there is a route
 * between each point and if this route gives a new route.
 *
 * This routine could probably halve its processing time if it doubled up on
 * the checks after each route check
 *****************************************************************************/
{
	int32	i;
	int32	k;
	int32 x1;
	int32 y1;
	int32 x2;
	int32 y2;
	int32	distance;
	int32	changed = 0;
 	// For all the nodes that have new values and a distance less than enddist
	// ie dont check for new routes from a point we checked before or from a point
	// that is already further away than the best route so far.
	i = 0;
	do
	{
		if ((node[i].dist < node[nnodes].dist) && (node[i].level == level))
		{
			x1 = node[i].x;
			y1 = node[i].y;
			k=nnodes;
			do
			{
				if (node[k].dist > node[i].dist)
				{
					x2 = node[k].x;
					y2 = node[k].y;

					if (ABS(x2-x1)>(4.5*ABS(y2-y1)))
					{
						distance = (8*ABS(x2-x1)+18*ABS(y2-y1))/(54*8)+1;
					}
					else
					{
						distance = (6*ABS(x2-x1)+36*ABS(y2-y1))/(36*14)+1;
					}

					if ((distance + node[i].dist < node[nnodes].dist) && (distance + node[i].dist < node[k].dist))
					{
						if (NewCheck(0, x1,y1,x2,y2))
						{
							node[k].level = level + 1;
							node[k].dist  = distance + node[i].dist;
							node[k].prev  = i;
							changed = 1;
						}
					}
				}
				k-=1;
			}
			while (k > 0);
		}
		i=i+1;
	}
	while (i < nnodes);
	return changed;
}


int32 Router::NewCheck(int32 status, int32 x1 , int32 y1 , int32 x2 ,int32 y2)
/******************************************************************************
 * NewCheck routine checks if the route between two points can be achieved
 * without crossing any of the bars in the Bars array.
 *
 * NewCheck differs from check in that that 4 route options are considered
 * corresponding to actual walked routes.
 *
 * Note distance doesnt take account of shrinking ???
 *
 * Note Bars array must be properly calculated ie min max dx dy co
 *****************************************************************************/
{
	int32   dx;
	int32   dy;
	int32   dlx;
	int32   dly;
	int32   dirX;
	int32   dirY;
	int32   step1;
	int32   step2;
	int32   step3;
	int32   steps;
	int32   options;

	steps = 0;
	options = 0;
	dx = x2 - x1;
	dy = y2 - y1;
	dirX = 1;
	dirY = 1;
	if (dx < 0)
	{
		dx = -dx;
		dirX = -1;
	}

	if (dy < 0)
	{
		dy = -dy;
		dirY = -1;
	}

	//make the route options
	if ((diagonaly * dx) > (diagonalx * dy))	// dir  = 1,2 or 2,3 or 5,6 or 6,7
	{
		dly = dy;
		dlx = (dy*diagonalx)/diagonaly;
		dx = dx - dlx;
		dlx = dlx * dirX;
		dly = dly * dirY;
		dx = dx * dirX;
		dy = 0;

	 	//options are
		//square, diagonal a code 1 route
		step1 = Check(x1, y1, x1+dx, y1);
		if (step1 != 0)
		{
			step2 = Check(x1+dx, y1, x2, y2);
			if (step2 != 0)
			{
				steps = step1 + step2;	// yes
				options = options + 2;
#ifdef PLOT_PATHS
				if (status == 1)
					RouteLine(x1, y1, x1+dx, y1, 231);
#endif
#ifdef PLOT_PATHS
				if (status == 1)
					RouteLine(x1+dx, y1, x2, y2, 231);
#endif
			}
		}
		//diagonal, square a code 2 route
		if ((steps == 0) || (status == 1))
		{
			step1 = Check(x1, y1, x1+dlx,y1+dly);
			if (step1 != 0)
			{
				step2 = Check(x1+dlx, y2, x2, y2);
				if (step2 != 0)
				{
					steps = step1 + step2;	// yes
					options = options + 4;
#ifdef PLOT_PATHS
					if (status == 1)
						RouteLine(x1, y1, x1+dlx,y1+dly, 231);
#endif
#ifdef PLOT_PATHS
					if (status == 1)
						RouteLine(x1+dlx, y2, x2, y2, 231);
#endif
				}
			}
		}
		//halfsquare, diagonal, halfsquare a code 0 route
		if ((steps == 0) || (status == 1))
		{
			step1 = Check(x1, y1, x1+dx/2, y1);
			if (step1 != 0)
			{
				step2 = Check(x1+dx/2, y1, x1+dx/2+dlx, y2);
				if (step2 != 0)
				{
					step3 = Check(x1+dx/2+dlx, y2, x2, y2);
					if (step3 != 0)
					{
						steps = step1 + step2 + step3;	// yes
						options = options + 1;
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1, y1, x1+dx/2, y1, 231);
#endif
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1+dx/2, y1, x1+dx/2+dlx, y2, 231);
#endif
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1+dx/2+dlx, y2, x2, y2, 231);
#endif
					}
				}
			}
		}
		//halfdiagonal, square, halfdiagonal a code 3 route
		if ((steps == 0) || (status == 1))
		{
			step1 = Check(x1, y1, x1+dlx/2, y1+dly/2);
			if (step1 != 0)
			{
				step2 = Check(x1+dlx/2, y1+dly/2, x1+dx+dlx/2, y1+dly/2);
				if (step2 != 0)
				{
					step3 = Check(x1+dx+dlx/2, y1+dly/2, x2, y2);
					if (step3 != 0)
					{
						steps = step1 + step2 + step3;	// yes
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1, y1, x1+dlx/2, y1+dly/2, 231);
#endif
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1+dlx/2, y1+dly/2, x1+dx+dlx/2, y1+dly/2, 231);
#endif
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1+dx+dlx/2, y1+dly/2, x2, y2, 231);
#endif
						options = options + 8;
					}
				}
			}
		}
	}
	else // dir  = 7,0 or 0,1 or 3,4 or 4,5
	{
		dlx = dx;
		dly = (dx*diagonaly)/diagonalx;
		dy = dy - dly;
		dlx = dlx * dirX;
		dly = dly * dirY;
		dy = dy * dirY;
		dx = 0;

	 	//options are
		//square, diagonal a code 1 route
		step1 = Check(x1 ,y1 ,x1 ,y1+dy );
		if (step1 != 0)
		{
			step2 = Check(x1 ,y1+dy ,x2,y2);
			if (step2 != 0)
			{
				steps = step1 + step2;	// yes
#ifdef PLOT_PATHS
				if (status == 1)
					RouteLine(x1 ,y1 ,x1 ,y1+dy, 231);
#endif
#ifdef PLOT_PATHS
				if (status == 1)
					RouteLine(x1 ,y1+dy ,x2, y2, 231);
#endif
				options = options + 2;
			}
		}
		//diagonal, square a code 2 route
		if ((steps == 0) || (status == 1))
		{
			step1 = Check(x1, y1, x2, y1+dly);
			if (step1 != 0)
			{
				step2 = Check(x2, y1+dly, x2, y2);
				if (step2 != 0)
				{
					steps = step1 + step2;	// yes
#ifdef PLOT_PATHS
					if (status == 1)
						RouteLine(x1, y1, x2, y1+dly, 231);
#endif
#ifdef PLOT_PATHS
					if (status == 1)
						RouteLine(x2, y1+dly, x2, y2, 231);
#endif
					options = options + 4;
				}
			}
		}
		//halfsquare, diagonal, halfsquare a code 0 route
		if ((steps == 0) || (status == 1))
		{
			step1 = Check(x1, y1, x1, y1+dy/2);
			if (step1 != 0)
			{
				step2 = Check(x1, y1+dy/2, x2, y1+dy/2+dly);
				if (step2 != 0)
				{
					step3 = Check(x2, y1+dy/2+dly, x2, y2);
					if (step3 != 0)
					{
						steps = step1 + step2 + step3;	// yes
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1, y1, x1, y1+dy/2, 231);
#endif
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1, y1+dy/2, x2, y1+dy/2+dly, 231);
#endif
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x2, y1+dy/2+dly, x2, y2, 231);
#endif
						options = options + 1;
					}
				}
			}
		}
		//halfdiagonal, square, halfdiagonal a code 3 route
		if ((steps == 0) || (status == 1))
		{
			step1 = Check(x1, y1, x1+dlx/2, y1+dly/2);
			if (step1 != 0)
			{
				step2 = Check(x1+dlx/2, y1+dly/2, x1+dlx/2, y1+dy+dly/2);
				if (step2 != 0)
				{
					step3 = Check(x1+dlx/2, y1+dy+dly/2, x2, y2);
					if (step3 != 0)
					{
						steps = step1 + step2 + step3;	// yes
						options = options + 8;
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1, y1, x1+dlx/2, y1+dly/2, 231);
#endif
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1+dlx/2, y1+dly/2, x1+dlx/2, y1+dy+dly/2, 231);
#endif
#ifdef PLOT_PATHS
						if (status == 1)
							RouteLine(x1+dlx/2, y1+dy+dly/2, x2, y2, 231);
#endif
					}
				}
			}
		}
	}
	if (status == 0)
	{
		status = steps;
	}
	else
	{
		status = options;
	}
	return status;
}

// ****************************************************************************
// * CHECK ROUTINES
// ****************************************************************************

int32 Router::Check(int32 x1 , int32 y1 , int32 x2 ,int32 y2)
{
//call the fastest line check for the given line
//returns 1 if line didn't cross any bars
	int32   steps;

	if ((x1 == x2) &&	(y1 == y2))
	{
		steps = 1;
	}
	else if (x1 == x2)
	{
			steps = VertCheck(x1, y1, y2);
	}
	else if (y1 == y2)
	{
			steps = HorizCheck(x1, y1, x2);
	}
	else
	{
			steps = LineCheck(x1, y1, x2, y2);
	}
	return steps;

}


int32 Router::LineCheck(int32 x1 , int32 y1 , int32 x2 ,int32 y2)
{
	int32   dirx;
	int32   diry;
	int32   co;
	int32   slope;
	int32   i;
	int32   xc;
	int32   yc;
	int32   xmin;
	int32   ymin;
	int32   xmax;
	int32   ymax;
	int32   linesCrossed = 1;


	if (x1 > x2)
	{
		xmin = x2;
		xmax = x1;
	}
	else
	{
		xmin = x1;
		xmax = x2;
	}
	if (y1 > y2)
	{
		ymin = y2;
		ymax = y1;
	}
	else
	{
		ymin = y1;
		ymax = y2;
	}
	//line set to go one step in chosen direction
	//so ignore if it hits anything
	dirx = x2 - x1;
	diry = y2 - y1;
	co = (y1 *dirx)- (x1*diry);       //new line equation

	i = 0;
	do
	{
		// this is the inner inner loop
		if ((xmax >= bars[i].xmin) && ( xmin <= bars[i].xmax))  //skip if not on module
		{
			if ((ymax >= bars[i].ymin) && ( ymin <= bars[i].ymax))  //skip if not on module
			{
				// okay its a valid line calculate an intersept
				// wow but all this arithmatic we must have loads of time
				slope = (bars[i].dx * diry) - (bars[i].dy *dirx);// slope it he slope between the two lines
				if (slope != 0)//assuming parallel lines don't cross
				{
					//calculate x intercept and check its on both lines
					xc = ((bars[i].co * dirx) - (co * bars[i].dx)) / slope;

					if ((xc >= xmin-1) && (xc <= xmax+1))   //skip if not on module
					{
						if ((xc >= bars[i].xmin-1) && (xc <= bars[i].xmax+1))   //skip if not on line
						{

							yc = ((bars[i].co * diry) - (co * bars[i].dy)) / slope;

							if ((yc >= ymin-1) && (yc <= ymax+1))   //skip if not on module
							{
								if ((yc >= bars[i].ymin-1) && (yc <= bars[i].ymax+1))   //skip if not on line
								{
									linesCrossed = 0;
								}
							}
						}
					}
				}
			}
		}
		i = i + 1;
	}
	while ((i < nbars) && linesCrossed);

	return linesCrossed;
}

int32 Router::HorizCheck(int32 x1 , int32 y , int32 x2)
{
	int32   dy;
	int32   i;
	int32   xc;
	int32   xmin;
	int32   xmax;
	int32   linesCrossed = 1;

	if (x1 > x2)
	{
		xmin = x2;
		xmax = x1;
	}
	else
	{
		xmin = x1;
		xmax = x2;
	}
	//line set to go one step in chosen direction
	//so ignore if it hits anything

	i = 0;
	do
	{
		// this is the inner inner loop
		if ((xmax >= bars[i].xmin) && ( xmin <= bars[i].xmax))  //skip if not on module
		{
			if ((y >= bars[i].ymin) && ( y <= bars[i].ymax))  //skip if not on module
			{
				// okay its a valid line calculate an intersept
				// wow but all this arithmatic we must have loads of time
				if (bars[i].dy == 0)
				{
					linesCrossed = 0;
				}
				else
				{
					dy = y-bars[i].y1;
					xc = bars[i].x1 + (bars[i].dx * dy)/bars[i].dy;
					if ((xc >= xmin-1) && (xc <= xmax+1))   //skip if not on module
					{
						linesCrossed = 0;
					}
				}
			}
		}
		i = i + 1;
	}
	while ((i < nbars) && linesCrossed);

	return linesCrossed;
}


int32 Router::VertCheck(int32 x, int32 y1, int32 y2)
{
	int32   dx;
	int32   i;
	int32   yc;
	int32   ymin;
	int32   ymax;
	int32   linesCrossed = 1;

	if (y1 > y2)
	{
		ymin = y2;
		ymax = y1;
	}
	else
	{
		ymin = y1;
		ymax = y2;
	}
	//line set to go one step in chosen direction
	//so ignore if it hits anything
	i = 0;
	do		// this is the inner inner loop
	{
		if ((x >= bars[i].xmin) && ( x <= bars[i].xmax))  //overlapping
		{
			if ((ymax >= bars[i].ymin) && ( ymin <= bars[i].ymax))  //skip if not on module
			{
				// okay its a valid line calculate an intersept
				// wow but all this arithmatic we must have loads of time
				if (bars[i].dx == 0)//both lines vertical and overlap in x and y so they cross
				{
					linesCrossed = 0;
				}
				else
				{
					dx = x-bars[i].x1;
					yc = bars[i].y1 + (bars[i].dy * dx)/bars[i].dx;
					if ((yc >= ymin-1) && (yc <= ymax+1))   //the intersept overlaps
					{
						linesCrossed = 0;
					}
				}
			}
		}
		i = i + 1;
	}
	while ((i < nbars) && linesCrossed);

	return linesCrossed;
}

int32 Router::CheckTarget(int32 x , int32 y)
{
	int32   dx;
	int32   dy;
	int32   i;
	int32   xc;
	int32   yc;
	int32   xmin;
	int32   xmax;
	int32   ymin;
	int32   ymax;
	int32   onLine = 0;

 	xmin = x - 1;
 	xmax = x + 1;
 	ymin = y - 1;
 	ymax = y + 1;

	// check if point +- 1 is on the line
	//so ignore if it hits anything

	i = 0;
	do
	{

		// this is the inner inner loop

		if ((xmax >= bars[i].xmin) && ( xmin <= bars[i].xmax))  //overlapping line
		{
			if ((ymax >= bars[i].ymin) && ( ymin <= bars[i].ymax))  //overlapping line
			{

				// okay this line overlaps the target calculate an y intersept for x

				if (bars[i].dx == 0)// vertical line so we know it overlaps y
				{
					yc = 0;
				}
				else
				{
					dx = x-bars[i].x1;
					yc = bars[i].y1 + (bars[i].dy * dx)/bars[i].dx;
				}

				if ((yc >= ymin) && (yc <= ymax))   //overlapping point for y
				{
					onLine = 3;// target on a line so drop out
					//Tdebug("RouteFail due to target on a line %d %d",x,y);
				}
				else
				{
					if (bars[i].dy == 0)// vertical line so we know it overlaps y
					{
						xc = 0;
					}
					else
					{
						dy = y-bars[i].y1;
						xc = bars[i].x1 + (bars[i].dx * dy)/bars[i].dy;
					}

					if ((xc >= xmin) && (xc <= xmax))   //skip if not on module
					{
						onLine = 3;// target on a line so drop out
						//Tdebug("RouteFail due to target on a line %d %d",x,y);
					}
				}
			}
		}
		i = i + 1;
	}
	while ((i < nbars) && (onLine == 0));

	return onLine;
}

// ****************************************************************************
// * THE SETUP ROUTINES
// ****************************************************************************

int32 Router::LoadWalkResources(Object *megaObject, int32 x, int32 y, int32 dir)
{
	WalkGridHeader	floorHeader;
	int32 	i;
	int32 	j;
	uint8  *fPolygrid;
	uint8  *fMegaWalkData;

	int32 	floorId;
	int32		walkGridResourceId;
	Object *floorObject;

	int32  cnt;
	uint32 cntu;

	// load in floor grid for current mega

	floorId = megaObject->o_place;

	//floorObject = (object *) Lock_object(floorId);
	floorObject = _objMan->fetchObject(floorId);
	walkGridResourceId = floorObject->o_resource;
	//Unlock_object(floorId);

	//ResOpen(walkGridResourceId);			// mouse wiggle
	//fPolygrid = ResLock(walkGridResourceId);			// mouse wiggle
	fPolygrid = (uint8*)_resMan->openFetchRes(walkGridResourceId);


 	fPolygrid += sizeof(Header);
 	memcpy(&floorHeader,fPolygrid,sizeof(WalkGridHeader));
 	fPolygrid += sizeof(WalkGridHeader);
	nbars = FROM_LE_32(floorHeader.numBars);

	if (nbars >= O_GRID_SIZE)
	{
		#ifdef DEBUG												//check for id > number in file,
		error("RouteFinder Error too many bars %d", nbars);
		#endif
		nbars = 0;
	}

	nnodes = FROM_LE_32(floorHeader.numNodes)+1;	//array starts at 0	begins at a start node has nnodes nodes and a target node

	if (nnodes >= O_GRID_SIZE)
	{
		#ifdef DEBUG												//check for id > number in file,
			error("RouteFinder Error too many nodes %d", nnodes);
		#endif
		nnodes = 0;
	}

 	/*memmove(&bars[0],fPolygrid,nbars*sizeof(BarData));
	fPolygrid += nbars*sizeof(BarData);//move pointer to start of node data*/
	for (cnt = 0; cnt < nbars; cnt++) {
		bars[cnt].x1   = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		bars[cnt].y1   = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		bars[cnt].x2   = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		bars[cnt].y2   = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		bars[cnt].xmin = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		bars[cnt].ymin = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		bars[cnt].xmax = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		bars[cnt].ymax = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		bars[cnt].dx   = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		bars[cnt].dy   = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		bars[cnt].co   = READ_LE_UINT32(fPolygrid); fPolygrid += 4;
	}

	/*j = 1;// leave node 0 for start node
	do
	{
		memmove(&node[j].x,fPolygrid,2*sizeof(int16));
		fPolygrid += 2*sizeof(int16);
		j ++;
	}
	while (j < nnodes);//array starts at 0*/
	for (cnt = 1; cnt < nnodes; cnt++) {
		node[cnt].x = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
		node[cnt].y = READ_LE_UINT16(fPolygrid); fPolygrid += 2;
	}

	//ResUnlock(walkGridResourceId);			// mouse wiggle
	//ResClose(walkGridResourceId);			// mouse wiggle
	_resMan->resClose(walkGridResourceId);


	// floor grid loaded
	// if its george copy extra bars and nodes

	if (megaId == GEORGE)
	{
		// copy any extra bars from extraBars array

		//Zdebug("%d", nExtraBars);

	 	memmove(&bars[nbars], &_extraBars[0], _numExtraBars*sizeof(BarData));
		nbars += _numExtraBars;

		// copy any extra nodes from extraNode array
		j = 0;
		while (j < _numExtraNodes)//array starts at 0
		{
			node[nnodes+j].x = _extraNodes[j].x ;
			node[nnodes+j].y = _extraNodes[j].y ;
			j++;
		}

		nnodes += _numExtraNodes;
	}

// copy the mega structure into the local variables for use in all subroutines

	startX = megaObject->o_xcoord;
	startY = megaObject->o_ycoord;
	startDir = megaObject->o_dir;
	targetX = x;
	targetY= y;
	targetDir = dir;

	scaleA = megaObject->o_scale_a;
	scaleB = megaObject->o_scale_b;

	//ResOpen(megaObject->o_mega_resource);			// mouse wiggle
	//fMegaWalkData = ResLock(megaObject->o_mega_resource);			// mouse wiggle
	fMegaWalkData = (uint8*)_resMan->openFetchRes(megaObject->o_mega_resource);

	nWalkFrames = fMegaWalkData[0];
	nTurnFrames = fMegaWalkData[1];
 	fMegaWalkData += 2;

	for (cnt = 0; cnt < NO_DIRECTIONS * (nWalkFrames + 1 + nTurnFrames); cnt++) {
		_dx[cnt] = (int32)READ_LE_UINT32(fMegaWalkData);
		fMegaWalkData += 4;
	}
	for (cnt = 0; cnt < NO_DIRECTIONS * (nWalkFrames + 1 + nTurnFrames); cnt++) {
		_dy[cnt] = (int32)READ_LE_UINT32(fMegaWalkData);
		fMegaWalkData += 4;
	}
 	/*memmove(&_dx[0],fMegaWalkData,NO_DIRECTIONS*(nWalkFrames+1+nTurnFrames)*sizeof(int32));
 	fMegaWalkData += NO_DIRECTIONS*(nWalkFrames+1+nTurnFrames)*sizeof(int32);
 	memmove(&_dy[0],fMegaWalkData,NO_DIRECTIONS*(nWalkFrames+1+nTurnFrames)*sizeof(int32));
 	fMegaWalkData += NO_DIRECTIONS*(nWalkFrames+1+nTurnFrames)*sizeof(int32);*/

	for (cntu = 0; cntu < NO_DIRECTIONS; cntu++) {
		modX[cntu] = (int32)READ_LE_UINT32(fMegaWalkData);
		fMegaWalkData += 4;
	}
	for (cntu = 0; cntu < NO_DIRECTIONS; cntu++) {
		modY[cntu] = (int32)READ_LE_UINT32(fMegaWalkData);
		fMegaWalkData += 4;
	}
 	/*memmove(&modX[0],fMegaWalkData,NO_DIRECTIONS*sizeof(int32));
 	fMegaWalkData += NO_DIRECTIONS*sizeof(int32);
 	memmove(&modY[0],fMegaWalkData,NO_DIRECTIONS*sizeof(int32));
 	fMegaWalkData += NO_DIRECTIONS*sizeof(int32);*/

	//ResUnlock(megaObject->o_mega_resource);			// mouse wiggle
	//ResClose(megaObject->o_mega_resource);			// mouse wiggle
	_resMan->resClose(megaObject->o_mega_resource);

	diagonalx =  modX[3] ;//36
	diagonaly =  modY[3] ;//8

// mega data ready

// finish setting grid by putting mega node at begining
// and target node at end	and reset current values
	node[0].x = startX;
	node[0].y = startY;
	node[0].level = 1;
	node[0].prev = 0;
	node[0].dist = 0;
	i=1;
	do
	{
		node[i].level = 0;
		node[i].prev = 0;
		node[i].dist = 9999;
		i=i+1;
	}
	while (i < nnodes);
	node[nnodes].x = targetX;
	node[nnodes].y = targetY;
	node[nnodes].level = 0;
	node[nnodes].prev = 0;
	node[nnodes].dist = 9999;

	return 1;
}

// ****************************************************************************
// * THE ROUTE EXTRACTOR
// ****************************************************************************

void	Router::ExtractRoute()
/****************************************************************************
 *    ExtractRoute	gets route from the node data after a full scan, route is
 *		written with just the basic way points and direction options for heading
 *		to the next point.
 ****************************************************************************/
{
	int32	prev;
	int32	prevx;
	int32	prevy;
	int32	last;
	int32	point;
	int32 p;
	int32	dirx;
	int32	diry;
	int32	dir;
	int32	dx;
	int32	dy;


 	// extract the route from the node data
	prev = nnodes;
	last = prev;
	point = O_ROUTE_SIZE - 1;
	route[point].x = node[last].x;
	route[point].y = node[last].y;
	do
	{
		point = point -	1;
		prev = node[last].prev;
		prevx = node[prev].x;
		prevy = node[prev].y;
		route[point].x = prevx;
		route[point].y = prevy;
		last = prev;
	}
	while (prev > 0);

	// now shuffle route down in the buffer
	routeLength = 0;
	do
	{
		route[routeLength].x = route[point].x;
		route[routeLength].y = route[point].y;
		point = point + 1;
		routeLength = routeLength + 1;
	}
	while (point < O_ROUTE_SIZE);
	routeLength = routeLength - 1;

	// okay the route exists as a series point now put in some directions
	p = 0;
	do
	{
#ifdef PLOT_PATHS
		BresenhamLine(route[p+1].x-128,route[p+1].y-128, route[p].x-128,route[p].y-128, (uint8*)screen_ad, true_pixel_size_x, pixel_size_y, ROUTE_END_FLAG);
#endif
		dx = route[p+1].x - route[p].x;
		dy = route[p+1].y - route[p].y;
		dirx = 1;
		diry = 1;
		if (dx < 0)
		{
			dx = -dx;
			dirx = -1;
		}
		if (dy < 0)
		{
			dy = -dy;
			diry = -1;
		}

		if ((diagonaly * dx) > (diagonalx * dy))	// dir  = 1,2 or 2,3 or 5,6 or 6,7
		{
			dir = 4 - 2 * dirx;	 // 2 or 6
			route[p].dirS = dir;
			dir = dir + diry * dirx; // 1,3,5 or 7
			route[p].dirD = dir;
		}
		else // dir  = 7,0 or 0,1 or 3,4 or 4,5
		{
			dir = 2 + 2 * diry;	// 0 or 4
			route[p].dirS = dir;
			dir = 4 - 2 * dirx;	 // 2 or 6
			dir = dir + diry * dirx; // 1,3,5 or 7
			route[p].dirD = dir;
		}
		p = p + 1;
	}
	while (p < (routeLength));
	// set the last dir to continue previous route unless specified
	if (targetDir == NO_DIRECTIONS)
	{
		route[p].dirS = route[p-1].dirS;
		route[p].dirD = route[p-1].dirD;
	}
	else
	{
		route[p].dirS = targetDir;
		route[p].dirD = targetDir;
	}
	return;
}

#define screen_ad NULL
#define pixel_size_y 1
#define true_pixel_size_x 1
void Router::RouteLine(int32 x1,int32 y1,int32 x2,int32 y2 ,int32 colour)
{
	BresenhamLine(x1-128, y1-128, x2-128, y2-128, (uint8*)screen_ad, true_pixel_size_x, pixel_size_y, colour);
	return;
}

void Router::BresenhamLine(int32 x1,int32 y1,int32 x2,int32 y2, uint8 *screen, int32 width, int32 height, int32 colour) {

}

#define DIAGONALX 36
#define DIAGONALY 8
int whatTarget(int32 startX, int32 startY, int32 destX, int32 destY) {
	int tar_dir;
//setting up
	int deltaX = destX-startX;
	int deltaY = destY-startY;
	int signX = (deltaX > 0);
	int signY = (deltaY > 0);
	int	slope;

	if ( (ABS(deltaY) * DIAGONALX ) < (ABS(deltaX) * DIAGONALY / 2))
		slope = 0;// its flat
	else if ( (ABS(deltaY) * DIAGONALX / 2) > (ABS(deltaX) * DIAGONALY ) )
		slope = 2;// its vertical
	else
		slope = 1;// its diagonal

	if (slope == 0) { //flat
		if (signX == 1)	// going right
			tar_dir = 2;
		else
			tar_dir = 6;
	} else if (slope == 2) { //vertical
		if (signY == 1)	// going down
			tar_dir = 4;
		else
			tar_dir = 0;
	} else if (signX == 1) { //right diagonal
		if (signY == 1)	// going down
			tar_dir = 3;
		else
			tar_dir = 1;
	} else { //left diagonal
		if (signY == 1)	// going down
			tar_dir = 5;
		else
			tar_dir = 7;
	}
	return tar_dir;
}

void Router::resetExtraData(void) {
	_numExtraBars = _numExtraNodes = 0;
}

void Router::setPlayerTarget(int32 x, int32 y, int32 dir, int32 stance) {
	_playerTargetX = x;
	_playerTargetY = y;
	_playerTargetDir = dir;
	_playerTargetStance = stance;
}

} // End of namespace Sword1
