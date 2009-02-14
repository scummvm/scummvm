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

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "common/endian.h"

namespace Kyra {

void LoLEngine::loadLevel(int index) {
	_unkFlag |= 0x800;
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
	resetLvlBuffer();
	resetBlockProperties();

	releaseMonsterShapes(0);
	releaseMonsterShapes(1);

	// TODO

	_currentLevel = index;
	_updateFlags = 0;

	restoreDefaultGui();

	loadTalkFile(index);

	loadLevelWLL(index, true);
	_loadLevelFlag = 1;

	char filename[13];
	snprintf(filename, sizeof(filename), "LEVEL%d.INI", index);

	int f = _levelFlagUnk & (1 << ((index + 0xff) & 0xff));

	runInitScript(filename, f ? 0 : 1);

	if (f)
		loadLevelCmzFile(index);

	snprintf(filename, sizeof(filename), "LEVEL%d.INF", index);
	runInfScript(filename);

	addLevelItems();
	initCmzWithScript(_currentBlock);

	_screen->generateGrayOverlay(_screen->_currentPalette, _screen->_grayOverlay,32, 16, 0, 0, 128, true);

	_sceneDefaultUpdate = 0;
	if (_screen->_fadeFlag == 3)
		_screen->fadeToBlack(10);

	gui_drawPlayField();

	_screen->setPaletteBrightness(_screen->_currentPalette, _brightness, _lampOilStatus);
	setMouseCursorToItemInHand();

	snd_playTrack(_curMusicTheme);
}

void LoLEngine::addLevelItems() {
	for (int i = 0; i < 400; i++) {
		if (_itemsInPlay[i].level != _currentLevel)
			continue;

		moveItemToBlock(&_levelBlockProperties[_itemsInPlay[i].blockPropertyIndex].itemIndex, i);

		_levelBlockProperties[_itemsInPlay[i].blockPropertyIndex].field_8 = 5;
		_itemsInPlay[i].unk2 = 0;
	}
}

int LoLEngine::initCmzWithScript(int block) {
	int i = _levelBlockProperties[block].itemIndex;
	int cnt = 0;
	CLevelItem *t = 0;

	while (i) {
		t = findItem(i);
		i = t->itemIndexUnk;
		if (!(i & 0x8000))
			continue;

		i &= 0x7fff;
		t = &_cLevelItems[i];

		cnt++;
		initCMZ1(t, 14);

		checkScriptUnk(t->blockPropertyIndex);

		initCMZ2(t, 0, 0);
	}
	return cnt;
}

void LoLEngine::initCMZ1(CLevelItem *l, int a) {
	if (l->field_14 == 13 && a != 14)
		return;
	if (a == 7) {
		l->itemPosX = _partyPosX;
		l->itemPosY = _partyPosX;
	}

	if (l->field_14 == 1 && a == 7) {
		for (int i = 0; i < 30; i++) {
			if (l->field_14 != 1)
				continue;
			l->field_14 = a;
			l->field_15 = 0;
			l->itemPosX = _partyPosX;
			l->itemPosY = _partyPosY;
			cmzS2(l, cmzS1(l->p_1a, l->p_1b, l->itemPosX, l->itemPosY));
		}
	} else {
		l->field_14 = a;
		l->field_15 = 0;
		if (a == 14)
			l->field_1D = 0;
		if (a == 13 && (l->field_19 & 0x20)) {
			l->field_14 = 0;
			cmzS3(l);
			if (_currentLevel != 29)
				initCMZ1(l, 14);
			runSceneScriptCustom(0x404, -1, l->field_16, l->field_16, 0, 0);
			checkScriptUnk(l->blockPropertyIndex);
			if (l->field_14 == 14)
				initCMZ2(l, 0, 0);
		}
	}

}

void LoLEngine::initCMZ2(CLevelItem *l, uint16 a, uint16 b) {
	bool cont = true;
	int t = l->blockPropertyIndex;
	if (l->blockPropertyIndex) {
		cmzS4(_levelBlockProperties[l->blockPropertyIndex].itemIndex, ((uint16)l->field_16) | 0x8000);
		_levelBlockProperties[l->blockPropertyIndex].field_8 = 5;
		checkScriptUnk(l->blockPropertyIndex);
	} else {
		cont = false;
	}

	l->blockPropertyIndex = cmzS5(a, b);

	if (l->p_1a != a || l->p_1b != b) {
		l->p_1a = a;
		l->p_1b = b;
		l->anon9 = (++l->anon9) & 3;
	}

	if (l->blockPropertyIndex == 0)
		return;

	cmzS6(_levelBlockProperties[l->blockPropertyIndex].itemIndex, ((uint16)l->field_16) | 0x8000);
	_levelBlockProperties[l->blockPropertyIndex].field_8 = 5;
	checkScriptUnk(l->blockPropertyIndex);

	if (l->monsters->unk8[0] == 0 || cont == false)
		return;

	if ((!(l->monsters->unk5[0] & 0x100) || ((l->anon9 & 1) == 0)) && l->blockPropertyIndex == t)
		return;

	if (l->blockPropertyIndex != t)
		runSceneScriptCustom(l->blockPropertyIndex, 0x800, -1, l->field_16, 0, 0);

	if (_updateFlags & 1)
		return;

	cmzS7(l->monsters->unk3[5], l->blockPropertyIndex);
}

int LoLEngine::cmzS1(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int16 r = 0;
	int16 t1 = y1 - y2;
	if (t1 < 0) {
		r++;
		t1 = -t1;
	}

	r <<= 1;

	int16 t2 = x2 - x1;

	if (t2 < 0) {
		r++;
		t2 = -t2;
	}

	uint8 f = 0;

	if (t2 >= t1) {
		if (t2 > t1)
			f = 1;
		SWAP(t1, t2);
	}

	r = (r << 1) | f;

	t1 = (t1 + 1) >> 1;

	f = 0;
	f = (t2 > t1) ? 1 : 0;
	r = (r << 1) | f;

	static const uint8 Retv[] = { 1, 2, 1, 0, 7, 6, 7, 0, 3, 2, 3, 4, 5, 6, 5, 4};
	return Retv[r];
}

void LoLEngine::cmzS2(CLevelItem *l, int a) {
	// TODO
}

void LoLEngine::cmzS3(CLevelItem *l) {
	// TODO
}

void LoLEngine::cmzS4(uint16 &itemIndex, int a) {
	// TODO
}

int LoLEngine::cmzS5(uint16 a, uint16 b) {
	// TODO
	return 0;
}

void LoLEngine::cmzS6(uint16 &itemIndex, int a) {
	// TODO
}

void LoLEngine::cmzS7(int a, int block) {
	if (!(_unkGameFlag & 1))
		return;

	// TODO
}

void LoLEngine::moveItemToBlock(uint16 *cmzItemIndex, uint16 item) {
	CLevelItem *tmp = 0;

	while (*cmzItemIndex & 0x8000) {
		tmp = findItem(*cmzItemIndex);
		cmzItemIndex = &tmp->itemIndexUnk;
	}

	tmp = findItem(item);
	tmp->level = -1;

	uint16 ix = *cmzItemIndex;

	if (ix == item)
		return;

	*cmzItemIndex = item;
	cmzItemIndex = &tmp->itemIndexUnk;

	while (*cmzItemIndex) {
		tmp = findItem(*cmzItemIndex);
		cmzItemIndex = &tmp->itemIndexUnk;
	}

	*cmzItemIndex = ix;
}

void LoLEngine::loadLevelWLL(int index, bool mapShapes) {
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
	uint16 *p1 = (uint16 *) _tempBuffer5120;
	uint16 *p2 = (uint16 *) (_tempBuffer5120 + 4000);

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

void LoLEngine::loadLevelCmzFile(int index) {
	//char filename[13];
	//snprintf(filename, sizeof(filename), "_LEVEL%d.TMP", index);
	// TODO ???
	memset(_tempBuffer5120, 0, 5120);
	uint16 tmpLvlVal = 0;


	char filename[13];
	snprintf(filename, sizeof(filename), "LEVEL%d.CMZ", index);

	_screen->loadBitmap(filename, 3, 3, 0);
	const uint8 *p = _screen->getCPagePtr(2);
	uint16 len = READ_LE_UINT16(p + 4);

	uint8 *cmzdata = new uint8[0x1000];

	for (int i = 0; i < 1024; i++)
		 memcpy(&cmzdata[i << 2], &p[i * len + 6], 4);

	memset(_levelBlockProperties, 0, 1024 * sizeof(LevelBlockProperty));

	uint8 *c = cmzdata;
	uint8 *t = _tempBuffer5120;

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			_levelBlockProperties[i].walls[ii] = *c++ ^ *t++;
	}

	for (int i = 0; i < 1024; i++)
		_levelBlockProperties[i].flags = *t++;

	for (int i = 0; i < 30; i++) {
		if (_cLevelItems[i].blockPropertyIndex) {
			_cLevelItems[i].blockPropertyIndex = 0;
			_cLevelItems[i].monsters = _monsterProperties + _cLevelItems[i].field_20;
			initCMZ2(&_cLevelItems[i], _cLevelItems[i].p_1a, _cLevelItems[i].p_1b);
		}
	}

	loadCMZ_Sub(tmpLvlVal, (_unkGameFlag & 0x30) >> 4);

	delete[] cmzdata;
}

void LoLEngine::loadCMZ_Sub(int index1, int index2) {
	static const int table[] = { 0x66, 0x100, 0x180, 0x100, 0x100, 0xC0, 0x140, 0x100, 0x80, 0x80, 0x100, 0x100 };
	int val = (table[index2] << 8) / table[index1];

	//int r = 0;

	for (int i = 0; i < 30; i++) {
		if (_cLevelItems[i].field_14 >= 14 || _cLevelItems[i].blockPropertyIndex == 0 || _cLevelItems[i].field_1D <= 0)
			continue;

		int t = (val * _cLevelItems[i].field_1D) >> 8;
		_cLevelItems[i].field_1D = t;
		if (index2 < index1)
			_cLevelItems[i].field_1D++;
		if (_cLevelItems[i].field_1D == 0)
			_cLevelItems[i].field_1D = 1;
	}
}

void LoLEngine::loadCmzFile(const char *file) {
	memset(_levelBlockProperties, 0, 1024 * sizeof(LevelBlockProperty));
	_screen->loadBitmap(file, 2, 2, 0);
	const uint8 *h = _screen->getCPagePtr(2);
	uint16 len = READ_LE_UINT16(&h[4]);
	const uint8 *p = h + 6;

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			_levelBlockProperties[i].walls[ii] = p[i * len + ii];

		_levelBlockProperties[i].field_8 = 5;

		if (_wllBuffer4[_levelBlockProperties[i].walls[0]] == 17) {
			_levelBlockProperties[i].flags &= 0xef;
			_levelBlockProperties[i].flags |= 0x20;
		}
	}
}

