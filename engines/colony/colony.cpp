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

#include "colony/colony.h"
#include "colony/gfx.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"

namespace Colony {

static const int16 g_sintTable[256] = {
	90,  93,  95,  97,  99, 101, 103, 105,
	106, 108, 110, 111, 113, 114, 116, 117,
	118, 119, 121, 122, 122, 123, 124, 125,
	126, 126, 127, 127, 127, 128, 128, 128,
	128, 128, 128, 128, 127, 127, 127, 126,
	126, 125, 124, 123, 122, 122, 121, 119,
	118, 117, 116, 114, 113, 111, 110, 108,
	106, 105, 103, 101,  99,  97,  95,  93,
	90,  88,  86,  84,  81,  79,  76,  74,
	71,  68,  66,  63,  60,  56,  54,  52,
	49,  46,  43,  40,  37,  34,  31,  28,
	25,  23,  19,  16,  13,   9,   6,   3,
	0,  -3,  -6,  -9, -13, -16, -19, -23,
	-25, -28, -31, -34, -37, -40, -43, -46,
	-49, -52, -54, -56, -60, -63, -66, -68,
	-71, -74, -76, -79, -81, -84, -86, -88,
	-88, -90, -93, -95, -97, -99,-101,-103,
	-105,-106,-108,-110,-111,-113,-114,-116,
	-117,-118,-119,-121,-122,-122,-123,-124,
	-125,-126,-126,-127,-127,-127,-128,-128,
	-128,-128,-128,-128,-127,-127,-127,-126,
	-126,-125,-124,-123,-122,-122,-121,-119,
	-118,-117,-116,-114,-113,-111,-110,-108,
	-106,-105,-103,-101, -99, -97, -95, -93,
	-90, -88, -86, -84, -81, -79, -76, -74,
	-71, -68, -66, -63, -60, -56, -54, -52,
	-49, -46, -43, -40, -37, -34, -31, -28,
	-25, -23, -19, -16, -13,  -9,  -6,  -3,
	0,   3,   6,   9,  13,  16,  19,  23,
	25,  28,  31,  34,  37,  40,  43,  46,
	49,  52,  54,  56,  60,  63,  66,  68,
	71,  74,  76,  79,  81,  84,  86,  88
};

static const int g_indexTable[4][10] = {
	{0, 0,  0, 0,  0,  1,  1,  0,  1, 2},
	{1, 0,  0, 0, -1,  0,  0,  1,  2, 1},
	{0, 1,  1, 0,  0, -1, -1,  0,  1, 2},
	{0, 0,  0, 1,  1,  0,  0, -1,  2, 1}
};

enum WallFeatureType {
	kWallFeatureNone = 0,
	kWallFeatureDoor = 2,
	kWallFeatureWindow = 3,
	kWallFeatureShelves = 4,
	kWallFeatureUpStairs = 5,
	kWallFeatureDnStairs = 6,
	kWallFeatureChar = 7,
	kWallFeatureGlyph = 8,
	kWallFeatureElevator = 9,
	kWallFeatureTunnel = 10,
	kWallFeatureAirlock = 11,
	kWallFeatureColor = 12
};

static const int g_dirRight[4] = {1, 3, 0, 2};
static const int g_dirLeft[4] = {2, 0, 3, 1};

ColonyEngine::ColonyEngine(OSystem *syst, const ADGameDescription *gd) : Engine(syst), _gameDescription(gd) {
	_level = 0;
	_robotNum = 0;
	_gfx = nullptr;
	_width = 640;
	_height = 480;
	_centerX = _width / 2;
	_centerY = _height / 2;
	_flip = false;
	_mouseSensitivity = 1;
	_change = true;
	
	memset(_wall, 0, sizeof(_wall));
	memset(_mapData, 0, sizeof(_mapData));
	memset(_robotArray, 0, sizeof(_robotArray));
	memset(_foodArray, 0, sizeof(_foodArray));
	
	_me.xindex = 10;
	_me.yindex = 10;
	_me.xloc = 10 * 256 + 128;
	_me.yloc = 10 * 256 + 128;
	_me.look = 0;
	_me.ang = 0;

	initTrig();
}

ColonyEngine::~ColonyEngine() {
}

void ColonyEngine::loadMap(int mnum) {
	Common::String mapName = Common::String::format("MAP.%d", mnum);
	Common::File file;
	if (!file.open(Common::Path(mapName))) {
		warning("Could not open map file %s", mapName.c_str());
		return;
	}

	file.readUint32BE(); // "DAVE" header
	int16 mapDefs[10];
	for (int i = 0; i < 10; i++) {
		mapDefs[i] = file.readSint16BE(); // Swapped in original code
	}

	uint16 bLength = file.readUint16BE(); // Swapped in original code
	uint8 *buffer = (uint8 *)malloc(bLength);
	if (!buffer) {
		error("Out of memory loading map");
	}
	file.read(buffer, bLength);
	file.close();

	// expand logic
	int c = 0;
	_robotNum = MENUM + 1;
	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < 32; j++) {
			_wall[i][j] = buffer[c++];
			if (i < 31 && j < 31) {
				for (int k = 0; k < 5; k++) {
					if (_wall[i][j] & (1 << (k + 2))) {
						for (int l = 0; l < 5; l++) {
							_mapData[i][j][k][l] = buffer[c++];
						}
						// Robot creation logic will be added here
					} else {
						_mapData[i][j][k][0] = 0;
					}
				}
			}
		}
	}
	free(buffer);
	_level = mnum;
	debug("Successfully loaded map %d", mnum);
}

void ColonyEngine::initTrig() {
	for (int i = 0; i < 256; i++) {
		_sint[i] = g_sintTable[i];
		_cost[i] = g_sintTable[(i + 64) & 0xFF];
	}

	_rtable[0] = 32000;
	for (int i = 1; i < 11585; i++) {
		_rtable[i] = (160 * 128) / i;
	}
}

void ColonyEngine::rot_init(int x, int y) {
	_rox = ((long)x * _tsin - (long)y * _tcos) >> 8;
	_roy = ((long)y * _tsin + (long)x * _tcos) >> 8;
}

void ColonyEngine::perspective(int pnt[2], int rox, int roy) {
	long p;

	if (roy <= 0)
		roy = 1;
	p = _centerX + ((long)rox * 256) / roy;

	if (p < -32000)
		p = -32000;
	else if (p > 32000)
		p = 32000;
	pnt[0] = (int)p;

	if (_flip)
		pnt[1] = _centerY + _rtable[roy];
	else
		pnt[1] = _centerY - _rtable[roy];
}

