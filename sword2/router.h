/* Copyright (C) 1994-2004 Revolution Software Ltd
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

#ifndef _ROUTER_H
#define _ROUTER_H

// This used to be a variable, but it was never set. Actually, it wasn't even
// initialised!
//
// Define this to force the use of slidy router (so solid path not used when
// ending walk in ANY direction)
//
// #define FORCE_SLIDY

#include "sword2/object.h"

namespace Sword2 {

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct WalkData {
	uint16 frame;
	int16 x;
	int16 y;
	uint8 step;
	uint8 dir;
} GCC_PACK;

struct BarData {
	int16 x1;
  	int16 y1;
  	int16 x2;
	int16 y2;
	int16 xmin;
	int16 ymin;
	int16 xmax;
	int16 ymax;
	int16 dx;	// x2 - x1
	int16 dy;	// y2 - y1
	int32 co;	// co = (y1 * dx) - (x1 * dy) from an equation for a
			// line y * dx = x * dy + co
} GCC_PACK;

struct NodeData {
	int16 x;
	int16 y;
	int16 level;
	int16 prev;
	int16 dist;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

// because we only have 2 megas in the game!
#define TOTAL_ROUTE_SLOTS	2

#define MAX_FRAMES_PER_CYCLE	16
#define NO_DIRECTIONS		8
#define MAX_FRAMES_PER_CHAR	(MAX_FRAMES_PER_CYCLE * NO_DIRECTIONS)
#define ROUTE_END_FLAG		255

#define MAX_WALKGRIDS		10

#define	O_WALKANIM_SIZE		600	// max number of nodes in router output
#define	O_GRID_SIZE		200	// max 200 lines & 200 points
#define	O_ROUTE_SIZE		50	// max number of modules in a route

struct RouteData {
	int32 x;
	int32 y;
	int32 dirS;
	int32 dirD;
};

struct PathData {
	int32 x;
	int32 y;
	int32 dir;
	int32 num;
};

class Router {
private:
	Sword2Engine *_vm;

	// stores pointers to mem blocks containing routes created & used by
	// megas (NULL if slot not in use)
	Memory *_routeSlots[TOTAL_ROUTE_SLOTS];

	BarData _bars[O_GRID_SIZE];
	NodeData _node[O_GRID_SIZE];

	int32 _walkGridList[MAX_WALKGRIDS];

	int32 _nBars;
	int32 _nNodes;

	int32 _startX;
	int32 _startY;
	int32 _startDir;
	int32 _targetX;
	int32 _targetY;
	int32 _targetDir;
	int32 _scaleA;
	int32 _scaleB;

	RouteData _route[O_ROUTE_SIZE];
	PathData _smoothPath[O_ROUTE_SIZE];
	PathData _modularPath[O_ROUTE_SIZE];
	int32 _routeLength;

	int32 _framesPerStep;
	int32 _framesPerChar;

	uint8 _nWalkFrames;			// no. of frames per walk cycle
	uint8 _usingStandingTurnFrames;		// any standing turn frames?
	uint8 _usingWalkingTurnFrames;		// any walking turn frames?
	uint8 _usingSlowInFrames;		// any slow-in frames?
	uint8 _usingSlowOutFrames;		// any slow-out frames?
	int32 _dx[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
	int32 _dy[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
	int8 _modX[NO_DIRECTIONS];
	int8 _modY[NO_DIRECTIONS];
	int32 _diagonalx;
	int32 _diagonaly;

	int32 _firstStandFrame;

	int32 _firstStandingTurnLeftFrame;
	int32 _firstStandingTurnRightFrame;

	int32 _firstWalkingTurnLeftFrame;	// left walking turn
	int32 _firstWalkingTurnRightFrame;	// right walking turn

	uint32 _firstSlowInFrame[NO_DIRECTIONS];
	uint32 _numberOfSlowInFrames[NO_DIRECTIONS];

	uint32 _leadingLeg[NO_DIRECTIONS];

	int32 _firstSlowOutFrame;

	// number of slow-out frames on for each leading-leg in each direction
	// ie. total number of slow-out frames = (numberOfSlowOutFrames * 2 *
	// NO_DIRECTIONS)

	int32 _numberOfSlowOutFrames;

	int32 _stepCount;

	int32 _moduleX;
	int32 _moduleY;
	int32 _currentDir;
	int32 _lastCount;
	int32 _frame;

	uint8 returnSlotNo(uint32 megaId);

	int32 getRoute(void);
	void extractRoute(void);
	void loadWalkGrid(void);
	void setUpWalkGrid(ObjectMega *ob_mega, int32 x, int32 y, int32 dir);
	void loadWalkData(ObjectWalkdata *ob_walkdata);
	bool scan(int32 level);

	int32 newCheck(int32 status, int32 x1, int32 y1, int32 x2, int32 y2);
	bool lineCheck(int32 x1, int32 x2, int32 y1, int32 y2);
	bool vertCheck(int32 x, int32 y1, int32 y2);
	bool horizCheck(int32 x1, int32 y, int32 x2);
	bool check(int32 x1, int32 y1, int32 x2, int32 y2);
	int32 checkTarget(int32 x, int32 y);

	int32 smoothestPath(void);
	void slidyPath(void);

	int32 smoothCheck(int32 best, int32 p, int32 dirS, int32 dirD);

	bool addSlowInFrames(WalkData *walkAnim);
	void addSlowOutFrames(WalkData *walkAnim);
	void slidyWalkAnimator(WalkData *walkAnim);

#ifndef FORCE_SLIDY
	int32 solidPath(void);
	int32 solidWalkAnimator(WalkData *walkAnim);
#endif

	void plotCross(int16 x, int16 y, uint8 colour);

public:
	Router(Sword2Engine *vm) : _vm(vm), _diagonalx(0), _diagonaly(0) {
		memset(_routeSlots, 0, sizeof(_routeSlots));
		memset(_bars, 0, sizeof(_bars));
		memset(_node, 0, sizeof(_node));
		memset(_walkGridList, 0, sizeof(_walkGridList));
		memset(_route, 0, sizeof(_route));
		memset(_smoothPath, 0, sizeof(_smoothPath));
		memset(_modularPath, 0, sizeof(_modularPath));
		memset(_dx, 0, sizeof(_dx));
		memset(_dy, 0, sizeof(_dy));
		memset(_modX, 0, sizeof(_modX));
		memset(_modY, 0, sizeof(_modY));
		memset(_firstSlowInFrame, 0, sizeof(_firstSlowInFrame));
		memset(_numberOfSlowInFrames, 0, sizeof(_numberOfSlowInFrames));
		memset(_leadingLeg, 0, sizeof(_leadingLeg));
	}

	int32 routeFinder(ObjectMega *ob_mega, ObjectWalkdata *ob_walkdata, int32 x, int32 y, int32 dir);

	void earlySlowOut(ObjectMega *ob_mega, ObjectWalkdata *ob_walkdata);

	void allocateRouteMem(void);
	WalkData *lockRouteMem(void);
	void floatRouteMem(void);
	void freeRouteMem(void);
	void freeAllRouteMem(void);
	void addWalkGrid(int32 gridResource);
	void removeWalkGrid(int32 gridResource);
	void clearWalkGridList(void);

	void plotWalkGrid(void);
};

} // End of namespace Sword2

#endif
