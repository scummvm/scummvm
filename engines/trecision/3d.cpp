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

#include "trecision/trecision.h"
#include "trecision/3d.h"

#include "trecision/actor.h"
#include "trecision/graphics.h"

namespace Trecision {
#define SHADOWVERTSNUM 42

static int16 _shadowVerts[SHADOWVERTSNUM] = {
	6, 15, 23,
	24, 32, 78,
	80, 81, 83,
	86, 90, 99,
	107, 108, 116,
	155, 157, 158,
	160, 164, 168,
	169, 173, 174,
	187, 188, 192,
	193, 213, 215,
	227, 229, 235,
	238, 249, 250,
	252, 253, 299,
	306, 330, 336
};
#define SHADOWFACESNUM 48

int16 _shadowFaces[SHADOWFACESNUM][3] = {
	22, 21, 5,
	7, 5, 22,
	7, 19, 5,
	5, 2, 19,
	27, 24, 16,
	27, 16, 18,
	18, 16, 9,
	18, 13, 9,
	13, 9, 2,
	3, 19, 12,
	25, 26, 17,
	17, 15, 25,
	17, 19, 15,
	15, 12, 19,
	20, 23, 8,
	8, 6, 20,
	6, 9, 3,
	3, 8, 6,
	12, 3, 4,
	4, 11, 12,
	35, 4, 11,
	13, 2, 1,
	1, 14, 13,
	14, 37, 1,
	1, 34, 37,
	31, 36, 37,
	37, 30, 31,
	29, 34, 35,
	35, 29, 28,
	36, 11, 31,
	30, 37, 14,
	29, 1, 34,
	28, 4, 35,
	36, 10, 35,
	35, 32, 10,
	37, 0, 34,
	37, 33, 0,
	0, 33, 39,
	39, 40, 0,
	10, 38, 32,
	32, 41, 38,
	36, 35, 34,
	36, 37, 35,
	11, 36, 35,
	38, 40, 41,
	41, 38, 39,
	2, 19, 13,
	3, 9, 12
};

Renderer3D::Renderer3D(TrecisionEngine *vm) : _vm(vm) {
	_zBuf = nullptr;
	_curPage = nullptr;

	_minXClip = 0;
	_minYClip = 0;
	_maxXClip = 0;
	_maxYClip = 0;
	_zBufStartX = 0;
	_zBufStartY = 0;
	_zBufWid = 0;
	_shadowLightNum = 0;
	_totalShadowVerts = 0;

	// data for the triangle routines
	for (int i = 0; i < 480; ++i) {
		_lEdge[i] = 0;
		_rEdge[i] = 0;
		_lColor[i] = 0;
		_rColor[i] = 0;
		_lZ[i] = 0;
		_rZ[i] = 0;
		_lTextX[i] = 0;
		_rTextX[i] = 0;
		_lTextY[i] = 0;
		_rTextY[i] = 0;
	}

	for (int i = 0; i < 10; ++i)
		_shadowIntens[i] = 0;

	for (int i = 0; i < MAXVERTEX; ++i) {
		_vVertex[i]._x = 0;
		_vVertex[i]._y = 0;
		_vVertex[i]._z = 0;
		_vVertex[i]._angle = 0;

		_shVertex[i]._x = 0;
		_shVertex[i]._y = 0;
		_shVertex[i]._z = 0;
		_shVertex[i]._nx = 0;
		_shVertex[i]._ny = 0;
		_shVertex[i]._nz = 0;
	}
}

Renderer3D::~Renderer3D() {
}

void Renderer3D::textureTriangle(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1,
								 int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2,
								 int32 x3, int32 y3, int32 z3, int32 c3, int32 tx3, int32 ty3,
								 STexture *t) {
	int32 cl;  // color of left edge of horizontal scanline
	int32 zl;  // zbuffer of left edge of horizontal scanline
	int32 olx; // texture x of left edge of horizontal scanline
	int32 oly; // texture y of left edge of horizontal scanline
	int16 y;   // looping variable

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
			cl <<= 8;
			olx <<= 16;
			oly <<= 16;
			// loop through every pixel in horizontal scanline
			while (dx) {
				sl = zl >> 16;
				if (*z > sl) {
					*screenPtr = (uint16)(_vm->_actor->_textureMat[texture[(olx >> 16) + t->_dx * (oly >> 16)]][cl >> 9]);
					*z = (int16)sl;
				}
				screenPtr++; // increase screen x
				z++;         // increase zbuffer
				zl += mz;    // increase the zbuffer by _dz/_dx
				cl += mc;    // increase the color by dc/_dx
				olx += mtx;
				oly += mty;
				dx--; // pixel to do --
			}
		}
	}
}

void Renderer3D::textureScanEdge(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2) {
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
	int32 mc = ((c2 - c1) << 8) / dy;  // dc/dy
	int32 mtx = ((tx2 - tx1) << 16) / dy;
	int32 mty = ((ty2 - ty1) << 16) / dy;

	x1 <<= 16; // starting x coordinate
	z1 <<= 16; // starting z coordinate
	c1 <<= 8;  // starting c color

	tx1 <<= 16;
	ty1 <<= 16;

	// step through edge and record color values along the way
	for (int32 count = y1; count < y2; count++) {
		int16 x = (uint16)(x1 >> 16);
		if (x < _lEdge[count]) {
			_lEdge[count] = x;
			_lZ[count] = (int16)(z1 >> 16);
			_lTextX[count] = (uint16)(tx1 >> 16);
			_lTextY[count] = (uint16)(ty1 >> 16);
			_lColor[count] = (uint8)(c1 >> 8);
		}
		if (x > _rEdge[count]) {
			_rEdge[count] = x;
			_rZ[count] = (int16)(z1 >> 16);
			_rTextX[count] = (uint16)(tx1 >> 16);
			_rTextY[count] = (uint16)(ty1 >> 16);
			_rColor[count] = (uint8)(c1 >> 8);
		}

		x1 += mx; // x = x + dx/dy
		c1 += mc; // c = c + dc/dy
		z1 += mz; // z = z + dz/dy

		tx1 += mtx;
		ty1 += mty;
	}
}

