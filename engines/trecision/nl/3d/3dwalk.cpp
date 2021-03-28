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

#include "common/util.h"
#include "common/scummsys.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/trecision.h"
#include "trecision/graphics.h"

namespace Trecision {

// locals
float _lookX, _lookZ;

/*-----------------06/11/95 15.20-------------------
					read3D
--------------------------------------------------*/
int read3D(Common::String c) {
	ff = FastFileOpen(c.c_str());
	if (ff == nullptr)
		CloseSys("Can't open 3D file!\n");

	// read rooms and lights
	FastFileRead(ff, _actor._camera, sizeof(SCamera));
	FastFileRead(ff, &_actor._lightNum, 4);
	FastFileRead(ff, _actor._light, sizeof(SLight) * _actor._lightNum);

	if (_actor._lightNum > 40)
		CloseSys("Too many lights");

	// read panels
	FastFileRead(ff, &_panelNum, 4);
	FastFileRead(ff, _panel, sizeof(SPan) * _panelNum);
	FastFileClose(ff);

	// projection matrix
	_proj[0][0] = _actor._camera->_e1[0];
	_proj[0][1] = _actor._camera->_e1[1];
	_proj[0][2] = _actor._camera->_e1[2];
	_proj[1][0] = _actor._camera->_e2[0];
	_proj[1][1] = _actor._camera->_e2[1];
	_proj[1][2] = _actor._camera->_e2[2];
	_proj[2][0] = _actor._camera->_e3[0];
	_proj[2][1] = _actor._camera->_e3[1];
	_proj[2][2] = _actor._camera->_e3[2];

	// Compute 3x3 inverse matrix for 2D points on 3D
	float det = _proj[0][0] * _proj[1][1] * _proj[2][2] +
				_proj[0][1] * _proj[1][2] * _proj[2][0] +
				_proj[0][2] * _proj[1][0] * _proj[2][1] -
				_proj[2][0] * _proj[1][1] * _proj[0][2] -
				_proj[2][1] * _proj[1][2] * _proj[2][0] -
				_proj[2][2] * _proj[1][0] * _proj[2][1];

	if (det == 0.0)
		CloseSys(g_vm->_sysText[5]);

	_invP[0][0] = (_proj[1][1] * _proj[2][2] - _proj[1][2] * _proj[2][1]) / det;
	_invP[0][1] = (_proj[0][1] * _proj[2][2] - _proj[0][2] * _proj[2][1]) / (-det);
	_invP[0][2] = (_proj[0][1] * _proj[1][2] - _proj[0][2] * _proj[1][1]) / det;
	_invP[1][0] = (_proj[1][0] * _proj[2][2] - _proj[1][2] * _proj[2][0]) / (-det);
	_invP[1][1] = (_proj[0][0] * _proj[2][2] - _proj[0][2] * _proj[2][0]) / det;
	_invP[1][2] = (_proj[0][0] * _proj[1][2] - _proj[0][2] * _proj[1][0]) / (-det);
	_invP[2][0] = (_proj[1][0] * _proj[2][1] - _proj[1][1] * _proj[2][0]) / det;
	_invP[2][1] = (_proj[0][0] * _proj[2][1] - _proj[0][1] * _proj[2][0]) / (-det);
	_invP[2][2] = (_proj[0][0] * _proj[1][1] - _proj[0][1] * _proj[1][0]) / det;

	_cx = 320;
	_cy = 240;

	initSortPan();

	init3DRoom(CurRoomMaxX, g_vm->_video2, g_vm->ZBuffer);
	setClipping(0, TOP, CurRoomMaxX, AREA + TOP);

	return 10L;
}

#define NOOLDINTERS		1
#define NOCURINTERS		2
#define OLDANGLESKIP	4
#define CURANGLESKIP	8
#define CLICKINTO		16
#define POINTOUT1		32
#define POINTOUT2		64

/*------------------------------------------------
				Create path
--------------------------------------------------*/
void findPath() {
	int b;

	_actor._px += _actor._dx;
	_actor._pz += _actor._dz;

	int inters = 0;
	_numPathNodes = 0;

	// if you have clicked behind the starting panel or the corner it's not possible to walk
	if ((_curPanel < 0) && (_oldPanel >= 0) &&
			// behind the starting panel
			((pointInside(b = _oldPanel, (double)_curX, (double)_curZ)) ||
			 // behind the panel corner1
			 ((distF(_panel[_oldPanel]._x1, _panel[_oldPanel]._z1, _actor._px, _actor._pz) < EPSILON) &&
			  (pointInside(b = _panel[_oldPanel]._near1, (double)_curX, (double)_curZ) ||
			   pointInside(b = _panel[_oldPanel]._near2, (double)_curX, (double)_curZ))) ||
			 // behind the panel corner2
			 ((distF(_panel[_oldPanel]._x2, _panel[_oldPanel]._z2, _actor._px, _actor._pz) < EPSILON) &&
			  (pointInside(b = _panel[_oldPanel]._near2, (double)_curX, (double)_curZ) ||
			   pointInside(b = _panel[_oldPanel]._near1, (double)_curX, (double)_curZ))))) {
		_curX = _actor._px;
		_curZ = _actor._pz;
		_actor._px -= _actor._dx;
		_actor._pz -= _actor._dz;
		_curPanel = b;
		_numPathNodes = 0;
		lookAt(_lookX, _lookZ);
		return ;
	}

	float dist = distF(_actor._px, _actor._pz, _curX, _curZ);

	for (b = 0; b < _panelNum; b++) {
		if (_panel[b]._flags & 0x80000000) {       // it must be a wide panel
			if (intersectLineLine(_panel[b]._x1, _panel[b]._z1,
								  _panel[b]._x2, _panel[b]._z2,
								  _actor._px, _actor._pz, _curX, _curZ)) {
				inters++;

				_pathNode[_numPathNodes]._x    = _x3d;
				_pathNode[_numPathNodes]._z    = _z3d;
				_pathNode[_numPathNodes]._dist = distF(_actor._px, _actor._pz, _x3d, _z3d);
				_pathNode[_numPathNodes]._oldp = b;
				_pathNode[_numPathNodes]._curp = b;
				_numPathNodes++;

				// CORNERS - lever intersections in corners
				if ((b == _panel[_oldPanel]._near1) || (b == _panel[_oldPanel]._near2)) {
					// otherwise if it's near the starting panel
					if ((_pathNode[_numPathNodes - 1]._dist < EPSILON) &&
							(b != _oldPanel) && (b != _curPanel)) {
						// and the distance is very small to the intersection
						inters--;
						_numPathNodes--;

						// If the click is inside the nearby panel
						if ((_curPanel < 0) && (pointInside(b, (double)_curX, (double)_curZ))) {
							_curX = _actor._px;
							_curZ = _actor._pz;
							_actor._px -= _actor._dx;
							_actor._pz -= _actor._dz;

							_curPanel = b;
							lookAt(_lookX, _lookZ);
							return ;
						}
					}
				} else if ((b == _panel[_curPanel]._near1) || (b == _panel[_curPanel]._near2)) {
					// otherwise if it is near the finish panel
					if ((fabs(_pathNode[_numPathNodes - 1]._dist - dist) < EPSILON) &&
							(b != _oldPanel) && (b != _curPanel)) {
						// and the distance is very small to the intersection
						inters--;
						_numPathNodes--;
					}
				}

			} else if (b == _oldPanel) {
				// always adds start and finish node only in on a panel
				inters++;

				_pathNode[_numPathNodes]._x    = _actor._px;
				_pathNode[_numPathNodes]._z    = _actor._pz;
				_pathNode[_numPathNodes]._dist = 0.0;
				_pathNode[_numPathNodes]._oldp = _oldPanel;
				_pathNode[_numPathNodes]._curp = _oldPanel;
				_numPathNodes++;
			} else if (b == _curPanel) {
				inters++;

				_pathNode[_numPathNodes]._x    = _curX;
				_pathNode[_numPathNodes]._z    = _curZ;
				_pathNode[_numPathNodes]._dist = dist;
				_pathNode[_numPathNodes]._oldp = _curPanel;
				_pathNode[_numPathNodes]._curp = _curPanel;
				_numPathNodes ++;
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
				((inters - 1 & 1) && (_curPanel < 0) &&
				 (!(findAttachedPanel(_pathNode[_numPathNodes - 2]._curp, _pathNode[_numPathNodes - 1]._curp)) ||
				  (pointInside(_pathNode[_numPathNodes - 1]._curp, (double)_curX, (double)_curZ))))) {

			_curPanel = _pathNode[_numPathNodes - 1]._curp;

			pointOut();   // remove the point found

			_pathNode[_numPathNodes]._x    = _curX;
			_pathNode[_numPathNodes]._z    = _curZ;
			_pathNode[_numPathNodes]._oldp = _curPanel;
			_pathNode[_numPathNodes]._curp = _curPanel;

			_numPathNodes++;
		}

		// if it arrives on the floor
		inters = 0;

		// Count the intersections with narrow panels
		// and with the union of large panels and small panels
		for (b = 0; b < _panelNum; b++) {
			if (!(_panel[b]._flags & 0x80000000)) {
				if (intersectLineLine(_panel[b]._x1, _panel[b]._z1,
									  _panel[b]._x2, _panel[b]._z2,
									  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
									  _curX, _curZ))
					inters++;
			} else {
				if (_panel[b]._col1 & 0x80) {
					if (intersectLineLine(_panel[b]._x1, _panel[b]._z1,
										  _panel[_panel[b]._col1 & 0x7F]._x2, _panel[_panel[b]._col1 & 0x7F]._z2,
										  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
										  _curX, _curZ))
						if ((distF(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON) &&
								(distF(_x3d, _z3d, _curX, _curZ) > EPSILON))
							inters++;
				} else {
					if (intersectLineLine(_panel[b]._x1, _panel[b]._z1,
										  _panel[_panel[b]._col1 & 0x7F]._x1, _panel[_panel[b]._col1 & 0x7F]._z1,
										  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
										  _curX, _curZ))
						if ((distF(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON) &&
								(distF(_x3d, _z3d, _curX, _curZ) > EPSILON))
							inters++;
				}

				if (_panel[b]._col2 & 0x80) {
					if (intersectLineLine(_panel[b]._x2, _panel[b]._z2,
										  _panel[_panel[b]._col2 & 0x7F]._x2, _panel[_panel[b]._col2 & 0x7F]._z2,
										  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
										  _curX, _curZ))
						if ((distF(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON) &&
								(distF(_x3d, _z3d, _curX, _curZ) > EPSILON))
							inters++;
				} else {
					if (intersectLineLine(_panel[b]._x2, _panel[b]._z2,
										  _panel[_panel[b]._col2 & 0x7F]._x1, _panel[_panel[b]._col2 & 0x7F]._z1,
										  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
										  _curX, _curZ))
						if ((distF(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON) &&
								(distF(_x3d, _z3d, _curX, _curZ) > EPSILON))
							inters++;
				}
			}

			if (inters)
				break;
		}

		// If in the last line there's an obstacle, remove the first node
		if (inters) {
			_curPanel = _pathNode[_numPathNodes - 1]._curp;

			pointOut();		// take out the point found

			_pathNode[_numPathNodes]._x    = _curX;
			_pathNode[_numPathNodes]._z    = _curZ;
			_pathNode[_numPathNodes]._oldp = _curPanel;
			_pathNode[_numPathNodes]._curp = _curPanel;

			_numPathNodes++;
		}

		_pathNode[_numPathNodes]._x    = _curX;
		_pathNode[_numPathNodes]._z    = _curZ;
		_pathNode[_numPathNodes]._dist = distF(_actor._px, _actor._pz, _curX, _curZ);
		_pathNode[_numPathNodes]._oldp = _curPanel;
		_pathNode[_numPathNodes]._curp = _curPanel;
		_numPathNodes ++;

		findShortPath();
		displayPath();
	} else {     // otherwise if it's direct
		_pathNode[_numPathNodes]._x    = _actor._px;
		_pathNode[_numPathNodes]._z    = _actor._pz;
		_pathNode[_numPathNodes]._dist = 0.0;
		_pathNode[_numPathNodes]._oldp = _oldPanel;
		_pathNode[_numPathNodes]._curp = _oldPanel;
		_numPathNodes++;

		_pathNode[_numPathNodes]._x    = _curX;
		_pathNode[_numPathNodes]._z    = _curZ;
		_pathNode[_numPathNodes]._dist = distF(_actor._px, _actor._pz, _curX, _curZ);
		_pathNode[_numPathNodes]._oldp = _curPanel;
		_pathNode[_numPathNodes]._curp = _curPanel;
		_numPathNodes++;

		displayPath();
	}

	_actor._px -= _actor._dx;
	_actor._pz -= _actor._dz;
}
/*------------------------------------------------
  Look for the shorter route avoiding obstacle
--------------------------------------------------*/
void findShortPath() {
	SPathNode TempPath[MAXPATHNODES];
	float  len1, len2;
	int    curp, nearp, oldp;
	float  destx, destz;
	signed int a, b, c, fail = 0;

	int count = 0;
	// Add departure
	TempPath[count]._x = _actor._px;
	TempPath[count]._z = _actor._pz;
	TempPath[count]._dist = 0.0;
	TempPath[count]._oldp = _oldPanel;
	TempPath[count]._curp = _oldPanel;
	count++;

	// for every obstacle, try to go around it by the right and the left
	// then take the sorter path
	for (a = 0; a < _numPathNodes - 1; a++) {
		memcpy(&TempPath[count], &_pathNode[a], sizeof(SPathNode));
		count++;
		if (count >= MAXPATHNODES - 2)
			count = MAXPATHNODES - 2;

		curp = _pathNode[a]._curp;

		// if source and destination panel are on the same block
		if (!(findAttachedPanel(curp, _pathNode[a + 1]._curp)))
			continue;

		// go around obstacle starting with _near1
		len1 = evalPath(a, _panel[curp]._x1, _panel[curp]._z1, _panel[curp]._near1) + distF(_pathNode[a]._x, _pathNode[a]._z, _panel[curp]._x1, _panel[curp]._z1);

		// go around obstacle starting with _near2
		len2 = evalPath(a, _panel[curp]._x2, _panel[curp]._z2, _panel[curp]._near2) + distF(_pathNode[a]._x, _pathNode[a]._z, _panel[curp]._x2, _panel[curp]._z2);

		// Check which route was shorter
		if ((len1 < 32000.0) && (len2 < 32000.0)) {
			if (len1 < len2) {
				destx = _panel[curp]._x1;
				destz = _panel[curp]._z1;
				nearp = _panel[curp]._near1;
			} else {
				destx = _panel[curp]._x2;
				destz = _panel[curp]._z2;
				nearp = _panel[curp]._near2;
			}

			float curx = _pathNode[a]._x;
			float curz = _pathNode[a]._z;
			oldp = curp;

			b = 0;

			// Save the shorter path
			for (;;) {
				TempPath[count]._x = curx;
				TempPath[count]._z = curz;
				TempPath[count]._oldp = oldp;
				TempPath[count]._curp = curp;
				count++;
				if (count >= MAXPATHNODES - 2)
					count = MAXPATHNODES - 2;

				// if it reaches the point, exit the loop
				if (curp == _pathNode[a + 1]._curp) {
					memcpy(&TempPath[count], &_pathNode[a + 1], sizeof(SPathNode));
					count++;
					if (count >= MAXPATHNODES - 2)
						count = MAXPATHNODES - 2;
					break;
				}

				// If it's back to the starting panel, it didn't find a route
				if (((curp == _pathNode[a]._curp) && b) || (b > _panelNum)) {
					fail = 1;   // stop at the edge first
					break;      // and stop walking
				}

				// otherwise go to the next panel

				if (_panel[nearp]._near1 == curp) {
					// go to summit 2 next time
					curx = destx;
					curz = destz;

					destx = _panel[nearp]._x2;
					destz = _panel[nearp]._z2;

					oldp  = curp;
					curp  = nearp;
					nearp = _panel[curp]._near2;
				} else {
					// go to summit 1 next time
					curx = destx;
					curz = destz;

					destx = _panel[nearp]._x1;
					destz = _panel[nearp]._z1;

					oldp  = curp;
					curp  = nearp;
					nearp = _panel[curp]._near1;
				}

				b++;
			}
		} else {
			fail = 1;
		}

		if (fail)   // if it failed to go around the obstacle, stop
			break;
	}

	// adds arrival
	TempPath[count]._x = _curX;
	TempPath[count]._z = _curZ;
	TempPath[count]._dist = 0.0;
	TempPath[count]._oldp = _curPanel;
	TempPath[count]._curp = _curPanel;
	count++;

	// after walking around all obstacles, optimize
	_numPathNodes = 0;
	for (a = 0; a < count; a++) {
		if (_numPathNodes > MAXPATHNODES - 2)
			_numPathNodes = MAXPATHNODES - 2;

		// remove all the attached nodes
		for (b = count - 1; b >= a; b--) {
			if (distF(TempPath[b]._x, TempPath[b]._z, TempPath[a]._x, TempPath[a]._z) < EPSILON)
				break;
		}

		a = b;

		memcpy(&_pathNode[_numPathNodes], &TempPath[a], sizeof(SPathNode));
		_numPathNodes++;

		for (b = count - 1; b > a + 1; b--) {
			int inters = 0;
			for (c = 0; c < _panelNum; c++) {
				// it must never intersect the small panel
				if (!(_panel[c]._flags & 0x80000000)) {
					if (intersectLineLine(_panel[c]._x1, _panel[c]._z1,
										  _panel[c]._x2, _panel[c]._z2,
										  TempPath[a]._x, TempPath[a]._z,
										  TempPath[b]._x, TempPath[b]._z))
						inters++;

					if (_panel[c]._col1 & 0x80) {
						if (intersectLineLine(_panel[c]._x1, _panel[c]._z1,
											  _panel[_panel[c]._col1 & 0x7F]._x2, _panel[_panel[c]._col1 & 0x7F]._z2,
											  TempPath[a]._x, TempPath[a]._z,
											  TempPath[b]._x, TempPath[b]._z)) {
							len2 = distF(_x3d, _z3d, TempPath[a]._x, TempPath[a]._z);
							len1 = distF(_x3d, _z3d, TempPath[b]._x, TempPath[b]._z);

							// intersect at a point distant from the start and the finish
							if ((len1 > EPSILON) && (len2 > EPSILON))
								inters++;
						}
					} else {
						if (intersectLineLine(_panel[c]._x1, _panel[c]._z1,
											  _panel[_panel[c]._col1 & 0x7F]._x1, _panel[_panel[c]._col1 & 0x7F]._z1,
											  TempPath[a]._x, TempPath[a]._z,
											  TempPath[b]._x, TempPath[b]._z)) {
							len2 = distF(_x3d, _z3d, TempPath[a]._x, TempPath[a]._z);
							len1 = distF(_x3d, _z3d, TempPath[b]._x, TempPath[b]._z);

							// intersect at a point distant from the start and the finish
							if ((len1 > EPSILON) && (len2 > EPSILON))
								inters++;
						}
					}

					if (_panel[c]._col2 & 0x80) {
						if (intersectLineLine(_panel[c]._x2, _panel[c]._z2,
											  _panel[_panel[c]._col2 & 0x7F]._x2, _panel[_panel[c]._col2 & 0x7F]._z2,
											  TempPath[a]._x, TempPath[a]._z,
											  TempPath[b]._x, TempPath[b]._z)) {
							len2 = distF(_x3d, _z3d, TempPath[a]._x, TempPath[a]._z);
							len1 = distF(_x3d, _z3d, TempPath[b]._x, TempPath[b]._z);

							// intersect at a point distant from the start and the finish
							if ((len1 > EPSILON) && (len2 > EPSILON))
								inters++;
						}
					} else {
						if (intersectLineLine(_panel[c]._x2, _panel[c]._z2,
											  _panel[_panel[c]._col2 & 0x7F]._x1, _panel[_panel[c]._col2 & 0x7F]._z1,
											  TempPath[a]._x, TempPath[a]._z,
											  TempPath[b]._x, TempPath[b]._z)) {
							len2 = distF(_x3d, _z3d, TempPath[a]._x, TempPath[a]._z);
							len1 = distF(_x3d, _z3d, TempPath[b]._x, TempPath[b]._z);

							// intersect at a point distant from the start and the finish
							if ((len1 > EPSILON) && (len2 > EPSILON))
								inters++;
						}
					}

					if (inters)
						break;
				}
			}

			// if from A it's possible to reach B directly
			if (!inters) {
				curp = _pathNode[_numPathNodes - 1]._curp;
				oldp = TempPath[b]._oldp;

				for (c = a; c <= b; c++) {
					if ((TempPath[c]._oldp == curp) && (TempPath[c]._curp == oldp))
						break;
				}

				// if they weren't connected it means it went through the floor
				if (c > b) {
					_pathNode[_numPathNodes - 1]._curp = -1; // start
					TempPath[b]._oldp = -1;                 // destination
				}
				a = b - 1;
				break;
			}
		}
	}
}

/*------------------------------------------------
			Evaluate path length
--------------------------------------------------*/
float evalPath(int a, float destX, float destZ, int nearP) {
	int b = 0;
	float len = 0.0;

	int curp = _pathNode[a]._curp;
	float curx = _pathNode[a]._x;
	float curz = _pathNode[a]._z;

	for (;;) {
		// if the point is reached, stop
		if (curp == _pathNode[a + 1]._curp) {
			len += distF(curx, curz, _pathNode[a + 1]._x, _pathNode[a + 1]._z);
			break;
		}

		// if it's back to the starting plane, there's no route
		if (((curp == _pathNode[a]._curp) && b) || (b > _panelNum)) {
			len += 32000.0;		// Absurd length
			break;
		}

		// Otherwise it goes to the next plane

		// if nearP is attached to curp via vertex1
		if (_panel[nearP]._near1 == curp) {
			// go to vertex 2 next time
			len += distF(curx, curz, destX, destZ);

			curx = destX;
			curz = destZ;

			destX = _panel[nearP]._x2;
			destZ = _panel[nearP]._z2;

			curp  = nearP;
			nearP = _panel[curp]._near2;
		} else {
			// go to vertex 1 newt time
			len += distF(curx, curz, destX, destZ);

			curx = destX;
			curz = destZ;

			destX = _panel[nearP]._x1;
			destZ = _panel[nearP]._z1;

			curp  = nearP;
			nearP = _panel[curp]._near1;
		}

		b++;
	}

	return (len);
}

/*------------------------------------------------
		Build list containing all the frames
--------------------------------------------------*/
void buildFramelist() {
	int a, b, c, CurA, CurF, cfp;
	float ox, oz, cx, cz;
	float startpos, approx, theta = 0.0, oldtheta, firstframe;
	SVertex *v;

	// controlla che in nessun caso attraversi o sfiori un pannello stretto
	for (a = 1; a < _numPathNodes; a++) {
		for (c = 0; c < _panelNum; c++) {
			// non deve intersecare pannello stretto mai
			if (!(_panel[c]._flags & 0x80000000)) {
				if (intersectLineLine(_panel[c]._x1, _panel[c]._z1,
									  _panel[c]._x2, _panel[c]._z2,
									  _pathNode[a - 1]._x, _pathNode[a - 1]._z,
									  _pathNode[a]._x, _pathNode[a]._z)) {
					_numPathNodes = a;
					break;
				}
			}
		}
	}

	float len    = 0.0;
	float curlen = 0.0;

	ox = _pathNode[0]._x;
	oz = _pathNode[0]._z;

	for (a = 1; a < _numPathNodes; a++) {
		len += dist3D(_pathNode[a]._x, 0.0, _pathNode[a]._z, ox, 0.0, oz);

		ox = _pathNode[a]._x;
		oz = _pathNode[a]._z;
	}
	// ha calcolato lunghezza totale percorso - se troppo piccola esce
	if (len < 2.0) {
		lookAt(_lookX, _lookZ);
		return;
	}

	a = 0;
	// calcola offset
	v = (SVertex *)_characterArea;
	firstframe = FRAMECENTER(v);
	startpos = 0.0;

	// se stava gia' camminando
	if (_actor._curAction == hWALK) {
		// calcola frame attuale
		cfp = _defActionLen[hSTART] + 1 + _actor._curFrame;
		v += cfp * _actor._vertexNum;

		CurA = hWALK;
		CurF = _actor._curFrame;

		// se non era all'ultimo frame fa il passo dodpo
		if (_actor._curFrame < _defActionLen[hWALK] - 1) {
			cfp ++;
			CurF ++;
			v += _actor._vertexNum;
		}
	}
	// se era in stop riparte
	else if ((_actor._curAction >= hSTOP0) && (_actor._curAction <= hSTOP9)) {
		// calcola frame attuale
		CurA = hWALK;
//o		CurF = _actor._curAction - hSTOP1;
		CurF = _actor._curAction - hSTOP0;

		cfp = _defActionLen[hSTART] + 1 + CurF;
		v += cfp * _actor._vertexNum;
	}
	// se era fermo, partiva o girava riparte da stand
	else {
		oz  = 0.0;
		cfp = 1;

		CurA = hSTART;
		CurF = 0;

		// parte dal primo frame
		v += _actor._vertexNum;
	}
	oz   =  - FRAMECENTER(v) + firstframe;

	// finche' non arrivo al punto destinazione
	while (((curlen = oz + FRAMECENTER(v) - firstframe) < len) || (!a)) {
		_step[a]._pz = oz - firstframe;		// dove renderizzare
		_step[a]._dz = curlen;			// dove si trova
		_step[a]._curAction = CurA;
		_step[a]._curFrame  = CurF;

		a ++;
		v += _actor._vertexNum;

		CurF++;
		cfp ++;

		if (CurF >= _defActionLen[CurA]) {
			if (CurA == hSTART) {
				CurA = hWALK;
				CurF = 0;
				cfp  = _defActionLen[hSTART] + 1;

				ox   = 0.0;
			} else if (CurA == hWALK) {
				CurA = hWALK;
				CurF = 0;
				cfp  = _defActionLen[hSTART] + 1;

				// frame fine camminata
				ox = FRAMECENTER(v) - firstframe;

				v = (SVertex *)_characterArea;
				v += cfp * _actor._vertexNum;
				ox -= FRAMECENTER(v);

			}

			v = (SVertex *)_characterArea;
			v += cfp * _actor._vertexNum;

			// solo se non finisce
			if ((oz + ox + FRAMECENTER(v) - firstframe) < len)
				oz += ox;
			else
				break;
		}
	}

	if (!a)
		warning("buildFramelist - Unknown error: step number = 0");

	// oltrepassata la destinazione aggiungo i frame di stop

	// se stava camminando
	if (_step[a - 1]._curAction == hWALK)
//o		CurA = _step[a-1]._curFrame + hSTOP1;		// stop passo prec.
		CurA = _step[a - 1]._curFrame + hSTOP0;		// stop passo prec.
	else
		CurA = hSTOP0;		   				// stop passo 01

	CurF  = 0;

	b = 0;
	cfp = 0;
	while (b != CurA)
		cfp += _defActionLen[b++];

	v = (SVertex *)_characterArea;
	v += cfp * _actor._vertexNum;

	for (b = 0; b < _defActionLen[CurA]; b++) {
		curlen = oz + FRAMECENTER(v) - firstframe;
		_step[a]._pz = oz - firstframe;		// dove renderizzare
		_step[a]._dz = curlen;			// dove si trova
		_step[a]._curAction = CurA;
		_step[a]._curFrame  = CurF;

		a ++;
		CurF ++;
		v += _actor._vertexNum;
	}

	// di quanto ha sbagliato?
	approx = (len - curlen - EPSILON) / (a - 2);
	// riaggiusta tutti i passi di modo che arrivi nel pto esatto cliccato
	for (b = 1; b < a; b++) {
		// controlla che non inverta passi
		if ((_step[b - 1]._dz > (_step[b]._dz + approx * b)) || ((_step[b]._dz + approx * b + EPSILON) >= len)) {
			theta = _step[b]._dz - _step[b]._pz;
			_step[b]._dz = _step[b - 1]._dz;
			_step[b]._pz = _step[b]._dz - theta;
		} else {
			_step[b]._pz += (approx * b);
			_step[b]._dz += (approx * b);
		}
	}
	cx = _step[b - 1]._dz;

	_lastStep = b;		// ultimo step
	_curStep  = 0;		// step attuale

	// ora inserisce direzioni e pto partenza arrivo esatti
	b = 0;
	//startpos = _step[0]._pz;

	len = 0.0;
	startpos = 0.0;
	oldtheta = -1.0;
	for (a = 0; a < _numPathNodes - 1; a++) {
		curlen = 0.0;
		len   += dist3D(_pathNode[a]._x, 0.0, _pathNode[a]._z,
						_pathNode[a + 1]._x, 0.0, _pathNode[a + 1]._z);

		// cerca direzione del tratto
		ox = _pathNode[a + 1]._x - _pathNode[a]._x;
		oz = _pathNode[a + 1]._z - _pathNode[a]._z;
		// se e' un nodo inutile lo elimino
		if ((ox == 0.0) && (oz == 0.0)) {
			continue;
		}

		approx = sqrt(ox * ox + oz * oz);
		ox /= approx;
		oz /= approx;

		theta = sinCosAngle(ox, oz) * 180.0 / PI + 180.0;
		if (theta >= 360.0)
			theta -= 360.0;
		if (theta <  0.0)
			theta += 360.0;

		while ((b < _lastStep) && (_step[b]._dz <= len)) {
			curlen = (_step[b]._dz - _step[b]._pz);

			_step[b]._px = _pathNode[a]._x + (_step[b]._pz - startpos) * ox;
			_step[b]._pz = _pathNode[a]._z + (_step[b]._pz - startpos) * oz;
			_step[b]._dx = curlen * ox;
			_step[b]._dz = curlen * oz;
			_step[b]._theta = theta;

			_step[b]._curPanel = _pathNode[a]._curp;

			b++;
		}
		oldtheta  = theta;
		startpos = len;
	}

	_step[b]._px = _curX;
	_step[b]._pz = _curZ;
	_step[b]._dx = 0;
	_step[b]._dz = 0;
	_step[b]._theta = theta;
	_step[b]._curAction = hSTAND;
	_step[b]._curFrame  = 0;
	_step[b]._curPanel  = _curPanel;

	_lastStep = b;		// ultimo step
	_curStep  = 0;		// step attuale

	// angolo di partenza
	oldtheta = _actor._theta;
	// primo angolo camminata
	theta    = _step[0]._theta;

	// se partiva da fermo
	if ((_step[0]._curAction == hSTART) && (_step[0]._curFrame == 0) && (_lastStep > 4) && (_step[0]._theta == _step[1]._theta)) {
		approx = theta - oldtheta;

		if (approx > 180.0)
			approx = -360.0 + approx;
		else if (approx < -180.0)
			approx = 360.0 + approx;

		approx /= 3.0;

		for (b = 0; b < 2; b++) {
			_step[b]._theta = oldtheta + (float)(b + 1) * approx;
			_step[b]._theta = (_step[b]._theta > 360.0) ? _step[b]._theta - 360.0 : (_step[b]._theta < 0.0) ? _step[b]._theta + 360.0 : _step[b]._theta;

			theta = _step[b]._theta;

			curlen = sqrt(_step[b]._dx * _step[b]._dx + _step[b]._dz * _step[b]._dz);

			theta = ((270.0 - theta) * PI) / 180.0;
			ox = cos(theta) * curlen;
			oz = sin(theta) * curlen;

			cx = _step[b]._px + _step[b]._dx;
			cz = _step[b]._pz + _step[b]._dz;

			_step[b]._px += _step[b]._dx - ox;
			_step[b]._pz += _step[b]._dz - oz;

			_step[b]._dx = cx - _step[b]._px;
			_step[b]._dz = cz - _step[b]._pz;
		}
	}

	// fa le curve
	oldtheta = _step[2]._theta;
	for (b = 3; b <= _lastStep; b++) {
		theta = _step[b]._theta;

		// se ha fatto una curva
		if (oldtheta != theta) {
			approx = theta - oldtheta;

			if (approx > 180.0)
				approx = -360.0 + approx;
			else if (approx < -180.0)
				approx = 360.0 + approx;

			approx /= 3.0;

			// per il precedente
			_step[b - 1]._theta += approx;
			_step[b - 1]._theta = (_step[b - 1]._theta > 360.0) ? _step[b - 1]._theta - 360.0 : (_step[b - 1]._theta < 0.0) ? _step[b - 1]._theta + 360.0 : _step[b - 1]._theta;

			oldtheta = _step[b - 1]._theta;
			startpos = oldtheta;

			curlen = sqrt(_step[b - 1]._dx * _step[b - 1]._dx + _step[b - 1]._dz * _step[b - 1]._dz);

			oldtheta = ((270.0 - oldtheta) * PI) / 180.0;
			ox = cos(oldtheta) * curlen;
			oz = sin(oldtheta) * curlen;

			cx = _step[b - 1]._px + _step[b - 1]._dx;
			cz = _step[b - 1]._pz + _step[b - 1]._dz;

			_step[b - 1]._px += _step[b - 1]._dx - ox;
			_step[b - 1]._pz += _step[b - 1]._dz - oz;

			_step[b - 1]._dx = cx - _step[b - 1]._px;
			_step[b - 1]._dz = cz - _step[b - 1]._pz;

			// per il seguente
			_step[b]._theta -= approx;
			_step[b]._theta = (_step[b]._theta > 360.0) ? _step[b]._theta - 360.0 : (_step[b]._theta < 0.0) ? _step[b]._theta + 360.0 : _step[b]._theta;

			oldtheta = theta;
			theta = _step[b]._theta;

			curlen = sqrt(_step[b]._dx * _step[b]._dx + _step[b]._dz * _step[b]._dz);

			theta = ((270.0 - theta) * PI) / 180.0;
			ox = cos(theta) * curlen;
			oz = sin(theta) * curlen;

			cx = _step[b]._px + _step[b]._dx;
			cz = _step[b]._pz + _step[b]._dz;

			_step[b]._px += _step[b]._dx - ox;
			_step[b]._pz += _step[b]._dz - oz;

			_step[b]._dx = cx - _step[b]._px;
			_step[b]._dz = cz - _step[b]._pz;

		} else
			oldtheta = theta;
	}

	lookAt(_lookX, _lookZ);
}

/*-----------------07/11/96 20.55-------------------
			Prende prossimo frame camminata
--------------------------------------------------*/
int nextStep() {
	_actor._px        = _step[_curStep]._px;
	_actor._pz        = _step[_curStep]._pz;
	_actor._dx        = _step[_curStep]._dx;
	_actor._dz        = _step[_curStep]._dz;
	_actor._theta     = _step[_curStep]._theta;
	_actor._curAction = _step[_curStep]._curAction;
	_actor._curFrame  = _step[_curStep]._curFrame;
	_curPanel      = _step[_curStep]._curPanel;
	// avanza solo se non e' ultimo frame
	if (_curStep < _lastStep) {
		_curStep ++;
		return false;
	} else {
		if (_characterGoToPosition != -1)
			setPosition(_characterGoToPosition);
		return true;
	}
}
/*-----------------15/10/96 11.42-------------------
				Visualizza percorso
--------------------------------------------------*/
void displayPath() {
//	int a;
//	int oldx,oldy;
//	float ox,oz;

	buildFramelist();

	/*	pointProject( _pathNode[0]._x, 0.0, _pathNode[0]._z );
		oldx = _x2d;
		oldy = _y2d;

		ox = _pathNode[0]._x;
		oz = _pathNode[0]._z;

		for ( a=1; a<_numPathNodes; a++ )
		{
			pointProject( _pathNode[a]._x, 0.0, _pathNode[a]._z );

			putLine( oldx, oldy, _x2d, _y2d, 0x4210 );

			putLine( (ox           -minx)*300/(maxx-minx)+640-200+a,
					 (oz           -minx)*300/(maxx-minx)+100,
					 (_pathNode[a]._x-minx)*300/(maxx-minx)+640-200+a,
					 (_pathNode[a]._z-minx)*300/(maxx-minx)+100, 0x4210 );

			oldx = _x2d;
			oldy = _y2d;

			ox = _pathNode[a]._x;
			oz = _pathNode[a]._z;
		}
	*/

	/*	for ( a=0; a<(_lastStep+1); a++ )
		{
			//cross3D( _step[a]._px+_step[a]._dx, 0.0, _step[a]._pz+_step[a]._dz, 0x4210 );
			cross3D( _step[a]._px, 0.0, _step[a]._pz, 0x7210 );
		}*/
//	_actor._px=_curX;
//	_actor._pz=_curZ;
}

/*-----------------16/10/96 11.07-------------------
	Guarda e 2 pannelli sono nello stesso blocco
--------------------------------------------------*/
int findAttachedPanel(int srcP, int destP) {
	int curp;
	int nearp;
	int b;

	// se almeno uno e' sul pavimento sul pavimento esci
	if ((srcP < 0) || (destP < 0))
		return (0);

	// se sono uguali torna 1
	if (srcP == destP)
		return (1);

	curp  = srcP;
	nearp = _panel[srcP]._near1;

	for (b = 0;; b++) {
		// se sono attaccati torna 1
		if (curp == destP)
			return (1);

		// se e' tornato al pannello di partenza torna 0
		if ((srcP == curp) && (b))
			return (0);

		if (b > _panelNum)
			return (0);

		// se sono attaccati al vertice 1 prende il 2
		if (_panel[nearp]._near1 == curp) {
			curp  = nearp;
			nearp = _panel[curp]._near2;
		} else {
			curp  = nearp;
			nearp = _panel[curp]._near1;
		}
	}
}

/*-----------------02/11/96 21.50-------------------
	Guarda se un pto e' all'interno di un pannello
--------------------------------------------------*/
bool pointInside(int pan, double x, double z) {
	int inside_flag;
	double pgon[4][2], ox, oz, s;

	if (pan < 0)
		return false;

	if (!(_panel[pan]._flags & 0x80000000))
		return true;

	pgon[0][0] = (double)_panel[pan]._x1;
	pgon[0][1] = (double)_panel[pan]._z1;
	pgon[3][0] = (double)_panel[pan]._x2;
	pgon[3][1] = (double)_panel[pan]._z2;

	if (_panel[pan]._col1 & 0x80) {
		pgon[1][0] = (double)_panel[_panel[pan]._col1 & 0x7F]._x2;
		pgon[1][1] = (double)_panel[_panel[pan]._col1 & 0x7F]._z2;
	} else {
		pgon[1][0] = (double)_panel[_panel[pan]._col1 & 0x7F]._x1;
		pgon[1][1] = (double)_panel[_panel[pan]._col1 & 0x7F]._z1;
	}

	if (_panel[pan]._col2 & 0x80) {
		pgon[2][0] = (double)_panel[_panel[pan]._col2 & 0x7F]._x2;
		pgon[2][1] = (double)_panel[_panel[pan]._col2 & 0x7F]._z2;
	} else {
		pgon[2][0] = (double)_panel[_panel[pan]._col2 & 0x7F]._x1;
		pgon[2][1] = (double)_panel[_panel[pan]._col2 & 0x7F]._z1;
	}

	ox = pgon[3][0] - pgon[0][0];
	oz = pgon[3][1] - pgon[0][1];
	s = sqrt(ox * ox + oz * oz);
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

//	Crossing-Multiply algorithm
	{
		register double *vtx0 = pgon[3];
		// get test bit for above/below X axis
		register bool yflag0 = (vtx0[1] >= z);
		register double *vtx1 = pgon[0];

		inside_flag = 0;
		for (register int j = 5; --j ;) {
			register bool yflag1 = (vtx1[1] >= z);
			if (yflag0 != yflag1) {
				register bool xflag0 = (vtx0[0] >= x);
				if ((xflag0 == (vtx1[0] >= x)) && (xflag0))
					inside_flag += (yflag0 ? -1 : 1);
				else if ((vtx1[0] - (vtx1[1] - z) * (vtx0[0] - vtx1[0]) / (vtx0[1] - vtx1[1])) >= x)
					inside_flag += (yflag0 ? -1 : 1);
			}

			// Move to the next pair of vertices, retaining info as possible.
			yflag0 = yflag1 ;
			vtx0 = vtx1 ;
			vtx1 += 2 ;
		}
	}
	return (inside_flag != 0) ;
}

/*------------------------------------------------
		Compare route distance (qsort)
--------------------------------------------------*/
int pathCompare(const void *arg1, const void *arg2) {
	SPathNode *p1 = (SPathNode *)arg1;
	SPathNode *p2 = (SPathNode *)arg2;

	if (p1->_dist < p2->_dist)
		return -1;

	if (p1->_dist > p2->_dist)
		return 1;

	return 0;
}

/*-----------------15/10/96 10.34-------------------
		Sorta i nodi del percorso trovato
--------------------------------------------------*/
void sortPath() {
	qsort(&_pathNode[0], _numPathNodes, sizeof(SPathNode), pathCompare);
}

/*------------------------------------------------
		Fake distance between two 2D points
--------------------------------------------------*/
float distF(float x1, float y1, float x2, float y2) {
	float d1 = fabs(x1 - x2);
	float d2 = fabs(y1 - y2);

	return sqrt(d1 * d1 + d2 * d2);
}

/*------------------------------------------------
			Distance between two 3D points
--------------------------------------------------*/
float dist3D(float x1, float y1, float z1, float x2, float y2, float z2) {
	return (sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2)));
}

/*-----------------06/11/95 20.45-------------------
					PutPixel
--------------------------------------------------*/
void putPix(int x, int y, uint16 c) {
	extern uint16 *ImagePointer;
	extern uint16 *SmackImagePointer;

	if ((x >  0) && (x < CurRoomMaxX) && (y > 60) && (y < 420)) {
		g_vm->_video2[x + CurRoomMaxX * y] = c;
		ImagePointer[x + CurRoomMaxX * (y - 60)] = c;
		SmackImagePointer[x + CurRoomMaxX * (y - 60)] = c;
	}
}

/*------------------------------------------------
  Find the 3D point corresponding to the 2D point
--------------------------------------------------*/
void whereIs(int px, int py) {
	float inters = 32000.0;

	_actor._px += _actor._dx;
	_actor._pz += _actor._dz;

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
	for (int b = 0; b < _panelNum; b++) {
		if (intersectLinePanel(&_panel[b], x, y, z)) {
			float temp = dist3D(_actor._camera->_ex, _actor._camera->_ey, _actor._camera->_ez, _x3d, _y3d, _z3d);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				_curX = _x3d;
				_curZ = _z3d;
			}
		}
	}

	inters = 32000.0;

	_lookX = _curX;
	_lookZ = _curZ;

	pointOut();

	_actor._px -= _actor._dx;
	_actor._pz -= _actor._dz;
}

/*------------------------------------------------
  Brings out point from inner panel
--------------------------------------------------*/
void pointOut() {
#define LARGEVAL	60.0	// 30 cm = 15 enlarge * 2

	float x = 0.0, z = 0.0;
	float inters = 32000.0;

	// TODO: ask an italian speaker the meaning of this comment :)
	// se ho beccato il pavimento devo contare quante volte interseca
	// i box larghi la retta omino pto
	if (_curPanel < 0)
		return;

	float nx = _panel[_curPanel]._z1 - _panel[_curPanel]._z2;
	float nz = _panel[_curPanel]._x2 - _panel[_curPanel]._x1;
	float temp = sqrt(nx * nx + nz * nz);
	nx /= temp;
	nz /= temp;

	// move the point on the wide panel
	for (int b = 0; b < _panelNum; b++) {
		// Only check the external panels with the same flag
		if ((_panel[b]._flags & 0x80000000) && (_panel[b]._flags & (_panel[_curPanel]._flags & 0x7FFFFFFF))) {
			// check point 1
			temp = distF(_curX, _curZ, _panel[b]._x1, _panel[b]._z1);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				x = _panel[b]._x1;
				z = _panel[b]._z1;
			}

			// check point 2
			temp = distF(_curX, _curZ, _panel[b]._x2, _panel[b]._z2);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				x = _panel[b]._x2;
				z = _panel[b]._z2;
			}

			// check point a 1/3
			temp = distF(_curX, _curZ, (_panel[b]._x1 * 2.0 + _panel[b]._x2) / 3.0, (_panel[b]._z1 * 2.0 + _panel[b]._z2) / 3.0);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				x = (_panel[b]._x1 * 2.0 + _panel[b]._x2) / 3.0;
				z = (_panel[b]._z1 * 2.0 + _panel[b]._z2) / 3.0;
			}

			// check point a 2/3
			temp = distF(_curX, _curZ, (_panel[b]._x1 + _panel[b]._x2 * 2.0) / 3.0, (_panel[b]._z1 + _panel[b]._z2 * 2.0) / 3.0);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				x = (_panel[b]._x1 + _panel[b]._x2 * 2.0) / 3.0;
				z = (_panel[b]._z1 + _panel[b]._z2 * 2.0) / 3.0;
			}

			// check intersection with camera
			if (intersectLineLine(_panel[b]._x1, _panel[b]._z1, _panel[b]._x2, _panel[b]._z2, _actor._camera->_ex, _actor._camera->_ez, _curX, _curZ)) {
				temp = distF(_curX, _curZ, _x3d, _z3d);

				if (temp < inters) {
					inters = temp;
					_curPanel = b;
					x = _x3d;
					z = _z3d;
				}
			}

			// check intersection with character
			if (intersectLineLine(_panel[b]._x1, _panel[b]._z1,
								  _panel[b]._x2, _panel[b]._z2,
								  _actor._px, _actor._pz, _curX, _curZ)) {
				temp = distF(_curX, _curZ, _x3d, _z3d);

				if (temp < inters) {
					inters = temp;
					_curPanel = b;
					x = _x3d;
					z = _z3d;
				}
			}

			// check intersection with normal panel
			if (intersectLineLine(_panel[b]._x1, _panel[b]._z1, _panel[b]._x2, _panel[b]._z2,
								  _curX + nx * LARGEVAL, _curZ + nz * LARGEVAL, _curX - nx * LARGEVAL, _curZ - nz * LARGEVAL)) {
				temp = distF(_curX, _curZ, _x3d, _z3d);

				if (temp < inters) {
					inters = temp;
					_curPanel = b;
					x = _x3d;
					z = _z3d;
				}
			}
		}
	}

	_curX = x;
	_curZ = z;

