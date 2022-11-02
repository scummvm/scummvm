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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "trecision/actor.h"
#include "trecision/animtype.h"
#include "trecision/pathfinding3d.h"
#include "trecision/sound.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {

PathFinding3D::PathFinding3D(TrecisionEngine *vm) : _vm(vm) {
	_lookX = 0.0f;
	_lookZ = 0.0f;
	_curStep = 0;
	_lastStep = 0;

	_characterInMovement = false;
	_characterGoToPosition = -1;

	_actorPos = 0;
	_forcedActorPos = 0;
	_panelNum = 0;
	_curPanel = -1;
	_oldPanel = -1;

	for (int i = 0; i < MAXPATHNODES; ++i)
		_pathNode[i].clear();

	_numPathNodes = 0;
	_numSortPanel = 0;
	_x3d = 0.0f;
	_y3d = 0.0f;
	_z3d = 0.0f;

	_curX = 0.0f;
	_curZ = 0.0f;

	for (int i = 0; i < 3; ++i) {
		_invP[i][0] = 0.0f;
		_invP[i][1] = 0.0f;
		_invP[i][2] = 0.0f;
	}

	for (int i = 0; i < MAXPANELSINROOM; ++i)
		_panel[i].clear();

	for (int i = 0; i < 32; ++i) {
		_sortPan[i]._num = 0;
		_sortPan[i]._min = 0.0f;
	}

	for (int i = 0; i < MAXSTEP; ++i)
		_step[i].clear();
}

PathFinding3D::~PathFinding3D() {
}

