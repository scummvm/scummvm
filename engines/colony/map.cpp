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
 * Based on the original sources
 *   https://github.com/Croquetx/thecolony
 * Copyright (C) 1988, David A. Smith
 *
 * Distributed under Apache Version 2.0 License
 *
 */

#include "common/debug.h"
#include "common/file.h"
#include "common/macresman.h"

#include "colony/colony.h"

namespace Colony {

const int kRobotTypeOrder[] = {
	0,
	kRobQueen,
	kRobSnoop,
	kRobDrone,
	kRobSoldier,
	kRobEye,
	kRobPyramid,
	kRobUPyramid,
	kRobCube,
	kRobMEye,
	kRobMPyramid,
	kRobMUPyramid,
	kRobMCube,
	kRobFEye,
	kRobFPyramid,
	kRobFUPyramid,
	kRobFCube,
	kRobSEye,
	kRobSPyramid,
	kRobSUPyramid,
	kRobSCube
};

const int kDynamicObjectLimit = kMeNum - 1;
const int kReservedPlayerSlotIndex = kMeNum - 1;
const int kStaticObjectStartIndex = kMeNum;
const int kMaxObjectSlots = 255;

void clearThing(Thing &thing) {
	thing = Thing();
}

void ensureObjectLayout(Common::Array<Thing> &objects) {
	const int oldSize = (int)objects.size();
	if (oldSize >= kMeNum)
		return;

	objects.resize(kMeNum);
	for (int i = oldSize; i < kMeNum; i++)
		clearThing(objects[i]);
}

void resetObjectLayout(Common::Array<Thing> &objects) {
	objects.clear();
	objects.resize(kMeNum);
	for (int i = 0; i < kMeNum; i++)
		clearThing(objects[i]);
}

void ColonyEngine::loadMap(int mnum) {
	saveLevelState();

	Common::Path mapPath(Common::String::format("MAP.%d", mnum));
	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(mapPath);
	if (!file) {
		warning("Could not open map file %s", mapPath.toString().c_str());
		return;
	}

	file->readUint32BE(); // "DAVE" header
	for (int i = 0; i < 10; i++) {
		file->readSint16BE(); // skip map defs
	}

	uint16 bLength = file->readUint16BE();
	uint8 *buffer = new uint8[bLength];
	file->read(buffer, bLength);
	delete file;

	memset(_mapData, 0, sizeof(_mapData));
	memset(_robotArray, 0, sizeof(_robotArray));
	memset(_foodArray, 0, sizeof(_foodArray));
	memset(_dirXY, 0, sizeof(_dirXY));
	resetObjectLayout(_objects);

	// expand logic
	int c = 0;
	_robotNum = kMeNum + 1;
	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < 32; j++) {
			_wall[i][j] = buffer[c++];
			if (i >= 31 || j >= 31)
				continue;

			for (int k = 0; k < 5; k++) {
				if (_wall[i][j] & (1 << (k + 2))) {
					for (int l = 0; l < 5; l++) {
						_mapData[i][j][k][l] = buffer[c++];
					}
					// PACKIT.C: center feature type 6 marks static map objects.
					if (k == 4 && _mapData[i][j][4][0] == 6) {
						Thing obj;
						clearThing(obj);
						obj.alive = 1;
						obj.visible = 0;
						obj.type = _mapData[i][j][4][1] + kBaseObject;
						obj.where.xloc = (i << 8) + 128;
						obj.where.yloc = (j << 8) + 128;
						obj.where.xindex = i;
						obj.where.yindex = j;
						obj.where.ang = (uint8)(_mapData[i][j][4][2] + 32);
						obj.where.look = obj.where.ang;
						if ((int)_objects.size() >= kMaxObjectSlots) {
							warning("loadMap: object table full on level %d, skipping static object type %d at (%d,%d)",
								mnum, obj.type, i, j);
							continue;
						}
						_objects.push_back(obj);
						const int objNum = (int)_objects.size(); // 1-based, DOS-style robot slots
						// CWall/FWall use diagonal collision, not cell-based blocking.
						if (obj.type != kObjFWall && obj.type != kObjCWall &&
							objNum > 0 && objNum < 256 && _robotArray[i][j] == 0)
							_robotArray[i][j] = (uint8)objNum;
					}
				} else {
					_mapData[i][j][k][0] = 0;
				}
			}
		}
	}
	delete[] buffer;
	_dynamicObjectBase = kStaticObjectStartIndex;
	_robotNum = MAX<int>(_robotNum, (int)_objects.size() + 1);
	_level = mnum;
	_me.type = kMeNum;

	getWall();  // restore saved wall state changes (airlocks)
	doPatch();  // apply object relocations from patch table
	initRobots();  // spawn robot objects for this level

	if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
		_robotArray[_me.xindex][_me.yindex] = kMeNum;
	debugC(1, kColonyDebugMap, "Successfully loaded map %d (objects: %d)", mnum, (int)_objects.size());
}