#undef LARGEVAL
}

/*------------------------------------------------
	Draw 2D line
--------------------------------------------------*/
void putLine(int x1, int y1, int x2, int y2, uint16 color) {
	int deltax = x2 - x1;
	if (deltax < 0)
		deltax = -deltax;

	int deltay = y2 - y1;
	if (deltay < 0)
		deltay = -deltay;

	int x = x1;
	int y = y1;

	int incX = (x1 < x2) ? 1 : -1;
	int incY = (y1 < y2) ? 1 : -1;

	if (deltax < deltay) {
		int d = (deltax << 1) - deltay;
		int delta_constant = (deltax - deltay) << 1;
		int numCycles = deltay + 1;

		for (int cycle = 0; cycle < numCycles; cycle++) {
			if ((x >= 0) && (x < MAXX) && (y >= 0) && (y < MAXY))
				putPix(x, y, color);
			
			if (d < 0)
				d += (deltax << 1);
			else {
				d += delta_constant;
				x += incX;
			}
			y += incY;
		}
	} else {
		int d = (deltay << 1) - deltax;
		int delta_constant = (deltay - deltax) << 1;
		int numCycles = deltax + 1;

		for (int cycle = 0; cycle < numCycles; cycle++) {
			if ((x >= 0) && (x < MAXX) && (y >= 0) && (y < MAXY))
				putPix(x, y, color);
			if (d < 0)
				d += (deltay << 1);
			else {
				d += delta_constant;
				y += incY;
			}
			x += incX;
		}
	}
}