int ColonyEngine::checkwall(int xnew, int ynew, Locate *pobject) {
	int xind2, yind2;
	xind2 = xnew >> 8;
	yind2 = ynew >> 8;
	_change = true;

	if (xind2 == pobject->xindex) {
		if (yind2 == pobject->yindex) {
			pobject->dx = xnew - pobject->xloc;
			pobject->dy = ynew - pobject->yloc;
			pobject->xloc = xnew;
			pobject->yloc = ynew;
			return 0;
		} else {
			if (yind2 > pobject->yindex) {
				if (!(_wall[pobject->xindex][yind2] & 1)) {
					pobject->yindex = yind2;
					pobject->xindex = xind2;
					pobject->dx = xnew - pobject->xloc;
					pobject->dy = ynew - pobject->yloc;
					pobject->xloc = xnew;
					pobject->yloc = ynew;
					return 0;
				} else {
					debug("Collision South at x=%d y=%d", pobject->xindex, yind2);
					return -1;
				}
			} else {
				if (!(_wall[pobject->xindex][pobject->yindex] & 1)) {
					pobject->yindex = yind2;
					pobject->xindex = xind2;
					pobject->dx = xnew - pobject->xloc;
					pobject->dy = ynew - pobject->yloc;
					pobject->xloc = xnew;
					pobject->yloc = ynew;
					return 0;
				} else {
					debug("Collision North at x=%d y=%d", pobject->xindex, pobject->yindex);
					return -1;
				}
			}
		}
	} else if (yind2 == pobject->yindex) {
		if (xind2 > pobject->xindex) {
			if (!(_wall[xind2][pobject->yindex] & 2)) {
				pobject->yindex = yind2;
				pobject->xindex = xind2;
				pobject->dx = xnew - pobject->xloc;
				pobject->dy = ynew - pobject->yloc;
				pobject->xloc = xnew;
				pobject->yloc = ynew;
				return 0;
			} else {
				debug("Collision East at x=%d y=%d", xind2, pobject->yindex);
				return -1;
			}
		} else {
			if (!(_wall[pobject->xindex][pobject->yindex] & 2)) {
				pobject->yindex = yind2;
				pobject->xindex = xind2;
				pobject->dx = xnew - pobject->xloc;
				pobject->dy = ynew - pobject->yloc;
				pobject->xloc = xnew;
				pobject->yloc = ynew;
				return 0;
			} else {
				debug("Collision West at x=%d y=%d", pobject->xindex, pobject->yindex);
				return -1;
			}
		}
	} else {
		// Diagonal
		if (xind2 > pobject->xindex) {
			if (yind2 > pobject->yindex) {
				if ((_wall[pobject->xindex][yind2] & 1) || (_wall[xind2][pobject->yindex] & 2) || (_wall[xind2][yind2] & 3)) {
					debug("Collision Diagonal SE");
					return -1;
				}
			} else {
				if ((_wall[pobject->xindex][pobject->yindex] & 1) || (_wall[xind2][yind2] & 2) || (_wall[xind2][pobject->yindex] & 3)) {
					debug("Collision Diagonal NE");
					return -1;
				}
			}
		} else {
			if (yind2 > pobject->yindex) {
				if ((_wall[xind2][yind2] & 1) || (_wall[pobject->xindex][pobject->yindex] & 2) || (_wall[pobject->xindex][yind2] & 3)) {
					debug("Collision Diagonal SW");
					return -1;
				}
			} else {
				if ((_wall[xind2][pobject->yindex] & 1) || (_wall[pobject->xindex][yind2] & 2) || (_wall[pobject->xindex][pobject->yindex] & 3)) {
					debug("Collision Diagonal NW");
					return -1;
				}
			}
		}
		pobject->yindex = yind2;
		pobject->xindex = xind2;
		pobject->dx = xnew - pobject->xloc;
		pobject->dy = ynew - pobject->yloc;
		pobject->xloc = xnew;
		pobject->yloc = ynew;
		return 0;
	}
	return -1;
}

void ColonyEngine::quadrant() {
	int remain;
	int quad;

	quad = _me.look >> 6;				/*divide by 64		*/
	remain = _me.look - (quad << 6);			/*multiply by 64	*/
	_tsin = _sint[remain];
	_tcos = _cost[remain];

	switch (quad) {
	case 0:
		rot_init((_me.xindex << 8) - _me.xloc, (_me.yindex << 8) - _me.yloc);
		_direction = 0; // NORTH
		break;
	case 1:
		rot_init((_me.yindex << 8) - _me.yloc, _me.xloc - ((_me.xindex + 1) << 8));
		_direction = 2; // WEST
		break;
	case 2:
		rot_init(_me.xloc - ((_me.xindex + 1) << 8), _me.yloc - ((_me.yindex + 1) << 8));
		_direction = 3; // SOUTH
		break;
	case 3:
		rot_init(_me.yloc - ((_me.yindex + 1) << 8), (_me.xindex << 8) - _me.xloc);
		_direction = 1; // EAST
		break;
	}

	_frntxWall = g_indexTable[quad][0];
	_frntyWall = g_indexTable[quad][1];
	_sidexWall = g_indexTable[quad][2];
	_sideyWall = g_indexTable[quad][3];
	_frntx = g_indexTable[quad][4];
	_frnty = g_indexTable[quad][5];
	_sidex = g_indexTable[quad][6];
	_sidey = g_indexTable[quad][7];
	_front = g_indexTable[quad][8];
	_side = g_indexTable[quad][9];
}

void ColonyEngine::corridor() {
	int length = 1;
	int xFrontLeft, yFrontLeft;
	int xFrontRight, yFrontRight;
	int xsstart, ysstart;
	int xfbehind, yfbehind;
	int roxsave, roysave;
	int left, right;
	int left2, right2;
	int cellx, celly;
	int cellxsave, cellysave;
	int dr[2];
	const int screenLeft = (int)_screenR.left;
	const int screenRight = (int)_screenR.right;

	quadrant();

	right = screenRight;
	left = screenLeft;
	right2 = right;
	left2 = left;

	xfbehind = _me.xindex + _frntxWall;
	yfbehind = _me.yindex + _frntyWall;
	xFrontLeft = xfbehind + _frntx;
	yFrontLeft = yfbehind + _frnty;
	xFrontRight = xFrontLeft + _sidex;
	yFrontRight = yFrontLeft + _sidey;
	xsstart = _me.xindex + _sidexWall;
	ysstart = _me.yindex + _sideyWall;
	cellxsave = cellx = _me.xindex;
	cellysave = celly = _me.yindex;

	int rox = _rox;
	int roy = _roy;

	if (_change) {
		perspective(dr, rox, roy);
		if (xfbehind >= 0 && xfbehind < 34 && yfbehind >= 0 && yfbehind < 34) {
			_drY[xfbehind][yfbehind] = dr[1];
			if (dr[0] > _screenR.left)
				_drX[xfbehind][yfbehind] = -32000;
			else
				_drX[xfbehind][yfbehind] = dr[0];
		}

		perspective(dr, rox + _tsin, roy + _tcos);
		if (xfbehind + _sidex >= 0 && xfbehind + _sidex < 34 && yfbehind + _sidey >= 0 && yfbehind + _sidey < 34) {
			_drY[xfbehind + _sidex][yfbehind + _sidey] = dr[1];
			if (dr[0] < _screenR.right)
				_drX[xfbehind + _sidex][yfbehind + _sidey] = 32000;
			else
				_drX[xfbehind + _sidex][yfbehind + _sidey] = dr[0];
		}
	}

	roxsave = rox;
	roysave = roy;

	// Move to the first wall in front of the observer.
	rox -= _tcos;
	roy += _tsin;

	if (_change) {
		perspective(dr, rox, roy);
		if (xFrontLeft >= 0 && xFrontLeft < 34 && yFrontLeft >= 0 && yFrontLeft < 34) {
			_drX[xFrontLeft][yFrontLeft] = dr[0];
			_drY[xFrontLeft][yFrontLeft] = dr[1];
		}
		perspective(dr, rox + _tsin, roy + _tcos);
		if (xFrontRight >= 0 && xFrontRight < 34 && yFrontRight >= 0 && yFrontRight < 34) {
			_drX[xFrontRight][yFrontRight] = dr[0];
			_drY[xFrontRight][yFrontRight] = dr[1];
		}
	}

	if (wallAt(cellx + _sidexWall, celly + _sideyWall) & _side)
		left2 = MAX(_drX[xFrontLeft][yFrontLeft], screenLeft);
	else
		left2 = MAX(left, left2);
	left2 = MAX(left2, screenLeft);

	if (wallAt(cellx + _sidexWall + _sidex, celly + _sideyWall + _sidey) & _side)
		right2 = _drX[xFrontRight][yFrontRight];
	else
		right2 = MIN(right, right2);

	uint32 white = _gfx->white();
	_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _drY[xFrontLeft][yFrontLeft],
	               _drX[xFrontRight][yFrontRight], _drY[xFrontRight][yFrontRight], white);
	if (wallAt(cellx, celly) & ~0x03)
		frontfeature(cellx, celly, xFrontLeft, yFrontLeft, left2, right2, rox, roy);

	while (!(wallAt(xFrontLeft, yFrontLeft) & _front)) {
		rox -= _tcos;
		roy += _tsin;
		xFrontLeft += _frntx;
		yFrontLeft += _frnty;
		xFrontRight += _frntx;
		yFrontRight += _frnty;
		if (_change) {
			perspective(dr, rox, roy);
			if (xFrontLeft >= 0 && xFrontLeft < 34 && yFrontLeft >= 0 && yFrontLeft < 34) {
				_drX[xFrontLeft][yFrontLeft] = dr[0];
				_drY[xFrontLeft][yFrontLeft] = dr[1];
			}
			perspective(dr, rox + _tsin, roy + _tcos);
			if (xFrontRight >= 0 && xFrontRight < 34 && yFrontRight >= 0 && yFrontRight < 34) {
				_drX[xFrontRight][yFrontRight] = dr[0];
				_drY[xFrontRight][yFrontRight] = dr[1];
			}
		}

		cellx += _frntx;
		celly += _frnty;
		if (wallAt(cellx + _sidexWall, celly + _sideyWall) & _side)
			left2 = MAX(screenLeft, _drX[xFrontLeft][yFrontLeft]);
		else
			left2 = MAX(left, left2);
		left2 = MAX(left2, screenLeft);
		if (wallAt(cellx + _sidexWall + _sidex, celly + _sideyWall + _sidey) & _side)
			right2 = _drX[xFrontRight][yFrontRight];
		else
			right2 = MIN(right, right2);
		if (wallAt(cellx, celly) & ~0x03)
			features(cellx, celly, xFrontLeft, yFrontLeft, left2, right2, rox, roy);

		_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _drY[xFrontLeft][yFrontLeft],
		               _drX[xFrontRight][yFrontRight], _drY[xFrontRight][yFrontRight], white);

		length++;
		if (length > 30)
			break; // Safety break
	}
	drawend(xfbehind, yfbehind, xFrontLeft, yFrontLeft);

	left = screenLeft;
	right = MIN(right, _drX[xFrontLeft][yFrontLeft]);
	if (left < right)
		checkleft(xsstart, ysstart, xfbehind, yfbehind, left, right, roxsave, roysave, cellxsave, cellysave, length);

	left = MAX(left, _drX[xFrontRight][yFrontRight]);
	if (left < screenLeft)
		left = screenLeft;
	right = screenRight;
	xsstart += _sidex;
	ysstart += _sidey;
	xfbehind += _sidex;
	yfbehind += _sidey;
	if (left < right)
		checkright(xsstart, ysstart, xfbehind, yfbehind, left, right, roxsave + _tsin, roysave + _tcos, cellxsave, cellysave, length);

	_change = false;
}

