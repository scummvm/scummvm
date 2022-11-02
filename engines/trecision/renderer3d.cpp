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
#include "trecision/graphics.h"
#include "trecision/renderer3d.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {
#define SHADOWVERTSNUM 42

static const int16 _shadowVerts[SHADOWVERTSNUM] = {
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

const int16 _shadowFaces[SHADOWFACESNUM][3] = {
	{22, 21, 5}, {7, 5, 22},
	{7, 19, 5}, {5, 2, 19},
	{27, 24, 16}, {27, 16, 18},
	{18, 16, 9}, {18, 13, 9},
	{13, 9, 2}, {3, 19, 12},
	{25, 26, 17}, {17, 15, 25},
	{17, 19, 15}, {15, 12, 19},
	{20, 23, 8}, {8, 6, 20},
	{6, 9, 3}, {3, 8, 6},
	{12, 3, 4}, {4, 11, 12},
	{35, 4, 11}, {13, 2, 1},
	{1, 14, 13}, {14, 37, 1},
	{1, 34, 37}, {31, 36, 37},
	{37, 30, 31}, {29, 34, 35},
	{35, 29, 28}, {36, 11, 31},
	{30, 37, 14}, {29, 1, 34},
	{28, 4, 35}, {36, 10, 35},
	{35, 32, 10}, {37, 0, 34},
	{37, 33, 0}, {0, 33, 39},
	{39, 40, 0}, {10, 38, 32},
	{32, 41, 38}, {36, 35, 34},
	{36, 37, 35}, {11, 36, 35},
	{38, 40, 41}, {41, 38, 39},
	{2, 19, 13}, {3, 9, 12}
};

Renderer3D::Renderer3D(TrecisionEngine *vm) : _vm(vm) {
	_zBuffer = new int16[ZBUFFERSIZE / 2];

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
		_vVertex[i].clear();
		_shVertex[i].clear();
	}
}

Renderer3D::~Renderer3D() {
	delete[] _zBuffer;
}