/*------------------------------------------------
				View Panel
--------------------------------------------------*/
void viewPanel(SPan *p) {
	int projVerts[4][2];

	uint16 col = (p->_flags & 0x80000000) ? 0x3C0 : 0x3FF;
	if (p->_flags & 0x80000000) {
		pointProject(p->_x1, 0.0, p->_z1);
		projVerts[0][0] = _x2d;
		projVerts[0][1] = _y2d;

		pointProject(p->_x2, 0.0, p->_z2);
		projVerts[1][0] = _x2d;
		projVerts[1][1] = _y2d;

		pointProject((p->_col1 & 0x80) ? _panel[p->_col1 & 0x7F]._x2 : _panel[p->_col1 & 0x7F]._x1, 0.0,
					 (p->_col1 & 0x80) ? _panel[p->_col1 & 0x7F]._z2 : _panel[p->_col1 & 0x7F]._z1);
		projVerts[2][0] = _x2d;
		projVerts[2][1] = _y2d;

		pointProject((p->_col2 & 0x80) ? _panel[p->_col2 & 0x7F]._x2 : _panel[p->_col2 & 0x7F]._x1, 0.0,
					 (p->_col2 & 0x80) ? _panel[p->_col2 & 0x7F]._z2 : _panel[p->_col2 & 0x7F]._z1);
		projVerts[3][0] = _x2d;
		projVerts[3][1] = _y2d;

		putLine(projVerts[0][0], projVerts[0][1], projVerts[2][0], projVerts[2][1], 0x1C1);
		putLine(projVerts[1][0], projVerts[1][1], projVerts[3][0], projVerts[3][1], 0x1C1);
	}

	if (p->_flags & (1 << 28))
		col = g_vm->_graphicsMgr->palTo16bit(233, 238, 21);

	pointProject(p->_x1, 0.0, p->_z1);
	projVerts[0][0] = _x2d;
	projVerts[0][1] = _y2d;
	pointProject(p->_x1, p->_h, p->_z1);
	projVerts[1][0] = _x2d;
	projVerts[1][1] = _y2d;

	pointProject(p->_x2, 0.0, p->_z2);
	projVerts[2][0] = _x2d;
	projVerts[2][1] = _y2d;
	pointProject(p->_x2, p->_h, p->_z2);
	projVerts[3][0] = _x2d;
	projVerts[3][1] = _y2d;

	// H1
	putLine(projVerts[0][0], projVerts[0][1], projVerts[1][0], projVerts[1][1], col);
	// H2
	putLine(projVerts[2][0], projVerts[2][1], projVerts[3][0], projVerts[3][1], col);
	// B
	putLine(projVerts[0][0], projVerts[0][1], projVerts[2][0], projVerts[2][1], col);
	// T
	putLine(projVerts[1][0], projVerts[1][1], projVerts[3][0], projVerts[3][1], col);
}

