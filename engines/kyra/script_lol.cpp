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

#include "common/endian.h"

namespace Kyra {

void LoLEngine::runInitScript(const char *filename, int func) {
	EMCData scriptData;
	EMCState scriptState;
	memset(&scriptData, 0, sizeof(EMCData));
	_emc->load(filename, &scriptData, &_opcodes);

	_emc->init(&scriptState, &scriptData);
	_emc->start(&scriptState, 0);
	while (_emc->isValid(&scriptState))
		_emc->run(&scriptState);

	if (func) {
		_emc->init(&scriptState, &scriptData);
		_emc->start(&scriptState, func);
		while (_emc->isValid(&scriptState))
			_emc->run(&scriptState);
	}

	_emc->unload(&scriptData);
}

void LoLEngine::runInfScript(const char *filename) {
	_emc->load(filename, &_scriptData, &_opcodes);
	runResidentScript(0x400, -1);
}

void LoLEngine::runResidentScript(int func, int reg0) {
	runResidentScriptCustom(func, reg0, -1, 0, 0, 0);
}

void LoLEngine::runResidentScriptCustom(int func, int reg0, int reg1, int reg2, int reg3, int reg4) {
	EMCState scriptState;
	memset(&scriptState, 0, sizeof(EMCState));

	if (!_scriptBoolSkipExec) {
		_emc->init(&scriptState, &_scriptData);
		_emc->start(&scriptState, func);

		scriptState.regs[0] = reg0;
		scriptState.regs[1] = reg1;
		scriptState.regs[2] = reg2;
		scriptState.regs[3] = reg3;
		scriptState.regs[4] = reg4;
		scriptState.regs[5] = func;
		scriptState.regs[6] = _unkScriptByte;

		while (_emc->isValid(&scriptState))
			_emc->run(&scriptState);
	}

	checkScriptUnk(func);
}

bool LoLEngine::checkScriptUnk(int func) {
	if (_boolScriptFuncDone)
		return true;

	for (int i = 0; i < 15; i++) {
		if (_scriptExecutedFuncs[i] == func) {
			_boolScriptFuncDone = true;
			return true;
		}
	}

	if (_currentBlock == func){
		_boolScriptFuncDone = true;
		return true;
	}

	return false;
}

int LoLEngine::o2_setGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_setGameFlag(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (stackPos(1))
		_gameFlags[stackPos(0) >> 4] |= (1 << (stackPos(0) & 0x0f));
	else
		_gameFlags[stackPos(0) >> 4] &= (~(1 << (stackPos(0) & 0x0f)));

	return 1;
}

int LoLEngine::o2_testGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_testGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	if (!stackPos(0))
		return 0;

	if (_gameFlags[stackPos(0) >> 4] & (1 << (stackPos(0) & 0x0f)))
		return 1;
	
	return 0;
}

int LoLEngine::o2_loadLevelGraphics(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_loadLevelGraphics(%p) (%s, %d, %d, %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	loadLevelGraphics(stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), (stackPos(5) == -1) ? 0 : stackPosString(5));
	return 1;
}

int LoLEngine::o2_loadCmzFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_loadCmzFile(%p) (%s)", (const void *)script, stackPosString(0));
	loadCmzFile(stackPosString(0));
	return 1;
}

int LoLEngine::o2_loadMonsterShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_loadMonsterShapes(%p) (%s, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2));
	loadMonsterShapes(stackPosString(0), stackPos(1), stackPos(2));
	return 1;
}

int LoLEngine::o2_allocItemPropertiesBuffer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_allocItemPropertiesBuffer(%p) (%d)", (const void *)script, stackPos(0));
	delete []_itemProperties;
	_itemProperties = new ItemProperty[stackPos(0)];
	return 1;
}

int LoLEngine::o2_setItemProperty(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_setItemProperty(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));
	ItemProperty *tmp = &_itemProperties[stackPos(0)];
	
	tmp->nameStringId = stackPos(1);
	tmp->shpIndex = stackPos(2);
	tmp->unk5 = stackPos(3);
	tmp->itemScriptFunc = stackPos(4);
	tmp->unk8 = stackPos(5);
	tmp->unk9 = stackPos(6);
	tmp->unkA = stackPos(7);
	tmp->flags = stackPos(8);
	tmp->unkB = stackPos(9);
	return 1;
}

int LoLEngine::o2_makeItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_makeItem(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	return makeItem(stackPos(0), stackPos(1), stackPos(2));
}

