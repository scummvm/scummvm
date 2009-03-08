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
#include "kyra/timer.h"
#include "kyra/resource.h"

#include "common/endian.h"

namespace Kyra {

void LoLEngine::runInitScript(const char *filename, int optionalFunc) {
	_suspendScript = true;
	EMCData scriptData;
	EMCState scriptState;
	memset(&scriptData, 0, sizeof(EMCData));
	_emc->load(filename, &scriptData, &_opcodes);

	_emc->init(&scriptState, &scriptData);
	_emc->start(&scriptState, 0);
	while (_emc->isValid(&scriptState))
		_emc->run(&scriptState);

	if (optionalFunc) {
		_emc->init(&scriptState, &scriptData);
		_emc->start(&scriptState, optionalFunc);
		while (_emc->isValid(&scriptState))
			_emc->run(&scriptState);
	}

	_emc->unload(&scriptData);
	_suspendScript = false;
}

void LoLEngine::runInfScript(const char *filename) {
	_emc->load(filename, &_scriptData, &_opcodes);
	runLevelScript(0x400, -1);
}

void LoLEngine::runLevelScript(int block, int sub) {
	runLevelScriptCustom(block, sub, -1, 0, 0, 0);
}

void LoLEngine::runLevelScriptCustom(int block, int sub, int charNum, int item, int reg3, int reg4) {
	EMCState scriptState;
	memset(&scriptState, 0, sizeof(EMCState));

	if (!_suspendScript) {
		_emc->init(&scriptState, &_scriptData);
		_emc->start(&scriptState, block);

		scriptState.regs[0] = sub;
		scriptState.regs[1] = charNum;
		scriptState.regs[2] = item;
		scriptState.regs[3] = reg3;
		scriptState.regs[4] = reg4;
		scriptState.regs[5] = block;
		scriptState.regs[6] = _scriptDirection;

		if (_emc->isValid(&scriptState)) {
			if (*(scriptState.ip - 1) & sub) {
				while (_emc->isValid(&scriptState))
					_emc->run(&scriptState);
			}
		}
	}

	checkSceneUpdateNeed(block);
}

bool LoLEngine::checkSceneUpdateNeed(int func) {
	if (_sceneUpdateRequired)
		return true;

	for (int i = 0; i < 15; i++) {
		if (_currentBlockPropertyIndex[i] == func) {
			_sceneUpdateRequired = true;
			return true;
		}
	}

	if (_currentBlock == func){
		_sceneUpdateRequired = true;
		return true;
	}

	return false;
}

int LoLEngine::olol_drawScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_drawScene(%p) (%d)", (const void *)script, stackPos(0));
	drawScene(stackPos(0));
	return 1;
}

int LoLEngine::olol_delay(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_delay(%p) (%d)", (const void *)script, stackPos(0));
	delay(stackPos(0) * _tickLength);
	return 1;
}

int LoLEngine::olol_setGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setGameFlag(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (stackPos(1))
		_gameFlags[stackPos(0) >> 4] |= (1 << (stackPos(0) & 0x0f));
	else
		_gameFlags[stackPos(0) >> 4] &= (~(1 << (stackPos(0) & 0x0f)));

	return 1;
}

int LoLEngine::olol_testGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_testGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	if (stackPos(0) < 0)
		return 0;

	if (_gameFlags[stackPos(0) >> 4] & (1 << (stackPos(0) & 0x0f)))
		return 1;

	return 0;
}

int LoLEngine::olol_loadLevelGraphics(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadLevelGraphics(%p) (%s, %d, %d, %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	loadLevelGraphics(stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), (stackPos(5) == -1) ? 0 : stackPosString(5));
	return 1;
}

int LoLEngine::olol_loadCmzFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadCmzFile(%p) (%s)", (const void *)script, stackPosString(0));
	loadCmzFile(stackPosString(0));
	return 1;
}

int LoLEngine::olol_loadMonsterShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadMonsterShapes(%p) (%s, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2));
	loadMonsterShapes(stackPosString(0), stackPos(1), stackPos(2));
	return 1;
}

int LoLEngine::olol_deleteHandItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_deleteHandItem(%p) ()", (const void *)script);
	int r = _itemInHand;
	deleteItem(_itemInHand);
	setHandItem(0);
	return r;
}

int LoLEngine::olol_allocItemPropertiesBuffer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_allocItemPropertiesBuffer(%p) (%d)", (const void *)script, stackPos(0));
	delete[] _itemProperties;
	_itemProperties = new ItemProperty[stackPos(0)];
	return 1;
}

