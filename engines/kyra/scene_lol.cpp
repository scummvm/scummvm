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

#ifdef ENABLE_LOL

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/resource.h"
#include "kyra/timer.h"
#include "kyra/sound.h"

#include "common/endian.h"

namespace Kyra {

void LoLEngine::loadLevel(int index) {
	_flagsTable[73] |= 0x08;
	setMouseCursorToIcon(0x85);
	_nextScriptFunc = 0;

	snd_stopMusic();

	updatePortraits();

	for (int i = 0; i < 400; i++) {
		delete[] _levelShapes[i];
		_levelShapes[i] = 0;
	}
	_emc->unload(&_scriptData);

	resetItems(1);
	disableMonsters();
	resetBlockProperties();

	releaseMonsterShapes(0);
	releaseMonsterShapes(1);

	for (int i = 0x50; i < 0x53; i++)
		_timer->disable(i);

	_currentLevel = index;
	_updateFlags = 0;

	setDefaultButtonState();

	loadTalkFile(index);

	loadLevelWallData(index, true);
	_loadLevelFlag = 1;

	char filename[13];
	snprintf(filename, sizeof(filename), "LEVEL%d.INI", index);

	int f = _hasTempDataFlags & (1 << (index - 1));

	runInitScript(filename, f ? 0 : 1);

	if (f)
		restoreBlockTempData(index);

	snprintf(filename, sizeof(filename), "LEVEL%d.INF", index);
	runInfScript(filename);

	addLevelItems();
	deleteMonstersFromBlock(_currentBlock);

	if (!_flags.use16ColorMode)
		_screen->generateGrayOverlay(_screen->getPalette(0), _screen->_grayOverlay, 32, 16, 0, 0, 128, true);

	_sceneDefaultUpdate = 0;
	if (_screen->_fadeFlag == 3)
		_screen->fadeToBlack(10);

	gui_drawPlayField();

	setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
	setMouseCursorToItemInHand();

	if (_flags.use16ColorMode)
		_screen->fadeToPalette1(10);	

	snd_playTrack(_curMusicTheme);
}

void LoLEngine::addLevelItems() {
	for (int i = 0; i < 400; i++) {
		if (_itemsInPlay[i].level != _currentLevel)
			continue;

		assignBlockObject(&_levelBlockProperties[_itemsInPlay[i].block], i);

		_levelBlockProperties[_itemsInPlay[i].block].direction = 5;
		_itemsInPlay[i].nextDrawObject = 0;
	}
}

void LoLEngine::assignBlockObject(LevelBlockProperty *l, uint16 item) {
	uint16 *index = &l->assignedObjects;
	ItemInPlay *tmp = 0;

	while (*index & 0x8000) {
		tmp = findObject(*index);
		index = &tmp->nextAssignedObject;
	}

	tmp = findObject(item);
	tmp->level = -1;

	uint16 ix = *index;

	if (ix == item)
		return;

	*index = item;
	index = &tmp->nextAssignedObject;

	while (*index) {
		tmp = findObject(*index);
		index = &tmp->nextAssignedObject;
	}

	*index = ix;
}

void LoLEngine::loadLevelWallData(int index, bool mapShapes) {
	char filename[13];
	snprintf(filename, sizeof(filename), "LEVEL%d.WLL", index);

	uint32 size;
	uint8 *file = _res->fileData(filename, &size);

	uint16 c = READ_LE_UINT16(file);
	loadLevelShpDat(_levelShpList[c], _levelDatList[c], false);

	uint8 *d = file + 2;
	size = (size - 2) / 12;
	for (uint32 i = 0; i < size; i++) {
		c = READ_LE_UINT16(d);
		d += 2;
		_wllVmpMap[c] = *d;
		d += 2;

		if (mapShapes) {
			int16 sh = (int16) READ_LE_UINT16(d);
			if (sh > 0)
				_wllShapeMap[c] = assignLevelShapes(sh);
			else
				_wllShapeMap[c] = *d;
		}
		d += 2;
		_wllBuffer3[c] = *d;
		d += 2;
		_wllWallFlags[c] = *d;
		d += 2;
		_wllBuffer4[c] = *d;
		d += 2;
	}

	delete[] file;

	delete _lvlShpFileHandle;
	_lvlShpFileHandle = 0;
}

int LoLEngine::assignLevelShapes(int index) {
	uint16 *p1 = (uint16 *)_tempBuffer5120;
	uint16 *p2 = (uint16 *)(_tempBuffer5120 + 4000);

	uint16 r = p2[index];
	if (r)
		return r;

	uint16 o = _lvlBlockIndex++;

	memcpy(&_levelShapeProperties[o], &_levelFileData[index], sizeof(LevelShapeProperty));

	for (int i = 0; i < 10; i++) {
		uint16 t = _levelShapeProperties[o].shapeIndex[i];
		if (t == 0xffff)
			continue;

		uint16 pv = p1[t];
		if (pv) {
			_levelShapeProperties[o].shapeIndex[i] = pv;
		} else {
			_levelShapes[_lvlShapeIndex] = getLevelShapes(t);
			p1[t] = _lvlShapeIndex;
			_levelShapeProperties[o].shapeIndex[i] = _lvlShapeIndex++;
		}
	}

	p2[index] = o;
	if (_levelShapeProperties[o].next)
		_levelShapeProperties[o].next = assignLevelShapes(_levelShapeProperties[o].next);

	return o;
}

uint8 *LoLEngine::getLevelShapes(int shapeIndex) {
	if (_lvlShpNum <= shapeIndex)
		return 0;

	_lvlShpFileHandle->seek(shapeIndex * 4 + 2, SEEK_SET);
	uint32 offs = _lvlShpFileHandle->readUint32LE() + 2;
	_lvlShpFileHandle->seek(offs, SEEK_SET);

	uint8 tmp[16];
	_lvlShpFileHandle->read(tmp, 16);
	uint16 size = _screen->getShapeSize(tmp);

	_lvlShpFileHandle->seek(offs, SEEK_SET);
	uint8 *res = new uint8[size];
	_lvlShpFileHandle->read(res, size);

	return res;
}

void LoLEngine::restoreBlockTempData(int index) {
	memset(_tempBuffer5120, 0, 5120);
	int l = index - 1;

	memcpy(_monsters, _lvlTempData[l]->monsters, sizeof(MonsterInPlay) * 30);
	memcpy(_flyingObjects, _lvlTempData[l]->flyingObjects, sizeof(FlyingObject) * 8);

	char filename[13];
	snprintf(filename, sizeof(filename), "LEVEL%d.CMZ", index);

	_screen->loadBitmap(filename, 3, 3, 0);
	const uint8 *p = _screen->getCPagePtr(2);
	uint16 len = READ_LE_UINT16(p + 4);
	p += 6;

	memset(_levelBlockProperties, 0, 1024 * sizeof(LevelBlockProperty));

	uint8 *t = _lvlTempData[l]->wallsXorData;
	uint8 *t2 = _lvlTempData[l]->flags;

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			_levelBlockProperties[i].walls[ii] = p[i * len + ii] ^ *t++;
		_levelBlockProperties[i].flags = *t2++;
	}

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].block) {
			_monsters[i].block = 0;
			_monsters[i].properties = &_monsterProperties[_monsters[i].type];
			placeMonster(&_monsters[i], _monsters[i].x, _monsters[i].y);
		}
	}

	restoreTempDataAdjustMonsterStrength(l);
}

void LoLEngine::restoreTempDataAdjustMonsterStrength(int index) {
	if (_lvlTempData[index]->monsterDifficulty == _monsterDifficulty)
		return;

	uint16 d = (_monsterModifiers[_lvlTempData[index]->monsterDifficulty] << 8) / _monsterModifiers[_monsterDifficulty];

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].mode >= 14 || _monsters[i].block == 0 || _monsters[i].hitPoints <= 0)
			continue;

		_monsters[i].hitPoints = (d * _monsters[i].hitPoints) >> 8;
		if (_monsterDifficulty < _lvlTempData[index]->monsterDifficulty)
			_monsters[i].hitPoints++;
		if (_monsters[i].hitPoints == 0)
			_monsters[i].hitPoints = 1;
	}
}

