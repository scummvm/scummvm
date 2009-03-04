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

namespace Kyra {

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
			uint8 **of = &_monsterShapesEx[monsterIndex * 192 + i * 48 + ii * 3];
			int s = (i << 4) + ii + 17;
			of[0] = _screen->makeShapeCopy(p, s);
			of[1] = _screen->makeShapeCopy(p, s + 1);
			of[2] = _screen->makeShapeCopy(p, s + 2);
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

int LoLEngine::placeMonstersUnk(int block) {
	int i = _levelBlockProperties[block].itemMonsterIndex;
	int cnt = 0;
	MonsterInPlay *t = 0;

	while (i) {
		t = (MonsterInPlay *)findItem(i);
		i = t->next;
		if (!(i & 0x8000))
			continue;

		i &= 0x7fff;
		t = &_monsters[i];

		cnt++;
		setMonsterMode(t, 14);

		checkSceneUpdateNeed(t->blockPropertyIndex);

		placeMonster(t, 0, 0);
	}
	return cnt;
}

void LoLEngine::setMonsterMode(MonsterInPlay *monster, int mode) {
	if (monster->mode == 13 && mode != 14)
		return;
	if (mode == 7) {
		monster->destX = _partyPosX;
		monster->destY = _partyPosX;
	}

	if (monster->mode == 1 && mode == 7) {
		for (int i = 0; i < 30; i++) {
			if (monster->mode != 1)
				continue;
			monster->mode = mode;
			monster->field_15 = 0;
			monster->destX = _partyPosX;
			monster->destY = _partyPosY;
			setMonsterDirection(monster, calcMonsterDirection(monster->x, monster->y, monster->destX, monster->destY));
		}
	} else {
		monster->mode = mode;
		monster->field_15 = 0;
		if (mode == 14)
			monster->might = 0;
		if (mode == 13 && (monster->flags & 0x20)) {
			monster->mode = 0;
			cmzS3(monster);
			if (_currentLevel != 29)
				setMonsterMode(monster, 14);
			runLevelScriptCustom(0x404, -1, monster->id, monster->id, 0, 0);
			checkSceneUpdateNeed(monster->blockPropertyIndex);
			if (monster->mode == 14)
				placeMonster(monster, 0, 0);
		}
	}
}

void LoLEngine::placeMonster(MonsterInPlay *monster, uint16 x, uint16 y) {
	bool cont = true;
	int t = monster->blockPropertyIndex;
	if (monster->blockPropertyIndex) {
		removeItemOrMonsterFromBlock(&_levelBlockProperties[t].itemMonsterIndex, ((uint16)monster->id) | 0x8000);
		_levelBlockProperties[t].direction = 5;
		checkSceneUpdateNeed(t);
	} else {
		cont = false;
	}

	monster->blockPropertyIndex = calcBlockIndex(x, y);

	if (monster->x != x || monster->y != y) {
		monster->x = x;
		monster->y = y;
		monster->anon9 = (++monster->anon9) & 3;
	}

	if (monster->blockPropertyIndex == 0)
		return;

	assignItemOrMonsterToBlock(&_levelBlockProperties[monster->blockPropertyIndex].itemMonsterIndex, ((uint16)monster->id) | 0x8000);
	_levelBlockProperties[monster->blockPropertyIndex].direction = 5;
	checkSceneUpdateNeed(monster->blockPropertyIndex);

	if (monster->properties->sounds[0] == 0 || cont == false)
		return;

	if ((!(monster->properties->flags & 0x100) || ((monster->anon9 & 1) == 0)) && monster->blockPropertyIndex == t)
		return;

	if (monster->blockPropertyIndex != t)
		runLevelScriptCustom(monster->blockPropertyIndex, 0x800, -1, monster->id, 0, 0);

	if (_updateFlags & 1)
		return;

	snd_processEnvironmentalSoundEffect(monster->properties->sounds[0], monster->blockPropertyIndex);
}

int LoLEngine::calcMonsterDirection(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
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

	uint8 f = 1;

	if (t2 >= t1) {
		if (t2 > t1)
			f = 0;
		SWAP(t1, t2);
	}

	r = (r << 1) | f;

	t1 = (t1 + 1) >> 1;

	f = (t1 > t2) ? 1 : 0;
	r = (r << 1) | f;

	static const uint8 retVal[] = { 1, 2, 1, 0, 7, 6, 7, 0, 3, 2, 3, 4, 5, 6, 5, 4};
	return retVal[r];
}

void LoLEngine::setMonsterDirection(MonsterInPlay *monster, int dir) {
	monster->direction = dir;

	if (!(dir & 1) || ((monster->direction - (monster->facing << 1)) >= 2))
		monster->facing = monster->direction >> 1;

	checkSceneUpdateNeed(monster->blockPropertyIndex);
}

void LoLEngine::cmzS3(MonsterInPlay *l) {
	// TODO
}

void LoLEngine::removeItemOrMonsterFromBlock(uint16 *blockItemIndex, int id) {
	while (*blockItemIndex) {
		if (*blockItemIndex == id) {
			ItemInPlay *t = findItem(id);
			*blockItemIndex = t->next;
			t->next = 0;
			return;
		} else {
			ItemInPlay *t = findItem(*blockItemIndex);
			blockItemIndex = &t->next;
		}
	}
}

void LoLEngine::assignItemOrMonsterToBlock(uint16 *blockItemIndex, int id) {
	ItemInPlay *t = findItem(id);
	t->next = *blockItemIndex;
	*blockItemIndex = id;
}

int LoLEngine::checkBlockBeforeMonsterPlacement(int x, int y, int monsterWidth, int testFlag, int wallFlag) {
	_monsterUnkDir = 0;
	int x2 = 0;
	int y2 = 0;
	int xOffs = 0;
	int yOffs = 0;
	int flag = 0;
	
	int r = checkBlockForWallsAndSufficientSpace(calcBlockIndex(x, y), x, y, monsterWidth, testFlag, wallFlag);
	if (r)
		return r;

	r = checkBlockOccupiedByParty(x, y, testFlag);
	if (r)
		return 4;

	if (x & 0x80) {
		if (((x & 0xff) + monsterWidth) & 0xff00) {
			xOffs = 1;
			_monsterUnkDir = 2;
			x2 = x + monsterWidth;

			r = checkBlockForWallsAndSufficientSpace(calcBlockIndex(x2, y), x, y, monsterWidth, testFlag, wallFlag);
			if (r)
				return r;

			r = checkBlockOccupiedByParty(x + xOffs, y, testFlag);
			if (r)
				return 4;

			flag = 1;
		}
	} else {
		if (((x & 0xff) - monsterWidth) & 0xff00) {
			xOffs = -1;
			_monsterUnkDir = 6;
			x2 = x - monsterWidth;

			r = checkBlockForWallsAndSufficientSpace(calcBlockIndex(x2, y), x, y, monsterWidth, testFlag, wallFlag);
			if (r)
				return r;

			r = checkBlockOccupiedByParty(x + xOffs, y, testFlag);
			if (r)
				return 4;

			flag = 1;
		}
	}

	if (y & 0x80) {
		if (((y & 0xff) + monsterWidth) & 0xff00) {
			yOffs = 1;
			_monsterUnkDir = 4;
			y2 = y + monsterWidth;

			r = checkBlockForWallsAndSufficientSpace(calcBlockIndex(x, y2), x, y, monsterWidth, testFlag, wallFlag);
			if (r)
				return r;

			r = checkBlockOccupiedByParty(x, y + yOffs, testFlag);
			if (r)
				return 4;
		} else {
			flag = 0;
		}
	} else {
		if (((y & 0xff) - monsterWidth) & 0xff00) {
			yOffs = -1;
			_monsterUnkDir = 0;
			y2 = y - monsterWidth;

			r = checkBlockForWallsAndSufficientSpace(calcBlockIndex(x, y2), x, y, monsterWidth, testFlag, wallFlag);
			if (r)
				return r;

			r = checkBlockOccupiedByParty(x, y + yOffs, testFlag);
			if (r)
				return 4;
		} else {
			flag = 0;
		}
	}

	if (!flag)
		return 0;

	r = checkBlockForWallsAndSufficientSpace(calcBlockIndex(x2, y2), x, y, monsterWidth, testFlag, wallFlag);
	if (r)
		return r;

	r = checkBlockOccupiedByParty(x + xOffs, y + yOffs, testFlag);
	if (r)
		return 4;

	return 0;
}

int LoLEngine::calcMonsterSkillLevel(int id, int a) {
	const uint16 *c = getCharacterOrMonsterStats(id);
	int r = (a << 8) / c[4];

	if (!(id & 0x8000))
		r = (r * _monsterModifiers[3 + ((_unkGameFlag & 0x30) << 4)]) >> 8;

	id &= 0x7fff;

	if (_characters[id].skillLevels[1] <= 3)
		return r;
	else if (_characters[id].skillLevels[1] <= 7)
		return (r- (r >> 2));
	
	return (r- (r >> 1));
}

int LoLEngine::checkBlockForWallsAndSufficientSpace(int block, int x, int y, int monsterWidth, int testFlag, int wallFlag) {
	if (block == _currentBlock)
		testFlag &= 0xfffe;

	if (testFlag & 1) {
		_monsterCurBlock = block;
		if (testWallFlag(block, -1, wallFlag))
			return 1;
	}

	_monsterCurBlock = 0;

	if (!(testFlag & 2))
		return 0;

	uint16 b = _levelBlockProperties[block].itemMonsterIndex;
	while (b & 0x8000) {
		MonsterInPlay *monster = &_monsters[b & 0x7fff];
		
		if (monster->mode < 13) {
			int r = checkMonsterSpace(x, y, monster->x, monster->y);
			if ((monsterWidth + monster->properties->maxWidth) > r)
				return 2;
		}

		b = findItem(b)->next;
	}

	return 0;
}

bool LoLEngine::checkBlockOccupiedByParty(int x, int y, int testFlag) {
	if ((testFlag & 4) && (_currentBlock == calcBlockIndex(x, y)))
		return true;

	return false;
}

void LoLEngine::drawMonstersAndItems(int block) {
	LevelBlockProperty *l = _curBlockCaps[block];
	uint16 s = l->itemMonsterIndex;		
	ItemInPlay *i = findItem(s);

	if (l->direction != _currentDirection) {
		l->field_6 = 0;
		l->direction = _currentDirection;

		while (s) {
			i = findItem(s);
			recalcItemMonsterPositions(_currentDirection, s, l, true);
			i = findItem(s);
			s = i->next;
		}
	}

	s = l->field_6;
	while (s) {
		if (s & 0x8000) {
			s &= 0x7fff;
			if (block < 15)
				drawMonster(s);
			s = _monsters[s].unk2;
		} else {
			//////////
			// TODO

		}
	}
}

void LoLEngine::drawMonster(uint16 id) {
	MonsterInPlay *m = &_monsters[id];	
	int16 flg = _monsterDirFlags[(_currentDirection << 2) + m->facing];
	int curFrm = getMonsterCurFrame(m, flg & 0xffef);

	if (curFrm == -1) {
		////////////
		// TODO
		curFrm=curFrm;

	} else {
		int d = m->flags & 7;
		bool flip = m->properties->flags & 0x200 ? true : false;
		flg &= 0x10;
		uint8 *shp = _monsterShapes[(m->properties->shapeIndex << 4) + curFrm];
		
		if (m->properties->flags & 0x800)
			flg |= 0x20;

		uint8 *ovl1 = d ? _monsterPalettes[(m->properties->shapeIndex << 4) + (curFrm & 0x0f)] + (shp[10] * (d - 1)) : 0;
		uint8 *ovl2 = drawItemOrMonster(shp, ovl1, m->x + _monsterLevelOffs[m->level << 1], m->y + _monsterLevelOffs[(m->level << 1) + 1], 0, 0, flg | 1, -1, flip);

		for (int i = 0; i <	4; i++) {
			int v = m->field_2A[i] - 1;
			if (v == -1)
				break;

			uint8 *shp2 = _monsterShapesEx[m->properties->shapeIndex * 192 + v * 48 + curFrm * 3];
			if (!shp2)
				continue;

			//int dW = (READ_LE_UINT16(shp2 + 4) * _dmScaleW) >> 8;
			//int dH = (READ_LE_UINT16(shp2 + 6) * _dmScaleH) >> 8;
			//if (flg)
			//	_screen->getShapeScaledWidth(shp2, _dmScaleW);

			drawDoorOrMonsterShape(shp2, 0, _shpDmX, _shpDmY, flg | 1, ovl2);
		}

		if (!m->field_1B)
			return;

		int dW = _screen->getShapeScaledWidth(shp, _dmScaleW) >> 1;
		int dH = _screen->getShapeScaledHeight(shp, _dmScaleH) >> 1;

		int a = (m->mode == 13) ? (m->field_15 << 1) : (m->properties->might / (m->field_1B & 0x7fff));
		
		shp = _gameShapes[6];
		
		int cF = m->properties->flags & 0xc000;
		if (cF == 0x4000)
			cF = 63;
		else if (cF == 0x8000)
			cF = 15;
		else if (cF == 0xc000)
			cF = 74;
		else
			cF = 0;

		uint8 *tbl = new uint8[256];
		if (cF) {			
			for (int i = 0; i < 256; i++) {
				tbl[i] = i;
				if (i < 2 || i > 7)
					continue;
				tbl[i] += cF;
			}
		}

		dW += m->anon8;
		dH += m->anonh;

		a = CLIP(a, 1, 4);

		int sW = _dmScaleW / a;
		int sH = _dmScaleH / a;

		_screen->drawShape(_sceneDrawPage1, shp, _shpDmX + dW, _shpDmY + dH, 13, 0x124, tbl, cF ? 1 : 0, sW, sH);

		delete[] tbl;
	}
}

int LoLEngine::getMonsterCurFrame(MonsterInPlay *m, uint16 dirFlags) {
	switch (_monsterUnk[m->properties->shapeIndex]) {
		case 0:
			if (dirFlags) {
				return (*m->properties->pos & 0xff) == 13 ? -1 : (dirFlags + m->anon9);
			} else {
				if (m->field_1B)
					return 12;

				switch (m->mode - 5) {
					case 0:
						return m->field_1B ? 12 : ((m->properties->flags & 4) ? 13 : 0);
					case 3:
						return (m->field_15 + 13);
					case 6:
						return 14;
					case 8:
						return -1;
					default:
						return m->field_1B ? 12 : m->anon9;
				}
			}
			break;
		case 1:
			///////
			// TODO
			break;
		case 2:
			///////
			// TODO
			break;
		case 3:
			///////
			// TODO
			break;
		default:
			break;
	}

	return 0;
}

void LoLEngine::recalcItemMonsterPositions(uint16 direction, uint16 itemIndex, LevelBlockProperty *l, bool flag) {
	if (l->direction != direction) {
		l->direction = 5;
		return;
	}

	ItemInPlay *i1 = findItem(itemIndex);
	int r = calcItemMonsterPosition(i1, direction);
	uint16 *b = &l->field_6;
	ItemInPlay *i2 = 0;

	while (*b) {
		i2 = findItem(itemIndex);

		if (flag) {
			if (calcItemMonsterPosition(i2, direction) >= r)
				break;
		} else {
			if (calcItemMonsterPosition(i2, direction) > r)
				break;
		}

		b = &i2->unk2;
	}

	i1->unk2 = *b;
	*b = itemIndex;
}

int LoLEngine::calcItemMonsterPosition(ItemInPlay *i, uint16 direction) {
	int x = i->x;
	int y = i->y;

	recalcSpritePosition(_partyPosX, _partyPosY, x, y, direction);

	if (y < 0)
		y = 0;

	int res = (i->unk4 << 12);
	res |= (4095 - y);

	return res;
}

void LoLEngine::recalcSpritePosition(uint16 partyX, uint16 partyY, int &itemX, int &itemY, uint16 direction) {
	int a = itemX - partyX;
	int b = partyY - itemY;

	if (direction) {
		if (direction != 2)
			SWAP(a, b);
		if (direction != 3) {
			a = -a;
			if (direction != 1)
				b = -b;
		} else {
			b = -b;
		}
	}

	itemX = a;
	itemY = b;
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

uint8 *LoLEngine::drawItemOrMonster(uint8 *shape, uint8 *table, int x, int y, int w, int h, int flags, int tblValue, bool flip) {
	uint8 *ovl2 = 0;
	uint8 *ovl = 0;
	uint8 tmpOvl[16];

	if (flags & 0x80) {
		flags &= 0xff7f;
		ovl2 = table;
		table = 0;
	} else {
		ovl2 = _screen->getLevelOverlay(4);
	}

	int r = calcDrawingLayerParameters(x, y, _shpDmX, _shpDmY, _dmScaleW, _dmScaleH, shape, flip);

	if (tblValue == -1) {
		r = 7 - ((r / 3) - 1);
		r = CLIP(r, 0, 7);
		ovl = _screen->getLevelOverlay(r);
	} else {		
		memset (tmpOvl + 1, tblValue, 15);
		table = tmpOvl;
		ovl = _screen->getLevelOverlay(7);
	}

	int flg = flags & 0x10 ? 1 : 0;
	if (flags & 0x20)
		flg |= 0x1000;
	if (flags & 0x40)
		flg |= 2;

	if (_currentLevel == 22) {
		if (ovl)
			ovl[255] = 0;
	} else {
		flg |= 0x2000;
	}

	_shpDmX += ((_dmScaleW * w) >> 8);
	_shpDmY += ((_dmScaleH * h) >> 8);

	int dH = _screen->getShapeScaledHeight(shape, _dmScaleH) >> 1;

	if (flg & 0x1000) {
		if (table)
			_screen->drawShape(_sceneDrawPage1, shape, _shpDmX, _shpDmY, 13, flg | 0x8124, table, ovl, 0, _trueLightTable1, _trueLightTable2, _dmScaleW, _dmScaleH, ovl2);
		else
			_screen->drawShape(_sceneDrawPage1, shape, _shpDmX, _shpDmY, 13, flg | 0x124, ovl, 0, _trueLightTable1, _trueLightTable2, _dmScaleW, _dmScaleH, ovl2);
	} else {
		if (table)
			_screen->drawShape(_sceneDrawPage1, shape, _shpDmX, _shpDmY, 13, flg | 0x8124, table, ovl, 1, _dmScaleW, _dmScaleH, ovl2);
		else
			_screen->drawShape(_sceneDrawPage1, shape, _shpDmX, _shpDmY, 13, flg | 0x124, ovl, 1, _dmScaleW, _dmScaleH, ovl2);
	}

	_shpDmX -= (_screen->getShapeScaledWidth(shape, _dmScaleW) >> 1);
	_shpDmY -= dH;

	return ovl;
}

int LoLEngine::calcDrawingLayerParameters(int x1, int y1, int16 &x2, int16 &y2, int16 &w, int16 &h, uint8 *shape, int flip) {
	recalcSpritePosition(_partyPosX, _partyPosY, x1, y1, _currentDirection);

	if (y1 < 0) {
		w = h = x2 = y2 = 0;
		return 0;
	}

	int l = y1 >> 5;
	y2 = _monsterScaleY[l];
	x2 = ((_monsterScaleX[l] * x1) >> 8) + 200;
	w = h = _monsterScaleWH[_shpDmY - 56];

	if (flip)
		y2 = ((120 - y2) >> 1) + _screen->getShapeScaledHeight(shape, _dmScaleH);
	else 
		y2 -= (_screen->getShapeScaledHeight(shape, _dmScaleH) >> 1);

	return l;
}

void LoLEngine::updateMonster(MonsterInPlay *monster) {
	static const uint8 monsterState[] = { 1, 0, 1, 3, 3, 0, 0, 3, 4, 1, 0, 0, 4, 0, 0 };
	if (monster->mode > 14)
		return;

	int s = monsterState[monster->mode];
	int a = monster->field_1F++;

	if ((a < monster->properties->b) && (s & 4))
		return;

	monster->field_1F = 0;

	if (monster->properties->flags & 0x40) {
		monster->might += _rnd.getRandomNumberRng(1, 8);
		if (monster->might > monster->properties->might)
			monster->might = monster->properties->might;
	}

	if (monster->flags & 8) {
		monster->destX = _partyPosX;
		monster->destY = _partyPosY;
	}

	if (s & 2) {

		/////
		// TODO
	}

	if ((s & 1) && (monster->flags & 0x10))
		setMonsterMode(monster, 7);

	////
	// TODO

	switch (monster->mode) {
		case 0:
		case 1:
			if (monster->flags & 0x10) {
				for (int i = 0; i < 30; i++) {
					if (_monsters[i].mode == 1)
						setMonsterMode(&_monsters[i], 7);
				}
			} else if (monster->mode == 1) {
				moveMonster(monster);
			}
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		case 9:
			break;
		case 12:
			break;
		case 13:
			break;
		case 14:
			break;
		default:
			break;
	}

	if (monster->field_1B) {
		if (monster->field_1B & 0x8000)
			monster->field_1B &= 0x7fff;
		else
			monster->field_1B = 0;
		checkSceneUpdateNeed(monster->blockPropertyIndex);
	}

	monster->flags &= 0xffef;
}

void LoLEngine::moveMonster(MonsterInPlay *monster) {
	static const int8 turnPos[] = { 0, 2, 6, 6, 0, 2, 4, 4, 2, 2, 4, 6, 0, 0, 4, 6, 0 };
	if (monster->x != monster->destX || monster->y != monster->destY) {
		walkMonster(monster);		
	} else if (monster->direction != monster->destDirection) {
		int i = (monster->facing << 2) + (monster->destDirection >> 1);
		setMonsterDirection(monster, turnPos[i]);
	}
}

void LoLEngine::walkMonster(MonsterInPlay *monster) {
	if (monster->properties->flags & 0x400)
		return;

	int s = walkMonsterCalcNextStep(monster);
	
	if (s == -1) {
		if (walkMonsterCheckDest(monster->x, monster->y, monster, 4) != 1)
			return;

		_monsterUnkDir ^= 4;
		setMonsterDirection(monster, _monsterUnkDir);
	} else {
		setMonsterDirection(monster, s);
		if (monster->field_25) {
			if (getMonsterDistance(monster->blockPropertyIndex, _currentBlock) >= 2) {
				if (walkMonster_s3(monster->blockPropertyIndex, monster->direction, 3, _currentBlock) != 5) {
					if (monster->field_27)
						return;
				}
			}
		}		
	}

	int fx = 0;
	int fy = 0;

	walkMonsterGetNextStepCoords(monster->x, monster->y, fx, fy, (s == -1) ? _monsterUnkDir : s);
	placeMonster(monster, fx, fy);
}

int LoLEngine::walkMonsterCalcNextStep(MonsterInPlay *monster) {
	static const int8 walkMonsterTable1[] = { 7, -6, 5, -4, 3, -2, 1, 0 };
	static const int8 walkMonsterTable2[] = { -7, 6, -5, 4, -3, 2, -1, 0 };

	if (++_monsterCountUnk > 10) {
		_monsterCountUnk = 0;
		_monsterShiftAlt ^= 1;
	}

	const int8 *tbl = _monsterShiftAlt ? walkMonsterTable2 : walkMonsterTable1;

	int sx = monster->x;
	int sy = monster->y;	
	int s = monster->direction;
	int d = calcMonsterDirection(monster->x, monster->y, monster->destX, monster->destY);

	if (monster->flags & 8)
		d ^= 4;

	d = (d - s) & 7;

	if (d >= 5)
		s = (s - 1) & 7;
	else if (d)
		s = (s + 1) & 7;

	for (int i = 7; i > -1; i--) {
		s = (s + tbl[i]) & 7;

		int fx = 0;
		int fy = 0;
		walkMonsterGetNextStepCoords(sx, sy, fx, fy, s);
		d = walkMonsterCheckDest(fx, fy, monster, 4);

		if (!d)
			return s;

		if ((d != 1) || (s & 1) || (!(monster->properties->flags & 0x80)))
			continue;

		uint8 w = _levelBlockProperties[_monsterCurBlock].walls[(s >> 1) ^ 2];
		
		if (_wllWallFlags[w] & 0x20) {
			if (_wllBuffer3[w] == 5)
				openDoorSub2(_monsterCurBlock, 1);
		}

		if (_wllWallFlags[w] & 8)
			return -1;
	}

	return -1;
}

int LoLEngine::getMonsterDistance(uint16 block1, uint16 block2) {
	int8 b1x = block1 & 0x1f;
	int8 b1y = (block1 >> 8) & 0x1f;
	int8 b2x = block2 & 0x1f;
	int8 b2y = (block2 >> 8) & 0x1f;

	uint8 dy = ABS(b2y - b1y);
	uint8 dx = ABS(b2x - b1x);

	if (dx > dy)
		SWAP(dx, dy);

	return (dx << 1) + dy;
}

int LoLEngine::walkMonster_s3(uint16 monsterBlock, int unk1, int unk2, uint16 curBlock) {
	return 0;
}

int LoLEngine::walkMonsterCheckDest(int x, int y, MonsterInPlay *monster, int unk) {
	uint8 m = monster->mode;
	monster->mode = 15;

	int res = checkBlockBeforeMonsterPlacement(x, y, monster->properties->maxWidth, 7, monster->properties->flags & 0x1000 ? 32 : unk);

	monster->mode = m;
	return res;
}

void LoLEngine::walkMonsterGetNextStepCoords(int16 srcX, int16 srcY, int &newX, int &newY, uint16 unk) {
	static const int8 shiftTableX[] = { 0, 32, 32, 32, 0, -32, -32, -32 };
	static const int8 shiftTableY[] = { -32, -32, 0, 32, 32, 32, 0, -32 };

	newX = (srcX + shiftTableX[unk]) & 0x1fff;
	newY = (srcY + shiftTableY[unk]) & 0x1fff;
}

} // end of namespace Kyra

#endif // ENABLE_LOL

