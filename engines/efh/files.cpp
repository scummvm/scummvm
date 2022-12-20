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

#include "efh/efh.h"

namespace Efh {

int32 EfhEngine::readFileToBuffer(Common::String &filename, uint8 *destBuffer) {
	debugC(1, kDebugUtils, "readFileToBuffer %s", filename.c_str());
	Common::File f;
	if (!f.open(filename))
		error("Unable to find file %s", filename.c_str());

	int size = f.size();

	return f.read(destBuffer, size);
}

void EfhEngine::readAnimInfo() {
	debugC(6, kDebugEngine, "readAnimInfo");

	Common::String fileName = "animinfo";
	Common::File f;
	if (!f.open(fileName))
		error("Unable to find file %s", fileName.c_str());

	for (int i = 0; i < 100; ++i) {
		for (int id = 0; id < 15; ++id) {
			_animInfo[i]._unkAnimArray[id]._field[0] = f.readByte();
			_animInfo[i]._unkAnimArray[id]._field[1] = f.readByte();
			_animInfo[i]._unkAnimArray[id]._field[2] = f.readByte();
			_animInfo[i]._unkAnimArray[id]._field[3] = f.readByte();

			debugC(6, kDebugEngine, "%d %d %d %d", _animInfo[i]._unkAnimArray[id]._field[0], _animInfo[i]._unkAnimArray[id]._field[1], _animInfo[i]._unkAnimArray[id]._field[2], _animInfo[i]._unkAnimArray[id]._field[3]);
		}

		Common::String debugStr = "";
		for (int id = 0; id < 10; ++id) {
			_animInfo[i]._field3C_startY[id] = f.readByte();
			debugStr += Common::String::format("%d ", _animInfo[i]._field3C_startY[id]);
		}
		debugC(6, kDebugEngine, "%s", debugStr.c_str());

		debugStr = "";
		for (int id = 0; id < 10; ++id) {
			_animInfo[i]._field46_startX[id] = f.readUint16LE();
			debugStr += Common::String::format("%d ", _animInfo[i]._field46_startX[id]);
		}
		debugC(6, kDebugEngine, "%s", debugStr.c_str());
		debugC(6, kDebugEngine, "---------");
	}
}

void EfhEngine::findMapFile(int16 mapId) {
	debugC(7, kDebugEngine, "findMapFile %d", mapId);

	if (!_introDoneFl)
		return;

	Common::String fileName = Common::String::format("map.%d", mapId);
	Common::File f;
	// The original was checking for the file and eventually asking to change floppies
	if (!f.open(fileName))
		error("File not found: %s", fileName.c_str());

	f.close();
}

void EfhEngine::rImageFile(Common::String filename, uint8 *targetBuffer, uint8 **subFilesArray, uint8 *packedBuffer) {
	debugC(1, kDebugUtils, "rImageFile %s", filename.c_str());
	readFileToBuffer(filename, packedBuffer);

#ifndef debug
	uncompressBuffer(packedBuffer, targetBuffer);
#else
	uint32 size = uncompressBuffer(packedBuffer, targetBuffer);
	// dump a decompressed image file
	Common::DumpFile dump;
	dump.open(filename + ".dump");
	dump.write(targetBuffer, size);
	dump.flush();
	dump.close();
	// End of dump
#endif

	// TODO: Refactoring: once uncompressed, the container contains for each image its width, its height, and raw data (4 Bpp)
	// => Write a class to handle that more properly
	uint8 *ptr = targetBuffer;
	uint16 counter = 0;
	while (READ_LE_INT16(ptr) != 0) {
		subFilesArray[counter] = ptr;
		++counter;
		int16 imageWidth = READ_LE_INT16(ptr);
		ptr += 2;
		int16 imageHeight = READ_LE_INT16(ptr);
		ptr += 2;
		ptr += (imageWidth * imageHeight);
	}
}

void EfhEngine::readImpFile(int16 id, bool techMapFl) {
	debugC(6, kDebugEngine, "readImpFile %d %s", id, techMapFl ? "True" : "False");

	Common::String fileName = Common::String::format("imp.%d", id);

	if (techMapFl)
		readFileToBuffer(fileName, _imp1);
	else
		readFileToBuffer(fileName, _imp2);

	decryptImpFile(techMapFl);
}

void EfhEngine::readItems() {
	debugC(7, kDebugEngine, "readItems");

	Common::String fileName = "items";
	Common::File f;
	if (!f.open(fileName))
		error("Unable to find file %s", fileName.c_str());

	for (int i = 0; i < 300; ++i) {
		for (uint idx = 0; idx < 15; ++idx)
			_items[i]._name[idx] = f.readByte();

		_items[i]._damage = f.readByte();
		_items[i]._defense = f.readByte();
		_items[i]._attacks = f.readByte();
		_items[i]._uses = f.readByte();
		_items[i].field_13 = f.readByte();
		_items[i]._range = f.readByte();
		_items[i]._attackType = f.readByte();
		_items[i].field_16 = f.readByte();
		_items[i].field17_attackTypeDefense = f.readByte();
		_items[i].field_18 = f.readByte();
		_items[i].field_19 = f.readByte();
		_items[i].field_1A = f.readByte();

		debugC(7, kDebugEngine, "%s\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x", _items[i]._name, _items[i]._damage, _items[i]._defense, _items[i]._attacks, _items[i]._uses, _items[i].field_13, _items[i]._range, _items[i]._attackType, _items[i].field_16, _items[i].field17_attackTypeDefense, _items[i].field_18, _items[i].field_19, _items[i].field_1A);
	}
}

void EfhEngine::loadNewPortrait() {
	debugC(7, kDebugEngine, "loadNewPortrait");

	static int16 const unkConstRelatedToAnimImageSetId[19] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2};
	_unkRelatedToAnimImageSetId = unkConstRelatedToAnimImageSetId[_techId];

