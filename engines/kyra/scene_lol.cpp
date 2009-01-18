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

#include "common/endian.h"

namespace Kyra {

void LoLEngine::loadLevel(int index) {
	_unkFlag |= 0x800;
	setMouseCursorToIcon(0x85);
	_scriptFuncIndex = 0;

	// TODO

	updatePortraits();

	for (int i = 0; i < 400; i++) {
		delete [] _levelShapes[i];
		_levelShapes[i] = 0;
	}

	_emc->unload(&_scriptData);

	_currentLevel = index;
	_charFlagUnk = 0;

	releaseMonsterShapes(0);
	releaseMonsterShapes(1);

	//TODO

	//loadTalkFile(index);
	loadLevelWLL(index, true);
	_loadLevelFlag = 1;

	char filename[16];
	sprintf(filename, "level%d.ini", index);
	
	int f = _levelFlagUnk & (1 << ((index + 0xff) & 0xff));

	runInitScript(filename, f ? 0 : 1);

	if (f)
		loadLevelCMZ(index);

	sprintf(filename, "level%d.inf", index);
	runInfScript(filename);

	addLevelItems();
	initCmzWithScript(_currentBlock);

	_screen->generateGrayOverlay(_screen->_currentPalette, _screen->_grayOverlay,32, 16, 0, 0, 128, true);

	_loadLevelFlag2 = false;
	if (_screen->_fadeFlag == 3)
		_screen->fadeToBlack(10);

	gui_drawPlayField();

	_screen->setPaletteBrightness(_screen->_currentPalette, _brightness, _lampOilStatus);
	setMouseCursorToItemInHand();
	//TODO
}

void LoLEngine::addLevelItems() {
	for (int i = 0; i < 400; i++) {
		if (_itemsInPlay[i].level != _currentLevel)
			continue;

		moveItemToCMZ(&_cmzBuffer[_itemsInPlay[i].cmzIndex].itemIndex, i);

		_cmzBuffer[_itemsInPlay[i].cmzIndex].field_8 = 5;
		_itemsInPlay[i].unk2 = 0;
	}
}

int LoLEngine::initCmzWithScript(int block) {
	int i = _cmzBuffer[block].itemIndex;
	int cnt = 0;

	while (i) {
		void *t = cmzGetItemOffset(i);
		i = (i & 0x8000) ? ((LVL*)t)->field_0 : ((ItemInPlay*)t)->itemIndexUnk;
		if (!(i & 0x8000))
			continue;

		i &= 0x7fff;
		LVL *l = &_lvlBuffer[i];

		cnt++;
		initCMZ1(l, 14);

		checkScriptUnk(l->cmzIndex);

		initCMZ2(l, 0, 0);
	}
	return cnt;
}

void LoLEngine::initCMZ1(LVL *l, int a) {
	if (l->field_14 == 13 && a != 14)
		return;
	if (a == 7) {
		l->p_2a = _unkCmzU1;
		l->p_2b = _unkCmzU2;
	}

	if (l->field_14 == 1 && a == 7) {
		for (int i = 0; i < 30; i++) {
			if (l->field_14 != 1)
				continue;
			l->field_14 = a;
			l->field_15 = 0;
			l->p_2a = _unkCmzU1;
			l->p_2b = _unkCmzU2;
			cmzS2(l, cmzS1(l->p_1a, l->p_1b, l->p_2a, l->p_2b));
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
			runResidentScriptCustom(0x404, -1, l->field_16, l->field_16, 0, 0);
			checkScriptUnk(l->cmzIndex);
			if (l->field_14 == 14)
				initCMZ2(l, 0, 0);
		}
	}

}

void LoLEngine::initCMZ2(LVL *l, uint16 a, uint16 b) {
	bool cont = true;
	int t = l->cmzIndex;
	if (l->cmzIndex) {
		cmzS4(_cmzBuffer[l->cmzIndex].itemIndex, ((uint16)l->field_16) | 0x8000);
		_cmzBuffer[l->cmzIndex].field_8 = 5;
		checkScriptUnk(l->cmzIndex);
	} else {
		cont = false;
	}
	
	l->cmzIndex = cmzS5(a, b);
	
	if (l->p_1a != a || l->p_1b != b) {
		l->p_1a = a;
		l->p_1b = b;
		l->field_13 = (++l->field_13) & 3;
	}

	if (l->cmzIndex == 0)
		return;

	cmzS6(_cmzBuffer[l->cmzIndex].itemIndex, ((uint16)l->field_16) | 0x8000);
	_cmzBuffer[l->cmzIndex].field_8 = 5;
	checkScriptUnk(l->cmzIndex);
	uint8 *v = l->offs_lvl415;
	
	if (v[80] == 0 || cont == false)
		return;

	if ((!(READ_LE_UINT16(&v[62]) & 0x100) || ((l->field_13 & 1) == 0)) && l->cmzIndex == t)
		return;

	if (l->cmzIndex != t)
		runResidentScriptCustom(l->cmzIndex, 0x800, -1, l->field_16, 0, 0);

	if (_charFlagUnk & 1)
		return;

	cmzS7(l->offs_lvl415[50], l->cmzIndex);
}

int LoLEngine::cmzS1(uint16 a, uint16 b, uint16 c, uint16 d) {
	// TODO

	return 0;
}

void LoLEngine::cmzS2(LVL *l, int a) {
	// TODO
}

void LoLEngine::cmzS3(LVL *l) {
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

void LoLEngine::cmzS7(int itemIndex, int a) {
	if (!(_unkGameFlag & 1))
		return;

	// TODO
}

void LoLEngine::moveItemToCMZ(uint16 *cmzItemIndex, uint16 item) {
	uint16 *tmp = 0;
	while (*cmzItemIndex & 0x8000) {
		tmp = (uint16*) cmzGetItemOffset(*cmzItemIndex);
		cmzItemIndex = tmp;
	}
	uint16 *t = (uint16*) cmzGetItemOffset(*cmzItemIndex);

	((ItemInPlay*)t)->level = -1;
	uint16 ix = *cmzItemIndex;

	if (ix == item)
		return;

	*cmzItemIndex = item;
	cmzItemIndex = t;

	while (*cmzItemIndex)
		cmzItemIndex = (uint16*) cmzGetItemOffset(*cmzItemIndex);

	*cmzItemIndex = ix;
}

void LoLEngine::loadLevelWLL(int index, bool mapShapes) {
	char filename[16];
	sprintf(filename, "level%2d.wll", index);

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
		_wllBuffer5[c] = *d;
		d += 2;
		_wllBuffer4[c] = *d;
		d += 2;		
	}

	delete []file;

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

	uint32 offs = _lvlShpHeader[shapeIndex] + 2;
	_lvlShpFileHandle->seek(offs, 0);

	uint8 tmp[16];
	_lvlShpFileHandle->read(tmp, 16);
	uint16 size = _screen->getShapeSize(tmp);

	_lvlShpFileHandle->seek(offs, 0);
	uint8 *res = new uint8[size];
	_lvlShpFileHandle->read(res, size);
	
	return res;
}

void LoLEngine::loadLevelCMZ(int index) {
	//char filename[16];
	//sprintf(filename, "_LEVEL%d.TMP", index);
	// TODO ???
	memset(_tempBuffer5120, 0, 5120);
	uint16 tmpLvlVal = 0;


	char filename[16];
	sprintf(filename, "level%d.cmz", index);
	
	_screen->loadBitmap(filename, 3, 3, 0);
	const uint8 *p = _screen->getCPagePtr(2);
	uint16 len = READ_LE_UINT16(p + 4);

	uint8 *cmzdata = new uint8[0x1000];

	for (int i = 0; i < 1024; i++)
		 memcpy(&cmzdata[i << 2], &p[i * len + 6], 4);

	memset(_cmzBuffer, 0, 1024 * sizeof(CMZ));

	uint8 *c = cmzdata;
	uint8 *t = _tempBuffer5120;

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			_cmzBuffer[i].unk[ii] = *c++ ^ *t++;
	}