void ColonyEngine::drawend(int xstart, int ystart, int xFrontLeft, int yFrontLeft) {
	int xFrontRight, yFrontRight;

	xFrontRight = xFrontLeft + _sidex;
	yFrontRight = yFrontLeft + _sidey;

	uint32 white = _gfx->white();

	if ((xstart != xFrontLeft) || (ystart != yFrontLeft)) {
		if (_drY[xstart + _frntx][ystart + _frnty] > 0) {
			_gfx->drawLine(_drX[xstart][ystart], _drY[xstart][ystart],
			               _drX[xstart + _frntx][ystart + _frnty], _drY[xstart + _frntx][ystart + _frnty], white);
		}
		_gfx->drawLine(_drX[xstart + _frntx][ystart + _frnty], _drY[xstart + _frntx][ystart + _frnty],
		               _drX[xFrontLeft][yFrontLeft], _drY[xFrontLeft][yFrontLeft], white);
		_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _drY[xFrontLeft][yFrontLeft],
		               _drX[xFrontLeft][yFrontLeft], _height - _drY[xFrontLeft][yFrontLeft], white);
		_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _height - _drY[xFrontLeft][yFrontLeft],
		               _drX[xFrontRight][yFrontRight], _height - _drY[xFrontRight][yFrontRight], white);
		_gfx->drawLine(_drX[xFrontRight][yFrontRight], _height - _drY[xFrontRight][yFrontRight],
		               _drX[xFrontRight][yFrontRight], _drY[xFrontRight][yFrontRight], white);
		_gfx->drawLine(_drX[xFrontRight][yFrontRight], _drY[xFrontRight][yFrontRight],
		               _drX[xstart + _sidex][ystart + _sidey], _drY[xstart + _sidex][ystart + _sidey], white);
		if (_drY[xstart + _sidex][ystart + _sidey] > 0) {
			_gfx->drawLine(_drX[xstart + _sidex][ystart + _sidey], _drY[xstart + _sidex][ystart + _sidey],
			               _drX[xstart + _frntx + _sidex][ystart + _frnty + _sidey], _drY[xstart + _frntx + _sidex][ystart + _frnty + _sidey], white);
		}
	} else {
		_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _drY[xFrontLeft][yFrontLeft],
		               _drX[xFrontLeft][yFrontLeft], _height - _drY[xFrontLeft][yFrontLeft], white);
		_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _height - _drY[xFrontLeft][yFrontLeft],
		               _drX[xFrontRight][yFrontRight], _height - _drY[xFrontRight][yFrontRight], white);
		_gfx->drawLine(_drX[xFrontRight][yFrontRight], _height - _drY[xFrontRight][yFrontRight],
		               _drX[xFrontRight][yFrontRight], _drY[xFrontRight][yFrontRight], white);
	}
}

uint8 ColonyEngine::wallAt(int x, int y) const {
	if (x < 0 || x >= 32 || y < 0 || y >= 32)
		return 3;
	return _wall[x][y];
}

