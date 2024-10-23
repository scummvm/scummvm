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

#ifdef ENABLE_EOB

#include "kyra/engine/eobcommon.h"
#include "kyra/resource/resource.h"
#include "kyra/script/script_eob.h"
#include "kyra/engine/timer.h"
#include "kyra/sound/sound.h"

#include "common/system.h"

namespace Kyra {

void EoBCoreEngine::loadLevel(int level, int sub) {
	_currentLevel = level;
	_currentSub = sub;

	if (!_loading)
		setHandItem(-1);

	disableSysTimer(2);
	uint32 end = _system->getMillis() + 500;

	resetWallData();

	readLevelFileData(level);

	Common::String gfxFile;
	// Work around for issue with corrupt (incomplete) monster property data
	// when loading a savegame saved in a sub level
	for (int i = 0; i <= sub; i++)
		gfxFile = initLevelData(i);

	const uint8 *data = _screen->getCPagePtr(5);
	const uint8 *pos = data + READ_LE_UINT16(data);
	uint16 len = READ_LE_UINT16(pos);
	uint16 len2 = len;
	pos += 2;

	if (_flags.gameID == GI_EOB2) {
		if (*pos++ == 0xEC)
			pos = loadActiveMonsterData(pos, level);
		else if (!(_hasTempDataFlags & (1 << (level - 1))))
			memset(_monsters, 0, 30 * sizeof(EoBMonsterInPlay));

		len2 = len - (pos - data);
		_inf->loadData(pos, len2);
	} else {
		_inf->loadData(data, READ_LE_UINT16(data));
	}

	_screen->setCurPage(2);
	addLevelItems();

	if (_flags.gameID == GI_EOB2) {
		pos = data + len;
		len2 = READ_LE_UINT16(pos);
		pos += 2;
	}

	for (uint16 i = 0; i < len2; i++) {
		LevelBlockProperty *p = &_levelBlockProperties[READ_LE_UINT16(pos)];
		pos += 2;
		if (_flags.gameID == GI_EOB2) {
			p->flags |= READ_LE_UINT16(pos);
			pos += 2;
		} else {
			p->flags |= *pos++;
		}
		p->assignedObjects = READ_LE_UINT16(pos);
		pos += 2;
	}

	// WORKAROUND for bug #6211 (EOB1: Door Buttons Don't Work)
	if (_flags.gameID == GI_EOB1 && level == 7 && _levelBlockProperties[0x035C].assignedObjects == 0x0E89)
		_levelBlockProperties[0x035C].assignedObjects = 0x0E8D;

	loadVcnData(gfxFile.c_str(), _cgaLevelMappingIndex ? _cgaMappingLevel[_cgaLevelMappingIndex[level - 1]] : 0);
	gui_setupPlayFieldHelperPages();
	if (_flags.platform == Common::kPlatformAmiga && _flags.gameID == GI_EOB1)
		_screen->getPalette(0).copy(_screen->getPalette(1), 1, 5, 1);

	delayUntil(end);
	snd_stopSound();

	enableSysTimer(2);
	_sceneDrawPage1 = 2;
	_sceneDrawPage2 = 1;
	_screen->setCurPage(0);
	setHandItem(_itemInHand);

	snd_playLevelScore();
}

void EoBCoreEngine::readLevelFileData(int level) {
	Common::String file;
	Common::SeekableReadStream *s = 0;
	static const char *const suffix[] = { "DRO", "INF", "ELO", "JOT", 0 };

	for (const char *const *sf = suffix; *sf && !s; sf++) {
		file = Common::String::format("LEVEL%d.%s", level, *sf);
		s = _res->createReadStream(Common::Path(file));
	}

	if (!s)
		error("Failed to load level file LEVEL%d.INF/DRO/ELO/JOT", level);

	if (_flags.gameID == GI_EOB2 && _flags.lang == Common::Language::ZH_TWN) {
		_screen->loadChineseEOB2LZBitmap(s, 5, 15000);
		return;
	}

	if (s->readUint16LE() + 2 == s->size()) {
		// check for valid compression type
		if (s->readUint16LE() <= 4) {
			delete s;
			s = 0;
			_screen->loadBitmap(file.c_str(), 5, 5, 0, true);
		}
	}

	if (s) {
		s->seek(0);
		if (s->readSint32BE() + 12 == s->size()) {
			_screen->loadSpecialAmigaCPS(file.c_str(), 5, false);
		} else {
			s->seek(0);
			_screen->loadFileDataToPage(s, 5, 15000);
		}
		delete s;
	}
}

Common::String EoBCoreEngine::initLevelData(int sub) {
	const uint8 *data = _screen->getCPagePtr(5) + 2;
	const uint8 *pos = data;

	int slen = (_flags.gameID == GI_EOB1) ? 12 : 13;

	for (int i = 0; i < sub; i++)
		pos = data + READ_LE_UINT16(pos);

	pos += 2;
	if (*pos++ == 0xEC || _flags.gameID == GI_EOB1) {
		if (_flags.gameID == GI_EOB1)
			pos -= 3;

		loadBlockProperties((const char *)pos);
		pos += slen;

		Common::SeekableReadStreamEndian *s = getVmpData((const char*)pos);
		assert(s);
		uint16 size = (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformSegaCD) ? 2916 : s->readUint16();
		delete[] _vmpPtr;
		_vmpPtr = new uint16[size];
		if (_flags.gameID == GI_EOB1) {
			for (int i = 0; i < size; i++)
				_vmpPtr[i] = s->readUint16();
		} else {
			for (int i = 0; i < size; i++)
				_vmpPtr[i] = s->readUint16LE();
		}
		delete s;

		const char *paletteFilePattern = (_flags.gameID == GI_EOB2 && _configRenderMode == Common::kRenderEGA) ? "%s.EGA" : "%s.PAL";

		Common::String tmpStr = Common::String::format(paletteFilePattern, (const char *)pos);
		_curGfxFile = (const char *)pos;
		pos += slen;

		if (*pos++ != 0xFF && _flags.gameID == GI_EOB2) {
			tmpStr = Common::String::format(paletteFilePattern, (const char *)pos);
			pos += 13;
		}

		if (_flags.gameID == GI_EOB1) {
			pos += 11;
			_screen->setShapeFadingLevel(0);
			_screen->enableShapeBackgroundFading(false);
		}

		if (_flags.gameID == GI_EOB2 || (_flags.platform != Common::kPlatformAmiga && _configRenderMode != Common::kRenderCGA && _configRenderMode != Common::kRenderEGA))
			_screen->loadPalette(tmpStr.c_str(), _screen->getPalette(_flags.platform == Common::kPlatformAmiga ? 6 : 0));

		setLevelPalettes(_currentLevel);

		if (_flags.platform == Common::kPlatformFMTowns) {
			uint16 *src = (uint16*)_screen->getPalette(0).getData();
			_screen->createFadeTable16bit(src, (uint16*)_greenFadingTable, 4, 75);
			_screen->createFadeTable16bit(src, (uint16*)_blackFadingTable, 12, 200);
			_screen->createFadeTable16bit(src, (uint16*)_blueFadingTable, 10, 85);
			_screen->createFadeTable16bit(src, (uint16*)_lightBlueFadingTable, 11, 125);
			_screen->createFadeTable16bit(src, (uint16*)_greyFadingTable, 0, 85);
			_screen->setScreenPalette(_screen->getPalette(0));
		} else if (_configRenderMode != Common::kRenderCGA && _flags.platform != Common::kPlatformAmiga && _flags.platform != Common::kPlatformSegaCD && !(_flags.gameID == GI_EOB1 && _flags.platform == Common::kPlatformPC98)) {
			Palette backupPal(256);
			backupPal.copy(_screen->getPalette(0), 224, 32, 224);
			_screen->getPalette(0).fill(224, 32, 0x3F);
			uint8 *src = _screen->getPalette(0).getData();

			_screen->createFadeTable(src, _greenFadingTable, 4, 75);
			_screen->createFadeTable(src, _blackFadingTable, 12, 200);
			_screen->createFadeTable(src, _blueFadingTable, 10, 85);
			_screen->createFadeTable(src, _lightBlueFadingTable, 11, 125);

			_screen->getPalette(0).copy(backupPal, 224, 32, 224);
			_screen->createFadeTable(src, _greyFadingTable, 12, 85);
			_screen->setFadeTable(_greyFadingTable);
			if (_flags.gameID == GI_EOB2 && _configRenderMode == Common::kRenderEGA)
				_screen->setScreenPalette(_screen->getPalette(0));
		}
	}

	if (_flags.platform == Common::kPlatformAmiga) {
		delay(3 * _tickLength);
		snd_loadAmigaSounds(_currentLevel, sub);
		if (_flags.gameID == GI_EOB2)
			pos += 13;
	} else if (_flags.gameID == GI_EOB2) {
		delay(3 * _tickLength);
		_sound->loadSoundFile((const char *)pos);
		pos += 13;
	}

	releaseDoorShapes();
	releaseMonsterShapes(0, 36);
	releaseDecorations();

	if (_flags.gameID == GI_EOB1) {
		loadDoorShapes(pos[0], pos[1], pos[2], pos[3]);
		pos += 4;
		_scriptTimersMode = *pos++;
		_scriptTimers[0].ticks = READ_LE_UINT16(pos);
		_scriptTimers[0].func = 0;
		_scriptTimers[0].next = _system->getMillis() + _scriptTimers[0].ticks * _tickLength;
		pos += 2;
	} else {
		for (int i = 0; i < 2; i++) {
			int a = (*pos == 0xEC) ? 0 : ((*pos == 0xEA) ? 1 : -1);
			pos++;
			if (a == -1)
				continue;
			toggleWallState(pos[13], a);
			_doorType[pos[13]] = pos[14];
			_noDoorSwitch[pos[13]] = pos[15];
			pos = loadDoorShapes((const char *)pos, pos[13], pos + 16);
		}
	}

	_stepsUntilScriptCall = READ_LE_UINT16(pos);
	pos += 2;
	_stepCounter = 0;

	for (int i = 0; i < 2; i++) {
		if (_flags.gameID == GI_EOB1) {
			if (*pos != 0xFF)
				loadMonsterShapes((const char *)(pos + 1), i * 18, false, *pos * 18);
			pos += 13;
		} else {
			if (*pos++ != 0xEC)
				continue;
			loadMonsterShapes((const char *)(pos + 2), pos[1] * 18, pos[15] ? true : false, *pos * (_flags.platform == Common::kPlatformAmiga ? 6 : 18));
			pos += 16;
		}
	}

	if (_flags.gameID == GI_EOB1)
		pos = loadActiveMonsterData(pos, _currentLevel) - 1;
	else
		pos = loadMonsterProperties(pos);

	if (*pos++ == 0xEC || _flags.gameID == GI_EOB1) {
		int num = READ_LE_UINT16(pos);
		pos += 2;

		for (int i = 0; i < num; i++) {
			if (*pos++ == 0xEC) {
				loadDecorations((const char *)pos, (const char *)(pos + slen));
				pos += (slen << 1);
			} else {
				assignWallsAndDecorations(pos[0], pos[1], (int8)pos[2], pos[3], pos[4]);
				pos += 5;
			}
		}
	}

	if (_flags.gameID == GI_EOB2)
		initScriptTimers(pos);

	return _curGfxFile;
}

void EoBCoreEngine::addLevelItems() {
	for (int i = 0; i < 1024; i++)
		_levelBlockProperties[i].drawObjects = 0;

	for (int i = 0; i < 600; i++) {
		if (_items[i].level != _currentLevel || _items[i].block <= 0)
			continue;
		setItemPosition((Item *)&_levelBlockProperties[_items[i].block & 0x3FF].drawObjects, _items[i].block, i, _items[i].pos);
	}
}

void EoBCoreEngine::loadVcnData(const char *file, const uint8 *cgaMapping) {
	uint32 vcnSize = 0;
	Common::String fn = Common::String::format(_vcnFilePattern.c_str(), _lastBlockDataFile.c_str());
	_screen->loadBitmap(fn.c_str(), 3, 3, 0, true);

	const uint8 *pos = _screen->getCPagePtr(3);
	vcnSize = READ_LE_UINT16(pos) * (_vcnSrcBitsPerPixel << 3);
	pos += 2;

	const uint8 *colMap = pos;
	pos += 32;

	_vcnBlocks = new uint8[vcnSize];

	if (_configRenderMode == Common::kRenderCGA) {
		uint8 *tmp = _screen->encodeShape(0, 0, 1, 8, false, cgaMapping);
		delete[] tmp;

		delete[] _vcnTransitionMask;
		_vcnTransitionMask = new uint8[vcnSize];
		uint8 tblSwitch = 1;
		uint8 *dst = _vcnBlocks;
		uint8 *dst2 = _vcnTransitionMask;

		while (dst < _vcnBlocks + vcnSize) {
			const uint16 *table = _screen->getCGADitheringTable((tblSwitch++) & 1);
			for (int ii = 0; ii < 2; ii++) {
				*dst++ = (table[pos[0]] & 0x000F) | ((table[pos[0]] & 0x0F00) >> 4);
				*dst++ = (table[pos[1]] & 0x000F) | ((table[pos[1]] & 0x0F00) >> 4);
				*dst2++ = ((pos[0] & 0xF0 ? 0x30 : 0) | (pos[0] & 0x0F ? 0x03 : 0)) ^ 0x33;
				*dst2++ = ((pos[1] & 0xF0 ? 0x30 : 0) | (pos[1] & 0x0F ? 0x03 : 0)) ^ 0x33;
				pos += 2;
			}
		}
	} else {
		if (_flags.platform != Common::kPlatformAmiga && !(_flags.gameID == GI_EOB1 && _configRenderMode == Common::kRenderEGA))
			memcpy(_vcnColTable, colMap, 32);

		memcpy(_vcnBlocks, pos, vcnSize);
	}
}

Common::SeekableReadStreamEndian *EoBCoreEngine::getVmpData(const char *file) {
	return _res->createEndianAwareReadStream(Common::Path(Common::String::format(_vmpFilePattern.c_str(), file)));
}

void EoBCoreEngine::loadBlockProperties(const char *mazFile) {
	memset(_levelBlockProperties, 0, 1024 * sizeof(LevelBlockProperty));
	const uint8 *p = getBlockFileData(mazFile) + 6;

	if (_hasTempDataFlags & (1 << (_currentLevel - 1))) {
		restoreBlockTempData(_currentLevel);
		return;
	}

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			_levelBlockProperties[i].walls[ii] = *p++;
	}
}

