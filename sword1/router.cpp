/* ScummVM - Scumm Interpreter
 * Copyright (C) Revolution Software Ltd.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "router.h"
#include "util.h"
#include "objectman.h"
#include "resman.h"
#include "sworddefs.h"

#define SLOW_IN 3
#define SLOW_OUT 7

SwordRouter::SwordRouter(ObjectMan *pObjMan, ResMan *pResMan) {
	_objMan = pObjMan;
	_resMan = pResMan;
	_numExtraBars = _numExtraNodes = 0;
	_nNodes = _nBars = 0;
	_playerTargetX = _playerTargetY = _playerTargetDir = _playerTargetStance = 0;
}

int SwordRouter::routeFinder(int32 id, BsObject *mega, int32 x, int32 y, int32 targetDir) {
	loadWalkResources(id, mega, x, y, targetDir); // init vars for subs
	// init offset pointers
	FrameInfos frameInfo;

	frameInfo.framesPerStep = _nWalkFrames / 2;
	frameInfo.framesPerChar = _nWalkFrames * NO_DIRECTIONS;
	frameInfo.standFrames = frameInfo.framesPerChar;
	frameInfo.turnFramesLeft = frameInfo.framesPerChar;
	frameInfo.turnFramesRight = frameInfo.framesPerChar;
	frameInfo.walkFramesLeft = 0;
	frameInfo.walkFramesRight = 0;
	frameInfo.slowInFrames = 0;
	frameInfo.slowOutFrames = 0;
	frameInfo.startX = mega->o_xcoord;
	frameInfo.startY = mega->o_ycoord;
	frameInfo.targetX = x;
	frameInfo.targetY = y;
	frameInfo.targetDir = targetDir;
	frameInfo.scaleA = mega->o_scale_a;
	frameInfo.scaleB = mega->o_scale_b;

	if (id == GEORGE) {
		frameInfo.turnFramesLeft = 3 * frameInfo.framesPerChar + NO_DIRECTIONS + 2 * SLOW_IN + 4 * SLOW_OUT;
		frameInfo.turnFramesRight = 3 * frameInfo.framesPerChar + NO_DIRECTIONS + 2 * SLOW_IN + 4 * SLOW_OUT + NO_DIRECTIONS;
		frameInfo.walkFramesLeft = frameInfo.framesPerChar + NO_DIRECTIONS;
		frameInfo.walkFramesRight = 2 * frameInfo.framesPerChar + NO_DIRECTIONS;
		frameInfo.slowInFrames = 3 * frameInfo.framesPerChar + NO_DIRECTIONS;
		frameInfo.slowOutFrames = 3 * frameInfo.framesPerChar + NO_DIRECTIONS + 2 * SLOW_IN;
	} else if (id == NICO) {
		frameInfo.turnFramesLeft = frameInfo.framesPerChar + NO_DIRECTIONS;
		frameInfo.turnFramesRight = frameInfo.framesPerChar + 2 * NO_DIRECTIONS;
		frameInfo.walkFramesLeft = 0;	
		frameInfo.walkFramesRight = 0;
		frameInfo.slowInFrames = 0;
		frameInfo.slowOutFrames = 0;
	}
	int32 routeFlag = getRoute();
	int32 routeLength;
	if (routeFlag == 1) {
		// extract the route as nodes and the directions to go between each node
		// route.X,route.Y and route.Dir now hold all the route infomation with
		// the target dir or route continuation
		routeLength = extractRoute(targetDir);
	}
	int32 solidFlag = 0;
	if (routeFlag == 2)  //special case for zero length route
	{
		if (targetDir >7)// if target direction specified as any
			targetDir = mega->o_dir;
		// just a turn on the spot is required set an end module for the route let the animator deal with it
		// modularPath is normally set by ExtractRoute
		_modularPath[0].dir = mega->o_dir;
 		_modularPath[0].num = 0;
		_modularPath[0].x = mega->o_xcoord;
		_modularPath[0].y = mega->o_ycoord;
		_modularPath[1].dir = targetDir;
 		_modularPath[1].num = 0;
 		_modularPath[1].x = mega->o_xcoord;
 		_modularPath[1].y = mega->o_ycoord;
 		_modularPath[2].dir = 9;
 		_modularPath[2].num = ROUTE_END_FLAG;

 		slidyWalkAnimator(mega->o_route, &frameInfo, id);
 		routeFlag = 2;
	} else if (routeFlag == 1) { // a normal route
		smoothestPath(mega->o_xcoord, mega->o_ycoord, mega->o_dir, routeLength);//Converts the route to an exact path
		// The Route had waypoints and direction options
		// The Path is an exact set of lines in 8 directions that reach the target.
		// The path is in module format, but steps taken in each direction are not accurate   
		// if target dir = 8 then the walk isn't linked to an anim so 
		// we can create a route without sliding and miss the exact target
		if (targetDir == NO_DIRECTIONS) {
			solidPath(mega->o_scale_a, mega->o_scale_b);
			solidFlag = solidWalkAnimator(mega->o_route, &frameInfo, id);
		}

		if (!solidFlag)	{
			slidyPath(mega->o_scale_a, mega->o_scale_b, targetDir);
			slidyWalkAnimator(mega->o_route, &frameInfo, id);
		}
	}
	return routeFlag;
}

void SwordRouter::slidyPath(int32 scaleA, int32 scaleB, uint16 targetDir) {
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
	_modularPath[0].x = _smoothPath[0].x;
	_modularPath[0].y = _smoothPath[0].y;
	_modularPath[0].dir = _smoothPath[0].dir;
	_modularPath[0].num = 0;

	while (_smoothPath[smooth].num < ROUTE_END_FLAG)
	{
		scale = scaleA * _smoothPath[smooth].y + scaleB;
		deltaX = _smoothPath[smooth].x - _modularPath[slidy-1].x;
		deltaY = _smoothPath[smooth].y - _modularPath[slidy-1].y;
		stepX = _modX[_smoothPath[smooth].dir];
		stepY = _modY[_smoothPath[smooth].dir];
		stepX = stepX * scale;
		stepY = stepY * scale;
		stepX = stepX >> 19;// quarter a step minimum
		stepY = stepY >> 19;
		if ((abs(deltaX)>=abs(stepX)) &&	(abs(deltaY)>=abs(stepY)))
		{
	 		_modularPath[slidy].x = _smoothPath[smooth].x;
			_modularPath[slidy].y = _smoothPath[smooth].y;
			_modularPath[slidy].dir = _smoothPath[smooth].dir;
			_modularPath[slidy].num = 1;
			slidy += 1;
		}
		smooth += 1;
	}
	// in	case the last bit had no steps
	if (slidy > 1)
	{
		_modularPath[slidy-1].x = _smoothPath[smooth-1].x;
		_modularPath[slidy-1].y = _smoothPath[smooth-1].y;
	}
	// set up the end of the walk
	_modularPath[slidy].x = _smoothPath[smooth-1].x;
	_modularPath[slidy].y = _smoothPath[smooth-1].y;
	_modularPath[slidy].dir = targetDir;
	_modularPath[slidy].num = 0;
	slidy += 1;
	_modularPath[slidy].x = _smoothPath[smooth-1].x;
	_modularPath[slidy].y = _smoothPath[smooth-1].y;
	_modularPath[slidy].dir = 9;
	_modularPath[slidy].num = ROUTE_END_FLAG;
}

int32 SwordRouter::solidPath(int32 scaleA, int32 scaleB) {
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

	solid = 1;
	smooth = 1;
	_modularPath[0].x = _smoothPath[0].x;
	_modularPath[0].y = _smoothPath[0].y;
	_modularPath[0].dir = _smoothPath[0].dir;
	_modularPath[0].num = 0;

	do {
		scale = scaleA * _smoothPath[smooth].y + scaleB;
		deltaX = _smoothPath[smooth].x - _modularPath[solid-1].x;
		deltaY = _smoothPath[smooth].y - _modularPath[solid-1].y;
		stepX = _modX[_smoothPath[smooth].dir];
		stepY = _modY[_smoothPath[smooth].dir];
		stepX = stepX * scale;
		stepY = stepY * scale;
		stepX = stepX >> 16;
		stepY = stepY >> 16;
		if ((abs(deltaX)>=abs(stepX)) && (abs(deltaY)>=abs(stepY))) {
			_modularPath[solid].x = _smoothPath[smooth].x;
			_modularPath[solid].y = _smoothPath[smooth].y;
			_modularPath[solid].dir = _smoothPath[smooth].dir;
			_modularPath[solid].num = 1;
			solid += 1;
		}
		smooth += 1;
	} while (_smoothPath[smooth].num < ROUTE_END_FLAG);
	// in	case the last bit had no steps
	if (solid == 1) { //there were no paths so put in a dummy end
		solid = 2;
		_modularPath[1].dir = _smoothPath[0].dir;
		_modularPath[1].num = 0;
	}	
	_modularPath[solid-1].x = _smoothPath[smooth-1].x;
	_modularPath[solid-1].y = _smoothPath[smooth-1].y;
	// set up the end of the walk
	_modularPath[solid].x = _smoothPath[smooth-1].x;
	_modularPath[solid].y = _smoothPath[smooth-1].y;
	_modularPath[solid].dir = 9;
	_modularPath[solid].num = ROUTE_END_FLAG;
	return 1;
}

int32 SwordRouter::solidWalkAnimator(WalkData *walkAnim, FrameInfos *frInfo, int32 megaId) {
	int32 p;
	int32 i;
	int32 left;
	int32 lastDir;
	int32 currentDir;
	int32 turnDir;
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
	lastDir = _modularPath[0].dir;
	p = 1;
	currentDir = _modularPath[1].dir;
	module = frInfo->framesPerChar + lastDir;
	moduleX = frInfo->startX;
	moduleY = frInfo->startY;
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
				module = frInfo->turnFramesLeft + lastDir;
			}
			else
			{
				module = frInfo->turnFramesRight + lastDir;
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
					module = frInfo->turnFramesLeft + lastDir;
			}
			else
			{
				if ( lastDir > 7)
					lastDir -= NO_DIRECTIONS;
					module = frInfo->turnFramesRight + lastDir;
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
		if (_modularPath[1].num > 0)
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
		left = frInfo->framesPerStep;
	else
		left = 0;

	lastCount = stepCount;
	lastDir = 99;// this ensures that we don't put in turn frames for the start		
	currentDir = 99;// this ensures that we don't put in turn frames for the start		

	do
	{
		while(_modularPath[p].num > 0)
		{
			currentDir = _modularPath[p].dir;
			if (currentDir< NO_DIRECTIONS)
			{

				module = currentDir * frInfo->framesPerStep * 2 + left;
				if (left == 0)
					left = frInfo->framesPerStep;
				else
					left = 0;
				moduleEnd = module + frInfo->framesPerStep;
				step = 0;
				scale = (frInfo->scaleA * moduleY + frInfo->scaleB);
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
				while( module < moduleEnd) ;
				errorX = _modularPath[p].x -	moduleX;
				errorX = errorX * _modX[_modularPath[p].dir];
				errorY = _modularPath[p].y -	moduleY;
				errorY = errorY * _modY[_modularPath[p].dir];
				if ((errorX < 0) || (errorY < 0))
				{
					_modularPath[p].num = 0;
					stepCount -= frInfo->framesPerStep;
					if (left == 0)
						left = frInfo->framesPerStep;
					else
						left = 0;
					// Okay this is the end of a section
					moduleX = walkAnim[stepCount-1].x;
					moduleY =	walkAnim[stepCount-1].y;
					module16X = moduleX << 16;
					module16Y = moduleY << 16;
					_modularPath[p].x =moduleX;
					_modularPath[p].y =moduleY;
					// Now is the time to put in the turn frames for the last turn
					if ((stepCount - lastCount) < frInfo->framesPerStep)// no step taken
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
							frame = lastCount - frInfo->framesPerStep;
							do
							{
								walkAnim[frame].frame += 104;//turning left 
								frame += 1;
							}
							while(frame < lastCount );
						}
						if (((lastDir == 1) || (lastDir == -7)) || ((lastDir == 2) || (lastDir == -6)))
						{	
							// turn at the end of the current walk
							frame = lastCount - frInfo->framesPerStep;
							do
							{
								walkAnim[frame].frame += 200; //was 60 now 116
								frame += 1;
							}
							while(frame < lastCount );
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
	while (_modularPath[p].dir < NO_DIRECTIONS);

	//****************************************************************************
	// SOLID
	// THE SLOW OUT
	//****************************************************************************

	if ((currentDir == 2) && (megaId == GEORGE)) // only for george
	{
		// place stop frames here
		// slowdown at the end of the last walk
		frame = lastCount - frInfo->framesPerStep;
		if (walkAnim[frame].frame == 24) 
		{
			do
			{
				walkAnim[frame].frame += 278;//stopping right 
				frame += 1;
			}
			while(frame < lastCount );
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
			while(frame < lastCount );
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
		frame = lastCount - frInfo->framesPerStep;
		if (walkAnim[frame].frame == 72) 
		{
			do
			{
				walkAnim[frame].frame += 244;//stopping left 
				frame += 1;
			}
			while(frame < lastCount );
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
			while(frame < lastCount );
			walkAnim[stepCount].frame = 329;
			walkAnim[stepCount].step = 7;
			walkAnim[stepCount].dir = currentDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
	}
	module = frInfo->framesPerChar + _modularPath[p-1].dir;
	walkAnim[stepCount].frame = module;
	walkAnim[stepCount].step = 0;
	walkAnim[stepCount].dir = _modularPath[p-1].dir;
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
			if (!check(_modularPath[i].x, _modularPath[i].y, _modularPath[i+1].x, _modularPath[i+1].y))
				p=0;
			i += 1;
		}
		while(i<p-1);
		if (p != 0) {
			frInfo->targetDir = _modularPath[p-1].dir;
			if (checkTarget(moduleX,moduleY) == 3)// new target on a line
				p = 0;
		}

	return p;
}

int32 SwordRouter::smoothestPath(uint16 startX, uint16 startY, uint16 startDir, int32 routeLength) {
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
	int32	dirS;
	int32	dirD;
	int32	dS;
	int32	dD;
	int32	dSS;
	int32	dSD;
	int32	dDS;
	int32	dDD;
	int32	SS;
	int32	SD;
	int32	DS;
	int32	DD;
	int32	temp;
	int32	steps;
	int32	option;
	int32	options;
	int32 lastDir;
	int32 nextDirS;
	int32 nextDirD;
	int32 tempturns[4];     
	int32 turns[4];     
	int32 turntable[NO_DIRECTIONS] = {0,1,3,5,7,5,3,1};

	// route.X route.Y and route.Dir start at far end
	_smoothPath[0].x = startX;
	_smoothPath[0].y = startY;
	_smoothPath[0].dir = startDir;
	_smoothPath[0].num = 0;
	lastDir = startDir;
	// for each section of the route
	for (int32 p = 0; p < routeLength; p++) {
		dirS = _route[p].dirS;
		dirD = _route[p].dirD;
		nextDirS = _route[p+1].dirS;
		nextDirD = _route[p+1].dirD;

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
		// rate each option
		SS = dS + dSS + 3;		// Split routes look crap so weight against them
		SD = dS + dDD;
		DS = dD + dSS;
		DD = dD + dDD + 3;
		// set up turns as a sorted	array of the turn values
		tempturns[0] = SS;
		turns[0] = 0;
		tempturns[1] = SD;
		turns[1] = 1;
		tempturns[2] = DS;
		turns[2] = 2;
		tempturns[3] = DD;
		turns[3] = 3;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				if (tempturns[j] > tempturns[j + 1]) {
					temp = turns[j];
					turns[j] = turns[j+1];
					turns[j+1] = temp;
					temp = tempturns[j];
					tempturns[j] = tempturns[j+1];
					tempturns[j+1] = temp;
				}
			}
		}

		// best option matched in order of the priority we would like to see on the screen
		// but each option must be checked to see if it can be walked

		options = newCheck(1, _route[p].x, _route[p].y, _route[p + 1].x, _route[p + 1].y);

		if (options == 0)
			error("BestTurns failed");
		steps = 0;
		for (int i = 0; (i < 4) && (steps == 0); i++) {
			option = 1 << turns[i];
			if (option & options)
				steps = smoothCheck(turns[i],p,dirS,dirD);
		}

		if (steps == 0)
			error("BestTurns failed");
		// route.X route.Y route.dir and bestTurns start at far end
	}
	_smoothPath[steps].dir = 9;
	_smoothPath[steps].num = ROUTE_END_FLAG;
	return 1;
}

int32 SwordRouter::smoothCheck(int32 best, int32 p, int32 dirS, int32 dirD) {
	static int32 k;   
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
	int32 ss0;
	int32 ss1;
	int32 ss2;
	int32 sd0;
	int32 sd1;
	int32 sd2;

	if (p == 0)
		k = 1;
	tempK = 0;
	x = _route[p].x;
	y = _route[p].y;
	x2 = _route[p + 1].x;
	y2 = _route[p + 1].y;
	dx = x2 - x;
	dy = y2 - y;
	dirX = 1;
	dirY = 1;
	if (dx < 0) {
		dx = -dx;
		dirX = -1;
	}

	if (dy < 0) {
		dy = -dy;
		dirY = -1;
	}

// set up sd0-ss2 to reflect possible movement in each direction
	if ((dirS == 0)	|| (dirS == 4)) { // vert and diag
		ddx = dx;
		ddy = (dx*_diagonaly)/_diagonalx;
		dsy = dy - ddy;
		ddx = ddx * dirX;
		ddy = ddy * dirY;
		dsy = dsy * dirY;
		dsx = 0;

		sd0 = (ddx + _modX[dirD]/2) / _modX[dirD];
		ss0 = (dsy + _modY[dirS]/2) / _modY[dirS];
		sd1 = sd0/2;
		ss1 = ss0/2;
		sd2 = sd0 - sd1;
		ss2 = ss0 - ss1;
	} else {
		ddy = dy;
		ddx = (dy*_diagonalx)/_diagonaly;
		dsx = dx - ddx;
		ddy = ddy * dirY;
		ddx = ddx * dirX;
		dsx = dsx * dirX;
		dsy = 0;

		sd0 = (ddy + _modY[dirD]/2) / _modY[dirD];
		ss0 = (dsx + _modX[dirS]/2) / _modX[dirS];
		sd1 = sd0/2;
		ss1 = ss0/2;
		sd2 = sd0 - sd1;
		ss2 = ss0 - ss1;
	}

	if (best == 0) { //halfsquare, diagonal,	halfsquare
		_smoothPath[k].x = x+dsx/2;
		_smoothPath[k].y = y+dsy/2;
		_smoothPath[k].dir = dirS;
		_smoothPath[k].num = ss1;
		k = k + 1;
		_smoothPath[k].x = x+dsx/2+ddx;
		_smoothPath[k].y = y+dsy/2+ddy;
		_smoothPath[k].dir = dirD;
		_smoothPath[k].num = sd0;
		k = k + 1;
		_smoothPath[k].x = x+dsx+ddx;
		_smoothPath[k].y = y+dsy+ddy;
		_smoothPath[k].dir = dirS;
		_smoothPath[k].num = ss2;
		k = k + 1;
		tempK = k;
	} else if (best == 1) { //square, diagonal
		_smoothPath[k].x = x+dsx;
		_smoothPath[k].y = y+dsy;
		_smoothPath[k].dir = dirS;
		_smoothPath[k].num = ss0;
		k = k + 1;
		_smoothPath[k].x = x2;
		_smoothPath[k].y = y2;
		_smoothPath[k].dir = dirD;
		_smoothPath[k].num = sd0;
		k = k + 1;
		tempK = k;
	} else if (best == 2) { //diagonal square
		_smoothPath[k].x = x+ddx;
		_smoothPath[k].y = y+ddy;
		_smoothPath[k].dir = dirD;
		_smoothPath[k].num = sd0;
		k = k + 1;
		_smoothPath[k].x = x2;
		_smoothPath[k].y = y2;
		_smoothPath[k].dir = dirS;
		_smoothPath[k].num = ss0;
		k = k + 1;
		tempK = k;
	} else { //halfdiagonal, square, halfdiagonal
		_smoothPath[k].x = x+ddx/2;
		_smoothPath[k].y = y+ddy/2;
		_smoothPath[k].dir = dirD;
		_smoothPath[k].num = sd1;
		k = k + 1;
		_smoothPath[k].x = x+dsx+ddx/2;
		_smoothPath[k].y = y+dsy+ddy/2;
		_smoothPath[k].dir = dirS;
		_smoothPath[k].num = ss0;
		k = k + 1;
		_smoothPath[k].x = x2;
		_smoothPath[k].y = y2;
		_smoothPath[k].dir = dirD;
		_smoothPath[k].num = sd2;
		k = k + 1;
		tempK = k;
	}
	return tempK;
}

void SwordRouter::slidyWalkAnimator(WalkData *walkAnim, FrameInfos *frInfo, int32 megaId) {
/****************************************************************************
 *  Skidding every where HardWalk creates an animation that exactly fits the
 *	smoothPath and uses foot slipping to fit whole steps into the route
 *	Parameters: georgeg,mouseg
 *	Returns:		rout 
 *
 *	produce a module list from the line data
 *
 ****************************************************************************/
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

	lastDir = _modularPath[0].dir;
	currentDir = _modularPath[1].dir;
	if (currentDir == NO_DIRECTIONS)
		currentDir = lastDir;
	moduleX = frInfo->startX;
	moduleY = frInfo->startY;
	module16X = moduleX << 16;
	module16Y = moduleY << 16;
	stepCount = 0;
	//****************************************************************************
	// SLIDY
	// START THE WALK WITH THE FIRST STANDFRAME THIS MAY CAUSE A DELAY
	// BUT IT STOPS THE PLAYER MOVING FOR COLLISIONS ARE DETECTED
	//****************************************************************************
	module =	frInfo->framesPerChar + lastDir;
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
				module =	frInfo->turnFramesLeft + lastDir;
			}
			else
			{
				module =	frInfo->turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}

		// rotate till were facing new dir then go back 45 degrees
		while (lastDir != currentDir) {
			lastDir += turnDir;
			if ( turnDir < 0) { // new frames for turn frames
				if ( lastDir < 0)
					lastDir += NO_DIRECTIONS;
				module = frInfo->turnFramesLeft + lastDir;
			} else {
				if ( lastDir > 7)
					lastDir -= NO_DIRECTIONS;
				module = frInfo->turnFramesRight + lastDir;
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
	 left = frInfo->framesPerStep;
	else
	 left = 0;

	lastCount = stepCount;
	lastDir = 99;// this ensures that we don't put in turn frames for the start
	currentDir = 99;// this ensures that we don't put in turn frames for the start
	p = 0;
	do
	{
		while (_modularPath[p].num == 0)
		{
			p = p + 1;
			if (currentDir != 99)
				lastRealDir = currentDir;
			lastDir = currentDir;
			lastCount = stepCount;
		}
		//calculate	average amount to lose in each step on the way to the next node
		currentDir = _modularPath[p].dir;
		if (currentDir < NO_DIRECTIONS)
		{
			module = currentDir * frInfo->framesPerStep * 2 + left;
			if (left == 0)
				left = frInfo->framesPerStep;
			else
				left = 0;
			moduleEnd = module + frInfo->framesPerStep;
			step = 0;
			scale = (frInfo->scaleA * moduleY + frInfo->scaleB);
			do {
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
			} while( module < moduleEnd);
			stepX = _modX[_modularPath[p].dir];
			stepY = _modY[_modularPath[p].dir];
			errorX = _modularPath[p].x - moduleX;
			errorX = errorX * stepX;
			errorY = _modularPath[p].y - moduleY;
			errorY = errorY * stepY;
			if ((errorX < 0) || (errorY < 0)) {
				_modularPath[p].num = 0;	// the end of the path
				// okay those last steps took us past our target but do we want to scoot or moonwalk
				frames = stepCount - lastCount;
				errorX = _modularPath[p].x - walkAnim[stepCount-1].x;
				errorY = _modularPath[p].y - walkAnim[stepCount-1].y;

				if (frames > frInfo->framesPerStep) {
					lastErrorX = _modularPath[p].x - walkAnim[stepCount-7].x;
					lastErrorY = _modularPath[p].y - walkAnim[stepCount-7].y;
					if (stepX==0) {
						if (3*abs(lastErrorY) < abs(errorY)) { //the last stop was closest
							stepCount -= frInfo->framesPerStep;
							if (left == 0)
						 		left = frInfo->framesPerStep;
							else
							 	left = 0;
						}
					} else {
						if (3*abs(lastErrorX) < abs(errorX)) { //the last stop was closest
							stepCount -= frInfo->framesPerStep;
							if (left == 0)
						 		left = frInfo->framesPerStep;
							else
							 	left = 0;
						}
					}
				}
				errorX = _modularPath[p].x - walkAnim[stepCount-1].x;
				errorY = _modularPath[p].y - walkAnim[stepCount-1].y;
				// okay we've reached the end but we still have an error
				if (errorX != 0) {
					frameCount = 0;
					frames = stepCount - lastCount;
					do {
						frameCount += 1;
						walkAnim[lastCount + frameCount - 1].x += errorX*frameCount/frames;
					} while(frameCount<frames);	
				}
				if (errorY != 0) {
					frameCount = 0;
					frames = stepCount - lastCount;
					do {
						frameCount += 1;
						walkAnim[lastCount + frameCount-1].y +=	errorY*frameCount/frames;
					} while(frameCount<frames);	
				}
				// Now is the time to put in the turn frames for the last turn
				if (frames < frInfo->framesPerStep)
					currentDir = 99;// this ensures that we don't put in turn frames for this walk or the next		
				if (currentDir != 99)
					lastRealDir = currentDir;
				// check each turn condition in turn
				if (((lastDir != 99) && (currentDir != 99)) && (megaId == GEORGE)) { // only for george
					lastDir = currentDir - lastDir;//1 and -7 going right -1 and 7 going left
					if (((lastDir == -1) || (lastDir == 7)) || ((lastDir == -2) || (lastDir == 6))) {
						// turn at the end of the last walk
						frame = lastCount - frInfo->framesPerStep;
						do {
							walkAnim[frame].frame += 104;//turning left 
							frame += 1;
						} while(frame < lastCount );
					}
					if (((lastDir == 1) || (lastDir == -7)) || ((lastDir == 2) || (lastDir == -6))) {
						// turn at the end of the current walk
						frame = lastCount - frInfo->framesPerStep;
						do {
							walkAnim[frame].frame += 200; //was 60 now 116
							frame += 1;
						} while(frame < lastCount );
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
	} while (_modularPath[p].dir < NO_DIRECTIONS);

	if (lastRealDir == 99)
		error("SlidyWalkAnimator direction error");

	if (frInfo->targetDir == NO_DIRECTIONS)	{ // stand in the last direction
		module = frInfo->standFrames + lastRealDir;
		frInfo->targetDir = lastRealDir;
		walkAnim[stepCount].frame = module;
		walkAnim[stepCount].step = 0;
		walkAnim[stepCount].dir = lastRealDir;
		walkAnim[stepCount].x = moduleX;
		walkAnim[stepCount].y = moduleY;
		stepCount += 1;
	}
	if (frInfo->targetDir == 9) {
		if (stepCount == 0) {
			module = frInfo->framesPerChar + lastRealDir;
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
	} else if (frInfo->targetDir != lastRealDir) { // rotate to targetDir
		// rotate to target direction
		turnDir = frInfo->targetDir - lastRealDir;
		if ( turnDir < 0)
				turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to target direction
		// for george and nico put in a head turn at the start
		if ((megaId == GEORGE) || (megaId == NICO)) {
			if ( turnDir < 0)	// new frames for turn frames	29oct95jps
				module = frInfo->turnFramesLeft + lastDir;
			else
				module = frInfo->turnFramesRight + lastDir;
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}

		// rotate if we need to
		while (lastRealDir != frInfo->targetDir) {
			lastRealDir += turnDir;
			if ( turnDir < 0) { // new frames for turn frames	29oct95jps
				if ( lastRealDir < 0)
					lastRealDir += NO_DIRECTIONS;
				module = frInfo->turnFramesLeft + lastRealDir;
			} else {
				if ( lastRealDir > 7)
					lastRealDir -= NO_DIRECTIONS;
				module = frInfo->turnFramesRight + lastRealDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
		module = frInfo->standFrames + lastRealDir;
		walkAnim[stepCount-1].frame = module;
	} else { // just stand at the end
		module = frInfo->standFrames + lastRealDir;
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
}

/****************************************************************************
 *    ExtractRoute	gets route from the node data after a full scan, route is
 *		written with just the basic way points and direction options for heading
 *		to the next point. 
 ****************************************************************************/
int32 SwordRouter::extractRoute(int32 targetDir) {
	int32	prev;
	int32	prevx;
	int32	prevy;
	int32	last;
	int32	point;
	int32	dirx;
	int32	diry;
	int32	dir;

	// extract the route from the node data
	prev = _nNodes;
	last = prev;
	point = O_ROUTE_SIZE - 1;
	_route[point].x = _node[last].x;
	_route[point].y = _node[last].y;
	while (prev > 0) {
		point = point -	1;
		prev = _node[last].prev;
		prevx = _node[prev].x;
		prevy = _node[prev].y;
		_route[point].x = prevx;
		_route[point].y = prevy;
		last = prev;
	}

	int32 routeLength = 0;
	do {
        _route[routeLength].x = _route[point].x;
		_route[routeLength].y = _route[point].y;
		point = point + 1;
		routeLength = routeLength + 1;
	} while (point < O_ROUTE_SIZE);

	for (int p = 0; p < routeLength; p++) {
		int32 dx = _route[p+1].x - _route[p].x;
		int32 dy = _route[p+1].y - _route[p].y;
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

		if ((_diagonaly * dx) > (_diagonalx * dy))	// dir  = 1,2 or 2,3 or 5,6 or 6,7
		{
			dir = 4 - 2 * dirx;	 // 2 or 6
			_route[p].dirS = dir;
			dir = dir + diry * dirx; // 1,3,5 or 7
			_route[p].dirD = dir;
		}
		else // dir  = 7,0 or 0,1 or 3,4 or 4,5
		{
			dir = 2 + 2 * diry;	// 0 or 4
			_route[p].dirS = dir;
			dir = 4 - 2 * dirx;	 // 2 or 6
			dir = dir + diry * dirx; // 1,3,5 or 7
			_route[p].dirD = dir;
		}
	}
	if (targetDir == 8)	// ANY direction
	{ 
		_route[routeLength].dirS = _route[routeLength-1].dirS;
		_route[routeLength].dirD = _route[routeLength-1].dirD;
	}
	else
	{ 
		_route[routeLength].dirS = targetDir;
		_route[routeLength].dirD = targetDir;
	}
	return routeLength;
}

int SwordRouter::getRoute() {
	int32 routeRes;
	if ((_node[0].x == _node[_nNodes].x) && (_node[0].y == _node[_nNodes].y))
		routeRes = 2; // start is the same as destination
	else
		routeRes = checkTarget(_node[_nNodes].x, _node[_nNodes].y);

	if (routeRes == 0) { // still looking for a route check if target is within a pixel of a line
		int32 level = 1;
		int changed;
		do
		{
			changed = scan(level);
			level++;
		} while(changed == 1);
		if (_node[_nNodes].dist < 9999) // did we reach the target?
			routeRes = 1;
	}
	return routeRes;
}

int SwordRouter::scan(int32 level) {
	int32 distance;
	int changed = 0;
	// For all the nodes that have new values and a distance less than enddist
	// ie dont check for new routes from a point we checked before or from a point
	// that is already further away than the best route so far. 
	for (int32 i = 0; i < _nNodes; i++) {
		if ((_node[i].dist < _node[_nNodes].dist) && (_node[i].level == level)) {
			int16 x1 = _node[i].x;
			int16 y1 = _node[i].y;

			for (int32 k = _nNodes; k > 0; k--) {
				if (_node[k].dist > _node[i].dist) {
					int16 x2 = _node[k].x;
					int16 y2 = _node[k].y;
					if (abs(x2 - x1) > (4.5 * abs(y2 - y1)))
						distance = (8 * abs(x2 - x1) + 18 * abs(y2 - y1)) / (54 * 8) + 1;
					else
						distance = (6 * abs(x2 - x1) + 36 * abs(y2 - y1)) / (36 * 14) + 1;

					if ((distance + _node[i].dist < _node[_nNodes].dist) && (distance + _node[i].dist < _node[k].dist)) {
						//if (int temp = newCheck(0, x1, y1, x2, y2)) {
						if (newCheck(0, x1, y1, x2, y2)) {
							_node[k].level = level + 1;
							_node[k].dist  = distance + _node[i].dist;
							_node[k].prev  = i;
							changed = 1;
						}
					}
				}
			}
		}
	}
	return changed;
}

/*******************************************************************************
 * NewCheck routine checks if the route between two points can be achieved
 * without crossing any of the bars in the Bars array. 
 *
 * NewCheck differs from check in that that 4 route options are considered
 * corresponding to actual walked routes.
 *
 * Note distance doesnt take account of shrinking ??? 
 *
 * Note Bars array must be properly calculated ie min max dx dy co
 *******************************************************************************/
int SwordRouter::newCheck(int32 status, int16 x1, int16 x2, int16 y1, int16 y2) {

	int32 ldx;
	int32 ldy;
	int32 dlx;
	int32 dly;
	int32 dirX;
	int32 dirY;
	int32 step1;
	int32 step2;
	int32 step3;
	int32 steps;
	int32 options;

	steps = 0;
	options = 0;
	ldx = x2 - x1;
	ldy = y2 - y1;
	dirX = 1;
	dirY = 1;

	if (ldx < 0) {
		ldx = -ldx;
		dirX = -1;
	}

	if (ldy < 0) {
		ldy = -ldy;
		dirY = -1;
	}

	// make the route options

	if (_diagonaly * ldx > _diagonalx * ldy) {
		// dir  = 1,2 or 2,3 or 5,6 or 6,7

		dly = ldy;
		dlx = (ldy * _diagonalx) / _diagonaly;
		ldx = ldx - dlx;
		dlx = dlx * dirX;
		dly = dly * dirY;
		ldx = ldx * dirX;
		ldy = 0;

	 	//options are
		//square, diagonal a code 1 route

		step1 = check(x1, y1, x1 + ldx, y1);
		if (step1 != 0) {
			step2 = check(x1 + ldx, y1, x2, y2);
			if (step2 != 0) {
				steps = step1 + step2;
				options = options + 2;
			}
		}

		//diagonal, square a code 2 route

		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x1 + dlx, y1 + dly);
			if (step1 != 0) {
				step2 = check(x1 + dlx, y2, x2, y2);
				if (step2 != 0) {
					steps = step1 + step2;
					options = options + 4;
				}
			}
		}

		//halfsquare, diagonal, halfsquare a code 0 route

		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x1 + ldx / 2, y1);
			if (step1 != 0) {
				step2 = check(x1 + ldx / 2, y1, x1 + ldx / 2 + dlx, y2);
				if (step2 != 0) {
					step3 = check(x1 + ldx / 2 + dlx, y2, x2, y2);
					if (step3 != 0)	{
						steps = step1 + step2 + step3;
						options++;
					}
				}
			}
		}

		//halfdiagonal, square, halfdiagonal a code 3 route

		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x1 + dlx / 2, y1 + dly / 2);
			if (step1 != 0) {
				step2 = check(x1 + dlx / 2, y1 + dly / 2, x1 + ldx + dlx / 2, y1 + dly / 2);
				if (step2 != 0) {
					step3 = check(x1 + ldx + dlx / 2, y1 + dly / 2, x2, y2);
					if (step3 != 0) {
						steps = step1 + step2 + step3;
						options = options + 8;
					}
				}
			}
		}
	} else {
		// dir  = 7,0 or 0,1 or 3,4 or 4,5

		dlx = ldx;
		dly = (ldx * _diagonaly) / _diagonalx;
		ldy = ldy - dly;
		dlx = dlx * dirX;
		dly = dly * dirY;
		ldy = ldy * dirY;
		ldx = 0;

	 	//options are
		//square, diagonal a code 1 route

		step1 = check(x1 ,y1, x1, y1 + ldy);
		if (step1 != 0)	{
			step2 = check(x1, y1 + ldy, x2, y2);
			if (step2 != 0) {
				steps = step1 + step2;
				options = options + 2;
			}
		}

		//diagonal, square a code 2 route

		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x2, y1 + dly);
			if (step1 != 0) {
				step2 = check(x2, y1 + dly, x2, y2);
				if (step2 != 0) {
					steps = step1 + step2;
					options = options + 4;
				}
			}
		}

		//halfsquare, diagonal, halfsquare a code 0 route

		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x1, y1 + ldy / 2);
			if (step1 != 0) {
				step2 = check(x1, y1 + ldy / 2, x2, y1 + ldy / 2 + dly);
				if (step2 != 0) {
					step3 = check(x2, y1 + ldy / 2 + dly, x2, y2);
					if (step3 != 0) {
						steps = step1 + step2 + step3;
						options++;
					}
				}
			}
		}

		//halfdiagonal, square, halfdiagonal a code 3 route

		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x1 + dlx / 2, y1 + dly / 2);
			if (step1 != 0) {
				step2 = check(x1 + dlx / 2, y1 + dly / 2, x1 + dlx / 2, y1 + ldy + dly / 2);
				if (step2 != 0) {
					step3 = check(x1 + dlx / 2, y1 + ldy + dly / 2, x2, y2);
					if (step3 != 0)	{
						steps = step1 + step2 + step3;
						options = options + 8;
					}
				}
			}
		}
	}

	if (status == 0)
		status = steps;
	else
		status = options;

	return status;
}