int LoLEngine::olol_setItemProperty(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setItemProperty(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));
	ItemProperty *tmp = &_itemProperties[stackPos(0)];

	tmp->nameStringId = stackPos(1);
	tmp->shpIndex = stackPos(2);
	tmp->type = stackPos(3);
	tmp->itemScriptFunc = stackPos(4);
	tmp->might = stackPos(5);
	tmp->skill = stackPos(6);
	tmp->protection = stackPos(7);
	tmp->flags = stackPos(8);
	tmp->unkB = stackPos(9);
	return 1;
}

int LoLEngine::olol_makeItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_makeItem(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	return makeItem(stackPos(0), stackPos(1), stackPos(2));
}

int LoLEngine::olol_getItemPara(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getItemPara(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (!stackPos(0))
		return 0;

	ItemInPlay *i = &_itemsInPlay[stackPos(0)];
	ItemProperty *p = &_itemProperties[i->itemPropertyIndex];

	switch (stackPos(1)) {
	case 0:
		return i->blockPropertyIndex;
	case 1:
		return i->x;
	case 2:
		return i->y;
	case 3:
		return i->level;
	case 4:
		return i->itemPropertyIndex;
	case 5:
		return i->shpCurFrame_flg;
	case 6:
		return p->nameStringId;
	case 7:
		break;
	case 8:
		return p->shpIndex;
	case 9:
		return p->type;
	case 10:
		return p->itemScriptFunc;
	case 11:
		return p->might;
	case 12:
		return p->skill;
	case 13:
		return p->protection;
	case 14:
		return p->unkB;
	case 15:
		return i->shpCurFrame_flg & 0x1fff;
	case 16:
		return p->flags;
	case 17:
		return (p->skill << 8) | p->might;
	default:
		break;
	}

	return -1;
}

int LoLEngine::olol_getCharacterStat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getCharacterStat(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	LoLCharacter *c = &_characters[stackPos(0)];
	int d = stackPos(2);

	switch (stackPos(1)) {
	case 0:
		return c->flags;

	case 1:
		return c->raceClassSex;

	case 5:
		return c->hitPointsCur;

	case 6:
		return c->hitPointsMax;

	case 7:
		return c->magicPointsCur;

	case 8:
		return c->magicPointsMax;

	case 9:
		return c->itemsProtection;

	case 10:
		return c->items[d];

	case 11:
		return c->skillLevels[d] + c->skillModifiers[d];

	case 12:
		return c->field_27[d];

	case 13:
		return (d & 0x80) ? c->itemsMight[7] : c->itemsMight[d];

	case 14:
		return c->skillModifiers[d];

	case 15:
		return c->id;

	default:
		break;
	}

	return 0;
}

int LoLEngine::olol_setCharacterStat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setCharacterStat(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	LoLCharacter *c = &_characters[stackPos(0)];
	int d = stackPos(2);
	int e = stackPos(3);

	switch (stackPos(1)) {
	case 0:
		c->flags = e;
		break;

	case 1:
		c->raceClassSex = e & 0x0f;
		break;

	case 5:
		//// TODO
		break;

	case 6:
		c->hitPointsMax = e;
		break;

	case 7:
		//// TODO
		break;

	case 8:
		c->magicPointsMax = e;
		break;

	case 9:
		c->itemsProtection = e;
		break;

	case 10:
		c->items[d] = 0;
		break;

	case 11:
		c->skillLevels[d] = e;
		break;

	case 12:
		c->field_27[d] = e;
		break;

	case 13:
		if (d & 0x80)
			c->itemsMight[7] = e;
		else
			c->itemsMight[d] = e;
		break;

	case 14:
		c->skillModifiers[d] = e;
		break;

	default:
		break;
	}

	return 0;
}

int LoLEngine::olol_loadLevelShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadLevelShapes(%p) (%s, %s)", (const void *)script, stackPosString(0), stackPosString(1));
	loadLevelShpDat(stackPosString(0), stackPosString(1), true);
	return 1;
}

int LoLEngine::olol_closeLevelShapeFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_closeLevelShapeFile(%p) ()", (const void *)script);
	delete _lvlShpFileHandle;
	_lvlShpFileHandle = 0;
	return 1;
}