void ColonyEngine::resetObjectSlot(int slot, int type, int xloc, int yloc, uint8 ang) {
	if (slot < 0 || slot >= (int)_objects.size())
		return;
	if (slot == kReservedPlayerSlotIndex) {
		warning("resetObjectSlot: refusing to use reserved player slot for type %d on level %d", type, _level);
		return;
	}

	Thing &obj = _objects[slot];
	clearThing(obj);
	const int lvl = MIN<int>(MAX<int>(_level - 1, 0), 5);
	obj.opcode = 3; // FORWARD
	obj.alive = 1;
	obj.visible = 0;
	obj.type = type;
	obj.where.xloc = xloc;
	obj.where.yloc = yloc;
	obj.where.xindex = xloc >> 8;
	obj.where.yindex = yloc >> 8;
	obj.where.delta = 4;
	obj.where.ang = ang;
	obj.where.look = ang;
	obj.where.lookx = 0;
	obj.where.wallPad = robotWallPad(type);
	obj.count = 0;
	obj.time = 10 + (_randomSource.getRandomNumber(0x3F) & 0x3F);
	obj.grow = 0;

	if (type <= kBaseObject) {
		int basePower = 0;
		if (type == kRobQueen) {
			if (_level == 7)
				basePower = 32000;
			else
				basePower = 40 + ((_randomSource.getRandomNumber(0x5F) & 0x5F) << lvl);
		} else if (type == kRobSoldier) {
			basePower = 30 + ((_randomSource.getRandomNumber(0x1F) & 0x1F) << lvl);
		} else {
			basePower = 20 + ((_randomSource.getRandomNumber(0x0F) & 0x0F) << lvl);
		}

		obj.where.power[0] = basePower;
		obj.where.power[1] = basePower;
		obj.where.power[2] = basePower;
	}

	const int objNum = slot + 1;
	if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
		obj.where.yindex >= 0 && obj.where.yindex < 32) {
		if (type > kRobUPyramid && type < kRobQueen)
			_foodArray[obj.where.xindex][obj.where.yindex] = (uint8)objNum;
		else
			_robotArray[obj.where.xindex][obj.where.yindex] = (uint8)objNum;
	}
	if (objNum >= _robotNum)
		_robotNum = objNum + 1;
}

// PATCH.C: Create a new object in _objects and register in the proper grid.
bool ColonyEngine::createObject(int type, int xloc, int yloc, uint8 ang) {
	ensureObjectLayout(_objects);
	if (_dynamicObjectBase < kStaticObjectStartIndex || _dynamicObjectBase > (int)_objects.size())
		_dynamicObjectBase = kStaticObjectStartIndex;
	if (_robotNum < kMeNum + 1)
		_robotNum = kMeNum + 1;

	int slot = -1;
	if (type <= kBaseObject) {
		for (int j = 0; j < kDynamicObjectLimit; j++) {
			if (!_objects[j].alive) {
				slot = j;
				break;
			}
		}
		if (slot < 0) {
			debugC(2, kColonyDebugMap, "createObject: no free dynamic slot for type %d on level %d", type, _level);
			return false;
		}
	} else {
		const int searchEnd = MIN<int>((int)_objects.size(), kMaxObjectSlots);
		for (int j = _dynamicObjectBase; j < searchEnd; j++) {
			if (!_objects[j].alive) {
				slot = j;
				break;
			}
		}
		if (slot < 0) {
			if ((int)_objects.size() >= kMaxObjectSlots) {
				warning("createObject: object table full, cannot place type %d on level %d", type, _level);
				return false;
			}

			Thing obj;
			clearThing(obj);
			_objects.push_back(obj);
			slot = (int)_objects.size() - 1;
		}
	}

	resetObjectSlot(slot, type, xloc, yloc, ang);
	return true;
}