/*------------------------------------------------
		Projects 3D point on 2D screen
--------------------------------------------------*/
void pointProject(float x, float y, float z) {
	float pa0 = _actor._camera->_ex - x;
	float pa1 = _actor._camera->_ey - y;
	float pa2 = _actor._camera->_ez - z;

	float p0 = pa0 * _proj[0][0] + pa1 * _proj[0][1] + pa2 * _proj[0][2];
	float p1 = pa0 * _proj[1][0] + pa1 * _proj[1][1] + pa2 * _proj[1][2];
	float p2 = pa0 * _proj[2][0] + pa1 * _proj[2][1] + pa2 * _proj[2][2];

	_x2d = _cx + (int)((p0 * _actor._camera->_fovX) / p2);
	_y2d = _cy + (int)((p1 * _actor._camera->_fovY) / p2);
}

/*------------------------------------------------
		Projects 2D point in a 3D world
--------------------------------------------------*/
void invPointProject(int x, int y) {
	float px = (float)(x - _cx) / _actor._camera->_fovX;
	float py = (float)(y - _cy) / _actor._camera->_fovY;

	_x3d = (float)(px * _invP[0][0] + py * _invP[0][1] + _invP[0][2]);
	_y3d = (float)(px * _invP[1][0] + py * _invP[1][1] + _invP[1][2]);
	_z3d = (float)(px * _invP[2][0] + py * _invP[2][1] + _invP[2][2]);

	_x3d += _actor._camera->_ex;
	_y3d += _actor._camera->_ey;
	_z3d += _actor._camera->_ez;
}