int LoLEngine::olol_loadDoorShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadDoorShapes(%p) (%s, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2));
	_screen->loadBitmap(stackPosString(0), 3, 3, 0);
	const uint8 *p = _screen->getCPagePtr(2);
	if (_doorShapes[0])
		delete[] _doorShapes[0];
	_doorShapes[0] = _screen->makeShapeCopy(p, stackPos(1));
	if (_doorShapes[1])
		delete[] _doorShapes[1];
	_doorShapes[1] = _screen->makeShapeCopy(p, stackPos(2));

	for (int i = 0; i < 20; i++) {
		_wllWallFlags[i + 3] |= 7;
		int t = i % 5;
		if (t == 4)
			_wllWallFlags[i + 3] &= 0xf8;
		if (t == 3)
			_wllWallFlags[i + 3] &= 0xfd;
	}

	if (stackPos(3)) {
		for (int i = 3; i < 13; i++)
			_wllWallFlags[i] &= 0xfd;
	}

	if (stackPos(4)) {
		for (int i = 13; i < 23; i++)
			_wllWallFlags[i] &= 0xfd;
	}

	return 1;
}

int LoLEngine::olol_initAnimStruct(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_initAnimStruct(%p) (%s, %d, %d, %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	if (_tim->initAnimStruct(stackPos(1), stackPosString(0), stackPos(2), stackPos(3), stackPos(4), 0, stackPos(5)))
		return 1;
	return 0;
}

int LoLEngine::olol_freeAnimStruct(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_freeAnimStruct(%p) (%d)", (const void *)script, stackPos(0));
	if (_tim->freeAnimStruct(stackPos(0)))
		return 1;
	return 0;
}

int LoLEngine::olol_getDirection(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getDirection(%p)", (const void *)script);
	return _currentDirection;
}

int LoLEngine::olol_setMusicTrack(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setMusicTrack(%p) (%d)", (const void *)script, stackPos(0));
	_curMusicTheme = stackPos(0);
	return 1;
}

int LoLEngine::olol_clearDialogueField(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_clearDialogueField(%p) (%d)", (const void *)script, stackPos(0));
	if (_hideControls && (!textEnabled()))
		return 1;

	_screen->setScreenDim(5);
	const ScreenDim *d = _screen->getScreenDim(5);
	_screen->fillRect(d->sx, d->sy, d->sx + d->w - 2, d->sy + d->h - 2, d->unkA);
	_screen->clearDim(4);

	return 1;
}

int LoLEngine::olol_getUnkArrayVal(EMCState *script) {
	return _unkEMC46[stackPos(0)];
}

int LoLEngine::olol_setUnkArrayVal(EMCState *script) {
	_unkEMC46[stackPos(0)] = stackPos(1);
	return 1;
}

int LoLEngine::olol_getGlobalVar(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getGlobalVar(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));

	switch (stackPos(0)) {
	case 0:
		return _currentBlock;
	case 1:
		return _currentDirection;
	case 2:
		return _currentLevel;
	case 3:
		return _itemInHand;
	case 4:
		return _brightness;
	case 5:
		return _credits;
	case 6:
		return _unkWordArraySize8[stackPos(1)];
	case 8:
		return _updateFlags;
	case 9:
		return _lampStatusUnk;
	case 10:
		return _sceneDefaultUpdate;
	case 11:
		return _unkBt1;
	case 12:
		return _unkBt2;
	case 13:
		return _speechFlag;
	default:
		break;
	}

	return 0;
}