const uint8 *EoBCoreEngine::getBlockFileData(int) {
	if (_flags.gameID == GI_EOB2 && _flags.platform == Common::kPlatformPC98) {
		_screen->loadBitmap(_curBlockFile.c_str(), 15, 15, 0);
	} else {
		Common::SeekableReadStream *s = _res->createReadStream(Common::Path(_curBlockFile));
		_screen->loadFileDataToPage(s, 15, s->size());
		delete s;
	}
	return _screen->getCPagePtr(15);
}

Common::String EoBCoreEngine::getBlockFileName(int levelIndex, int sub) {
	readLevelFileData(levelIndex);
	const uint8 *data = _screen->getCPagePtr(5) + 2;
	const uint8 *pos = data;

	for (int i = 0; i < sub; i++)
		pos = data + READ_LE_UINT16(pos);

	pos += 2;

	if (*pos++ == 0xEC || _flags.gameID == GI_EOB1) {
		if (_flags.gameID == GI_EOB1)
			pos -= 3;

		return Common::String((const char *)pos);
	}

	return Common::String();
}

const uint8 *EoBCoreEngine::getBlockFileData(const char *mazFile) {
	_curBlockFile = mazFile;
	return getBlockFileData(_currentLevel);
}


Common::SeekableReadStreamEndian *EoBCoreEngine::getDecDefinitions(const char *decFile) {
	return _res->createEndianAwareReadStream(decFile, Resource::kForceLE);
}

