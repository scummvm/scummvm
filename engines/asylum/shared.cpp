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
 * $URL$
 * $Id$
 *
 */

#include "asylum/shared.h"

#include "common/system.h"

DECLARE_SINGLETON(Asylum::SharedResources);

namespace Asylum {

static bool g_initialized = false;

SharedResources::SharedResources() {
	if (!g_initialized) {
		g_initialized = true;
	}
    memset(_gameFlags, 0, 1512);
}

SharedResources::~SharedResources() {
	g_initialized = false;
}

bool SharedResources::pointInPoly(PolyDefinitions *poly, int x, int y) {
	// Copied from backends/vkeybd/polygon.cpp
	int  yflag0;
	int  yflag1;
	bool inside_flag = false;
	unsigned int pt;

	Common::Point *vtx0 = &poly->points[poly->numPoints - 1];
	Common::Point *vtx1 = &poly->points[0];

	yflag0 = (vtx0->y >= y);
	for (pt = 0; pt < poly->numPoints; pt++, vtx1++) {
		yflag1 = (vtx1->y >= y);
		if (yflag0 != yflag1) {
			if (((vtx1->y - y) * (vtx0->x - vtx1->x) >=
				(vtx1->x - x) * (vtx0->y - vtx1->y)) == yflag1) {
				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0   = vtx1;
	}

	return inside_flag;
}

int SharedResources::getAngle(int x1, int y1, int x2, int y2) {
	uint32 v5 = (x2 << 16) - (x1 << 16);
	int v6 = 0;
	int v4 = (y1 << 16) - (y2 << 16);

	if (v5 < 0) {
		v6 = 2;
		v5 = -v5;
	}

	if (v4 < 0) {
		v6 |= 1;
		v4 = -v4;
	}

	int v7;
	int v8 = -1;

	if (v5) {
		v7 = (v4 << 8) / v5;

		if (v7 < 0x100)
			v8 = angleTable01[v7];
		if (v7 < 0x1000 && v8 < 0)
			v8 = angleTable02[v7 >> 4];
		if (v7 < 0x10000 && v8 < 0)
			v8 = angleTable03[v7 >> 8];
	} else {
		v8 = 90;
	}

	switch (v6) {
	case 1:
		v8 = 360 - v8;
		break;
	case 2:
		v8 = 180 - v8;
		break;
	case 3:
		v8 += 180;
		break;
	}

	if (v8 >= 360)
		v8 -= 360;

	int result;

	if (v8 < 157 || v8 >= 202) {
		if (v8 < 112 || v8 >= 157) {
			if (v8 < 67 || v8 >= 112) {
				if (v8 < 22 || v8 >= 67) {
					if ((v8 < 0 || v8 >= 22) && (v8 < 337 || v8 > 359)) {
						if (v8 < 292 || v8 >= 337) {
							if (v8 < 247 || v8 >= 292) {
								if (v8 < 202 || v8 >= 247) {
									error("getAngle returned a bad angle: %d.", v8);
									result = x1;
								} else {
									result = 3;
								}
							} else {
								result = 4;
							}
						} else {
							result = 5;
						}
					} else {
						result = 6;
					}
				} else {
					result = 7;
				}
			} else {
				result = 0;
			}
		} else {
			result = 1;
		}
	} else {
		result = 2;
	}

	return result;
}

void SharedResources::setGameFlag(int flag) {
	_gameFlags[flag / 32] |= 1 << flag % -32;
}

void SharedResources::clearGameFlag(int flag) {
	_gameFlags[flag / 32] &= ~(1 << flag % -32);
}

void SharedResources::toggleGameFlag(int flag) {
	_gameFlags[flag / 32] ^= 1 << flag % -32;
}

bool SharedResources::isGameFlagSet(int flag) {
	return ((1 << flag % -32) & (unsigned int)_gameFlags[flag / 32]) >> flag % -32 != 0;
}

bool SharedResources::isGameFlagNotSet(int flag) {
	return ((1 << flag % -32) & (unsigned int)_gameFlags[flag / 32]) >> flag % -32 == 0;
}

} // end of namespace Asylum