int LoLEngine::olol_setGlobalVar(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setGlobalVar(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	uint16 a = stackPos(1);
	uint16 b = stackPos(2);

	switch (stackPos(0)) {
	case 0:
		_currentBlock = b;
		calcCoordinates(_partyPosX, _partyPosY, _currentBlock, 0x80, 0x80);
		setLF2(_currentBlock);
		break;

	case 1:
		_currentDirection = b;
		break;

	case 2:
		_currentLevel = b & 0xff;
		break;

	case 3:
		setHandItem(b);
		break;

	case 4:
		_brightness = b & 0xff;
		break;

	case 5:
		_credits = b;
		break;

	case 6:
		_unkWordArraySize8[a] = b;
		break;

	case 7:
		break;

	case 8:
		_updateFlags = b;
		if (b == 1) {
			if (!textEnabled() || (!(_hideControls & 2)))
				timerUpdatePortraitAnimations(1);
			disableSysTimer(2);
		} else {
			enableSysTimer(2);
		}
		break;

	case 9:
		_lampStatusUnk = b & 0xff;
		break;

	case 10:
		_sceneDefaultUpdate = b & 0xff;
		gui_toggleButtonDisplayMode(0, 0);
		break;

	case 11:
		_unkBt1 = a & 0xff;
		break;

	case 12:
		_unkBt2 = a & 0xff;
		break;

	default:
		break;
	}

	return 1;
}

int LoLEngine::olol_triggerDoorSwitch(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_triggerDoorSwitch(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	processDoorSwitch(stackPos(0)/*, (_wllWallFlags[_levelBlockProperties[stackPos(0)].walls[0]] & 8) ? 0 : 1*/, stackPos(1));
	return 1;
}

int LoLEngine::olol_mapShapeToBlock(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_mapShapeToBlock(%p) (%d)", (const void *)script, stackPos(0));
	return assignLevelShapes(stackPos(0));
}

int LoLEngine::olol_resetBlockShapeAssignment(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_resetBlockShapeAssignment(%p) (%d)", (const void *)script, stackPos(0));
	uint8 v = stackPos(0) & 0xff;
	memset(_wllShapeMap + 3, v, 5);
	memset(_wllShapeMap + 13, v, 5);
	return 1;
}

int LoLEngine::olol_initMonster(EMCState *script) {
	uint16 x = 0;
	uint16 y = 0;
	calcCoordinates(x, y, stackPos(0), stackPos(1), stackPos(2));
	uint16 w = _monsterProperties[stackPos(4)].maxWidth;

	if (checkBlockBeforeMonsterPlacement(x, y, w, 7, 7))
		return -1;

	for (uint8 i = 0; i < 30; i++) {
		MonsterInPlay *l = &_monsters[i];
		if (l->might || l->mode == 13)
			continue;

		memset(l, 0, sizeof(MonsterInPlay));
		l->id = i;
		l->x = x;
		l->y = y;
		l->facing = stackPos(3);
		l->type = stackPos(4);
		l->properties = &_monsterProperties[l->type];
		l->direction = l->facing << 1;
		l->might = (l->properties->might * _monsterModifiers[((_unkGameFlag & 0x30) >> 4)]) >> 8;

		if (_currentLevel == 12 && l->type == 2)
			l->might = (l->might * (_rnd.getRandomNumberRng(1, 128) + 192)) >> 8;

		l->field_25 = l->properties->unk6[0];
		l->field_27 = _rnd.getRandomNumberRng(1, calcMonsterSkillLevel(l->id | 0x8000, 8)) - 1;
		l->unk4 = 2;
		l->flags = stackPos(5);
		l->assignedItems = 0;

		setMonsterMode(l, stackPos(6));
		placeMonster(l, l->x, l->y);

		l->destX = l->x;
		l->destY = l->y;
		l->destDirection = l->direction;

		for (int ii = 0; ii < 4; ii++)
			l->field_2A[ii] = stackPos(7 + ii);

		checkSceneUpdateNeed(l->blockPropertyIndex);
		return i;
	}

	return -1;
}

int LoLEngine::olol_loadMonsterProperties(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadMonsterProperties(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
		(const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5),
		stackPos(6), stackPos(7), stackPos(8), stackPos(9), stackPos(10), stackPos(11), stackPos(12), stackPos(13),
		stackPos(14), stackPos(15), stackPos(16), stackPos(17), stackPos(18), stackPos(19), stackPos(20),
		stackPos(21), stackPos(22), stackPos(23), stackPos(24), stackPos(25), stackPos(26),	stackPos(27),
		stackPos(28), stackPos(29), stackPos(30), stackPos(31), stackPos(32), stackPos(33), stackPos(34),
		stackPos(35), stackPos(36), stackPos(37), stackPos(38), stackPos(39), stackPos(40), stackPos(41));

	MonsterProperty *l = &_monsterProperties[stackPos(0)];
	l->shapeIndex = stackPos(1) & 0xff;

	int shpWidthMax = 0;

	for (int i = 0; i < 16; i++) {
		uint8 m = _monsterShapes[(l->shapeIndex << 4) + i][3];
		if (m > shpWidthMax)
			shpWidthMax = m;
	}

	l->maxWidth = shpWidthMax;

	l->field2[0] = (stackPos(2) << 8) / 100;
	l->field2[1] = 256;
	l->protection = (stackPos(3) << 8) / 100;
	l->unk[0] = stackPos(4);
	l->unk[1] = (stackPos(5) << 8) / 100;
	l->unk[2] = (stackPos(6) << 8) / 100;
	l->unk[3] = (stackPos(7) << 8) / 100;
	l->unk[4] = (stackPos(8) << 8) / 100;
	l->unk[5] = 0;

	for (int i = 0; i < 8; i++) {
		l->unk2[i] = stackPos(9 + i);
		l->unk3[i] = (stackPos(17 + i) << 8) / 100;
	}

	l->pos = &l->field2[0];
	l->itemProtection = stackPos(25);
	l->might = stackPos(26);
	l->b = 1;
	l->flags = stackPos(27);
	l->unk5 = stackPos(28);
	// FIXME???
	l->unk5 = stackPos(29);
	//

	for (int i = 0; i < 5; i++)
		l->unk6[i] = stackPos(30 + i);

	for (int i = 0; i < 2; i++) {
		l->unk7[i] = stackPos(35 + i);
		l->unk7[i + 2] = stackPos(37 + i);
	}

	for (int i = 0; i < 3; i++)
		l->sounds[i] = stackPos(39 + i);

	return 1;
}

int LoLEngine::olol_moveMonster(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_moveMonster(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	MonsterInPlay *m = &_monsters[stackPos(0)];

	if (m->mode == 1 || m->mode == 2) {
		calcCoordinates(m->destX, m->destY, stackPos(1), stackPos(2), stackPos(3));
		m->destDirection = stackPos(4) << 1;
		if (m->x != m->destX || m->y != m->destY)
			setMonsterDirection(m, calcMonsterDirection(m->x, m->y, m->destX, m->destY));
	}

	return 1;
}

int LoLEngine::olol_setScriptTimer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setScriptTimer(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	uint8 id = 0x50 + stackPos(0);

	if (stackPos(1)) {
		_timer->enable(id);
		_timer->setCountdown(id, stackPos(1));

	} else {
		_timer->disable(id);
	}

	return true;
}

int LoLEngine::olol_loadTimScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadTimScript(%p) (%d, %s)", (const void *)script, stackPos(0), stackPosString(1));
	if (_activeTim[stackPos(0)])
		return 1;
	char file[13];
	snprintf(file, sizeof(file), "%s.TIM", stackPosString(1));
	_activeTim[stackPos(0)] = _tim->load(file, &_timIngameOpcodes);
	return 1;
}

int LoLEngine::olol_runTimScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_runTimScript(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return _tim->exec(_activeTim[stackPos(0)], stackPos(1));
}

int LoLEngine::olol_releaseTimScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_releaseTimScript(%p) (%d)", (const void *)script, stackPos(0));
	_tim->unload(_activeTim[stackPos(0)]);
	return 1;
}

int LoLEngine::olol_initDialogueSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_initDialogueSequence(%p) (%d)", (const void *)script, stackPos(0));
	initDialogueSequence(stackPos(0));
	return 1;
}