void ColonyEngine::checkleft(int xs, int ys, int xf, int yf, int left, int right, int rx, int ry, int cellx, int celly, int len) {
	int i = 0, j;
	int xf2, yf2;
	int rox, roy;
	int xsstart, ysstart;
	int xfstart, yfstart;
	int xestart, yestart;
	int cellxsave, cellysave;
	int dr[2];
	uint32 white = _gfx->white();

	cellx -= _sidex;
	celly -= _sidey;
	rx = rx - _tsin;
	ry = ry - _tcos;

	while (i < len && left <= right) {
		if (wallAt(xs, ys) & _side) {
			if (_flip)
				_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);

			while ((wallAt(xs, ys) & _side) && i < len && left <= right) {
				_gfx->drawLine(_drX[xf][yf], _drY[xf][yf], _drX[xf][yf], _height - _drY[xf][yf], white);
				_gfx->drawLine(_drX[xf][yf], _height - _drY[xf][yf],
				               _drX[xf + _frntx][yf + _frnty], _height - _drY[xf + _frntx][yf + _frnty], white);

				left = MAX(_drX[xf][yf], left);
				xf += _frntx;
				yf += _frnty;
				xs += _frntx;
				ys += _frnty;
				cellx += _frntx;
				celly += _frnty;
				rx -= _tcos;
				ry += _tsin;
				i++;
			}

			if (_flip)
				_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);
			_gfx->drawLine(_drX[xf][yf], _height - _drY[xf][yf], _drX[xf][yf], _drY[xf][yf], white);
			left = MAX(_drX[xf][yf], left);
		}

		if (i < len && left <= right) {
			j = 0;
			xf2 = xf - _sidex;
			yf2 = yf - _sidey;
			xfstart = xf2;
			yfstart = yf2;
			xsstart = xs - _sidex;
			ysstart = ys - _sidey;
			cellxsave = cellx;
			cellysave = celly;

			rox = rx;
			roy = ry;
			if (_change) {
				perspective(dr, rx, ry);
				_drX[xf2][yf2] = dr[0];
				_drY[xf2][yf2] = dr[1];
			}

			while (!(wallAt(xs, ys) & _side) && i < len) {
				rx -= _tcos;
				ry += _tsin;
				if (_change) {
					perspective(dr, rx, ry);
					_drX[xf2 + _frntx][yf2 + _frnty] = dr[0];
					_drY[xf2 + _frntx][yf2 + _frnty] = dr[1];
				}

				if (_drX[xf + _frntx][yf + _frnty] > left) {
					_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2],
					               _drX[xf2 + _frntx][yf2 + _frnty], _drY[xf2 + _frntx][yf2 + _frnty], white);
					_gfx->drawLine(_drX[xf2 + _frntx][yf2 + _frnty], _drY[xf2 + _frntx][yf2 + _frnty],
					               _drX[xf + _frntx][yf + _frnty], _drY[xf + _frntx][yf + _frnty], white);
					if (wallAt(cellx, celly) & ~0x03)
						features(cellx, celly, xf2 + _frntx, yf2 + _frnty, left, right, rx, ry);
				} else {
					j = 0;
					xfstart = xf2;
					yfstart = yf2;
					xsstart = xs - _sidex;
					ysstart = ys - _sidey;
					rox = rx + _tcos;
					roy = ry - _tsin;
					cellxsave = cellx;
					cellysave = celly;
				}

				xf2 += _frntx;
				yf2 += _frnty;
				xf += _frntx;
				yf += _frnty;
				xs += _frntx;
				ys += _frnty;
				cellx += _frntx;
				celly += _frnty;
				i++;
				j++;
			}

			if (wallAt(xf - _sidex, yf - _sidey) & _front) {
				_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf2][yf2], _height - _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _height - _drY[xf2][yf2], _drX[xf][yf], _height - _drY[xf][yf], white);

				if (MIN(_drX[xf2][yf2], right) >= left) {
					checkleft(xsstart, ysstart, xfstart, yfstart, left, MIN(right, _drX[xf2][yf2]),
					          rox, roy, cellxsave, cellysave, j);
				}
			} else {
				if (_flip)
					_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);
				xestart = xf2;
				yestart = yf2;

				while (!(wallAt(xf2, yf2) & _front)) {
					rx -= _tcos;
					ry += _tsin;
					cellx += _frntx;
					celly += _frnty;
					xf2 += _frntx;
					yf2 += _frnty;
					xf += _frntx;
					yf += _frnty;
					xs += _frntx;
					ys += _frnty;
					if (_change) {
						perspective(dr, rx, ry);
						_drX[xf2][yf2] = dr[0];
						_drY[xf2][yf2] = dr[1];
					}
					if (_change) {
						perspective(dr, rx + _tsin, ry + _tcos);
						_drX[xf][yf] = dr[0];
						_drY[xf][yf] = dr[1];
					}
					_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf][yf], _drY[xf][yf], white);
					if (wallAt(cellx - _frntx, celly - _frnty) & ~0x03)
						features(cellx - _frntx, celly - _frnty, xf2, yf2, left, right, rx, ry);
					i++;
					j++;
				}

				_gfx->drawLine(_drX[xestart][yestart], _drY[xestart][yestart], _drX[xf2][yf2], _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf2][yf2], _height - _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _height - _drY[xf2][yf2],
				               _drX[xf2 + _sidex][yf2 + _sidey], _height - _drY[xf2 + _sidex][yf2 + _sidey], white);

				if (MIN(_drX[xf2][yf2], right) >= left) {
					checkleft(xsstart, ysstart, xfstart, yfstart, left, MIN(_drX[xf2][yf2], right),
					          rox, roy, cellxsave, cellysave, j);
				}
			}
		}
	}
}

void ColonyEngine::checkright(int xs, int ys, int xf, int yf, int left, int right, int rx, int ry, int cellx, int celly, int len) {
	int i = 0, j;
	int xf2, yf2;
	int rox, roy;
	int xsstart, ysstart;
	int xfstart, yfstart;
	int xestart, yestart;
	int cellxsave, cellysave;
	int dr[2];
	uint32 white = _gfx->white();

	cellx += _sidex;
	celly += _sidey;
	rx = rx + _tsin;
	ry = ry + _tcos;

	while (i < len && left < right) {
		if (wallAt(xs, ys) & _side) {
			if (_flip)
				_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);

			while ((wallAt(xs, ys) & _side) && i < len && left < right) {
				_gfx->drawLine(_drX[xf][yf], _drY[xf][yf], _drX[xf][yf], _height - _drY[xf][yf], white);
				_gfx->drawLine(_drX[xf][yf], _height - _drY[xf][yf],
				               _drX[xf + _frntx][yf + _frnty], _height - _drY[xf + _frntx][yf + _frnty], white);

				right = MIN(_drX[xf][yf], right);
				xf += _frntx;
				yf += _frnty;
				xs += _frntx;
				ys += _frnty;
				cellx += _frntx;
				celly += _frnty;
				rx -= _tcos;
				ry += _tsin;
				i++;
			}

			if (_flip)
				_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);
			_gfx->drawLine(_drX[xf][yf], _height - _drY[xf][yf], _drX[xf][yf], _drY[xf][yf], white);
			right = MIN(_drX[xf][yf], right);
		}

		if (i < len && left < right) {
			j = 0;
			xf2 = xf + _sidex;
			yf2 = yf + _sidey;
			xfstart = xf2;
			yfstart = yf2;
			xsstart = xs + _sidex;
			ysstart = ys + _sidey;
			cellxsave = cellx;
			cellysave = celly;

			rox = rx;
			roy = ry;
			if (_change) {
				perspective(dr, rx, ry);
				_drX[xf2][yf2] = dr[0];
				_drY[xf2][yf2] = dr[1];
			}

			while (!(wallAt(xs, ys) & _side) && i < len) {
				rx -= _tcos;
				ry += _tsin;
				if (_change) {
					perspective(dr, rx, ry);
					_drX[xf2 + _frntx][yf2 + _frnty] = dr[0];
					_drY[xf2 + _frntx][yf2 + _frnty] = dr[1];
				}

				if (_drX[xf + _frntx][yf + _frnty] < right) {
					_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2],
					               _drX[xf2 + _frntx][yf2 + _frnty], _drY[xf2 + _frntx][yf2 + _frnty], white);
					_gfx->drawLine(_drX[xf2 + _frntx][yf2 + _frnty], _drY[xf2 + _frntx][yf2 + _frnty],
					               _drX[xf + _frntx][yf + _frnty], _drY[xf + _frntx][yf + _frnty], white);
					if (wallAt(cellx, celly) & ~0x03)
						features(cellx, celly, xf + _frntx, yf + _frnty, left, right, rx - _tsin, ry - _tcos);
				} else {
					j = 0;
					xfstart = xf2;
					yfstart = yf2;
					xsstart = xs + _sidex;
					ysstart = ys + _sidey;
					rox = rx + _tcos;
					roy = ry - _tsin;
					cellxsave = cellx;
					cellysave = celly;
				}

				xf2 += _frntx;
				yf2 += _frnty;
				xf += _frntx;
				yf += _frnty;
				xs += _frntx;
				ys += _frnty;
				cellx += _frntx;
				celly += _frnty;
				i++;
				j++;
			}

			if (wallAt(xf, yf) & _front) {
				_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf2][yf2], _height - _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _height - _drY[xf2][yf2], _drX[xf][yf], _height - _drY[xf][yf], white);

				if (MAX(_drX[xf2][yf2], left) < right) {
					checkright(xsstart, ysstart, xfstart, yfstart, MAX(left, _drX[xf2][yf2]), right,
					           rox, roy, cellxsave, cellysave, j);
				}
			} else {
				if (_flip)
					_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);
				xestart = xf2;
				yestart = yf2;

				while (!(wallAt(xf, yf) & _front)) {
					rx -= _tcos;
					ry += _tsin;
					cellx += _frntx;
					celly += _frnty;
					xf2 += _frntx;
					yf2 += _frnty;
					xf += _frntx;
					yf += _frnty;
					xs += _frntx;
					ys += _frnty;
					if (_change) {
						perspective(dr, rx, ry);
						_drX[xf2][yf2] = dr[0];
						_drY[xf2][yf2] = dr[1];
					}
					if (_change) {
						perspective(dr, rx - _tsin, ry - _tcos);
						_drX[xf][yf] = dr[0];
						_drY[xf][yf] = dr[1];
					}
					_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf][yf], _drY[xf][yf], white);
					if (wallAt(cellx - _frntx, celly - _frnty) & ~0x03)
						features(cellx - _frntx, celly - _frnty, xf, yf, left, right, rx - _tsin, ry - _tcos);
					i++;
					j++;
				}

				_gfx->drawLine(_drX[xestart][yestart], _drY[xestart][yestart], _drX[xf2][yf2], _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf2][yf2], _height - _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _height - _drY[xf2][yf2],
				               _drX[xf2 - _sidex][yf2 - _sidey], _height - _drY[xf2 - _sidex][yf2 - _sidey], white);

				if (MAX(_drX[xf2][yf2], left) < right) {
					checkright(xsstart, ysstart, xfstart, yfstart, MAX(_drX[xf2][yf2], left), right,
					           rox, roy, cellxsave, cellysave, j);
				}
			}
		}
	}
}

