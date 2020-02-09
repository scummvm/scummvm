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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ACCESS_AMAZON_RESOURCES_H
#define ACCESS_AMAZON_RESOURCES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "access/resources.h"
#include "access/font.h"

namespace Access {

namespace Amazon {

enum InventoryEnum {
	INV_BAITED_POLE = 67, INV_TORCH = 76, INV_KNIFE_SPEAR = 78
};

struct RiverStruct {
	int _id;
	int _width;
	int _riverX;
	int _xp;
	int _lane;
	int _offsetY;
};

extern const int SIDEOFFR[];
extern const int SIDEOFFL[];
extern const int SIDEOFFU[];
extern const int SIDEOFFD[];
extern const int DIAGOFFURX[];
extern const int DIAGOFFURY[];
extern const int DIAGOFFDRX[];
extern const int DIAGOFFDRY[];
extern const int DIAGOFFULX[];
extern const int DIAGOFFULY[];
extern const int DIAGOFFDLX[];
extern const int DIAGOFFDLY[];

extern const int _travelPos[][2];

extern const int OVEROFFR[];
extern const int OVEROFFL[];
extern const int OVEROFFU[];
extern const int OVEROFFD[];
extern const int OVEROFFURX[];
extern const int OVEROFFURY[];
extern const int OVEROFFDRX[];
extern const int OVEROFFDRY[];
extern const int OVEROFFULX[];
extern const int OVEROFFULY[];
extern const int OVEROFFDLX[];
extern const int OVEROFFDLY[];

extern const int DEATH_CELLS[13][3];

extern const int CHAPTER_CELLS[17][3];

extern const int CHAPTER_TABLE[14][5];

extern const int CHAPTER_JUMP[14];

extern const int COMBO_TABLE[85][4];

extern const int ANTWALK[24];

extern const int ANTEAT[33];

extern const int ANTDIE[21];

extern const int PITWALK[27];

extern const int PITSTAB[21];

extern const int TORCH[12];

extern const int SPEAR[3];

extern const int OPENING_OBJS[10][4];

extern const byte MAP0[26];
extern const byte MAP1[27];
extern const byte MAP2[32];

extern const byte *const MAPTBL[3];

extern const int DOWNRIVEROBJ[14][4];

extern RiverStruct RIVER0OBJECTS[46];
extern RiverStruct RIVER1OBJECTS[50];
extern RiverStruct RIVER2OBJECTS[54];
extern RiverStruct *RIVER_OBJECTS[3][2];
enum { RIVER_START = 0, RIVER_END = 1 };

extern const int HELP1COORDS[2][4];

extern const int RIVER1OBJ[23][4];

extern const int CAST_END_OBJ[26][4];
extern const int CAST_END_OBJ1[4][4];

extern const int RMOUSE[10][2];

class AmazonResources: public Resources {
protected:
	/**
	 * Load data from the access.dat file
	 */
	void load(Common::SeekableReadStream &s) override;
public:
	AmazonFont *_font3x5, *_font6x6;
	Common::String NO_HELP_MESSAGE;
	Common::String NO_HINTS_MESSAGE;
	Common::String RIVER_HIT1;
	Common::String RIVER_HIT2;
	Common::String BAR_MESSAGE;
	Common::String HELPLVLTXT[3];
	Common::String IQLABELS[9];
public:
	AmazonResources(AccessEngine *vm) : Resources(vm), _font3x5(nullptr), _font6x6(nullptr) {}
	~AmazonResources() override;
};

#define AMRES (*((Amazon::AmazonResources *)_vm->_res))

} // End of namespace Amazon
} // End of namespace Access

#endif /* ACCESS_AMAZON_RESOURCES_H */