void PathFinding3D::findPath() {
	Actor *actor = _vm->_actor;
	actor->_px += actor->_dx;
	actor->_pz += actor->_dz;

	int inters = 0;
	_numPathNodes = 0;

	// if you have clicked behind the starting panel or the corner it's not possible to walk
	if (_curPanel < 0 && _oldPanel >= 0) {
		int16 b;
		// behind the starting panel
		if (pointInside(b = _oldPanel, _curX, _curZ) ||
			// behind the panel corner1
			((_vm->dist2D(_panel[_oldPanel]._x1, _panel[_oldPanel]._z1, actor->_px, actor->_pz) < EPSILON) &&
			 (pointInside(b = _panel[_oldPanel]._nearPanel1, _curX, _curZ) || pointInside(b = _panel[_oldPanel]._nearPanel2, _curX, _curZ))) ||
			// behind the panel corner2
			((_vm->dist2D(_panel[_oldPanel]._x2, _panel[_oldPanel]._z2, actor->_px, actor->_pz) < EPSILON) &&
			 (pointInside(b = _panel[_oldPanel]._nearPanel2, _curX, _curZ) || pointInside(b = _panel[_oldPanel]._nearPanel1, _curX, _curZ)))) {
			_curX = actor->_px;
			_curZ = actor->_pz;
			actor->_px -= actor->_dx;
			actor->_pz -= actor->_dz;
			_curPanel = b;
			_numPathNodes = 0;
			lookAt(_lookX, _lookZ);
			return;
		}
	}

	float dist = _vm->dist2D(actor->_px, actor->_pz, _curX, _curZ);

	for (int i = 0; i < _panelNum; ++i) {
		if (_panel[i]._flags & 0x80000000) { // it must be a wide panel
			if (intersectLineLine(_panel[i]._x1, _panel[i]._z1,
								  _panel[i]._x2, _panel[i]._z2,
								  actor->_px, actor->_pz, _curX, _curZ)) {
				++inters;

				_pathNode[_numPathNodes]._x = _x3d;
				_pathNode[_numPathNodes]._z = _z3d;
				_pathNode[_numPathNodes]._dist = _vm->dist2D(actor->_px, actor->_pz, _x3d, _z3d);
				_pathNode[_numPathNodes]._oldPanel = i;
				_pathNode[_numPathNodes]._curPanel = i;
				++_numPathNodes;

				// CORNERS - lever intersections in corners
				if (_oldPanel >= 0 && (i == _panel[_oldPanel]._nearPanel1 || i == _panel[_oldPanel]._nearPanel2)) {
					// otherwise if it's near the starting panel
					if ((_pathNode[_numPathNodes - 1]._dist < EPSILON) &&
						(i != _oldPanel) && (i != _curPanel)) {
						// and the distance is very small to the intersection
						--inters;
						--_numPathNodes;

						// If the click is inside the nearby panel
						if (_curPanel < 0 && pointInside(i, _curX, _curZ)) {
							_curX = actor->_px;
							_curZ = actor->_pz;
							actor->_px -= actor->_dx;
							actor->_pz -= actor->_dz;

							_curPanel = i;
							lookAt(_lookX, _lookZ);
							return;
						}
					}
				} else if (_curPanel >= 0 && (i == _panel[_curPanel]._nearPanel1 || i == _panel[_curPanel]._nearPanel2)) {
					// otherwise if it is near the finish panel
					if (ABS(_pathNode[_numPathNodes - 1]._dist - dist) < EPSILON && i != _oldPanel && i != _curPanel) {
						// and the distance is very small to the intersection
						--inters;
						--_numPathNodes;
					}
				}

			} else if (i == _oldPanel) {
				// always adds start and finish node only in on a panel
				++inters;

				_pathNode[_numPathNodes]._x = actor->_px;
				_pathNode[_numPathNodes]._z = actor->_pz;
				_pathNode[_numPathNodes]._dist = 0.0f;
				_pathNode[_numPathNodes]._oldPanel = _oldPanel;
				_pathNode[_numPathNodes]._curPanel = _oldPanel;
				++_numPathNodes;
			} else if (i == _curPanel) {
				++inters;

				_pathNode[_numPathNodes]._x = _curX;
				_pathNode[_numPathNodes]._z = _curZ;
				_pathNode[_numPathNodes]._dist = dist;
				_pathNode[_numPathNodes]._oldPanel = _curPanel;
				_pathNode[_numPathNodes]._curPanel = _curPanel;
				++_numPathNodes;
			}
		}
	}

	// the path is defined by:
	// start        _actor._px, _actor._pz
	// _numPathNodes _pathNode
	// end          _curX, _curZ

	// if it collides with any panel
	if (inters) {
		sortPath();

		// if odd and I go to the floor but I did not start from the panel
		// if it arrives on the floor and the last two nodes are not on the same block
		// if outside the last panel it moves the last node

		if (((inters & 1) && (_curPanel < 0) && (_oldPanel < 0)) ||
			(((inters - 1) & 1) && (_curPanel < 0) &&
			 (!findAttachedPanel(_pathNode[_numPathNodes - 2]._curPanel, _pathNode[_numPathNodes - 1]._curPanel) ||
			  pointInside(_pathNode[_numPathNodes - 1]._curPanel, _curX, _curZ)))) {

			_curPanel = _pathNode[_numPathNodes - 1]._curPanel;

			pointOut(); // remove the point found

			_pathNode[_numPathNodes]._x = _curX;
			_pathNode[_numPathNodes]._z = _curZ;
			_pathNode[_numPathNodes]._oldPanel = _curPanel;
			_pathNode[_numPathNodes]._curPanel = _curPanel;

			++_numPathNodes;
		}

		// if it arrives on the floor
		inters = 0;

		// Count the intersections with narrow panels
		// and with the union of large panels and small panels
		for (int i = 0; i < _panelNum; ++i) {
			if (!(_panel[i]._flags & 0x80000000)) {
				if (intersectLineLine(_panel[i]._x1, _panel[i]._z1,
									  _panel[i]._x2, _panel[i]._z2,
									  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
									  _curX, _curZ))
					++inters;
			} else {
				if (_panel[i]._col1 & 0x80) {
					if (intersectLineLine(_panel[i]._x1, _panel[i]._z1,
										  _panel[_panel[i]._col1 & 0x7F]._x2, _panel[_panel[i]._col1 & 0x7F]._z2,
										  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
										  _curX, _curZ)) {
						if (_vm->dist2D(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON &&
							_vm->dist2D(_x3d, _z3d, _curX, _curZ) > EPSILON)
							++inters;
					}
				} else if (intersectLineLine(_panel[i]._x1, _panel[i]._z1,
											 _panel[_panel[i]._col1 & 0x7F]._x1, _panel[_panel[i]._col1 & 0x7F]._z1,
											 _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
											 _curX, _curZ)) {
					if (_vm->dist2D(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON &&
						_vm->dist2D(_x3d, _z3d, _curX, _curZ) > EPSILON)
						++inters;
				}

				if (_panel[i]._col2 & 0x80) {
					if (intersectLineLine(_panel[i]._x2, _panel[i]._z2,
										  _panel[_panel[i]._col2 & 0x7F]._x2, _panel[_panel[i]._col2 & 0x7F]._z2,
										  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
										  _curX, _curZ)) {
						if (_vm->dist2D(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON &&
							_vm->dist2D(_x3d, _z3d, _curX, _curZ) > EPSILON)
							++inters;
					}
				} else if (intersectLineLine(_panel[i]._x2, _panel[i]._z2,
											 _panel[_panel[i]._col2 & 0x7F]._x1, _panel[_panel[i]._col2 & 0x7F]._z1,
											 _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
											 _curX, _curZ)) {
					if (_vm->dist2D(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON &&
						_vm->dist2D(_x3d, _z3d, _curX, _curZ) > EPSILON)
						++inters;
				}
			}

			if (inters)
				break;
		}

		// If in the last line there's an obstacle, remove the first node
		if (inters) {
			_curPanel = _pathNode[_numPathNodes - 1]._curPanel;

			pointOut(); // take out the point found

			_pathNode[_numPathNodes]._x = _curX;
			_pathNode[_numPathNodes]._z = _curZ;
			_pathNode[_numPathNodes]._oldPanel = _curPanel;
			_pathNode[_numPathNodes]._curPanel = _curPanel;

			++_numPathNodes;
		}

		_pathNode[_numPathNodes]._x = _curX;
		_pathNode[_numPathNodes]._z = _curZ;
		_pathNode[_numPathNodes]._dist = _vm->dist2D(actor->_px, actor->_pz, _curX, _curZ);
		_pathNode[_numPathNodes]._oldPanel = _curPanel;
		_pathNode[_numPathNodes]._curPanel = _curPanel;
		++_numPathNodes;

		findShortPath();
		displayPath();
	} else { // otherwise if it's direct
		_pathNode[_numPathNodes]._x = actor->_px;
		_pathNode[_numPathNodes]._z = actor->_pz;
		_pathNode[_numPathNodes]._dist = 0.0f;
		_pathNode[_numPathNodes]._oldPanel = _oldPanel;
		_pathNode[_numPathNodes]._curPanel = _oldPanel;
		++_numPathNodes;

		_pathNode[_numPathNodes]._x = _curX;
		_pathNode[_numPathNodes]._z = _curZ;
		_pathNode[_numPathNodes]._dist = _vm->dist2D(actor->_px, actor->_pz, _curX, _curZ);
		_pathNode[_numPathNodes]._oldPanel = _curPanel;
		_pathNode[_numPathNodes]._curPanel = _curPanel;
		++_numPathNodes;

		displayPath();
	}

	actor->_px -= actor->_dx;
	actor->_pz -= actor->_dz;
}
/**
 * Look for the shorter route avoiding obstacle
 */
void PathFinding3D::findShortPath() {
	SPathNode tempPath[MAXPATHNODES];
	float len1, len2;
	int curPanel, nearPanel, oldPanel;
	float destX, destZ;

	bool fail = false;

	int count = 0;
	// Add departure
	tempPath[count]._x = _vm->_actor->_px;
	tempPath[count]._z = _vm->_actor->_pz;
	tempPath[count]._dist = 0.0f;
	tempPath[count]._oldPanel = _oldPanel;
	tempPath[count]._curPanel = _oldPanel;
	++count;

	// for every obstacle, try to go around it by the right and the left
	// then take the sorter path
	for (int i = 0; i < _numPathNodes - 1; ++i) {
		memcpy(&tempPath[count], &_pathNode[i], sizeof(SPathNode));
		++count;
		if (count >= MAXPATHNODES - 2)
			count = MAXPATHNODES - 2;

		curPanel = _pathNode[i]._curPanel;

		// if source and destination panel are on the same block
		if (!findAttachedPanel(curPanel, _pathNode[i + 1]._curPanel))
			continue;

		// go around obstacle starting with _nearPanel1
		len1 = evalPath(i, _panel[curPanel]._x1, _panel[curPanel]._z1, _panel[curPanel]._nearPanel1) + _vm->dist2D(_pathNode[i]._x, _pathNode[i]._z, _panel[curPanel]._x1, _panel[curPanel]._z1);

		// go around obstacle starting with _nearPanel2
		len2 = evalPath(i, _panel[curPanel]._x2, _panel[curPanel]._z2, _panel[curPanel]._nearPanel2) + _vm->dist2D(_pathNode[i]._x, _pathNode[i]._z, _panel[curPanel]._x2, _panel[curPanel]._z2);

		// Check which route was shorter
		if ((len1 < 32000.0f) && (len2 < 32000.0f)) {
			if (len1 < len2) {
				destX = _panel[curPanel]._x1;
				destZ = _panel[curPanel]._z1;
				nearPanel = _panel[curPanel]._nearPanel1;
			} else {
				destX = _panel[curPanel]._x2;
				destZ = _panel[curPanel]._z2;
				nearPanel = _panel[curPanel]._nearPanel2;
			}

			float curX = _pathNode[i]._x;
			float curZ = _pathNode[i]._z;
			oldPanel = curPanel;

			int index = 0;

			// Save the shorter path
			for (;;) {
				tempPath[count]._x = curX;
				tempPath[count]._z = curZ;
				tempPath[count]._oldPanel = oldPanel;
				tempPath[count]._curPanel = curPanel;
				++count;
				if (count >= MAXPATHNODES - 2)
					count = MAXPATHNODES - 2;

				// if it reaches the point, exit the loop
				if (curPanel == _pathNode[i + 1]._curPanel) {
					memcpy(&tempPath[count], &_pathNode[i + 1], sizeof(SPathNode));
					++count;
					if (count >= MAXPATHNODES - 2)
						count = MAXPATHNODES - 2;
					break;
				}

				// If it's back to the starting panel, it didn't find a route
				if (((curPanel == _pathNode[i]._curPanel) && index) || (index > _panelNum)) {
					fail = true; // stop at the edge first
					break;    // and stop walking
				}

				// otherwise go to the next panel

				if (_panel[nearPanel]._nearPanel1 == curPanel) {
					// go to summit 2 next time
					curX = destX;
					curZ = destZ;

					destX = _panel[nearPanel]._x2;
					destZ = _panel[nearPanel]._z2;

					oldPanel = curPanel;
					curPanel = nearPanel;
					nearPanel = _panel[curPanel]._nearPanel2;
				} else {
					// go to summit 1 next time
					curX = destX;
					curZ = destZ;

					destX = _panel[nearPanel]._x1;
					destZ = _panel[nearPanel]._z1;

					oldPanel = curPanel;
					curPanel = nearPanel;
					nearPanel = _panel[curPanel]._nearPanel1;
				}

				++index;
			}
		} else {
			fail = true;
		}

		if (fail) // if it failed to go around the obstacle, stop
			break;
	}

	// adds arrival
	tempPath[count]._x = _curX;
	tempPath[count]._z = _curZ;
	tempPath[count]._dist = 0.0f;
	tempPath[count]._oldPanel = _curPanel;
	tempPath[count]._curPanel = _curPanel;
	++count;

	// after walking around all obstacles, optimize
	_numPathNodes = 0;
	for (int i = 0; i < count; ++i) {
		if (_numPathNodes > MAXPATHNODES - 2)
			_numPathNodes = MAXPATHNODES - 2;

		int j;
		// remove all the attached nodes
		for (j = count - 1; j >= i; --j) {
			if (_vm->dist2D(tempPath[j]._x, tempPath[j]._z, tempPath[i]._x, tempPath[i]._z) < EPSILON)
				break;
		}

		i = j;

		memcpy(&_pathNode[_numPathNodes], &tempPath[i], sizeof(SPathNode));
		++_numPathNodes;

		for (j = count - 1; j > i + 1; --j) {
			int inters = 0;
			for (int k = 0; k < _panelNum; ++k) {
				// it must never intersect the small panel
				if (!(_panel[k]._flags & 0x80000000)) {
					if (intersectLineLine(_panel[k]._x1, _panel[k]._z1,
										  _panel[k]._x2, _panel[k]._z2,
										  tempPath[i]._x, tempPath[i]._z,
										  tempPath[j]._x, tempPath[j]._z))
						++inters;

					if (_panel[k]._col1 & 0x80) {
						if (intersectLineLine(_panel[k]._x1, _panel[k]._z1,
											  _panel[_panel[k]._col1 & 0x7F]._x2, _panel[_panel[k]._col1 & 0x7F]._z2,
											  tempPath[i]._x, tempPath[i]._z,
											  tempPath[j]._x, tempPath[j]._z)) {
							len2 = _vm->dist2D(_x3d, _z3d, tempPath[i]._x, tempPath[i]._z);
							len1 = _vm->dist2D(_x3d, _z3d, tempPath[j]._x, tempPath[j]._z);

							// intersect at a point distant from the start and the finish
							if ((len1 > EPSILON) && (len2 > EPSILON))
								++inters;
						}
					} else if (intersectLineLine(_panel[k]._x1, _panel[k]._z1,
												 _panel[_panel[k]._col1 & 0x7F]._x1, _panel[_panel[k]._col1 & 0x7F]._z1,
												 tempPath[i]._x, tempPath[i]._z,
												 tempPath[j]._x, tempPath[j]._z)) {
						len2 = _vm->dist2D(_x3d, _z3d, tempPath[i]._x, tempPath[i]._z);
						len1 = _vm->dist2D(_x3d, _z3d, tempPath[j]._x, tempPath[j]._z);

						// intersect at a point distant from the start and the finish
						if ((len1 > EPSILON) && (len2 > EPSILON))
							++inters;
					}

					if (_panel[k]._col2 & 0x80) {
						if (intersectLineLine(_panel[k]._x2, _panel[k]._z2,
											  _panel[_panel[k]._col2 & 0x7F]._x2, _panel[_panel[k]._col2 & 0x7F]._z2,
											  tempPath[i]._x, tempPath[i]._z,
											  tempPath[j]._x, tempPath[j]._z)) {
							len2 = _vm->dist2D(_x3d, _z3d, tempPath[i]._x, tempPath[i]._z);
							len1 = _vm->dist2D(_x3d, _z3d, tempPath[j]._x, tempPath[j]._z);

							// intersect at a point distant from the start and the finish
							if ((len1 > EPSILON) && (len2 > EPSILON))
								++inters;
						}
					} else if (intersectLineLine(_panel[k]._x2, _panel[k]._z2,
												 _panel[_panel[k]._col2 & 0x7F]._x1, _panel[_panel[k]._col2 & 0x7F]._z1,
												 tempPath[i]._x, tempPath[i]._z,
												 tempPath[j]._x, tempPath[j]._z)) {
						len2 = _vm->dist2D(_x3d, _z3d, tempPath[i]._x, tempPath[i]._z);
						len1 = _vm->dist2D(_x3d, _z3d, tempPath[j]._x, tempPath[j]._z);

						// intersect at a point distant from the start and the finish
						if ((len1 > EPSILON) && (len2 > EPSILON))
							++inters;
					}

					if (inters)
						break;
				}
			}

			// if from A it's possible to reach B directly
			if (!inters) {
				curPanel = _pathNode[_numPathNodes - 1]._curPanel;
				oldPanel = tempPath[j]._oldPanel;

				int c;
				for (c = i; c <= j; ++c) {
					if ((tempPath[c]._oldPanel == curPanel) && (tempPath[c]._curPanel == oldPanel))
						break;
				}

				// if they weren't connected it means it went through the floor
				if (c > j) {
					_pathNode[_numPathNodes - 1]._curPanel = -1; // start
					tempPath[j]._oldPanel = -1;                  // destination
				}
				i = j - 1;
				break;
			}
		}
	}
}

/**
 *			Evaluate path length
 */
float PathFinding3D::evalPath(int a, float destX, float destZ, int nearP) {
	int index = 0;
	float len = 0.0f;

	int curPanel = _pathNode[a]._curPanel;
	float curX = _pathNode[a]._x;
	float curZ = _pathNode[a]._z;

	for (;;) {
		// if the point is reached, stop
		if (curPanel == _pathNode[a + 1]._curPanel) {
			len += _vm->dist2D(curX, curZ, _pathNode[a + 1]._x, _pathNode[a + 1]._z);
			break;
		}

		// if it's back to the starting plane, there's no route
		if (((curPanel == _pathNode[a]._curPanel) && index) || (index > _panelNum)) {
			len += 32000.0f; // Absurd length
			break;
		}

		// Otherwise it goes to the next plane

		// if nearP is attached to curp via vertex1
		if (_panel[nearP]._nearPanel1 == curPanel) {
			// go to vertex 2 next time
			len += _vm->dist2D(curX, curZ, destX, destZ);

			curX = destX;
			curZ = destZ;

			destX = _panel[nearP]._x2;
			destZ = _panel[nearP]._z2;

			curPanel = nearP;
			nearP = _panel[curPanel]._nearPanel2;
		} else {
			// go to vertex 1 newt time
			len += _vm->dist2D(curX, curZ, destX, destZ);

			curX = destX;
			curZ = destZ;

			destX = _panel[nearP]._x1;
			destZ = _panel[nearP]._z1;

			curPanel = nearP;
			nearP = _panel[curPanel]._nearPanel1;
		}

		++index;
	}

	return len;
}

/**
 *	Check if a point is inside a panel
 */
bool PathFinding3D::pointInside(int pan, float x, float z) const {
	if (pan < 0)
		return false;

	if (!(_panel[pan]._flags & 0x80000000))
		return true;

	double pgon[4][2];
	pgon[0][0] = (double)_panel[pan]._x1;
	pgon[0][1] = (double)_panel[pan]._z1;
	pgon[3][0] = (double)_panel[pan]._x2;
	pgon[3][1] = (double)_panel[pan]._z2;

	uint8 idx = _panel[pan]._col1 & 0x7F;
	if (_panel[pan]._col1 & 0x80) {
		pgon[1][0] = (double)_panel[idx]._x2;
		pgon[1][1] = (double)_panel[idx]._z2;
	} else {
		pgon[1][0] = (double)_panel[idx]._x1;
		pgon[1][1] = (double)_panel[idx]._z1;
	}

	idx = _panel[pan]._col2 & 0x7F;
	if (_panel[pan]._col2 & 0x80) {
		pgon[2][0] = (double)_panel[idx]._x2;
		pgon[2][1] = (double)_panel[idx]._z2;
	} else {
		pgon[2][0] = (double)_panel[idx]._x1;
		pgon[2][1] = (double)_panel[idx]._z1;
	}

	double ox = pgon[3][0] - pgon[0][0];
	double oz = pgon[3][1] - pgon[0][1];
	double s = sqrt(ox * ox + oz * oz);
	ox /= s;
	oz /= s;
	pgon[0][0] -= EPSILON * ox;
	pgon[0][1] -= EPSILON * oz;
	pgon[3][0] += EPSILON * ox;
	pgon[3][1] += EPSILON * oz;

	ox = pgon[2][0] - pgon[1][0];
	oz = pgon[2][1] - pgon[1][1];
	s = sqrt(ox * ox + oz * oz);
	ox /= s;
	oz /= s;
	pgon[1][0] -= EPSILON * ox;
	pgon[1][1] -= EPSILON * oz;
	pgon[2][0] += EPSILON * ox;
	pgon[2][1] += EPSILON * oz;

	// Crossing-Multiply algorithm
	double *vtx0 = pgon[3];
	// get test bit for above/below X axis
	bool yFlag0 = (vtx0[1] >= z);
	double *vtx1 = pgon[0];

	int counter = 0;
	for (int j = 5; --j;) {
		const bool yFlag1 = (vtx1[1] >= z);
		if (yFlag0 != yFlag1) {
			const bool xFlag0 = (vtx0[0] >= x);
			if ((xFlag0 == (vtx1[0] >= x)) && (xFlag0))
				counter += (yFlag0 ? -1 : 1);
			else if ((vtx1[0] - (vtx1[1] - z) * (vtx0[0] - vtx1[0]) / (vtx0[1] - vtx1[1])) >= x)
				counter += (yFlag0 ? -1 : 1);
		}

		// Move to the next pair of vertices, retaining info as possible.
		yFlag0 = yFlag1;
		vtx0 = vtx1;
		vtx1 += 2;
	}

	return (counter != 0);
}

void PathFinding3D::setPosition(int num) {
	SLight *curLight = _vm->_actor->_light;

	for (uint32 i = 0; i < _vm->_actor->_lightNum; ++i, ++curLight) {
		if (curLight->_inten != 0 || curLight->_position != num)
			continue;

		// If it's off and If it's the required position

		_vm->_actor->_px = curLight->_x;
		_vm->_actor->_pz = curLight->_z;
		_vm->_actor->_dx = 0.0f;
		_vm->_actor->_dz = 0.0f;

		float ox = curLight->_dx;
		float oz = curLight->_dz;

		// If it's a null light
		if (_vm->floatComp(ox, 0.0f) == 0 && _vm->floatComp(oz, 0.0f) == 0) // ox == 0.0f && oz == 0.0f
			warning("setPosition: Unknown error : null light");

		float t = sqrt(ox * ox + oz * oz);
		ox /= t;
		oz /= t;

		float theta = _vm->sinCosAngle(ox, oz) * 180.0f / PI;
		if (_vm->floatComp(theta, 360.0f) >= 0) // theta >= 360.0f
			theta -= 360.0f;
		if (_vm->floatComp(theta, 0.0f) == -1) // theta < 0.0f
			theta += 360.0f;

		_vm->_actor->_theta = theta;

		_curStep = 0;
		_lastStep = 0;
		_curPanel = -1;
		_oldPanel = -1;

		reset(0, _vm->_actor->_px + _vm->_actor->_dx, _vm->_actor->_pz + _vm->_actor->_dz, _vm->_actor->_theta);

		_characterGoToPosition = num;
		return;
	}
}

void PathFinding3D::goToPosition(int num) {
	SLight *curLight = _vm->_actor->_light;

	for (uint32 i = 0; i < _vm->_actor->_lightNum; ++i, ++curLight) {
		if (curLight->_inten != 0 || curLight->_position != num)
			continue;

		// If it's off and if it's the right position
		_curX = curLight->_x;
		_curZ = curLight->_z;
		_lookX = _curX - curLight->_dx;
		_lookZ = _curZ - curLight->_dz;

		_curStep = 0;
		_lastStep = 0;

		reset(0, _vm->_actor->_px + _vm->_actor->_dx, _vm->_actor->_pz + _vm->_actor->_dz, _vm->_actor->_theta);

		_oldPanel = _curPanel;
		_curPanel = -1;

		findPath();

		_characterGoToPosition = num;
		break;
	}
}

void PathFinding3D::lookAt(float x, float z) {
	float ox = _step[_lastStep]._px - x;
	float oz = _step[_lastStep]._pz - z;

	// If the light is null
	if (_vm->floatComp(ox, 0.0f) == 0 && _vm->floatComp(oz, 0.0f) == 0) {
		memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
		memcpy(&_step[_lastStep + 2], &_step[_lastStep + 1], sizeof(SStep));
		_lastStep += 2;

		return;
	}

	float t = sqrt(ox * ox + oz * oz);
	ox /= t;
	oz /= t;

	float theta = _vm->sinCosAngle(ox, oz) * 180.0f / PI;
	if (_vm->floatComp(theta, 360.0f) >= 0) //theta >= 360.0f
		theta -= 360.0f;
	if (_vm->floatComp(theta, 0.0f) == -1) // theta < 0.0f
		theta += 360.0f;

	float approx = theta - _step[_lastStep]._theta;

	if (_vm->floatComp(approx, 30.0f) == -1 && _vm->floatComp(approx, -30.0f) == 1) // approx < 30.0f && approx > -30.0f
		approx = 0.0f;
	else if (_vm->floatComp(approx, 180.0f) == 1) // approx > 180.0f
		approx = -360.0f + approx;
	else if (_vm->floatComp(approx, -180.0f) == -1) // approx < -180.0f
		approx = 360.0f + approx;

	approx /= 3.0f;

	// Antepenultimate 1/3
	_step[_lastStep]._theta += approx;
	_step[_lastStep]._theta = _vm->floatComp(_step[_lastStep]._theta, 360.0f) == 1 ? _step[_lastStep]._theta - 360.0f : _vm->floatComp(_step[_lastStep]._theta, 0.0f) == -1 ? _step[_lastStep]._theta + 360.0f : _step[_lastStep]._theta;

	// Penultimate 2/3
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
	++_lastStep;
	_step[_lastStep]._theta += approx;
	_step[_lastStep]._theta = _vm->floatComp(_step[_lastStep]._theta, 360.0f) == 1 ? _step[_lastStep]._theta - 360.0f : _vm->floatComp(_step[_lastStep]._theta, 0.0f) == -1 ? _step[_lastStep]._theta + 360.0f : _step[_lastStep]._theta;

	// Last right step
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
	++_lastStep;
	_step[_lastStep]._theta = theta;

	//	????
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
	++_lastStep;
	_step[_lastStep]._theta = theta;
}

/**
 *		Build list containing all the frames
 */
void PathFinding3D::buildFramelist() {
	// check that it never crosses or touches a narrow panel
	for (int i = 1; i < _numPathNodes; ++i) {
		for (int c = 0; c < _panelNum; ++c) {
			// it must never intersect narrow panel
			if (!(_panel[c]._flags & 0x80000000) && intersectLineLine(_panel[c]._x1, _panel[c]._z1,
																	  _panel[c]._x2, _panel[c]._z2,
																	  _pathNode[i - 1]._x, _pathNode[i - 1]._z,
																	  _pathNode[i]._x, _pathNode[i]._z)) {
				_numPathNodes = i;
				break;
			}
		}
	}

	float len = 0.0f;
	float curLen = 0.0f;

	float ox = _pathNode[0]._x;
	float oz = _pathNode[0]._z;

	for (int i = 1; i < _numPathNodes; ++i) {
		len += _vm->dist3D(_pathNode[i]._x, 0.0f, _pathNode[i]._z, ox, 0.0f, oz);

		ox = _pathNode[i]._x;
		oz = _pathNode[i]._z;
	}
	// total route length calculated - if too small, returns
	if (_vm->floatComp(len, 2.0f) == -1) {
		lookAt(_lookX, _lookZ);
		return;
	}

	int i = 0;
	// compute offset
	SVertex *v = _vm->_actor->_characterArea;
	float firstFrame = _vm->_actor->frameCenter(v);

	// if he was already walking
	int curAction, curFrame, cfp;
	if (_vm->_actor->_curAction == hWALK) {
		// compute current frame
		cfp = defActionLen[hSTART] + 1 + _vm->_actor->_curFrame;
		v += cfp * _vm->_actor->_vertexNum;

		curAction = hWALK;
		curFrame = _vm->_actor->_curFrame;

		// if it wasn't the last frame, take the next step
		if (_vm->_actor->_curFrame < defActionLen[hWALK] - 1) {
			++cfp;
			++curFrame;
			v += _vm->_actor->_vertexNum;
		}
	} else if ((_vm->_actor->_curAction >= hSTOP0) && (_vm->_actor->_curAction <= hSTOP9)) {
		// if he was stopped, starts moving again

		// compute current frame
		curAction = hWALK;
		curFrame = _vm->_actor->_curAction - hSTOP0;

		cfp = defActionLen[hSTART] + 1 + curFrame;
		v += cfp * _vm->_actor->_vertexNum;
	} else {
		// if he was standing, start working or turn
		oz = 0.0f;
		cfp = 1;

		curAction = hSTART;
		curFrame = 0;

		// start from the first frame
		v += _vm->_actor->_vertexNum;
	}
	oz = -_vm->_actor->frameCenter(v) + firstFrame;

	// at this point, CurA / _curAction is either hSTART or hWALK

	// until it arrives at the destination
	curLen = oz + _vm->_actor->frameCenter(v) - firstFrame;
	while (_vm->floatComp(curLen, len) == -1 || !i) {
		_step[i]._pz = oz - firstFrame; // where to render
		_step[i]._dz = curLen;          // where it is
		_step[i]._curAction = curAction;
		_step[i]._curFrame = curFrame;

		++i;
		v += _vm->_actor->_vertexNum;

		++curFrame;
		++cfp;

		if (curFrame >= defActionLen[curAction]) {
			if (curAction == hSTART) {
				curAction = hWALK;
				curFrame = 0;
				cfp = defActionLen[hSTART] + 1;

				ox = 0.0f;
			} else if (curAction == hWALK) {
				curAction = hWALK;
				curFrame = 0;
				cfp = defActionLen[hSTART] + 1;

				// end walk frame
				ox = _vm->_actor->frameCenter(v) - firstFrame;

				v = &_vm->_actor->_characterArea[cfp * _vm->_actor->_vertexNum];
				ox -= _vm->_actor->frameCenter(v);
			}

			v = &_vm->_actor->_characterArea[cfp * _vm->_actor->_vertexNum];

			// only if it doesn't end
			if (_vm->floatComp(oz + ox + _vm->_actor->frameCenter(v) - firstFrame, len) == -1) // oz + ox + _vm->_actor->frameCenter(v) - firstFrame < len
				oz += ox;
			else
				break;
		}
		curLen = oz + _vm->_actor->frameCenter(v) - firstFrame;
	}

	// After the destination, add the stop frame

	// if he was walking
	if (_step[i - 1]._curAction == hWALK)
		curAction = _step[i - 1]._curFrame + hSTOP0; // stop previous step.
	else
		curAction = hSTOP0; // stop step 01

	assert(curAction <= hLAST); // _defActionLen below has a size of hLAST + 1

	curFrame = 0;

	int index = 0;
	cfp = 0;
	while (index != curAction)
		cfp += defActionLen[index++];

	v = &_vm->_actor->_characterArea[cfp * _vm->_actor->_vertexNum];

	for (index = 0; index < defActionLen[curAction]; ++index) {
		curLen = oz + _vm->_actor->frameCenter(v) - firstFrame;
		_step[i]._pz = oz - firstFrame; // where to render
		_step[i]._dz = curLen;          // where it is
		_step[i]._curAction = curAction;
		_step[i]._curFrame = curFrame;

		++i;
		++curFrame;
		v += _vm->_actor->_vertexNum;
	}

	// how far is it from the destination?
	int divider = i - 2;
	if (divider == 0)
		// Safeguard, should never be useful... but if it is, it'll avoid a divide by 0 error.
		divider = 1;
	
	float approx = (len - curLen - EPSILON) / (float)divider;
	float theta = 0.0f;
	// Adjust all the steps so it arrives exactly where clicked
	for (index = 1; index < i; ++index) {
		// verify there's no reverse step
		if (_vm->floatComp(_step[index - 1]._dz, _step[index]._dz + approx * index) == 1 || _vm->floatComp(_step[index]._dz + approx * index + EPSILON, len) >= 0) {
			theta = _step[index]._dz - _step[index]._pz;
			_step[index]._dz = _step[index - 1]._dz;
			_step[index]._pz = _step[index]._dz - theta;
		} else {
			_step[index]._pz += (approx * index);
			_step[index]._dz += (approx * index);
		}
	}
	float cx = _step[index - 1]._dz;

	_lastStep = index; // last step
	_curStep = 0;  // current step

	// now insert exact directions and start and destination points
	index = 0;

	len = 0.0f;
	float startPos = 0.0f;
	for (i = 0; i < _numPathNodes - 1; ++i) {
		curLen = 0.0f;
		len += _vm->dist3D(_pathNode[i]._x, 0.0f, _pathNode[i]._z,
					  _pathNode[i + 1]._x, 0.0f, _pathNode[i + 1]._z);

		// determine the direction
		ox = _pathNode[i + 1]._x - _pathNode[i]._x;
		oz = _pathNode[i + 1]._z - _pathNode[i]._z;
		// if it's a useless node, remove it
		if (_vm->floatComp(ox, 0.0f) == 0 && _vm->floatComp(oz, 0.0f) == 0)
			continue;

		approx = sqrt(ox * ox + oz * oz);
		ox /= approx;
		oz /= approx;

		theta = _vm->sinCosAngle(ox, oz) * 180.0f / PI + 180.0f;
		if (_vm->floatComp(theta, 360.0f) >= 0)
			theta -= 360.0f;
		if (_vm->floatComp(theta, 0.0f) == -1)
			theta += 360.0f;

		while (index < _lastStep && _vm->floatComp(_step[index]._dz, len) <= 0) {
			curLen = _step[index]._dz - _step[index]._pz;

			_step[index]._px = _pathNode[i]._x + (_step[index]._pz - startPos) * ox;
			_step[index]._pz = _pathNode[i]._z + (_step[index]._pz - startPos) * oz;
			_step[index]._dx = curLen * ox;
			_step[index]._dz = curLen * oz;
			_step[index]._theta = theta;

			_step[index]._curPanel = _pathNode[i]._curPanel;

			++index;
		}
		startPos = len;
	}

	reset(index, _curX, _curZ, theta);

	_lastStep = index; // last step
	_curStep = 0;  // current step

	// starting angle
	float oldTheta = _vm->_actor->_theta;
	// first angle walk
	theta = _step[0]._theta;

	// if he starts from standstill position
	if ((_step[0]._curAction == hSTART) && (_step[0]._curFrame == 0) && (_lastStep > 4) && _vm->floatComp(_step[0]._theta, _step[1]._theta) == 0) {
		approx = theta - oldTheta;

		if (_vm->floatComp(approx, 180.0f) == 1)
			approx = -360.0f + approx;
		else if (_vm->floatComp(approx, -180.0f) == -1)
			approx = 360.0f + approx;

		approx /= 3.0f;

		for (index = 0; index < 2; ++index) {
			_step[index]._theta = oldTheta + (float)(index + 1) * approx;
			_step[index]._theta = _vm->floatComp(_step[index]._theta, 360.0f) == 1 ? _step[index]._theta - 360.0f : _vm->floatComp(_step[index]._theta, 0.0f) == -1 ? _step[index]._theta + 360.0f : _step[index]._theta;

			theta = _step[index]._theta;

			curLen = sqrt(_step[index]._dx * _step[index]._dx + _step[index]._dz * _step[index]._dz);

			theta = ((270.0f - theta) * PI) / 180.0f;
			ox = cos(theta) * curLen;
			oz = sin(theta) * curLen;

			cx = _step[index]._px + _step[index]._dx;
			float cz = _step[index]._pz + _step[index]._dz;

			_step[index]._px += _step[index]._dx - ox;
			_step[index]._pz += _step[index]._dz - oz;

			_step[index]._dx = cx - _step[index]._px;
			_step[index]._dz = cz - _step[index]._pz;
		}
	}

	// makes the curve
	oldTheta = _step[2]._theta;
	for (index = 3; index <= _lastStep; ++index) {
		theta = _step[index]._theta;

		// if it made a curve
		if (_vm->floatComp(oldTheta, theta) != 0) {
			approx = theta - oldTheta;

			if (_vm->floatComp(approx, 180.0f) == 1)
				approx = -360.0f + approx;
			else if (_vm->floatComp(approx, -180.0f) == -1)
				approx = 360.0f + approx;

			approx /= 3.0f;

			// for the previous one
			_step[index - 1]._theta += approx;
			_step[index - 1]._theta = _vm->floatComp(_step[index - 1]._theta, 360.0f) == 1 ? _step[index - 1]._theta - 360.0f : _vm->floatComp(_step[index - 1]._theta, 0.0f) == -1 ? _step[index - 1]._theta + 360.0f : _step[index - 1]._theta;

			oldTheta = _step[index - 1]._theta;

			curLen = sqrt(_step[index - 1]._dx * _step[index - 1]._dx + _step[index - 1]._dz * _step[index - 1]._dz);

			oldTheta = ((270.0f - oldTheta) * PI) / 180.0f;
			ox = cos(oldTheta) * curLen;
			oz = sin(oldTheta) * curLen;

			cx = _step[index - 1]._px + _step[index - 1]._dx;
			float cz = _step[index - 1]._pz + _step[index - 1]._dz;

			_step[index - 1]._px += _step[index - 1]._dx - ox;
			_step[index - 1]._pz += _step[index - 1]._dz - oz;

			_step[index - 1]._dx = cx - _step[index - 1]._px;
			_step[index - 1]._dz = cz - _step[index - 1]._pz;

			// for the next one
			_step[index]._theta -= approx;
			_step[index]._theta = _vm->floatComp(_step[index]._theta, 360.0f) == 1 ? _step[index]._theta - 360.0f : _vm->floatComp(_step[index]._theta, 0.0f) == -1 ? _step[index]._theta + 360.0f : _step[index]._theta;

			oldTheta = theta;
			theta = _step[index]._theta;

			curLen = sqrt(_step[index]._dx * _step[index]._dx + _step[index]._dz * _step[index]._dz);

			theta = ((270.0f - theta) * PI) / 180.0f;
			ox = cos(theta) * curLen;
			oz = sin(theta) * curLen;

			cx = _step[index]._px + _step[index]._dx;
			cz = _step[index]._pz + _step[index]._dz;

			_step[index]._px += _step[index]._dx - ox;
			_step[index]._pz += _step[index]._dz - oz;

			_step[index]._dx = cx - _step[index]._px;
			_step[index]._dz = cz - _step[index]._pz;

		} else
			oldTheta = theta;
	}

	lookAt(_lookX, _lookZ);
}

/**
 *			Take the next frame walk
 */
int PathFinding3D::nextStep() {
	Actor *actor = _vm->_actor;
	actor->_px = _step[_curStep]._px;
	actor->_pz = _step[_curStep]._pz;
	actor->_dx = _step[_curStep]._dx;
	actor->_dz = _step[_curStep]._dz;
	actor->_theta = _step[_curStep]._theta;
	actor->_curAction = _step[_curStep]._curAction;
	actor->_curFrame = _step[_curStep]._curFrame;
	_curPanel = _step[_curStep]._curPanel;

	// increase the current step if it's not the last frame
	if (_curStep < _lastStep) {
		++_curStep;
		return false;
	}

	if (_characterGoToPosition != -1)
		setPosition(_characterGoToPosition);

	return true;
}
/**
 *				View route
 */
void PathFinding3D::displayPath() {
	buildFramelist();
}

/**
 *	Check if two panels are in the same block
 */
bool PathFinding3D::findAttachedPanel(int16 srcPanel, int16 destPanel) {
	// if at least one is on the floor, return false
	if (srcPanel < 0 || destPanel < 0)
		return false;

	// if they are equal, return true
	if (srcPanel == destPanel)
		return true;

	int16 curPanel = srcPanel;
	int16 nearPanel = _panel[srcPanel]._nearPanel1;

	for (int i = 0;; ++i) {
		// if they are attached, return true
		if (curPanel == destPanel)
			return true;

		// if it has returned to the starting panel, return false
		if (srcPanel == curPanel && i)
			return false;

		if (i > _panelNum)
			return false;

		// if they are attached to vertex 1, take 2
		if (_panel[nearPanel]._nearPanel1 == curPanel) {
			curPanel = nearPanel;
			nearPanel = _panel[curPanel]._nearPanel2;
		} else {
			curPanel = nearPanel;
			nearPanel = _panel[curPanel]._nearPanel1;
		}
	}
}

/**
 *		Compare route distance (qsort)
 */
int pathCompare(const void *arg1, const void *arg2) {
	const SPathNode *p1 = (const SPathNode *)arg1;
	const SPathNode *p2 = (const SPathNode *)arg2;

	if (p1->_dist < p2->_dist)
		return -1;

	if (p1->_dist > p2->_dist)
		return 1;

	return 0;
}

/**
 *		sort the nodes of the path found
 */
void PathFinding3D::sortPath() {
	qsort(&_pathNode[0], _numPathNodes, sizeof(SPathNode), pathCompare);
}

/**
 *			Initializes sort panel
 */
void PathFinding3D::initSortPan() {
	_numSortPanel = 31;

	for (int i = 1; i < _numSortPanel - 1; ++i) {
		_sortPan[i]._min = 32000.0f;
		_sortPan[i]._num = i;
	}

	// First panel is behind everything and is not sorted
	_sortPan[0]._min = 30000.0f;
	_sortPan[0]._num = BOX_BACKGROUND;

	// Last panel is in front of everything and is not sorted
	_sortPan[30]._min = 0.0f;
	_sortPan[30]._num = BOX_FOREGROUND;

	Actor *actor = _vm->_actor;
	// Sort panel blocks by increasing distance from the camera
	for (int i = 0; i < _panelNum; ++i) {
		if (!(_panel[i]._flags & 0x80000000)) {
			float dist1 = _vm->dist3D(actor->_camera->_ex, 0.0, actor->_camera->_ez, _panel[i]._x1, 0.0, _panel[i]._z1);
			float dist2 = _vm->dist3D(actor->_camera->_ex, 0.0, actor->_camera->_ez, _panel[i]._x2, 0.0, _panel[i]._z2);

			float min = MIN(dist1, dist2);

			for (int j = 0; j < _numSortPanel; ++j) {
				if (_panel[i]._flags & (1 << j)) {
					if (_sortPan[j + 1]._min > min)
						_sortPan[j + 1]._min = min;
				}
			}
		}
	}

	sortPanel();

	for (int i = 0; i < _numSortPanel; ++i) {
		if (_sortPan[i]._num == BOX_BACKGROUND) {
			// now the panels go from 0 (foreground) to _numSortPanel (background)
			_numSortPanel = i;
			break;
		}
	}
}

void PathFinding3D::read3D(Common::SeekableReadStreamEndian *ff) {
	// read panels
	_panelNum = ff->readSint32();
	if (_panelNum > MAXPANELSINROOM)
		error("read3D(): Too many panels");

	for (int i = 0; i < _panelNum; ++i) {
		_panel[i]._x1 = ff->readFloat();
		_panel[i]._z1 = ff->readFloat();
		_panel[i]._x2 = ff->readFloat();
		_panel[i]._z2 = ff->readFloat();
		_panel[i]._h = ff->readFloat();
		_panel[i]._flags = ff->readUint32();

		// Note : Despite the panels are stored in an int16 with a MAXPANELSINROOM set to 400,
		// _panelNum is stored in a int32 and nearPanel1 and 2 were stored in an int8
		// in the data files. It's weird, but that's how the original game works so please
		// don't change that.
		_panel[i]._nearPanel1 = ff->readSByte();
		_panel[i]._nearPanel2 = ff->readSByte();
		_panel[i]._col1 = ff->readSByte();
		_panel[i]._col2 = ff->readSByte();
	}

	// projection matrix
	float _proj[3][3];
	SCamera *cam = _vm->_actor->_camera;
	_proj[0][0] = cam->_e1[0];
	_proj[0][1] = cam->_e1[1];
	_proj[0][2] = cam->_e1[2];
	_proj[1][0] = cam->_e2[0];
	_proj[1][1] = cam->_e2[1];
	_proj[1][2] = cam->_e2[2];
	_proj[2][0] = cam->_e3[0];
	_proj[2][1] = cam->_e3[1];
	_proj[2][2] = cam->_e3[2];

	// Compute 3x3 inverse matrix for 2D points on 3D
	float det = _proj[0][0] * _proj[1][1] * _proj[2][2] +
				_proj[0][1] * _proj[1][2] * _proj[2][0] +
				_proj[0][2] * _proj[1][0] * _proj[2][1] -
				_proj[2][0] * _proj[1][1] * _proj[0][2] -
				_proj[2][1] * _proj[1][2] * _proj[2][0] -
				_proj[2][2] * _proj[1][0] * _proj[2][1];

	if (_vm->floatComp(det, 0.0f) == 0)
		error("read3D : Unexpected data error while computing inverse matrix");

	_invP[0][0] = (_proj[1][1] * _proj[2][2] - _proj[1][2] * _proj[2][1]) / det;
	_invP[0][1] = (_proj[0][1] * _proj[2][2] - _proj[0][2] * _proj[2][1]) / (-det);
	_invP[0][2] = (_proj[0][1] * _proj[1][2] - _proj[0][2] * _proj[1][1]) / det;
	_invP[1][0] = (_proj[1][0] * _proj[2][2] - _proj[1][2] * _proj[2][0]) / (-det);
	_invP[1][1] = (_proj[0][0] * _proj[2][2] - _proj[0][2] * _proj[2][0]) / det;
	_invP[1][2] = (_proj[0][0] * _proj[1][2] - _proj[0][2] * _proj[1][0]) / (-det);
	_invP[2][0] = (_proj[1][0] * _proj[2][1] - _proj[1][1] * _proj[2][0]) / det;
	_invP[2][1] = (_proj[0][0] * _proj[2][1] - _proj[0][1] * _proj[2][0]) / (-det);
	_invP[2][2] = (_proj[0][0] * _proj[1][1] - _proj[0][1] * _proj[1][0]) / det;
}

void PathFinding3D::reset(uint16 idx,float px, float pz, float theta) {
	_step[idx]._px = px;
	_step[idx]._pz = pz;
	_step[idx]._dx = 0.0f;
	_step[idx]._dz = 0.0f;

	_step[idx]._theta = theta;
	_step[idx]._curAction = hSTAND;
	_step[idx]._curFrame = 0;
	_step[idx]._curPanel = _curPanel;
}

/**
 *		Compare panel distance (qsort)
 */
int panelCompare(const void *arg1, const void *arg2) {
	const SSortPan *p1 = (const SSortPan *)arg1;
	const SSortPan *p2 = (const SSortPan *)arg2;

	if (p1->_min > p2->_min)
		return 1;

	if (p1->_min < p2->_min)
		return -1;

	return 0;
}

/**
 *				Sort the panels
 */
void PathFinding3D::sortPanel() {
	qsort(&_sortPan[0], _numSortPanel, sizeof(SSortPan), panelCompare);
}

/**
 *  Find the 3D point corresponding to the 2D point
 */
void PathFinding3D::whereIs(int px, int py) {
	float inters = 32000.0f;

	_vm->_actor->_px += _vm->_actor->_dx;
	_vm->_actor->_pz += _vm->_actor->_dz;

	_oldPanel = _curPanel;
	_curPanel = -2;

	invPointProject(px, py);
	float x = _x3d;
	float y = _y3d;
	float z = _z3d;

	// Try to intersect with the floor
	if (intersectLineFloor(x, y, z)) {
		_curPanel = -1;
		_curX = _x3d;
		_curZ = _z3d;
	}

	// try all the panels and choose the closest one
	for (int i = 0; i < _panelNum; ++i) {
		if (intersectLinePanel(&_panel[i], x, y, z)) {
			float temp = _vm->dist3D(_vm->_actor->_camera->_ex, _vm->_actor->_camera->_ey, _vm->_actor->_camera->_ez, _x3d, _y3d, _z3d);

			if (_vm->floatComp(temp, inters) == -1) {
				inters = temp;
				_curPanel = i;
				_curX = _x3d;
				_curZ = _z3d;
			}
		}
	}

	_lookX = _curX;
	_lookZ = _curZ;

	pointOut();

	_vm->_actor->_px -= _vm->_actor->_dx;
	_vm->_actor->_pz -= _vm->_actor->_dz;
}

/**
 *  Brings out point from inner panel
 */
void PathFinding3D::pointOut() {
	const float largeValue = 60.0f; // 30 cm = 15 enlarge * 2

	float x = 0.0f, z = 0.0f;
	float inters = 32000.0f;

	// If I hit the floor, I have to count how many times
	// the straight line intersects with the wide panels
	if (_curPanel < 0)
		return;

	SPan *panel = &_panel[_curPanel];
	float nx = panel->_z1 - panel->_z2;
	float nz = panel->_x2 - panel->_x1;
	float temp = sqrt(nx * nx + nz * nz);
	nx /= temp;
	nz /= temp;

	// move the point on the wide panel
	for (int i = 0; i < _panelNum; ++i) {
		panel = &_panel[i];
		// Only check the external panels with the same flag
		if ((panel->_flags & 0x80000000) && (panel->_flags & (_panel[_curPanel]._flags & 0x7FFFFFFF))) {
			// check point 1
			temp = _vm->dist2D(_curX, _curZ, panel->_x1, panel->_z1);

			if (_vm->floatComp(temp, inters) == -1) {
				inters = temp;
				_curPanel = i;
				x = panel->_x1;
				z = panel->_z1;
			}

			// check point 2
			temp = _vm->dist2D(_curX, _curZ, panel->_x2, panel->_z2);

			if (_vm->floatComp(temp, inters) == -1) {
				inters = temp;
				_curPanel = i;
				x = panel->_x2;
				z = panel->_z2;
			}

			// check point a 1/3
			temp = _vm->dist2D(_curX, _curZ, (panel->_x1 * 2.0f + panel->_x2) / 3.0f, (panel->_z1 * 2.0f + panel->_z2) / 3.0f);

			if (_vm->floatComp(temp, inters) == -1) {
				inters = temp;
				_curPanel = i;
				x = (panel->_x1 * 2.0f + panel->_x2) / 3.0f;
				z = (panel->_z1 * 2.0f + panel->_z2) / 3.0f;
			}

			// check point a 2/3
			temp = _vm->dist2D(_curX, _curZ, (panel->_x1 + panel->_x2 * 2.0f) / 3.0f, (panel->_z1 + panel->_z2 * 2.0f) / 3.0f);

			if (_vm->floatComp(temp, inters) == -1) {
				inters = temp;
				_curPanel = i;
				x = (panel->_x1 + panel->_x2 * 2.0f) / 3.0f;
				z = (panel->_z1 + panel->_z2 * 2.0f) / 3.0f;
			}

			// check intersection with camera
			if (intersectLineLine(panel->_x1, panel->_z1, panel->_x2, panel->_z2, _vm->_actor->_camera->_ex, _vm->_actor->_camera->_ez, _curX, _curZ)) {
				temp = _vm->dist2D(_curX, _curZ, _x3d, _z3d);

				if (_vm->floatComp(temp, inters) == -1) {
					inters = temp;
					_curPanel = i;
					x = _x3d;
					z = _z3d;
				}
			}

			// check intersection with character
			if (intersectLineLine(panel->_x1, panel->_z1, panel->_x2, panel->_z2, _vm->_actor->_px, _vm->_actor->_pz, _curX, _curZ)) {
				temp = _vm->dist2D(_curX, _curZ, _x3d, _z3d);

				if (_vm->floatComp(temp, inters) == -1) {
					inters = temp;
					_curPanel = i;
					x = _x3d;
					z = _z3d;
				}
			}

			// check intersection with normal panel
			if (intersectLineLine(panel->_x1, panel->_z1, panel->_x2, panel->_z2,
								  _curX + nx * largeValue, _curZ + nz * largeValue, _curX - nx * largeValue, _curZ - nz * largeValue)) {
				temp = _vm->dist2D(_curX, _curZ, _x3d, _z3d);

				if (_vm->floatComp(temp, inters) == -1) {
					inters = temp;
					_curPanel = i;
					x = _x3d;
					z = _z3d;
				}
			}
		}
	}

	_curX = x;
	_curZ = z;
}

/**
 *		Projects 2D point in a 3D world
 */
void PathFinding3D::invPointProject(int x, int y) {
	float px = (float)(x - _vm->_cx) / _vm->_actor->_camera->_fovX;
	float py = (float)(y - _vm->_cy) / _vm->_actor->_camera->_fovY;

	_x3d = px * _invP[0][0] + py * _invP[0][1] + _invP[0][2];
	_y3d = px * _invP[1][0] + py * _invP[1][1] + _invP[1][2];
	_z3d = px * _invP[2][0] + py * _invP[2][1] + _invP[2][2];

	_x3d += _vm->_actor->_camera->_ex;
	_y3d += _vm->_actor->_camera->_ey;
	_z3d += _vm->_actor->_camera->_ez;
}

/**
 *		Intersects a 3D line with the panel
 */
bool PathFinding3D::intersectLinePanel(SPan *p, float x, float y, float z) {
	// If it's an enlarged panel
	if (p->_flags & 0x80000000)
		return false;

	float x1 = _vm->_actor->_camera->_ex;
	float y1 = _vm->_actor->_camera->_ey;
	float z1 = _vm->_actor->_camera->_ez;

	float dx = (x - x1);
	float dy = (y - y1);
	float dz = (z - z1);
	float t = sqrt(dx * dx + dy * dy + dz * dz);
	dx /= t;
	dy /= t;
	dz /= t;

	float nx = p->_z1 - p->_z2;
	float nz = p->_x2 - p->_x1;
	t = sqrt(nx * nx + nz * nz);
	nx /= t;
	nz /= t;
	// ny is always equal to zero for panels

	float n = nx * p->_x1 + nz * p->_z1 - nx * x - nz * z;
	float d = dx * nx + dz * nz;

	if (_vm->floatComp(d, 0.0f) != 0) {
		t = n / d;

		if (_vm->floatComp(t, 0.0f) <= 0)
			return false;

		_x3d = x1 + dx * t;
		_y3d = y1 + dy * t;
		_z3d = z1 + dz * t;

		float minX = MIN(p->_x1, p->_x2) - 1.5f;
		float maxX = MAX(p->_x1, p->_x2) + 1.5f;
		float minZ = MIN(p->_z1, p->_z2) - 1.5f;
		float maxZ = MAX(p->_z1, p->_z2) + 1.5f;

		// check if it fits inside the panel
		if (_vm->floatComp(_x3d, minX) >= 0 && _vm->floatComp(_x3d, maxX) <= 0 && _vm->floatComp(_y3d, 0.0) >= 0 && _vm->floatComp(_y3d, p->_h) <= 0 && _vm->floatComp(_z3d, minZ) >= 0 && _vm->floatComp(_z3d, maxZ) <= 0)
			return true;

		return false;
	}

	return false;
}

/**
 *		Intersects 3D line with the floor
 */
bool PathFinding3D::intersectLineFloor(float x, float y, float z) {
	float x1 = _vm->_actor->_camera->_ex;
	float y1 = _vm->_actor->_camera->_ey;
	float z1 = _vm->_actor->_camera->_ez;

	float dx = (x - x1);
	float dy = (y - y1);
	float dz = (z - z1);
	float t = sqrt(dx * dx + dy * dy + dz * dz);
	dx /= t;
	dy /= t;
	dz /= t;

	// ny is always equal to 1 for the floor

	if (_vm->floatComp(dy, 0.0f) != 0) {
		t = -y / dy;

		if (_vm->floatComp(t, 0.0f) <= 0)
			return false;

		_x3d = x1 + dx * t;
		_y3d = y1 + dy * t;
		_z3d = z1 + dz * t;

		return true;
	}

	return false;
}

/**
 *		Intersects a 2D line with a 2D line
 */
bool PathFinding3D::intersectLineLine(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd) {
	float divisor = (float)((xb - xa) * (yd - yc) - (yb - ya) * (xd - xc));
	if (_vm->floatComp(divisor, 0.0f) == 0)
		return false;

	float r = (float)((ya - yc) * (xd - xc) - (xa - xc) * (yd - yc)) / divisor;
	float s = (float)((ya - yc) * (xb - xa) - (xa - xc) * (yb - ya)) / divisor;
	if (_vm->floatComp(r, 0.0f) == -1 || _vm->floatComp(r, 1.0f) == 1 || _vm->floatComp(s, 0.0f) == -1 || _vm->floatComp(s, 1.0f) == 1)
		return false;

	_x3d = xa + r * (xb - xa);
	_y3d = 0.0f;
	_z3d = ya + r * (yb - ya);

	return true;
}

/**
 * Tells after which panel the character stands
 */
void PathFinding3D::actorOrder() {
	const float largeValue = 15.0f; // 30 cm (max)
	Actor *actor = _vm->_actor;

	if (_forcedActorPos != BOX_NORMAL) {
		_actorPos = _forcedActorPos;
		return;
	}

	float ox = actor->_px + actor->_dx - actor->_camera->_ex;
	float oz = actor->_pz + actor->_dz - actor->_camera->_ez;
	float dist = sqrt(ox * ox + oz * oz);
	float lx = (-oz / dist) * largeValue;
	float lz = (ox / dist) * largeValue;

	ox = actor->_px + actor->_dx;
	oz = actor->_pz + actor->_dz;

	// It must be copied in front of the nearest box
	_actorPos = _sortPan[1]._num;
	// from closest to farthest
	for (int i = 1; i < _numSortPanel; ++i) {
		for (int j = 0; j < _panelNum; ++j) {
			// If it's not wide and belongs to this level
			if (!(_panel[j]._flags & 0x80000000) && (_panel[j]._flags & (1 << (_sortPan[i]._num - 1)))) {
				// If it intersects the center of the character camera
				if (intersectLineLine(_panel[j]._x1, _panel[j]._z1, _panel[j]._x2, _panel[j]._z2, actor->_camera->_ex, actor->_camera->_ez, ox, oz) || intersectLineLine(_panel[j]._x1, _panel[j]._z1, _panel[j]._x2, _panel[j]._z2, actor->_camera->_ex, actor->_camera->_ez, ox + lx, oz + lz) || intersectLineLine(_panel[j]._x1, _panel[j]._z1, _panel[j]._x2, _panel[j]._z2, actor->_camera->_ex, actor->_camera->_ez, ox - lx, oz - lz)) {
					// If it intersects it must be copied after the next box
					_actorPos = _sortPan[i + 1]._num;
				}
			}
		}
	}
}

void PathFinding3D::syncGameStream(Common::Serializer &ser) {
	ser.syncAsSint32LE(_curPanel);
	ser.syncAsSint32LE(_oldPanel);
}

} // End of namespace Trecision
