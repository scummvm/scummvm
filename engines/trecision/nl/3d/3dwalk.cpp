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
#include "trecision/3d.h"

#include "trecision/defines.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/graphics.h"
#include "trecision/trecision.h"
#include "trecision/actor.h"

namespace Trecision {

void read3D(Common::String filename) {
	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember(filename);
	if (ff == nullptr)
		error("read3D: Can't open 3D file %s", filename.c_str());

	g_vm->_actor->read3D(ff);
	
	// read panels
	_panelNum = ff->readSint32LE();
	if (_panelNum > MAXPANELSINROOM)
		error("read3D(): Too many panels");

	for (int i = 0; i < _panelNum; ++i) {
		g_vm->_pathFind->_panel[i]._x1 = ff->readFloatLE();
		g_vm->_pathFind->_panel[i]._z1 = ff->readFloatLE();
		g_vm->_pathFind->_panel[i]._x2 = ff->readFloatLE();
		g_vm->_pathFind->_panel[i]._z2 = ff->readFloatLE();
		g_vm->_pathFind->_panel[i]._h = ff->readFloatLE();
		g_vm->_pathFind->_panel[i]._flags = ff->readUint32LE();

		g_vm->_pathFind->_panel[i]._near1 = ff->readSByte();
		g_vm->_pathFind->_panel[i]._near2 = ff->readSByte();
		g_vm->_pathFind->_panel[i]._col1 = ff->readSByte();
		g_vm->_pathFind->_panel[i]._col2 = ff->readSByte();
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

	g_vm->_pathFind->initSortPan();

	g_vm->_renderer->init3DRoom(g_vm->_graphicsMgr->getScreenBufferPtr(), g_vm->_zBuffer);
	g_vm->_renderer->setClipping(0, TOP, MAXX, AREA + TOP);
}

#define NOOLDINTERS		1
#define NOCURINTERS		2
#define OLDANGLESKIP	4
#define CURANGLESKIP	8
#define CLICKINTO		16
#define POINTOUT1		32
#define POINTOUT2		64

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
		if (intersectLinePanel(&g_vm->_pathFind->_panel[b], x, y, z)) {
			float temp = g_vm->dist3D(g_vm->_actor->_camera->_ex, g_vm->_actor->_camera->_ey, g_vm->_actor->_camera->_ez, _x3d, _y3d, _z3d);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				_curX = _x3d;
				_curZ = _z3d;
			}
		}
	}

	g_vm->_pathFind->_lookX = _curX;
	g_vm->_pathFind->_lookZ = _curZ;

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

	SPan *panel = &g_vm->_pathFind->_panel[_curPanel];
	float nx = panel->_z1 - panel->_z2;
	float nz = panel->_x2 - panel->_x1;
	float temp = sqrt(nx * nx + nz * nz);
	nx /= temp;
	nz /= temp;

	// move the point on the wide panel
	for (int b = 0; b < _panelNum; b++) {
		panel = &g_vm->_pathFind->_panel[b];
		// Only check the external panels with the same flag
		if ((panel->_flags & 0x80000000) && (panel->_flags & (g_vm->_pathFind->_panel[_curPanel]._flags & 0x7FFFFFFF))) {
			// check point 1
			temp = g_vm->dist2D(_curX, _curZ, panel->_x1, panel->_z1);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				x = panel->_x1;
				z = panel->_z1;
			}

			// check point 2
			temp = g_vm->dist2D(_curX, _curZ, panel->_x2, panel->_z2);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				x = panel->_x2;
				z = panel->_z2;
			}

			// check point a 1/3
			temp = g_vm->dist2D(_curX, _curZ, (panel->_x1 * 2.0 + panel->_x2) / 3.0, (panel->_z1 * 2.0 + panel->_z2) / 3.0);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				x = (panel->_x1 * 2.0 + panel->_x2) / 3.0;
				z = (panel->_z1 * 2.0 + panel->_z2) / 3.0;
			}

			// check point a 2/3
			temp = g_vm->dist2D(_curX, _curZ, (panel->_x1 + panel->_x2 * 2.0) / 3.0, (panel->_z1 + panel->_z2 * 2.0) / 3.0);

			if (temp < inters) {
				inters = temp;
				_curPanel = b;
				x = (panel->_x1 + panel->_x2 * 2.0) / 3.0;
				z = (panel->_z1 + panel->_z2 * 2.0) / 3.0;
			}

			// check intersection with camera
			if (intersectLineLine(panel->_x1, panel->_z1, panel->_x2, panel->_z2, g_vm->_actor->_camera->_ex, g_vm->_actor->_camera->_ez, _curX, _curZ)) {
				temp = g_vm->dist2D(_curX, _curZ, _x3d, _z3d);

				if (temp < inters) {
					inters = temp;
					_curPanel = b;
					x = _x3d;
					z = _z3d;
				}
			}

			// check intersection with character
			if (intersectLineLine(panel->_x1, panel->_z1, panel->_x2, panel->_z2, g_vm->_actor->_px, g_vm->_actor->_pz, _curX, _curZ)) {
				temp = g_vm->dist2D(_curX, _curZ, _x3d, _z3d);

				if (temp < inters) {
					inters = temp;
					_curPanel = b;
					x = _x3d;
					z = _z3d;
				}
			}

			// check intersection with normal panel
			if (intersectLineLine(panel->_x1, panel->_z1, panel->_x2, panel->_z2,
								  _curX + nx * LARGEVAL, _curZ + nz * LARGEVAL, _curX - nx * LARGEVAL, _curZ - nz * LARGEVAL)) {
				temp = g_vm->dist2D(_curX, _curZ, _x3d, _z3d);

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
	Common::Point projVerts[4];

	uint16 col = (p->_flags & 0x80000000) ? 0x3C0 : 0x3FF;
	if (p->_flags & 0x80000000) {
		pointProject(p->_x1, 0.0, p->_z1);
		projVerts[0] = Common::Point(_x2d, _y2d);

		pointProject(p->_x2, 0.0, p->_z2);
		projVerts[1] = Common::Point(_x2d, _y2d);

		SPan *panel = &g_vm->_pathFind->_panel[p->_col1 & 0x7F];
		pointProject((p->_col1 & 0x80) ? panel->_x2 : panel->_x1, 0.0, (p->_col1 & 0x80) ? panel->_z2 : panel->_z1);
		projVerts[2] = Common::Point(_x2d, _y2d);

		panel = &g_vm->_pathFind->_panel[p->_col2 & 0x7F];
		pointProject((p->_col2 & 0x80) ? panel->_x2 : panel->_x1, 0.0, (p->_col2 & 0x80) ? panel->_z2 : panel->_z1);
		projVerts[3] = Common::Point(_x2d, _y2d);

		g_vm->_graphicsMgr->drawLine(projVerts[0].x, projVerts[0].y, projVerts[2].x, projVerts[2].y, 0x1C1);
		g_vm->_graphicsMgr->drawLine(projVerts[1].x, projVerts[1].y, projVerts[3].x, projVerts[3].y, 0x1C1);
	}

	if (p->_flags & (1 << 28))
		col = g_vm->_graphicsMgr->palTo16bit(233, 238, 21);

	pointProject(p->_x1, 0.0, p->_z1);
	projVerts[0] = Common::Point(_x2d, _y2d);
	pointProject(p->_x1, p->_h, p->_z1);
	projVerts[1] = Common::Point(_x2d, _y2d);

	pointProject(p->_x2, 0.0, p->_z2);
	projVerts[2] = Common::Point(_x2d, _y2d);
	pointProject(p->_x2, p->_h, p->_z2);
	projVerts[3] = Common::Point(_x2d, _y2d);

	// H1
	g_vm->_graphicsMgr->drawLine(projVerts[0].x, projVerts[0].y, projVerts[1].x, projVerts[1].y, col);
	// H2
	g_vm->_graphicsMgr->drawLine(projVerts[2].x, projVerts[2].y, projVerts[3].x, projVerts[3].y, col);
	// B
	g_vm->_graphicsMgr->drawLine(projVerts[0].x, projVerts[0].y, projVerts[2].x, projVerts[2].y, col);
	// T
	g_vm->_graphicsMgr->drawLine(projVerts[1].x, projVerts[1].y, projVerts[3].x, projVerts[3].y, col);
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
	qsort(&g_vm->_pathFind->_sortPan[0], _numSortPan, sizeof(SSortPan), panCompare);
}

