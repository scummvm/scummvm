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

struct FrameInfos {
	int32 framesPerStep, framesPerChar;
	int32 standFrames;
	int32 slowInFrames, slowOutFrames;
	int32 turnFramesLeft, turnFramesRight;
	int32 walkFramesLeft, walkFramesRight;
	uint16 startX, startY, targetX, targetY, targetDir;
	int32 scaleA, scaleB;
};

#define ROUTE_END_FLAG 255
#define NO_DIRECTIONS 8
#define MAX_FRAMES_PER_CYCLE 16
#define MAX_FRAMES_PER_CHAR (MAX_FRAMES_PER_CYCLE * NO_DIRECTIONS)
#define O_ROUTE_SIZE 50

class ObjectMan;
class ResMan;

class SwordRouter {
public:
	SwordRouter(ObjectMan *pObjMan, ResMan *pResMan);
	~SwordRouter(void);
	int routeFinder(int32 id, BsObject *mega, int32 x, int32 y, int32 dir);
	int whatTarget(int32 startX, int32 startY, int32 destX, int32 destY);
	void setPlayerTarget(int32 x, int32 y, int32 dir, int32 stance);
	void resetExtraData(void);

	// these should be private but are read by SwordScreen for debugging:
	int32 _nBars, _nNodes;
	BarData _bars[O_GRID_SIZE + EXTRA_GRID_SIZE];
	NodeData _node[O_GRID_SIZE + EXTRA_GRID_SIZE];
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

	uint8 _nWalkFrames, _nTurnFrames;
	int32 _dx[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
	int32 _dy[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
	int32 _modX[NO_DIRECTIONS];
	int32 _modY[NO_DIRECTIONS];
	int32 _diagonalx, _diagonaly;

	RouteData _route[O_ROUTE_SIZE];
	//int32 _routeLength;
	PathData  _smoothPath[O_ROUTE_SIZE];
	PathData  _modularPath[O_ROUTE_SIZE];


	void loadWalkResources(int32 megaId, BsObject *mega, int32 x, int32 y, int32 targetDir);
	int getRoute(void);
	int checkTarget(int16 x, int16 y);
	
	int scan(int32 level);	
	int newCheck(int32 status, int16 x1, int16 x2, int16 y1, int16 y2);
	int check(int16 x1, int16 y1, int16 x2, int16 y2);
	int horizCheck(int16 x1, int16 y, int16 x2);
	int vertCheck(int16 x, int16 y1, int16 y2);
	int lineCheck(int16 x1, int16 y1, int16 x2, int16 y2);

	int32 extractRoute(int32 targetDir);

	void slidyPath(int32 scaleA, int32 scaleB, uint16 targetDir);
	void slidyWalkAnimator(WalkData *walkAnim, FrameInfos *frInfo, int32 megaId);
	
	int32 smoothestPath(uint16 startX, uint16 startY, uint16 startDir, int32 routeLength);
	int32 smoothCheck(int32 best, int32 p, int32 dirS, int32 dirD);

    int32 solidPath(int32 scaleA, int32 scaleB);
	int32 solidWalkAnimator(WalkData *walkAnim, FrameInfos *frInfo, int32 megaId);
};

#endif //BSROUTER_H
