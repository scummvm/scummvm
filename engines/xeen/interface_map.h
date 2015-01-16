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

#ifndef XEEN_INTERFACE_MAP_H
#define XEEN_INTERFACE_MAP_H

#include "common/scummsys.h"
#include "xeen/map.h"
#include "xeen/screen.h"

namespace Xeen {

class XeenEngine;

class OutdoorDrawList {
public:
	DrawStruct _data[132];
	DrawStruct &_skySprite;
	DrawStruct &_groundSprite;
public:
	OutdoorDrawList();

	DrawStruct &operator[](int idx) { 
		assert(idx < size());
		return _data[idx]; 
	}

	int size() const { return 132; }
};

class IndoorDrawList {
public:
	DrawStruct _data[170];
	DrawStruct &_sky;
	DrawStruct &_ground;
	DrawStruct &_horizon;
	DrawStruct &_swl_0F1R, _swl_0F1L, _swl_1F1R, _swl_1F1L,
		_swl_2F2R, _swl_2F1R, _swl_2F1L, _swl_2F2L,
		_swl_3F1R, _swl_3F2R, _swl_3F3R, _swl_3F4R,
		_swl_3F1L, _swl_3F2L, _swl_3F3L, _swl_3F4L,
		_swl_4F4R, _swl_4F3R, _swl_4F2R, _swl_4F1R,
		_swl_4F1L, _swl_4F2L, _swl_4F3L, _swl_4F4L;
	DrawStruct &_fwl_4F4R, _fwl_4F3R, _fwl_4F2R, _fwl_4F1R,
		_fwl_4F, _fwl_4F1L, _fwl_4F2L, _fwl_4F3L, _fwl_4F4L;
	DrawStruct &_fwl_2F1R, _fwl_2F, _fwl_2F1L, _fwl_3F2R,
		_fwl_3F1R, _fwl_3F, _fwl_3F1L, _fwl_3F2L;
	DrawStruct &_fwl_1F, _fwl_1F1R, _fwl_1F1L;
	DrawStruct &_objects0, _objects1, _objects2, _objects3;
	DrawStruct &_objects4, _objects5, _objects6, _objects7;
	DrawStruct &_objects8, _objects9, _objects10, _objects11;
public:
	IndoorDrawList();

	DrawStruct &operator[](int idx) { 
		assert(idx < size());
		return _data[idx]; 
	}

	int size() const { return 170; }
};


class InterfaceMap {
private:
	XeenEngine *_vm;
	int _combatFloatCounter;

	void initDrawStructs();

	void setMonsterSprite(DrawStruct &drawStruct, MazeMonster &monster, 
		SpriteResource *sprites, int frame, int defaultY);
protected:
	int8 _wp[20];
	byte _wo[308];
	int _overallFrame;
	bool _flipWtr;
	bool _flipWall;
	bool _flipSky;
	bool _flipGround;
	bool _isShooting;
	bool _charsShooting;
	int _objNumber;
public:
	OutdoorDrawList _outdoorList;
	IndoorDrawList _indoorList;
public:
	InterfaceMap(XeenEngine *vm);

	void setIndoorsAttackingMonsters();

	void setIndoorObjects();

	void setIndoorsWallPics();

	void drawIndoors();

	void setOutdoorsMonsters();

	void setOutdoorsObjects();
};

} // End of namespace Xeen

#endif /* XEEN_INTERFACE_MAP_H */
