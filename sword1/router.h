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

#ifndef BSROUTER_H
#define BSROUTER_H

#include "scummsys.h"
#include "object.h"

#define EXTRA_GRID_SIZE 20
#define O_GRID_SIZE 200

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct BarData {
	int16   x1;
  	int16   y1;
  	int16   x2;
	int16   y2;
	int16   xmin;
	int16   ymin;
	int16   xmax;
	int16   ymax;
	int16   dx;	   // x2 - x1
	int16   dy;	   // y2 - y1
	int32   co;	   // co = (y1 *dx)- (x1*dy) from an equation for a line y*dx = x*dy + co
} GCC_PACK;

struct NodeData {
	int16   x;
	int16   y;
	int16	level;
	int16   prev;
	int16   dist;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

struct FloorData {
	int32		nbars;
	BarData		*bars;
	int32		nnodes;
	NodeData	*node;
};

struct RouteData {
	int32	x;
	int32	y;
	int32	dirS;
	int32	dirD;
};

struct PathData {
	int32	x;
	int32	y;
	int32	dir;
	int32	num;
};

/*struct FrameInfos {
	int32 framesPerStep, framesPerChar;
	int32 standFrames;
	int32 slowInFrames, slowOutFrames;
	int32 turnFramesLeft, turnFramesRight;
	int32 walkFramesLeft, walkFramesRight;
	uint16 startX, startY, targetX, targetY, targetDir;
	int32 scaleA, scaleB;
};*/

#define ROUTE_END_FLAG 255
#define NO_DIRECTIONS 8
#define MAX_FRAMES_PER_CYCLE 16
#define MAX_FRAMES_PER_CHAR (MAX_FRAMES_PER_CYCLE * NO_DIRECTIONS)
#define O_ROUTE_SIZE 50

class ObjectMan;
class ResMan;
class SwordScreen;

extern int whatTarget(int32 startX, int32 startY, int32 destX, int32 destY);

class SwordRouter {
public:
	SwordRouter(ObjectMan *pObjMan, ResMan *pResMan);
	~SwordRouter(void);
	int32 routeFinder(int32 id, BsObject *mega, int32 x, int32 y, int32 dir);
	void setPlayerTarget(int32 x, int32 y, int32 dir, int32 stance);
	void resetExtraData(void);

	// these should be private but are read by SwordScreen for debugging:
	BarData   bars[O_GRID_SIZE+EXTRA_GRID_SIZE];
	NodeData  node[O_GRID_SIZE+EXTRA_GRID_SIZE];
	int32 nbars, nnodes;
private:
	// when the player collides with another mega, we'll receive a ReRouteRequest here.
	// that's why we need to remember the player's target coordinates
	int32 _playerTargetX, _playerTargetY, _playerTargetDir, _playerTargetStance;
	// additional route data to block parts of the floor and enable routine around megas:
	int32 _numExtraBars, _numExtraNodes;
	BarData _extraBars[EXTRA_GRID_SIZE];
	NodeData _extraNodes[EXTRA_GRID_SIZE];
	ObjectMan *_objMan;
	ResMan *_resMan;

	/*uint8 _nWalkFrames, _nTurnFrames;
	int32 _dx[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
	int32 _dy[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
	int32 _modX[NO_DIRECTIONS];
	int32 _modY[NO_DIRECTIONS];
	int32 _diagonalx, _diagonaly;*/

	int32 startX, startY, startDir;
	int32 targetX, targetY, targetDir;
	int32 scaleA, scaleB;
	int32 megaId;

	/*RouteData _route[O_ROUTE_SIZE];
	//int32 _routeLength;
	PathData  _smoothPath[O_ROUTE_SIZE];
	PathData  _modularPath[O_ROUTE_SIZE];*/
	RouteData			route[O_ROUTE_SIZE];
	PathData			smoothPath[O_ROUTE_SIZE];
	PathData			modularPath[O_ROUTE_SIZE];
	int32   			routeLength;

	int32		framesPerStep, framesPerChar;
	uint8		nWalkFrames, nTurnFrames;
	int32		_dx[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
	int32		_dy[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
	int32		modX[NO_DIRECTIONS];
	int32		modY[NO_DIRECTIONS];
	int32		diagonalx, diagonaly;
	int32		standFrames;
	int32		turnFramesLeft, turnFramesRight;
	int32		walkFramesLeft, walkFramesRight; // left/right walking turn
	int32		slowInFrames, slowOutFrames;


	int32 LoadWalkResources(BsObject *mega, int32 x, int32 y, int32 targetDir);
	int32 GetRoute(void);
	int32 CheckTarget(int32 x, int32 y);
	
	int32 Scan(int32 level);	
	int32 NewCheck(int32 status, int32 x1, int32 x2, int32 y1, int32 y2);
	int32 Check(int32 x1, int32 y1, int32 x2, int32 y2);
	int32 HorizCheck(int32 x1, int32 y, int32 x2);
	int32 VertCheck(int32 x, int32 y1, int32 y2);
	int32 LineCheck(int32 x1, int32 y1, int32 x2, int32 y2);

	void ExtractRoute();

	int32 SlidyPath();
	void SlidyWalkAnimator(WalkData *walkAnim);
	
	int32 SmoothestPath();
	int32 SmoothCheck(int32 best, int32 p, int32 dirS, int32 dirD);

	int32 SolidPath();
	int32 SolidWalkAnimator(WalkData *walkAnim);
	void RouteLine(int32 x1,int32 y1,int32 x2,int32 y2 ,int32 colour);
	void BresenhamLine(int32 x1,int32 y1,int32 x2,int32 y2, uint8 *screen, int32 width, int32 height, int32 colour);
};

#endif //BSROUTER_H