/*------------------------------------------------
		Intersects a 3D line with the panel
--------------------------------------------------*/
int intersectLinePanel(SPan *p, float x, float y, float z) {
	// If it's an enlarged panel
	if (p->_flags & 0x80000000)
		return false;

	float x1 = _actor._camera->_ex;
	float y1 = _actor._camera->_ey;
	float z1 = _actor._camera->_ez;

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

	if (d != 0.0) {
		t = n / d;

		if (t <= 0.0)
			return false;

		_x3d = x1 + dx * t;
		_y3d = y1 + dy * t;
		_z3d = z1 + dz * t;

		float minx = MIN(p->_x1, p->_x2) - 1.5;
		float maxx = MAX(p->_x1, p->_x2) + 1.5;
		float minz = MIN(p->_z1, p->_z2) - 1.5;
		float maxz = MAX(p->_z1, p->_z2) + 1.5;

		// check if it fits inside the panel
		if ((_x3d >= minx) && (_x3d <= maxx) && (_y3d >= 0.0) && (_y3d <= p->_h) && (_z3d >= minz) && (_z3d <= maxz))
			return true;


		return false;
	}

	return false;
}

/*------------------------------------------------
		Intersects 3D line with the floor
--------------------------------------------------*/
int intersectLineFloor(float x, float y, float z) {
	float x1 = _actor._camera->_ex;
	float y1 = _actor._camera->_ey;
	float z1 = _actor._camera->_ez;

	float dx = (x - x1);
	float dy = (y - y1);
	float dz = (z - z1);
	float t = sqrt(dx * dx + dy * dy + dz * dz);
	dx /= t;
	dy /= t;
	dz /= t;

	// ny is always equal to 1 for the floor

	if (dy != 0.0) {
		t = -y / dy;

		if (t <= 0.0)
			return false;

		_x3d = x1 + dx * t;
		_y3d = y1 + dy * t;
		_z3d = z1 + dz * t;

		return true;
	}

	return false;
}