void Renderer3D::shadowTriangle(int32 x1, int32 y1, int32 x2, int32 y2,
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
					*screenPtr = _vm->_graphicsMgr->shadow(*screenPtr, cv);
					*zBufferPtr = zv;
				}
				screenPtr++;  // increase screen x
				zBufferPtr++; // increase zbuffer
				dx--;         // pixel to do --
			}
		}
	}
}

void Renderer3D::shadowScanEdge(int32 x1, int32 y1, int32 x2, int32 y2) {
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
	for (int32 count = y1; count < y2; count++) {
		int16 x = (int16)(x1 >> 16);
		if (x < _lEdge[count])
			_lEdge[count] = x;

		if (x > _rEdge[count])
			_rEdge[count] = x;

		x1 += mx; // x = x + dx/dy
	}
}

/*------------------------------------------------
	Initialize a 3D Room
--------------------------------------------------*/
void Renderer3D::init3DRoom(uint16 *destBuffer, int16 *zBuffer) {
	_curPage = destBuffer;
	_zBuf = zBuffer;
	_cx = (MAXX - 1) / 2;
	_cy = (MAXY - 1) / 2;
}

/*------------------------------------------------
	Change the clipping area
--------------------------------------------------*/
void Renderer3D::setClipping(int16 x1, int16 y1, int16 x2, int16 y2) {
	_minXClip = x1;
	_minYClip = y1;
	_maxXClip = x2;
	_maxYClip = y2;
}

void Renderer3D::setZBufferRegion(int16 sx, int16 sy, int16 dx) {
	_zBufStartX = sx;
	_zBufStartY = sy;
	_zBufWid = dx;
}