void EoBCoreEngine::loadDecShapesToPage3(const char *shpFile) {
	_screen->loadShapeSetBitmap(shpFile, 5, 3);
}

void EoBCoreEngine::loadDecorations(const char *cpsFile, const char *decFile) {
	loadDecShapesToPage3(cpsFile);
	Common::SeekableReadStreamEndian *s = getDecDefinitions(decFile);

	_levelDecorationDataSize = s->readUint16();
	delete[] _levelDecorationData;
	_levelDecorationData = new LevelDecorationProperty[_levelDecorationDataSize]();

	for (int i = 0; i < _levelDecorationDataSize; i++) {
		LevelDecorationProperty *l = &_levelDecorationData[i];
		for (int ii = 0; ii < 10; ii++) {
			l->shapeIndex[ii] = s->readByte();
			if (l->shapeIndex[ii] == 0xFF)
				l->shapeIndex[ii] = 0xFFFF;
		}
		l->next = s->readByte();
		l->flags = s->readByte();
		for (int ii = 0; ii < 10; ii++)
			l->shapeX[ii] = s->readSint16();
		for (int ii = 0; ii < 10; ii++)
			l->shapeY[ii] = s->readSint16();
	}

	int len = s->readUint16();
	delete[] _levelDecorationRects;
	_levelDecorationRects = new EoBRect8[len];
	for (int i = 0; i < len; i++) {
		EoBRect8 *l = &_levelDecorationRects[i];
		l->x = s->readUint16();
		l->y = s->readUint16();
		l->w = s->readUint16();
		l->h = s->readUint16();
	}

	delete s;
}