const uint8 *ColonyEngine::mapFeatureAt(int x, int y, int direction) const {
	if (x < 0 || x >= 31 || y < 0 || y >= 31 || direction < 0 || direction >= 5)
		return nullptr;
	return _mapData[x][y][direction];
}

void ColonyEngine::frontfeature(int cellx, int celly, int xFront, int yFront, int left, int right, int rx, int ry) {
	int l[4], r[4];

	l[0] = _drX[xFront][yFront];
	l[2] = rx - _tcos;
	l[3] = ry + _tsin;
	r[0] = _drX[xFront + _sidex][yFront + _sidey];
	r[2] = rx + _tsin - _tcos;
	r[3] = ry + _tsin + _tcos;
	if (_flip) {
		l[1] = _height - _drY[xFront][yFront];
		r[1] = _height - _drY[xFront + _sidex][yFront + _sidey];
	} else {
		l[1] = _drY[xFront][yFront];
		r[1] = _drY[xFront + _sidex][yFront + _sidey];
	}

	if (MAX(left, l[0]) < MIN(right, r[0])) {
		const uint8 *map = mapFeatureAt(cellx, celly, _direction);
		if (map && map[0])
			dowall(cellx, celly, _direction, l, r);
	}
}

void ColonyEngine::features(int cellx, int celly, int xFront, int yFront, int left, int right, int rx, int ry) {
	int l[4], r[4], ll[4], rr[4];

	l[0] = _drX[xFront][yFront];
	l[2] = rx - _tcos;
	l[3] = ry + _tsin;
	r[0] = _drX[xFront + _sidex][yFront + _sidey];
	r[2] = rx + _tsin - _tcos;
	r[3] = ry + _tsin + _tcos;
	if (_flip) {
		l[1] = _height - _drY[xFront][yFront];
		r[1] = _height - _drY[xFront + _sidex][yFront + _sidey];
	} else {
		l[1] = _drY[xFront][yFront];
		r[1] = _drY[xFront + _sidex][yFront + _sidey];
	}

	if (MAX(left, l[0]) + 1 < MIN(right, r[0]) - 1) {
		const uint8 *map = mapFeatureAt(cellx, celly, _direction);
		if (map && map[0])
			dowall(cellx, celly, _direction, l, r);
	}

	ll[0] = r[0];
	ll[1] = r[1];
	ll[2] = rx + _tsin + _tsin;
	ll[3] = ry + _tcos + _tcos;
	rr[0] = _drX[xFront + _sidex - _frntx][yFront + _sidey - _frnty];
	if (_flip)
		rr[1] = _height - _drY[xFront + _sidex - _frntx][yFront + _sidey - _frnty];
	else
		rr[1] = _drY[xFront + _sidex - _frntx][yFront + _sidey - _frnty];
	rr[2] = rx + _tsin + _tsin + _tcos;
	rr[3] = ry + _tcos + _tcos - _tsin;
	if (MAX(left, ll[0]) + 1 < MIN(right, rr[0]) - 1) {
		const uint8 *map = mapFeatureAt(cellx, celly, g_dirRight[_direction]);
		if (map && map[0])
			dowall(cellx, celly, g_dirRight[_direction], ll, rr);
	}

	ll[0] = _drX[xFront - _frntx][yFront - _frnty];
	if (_flip)
		ll[1] = _height - _drY[xFront - _frntx][yFront - _frnty];
	else
		ll[1] = _drY[xFront - _frntx][yFront - _frnty];
	ll[2] = rx + _tcos - _tsin;
	ll[3] = (ry - _tcos) - _tsin;
	rr[0] = l[0];
	rr[1] = l[1];
	rr[2] = rx - _tsin;
	rr[3] = ry - _tcos;
	if (MAX(left, ll[0]) + 1 < MIN(right, rr[0]) - 1) {
		const uint8 *map = mapFeatureAt(cellx, celly, g_dirLeft[_direction]);
		if (map && map[0])
			dowall(cellx, celly, g_dirLeft[_direction], ll, rr);
	}
}

void ColonyEngine::dowall(int cellx, int celly, int direction, int left[4], int right[4]) {
	const uint8 *map = mapFeatureAt(cellx, celly, direction);
	int left2[2], right2[2];
	if (!map)
		return;

	switch (map[0]) {
	case kWallFeatureDoor:
		if (_level == 1 || _level == 5 || _level == 6) {
			if (map[1] == 0)
				drawOpenSSDoor(left, right);
			else
				drawClosedSSDoor(left, right);
		} else {
			if (map[1] == 0) {
				perspective(left2, left[2], left[3]);
				perspective(right2, right[2], right[3]);
				if (_flip) {
					left2[1] = _height - left2[1];
					right2[1] = _height - right2[1];
				}
				drawOpenDoor(left, right, left2, right2);
			} else {
				drawClosedDoor(left, right);
			}
		}
		break;
	case kWallFeatureWindow:
		drawWindow(left, right);
		break;
	case kWallFeatureShelves:
		perspective(left2, left[2], left[3]);
		perspective(right2, right[2], right[3]);
		if (_flip) {
			left2[1] = _height - left2[1];
			right2[1] = _height - right2[1];
		}
		drawBooks(left, right, left2, right2);
		break;
	case kWallFeatureUpStairs:
		perspective(left2, left[2], left[3]);
		perspective(right2, right[2], right[3]);
		if (_flip) {
			left2[1] = _height - left2[1];
			right2[1] = _height - right2[1];
		}
		drawUpStairs(left, right, left2, right2);
		break;
	case kWallFeatureDnStairs:
		perspective(left2, left[2], left[3]);
		perspective(right2, right[2], right[3]);
		if (_flip) {
			left2[1] = _height - left2[1];
			right2[1] = _height - right2[1];
		}
		drawDnStairs(left, right, left2, right2);
		break;
	case kWallFeatureGlyph:
		drawGlyphs(left, right);
		break;
	case kWallFeatureElevator:
		drawElevator(left, right);
		break;
	case kWallFeatureTunnel:
		perspective(left2, left[2], left[3]);
		perspective(right2, right[2], right[3]);
		if (_flip) {
			left2[1] = _height - left2[1];
			right2[1] = _height - right2[1];
		}
		drawTunnel(left, right, left2, right2);
		break;
	case kWallFeatureAirlock:
		if (map[1] == 0)
			drawALOpen(left, right);
		else
			drawALClosed(left, right);
		break;
	case kWallFeatureColor:
		drawColor(map, left, right);
		break;
	default:
		break;
	}
}