int LoLEngine::o2_getItemPara(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_getItemPara(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if(!stackPos(0))
		return 0;

	ItemInPlay *i = &_itemsInPlay[stackPos(0)];
	ItemProperty *p = &_itemProperties[i->itemPropertyIndex];

	switch(stackPos(1)) {
		case 0:
			return i->blockPropertyIndex;
		case 1:
			return i->unk7;
		case 2:
			return i->anonymous_4;
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
			return p->unk5;
		case 10:
			return p->itemScriptFunc;
		case 11:
		case 12:
		case 13:
			return p[stackPos(1)].unkB & 0x0f;
		case 14:
			return p->unkB;
		case 15:
			return i->shpCurFrame_flg & 0x1fff;
		case 16:
			return p->flags;
		case 17:
			return (p->unk9 << 8) | p->unk8;
		default:
			break;
	}

	return -1;
}

int LoLEngine::o2_getCharacterStat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_getCharacterStat(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	LoLCharacter *c = &_characters[stackPos(0)];
	int d = stackPos(2);

	switch(stackPos(1)) {
		case 0:
			return c->flags;
		case 1:
			return c->raceClassSex;
		case 2:
		case 3:
		case 4:
		default:
			break;
		case 5:
			return c->hitPointsCur;
		case 6:
			return c->hitPointsMax;
		case 7:
			return c->magicPointsCur;
		case 8:
			return c->magicPointsMax;
		case 9:
			return c->field_37;
		case 10:
			return c->items[d];
		case 11:
			return c->field_66[d] + c->field_69[d];
		case 12:
			return c->field_27[d];
		case 13:
			return (d & 0x80) ? c->field_25 : c->field_17[d];
		case 14:
			return c->field_69[d];
		case 15:
			return c->id;
	}

	return 0;
}

int LoLEngine::o2_setCharacterStat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_setCharacterStat(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	LoLCharacter *c = &_characters[stackPos(0)];
	int d = stackPos(2);
	int e = stackPos(3);

	switch(stackPos(1)) {
		case 0:
			c->flags = e;
		case 1:
			c->raceClassSex = e & 0x0f;
		case 2:
		case 3:
		case 4:
		default:
			break;
		case 5:
			//// TODO
			break;
		case 6:
			c->hitPointsMax = e;
		case 7:
			//// TODO
			break;
		case 8:
			c->magicPointsMax = e;
		case 9:
			c->field_37 = e;
		case 10:
			c->items[d] = 0;
		case 11:
			c->field_66[d] = e;
		case 12:
			c->field_27[d] = e;
		case 13:
			if (d & 0x80)
				c->field_25 = e;
			else
				c->field_17[d] = e;
		case 14:
			c->field_69[d] = e;
	}

	return 0;
}

int LoLEngine::o2_loadLevelShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_loadLevelShapes(%p) (%s, %s)", (const void *)script, stackPosString(0), stackPosString(1));
	loadLevelShpDat(stackPosString(0), stackPosString(1), true);
	return 1;
}

int LoLEngine::o2_closeLevelShapeFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_closeLevelShapeFile(%p) ()", (const void *)script);
	delete _lvlShpFileHandle;
	_lvlShpFileHandle = 0;
	return 1;
}

int LoLEngine::o2_loadDoorShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_loadDoorShapes(%p) (%s, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2));
	_screen->loadBitmap(stackPosString(0), 3, 3, 0);
	const uint8 *p = _screen->getCPagePtr(2);
	if (_doorShapes[0])
		delete []_doorShapes[0];
	_doorShapes[0] = _screen->makeShapeCopy(p, stackPos(1));
	if (_doorShapes[1])
		delete []_doorShapes[1];
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

int LoLEngine::o2_setGlobalVar(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_setGlobalVar(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	//uint16 a = stackPos(1);
	uint16 b = stackPos(2);
	
	switch (stackPos(0)) {
		case 0:
			_currentBlock = b;
			setLF1(_unkCmzU1, _unkCmzU2, _currentBlock, 0x80, 0x80);
			setLF2(_currentBlock);			
			break;
		case 1:
			_currentDirection = b;
			break;
		case 2:
			_currentLevel = b & 0xff;
			break;
		case 3:
			break;
		case 4:
			_brightness = b & 0xff;
			break;
		case 5:
			_credits = b;
			break;
		case 6:
			//TODO
			break;
		case 7:			
			break;
		case 8:
			_charFlagUnk = b;
			//TODO
			break;
		case 9:
			_lampStatusUnk = b & 0xff;
			break;
		case 10:
			_loadLevelFlag2 = b & 0xff;
			//TODO
			break;
		case 11:
			//TODO
			break;
		case 12:
			//TODO
			break;
		default:
			break;
	}

	return 1;
}

int LoLEngine::o2_mapShapeToBlock(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_mapShapeToBlock(%p) (%d)", (const void *)script, stackPos(0));
	return assignLevelShapes(stackPos(0));
}
	
int LoLEngine::o2_resetBlockShapeAssignment(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_resetBlockShapeAssignment(%p) (%d)", (const void *)script, stackPos(0));
	uint8 v = stackPos(0) & 0xff;
	memset(_wllShapeMap + 3, v, 5);
	memset(_wllShapeMap + 13, v, 5);
	return 1;
}

int LoLEngine::o2_loadLangFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_loadLangFile(%p) (%s)", (const void *)script, stackPosString(0));
	char filename[13];
	snprintf(filename, sizeof(filename), "%s.%s", stackPosString(0), _languageExt[_lang]);
	if (_levelLangFile)
		delete[] _levelLangFile;
	_levelLangFile = _res->fileData(filename, 0);
	return 1;
}

int LoLEngine::o2_playTrack(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_playTrack(%p) (%d)", (const void *)script, stackPos(0));
	snd_playTrack(stackPos(0));
	return 1;
}

int LoLEngine::o2_setPaletteBrightness(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_setPaletteBrightness(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	uint16 old = _brightness;
	_brightness = stackPos(0);
	if (stackPos(1) == 1)
		_screen->setPaletteBrightness(_screen->_currentPalette, stackPos(0), _lampOilStatus);
	return old;
}

int LoLEngine::o2_assignCustomSfx(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::o2_assignCustomSfx(%p) (%s, %d)", (const void *)script, stackPosString(0), stackPos(1));
	const char *c = stackPosString(0);
	int i = stackPos(1);

	if (!c || i > 250)
		return 0;

	if (_ingameSoundIndex[i] == 0xffff)
		return 0;

	strcpy(_ingameSoundList[_ingameSoundIndex[i]], c);

	return 0;
}

} // end of namespace Kyra


#endif // ENABLE_LOL