void LoLEngine::loadBlockProperties(const char *cmzFile) {
	memset(_levelBlockProperties, 0, 1024 * sizeof(LevelBlockProperty));
	_screen->loadBitmap(cmzFile, 2, 2, 0);
	const uint8 *h = _screen->getCPagePtr(2);
	uint16 len = READ_LE_UINT16(&h[4]);
	const uint8 *p = h + 6;

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			_levelBlockProperties[i].walls[ii] = p[i * len + ii];

		_levelBlockProperties[i].direction = 5;

		if (_wllBuffer4[_levelBlockProperties[i].walls[0]] == 17) {
			_levelBlockProperties[i].flags &= 0xef;
			_levelBlockProperties[i].flags |= 0x20;
		}
	}
}

void LoLEngine::loadLevelShpDat(const char *shpFile, const char *datFile, bool flag) {
	memset(_tempBuffer5120, 0, 5120);

	_lvlShpFileHandle = _res->createReadStream(shpFile);
	_lvlShpNum = _lvlShpFileHandle->readUint16LE();

	Common::SeekableReadStream *s = _res->createReadStream(datFile);

	_levelFileDataSize = s->readUint16LE();
	delete[] _levelFileData;
	_levelFileData = new LevelShapeProperty[_levelFileDataSize];
	for (int i = 0; i < _levelFileDataSize; i++) {
		LevelShapeProperty * l = &_levelFileData[i];
		for (int ii = 0; ii < 10; ii++)
			l->shapeIndex[ii] = s->readUint16LE();
		for (int ii = 0; ii < 10; ii++)
			l->scaleFlag[ii] = s->readByte();
		for (int ii = 0; ii < 10; ii++)
			l->shapeX[ii] = s->readSint16LE();
		for (int ii = 0; ii < 10; ii++)
			l->shapeY[ii] = s->readSint16LE();
		l->next = s->readByte();
		l->flags = s->readByte();
	}

	delete s;

	if (!flag) {
		_lvlBlockIndex = 1;
		_lvlShapeIndex = 1;
	}
}

void LoLEngine::loadLevelGraphics(const char *file, int specialColor, int weight, int vcnLen, int vmpLen, const char *palFile) {
	if (file) {
		_lastSpecialColor = specialColor;
		_lastSpecialColorWeight = weight;
		strcpy(_lastBlockDataFile, file);
		if (palFile) {
			strcpy(_lastOverridePalFile, palFile);
			_lastOverridePalFilePtr = _lastOverridePalFile;
		} else {
			_lastOverridePalFilePtr = 0;
		}
	}

	if (_flags.use16ColorMode) {
		if (_lastSpecialColor == 1)
			_lastSpecialColor = 0x44;
		else if (_lastSpecialColor == 0x66)
			_lastSpecialColor = scumm_stricmp(file, "YVEL2") ? 0xcc : 0x44;
		else if (_lastSpecialColor == 0x6b)
			_lastSpecialColor = 0xcc;
		else
			_lastSpecialColor = 0x44;
	}

	char fname[13];
	const uint8 *v = 0;
	int tlen = 0;

	if (_flags.use16ColorMode) {
		snprintf(fname, sizeof(fname), "%s.VCF", _lastBlockDataFile);
		_screen->loadBitmap(fname, 3, 3, 0);
		v = _screen->getCPagePtr(2);
		tlen = READ_LE_UINT16(v) << 5;
		v += 2;

		delete[] _vcfBlocks;
		_vcfBlocks = new uint8[tlen];

		memcpy(_vcfBlocks, v, tlen);
	}

	snprintf(fname, sizeof(fname), "%s.VCN", _lastBlockDataFile);
	_screen->loadBitmap(fname, 3, 3, 0);
	v = _screen->getCPagePtr(2);
	tlen = READ_LE_UINT16(v);
	v += 2;

	if (vcnLen == -1)
		vcnLen = tlen << 5;

	delete[] _vcnBlocks;
	_vcnBlocks = new uint8[vcnLen];

	if (!_flags.use16ColorMode) {
		delete[] _vcnShift;
		_vcnShift = new uint8[tlen];

		memcpy(_vcnShift, v, tlen);
		v += tlen;

		memcpy(_vcnExpTable, v, 128);
		v += 128;

		if (_lastOverridePalFilePtr) {
			_res->loadFileToBuf(_lastOverridePalFilePtr, _screen->getPalette(0).getData(), 384);
		} else {
			_screen->getPalette(0).copy(v, 0, 128);
		}

		v += 384;
	}	

	if (_currentLevel == 11) {
		if (_flags.use16ColorMode) {
			_screen->loadPalette("LOLICE.NOL", _screen->getPalette(2));

		} else {
			_screen->loadPalette("SWAMPICE.COL", _screen->getPalette(2));
			_screen->getPalette(2).copy(_screen->getPalette(0), 128);
		}

		if (_flagsTable[52] & 0x04) {
			uint8 *pal0 = _screen->getPalette(0).getData();
			uint8 *pal2 = _screen->getPalette(2).getData();
			for (int i = 1; i < _screen->getPalette(0).getNumColors() * 3; i++)
				SWAP(pal0[i], pal2[i]);
		}
	}

	memcpy(_vcnBlocks, v, vcnLen);
	v += vcnLen;

	snprintf(fname, sizeof(fname), "%s.VMP", _lastBlockDataFile);
	_screen->loadBitmap(fname, 3, 3, 0);
	v = _screen->getCPagePtr(2);

	if (vmpLen == -1)
		vmpLen = READ_LE_UINT16(v);
	v += 2;

	delete[] _vmpPtr;
	_vmpPtr = new uint16[vmpLen];

	for (int i = 0; i < vmpLen; i++)
		_vmpPtr[i] = READ_LE_UINT16(&v[i << 1]);

	Palette tpal(256);
	if (_flags.use16ColorMode) {
		uint8 *dst = tpal.getData();
		_res->loadFileToBuf("LOL.NOL", dst, 48);
		for (int i = 1; i < 16; i++) {
			int s = ((i << 4) | i) * 3;
			SWAP(dst[s], dst[i * 3]);
			SWAP(dst[s + 1], dst[i * 3 + 1]);
			SWAP(dst[s + 2], dst[i * 3 + 2]);
		}
	} else {
		tpal.copy(_screen->getPalette(0));
	}

	for (int i = 0; i < 7; i++) {
		weight = 100 - (i * _lastSpecialColorWeight);
		weight = (weight > 0) ? (weight * 255) / 100 : 0;
		_screen->generateLevelOverlay(tpal, _screen->getLevelOverlay(i), _lastSpecialColor, weight);

		int l = _flags.use16ColorMode ? 256 : 128;
		uint8 *levelOverlay = _screen->getLevelOverlay(i);
		for (int ii = 0; ii < l; ii++) {
			if (levelOverlay[ii] == 255)
				levelOverlay[ii] = 0;
		}

		for (int ii = l; ii < 256; ii++)
			levelOverlay[ii] = ii & 0xff;
	}

	uint8 *levelOverlay = _screen->getLevelOverlay(7);
	for (int i = 0; i < 256; i++)
		levelOverlay[i] = i & 0xff;

	if (_flags.use16ColorMode) {
		_screen->getLevelOverlay(6)[0xee] = 0xee;
		if (_lastSpecialColor == 0x44)
			_screen->getLevelOverlay(5)[0xee] = 0xee;

		for (int i = 0; i < 7; i++)
			memcpy(_screen->getLevelOverlay(i), _screen->getLevelOverlay(i + 1), 256);

		_screen->loadPalette("LOL.NOL", _screen->getPalette(0));

		for (int i = 0; i < 8; i++) {
			uint8 *pl = _screen->getLevelOverlay(7 - i);
			for (int ii = 0; ii < 16; ii++)
				_vcnExpTable[(i << 4) + ii] = pl[(ii << 4) | ii];
		}
	}

	_loadSuppFilesFlag = 0;
	generateBrightnessPalette(_screen->getPalette(0), _screen->getPalette(1), _brightness, _lampEffect);

	if (_flags.isTalkie) {
		char tname[13];
		snprintf(tname, sizeof(tname), "LEVEL%.02d.TLC", _currentLevel);
		Common::SeekableReadStream *s = _res->createReadStream(tname);
		s->read(_transparencyTable1, 256);
		s->read(_transparencyTable2, 5120);
		delete s;
	} else {
		createTransparencyTables();
	}

	_loadSuppFilesFlag = 1;
}