int LoLEngine::olol_restoreSceneAfterDialogueSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_restoreSceneAfterDialogueSequence(%p) (%d)", (const void *)script, stackPos(0));
	restoreSceneAfterDialogueSequence(stackPos(0));
	return 1;
}

int LoLEngine::olol_getItemInHand(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getItemInHand(%p))", (const void *)script);
	return _itemInHand;
}

int LoLEngine::olol_giveItemToMonster(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_giveItemToMonster(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (stackPos(0) == -1)
		return 0;
	giveItemToMonster(&_monsters[stackPos(0)], stackPos(1));
	return 1;
}

int LoLEngine::olol_loadLangFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadLangFile(%p) (%s)", (const void *)script, stackPosString(0));
	char filename[13];
	snprintf(filename, sizeof(filename), "%s.%s", stackPosString(0), _languageExt[_lang]);
	if (_levelLangFile)
		delete[] _levelLangFile;
	_levelLangFile = _res->fileData(filename, 0);
	return 1;
}

int LoLEngine::olol_playSoundEffect(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playSoundEffect(%p) (%d)", (const void *)script, stackPos(0));
	snd_playSoundEffect(stackPos(0), 255);
	return 1;
}

int LoLEngine::olol_stopTimScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_stopTimScript(%p) (%d)", (const void *)script, stackPos(0));
	_tim->stopAllFuncs(_activeTim[stackPos(0)]);
	return 1;
}

int LoLEngine::olol_getWallFlags(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getWallFlags(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return _wllWallFlags[_levelBlockProperties[stackPos(0)].walls[stackPos(1) & 3]];
}

int LoLEngine::olol_playCharacterScriptChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playCharacterScriptChat(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	snd_stopSpeech(1);
	updatePortraits();
	return playCharacterScriptChat(stackPos(0), stackPos(1), 1, getLangString(stackPos(2)), script, 0, 3);
}

int LoLEngine::olol_loadSoundFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadSoundFile(%p) (%d)", (const void *)script, stackPos(0));
	snd_loadSoundFile(stackPos(0));
	return 1;
}

int LoLEngine::olol_stopCharacterSpeech(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_stopCharacterSpeech(%p)", (const void *)script);
	snd_stopSpeech(1);
	updatePortraits();
	return 1;
}