/*------------------------------------------------
	Determines whether a triangle has clockwise
	or counterclockwise vertices
--------------------------------------------------*/
int8 Renderer3D::clockWise(int16 x1, int16 y1, int16 x2, int16 y2, int16 x3, int16 y3) {
	x2 -= x1;
	y2 -= y1;

	x3 -= x1;
	y3 -= y1;

	int32 a1 = ((int32)x2) * y3;
	int32 a2 = ((int32)y2) * x3;

	if (a1 > a2)
		return 1; // clockwise
	if (a1 < a2)
		return -1; // counterclockwise

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

/*------------------------------------------------
	Draw the character
--------------------------------------------------*/
void Renderer3D::drawCharacter(uint8 flag) {
	Actor *actor = _vm->_actor;

	// Compute pointer to frame
	if (flag & CALCPOINTS) {
		if (actor->_curAction > hLAST)
			error("Error in drawCharacter() - _curAction > hLAST");

		int cfp = 0;
		int cur = 0;
		while (cur < actor->_curAction)
			cfp += _defActionLen[cur++];

		if (actor->_curAction == hWALKOUT)
			cfp = 1;

		cfp += actor->_curFrame;

		if (actor->_curAction == hLAST)
			cfp = 0;

		actor->_vertex = &actor->_characterArea[cfp * actor->_vertexNum];
	}

	SCamera *_curCamera = actor->_camera;
	SLight *_curLight = actor->_light;
	STexture *_curTexture = actor->_texture;
	SVertex *_curVertex = actor->_vertex;
	SFace *_curFace = actor->_face;

	int vertexNum = actor->_vertexNum;

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

		actor->_lim[0] = 32000;
		actor->_lim[1] = -32000;
		actor->_lim[2] = 32000;
		actor->_lim[3] = -32000;
		actor->_lim[4] = 32000;
		actor->_lim[5] = -32000;

		float t = (actor->_theta * PI2) / 360.0;
		float cost = cos(t);
		float sint = sin(t);

		// Put all vertices in dark color
		for (int i = 0; i < MAXVERTEX; ++i)
			_vVertex[i]._angle = 180;

		float dist;
		float tx = 0;
		float ty = 0;
		float tz = 0;
		float pa0, pa1, pa2;

		for (uint32 b = 0; b < actor->_lightNum; b++) {
			// if off                lint == 0
			// if it has a shadow    lint & 0x80

			int lint = _curLight->_inten & 0x7F;
			if (lint) {                                       // if it's not turned off
				tx = _curLight->_x - actor->_px - actor->_dx; // computes direction vector
				tz = _curLight->_z - actor->_pz - actor->_dz; // between light and actor
				ty = _curLight->_y;

				if (_curLight->_position) {                   // if it's attenuated
					dist = sqrt(tx * tx + ty * ty + tz * tz); // Distance light <--> actor

					// adjust light intensity due to the distance
					if (dist > _curLight->_outr) // if it's out of range it's off
						lint = 0;
					else if (dist > _curLight->_inr) // if it's inside the circle it's decreased
						lint = (int)((float)lint * (_curLight->_outr - dist) / (_curLight->_outr - _curLight->_inr));
				}
			}

			if (lint) { // If it's still on
				// Light rotates around the actor
				l0 = tx * cost - tz * sint;
				l2 = tx * sint + tz * cost;
				l1 = ty;
				t = sqrt(l0 * l0 + l1 * l1 + l2 * l2);
				l0 /= t;
				l1 /= t;
				l2 /= t;

				// Adjust light intensity according to the spot
				tx = (float)_curLight->_fallOff;
				if (tx) { // for light spot only
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

					if (tz > tx) { // if it's out of the falloff
						lint = 0;
						_shadowIntens[_shadowLightNum] = 0;
					} else if (tz > ty) { // if it's between the falloff and the hotspot
						lint = (int)((float)lint * (tx - tz) / (tx - ty));
						_shadowIntens[_shadowLightNum] = (int)((float)_shadowIntens[_shadowLightNum] * (tx - tz) / (tx - ty));
					}
				}
			}

			if ((_curLight->_inten & 0x80) && lint) { // if it's shadowed and still on
				_curVertex = actor->_vertex;

				// casts shadow vertices
				for (int a = 0; a < SHADOWVERTSNUM; a++) {
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
				_totalShadowVerts += SHADOWVERTSNUM;
			}

			if (lint) { // if still on
				// adapts the light vector o its intensity
				t = (float)(lint) / 127.0;
				l0 = l0 * t;
				l1 = l1 * t;
				l2 = l2 * t;

				_curVertex = actor->_vertex;
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

		_curVertex = actor->_vertex;

		// Calculate the distance of the character from the room
		tx = _curCamera->_ex - actor->_px;
		ty = _curCamera->_ey - actor->_py;
		tz = _curCamera->_ez - actor->_pz;

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

			pa0 = tx - (l0 * cost + l1 * sint); // rotate _curVertex
			pa2 = tz - (-l0 * sint + l1 * cost);

			l0 = pa0 * e10 + pa1 * e11 + pa2 * e12; // project _curVertex
			l1 = pa0 * e20 + pa1 * e21 + pa2 * e22;
			l2 = pa0 * e30 + pa1 * e31 + pa2 * e32;

			_x2d = _cx + (int)((l0 * _curCamera->_fovX) / l2);
			_y2d = _cy + (int)((l1 * _curCamera->_fovY) / l2);

			_vVertex[a]._x = _x2d;
			_vVertex[a]._y = _y2d;
			_vVertex[a]._z = (int32)((dist - l2) * 128.0);

			actor->_lim[0] = MIN(_x2d, actor->_lim[0]);
			actor->_lim[1] = MAX(_x2d, actor->_lim[1]);
			actor->_lim[2] = MIN(_y2d, actor->_lim[2]);
			actor->_lim[3] = MAX(_y2d, actor->_lim[3]);

			actor->_lim[4] = MIN(_vVertex[a]._z, actor->_lim[4]);
			actor->_lim[5] = MAX(_vVertex[a]._z, actor->_lim[5]);

			_curVertex++;
		}
		actor->_lim[4] = (int)dist;
		actor->_lim[5] = (int)dist;

		// vertex clipping
		actor->_lim[0] = (actor->_lim[0] <= _minXClip + 1) ? _minXClip : actor->_lim[0]--;
		actor->_lim[1] = (actor->_lim[1] >= _maxXClip - 1) ? _maxXClip : actor->_lim[1]++;
		actor->_lim[2] = (actor->_lim[2] <= _minYClip + 1) ? _minYClip : actor->_lim[2]--;
		actor->_lim[3] = (actor->_lim[3] >= _maxYClip - 1) ? _maxYClip : actor->_lim[3]++;

		if (actor->_curAction == hLAST) // exit displacer
			actor->_lim[2] = actor->_lim[3] - (((actor->_lim[3] - actor->_lim[2]) * actor->_curFrame) / _defActionLen[hLAST]);

		// set zbuffer vars
		setZBufferRegion(actor->_lim[0], actor->_lim[2], actor->_lim[1] - actor->_lim[0]);
	}

	if (flag & DRAWFACES) {
		if (actor->_curAction == hLAST)
			setClipping(0, actor->_lim[2], MAXX, actor->_lim[3]);

		for (int b = 0; b < _shadowLightNum; b++) {
			for (int a = 0; a < SHADOWFACESNUM; a++) {
				int p0 = _shadowFaces[a][0] + vertexNum + b * SHADOWVERTSNUM;
				int p1 = _shadowFaces[a][1] + vertexNum + b * SHADOWVERTSNUM;
				int p2 = _shadowFaces[a][2] + vertexNum + b * SHADOWVERTSNUM;

				int px0 = _vVertex[p0]._x;
				int py0 = _vVertex[p0]._y;
				int px1 = _vVertex[p1]._x;
				int py1 = _vVertex[p1]._y;
				int px2 = _vVertex[p2]._x;
				int py2 = _vVertex[p2]._y;

				shadowTriangle(px0, py0, px1, py1, px2, py2, 127 - _shadowIntens[b], (int16)(0x7FF0 + b));
			}
		}

		for (uint a = 0; a < actor->_faceNum; ++a) {
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
				uint16 b = _curFace->_mat;
				if (_curTexture[b]._flag & TEXTUREACTIVE) {
					textureTriangle(px0, py0, _vVertex[p0]._z, _vVertex[p0]._angle, actor->_textureCoord[a][0][0], actor->_textureCoord[a][0][1],
									px1, py1, _vVertex[p1]._z, _vVertex[p1]._angle, actor->_textureCoord[a][1][0], actor->_textureCoord[a][1][1],
									px2, py2, _vVertex[p2]._z, _vVertex[p2]._angle, actor->_textureCoord[a][2][0], actor->_textureCoord[a][2][1],
									&_curTexture[b]);
				}
			}

			_curFace++;
		}

		int p0 = 0;
		for (int b = _zBufStartY; b < actor->_lim[3]; b++) {
			int px0 = b * MAXX + _zBufStartX;
			for (int a = 1; a < _zBufWid; a++) {
				// CHECKME: These are always 0
				//bool _shadowSplit;
				int py1 = 0; //(_zBuf[p0]   >= 0x7FF0) * 0x8000 * _shadowSplit;
				int py2 = 0; //(_zBuf[p0 + 1] >= 0x7FF0) * 0x8000 * _shadowSplit;

				int p1 = _zBuf[p0] < 0x7FFF;
				int p2 = _zBuf[p0 + 1] < 0x7FFF;

				if (p1 != p2) {
					int px1 = _curPage[px0 + a - 1];
					int px2 = _curPage[px0 + a];

					_curPage[px0 + a - 1] = _vm->_graphicsMgr->aliasing(px1, px2, 6); // 75% 25%
					_curPage[px0 + a] = _vm->_graphicsMgr->aliasing(px1, px2, 2);     // 25% 75%

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
		if (actor->_curAction == hLAST)
			setClipping(0, TOP, MAXX, AREA + TOP);
	}
}

// Path Finding
PathFinding3D::PathFinding3D(TrecisionEngine *vm) : _vm(vm) {
	_lookX = 0.0f;
	_lookZ = 0.0f;
	_curStep = 0;
	_lastStep = 0;

	_characterInMovement = false;
	_characterGoToPosition = -1;
}

PathFinding3D::~PathFinding3D() {
}

void PathFinding3D::findPath() {
	int b;

	Actor *actor = _vm->_actor;
	actor->_px += actor->_dx;
	actor->_pz += actor->_dz;

	int inters = 0;
	_numPathNodes = 0;

	// if you have clicked behind the starting panel or the corner it's not possible to walk
	if ((_curPanel < 0) && (_oldPanel >= 0) &&
		// behind the starting panel
		(pointInside(b = _oldPanel, _curX, _curZ) ||
		 // behind the panel corner1
		 ((_vm->dist2D(_panel[_oldPanel]._x1, _panel[_oldPanel]._z1, actor->_px, actor->_pz) < EPSILON) &&
		  (pointInside(b = _panel[_oldPanel]._near1, _curX, _curZ) ||
		   pointInside(b = _panel[_oldPanel]._near2, _curX, _curZ))) ||
		 // behind the panel corner2
		 ((_vm->dist2D(_panel[_oldPanel]._x2, _panel[_oldPanel]._z2, actor->_px, actor->_pz) < EPSILON) &&
		  (pointInside(b = _panel[_oldPanel]._near2, _curX, _curZ) ||
		   pointInside(b = _panel[_oldPanel]._near1, _curX, _curZ))))) {
		_curX = actor->_px;
		_curZ = actor->_pz;
		actor->_px -= actor->_dx;
		actor->_pz -= actor->_dz;
		_curPanel = b;
		_numPathNodes = 0;
		lookAt(_lookX, _lookZ);
		return;
	}

	float dist = _vm->dist2D(actor->_px, actor->_pz, _curX, _curZ);

	for (b = 0; b < _panelNum; b++) {
		if (_panel[b]._flags & 0x80000000) { // it must be a wide panel
			if (intersectLineLine(_panel[b]._x1, _panel[b]._z1,
								  _panel[b]._x2, _panel[b]._z2,
								  actor->_px, actor->_pz, _curX, _curZ)) {
				inters++;

				_pathNode[_numPathNodes]._x = _x3d;
				_pathNode[_numPathNodes]._z = _z3d;
				_pathNode[_numPathNodes]._dist = _vm->dist2D(actor->_px, actor->_pz, _x3d, _z3d);
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
						if ((_curPanel < 0) && (pointInside(b, _curX, _curZ))) {
							_curX = actor->_px;
							_curZ = actor->_pz;
							actor->_px -= actor->_dx;
							actor->_pz -= actor->_dz;

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

				_pathNode[_numPathNodes]._x = actor->_px;
				_pathNode[_numPathNodes]._z = actor->_pz;
				_pathNode[_numPathNodes]._dist = 0.0;
				_pathNode[_numPathNodes]._oldp = _oldPanel;
				_pathNode[_numPathNodes]._curp = _oldPanel;
				_numPathNodes++;
			} else if (b == _curPanel) {
				inters++;

				_pathNode[_numPathNodes]._x = _curX;
				_pathNode[_numPathNodes]._z = _curZ;
				_pathNode[_numPathNodes]._dist = dist;
				_pathNode[_numPathNodes]._oldp = _curPanel;
				_pathNode[_numPathNodes]._curp = _curPanel;
				_numPathNodes++;
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
			  pointInside(_pathNode[_numPathNodes - 1]._curp, _curX, _curZ)))) {

			_curPanel = _pathNode[_numPathNodes - 1]._curp;

			pointOut(); // remove the point found

			_pathNode[_numPathNodes]._x = _curX;
			_pathNode[_numPathNodes]._z = _curZ;
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
						if ((_vm->dist2D(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON) &&
							(_vm->dist2D(_x3d, _z3d, _curX, _curZ) > EPSILON))
							inters++;
				} else {
					if (intersectLineLine(_panel[b]._x1, _panel[b]._z1,
										  _panel[_panel[b]._col1 & 0x7F]._x1, _panel[_panel[b]._col1 & 0x7F]._z1,
										  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
										  _curX, _curZ))
						if ((_vm->dist2D(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON) &&
							(_vm->dist2D(_x3d, _z3d, _curX, _curZ) > EPSILON))
							inters++;
				}

				if (_panel[b]._col2 & 0x80) {
					if (intersectLineLine(_panel[b]._x2, _panel[b]._z2,
										  _panel[_panel[b]._col2 & 0x7F]._x2, _panel[_panel[b]._col2 & 0x7F]._z2,
										  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
										  _curX, _curZ))
						if ((_vm->dist2D(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON) &&
							(_vm->dist2D(_x3d, _z3d, _curX, _curZ) > EPSILON))
							inters++;
				} else {
					if (intersectLineLine(_panel[b]._x2, _panel[b]._z2,
										  _panel[_panel[b]._col2 & 0x7F]._x1, _panel[_panel[b]._col2 & 0x7F]._z1,
										  _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z,
										  _curX, _curZ))
						if ((_vm->dist2D(_x3d, _z3d, _pathNode[_numPathNodes - 1]._x, _pathNode[_numPathNodes - 1]._z) > EPSILON) &&
							(_vm->dist2D(_x3d, _z3d, _curX, _curZ) > EPSILON))
							inters++;
				}
			}

			if (inters)
				break;
		}

		// If in the last line there's an obstacle, remove the first node
		if (inters) {
			_curPanel = _pathNode[_numPathNodes - 1]._curp;

			pointOut(); // take out the point found

			_pathNode[_numPathNodes]._x = _curX;
			_pathNode[_numPathNodes]._z = _curZ;
			_pathNode[_numPathNodes]._oldp = _curPanel;
			_pathNode[_numPathNodes]._curp = _curPanel;

			_numPathNodes++;
		}

		_pathNode[_numPathNodes]._x = _curX;
		_pathNode[_numPathNodes]._z = _curZ;
		_pathNode[_numPathNodes]._dist = _vm->dist2D(actor->_px, actor->_pz, _curX, _curZ);
		_pathNode[_numPathNodes]._oldp = _curPanel;
		_pathNode[_numPathNodes]._curp = _curPanel;
		_numPathNodes++;

		findShortPath();
		displayPath();
	} else { // otherwise if it's direct
		_pathNode[_numPathNodes]._x = actor->_px;
		_pathNode[_numPathNodes]._z = actor->_pz;
		_pathNode[_numPathNodes]._dist = 0.0;
		_pathNode[_numPathNodes]._oldp = _oldPanel;
		_pathNode[_numPathNodes]._curp = _oldPanel;
		_numPathNodes++;

		_pathNode[_numPathNodes]._x = _curX;
		_pathNode[_numPathNodes]._z = _curZ;
		_pathNode[_numPathNodes]._dist = _vm->dist2D(actor->_px, actor->_pz, _curX, _curZ);
		_pathNode[_numPathNodes]._oldp = _curPanel;
		_pathNode[_numPathNodes]._curp = _curPanel;
		_numPathNodes++;

		displayPath();
	}

	actor->_px -= actor->_dx;
	actor->_pz -= actor->_dz;
}
/*------------------------------------------------
  Look for the shorter route avoiding obstacle
--------------------------------------------------*/
void PathFinding3D::findShortPath() {
	SPathNode TempPath[MAXPATHNODES];
	float len1, len2;
	int curp, nearp, oldp;
	float destx, destz;
	signed int a, b, c, fail = 0;

	int count = 0;
	// Add departure
	TempPath[count]._x = _vm->_actor->_px;
	TempPath[count]._z = _vm->_actor->_pz;
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
		len1 = evalPath(a, _panel[curp]._x1, _panel[curp]._z1, _panel[curp]._near1) + _vm->dist2D(_pathNode[a]._x, _pathNode[a]._z, _panel[curp]._x1, _panel[curp]._z1);

		// go around obstacle starting with _near2
		len2 = evalPath(a, _panel[curp]._x2, _panel[curp]._z2, _panel[curp]._near2) + _vm->dist2D(_pathNode[a]._x, _pathNode[a]._z, _panel[curp]._x2, _panel[curp]._z2);

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
					fail = 1; // stop at the edge first
					break;    // and stop walking
				}

				// otherwise go to the next panel

				if (_panel[nearp]._near1 == curp) {
					// go to summit 2 next time
					curx = destx;
					curz = destz;

					destx = _panel[nearp]._x2;
					destz = _panel[nearp]._z2;

					oldp = curp;
					curp = nearp;
					nearp = _panel[curp]._near2;
				} else {
					// go to summit 1 next time
					curx = destx;
					curz = destz;

					destx = _panel[nearp]._x1;
					destz = _panel[nearp]._z1;

					oldp = curp;
					curp = nearp;
					nearp = _panel[curp]._near1;
				}

				b++;
			}
		} else {
			fail = 1;
		}

		if (fail) // if it failed to go around the obstacle, stop
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
			if (_vm->dist2D(TempPath[b]._x, TempPath[b]._z, TempPath[a]._x, TempPath[a]._z) < EPSILON)
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
							len2 = _vm->dist2D(_x3d, _z3d, TempPath[a]._x, TempPath[a]._z);
							len1 = _vm->dist2D(_x3d, _z3d, TempPath[b]._x, TempPath[b]._z);

							// intersect at a point distant from the start and the finish
							if ((len1 > EPSILON) && (len2 > EPSILON))
								inters++;
						}
					} else {
						if (intersectLineLine(_panel[c]._x1, _panel[c]._z1,
											  _panel[_panel[c]._col1 & 0x7F]._x1, _panel[_panel[c]._col1 & 0x7F]._z1,
											  TempPath[a]._x, TempPath[a]._z,
											  TempPath[b]._x, TempPath[b]._z)) {
							len2 = _vm->dist2D(_x3d, _z3d, TempPath[a]._x, TempPath[a]._z);
							len1 = _vm->dist2D(_x3d, _z3d, TempPath[b]._x, TempPath[b]._z);

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
							len2 = _vm->dist2D(_x3d, _z3d, TempPath[a]._x, TempPath[a]._z);
							len1 = _vm->dist2D(_x3d, _z3d, TempPath[b]._x, TempPath[b]._z);

							// intersect at a point distant from the start and the finish
							if ((len1 > EPSILON) && (len2 > EPSILON))
								inters++;
						}
					} else {
						if (intersectLineLine(_panel[c]._x2, _panel[c]._z2,
											  _panel[_panel[c]._col2 & 0x7F]._x1, _panel[_panel[c]._col2 & 0x7F]._z1,
											  TempPath[a]._x, TempPath[a]._z,
											  TempPath[b]._x, TempPath[b]._z)) {
							len2 = _vm->dist2D(_x3d, _z3d, TempPath[a]._x, TempPath[a]._z);
							len1 = _vm->dist2D(_x3d, _z3d, TempPath[b]._x, TempPath[b]._z);

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
					TempPath[b]._oldp = -1;                  // destination
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
float PathFinding3D::evalPath(int a, float destX, float destZ, int nearP) {
	int b = 0;
	float len = 0.0;

	int curp = _pathNode[a]._curp;
	float curx = _pathNode[a]._x;
	float curz = _pathNode[a]._z;

	for (;;) {
		// if the point is reached, stop
		if (curp == _pathNode[a + 1]._curp) {
			len += _vm->dist2D(curx, curz, _pathNode[a + 1]._x, _pathNode[a + 1]._z);
			break;
		}

		// if it's back to the starting plane, there's no route
		if (((curp == _pathNode[a]._curp) && b) || (b > _panelNum)) {
			len += 32000.0; // Absurd length
			break;
		}

		// Otherwise it goes to the next plane

		// if nearP is attached to curp via vertex1
		if (_panel[nearP]._near1 == curp) {
			// go to vertex 2 next time
			len += _vm->dist2D(curx, curz, destX, destZ);

			curx = destX;
			curz = destZ;

			destX = _panel[nearP]._x2;
			destZ = _panel[nearP]._z2;

			curp = nearP;
			nearP = _panel[curp]._near2;
		} else {
			// go to vertex 1 newt time
			len += _vm->dist2D(curx, curz, destX, destZ);

			curx = destX;
			curz = destZ;

			destX = _panel[nearP]._x1;
			destZ = _panel[nearP]._z1;

			curp = nearP;
			nearP = _panel[curp]._near1;
		}

		b++;
	}

	return len;
}

/*------------------------------------------------
	Check if a point is inside a panel
--------------------------------------------------*/
bool PathFinding3D::pointInside(int pan, float x, float z) {
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

void PathFinding3D::setPosition(int num) {
	SLight *curLight = _vm->_actor->_light;

	for (uint32 a = 0; a < _vm->_actor->_lightNum; a++) {
		// If it's off
		if (curLight->_inten == 0) {
			// If it's the required position
			if (curLight->_position == num) {
				_vm->_actor->_px = curLight->_x;
				_vm->_actor->_pz = curLight->_z;
				_vm->_actor->_dx = 0.0;
				_vm->_actor->_dz = 0.0;

				float ox = curLight->_dx;
				float oz = curLight->_dz;

				// If it's a null light
				if ((ox == 0.0) && (oz == 0.0))
					warning("setPosition: Unknown error : null light");

				float t = sqrt(ox * ox + oz * oz);
				ox /= t;
				oz /= t;

				float theta = _vm->sinCosAngle(ox, oz) * 180.0f / PI;
				if (theta >= 360.0)
					theta -= 360.0;
				if (theta < 0.0)
					theta += 360.0;

				_vm->_actor->_theta = theta;

				_curStep = 0;
				_lastStep = 0;
				_curPanel = -1;
				_oldPanel = -1;

				_step[0]._px = _vm->_actor->_px + _vm->_actor->_dx;
				_step[0]._pz = _vm->_actor->_pz + _vm->_actor->_dz;
				_step[0]._dx = 0.0;
				_step[0]._dz = 0.0;

				_step[0]._theta = _vm->_actor->_theta;
				_step[0]._curAction = hSTAND;
				_step[0]._curFrame = 0;
				_step[0]._curPanel = _curPanel;

				_characterGoToPosition = num;
				return;
			}
		}

		curLight++;
	}
}

void PathFinding3D::goToPosition(int num) {
	SLight *_curLight = _vm->_actor->_light;

	for (uint32 a = 0; a < _vm->_actor->_lightNum; a++) {
		// If it's off and if it's a position
		if (_curLight->_inten == 0) {
			// If it's the right position
			if (_curLight->_position == num) {
				_curX = _curLight->_x;
				_curZ = _curLight->_z;
				_lookX = _curX - _curLight->_dx;
				_lookZ = _curZ - _curLight->_dz;

				_curStep = 0;
				_lastStep = 0;

				_step[0]._px = _vm->_actor->_px + _vm->_actor->_dx;
				_step[0]._pz = _vm->_actor->_pz + _vm->_actor->_dz;
				_step[0]._dx = 0.0;
				_step[0]._dz = 0.0;

				_step[0]._theta = _vm->_actor->_theta;
				_step[0]._curAction = hSTAND;
				_step[0]._curFrame = 0;
				_step[0]._curPanel = _curPanel;

				_oldPanel = _curPanel;
				_curPanel = -1;

				findPath();

				_characterGoToPosition = num;
				break;
			}
		}

		_curLight++;
	}
}

void PathFinding3D::lookAt(float x, float z) {
	float ox = _step[_lastStep]._px - x;
	float oz = _step[_lastStep]._pz - z;

	// If the light is null
	if ((ox == 0.0) && (oz == 0.0)) {
		memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
		memcpy(&_step[_lastStep + 2], &_step[_lastStep + 1], sizeof(SStep));
		_lastStep += 2;

		return;
	}

	float t = sqrt(ox * ox + oz * oz);
	ox /= t;
	oz /= t;

	float theta = _vm->sinCosAngle(ox, oz) * 180.0f / PI;
	if (theta >= 360.0f)
		theta -= 360.0f;
	if (theta < 0.0f)
		theta += 360.0f;

	float approx = theta - _step[_lastStep]._theta;

	if ((approx < 30.0f) && (approx > -30.0f))
		approx = 0.0f;
	else if (approx > 180.0f)
		approx = -360.0f + approx;
	else if (approx < -180.0)
		approx = 360.0f + approx;

	approx /= 3.0;

	// Antepenultimate 1/3
	_step[_lastStep]._theta += approx;
	_step[_lastStep]._theta = (_step[_lastStep]._theta > 360.0) ? _step[_lastStep]._theta - 360.0 : (_step[_lastStep]._theta < 0.0) ? _step[_lastStep]._theta + 360.0 : _step[_lastStep]._theta;

	// Penultimate 2/3
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
	_lastStep++;
	_step[_lastStep]._theta += approx;
	_step[_lastStep]._theta = (_step[_lastStep]._theta > 360.0) ? _step[_lastStep]._theta - 360.0 : (_step[_lastStep]._theta < 0.0) ? _step[_lastStep]._theta + 360.0 : _step[_lastStep]._theta;

	// Last right step
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
	_lastStep++;
	_step[_lastStep]._theta = theta;

	//	????
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
	_lastStep++;
	_step[_lastStep]._theta = theta;
}

/*------------------------------------------------
		Build list containing all the frames
--------------------------------------------------*/
void PathFinding3D::buildFramelist() {
	// check that it never crosses or touches a narrow panel
	for (int a = 1; a < _numPathNodes; a++) {
		for (int c = 0; c < _panelNum; c++) {
			// it must never intersect narrow panel
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

	float len = 0.0;
	float curlen = 0.0;

	float ox = _pathNode[0]._x;
	float oz = _pathNode[0]._z;

	for (int a = 1; a < _numPathNodes; a++) {
		len += _vm->dist3D(_pathNode[a]._x, 0.0, _pathNode[a]._z, ox, 0.0, oz);

		ox = _pathNode[a]._x;
		oz = _pathNode[a]._z;
	}
	// total route length calculated - if too small, returns
	if (len < 2.0) {
		lookAt(_lookX, _lookZ);
		return;
	}

	int a = 0;
	// compute offset
	SVertex *v = _vm->_actor->_characterArea;
	float firstframe = FRAMECENTER(v);
	float startpos = 0.0;

	// if he was already walking
	int CurA, CurF, cfp;
	if (_vm->_actor->_curAction == hWALK) {
		// compute current frame
		cfp = _defActionLen[hSTART] + 1 + _vm->_actor->_curFrame;
		v += cfp * _vm->_actor->_vertexNum;

		CurA = hWALK;
		CurF = _vm->_actor->_curFrame;

		// if it wasn't the last frame, take the next step
		if (_vm->_actor->_curFrame < _defActionLen[hWALK] - 1) {
			cfp++;
			CurF++;
			v += _vm->_actor->_vertexNum;
		}
	} else if ((_vm->_actor->_curAction >= hSTOP0) && (_vm->_actor->_curAction <= hSTOP9)) {
		// if he was stopped, starts moving qgqin

		// compute current frame
		CurA = hWALK;
		//o		CurF = _vm->_actor->_curAction - hSTOP1;
		CurF = _vm->_actor->_curAction - hSTOP0;

		cfp = _defActionLen[hSTART] + 1 + CurF;
		v += cfp * _vm->_actor->_vertexNum;
	} else {
		// if he was standing, start working or turn
		oz = 0.0;
		cfp = 1;

		CurA = hSTART;
		CurF = 0;

		// start from the first frame
		v += _vm->_actor->_vertexNum;
	}
	oz = -FRAMECENTER(v) + firstframe;

	// at this point, CurA / _curAction is either hSTART or hWALK

	// until it arrives at the destination
	while (((curlen = oz + FRAMECENTER(v) - firstframe) < len) || (!a)) {
		_step[a]._pz = oz - firstframe; // where to render
		_step[a]._dz = curlen;          // where it is
		_step[a]._curAction = CurA;
		_step[a]._curFrame = CurF;

		a++;
		v += _vm->_actor->_vertexNum;

		CurF++;
		cfp++;

		if (CurF >= _defActionLen[CurA]) {
			if (CurA == hSTART) {
				CurA = hWALK;
				CurF = 0;
				cfp = _defActionLen[hSTART] + 1;

				ox = 0.0;
			} else if (CurA == hWALK) {
				CurA = hWALK;
				CurF = 0;
				cfp = _defActionLen[hSTART] + 1;

				// end walk frame
				ox = FRAMECENTER(v) - firstframe;

				v = &_vm->_actor->_characterArea[cfp * _vm->_actor->_vertexNum];
				ox -= FRAMECENTER(v);
			}

			v = &_vm->_actor->_characterArea[cfp * _vm->_actor->_vertexNum];

			// only if it doesn't end
			if ((oz + ox + FRAMECENTER(v) - firstframe) < len)
				oz += ox;
			else
				break;
		}
	}

	if (!a)
		warning("buildFramelist - Unknown error: step number = 0");

	// After the destination, add the stop frame

	// if he was walking
	if (_step[a - 1]._curAction == hWALK)
		//o		CurA = _step[a-1]._curFrame + hSTOP1;		// stop previous step.
		CurA = _step[a - 1]._curFrame + hSTOP0; // stop previous step.
	else
		CurA = hSTOP0; // stop step 01

	assert(CurA <= hLAST); // _defActionLen below has a size of hLAST + 1

	CurF = 0;

	int b = 0;
	cfp = 0;
	while (b != CurA)
		cfp += _defActionLen[b++];

	v = &_vm->_actor->_characterArea[cfp * _vm->_actor->_vertexNum];

	for (b = 0; b < _defActionLen[CurA]; b++) {
		curlen = oz + FRAMECENTER(v) - firstframe;
		_step[a]._pz = oz - firstframe; // where to render
		_step[a]._dz = curlen;          // where it is
		_step[a]._curAction = CurA;
		_step[a]._curFrame = CurF;

		a++;
		CurF++;
		v += _vm->_actor->_vertexNum;
	}

	// how far is it from the destination?
	float approx = (len - curlen - EPSILON) / (a - 2);
	float theta = 0.0;
	// Adjust all the steps so it arrives exactly where clicked 
	for (b = 1; b < a; b++) {
		// verify there's no reverse step
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

	_lastStep = b; // last step
	_curStep = 0;  // current step

	// now insert exact directions and start and destination points
	b = 0;
	//startpos = _step[0]._pz;

	len = 0.0;
	startpos = 0.0;
	for (a = 0; a < _numPathNodes - 1; a++) {
		curlen = 0.0;
		len += _vm->dist3D(_pathNode[a]._x, 0.0, _pathNode[a]._z,
					  _pathNode[a + 1]._x, 0.0, _pathNode[a + 1]._z);

		// determine the direction
		ox = _pathNode[a + 1]._x - _pathNode[a]._x;
		oz = _pathNode[a + 1]._z - _pathNode[a]._z;
		// if it's a useless node, remove it
		if ((ox == 0.0) && (oz == 0.0)) {
			continue;
		}

		approx = sqrt(ox * ox + oz * oz);
		ox /= approx;
		oz /= approx;

		theta = _vm->sinCosAngle(ox, oz) * 180.0f / PI + 180.0f;
		if (theta >= 360.0)
			theta -= 360.0;
		if (theta < 0.0)
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
		startpos = len;
	}

	_step[b]._px = _curX;
	_step[b]._pz = _curZ;
	_step[b]._dx = 0;
	_step[b]._dz = 0;
	_step[b]._theta = theta;
	_step[b]._curAction = hSTAND;
	_step[b]._curFrame = 0;
	_step[b]._curPanel = _curPanel;

	_lastStep = b; // last step
	_curStep = 0;  // current step

	// starting angle
	float oldtheta = _vm->_actor->_theta;
	// first angle walk
	theta = _step[0]._theta;

	// if he starts from standstill position
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

	// makes the curve
	oldtheta = _step[2]._theta;
	for (b = 3; b <= _lastStep; b++) {
		theta = _step[b]._theta;

		// if it made a curve
		if (oldtheta != theta) {
			approx = theta - oldtheta;

			if (approx > 180.0)
				approx = -360.0 + approx;
			else if (approx < -180.0)
				approx = 360.0 + approx;

			approx /= 3.0;

			// for the previous one
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

			// for the next one
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

/*------------------------------------------------
			Take the next frame walk
--------------------------------------------------*/
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
		_curStep++;
		return false;
	}

	if (_characterGoToPosition != -1)
		setPosition(_characterGoToPosition);

	return true;
}
/*------------------------------------------------
				View route
--------------------------------------------------*/
void PathFinding3D::displayPath() {
	buildFramelist();
}

/*------------------------------------------------
	Check if two panels are in the same block
--------------------------------------------------*/
bool PathFinding3D::findAttachedPanel(int srcPanel, int destPanel) {
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
			curPanel = nearPanel;
			nearPanel = _panel[curPanel]._near2;
		} else {
			curPanel = nearPanel;
			nearPanel = _panel[curPanel]._near1;
		}
	}
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

/*------------------------------------------------
		sort the nodes of the path found
--------------------------------------------------*/
void PathFinding3D::sortPath() {
	qsort(&_pathNode[0], _numPathNodes, sizeof(SPathNode), pathCompare);
}

/*------------------------------------------------
			Initializes sort panel
--------------------------------------------------*/
void PathFinding3D::initSortPan() {
	_numSortPan = 31;

	for (int a = 1; a < _numSortPan - 1; ++a) {
		_sortPan[a]._min = 32000.0f;
		_sortPan[a]._num = a;
	}

	// First panel is behind everything and is not sorted
	_sortPan[0]._min = 30000.0f;
	_sortPan[0]._num = BOX_BACKGROUND;

	// Last panel is in front of everything and is not sorted
	_sortPan[30]._min = 0.0f;
	_sortPan[30]._num = BOX_FOREGROUND;

	Actor *actor = _vm->_actor;
	// Sort panel blocks by increasing distance from the camera
	for (int b = 0; b < _panelNum; ++b) {
		if (!(_panel[b]._flags & 0x80000000)) {
			float dist1 = _vm->dist3D(actor->_camera->_ex, 0.0, actor->_camera->_ez, _panel[b]._x1, 0.0, _panel[b]._z1);
			float dist2 = _vm->dist3D(actor->_camera->_ex, 0.0, actor->_camera->_ez, _panel[b]._x2, 0.0, _panel[b]._z2);

			float min = MIN(dist1, dist2);

			for (int a = 0; a < _numSortPan; ++a) {
				if (_panel[b]._flags & (1 << a)) {
					if (_sortPan[a + 1]._min > min)
						_sortPan[a + 1]._min = min;
				}
			}
		}
	}

	sortPanel();

	for (int b = 0; b < _numSortPan; ++b) {
		if (_sortPan[b]._num == BOX_BACKGROUND) {
			// now the panels go from 0 (foreground) to _numSortPan (background)
			_numSortPan = b;
			break;
		}
	}
}

} // End of namespace Trecision