void EoBCoreEngine::assignWallsAndDecorations(int wallIndex, int vmpIndex, int decIndex, int specialType, int flags) {
	_wllVmpMap[wallIndex] = vmpIndex;

	if (wallIndex == 46) {
		// This is not part of the original code. The original will discard the true seeing spell effect when entering a new level.
		for (int i = 0; i < 6; i++) {
			for (int ii = 0; ii < 10; ii++) {
				if (_characters[i].events[ii] == -57 && _characters[i].timers[ii])
					spellCallback_start_trueSeeing();
			}
		}
	}

	_wllShapeMap[wallIndex] = _mappedDecorationsCount + 1;
	_specialWallTypes[wallIndex] = specialType;
	_wllWallFlags[wallIndex] = flags ^ 4;

	if (decIndex == -1) {
		_wllShapeMap[wallIndex] = 0;
		return;
	}

	do {
		assert(decIndex < _levelDecorationDataSize);
		memcpy(&_levelDecorationProperties[_mappedDecorationsCount], &_levelDecorationData[decIndex], sizeof(LevelDecorationProperty));

		for (int i = 0; i < 10; i++) {
			uint16 t = _levelDecorationProperties[_mappedDecorationsCount].shapeIndex[i];
			if (t == 0xFFFF)
				continue;

			if (_levelDecorationShapes[t])
				continue;

			EoBRect8 *r = &_levelDecorationRects[t];
			if (r->w == 0 || r->h == 0)
				error("Error trying to make decoration %d (x: %d, y: %d, w: %d, h: %d)", decIndex, r->x, r->y, r->w, r->h);

			if (_flags.platform == Common::kPlatformSegaCD) {
				_levelDecorationShapes[t] = _screen->sega_convertShape(_dcrShpDataPos, r->w << 3, r->h, 0);
				_dcrShpDataPos += ((r->w << 2) * r->h);
			} else {
				_levelDecorationShapes[t] = _screen->encodeShape(r->x, r->y, r->w, r->h, false, _cgaLevelMappingIndex ? _cgaMappingLevel[_cgaLevelMappingIndex[_currentLevel - 1]] : 0);
			}
		}

		decIndex = _levelDecorationProperties[_mappedDecorationsCount++].next;

		if (decIndex)
			_levelDecorationProperties[_mappedDecorationsCount - 1].next = _mappedDecorationsCount + 1;
		else
			decIndex = -1;

	} while (decIndex != -1);
}

