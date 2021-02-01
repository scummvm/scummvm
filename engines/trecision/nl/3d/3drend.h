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

#define MAXVERTEX	1000
#define MAXFACE		1000

#define CALCPOINTS	1
#define DRAWFACES	2
#define DOALL		3

#define TEXTUREACTIVE	1
#define TEXTUREPLANAR	2
#define TEXTURECYLIND	4
#define TEXTURELATERAL	8

#define SHADOWAMBIENT	 27      // 0 (black) ... 127 (blank) shadow colors - opposite
#define LIGHTRANGE       2048.0  // after 127 * val the light doesn't have an effect (deleted)
#define CHARACTERMIDSIZE 91.0    // character half height

namespace Trecision {

struct SVertex {
	float _x, _y, _z;
	float _nx, _ny, _nz;
};

struct SFace {
	uint16 _a, _b, _c;
	uint16 _mat;
};

struct SLight {
	float _x, _y, _z;
	float _dx, _dy, _dz;
	float _inr, _outr;
	uint8 _hotspot;
	uint8 _fallOff;
	int8 _inten;
	int8 _position;
};

struct SCamera {
	float _ex, _ey, _ez;
	float _e1[3];
	float _e2[3];
	float _e3[3];
	float _fovX, _fovY;
};

struct STexture {
	int16 _dx, _dy, _angle;
	uint8 *_texture;
	uint8 *_palette;
	uint8 _flag;
};

/*------------------------------------------------
	Create reference table for the vertices of
	a texture
--------------------------------------------------*/
void createTextureMapping(int16 mat);
/*------------------------------------------------
	Initialize a 3D Room
--------------------------------------------------*/
void init3DRoom(int16 dx, uint16 *destBuffer, int16 *zBuffer);
/*------------------------------------------------
	Change the clipping area
--------------------------------------------------*/
void setClipping(int16 x1, int16 y1, int16 x2, int16 y2);
/*------------------------------------------------
	Draw the character
--------------------------------------------------*/
void drawCharacter(uint8 flag);

} // End of namespace Trecision
