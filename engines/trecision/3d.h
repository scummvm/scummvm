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

#ifndef TRECISION_3D_H
#define TRECISION_3D_H

#include "trecision/struct.h"
#include "common/serializer.h"

namespace Trecision {

struct SVVertex {
	int32 _x, _y, _z;
	int32 _angle;

	void clear() {
		_x = _y = _z = 0;
		_angle = 0;
	}
};

struct SSortPan {
	int _num;
	float _min;
};

struct SPathNode {
	float _x, _z;
	float _dist;
	int16 _oldPanel;
	int16 _curPanel;

	void clear() {
		_x = _z = 0.0f;
		_dist = 0.0f;
		_oldPanel = 0;
		_curPanel = 0;
	}
};

struct SPan {
	float _x1, _z1;
	float _x2, _z2;
	float _h;
	int _flags;
	int16 _nearPanel1;
	int16 _nearPanel2;
	int8 _col1;
	int8 _col2;

	void clear() {
		_x1 = _z1 = 0.0f;
		_x2 = _z2 = 0.0f;
		_h = 0.0f;
		_flags = 0;
		_nearPanel1 = _nearPanel2 = 0;
		_col1 = _col2 = 0;
	}
};

struct SStep {
	float _px, _pz;
	float _dx, _dz;
	float _theta;
	int _curAction;
	int _curFrame;
	int16 _curPanel;

	void clear() {
		_px = _pz = 0.0f;
		_dx = _dz = 0.0f;
		_theta = 0.0f;
		_curAction = 0;
		_curFrame = 0;
		_curPanel = 0;
	}
};
class TrecisionEngine;

class Renderer3D {
private:
	TrecisionEngine *_vm;

	int16 _minXClip;
	int16 _minYClip;
	int16 _maxXClip;
	int16 _maxYClip;

	int16 *_zBuffer;

	int16 _zBufStartX;
	int16 _zBufStartY;
	int16 _zBufWid;

	int16 _shadowLightNum;
	int16 _totalShadowVerts;
	uint8 _shadowIntens[10];

	SVVertex _vVertex[MAXVERTEX];
	SVertex _shVertex[MAXVERTEX];

	// data for the triangle routines
	int16 _lEdge[480];
	int16 _rEdge[480];
	uint8 _lColor[480];
	uint8 _rColor[480];
	int16 _lZ[480];
	int16 _rZ[480];
	uint16 _lTextX[480];
	uint16 _rTextX[480];
	uint16 _lTextY[480];
	uint16 _rTextY[480];

	void setZBufferRegion(int16 sx, int16 sy, int16 dx);
	int8 clockWise(int16 x1, int16 y1, int16 x2, int16 y2, int16 x3, int16 y3);
	void textureTriangle(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2, int32 x3, int32 y3, int32 z3, int32 c3, int32 tx3, int32 ty3, const STexture *t);
	void textureScanEdge(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2);
	void shadowTriangle(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 cv, int32 zv);
	void shadowScanEdge(int32 x1, int32 y1, int32 x2, int32 y2);
	void calcCharacterPoints();
	void drawCharacterFaces();

public:
	Renderer3D(TrecisionEngine *vm);
	~Renderer3D();

	void init3DRoom();
	void resetZBuffer(int x1, int y1, int x2, int y2);
	void setClipping(int16 x1, int16 y1, int16 x2, int16 y2);
	void drawCharacter(uint8 flag);
};

class PathFinding3D {
	TrecisionEngine *_vm;

	SPathNode _pathNode[MAXPATHNODES];
	float _invP[3][3];
	int _numPathNodes;
	float _x3d, _y3d, _z3d;
	float _curX, _curZ;
	float _lookX, _lookZ;
	int32 _panelNum;
	int16 _oldPanel;

	bool pointInside(int pan, float x, float z) const;
	void sortPanel();
	void pointOut();
	void invPointProject(int x, int y);
	bool intersectLinePanel(SPan *p, float x, float y, float z);
	bool intersectLineFloor(float x, float y, float z);
	bool intersectLineLine(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd);
	void findShortPath();
	float evalPath(int a, float destX, float destZ, int nearP);
	void lookAt(float x, float z);
	void buildFramelist();
	void displayPath();
	bool findAttachedPanel(int16 srcPanel, int16 destPanel);
	void sortPath();

public:
	PathFinding3D(TrecisionEngine *vm);
	~PathFinding3D();

	int _curStep;
	int _lastStep;
	int16 _curPanel;
	int _numSortPanel;

	int8 _characterGoToPosition;
	bool _characterInMovement;
	SSortPan _sortPan[32];
	SStep _step[MAXSTEP];
	SPan _panel[MAXPANELSINROOM];

	void findPath();
	void setPosition(int num);
	void goToPosition(int num);
	int nextStep();
	void initSortPan();
	void read3D(Common::SeekableReadStreamEndian *ff);
	void reset(uint16 idx, float px, float pz, float theta);
	void whereIs(int px, int py);
	void actorOrder();
	void syncGameStream(Common::Serializer &ser);
};

} // End of namespace Trecision
#endif

