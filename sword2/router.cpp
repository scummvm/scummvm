/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

// ---------------------------------------------------------------------------
// ROUTER.CPP by James
//
// A rehash of Jeremy's original jrouter.c, containing low-level system
// routines for calculating routes between points inside a walk-grid, and
// constructing walk animations from mega-sets.
//
// jrouter.c underwent 2 major reworks from the original:
// (1)	Restructured to allow more flexibility in the mega-sets, ie. more info
//      taken from the walk-data
//	- the new George & Nico mega-sets & walk-data were then tested &
//        tweaked in the Sword1 system
// (2)	Updated for the new Sword2 system, ie. new object structures
//	- now compatible with Sword2, the essential code already having been
//        tested
//
// ---------------------------------------------------------------------------

/****************************************************************************
 *    JROUTER.C				polygon router with modular walks
 *       					using a tree of modules
 *       					21 july 94  
 *  3  november 94  
 *  System currently works by scanning grid data and coming up with a	ROUTE
 *  as a series of way points(nodes), the smoothest eight directional PATH
 * 	through these nodes is then found, and a WALK created to fit the PATH.
 *
 *	Two funtions are called by the user, RouteFinder creates a route as a
 *	module list, HardWalk creates an animation list from the module list.
 *	The split is only provided to allow the possibility of turning the
 *	autorouter over two game cycles.     
 ****************************************************************************
 *    
 *	Routine timings on osborne 486
 *
 *	Read floor resource (file already loaded)	 112 pixels
 *
 *	Read mega resource (file already loaded)	 112 pixels
 *
 *
 *
 ****************************************************************************
 *    
 *  Modified 12 Oct 95
 *
 *	Target Points within 1 pixel of a line are ignored ???
 *
 *	Modules split into  Points within 1 pixel of a line are ignored ???
 *
 ****************************************************************************
 *
 *  TOTALLY REHASHED BY JAMES FOR NEW MEGAS USING OLD SYSTEM
 *  THEN REINCARNATED BY JAMES FOR NEW MEGAS USING NEW SYSTEM
 *
 ****************************************************************************/

/*
 * Include Files
 */

#include "stdafx.h"
#include "driver/driver96.h"
#include "console.h"
#include "debug.h"
#include "defs.h"
#include "header.h"
#include "interpreter.h"
#include "memory.h"
#include "object.h"
#include "resman.h"
#include "router.h"

#define MAX_FRAMES_PER_CYCLE	16
#define NO_DIRECTIONS		8
#define MAX_FRAMES_PER_CHAR	(MAX_FRAMES_PER_CYCLE * NO_DIRECTIONS)
#define ROUTE_END_FLAG		255

//---------------------------------------
// TEMP!
// 1 = force the use of slidy router (so solid path not used when ending walk
// in ANY direction)
int8 forceSlidy;
//---------------------------------------

/*
 * Type Defines
 */

#define	O_WALKANIM_SIZE		600	// max number of nodes in router output
#define	O_GRID_SIZE		200	// max 200 lines & 200 points
#define	EXTRA_GRID_SIZE		20	// max 20 lines & 20 points
#define	O_ROUTE_SIZE		50	// max number of modules in a route

typedef	struct {
	int32 x;
	int32 y;
	int32 dirS;
	int32 dirD;
} _routeData;

typedef	struct {
	int32 x;
	int32 y;
	int32 dir;
	int32 num;
} _pathData;

// Function prototypes

static int32 GetRoute(void);
static void ExtractRoute(void);
static void LoadWalkGrid(void);
static void SetUpWalkGrid(Object_mega *ob_mega, int32 x, int32 y, int32 dir);
static void LoadWalkData(Object_walkdata *ob_walkdata);
static void PlotCross(int16 x, int16 y, uint8 colour);

static int32 Scan(int32 level);
static int32 NewCheck(int32 status, int32 x1, int32 y1, int32 x2, int32 y2);
static int32 LineCheck(int32 x1, int32 x2, int32 y1, int32 y2);
static int32 VertCheck(int32 x, int32 y1, int32 y2);
static int32 HorizCheck(int32 x1, int32 y, int32 x2);
static int32 Check(int32 x1, int32 y1, int32 x2, int32 y2);
static int32 CheckTarget(int32 x, int32 y);

static int32 SmoothestPath(void);
static int32 SlidyPath(void);
static int32 SolidPath(void);

static int32 SmoothCheck(int32 best, int32 p, int32 dirS, int32 dirD);

static int32 AddSlowInFrames(_walkData *walkAnim);
static void AddSlowOutFrames(_walkData *walkAnim);
static void SlidyWalkAnimator(_walkData *walkAnim);
static int32 SolidWalkAnimator(_walkData *walkAnim);

#define MAX_WALKGRIDS		10

static int32 walkGridList[MAX_WALKGRIDS];

// because we only have 2 megas in the game!
#define TOTAL_ROUTE_SLOTS	2

// stores pointers to mem blocks containing routes created & used by megas
// (NULL if slot not in use)
static mem *route_slots[TOTAL_ROUTE_SLOTS];

// Local Variables

static int32 nbars;
static int32 nnodes;

// because extra bars will be copied into here afer walkgrid loaded
static _barData bars[O_GRID_SIZE+EXTRA_GRID_SIZE];

static _nodeData node[O_GRID_SIZE+EXTRA_GRID_SIZE];

// area for extra route data to block parts of floors and enable routing
// round mega charaters

static int32 nExtraBars = 0;
static int32 nExtraNodes = 0;
static _barData extraBars[EXTRA_GRID_SIZE];
static _nodeData extraNode[EXTRA_GRID_SIZE];

static int32 startX;
static int32 startY;
static int32 startDir;
static int32 targetX;
static int32 targetY;
static int32 targetDir;
static int32 scaleA;
static int32 scaleB;
static _routeData route[O_ROUTE_SIZE];
static _pathData smoothPath[O_ROUTE_SIZE];
static _pathData modularPath[O_ROUTE_SIZE];
static int32 routeLength;

static int32 framesPerStep;
static int32 framesPerChar;