/*------------------------------------------------
  Tells after which panel the character stands
--------------------------------------------------*/
void actorOrder() {
#define LARGEVAL	15.0	// 30 cm (max)
	if (_forcedActorPos != BOX_NORMAL) {
		_actorPos = _forcedActorPos;
		return;
	}

	Actor *actor = g_vm->_actor;

	float ox = actor->_px + actor->_dx - actor->_camera->_ex;
	float oz = actor->_pz + actor->_dz - actor->_camera->_ez;
	float dist = sqrt(ox * ox + oz * oz);
	float lx = (-oz / dist) * LARGEVAL;
	float lz = (ox / dist) * LARGEVAL;

	ox = actor->_px + actor->_dx;
	oz = actor->_pz + actor->_dz;

	// It must be copied in front of the nearest box
	_actorPos = g_vm->_pathFind->_sortPan[1]._num;
	// from closest to farthest
	for (int b = 1; b < _numSortPan; b++) {
		for (int a = 0; a < _panelNum; a++) {
			// If it's not wide and belongs to this level
			if (!(g_vm->_pathFind->_panel[a]._flags & 0x80000000) && (g_vm->_pathFind->_panel[a]._flags & (1 << (g_vm->_pathFind->_sortPan[b]._num - 1)))) {
				// If it intersects the center of the character camera
				if (intersectLineLine(g_vm->_pathFind->_panel[a]._x1, g_vm->_pathFind->_panel[a]._z1, g_vm->_pathFind->_panel[a]._x2, g_vm->_pathFind->_panel[a]._z2, actor->_camera->_ex, actor->_camera->_ez, ox, oz)
				|| intersectLineLine(g_vm->_pathFind->_panel[a]._x1, g_vm->_pathFind->_panel[a]._z1, g_vm->_pathFind->_panel[a]._x2, g_vm->_pathFind->_panel[a]._z2, actor->_camera->_ex, actor->_camera->_ez, ox + lx, oz + lz)
				|| intersectLineLine(g_vm->_pathFind->_panel[a]._x1, g_vm->_pathFind->_panel[a]._z1, g_vm->_pathFind->_panel[a]._x2, g_vm->_pathFind->_panel[a]._z2, actor->_camera->_ex, actor->_camera->_ez, ox - lx, oz - lz)) {
					// If it intersects it must be copied after the next box
					_actorPos = g_vm->_pathFind->_sortPan[b + 1]._num;
				}
			}
		}
	}
}

} // End of namespace Trecision
