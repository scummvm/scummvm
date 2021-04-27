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
 */
#ifndef TRECISION_3DINC_H
#define TRECISION_3DINC_H

#include "common/str.h"
#include "trecision/nl/3d/3drend.h"

/*--------------------------------------------------
Useful global variables:

 _panel        s : where the panels are
 _panelNum     i : number of panels
 _curPanel     i : currently affected panel
 _oldPanel     i : starting panel
 _curX         f : X position of the affected panel
 _curZ         f : Z position of the affected panel
 _cam          s : camera
 _proj         f : camera projection matrix
 _invP         f : camera antiprojection matrix
 _x3d,_y3d,_z3d f : float variables for output functions
 _x2d, _y2d    i : int variables for output functions
 _cx, _cy      i : center of the 2D screen
 _actor        s : actor structure
 _step         s : character frame structure
 _pathNode     s : path nodes
 _numPathNodes i : number of path nodes
 _curStep      i : current character frame
 _lastStep     i : last character frame
 _defActionLen u : character default action length
--------------------------------------------------*/

#define PI				3.1415927f
#define PI2				6.2831853f
#define EPSILON		    0.007f

#define MAXPANELSINROOM	400
#define PANELIN			0x20000000

#define MAXPATHNODES 	50

#define hSTAND			0
#define hSTART			1
#define hWALK 			2
#define hEND  			3
#define hSTOP0			4
#define hSTOP1			5
#define hSTOP2			6
#define hSTOP3			7
#define hSTOP4			8
#define hSTOP5			9
#define hSTOP6			10
#define hSTOP7			11
#define hSTOP8			12
#define hSTOP9			13
#define hWALKIN 		14
#define hBOH  			15
#define hUSEGG			16
#define hUSETT			17
#define hWALKOUT		18

#define hLAST			19  // Last Default Action

#define MAXSTEP	1000
#define FRAMECENTER(v) (-v[86]._z - v[164]._z) / 2.0

namespace Common {
class Serializer;
}

namespace Trecision {

struct SPan {
	float _x1, _z1;
	float _x2, _z2;
	float _h;
	int   _flags;
	char _near1;
	char _near2;
	char _col1;
	char _col2;
};

struct SSortPan {
	int   _num;
	float _min;
};

struct SPathNode {
	float _x, _z;
	float _dist;
	short _oldp;
	short _curp;
};

struct SStep {
	float _px, _pz;
	float _dx, _dz;
	float _theta;
	int   _curAction;
	int   _curFrame;
	short _curPanel;
};

class TrecisionEngine;


extern SPan      _panel[];
extern SSortPan  _sortPan[];
extern SPathNode _pathNode[];
extern SStep     _step[];

extern float  _proj[3][3];
extern float  _invP[3][3];

extern int    _panelNum;
extern int    _actorPos;
extern int    _forcedActorPos;

extern int    _cx, _cy;
extern int    _x2d, _y2d;

extern float  _x3d, _y3d, _z3d;

extern int    _curPanel, _oldPanel;
extern float  _curX, _curZ;

extern int    _numPathNodes;
extern int    _numSortPan;
extern int    _curStep;
extern int    _lastStep;

extern uint8  _defActionLen[];

int actionInRoom(int curA);
void actorDoAction(int action);
void actorStop();
void setPosition(int num);
void goToPosition(int num);
void lookAt(float x, float z);
void textureTriangle(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2, int32 x3, int32 y3, int32 z3, int32 c3, int32 tx3, int32 ty3, STexture *t);
void textureScanEdge(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2);
void shadowTriangle(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 cv, int32 zv);
void shadowScanEdge(int32 x1, int32 y1, int32 x2, int32 y2);
int8 clockWise(int16 x1, int16 y1, int16 x2, int16 y2, int16 x3, int16 y3);
int read3D(Common::String c);
void findPath();
void findShortPath();
float evalPath(int a, float destX, float destZ, int nearP);
void buildFramelist();
int nextStep();
void displayPath();
int findAttachedPanel(int srcP, int destP);
int pathCompare(const void *arg1, const void *arg2);
void sortPath();
float distF(float x1, float y1, float x2, float y2);
float dist3D(float x1, float y1, float z1, float x2, float y2, float z2);
void whereIs(int px, int py);
void pointOut();
void viewPanel(SPan *p);
void pointProject(float x, float y, float z);
void invPointProject(int x, int y);
bool intersectLinePanel(SPan *p, float x, float y, float z);
bool intersectLineFloor(float x, float y, float z);
bool intersectLineLine(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd);
void initSortPan();
int panCompare(const void *arg1, const void *arg2);
void sortPanel();
void actorOrder();

} // End of namespace Trecision

#endif