static uint8 nWalkFrames;			// no. of frames per walk cycle
static uint8 usingStandingTurnFrames;		// any standing turn frames?
static uint8 usingWalkingTurnFrames;		// any walking turn frames?
static uint8 usingSlowInFrames;			// any slow-in frames?
static uint8 usingSlowOutFrames;		// any slow-out frames?
static int32 dx[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
static int32 dy[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
static int8 modX[NO_DIRECTIONS];
static int8 modY[NO_DIRECTIONS];
static int32 diagonalx = 0;
static int32 diagonaly = 0;

static int32 firstStandFrame;

static int32 firstStandingTurnLeftFrame;
static int32 firstStandingTurnRightFrame;

static int32 firstWalkingTurnLeftFrame;		// left walking turn
static int32 firstWalkingTurnRightFrame;	// right walking turn

static uint32 firstSlowInFrame[NO_DIRECTIONS];
static uint32 numberOfSlowInFrames[NO_DIRECTIONS];

static uint32 leadingLeg[NO_DIRECTIONS];

static int32 firstSlowOutFrame;

// number of slow-out frames on for each leading-leg in each direction
// ie. total number of slow-out frames = (numberOfSlowOutFrames * 2 *
// NO_DIRECTIONS)

static int32 numberOfSlowOutFrames;

static int32 stepCount;

static int32 moduleX;
static int32 moduleY;
static int32 currentDir;
static int32 lastCount;
static int32 frame;

/*
 *    CODE
 */

// **************************************************************************

uint8 ReturnSlotNo(uint32 megaId) {
	if (ID == CUR_PLAYER_ID) {
		// George (8)
		return 0;
	} else {
		// One of Nico's mega id's
		return 1;
	}
}

void AllocateRouteMem(void) {
	uint8 slotNo;

	// added (James23June96)
	// Player character always always slot 0, while the other mega
	// (normally Nico) always uses slot 1
	// Better this way, so that if mega object removed from memory while
	// in middle of route, the old route will be safely cleared from
	// memory just before they create a new one

	slotNo = ReturnSlotNo(ID); 

	// if this slot is already used, then it can't be needed any more
	// because this id is creating a new route!

	if (route_slots[slotNo])
		FreeRouteMem();

	route_slots[slotNo] = Twalloc(sizeof(_walkData) * O_WALKANIM_SIZE, MEM_locked, UID_walk_anim);

	// 12000 bytes were used for this in Sword1 mega compacts, based on
	// 20 bytes per '_walkData' frame
	// ie. allowing for 600 frames including end-marker
	// Now '_walkData' is 8 bytes, so 8*600 = 4800 bytes.
	// Note that a 600 frame walk lasts about 48 seconds!
	// (600fps / 12.5s = 48s)

	// mega keeps note of which slot contains the pointer to it's walk
	// animation mem block
	// +1 so that '0' can mean "not walking"
	// megaObject->route_slot_id = slotNo + 1;
}

_walkData* LockRouteMem(void) {
	uint8 slotNo = ReturnSlotNo(ID); 
	
	Lock_mem(route_slots[slotNo]);
	return (_walkData *) route_slots[slotNo]->ad;
}

void FloatRouteMem(void) {
	uint8 slotNo = ReturnSlotNo(ID); 

	Float_mem(route_slots[slotNo]);
}

void FreeRouteMem(void) {
	uint8 slotNo = ReturnSlotNo(ID); 

	// free the mem block pointed to from this entry of route_slots[]

	Free_mem(route_slots[slotNo]);
	route_slots[slotNo] = NULL;
}

void FreeAllRouteMem(void) {
	for (int slotNo = 0; slotNo < TOTAL_ROUTE_SLOTS; slotNo++) {
		if (route_slots[slotNo]) {
			// free the mem block pointed to from this entry of
			// route_slots[]
			Free_mem(route_slots[slotNo]);
			route_slots[slotNo] = NULL;
		}
	}
}

int32 RouteFinder(Object_mega *ob_mega, Object_walkdata *ob_walkdata, int32 x, int32 y, int32 dir) {
	/*********************************************************************
	 * RouteFinder.C		polygon router with modular walks
	 * 						21 august 94  
	 *						3  november 94  
	 * RouteFinder creates a list of modules that enables HardWalk to
	 * create an animation list.
	 *
	 * RouteFinder currently works by scanning grid data and coming up
	 * with a ROUTE as a series of way points(nodes), the smoothest eight
	 * directional PATH through these nodes is then found, this
	 * information is made available to HardWalk for a WALK to be created
	 * to fit the PATH.
	 *    
	 * 30 november 94 return values modified
	 *    
	 * return	0 = failed to find a route
	 *    
	 *    		1 = found a route
	 *
	 *    		2 = mega already at target
	 *    
	 *********************************************************************/

	int32 routeFlag = 0;
	int32 solidFlag = 0;
	_walkData *walkAnim;

	// megaId = id;

	SetUpWalkGrid(ob_mega, x, y, dir);
	LoadWalkData(ob_walkdata);

	// lock the _walkData array (NB. AFTER loading walkgrid & walkdata!)
	walkAnim = LockRouteMem();

	// All route data now loaded start finding a route

	// Check if we can get a route through the floor. changed 12 Oct95 JPS 

	routeFlag = GetRoute();

	if (routeFlag == 2) {
		// special case for zero length route

		// if target direction specified as any
		if (targetDir > 7)
			targetDir = startDir;

		// just a turn on the spot is required set an end module for
		// the route let the animator deal with it
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

 		SlidyWalkAnimator(walkAnim);
 		routeFlag = 2;
	} else if (routeFlag == 1) {
		// a normal route

		// Convert the route to an exact path
		SmoothestPath();

		// The Route had waypoints and direction options

		// The Path is an exact set of lines in 8 directions that
		// reach the target.

		// The path is in module format, but steps taken in each
		// direction are not accurate

		// if target dir = 8 then the walk isn't linked to an anim so 
		// we can create a route without sliding and miss the exact
		// target

		if (!forceSlidy) {
			if (targetDir == 8) {
				// can end facing ANY direction (ie. exact end
				// position not vital) - so use SOLID walk to
				// avoid sliding to exact position

				SolidPath();
				solidFlag = SolidWalkAnimator(walkAnim);
			}
		}

		if (!solidFlag) {
			// if we failed to create a SOLID route, do a SLIDY
			// one instead

			SlidyPath();
			SlidyWalkAnimator(walkAnim);
		}
	} else {
		// Route didn't reach target so assume point was off the floor
		// routeFlag = 0;
	}

   	FloatRouteMem();	// float the _walkData array again

	return routeFlag;	// send back null route
}

int32 GetRoute(void) {
	/*********************************************************************
	 * GetRoute.C				extract a path from walk grid
	 *							12 october 94  
	 *
	 * GetRoute currently works by scanning grid data and coming up with
	 * a ROUTE as a series of way points(nodes).
	 *
	 * static _routeData route[O_ROUTE_SIZE];
	 *    
	 * return 	0 = failed to find a route
	 *    
	 *		1 = found a route
	 *
	 *		2 = mega already at target
	 *
	 *		3 = failed to find a route because target was on a line
	 *
	 *********************************************************************/

	int32 routeGot = 0;
	int32 level;
	int32 changed;

	if (startX == targetX && startY == targetY)
		routeGot = 2;
	else {
		// 'else' added by JEL (23jan96) otherwise 'routeGot' affected
		// even when already set to '2' above - causing some 'turns'
		// to walk downwards on the spot

		// returns 3 if target on a line ( +- 1 pixel )
		routeGot = CheckTarget(targetX,targetY);
	}

	if (routeGot == 0) {
		// still looking for a route check if target is within a pixel
		// of a line 

		// scan through the nodes linking each node to its nearest
		// neighbour until no more nodes change

		// This is the routine that finds a route using Scan()

		level = 1;

		do {
			changed = Scan(level);
			level++;
		} while(changed == 1);

		// Check to see if the route reached the target

		if (node[nnodes].dist < 9999) {
			// it did so extract the route as nodes and the
			// directions to go between each node

			routeGot = 1;
			ExtractRoute();

			// route.X,route.Y and route.Dir now hold all the
			// route infomation with the target dir or route
			// continuation
		}
	}

	return routeGot;
}

// THE SLIDY PATH ROUTINES

int32 SmoothestPath() {
	// This is the second big part of the route finder and the the only
	// bit that tries to be clever (the other bits are clever).
	//
	// This part of the autorouter creates a list of modules from a set of
	// lines running across the screen. The task is complicated by two
	// things:
	//
	// Firstly in choosing a route through the maze of nodes the routine
	// tries to minimise the amount of each individual turn avoiding 90
	// degree and greater turns (where possible) and reduces the total
	// number of turns (subject to two 45 degree turns being better than
	// one 90 degree turn).
	//
	// Secondly when walking in a given direction the number of steps
	// required to reach the end of that run is not calculated accurately.
	// This is because I was unable to derive a function to relate number
	// of steps taken between two points to the shrunken step size   

	int32 p;     
	int32 dirS;
	int32 dirD;
	int32 dS;
	int32 dD;
	int32 dSS;
	int32 dSD;
	int32 dDS;
	int32 dDD;
	int32 SS;
	int32 SD;
	int32 DS;
	int32 DD;
	int32 i;
	int32 j;
	int32 temp;
	int32 steps;
	int32 option;
	int32 options;
	int32 lastDir;
	int32 nextDirS;
	int32 nextDirD;
	int32 tempturns[4];     
	int32 turns[4];     
	int32 turntable[NO_DIRECTIONS] = { 0, 1, 3, 5, 7, 5, 3, 1 };

	// route.X route.Y and route.Dir start at far end

	smoothPath[0].x = startX;
	smoothPath[0].y = startY;
	smoothPath[0].dir = startDir;
	smoothPath[0].num = 0;

	p = 0;
	lastDir = startDir;

	// for each section of the route

	do {     
		dirS = route[p].dirS;
		dirD = route[p].dirD;
		nextDirS = route[p+1].dirS;
		nextDirD = route[p+1].dirD;

		// Check directions into and out of a pair of nodes going in
		dS = dirS - lastDir;
		if (dS < 0)
			dS = dS + NO_DIRECTIONS;

		dD = dirD - lastDir;
		if (dD < 0)
			dD = dD + NO_DIRECTIONS;

		// coming out
		dSS = dirS - nextDirS;
		if (dSS < 0)
			dSS = dSS + NO_DIRECTIONS;

		dDD = dirD - nextDirD;
		if (dDD < 0)
			dDD = dDD + NO_DIRECTIONS;

		dSD = dirS - nextDirD;
		if (dSD < 0)
			dSD = dSD + NO_DIRECTIONS;

		dDS = dirD - nextDirS;
		if (dDS < 0)
			dDS = dDS + NO_DIRECTIONS;

		// Determine the amount of turning involved in each possible
		// path 

		dS = turntable[dS];
		dD = turntable[dD];
		dSS = turntable[dSS];
		dDD = turntable[dDD];
		dSD = turntable[dSD];
		dDS = turntable[dDS];

		// get the best path out ie assume next section uses best
		// direction 

		if (dSD < dSS)
			dSS = dSD;

		if (dDS < dDD)
			dDD = dDS;

		// Rate each option. Split routes look crap so weight against
		// them

		SS = dS + dSS + 3;
		SD = dS + dDD;
		DS = dD + dSS;
		DD = dD + dDD + 3;

		// set up turns as a sorted array of the turn values

		tempturns[0] = SS;
		turns[0] = 0;
		tempturns[1] = SD;
		turns[1] = 1;
		tempturns[2] = DS;
		turns[2] = 2;
		tempturns[3] = DD;
		turns[3] = 3;
		i = 0;

		for (i = 0; i < 3; i++) {
			for (j = 0; j < 3; j++) {
				if (tempturns[j] > tempturns[j + 1]) {
					temp = turns[j];
					turns[j] = turns[j + 1];
					turns[j + 1] = temp;
					temp = tempturns[j];
					tempturns[j] = tempturns[j + 1];
					tempturns[j + 1] = temp;
				}
			}
		}

		// best option matched in order of the priority we would like
		// to see on the screen but each option must be checked to see
		// if it can be walked

		options = NewCheck(1, route[p].x, route[p].y, route[p + 1].x, route[p + 1].y);

#ifdef _SWORD2_DEBUG
		if (options == 0) {
			Zdebug("BestTurns fail %d %d %d %d", route[p].x, route[p].y, route[p + 1].x, route[p + 1].y);
			Zdebug("BestTurns fail %d %d %d %d", turns[0], turns[1], turns[2], options);
			Con_fatal_error("BestTurns failed (%s line %u)", __FILE__, __LINE__);
		}
#endif

		i = 0; 
		steps = 0; 

		do {
			option = 1 << turns[i];
			if (option & options)
				steps = SmoothCheck(turns[i], p, dirS, dirD);
			i++;
		} while (steps == 0 && i < 4);

#ifdef _SWORD2_DEBUG
		if (steps == 0) {
			Zdebug("BestTurns failed %d %d %d %d", route[p].x, route[p].y, route[p + 1].x, route[p + 1].y);
			Zdebug("BestTurns failed %d %d %d %d", turns[0], turns[1], turns[2], options);
			Con_fatal_error("BestTurns failed (%s line %u)", __FILE__, __LINE__);
		}
#endif

		// route.X route.Y route.dir and bestTurns start at far end
		p++;
	} while (p < (routeLength));

	// best turns will end heading as near as possible to target dir rest
	// is down to anim for now

	smoothPath[steps].dir = 9;
	smoothPath[steps].num = ROUTE_END_FLAG;
	return 1;				 
}

int32 SmoothCheck(int32 best, int32 p, int32 dirS, int32 dirD) {
	/*********************************************************************
	 * Slip sliding away
	 * This path checker checks to see if a walk that exactly follows the
	 * path would be valid. This should be inherently true for atleast one
	 * of the turn options.
	 * No longer checks the data it only creates the smoothPath array JPS
	 *********************************************************************/

	static int32 k;   
	int32 x;
	int32 y;
	int32 x2;
	int32 y2;
	int32 ldx;
	int32 ldy;
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

	x = route[p].x;
	y = route[p].y;
	x2 = route[p + 1].x;
	y2 = route[p + 1].y;
	ldx = x2 - x;
	ldy = y2 - y;
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

	// set up sd0-ss2 to reflect possible movement in each direction

	if (dirS == 0 || dirS == 4) {	// vert and diag
		ddx = ldx;
		ddy = (ldx * diagonaly) / diagonalx;
		dsy = ldy - ddy;
		ddx = ddx * dirX;
		ddy = ddy * dirY;
		dsy = dsy * dirY;
		dsx = 0;

		sd0 = (ddx + modX[dirD] / 2) / modX[dirD];
		ss0 = (dsy + modY[dirS] / 2) / modY[dirS];
		sd1 = sd0 / 2;
		ss1 = ss0 / 2;
		sd2 = sd0 - sd1;
		ss2 = ss0 - ss1;
	} else {
		ddy = ldy;
		ddx = (ldy * diagonalx) / diagonaly;
		dsx = ldx - ddx;
		ddy = ddy * dirY;
		ddx = ddx * dirX;
		dsx = dsx * dirX;
		dsy = 0;

		sd0 = (ddy + modY[dirD] / 2) / modY[dirD];
		ss0 = (dsx + modX[dirS] / 2) / modX[dirS];
		sd1 = sd0 / 2;
		ss1 = ss0 / 2;
		sd2 = sd0 - sd1;
		ss2 = ss0 - ss1;
	}

	if (best == 0) {	// halfsquare, diagonal, halfsquare
		smoothPath[k].x = x + dsx / 2;
		smoothPath[k].y = y + dsy / 2;
		smoothPath[k].dir = dirS;
		smoothPath[k].num = ss1;
		k++;

		smoothPath[k].x = x + dsx / 2 + ddx;
		smoothPath[k].y = y + dsy / 2 + ddy;
		smoothPath[k].dir = dirD;
		smoothPath[k].num = sd0;
		k++;

		smoothPath[k].x = x + dsx + ddx;
		smoothPath[k].y = y + dsy + ddy;
		smoothPath[k].dir = dirS;
		smoothPath[k].num = ss2;
		k++;
	} else if (best == 1) {	// square, diagonal
		smoothPath[k].x = x + dsx;
		smoothPath[k].y = y + dsy;
		smoothPath[k].dir = dirS;
		smoothPath[k].num = ss0;
		k++;

		smoothPath[k].x = x2;
		smoothPath[k].y = y2;
		smoothPath[k].dir = dirD;
		smoothPath[k].num = sd0;
		k++;
	} else if (best == 2) {	// diagonal square
		smoothPath[k].x = x + ddx;
		smoothPath[k].y = y + ddy;
		smoothPath[k].dir = dirD;
		smoothPath[k].num = sd0;
		k++;

		smoothPath[k].x = x2;
		smoothPath[k].y = y2;
		smoothPath[k].dir = dirS;
		smoothPath[k].num = ss0;
		k++;
	} else {		// halfdiagonal, square, halfdiagonal
		smoothPath[k].x = x + ddx / 2;
		smoothPath[k].y = y + ddy / 2;
		smoothPath[k].dir = dirD;
		smoothPath[k].num = sd1;
		k++;

		smoothPath[k].x = x + dsx + ddx / 2;
		smoothPath[k].y = y + dsy + ddy / 2;
		smoothPath[k].dir = dirS;
		smoothPath[k].num = ss0;
		k++;

		smoothPath[k].x = x2;
		smoothPath[k].y = y2;
		smoothPath[k].dir = dirD;
		smoothPath[k].num = sd2;
		k++;
	}
	
	return k;	
}

int32 SlidyPath() {
	/*********************************************************************
	 * SlidyPath creates a path based on part steps with no sliding to get
	 * as near as possible to the target without any sliding this routine
	 * is currently unused, but is intended for use when just clicking
	 * about.
	 *
	 * produce a module list from the line data
	 *
	 *********************************************************************/

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

	while (smoothPath[smooth].num < ROUTE_END_FLAG) {
		scale = scaleA * smoothPath[smooth].y + scaleB;
		deltaX = smoothPath[smooth].x - modularPath[slidy - 1].x;
		deltaY = smoothPath[smooth].y - modularPath[slidy - 1].y;
		stepX = modX[smoothPath[smooth].dir];
		stepY = modY[smoothPath[smooth].dir];
		stepX = stepX * scale;
		stepY = stepY * scale;
		stepX = stepX >> 19;	// quarter a step minimum
		stepY = stepY >> 19;

		if (ABS(deltaX) >= ABS(stepX) && ABS(deltaY) >= ABS(stepY)) {
	 		modularPath[slidy].x = smoothPath[smooth].x;
			modularPath[slidy].y = smoothPath[smooth].y;
			modularPath[slidy].dir = smoothPath[smooth].dir;
			modularPath[slidy].num = 1;
			slidy++;
		}
		smooth++;
	}

	// in case the last bit had no steps

	if (slidy > 1) {
		modularPath[slidy - 1].x = smoothPath[smooth - 1].x;
		modularPath[slidy - 1].y = smoothPath[smooth - 1].y;
	}

	// set up the end of the walk

	modularPath[slidy].x = smoothPath[smooth - 1].x;
	modularPath[slidy].y = smoothPath[smooth - 1].y;
	modularPath[slidy].dir = targetDir;
	modularPath[slidy].num = 0;
	slidy++;

	modularPath[slidy].x = smoothPath[smooth - 1].x;
	modularPath[slidy].y = smoothPath[smooth - 1].y;
	modularPath[slidy].dir = 9;
	modularPath[slidy].num = ROUTE_END_FLAG;

	return 1;
}

// SLOW IN

int32 AddSlowInFrames(_walkData *walkAnim) {
	uint32 slowInFrameNo;

	if (usingSlowInFrames && modularPath[1].num > 0) {
		for (slowInFrameNo = 0; slowInFrameNo < numberOfSlowInFrames[currentDir]; slowInFrameNo++) {
			walkAnim[stepCount].frame = firstSlowInFrame[currentDir] + slowInFrameNo;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = currentDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount++;
		}
		return 1;
	}

	return 0;
}

void EarlySlowOut(Object_mega *ob_mega, Object_walkdata *ob_walkdata) {
	int32 slowOutFrameNo;
	int32 walk_pc;
	_walkData *walkAnim;

	// Zdebug("\nEARLY SLOW-OUT");

	LoadWalkData(ob_walkdata);

	// Zdebug("********************************");
	// Zdebug("framesPerStep              =%d", framesPerStep);
	// Zdebug("numberOfSlowOutFrames      =%d", numberOfSlowOutFrames);
	// Zdebug("firstWalkingTurnLeftFrame  =%d", firstWalkingTurnLeftFrame);
	// Zdebug("firstWalkingTurnRightFrame =%d", firstWalkingTurnRightFrame);
	// Zdebug("firstSlowOutFrame          =%d", firstSlowOutFrame);
	// Zdebug("********************************");

 	walk_pc  = ob_mega->walk_pc;

	// lock the _walkData array (NB. AFTER loading walkgrid & walkdata!)
	walkAnim = LockRouteMem();

	// if this mega does actually have slow-out frames
	if (usingSlowOutFrames) {
		// overwrite the next step (half a cycle) of the walk 
		// (ie .step - 0..5)

		do {
			// Zdebug("\nSTEP NUMBER:    walkAnim[%d].step  = %d", walk_pc, walkAnim[walk_pc].step);
			// Zdebug("ORIGINAL FRAME: walkAnim[%d].frame = %d", walk_pc, walkAnim[walk_pc].frame);

			// map from existing walk frame across to correct
			// frame number of slow-out - remember, there may be
			// more slow-out frames than walk-frames!

			if (walkAnim[walk_pc].frame >= firstWalkingTurnRightFrame) {
				// if it's a walking turn-right, rather than a
				// normal step, then map it to a normal step
				// frame first

				walkAnim[walk_pc].frame -= firstWalkingTurnRightFrame;
				// Zdebug("MAPPED TO WALK: walkAnim[%d].frame = %d  (walking turn-right frame --> walk frame)", walk_pc, walkAnim[walk_pc].frame);
			} else if (walkAnim[walk_pc].frame >= firstWalkingTurnLeftFrame) {
				// if it's a walking turn-left, rather than a
				// normal step, then map it to a normal step
				// frame first

				walkAnim[walk_pc].frame -= firstWalkingTurnLeftFrame;
				// Zdebug("MAPPED TO WALK: walkAnim[%d].frame = %d  (walking turn-left frame --> walk frame)", walk_pc, walkAnim[walk_pc].frame);
			}

			walkAnim[walk_pc].frame += firstSlowOutFrame + ((walkAnim[walk_pc].frame / framesPerStep) * (numberOfSlowOutFrames - framesPerStep));
			walkAnim[walk_pc].step = 0;
			// Zdebug("SLOW-OUT FRAME: walkAnim[%d].frame = %d",walk_pc, walkAnim[walk_pc].frame);
			walk_pc++;
		} while(walkAnim[walk_pc].step > 0);

		// Zdebug("\n");

		// add stationary frame(s) (OPTIONAL)

		for (slowOutFrameNo = framesPerStep; slowOutFrameNo < numberOfSlowOutFrames; slowOutFrameNo++) {
			walkAnim[walk_pc].frame = walkAnim[walk_pc - 1].frame + 1;
			// Zdebug("EXTRA FRAME:    walkAnim[%d].frame = %d", walk_pc, walkAnim[walk_pc].frame);
			walkAnim[walk_pc].step = 0;
			walkAnim[walk_pc].dir = walkAnim[walk_pc - 1].dir;
			walkAnim[walk_pc].x = walkAnim[walk_pc - 1].x;
			walkAnim[walk_pc].y = walkAnim[walk_pc - 1].y;
			walk_pc++;
		}
	} else {
		// this mega doesn't have slow-out frames
		// stand in current direction

		walkAnim[walk_pc].frame = firstStandFrame + walkAnim[walk_pc - 1].dir;
		walkAnim[walk_pc].step = 0;
		walkAnim[walk_pc].dir = walkAnim[walk_pc - 1].dir;
		walkAnim[walk_pc].x = walkAnim[walk_pc - 1].x;
		walkAnim[walk_pc].y = walkAnim[walk_pc - 1].y;
		walk_pc++;
	}

	// end of sequence
	walkAnim[walk_pc].frame	= 512;

	// so that this doesn't happen again while 'george_walking' is still
	// '2'
	walkAnim[walk_pc].step = 99;
}

// SLOW OUT

void AddSlowOutFrames(_walkData *walkAnim) {
	int32 slowOutFrameNo;

	// if the mega did actually walk, we overwrite the last step (half a
	// cycle) with slow-out frames + add any necessary stationary frames

 	if (usingSlowOutFrames && lastCount >= framesPerStep) {
		// place stop frames here
		// slowdown at the end of the last walk
	
		slowOutFrameNo = lastCount - framesPerStep;

		// Zdebug("SLOW OUT: slowOutFrameNo(%d) = lastCount(%d) - framesPerStep(%d)", slowOutFrameNo, lastCount, framesPerStep);
	
		// overwrite the last step (half a cycle) of the walk

		do {
			// map from existing walk frame across to correct
			// frame number of slow-out - remember, there may be
			// more slow-out frames than walk-frames!

			walkAnim[slowOutFrameNo].frame += firstSlowOutFrame + ((walkAnim[slowOutFrameNo].frame / framesPerStep) * (numberOfSlowOutFrames - framesPerStep));

			// because no longer a normal walk-step
			walkAnim[slowOutFrameNo].step = 0;

			//Zdebug("walkAnim[%d].frame = %d",slowOutFrameNo,walkAnim[slowOutFrameNo].frame);
			slowOutFrameNo++;
		} while(slowOutFrameNo < lastCount);
	
		// add stationary frame(s) (OPTIONAL)

		for (slowOutFrameNo = framesPerStep; slowOutFrameNo < numberOfSlowOutFrames; slowOutFrameNo++) {
			walkAnim[stepCount].frame = walkAnim[stepCount - 1].frame + 1;

			// Zdebug("EXTRA FRAMES: walkAnim[%d].frame = %d", stepCount, walkAnim[stepCount].frame);

			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = walkAnim[stepCount - 1].dir;
			walkAnim[stepCount].x = walkAnim[stepCount - 1].x;
			walkAnim[stepCount].y = walkAnim[stepCount - 1].y;
			stepCount++;
		}
	}
}

void SlidyWalkAnimator(_walkData *walkAnim) {
	/*********************************************************************
	 * Skidding every where HardWalk creates an animation that exactly
	 * fits the smoothPath and uses foot slipping to fit whole steps into
	 * the route
	 *
	 *	Parameters:	georgeg, mouseg
	 *	Returns:	rout 
	 *
	 * produce a module list from the line data
	 *
	 *********************************************************************/

	static int32 left = 0;
	int32 p;
	int32 lastDir;
	int32 lastRealDir;
	int32 turnDir;
	int32 scale;
	int32 step;
	int32 module;
	int32 moduleEnd;
	int32 module16X;
	int32 module16Y;
	int32 stepX;
	int32 stepY;
	int32 errorX;
	int32 errorY;
	int32 lastErrorX;
	int32 lastErrorY;
	int32 frameCount;
	int32 frames;

	p = 0;
	lastDir = modularPath[0].dir;
	currentDir = modularPath[1].dir;

	if (currentDir == NO_DIRECTIONS)
		currentDir = lastDir;

	moduleX = startX;
	moduleY = startY;
	module16X = moduleX << 16;
	module16Y = moduleY << 16;
	stepCount = 0;

	// START THE WALK WITH THE FIRST STANDFRAME THIS MAY CAUSE A DELAY
	// BUT IT STOPS THE PLAYER MOVING FOR COLLISIONS ARE DETECTED

	// Zdebug("\nSLIDY: STARTING THE WALK");

	module = framesPerChar + lastDir;
	walkAnim[stepCount].frame = module;
	walkAnim[stepCount].step = 0;
	walkAnim[stepCount].dir = lastDir;
	walkAnim[stepCount].x = moduleX;
	walkAnim[stepCount].y = moduleY;
	stepCount++;

	// TURN TO START THE WALK

	// Zdebug("\nSLIDY: TURNING TO START THE WALK");
	// rotate if we need to

	if (lastDir != currentDir) {
		// get the direction to turn
		turnDir = currentDir - lastDir;
		if (turnDir < 0)
			turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to new walk direction
		// for george and nico put in a head turn at the start

		if (usingStandingTurnFrames) {
			// new frames for turn frames	29oct95jps
			if (turnDir < 0)
				module = firstStandingTurnLeftFrame + lastDir;
			else
				module = firstStandingTurnRightFrame + lastDir;

			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount++;
		}

		// rotate till were facing new dir then go back 45 degrees
		while (lastDir != currentDir) {
			lastDir += turnDir;

			// new frames for turn frames	29oct95jps
			if (turnDir < 0) {
				if ( lastDir < 0)
					lastDir += NO_DIRECTIONS;
				module = firstStandingTurnLeftFrame + lastDir;
			} else {
				if ( lastDir > 7)
					lastDir -= NO_DIRECTIONS;
				module = firstStandingTurnRightFrame + lastDir;
			}

			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount++;
		}

		// the back 45 degrees bit
		// step back one because new head turn for george takes us
		// past the new dir
		stepCount--;
	}

	// his head is in the right direction
	lastRealDir = currentDir;

	// SLIDY: THE SLOW IN

	AddSlowInFrames(walkAnim);

	// THE WALK

	// Zdebug("\nSLIDY: THE WALK");

	// start the walk on the left or right leg, depending on how the
	// slow-in frames were drawn

	// (0 = left; 1 = right)

	if (leadingLeg[currentDir] == 0) {
		// start the walk on the left leg (ie. at beginning of the
		// first step of the walk cycle)
		left = 0;
	} else {
		// start the walk on the right leg (ie. at beginning of the
		// second step of the walk cycle)
		left = framesPerStep;
	}

	lastCount = stepCount;

	// this ensures that we don't put in turn frames for the start
	lastDir = 99;

	// this ensures that we don't put in turn frames for the start
	currentDir = 99;

	do {
		assert(stepCount < O_WALKANIM_SIZE);
		while (modularPath[p].num == 0) {
			p++;
			if (currentDir != 99)
				lastRealDir = currentDir;
			lastDir = currentDir;
			lastCount = stepCount;
		}

		// calculate average amount to lose in each step on the way
		// to the next node

		currentDir = modularPath[p].dir;

		if (currentDir < NO_DIRECTIONS) {
			module = currentDir * framesPerStep * 2 + left;

			if (left == 0)
				left = framesPerStep;
			else
				left = 0;

			moduleEnd = module + framesPerStep;
			step = 0;
			scale = (scaleA * moduleY + scaleB);

			do {
				module16X += dx[module] * scale;
				module16Y += dy[module] * scale;
				moduleX = module16X >> 16;
				moduleY = module16Y >> 16;
				walkAnim[stepCount].frame = module;
				walkAnim[stepCount].step = step;	// normally 0,1,2,3,4,5,0,1,2,etc
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount++;
				step++;
				module++;
			} while(module < moduleEnd);

			stepX = modX[modularPath[p].dir];
			stepY = modY[modularPath[p].dir];
			errorX = modularPath[p].x - moduleX;
			errorX = errorX * stepX;
			errorY = modularPath[p].y - moduleY;
			errorY = errorY * stepY;

			if (errorX < 0 || errorY < 0) {
				modularPath[p].num = 0;	// the end of the path

				// okay those last steps took us past our
				// target but do we want to scoot or moonwalk

				frames = stepCount - lastCount;
				errorX = modularPath[p].x - walkAnim[stepCount - 1].x;
				errorY = modularPath[p].y - walkAnim[stepCount - 1].y;

				if (frames > framesPerStep) {
					lastErrorX = modularPath[p].x - walkAnim[stepCount - 7].x;
					lastErrorY = modularPath[p].y - walkAnim[stepCount - 7].y;

					if (stepX == 0) {
						if (3 * ABS(lastErrorY) < ABS(errorY)) {
							// the last stop was
							// closest
							stepCount -= framesPerStep;
							if (left == 0)
						 		left = framesPerStep;
							else
							 	left = 0;
						}
					} else {
						if (3 * ABS(lastErrorX) < ABS(errorX)) {
							//the last stop was
							// closest
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

				// okay we've reached the end but we still
				// have an error

				if (errorX != 0) {
					frameCount = 0;
					frames = stepCount - lastCount;

					do {
						frameCount++;
						walkAnim[lastCount + frameCount - 1].x += errorX * frameCount / frames;
					} while(frameCount < frames);	
				}

				if (errorY != 0) {
					frameCount = 0;
					frames = stepCount - lastCount;
					do {
						frameCount++;
						walkAnim[lastCount + frameCount - 1].y += errorY * frameCount / frames;
					} while(frameCount < frames);	
				}

				// Now is the time to put in the turn frames
				// for the last turn

				if (frames < framesPerStep) {
					// this ensures that we don't put in
					// turn frames for this walk or the
					// next		
					currentDir = 99;
				}

				if (currentDir != 99)
					lastRealDir = currentDir;

				// check each turn condition in turn

				 // only for george
				if (lastDir != 99 && currentDir != 99 && usingWalkingTurnFrames) {
					// 1 and -7 going right -1 and 7 going
					// left
					lastDir = currentDir - lastDir;

					if (lastDir == -1 || lastDir == 7 || lastDir == -2 || lastDir == 6) {
						// turn at the end of the last
						// walk

						frame = lastCount - framesPerStep;
						do {
							// turning left 
							walkAnim[frame].frame += firstWalkingTurnLeftFrame;
							frame++;
						} while (frame < lastCount);
					} else if (lastDir == 1 || lastDir == -7 || lastDir == 2 || lastDir == -6) {	
						// turn at the end of the
						// current walk

						frame = lastCount - framesPerStep;
						do {
							// turning right
							walkAnim[frame].frame += firstWalkingTurnRightFrame;
							frame++;
						} while(frame < lastCount);
					}
					lastDir = currentDir;
				}

				// all turns checked

				lastCount = stepCount;
				moduleX = walkAnim[stepCount - 1].x;
				moduleY = walkAnim[stepCount - 1].y;
				module16X = moduleX << 16;
				module16Y = moduleY << 16;
			}
		}
	} while (modularPath[p].dir < NO_DIRECTIONS);

#ifdef _SWORD2_DEBUG
	if (lastRealDir == 99)
		Con_fatal_error("SlidyWalkAnimatorlast direction error (%s line %u)", __FILE__, __LINE__);
#endif

	// THE SLOW OUT
	AddSlowOutFrames(walkAnim);

	// TURNS TO END THE WALK ?

	// We've done the walk now put in any turns at the end

	if (targetDir == 8) {
		// ANY direction -> stand in the last direction

		module = firstStandFrame + lastRealDir;
		targetDir = lastRealDir;
		walkAnim[stepCount].frame = module;
		walkAnim[stepCount].step = 0;
		walkAnim[stepCount].dir = lastRealDir;
		walkAnim[stepCount].x = moduleX;
		walkAnim[stepCount].y = moduleY;
		stepCount++;
	}

	if (targetDir == 9) {
		// 'stance' was non-zero
		if (stepCount == 0) {
			module = framesPerChar + lastRealDir;
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount++;
		}
	} else if (targetDir != lastRealDir) {
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

		if (usingStandingTurnFrames) {
			// new frames for turn frames	29oct95jps
			if (turnDir < 0)
				module = firstStandingTurnLeftFrame + lastDir;
			else
				module = firstStandingTurnRightFrame + lastDir;

			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount++;
		}

		// rotate if we need to

		while (lastRealDir != targetDir) {
			lastRealDir += turnDir;

			// new frames for turn frames	29oct95jps
			if (turnDir < 0) {
				if (lastRealDir < 0)
					lastRealDir += NO_DIRECTIONS;
				module = firstStandingTurnLeftFrame + lastRealDir;
			} else {
				if (lastRealDir > 7)
					lastRealDir -= NO_DIRECTIONS;
				module = firstStandingTurnRightFrame + lastRealDir;
			}

			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount++;
		}

		module = firstStandFrame + lastRealDir;
		walkAnim[stepCount - 1].frame = module;
	} else {
		// just stand at the end
		module = firstStandFrame + lastRealDir;
		walkAnim[stepCount].frame = module;
		walkAnim[stepCount].step = 0;
		walkAnim[stepCount].dir = lastRealDir;
		walkAnim[stepCount].x = moduleX;
		walkAnim[stepCount].y = moduleY;
		stepCount++;
	}

	walkAnim[stepCount].frame = 512;
	walkAnim[stepCount].step = 99;
	stepCount++;

	walkAnim[stepCount].frame = 512;
	walkAnim[stepCount].step = 99;
	stepCount++;

	walkAnim[stepCount].frame = 512;
	walkAnim[stepCount].step = 99;

#ifdef _SWORD2_DEBUG
	// write all the frames to "debug.txt"
	// Zdebug("\nTHE WALKDATA:");

	for (frame = 0; frame <= stepCount; frame++)
		Zdebug("walkAnim[%d].frame=%d", frame, walkAnim[frame].frame);
#endif

	// Zdebug("RouteFinder RouteSize is %d", stepCount);
	return;
}

// THE SOLID PATH ROUTINES

int32 SolidPath() {
	/*********************************************************************
	 * SolidPath creates a path based on whole steps with no sliding to
	 * get as near as possible to the target without any sliding this
	 * routine is currently unused, but is intended for use when just
	 * clicking about.
	 *
	 * produce a module list from the line data
	 *
	 *********************************************************************/

	// FIXME: It says in the comment about that this function is currently
	// unused, but is it really?

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

	do {
		scale = scaleA * smoothPath[smooth].y + scaleB;
		deltaX = smoothPath[smooth].x - modularPath[solid - 1].x;
		deltaY = smoothPath[smooth].y - modularPath[solid - 1].y;
		stepX = modX[smoothPath[smooth].dir];
		stepY = modY[smoothPath[smooth].dir];
		stepX = stepX * scale;
		stepY = stepY * scale;
		stepX = stepX >> 16;
		stepY = stepY >> 16;

		if (ABS(deltaX) >= ABS(stepX) && ABS(deltaY) >= ABS(stepY)) {
			modularPath[solid].x = smoothPath[smooth].x;
			modularPath[solid].y = smoothPath[smooth].y;
			modularPath[solid].dir = smoothPath[smooth].dir;
			modularPath[solid].num = 1;
			solid++;
		}
		smooth++;
	} while (smoothPath[smooth].num < ROUTE_END_FLAG);

	// in case the last bit had no steps

	if (solid == 1) {
		// there were no paths so put in a dummy end
		solid = 2;
		modularPath[1].dir = smoothPath[0].dir;
		modularPath[1].num = 0;
	}	

	modularPath[solid - 1].x = smoothPath[smooth - 1].x;
	modularPath[solid - 1].y = smoothPath[smooth - 1].y;

	// set up the end of the walk
	modularPath[solid].x = smoothPath[smooth - 1].x;
	modularPath[solid].y = smoothPath[smooth - 1].y;
	modularPath[solid].dir = 9;
	modularPath[solid].num = ROUTE_END_FLAG;

	return 1;
}

int32 SolidWalkAnimator(_walkData *walkAnim) {
	/*********************************************************************
	 * SolidWalk creates an animation based on whole steps with no sliding
	 * to get as near as possible to the target without any sliding. This
	 * routine is is intended for use when just clicking about.
	 *
	 * produce a module list from the line data
	 *
	 * returns 0 if solid route not found
	 *********************************************************************/

	int32 p;
	int32 i;
	int32 left;
	int32 lastDir;
	int32 turnDir;
	int32 scale;
	int32 step;
	int32 module;
	int32 module16X;
	int32 module16Y;
	int32 errorX;
	int32 errorY;
	int32 moduleEnd;
	int32 slowStart = 0;

	// start at the beginning for a change

	lastDir = modularPath[0].dir;
	p = 1;
	currentDir = modularPath[1].dir;
	module = framesPerChar + lastDir;
	moduleX = startX;
	moduleY = startY;
	module16X = moduleX << 16;
	module16Y = moduleY << 16;
	stepCount = 0;

	// START THE WALK WITH THE FIRST STANDFRAME THIS MAY CAUSE A DELAY
	// BUT IT STOPS THE PLAYER MOVING FOR COLLISIONS ARE DETECTED

	//Zdebug("\nSOLID: STARTING THE WALK");
	walkAnim[stepCount].frame = module;
	walkAnim[stepCount].step = 0;
	walkAnim[stepCount].dir = lastDir;
	walkAnim[stepCount].x = moduleX;
	walkAnim[stepCount].y = moduleY;
	stepCount++;

	// TURN TO START THE WALK

	// Zdebug("\nSOLID: TURNING TO START THE WALK");

	// rotate if we need to

	if (lastDir != currentDir) {
		// get the direction to turn
		turnDir = currentDir - lastDir;
		if (turnDir < 0)
			turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to new walk direction
		// for george and nico put in a head turn at the start

		if (usingStandingTurnFrames) {
			// new frames for turn frames	29oct95jps
			if (turnDir < 0)
				module = firstStandingTurnLeftFrame + lastDir;
			else
				module = firstStandingTurnRightFrame + lastDir;

			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount++;
		}

		// rotate till were facing new dir then go back 45 degrees

		while (lastDir != currentDir) {
			lastDir += turnDir;

			// new frames for turn frames	29oct95jps
			if (turnDir < 0) {
				if (lastDir < 0)
					lastDir += NO_DIRECTIONS;
				module = firstStandingTurnLeftFrame + lastDir;
			} else {
				if ( lastDir > 7)
					lastDir -= NO_DIRECTIONS;
				module = firstStandingTurnRightFrame + lastDir;
			}

			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount++;
		}

		// the back 45 degrees bit
		// step back one because new head turn for george takes us
		// past the new dir

		stepCount--;
	}

	// THE SLOW IN

	slowStart = AddSlowInFrames(walkAnim);

	// THE WALK

	// Zdebug("\nSOLID: THE WALK");

	// start the walk on the left or right leg, depending on how the
	// slow-in frames were drawn

	// (0 = left; 1 = right)
	if (leadingLeg[currentDir] == 0) {
		// start the walk on the left leg (ie. at beginning of the
		// first step of the walk cycle)
		left = 0;
	} else {
		// start the walk on the right leg (ie. at beginning of the
		// second step of the walk cycle)
		left = framesPerStep;
	}

	lastCount = stepCount;

	// this ensures that we don't put in turn frames for the start
	lastDir = 99;

	// this ensures that we don't put in turn frames for the start
	currentDir = 99;

	do {
		while(modularPath[p].num > 0) {
			currentDir = modularPath[p].dir;
			if (currentDir< NO_DIRECTIONS) {
				module = currentDir * framesPerStep * 2 + left;

				if (left == 0)
					left = framesPerStep;
				else
					left = 0;

				moduleEnd = module + framesPerStep;
				step = 0;
				scale = (scaleA * moduleY + scaleB);

				do {
					module16X += dx[module] * scale;
					module16Y += dy[module] * scale;
					moduleX = module16X >> 16;
					moduleY = module16Y >> 16;
					walkAnim[stepCount].frame = module;
					walkAnim[stepCount].step = step;	// normally 0,1,2,3,4,5,0,1,2,etc
					walkAnim[stepCount].dir = currentDir;
					walkAnim[stepCount].x = moduleX;
					walkAnim[stepCount].y = moduleY;
					stepCount++;
					module++;
					step++;
				} while( module < moduleEnd);

				errorX = modularPath[p].x - moduleX;
				errorX = errorX * modX[modularPath[p].dir];
				errorY = modularPath[p].y - moduleY;
				errorY = errorY * modY[modularPath[p].dir];

				if (errorX < 0 || errorY < 0) {
					modularPath[p].num = 0;
					stepCount -= framesPerStep;

					if (left == 0)
						left = framesPerStep;
					else
						left = 0;

					// Okay this is the end of a section

					moduleX = walkAnim[stepCount - 1].x;
					moduleY = walkAnim[stepCount - 1].y;
					module16X = moduleX << 16;
					module16Y = moduleY << 16;
					modularPath[p].x =moduleX;
					modularPath[p].y =moduleY;

					// Now is the time to put in the turn
					// frames for the last turn

					if (stepCount - lastCount < framesPerStep) {
						// no step taken

						// clean up if a slow in but no
						// walk

						if (slowStart == 1) {
							stepCount -= numberOfSlowInFrames[currentDir];	// (James08sep97)
							lastCount -= numberOfSlowInFrames[currentDir];	// (James08sep97)
							slowStart = 0;
						}

						// this ensures that we don't
						// put in turn frames for this
						// walk or the next

						currentDir = 99;
					}

					// check each turn condition in turn
					if (lastDir != 99 && currentDir != 99 && usingWalkingTurnFrames) {	
						// only for george
						// 1 and -7 going right -1 and
						// 7 going left

						lastDir = currentDir - lastDir;

						if (lastDir == -1 || lastDir == 7 || lastDir == -2 || lastDir == 6) {
							// turn at the end of
							// the last walk

							frame = lastCount - framesPerStep;

							do {
								// turning left
								walkAnim[frame].frame += firstWalkingTurnLeftFrame;
								frame++;
							} while (frame < lastCount);
						} else if (lastDir == 1 || lastDir == -7 || lastDir == 2 || lastDir == -6) {	
							// turn at the end of
							// the current walk

							frame = lastCount - framesPerStep;
							do {
								// turning right
								walkAnim[frame].frame += firstWalkingTurnRightFrame;
								frame++;
							} while (frame < lastCount);
						}
					}

					// all turns checked
					lastCount = stepCount;
				}
			}
		}
		p++;
		lastDir = currentDir;

		// can only be valid first time round 
		slowStart = 0;
	} while (modularPath[p].dir < NO_DIRECTIONS);


	// THE SLOW OUT

	AddSlowOutFrames(walkAnim);

	module = framesPerChar + modularPath[p - 1].dir;
	walkAnim[stepCount].frame = module;
	walkAnim[stepCount].step = 0;
	walkAnim[stepCount].dir = modularPath[p - 1].dir;
	walkAnim[stepCount].x = moduleX;
	walkAnim[stepCount].y = moduleY;
	stepCount++;

	walkAnim[stepCount].frame = 512;
	walkAnim[stepCount].step = 99;
	stepCount++;

	walkAnim[stepCount].frame = 512;
	walkAnim[stepCount].step = 99;
	stepCount++;

	walkAnim[stepCount].frame = 512;
	walkAnim[stepCount].step = 99;

#ifdef _SWORD2_DEBUG
	// write all the frames to "debug.txt"
	//Zdebug("\nTHE WALKDATA:");

	for (frame = 0; frame <= stepCount; frame++)
		Zdebug("walkAnim[%d].frame=%d", frame, walkAnim[frame].frame);
#endif

	// NO END TURNS

	// Zdebug("RouteFinder RouteSize is %d", stepCount);
	// now check the route

	i = 0;

	do {
		if (!Check(modularPath[i].x, modularPath[i].y, modularPath[i + 1].x, modularPath[i + 1].y))
			p = 0;
		i++;
	} while (i < p - 1);

	if (p != 0) {
		targetDir = modularPath[p - 1].dir;
		if (CheckTarget(moduleX, moduleY) == 3) {
			// new target on a line
			p = 0;
			// Zdebug("Solid walk target was on a line %d %d", moduleX, moduleY);
		}
	}

	return p;
}

// THE SCAN ROUTINES

int32 Scan(int32 level) {
	/*********************************************************************
	 * Called successively from RouteFinder	until no more changes take
	 * place in the grid array, ie he best path has been found
	 *
	 * Scans through every point in the node array and checks if there is
	 * a route between each point and if this route gives a new route.
	 *
	 * This routine could probably halve its processing time if it doubled
	 * up on the checks after each route check
	 *
	 *********************************************************************/

	int32 i;
	int32 k;
	int32 x1;
	int32 y1;
	int32 x2;
	int32 y2;
	int32 distance;
	int32 changed = 0;

 	// For all the nodes that have new values and a distance less than
	// enddist, ie dont check for new routes from a point we checked
	// before or from a point that is already further away than the best
	// route so far. 

	i = 0;

	do {
		if (node[i].dist < node[nnodes].dist && node[i].level == level) {
			x1 = node[i].x;
			y1 = node[i].y;
			k = nnodes;

			do {
				if (node[k].dist > node[i].dist) {
					x2 = node[k].x;
					y2 = node[k].y;

					if (ABS(x2 - x1) > 4.5 * ABS(y2-y1))
						distance = (8 * ABS(x2 - x1) + 18 * ABS(y2 - y1)) / (54 * 8) + 1;
					else
						distance = (6 * ABS(x2 - x1) + 36 * ABS(y2 - y1)) / (36 * 14) + 1;

					if (distance + node[i].dist < node[nnodes].dist && distance + node[i].dist < node[k].dist) {
						if (NewCheck(0, x1, y1, x2, y2)) {
							node[k].level = level + 1;
							node[k].dist = distance + node[i].dist;
							node[k].prev = i;
							changed = 1;
						}
					}
				}
				k--;
			} while (k > 0);
		}
		i++;
	} while (i < nnodes);

	return changed;
}

int32 NewCheck(int32 status, int32 x1, int32 y1, int32 x2, int32 y2) {
	/*********************************************************************
	 * NewCheck routine checks if the route between two points can be
	 * achieved without crossing any of the bars in the Bars array. 
	 *
	 * NewCheck differs from check in that that 4 route options are
	 * considered corresponding to actual walked routes.
	 *
	 * Note distance doesnt take account of shrinking ??? 
	 *
	 * Note Bars array must be properly calculated ie min max dx dy co
	 *********************************************************************/

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

	if (diagonaly * ldx > diagonalx * ldy) {
		// dir  = 1,2 or 2,3 or 5,6 or 6,7

		dly = ldy;
		dlx = (ldy * diagonalx) / diagonaly;
		ldx = ldx - dlx;
		dlx = dlx * dirX;
		dly = dly * dirY;
		ldx = ldx * dirX;
		ldy = 0;

	 	//options are
		//square, diagonal a code 1 route

		step1 = Check(x1, y1, x1 + ldx, y1);
		if (step1 != 0) {
			step2 = Check(x1 + ldx, y1, x2, y2);
			if (step2 != 0) {
				steps = step1 + step2;
				options = options + 2;
			}
		}

		//diagonal, square a code 2 route

		if (steps == 0 || status == 1) {
			step1 = Check(x1, y1, x1 + dlx, y1 + dly);
			if (step1 != 0) {
				step2 = Check(x1 + dlx, y2, x2, y2);
				if (step2 != 0) {
					steps = step1 + step2;
					options = options + 4;
				}
			}
		}

		//halfsquare, diagonal, halfsquare a code 0 route

		if (steps == 0 || status == 1) {
			step1 = Check(x1, y1, x1 + ldx / 2, y1);
			if (step1 != 0) {
				step2 = Check(x1 + ldx / 2, y1, x1 + ldx / 2 + dlx, y2);
				if (step2 != 0) {
					step3 = Check(x1 + ldx / 2 + dlx, y2, x2, y2);
					if (step3 != 0)	{
						steps = step1 + step2 + step3;
						options++;
					}
				}
			}
		}

		//halfdiagonal, square, halfdiagonal a code 3 route

		if (steps == 0 || status == 1) {
			step1 = Check(x1, y1, x1 + dlx / 2, y1 + dly / 2);
			if (step1 != 0) {
				step2 = Check(x1 + dlx / 2, y1 + dly / 2, x1 + ldx + dlx / 2, y1 + dly / 2);
				if (step2 != 0) {
					step3 = Check(x1 + ldx + dlx / 2, y1 + dly / 2, x2, y2);
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
		dly = (ldx * diagonaly) / diagonalx;
		ldy = ldy - dly;
		dlx = dlx * dirX;
		dly = dly * dirY;
		ldy = ldy * dirY;
		ldx = 0;

	 	//options are
		//square, diagonal a code 1 route

		step1 = Check(x1 ,y1, x1, y1 + ldy);
		if (step1 != 0)	{
			step2 = Check(x1, y1 + ldy, x2, y2);
			if (step2 != 0) {
				steps = step1 + step2;
				options = options + 2;
			}
		}

		//diagonal, square a code 2 route

		if (steps == 0 || status == 1) {
			step1 = Check(x1, y1, x2, y1 + dly);
			if (step1 != 0) {
				step2 = Check(x2, y1 + dly, x2, y2);
				if (step2 != 0) {
					steps = step1 + step2;
					options = options + 4;
				}
			}
		}

		//halfsquare, diagonal, halfsquare a code 0 route

		if (steps == 0 || status == 1) {
			step1 = Check(x1, y1, x1, y1 + ldy / 2);
			if (step1 != 0) {
				step2 = Check(x1, y1 + ldy / 2, x2, y1 + ldy / 2 + dly);
				if (step2 != 0) {
					step3 = Check(x2, y1 + ldy / 2 + dly, x2, y2);
					if (step3 != 0) {
						steps = step1 + step2 + step3;
						options++;
					}
				}
			}
		}

		//halfdiagonal, square, halfdiagonal a code 3 route

		if (steps == 0 || status == 1) {
			step1 = Check(x1, y1, x1 + dlx / 2, y1 + dly / 2);
			if (step1 != 0) {
				step2 = Check(x1 + dlx / 2, y1 + dly / 2, x1 + dlx / 2, y1 + ldy + dly / 2);
				if (step2 != 0) {
					step3 = Check(x1 + dlx / 2, y1 + ldy + dly / 2, x2, y2);
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

// CHECK ROUTINES

int32 Check(int32 x1, int32 y1, int32 x2, int32 y2) {
	// call the fastest line check for the given line 
	// returns 1 if line didn't cross any bars

	if (x1 == x2 && y1 == y2)
		return 1;

	if (x1 == x2)
		return VertCheck(x1, y1, y2);

	if (y1 == y2)
		return HorizCheck(x1, y1, x2);

	return LineCheck(x1, y1, x2, y2);
}

int32 LineCheck(int32 x1, int32 y1, int32 x2, int32 y2) {
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
		if (xmax >= bars[i].xmin && xmin <= bars[i].xmax) {
			// skip if not on module 
			if (ymax >= bars[i].ymin && ymin <= bars[i].ymax) {
				// okay its a valid line calculate an intersept
				// wow but all this arithmatic we must have
				// loads of time

				// slope it he slope between the two lines
				slope = (bars[i].dx * diry) - (bars[i].dy *dirx);
				// assuming parallel lines don't cross
				if (slope != 0) {
					// calculate x intercept and check its
					// on both lines
					xc = ((bars[i].co * dirx) - (co * bars[i].dx)) / slope;

					// skip if not on module
					if (xc >= xmin - 1 &&  xc <= xmax + 1) {
						// skip if not on line 
						if (xc >= bars[i].xmin - 1 && xc <= bars[i].xmax + 1) {
							yc = ((bars[i].co * diry) - (co * bars[i].dy)) / slope;

							// skip if not on module
							if (yc >= ymin - 1 && yc <= ymax + 1) {
								// skip if not on line 
								if (yc >= bars[i].ymin - 1 && yc <= bars[i].ymax + 1) {
									linesCrossed = 0;
								}
							}
						}
					}
				}
			}
		}
		i++;
	} while(i < nbars && linesCrossed);

	return linesCrossed;
}

int32 HorizCheck(int32 x1, int32 y, int32 x2) {
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
		if (xmax >= bars[i].xmin && xmin <= bars[i].xmax) {
			// skip if not on module 
			if (y >= bars[i].ymin && y <= bars[i].ymax) {
				// okay its a valid line calculate an intercept
				// wow but all this arithmatic we must have
				// loads of time

				if (bars[i].dy == 0)
					linesCrossed = 0;          
				else {
					ldy = y - bars[i].y1;
					xc = bars[i].x1 + (bars[i].dx * ldy) / bars[i].dy;
					// skip if not on module 
				    	if (xc >= xmin - 1 && xc <= xmax + 1)
						linesCrossed = 0;          
				}
			}
		}
		i++;
	} while(i < nbars && linesCrossed);

	return linesCrossed;
}

int32 VertCheck(int32 x, int32 y1, int32 y2) {
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
		if (x >= bars[i].xmin && x <= bars[i].xmax) {
			// overlapping
			// skip if not on module 
			if (ymax >= bars[i].ymin && ymin <= bars[i].ymax) {
				// okay its a valid line calculate an intercept
				// wow but all this arithmatic we must have
				// loads of time

				// both lines vertical and overlap in x and y
				// so they cross

				if (bars[i].dx == 0)
					linesCrossed = 0;
				else {
			 		ldx = x - bars[i].x1;
					yc = bars[i].y1 + (bars[i].dy * ldx) / bars[i].dx;
					// the intercept overlaps 
					if (yc >= ymin - 1 && yc <= ymax + 1)
						linesCrossed = 0;
				}
			}
		}
		i++;
	} while(i < nbars && linesCrossed);

	return linesCrossed;
}

int32 CheckTarget(int32 x, int32 y) {
	int32 ldx;
	int32 ldy;
	int32 i;
	int32 xc;
	int32 yc;
	int32 xmin;
	int32 xmax;
	int32 ymin;
	int32 ymax;
	int32 onLine = 0;   

 	xmin = x - 1;    
 	xmax = x + 1;
 	ymin = y - 1;    
 	ymax = y + 1;

	// check if point +- 1 is on the line
	//so ignore if it hits anything

	i = 0;          

	do {
		// overlapping line 
		if (xmax >= bars[i].xmin && xmin <= bars[i].xmax) {
			//overlapping line 
			if (ymax >= bars[i].ymin && ymin <= bars[i].ymax) {
				// okay this line overlaps the target calculate
				// an y intercept for x 

				// vertical line so we know it overlaps y
				if (bars[i].dx == 0)
					yc = 0; 	
				else {
					ldx = x - bars[i].x1;
					yc = bars[i].y1 + (bars[i].dy * ldx) / bars[i].dx;
				}

				// overlapping point for y 
				if (yc >= ymin && yc <= ymax) {
					// target on a line so drop out
					onLine = 3;
					// Zdebug("RouteFail due to target on a line %d %d", x, y);
				} else {
					// vertical line so we know it overlaps y
					if (bars[i].dy == 0)
						xc = 0;
					else {
						ldy = y - bars[i].y1;
						xc = bars[i].x1 + (bars[i].dx * ldy) / bars[i].dy;
					}

					// skip if not on module 
					if (xc >= xmin && xc <= xmax) {
						// target on a line so drop out
						onLine = 3;
						// Zdebug("RouteFail due to target on a line %d %d", x, y);
					}
				}
			}
		}
	 	i++;
	} while(i < nbars && onLine == 0);

	return onLine;
}

// THE SETUP ROUTINES

void LoadWalkData(Object_walkdata *ob_walkdata) {
	uint8 direction;
	uint16 firstFrameOfDirection;
	uint16 walkFrameNo;
	uint32 frameCounter = 0; // starts at frame 0 of mega set (16sep96 JEL)
      
	nWalkFrames = ob_walkdata->nWalkFrames;
	usingStandingTurnFrames = ob_walkdata->usingStandingTurnFrames;
	usingWalkingTurnFrames = ob_walkdata->usingWalkingTurnFrames;
	usingSlowInFrames = ob_walkdata->usingSlowInFrames;
	usingSlowOutFrames = ob_walkdata->usingSlowOutFrames;

	// 0 = not using slow out frames; non-zero = using that many frames
	// for each leading leg for each direction

	numberOfSlowOutFrames = usingSlowOutFrames;

 	memcpy(&numberOfSlowInFrames[0], ob_walkdata->nSlowInFrames, NO_DIRECTIONS * sizeof(numberOfSlowInFrames[0]));
 	memcpy(&leadingLeg[0], ob_walkdata->leadingLeg, NO_DIRECTIONS * sizeof(leadingLeg[0]));
 	memcpy(&dx[0], ob_walkdata->dx, NO_DIRECTIONS * (nWalkFrames + 1) * sizeof(dx[0]));
 	memcpy(&dy[0], ob_walkdata->dy, NO_DIRECTIONS * (nWalkFrames + 1) * sizeof(dy[0]));

	for (direction=0; direction < NO_DIRECTIONS; direction++) {
		firstFrameOfDirection = direction * nWalkFrames;

		modX[direction] = 0;
		modY[direction] = 0;

		for (walkFrameNo = firstFrameOfDirection; walkFrameNo < firstFrameOfDirection + nWalkFrames / 2; walkFrameNo++) {
			// eg. modX[0] is the sum of the x-step sizes for the
			// first half of the walk cycle for direction 0
			modX[direction] += dx[walkFrameNo];
			modY[direction] += dy[walkFrameNo];
		}
	}

	diagonalx =  modX[3];
	diagonaly =  modY[3];

	// interpret the walk data

	framesPerStep = nWalkFrames / 2;
	framesPerChar = nWalkFrames * NO_DIRECTIONS;

	// offset pointers added Oct 30 95 JPS
	// mega id references removed 16sep96 by JEL

	// WALK FRAMES
	// start on frame 0

	frameCounter += framesPerChar;

	// STAND FRAMES
	// stand frames come after the walk frames
	// one stand frame for each direction

	firstStandFrame = frameCounter;
	frameCounter += NO_DIRECTIONS;

	// STANDING TURN FRAMES - OPTIONAL!
	// standing turn-left frames come after the slow-out frames
	// one for each direction
	// standing turn-left frames come after the standing turn-right frames
	// one for each direction

	if (usingStandingTurnFrames) {
		firstStandingTurnLeftFrame = frameCounter;
		frameCounter += NO_DIRECTIONS;

		firstStandingTurnRightFrame = frameCounter;
		frameCounter += NO_DIRECTIONS;
	} else {
		// refer instead to the normal stand frames
		firstStandingTurnLeftFrame = firstStandFrame;
		firstStandingTurnRightFrame = firstStandFrame;
	}

	// WALKING TURN FRAMES - OPTIONAL!
	// walking left-turn frames come after the stand frames
	// walking right-turn frames come after the walking left-turn frames

	if (usingWalkingTurnFrames) {
		firstWalkingTurnLeftFrame = frameCounter;
		frameCounter += framesPerChar;

		firstWalkingTurnRightFrame = frameCounter;
		frameCounter += framesPerChar;
	} else {
		firstWalkingTurnLeftFrame = 0;
		firstWalkingTurnRightFrame = 0;
	}

	// SLOW-IN FRAMES - OPTIONAL!
	// slow-in frames come after the walking right-turn frames

	if (usingSlowInFrames) {
		// Make note of frame number of first slow-in frame for each
		// direction. There may be a different number of slow-in
		// frames in each direction

		for (direction = 0; direction < NO_DIRECTIONS; direction++) {
			firstSlowInFrame[direction] = frameCounter;
			frameCounter += numberOfSlowInFrames[direction];
		}
	}

	// SLOW-OUT FRAMES - OPTIONAL!
	// slow-out frames come after the slow-in frames

	if (usingSlowOutFrames)
		firstSlowOutFrame = frameCounter;
}

// THE ROUTE EXTRACTOR

void ExtractRoute() {
	/*********************************************************************
	 * ExtractRoute gets route from the node data after a full scan, route
	 * is written with just the basic way points and direction options for
	 * heading to the next point. 
	 *********************************************************************/

	int32 prev;
	int32 prevx;
	int32 prevy;
	int32 last;
	int32 point;
	int32 p;     
	int32 dirx;
	int32 diry;
	int32 dir;
	int32 ldx;
	int32 ldy;

 	// extract the route from the node data

	prev = nnodes;
	last = prev;
	point = O_ROUTE_SIZE - 1;
	route[point].x = node[last].x;
	route[point].y = node[last].y;

	do {
		point--;
		prev = node[last].prev;
		prevx = node[prev].x;
		prevy = node[prev].y;
		route[point].x = prevx;
		route[point].y = prevy;
		last = prev;
	} while (prev > 0);

	// now shuffle route down in the buffer

	routeLength = 0;

	do {
		route[routeLength].x = route[point].x;
		route[routeLength].y = route[point].y;
		point++;
		routeLength++;
	} while (point < O_ROUTE_SIZE);

	routeLength = routeLength - 1;

	// okay the route exists as a series point now put in some directions

	p = 0;

	do {
		ldx = route[p + 1].x - route[p].x;
		ldy = route[p + 1].y - route[p].y;
		dirx = 1;
		diry = 1;

		if (ldx < 0) {
			ldx = -ldx;
			dirx = -1;
		}

		if (ldy < 0) {
			ldy = -ldy;
			diry = -1;
		}

		if (diagonaly * ldx > diagonalx * ldy) {
			// dir  = 1,2 or 2,3 or 5,6 or 6,7

			// 2 or 6
			dir = 4 - 2 * dirx;
			route[p].dirS = dir;

			// 1, 3, 5 or 7
			dir = dir + diry * dirx;
			route[p].dirD = dir;
		} else {
			// dir  = 7,0 or 0,1 or 3,4 or 4,5

			// 0 or 4
			dir = 2 + 2 * diry;
			route[p].dirS = dir;

			// 2 or 6
			dir = 4 - 2 * dirx;

			// 1, 3, 5 or 7
			dir = dir + diry * dirx;
			route[p].dirD = dir;
		}
		p++;
	} while (p < routeLength);

	// set the last dir to continue previous route unless specified

	if (targetDir == 8) {
		// ANY direction
		route[p].dirS = route[p - 1].dirS;
		route[p].dirD = route[p - 1].dirD;
	} else { 
		route[p].dirS = targetDir;
		route[p].dirD = targetDir;
	}

	return;
}

void SetUpWalkGrid(Object_mega *ob_mega, int32 x, int32 y, int32 dir) {
	int32 i;

	// get walk grid file + extra grid into 'bars' & 'node' arrays
	LoadWalkGrid();

	// copy the mega structure into the local variables for use in all
	// subroutines

	startX = ob_mega->feet_x;
	startY = ob_mega->feet_y;
	startDir = ob_mega->current_dir;
	targetX = x;
	targetY = y;
	targetDir = dir;

	scaleA = ob_mega->scale_a;
	scaleB = ob_mega->scale_b;

	// mega's current position goes into first node

	node[0].x = startX;
	node[0].y = startY;
	node[0].level = 1;
	node[0].prev = 0;
	node[0].dist = 0;

	// reset other nodes

	for (i = 1; i < nnodes; i++) {
		node[i].level = 0;
	  	node[i].prev = 0;
	  	node[i].dist = 9999;
	}

	// target position goes into final node
	node[nnodes].x = targetX;
	node[nnodes].y = targetY;
	node[nnodes].level = 0;
	node[nnodes].prev = 0;
	node[nnodes].dist = 9999;
}

void PlotWalkGrid(void) {
	int32 j;
      
	// get walk grid file + extra grid into 'bars' & 'node' arrays
	LoadWalkGrid();

	// lines

	for (j = 0; j < nbars; j++)
		DrawLine(bars[j].x1, bars[j].y1, bars[j].x2, bars[j].y2, 254);

	// nodes

	// leave node 0 for start node
	for (j = 1; j < nnodes; j++)
		PlotCross(node[j].x, node[j].y, 184);
}

void PlotCross(int16 x, int16 y, uint8 colour) {
	DrawLine(x - 1, y - 1, x + 1, y + 1, colour);
	DrawLine(x + 1, y - 1, x - 1, y + 1, colour);	
}

void LoadWalkGrid(void) {
	_walkGridHeader floorHeader;
	uint32 j;
	uint8 *fPolygrid;
 	int entry;
	uint32 theseBars;
	uint32 theseNodes;

	nbars	= 0;	// reset counts
	nnodes	= 1;	// leave node 0 for start-node

	// STATIC GRIDS (added/removed by object logics)

	// go through walkgrid list
	for (entry = 0; entry < MAX_WALKGRIDS; entry++) {
		if (walkGridList[entry]) {
			// open walk grid file
			fPolygrid = res_man.Res_open(walkGridList[entry]);
 			fPolygrid += sizeof(_standardHeader);
 			memmove((uint8 *) &floorHeader, fPolygrid, sizeof(_walkGridHeader));
 			fPolygrid += sizeof(_walkGridHeader);

			// how many bars & nodes are we getting from this
			// walkgrid file

			theseBars = floorHeader.numBars;
			theseNodes = floorHeader.numNodes;

#ifdef _SWORD2_DEBUG
			// check that we're not going to exceed the max
			// allowed in the complete walkgrid arrays

			if (nbars + theseBars >= O_GRID_SIZE)
				Con_fatal_error("Adding walkgrid(%d): %d+%d bars exceeds max %d (%s line %u)",
					walkGridList[entry], nbars, theseBars,
					O_GRID_SIZE, __FILE__, __LINE__);

			if (nnodes + theseNodes >= O_GRID_SIZE)
				Con_fatal_error("Adding walkgrid(%d): %d+%d nodes exceeds max %d (%s line %u)",
					walkGridList[entry], nnodes, theseBars,
					O_GRID_SIZE, __FILE__, __LINE__);
#endif

			// lines

 			memmove((uint8 *) &bars[nbars], fPolygrid, theseBars * sizeof(_barData));

			//move pointer to start of node data
			fPolygrid += theseBars * sizeof(_barData);

			// nodes

			// leave node 0 for start node
			for (j = 0; j < theseNodes; j++) {
				memmove((uint8 *) &node[nnodes + j].x, fPolygrid, 2 * sizeof(int16));
				fPolygrid += 2 * sizeof(int16);
			}

			// close walk grid file
			res_man.Res_close(walkGridList[entry]);

			// increment counts of total bars & nodes in whole
			// walkgrid

			nbars += theseBars;
			nnodes	+= theseNodes;
		}
	}

	// EXTRA GRIDS (moveable grids added by megas)

	// Note that these will be checked against allowed max at the time of
	// creating them

	// extra lines

 	memmove((uint8 *) &bars[nbars], (uint8 *) &extraBars[0], nExtraBars * sizeof(_barData));
	nbars += nExtraBars;

	// extra nodes

 	memmove((uint8 *) &node[nnodes], (uint8 *) &extraNode[0], nExtraNodes * sizeof(_nodeData));
	nnodes += nExtraNodes;
}

void ClearWalkGridList(void) {
	for (int entry=0; entry < MAX_WALKGRIDS; entry++)
		walkGridList[entry] = 0;
}

// called from FN_add_walkgrid

void AddWalkGrid(int32 gridResource) {
	int entry;

	// first, scan list to see if this grid is already included

	entry = 0;
	while (entry < MAX_WALKGRIDS && walkGridList[entry] != gridResource)
		entry++;

	// if this new resource isn't already in the list, then add it,
	// (otherwise finish)

	if (entry == MAX_WALKGRIDS) {
		// scan the list for a free slot
		entry = 0;
		while (entry < MAX_WALKGRIDS && walkGridList[entry])
			entry++;

		// if we found a free slot
		if (entry < MAX_WALKGRIDS)
			walkGridList[entry] = gridResource;
		else
			Con_fatal_error("ERROR: walkGridList[] full in %s line %d", __FILE__, __LINE__);
	}
}

// called from FN_remove_walkgrid

void RemoveWalkGrid(int32 gridResource) {
	int entry;

	// first, scan list to see if this grid is actually there
	entry = 0;
	while (entry < MAX_WALKGRIDS && walkGridList[entry] != gridResource)
		entry++;

	// if we've found it in the list, reset entry to zero (otherwise just
	// ignore the request)
	if (entry < MAX_WALKGRIDS)
		walkGridList[entry] = 0;
}
