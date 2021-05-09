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

#include "trecision/defines.h"
#include "trecision/trecision.h"
#include "trecision/nl/3d/3dinc.h"

namespace Trecision {
struct SVVertex {
	int32 _x, _y, _z;
	int32 _angle;
};

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

	uint16 *_curPage;

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
	void textureTriangle(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2, int32 x3, int32 y3, int32 z3, int32 c3, int32 tx3, int32 ty3, STexture *t);
	void textureScanEdge(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2);
	void shadowTriangle(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 cv, int32 zv);
	void shadowScanEdge(int32 x1, int32 y1, int32 x2, int32 y2);

public:
	Renderer3D(TrecisionEngine *vm);
	~Renderer3D();

	void init3DRoom(uint16 *destBuffer);
	void resetZBuffer(int x1, int y1, int x2, int y2);
	void setClipping(int16 x1, int16 y1, int16 x2, int16 y2);
	void drawCharacter(uint8 flag);
}; // end of class

class PathFinding3D {
private:
	TrecisionEngine *_vm;

	bool pointInside(int pan, float x, float z);

	SPathNode _pathNode[MAXPATHNODES];
	void sortPanel();
	void pointOut();
	void invPointProject(int x, int y);
	bool intersectLinePanel(SPan *p, float x, float y, float z);
	bool intersectLineFloor(float x, float y, float z);
	bool intersectLineLine(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd);

	float _invP[3][3];
	int _numPathNodes;

public:
	PathFinding3D(TrecisionEngine *vm);
	~PathFinding3D();

	float _lookX;
	float _lookZ;
	int _curStep;
	int _lastStep;
	int _panelNum;
	int _curPanel;
	int _oldPanel;
	int _numSortPan;

	int8 _characterGoToPosition;
	bool _characterInMovement;
	SSortPan _sortPan[32];
	SStep _step[MAXSTEP];
	SPan _panel[MAXPANELSINROOM];

	void findPath();
	void findShortPath();
	float evalPath(int a, float destX, float destZ, int nearP);
	void setPosition(int num);
	void goToPosition(int num);
	void lookAt(float x, float z);
	void buildFramelist();
	int nextStep();
	void displayPath();
	bool findAttachedPanel(int srcPanel, int destPanel);
//	int pathCompare(const void *arg1, const void *arg2);
	void sortPath();
	void initSortPan();
	void read3D(Common::SeekableReadStream *ff);
	void reset(uint16 idx, float px, float pz, float theta);
	void whereIs(int px, int py);
	void actorOrder();

}; // end of class

} // end of namespace
#endif

