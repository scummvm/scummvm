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
#include "trecision/actor.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/trecision.h"
#include "trecision/graphics.h"

namespace Trecision {

int16 _shadowLightNum = 0;
int16 _totalShadowVerts = 0;

int16 _shadowVertsNum = 42;
int16 _shadowVerts[42] = {
	  6,	 15,	 23,
	 24,	 32,	 78,
	 80,	 81,	 83,
	 86,	 90,	 99,
	107,	108,	116,
	155,	157,	158,
	160,	164,	168,
	169,	173,	174,
	187,	188,	192,
	193,	213,	215,
	227,	229,	235,
	238,	249,	250,
	252,	253,	299,
	306,	330,	336
};

int16 _shadowFacesNum = 48;
int16 _shadowFaces[48][3] = {
	22,	21,	5,
	7,	5,	22,
	7,	19,	5,
	5,	2,	19,
	27,	24,	16,
	27,	16,	18,
	18,	16,	9,
	18,	13,	9,
	13,	9,	2,
	3,	19,	12,
	25,	26,	17,
	17,	15,	25,
	17,	19,	15,
	15,	12,	19,
	20,	23,	8,
	8,	6,	20,
	6,	9,	3,
	3,	8,	6,
	12,	3,	4,
	4,	11,	12,
	35,	4,	11,
	13,	2,	1,
	1,	14,	13,
	14,	37,	1,
	1,	34,	37,
	31,	36,	37,
	37,	30,	31,
	29,	34,	35,
	35,	29,	28,
	36,	11,	31,
	30,	37,	14,
	29,	1,	34,
	28,	4,	35,
	36,	10,	35,
	35,	32,	10,
	37,	0,	34,
	37,	33,	0,
	0,	33,	39,
	39,	40,	0,
	10,	38,	32,
	32,	41,	38,
	36,	35,	34,
	36,	37,	35,
	11,	36,	35,
	38,	40,	41,
	41,	38,	39,
	2,	19,	13,
	3,	9,	12
};

uint8 _shadowIntens[10];

struct SVVertex {
	int32 _x, _y, _z;
	int32 _angle;
} _vVertex[MAXVERTEX];

SVertex  _shVertex[MAXVERTEX];

uint32 _materials[21][181];

uint16 *_curPage;
int16  *_zBuf;
int16 _minXClip;
int16 _minYClip;
int16 _maxXClip;
int16 _maxYClip;

int16 _zBufStartX;
int16 _zBufStartY;
int16 _zBufWid;

// data for the triangle routines
int16  _lEdge[480];
int16  _rEdge[480];
uint8 _lColor[480];
uint8 _rColor[480];
int16  _lZ[480];
int16  _rZ[480];
uint16 _lTextX[480];
uint16 _rTextX[480];
uint16 _lTextY[480];
uint16 _rTextY[480];

void textureTriangle(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1,
					 int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2,
					 int32 x3, int32 y3, int32 z3, int32 c3, int32 tx3, int32 ty3,
					 STexture *t) {
	int32 cl; 	// color of left edge of horizontal scanline
	int32 zl; 	// zbuffer of left edge of horizontal scanline
	int32 olx; 	// texture x of left edge of horizontal scanline
	int32 oly; 	// texture y of left edge of horizontal scanline
	int16 y; 	// looping variable

	if (y1 > _maxYClip)
		y1 = _maxYClip;
	if (y1 < _minYClip)
		y1 = _minYClip;

	int16 yBottom = y1;
	int16 yTop = y1;
	uint8 *texture = t->_texture;

	if (yBottom > y2) {
		if (y2 < _minYClip)
			y2 = _minYClip;
		yBottom = y2;
	}
	if (yTop < y2) {
		if (y2 > _maxYClip)
			y2 = _maxYClip;
		yTop = y2;
	}
	if (yBottom > y3) {
		if (y3 < _minYClip)
			y3 = _minYClip;
		yBottom = y3;
	}
	if (yTop < y3) {
		if (y3 > _maxYClip)
			y3 = _maxYClip;
		yTop = y3;
	}
	for (y = yBottom; y < yTop; y++) {
		_lEdge[y] = _maxXClip;
		_rEdge[y] = _minXClip;
	}

	// scan the edges of the triangle
	textureScanEdge(x1, y1, z1, c1, tx1, ty1, x2, y2, z2, c2, tx2, ty2);
	textureScanEdge(x2, y2, z2, c2, tx2, ty2, x3, y3, z3, c3, tx3, ty3);
	textureScanEdge(x3, y3, z3, c3, tx3, ty3, x1, y1, z1, c1, tx1, ty1);

	// Gouraud fill the horizontal scanlines
	for (y = yBottom; y < yTop; y++) {
		int32 el = _lEdge[y];
		if (el < _minXClip)
			el = _minXClip;
		int32 er = _rEdge[y];
		if (er > _maxXClip)
			er = _maxXClip;

		// edge right - edge left
		int16 dx = er - el;

		if (!(dx <= 0)) {
			// slope dc/_dx
			int32 mc = ((int16)(_rColor[y] - (cl = _lColor[y])) << 8) / dx;
			// slope _dz/_dx
			int32 mz = ((int32)(_rZ[y] - (zl = _lZ[y])) << 16) / dx;
			// slope dty/_dx
			int32 mtx = ((int32)(_rTextX[y] - (olx = _lTextX[y])) << 16) / dx;
			// slope dty/_dx
			int32 mty = ((int32)(_rTextY[y] - (oly = _lTextY[y])) << 16) / dx;
			// screen offset
			int32 sl = el + MAXX * y;
			// pointer to zbuffer
			int16 *z = _zBuf + (y - _zBufStartY) * _zBufWid + (el - _zBufStartX);
			uint16 *screenPtr = _curPage + sl;

			zl <<= 16;
			cl <<=  8;
			olx <<= 16;
			oly <<= 16;
			// loop through every pixel in horizontal scanline
			while (dx) {
				sl = zl >> 16;
				if (*z > sl) {
					*screenPtr = (uint16)(g_vm->_actor->_textureMat[texture[(olx >> 16) + t->_dx * (oly >> 16)]][cl >> 9]);
					*z = (int16)sl;
				}
				screenPtr++;		// increase screen x
				z++;		// increase zbuffer
				zl += mz;   	// increase the zbuffer by _dz/_dx
				cl += mc; 	// increase the color by dc/_dx
				olx += mtx;
				oly += mty;
				dx--;		// pixel to do --
			}
		}
	}
}

void textureScanEdge(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2) {
	// make sure that edge goes from top to bottom
	int16 dy = y2 - y1;
	if (dy < 0) {
		SWAP(y1, y2);
		SWAP(x1, x2);
		SWAP(c1, c2);
		SWAP(z1, z2);
		SWAP(tx1, tx2);
		SWAP(ty1, ty2);

		dy = -dy;
	}

	if (dy == 0)
		dy = 1;

	// initialize for stepping
	int32 mx = ((x2 - x1) << 16) / dy; // dx/dy
	int32 mz = ((z2 - z1) << 16) / dy; // dz/dy
	int32 mc = ((c2 - c1) << 8) / dy; // dc/dy
	int32 mtx = ((tx2 - tx1) << 16) / dy;
	int32 mty = ((ty2 - ty1) << 16) / dy;

	x1 <<= 16; // starting x coordinate
	z1 <<= 16; // starting x coordinate
	c1 <<=  8; // starting c color

	tx1 <<= 16;
	ty1 <<= 16;

	// step through edge and record color values along the way
	for (int32 count = y1; count < y2; count++) {
		int16 x = (uint16)(x1 >> 16);
		if (x < _lEdge[count]) {
			_lEdge[count]  = x;
			_lZ[count]     = (int16)(z1 >> 16);
			_lTextX[count] = (uint16)(tx1 >> 16);
			_lTextY[count] = (uint16)(ty1 >> 16);
			_lColor[count] = (uint8)(c1 >> 8);
		}
		if (x > _rEdge[count]) {
			_rEdge[count] = x;
			_rZ[count]     = (int16)(z1 >> 16);
			_rTextX[count] = (uint16)(tx1 >> 16);
			_rTextY[count] = (uint16)(ty1 >> 16);
			_rColor[count] = (uint8)(c1 >> 8);
		}

		x1 += mx;  // x = x + dx/dy
		c1 += mc;  // c = c + dc/dy
		z1 += mz;  // z = z + dz/dy

		tx1 += mtx;
		ty1 += mty;
	}
}

void shadowTriangle(int32 x1, int32 y1, int32 x2, int32 y2,
					int32 x3, int32 y3, uint8 cv, int32 zv) {
	if (y1 > _maxYClip)
		y1 = _maxYClip;
	if (y1 < _minYClip)
		y1 = _minYClip;

	int16 yBottom = y1;
	int16 yTop = y1;

	if (yBottom > y2) {
		if (y2 < _minYClip)
			y2 = _minYClip;
		yBottom = y2;
	}
	if (yTop < y2) {
		if (y2 > _maxYClip)
			y2 = _maxYClip;
		yTop = y2;
	}
	if (yBottom > y3) {
		if (y3 < _minYClip)
			y3 = _minYClip;
		yBottom = y3;
	}
	if (yTop < y3) {
		if (y3 > _maxYClip)
			y3 = _maxYClip;
		yTop = y3;
	}

	for (int16 y = yBottom; y < yTop; y++) {
		_lEdge[y] = _maxXClip;
		_rEdge[y] = _minXClip;
	}

	// scan the edges of the triangle
	shadowScanEdge(x1, y1, x2, y2);
	shadowScanEdge(x2, y2, x3, y3);
	shadowScanEdge(x3, y3, x1, y1);

	// gouraud fill the horizontal scanlines
	for (int16 y = yBottom; y < yTop; y++) {
		// coordinate of left edge of horizontal scanline
		int32 el = _lEdge[y];
		if (el < _minXClip)
			el = _minXClip;
		// coordinate of right edge of horizontal scanline
		int32 er = _rEdge[y];
		if (er > _maxXClip)
			er = _maxXClip;

		// edge right - edge left
		int16 dx = er - el;

		if (!(dx <= 0)) {
			// screen offset
			int32 sl = el + MAXX * y;

			int16 *zBufferPtr = _zBuf + (y - _zBufStartY) * _zBufWid + (el - _zBufStartX);
			uint16 *screenPtr = _curPage + sl;

			// loop through every pixel in horizontal scanline
			while (dx) {
				if (*zBufferPtr != zv) {
					*screenPtr = g_vm->_graphicsMgr->shadow(*screenPtr, cv);
					*zBufferPtr = (int16)zv;
				}
				screenPtr++;		// increase screen x
				zBufferPtr++;		// increase zbuffer
				dx--;		// pixel to do --
			}
		}
	}
}

void shadowScanEdge(int32 x1, int32 y1, int32 x2, int32 y2) {
	// make sure that edge goes from top to bottom
	int16 dy = y2 - y1;
	if (dy < 0) {
		SWAP(y1, y2);
		SWAP(x1, x2);

		dy = -dy;
	}

	if (dy == 0)
		dy = 1;

	// initialize for stepping
	int32 mx = ((x2 - x1) << 16) / dy; // slope dx/dy

	x1 <<= 16; // starting x coordinate

	// step through edge and record color values along the way
	for (int16 count = y1; count < y2; count++) {
		int16 x = (uint16)(x1 >> 16);
		if (x < _lEdge[count])
			_lEdge[count] = x;

		if (x > _rEdge[count])
			_rEdge[count] = x;

		x1 += mx;  // x = x + dx/dy
	}
}

void init3DRoom(uint16 *destBuffer, int16 *zBuffer) {
	_curPage = destBuffer;
	_zBuf = zBuffer;
	_cx = (MAXX - 1) / 2;
	_cy = (MAXY - 1) / 2;
}

void setClipping(int16 x1, int16 y1, int16 x2, int16 y2) {
	_minXClip = x1;
	_minYClip = y1;
	_maxXClip = x2;
	_maxYClip = y2;
}

void setZBufferRegion(int16 sx, int16 sy, int16 dx) {
	_zBufStartX = sx;
	_zBufStartY = sy;
	_zBufWid    = dx;
}

/*------------------------------------------------
	Determines whether a triangle has clockwise
	or counterclockwise vertices
--------------------------------------------------*/
int8 clockWise(int16 x1, int16 y1, int16 x2, int16 y2, int16 x3, int16 y3) {
	x2 -= x1;
	y2 -= y1;

	x3 -= x1;
	y3 -= y1;

	int32 a1 = ((int32)x2) * y3;
	int32 a2 = ((int32)y2) * x3;

	if (a1 > a2)
		return  1;		// clockwise
	if (a1 < a2)
		return -1;    	// counterclockwise

	a1 = ((int32)x2) * x3;
	a2 = ((int32)y2) * y3;
	if (a1 < 0 || a2 < 0)
		return -1;

	a1 = ((int32)x2) * x2 + ((int32)y2) * y2;
	a2 = ((int32)x3) * x3 + ((int32)y3) * y3;
	if (a1 < a2)
		return 1;

	return 0;
}


void drawCharacter(uint8 flag) {
	// Compute pointer to frame
	if (flag & CALCPOINTS) {
		if (g_vm->_actor->_curAction > hLAST)
			error("Error in drawCharacter() - _curAction > hLAST");

		int cfp = 0;
		int cur = 0;
		while (cur < g_vm->_actor->_curAction)
			cfp += _defActionLen[cur++];

		if (g_vm->_actor->_curAction == hWALKOUT)
			cfp = 1;

		cfp += g_vm->_actor->_curFrame;

		if (g_vm->_actor->_curAction == hLAST)
			cfp = 0;

		g_vm->_actor->_vertex = &g_vm->_actor->_characterArea[cfp * g_vm->_actor->_vertexNum];
	}

	SCamera  *_curCamera = g_vm->_actor->_camera;
	SLight   *_curLight = g_vm->_actor->_light;
	STexture *_curTexture = g_vm->_actor->_texture;
	SVertex  *_curVertex = g_vm->_actor->_vertex;
	SFace    *_curFace = g_vm->_actor->_face;

	int vertexNum = g_vm->_actor->_vertexNum;

	if (flag & CALCPOINTS) {
		_shadowLightNum = 0;
		_totalShadowVerts = 0;

		// camera matrix
		float e10 = _curCamera->_e1[0];
		float e11 = _curCamera->_e1[1];
		float e12 = _curCamera->_e1[2];

		float e20 = _curCamera->_e2[0];
		float e21 = _curCamera->_e2[1];
		float e22 = _curCamera->_e2[2];

		float e30 = _curCamera->_e3[0];
		float e31 = _curCamera->_e3[1];
		float e32 = _curCamera->_e3[2];

		// Light directions
		float l0 = 0.0;
		float l1 = 0.0;
		float l2 = 0.0;

		g_vm->_actor->_lim[0] = 32000;
		g_vm->_actor->_lim[1] = -32000;
		g_vm->_actor->_lim[2] = 32000;
		g_vm->_actor->_lim[3] = -32000;
		g_vm->_actor->_lim[4] = 32000;
		g_vm->_actor->_lim[5] = -32000;

		float t = (g_vm->_actor->_theta * PI2) / 360.0;
		float cost = cos(t);
		float sint = sin(t);

		// Put all vertices in dark color
		for (int a = 0; a < vertexNum; a++)
			_vVertex[a]._angle = 180;

		float dist;
		float tx = 0;
		float ty = 0;
		float tz = 0;
		float pa0, pa1, pa2;

		for (int b = 0; b < g_vm->_actor->_lightNum; b++) {
			// if off                lint == 0
			// if it has a shadow    lint & 0x80

			int lint = _curLight->_inten & 0x7F;
			if (lint) {    // if it's not turned off
				tx = _curLight->_x - g_vm->_actor->_px - g_vm->_actor->_dx; // computes direction vector
				tz = _curLight->_z - g_vm->_actor->_pz - g_vm->_actor->_dz; // between light and actor
				ty = _curLight->_y;

				if (_curLight->_position) {     // if it's attenuated
					dist = sqrt(tx * tx + ty * ty + tz * tz);   // Distance light <--> actor

					// adjust light intensity due to the distance
					if (dist > _curLight->_outr)           // if it's out of range it's off
						lint = 0;
					else if (dist > _curLight->_inr)       // if it's inside the circle it's decreased
						lint = (int)((float)lint * (_curLight->_outr - dist) / (_curLight->_outr - _curLight->_inr));
				}
			}

			if (lint) {   // If it's still on
				// Light rotates around the actor
				l0 = tx * cost - tz * sint;
				l2 = tx * sint + tz * cost;
				l1 = ty;
				t  = sqrt(l0 * l0 + l1 * l1 + l2 * l2);
				l0 /= t;
				l1 /= t;
				l2 /= t;

				// Adjust light intensity according to the spot
				tx = (float)_curLight->_fallOff;
				if (tx) {    // for light spot only
					ty = (float)_curLight->_hotspot;

					pa0 = _curLight->_dx * cost - _curLight->_dz * sint;
					pa1 = _curLight->_dy;
					pa2 = _curLight->_dx * sint + _curLight->_dz * cost;

					t = sqrt(pa0 * pa0 + pa1 * pa1 + pa2 * pa2);
					pa0 /= t;
					pa1 /= t;
					pa2 /= t;

					tz = acos((pa0 * l0) + (pa1 * l1) + (pa2 * l2)) * 360.0 / PI2;
					tz = CLIP(tz, 0.f, 180.f);

					// tx falloff
					// ty hotspot
					// tz current angle

					_shadowIntens[_shadowLightNum] = SHADOWAMBIENT;

					if (tz > tx) {           // if it's out of the falloff
						lint = 0;
						_shadowIntens[_shadowLightNum] = 0;
					} else if (tz > ty) {    // if it's between the falloff and the hotspot
						lint = (int)((float)lint * (tx - tz) / (tx - ty));
						_shadowIntens[_shadowLightNum] = (int)((float)_shadowIntens[_shadowLightNum] * (tx - tz) / (tx - ty));
					}
				}
			}

			if ((_curLight->_inten & 0x80) && lint) {    // if it's shadowed and still on
				_curVertex = g_vm->_actor->_vertex;

				// casts shadow vertices
				for (int a = 0; a < _shadowVertsNum; a++) {
					pa0 = _curVertex[_shadowVerts[a]]._x;
					pa1 = _curVertex[_shadowVerts[a]]._y;
					pa2 = _curVertex[_shadowVerts[a]]._z;

					_shVertex[vertexNum + _totalShadowVerts + a]._x = pa0 - (pa1 * l0);
					_shVertex[vertexNum + _totalShadowVerts + a]._z = pa2 - (pa1 * l2);
					_shVertex[vertexNum + _totalShadowVerts + a]._y = 0;
				}

				// per default all shadows are equally faint
				// _shadowIntens[_shadowLightNum] = SHADOWAMBIENT;

				_shadowLightNum++;
				_totalShadowVerts += _shadowVertsNum;
			}

			if (lint) { // if still on
				// adapts the light vector o its intensity
				t = (float)(lint) / 127.0;
				l0 = l0 * t;
				l1 = l1 * t;
				l2 = l2 * t;

				_curVertex = g_vm->_actor->_vertex;
				for (int a = 0; a < vertexNum; a++) {
					pa0 = _curVertex->_nx;
					pa1 = _curVertex->_ny;
					pa2 = _curVertex->_nz;

					lint = (int)((acos(pa0 * l0 + pa1 * l1 + pa2 * l2) * 360.0) / PI);
					lint = CLIP(lint, 0, 180);

					_vVertex[a]._angle -= (180 - lint);
					_curVertex++;
				}
			}

			_curLight++;
		}

		// rearranged light values so they can be viewed
		for (int a = 0; a < vertexNum; a++)
			_vVertex[a]._angle = CLIP(_vVertex[a]._angle, 0, 180);

		_curVertex = g_vm->_actor->_vertex;

		// Calculate the distance of the character from the room
		tx = _curCamera->_ex - g_vm->_actor->_px;
		ty = _curCamera->_ey - g_vm->_actor->_py;
		tz = _curCamera->_ez - g_vm->_actor->_pz;

		dist = tx * e30 + ty * e31 + tz * e32;

		for (int a = 0; a < vertexNum + _totalShadowVerts; a++) {
			if (a < vertexNum) {
				l0 = _curVertex->_x;
				l1 = _curVertex->_z;
				pa1 = ty - _curVertex->_y;
			} else {
				l0 = _shVertex[a]._x;
				l1 = _shVertex[a]._z;
				pa1 = ty - _shVertex[a]._y;
			}

			pa0 = tx - (l0 * cost + l1 * sint);     // rotate _curVertex
			pa2 = tz - (-l0 * sint + l1 * cost);

			l0 = pa0 * e10 + pa1 * e11 + pa2 * e12; // project _curVertex
			l1 = pa0 * e20 + pa1 * e21 + pa2 * e22;
			l2 = pa0 * e30 + pa1 * e31 + pa2 * e32;

			_x2d = _cx + (int)((l0 * _curCamera->_fovX) / l2);
			_y2d = _cy + (int)((l1 * _curCamera->_fovY) / l2);

			_vVertex[a]._x = (short)_x2d;
			_vVertex[a]._y = (short)_y2d;
			_vVertex[a]._z = (short)((dist - l2) * 128.0);

			g_vm->_actor->_lim[0] = MIN(_x2d, g_vm->_actor->_lim[0]);
			g_vm->_actor->_lim[1] = MAX(_x2d, g_vm->_actor->_lim[1]);
			g_vm->_actor->_lim[2] = MIN(_y2d, g_vm->_actor->_lim[2]);
			g_vm->_actor->_lim[3] = MAX(_y2d, g_vm->_actor->_lim[3]);

			g_vm->_actor->_lim[4] = MIN(_vVertex[a]._z, g_vm->_actor->_lim[4]);
			g_vm->_actor->_lim[5] = MAX(_vVertex[a]._z, g_vm->_actor->_lim[5]);

			_curVertex++;
		}
		g_vm->_actor->_lim[4] = (short)dist;
		g_vm->_actor->_lim[5] = (short)dist;

		// vertex clipping
		g_vm->_actor->_lim[0] = (g_vm->_actor->_lim[0] <= _minXClip + 1) ? _minXClip : g_vm->_actor->_lim[0]--;
		g_vm->_actor->_lim[1] = (g_vm->_actor->_lim[1] >= _maxXClip - 1) ? _maxXClip : g_vm->_actor->_lim[1]++;
		g_vm->_actor->_lim[2] = (g_vm->_actor->_lim[2] <= _minYClip + 1) ? _minYClip : g_vm->_actor->_lim[2]--;
		g_vm->_actor->_lim[3] = (g_vm->_actor->_lim[3] >= _maxYClip - 1) ? _maxYClip : g_vm->_actor->_lim[3]++;

		if (g_vm->_actor->_curAction == hLAST) // exit displacer
			g_vm->_actor->_lim[2] = g_vm->_actor->_lim[3] - (((g_vm->_actor->_lim[3] - g_vm->_actor->_lim[2]) * g_vm->_actor->_curFrame) / _defActionLen[hLAST]);

		// set zbuffer vars
		setZBufferRegion(g_vm->_actor->_lim[0], g_vm->_actor->_lim[2], g_vm->_actor->_lim[1] - g_vm->_actor->_lim[0]);
	}

	if (flag & DRAWFACES) {
		if (g_vm->_actor->_curAction == hLAST)
			setClipping(0, g_vm->_actor->_lim[2], MAXX, g_vm->_actor->_lim[3]);

		for (int b = 0; b < _shadowLightNum; b++) {
			for (int a = 0; a < _shadowFacesNum; a++) {
				int p0 = _shadowFaces[a][0] + vertexNum + b * _shadowVertsNum;
				int p1 = _shadowFaces[a][1] + vertexNum + b * _shadowVertsNum;
				int p2 = _shadowFaces[a][2] + vertexNum + b * _shadowVertsNum;

				int px0 = _vVertex[p0]._x;
				int py0 = _vVertex[p0]._y;
				int px1 = _vVertex[p1]._x;
				int py1 = _vVertex[p1]._y;
				int px2 = _vVertex[p2]._x;
				int py2 = _vVertex[p2]._y;

				shadowTriangle(px0, py0, px1, py1, px2, py2, 127 - _shadowIntens[b], (int16)(0x7FF0 + b));
			}
		}

		for (int a = 0; a < g_vm->_actor->_faceNum; a++) {
			int p0 = _curFace->_a;
			int p1 = _curFace->_b;
			int p2 = _curFace->_c;

			int px0 = _vVertex[p0]._x;
			int py0 = _vVertex[p0]._y;
			int px1 = _vVertex[p1]._x;
			int py1 = _vVertex[p1]._y;
			int px2 = _vVertex[p2]._x;
			int py2 = _vVertex[p2]._y;

			if (clockWise(px0, py0, px1, py1, px2, py2) > 0) {
				int b = _curFace->_mat;
				if (_curTexture[b]._flag & TEXTUREACTIVE) {
					textureTriangle(px0, py0, _vVertex[p0]._z, _vVertex[p0]._angle, g_vm->_actor->_textureCoord[a][0][0], g_vm->_actor->_textureCoord[a][0][1],
									px1, py1, _vVertex[p1]._z, _vVertex[p1]._angle, g_vm->_actor->_textureCoord[a][1][0], g_vm->_actor->_textureCoord[a][1][1],
									px2, py2, _vVertex[p2]._z, _vVertex[p2]._angle, g_vm->_actor->_textureCoord[a][2][0], g_vm->_actor->_textureCoord[a][2][1],
									&_curTexture[b]);
				}
			}

			_curFace++;
		}

		int p0 = 0;
		for (int b = _zBufStartY; b < g_vm->_actor->_lim[3]; b++) {
			int px0 = b * MAXX + _zBufStartX;
			for (int a = 1; a < _zBufWid; a++) {
				// CHECKME: These are always 0
				//bool _shadowSplit;
				int py1 = 0;	//(_zBuf[p0]   >= 0x7FF0) * 0x8000 * _shadowSplit;
				int py2 = 0;	//(_zBuf[p0 + 1] >= 0x7FF0) * 0x8000 * _shadowSplit;

				int p1 = _zBuf[p0] < 0x7FFF;
				int p2 = _zBuf[p0 + 1] < 0x7FFF;

				if (p1 != p2) {
					int px1 = _curPage[px0 + a - 1];
					int px2 = _curPage[px0 + a];

					_curPage[px0 + a - 1] = g_vm->_graphicsMgr->aliasing(px1, px2, 6); // 75% 25%
					_curPage[px0 + a] = g_vm->_graphicsMgr->aliasing(px1, px2, 2);     // 25% 75%

					// if the first is the character
					if (p1)
						_zBuf[p0] = 0x00BF | py1;
					else
						_zBuf[p0] = 0x003F | py2;

					if (a + 1 < _zBufWid) {
						p0++;
						a++;

						// if the second is the character
						if (p2)
							_zBuf[p0] = 0x00BF | py2;
						else
							_zBuf[p0] = 0x003F | py1;
					}
				} else {
					// set value alpha max
					if (p1)
						_zBuf[p0] = 0x00FF | py1;
					else
						_zBuf[p0] = 0x0000 | py1;
				}

				p0++;

				// if it's the last of the line
				if (a == _zBufWid - 1) {
					if (p2)
						_zBuf[p0] = 0x00FF | py2;
					else
						_zBuf[p0] = 0x0000 | py2;
				}
			}
			p0++;
		}
		if (g_vm->_actor->_curAction == hLAST)
			setClipping(0, TOP, MAXX, AREA + TOP);
	}
}

} // End of namespace Trecision