	for (int i = 0; i < 1024; i++)
		_cmzBuffer[i].flags = *t++;

	for (int i = 0; i < 30; i++) {
		if (_lvlBuffer[i].cmzIndex) {
			_lvlBuffer[i].cmzIndex = 0;
			_lvlBuffer[i].offs_lvl415 = _lvl415 + _lvlBuffer[i].field_20;
			initCMZ2(&_lvlBuffer[i], _lvlBuffer[i].p_1a, _lvlBuffer[i].p_1b);
		}
	}
		
	loadCMZ_Sub(tmpLvlVal, (_unkGameFlag & 0x30) >> 4);

	delete []cmzdata;
}

void LoLEngine::loadCMZ_Sub(int index1, int index2) {
	static const int table[] = { 0x66, 0x100, 0x180, 0x100, 0x100, 0xC0, 0x140, 0x100, 0x80, 0x80, 0x100, 0x100 };
	int val = (table[index2] << 8) / table[index1];

	//int r = 0;
	
	for (int i = 0; i < 30; i++) {
		if (_lvlBuffer[i].field_14 >= 14 || _lvlBuffer[i].cmzIndex == 0 || _lvlBuffer[i].field_1D <= 0)
			continue;

		int t = (val * _lvlBuffer[i].field_1D) >> 8;
		_lvlBuffer[i].field_1D = t;
		if (index2 < index1)
			_lvlBuffer[i].field_1D++;
		if (_lvlBuffer[i].field_1D == 0)
			_lvlBuffer[i].field_1D = 1;
	}
}