void Renderer3D::textureTriangle(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1,
								 int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2,
								 int32 x3, int32 y3, int32 z3, int32 c3, int32 tx3, int32 ty3,
								 const STexture *t) {
	if (y1 > _maxYClip)
		y1 = _maxYClip;
	if (y1 < _minYClip)
		y1 = _minYClip;

	int16 yBottom = y1;
	int16 yTop = y1;
	const uint8 *texture = t->_texture;

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
	for (int16 y = yBottom; y < yTop; ++y) {
		_lEdge[y] = _maxXClip;
		_rEdge[y] = _minXClip;
	}

	// scan the edges of the triangle
	textureScanEdge(x1, y1, z1, c1, tx1, ty1, x2, y2, z2, c2, tx2, ty2);
	textureScanEdge(x2, y2, z2, c2, tx2, ty2, x3, y3, z3, c3, tx3, ty3);
	textureScanEdge(x3, y3, z3, c3, tx3, ty3, x1, y1, z1, c1, tx1, ty1);

	// Gouraud fill the horizontal scanlines
	for (int16 y = yBottom; y < yTop; ++y) {
		int32 el = _lEdge[y];
		if (el < _minXClip)
			el = _minXClip;
		int32 er = _rEdge[y];
		if (er > _maxXClip)
			er = _maxXClip;

		// edge right - edge left
		int16 dx = er - el;

		if (dx > 0) {
			// color of left edge of horizontal scanline
			int32 cl = _lColor[y];
			// slope dc/_dx
			int32 mc = ((int16)(_rColor[y] - cl) << 8) / dx;
			// zbuffer of left edge of horizontal scanline
			int32 zl = _lZ[y];
			// slope _dz/_dx
			int32 mz = ((int32)(_rZ[y] - zl) << 16) / dx;
			// texture x of left edge of horizontal scanline
			int32 olx = _lTextX[y];
			// slope dty/_dx
			int32 mtx = ((int32)(_rTextX[y] - olx) << 16) / dx;
			// texture y of left edge of horizontal scanline
			int32 oly = _lTextY[y];
			// slope dty/_dx
			int32 mty = ((int32)(_rTextY[y] - oly) << 16) / dx;
			// pointer to zbuffer
			int16 *z = _zBuffer + (y - _zBufStartY) * _zBufWid + (el - _zBufStartX);
			uint16 x = el;

			zl <<= 16;
			cl <<= 8;
			olx <<= 16;
			oly <<= 16;
			// loop through every pixel in horizontal scanline
			while (dx) {
				const int32 screenOffset = zl >> 16;
				if (*z > screenOffset) {
					const uint16 textureX = (uint16)(cl >> 9);
					const uint16 textureY = texture[(olx >> 16) + t->_dx * (oly >> 16)];
					_vm->_graphicsMgr->drawTexturePixel(textureX, textureY, x, y);
					*z = (int16)screenOffset;
				}
				++x;         // increase screen x
				++z;         // increase zbuffer
				zl += mz;    // increase the zbuffer by _dz/_dx
				cl += mc;    // increase the color by dc/_dx
				olx += mtx;
				oly += mty;
				--dx; // pixel to do --
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
	for (int32 count = y1; count < y2; ++count) {
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

	for (int16 y = yBottom; y < yTop; ++y) {
		_lEdge[y] = _maxXClip;
		_rEdge[y] = _minXClip;
	}

	// scan the edges of the triangle
	shadowScanEdge(x1, y1, x2, y2);
	shadowScanEdge(x2, y2, x3, y3);
	shadowScanEdge(x3, y3, x1, y1);

	// gouraud fill the horizontal scanlines
	for (int16 y = yBottom; y < yTop; ++y) {
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

		if (dx > 0) {
			// screen offset
			int16 x = el;
			int16 *zBufferPtr = _zBuffer + (y - _zBufStartY) * _zBufWid + (el - _zBufStartX);
		
			// loop through every pixel in horizontal scanline
			while (dx) {
				if (*zBufferPtr != zv) {
					_vm->_graphicsMgr->shadow(x, y, cv);
					*zBufferPtr = zv;
				}
				++x;          // increase screen x
				++zBufferPtr; // increase zbuffer
				--dx;         // pixel to do --
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
	for (int32 count = y1; count < y2; ++count) {
		int16 x = (int16)(x1 >> 16);
		if (x < _lEdge[count])
			_lEdge[count] = x;

		if (x > _rEdge[count])
			_rEdge[count] = x;

		x1 += mx; // x = x + dx/dy
	}
}

/**
 *	Initialize a 3D Room
 */
void Renderer3D::init3DRoom() {
	_vm->_cx = (MAXX - 1) / 2;
	_vm->_cy = (MAXY - 1) / 2;

	for (int c = 0; c < ZBUFFERSIZE / 2; ++c)
		_zBuffer[c] = 0x7FFF;
}

void Renderer3D::resetZBuffer(Common::Rect area) {
	if (!area.isValidRect())
		return;

	int size = area.width() * area.height();
	if (size * 2 > ZBUFFERSIZE)
		warning("Warning: _zBuffer size %d!\n", size * 2);

	int16 *d = _zBuffer;
	for (int i = 0; i < size; ++i)
		*d++ = 0x7FFF;
}

/**
 *	Change the clipping area
 */
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

/**
 *	Determines whether a triangle has clockwise
 *	or counterclockwise vertices
 */
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

void Renderer3D::calcCharacterPoints() {
	Actor *actor = _vm->_actor;
	SCamera *camera = actor->_camera;
	SLight *light = actor->_light;
	int vertexNum = actor->_vertexNum;

	if (actor->_curAction > hLAST)
		error("Error in drawCharacter() - _curAction > hLAST");

	int cfp = 0;
	int cur = 0;
	while (cur < actor->_curAction)
		cfp += defActionLen[cur++];

	if (actor->_curAction == hWALKOUT)
		cfp = 1;

	cfp += actor->_curFrame;

	if (actor->_curAction == hLAST)
		cfp = 0;

	actor->_vertex = &actor->_characterArea[cfp * actor->_vertexNum];

	_shadowLightNum = 0;
	_totalShadowVerts = 0;

	// camera matrix
	float e10 = camera->_e1[0];
	float e11 = camera->_e1[1];
	float e12 = camera->_e1[2];

	float e20 = camera->_e2[0];
	float e21 = camera->_e2[1];
	float e22 = camera->_e2[2];

	float e30 = camera->_e3[0];
	float e31 = camera->_e3[1];
	float e32 = camera->_e3[2];

	// Light directions
	float l0 = 0.0f;
	float l1 = 0.0f;
	float l2 = 0.0f;

	actor->_area[0] = 32000;
	actor->_area[1] = -32000;
	actor->_area[2] = 32000;
	actor->_area[3] = -32000;
	actor->_area[4] = 32000;
	actor->_area[5] = -32000;

	float t = (actor->_theta * PI2) / 360.0;
	float cost = cos(t);
	float sint = sin(t);

	// Put all vertices in dark color
	for (int i = 0; i < MAXVERTEX; ++i)
		_vVertex[i]._angle = 180;

	float dist;
	float tx = 0.0f;
	float ty = 0.0f;
	float tz = 0.0f;
	float pa0, pa1, pa2;

	for (uint32 i = 0; i < actor->_lightNum; ++i) {
		// if off                lint == 0
		// if it has a shadow    lint & 0x80

		int lint = light->_inten & 0x7F;
		if (lint) {                                   // if it's not turned off
			tx = light->_x - actor->_px - actor->_dx; // computes direction vector
			tz = light->_z - actor->_pz - actor->_dz; // between light and actor
			ty = light->_y;

			if (light->_position) {                       // if it's attenuated
				dist = sqrt(tx * tx + ty * ty + tz * tz); // Distance light <--> actor

				// adjust light intensity due to the distance
				if (_vm->floatComp(dist, light->_outr) == 1) // if it's out of range it's off
					lint = 0;
				else if (_vm->floatComp(dist, light->_inr) == 1) // if it's inside the circle it's decreased
					lint = (int)((float)lint * (light->_outr - dist) / (light->_outr - light->_inr));
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
			tx = (float)light->_fallOff;
			if (light->_fallOff) { // for light spot only
				ty = (float)light->_hotspot;

				pa0 = light->_dx * cost - light->_dz * sint;
				pa1 = light->_dy;
				pa2 = light->_dx * sint + light->_dz * cost;

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

				if (_vm->floatComp(tz, tx) == 1) { // tz > tx - if it's out of the falloff
					lint = 0;
					_shadowIntens[_shadowLightNum] = 0;
				} else if (_vm->floatComp(tz, ty) == 1) { // tz > ty - if it's between the falloff and the hotspot
					lint = (int)((float)lint * (tx - tz) / (tx - ty));
					_shadowIntens[_shadowLightNum] = (int)((float)_shadowIntens[_shadowLightNum] * (tx - tz) / (tx - ty));
				}
			}
		}

		if ((light->_inten & 0x80) && lint) { // if it's shadowed and still on

			// casts shadow vertices
			for (int j = 0; j < SHADOWVERTSNUM; ++j) {
				pa0 = actor->_vertex[_shadowVerts[j]]._x;
				pa1 = actor->_vertex[_shadowVerts[j]]._y;
				pa2 = actor->_vertex[_shadowVerts[j]]._z;

				_shVertex[vertexNum + _totalShadowVerts + j]._x = pa0 - (pa1 * l0);
				_shVertex[vertexNum + _totalShadowVerts + j]._z = pa2 - (pa1 * l2);
				_shVertex[vertexNum + _totalShadowVerts + j]._y = 0;
			}

			// per default all shadows are equally faint
			// _shadowIntens[_shadowLightNum] = SHADOWAMBIENT;

			++_shadowLightNum;
			_totalShadowVerts += SHADOWVERTSNUM;
		}

		if (lint) { // if still on
			// adapts the light vector o its intensity
			t = (float)(lint) / 127.0;
			l0 = l0 * t;
			l1 = l1 * t;
			l2 = l2 * t;

			SVertex *curVertex = actor->_vertex;
			for (int j = 0; j < vertexNum; ++j) {
				pa0 = curVertex->_nx;
				pa1 = curVertex->_ny;
				pa2 = curVertex->_nz;

				lint = (int)((acos(pa0 * l0 + pa1 * l1 + pa2 * l2) * 360.0) / PI);
				lint = CLIP(lint, 0, 180);

				_vVertex[j]._angle -= (180 - lint);
				++curVertex;
			}
		}

		++light;
	}

	// rearranged light values so they can be viewed
	for (int i = 0; i < vertexNum; ++i)
		_vVertex[i]._angle = CLIP<int32>(_vVertex[i]._angle, 0, 180);

	// Calculate the distance of the character from the room
	tx = camera->_ex - actor->_px;
	ty = camera->_ey;
	tz = camera->_ez - actor->_pz;

	dist = tx * e30 + ty * e31 + tz * e32;

	SVertex *curVertex = actor->_vertex;

	for (int i = 0; i < vertexNum + _totalShadowVerts; ++i) {
		if (i < vertexNum) {
			l0 = curVertex->_x;
			l1 = curVertex->_z;
			pa1 = ty - curVertex->_y;
		} else {
			l0 = _shVertex[i]._x;
			l1 = _shVertex[i]._z;
			pa1 = ty - _shVertex[i]._y;
		}

		pa0 = tx - (l0 * cost + l1 * sint); // rotate _curVertex
		pa2 = tz - (-l0 * sint + l1 * cost);

		l0 = pa0 * e10 + pa1 * e11 + pa2 * e12; // project _curVertex
		l1 = pa0 * e20 + pa1 * e21 + pa2 * e22;
		l2 = pa0 * e30 + pa1 * e31 + pa2 * e32;

		int x2d = _vm->_cx + (int)((l0 * camera->_fovX) / l2);
		int y2d = _vm->_cy + (int)((l1 * camera->_fovY) / l2);

		_vVertex[i]._x = x2d;
		_vVertex[i]._y = y2d;
		_vVertex[i]._z = (int32)((dist - l2) * 128.0);

		actor->_area[0] = MIN(x2d, actor->_area[0]);
		actor->_area[1] = MAX(x2d, actor->_area[1]);
		actor->_area[2] = MIN(y2d, actor->_area[2]);
		actor->_area[3] = MAX(y2d, actor->_area[3]);

		actor->_area[4] = MIN<int32>(_vVertex[i]._z, actor->_area[4]);
		actor->_area[5] = MAX<int32>(_vVertex[i]._z, actor->_area[5]);

		++curVertex;
	}
	actor->_area[4] = (int)dist;
	actor->_area[5] = (int)dist;

	// vertex clipping
	if (actor->_area[0] <= _minXClip + 1) {
		actor->_area[0] = _minXClip;
	} else {
		--actor->_area[0];
	}

	if (actor->_area[1] >= _maxXClip - 1) {
		actor->_area[1] = _maxXClip;
	} else {
		++actor->_area[1];
	}

	if (actor->_area[2] <= _minYClip + 1) {
		actor->_area[2] = _minYClip;
	} else {
		--actor->_area[2];
	}

	if (actor->_area[3] >= _maxYClip - 1) {
		actor->_area[3] = _maxYClip;
	} else {
		++actor->_area[3];
	}

	if (actor->_curAction == hLAST) // exit displacer
		actor->_area[2] = actor->_area[3] - (((actor->_area[3] - actor->_area[2]) * actor->_curFrame) / defActionLen[hLAST]);

	// set zbuffer vars
	setZBufferRegion(actor->_area[0], actor->_area[2], actor->_area[1] - actor->_area[0]);
}

void Renderer3D::drawCharacterFaces() {
	Actor *actor = _vm->_actor;
	STexture *textures = actor->_textures;
	SFace *face = actor->_face;
	int vertexNum = actor->_vertexNum;

	if (actor->_curAction == hLAST)
		setClipping(0, actor->_area[2], MAXX, actor->_area[3]);

	for (int i = 0; i < _shadowLightNum; ++i) {
		for (int j = 0; j < SHADOWFACESNUM; ++j) {
			int p0 = _shadowFaces[j][0] + vertexNum + i * SHADOWVERTSNUM;
			int p1 = _shadowFaces[j][1] + vertexNum + i * SHADOWVERTSNUM;
			int p2 = _shadowFaces[j][2] + vertexNum + i * SHADOWVERTSNUM;

			int px0 = _vVertex[p0]._x;
			int py0 = _vVertex[p0]._y;
			int px1 = _vVertex[p1]._x;
			int py1 = _vVertex[p1]._y;
			int px2 = _vVertex[p2]._x;
			int py2 = _vVertex[p2]._y;

			shadowTriangle(px0, py0, px1, py1, px2, py2, 127 - _shadowIntens[i], (int16)(0x7FF0 + i));
		}
	}

	for (uint i = 0; i < actor->_faceNum; ++i) {
		int p0 = face->_a;
		int p1 = face->_b;
		int p2 = face->_c;

		int px0 = _vVertex[p0]._x;
		int py0 = _vVertex[p0]._y;
		int px1 = _vVertex[p1]._x;
		int py1 = _vVertex[p1]._y;
		int px2 = _vVertex[p2]._x;
		int py2 = _vVertex[p2]._y;

		if (clockWise(px0, py0, px1, py1, px2, py2) > 0) {
			uint16 textureId = face->_mat;
			if (textureId < MAXMAT && textures[textureId].isActive()) {
				textureTriangle(px0, py0, _vVertex[p0]._z, _vVertex[p0]._angle, actor->_textureCoord[i][0][0], actor->_textureCoord[i][0][1],
								px1, py1, _vVertex[p1]._z, _vVertex[p1]._angle, actor->_textureCoord[i][1][0], actor->_textureCoord[i][1][1],
								px2, py2, _vVertex[p2]._z, _vVertex[p2]._angle, actor->_textureCoord[i][2][0], actor->_textureCoord[i][2][1],
								&textures[textureId]);
			}
		}

		++face;
	}

	int p0 = 0;
	for (int i = _zBufStartY; i < actor->_area[3]; ++i) {
		for (int j = 1; j < _zBufWid; ++j) {
			int py1 = (_zBuffer[p0] >= 0x7FF0) * 0x8000;
			int py2 = (_zBuffer[p0 + 1] >= 0x7FF0) * 0x8000;

			int p1 = _zBuffer[p0] < 0x7FFF;
			int p2 = _zBuffer[p0 + 1] < 0x7FFF;

			if (p1 != p2) {
				_vm->_graphicsMgr->pixelAliasing(j + _zBufStartX, i);

				// if the first is the character
				if (p1)
					_zBuffer[p0] = 0x00BF | py1;
				else
					_zBuffer[p0] = 0x003F | py2;

				if (j + 1 < _zBufWid) {
					++p0;
					++j;

					// if the second is the character
					if (p2)
						_zBuffer[p0] = 0x00BF | py2;
					else
						_zBuffer[p0] = 0x003F | py1;
				}
			} else {
				// set value alpha max
				if (p1)
					_zBuffer[p0] = 0x00FF | py1;
				else
					_zBuffer[p0] = 0x0000 | py1;
			}

			++p0;

			// if it's the last of the line
			if (j == _zBufWid - 1) {
				if (p2)
					_zBuffer[p0] = 0x00FF | py2;
				else
					_zBuffer[p0] = 0x0000 | py2;
			}
		}
		++p0;
	}
	if (actor->_curAction == hLAST)
		setClipping(0, TOP, MAXX, AREA + TOP);
}

/**
 *	Draw the character
 */
void Renderer3D::drawCharacter(uint8 flag) {
	if (!_vm->_flagShowCharacter)
		return;

	// Compute pointer to frame
	if (flag & CALCPOINTS)
		calcCharacterPoints();

	if (flag & DRAWFACES)
		drawCharacterFaces();
}

} // End of namespace Trecision