void ColonyEngine::drawWindow(int left[4], int right[4]) {
	const uint32 dark = 160;
	int x1 = left[0];
	int x2 = right[0];
	int y1 = left[1];
	int y2 = right[1];
	int y3 = _height - right[1];
	int y4 = _height - left[1];
	int xc = (x1 + x2) >> 1;
	int xx1 = (xc + x1) >> 1;
	int xx2 = (xc + x2) >> 1;
	if (xx2 < _screenR.left || xx1 > _screenR.right)
		return;
	int yl = (y1 + y4) >> 1;
	int yr = (y2 + y3) >> 1;
	int yy1 = (yl + y1) >> 1;
	int yy2 = (yr + y2) >> 1;
	int yy3 = (yl + y3) >> 1;
	int yy4 = (yr + y4) >> 1;
	int yy[4];
	yy[0] = _height - ((((yy1 + yy2) >> 1) + yy1) >> 1);
	yy[1] = _height - ((((yy1 + yy2) >> 1) + yy2) >> 1);
	yy[2] = _height - ((((yy3 + yy4) >> 1) + yy3) >> 1);
	yy[3] = _height - ((((yy3 + yy4) >> 1) + yy4) >> 1);
	_gfx->drawLine(xx1, yy[0], xx2, yy[1], dark);
	_gfx->drawLine(xx2, yy[1], xx2, yy[2], dark);
	_gfx->drawLine(xx2, yy[2], xx1, yy[3], dark);
	_gfx->drawLine(xx1, yy[3], xx1, yy[0], dark);
	_gfx->drawLine(xc, (yy[0] + yy[1]) >> 1, xc, (yy[2] + yy[3]) >> 1, dark);
	_gfx->drawLine(xx1, yl, xx2, yr, dark);
}

void ColonyEngine::drawClosedDoor(int left[4], int right[4]) {
	const uint32 dark = 160;
	int x1 = left[0];
	int x2 = right[0];
	int y1 = left[1];
	int y2 = right[1];
	int y3 = _height - right[1];
	int y4 = _height - left[1];
	int xc = (x1 + x2) >> 1;
	int xx1 = (xc + x1) >> 1;
	int xx2 = (xc + x2) >> 1;
	if (xx2 < _screenR.left || xx1 > _screenR.right)
		return;

	int yc = (y1 + y2) >> 1;
	int ytl = (yc + y1) >> 1;
	int ytr = (yc + y2) >> 1;
	yc = (y4 + y3) >> 1;
	int ybl = (yc + y4) >> 1;
	int ybr = (yc + y3) >> 1;
	ytl = (((((ybl + ytl) >> 1) + ytl) >> 1) + ytl) >> 1;
	ytr = (((((ybr + ytr) >> 1) + ytr) >> 1) + ytr) >> 1;

	_gfx->drawLine(xx1, ybl, xx1, ytl, dark);
	_gfx->drawLine(xx1, ytl, xx2, ytr, dark);
	_gfx->drawLine(xx2, ytr, xx2, ybr, dark);
	_gfx->drawLine(xx2, ybr, xx1, ybl, dark);

	ybl = (ybl + ytl) >> 1;
	ybr = (ybr + ytr) >> 1;
	yc = (ybl + ybr) >> 1;
	ybl = (((yc + ybl) >> 1) + ybl) >> 1;
	ybr = (((yc + ybr) >> 1) + ybr) >> 1;
	xx1 = (((xx1 + xc) >> 1) + xx1) >> 1;
	xx2 = (((xx2 + xc) >> 1) + xx2) >> 1;
	_gfx->drawLine(xx1, ybl, xx2, ybr, dark);
}

void ColonyEngine::drawOpenDoor(int left[4], int right[4], int left2[2], int right2[2]) {
	const uint32 dark = 160;
	const uint32 light = 210;
	int x1 = left[0];
	int x2 = right[0];
	int y1 = left[1];
	int y2 = right[1];
	int y3 = _height - right[1];
	int y4 = _height - left[1];
	int xc = (x1 + x2) >> 1;
	int xl = (xc + x1) >> 1;
	int xr = (xc + x2) >> 1;
	int yc = (y1 + y2) >> 1;
	int ytl = (yc + y1) >> 1;
	int ytr = (yc + y2) >> 1;
	yc = (y4 + y3) >> 1;
	int ybl = (yc + y4) >> 1;
	int ybr = (yc + y3) >> 1;
	ytl = (((((ybl + ytl) >> 1) + ytl) >> 1) + ytl) >> 1;
	ytr = (((((ybr + ytr) >> 1) + ytr) >> 1) + ytr) >> 1;
	if (xr < _screenR.left || xl > _screenR.right)
		return;

	_gfx->drawLine(xl, ybl, xl, ytl, dark);
	_gfx->drawLine(xl, ytl, xr, ytr, dark);
	_gfx->drawLine(xr, ytr, xr, ybr, dark);
	_gfx->drawLine(xr, ybr, xl, ybl, dark);

	x1 = left2[0];
	x2 = right2[0];
	y1 = _height - left2[1];
	y2 = _height - right2[1];
	xc = (x1 + x2) >> 1;
	int xfl = (xc + x1) >> 1;
	int xfr = (xc + x2) >> 1;
	yc = (y1 + y2) >> 1;
	int yfl = (yc + y1) >> 1;
	int yfr = (yc + y2) >> 1;

	_gfx->drawLine(xl, ybl, xfl, yfl, light);
	_gfx->drawLine(xfl, yfl, xfr, yfr, light);
	_gfx->drawLine(xfr, yfr, xr, ybr, light);
	_gfx->drawLine(xr, ybr, xl, ybl, light);
}

void ColonyEngine::drawTunnel(int left[4], int right[4], int left2[2], int right2[2]) {
	const uint32 dark = 120;
	int baseX[7], baseY[7], tunnelY[7][7];
	int xl = left[0];
	int xr = right[0];
	int ytl = left[1];
	int ytr = right[1];
	int ybr = _height - right[1];
	int ybl = _height - left[1];
	int hl = ybl - ytl;
	int hr = ybr - ytr;
	(void)left2;
	(void)right2;
	(void)MAX(hl, hr);
	split7(baseX, xl, xr);
	if (baseX[0] > _screenR.right || baseX[6] < _screenR.left)
		return;
	split7(baseY, ybl, ybr);
	for (int i = 0; i < 7; i++)
		split7(tunnelY[i], baseY[i], _height - baseY[i]);

	int x[6] = {baseX[0], baseX[0], baseX[1], baseX[5], baseX[6], baseX[6]};
	int y[6] = {baseY[0], tunnelY[0][5], tunnelY[1][6], tunnelY[5][6], tunnelY[6][5], baseY[6]};
	for (int i = 0; i < 6; i++) {
		int n = (i + 1) % 6;
		_gfx->drawLine(x[i], y[i], x[n], y[n], dark);
	}
}

