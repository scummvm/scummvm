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

#define PI				3.1415927
#define PI2				6.2831853
#define EPSILON		    0.007

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

class SActor {
private:
	TrecisionEngine *_vm;

public:
	SActor(TrecisionEngine *vm);
	~SActor();

	SVertex *_characterArea; // TODO: Make it private

	SVertex  *_vertex;
	SFace    *_face;
	SLight   *_light;
	SCamera  *_camera;
	STexture *_texture;

	int _vertexNum;
	int _faceNum;
	int _lightNum;
	int _matNum;

	float _px, _py, _pz;
	float _dx, _dz;
	float _theta;
	int   _lim[6];

	int _curFrame;
	int _curAction;
};


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

extern bool   _shadowSplit;

} // End of namespace Trecision

#include "trecision/nl/3d/3dproto.h"

#endif