void LoLEngine::resetItems(int flag) {
	for (int i = 0; i < 1024; i++) {
		_levelBlockProperties[i].direction = 5;
		uint16 id = _levelBlockProperties[i].assignedObjects;
		MonsterInPlay *r = 0;

		while (id & 0x8000) {
			r = (MonsterInPlay *)findObject(id);
			id = r->nextAssignedObject;
		}

		if (!id)
			continue;

		ItemInPlay *it = &_itemsInPlay[id];
		it->level = _currentLevel;
		it->block = i;
		if (r)
			r->nextAssignedObject = 0;
	}

	if (flag)
		memset(_flyingObjects, 0, 8 * sizeof(FlyingObject));
}

void LoLEngine::disableMonsters() {
	memset(_monsters, 0, 30 * sizeof(MonsterInPlay));
	for (int i = 0; i < 30; i++)
		_monsters[i].mode = 0x10;
}

void LoLEngine::resetBlockProperties() {
	for (int i = 0; i < 1024; i++) {
		LevelBlockProperty *l = &_levelBlockProperties[i];
		if (l->flags & 0x10) {
			l->flags &= 0xef;
			if (testWallInvisibility(i, 0) && testWallInvisibility(i, 1))
				l->flags |= 0x40;
		} else {
			if (l->flags & 0x40)
				l->flags &= 0xbf;
			else if (l->flags & 0x80)
				l->flags &= 0x7f;
		}
	}
}

bool LoLEngine::testWallFlag(int block, int direction, int flag) {
	if (_levelBlockProperties[block].flags & 0x10)
		return true;

	if (direction != -1)
		return (_wllWallFlags[_levelBlockProperties[block].walls[direction ^ 2]] & flag) ? true : false;

	for (int i = 0; i < 4; i++) {
		if (_wllWallFlags[_levelBlockProperties[block].walls[i]] & flag)
			return true;
	}

	return false;
}

bool LoLEngine::testWallInvisibility(int block, int direction) {
	uint8 w = _levelBlockProperties[block].walls[direction];
	if (_wllVmpMap[w] || _wllShapeMap[w] || _levelBlockProperties[block].flags & 0x80)
		return false;
	return true;
}

void LoLEngine::resetLampStatus() {
	_flagsTable[31] |= 0x04;
	_lampEffect = -1;
	updateLampStatus();
}

void LoLEngine::setLampMode(bool lampOn) {
	_flagsTable[31] &= 0xFB;
	if (!(_flagsTable[30] & 0x08) || !lampOn)
		return;

	_screen->drawShape(0, _gameShapes[_flags.isTalkie ? 43 : 41], 291, 56, 0, 0);
	_lampEffect = 8;
}

void LoLEngine::updateLampStatus() {
	uint8 newLampEffect = 0;
	uint8 tmpOilStatus = 0;

	if ((_updateFlags & 4) || !(_flagsTable[31] & 0x08))
		return;

	if (!_brightness || !_lampOilStatus) {
		newLampEffect = 8;
		if (newLampEffect != _lampEffect && _screen->_fadeFlag == 0)
			setPaletteBrightness(_screen->getPalette(0), _brightness, newLampEffect);
	} else {
		tmpOilStatus = (_lampOilStatus < 100) ? _lampOilStatus : 100;
		newLampEffect = (3 - ((tmpOilStatus - 1) / 25)) << 1;

		if (_lampEffect == -1) {
			if (_screen->_fadeFlag == 0)
				setPaletteBrightness(_screen->getPalette(0), _brightness, newLampEffect);
			_lampStatusTimer = _system->getMillis() + (10 + rollDice(1, 30)) * _tickLength;
		} else {
			if ((_lampEffect & 0xfe) == (newLampEffect & 0xfe)) {
				if (_system->getMillis() <= _lampStatusTimer) {
					newLampEffect = _lampEffect;
				} else {
					newLampEffect = _lampEffect ^ 1;
					_lampStatusTimer = _system->getMillis() + (10 + rollDice(1, 30)) * _tickLength;
				}
			} else {
				if (_screen->_fadeFlag == 0)
					setPaletteBrightness(_screen->getPalette(0), _lampEffect, newLampEffect);
			}
		}
	}

	if (newLampEffect == _lampEffect)
		return;

	_screen->hideMouse();

	_screen->drawShape(_screen->_curPage, _gameShapes[(_flags.isTalkie ? 35 : 33) + newLampEffect], 291, 56, 0, 0);
	_screen->showMouse();

	_lampEffect = newLampEffect;
}

void LoLEngine::updateCompass() {
	if (!(_flagsTable[31] & 0x40) || (_updateFlags & 4))
		return;

	if (_compassDirection == -1) {
		_compassStep = 0;
		gui_drawCompass();
		return;
	}

	if (_compassTimer >= _system->getMillis())
		return;

	if ((_currentDirection << 6) == _compassDirection && (!_compassStep))
		return;

	_compassTimer = _system->getMillis() + 3 * _tickLength;
	int dir = _compassStep >= 0 ? 1 : -1;
	if (_compassStep)
		_compassStep -= (((ABS(_compassStep) >> 4) + 2) * dir);

	int16 d = _compassBroken ? (int8(_rnd.getRandomNumber(255)) - _compassDirection) : (_currentDirection << 6) - _compassDirection;
	if (d <= -128)
		d += 256;
	if (d >= 128)
		d -= 256;

	d >>= 2;
	_compassStep += d;
	_compassStep = CLIP(_compassStep, -24, 24);
	_compassDirection += _compassStep;

	if (_compassDirection < 0)
		_compassDirection += 256;
	if (_compassDirection > 255)
		_compassDirection -= 256;

	if ((_compassDirection >> 6) == _currentDirection && _compassStep < 2) {
		int16 d2 = d >> 16;
		d ^= d2;
		d -= d2;
		if (d < 4) {
			_compassDirection = _currentDirection << 6;
			_compassStep = 0;
		}
	}

	gui_drawCompass();
}

void LoLEngine::moveParty(uint16 direction, int unk1, int unk2, int buttonShape) {
	if (buttonShape)
		gui_toggleButtonDisplayMode(buttonShape, 1);

	uint16 opos = _currentBlock;
	uint16 npos = calcNewBlockPosition(_currentBlock, direction);

	if (!checkBlockPassability(npos, direction)) {
		notifyBlockNotPassable(unk2 ? 0 : 1);
		gui_toggleButtonDisplayMode(buttonShape, 0);
		return;
	}

	_scriptDirection = direction;
	_currentBlock = npos;
	_sceneDefaultUpdate = 1;

	calcCoordinates(_partyPosX, _partyPosY, _currentBlock, 0x80, 0x80);
	_flagsTable[73] &= 0xFD;

	runLevelScript(opos, 4);
	runLevelScript(npos, 1);

	if (!(_flagsTable[73] & 0x02)) {
		initTextFading(2, 0);

		if (_sceneDefaultUpdate) {
			switch (unk2) {
			case 0:
				movePartySmoothScrollUp(2);
				break;
			case 1:
				movePartySmoothScrollDown(2);
				break;
			case 2:
				movePartySmoothScrollLeft(1);
				break;
			case 3:
				movePartySmoothScrollRight(1);
				break;
			default:
				break;
			}
		} else {
			gui_drawScene(0);
		}

		gui_toggleButtonDisplayMode(buttonShape, 0);

		if (npos == _currentBlock) {
			runLevelScript(opos, 8);
			runLevelScript(npos, 2);

			if (_levelBlockProperties[npos].walls[0] == 0x1a)
				memset(_levelBlockProperties[npos].walls, 0, 4);
		}
	}

	updateAutoMap(_currentBlock);
}

uint16 LoLEngine::calcNewBlockPosition(uint16 curBlock, uint16 direction) {
	static const int16 blockPosTable[] = { -32, 1, 32, -1 };
	return (curBlock + blockPosTable[direction]) & 0x3ff;
}

uint16 LoLEngine::calcBlockIndex(uint16 x, uint16 y) {
	return (((y & 0xff00) >> 3) | (x >> 8)) & 0x3ff;
}

void LoLEngine::calcCoordinates(uint16 &x, uint16 &y, int block, uint16 xOffs, uint16 yOffs) {
	x = (block & 0x1f) << 8 | xOffs;
	y = ((block & 0xffe0) << 3) | yOffs;
}

void LoLEngine::calcCoordinatesForSingleCharacter(int charNum, uint16 &x, uint16 &y) {
	static const uint8 xOffsets[] = {  0x80, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x40, 0x80, 0xC0 };
	int c = countActiveCharacters();
	if (!c)
		return;

	c = (c - 1) * 3 + charNum;

	x = xOffsets[c];
	y = 0x80;

	calcCoordinatesAddDirectionOffset(x, y, _currentDirection);

	x |= (_partyPosX & 0xff00);
	y |= (_partyPosY & 0xff00);
}