void LoLEngine::loadMonsterShapes(const char *file, int monsterIndex, int b) {
	releaseMonsterShapes(monsterIndex);
	_screen->loadBitmap(file, 3, 3, 0);

	const uint8 *p = _screen->getCPagePtr(2);
	const uint8 *ts[16];

	for (int i = 0; i < 16; i++) {
		ts[i] = _screen->getPtrToShape(p, i);

		bool replaced = false;
		int pos = monsterIndex << 4;

		for (int ii = 0; ii < i; ii++) {
			if (ts[i] != ts[ii])
				continue;

			_monsterShapes[pos + i] = _monsterShapes[pos + ii];
			replaced = true;
			break;
		}

		if (!replaced)
			_monsterShapes[pos + i] = _screen->makeShapeCopy(p, i);

		int size = _screen->getShapePaletteSize(_monsterShapes[pos + i]) << 3;
		_monsterPalettes[pos + i] = new uint8[size];
		memset(_monsterPalettes[pos + i], 0, size);
	}

	for (int i = 0; i < 4; i++) {
		for (int ii = 0; ii < 16; ii++) {
			uint8 **of = &_buf4[(monsterIndex << 7) + (i << 5) + (ii << 1)];
			int s = (i << 4) + ii + 17;
			*of = _screen->makeShapeCopy(p, s);

			////TODO
		}
	}
	_monsterUnk[monsterIndex] = b & 0xff;

	uint8 *tsh = _screen->makeShapeCopy(p, 16);

	_screen->clearPage(3);
	_screen->drawShape(2, tsh, 0, 0, 0, 0);

	uint8 *tmpPal1 = new uint8[64];
	uint8 *tmpPal2 = new uint8[256];
	uint16 *tmpPal3 = new uint16[256];
	memset (tmpPal1, 0, 64);
	memset (tmpPal2, 0, 256);
	memset (tmpPal3, 0xff, 512);

	for (int i = 0; i < 64; i++) {
		tmpPal1[i] = *p;
		p += 320;
	}

	p = _screen->getCPagePtr(2);

	for (int i = 0; i < 16; i++) {
		int pos = (monsterIndex << 4) + i;
		memcpy(tmpPal2, _monsterShapes[pos] + 10, 256);
		uint8 numCol = *tmpPal2;

		for (int ii = 0; ii < numCol; ii++) {
			uint8 *cl = (uint8*)memchr(tmpPal1, tmpPal2[1 + ii], 64);
			if (!cl)
				continue;
			tmpPal3[ii] = (uint16) (cl - tmpPal1);
		}

		for (int ii = 0; ii < 8; ii++) {
			memcpy(tmpPal2, _monsterShapes[pos] + 10, 256);
			for (int iii = 0; iii < numCol; iii++) {
				if (tmpPal3[iii] == 0xffff)
					continue;
				if (p[tmpPal3[iii] * 320 + ii + 1])
					tmpPal2[1 + iii] = p[tmpPal3[iii] * 320 + ii + 1];
			}
			memcpy(_monsterPalettes[pos] + ii * numCol, &tmpPal2[1], numCol);
		}
	}

	delete[] tmpPal1;
	delete[] tmpPal2;
	delete[] tmpPal3;
	delete[]  tsh;
}