int LoLEngine::olol_setPaletteBrightness(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setPaletteBrightness(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	uint16 old = _brightness;
	_brightness = stackPos(0);
	if (stackPos(1) == 1)
		setPaletteBrightness(_screen->_currentPalette, stackPos(0), _lampOilStatus);
	return old;
}

int LoLEngine::olol_printMessage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_printMessage(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));
	int snd = stackPos(2);
	_txt->printMessage(stackPos(0), getLangString(stackPos(1)), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));

	if (snd)
		snd_playSoundEffect(snd, 255);

	return 1;
}

int LoLEngine::olol_playDialogueTalkText(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playDialogueTalkText(%p) (%d)", (const void *)script, stackPos(0));
	int track = stackPos(0);

	if (!snd_playCharacterSpeech(track, 0, 0) || textEnabled()) {
		char *s = getLangString(track);
		_txt->printDialogueText(4, s, script, 0, 1);
	}

	return 1;
}

int LoLEngine::olol_checkForMonsterMode1(EMCState *script) {
	for (int i = 0; i < 30; i++) {
		if (stackPos(0) != _monsters[i].type && stackPos(0) != -1)
			continue;
		return (_monsters[i].mode == 1) ? 0 : 1;
	}
	return 1;
}

int LoLEngine::olol_setNextFunc(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setNextFunc(%p) (%d)", (const void *)script, stackPos(0));
	_nextScriptFunc = stackPos(0);
	return 1;
}

int LoLEngine::olol_setDoorState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setDoorState(%p) (%d)", (const void *)script, stackPos(0));
	_emcDoorState = stackPos(0);
	return _emcDoorState;
}

int LoLEngine::olol_assignCustomSfx(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_assignCustomSfx(%p) (%s, %d)", (const void *)script, stackPosString(0), stackPos(1));
	const char *c = stackPosString(0);
	int i = stackPos(1);

	if (!c || i > 250)
		return 0;

	uint16 t = READ_LE_UINT16(&_ingameSoundIndex[i << 1]);
	if (t == 0xffff)
		return 0;

	strcpy(_ingameSoundList[t], c);

	return 0;
}

int LoLEngine::olol_resetPortraitsArea(EMCState *script) {
	resetPortraitsArea();
	return 1;
}

int LoLEngine::olol_enableSysTimer(EMCState *script) {
	_hideInventory = 0;
	enableSysTimer(2);
	return 1;
}


#pragma mark -