int SwordRouter::check(int16 x1, int16 y1, int16 x2, int16 y2) {
	//call the fastest line check for the given line 
	//returns 1 if line didn't cross any bars
	int   steps;

	if ((x1 == x2) && (y1 == y2))
		steps = 1;
	else if (x1 == x2)
		steps = vertCheck(x1, y1, y2);
	else if (y1 == y2)
		steps = horizCheck(x1, y1, x2);
	else
		steps = lineCheck(x1, y1, x2, y2);
	return steps;
}

int SwordRouter::horizCheck(int16 x1, int16 y, int16 x2) {
	int32 ldy;
	int32 i;
	int32 xc;
	int32 xmin;
	int32 xmax;
	int32 linesCrossed = 1;

	if (x1 > x2) {
		xmin = x2;
		xmax = x1;
	} else {
		xmin = x1;
		xmax = x2;
	}

	// line set to go one step in chosen direction
	// so ignore if it hits anything

	i = 0;

	do {
		// skip if not on module
		if (xmax >= _bars[i].xmin && xmin <= _bars[i].xmax) {
			// skip if not on module
			if (y >= _bars[i].ymin && y <= _bars[i].ymax) {
				// okay its a valid line calculate an intercept
				// wow but all this arithmetic we must have
				// loads of time

				if (_bars[i].dy == 0)
					linesCrossed = 0;
				else {
					ldy = y - _bars[i].y1;
					xc = _bars[i].x1 + (_bars[i].dx * ldy) / _bars[i].dy;
					// skip if not on module 
					if (xc >= xmin - 1 && xc <= xmax + 1)
						linesCrossed = 0;
				}
			}
		}
		i++;
	} while (i < _nBars && linesCrossed);

	return linesCrossed;
}