/*------------------------------------------------
		Intersects a 2D line with a 2D line
--------------------------------------------------*/
int intersectLineLine(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd) {
	float divisor = (float)((xb - xa) * (yd - yc) - (yb - ya) * (xd - xc));
	if (divisor == 0.0)
		return false;

	float r = (float)((ya - yc) * (xd - xc) - (xa - xc) * (yd - yc)) / divisor;
	float s = (float)((ya - yc) * (xb - xa) - (xa - xc) * (yb - ya)) / divisor;
	if ((r < 0) || (r > 1) || (s < 0) || (s > 1))
		return false;

	_x3d = xa + r * (xb - xa);
	_y3d = 0.0;
	_z3d = ya + r * (yb - ya);

	return true;
}

/*------------------------------------------------
			Initializes sort panel
--------------------------------------------------*/
void initSortPan() {
	int a, b;

	_numSortPan = 31;

	for (a = 1; a < _numSortPan - 1; a++) {
		_sortPan[a]._min = 32000.0;
		_sortPan[a]._num = a;
	}

	// First panel is behind everything and is not sorted
	_sortPan[0]._min = 30000.0;
	_sortPan[0]._num = BACKGROUND;

	// Last panel is in front of everything and is not sorted
	_sortPan[30]._min = 0.0;
	_sortPan[30]._num = FOREGROUND;

	// Sort panel blocks by increasing distance from the camera
	for (b = 0; b < _panelNum; b++) {
		if (!(_panel[b]._flags & 0x80000000)) {
			float dist1 = dist3D(_actor._camera->_ex, 0.0, _actor._camera->_ez,
								 _panel[b]._x1, 0.0, _panel[b]._z1);
			float dist2 = dist3D(_actor._camera->_ex, 0.0, _actor._camera->_ez,
								 _panel[b]._x2, 0.0, _panel[b]._z2);

			float min = MIN(dist1, dist2);

			for (a = 0; a < _numSortPan; a++) {
				if (_panel[b]._flags & (1 << a)) {
					if (_sortPan[a + 1]._min > min)
						_sortPan[a + 1]._min = min;
				}
			}
		}
	}

	sortPanel();

	for (b = 0; b < _numSortPan; b++) {
		if (_sortPan[b]._num == BACKGROUND) {
			// now the panels go from 0 (foreground) to _numSortPan (background)
			_numSortPan = b;
			break;
		}
	}
}
/*------------------------------------------------
		Compare panel distance (qsort)
--------------------------------------------------*/
int panCompare(const void *arg1, const void *arg2) {
	SSortPan *p1 = (SSortPan *)arg1;
	SSortPan *p2 = (SSortPan *)arg2;

	if (p1->_min > p2->_min)
		return 1;

	if (p1->_min < p2->_min)
		return -1;

	return 0;
}