void EoBCoreEngine::releaseDecorations() {
	if (_levelDecorationShapes) {
		for (int i = 0; i < 400; i++) {
			delete[] _levelDecorationShapes[i];
			_levelDecorationShapes[i] = 0;
		}
	}
	_mappedDecorationsCount = 0;
	_dcrShpDataPos = 0;
}

void EoBCoreEngine::releaseDoorShapes() {
	for (int i = 0; i < 6; i++) {
		delete[] _doorShapes[i];
		_doorShapes[i] = 0;
		delete[] _doorSwitches[i].shp;
		_doorSwitches[i].shp = 0;
	}
}

void EoBCoreEngine::resetWallData() {
	memset(_wllVmpMap, 0, 256);
	_wllVmpMap[1] = 1;
	_wllVmpMap[2] = 2;
	memset(&_wllVmpMap[3], 3, 20);
	_wllVmpMap[23] = 4;
	_wllVmpMap[24] = 5;
	memset(_wllShapeMap, 0, 256);
	memset(&_wllShapeMap[3], -1, 5);
	memset(&_wllShapeMap[13], -1, 5);
	memset(_wllWallFlags, 0, 256);
	memcpy(_wllWallFlags, _wllFlagPreset, _wllFlagPresetSize);
	memset(_specialWallTypes, 0, 256);
	memset(&_specialWallTypes[3], 1, 5);
	memset(&_specialWallTypes[13], 1, 5);
	_specialWallTypes[8] = _specialWallTypes[18] = 6;
}