void ColonyEngine::drawGlyphs(int left[4], int right[4]) {
	const uint32 dark = 170;
	int xl = left[0];
	int xr = right[0];
	int y1 = left[1];
	int y2 = right[1];
	int y3 = _height - right[1];
	int y4 = _height - left[1];
	int xc = (xl + xr) >> 1;
	xl = (((xc + xl) >> 1) + xl) >> 1;
	xr = (((xc + xr) >> 1) + xr) >> 1;
	int ytc = (y1 + y2) >> 1;
	int ybc = (y3 + y4) >> 1;
	int ytl = (((y1 + ytc) >> 1) + y1) >> 1;
	int ytr = (((y2 + ytc) >> 1) + y2) >> 1;
	int ybl = (((y4 + ybc) >> 1) + y4) >> 1;
	int ybr = (((y3 + ybc) >> 1) + y3) >> 1;
	int yl1 = (ytl + ybl) >> 1;
	int yr1 = (ytr + ybr) >> 1;
	int yl2 = (yl1 + ytl) >> 1;
	int yr2 = (yr1 + ytr) >> 1;
	int yl3 = (yl2 + yl1) >> 1;
	int yr3 = (yr2 + yr1) >> 1;
	int yl4 = (yl1 + ybl) >> 1;
	int yr4 = (yr1 + ybr) >> 1;
	int yr5 = (yr4 + yr1) >> 1;
	int yl5 = (yl4 + yl1) >> 1;

	_gfx->drawLine(xl, yl1, xr, yr1, dark);
	_gfx->drawLine(xl, yl2, xr, yr2, dark);
	_gfx->drawLine(xl, yl3, xr, yr3, dark);
	_gfx->drawLine(xl, yl4, xr, yr4, dark);
	_gfx->drawLine(xl, yl5, xr, yr5, dark);
	_gfx->drawLine(xl, (yl2 + yl3) >> 1, xr, (yr2 + yr3) >> 1, dark);
	_gfx->drawLine(xl, (yl3 + yl1) >> 1, xr, (yr3 + yr1) >> 1, dark);
	_gfx->drawLine(xl, (yl1 + yl5) >> 1, xr, (yr1 + yr5) >> 1, dark);
	_gfx->drawLine(xl, (yl4 + yl5) >> 1, xr, (yr4 + yr5) >> 1, dark);
}

void ColonyEngine::drawBooks(int left[4], int right[4], int left2[2], int right2[2]) {
	const uint32 dark = 170;
	int l2[2] = {left2[0], left2[1]};
	int r2[2] = {right2[0], right2[1]};
	for (int i = 0; i < 2; i++) {
		l2[0] = (l2[0] + left[0]) >> 1;
		l2[1] = (l2[1] + left[1]) >> 1;
		r2[0] = (r2[0] + right[0]) >> 1;
		r2[1] = (r2[1] + right[1]) >> 1;
	}
	_gfx->drawLine(l2[0], l2[1], l2[0], _height - l2[1], dark);
	_gfx->drawLine(l2[0], _height - l2[1], r2[0], _height - r2[1], dark);
	_gfx->drawLine(r2[0], _height - r2[1], r2[0], r2[1], dark);
	_gfx->drawLine(r2[0], r2[1], l2[0], l2[1], dark);
	_gfx->drawLine(left[0], left[1], l2[0], l2[1], dark);
	_gfx->drawLine(left[0], _height - left[1], l2[0], _height - l2[1], dark);
	_gfx->drawLine(right[0], right[1], r2[0], r2[1], dark);
	_gfx->drawLine(right[0], _height - right[1], r2[0], _height - r2[1], dark);

	int lf[7], rf[7], lb[7], rb[7];
	split7(lf, left[1], _height - left[1]);
	split7(rf, right[1], _height - right[1]);
	split7(lb, l2[1], _height - l2[1]);
	split7(rb, r2[1], _height - r2[1]);
	for (int i = 0; i < 7; i++) {
		_gfx->drawLine(left[0], lf[i], right[0], rf[i], dark);
		_gfx->drawLine(right[0], rf[i], r2[0], rb[i], dark);
		_gfx->drawLine(r2[0], rb[i], l2[0], lb[i], dark);
		_gfx->drawLine(l2[0], lb[i], left[0], lf[i], dark);
	}
}

void ColonyEngine::drawUpStairs(int left[4], int right[4], int left2[2], int right2[2]) {
	const uint32 dark = 170;
	int xl[7], xr[7], yl[7], yr[7];
	split7(xl, left[0], left2[0]);
	split7(xr, right[0], right2[0]);
	split7(yl, _height - left[1], left2[1]);
	split7(yr, _height - right[1], right2[1]);
	for (int i = 0; i < 6; i++) {
		_gfx->drawLine(xl[i], yl[i], xl[i + 1], yl[i + 1], dark);
		_gfx->drawLine(xr[i], yr[i], xr[i + 1], yr[i + 1], dark);
		_gfx->drawLine(xl[i], yl[i], xr[i], yr[i], dark);
	}
}

void ColonyEngine::drawDnStairs(int left[4], int right[4], int left2[2], int right2[2]) {
	const uint32 dark = 170;
	int xl[7], xr[7], yl[7], yr[7];
	split7(xl, left[0], left2[0]);
	split7(xr, right[0], right2[0]);
	split7(yl, left[1], left2[1]);
	split7(yr, right[1], right2[1]);
	for (int i = 0; i < 6; i++) {
		_gfx->drawLine(xl[i], yl[i], xl[i + 1], yl[i + 1], dark);
		_gfx->drawLine(xr[i], yr[i], xr[i + 1], yr[i + 1], dark);
		_gfx->drawLine(xl[i], _height - yl[i], xr[i], _height - yr[i], dark);
	}
}

void ColonyEngine::drawALOpen(int left[4], int right[4]) {
	const uint32 dark = 150;
	int lr[7], ud[7][7];
	split7x7(left, right, lr, ud);
	int x[8] = {lr[0], lr[1], lr[3], lr[5], lr[6], lr[5], lr[3], lr[1]};
	int y[8] = {ud[3][0], ud[5][1], ud[6][3], ud[5][5], ud[3][6], ud[1][5], ud[0][3], ud[1][1]};
	for (int i = 0; i < 8; i++) {
		int n = (i + 1) % 8;
		_gfx->drawLine(x[i], y[i], x[n], y[n], dark);
	}
}

void ColonyEngine::drawALClosed(int left[4], int right[4]) {
	const uint32 dark = 170;
	int lr[7], ud[7][7];
	split7x7(left, right, lr, ud);
	int x[8] = {lr[0], lr[1], lr[3], lr[5], lr[6], lr[5], lr[3], lr[1]};
	int y[8] = {ud[3][0], ud[5][1], ud[6][3], ud[5][5], ud[3][6], ud[1][5], ud[0][3], ud[1][1]};
	for (int i = 0; i < 8; i++) {
		int n = (i + 1) % 8;
		_gfx->drawLine(x[i], y[i], x[n], y[n], dark);
	}
	_gfx->drawLine(lr[0], ud[3][0], lr[3], ud[3][3], dark);
	_gfx->drawLine(lr[3], ud[6][3], lr[3], ud[3][3], dark);
	_gfx->drawLine(lr[6], ud[3][6], lr[3], ud[3][3], dark);
	_gfx->drawLine(lr[3], ud[0][3], lr[3], ud[3][3], dark);
}

void ColonyEngine::drawOpenSSDoor(int left[4], int right[4]) {
	const uint32 dark = 140;
	int lr[7], ud[7][7];
	split7x7(left, right, lr, ud);
	int x[8] = {lr[2], lr[1], lr[1], lr[2], lr[4], lr[5], lr[5], lr[4]};
	int y[8] = {ud[0][2], ud[1][1], ud[5][1], ud[6][2], ud[6][4], ud[5][5], ud[1][5], ud[0][4]};
	for (int i = 0; i < 8; i++) {
		int n = (i + 1) % 8;
		_gfx->drawLine(x[i], y[i], x[n], y[n], dark);
	}
}