void ColonyEngine::saveLevelState() {
	if (_level < 1 || _level > 7 || _gameMode != kModeColony)
		return;

	LevelData &ld = _levelData[_level - 1];
	ld.visit = 1;
	ld.queen = _allGrow ? 1 : 0;
	memset(ld.object, 0, sizeof(ld.object));

	const int dynamicCount = MIN<int>((int)_objects.size(), kDynamicObjectLimit);
	for (int i = 0; i < dynamicCount; i++) {
		const Thing &obj = _objects[i];
		if (!obj.alive || obj.type <= 0 || obj.type > kBaseObject)
			continue;

		if (ld.object[obj.type] < 255)
			ld.object[obj.type]++;
	}
}

// PATCH.C: DoPatch()  remove originals and install relocated objects.
void ColonyEngine::doPatch() {
	// Pass 1: remove objects that were moved away from this level
	for (uint i = 0; i < _patches.size(); i++) {
		if (_level == _patches[i].from.level) {
			int robot = _robotArray[_patches[i].from.xindex][_patches[i].from.yindex];
			if (robot > 0 && robot <= (int)_objects.size()) {
				_robotArray[_objects[robot - 1].where.xindex][_objects[robot - 1].where.yindex] = 0;
				_objects[robot - 1].alive = 0;
			}
		}
	}
	// Pass 2: install objects that were moved to this level
	for (uint i = 0; i < _patches.size(); i++) {
		if (_level == _patches[i].to.level) {
			if (!createObject(
				(int)_patches[i].type,
				(int)_patches[i].to.xloc,
				(int)_patches[i].to.yloc,
				_patches[i].to.ang)) {
				warning("doPatch: failed to restore patched object type %d on level %d",
					(int)_patches[i].type, _level);
			}
		}
	}
}

// PATCH.C: savewall()  save 5 bytes of map feature data for persistence across level loads.
void ColonyEngine::saveWall(int x, int y, int direction) {
	if (_level < 1 || _level > 8)
		return;
	LevelData &ld = _levelData[_level - 1];

	// Search for existing entry at this location
	for (int i = 0; i < ld.size; i++) {
		if (ld.location[i][0] == x && ld.location[i][1] == y && ld.location[i][2] == direction) {
			for (int j = 0; j < 5; j++)
				ld.data[i][j] = _mapData[x][y][direction][j];
			return;
		}
	}
	// Add new entry (max 10)
	if (ld.size >= 10) {
		warning("saveWall: too many wall changes for level %d", _level);
		return;
	}
	int i = ld.size;
	for (int j = 0; j < 5; j++)
		ld.data[i][j] = _mapData[x][y][direction][j];
	ld.location[i][0] = x;
	ld.location[i][1] = y;
	ld.location[i][2] = direction;
	ld.size++;
}

// PATCH.C: getwall()  restore saved wall bytes into _mapData after level load.
void ColonyEngine::getWall() {
	if (_level < 1 || _level > 8)
		return;
	const LevelData &ld = _levelData[_level - 1];
	for (int i = 0; i < ld.size; i++) {
		int x = ld.location[i][0];
		int y = ld.location[i][1];
		int dir = ld.location[i][2];
		if (x < 31 && y < 31 && dir < 5) {
			for (int j = 0; j < 5; j++)
				_mapData[x][y][dir][j] = ld.data[i][j];
		}
	}
}

// PATCH.C: newpatch()  create or update a patch entry.
void ColonyEngine::newPatch(int type, const PassPatch &from, const PassPatch &to, const uint8 *mapdata) {
	// Search for existing patch where 'from' matches an existing 'to'
	for (uint i = 0; i < _patches.size(); i++) {
		if (from.level == _patches[i].to.level &&
			from.xindex == _patches[i].to.xindex &&
			from.yindex == _patches[i].to.yindex) {
			_patches[i].to.level = to.level;
			_patches[i].to.xindex = to.xindex;
			_patches[i].to.yindex = to.yindex;
			_patches[i].to.xloc = to.xloc;
			_patches[i].to.yloc = to.yloc;
			_patches[i].to.ang = to.ang;
			return;
		}
	}
	// Create new patch entry (max 100)
	if (_patches.size() >= 100)
		return;
	PatchEntry pe;
	pe.type = type;
	pe.from.level = from.level;
	pe.from.xindex = from.xindex;
	pe.from.yindex = from.yindex;
	pe.to.level = to.level;
	pe.to.xindex = to.xindex;
	pe.to.yindex = to.yindex;
	pe.to.xloc = to.xloc;
	pe.to.yloc = to.yloc;
	pe.to.ang = to.ang;
	if (mapdata) {
		for (int j = 0; j < 5; j++)
			pe.mapdata[j] = mapdata[j];
	} else {
		memset(pe.mapdata, 0, sizeof(pe.mapdata));
	}
	_patches.push_back(pe);
}