void EoBCoreEngine::toggleWallState(int wall, int toggle) {
	wall = wall * 10 + 3;

	for (int i = 0; i < 9 ; i++) {
		if (i == 4)
			continue;

		if (toggle)
			_wllWallFlags[wall + i] |= 2;
		else
			_wllWallFlags[wall + i] &= ~2;
	}
}

void EoBCoreEngine::drawScene(int refresh) {
	generateBlockDrawingBuffer();
	drawVcnBlocks();
	drawSceneShapes();

	if (_sceneDrawPage2) {
		if (refresh)
			_screen->fillRect(0, 0, 175, 119, guiSettings()->colors.guiColorBlack);

		if (!_loading)
			_screen->setScreenPalette(_screen->getPalette(0));

		_sceneDrawPage2 = 0;
	}

	uint32 ct = _system->getMillis();
	if (_flashShapeTimer > ct) {
		int diff = _flashShapeTimer - ct;
		while ((diff > 0) && !shouldQuit()) {
			updateInput();
			updateAnimations();
			uint32 step = MIN<uint32>(diff, _tickLength / 5);
			_system->delayMillis(step);
			diff -= step;
		}
	}

	if (_sceneDefaultUpdate)
		delayUntil(_drawSceneTimer);

	if (refresh && !_partyResting)
		_screen->copyRegion(0, 0, 0, 0, 176, 120, 2, 0, Screen::CR_NO_P_CHECK);

	snd_updateEnvironmentalSfx(0);

	if (!_dialogueField && refresh && !_updateFlags)
		gui_drawCompass(false);

	if (refresh && !_partyResting && !_loading)
		_screen->updateScreen();

	if (_sceneDefaultUpdate) {
		_sceneDefaultUpdate = 0;
		_drawSceneTimer = _system->getMillis() + 4 * _tickLength;
	}

	_sceneUpdateRequired = false;
}

void EoBCoreEngine::drawSceneShapes(int start, int end, int drawFlags) {
	for (int i = start; i < end; i++) {
		uint8 t = _dscTileIndex[i];
		uint8 s = _visibleBlocks[t]->walls[_sceneDrawVarDown];

		_shpDmX1 = 0;
		_shpDmX2 = 0;

		setLevelShapesDim(t, _shpDmX1, _shpDmX2, _sceneShpDim);

		if (_shpDmX2 <= _shpDmX1)
			continue;

		if (drawFlags & 0x01)
			drawDecorations(t);

		if ((drawFlags & 0x02) && _visibleBlocks[t]->drawObjects)
			drawBlockItems(t);

		if (t < 15) {
			uint16 w = _wllWallFlags[s];

			if ((drawFlags & 0x04) && (w & 8))
				drawDoor(t);

			if ((drawFlags & 0x08) && (_visibleBlocks[t]->flags & 7)) {
				const ScreenDim *dm = _screen->getScreenDim(5);
				_screen->modifyScreenDim(5, dm->sx, _lvlShapeTop[t], dm->w, _lvlShapeBottom[t] - _lvlShapeTop[t]);
				drawMonsters(t);
				drawLevelModifyScreenDim(5, _lvlShapeLeftRight[(t << 1)], 0, _lvlShapeLeftRight[(t << 1) + 1], 15);
			}

			if ((drawFlags & 0x10) && _flags.gameID == GI_EOB2 && s == 74)
				drawWallOfForce(t);
		}

		if (drawFlags & 0x20)
			drawFlyingObjects(t);

		if ((drawFlags & 0x40) && s == _teleporterWallId)
			drawTeleporter(t);
	}
}