	if (_currentAnimImageSetId == 200 + _unkRelatedToAnimImageSetId)
		return;

	findMapFile(_techId);
	_currentAnimImageSetId = 200 + _unkRelatedToAnimImageSetId;
	int imageSetId = _unkRelatedToAnimImageSetId + 13;
	loadImageSet(imageSetId, _portraitBuf, _portraitSubFilesArray, _hiResImageBuf);
}

void EfhEngine::loadAnimImageSet() {
	debug("loadAnimImageSet");

	if (_currentAnimImageSetId == _animImageSetId || _animImageSetId == 0xFF)
		return;

	findMapFile(_techId);

	_unkAnimRelatedIndex = 0;
	_currentAnimImageSetId = _animImageSetId;

	int16 animSetId = _animImageSetId + 17;
	loadImageSet(animSetId, _portraitBuf, _portraitSubFilesArray, _hiResImageBuf);
}

void EfhEngine::loadHistory() {
	debug("loadHistory");

	Common::String fileName = "history";
	readFileToBuffer(fileName, _history);
}

void EfhEngine::loadTechMapImp(int16 fileId) {
	debug("loadTechMapImp %d", fileId);

	if (fileId == 0xFF)
		return;

	_techId = fileId;
	findMapFile(_techId);

	// The original was loading the specific tech.%d and map.%d files.
	// This is gone in our implementation as we pre-load all the files to save them inside the savegames

	// This is not present in the original.
	// The purpose is to properly load the misc map data in arrays in order to use them without being a pain afterwards
	loadMapArrays(_techId);

	loadImageSetToTileBank(1, _mapBitmapRefArr[_techId][0] + 1);
	loadImageSetToTileBank(2, _mapBitmapRefArr[_techId][1] + 1);

	initMapMonsters();
	readImpFile(_techId, true);
	displayAnimFrames(0xFE, false);
}

void EfhEngine::loadPlacesFile(uint16 fullPlaceId, bool forceReloadFl) {
	debug("loadPlacesFile %d %s", fullPlaceId, forceReloadFl ? "True" : "False");

	if (fullPlaceId == 0xFF)
		return;

	findMapFile(_techId);
	_fullPlaceId = fullPlaceId;
	uint16 minPlace = _lastMainPlaceId * 20;
	uint16 maxPlace = minPlace + 19;

	if (_fullPlaceId < minPlace || _fullPlaceId > maxPlace || forceReloadFl) {
		_lastMainPlaceId = _fullPlaceId / 20;
		Common::String fileName = Common::String::format("places.%d", _lastMainPlaceId);
		readFileToBuffer(fileName, _hiResImageBuf);
		uncompressBuffer(_hiResImageBuf, _places);
	}
	copyCurrentPlaceToBuffer(_fullPlaceId % 20);
}