/*------------------------------------------------
				Sort the panels
--------------------------------------------------*/
void sortPanel() {
	qsort(&_sortPan[0], _numSortPan, sizeof(SSortPan), panCompare);
}

/*------------------------------------------------
  Tells after which panel the character stands
--------------------------------------------------*/
void actorOrder() {
#define LARGEVAL	15.0	// 30 cm (max)

	if (_forcedActorPos) {
		_actorPos = _forcedActorPos;
		return ;
	}

	float ox = _actor._px + _actor._dx - _actor._camera->_ex;
	float oz = _actor._pz + _actor._dz - _actor._camera->_ez;
	float dist = sqrt(ox * ox + oz * oz);
	float lx = (-oz / dist) * LARGEVAL;
	float lz = (ox / dist) * LARGEVAL;

	ox = _actor._px + _actor._dx;
	oz = _actor._pz + _actor._dz;

	// It must be copied in front of the nearest box
	_actorPos = _sortPan[1]._num;
	// from closest to farthest
	for (int b = 1; b < _numSortPan; b++) {
		for (int a = 0; a < _panelNum; a++) {
			// If it's not wide and belongs to this level
			if (!(_panel[a]._flags & 0x80000000) && (_panel[a]._flags & (1 << (_sortPan[b]._num - 1)))) {
				// If it intersects the center of the character camera
				if (intersectLineLine(_panel[a]._x1, _panel[a]._z1, _panel[a]._x2, _panel[a]._z2, _actor._camera->_ex, _actor._camera->_ez, ox, oz)
						|| intersectLineLine(_panel[a]._x1, _panel[a]._z1, _panel[a]._x2, _panel[a]._z2, _actor._camera->_ex, _actor._camera->_ez, ox + lx, oz + lz)
						|| intersectLineLine(_panel[a]._x1, _panel[a]._z1, _panel[a]._x2, _panel[a]._z2, _actor._camera->_ex, _actor._camera->_ez, ox - lx, oz - lz)) {
					// If it intersects it must be copied after the next box
					_actorPos = _sortPan[b + 1]._num;
				}
			}
		}
	}
}

} // End of namespace Trecision