void LoLEngine::calcCoordinatesAddDirectionOffset(uint16 &x, uint16 &y, int direction) {
	if (!direction)
		return;

	int tx = x;
	int ty = y;

	if (direction & 1)
		SWAP(tx, ty);

	if (direction != 1)
		ty = (ty - 256) * -1;

	if (direction != 3) {
		tx = (tx - 256) * -1;
	}

	x = tx;
	y = ty;
}

bool LoLEngine::checkBlockPassability(uint16 block, uint16 direction) {
	if (testWallFlag(block, direction, 1))
		return false;

	uint16 d = _levelBlockProperties[block].assignedObjects;

	while (d) {
		if (d & 0x8000)
			return false;
		d = findObject(d)->nextAssignedObject;
	}

	return true;
}

void LoLEngine::notifyBlockNotPassable(int scrollFlag) {
	if (scrollFlag)
		movePartySmoothScrollBlocked(2);

	snd_stopSpeech(true);
	_txt->printMessage(0x8002, "%s", getLangString(0x403f));
	snd_playSoundEffect(19, -1);
}

int LoLEngine::clickedWallShape(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	snd_stopSpeech(true);
	runLevelScript(block, 0x40);

	return 1;
}

int LoLEngine::clickedLeverOn(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	_levelBlockProperties[block].walls[direction]++;
	_sceneUpdateRequired = true;

	snd_playSoundEffect(30, -1);

	runLevelScript(block, 0x40);

	return 1;
}

int LoLEngine::clickedLeverOff(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	_levelBlockProperties[block].walls[direction]--;
	_sceneUpdateRequired = true;

	snd_playSoundEffect(29, -1);

	runLevelScript(block, 0x40);
	return 1;
}

int LoLEngine::clickedWallOnlyScript(uint16 block) {
	runLevelScript(block, 0x40);
	return 1;
}

int LoLEngine::clickedDoorSwitch(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	snd_playSoundEffect(78, -1);
	_blockDoor = 0;
	runLevelScript(block, 0x40);

	if (!_blockDoor) {
		delay(15 * _tickLength);
		processDoorSwitch(block, 0);
	}

	return 1;
}

int LoLEngine::clickedNiche(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v) || !_itemInHand)
		return 0;

	uint16 x = 0x80;
	uint16 y = 0xff;
	calcCoordinatesAddDirectionOffset(x, y, _currentDirection);
	calcCoordinates(x, y, block, x, y);
	setItemPosition(_itemInHand, x, y, 8, 1);
	setHandItem(0);
	return 1;
}

bool LoLEngine::clickedShape(int shapeIndex) {
	while (shapeIndex) {
		uint16 s = _levelShapeProperties[shapeIndex].shapeIndex[1];

		if (s == 0xffff) {
			shapeIndex = _levelShapeProperties[shapeIndex].next;
			continue;
		}

		int w = _levelShapes[s][3];
		int h = _levelShapes[s][2];
		int x = _levelShapeProperties[shapeIndex].shapeX[1] + 136;
		int y = _levelShapeProperties[shapeIndex].shapeY[1] + 8;

		if (_levelShapeProperties[shapeIndex].flags & 1)
			w <<= 1;

		if (posWithinRect(_mouseX, _mouseY, x - 4, y - 4, x + w + 8, y + h + 8))
			return true;

		shapeIndex = _levelShapeProperties[shapeIndex].next;
	}

	return false;
}

void LoLEngine::processDoorSwitch(uint16 block, int openClose) {
	if ((block == _currentBlock) || (_levelBlockProperties[block].assignedObjects & 0x8000))
		return;

	if (openClose == 0) {
		for (int i = 0; i < 3; i++) {
			if (_openDoorState[i].block != block)
				continue;
			openClose = -_openDoorState[i].state;
			break;
		}
	}

	if (openClose == 0)
		openClose = (_wllWallFlags[_levelBlockProperties[block].walls[_wllWallFlags[_levelBlockProperties[block].walls[0]] & 8 ? 0 : 1]] & 1) ? 1 : -1;

	openCloseDoor(block, openClose);
}

void LoLEngine::openCloseDoor(uint16 block, int openClose) {
	int s1 = -1;
	int s2 = -1;

	int c = (_wllWallFlags[_levelBlockProperties[block].walls[0]] & 8) ? 0 : 1;
	int v = _levelBlockProperties[block].walls[c];
	int flg = (openClose == 1) ? 0x10 : (openClose == -1 ? 0x20 : 0);

	if (_wllWallFlags[v] & flg)
		return;

	for (int i = 0; i < 3; i++) {
		if (_openDoorState[i].block == block) {
			s1 = i;
			break;
		} else if (_openDoorState[i].block == 0 && s2 == -1) {
			s2 = i;
		}
	}

	if (s1 != -1 || s2 != -1) {
		if (s1 == -1)
			s1 = s2;

		_openDoorState[s1].block = block;
		_openDoorState[s1].state = openClose;
		_openDoorState[s1].wall = c;

		flg = (-openClose == 1) ? 0x10 : (-openClose == -1 ? 0x20 : 0);

		if (_wllWallFlags[v] & flg) {
			_levelBlockProperties[block].walls[c] += openClose;
			_levelBlockProperties[block].walls[c ^ 2] += openClose;

			int snd = (openClose == -1) ? 32 : 31;

			snd_processEnvironmentalSoundEffect(snd, block);
			if (!checkSceneUpdateNeed(block))
				updateEnvironmentalSfx(0);
		}

		enableTimer(0);

	} else {
		while (!(flg & _wllWallFlags[v]))
			v += openClose;

		_levelBlockProperties[block].walls[c] = _levelBlockProperties[block].walls[c ^ 2] = v;
		checkSceneUpdateNeed(block);
	}
}

void LoLEngine::completeDoorOperations() {
	for (int i = 0; i < 3; i++) {
		if (!_openDoorState[i].block)
			continue;

		uint16 b = _openDoorState[i].block;

		do {
			_levelBlockProperties[b].walls[_openDoorState[i].wall] += _openDoorState[i].state;
			_levelBlockProperties[b].walls[_openDoorState[i].wall ^ 2] += _openDoorState[i].state;
		} while	(!(_wllWallFlags[_levelBlockProperties[b].walls[_openDoorState[i].wall]] & 0x30));

		_openDoorState[i].block = 0;
	}
}