void EfhEngine::readTileFact() {
	debugC(7, kDebugEngine, "readTileFact");

	Common::String fileName = "tilefact";
	Common::File f;
	if (!f.open(fileName))
		error("Unable to find file %s", fileName.c_str());

	for (int i = 0; i < 432; ++i) {
		_tileFact[i]._field0 = f.readByte();
		_tileFact[i]._field1 = f.readByte();
	}
}

void EfhEngine::loadNPCS() {
	debugC(7, kDebugEngine, "loadNPCS");

	Common::String fileName("npcs");
	Common::File f;
	if (!f.open(fileName))
		error("Unable to find file %s", fileName.c_str());

	for (int i = 0; i < 99; ++i) {
		for (int idx = 0; idx < 11; ++idx)
			_npcBuf[i]._name[idx] = f.readByte();
		_npcBuf[i].fieldB_textId = f.readByte();
		_npcBuf[i].field_C = f.readByte();
		_npcBuf[i].field_D = f.readByte();
		_npcBuf[i].fieldE_textId = f.readByte();
		_npcBuf[i].field_F = f.readByte();
		_npcBuf[i].field_10 = f.readByte();
		_npcBuf[i].field_11 = f.readByte();
		_npcBuf[i].field12_textId = f.readUint16LE();
		_npcBuf[i].field14_textId = f.readUint16LE();
		_npcBuf[i]._xp = f.readUint32LE();
		for (int idx = 0; idx < 15; ++idx) {
			_npcBuf[i]._activeScore[idx] = f.readByte();
		}
		for (int idx = 0; idx < 11; ++idx) {
			_npcBuf[i]._passiveScore[idx] = f.readByte();
		}
		for (int idx = 0; idx < 11; ++idx) {
			_npcBuf[i]._infoScore[idx] = f.readByte();
		}
		_npcBuf[i].field_3F = f.readByte();
		_npcBuf[i].field_40 = f.readByte();
		for (int idx = 0; idx < 10; ++idx) {
			_npcBuf[i]._inventory[idx]._ref = f.readSint16LE();
			_npcBuf[i]._inventory[idx]._stat1 = f.readByte();
			_npcBuf[i]._inventory[idx]._stat2 = f.readByte();
		}
		_npcBuf[i]._possessivePronounSHL6 = f.readByte();
		_npcBuf[i]._speed = f.readByte();
		_npcBuf[i].field_6B = f.readByte();
		_npcBuf[i].field_6C = f.readByte();
		_npcBuf[i].field_6D = f.readByte();
		_npcBuf[i]._unkItemId = f.readByte();
		_npcBuf[i].field_6F = f.readByte();
		_npcBuf[i].field_70 = f.readByte();
		_npcBuf[i].field_71 = f.readByte();
		_npcBuf[i].field_72 = f.readByte();
		_npcBuf[i].field_73 = f.readByte();
		_npcBuf[i]._hitPoints = f.readSint16LE();
		_npcBuf[i]._maxHP = f.readSint16LE();
		_npcBuf[i].field_78 = f.readByte();
		_npcBuf[i].field_79 = f.readUint16LE();
		_npcBuf[i].field_7B = f.readUint16LE();
		_npcBuf[i].field_7D = f.readByte();
		_npcBuf[i].field_7E = f.readByte();
		_npcBuf[i].field_7F = f.readByte();
		_npcBuf[i].field_80 = f.readByte();
		_npcBuf[i].field_81 = f.readByte();
		_npcBuf[i].field_82 = f.readByte();
		_npcBuf[i].field_83 = f.readByte();
		_npcBuf[i].field_84 = f.readByte();
		_npcBuf[i].field_85 = f.readByte();
	}
}

/**
 * Pre-Loads MAP and TECH files.
 * This is required in order to implement a clean savegame feature
 */
void EfhEngine::preLoadMaps() {
	for (int i = 0; i < 19; ++i) {
		Common::String fileName = Common::String::format("tech.%d", i);
		readFileToBuffer(fileName, _hiResImageBuf);
		uncompressBuffer(_hiResImageBuf, _techDataArr[i]);

		fileName = Common::String::format("map.%d", i);
		readFileToBuffer(fileName, _hiResImageBuf);
		uncompressBuffer(_hiResImageBuf, _mapArr[i]);

		_mapBitmapRefArr[i] = &_mapArr[i][0];
	}
}

} // End of namespace Efh