void ColonyEngine::drawClosedSSDoor(int left[4], int right[4]) {
	const uint32 dark = 170;
	int lr[7], ud[7][7];
	split7x7(left, right, lr, ud);
	int x[8] = {lr[2], lr[1], lr[1], lr[2], lr[4], lr[5], lr[5], lr[4]};
	int y[8] = {ud[0][2], ud[1][1], ud[5][1], ud[6][2], ud[6][4], ud[5][5], ud[1][5], ud[0][4]};
	for (int i = 0; i < 8; i++) {
		int n = (i + 1) % 8;
		_gfx->drawLine(x[i], y[i], x[n], y[n], dark);
	}
	_gfx->drawLine(lr[2], ud[1][2], lr[2], ud[5][2], dark);
	_gfx->drawLine(lr[2], ud[5][2], lr[4], ud[5][4], dark);
	_gfx->drawLine(lr[4], ud[5][4], lr[4], ud[1][4], dark);
	_gfx->drawLine(lr[4], ud[1][4], lr[2], ud[1][2], dark);
}

void ColonyEngine::drawElevator(int left[4], int right[4]) {
	const uint32 dark = 170;
	int x1 = left[0];
	int x2 = right[0];
	int y1 = left[1];
	int y2 = right[1];
	int y3 = _height - right[1];
	int y4 = _height - left[1];
	int xc = (x1 + x2) >> 1;
	int xx1 = (xc + x1) >> 1;
	xx1 = (x1 + xx1) >> 1;
	int xx2 = (xc + x2) >> 1;
	xx2 = (x2 + xx2) >> 1;
	if (xx2 < _screenR.left || xx1 > _screenR.right)
		return;
	int ytc = (y1 + y2) >> 1;
	int ytl = (ytc + y1) >> 1;
	ytl = (ytl + y1) >> 1;
	int ytr = (ytc + y2) >> 1;
	ytr = (ytr + y2) >> 1;
	int ybc = (y4 + y3) >> 1;
	int ybl = (ybc + y4) >> 1;
	ybl = (ybl + y4) >> 1;
	int ybr = (ybc + y3) >> 1;
	ybr = (ybr + y3) >> 1;
	ytl = (((((ybl + ytl) >> 1) + ytl) >> 1) + ytl) >> 1;
	ytr = (((((ybr + ytr) >> 1) + ytr) >> 1) + ytr) >> 1;
	_gfx->drawLine(xx1, ybl, xx1, ytl, dark);
	_gfx->drawLine(xx1, ytl, xx2, ytr, dark);
	_gfx->drawLine(xx2, ytr, xx2, ybr, dark);
	_gfx->drawLine(xx2, ybr, xx1, ybl, dark);
	_gfx->drawLine(xc, ybc, xc, (ytl + ytr) >> 1, dark);
}

void ColonyEngine::drawColor(const uint8 *map, int left[4], int right[4]) {
	int xl = left[0];
	int xr = right[0];
	int yl[5], yr[5];
	yl[0] = left[1];
	yr[0] = right[1];
	yl[4] = _height - yl[0];
	yr[4] = _height - yr[0];
	yl[2] = (yl[0] + yl[4]) >> 1;
	yr[2] = (yr[0] + yr[4]) >> 1;
	yl[1] = (yl[0] + yl[2]) >> 1;
	yl[3] = (yl[2] + yl[4]) >> 1;
	yr[1] = (yr[0] + yr[2]) >> 1;
	yr[3] = (yr[2] + yr[4]) >> 1;

	if (map[1] || map[2] || map[3] || map[4]) {
		for (int i = 1; i <= 3; i++) {
			uint32 c = 120 + map[i] * 20;
			_gfx->drawLine(xl, yl[i], xr, yr[i], c);
		}
	} else {
		uint32 c = 100 + (_level * 15);
		_gfx->drawLine(xl, yl[1], xr, yr[1], c);
		_gfx->drawLine(xl, yl[2], xr, yr[2], c);
		_gfx->drawLine(xl, yl[3], xr, yr[3], c);
	}
}

void ColonyEngine::split7(int arr[7], int x1, int x2) const {
	arr[3] = (x1 + x2) >> 1;
	arr[1] = (x1 + arr[3]) >> 1;
	arr[0] = (x1 + arr[1]) >> 1;
	arr[2] = (arr[1] + arr[3]) >> 1;
	arr[5] = (arr[3] + x2) >> 1;
	arr[6] = (arr[5] + x2) >> 1;
	arr[4] = (arr[3] + arr[5]) >> 1;
}

void ColonyEngine::split7x7(int left[4], int right[4], int lr[7], int ud[7][7]) const {
	int leftX, rightX, leftY, rightY;
	int lud[7], rud[7];
	if (right[0] < left[0]) {
		rightX = left[0];
		leftX = right[0];
		rightY = left[1];
		leftY = right[1];
	} else {
		leftX = left[0];
		rightX = right[0];
		leftY = left[1];
		rightY = right[1];
	}
	split7(lr, leftX, rightX);
	if (_flip) {
		split7(lud, leftY, _height - leftY);
		split7(rud, rightY, _height - rightY);
	} else {
		split7(lud, _height - leftY, leftY);
		split7(rud, _height - rightY, rightY);
	}
	for (int i = 0; i < 7; i++)
		split7(ud[i], lud[i], rud[i]);
}

Common::Error ColonyEngine::run() {
	Graphics::PixelFormat format8bpp = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(_width, _height, &format8bpp);

	_width = _system->getWidth();
	_height = _system->getHeight();
	_centerX = _width / 2;
	_centerY = _height / 2;
	_screenR = Common::Rect(0, 0, _width, _height);
	_clip = _screenR;
	const Graphics::PixelFormat format = _system->getScreenFormat();
	debug("Screen format: %d bytesPerPixel. Actual size: %dx%d", format.bytesPerPixel, _width, _height);

	// Setup a grayscale palette
	byte pal[256 * 3];
	for (int i = 0; i < 256; i++) {
		pal[i * 3 + 0] = i;
		pal[i * 3 + 1] = i;
		pal[i * 3 + 2] = i;
	}
	_system->getPaletteManager()->setPalette(pal, 0, 256);

	_gfx = new Gfx(_system, _width, _height);
	
	loadMap(1); // Try to load the first map
	
	_system->lockMouse(true);
	_system->warpMouse(_width / 2, _height / 2);

	// Temporary infinite loop to prevent ScummVM from closing immediately
	while (!shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN) {
				debug("Key down: %d", event.kbd.keycode);
				switch (event.kbd.keycode) {
				case Common::KEYCODE_UP:
				{
					int xnew = _me.xloc + (_cost[_me.look] >> 2);
					int ynew = _me.yloc + (_sint[_me.look] >> 2);
					checkwall(xnew, ynew, &_me);
					break;
				}
				case Common::KEYCODE_DOWN:
				{
					int xnew = _me.xloc - (_cost[_me.look] >> 2);
					int ynew = _me.yloc - (_sint[_me.look] >> 2);
					checkwall(xnew, ynew, &_me);
					break;
				}
				case Common::KEYCODE_LEFT:
					_me.look = (uint8)((int)_me.look + 8);
					_change = true;
					break;
				case Common::KEYCODE_RIGHT:
					_me.look = (uint8)((int)_me.look - 8);
					_change = true;
					break;
				default:
					break;
				}
				debug("Me: x=%d y=%d look=%d", _me.xloc, _me.yloc, _me.look);
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				if (event.relMouse.x != 0) {
					// Subtract because increasing look turns left, but mouse right is positive rel X
					// Reduced sensitivity by half
					_me.look = (uint8)((int)_me.look - (event.relMouse.x / 2));
					_change = true;
				}
			}
		}
		_system->warpMouse(_width / 2, _height / 2);

		_gfx->clear(_gfx->black());
		
		corridor();
		
		_gfx->copyToScreen();
		_system->delayMillis(10);
	}

	delete _gfx;
	return Common::kNoError;
}

} // End of namespace Colony