int LoLEngine::tlol_setupPaletteFade(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::t2_playSoundEffect(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	_screen->getFadeParams(_screen->getPalette(0), param[0], _tim->_palDelayInc, _tim->_palDiff);
	_tim->_palDelayAcc = 0;
	return 1;
}

int LoLEngine::tlol_loadPalette(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_loadPalette(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	const char *palFile = (const char *)(tim->text + READ_LE_UINT16(tim->text + (param[0]<<1)));
	_res->loadFileToBuf(palFile, _screen->getPalette(0), 768);
	return 1;
}

int LoLEngine::tlol_setupPaletteFadeEx(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_setupPaletteFadeEx(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	memcpy(_screen->getPalette(0), _screen->getPalette(1), 768);

	_screen->getFadeParams(_screen->getPalette(0), param[0], _tim->_palDelayInc, _tim->_palDiff);
	_tim->_palDelayAcc = 0;
	return 1;
}

int LoLEngine::tlol_processWsaFrame(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_processWsaFrame(%p, %p) (%d, %d, %d, %d, %d)",
		(const void*)tim, (const void*)param, param[0], param[1], param[2], param[3], param[4]);
	TIMInterpreter::Animation *anim = (TIMInterpreter::Animation *)tim->wsa[param[0]].anim;
	const int frame = param[1];
	const int x2 = param[2];
	const int y2 = param[3];
	const int factor = MAX<int>(0, (int16)param[4]);

	const int x1 = anim->x;
	const int y1 = anim->y;

	int w1 = anim->wsa->width();
	int h1 = anim->wsa->height();
	int w2 = (w1 * factor) / 100;
	int h2 = (h1 * factor) / 100;

	anim->wsa->setDrawPage(2);
	anim->wsa->setX(x1);
	anim->wsa->setY(y1);
	anim->wsa->displayFrame(frame, anim->wsaCopyParams & 0xF0FF, 0, 0);
	_screen->wsaFrameAnimationStep(x1, y1, x2, y2, w1, h1, w2, h2, 2, 8, 0);
	_screen->checkedPageUpdate(8, 4);
	_screen->updateScreen();

	return 1;
}

int LoLEngine::tlol_displayText(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_displayText(%p, %p) (%d, %d)", (const void*)tim, (const void*)param, param[0], (int16)param[1]);
	_tim->displayText(param[0], param[1]);
	return 1;
}

int LoLEngine::tlol_initDialogueSequence(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_initDialogueSequence(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	initDialogueSequence(param[0]);
	return 1;
}

int LoLEngine::tlol_restoreSceneAfterDialogueSequence(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_restoreSceneAfterDialogueSequence(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	restoreSceneAfterDialogueSequence(param[0]);
	return 1;
}

int LoLEngine::tlol_giveItem(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_giveItem(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	int item = makeItem(param[0], param[1], param[2]);
	if (addItemToInventory(item))
		return 1;

	deleteItem(item);
	return 0;
}

int LoLEngine::tlol_setPartyPosition(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_setPartyPosition(%p, %p) (%d, %d)", (const void*)tim, (const void*)param, param[0], param[1]);
	if (param[0] == 1) {
		_currentDirection = param[1];
	} else if (param[0] == 0) {
		_currentBlock = param[1];
		calcCoordinates(_partyPosX, _partyPosY, _currentBlock, 0x80, 0x80);
	}

	return 1;
}

int LoLEngine::tlol_fadeClearWindow(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_fadeClearWindow(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	uint8 *tmp = 0;

	switch (param[0]) {
		case 0:
			_screen->fadeClearSceneWindow(10);
			break;

		case 1:
			tmp = _screen->getPalette(3);
			memcpy(tmp + 0x180, _screen->_currentPalette + 0x180, 0x180);
			_screen->loadSpecialColours(tmp);
			_screen->fadePalette(tmp, 10);
			_screen->_fadeFlag = 0;
			break;

		case 2:
			_screen->fadeToBlack(10);
			break;

		case 3:
			tmp = _screen->getPalette(3);
			_screen->loadSpecialColours(tmp);
			_screen->fadePalette(tmp, 10);
			_screen->_fadeFlag = 0;
			break;

		case 4:
			if (_screen->_fadeFlag != 2)
				_screen->fadeClearSceneWindow(10);
			gui_drawPlayField();
			setPaletteBrightness(_screen->_currentPalette, _brightness, _lampOilStatus);
			_screen->_fadeFlag = 0;
			break;

		case 5:
			tmp = _screen->getPalette(3);
			_screen->loadSpecialColours(tmp);
			_screen->fadePalette(_screen->getPalette(1), 10);
			_screen->_fadeFlag = 0;
			break;

		default:
			break;
	}

	return 1;
}

int LoLEngine::tlol_update(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_update(%p, %p)", (const void*)tim, (const void*)param);
	update();
	return 1;
}

int LoLEngine::tlol_loadSoundFile(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_loadSoundFile(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	snd_loadSoundFile(param[0]);
	return 1;
}

int LoLEngine::tlol_playMusicTrack(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_playMusicTrack(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	snd_playTrack(param[0]);
	return 1;
}

int LoLEngine::tlol_playDialogueTalkText(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_playDialogueTalkText(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	if (!snd_playCharacterSpeech(param[0], 0, 0) || textEnabled())
		_txt->printDialogueText(4, getLangString(param[0]), 0, param, 1);
	return 1;
}

int LoLEngine::tlol_playSoundEffect(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_playSoundEffect(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	snd_playSoundEffect(param[0], -1);
	return 1;
}

#pragma mark -

typedef Common::Functor1Mem<EMCState*, int, LoLEngine> OpcodeV2;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV2(this, &LoLEngine::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV2(this, 0))

typedef Common::Functor2Mem<const TIM *, const uint16 *, int, LoLEngine> TIMOpcodeLoL;
#define SetTimOpcodeTable(x) timTable = &x;
#define OpcodeTim(x) timTable->push_back(new TIMOpcodeLoL(this, &LoLEngine::x))
#define OpcodeTimUnImpl() timTable->push_back(new TIMOpcodeLoL(this, 0))

void LoLEngine::setupOpcodeTable() {
	Common::Array<const Opcode*> *table = 0;

	SetOpcodeTable(_opcodes);
	// 0x00
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_drawScene);
	Opcode(o1_getRand);

	// 0x04
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_delay);
	Opcode(olol_setGameFlag);

	// 0x08
	Opcode(olol_testGameFlag);
	Opcode(olol_loadLevelGraphics);
	Opcode(olol_loadCmzFile);
	Opcode(olol_loadMonsterShapes);

	// 0x0C
	Opcode(olol_deleteHandItem);
	Opcode(olol_allocItemPropertiesBuffer);
	Opcode(olol_setItemProperty);
	Opcode(olol_makeItem);

	// 0x10
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_getItemPara);
	Opcode(olol_getCharacterStat);

	// 0x14
	Opcode(olol_setCharacterStat);
	Opcode(olol_loadLevelShapes);
	Opcode(olol_closeLevelShapeFile);
	OpcodeUnImpl();

	// 0x18
	Opcode(olol_loadDoorShapes);
	Opcode(olol_initAnimStruct);
	OpcodeUnImpl();
	Opcode(olol_freeAnimStruct);

	// 0x1C
	Opcode(olol_getDirection);
	OpcodeUnImpl();
	Opcode(olol_setMusicTrack);
	OpcodeUnImpl();

	// 0x20
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_clearDialogueField);
	OpcodeUnImpl();

	// 0x24
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x28
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x2C
	OpcodeUnImpl();
	Opcode(olol_getUnkArrayVal);
	Opcode(olol_setUnkArrayVal);
	Opcode(olol_getGlobalVar);

	// 0x30
	Opcode(olol_setGlobalVar);
	Opcode(olol_triggerDoorSwitch);
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x34
	OpcodeUnImpl();
	Opcode(olol_mapShapeToBlock);
	Opcode(olol_resetBlockShapeAssignment);
	OpcodeUnImpl();

	// 0x38
	Opcode(olol_initMonster);
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x3C
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_loadMonsterProperties);

	// 0x40
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x44
	Opcode(olol_moveMonster);
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x48
	Opcode(olol_setScriptTimer);
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x4C
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_loadTimScript);
	Opcode(olol_runTimScript);

	// 0x50
	Opcode(olol_releaseTimScript);
	Opcode(olol_initDialogueSequence);
	Opcode(olol_restoreSceneAfterDialogueSequence);
	Opcode(olol_getItemInHand);

	// 0x54
	OpcodeUnImpl();
	Opcode(olol_giveItemToMonster);
	Opcode(olol_loadLangFile);
	Opcode(olol_playSoundEffect);

	// 0x58
	OpcodeUnImpl();
	Opcode(olol_stopTimScript);
	Opcode(olol_getWallFlags);
	OpcodeUnImpl();

	// 0x5C
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_playCharacterScriptChat);
	OpcodeUnImpl();

	// 0x60
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_loadSoundFile);

	// 0x64
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x68
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_stopCharacterSpeech);
	Opcode(olol_setPaletteBrightness);

	// 0x6C
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_printMessage);

	// 0x70
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x74
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x78
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_playDialogueTalkText);
	Opcode(olol_checkForMonsterMode1);

	// 0x7C
	Opcode(olol_setNextFunc);
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x80
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x84
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_setDoorState);
	OpcodeUnImpl();

	// 0x88
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x8C
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x90
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x94
	Opcode(olol_assignCustomSfx);
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x98
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_resetPortraitsArea);
	Opcode(olol_enableSysTimer);

	// 0x9C
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0xA0
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0xA4
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0xA8
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0xAC
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0xB0
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0xB4
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0xB8
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0xBC
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	Common::Array<const TIMOpcode*> *timTable = 0;
	SetTimOpcodeTable(_timIntroOpcodes);

	// 0x00
	OpcodeTim(tlol_setupPaletteFade);
	OpcodeTimUnImpl();
	OpcodeTim(tlol_loadPalette);
	OpcodeTim(tlol_setupPaletteFadeEx);

	// 0x04
	OpcodeTim(tlol_processWsaFrame);
	OpcodeTim(tlol_displayText);
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();

	SetTimOpcodeTable(_timIngameOpcodes);

	// 0x00
	OpcodeTim(tlol_initDialogueSequence);
	OpcodeTim(tlol_restoreSceneAfterDialogueSequence);
	OpcodeTimUnImpl();
	OpcodeTim(tlol_giveItem);

	// 0x04
	OpcodeTim(tlol_setPartyPosition);
	OpcodeTim(tlol_fadeClearWindow);
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();

	// 0x08
	OpcodeTimUnImpl();
	OpcodeTim(tlol_update);
	OpcodeTimUnImpl();
	OpcodeTim(tlol_loadSoundFile);

	// 0x0C
	OpcodeTim(tlol_playMusicTrack);
	OpcodeTim(tlol_playDialogueTalkText);
	OpcodeTim(tlol_playSoundEffect);
	OpcodeTimUnImpl();

	// 0x10
	OpcodeTimUnImpl();
}

} // end of namespace Kyra

#endif // ENABLE_LOL