void EoBCoreEngine::drawDecorations(int index) {
	for (int i = 1; i >= 0; i--) {
		int s = index * 2 + i;
		if (_dscWallMapping[s]) {
			int16 d = *_dscWallMapping[s];
			int8 l = _wllShapeMap[_visibleBlocks[index]->walls[d]];

			const uint8 *shapeData = 0;

			int x = 0;

			while (l > 0) {
				l--;
				int8 ix = _dscShapeIndex[s];
				uint8 shpIx = ABS(ix) - 1;
				uint8 flg = _levelDecorationProperties[l].flags;

				if ((i == 0) && (flg & 1 || ((flg & 2) && _wllProcessFlag)))
					ix = -ix;

				if (_levelDecorationProperties[l].shapeIndex[shpIx] == 0xFFFF) {
					l = _levelDecorationProperties[l].next;
					continue;
				}

				shapeData = _levelDecorationShapes[_levelDecorationProperties[l].shapeIndex[shpIx]];
				if (shapeData) {
					x = 0;
					if (i == 0) {
						if (flg & 4)
							x += _dscShapeCoords[(index * 5 + 4) << 1];
						else
							x += _dscShapeX[index];
					}

					if (ix < 0) {
						x += (176 - _levelDecorationProperties[l].shapeX[shpIx] - (shapeData[2] << 3));
						drawBlockObject(1, 2, shapeData, x, _levelDecorationProperties[l].shapeY[shpIx], _sceneShpDim);
					} else {
						x += _levelDecorationProperties[l].shapeX[shpIx];
						drawBlockObject(0, 2, shapeData, x, _levelDecorationProperties[l].shapeY[shpIx], _sceneShpDim);

					}
				}
				l = _levelDecorationProperties[l].next;
				continue;
			}
		}
	}
}

int EoBCoreEngine::calcNewBlockPositionAndTestPassability(uint16 curBlock, uint16 direction) {
	uint16 b = calcNewBlockPosition(curBlock, direction);
	int w = _levelBlockProperties[b].walls[direction ^ 2];
	int f = _wllWallFlags[w];

	assert((_flags.gameID == GI_EOB1 && w < 70) || (_flags.gameID == GI_EOB2 && w < 80));

	if (_flags.gameID == GI_EOB2 && w == 74 && _currentBlock == curBlock) {
		for (int i = 0; i < 5; i++) {
			if (_wallsOfForce[i].block == b) {
				destroyWallOfForce(i);
				f = _wllWallFlags[0];
			}
		}
	}

	if (!(f & 1) || _levelBlockProperties[b].flags & 7)
		return -1;

	return b;
}

void EoBCoreEngine::notifyBlockNotPassable() {
	_txt->printMessage(_warningStrings[0]);
	snd_playSoundEffect(_flags.gameID == GI_EOB1 && _flags.platform == Common::kPlatformPC98 ? 45 : 29);
	removeInputTop();
}

void EoBCoreEngine::increaseStepsCounter() {
	if (_totalSteps < 0xFFFFFFFF)
		_totalSteps++;
}

void EoBCoreEngine::moveParty(uint16 block) {
	updateAllMonsterDests();
	uint16 old = _currentBlock;
	_currentBlock = block;

	runLevelScript(old, 2);

	if (++_moveCounter > 3) {
		_txt->printMessage("\r");
		_moveCounter = 0;
	}

	runLevelScript(block, 1);

	if (_flags.gameID == GI_EOB2 && _levelBlockProperties[block].walls[0] == 26)
		memset(_levelBlockProperties[block].walls, 0, 4);

	updateAllMonsterDests();
	_stepCounter++;
	//_keybControlUnk = -1;
	_sceneUpdateRequired = true;

	checkFlyingObjects();
}

int EoBCoreEngine::clickedDoorSwitch(uint16 block, uint16 direction) {
	uint8 v = _visibleBlocks[13]->walls[_sceneDrawVarDown];
	SpriteDecoration *d = &_doorSwitches[((v > 12 && v < 23) || v == 31) ? 3 : 0];
	int x1 = d->x + _dscShapeCoords[138] - 4;
	int y1 = d->y - 4;

	if (_flags.gameID == GI_EOB1 && _currentLevel >= 4 && _currentLevel <= 6) {
		if (v >= 30)
			x1 += 4;
		else
			x1 += ((v - _dscDoorXE[v]) * 9);
	}

	if (!posWithinRect(_mouseX, _mouseY, x1, y1, x1 + (d->shp[2] << 3) + 8, y1 + d->shp[1] + 8) && (_clickedSpecialFlag == 0x40))
		return clickedDoorNoPry(block, direction);

	processDoorSwitch(block, 0);
	snd_playSoundEffect(6);

	return 1;
}