// PATCH.C: patchmapto()  find patch entry by destination, fill mapdata.
bool ColonyEngine::patchMapTo(const PassPatch &to, uint8 *mapdata) {
	for (uint i = 0; i < _patches.size(); i++) {
		if (to.level == _patches[i].to.level &&
			to.xindex == _patches[i].to.xindex &&
			to.yindex == _patches[i].to.yindex) {
			for (int j = 0; j < 5; j++)
				mapdata[j] = _patches[i].mapdata[j];
			return true;
		}
	}
	return false;
}

// PATCH.C: patchmapfrom()  find patch entry by source, fill destination into mapdata.
bool ColonyEngine::patchMapFrom(const PassPatch &from, uint8 *mapdata) {
	for (uint i = 0; i < _patches.size(); i++) {
		if (from.level == _patches[i].from.level &&
			from.xindex == _patches[i].from.xindex &&
			from.yindex == _patches[i].from.yindex) {
			mapdata[2] = _patches[i].to.level;
			mapdata[3] = _patches[i].to.xindex;
			mapdata[4] = _patches[i].to.yindex;
			return true;
		}
	}
	return false;
}

// DOS InitObject()  spawn robots for the current level.
// Level 1 = no robots; Level 2 = 25; Level 3-4 = 30; Level 5-7 = 35.
// Robot #1 = QUEEN, #2 = SNOOP, rest = random type weighted by level.
void ColonyEngine::initRobots() {
	_allGrow = false;
	if (_level == 1)
		return;  // Level 1 has no robots

	LevelData &ld = _levelData[_level - 1];
	int maxrob;
	switch (_level) {
	case 2:  maxrob = 25; break;
	case 3:
	case 4:  maxrob = 30; break;
	default: maxrob = 35; break;
	}

	int lvl = _level - 1;
	if (lvl > 5)
		lvl = 5;

	auto spawnType = [&](int type) {
		uint8 ang = _randomSource.getRandomNumber(255);
		int xloc, yloc;

		// Find unoccupied cell (avoiding borders)
		do {
			if (_level == 7 && type == kRobQueen) {
				// Queen on level 7 has fixed position
				xloc = 27;
				yloc = 10;
			} else {
				xloc = 2 + _randomSource.getRandomNumber(26);  // 2..28
				yloc = 2 + _randomSource.getRandomNumber(26);  // 2..28
			}
		} while (_robotArray[xloc][yloc] != 0 || _foodArray[xloc][yloc] != 0);

		// Convert grid coords to world coords (center of cell)
		int wxloc = (xloc << 8) + 128;
		int wyloc = (yloc << 8) + 128;
		if (!createObject(type, wxloc, wyloc, ang)) {
			warning("initRobots: failed to spawn type %d on level %d", type, _level);
			return false;
		}
		return true;
	};

	if (ld.visit) {
		_allGrow = (ld.queen != 0);
		for (uint i = 1; i < ARRAYSIZE(kRobotTypeOrder); i++) {
			const int type = kRobotTypeOrder[i];
			for (int count = 0; count < ld.object[type]; count++)
				spawnType(type);
		}
		if (!_allGrow) {
			for (uint i = 0; i < _objects.size(); i++) {
				Thing &obj = _objects[i];
				if (obj.alive && obj.type >= kRobEye && obj.type <= kRobUPyramid)
					obj.grow = -1;
			}
		}
		debugC(1, kColonyDebugMap, "initRobots: restored %d robot/object types on level %d", maxrob, _level);
		return;
	}

	_allGrow = true;
	ld.queen = 1;
	for (int i = 1; i <= maxrob; i++) {
		int type;
		if (i == 1) {
			type = kRobQueen;
		} else if (i == 2) {
			type = kRobSnoop;
		} else {
			int rnd = _randomSource.getRandomNumber(lvl);
			if (rnd > 5)
				rnd = 5;
			switch (rnd) {
			case 0: type = kRobCube; break;
			case 1: type = kRobPyramid; break;
			case 2: type = kRobUPyramid; break;
			case 3: type = kRobEye; break;
			case 4: type = kRobDrone; break;
			case 5: type = kRobSoldier; break;
			default: type = kRobCube; break;
			}
		}

		spawnType(type);
	}

	debugC(1, kColonyDebugMap, "initRobots: spawned %d robots on level %d", maxrob, _level);
}

} // End of namespace Colony
