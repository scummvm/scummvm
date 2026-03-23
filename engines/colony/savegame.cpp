/* ScummVM - Graphic Adventure Engine
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

#include "common/translation.h"

#include "colony/colony.h"

namespace Colony {

namespace {

static const uint32 kSaveVersion = 1;
static const uint32 kMaxSaveObjects = 4096;
static const uint32 kMaxSavePatches = 100;

Common::Error makeReadError(const char *msg) {
	return Common::Error(Common::kReadingFailed, msg);
}

Common::Error makeWriteError(const char *msg) {
	return Common::Error(Common::kWritingFailed, msg);
}

Common::Error makeCorruptSaveError(const char *reason) {
	warning("Colony savegame load failed: %s", reason);
	return makeReadError("Invalid or corrupt Colony savegame");
}

void writeRect(Common::WriteStream *stream, const Common::Rect &rect) {
	stream->writeSint32LE(rect.left);
	stream->writeSint32LE(rect.top);
	stream->writeSint32LE(rect.right);
	stream->writeSint32LE(rect.bottom);
}

Common::Rect readRect(Common::SeekableReadStream *stream) {
	const int left = stream->readSint32LE();
	const int top = stream->readSint32LE();
	const int right = stream->readSint32LE();
	const int bottom = stream->readSint32LE();
	return Common::Rect(left, top, right, bottom);
}

void writeLocate(Common::WriteStream *stream, const Locate &loc) {
	stream->writeByte(loc.ang);
	stream->writeByte(loc.look);
	stream->writeByte((byte)loc.lookY);
	stream->writeSint32LE(loc.lookx);
	stream->writeSint32LE(loc.delta);
	stream->writeSint32LE(loc.xloc);
	stream->writeSint32LE(loc.yloc);
	stream->writeSint32LE(loc.xindex);
	stream->writeSint32LE(loc.yindex);
	stream->writeSint32LE(loc.xmx);
	stream->writeSint32LE(loc.xmn);
	stream->writeSint32LE(loc.zmx);
	stream->writeSint32LE(loc.zmn);
	for (int i = 0; i < 3; i++)
		stream->writeSint32LE(loc.power[i]);
	stream->writeSint32LE(loc.type);
	stream->writeSint32LE(loc.dx);
	stream->writeSint32LE(loc.dy);
	stream->writeSint32LE(loc.dist);
}

Locate readLocate(Common::SeekableReadStream *stream) {
	Locate loc;
	loc.ang = stream->readByte();
	loc.look = stream->readByte();
	loc.lookY = (int8)stream->readSByte();
	loc.lookx = stream->readSint32LE();
	loc.delta = stream->readSint32LE();
	loc.xloc = stream->readSint32LE();
	loc.yloc = stream->readSint32LE();
	loc.xindex = stream->readSint32LE();
	loc.yindex = stream->readSint32LE();
	loc.xmx = stream->readSint32LE();
	loc.xmn = stream->readSint32LE();
	loc.zmx = stream->readSint32LE();
	loc.zmn = stream->readSint32LE();
	for (int i = 0; i < 3; i++)
		loc.power[i] = stream->readSint32LE();
	loc.type = stream->readSint32LE();
	loc.dx = stream->readSint32LE();
	loc.dy = stream->readSint32LE();
	loc.dist = stream->readSint32LE();
	return loc;
}

void writeThing(Common::WriteStream *stream, const Thing &thing) {
	stream->writeSint32LE(thing.type);
	stream->writeSint32LE(thing.visible);
	stream->writeSint32LE(thing.alive);
	writeRect(stream, thing.clip);
	stream->writeSint32LE(thing.count);
	writeLocate(stream, thing.where);
	stream->writeSint32LE(thing.opcode);
	stream->writeSint32LE(thing.counter);
	stream->writeSint32LE(thing.time);
	stream->writeSint32LE(thing.grow);
}

Thing readThing(Common::SeekableReadStream *stream) {
	Thing thing;
	thing.type = stream->readSint32LE();
	thing.visible = stream->readSint32LE();
	thing.alive = stream->readSint32LE();
	thing.clip = readRect(stream);
	thing.count = stream->readSint32LE();
	thing.where = readLocate(stream);
	thing.opcode = stream->readSint32LE();
	thing.counter = stream->readSint32LE();
	thing.time = stream->readSint32LE();
	thing.grow = stream->readSint32LE();
	return thing;
}

void writePassPatch(Common::WriteStream *stream, const PassPatch &patch) {
	stream->writeByte(patch.level);
	stream->writeByte(patch.xindex);
	stream->writeByte(patch.yindex);
	stream->writeSint32LE(patch.xloc);
	stream->writeSint32LE(patch.yloc);
	stream->writeByte(patch.ang);
}

PassPatch readPassPatch(Common::SeekableReadStream *stream) {
	PassPatch patch;
	patch.level = stream->readByte();
	patch.xindex = stream->readByte();
	patch.yindex = stream->readByte();
	patch.xloc = stream->readSint32LE();
	patch.yloc = stream->readSint32LE();
	patch.ang = stream->readByte();
	return patch;
}

void writePatchEntry(Common::WriteStream *stream, const PatchEntry &entry) {
	stream->writeByte(entry.from.level);
	stream->writeByte(entry.from.xindex);
	stream->writeByte(entry.from.yindex);
	stream->writeByte(entry.to.level);
	stream->writeByte(entry.to.xindex);
	stream->writeByte(entry.to.yindex);
	stream->writeSint32LE(entry.to.xloc);
	stream->writeSint32LE(entry.to.yloc);
	stream->writeByte(entry.to.ang);
	stream->writeByte(entry.type);
	for (int i = 0; i < 5; i++)
		stream->writeByte(entry.mapdata[i]);
}

PatchEntry readPatchEntry(Common::SeekableReadStream *stream) {
	PatchEntry entry;
	entry.from.level = stream->readByte();
	entry.from.xindex = stream->readByte();
	entry.from.yindex = stream->readByte();
	entry.to.level = stream->readByte();
	entry.to.xindex = stream->readByte();
	entry.to.yindex = stream->readByte();
	entry.to.xloc = stream->readSint32LE();
	entry.to.yloc = stream->readSint32LE();
	entry.to.ang = stream->readByte();
	entry.type = stream->readByte();
	for (int i = 0; i < 5; i++)
		entry.mapdata[i] = stream->readByte();
	return entry;
}

void writeLevelData(Common::WriteStream *stream, const LevelData &levelData) {
	stream->writeByte(levelData.visit);
	stream->writeByte(levelData.queen);
	for (int i = 0; i <= kBaseObject; i++)
		stream->writeByte(levelData.object[i]);
	stream->writeByte(levelData.count);
	stream->writeByte(levelData.size);
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 3; j++)
			stream->writeByte(levelData.location[i][j]);
	}
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++)
			stream->writeByte(levelData.data[i][j]);
	}
}

LevelData readLevelData(Common::SeekableReadStream *stream) {
	LevelData levelData;
	levelData.visit = stream->readByte();
	levelData.queen = stream->readByte();
	for (int i = 0; i <= kBaseObject; i++)
		levelData.object[i] = stream->readByte();
	levelData.count = stream->readByte();
	levelData.size = stream->readByte();
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 3; j++)
			levelData.location[i][j] = stream->readByte();
	}
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++)
			levelData.data[i][j] = stream->readByte();
	}
	return levelData;
}

bool validateGridReferences(const uint8 grid[32][32], uint32 objectCount, bool allowPlayerMarker) {
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 32; x++) {
			const uint8 value = grid[x][y];
			if (value == 0)
				continue;
			if (allowPlayerMarker && value == kMeNum)
				continue;
			if (value > objectCount)
				return false;
		}
	}
	return true;
}

bool findInvalidGridReference(const uint8 grid[32][32], uint32 objectCount, bool allowPlayerMarker, int &invalidX, int &invalidY, uint8 &invalidValue) {
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 32; x++) {
			const uint8 value = grid[x][y];
			if (value == 0)
				continue;
			if (allowPlayerMarker && value == kMeNum)
				continue;
			if (value > objectCount) {
				invalidX = x;
				invalidY = y;
				invalidValue = value;
				return true;
			}
		}
	}
	return false;
}

bool findInvalidActiveObjectSlot(const Common::Array<Thing> &objects, uint32 &invalidObjectNum, int &invalidType) {
	for (uint i = 0; i < objects.size(); i++) {
		if (!objects[i].alive)
			continue;

		const uint32 objectNum = i + 1;
		if (objectNum == (uint32)kMeNum || objectNum > 255) {
			invalidObjectNum = objectNum;
			invalidType = objects[i].type;
			return true;
		}
	}

	return false;
}

} // anonymous namespace

bool ColonyEngine::hasFeature(EngineFeature f) const {
	return f == kSupportsReturnToLauncher ||
		f == kSupportsLoadingDuringRuntime ||
		f == kSupportsSavingDuringRuntime;
}

bool ColonyEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	const bool inFpsView = (_level >= 1 && _level <= 7) &&
		(_gameMode == kModeColony || _gameMode == kModeBattle) &&
		!_animationRunning;
	if (!inFpsView && msg)
		*msg = _("Saving is only available in first-person view.");
	return inFpsView;
}

bool ColonyEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	const bool inFpsView = (_level >= 1 && _level <= 7) &&
		(_gameMode == kModeColony || _gameMode == kModeBattle) &&
		!_animationRunning;
	if (!inFpsView && msg)
		*msg = _("Loading is only available in first-person view.");
	return inFpsView;
}

Common::Error ColonyEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (!stream)
		return makeWriteError("Could not open savegame stream");

	if (_level >= 1 && _level <= 7) {
		const int oldMode = _gameMode;
		if (_gameMode != kModeColony)
			_gameMode = kModeColony;
		saveLevelState();
		_gameMode = oldMode;
	}

	stream->writeUint32LE(kSaveVersion);
	stream->writeSint32LE(_gameMode);
	stream->writeSint32LE(_level);
	stream->writeSint32LE(_robotNum);
	stream->writeSint32LE(_dynamicObjectBase);
	stream->writeUint32LE(_randomSource.getSeed());
	stream->writeByte(_hasKeycard ? 1 : 0);
	stream->writeByte(_unlocked ? 1 : 0);
	stream->writeByte(_allGrow ? 1 : 0);
	stream->writeSint32LE(_weapons);
	stream->writeSint32LE(_armor);
	stream->writeSint32LE(_fl);
	stream->writeSint32LE(_orbit);
	stream->writeSint32LE(_carryType);
	stream->writeSint32LE(_coreIndex);
	stream->writeUint32LE(_displayCount);
	writeLocate(stream, _me);

	for (int i = 0; i < 4; i++) {
		stream->writeByte(_decode1[i]);
		stream->writeByte(_decode2[i]);
		stream->writeByte(_decode3[i]);
	}

	for (int i = 0; i < 2; i++) {
		stream->writeSint32LE(_coreState[i]);
		stream->writeSint32LE(_coreHeight[i]);
	}
	for (int i = 0; i < 3; i++)
		stream->writeSint32LE(_corePower[i]);

	for (uint i = 0; i < ARRAYSIZE(_levelData); i++)
		writeLevelData(stream, _levelData[i]);

	stream->writeUint32LE(_patches.size());
	for (uint i = 0; i < _patches.size(); i++)
		writePatchEntry(stream, _patches[i]);

	for (int i = 0; i < 2; i++)
		writePassPatch(stream, _carryPatch[i]);

	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 32; x++)
			stream->writeByte(_wall[x][y]);
	}
	for (int y = 0; y < 31; y++) {
		for (int x = 0; x < 31; x++) {
			for (int dir = 0; dir < 5; dir++) {
				for (int i = 0; i < 5; i++)
					stream->writeByte(_mapData[x][y][dir][i]);
			}
		}
	}
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 32; x++) {
			stream->writeByte(_robotArray[x][y]);
			stream->writeByte(_foodArray[x][y]);
			stream->writeByte(_dirXY[x][y]);
		}
	}

	stream->writeUint32LE(_objects.size());
	for (uint i = 0; i < _objects.size(); i++)
		writeThing(stream, _objects[i]);

	for (int i = 0; i < 16; i++)
		writeLocate(stream, _bfight[i]);
	writeLocate(stream, _battleEnter);
	writeLocate(stream, _battleShip);
	writeLocate(stream, _battleProj);
	stream->writeByte(_projon ? 1 : 0);
	stream->writeSint32LE(_pcount);
	stream->writeSint32LE(_battleRound);
	for (int i = 0; i < 256; i++)
		stream->writeSint32LE(_mountains[i]);
	for (int qx = 0; qx < 4; qx++) {
		for (int qy = 0; qy < 4; qy++) {
			for (int i = 0; i < 15; i++)
				writeLocate(stream, _pyramids[qx][qy][i]);
		}
	}

	if (stream->err())
		return makeWriteError(isAutosave ? "Failed to write Colony autosave" : "Failed to write Colony savegame");

	return Common::kNoError;
}

Common::Error ColonyEngine::loadGameStream(Common::SeekableReadStream *stream) {
	if (!stream)
		return makeReadError("Could not open savegame stream");

	const uint32 version = stream->readUint32LE();
	if (version != kSaveVersion) {
		warning("Colony savegame load failed: unsupported version %u, expected %u", version, kSaveVersion);
		return makeReadError("Unsupported Colony savegame version");
	}

	const int savedGameMode = stream->readSint32LE();
	const int savedLevel = stream->readSint32LE();
	const int savedRobotNum = stream->readSint32LE();
	const int savedDynamicObjectBase = stream->readSint32LE();
	const uint32 savedSeed = stream->readUint32LE();

	if ((savedGameMode != kModeColony && savedGameMode != kModeBattle) || savedLevel < 1 || savedLevel > 7)
		return makeCorruptSaveError(Common::String::format("invalid header values: mode=%d level=%d robotNum=%d dynamicObjectBase=%d seed=%u",
			savedGameMode, savedLevel, savedRobotNum, savedDynamicObjectBase, savedSeed).c_str());

	_gameMode = savedGameMode;
	_level = savedLevel;
	_robotNum = savedRobotNum;
	_dynamicObjectBase = savedDynamicObjectBase;
	_randomSource.setSeed(savedSeed);
	_hasKeycard = stream->readByte() != 0;
	_unlocked = stream->readByte() != 0;
	_allGrow = stream->readByte() != 0;
	_weapons = stream->readSint32LE();
	_armor = stream->readSint32LE();
	_fl = stream->readSint32LE();
	_orbit = stream->readSint32LE();
	_carryType = stream->readSint32LE();
	_coreIndex = stream->readSint32LE();
	_displayCount = stream->readUint32LE();
	_me = readLocate(stream);

	for (int i = 0; i < 4; i++) {
		_decode1[i] = stream->readByte();
		_decode2[i] = stream->readByte();
		_decode3[i] = stream->readByte();
	}

	for (int i = 0; i < 2; i++) {
		_coreState[i] = stream->readSint32LE();
		_coreHeight[i] = stream->readSint32LE();
	}
	for (int i = 0; i < 3; i++)
		_corePower[i] = stream->readSint32LE();

	for (uint i = 0; i < ARRAYSIZE(_levelData); i++)
		_levelData[i] = readLevelData(stream);

	const uint32 patchCount = stream->readUint32LE();
	if (patchCount > kMaxSavePatches)
		return makeCorruptSaveError(Common::String::format("patch count %u exceeds max %u", patchCount, kMaxSavePatches).c_str());
	_patches.resize(patchCount);
	for (uint i = 0; i < patchCount; i++)
		_patches[i] = readPatchEntry(stream);

	for (int i = 0; i < 2; i++)
		_carryPatch[i] = readPassPatch(stream);

	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 32; x++)
			_wall[x][y] = stream->readByte();
	}
	for (int y = 0; y < 31; y++) {
		for (int x = 0; x < 31; x++) {
			for (int dir = 0; dir < 5; dir++) {
				for (int i = 0; i < 5; i++)
					_mapData[x][y][dir][i] = stream->readByte();
			}
		}
	}
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 32; x++) {
			_robotArray[x][y] = stream->readByte();
			_foodArray[x][y] = stream->readByte();
			_dirXY[x][y] = stream->readByte();
		}
	}

	const uint32 objectCount = stream->readUint32LE();
	if (objectCount > kMaxSaveObjects)
		return makeCorruptSaveError(Common::String::format("object count %u exceeds max %u", objectCount, kMaxSaveObjects).c_str());
	_objects.resize(objectCount);
	for (uint i = 0; i < objectCount; i++) {
		_objects[i] = readThing(stream);
		// wallPad is not serialized — recompute from object type
		_objects[i].where.wallPad = robotWallPad(_objects[i].type);
	}

	for (int i = 0; i < 16; i++)
		_bfight[i] = readLocate(stream);
	_battleEnter = readLocate(stream);
	_battleShip = readLocate(stream);
	_battleProj = readLocate(stream);
	_projon = stream->readByte() != 0;
	_pcount = stream->readSint32LE();
	_battleRound = stream->readSint32LE();
	for (int i = 0; i < 256; i++)
		_mountains[i] = stream->readSint32LE();
	for (int qx = 0; qx < 4; qx++) {
		for (int qy = 0; qy < 4; qy++) {
			for (int i = 0; i < 15; i++)
				_pyramids[qx][qy][i] = readLocate(stream);
		}
	}

	if (stream->err())
		return makeCorruptSaveError("stream read error while decoding save payload");

	if (_coreIndex < 0 || _coreIndex > 1)
		return makeCorruptSaveError(Common::String::format("core index out of range: %d", _coreIndex).c_str());
	if (_weapons < 0 || _weapons > 3 || _armor < 0 || _armor > 3)
		return makeCorruptSaveError(Common::String::format("equipment out of range: weapons=%d armor=%d", _weapons, _armor).c_str());
	if (_fl < 0 || _fl > 2 || _orbit < 0 || _orbit > 1)
		return makeCorruptSaveError(Common::String::format("status out of range: fl=%d orbit=%d", _fl, _orbit).c_str());
	for (uint i = 0; i < ARRAYSIZE(_levelData); i++) {
		if (_levelData[i].size > 10)
			return makeCorruptSaveError(Common::String::format("levelData[%u].size out of range: %u", i, _levelData[i].size).c_str());
	}
	if (_dynamicObjectBase < 0 || _dynamicObjectBase > (int)_objects.size())
		return makeCorruptSaveError(Common::String::format("dynamicObjectBase out of range: %d (objects=%u)", _dynamicObjectBase, objectCount).c_str());
	if (_robotNum < kMeNum + 1 || _robotNum > 255)
		return makeCorruptSaveError(Common::String::format("robotNum out of range: %d", _robotNum).c_str());
	{
		uint32 invalidObjectNum = 0;
		int invalidType = 0;
		if (findInvalidActiveObjectSlot(_objects, invalidObjectNum, invalidType)) {
			return makeCorruptSaveError(Common::String::format("active object slot %u is not addressable by byte-sized grids (type=%d)",
				invalidObjectNum, invalidType).c_str());
		}
	}
	if (!validateGridReferences(_robotArray, objectCount, true)) {
		int invalidX = -1;
		int invalidY = -1;
		uint8 invalidValue = 0;
		findInvalidGridReference(_robotArray, objectCount, true, invalidX, invalidY, invalidValue);
		return makeCorruptSaveError(Common::String::format("robot grid reference out of range at (%d,%d): value=%u objectCount=%u",
			invalidX, invalidY, invalidValue, objectCount).c_str());
	}
	if (!validateGridReferences(_foodArray, objectCount, false)) {
		int invalidX = -1;
		int invalidY = -1;
		uint8 invalidValue = 0;
		findInvalidGridReference(_foodArray, objectCount, false, invalidX, invalidY, invalidValue);
		return makeCorruptSaveError(Common::String::format("food grid reference out of range at (%d,%d): value=%u objectCount=%u",
			invalidX, invalidY, invalidValue, objectCount).c_str());
	}

	deleteAnimation();
	_animationName.clear();
	_backgroundActive = false;
	_animationRunning = false;
	_animationResult = 0;
	_doorOpen = false;
	_elevatorFloor = 0;
	_airlockX = -1;
	_airlockY = -1;
	_airlockDirection = -1;
	_airlockTerminate = false;
	_battleMaxP = 0;
	_suppressCollisionSound = false;
	_moveForward = false;
	_moveBackward = false;
	_strafeLeft = false;
	_strafeRight = false;
	_rotateLeft = false;
	_rotateRight = false;
	_lastClickTime = 0;
	_lastAnimUpdate = 0;
	_lastWarningChimeTime = 0;
	_battledx = _width / 59;
	updateViewportLayout();

	return Common::kNoError;
}

} // End of namespace Colony