void LoLEngine::loadCmzFile(const char *file) {
	memset(_cmzBuffer, 0, 1024 * sizeof(CMZ));
	_screen->loadBitmap(file, 2, 2, 0);
	const uint8 *h = _screen->getCPagePtr(2);
	uint16 len = READ_LE_UINT16(&h[4]);
	const uint8 *p = h + 6;
	
	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			_cmzBuffer[i].unk[ii] = p[i * len + ii];

		_cmzBuffer[i].field_8 = 5;

		if (_wllBuffer4[_cmzBuffer[i].unk[0]] == 17) {
			_cmzBuffer[i].flags &= 0xef;
			_cmzBuffer[i].flags |= 0x20;
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

	/*for (int i = 0; i < 4; i++) {
		for (int ii = 0; ii < 16; ii++) {
			uint8 **of = &_buf4[(monsterIndex << 7) + (i << 5) + (ii << 1)];
			int s = (i << 4) + ii + 17;
			*of = _screen->makeShapeCopy(p, s);
			////TODO
		}
	}*/
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

	delete []tmpPal1;
	delete []tmpPal2;
	delete []tmpPal3;
	delete [] tsh;
}

void LoLEngine::releaseMonsterShapes(int monsterIndex) {
	for (int i = 0; i < 16; i++) {
		int pos = (monsterIndex << 4) + i;
		if (_monsterShapes[pos]) {
			delete []_monsterShapes[pos];
			_monsterShapes[pos] = 0;
		}

		if (_monsterPalettes[pos]) {
			delete []_monsterPalettes[pos];
			_monsterPalettes[pos] = 0;
		}		
	}
}

void LoLEngine::loadLevelShpDat(const char *shpFile, const char *datFile, bool flag) {
	memset(_tempBuffer5120, 0, 5120);

	_lvlShpFileHandle = _res->getFileStream(shpFile);
	_lvlShpNum = _lvlShpFileHandle->readUint16LE();
	delete []_lvlShpHeader;
	_lvlShpHeader = new uint32[_lvlShpNum];
	for (int i = 0; i < _lvlShpNum; i++)
		_lvlShpHeader[i] = _lvlShpFileHandle->readUint32LE();

	Common::SeekableReadStream *s = _res->getFileStream(datFile);

	_levelFileDataSize = s->readUint16LE();
	delete []_levelFileData;
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

	delete []s;

	if (!flag) {
		_lvlBlockIndex = 1;
		_lvlShapeIndex = 1;
	}
}

void LoLEngine::loadLevelSupplemenaryFiles(const char *file, int specialColor, int weight, int vcnLen, int vmpLen, const char *langFile) {
	if (file) {
		_lastSpecialColor = specialColor;
		_lastSpecialColorWeight = weight;
		strcpy(_lastSuppFile, file);
		if (langFile) {
			strcpy(_lastSuppLangFile, langFile);
			_lastSuppLangFilePtr = _lastSuppLangFile;
		} else {
			_lastSuppLangFilePtr = 0;
		}
	}
	
	char fname[] = "            ";
	sprintf(fname, "%s.%s", _lastSuppFile, "VCN");

	_screen->loadBitmap(fname, 3, 3, 0);
	const uint8 *v = _screen->getCPagePtr(2);
	int tlen = READ_LE_UINT16(v);
	v += 2;

	if (vcnLen == -1)
		vcnLen = tlen << 5;

	if (_vcnBlocks)
		delete []_vcnBlocks;
	_vcnBlocks = new uint8[vcnLen];

	if (_vcnShift)
		delete []_vcnShift;
	_vcnShift = new uint8[tlen];

	memcpy(_vcnShift, v, tlen);
	v += tlen;
	
	memcpy(_vcnExpTable, v, 128);
	v += 128;
	
	if (_lastSuppLangFilePtr) {
		if (_levelLangFile)
			delete []_levelLangFile;
		_levelLangFile = _res->fileData(_lastSuppLangFilePtr, 0);
	}

	memcpy(_screen->_currentPalette, v, 384);
	v += 384;
	/*uint8 tmpPal = new uint8[384];
	memcpy(tmpPal, _screen->_currentPalette + 384, 384);
	memset(_screen->_currentPalette + 384, 0xff, 384);
	memcpy(_screen->_currentPalette + 384, tmpPal, 384);*/
	
	//loadSwampIceCol();

	memcpy(_vcnBlocks, v, vcnLen);
	v += vcnLen;

	sprintf(fname, "%s.%s", _lastSuppFile, "VMP");
	_screen->loadBitmap(fname, 3, 3, 0);
	v = _screen->getCPagePtr(2);

	if (vmpLen == -1)
		vmpLen = READ_LE_UINT16(v);
	v += 2;

	if (_vmpPtr)
		delete []_vmpPtr;
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

	char tname[16];
	sprintf(tname, "LEVEL%2d.TLC", _currentLevel);
	Common::SeekableReadStream *s = _res->getFileStream(tname);
	s->read(_tlcTable1, 256);
	s->read(_tlcTable2, 5120);
	delete []s;

	_loadSuppFilesFlag = 1;
}

void LoLEngine::turnOnLamp() {
	_screen->_drawGuiFlag |= 0x400;
	_lampOilStatus = 255;
	updateLampStatus();
}

void LoLEngine::updateLampStatus() {
	uint8 newLampOilStatus = 0;
	uint8 tmp2 = 0;

	if ((_charFlagUnk & 4) || !(_screen->_drawGuiFlag & 0x800))
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

void LoLEngine::setLF1(uint16 & a, uint16 & b, int block, uint16 d, uint16 e) {
	a = block & 0x1f;
	a = ((a >> 8) | ((a & 0xff) << 8)) | d;
	b = ((block & 0xffe0) << 3) | e;
}

void LoLEngine::setLF2(int block) {
	if (!(_screen->_drawGuiFlag & 0x1000))
		return;		
	_cmzBuffer[block].flags |= 7;
	// TODO
}

void LoLEngine::drawScene(int pageNum) {
	if (pageNum && pageNum != _sceneDrawPage1) {
		_sceneDrawPage1 ^= _sceneDrawPage2;
		_sceneDrawPage2 ^= _sceneDrawPage1;
		_sceneDrawPage1 ^= _sceneDrawPage2;
		updateSceneWindow();
	}

	if (pageNum && pageNum != _sceneDrawPage1) {
		_sceneDrawPage1 ^= _sceneDrawPage2;
		_sceneDrawPage2 ^= _sceneDrawPage1;
		_sceneDrawPage1 ^= _sceneDrawPage2;
		updateSceneWindow();
	}

	generateBlockDrawingBuffer(_currentBlock, _unkPara2);
	drawVcnBlocks(_vcnBlocks, _blockDrawingBuffer, _vcnShift, _sceneDrawPage1);
	drawSceneShapes();

	if (pageNum) {
		drawScriptShapes(_sceneDrawPage1);
		_screen->copyRegion(112, 112, 0, 0, 176, 120, _sceneDrawPage1, _sceneDrawPage2);
		_screen->copyRegion(112, 112, 0, 0, 176, 120, _sceneDrawPage1, 0);
		_sceneDrawPage1 ^= _sceneDrawPage2;
		_sceneDrawPage2 ^= _sceneDrawPage1;
		_sceneDrawPage1 ^= _sceneDrawPage2;
	}

	gui_drawCompass();

	_boolScriptFuncDone = false;
}

void LoLEngine::updateSceneWindow() {
	_screen->hideMouse();
	_screen->copyRegion(112, 0, 112, 0, 176, 120, 0, _sceneDrawPage2);
	_screen->showMouse();
}

void LoLEngine::generateBlockDrawingBuffer(int block, int b) {
	_sceneDrawVar1 = _dscBlockMap[_unkPara2];
	_sceneDrawVar2 = _dscBlockMap[_unkPara2 + 4];
	_sceneDrawVar3 = _dscBlockMap[_unkPara2 + 8];

	memset(_blockDrawingBuffer, 0, 660 * sizeof(uint16));

	_wllProcessFlag = ((block >> 5) + (block & 0x1f) + _unkPara2) & 1;

	if (_wllProcessFlag)
		generateBlockDrawingBufferF1(0, 15, 1, -330, 22, 15);
	else
		generateBlockDrawingBufferF0(0, 15, 1, -330, 22, 15);

	assignBlockCaps(block, b);

	uint8 t = _curBlockCaps[0]->unk[_sceneDrawVar2];
	if (t)
		generateBlockDrawingBufferF0(-2, 3, t, 102, 3, 5);

	t = _curBlockCaps[6]->unk[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF1(21, 3, t, 102, 3, 5);

	t = _curBlockCaps[1]->unk[_sceneDrawVar2];
	uint8 t2 = _curBlockCaps[2]->unk[_sceneDrawVar1];

	if (testWllBuffer5Value(t) && !(_wllBuffer5[t2] & 8))
		generateBlockDrawingBufferF0(2, 3, t, 102, 3, 5);
	else if (t && (_wllBuffer5[t2] & 8))
		generateBlockDrawingBufferF0(2, 3, t2, 102, 3, 5);

	t = _curBlockCaps[5]->unk[_sceneDrawVar3];
	t2 = _curBlockCaps[4]->unk[_sceneDrawVar1];

	if (testWllBuffer5Value(t) && !(_wllBuffer5[t2] & 8))
		generateBlockDrawingBufferF1(17, 3, t, 102, 3, 5);
	else if(t && (_wllBuffer5[t2] & 8))
		generateBlockDrawingBufferF1(17, 3, t2, 102, 3, 5);

	t = _curBlockCaps[2]->unk[_sceneDrawVar2];
	if (t)
		generateBlockDrawingBufferF0(8, 3, t, 97, 1, 5);

	t = _curBlockCaps[4]->unk[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF1(13, 3, t, 97, 1, 5);

	t = _curBlockCaps[1]->unk[_sceneDrawVar1];
	if (testWllBuffer5Value(t))
		generateBlockDrawingBufferF0(-4, 3, t, 129, 6, 5);

	t = _curBlockCaps[5]->unk[_sceneDrawVar1];
	if (testWllBuffer5Value(t))
		generateBlockDrawingBufferF0(20, 3, t, 129, 6, 5);

	t = _curBlockCaps[2]->unk[_sceneDrawVar1];
	if (testWllBuffer5Value(t))
		generateBlockDrawingBufferF0(2, 3, t, 129, 6, 5);

	t = _curBlockCaps[4]->unk[_sceneDrawVar1];
	if (testWllBuffer5Value(t))
		generateBlockDrawingBufferF0(14, 3, t, 129, 6, 5);

	t = _curBlockCaps[3]->unk[_sceneDrawVar1];
	if (t)
		generateBlockDrawingBufferF0(8, 3, t, 129, 6, 5);

	t = _curBlockCaps[7]->unk[_sceneDrawVar2];
	if (t)
		generateBlockDrawingBufferF0(0, 3, t, 117, 2, 6);

	t = _curBlockCaps[11]->unk[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF1(20, 3, t, 117, 2, 6);

	t = _curBlockCaps[8]->unk[_sceneDrawVar2];
	if (t)
		generateBlockDrawingBufferF0(6, 2, t, 81, 2, 8);

	t = _curBlockCaps[10]->unk[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF1(14, 2, t, 81, 2, 8);

	t = _curBlockCaps[8]->unk[_sceneDrawVar1];
	if (testWllBuffer5Value(t))
		generateBlockDrawingBufferF0(-4, 2, t, 159, 10, 8);

	t = _curBlockCaps[10]->unk[_sceneDrawVar1];
	if (testWllBuffer5Value(t))
		generateBlockDrawingBufferF0(16, 2, t, 159, 10, 8);

	t = _curBlockCaps[9]->unk[_sceneDrawVar1];
	if (t)
		generateBlockDrawingBufferF0(6, 2, t, 159, 10, 8);

	t = _curBlockCaps[12]->unk[_sceneDrawVar2];
	if (t)
		generateBlockDrawingBufferF0(3, 1, t, 45, 3, 12);

	t = _curBlockCaps[14]->unk[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF1(16, 1, t, 45, 3, 12);

	t = _curBlockCaps[12]->unk[_sceneDrawVar1];
	if (!(_wllBuffer5[t] & 8))
		generateBlockDrawingBufferF0(-13, 1, t, 239, 16, 12);

	t = _curBlockCaps[14]->unk[_sceneDrawVar1];
	if (!(_wllBuffer5[t] & 8))
		generateBlockDrawingBufferF0(19, 1, t, 239, 16, 12);

	t = _curBlockCaps[13]->unk[_sceneDrawVar1];
	if (t)
		generateBlockDrawingBufferF0(3, 1, t, 239, 16, 12);

	t = _curBlockCaps[15]->unk[_sceneDrawVar2];
	t2 = _curBlockCaps[17]->unk[_sceneDrawVar3];
	if (t)
		generateBlockDrawingBufferF0(0, 0, t, 0, 3, 15);
	if (t2)
		generateBlockDrawingBufferF1(19, 0, t, 0, 3, 15);
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
			if ((wllOffset + ii >= 0) && (wllOffset + ii < 22)) {
				uint16 t = vmp[len * i + len - 1 - ii];
				if (t) {				
					if (t & 04000)
						t -= 0x4000;
					else
						t |= 0x4000;

					_blockDrawingBuffer[(wllIndex + i) * 22 + wllOffset + ii] = t;
				}
			}
		}
	}
}

bool LoLEngine::testWllBuffer5Value(int index) {
	if (!index || (_wllBuffer5[index] & 8))
		return false;
	return true;
}

void LoLEngine::assignBlockCaps(int a, int b) {
	for (int i = 0; i < 18; i++) {
		uint16 t = (a + _dscBlockIndex[b * 18 + i]) & 0x3ff;
		_scriptExecutedFuncs[i] = t;

		_curBlockCaps[i] = &_cmzBuffer[t];
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
		uint8 s = _curBlockCaps[t]->unk[_sceneDrawVar1];

		int16 x1 = 0;
		int16 x2 = 0;

		int16 dimY1 = 0;
		int16 dimY2 = 0;

		setLevelShapesDim(t, x1, x2, 13);

		if (x2 <= x1)
			continue;

		drawDecorations(t);

		uint16 w = _wllBuffer5[s];

		if (i == 16)
			w |= 0x80;

		drawIceShapes(t, 0);

		//if (_curBlockCaps[t]->itemIndex && (w & 0x80))
			//sub_3AA55(t);

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
			uint8 d = _curBlockCaps[i]->unk[_sceneDrawVar1];
			uint8 a = _wllBuffer5[d];

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
		uint8 ovlIndex = _dscShapeOvlIndex[_dscDimMap[index] * 5] + 2;
		if (ovlIndex > 7)
			ovlIndex = 7;

		if (!scaleW || !scaleH)
			continue;

		uint8 d = (_unkPara2 + _dscUnk1[s]) & 3;
		int8 l = _wllShapeMap[_curBlockCaps[index]->unk[d]];

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

void LoLEngine::drawDoor(uint8 *shape, uint8 *table, int index, int unk2, int w, int h, int flags) {
	uint8 c = _dscDoor1[(_unkPara2 << 5) + unk2];
	int r = (c / 5) + 5 * _dscDimMap[index];
	uint16 d = _dscDoor2[r];
	uint16 t = (index << 5) + c;

	_shpDoorY = _dscDoorY[t] + 120;

	if (flags & 1) {
		// TODO
	}

	int u = 0;

	if (flags & 2) {		
		uint8 dimW = _dscDimMap[index];		
		_doorScaleW = _dscDoorScaleTable[dimW << 1];
		_doorScaleH = _dscDoorScaleTable[(dimW << 1) + 1];
		u = _dscDoor4[dimW];
	}

	d += 2;

	if (!_doorScaleW || !_doorScaleH)
		return;

	int s = _screen->getShapeScaledHeight(shape, _doorScaleH) >> 1;

	if (w)
		w = (w * _doorScaleW) >> 8;

	if (h)
		h = (h * _doorScaleH) >> 8;

	_shpDoorX = _dscDoorX[t] + w + 200;
	_shpDoorY = _shpDoorY + 4 - s + h - u;

	if (d > 7)
		d = 7;

	uint8 *ovl = _screen->getLevelOverlay(d);
	int doorScaledWitdh = _screen->getShapeScaledWidth(shape, _doorScaleW);
	
	_shpDoorX -= (doorScaledWitdh >> 1);
	_shpDoorY -= s;

	drawDoorShapes(shape, table, _shpDoorX, _shpDoorY, flags, ovl);
}

void LoLEngine::drawDoorShapes(uint8 *shape, uint8 *table, int x, int y, int flags, const uint8 *ovl) {
	int flg = 0;

	if (flags & 0x10)
		flg |= 1;

	if (flags & 0x20)
		flg |= 0x1000;

	if (flags & 0x40)
		flg |= 2;

	if (flg & 0x1000) {
		if (table)
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x9104, table, ovl, 1, _tlcTable1, _tlcTable2, _doorScaleW, _doorScaleH);			
		else
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x1104, ovl, 1, _tlcTable1, _tlcTable2, _doorScaleW, _doorScaleH);			
	} else {
		if (table)
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x8104, table, ovl, 1, _doorScaleW, _doorScaleH);
		else
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x104, ovl, 1, _doorScaleW, _doorScaleH);
	}
}

void LoLEngine::drawScriptShapes(int pageNum) {
	// TODO
}

} // end of namespace Kyra