void LoLEngine::movePartySmoothScrollBlocked(int speed) {
	if (!_smoothScrollingEnabled || (_smoothScrollingEnabled && _needSceneRestore))
		return;

	_screen->backupSceneWindow(_sceneDrawPage2 == 2 ? 2 : 6, 6);

	for (int i = 0; i < 2; i++) {
		uint32 delayTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);
		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(delayTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	for (int i = 2; i; i--) {
		uint32 delayTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);
		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(delayTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	if (_sceneDefaultUpdate != 2) {
		_screen->restoreSceneWindow(6, 0);
		_screen->updateScreen();
	}

	updateDrawPage2();
}

void LoLEngine::movePartySmoothScrollUp(int speed) {
	if (!_smoothScrollingEnabled || (_smoothScrollingEnabled && _needSceneRestore))
		return;

	int d = 0;

	if (_sceneDrawPage2 == 2) {
		d = smoothScrollDrawSpecialGuiShape(6);
		gui_drawScene(6);
		_screen->backupSceneWindow(6, 12);
		_screen->backupSceneWindow(2, 6);
	} else {
		d = smoothScrollDrawSpecialGuiShape(2);
		gui_drawScene(2);
		_screen->backupSceneWindow(2, 12);
		_screen->backupSceneWindow(6, 6);
	}

	for (int i = 0; i < 5; i++) {
		uint32 delayTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);

		if (d)
			_screen->copyGuiShapeToSurface(14, 2);

		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(delayTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	if (d)
		_screen->copyGuiShapeToSurface(14, 12);

	if (_sceneDefaultUpdate != 2) {
		_screen->restoreSceneWindow(12, 0);
		_screen->updateScreen();
	}

	updateDrawPage2();
}

void LoLEngine::movePartySmoothScrollDown(int speed) {
	if (!_smoothScrollingEnabled)
		return;

	int d = smoothScrollDrawSpecialGuiShape(2);
	gui_drawScene(2);
	_screen->backupSceneWindow(2, 6);

	for (int i = 4; i >= 0; i--) {
		uint32 delayTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);

		if (d)
			_screen->copyGuiShapeToSurface(14, 2);

		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(delayTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	if (d)
		_screen->copyGuiShapeToSurface(14, 12);

	if (_sceneDefaultUpdate != 2) {
		_screen->restoreSceneWindow(6, 0);
		_screen->updateScreen();
	}

	updateDrawPage2();
}

void LoLEngine::movePartySmoothScrollLeft(int speed) {
	if (!_smoothScrollingEnabled)
		return;

	speed <<= 1;

	gui_drawScene(_sceneDrawPage1);

	for (int i = 88, d = 88; i > 22; i -= 22, d += 22) {
		uint32 delayTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollHorizontalStep(_sceneDrawPage2, 66, d, i);
		_screen->copyRegion(112 + i, 0, 112, 0, d, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		fadeText();
		delayUntil(delayTimer);
	}

	if (_sceneDefaultUpdate != 2) {
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	SWAP(_sceneDrawPage1, _sceneDrawPage2);
}

void LoLEngine::movePartySmoothScrollRight(int speed) {
	if (!_smoothScrollingEnabled)
		return;

	speed <<= 1;

	gui_drawScene(_sceneDrawPage1);

	uint32 delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->copyRegion(112, 0, 222, 0, 66, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollHorizontalStep(_sceneDrawPage2, 22, 0, 66);
	_screen->copyRegion(112, 0, 200, 0, 88, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollHorizontalStep(_sceneDrawPage2, 44, 0, 22);
	_screen->copyRegion(112, 0, 178, 0, 110, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	if (_sceneDefaultUpdate != 2) {
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	SWAP(_sceneDrawPage1, _sceneDrawPage2);
}

void LoLEngine::movePartySmoothScrollTurnLeft(int speed) {
	if (!_smoothScrollingEnabled)
		return;

	speed <<= 1;

	int d = smoothScrollDrawSpecialGuiShape(_sceneDrawPage1);
	gui_drawScene(_sceneDrawPage1);
	int dp = _sceneDrawPage2 == 2 ? _sceneDrawPage2 : _sceneDrawPage1;

	uint32 delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep1(_sceneDrawPage1, _sceneDrawPage2, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep2(_sceneDrawPage1, _sceneDrawPage2, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep3(_sceneDrawPage1, _sceneDrawPage2, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	if (_sceneDefaultUpdate != 2) {
		drawSpecialGuiShape(_sceneDrawPage1);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}
}

void LoLEngine::movePartySmoothScrollTurnRight(int speed) {
	if (!_smoothScrollingEnabled)
		return;

	speed <<= 1;

	int d = smoothScrollDrawSpecialGuiShape(_sceneDrawPage1);
	gui_drawScene(_sceneDrawPage1);
	int dp = _sceneDrawPage2 == 2 ? _sceneDrawPage2 : _sceneDrawPage1;

	uint32 delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep3(_sceneDrawPage2, _sceneDrawPage1, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep2(_sceneDrawPage2, _sceneDrawPage1, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep1(_sceneDrawPage2, _sceneDrawPage1, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	if (_sceneDefaultUpdate != 2) {
		drawSpecialGuiShape(_sceneDrawPage1);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}
}

void LoLEngine::pitDropScroll(int numSteps) {
	_screen->copyRegionSpecial(0, 320, 200, 112, 0, 6, 176, 120, 0, 0, 176, 120, 0);
	uint32 etime = 0;

	for (int i = 0; i < numSteps; i++) {
		etime = _system->getMillis() + _tickLength;
		int ys = ((30720 / numSteps) * i) >> 8;
		_screen->copyRegionSpecial(6, 176, 120, 0, ys, 0, 320, 200, 112, 0, 176, 120 - ys, 0);
		_screen->copyRegionSpecial(2, 320, 200, 112, 0, 0, 320, 200, 112, 120 - ys, 176, ys, 0);
		_screen->updateScreen();

		delayUntil(etime);
	}

	etime = _system->getMillis() + _tickLength;

	_screen->copyRegionSpecial(2, 320, 200, 112, 0, 0, 320, 200, 112, 0, 176, 120, 0);
	_screen->updateScreen();

	delayUntil(etime);

	updateDrawPage2();
}

void LoLEngine::shakeScene(int duration, int width, int height, int restore) {
	_screen->copyRegion(112, 0, 112, 0, 176, 120, 0, 6, Screen::CR_NO_P_CHECK);
	uint32 endTime = _system->getMillis() + duration * _tickLength;

	while (endTime > _system->getMillis()) {
		uint32 delayTimer = _system->getMillis() + 2 * _tickLength;

		int s1 = width ? (_rnd.getRandomNumber(255) % (width << 1)) - width : 0;
		int s2 = height ? (_rnd.getRandomNumber(255) % (height << 1)) - height : 0;

		int x1, y1, x2, y2, w, h;
		if (s1 >= 0) {
			x1 = 112;
			x2 = 112 + s1;
			w = 176 - s1;
		} else {
			x1 = 112 - s1;
			x2 = 112;
			w = 176 + s1;
		}

		if (s2 >= 0) {
			y1 = 0;
			y2 = s2;
			h = 120 - s2;
		} else {
			y1 = -s2;
			y2 = 0;
			h = 120 + s2;
		}

		_screen->copyRegion(x1, y1, x2, y2, w, h, 6, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		delayUntil(delayTimer);
	}

	if (restore) {
		_screen->copyRegion(112, 0, 112, 0, 176, 120, 6, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		updateDrawPage2();
	}
}

void LoLEngine::processGasExplosion(int soundId) {
	int cp = _screen->setCurPage(2);
	_screen->copyPage(0, 12);

	static const uint8 sounds[] = { 0x62, 0xA7, 0xA7, 0xA8 };
	snd_playSoundEffect(sounds[soundId], -1);

	uint16 targetBlock = 0;
	int dist = getSpellTargetBlock(_currentBlock, _currentDirection, 3, targetBlock);

	uint8 *p1 = _screen->getPalette(1).getData();
	uint8 *p2 = _screen->getPalette(3).getData();

	if (dist) {
		WSAMovie_v2 *mov = new WSAMovie_v2(this);
		char file[13];
		snprintf(file, 13, "gasexp%0d.wsa", dist);
		mov->open(file, 1, 0);
		if (!mov->opened())
			error("Gas: Unable to load gasexp.wsa");

		playSpellAnimation(mov, 0, 6, 1, (176 - mov->width()) / 2 + 112, (120 - mov->height()) / 2, 0, 0, 0, 0, false);

		mov->close();
		delete mov;

	} else {
		memcpy(p2, p1, 768);

		for (int i = 1; i < 128; i++)
			p2[i * 3] = 0x3f;

		uint32 ctime = _system->getMillis();
		while (_screen->fadePaletteStep(_screen->getPalette(0).getData(), p2, _system->getMillis() - ctime, 10))
			updateInput();

		ctime = _system->getMillis();
		while (_screen->fadePaletteStep(p2, _screen->getPalette(0).getData(), _system->getMillis() - ctime, 50))
			updateInput();
	}

	_screen->copyPage(12, 2);
	_screen->setCurPage(cp);

	updateDrawPage2();
	_sceneUpdateRequired = true;
	gui_drawScene(0);
}

int LoLEngine::smoothScrollDrawSpecialGuiShape(int pageNum) {
	if (!_specialGuiShape)
		return 0;

	_screen->clearGuiShapeMemory(pageNum);
	_screen->drawShape(pageNum, _specialGuiShape, _specialGuiShapeX, _specialGuiShapeY, 2, 0);
	_screen->copyGuiShapeFromSceneBackupBuffer(pageNum, 14);
	return 1;
}

void LoLEngine::drawScene(int pageNum) {
	if (pageNum && pageNum != _sceneDrawPage1) {
		SWAP(_sceneDrawPage1, _sceneDrawPage2);
		updateDrawPage2();
	}

	if (pageNum && pageNum != _sceneDrawPage1) {
		SWAP(_sceneDrawPage1, _sceneDrawPage2);
		updateDrawPage2();
	}

	generateBlockDrawingBuffer();
	drawVcnBlocks();
	drawSceneShapes();

	if (!pageNum) {
		drawSpecialGuiShape(_sceneDrawPage1);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		SWAP(_sceneDrawPage1, _sceneDrawPage2);
	}

	updateEnvironmentalSfx(0);
	gui_drawCompass();

	_sceneUpdateRequired = false;
}


void LoLEngine::setWallType(int block, int wall, int val) {
	if (wall == -1) {
		for (int i = 0; i < 4; i++)
			_levelBlockProperties[block].walls[i] = val;
		if (_wllBuffer4[val] == 17) {
			_levelBlockProperties[block].flags &= 0xef;
			_levelBlockProperties[block].flags |= 0x20;
		} else {
			_levelBlockProperties[block].flags &= 0xdf;
		}
	} else {
		_levelBlockProperties[block].walls[wall] = val;
	}

	checkSceneUpdateNeed(block);
}

void LoLEngine::updateDrawPage2() {
	_screen->copyRegion(112, 0, 112, 0, 176, 120, 0, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
}

void LoLEngine::prepareSpecialScene(int fieldType, int hasDialogue, int suspendGui, int allowSceneUpdate, int controlMode, int fadeFlag) {
	resetPortraitsAndDisableSysTimer();
	if (fieldType) {
		if (suspendGui)
			gui_specialSceneSuspendControls(1);

		if (!allowSceneUpdate)
			_sceneDefaultUpdate = 0;

		if (hasDialogue)
			initDialogueSequence(fieldType, 0);

		if (fadeFlag) {
			if (_flags.use16ColorMode)
				setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
			else
				_screen->fadePalette(_screen->getPalette(3), 10);
			_screen->_fadeFlag = 0;
		}

		setSpecialSceneButtons(0, 0, 320, 130, controlMode);

	} else {
		if (suspendGui)
			gui_specialSceneSuspendControls(0);

		if (!allowSceneUpdate)
			_sceneDefaultUpdate = 0;

		gui_disableControls(controlMode);

		if (fadeFlag) {
			if (_flags.use16ColorMode) {
				setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
			} else {
				_screen->getPalette(3).copy(_screen->getPalette(0), 128);
				_screen->loadSpecialColors(_screen->getPalette(3));
				_screen->fadePalette(_screen->getPalette(3), 10);
			}
			_screen->_fadeFlag = 0;
		}

		if (hasDialogue)
			initDialogueSequence(fieldType, 0);

		setSpecialSceneButtons(112, 0, 176, 120, controlMode);
	}
}

int LoLEngine::restoreAfterSpecialScene(int fadeFlag, int redrawPlayField, int releaseTimScripts, int sceneUpdateMode) {
	if (!_needSceneRestore)
		return 0;

	_needSceneRestore = 0;
	enableSysTimer(2);

	if (_dialogueField)
		restoreAfterDialogueSequence(_currentControlMode);

	if (_specialSceneFlag)
		gui_specialSceneRestoreControls(_currentControlMode);

	int l = _currentControlMode;
	_currentControlMode = 0;

	gui_specialSceneRestoreButtons();
	calcCharPortraitXpos();

	_currentControlMode = l;

	if (releaseTimScripts) {
		for (int i = 0; i < TIM::kWSASlots; i++)
			_tim->freeAnimStruct(i);

		for (int i = 0; i < 10; i++)
			_tim->unload(_activeTim[i]);
	}

	gui_enableControls();

	if (fadeFlag) {
		if ((_screen->_fadeFlag != 1 && _screen->_fadeFlag != 2) || (_screen->_fadeFlag == 1 && _currentControlMode)) {
			if (_currentControlMode)
				_screen->fadeToBlack(10);
			else
				_screen->fadeClearSceneWindow(10);
		}

		_currentControlMode = 0;
		calcCharPortraitXpos();

		if (redrawPlayField)
			gui_drawPlayField();

		setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);

	} else {
		_currentControlMode = 0;
		calcCharPortraitXpos();

		if (redrawPlayField)
			gui_drawPlayField();
	}

	_sceneDefaultUpdate = sceneUpdateMode;
	return 1;
}

void LoLEngine::setSequenceButtons(int x, int y, int w, int h, int enableFlags) {
	gui_enableSequenceButtons(x, y, w, h, enableFlags);
	_seqWindowX1 = x;
	_seqWindowY1 = y;
	_seqWindowX2 = x + w;
	_seqWindowY2 = y + h;
	int offs = _itemInHand ? 10 : 0;
	_screen->setMouseCursor(offs, offs, getItemIconShapePtr(_itemInHand));
	_currentFloatingCursor = -1;
	if (w == 320) {
		setLampMode(0);
		_lampStatusSuspended = true;
	}
}

void LoLEngine::setSpecialSceneButtons(int x, int y, int w, int h, int enableFlags) {
	gui_enableSequenceButtons(x, y, w, h, enableFlags);
	_spsWindowX = x;
	_spsWindowY = y;
	_spsWindowW = w;
	_spsWindowH = h;
}

void LoLEngine::setDefaultButtonState() {
	gui_enableDefaultPlayfieldButtons();
	_seqWindowX1 = _seqWindowY1 = _seqWindowX2 = _seqWindowY2 = _seqTrigger = 0;
	if (_lampStatusSuspended)
		resetLampStatus();
	_lampStatusSuspended = false;
}

void LoLEngine::generateBlockDrawingBuffer() {
	_sceneDrawVarDown = _dscBlockMap[_currentDirection];
	_sceneDrawVarRight = _dscBlockMap[_currentDirection + 4];
	_sceneDrawVarLeft = _dscBlockMap[_currentDirection + 8];

	/*******************************************
    *             _visibleBlocks map           *
	*                                          *
	*     |     |     |     |     |     |      *
	*  00 |  01 |  02 |  03 |  04 |  05 |  06  *
	* ____|_____|_____|_____|_____|_____|_____ *
	*     |     |     |     |     |     |      *
	*     |  07 |  08 |  09 |  10 |  11 |      *
	*     |_____|_____|_____|_____|_____|      *
	*           |     |     |     |            *
	*           |  12 |  13 |  14 |            *
	*           |_____|_____|_____|            *
	*                 |     |                  *
	*              15 |  16 |  17              *
	*                 | (P) |                  *
	********************************************/

	memset(_blockDrawingBuffer, 0, 660 * sizeof(uint16));

	_wllProcessFlag = ((_currentBlock >> 5) + (_currentBlock & 0x1f) + _currentDirection) & 1;

	if (_wllProcessFlag) // floor and ceiling
		generateVmpTileDataFlipped(0, 15, 1, -330, 22, 15);
	else
		generateVmpTileData(0, 15, 1, -330, 22, 15);

	assignVisibleBlocks(_currentBlock, _currentDirection);

	uint8 t = _visibleBlocks[0]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(-2, 3, t, 102, 3, 5);

	t = _visibleBlocks[6]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(21, 3, t, 102, 3, 5);

	t = _visibleBlocks[1]->walls[_sceneDrawVarRight];
	uint8 t2 = _visibleBlocks[2]->walls[_sceneDrawVarDown];

	if (hasWall(t) && !(_wllWallFlags[t2] & 8))
		generateVmpTileData(2, 3, t, 102, 3, 5);
	else if (t && (_wllWallFlags[t2] & 8))
		generateVmpTileData(2, 3, t2, 102, 3, 5);

	t = _visibleBlocks[5]->walls[_sceneDrawVarLeft];
	t2 = _visibleBlocks[4]->walls[_sceneDrawVarDown];

	if (hasWall(t) && !(_wllWallFlags[t2] & 8))
		generateVmpTileDataFlipped(17, 3, t, 102, 3, 5);
	else if (t && (_wllWallFlags[t2] & 8))
		generateVmpTileDataFlipped(17, 3, t2, 102, 3, 5);

	t = _visibleBlocks[2]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(8, 3, t, 97, 1, 5);

	t = _visibleBlocks[4]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(13, 3, t, 97, 1, 5);

	t = _visibleBlocks[1]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(-4, 3, t, 129, 6, 5);

	t = _visibleBlocks[5]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(20, 3, t, 129, 6, 5);

	t = _visibleBlocks[2]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(2, 3, t, 129, 6, 5);

	t = _visibleBlocks[4]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(14, 3, t, 129, 6, 5);

	t = _visibleBlocks[3]->walls[_sceneDrawVarDown];
	if (t)
		generateVmpTileData(8, 3, t, 129, 6, 5);

	t = _visibleBlocks[7]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(0, 3, t, 117, 2, 6);

	t = _visibleBlocks[11]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(20, 3, t, 117, 2, 6);

	t = _visibleBlocks[8]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(6, 2, t, 81, 2, 8);

	t = _visibleBlocks[10]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(14, 2, t, 81, 2, 8);

	t = _visibleBlocks[8]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(-4, 2, t, 159, 10, 8);

	t = _visibleBlocks[10]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(16, 2, t, 159, 10, 8);

	t = _visibleBlocks[9]->walls[_sceneDrawVarDown];
	if (t)
		generateVmpTileData(6, 2, t, 159, 10, 8);

	t = _visibleBlocks[12]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(3, 1, t, 45, 3, 12);

	t = _visibleBlocks[14]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(16, 1, t, 45, 3, 12);

	t = _visibleBlocks[12]->walls[_sceneDrawVarDown];
	if (!(_wllWallFlags[t] & 8))
		generateVmpTileData(-13, 1, t, 239, 16, 12);

	t = _visibleBlocks[14]->walls[_sceneDrawVarDown];
	if (!(_wllWallFlags[t] & 8))
		generateVmpTileData(19, 1, t, 239, 16, 12);

	t = _visibleBlocks[13]->walls[_sceneDrawVarDown];
	if (t)
		generateVmpTileData(3, 1, t, 239, 16, 12);

	t = _visibleBlocks[15]->walls[_sceneDrawVarRight];
	t2 = _visibleBlocks[17]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileData(0, 0, t, 0, 3, 15);
	if (t2)
		generateVmpTileDataFlipped(19, 0, t2, 0, 3, 15);
}

void LoLEngine::generateVmpTileData(int16 startBlockX, uint8 startBlockY, uint8 vmpMapIndex, int16 vmpOffset, uint8 numBlocksX, uint8 numBlocksY) {
	if (!_wllVmpMap[vmpMapIndex])
		return;

	uint16 *vmp = &_vmpPtr[(_wllVmpMap[vmpMapIndex] - 1) * 431 + vmpOffset + 330];

	for (int i = 0; i < numBlocksY; i++) {
		uint16 *bl = &_blockDrawingBuffer[(startBlockY + i) * 22 + startBlockX];
		for (int ii = 0; ii < numBlocksX; ii++) {
			if ((startBlockX + ii >= 0) && (startBlockX + ii < 22) && *vmp)
				*bl = *vmp;
			bl++;
			vmp++;
		}
	}
}

void LoLEngine::generateVmpTileDataFlipped(int16 startBlockX, uint8 startBlockY, uint8 vmpMapIndex, int16 vmpOffset, uint8 numBlocksX, uint8 numBlocksY) {
	if (!_wllVmpMap[vmpMapIndex])
		return;

	uint16 *vmp = &_vmpPtr[(_wllVmpMap[vmpMapIndex] - 1) * 431 + vmpOffset + 330];

	for (int i = 0; i < numBlocksY; i++) {
		for (int ii = 0; ii < numBlocksX; ii++) {
			if ((startBlockX + ii) < 0 || (startBlockX + ii) > 21)
				continue;

			uint16 v = vmp[i * numBlocksX + (numBlocksX - 1 - ii)];
			if (!v)
				continue;

			if (v & 0x4000)
				v -= 0x4000;
			else
				v |= 0x4000;

			_blockDrawingBuffer[(startBlockY + i) * 22 + startBlockX + ii] = v;
		}
	}
}

bool LoLEngine::hasWall(int index) {
	if (!index || (_wllWallFlags[index] & 8))
		return false;
	return true;
}

void LoLEngine::assignVisibleBlocks(int block, int direction) {
	for (int i = 0; i < 18; i++) {
		uint16 t = (block + _dscBlockIndex[direction * 18 + i]) & 0x3ff;
		_visibleBlockIndex[i] = t;

		_visibleBlocks[i] = &_levelBlockProperties[t];
		_lvlShapeLeftRight[i] = _lvlShapeLeftRight[18 + i] = -1;
	}
}

void LoLEngine::drawVcnBlocks() {
	uint8 *d = _sceneWindowBuffer;
	uint16 *bdb = _blockDrawingBuffer;

	for (int y = 0; y < 15; y++) {
		for (int x = 0; x < 22; x++) {
			bool horizontalFlip = false;
			int remainder = 0;

			uint16 vcnOffset = *bdb++;

			if (vcnOffset & 0x8000) {
				// this renders a wall block over the transparent pixels of a floor/ceiling block
				remainder = vcnOffset - 0x8000;
				vcnOffset = 0;
			}

			if (vcnOffset & 0x4000) {
				horizontalFlip = true;
				vcnOffset &= 0x3fff;
			}

			uint8 *src = 0;
			if (vcnOffset) {
				src = &_vcnBlocks[vcnOffset << 5];
			} else {
				// floor/ceiling blocks
				vcnOffset = bdb[329];
				if (vcnOffset & 0x4000) {
					horizontalFlip = true;
					vcnOffset &= 0x3fff;
				}

				src = (_vcfBlocks ? _vcfBlocks : _vcnBlocks) + (vcnOffset << 5);
			}

			uint8 shift = _vcnShift ? _vcnShift[vcnOffset] : _blockBrightness;

			if (horizontalFlip) {
				for (int blockY = 0; blockY < 8; blockY++) {
					src += 3;
					for (int blockX = 0; blockX < 4; blockX++) {
						uint8 t = *src--;
						*d++ = _vcnExpTable[(t & 0x0f) | shift];
						*d++ = _vcnExpTable[(t >> 4) | shift];
					}
					src += 5;
					d += 168;
				}
			} else {
				for (int blockY = 0; blockY < 8; blockY++) {
					for (int blockX = 0; blockX < 4; blockX++) {
						uint8 t = *src++;
						*d++ = _vcnExpTable[(t >> 4) | shift];
						*d++ = _vcnExpTable[(t & 0x0f) | shift];
					}
					d += 168;
				}
			}
			d -= 1400;

			if (remainder) {
				d -= 8;
				horizontalFlip = false;

				if (remainder & 0x4000) {
					remainder &= 0x3fff;
					horizontalFlip = true;
				}

				shift = _vcnShift? _vcnShift[remainder] : _blockBrightness;
				src = &_vcnBlocks[remainder << 5];

				if (horizontalFlip) {
					for (int blockY = 0; blockY < 8; blockY++) {
						src += 3;
						for (int blockX = 0; blockX < 4; blockX++) {
							uint8 t = *src--;
							uint8 h = _vcnExpTable[(t & 0x0f) | shift];
							uint8 l = _vcnExpTable[(t >> 4) | shift];
							if (h)
								*d = h;
							d++;
							if (l)
								*d = l;
							d++;
						}
						src += 5;
						d += 168;
					}
				} else {
					for (int blockY = 0; blockY < 8; blockY++) {
						for (int blockX = 0; blockX < 4; blockX++) {
							uint8 t = *src++;
							uint8 h = _vcnExpTable[(t >> 4) | shift];
							uint8 l = _vcnExpTable[(t & 0x0f) | shift];
							if (h)
								*d = h;
							d++;
							if (l)
								*d = l;
							d++;
						}
						d += 168;
					}
				}
				d -= 1400;
			}
		}
		d += 1232;
	}

	_screen->copyBlockToPage(_sceneDrawPage1, 112, 0, 176, 120, _sceneWindowBuffer);
}

void LoLEngine::drawSceneShapes() {
	for (int i = 0; i < 18; i++) {
		uint8 t = _dscTileIndex[i];
		uint8 s = _visibleBlocks[t]->walls[_sceneDrawVarDown];

		int16 x1 = 0;
		int16 x2 = 0;

		int16 dimY1 = 0;
		int16 dimY2 = 0;

		setLevelShapesDim(t, x1, x2, 13);

		if (x2 <= x1)
			continue;

		drawDecorations(t);

		uint16 w = _wllWallFlags[s];

		if (t == 16)
			w |= 0x80;

		drawBlockEffects(t, 0);

		if (_visibleBlocks[t]->assignedObjects && (w & 0x80))
			drawBlockObjects(t);

		drawBlockEffects(t, 1);

		if (!(w & 8))
			continue;

		uint16 v = 20 * (s - _dscUnk2[s]);

		scaleLevelShapesDim(t, dimY1, dimY2, 13);
		drawDoor(_doorShapes[_dscDoorShpIndex[s]], 0, t, 10, 0, -v, 2);
		setLevelShapesDim(t, dimY1, dimY2, 13);
	}
}

void LoLEngine::setLevelShapesDim(int index, int16 &x1, int16 &x2, int dim) {
	if (_lvlShapeLeftRight[index << 1] == -1) {
		x1 = 0;
		x2 = 22;

		int16 y1 = 0;
		int16 y2 = 120;

		int m = index * 18;

		for (int i = 0; i < 18; i++) {
			uint8 d = _visibleBlocks[i]->walls[_sceneDrawVarDown];
			uint8 a = _wllWallFlags[d];

			if (a & 8) {
				int t = _dscDim2[(m + i) << 1];

				if (t > x1) {
					x1 = t;
					if (!(a & 0x10))
						scaleLevelShapesDim(index, y1, y2, -1);
				}

				t = _dscDim2[((m + i) << 1) + 1];

				if (t < x2) {
					x2 = t;
					if (!(a & 0x10))
						scaleLevelShapesDim(index, y1, y2, -1);
				}
			} else {
				int t = _dscDim1[m + i];

				if (!_wllVmpMap[d] || t == -40)
					continue;

				if (t == -41) {
					x1 = 22;
					x2 = 0;
					break;
				}

				if (t > 0 && x2 > t)
					x2 = t;

				if (t < 0 && x1 < -t)
					x1 = -t;
			}

			if (x2 < x1)
				break;
		}

		x1 += 14;
		x2 += 14;

		_lvlShapeTop[index] = y1;
		_lvlShapeBottom[index] = y2;
		_lvlShapeLeftRight[index << 1] = x1;
		_lvlShapeLeftRight[(index << 1) + 1] = x2;
	} else {
		x1 = _lvlShapeLeftRight[index << 1];
		x2 = _lvlShapeLeftRight[(index << 1) + 1];
	}

	drawLevelModifyScreenDim(dim, x1, 0, x2, 15);
}

void LoLEngine::scaleLevelShapesDim(int index, int16 &y1, int16 &y2, int dim) {
	static const int8 dscY1[] = { 0x1E, 0x18, 0x10, 0x00 };
	static const int8 dscY2[] = { 0x3B, 0x47, 0x56, 0x78 };

	uint8 a = _dscDimMap[index];

	if (dim == -1 && a != 3)
		a++;

	y1 = dscY1[a];
	y2 = dscY2[a];

	if (dim == -1)
		return;

	const ScreenDim *cDim = _screen->getScreenDim(dim);

	_screen->modifyScreenDim(dim, cDim->sx, y1, cDim->w, y2 - y1);
}

void LoLEngine::drawLevelModifyScreenDim(int dim, int16 x1, int16 y1, int16 x2, int16 y2) {
	_screen->modifyScreenDim(dim, x1, y1 << 3, x2 - x1, (y2 - y1) << 3);
}

void LoLEngine::drawDecorations(int index) {
	for (int i = 1; i >= 0; i--) {
		int s = index * 2 + i;
		uint16 scaleW = _dscShapeScaleW[s];
		uint16 scaleH = _dscShapeScaleH[s];
		int8 ix = _dscShapeIndex[s];
		uint8 shpIx = ABS(ix);
		uint8 ovlIndex = _dscShapeOvlIndex[4 + _dscDimMap[index] * 5] + 2;
		if (ovlIndex > 7)
			ovlIndex = 7;

		if (!scaleW || !scaleH)
			continue;

		uint8 d = (_currentDirection + _dscUnk1[s]) & 3;
		int8 l = _wllShapeMap[_visibleBlocks[index]->walls[d]];

		uint8 *shapeData = 0;

		int x = 0;
		int y = 0;
		int flags = 0;

		while (l > 0) {
			if ((_levelShapeProperties[l].flags & 8) && index != 3 && index != 9 && index != 13) {
				l = _levelShapeProperties[l].next;
				continue;
			}

			if (_dscOvlMap[shpIx] == 1 && ((_levelShapeProperties[l].flags & 2) || ((_levelShapeProperties[l].flags & 4) && _wllProcessFlag)))
				ix = -ix;

			int xOffs = 0;
			int yOffs = 0;
			uint8 *ovl = 0;

			if (_levelShapeProperties[l].scaleFlag[shpIx] & 1) {
				xOffs = _levelShapeProperties[l].shapeX[shpIx];
				yOffs = _levelShapeProperties[l].shapeY[shpIx];
				shpIx = _dscOvlMap[shpIx];
				int ov = ovlIndex;
				if (_flags.use16ColorMode) {
					uint8 bb = _blockBrightness >> 4;
					if (ov > bb)
						ov -= bb;
					else
						ov = 0;
				}
				ovl = _screen->getLevelOverlay(ov);
			} else if (_levelShapeProperties[l].shapeIndex[shpIx] != 0xffff) {
				scaleW = scaleH = 0x100;
				int ov = 7;
				if (_flags.use16ColorMode) {
					uint8 bb = _blockBrightness >> 4;
					if (ov > bb)
						ov -= bb;
					else
						ov = 0;
				}
				ovl = _screen->getLevelOverlay(ov);
			}

			if (_levelShapeProperties[l].shapeIndex[shpIx] != 0xffff) {
				shapeData = _levelShapes[_levelShapeProperties[l].shapeIndex[shpIx]];
				if (shapeData) {
					if (ix < 0) {
						x = _dscShapeX[s] + xOffs + ((_levelShapeProperties[l].shapeX[shpIx] * scaleW) >> 8);
						if (ix == _dscShapeIndex[s]) {
							x = _dscShapeX[s] - ((_levelShapeProperties[l].shapeX[shpIx] * scaleW) >> 8) -
								_screen->getShapeScaledWidth(shapeData, scaleW) - xOffs;
						}
						flags = 0x105;
					} else {
						x = _dscShapeX[s] + xOffs + ((_levelShapeProperties[l].shapeX[shpIx] * scaleW) >> 8);
						flags = 0x104;
					}

					y = _dscShapeY[s] + yOffs + ((_levelShapeProperties[l].shapeY[shpIx] * scaleH) >> 8);
					_screen->drawShape(_sceneDrawPage1, shapeData, x + 112, y, 13, flags, ovl, 1, scaleW, scaleH);

					if ((_levelShapeProperties[l].flags & 1) && shpIx < 4) {
						//draw shadow
						x += (_screen->getShapeScaledWidth(shapeData, scaleW));
						flags ^= 1;
						_screen->drawShape(_sceneDrawPage1, shapeData, x + 112, y, 13, flags, ovl, 1, scaleW, scaleH);
					}
				}
			}

			l = _levelShapeProperties[l].next;
			shpIx = (_dscShapeIndex[s] < 0) ? -_dscShapeIndex[s] : _dscShapeIndex[s];
		}
	}
}

void LoLEngine::drawBlockEffects(int index, int type) {
	static const uint16 yOffs[] = { 0xff, 0xff, 0x80, 0x80 };
	uint8 flg = _visibleBlocks[index]->flags;
	// flags: 0x10 = ice wall, 0x20 = teleporter, 0x40 = blue slime spot, 0x80 = blood spot
	if (!(flg & 0xf0))
		return;

	type = (type == 0) ? 2 : 0;

	for (int i = 0; i < 2; i++, type++) {
		if (!((0x10 << type) & flg))
			continue;

		uint16 x = 0x80;
		uint16 y = yOffs[type];
		uint16 drawFlag = (type == 3) ? 0x80 : 0x20;
		uint8 *ovl = (type == 3) ? _screen->_grayOverlay : 0;

		if (_flags.use16ColorMode) {
			ovl = 0;
			drawFlag = (type == 0 || type == 3) ? 0 : 0x20;
		}

		calcCoordinatesAddDirectionOffset(x, y, _currentDirection);

		x |= ((_visibleBlockIndex[index] & 0x1f) << 8);
		y |= ((_visibleBlockIndex[index] & 0xffe0) << 3);

		drawItemOrMonster(_effectShapes[type], ovl, x, y, 0, (type == 1) ? -20 : 0, drawFlag, -1, false);
	}
}

void LoLEngine::drawSpecialGuiShape(int pageNum) {
	if (!_specialGuiShape)
		return;

	_screen->drawShape(pageNum, _specialGuiShape, _specialGuiShapeX, _specialGuiShapeY, 2, 0);

	if (_specialGuiShapeMirrorFlag & 1)
		_screen->drawShape(pageNum, _specialGuiShape, _specialGuiShapeX + _specialGuiShape[3], _specialGuiShapeY, 2, 1);
}

} // End of namespace Kyra

#endif // ENABLE_LOL