int SwordRouter::vertCheck(int16 x, int16 y1, int16 y2) {
	int32 ldx;
	int32 i;
	int32 yc;
	int32 ymin;
	int32 ymax;
	int32 linesCrossed = 1;

	if (y1 > y2) {
		ymin = y2;
		ymax = y1;
	} else {
		ymin = y1;
		ymax = y2;
	}

	// line set to go one step in chosen direction
	// so ignore if it hits anything

	i = 0;

	do {
		if (x >= _bars[i].xmin && x <= _bars[i].xmax) {
			// overlapping
			// skip if not on module 
			if (ymax >= _bars[i].ymin && ymin <= _bars[i].ymax) {
				// okay its a valid line calculate an intercept
				// wow but all this arithmetic we must have
				// loads of time

				// both lines vertical and overlap in x and y
				// so they cross

				if (_bars[i].dx == 0)
					linesCrossed = 0;
				else {
			 		ldx = x - _bars[i].x1;
					yc = _bars[i].y1 + (_bars[i].dy * ldx) / _bars[i].dx;
					// the intercept overlaps 
					if (yc >= ymin - 1 && yc <= ymax + 1)
						linesCrossed = 0;
				}
			}
		}
		i++;
	} while (i < _nBars && linesCrossed);

	return linesCrossed;
}

