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
	if (_sceneUpdateRequired)
		return true;

	for (int i = 0; i < 15; i++) {
		if (_scriptExecutedFuncs[i] == func) {
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
	if (!stackPos(0))
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
	tmp->unk5 = stackPos(3);
	tmp->itemScriptFunc = stackPos(4);
	tmp->unk8 = stackPos(5);
	tmp->unk9 = stackPos(6);
	tmp->unkA = stackPos(7);
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
		return i->p_1a;
	case 2:
		return i->p_1b;
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
		c->field_37 = e;
		break;

	case 10:
		c->items[d] = 0;
		break;

	case 11:
		c->field_66[d] = e;
		break;

	case 12:
		c->field_27[d] = e;
		break;

	case 13:
		if (d & 0x80)
			c->field_25 = e;
		else
			c->field_17[d] = e;
		break;

	case 14:
		c->field_69[d] = e;
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
	if (initTimAnimStruct(stackPos(1), stackPosString(0), stackPos(2), stackPos(3), stackPos(4), stackPos(5)))
		return 1;
	return 0;
}

int LoLEngine::olol_freeAnimStruct(EMCState *script) {
	if (_tim->freeAnimStruct(stackPos(0)))
		return 1;
	return 0;
}

int LoLEngine::olol_setMusicTrack(EMCState *script) {
	_curMusicTheme = stackPos(0);
	return 1;
}

int LoLEngine::olol_getUnkArrayVal(EMCState *script) {
	return _unkEMC46[stackPos(0)];
}

int LoLEngine::olol_setUnkArrayVal(EMCState *script) {
	_unkEMC46[stackPos(0)] = stackPos(1);
	return 1;
}

int LoLEngine::olol_setGlobalVar(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setGlobalVar(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	//uint16 a = stackPos(1);
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
		_updateFlags = b;
		if (b == 1) {
			if (!textEnabled() || !(_hideControls & 2))
				charCallback4(1);
			removeUnkFlags(2);
		} else {
			setUnkFlags(2);
		}
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

int LoLEngine::olol_loadMonsterProperties(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadMonsterProperties(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
		(const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5),
		stackPos(6), stackPos(7), stackPos(8), stackPos(9), stackPos(10), stackPos(11), stackPos(12), stackPos(13),
		stackPos(14), stackPos(15), stackPos(16), stackPos(17), stackPos(18), stackPos(19), stackPos(20),
		stackPos(21), stackPos(22), stackPos(23), stackPos(24), stackPos(25), stackPos(26),	stackPos(27),
		stackPos(28), stackPos(29), stackPos(30), stackPos(31), stackPos(32), stackPos(33), stackPos(34),
		stackPos(35), stackPos(36), stackPos(37), stackPos(38), stackPos(39), stackPos(40), stackPos(41));

	MonsterProperty *l = &_monsterProperties[stackPos(0)];
	l->id = stackPos(1) & 0xff;

	int shpWidthMax = 0;

	for (int i = 0; i < 16; i++) {
		uint8 m = _monsterShapes[(l->id << 4) + i][3];
		if (m > shpWidthMax)
			shpWidthMax = m;	
	}

	l->maxWidth = shpWidthMax;

	l->unk[0] = (stackPos(2) << 8) / 100;
	l->unk[1] = 256;
	l->unk[2] = (stackPos(3) << 8) / 100;
	l->unk[3] = stackPos(4);
	l->unk[4] = (stackPos(5) << 8) / 100;
	l->unk[5] = (stackPos(6) << 8) / 100;
	l->unk[6] = (stackPos(7) << 8) / 100;
	l->unk[7] = (stackPos(8) << 8) / 100;
	l->unk[8] = 0;

	for (int i = 0; i < 8; i++) {
		l->unk2[i] = stackPos(9 + i);
		l->unk3[i] = (stackPos(17 + i) << 8) / 100;
	}

	l->pos = &l->unk[0];
	l->unk4[0] = stackPos(25);
	l->unk4[1] = stackPos(26);
	l->b = 1;
	l->unk5[0] = stackPos(27);
	l->unk5[1] = stackPos(28);
	// FIXME???
	l->unk5[1] = stackPos(29);
	//

	for (int i = 0; i < 5; i++)
		l->unk6[2 + i] = stackPos(30 + i);

	for (int i = 0; i < 2; i++) {
		l->unk7[i] = stackPos(35 + i);
		l->unk7[i + 2] = stackPos(37 + i);
	}

	for (int i = 0; i < 3; i++)
		l->unk8[2 + i] = stackPos(39 + i);

	return 1;
}

int LoLEngine::olol_loadTimScript(EMCState *script) {
	if (_activeTim[stackPos(0)])
		return 1;	
	char file[13];
	snprintf(file, sizeof(file), "%s.TIM", stackPosString(1));
	_activeTim[stackPos(0)] = _tim->load(file, &_timIngameOpcodes);
	return 1;
}

int LoLEngine::olol_runTimScript(EMCState *script) {
	return _tim->exec(_activeTim[stackPos(0)], stackPos(1));
}

int LoLEngine::olol_releaseTimScript(EMCState *script) {
	_tim->unload(_activeTim[stackPos(0)]);
	return 1;
}

int LoLEngine::olol_initDialogueSequence(EMCState *script) {
	initDialogueSequence(stackPos(0));
	return 1;
}

int LoLEngine::olol_restoreSceneAfterDialogueSequence(EMCState *script) {
	restoreSceneAfterDialogueSequence(stackPos(0));
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

int LoLEngine::olol_stopTimScript(EMCState *script) {
	_tim->stopAllFuncs(_activeTim[stackPos(0)]);
	return 1;
}

int LoLEngine::olol_loadSoundFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadSoundFile(%p) (%d)", (const void *)script, stackPos(0));
	snd_loadSoundFile(stackPos(0));
	return 1;
}

int LoLEngine::olol_setPaletteBrightness(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setPaletteBrightness(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	uint16 old = _brightness;
	_brightness = stackPos(0);
	if (stackPos(1) == 1)
		_screen->setPaletteBrightness(_screen->_currentPalette, stackPos(0), _lampOilStatus);
	return old;
}

int LoLEngine::olol_playDialogueTalkText(EMCState *script) {
	int track = stackPos(0);
	
	if (!snd_playCharacterSpeech(track, 0, 0) || textEnabled()) {
		char *s = getLangString(track);
		_dlg->play(4, s, script, 0, 1);
	}

	return 1;
}

int LoLEngine::olol_setNextFunc(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setNextFunc(%p) (%d)", (const void *)script, stackPos(0));
	_nextScriptFunc = stackPos(0);
	return 1;
}

int LoLEngine::olol_assignCustomSfx(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_assignCustomSfx(%p) (%s, %d)", (const void *)script, stackPosString(0), stackPos(1));
	const char *c = stackPosString(0);
	int i = stackPos(1);

	if (!c || i > 250)
		return 0;

	if (_ingameSoundIndex[i] == 0xffff)
		return 0;

	strcpy(_ingameSoundList[_ingameSoundIndex[i]], c);

	return 0;
}

#pragma mark -

TIMInterpreter::Animation *LoLEngine::initTimAnimStruct(int index, const char *filename, int x, int y, uint16 copyPara, uint16 wsaFlags) {
	TIMInterpreter::Animation *a = _tim->initAnimStructIntern(index, filename, x, y, copyPara, wsaFlags);

	_tim->setWsaDrawPage2(0);

	if (wsaFlags & 1) {
		if (_screen->_fadeFlag != 1)
			_screen->fadeClearSceneWindow(10);
		memcpy(_screen->getPalette(3) + 384, _screen->_currentPalette + 384, 384);
	} else if (wsaFlags & 2) {
		_screen->fadeToBlack(10);
	}

	if (wsaFlags & 7) {
		_screen->hideMouse();
		a->wsa->setDrawPage(0);
		a->wsa->setX(x);
		a->wsa->setY(y);
		a->wsa->displayFrame(0, 0);
		_screen->showMouse();
	}

	if (wsaFlags & 3) {
		_screen->loadSpecialColours(_screen->getPalette(3));
		_screen->fadePalette(_screen->getPalette(3), 10);
		_screen->_fadeFlag = 0;
	}

	return a;
}

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
	OpcodeUnImpl();
	Opcode(o1_getRand);

	// 0x04
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_setGameFlag);

	// 0x08
	Opcode(olol_testGameFlag);
	Opcode(olol_loadLevelGraphics);
	Opcode(olol_loadCmzFile);
	Opcode(olol_loadMonsterShapes);

	// 0x0C
	OpcodeUnImpl();
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
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_setMusicTrack);
	OpcodeUnImpl();

	// 0x20
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
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
	OpcodeUnImpl();

	// 0x30
	Opcode(olol_setGlobalVar);
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x34
	OpcodeUnImpl();
	Opcode(olol_mapShapeToBlock);
	Opcode(olol_resetBlockShapeAssignment);
	OpcodeUnImpl();

	// 0x38
	OpcodeUnImpl();
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
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x48
	OpcodeUnImpl();
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
	OpcodeUnImpl();

	// 0x54
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_loadLangFile);
	OpcodeUnImpl();

	// 0x58
	OpcodeUnImpl();
	Opcode(olol_stopTimScript);
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x5C
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
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
	OpcodeUnImpl();
	Opcode(olol_setPaletteBrightness);

	// 0x6C
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();

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
	OpcodeUnImpl();

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
	OpcodeUnImpl();
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
	OpcodeUnImpl();
	OpcodeUnImpl();

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
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();

	// 0x04
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();

	// 0x08
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();

	// 0x0C
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();

	// 0x10
	OpcodeTimUnImpl();
}

} // end of namespace Kyra