void LoLEngine::releaseMonsterShapes(int monsterIndex) {
	for (int i = 0; i < 16; i++) {
		int pos = (monsterIndex << 4) + i;
		if (_monsterShapes[pos]) {
			delete[] _monsterShapes[pos];
			_monsterShapes[pos] = 0;
		}

		if (_monsterPalettes[pos]) {
			delete[] _monsterPalettes[pos];
			_monsterPalettes[pos] = 0;
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
			l->shapeX[ii] = s->readUint16LE();
		for (int ii = 0; ii < 10; ii++)
			l->shapeY[ii] = s->readUint16LE();
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
		strcpy(_lastSuppFile, file);
		if (palFile) {
			strcpy(_lastOverridePalFile, palFile);
			_lastOverridePalFilePtr = _lastOverridePalFile;
		} else {
			_lastOverridePalFilePtr = 0;
		}
	}

	char fname[13];
	snprintf(fname, sizeof(fname), "%s.VCN", _lastSuppFile);

	_screen->loadBitmap(fname, 3, 3, 0);
	const uint8 *v = _screen->getCPagePtr(2);
	int tlen = READ_LE_UINT16(v);
	v += 2;

	if (vcnLen == -1)
		vcnLen = tlen << 5;

	if (_vcnBlocks)
		delete[] _vcnBlocks;
	_vcnBlocks = new uint8[vcnLen];

	if (_vcnShift)
		delete[] _vcnShift;
	_vcnShift = new uint8[tlen];

	memcpy(_vcnShift, v, tlen);
	v += tlen;

	memcpy(_vcnExpTable, v, 128);
	v += 128;

	if (_lastOverridePalFilePtr) {
		uint8 *tpal = _res->fileData(_lastOverridePalFilePtr, 0);
		memcpy(_screen->_currentPalette, tpal, 384);
		delete[] tpal;
	} else {
		memcpy(_screen->_currentPalette, v, 384);
	}

	v += 384;
	/*uint8 tmpPal = new uint8[384];
	memcpy(tmpPal, _screen->_currentPalette + 384, 384);
	memset(_screen->_currentPalette + 384, 0xff, 384);
	memcpy(_screen->_currentPalette + 384, tmpPal, 384);*/

	//loadSwampIceCol();

	memcpy(_vcnBlocks, v, vcnLen);
	v += vcnLen;

	snprintf(fname, sizeof(fname), "%s.VMP", _lastSuppFile);
	_screen->loadBitmap(fname, 3, 3, 0);
	v = _screen->getCPagePtr(2);

	if (vmpLen == -1)
		vmpLen = READ_LE_UINT16(v);
	v += 2;

	if (_vmpPtr)
		delete[] _vmpPtr;
	_vmpPtr = new uint16[vmpLen];

	for (int i = 0; i < vmpLen; i++)
		_vmpPtr[i] = READ_LE_UINT16(&v[i << 1]);

	for (int i = 0; i < 7; i++) {
		weight = 100 - (i * _lastSpecialColorWeight);
		weight = (weight > 0) ? (weight * 255) / 100 : 0;
		_screen->generateLevelOverlay(_screen->_currentPalette, _screen->getLevelOverlay(i), _lastSpecialColor, weight);

		for (int ii = 0; ii < 128; ii++) {
			if (_screen->getLevelOverlay(i)[ii] == 255)
				_screen->getLevelOverlay(i)[ii] = 0;
		}

		for (int ii = 128; ii < 256; ii++)
			_screen->getLevelOverlay(i)[ii] = ii & 0xff;
	}

	for (int i = 0; i < 256; i++)
		_screen->getLevelOverlay(7)[i] = i & 0xff;

	_loadSuppFilesFlag = 0;
	_screen->generateBrightnessPalette(_screen->_currentPalette, _screen->getPalette(1), _brightness, _lampOilStatus);

	char tname[13];
	snprintf(tname, sizeof(tname), "LEVEL%.02d.TLC", _currentLevel);
	Common::SeekableReadStream *s = _res->createReadStream(tname);
	s->read(_trueLightTable1, 256);
	s->read(_trueLightTable2, 5120);
	delete s;

	_loadSuppFilesFlag = 1;
}

void LoLEngine::resetItems(int flag) {
	for (int i = 0; i < 1024; i++) {
		_levelBlockProperties[i].field_8 = 5;
		uint16 id = _levelBlockProperties[i].itemIndex;
		CLevelItem *r = 0;

		while (id & 0x8000) {
			r = (CLevelItem*)findItem(id);
			assert(r);
			id = r->itemIndexUnk;
		}

		if (!id)
			continue;

		ItemInPlay *it = &_itemsInPlay[id];
		it->level = _currentLevel;
		it->blockPropertyIndex = i;
		r->itemIndexUnk = 0;
	}

	if (flag)
		memset(_tmpData136, 0, 136);
}

void LoLEngine::resetLvlBuffer() {
	memset(_cLevelItems, 0, 30 * sizeof(CLevelItem));
	for (int i = 0; i < 30; i++)
		_cLevelItems[i].field_14 = 0x10;
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
	_screen->_drawGuiFlag |= 0x400;
	_lampOilStatus = 255;
	updateLampStatus();
}

void LoLEngine::setLampMode(bool lampOn) {
	_screen->_drawGuiFlag &= 0xFBFF;
	if (!(_screen->_drawGuiFlag & 0x800) || !lampOn)
		return;

	_screen->drawShape(0, _gameShapes[43], 291, 56, 0, 0);
	_lampOilStatus = 8;
}

void LoLEngine::updateLampStatus() {
	uint8 newLampOilStatus = 0;
	uint8 tmp2 = 0;

	if ((_updateFlags & 4) || !(_screen->_drawGuiFlag & 0x800))
		return;

	if (!_brightness || !_lampStatusUnk) {
		newLampOilStatus = 8;
		if (newLampOilStatus != _lampOilStatus && _screen->_fadeFlag == 0)
			_screen->setPaletteBrightness(_screen->_currentPalette, _lampOilStatus, newLampOilStatus);
	} else {
		tmp2 = (_lampStatusUnk < 100) ? _lampStatusUnk : 100;
		newLampOilStatus = (3 - (tmp2 - 1) / 25) << 1;

		if (_lampOilStatus == 255) {
			if (_screen->_fadeFlag == 0)
					_screen->setPaletteBrightness(_screen->_currentPalette, _brightness, newLampOilStatus);
			_lampStatusTimer = _system->getMillis() + (10 + _rnd.getRandomNumberRng(1, 30)) * _tickLength;
		} else {
			if ((_lampOilStatus & 0xfe) == (newLampOilStatus & 0xfe)) {
				if (_system->getMillis() <= _lampStatusTimer) {
					newLampOilStatus = _lampOilStatus;
				} else {
					newLampOilStatus = _lampOilStatus ^ 1;
					_lampStatusTimer = _system->getMillis() + (10 + _rnd.getRandomNumberRng(1, 30)) * _tickLength;
				}
			} else {
				if (_screen->_fadeFlag == 0)
					_screen->setPaletteBrightness(_screen->_currentPalette, _lampOilStatus, newLampOilStatus);
			}
		}
	}

	if (newLampOilStatus == _lampOilStatus)
		return;

	_screen->hideMouse();

	_screen->drawShape(_screen->_curPage, _gameShapes[35 + newLampOilStatus], 291, 56, 0, 0);
	_screen->showMouse();

	_lampOilStatus = newLampOilStatus;
}

void LoLEngine::updateCompass() {

}

void LoLEngine::moveParty(uint16 direction, int unk1, int unk2, int buttonShape) {
	if (buttonShape)
		gui_toggleButtonDisplayMode(buttonShape, 1);

	uint16 opos = _currentBlock;
	uint16 npos = calcNewBlockPostion(_currentBlock, direction);

	if (!checkBlockPassability(npos, direction)) {
		notifyBlockNotPassable(unk2 ? 0 : 1);
		gui_toggleButtonDisplayMode(buttonShape, 0);
		return;
	}

	_scriptDirection = direction;
	_currentBlock = npos;
	_sceneDefaultUpdate = 1;

	calcCoordinates(_partyPosX, _partyPosY, opos, 0x80, 0x80);
	_unkFlag &= 0xfdff;

	runSceneScript(opos, 4);
	runSceneScript(npos, 1);

	if (!(_unkFlag & 0x200)) {
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

		runSceneScript(opos, 8);
		runSceneScript(npos, 2);

		if (_levelBlockProperties[npos].walls[0] == 0x1a)
			memset(_levelBlockProperties[npos].walls, 0, 4);
	}

	setLF2(_currentBlock);
}

uint16 LoLEngine::calcNewBlockPostion(uint16 curBlock, uint16 direction) {
	static const int16 blockPosTable[] = { -32, 1, 32, -1, 1, -1, 3, 2, -1, 0, -1, 0, 1, -32, 0, 32 };
	return (curBlock + blockPosTable[direction]) & 0x3ff;
}

bool LoLEngine::checkBlockPassability(uint16 block, uint16 direction) {
	if (testWallFlag(block, direction, 1))
		return false;

	uint16 d = _levelBlockProperties[block].itemIndex;

	while (d) {
		if (d & 0x8000)
			return false;
		d = findItem(d)->itemIndexUnk;
	}

	return true;
}

void LoLEngine::notifyBlockNotPassable(int scrollFlag) {
	if (scrollFlag)
		movePartySmoothScrollBlocked(2);

	snd_dialogueSpeechUpdate(1);
	_txt->printMessage(0x8002, getLangString(0x403f));
	snd_playSoundEffect(19, 255);
}

void LoLEngine::movePartySmoothScrollBlocked(int speed) {
	if (!(_unkGameFlag & 8) || ((_unkGameFlag & 8) && _hideInventory))
		return;

	_screen->backupSceneWindow(_sceneDrawPage2 == 2 ? 2 : 6, 6);

	for (int i = 0; i < 2; i++) {
		_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);
		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(_smoothScrollTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	for (int i = 2; i; i--) {
		_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);
		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(_smoothScrollTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	if (_sceneDefaultUpdate != 2) {
		_screen->restoreSceneWindow(6, 0);
		_screen->updateScreen();
	}

	updateSceneWindow();
}

void LoLEngine::movePartySmoothScrollUp(int speed) {
	if (!(_unkGameFlag & 8) || ((_unkGameFlag & 8) && _hideInventory))
		return;

	int d = 0;

	if (_sceneDrawPage2 == 2) {
		d = smoothScrollDrawSpecialShape(6);
		gui_drawScene(6);
		_screen->copyRegionToBuffer(6, 112, 0, 176, 120, _scrollSceneBuffer);
		_screen->backupSceneWindow(2, 6);
	} else {
		d = smoothScrollDrawSpecialShape(2);
		gui_drawScene(2);
		_screen->copyRegionToBuffer(2, 112, 0, 176, 120, _scrollSceneBuffer);
		_screen->backupSceneWindow(6, 6);
	}

	for (int i = 0; i < 5; i++) {
		_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);

		//if (d)
		//	unk(_tempBuffer5120, page2);

		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(_smoothScrollTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	//if (d)
	//	unk(_tempBuffer5120, _scrollSceneBuffer);

	if (_sceneDefaultUpdate != 2) {
		_screen->copyBlockToPage(0, 112, 0, 176, 120, _scrollSceneBuffer);
		_screen->updateScreen();
	}

	updateSceneWindow();
}

void LoLEngine::movePartySmoothScrollDown(int speed) {
	if (!(_unkGameFlag & 8))
		return;

	//int d = smoothScrollDrawSpecialShape(2);
	gui_drawScene(2);
	_screen->backupSceneWindow(2, 6);

	for (int i = 4; i >= 0; i--) {
		_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);

		//if (d)
		//	unk(_tempBuffer5120, page2);

		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(_smoothScrollTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	//if (d)
	//	unk(_tempBuffer5120, _scrollSceneBuffer);

	if (_sceneDefaultUpdate != 2) {
		_screen->restoreSceneWindow(6, 0);
		_screen->updateScreen();
	}

	updateSceneWindow();
}

void LoLEngine::movePartySmoothScrollLeft(int speed) {
	if (!(_unkGameFlag & 8))
		return;

	speed <<= 1;

	gui_drawScene(_sceneDrawPage1);

	for (int i = 88, d = 88; i > 22; i -= 22, d += 22) {
		_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollHorizontalStep(_sceneDrawPage2, 66, d, i);
		_screen->copyRegion(112 + i, 0, 112, 0, d, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		fadeText();
		delayUntil(_smoothScrollTimer);
	}

	if (_sceneDefaultUpdate != 2) {
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	SWAP(_sceneDrawPage1, _sceneDrawPage2);
}

void LoLEngine::movePartySmoothScrollRight(int speed) {
	if (!(_unkGameFlag & 8))
		return;

	speed <<= 1;

	gui_drawScene(_sceneDrawPage1);

	_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
	_screen->copyRegion(112, 0, 222, 0, 66, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	fadeText();
	delayUntil(_smoothScrollTimer);

	_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollHorizontalStep(_sceneDrawPage2, 22, 0, 66);
	_screen->copyRegion(112, 0, 200, 0, 88, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	fadeText();
	delayUntil(_smoothScrollTimer);

	_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollHorizontalStep(_sceneDrawPage2, 44, 0, 22);
	_screen->copyRegion(112, 0, 178, 0, 110, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	fadeText();
	delayUntil(_smoothScrollTimer);

	if (_sceneDefaultUpdate != 2) {
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	SWAP(_sceneDrawPage1, _sceneDrawPage2);
}

void LoLEngine::movePartySmoothScrollTurnLeft(int speed) {
	if (!(_unkGameFlag & 8))
		return;

	speed <<= 1;

	//int d = smoothScrollDrawSpecialShape(_sceneDrawPage1);
	gui_drawScene(_sceneDrawPage1);
	int dp = _sceneDrawPage2 == 2 ? _sceneDrawPage2 : _sceneDrawPage1;

	_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep1(_sceneDrawPage1, _sceneDrawPage2, dp);
	//if (d)
	//	unk(_tempBuffer5120, _scrollSceneBuffer);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(_smoothScrollTimer);

	_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep2(_sceneDrawPage1, _sceneDrawPage2, dp);
	//if (d)
	//	unk(_tempBuffer5120, _scrollSceneBuffer);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(_smoothScrollTimer);

	_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep3(_sceneDrawPage1, _sceneDrawPage2, dp);
	//if (d)
	//	unk(_tempBuffer5120, _scrollSceneBuffer);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(_smoothScrollTimer);

	if (_sceneDefaultUpdate != 2) {
		drawScriptShapes(_sceneDrawPage1);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}
}

void LoLEngine::movePartySmoothScrollTurnRight(int speed) {
	if (!(_unkGameFlag & 8))
		return;

	speed <<= 1;

	//int d = smoothScrollDrawSpecialShape(_sceneDrawPage1);
	gui_drawScene(_sceneDrawPage1);
	int dp = _sceneDrawPage2 == 2 ? _sceneDrawPage2 : _sceneDrawPage1;

	_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep3(_sceneDrawPage2, _sceneDrawPage1, dp);
	//if (d)
	//	unk(_tempBuffer5120, _scrollSceneBuffer);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(_smoothScrollTimer);

	_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep2(_sceneDrawPage2, _sceneDrawPage1, dp);
	//if (d)
	//	unk(_tempBuffer5120, _scrollSceneBuffer);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(_smoothScrollTimer);

	_smoothScrollTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep1(_sceneDrawPage2, _sceneDrawPage1, dp);
	//if (d)
	//	unk(_tempBuffer5120, _scrollSceneBuffer);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(_smoothScrollTimer);

	if (_sceneDefaultUpdate != 2) {
		drawScriptShapes(_sceneDrawPage1);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}
}

int LoLEngine::smoothScrollDrawSpecialShape(int pageNum) {
	// TODO
	if(!_scriptAssignedLevelShape)
		return 0;
	return 0;
}

void LoLEngine::setLF2(int block) {
	if (!(_screen->_drawGuiFlag & 0x1000))
		return;
	_levelBlockProperties[block].flags |= 7;
	// TODO
}

void LoLEngine::drawScene(int pageNum) {
	if (pageNum && pageNum != _sceneDrawPage1) {
		SWAP(_sceneDrawPage1, _sceneDrawPage2);
		updateSceneWindow();
	}

	if (pageNum && pageNum != _sceneDrawPage1) {
		SWAP(_sceneDrawPage1, _sceneDrawPage2);
		updateSceneWindow();
	}

	generateBlockDrawingBuffer(_currentBlock, _currentDirection);
	drawVcnBlocks(_vcnBlocks, _blockDrawingBuffer, _vcnShift, _sceneDrawPage1);
	drawSceneShapes();

	if (!pageNum) {
		drawScriptShapes(_sceneDrawPage1);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		SWAP(_sceneDrawPage1, _sceneDrawPage2);
	}

	runLoopSub4(0);
	gui_drawCompass();

	_sceneUpdateRequired = false;
}

void LoLEngine::updateSceneWindow() {
	_screen->hideMouse();
	_screen->copyRegion(112, 0, 112, 0, 176, 120, 0, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
	_screen->showMouse();
}

void LoLEngine::setSequenceGui(int x, int y, int w, int h, int enableFlags) {
	gui_enableSequenceButtons(x, y, w, h, enableFlags);
	int offs = _itemInHand ? 10 : 0;
	_screen->setMouseCursor(offs, offs, getItemIconShapePtr(_itemInHand));
	setLampMode(0);
	_lampStatusSuspended = true;
}

void LoLEngine::restoreDefaultGui() {
	gui_enableDefaultPlayfieldButtons();
	if (_lampStatusSuspended)
		resetLampStatus();
	_lampStatusSuspended = false;
}

void LoLEngine::generateBlockDrawingBuffer(int block, int direction) {
	_sceneDrawVar1 = _dscBlockMap[_currentDirection];
	_sceneDrawVar2 = _dscBlockMap[_currentDirection + 4];
	_sceneDrawVar3 = _dscBlockMap[_currentDirection + 8];

	memset(_blockDrawingBuffer, 0, 660 * sizeof(uint16));

	_wllProcessFlag = ((block >> 5) + (block & 0x1f) + _currentDirection) & 1;

	if (_wllProcessFlag) // floor and ceiling
		generateBlockDrawingBufferF1(0, 15, 1, -330, 22, 15);
	else
		generateBlockDrawingBufferF0(0, 15, 1, -330, 22, 15);

	assignBlockCaps(block, direction);

	uint8 t = _curBlockCaps[0]->walls[_sceneDrawVar2];
	if (t)
		generateBlockDrawingBufferF0(-2, 3, t, 102, 3, 5);

	t = _curBlockCaps[6]->walls[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF1(21, 3, t, 102, 3, 5);

	t = _curBlockCaps[1]->walls[_sceneDrawVar2];
	uint8 t2 = _curBlockCaps[2]->walls[_sceneDrawVar1];

	if (hasWall(t) && !(_wllWallFlags[t2] & 8))
		generateBlockDrawingBufferF0(2, 3, t, 102, 3, 5);
	else if (t && (_wllWallFlags[t2] & 8))
		generateBlockDrawingBufferF0(2, 3, t2, 102, 3, 5);

	t = _curBlockCaps[5]->walls[_sceneDrawVar3];
	t2 = _curBlockCaps[4]->walls[_sceneDrawVar1];

	if (hasWall(t) && !(_wllWallFlags[t2] & 8))
		generateBlockDrawingBufferF1(17, 3, t, 102, 3, 5);
	else if (t && (_wllWallFlags[t2] & 8))
		generateBlockDrawingBufferF1(17, 3, t2, 102, 3, 5);

	t = _curBlockCaps[2]->walls[_sceneDrawVar2];
	if (t)
		generateBlockDrawingBufferF0(8, 3, t, 97, 1, 5);

	t = _curBlockCaps[4]->walls[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF1(13, 3, t, 97, 1, 5);

	t = _curBlockCaps[1]->walls[_sceneDrawVar1];
	if (hasWall(t))
		generateBlockDrawingBufferF0(-4, 3, t, 129, 6, 5);

	t = _curBlockCaps[5]->walls[_sceneDrawVar1];
	if (hasWall(t))
		generateBlockDrawingBufferF0(20, 3, t, 129, 6, 5);

	t = _curBlockCaps[2]->walls[_sceneDrawVar1];
	if (hasWall(t))
		generateBlockDrawingBufferF0(2, 3, t, 129, 6, 5);

	t = _curBlockCaps[4]->walls[_sceneDrawVar1];
	if (hasWall(t))
		generateBlockDrawingBufferF0(14, 3, t, 129, 6, 5);

	t = _curBlockCaps[3]->walls[_sceneDrawVar1];
	if (t)
		generateBlockDrawingBufferF0(8, 3, t, 129, 6, 5);

	t = _curBlockCaps[7]->walls[_sceneDrawVar2];
	if (t)
		generateBlockDrawingBufferF0(0, 3, t, 117, 2, 6);

	t = _curBlockCaps[11]->walls[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF1(20, 3, t, 117, 2, 6);

	t = _curBlockCaps[8]->walls[_sceneDrawVar2];
	if (t)
		generateBlockDrawingBufferF0(6, 2, t, 81, 2, 8);

	t = _curBlockCaps[10]->walls[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF1(14, 2, t, 81, 2, 8);

	t = _curBlockCaps[8]->walls[_sceneDrawVar1];
	if (hasWall(t))
		generateBlockDrawingBufferF0(-4, 2, t, 159, 10, 8);

	t = _curBlockCaps[10]->walls[_sceneDrawVar1];
	if (hasWall(t))
		generateBlockDrawingBufferF0(16, 2, t, 159, 10, 8);

	t = _curBlockCaps[9]->walls[_sceneDrawVar1];
	if (t)
		generateBlockDrawingBufferF0(6, 2, t, 159, 10, 8);

	t = _curBlockCaps[12]->walls[_sceneDrawVar2];
	if (t)
		generateBlockDrawingBufferF0(3, 1, t, 45, 3, 12);

	t = _curBlockCaps[14]->walls[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF1(16, 1, t, 45, 3, 12);

	t = _curBlockCaps[12]->walls[_sceneDrawVar1];
	if (!(_wllWallFlags[t] & 8))
		generateBlockDrawingBufferF0(-13, 1, t, 239, 16, 12);

	t = _curBlockCaps[14]->walls[_sceneDrawVar1];
	if (!(_wllWallFlags[t] & 8))
		generateBlockDrawingBufferF0(19, 1, t, 239, 16, 12);

	t = _curBlockCaps[13]->walls[_sceneDrawVar1];
	if (t)
		generateBlockDrawingBufferF0(3, 1, t, 239, 16, 12);

	t = _curBlockCaps[15]->walls[_sceneDrawVar2];
	t2 = _curBlockCaps[17]->walls[_sceneDrawVar3];
	if (t)	// wall to the immediate left
		generateBlockDrawingBufferF0(0, 0, t, 0, 3, 15);
	if (t2) // wall to the immediate right
		generateBlockDrawingBufferF1(19, 0, t2, 0, 3, 15);
}

void LoLEngine::generateBlockDrawingBufferF0(int16 wllOffset, uint8 wllIndex, uint8 wllVmpIndex, int16 vmpOffset, uint8 len, uint8 numEntries) {
	if (!_wllVmpMap[wllVmpIndex])
		return;

	uint16 *vmp = &_vmpPtr[(_wllVmpMap[wllVmpIndex] - 1) * 431 + vmpOffset + 330];

	for (int i = 0; i < numEntries; i++) {
		uint16 *bl = &_blockDrawingBuffer[(wllIndex + i) * 22 + wllOffset];
		for (int ii = 0; ii < len; ii++) {
			if ((wllOffset + ii >= 0) && (wllOffset + ii < 22) && *vmp)
				*bl = *vmp;
			bl++;
			vmp++;
		}
	}
}

void LoLEngine::generateBlockDrawingBufferF1(int16 wllOffset, uint8 wllIndex, uint8 wllVmpIndex, int16 vmpOffset, uint8 len, uint8 numEntries) {
	if (!_wllVmpMap[wllVmpIndex])
		return;

	uint16 *vmp = &_vmpPtr[(_wllVmpMap[wllVmpIndex] - 1) * 431 + vmpOffset + 330];

	for (int i = 0; i < numEntries; i++) {
		for (int ii = 0; ii < len; ii++) {
			if ((wllOffset + ii) < 0 || (wllOffset + ii) > 21)
				continue;

			uint16 v = vmp[i * len + (len - 1 - ii)];
			if (!v)
				continue;

			if (v & 0x4000)
				v -= 0x4000;
			else
				v |= 0x4000;

			_blockDrawingBuffer[(wllIndex + i) * 22 + wllOffset + ii] = v;
		}
	}
}

bool LoLEngine::hasWall(int index) {
	if (!index || (_wllWallFlags[index] & 8))
		return false;
	return true;
}

void LoLEngine::assignBlockCaps(int block, int direction) {
	for (int i = 0; i < 18; i++) {
		uint16 t = (block + _dscBlockIndex[direction * 18 + i]) & 0x3ff;
		_currentBlockPropertyIndex[i] = t;

		_curBlockCaps[i] = &_levelBlockProperties[t];
		_lvlShapeLeftRight[i] = _lvlShapeLeftRight[18 + i] = -1;
	}
}

void LoLEngine::drawVcnBlocks(uint8 *vcnBlocks, uint16 *blockDrawingBuffer, uint8 *vcnShift, int pageNum) {
	uint8 *d = _sceneWindowBuffer;

	for (int y = 0; y < 15; y++) {
		for (int x = 0; x < 22; x++) {
			bool flag = false;
			int remainder = 0;

			uint16 vcnOffset = *blockDrawingBuffer++;

			if (vcnOffset & 0x8000) {
				remainder = vcnOffset - 0x8000;
				vcnOffset = 0;
			}

			if (vcnOffset & 0x4000) {
				flag = true;
				vcnOffset &= 0x3fff;
			}

			if (!vcnOffset) {
				vcnOffset = blockDrawingBuffer[329];
				if (vcnOffset & 0x4000) {
					flag = true;
					vcnOffset &= 0x3fff;
				}
			}

			uint8 shift = vcnShift[vcnOffset];
			uint8 *src = &vcnBlocks[vcnOffset << 5];

			if (flag) {
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
				flag = false;

				if (remainder & 0x4000) {
					remainder &= 0x3fff;
					flag = true;
				}

				shift = vcnShift[remainder];
				src = &vcnBlocks[remainder << 5];

				if (flag) {
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

	_screen->copyBlockToPage(pageNum, 112, 0, 176, 120, _sceneWindowBuffer);
}

void LoLEngine::drawSceneShapes() {
	for (int i = 0; i < 18; i++) {
		uint8 t = _dscTileIndex[i];
		uint8 s = _curBlockCaps[t]->walls[_sceneDrawVar1];

		int16 x1 = 0;
		int16 x2 = 0;

		int16 dimY1 = 0;
		int16 dimY2 = 0;

		setLevelShapesDim(t, x1, x2, 13);

		if (x2 <= x1)
			continue;

		drawDecorations(t);

		uint16 w = _wllWallFlags[s];

		if (i == 16)
			w |= 0x80;

		drawIceShapes(t, 0);

		if (_curBlockCaps[t]->itemIndex && (w & 0x80))
			drawMonstersAndItems(t);

		drawIceShapes(t, 1);

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
			uint8 d = _curBlockCaps[i]->walls[_sceneDrawVar1];
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
		int8 l = _wllShapeMap[_curBlockCaps[index]->walls[d]];

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
				ovl = _screen->getLevelOverlay(ovlIndex);
			} else if (_levelShapeProperties[l].shapeIndex[shpIx] != 0xffff) {
				scaleW = scaleH = 0x100;
				ovl = _screen->getLevelOverlay(7);
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

void LoLEngine::drawIceShapes(int index, int iceShapeIndex) {
	uint8 f = _curBlockCaps[index]->flags;
	if (!(f & 0xf0))
		return;
}

void LoLEngine::drawMonstersAndItems(int index) {


}

void LoLEngine::drawDoor(uint8 *shape, uint8 *table, int index, int unk2, int w, int h, int flags) {
	uint8 c = _dscDoor1[(_currentDirection << 5) + unk2];
	int r = (c / 5) + 5 * _dscDimMap[index];
	uint16 d = _dscShapeOvlIndex[r];
	uint16 t = (index << 5) + c;

	_shpDmY = _dscDoorMonsterY[t] + 120;

	if (flags & 1) {
		// TODO
	}

	int u = 0;

	if (flags & 2) {
		uint8 dimW = _dscDimMap[index];
		_dmScaleW = _dscDoorMonsterScaleTable[dimW << 1];
		_dmScaleH = _dscDoorMonsterScaleTable[(dimW << 1) + 1];
		u = _dscDoor4[dimW];
	}

	d += 2;

	if (!_dmScaleW || !_dmScaleH)
		return;

	int s = _screen->getShapeScaledHeight(shape, _dmScaleH) >> 1;

	if (w)
		w = (w * _dmScaleW) >> 8;

	if (h)
		h = (h * _dmScaleH) >> 8;

	_shpDmX = _dscDoorMonsterX[t] + w + 200;
	_shpDmY = _shpDmY + 4 - s + h - u;

	if (d > 7)
		d = 7;

	uint8 *ovl = _screen->getLevelOverlay(d);
	int doorScaledWitdh = _screen->getShapeScaledWidth(shape, _dmScaleW);

	_shpDmX -= (doorScaledWitdh >> 1);
	_shpDmY -= s;

	drawDoorOrMonsterShape(shape, table, _shpDmX, _shpDmY, flags, ovl);
}

void LoLEngine::drawDoorOrMonsterShape(uint8 *shape, uint8 *table, int x, int y, int flags, const uint8 *ovl) {
	int flg = 0;

	if (flags & 0x10)
		flg |= 1;

	if (flags & 0x20)
		flg |= 0x1000;

	if (flags & 0x40)
		flg |= 2;

	if (flg & 0x1000) {
		if (table)
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x9104, table, ovl, 1, _trueLightTable1, _trueLightTable2, _dmScaleW, _dmScaleH);
		else
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x1104, ovl, 1, _trueLightTable1, _trueLightTable2, _dmScaleW, _dmScaleH);
	} else {
		if (table)
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x8104, table, ovl, 1, _dmScaleW, _dmScaleH);
		else
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x104, ovl, 1, _dmScaleW, _dmScaleH);
	}
}

void LoLEngine::drawScriptShapes(int pageNum) {
	if (!_scriptAssignedLevelShape)
		return;
}

} // end of namespace Kyra