int SwordRouter::lineCheck(int16 x1, int16 y1, int16 x2, int16 y2) {
	int32 dirx;
	int32 diry;
	int32 co;
	int32 slope;
	int32 i;
	int32 xc;
	int32 yc;
	int32 xmin;
	int32 ymin;
	int32 xmax;
	int32 ymax;
	int32 linesCrossed = 1;

	if (x1 > x2) {
		xmin = x2;
		xmax = x1;
	} else {
		xmin = x1;
		xmax = x2;
	}

	if (y1 > y2) {
		ymin = y2;
		ymax = y1;
	} else {
		ymin = y1;
		ymax = y2;
	}

	// line set to go one step in chosen direction
	// so ignore if it hits anything

	dirx = x2 - x1;
	diry = y2 - y1;

	co = (y1 * dirx)- (x1 * diry);		// new line equation

	i = 0;

	do {
		// skip if not on module 
		if (xmax >= _bars[i].xmin && xmin <= _bars[i].xmax) {
			// skip if not on module 
			if (ymax >= _bars[i].ymin && ymin <= _bars[i].ymax) {
				// okay its a valid line calculate an intercept
				// wow but all this arithmetic we must have
				// loads of time

				// slope it he slope between the two lines
				slope = (_bars[i].dx * diry) - (_bars[i].dy *dirx);
				// assuming parallel lines don't cross
				if (slope != 0) {
					// calculate x intercept and check its
					// on both lines
					xc = ((_bars[i].co * dirx) - (co * _bars[i].dx)) / slope;

					// skip if not on module
					if (xc >= xmin - 1 && xc <= xmax + 1) {
						// skip if not on line 
						if (xc >= _bars[i].xmin - 1 && xc <= _bars[i].xmax + 1) {
							yc = ((_bars[i].co * diry) - (co * _bars[i].dy)) / slope;

							// skip if not on module
							if (yc >= ymin - 1 && yc <= ymax + 1) {
								// skip if not on line 
								if (yc >= _bars[i].ymin - 1 && yc <= _bars[i].ymax + 1) {
									linesCrossed = 0;
								}
							}
						}
					}
				}
			}
		}
		i++;
	} while (i < _nBars && linesCrossed);

	return linesCrossed;
}