int EoBCoreEngine::clickedNiche(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	if (_itemInHand) {
		if (_dscItemShapeMap[_items[_itemInHand].icon] <= 14) {
			_txt->printMessage(_pryDoorStrings[5]);
		} else {
			setItemPosition((Item *)&_levelBlockProperties[block & 0x3FF].drawObjects, block, _itemInHand, 8);
			runLevelScript(block, 4);
			setHandItem(0);
			_sceneUpdateRequired = true;
		}
	} else {
		int d = getQueuedItem((Item *)&_levelBlockProperties[block].drawObjects, 8, -1);
		if (!d)
			return 1;
		runLevelScript(block, 8);
		setHandItem(d);
		_sceneUpdateRequired = true;
	}

	return 1;
}

int EoBCoreEngine::clickedDoorPry(uint16 block, uint16 direction) {
	if (!posWithinRect(_mouseX, _mouseY, 40, 16, 136, 88) && (_clickedSpecialFlag == 0x40))
		return 0;

	int d = -1;
	for (int i = 0; i < 6; i++)  {
		if (!testCharacter(i, 0x0D))
			continue;
		if (d >= 0) {
			int s1 = _characters[i].strengthCur + _characters[i].strengthExtCur;
			int s2 = _characters[d].strengthCur + _characters[d].strengthExtCur;
			if (s1 >= s2)
				d = i;
		} else {
			d = i;
		}
	}

	if (d == -1) {
		_txt->printMessage(_pryDoorStrings[_flags.gameID == GI_EOB2 ? 1 : 0]);
		return 1;
	}

	static const uint8 forceDoorChanceTable[] = { 1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10, 11, 12, 13 };
	int s = _characters[d].strengthCur > 18 ? 18 : _characters[d].strengthCur;

	if (rollDice(1, 20) < forceDoorChanceTable[s]) {
		_txt->printMessage(_pryDoorStrings[_flags.gameID == GI_EOB2 ? 2 : 1]);
		_levelBlockProperties[block].walls[direction] = _levelBlockProperties[block].walls[direction ^ 2] =
		            (_levelBlockProperties[block].walls[direction] == (_flags.gameID == GI_EOB2 ? 51 : 30)) ? 8 : 18;
		openDoor(block);
	} else {
		_txt->printMessage(_pryDoorStrings[3]);
	}

	return 1;
}

int EoBCoreEngine::clickedDoorNoPry(uint16 block, uint16 direction) {
	if (!posWithinRect(_mouseX, _mouseY, 40, 16, 136, 88) && (_clickedSpecialFlag == 0x40))
		return 0;

	if (!(_wllWallFlags[_levelBlockProperties[block].walls[direction]] & 0x20))
		return 0;
	_txt->printMessage(_pryDoorStrings[6]);
	return 1;
}

int EoBCoreEngine::specialWallAction(int block, int direction) {
	direction ^= 2;
	uint8 type = _specialWallTypes[_levelBlockProperties[block].walls[direction]];
	if (!type || !(_clickedSpecialFlag & (((_levelBlockProperties[block].flags & 0xF8) >> 3) | 0xE0)))
		return 0;

	int res = 0;
	switch (type) {
	case 1:
		res = clickedDoorSwitch(block, direction);
		break;

	case 2:
	case 8:
		res = clickedWallShape(block, direction);
		break;

	case 3:
		res = clickedLeverOn(block, direction);
		break;

	case 4:
		res = clickedLeverOff(block, direction);
		break;

	case 5:
		res =  clickedDoorPry(block, direction);
		break;

	case 6:
		res = clickedDoorNoPry(block, direction);
		break;

	case 7:
	case 9:
		res = clickedWallOnlyScript(block);
		break;

	case 10:
		res = clickedNiche(block, direction);
		break;

	default:
		break;
	}

	_clickedSpecialFlag = 0;
	_sceneUpdateRequired = true;

	return res;
}

void EoBCoreEngine::openDoor(int block) {
	openCloseDoor(block, 1);
}

void EoBCoreEngine::closeDoor(int block) {
	if (block == _currentBlock || _levelBlockProperties[block].flags & 7)
		return;
	openCloseDoor(block, -1);
}

} // End of namespace Kyra

#endif // ENABLE_EOB
