/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRECISION_PATHFINDING_H
#define TRECISION_PATHFINDING_H

#include "trecision/struct.h"
#include "common/serializer.h"

namespace Trecision {

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
	int _actorPos;
	int _forcedActorPos;

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
	int getActorPos() const { return _actorPos; }
	void setForcedActorPos(int actorPos) { _forcedActorPos = actorPos; }
};

} // End of namespace Trecision
#endif