int SwordRouter::checkTarget(int16 x, int16 y) {
	int32   dx, dy, xc, yc, xmin, xmax, ymin, ymax;
	int32   onLine = 0;

 	xmin = x - 1;    
 	xmax = x + 1;
 	ymin = y - 1;    
 	ymax = y + 1;

	for (int i = 0; (i < _nBars) && (onLine == 0); i++) {
		if ((xmax >= _bars[i].xmin) && (xmin <= _bars[i].xmax)) { //overlapping line 
			if ((ymax >= _bars[i].ymin) && ( ymin <= _bars[i].ymax)) { //overlapping line
				// okay this line overlaps the target, calculate an y intersept for x 
				
				if (_bars[i].dx == 0) // vertical line so we know it overlaps y
					yc = 0; 	
				else {
					dx = x - _bars[i].x1;
					yc = _bars[i].y1 + (_bars[i].dy * dx) / _bars[i].dx;
				}
				if ((yc >= ymin) && (yc <= ymax))  //overlapping point for y 
				    onLine = 3; // target on a line so drop out
				else {
					if (_bars[i].dy == 0) // vertical line so we know it overlaps y
						xc = 0;
					else {
						dy = y- _bars[i].y1;
						xc = _bars[i].x1 + (_bars[i].dx * dy) / _bars[i].dy;
					}
					if ((xc >= xmin) && (xc <= xmax)) //skip if not on module 
					    onLine = 3;// target on a line so drop out
				}
			}
		}
	}
	return onLine;
}

