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
#include "common/str.h"

#include "trecision/defines.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/graphics.h"
#include "trecision/trecision.h"
#include "trecision/actor.h"

namespace Trecision {

// locals
float _lookX, _lookZ;

int read3D(Common::String filename) {
	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember(filename);
	if (ff == nullptr)
		error("read3D: Can't open 3D file %s", filename.c_str());

	// read rooms and lights
	SCamera *cam = g_vm->_actor->_camera;
	cam->_ex = ff->readFloatLE();
	cam->_ey = ff->readFloatLE();
	cam->_ez = ff->readFloatLE();
	for (int i = 0; i < 3; ++i)
		cam->_e1[i] = ff->readFloatLE();
	for (int i = 0; i < 3; ++i)
		cam->_e2[i] = ff->readFloatLE();
	for (int i = 0; i < 3; ++i)
		cam->_e3[i] = ff->readFloatLE();
	cam->_fovX = ff->readFloatLE();
	cam->_fovY = ff->readFloatLE();

	g_vm->_actor->_lightNum = ff->readSint32LE();
	if (g_vm->_actor->_lightNum > MAXLIGHT)
		error("read3D(): Too many lights");

	for (int i = 0; i < g_vm->_actor->_lightNum; ++i) {
		g_vm->_actor->_light[i]._x = ff->readFloatLE();
		g_vm->_actor->_light[i]._y = ff->readFloatLE();
		g_vm->_actor->_light[i]._z = ff->readFloatLE();
		g_vm->_actor->_light[i]._dx = ff->readFloatLE();
		g_vm->_actor->_light[i]._dy = ff->readFloatLE();
		g_vm->_actor->_light[i]._dz = ff->readFloatLE();
		g_vm->_actor->_light[i]._inr = ff->readFloatLE();
		g_vm->_actor->_light[i]._outr = ff->readFloatLE();
		g_vm->_actor->_light[i]._hotspot = ff->readByte();
		g_vm->_actor->_light[i]._fallOff = ff->readByte();
		g_vm->_actor->_light[i]._inten = ff->readSByte();
		g_vm->_actor->_light[i]._position = ff->readSByte();
	}

	// read panels
	_panelNum = ff->readSint32LE();
	if (_panelNum > MAXPANELSINROOM)
		error("read3D(): Too many panels");

	for (int i = 0; i < _panelNum; ++i) {
		_panel[i]._x1 = ff->readFloatLE();
		_panel[i]._z1 = ff->readFloatLE();
		_panel[i]._x2 = ff->readFloatLE();
		_panel[i]._z2 = ff->readFloatLE();
		_panel[i]._h = ff->readFloatLE();
		_panel[i]._flags = ff->readUint32LE();

		_panel[i]._near1 = ff->readSByte();
		_panel[i]._near2 = ff->readSByte();
		_panel[i]._col1 = ff->readSByte();
		_panel[i]._col2 = ff->readSByte();
	}
	delete ff;

	// projection matrix
	_proj[0][0] = g_vm->_actor->_camera->_e1[0];
	_proj[0][1] = g_vm->_actor->_camera->_e1[1];
	_proj[0][2] = g_vm->_actor->_camera->_e1[2];
	_proj[1][0] = g_vm->_actor->_camera->_e2[0];
	_proj[1][1] = g_vm->_actor->_camera->_e2[1];
	_proj[1][2] = g_vm->_actor->_camera->_e2[2];
	_proj[2][0] = g_vm->_actor->_camera->_e3[0];
	_proj[2][1] = g_vm->_actor->_camera->_e3[1];
	_proj[2][2] = g_vm->_actor->_camera->_e3[2];

	// Compute 3x3 inverse matrix for 2D points on 3D
	float det = _proj[0][0] * _proj[1][1] * _proj[2][2] +
				_proj[0][1] * _proj[1][2] * _proj[2][0] +
				_proj[0][2] * _proj[1][0] * _proj[2][1] -
				_proj[2][0] * _proj[1][1] * _proj[0][2] -
				_proj[2][1] * _proj[1][2] * _proj[2][0] -
				_proj[2][2] * _proj[1][0] * _proj[2][1];

	if (det == 0.0)
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

	_cx = 320;
	_cy = 240;

	initSortPan();

	init3DRoom(g_vm->_screenBuffer, g_vm->_zBuffer);
	setClipping(0, TOP, MAXX, AREA + TOP);

	return 10L;
}

#define NOOLDINTERS		1
#define NOCURINTERS		2
#define OLDANGLESKIP	4
#define CURANGLESKIP	8
#define CLICKINTO		16
#define POINTOUT1		32
#define POINTOUT2		64

void findPath() {
	int b;

	g_vm->_actor->_px += g_vm->_actor->_dx;
	g_vm->_actor->_pz += g_vm->_actor->_dz;

	int inters = 0;
	_numPathNodes = 0;

	// if you have clicked behind the starting panel or the corner it's not possible to walk
	if ((_curPanel < 0) && (_oldPanel >= 0) &&
			// behind the starting panel
			((pointInside(b = _oldPanel, (double)_curX, (double)_curZ)) ||
			 // behind the panel corner1
		 ((distF(_panel[_oldPanel]._x1, _panel[_oldPanel]._z1, g_vm->_actor->_px, g_vm->_actor->_pz) < EPSILON) &&
			  (pointInside(b = _panel[_oldPanel]._near1, (double)_curX, (double)_curZ) ||
			   pointInside(b = _panel[_oldPanel]._near2, (double)_curX, (double)_curZ))) ||
			 // behind the panel corner2
		 ((distF(_panel[_oldPanel]._x2, _panel[_oldPanel]._z2, g_vm->_actor->_px, g_vm->_actor->_pz) < EPSILON) &&
			  (pointInside(b = _panel[_oldPanel]._near2, (double)_curX, (double)_curZ) ||
			   pointInside(b = _panel[_oldPanel]._near1, (double)_curX, (double)_curZ))))) {
		_curX = g_vm->_actor->_px;
		_curZ = g_vm->_actor->_pz;
		g_vm->_actor->_px -= g_vm->_actor->_dx;
		g_vm->_actor->_pz -= g_vm->_actor->_dz;
		_curPanel = b;
		_numPathNodes = 0;
		lookAt(_lookX, _lookZ);
		return;
	}

	float dist = distF(g_vm->_actor->_px, g_vm->_actor->_pz, _curX, _curZ);

	for (b = 0; b < _panelNum; b++) {
		if (_panel[b]._flags & 0x80000000) {       // it must be a wide panel
			if (intersectLineLine(_panel[b]._x1, _panel[b]._z1,
								  _panel[b]._x2, _panel[b]._z2,
								  g_vm->_actor->_px, g_vm->_actor->_pz, _curX, _curZ)) {
				inters++;

				_pathNode[_numPathNodes]._x    = _x3d;
				_pathNode[_numPathNodes]._z    = _z3d;
				_pathNode[_numPathNodes]._dist = distF(g_vm->_actor->_px, g_vm->_actor->_pz, _x3d, _z3d);
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
							_curX = g_vm->_actor->_px;
							_curZ = g_vm->_actor->_pz;
							g_vm->_actor->_px -= g_vm->_actor->_dx;
							g_vm->_actor->_pz -= g_vm->_actor->_dz;

							_curPanel = b;
							lookAt(_lookX, _lookZ);
							return;
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

				_pathNode[_numPathNodes]._x = g_vm->_actor->_px;
				_pathNode[_numPathNodes]._z = g_vm->_actor->_pz;
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
				 (!findAttachedPanel(_pathNode[_numPathNodes - 2]._curp, _pathNode[_numPathNodes - 1]._curp) ||
				  pointInside(_pathNode[_numPathNodes - 1]._curp, (double)_curX, (double)_curZ)))) {

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
		_pathNode[_numPathNodes]._dist = distF(g_vm->_actor->_px, g_vm->_actor->_pz, _curX, _curZ);
		_pathNode[_numPathNodes]._oldp = _curPanel;
		_pathNode[_numPathNodes]._curp = _curPanel;
		_numPathNodes ++;

		findShortPath();
		displayPath();
	} else {     // otherwise if it's direct
		_pathNode[_numPathNodes]._x = g_vm->_actor->_px;
		_pathNode[_numPathNodes]._z = g_vm->_actor->_pz;
		_pathNode[_numPathNodes]._dist = 0.0;
		_pathNode[_numPathNodes]._oldp = _oldPanel;
		_pathNode[_numPathNodes]._curp = _oldPanel;
		_numPathNodes++;

		_pathNode[_numPathNodes]._x    = _curX;
		_pathNode[_numPathNodes]._z    = _curZ;
		_pathNode[_numPathNodes]._dist = distF(g_vm->_actor->_px, g_vm->_actor->_pz, _curX, _curZ);
		_pathNode[_numPathNodes]._oldp = _curPanel;
		_pathNode[_numPathNodes]._curp = _curPanel;
		_numPathNodes++;

		displayPath();
	}

	g_vm->_actor->_px -= g_vm->_actor->_dx;
	g_vm->_actor->_pz -= g_vm->_actor->_dz;
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
	TempPath[count]._x = g_vm->_actor->_px;
	TempPath[count]._z = g_vm->_actor->_pz;
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
		if (!findAttachedPanel(curp, _pathNode[a + 1]._curp))
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
	// controlla che in nessun caso attraversi o sfiori un pannello stretto
	for (int a = 1; a < _numPathNodes; a++) {
		for (int c = 0; c < _panelNum; c++) {
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

	float ox = _pathNode[0]._x;
	float oz = _pathNode[0]._z;

	for (int a = 1; a < _numPathNodes; a++) {
		len += dist3D(_pathNode[a]._x, 0.0, _pathNode[a]._z, ox, 0.0, oz);

		ox = _pathNode[a]._x;
		oz = _pathNode[a]._z;
	}
	// ha calcolato lunghezza totale percorso - se troppo piccola esce
	if (len < 2.0) {
		lookAt(_lookX, _lookZ);
		return;
	}

	int a = 0;
	// compute offset
	SVertex *v = g_vm->_actor->_characterArea;
	float firstframe = FRAMECENTER(v);
	float startpos = 0.0;

	// se stava gia' camminando
	int CurA, CurF, cfp;
	if (g_vm->_actor->_curAction == hWALK) {
		// calcola frame attuale
		cfp = _defActionLen[hSTART] + 1 + g_vm->_actor->_curFrame;
		v += cfp * g_vm->_actor->_vertexNum;

		CurA = hWALK;
		CurF = g_vm->_actor->_curFrame;

		// se non era all'ultimo frame fa il passo dodpo
		if (g_vm->_actor->_curFrame < _defActionLen[hWALK] - 1) {
			cfp++;
			CurF++;
			v += g_vm->_actor->_vertexNum;
		}
	}
	// se era in stop riparte
	else if ((g_vm->_actor->_curAction >= hSTOP0) && (g_vm->_actor->_curAction <= hSTOP9)) {
		// calcola frame attuale
		CurA = hWALK;
//o		CurF = g_vm->_actor->_curAction - hSTOP1;
		CurF = g_vm->_actor->_curAction - hSTOP0;

		cfp = _defActionLen[hSTART] + 1 + CurF;
		v += cfp * g_vm->_actor->_vertexNum;
	}
	// se era fermo, partiva o girava riparte da stand
	else {
		oz  = 0.0;
		cfp = 1;

		CurA = hSTART;
		CurF = 0;

		// parte dal primo frame
		v += g_vm->_actor->_vertexNum;
	}
	oz   =  - FRAMECENTER(v) + firstframe;

	// at this point, CurA / _curAction is either hSTART or hWALK

	// until it arrives at the destination
	while (((curlen = oz + FRAMECENTER(v) - firstframe) < len) || (!a)) {
		_step[a]._pz = oz - firstframe;	// where to render
		_step[a]._dz = curlen;			// where it is
		_step[a]._curAction = CurA;
		_step[a]._curFrame  = CurF;

		a ++;
		v += g_vm->_actor->_vertexNum;

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

				// end walk frame
				ox = FRAMECENTER(v) - firstframe;

				v = &g_vm->_actor->_characterArea[cfp * g_vm->_actor->_vertexNum];
				ox -= FRAMECENTER(v);

			}

			v = &g_vm->_actor->_characterArea[cfp * g_vm->_actor->_vertexNum];

			// only if it doesn't end
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

	assert(CurA <= hLAST);		// _defActionLen below has a size of hLAST + 1

	CurF  = 0;

	int b = 0;
	cfp = 0;
	while (b != CurA)
		cfp += _defActionLen[b++];

	v = &g_vm->_actor->_characterArea[cfp * g_vm->_actor->_vertexNum];

	for (b = 0; b < _defActionLen[CurA]; b++) {
		curlen = oz + FRAMECENTER(v) - firstframe;
		_step[a]._pz = oz - firstframe;		// dove renderizzare
		_step[a]._dz = curlen;			// dove si trova
		_step[a]._curAction = CurA;
		_step[a]._curFrame  = CurF;

		a ++;
		CurF ++;
		v += g_vm->_actor->_vertexNum;
	}

	// di quanto ha sbagliato?
	float approx = (len - curlen - EPSILON) / (a - 2);
	float theta = 0.0;
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
	float cx = _step[b - 1]._dz;

	_lastStep = b;		// ultimo step
	_curStep  = 0;		// step attuale

	// ora inserisce direzioni e pto partenza arrivo esatti
	b = 0;
	//startpos = _step[0]._pz;

	len = 0.0;
	startpos = 0.0;
	float oldtheta = -1.0;
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

		theta = g_vm->sinCosAngle(ox, oz) * 180.0f / PI + 180.0f;
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
	oldtheta = g_vm->_actor->_theta;
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
			float cz = _step[b]._pz + _step[b]._dz;

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
			float cz = _step[b - 1]._pz + _step[b - 1]._dz;

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
	g_vm->_actor->_px = _step[_curStep]._px;
	g_vm->_actor->_pz = _step[_curStep]._pz;
	g_vm->_actor->_dx = _step[_curStep]._dx;
	g_vm->_actor->_dz = _step[_curStep]._dz;
	g_vm->_actor->_theta = _step[_curStep]._theta;
	g_vm->_actor->_curAction = _step[_curStep]._curAction;
	g_vm->_actor->_curFrame = _step[_curStep]._curFrame;
	_curPanel = _step[_curStep]._curPanel;

	// avanza solo se non e' ultimo frame
	if (_curStep < _lastStep) {
		_curStep ++;
		return false;
	}

	if (_characterGoToPosition != -1)
		setPosition(_characterGoToPosition);

	return true;
}
/*-----------------15/10/96 11.42-------------------
				Visualizza percorso
--------------------------------------------------*/
void displayPath() {
	buildFramelist();
}

/*------------------------------------------------
	Check if two panels are in the same block
--------------------------------------------------*/
bool findAttachedPanel(int srcPanel, int destPanel) {
	// if at least one is on the floor, return false
	if (srcPanel < 0 || destPanel < 0)
		return false;

	// if they are equal, return true
	if (srcPanel == destPanel)
		return true;

	int curPanel = srcPanel;
	int nearPanel = _panel[srcPanel]._near1;

	for (int b = 0;; b++) {
		// if they are attached, return true
		if (curPanel == destPanel)
			return true;

		// if it has returned to the starting panel, return false
		if (srcPanel == curPanel && b)
			return false;

		if (b > _panelNum)
			return false;

		// if they are attached to vertex 1, take 2
		if (_panel[nearPanel]._near1 == curPanel) {
			curPanel  = nearPanel;
			nearPanel = _panel[curPanel]._near2;
		} else {
			curPanel  = nearPanel;
			nearPanel = _panel[curPanel]._near1;
		}
	}
}

/*------------------------------------------------
	Check if a point is inside a panel
--------------------------------------------------*/
bool pointInside(int pan, float x, float z) {
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
	bool yflag0 = (vtx0[1] >= z);
	double *vtx1 = pgon[0];

	int counter = 0;
	for (int j = 5; --j;) {
		bool yflag1 = (vtx1[1] >= z);
		if (yflag0 != yflag1) {
			bool xflag0 = (vtx0[0] >= x);
			if ((xflag0 == (vtx1[0] >= x)) && (xflag0))
				counter += (yflag0 ? -1 : 1);
			else if ((vtx1[0] - (vtx1[1] - z) * (vtx0[0] - vtx1[0]) / (vtx0[1] - vtx1[1])) >= x)
				counter += (yflag0 ? -1 : 1);
		}

		// Move to the next pair of vertices, retaining info as possible.
		yflag0 = yflag1;
		vtx0 = vtx1;
		vtx1 += 2;
	}

	return (counter != 0);
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

/*------------------------------------------------
  Find the 3D point corresponding to the 2D point
--------------------------------------------------*/
void whereIs(int px, int py) {
	float inters = 32000.0;

	g_vm->_actor->_px += g_vm->_actor->_dx;
	g_vm->_actor->_pz += g_vm->_actor->_dz;

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
			float temp = dist3D(g_vm->_actor->_camera->_ex, g_vm->_actor->_camera->_ey, g_vm->_actor->_camera->_ez, _x3d, _y3d, _z3d);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				_curX = _x3d;
				_curZ = _z3d;
			}
		}
	}

	_lookX = _curX;
	_lookZ = _curZ;

	pointOut();

	g_vm->_actor->_px -= g_vm->_actor->_dx;
	g_vm->_actor->_pz -= g_vm->_actor->_dz;
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
			if (intersectLineLine(_panel[b]._x1, _panel[b]._z1, _panel[b]._x2, _panel[b]._z2, g_vm->_actor->_camera->_ex, g_vm->_actor->_camera->_ez, _curX, _curZ)) {
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
								  g_vm->_actor->_px, g_vm->_actor->_pz, _curX, _curZ)) {
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

		g_vm->_graphicsMgr->drawLine(projVerts[0][0], projVerts[0][1], projVerts[2][0], projVerts[2][1], 0x1C1);
		g_vm->_graphicsMgr->drawLine(projVerts[1][0], projVerts[1][1], projVerts[3][0], projVerts[3][1], 0x1C1);
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
	g_vm->_graphicsMgr->drawLine(projVerts[0][0], projVerts[0][1], projVerts[1][0], projVerts[1][1], col);
	// H2
	g_vm->_graphicsMgr->drawLine(projVerts[2][0], projVerts[2][1], projVerts[3][0], projVerts[3][1], col);
	// B
	g_vm->_graphicsMgr->drawLine(projVerts[0][0], projVerts[0][1], projVerts[2][0], projVerts[2][1], col);
	// T
	g_vm->_graphicsMgr->drawLine(projVerts[1][0], projVerts[1][1], projVerts[3][0], projVerts[3][1], col);
}

/*------------------------------------------------
		Projects 3D point on 2D screen
--------------------------------------------------*/
void pointProject(float x, float y, float z) {
	float pa0 = g_vm->_actor->_camera->_ex - x;
	float pa1 = g_vm->_actor->_camera->_ey - y;
	float pa2 = g_vm->_actor->_camera->_ez - z;

	float p0 = pa0 * _proj[0][0] + pa1 * _proj[0][1] + pa2 * _proj[0][2];
	float p1 = pa0 * _proj[1][0] + pa1 * _proj[1][1] + pa2 * _proj[1][2];
	float p2 = pa0 * _proj[2][0] + pa1 * _proj[2][1] + pa2 * _proj[2][2];

	_x2d = _cx + (int)((p0 * g_vm->_actor->_camera->_fovX) / p2);
	_y2d = _cy + (int)((p1 * g_vm->_actor->_camera->_fovY) / p2);
}

/*------------------------------------------------
		Projects 2D point in a 3D world
--------------------------------------------------*/
void invPointProject(int x, int y) {
	float px = (float)(x - _cx) / g_vm->_actor->_camera->_fovX;
	float py = (float)(y - _cy) / g_vm->_actor->_camera->_fovY;

	_x3d = (float)(px * _invP[0][0] + py * _invP[0][1] + _invP[0][2]);
	_y3d = (float)(px * _invP[1][0] + py * _invP[1][1] + _invP[1][2]);
	_z3d = (float)(px * _invP[2][0] + py * _invP[2][1] + _invP[2][2]);

	_x3d += g_vm->_actor->_camera->_ex;
	_y3d += g_vm->_actor->_camera->_ey;
	_z3d += g_vm->_actor->_camera->_ez;
}

/*------------------------------------------------
		Intersects a 3D line with the panel
--------------------------------------------------*/
bool intersectLinePanel(SPan *p, float x, float y, float z) {
	// If it's an enlarged panel
	if (p->_flags & 0x80000000)
		return false;

	float x1 = g_vm->_actor->_camera->_ex;
	float y1 = g_vm->_actor->_camera->_ey;
	float z1 = g_vm->_actor->_camera->_ez;

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
bool intersectLineFloor(float x, float y, float z) {
	float x1 = g_vm->_actor->_camera->_ex;
	float y1 = g_vm->_actor->_camera->_ey;
	float z1 = g_vm->_actor->_camera->_ez;

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
bool intersectLineLine(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd) {
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
			float dist1 = dist3D(g_vm->_actor->_camera->_ex, 0.0, g_vm->_actor->_camera->_ez, _panel[b]._x1, 0.0, _panel[b]._z1);
			float dist2 = dist3D(g_vm->_actor->_camera->_ex, 0.0, g_vm->_actor->_camera->_ez, _panel[b]._x2, 0.0, _panel[b]._z2);

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
		return;
	}

	float ox = g_vm->_actor->_px + g_vm->_actor->_dx - g_vm->_actor->_camera->_ex;
	float oz = g_vm->_actor->_pz + g_vm->_actor->_dz - g_vm->_actor->_camera->_ez;
	float dist = sqrt(ox * ox + oz * oz);
	float lx = (-oz / dist) * LARGEVAL;
	float lz = (ox / dist) * LARGEVAL;

	ox = g_vm->_actor->_px + g_vm->_actor->_dx;
	oz = g_vm->_actor->_pz + g_vm->_actor->_dz;

	// It must be copied in front of the nearest box
	_actorPos = _sortPan[1]._num;
	// from closest to farthest
	for (int b = 1; b < _numSortPan; b++) {
		for (int a = 0; a < _panelNum; a++) {
			// If it's not wide and belongs to this level
			if (!(_panel[a]._flags & 0x80000000) && (_panel[a]._flags & (1 << (_sortPan[b]._num - 1)))) {
				// If it intersects the center of the character camera
				if (intersectLineLine(_panel[a]._x1, _panel[a]._z1, _panel[a]._x2, _panel[a]._z2, g_vm->_actor->_camera->_ex, g_vm->_actor->_camera->_ez, ox, oz)
				|| intersectLineLine(_panel[a]._x1, _panel[a]._z1, _panel[a]._x2, _panel[a]._z2, g_vm->_actor->_camera->_ex, g_vm->_actor->_camera->_ez, ox + lx, oz + lz)
				|| intersectLineLine(_panel[a]._x1, _panel[a]._z1, _panel[a]._x2, _panel[a]._z2, g_vm->_actor->_camera->_ex, g_vm->_actor->_camera->_ez, ox - lx, oz - lz)) {
					// If it intersects it must be copied after the next box
					_actorPos = _sortPan[b + 1]._num;
				}
			}
		}
	}
}

} // End of namespace Trecision