void SwordRouter::resetExtraData(void) {
	_numExtraBars = _numExtraNodes = 0;
}

void SwordRouter::setPlayerTarget(int32 x, int32 y, int32 dir, int32 stance) {
	_playerTargetX = x;
	_playerTargetY = y;
	_playerTargetDir = dir;
	_playerTargetStance = stance;
}

void SwordRouter::loadWalkResources(int32 megaId, BsObject *mega, int32 x, int32 y, int32 dir) {
	WalkGridHeader *floorHeader;

	int32 walkGridId = _objMan->fetchObject(mega->o_place)->o_resource;

	uint8 *fPolyGrid = (uint8*)_resMan->openFetchRes(walkGridId);
	floorHeader = (WalkGridHeader*)(fPolyGrid + sizeof(Header));
	fPolyGrid += sizeof(WalkGridHeader) + sizeof(Header);

	_nBars = FROM_LE_32(floorHeader->numBars);
	_nNodes = FROM_LE_32(floorHeader->numNodes) + 1;
	if ((_nBars >= O_GRID_SIZE) || (_nNodes >= O_GRID_SIZE))
		error("loadWalkResources: resource has %d bars and %d nodes", _nBars, _nNodes);

	for (int32 cnt = 0; cnt < _nBars; cnt++) {
		_bars[cnt].x1   = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_bars[cnt].y1   = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_bars[cnt].x2   = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_bars[cnt].y2   = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_bars[cnt].xmin = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_bars[cnt].ymin = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_bars[cnt].xmax = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_bars[cnt].ymax = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_bars[cnt].dx   = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_bars[cnt].dy   = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_bars[cnt].co   = READ_LE_UINT32(fPolyGrid); fPolyGrid += 4;
	}
	//_nBars = 0;

	// leave node 0 for start node
	for (int32 cnt = 1; cnt < _nNodes; cnt++) {
		_node[cnt].x = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
		_node[cnt].y = READ_LE_UINT16(fPolyGrid); fPolyGrid += 2;
	}
	_resMan->resClose(walkGridId);

	// floor grid loaded.  Copy george's extra bars and nodes.
	if (megaId == GEORGE) {
		memcpy(_bars + _nBars, _extraBars, _numExtraBars * sizeof(BarData));
		_nBars += _numExtraBars;
		for (int32 cnt = 0; cnt < _numExtraNodes; cnt++) {
			_node[_nNodes + cnt].x = _extraNodes[cnt].x;
			_node[_nNodes + cnt].y = _extraNodes[cnt].y;
		}
		_nNodes += _numExtraNodes;
	}

	uint8 *walkData = (uint8*)_resMan->openFetchRes(mega->o_mega_resource);
	_nWalkFrames = walkData[0];
	_nTurnFrames = walkData[1];
	walkData += 2;

	for (int32 cnt = 0; cnt < NO_DIRECTIONS * (_nWalkFrames + 1 + _nTurnFrames); cnt++) {
		_dx[cnt] = READ_LE_UINT32(walkData);
		walkData += 4;
	}
	for (int32 cnt = 0; cnt < NO_DIRECTIONS * (_nWalkFrames + 1 + _nTurnFrames); cnt++) {
		_dy[cnt] = READ_LE_UINT32(walkData);
		walkData += 4;
	}
	for (int32 cnt = 0; cnt < NO_DIRECTIONS; cnt++) {
		_modX[cnt] = READ_LE_UINT32(walkData);
		walkData += 4;
	}
	for (int32 cnt = 0; cnt < NO_DIRECTIONS; cnt++) {
		_modY[cnt] = READ_LE_UINT32(walkData);
		walkData += 4;
	}
	_resMan->resClose(mega->o_mega_resource);
	_diagonalx = _modX[3];
	_diagonaly = _modY[3];
	if ((_diagonalx != 36) || (_diagonaly != 8))
		warning("DiagX = %d, DiagY = %d", _diagonalx, _diagonaly);
	// mega data ready
	// finish setting grid by putting mega node at begining
	// and target node at end	and reset current values

	_node[0].x = mega->o_xcoord; // the start
	_node[0].y = mega->o_ycoord; //
	_node[0].dist = 0;
	_node[0].prev = 0;
	_node[0].level = 1;

	for (int32 cnt = 1; cnt <= _nNodes; cnt++) {
		_node[cnt].dist = 9999;
		_node[cnt].prev = 0;
		_node[cnt].level = 0;
	}
	_node[_nNodes].x = x; // the destination
	_node[_nNodes].y = y; //
}

int SwordRouter::whatTarget(int32 startX, int32 startY, int32 destX, int32 destY) {
	int tar_dir;
//setting up
	int deltaX = destX-startX;
	int deltaY = destY-startY;
	int signX = (deltaX > 0);
	int signY = (deltaY > 0);
	int	slope;

	if ( (abs(deltaY) * _diagonalx ) < (abs(deltaX) * _diagonaly / 2))
		slope = 0;// its flat
	else if ( (abs(deltaY) * _diagonalx / 2) > (abs(deltaX) * _diagonaly ) )
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
