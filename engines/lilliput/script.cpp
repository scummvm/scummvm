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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lilliput/lilliput.h"
#include "lilliput/script.h"
#include "common/debug.h"

#include "common/system.h"

namespace Lilliput {

LilliputScript::LilliputScript(LilliputEngine *vm) : _vm(vm), _currScript(NULL) {
	_byte10806 = 0;
	_lastRandomValue = 0;
	_scriptForVal = 0;
	_byte1881A = 0;
	_byte18823 = 0;
	_speechDisplaySpeed = 3;
	_speechTimer = 0;
	_word16F00_characterId = -1;
	_word129A3 = 0;
	_viewportCharacterTarget = -1;
	_heroismBarX = 0;
	_heroismBarBottomY = 0;
	_viewportPos.x = 0;
	_viewportPos.y = 0;
	_currentSpeechId = 0;
	_array129A5[0] = 0;
	_array129A5[1] = 1;
	_array129A5[2] = 2;
	_array129A5[3] = 3;
	_savedBuffer215Ptr = NULL;
	_word1825E = Common::Point(0, 0);

	for (int i = 0; i < 20; i++) {
		_interfaceHotspotStatus[i] = kHotspotOff;
		_interfaceButtonActivationDelay[i] = 0;
	}

	for (int i = 0; i < 32; i++)
		_array1813BPos[i] = Common::Point(0, 0);

	for (int i = 0; i < 40; i++) {
		_characterScriptEnabled[i] = 1;
		_characterMapPixelColor[i] = 15;
		_array10AB1[i] = 0;
		_array12811[i] = 16;
		_array12839[i] = -1;
		_characterTilePosX[i] = 0;
		_characterTilePosY[i] = 0;
		_array122C1[i] = 0;
	}

	for (int i = 0; i < 640; i++) {
		_array12311[i] = Common::Point(-1, -1);
	}

	for (int i = 0; i < 1600; i++)
		_array10B51[i] = 0;
}

LilliputScript::~LilliputScript() {
}

byte LilliputScript::handleOpcodeType1(int curWord) {
	debugC(2, kDebugScript, "handleOpcodeType1(0x%x)", curWord);
	switch (curWord) {
	case 0x0:
		return OC_checkCharacterGoalPos();
		break;
	case 0x1:
		return OC_comparePos();
		break;
	case 0x2:
		return OC_checkIsoMap3();
		break;
	case 0x3:
		return OC_compareCharacterVariable();
		break;
	case 0x4:
		return OC_CompareLastRandomValue();
		break;
	case 0x5:
		return OC_getRandom();
		break;
	case 0x6:
		return OC_for();
		break;
	case 0x7:
		return OC_compCurrentSpeechId();
		break;
	case 0x8:
		return OC_checkSaveFlag();
		break;
	case 0x9:
		return OC_compScriptForVal();
		break;
	case 0xA:
		return OC_sub174D8();
		break;
	case 0xB:
		return OC_CompareCharacterVariables();
		break;
	case 0xC:
		return OC_compareCoords_1();
		break;
	case 0xD:
		return OC_compareCoords_2();
		break;
	case 0xE:
		return OC_CompareDistanceFromCharacterToPositionWith();
		break;
	case 0xF:
		return OC_compareRandomCharacterId();
		break;
	case 0x10:
		return OC_IsCurrentCharacterIndex();
		break;
	case 0x11:
		return OC_sub175C8();
		break;
	case 0x12:
		return OC_sub17640();
		break;
	case 0x13:
		return OC_sub176C4();
		break;
	case 0x14:
		return OC_compWord10804();
		break;
	case 0x15:
		return OC_sub17766();
		break;
	case 0x16:
		return OC_sub17782();
		break;
	case 0x17:
		return OC_CompareMapValueWith();
		break;
	case 0x18:
		return OC_IsCharacterValid();
		break;
	case 0x19:
		return OC_compWord16EFE();
		break;
	case 0x1A:
		return OC_AreCurrentCharacterVar0AndVar1EqualsTo();
		break;
	case 0x1B:
		return OC_CurrentCharacterVar0Equals();
		break;
	case 0x1C:
		return OC_checkLastInterfaceHotspotIndexMenu13();
		break;
	case 0x1D:
		return OC_checkLastInterfaceHotspotIndexMenu2();
		break;
	case 0x1E:
		return OC_CompareNumberOfCharacterWithVar0Equals();
		break;
	case 0x1F:
		return OC_IsPositionInViewport();
		break;
	case 0x20:
		return OC_CompareGameVariables();
		break;
	case 0x21:
		return OC_skipNextOpcode();
		break;
	case 0x22:
		return OC_CurrentCharacterVar2Equals1();
		break;
	case 0x23:
		return OC_sub178D2();
		break;
	case 0x24:
		return OC_CharacterVariableAnd();
		break;
	case 0x25:
		return OC_IsCurrentCharacterVar0LessEqualThan();
		break;
	case 0x26:
		return OC_sub1790F();
		break;
	case 0x27:
		return OC_CurrentCharacterVar1Equals();
		break;
	case 0x28:
		return OC_isCurrentCharacterActive();
		break;
	case 0x29:
		return OC_CurrentCharacterVar3Equals1();
		break;
	case 0x2A:
		return OC_checkCharacterDirection();
		break;
	case 0x2B:
		return OC_checkLastInterfaceHotspotIndex();
		break;
	case 0x2C:
		return OC_checkSelectedCharacter();
		break;
	case 0x2D:
		return OC_checkDelayedReactivation();
		break;
	case 0x2E:
		return OC_sub179C2();
		break;
	case 0x2F:
		return OC_checkFunctionKeyPressed();
		break;
	case 0x30:
		return OC_checkCodeEntered();
		break;
	case 0x31:
		return OC_checkViewPortCharacterTarget();
		break;
	default:
		error("Unexpected opcode %d", curWord);
		break;
	}
}

void LilliputScript::handleOpcodeType2(int curWord) {
	debugC(2, kDebugScript, "handleOpcodeType2(0x%x)", curWord);
	switch (curWord) {
	case 0x0:
		OC_setWord18821();
		break;
	case 0x1:
		OC_ChangeIsoMap();
		break;
	case 0x2:
		OC_startSpeech();
		break;
	case 0x3:
		OC_getComputedVariantSpeech();
		break;
	case 0x4:
		OC_getRotatingVariantSpeech();
		break;
	case 0x5:
		OC_startSpeechIfMute();
		break;
	case 0x6:
		OC_getComputedVariantSpeechIfMute();
		break;
	case 0x7:
		OC_startSpeechIfSilent();
		break;
	case 0x8:
		OC_ComputeCharacterVariable();
		break;
	case 0x9:
		OC_getRandom_type2();
		break;
	case 0xA:
		OC_setCharacterPosition();
		break;
	case 0xB:
		OC_DisableCharacter();
		break;
	case 0xC:
		OC_saveAndQuit();
		break;
	case 0xD:
		OC_sub17B93();
		break;
	case 0xE:
		OC_startSpeech5();
		break;
	case 0xF:
		OC_resetByte1714E();
		break;
	case 0x10:
		OC_deleteSavegameAndQuit();
		break;
	case 0x11:
		OC_incScriptForVal();
		break;
	case 0x12:
		OC_sub17BA5();
		break;
	case 0x13:
		OC_setByte18823();
		break;
	case 0x14:
		OC_callScript();
		break;
	case 0x15:
		OC_callScriptAndReturn();
		break;
	case 0x16:
		OC_setCurrentScriptCharacterPos();
		break;
	case 0x17:
		OC_initScriptFor();
		break;
	case 0x18:
		OC_sub17AE1();
		break;
	case 0x19:
		OC_sub17AEE();
		break;
	case 0x1A:
		OC_setWord10804();
		break;
	case 0x1B:
		OC_sub17C0E();
		break;
	case 0x1C:
		OC_sub17C55();
		break;
	case 0x1D:
		OC_sub17C76();
		break;
	case 0x1E:
		OC_setCurrentCharacter();
		break;
	case 0x1F:
		OC_sub17C8B();
		break;
	case 0x20:
		OC_sub17CA2();
		break;
	case 0x21:
		OC_sub17CB9();
		break;
	case 0x22:
		OC_sub17CD1();
		break;
	case 0x23:
		OC_resetWord16EFE();
		break;
	case 0x24:
		OC_enableCurrentCharacterScript();
		break;
	case 0x25:
		OC_IncCurrentCharacterVar1();
		break;
	case 0x26:
		OC_sub17D23();
		break;
	case 0x27:
		OC_sub17E6D();
		break;
	case 0x28:
		OC_changeCurrentCharacterSprite();
		break;
	case 0x29:
		OC_sub17E99();
		break;
	case 0x2A:
		OC_sub17EC5();
		break;
	case 0x2B:
		OC_setCharacterDirectionTowardsPos();
		break;
	case 0x2C:
		OC_turnCharacterTowardsAnother();
		break;
	case 0x2D:
		OC_sub17F4F();
		break;
	case 0x2E:
		OC_scrollAwayFromCharacter();
		break;
	case 0x2F:
		OC_skipNextVal();
		break;
	case 0x30:
		OC_setCurrentCharacterVar6();
		break;
	case 0x31:
		OC_sub17FDD();
		break;
	case 0x32:
		OC_setCharacterScriptEnabled();
		break;
	case 0x33:
		OC_setCurrentCharacterVar2();
		break;
	case 0x34:
		OC_SetCurrentCharacterVar2ToZero();
		break;
	case 0x35:
		OC_setCharacterProperties();
		break;
	case 0x36:
		OC_sub1805D();
		break;
	case 0x37:
		OC_sub18074();
		break;
	case 0x38:
		OC_setCurrentCharacterDirection();
		break;
	case 0x39:
		OC_setInterfaceHotspot();
		break;
	case 0x3A:
		OC_scrollViewPort();
		break;
	case 0x3B:
		OC_setViewPortPos();
		break;
	case 0x3C:
		OC_setCurrentCharacterAltitude();
		break;
	case 0x3D:
		OC_sub1817F();
		break;
	case 0x3E:
		OC_sub181BB();
		break;
	case 0x3F:
		OC_sub18213();
		break;
	case 0x40:
		OC_sub18252();
		break;
	case 0x41:
		OC_sub18260();
		break;
	case 0x42:
		OC_CharacterVariableAddOrRemoveFlag();
		break;
	case 0x43:
		OC_PaletteFadeOut();
		break;
	case 0x44:
		OC_PaletteFadeIn();
		break;
	case 0x45:
		OC_loadAndDisplayCUBESx_GFX();
		break;
	case 0x46:
		OC_setCurrentCharacterVar3();
		break;
	case 0x47:
		OC_setArray122C1();
		break;
	case 0x48:
		OC_sub18367();
		break;
	case 0x49:
		OC_enableCharacterScript();
		break;
	case 0x4A:
		OC_sub18387();
		break;
	case 0x4B:
		OC_setDebugFlag();
		break;
	case 0x4C:
		OC_setByte14837();
		break;
	case 0x4D:
		OC_waitForEvent();
		break;
	case 0x4E:
		OC_disableInterfaceHotspot();
		break;
	case 0x4F:
		OC_loadFileAerial();
		break;
	case 0x50:
		OC_startSpeechIfSoundOff();
		break;
	case 0x51:
		OC_sub1844A();
		break;
	case 0x52:
		OC_displayNumericCharacterVariable();
		break;
	case 0x53:
		OC_displayVGAFile();
		break;
	case 0x54:
		OC_startSpeechWithoutSpeeker();
		break;
	case 0x55:
		OC_displayTitleScreen();
		break;
	case 0x56:
		OC_initGameAreaDisplay();
		break;
	case 0x57:
		OC_displayCharacterStatBar();
		break;
	case 0x58:
		OC_initSmallAnim();
		break;
	case 0x59:
		OC_setCharacterHeroismBar();
		break;
	case 0x5A:
		OC_sub18690();
		break;
	case 0x5B:
		OC_setViewPortCharacterTarget();
		break;
	case 0x5C:
		OC_sub186A1();
		break;
	case 0x5D:
		OC_sub186E5_snd();
		break;
	case 0x5E:
		OC_sub1870A_snd();
		break;
	case 0x5F:
		OC_sub18725_snd();
		break;
	case 0x60:
		OC_sub18733_snd();
		break;
	case 0x61:
		OC_sub1873F_snd();
		break;
	case 0x62:
		OC_sub18746_snd();
		break;
	case 0x63:
		OC_sub1875D_snd();
		break;
	case 0x64:
		OC_setCharacterMapColor();
		break;
	case 0x65:
		OC_initGameAreaDisplay();
		break;
	default:
		error("Unknown opcode %d", curWord);
		break;
	}
}

static const OpCode opCodes1[] = {
	{ "OC_checkCharacterGoalPos", 1, kgetPosFromScript, kNone, kNone, kNone, kNone },
	{ "OC_comparePos", 2, kGetValue1, kgetPosFromScript, kNone, kNone, kNone },
	{ "OC_checkIsoMap3", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_compareCharacterVariable", 4, kGetValue1, kImmediateValue, kCompareOperation, kImmediateValue, kNone },
	{ "OC_CompareLastRandomValue", 2, kCompareOperation, kImmediateValue, kNone, kNone, kNone },
	{ "OC_getRandom", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_for", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
	{ "OC_compCurrentSpeechId", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_checkSaveFlag", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_compScriptForVal", 2, kCompareOperation, kImmediateValue, kNone, kNone, kNone },
	{ "OC_sub174D8", 2, kGetValue1, kGetValue1, kNone, kNone, kNone },
	{ "OC_CompareCharacterVariables", 5, kGetValue1, kImmediateValue, kCompareOperation, kGetValue1, kImmediateValue },
	{ "OC_compareCoords_1", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_compareCoords_2", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone },
	{ "OC_CompareDistanceFromCharacterToPositionWith", 3, kgetPosFromScript, kCompareOperation, kImmediateValue, kNone, kNone },
	{ "OC_compareRandomCharacterId", 3, kGetValue1, kCompareOperation, kImmediateValue, kNone, kNone },
	{ "OC_IsCurrentCharacterIndex", 1, kGetValue1, kNone, kNone, kNone, kNone },
	{ "OC_sub175C8", 2, kImmediateValue, kGetValue1, kNone, kNone, kNone },
	{ "OC_sub17640", 2, kImmediateValue, kGetValue1, kNone, kNone, kNone },
	{ "OC_sub176C4", 2, kImmediateValue, kGetValue1, kNone, kNone, kNone },
	{ "OC_compWord10804", 1, kGetValue1, kNone, kNone, kNone, kNone },
	{ "OC_sub17766", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub17782", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_CompareMapValueWith", 4, kgetPosFromScript, kImmediateValue, kImmediateValue, kCompareOperation, kNone },
	{ "OC_IsCharacterValid", 1, kGetValue1, kNone, kNone, kNone, kNone },
	{ "OC_compWord16EFE", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_AreCurrentCharacterVar0AndVar1EqualsTo", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
	{ "OC_CurrentCharacterVar0Equals", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_checkLastInterfaceHotspotIndexMenu13", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_checkLastInterfaceHotspotIndexMenu2", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_CompareNumberOfCharacterWithVar0Equals", 3, kImmediateValue, kCompareOperation, kImmediateValue, kNone, kNone },
	{ "OC_IsPositionInViewport", 1, kgetPosFromScript, kNone, kNone, kNone, kNone },
	{ "OC_CompareGameVariables", 2, kGetValue1, kGetValue1, kNone, kNone, kNone },
	{ "OC_skipNextOpcode", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_CurrentCharacterVar2Equals1", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_sub178D2", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone },
	{ "OC_CharacterVariableAnd", 3, kGetValue1, kImmediateValue, kImmediateValue, kNone, kNone },
	{ "OC_IsCurrentCharacterVar0LessEqualThan", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_sub1790F", 1, kGetValue1, kNone, kNone, kNone, kNone },
	{ "OC_CurrentCharacterVar1Equals", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_isCurrentCharacterActive", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_CurrentCharacterVar3Equals1", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_sub1796E", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone },
	{ "OC_checkLastInterfaceHotspotIndex", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
	{ "OC_checkSelectedCharacter", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_checkDelayedReactivation", 0, kNone, kNone, kNone, kNone, kNone },
	{ "OC_sub179C2", 1, kgetPosFromScript, kNone, kNone, kNone, kNone },
	{ "OC_checkFunctionKeyPressed", 1, kImmediateValue, kNone, kNone, kNone, kNone },
	{ "OC_checkCodeEntered", 3, kImmediateValue, kImmediateValue, kImmediateValue, kNone, kNone },
	{ "OC_checkViewPortCharacterTarget", 1, kGetValue1, kNone, kNone, kNone, kNone },
};


static const OpCode opCodes2[] = {
/* 0x00 */	{ "OC_setWord18821", 1, kGetValue1, kNone, kNone, kNone, kNone },
/* 0x01 */	{ "OC_ChangeIsoMap", 3, kgetPosFromScript, kImmediateValue, kImmediateValue, kNone, kNone },
/* 0x02 */	{ "OC_startSpeech", 1, kImmediateValue, kNone, kNone, kNone, kNone },
/* 0x03 */	{ "OC_getComputedVariantSpeech", 4, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kNone },
/* 0x04 */	{ "OC_getRotatingVariantSpeech", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, // todo
/* 0x05 */	{ "OC_startSpeechIfMute", 1, kImmediateValue, kNone, kNone, kNone, kNone },
/* 0x06 */	{ "OC_getComputedVariantSpeechIfMute", 4, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, // pb
/* 0x07 */	{ "OC_startSpeechIfSilent", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
/* 0x08 */	{ "OC_ComputeCharacterVariable", 4, kGetValue1, kImmediateValue, kComputeOperation, kImmediateValue, kNone },
/* 0x09 */	{ "OC_getRandom_type2", 3, kGetValue1, kImmediateValue, kImmediateValue, kNone, kNone },
/* 0x0a */	{ "OC_setCharacterPosition", 2, kGetValue1, kgetPosFromScript, kNone, kNone, kNone },
/* 0x0b */	{ "OC_DisableCharacter", 1, kGetValue1, kNone, kNone, kNone, kNone },
/* 0x0c */	{ "OC_saveAndQuit", 0, kNone, kNone, kNone, kNone, kNone },
/* 0x0d */	{ "OC_sub17B93", 1, kImmediateValue, kNone, kNone, kNone, kNone }, // todo : jump to other opcode
/* 0x0e */	{ "OC_startSpeech5", 0, kNone, kNone, kNone, kNone, kNone },  // todo
/* 0x0f */	{ "OC_resetByte1714E", 0, kNone, kNone, kNone, kNone, kNone },  
/* 0x10 */	{ "OC_deleteSavegameAndQuit", 0, kNone, kNone, kNone, kNone, kNone },  
/* 0x11 */	{ "OC_incScriptForVal", 0, kNone, kNone, kNone, kNone, kNone },  
/* 0x12 */	{ "OC_sub17BA5", 5, kGetValue1, kImmediateValue,kComputeOperation, kGetValue1, kImmediateValue },
/* 0x13 */	{ "OC_setByte18823", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone },  
/* 0x14 */	{ "OC_callScript", 2, kImmediateValue, kGetValue1, kNone, kNone, kNone },  // run script
/* 0x15 */	{ "OC_callScriptAndReturn", 2, kImmediateValue, kGetValue1, kNone, kNone, kNone },  // run script then stop
/* 0x16 */	{ "OC_setCurrentScriptCharacterPos", 1, kgetPosFromScript, kNone, kNone, kNone, kNone },  
/* 0x17 */	{ "OC_initScriptFor", 0, kNone, kNone, kNone, kNone, kNone },  
/* 0x18 */	{ "OC_sub17AE1", 1, kImmediateValue, kNone, kNone, kNone, kNone },  
/* 0x19 */	{ "OC_sub17AEE", 1, kImmediateValue, kNone, kNone, kNone, kNone },  
/* 0x1a */	{ "OC_setWord10804", 1, kGetValue1, kNone, kNone, kNone, kNone },  
/* 0x1b */	{ "OC_sub17C0E", 0, kNone, kNone, kNone, kNone, kNone },  
/* 0x1c */ 	{ "OC_sub17C55", 4, kGetValue1, kGetValue1, kImmediateValue, kImmediateValue, kNone }, 
/* 0x1d */	{ "OC_sub17C76", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x1e */	{ "OC_setCurrentCharacter", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x1f */	{ "OC_sub17C8B", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
/* 0x20 */	{ "OC_sub17CA2", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, 
/* 0x21 */	{ "OC_sub17CB9", 3, kImmediateValue, kGetValue1, kImmediateValue, kNone, kNone }, 
/* 0x22 */	{ "OC_sub17CD1", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },
/* 0x23 */	{ "OC_resetWord16EFE", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x24 */	{ "OC_enableCurrentCharacterScript", 1, kImmediateValue, kNone, kNone, kNone, kNone },   // stop script
/* 0x25 */	{ "OC_IncCurrentCharacterVar1", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x26 */	{ "OC_sub17D23", 2, kImmediateValue, kgetPosFromScript, kNone, kNone, kNone }, 
/* 0x27 */	{ "OC_sub17E6D", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x28 */	{ "OC_changeCurrentCharacterSprite", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, 
/* 0x29 */	{ "OC_sub17E99", 4, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, 
/* 0x2a */	{ "OC_sub17EC5", 4, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, 
/* 0x2b */	{ "OC_setCharacterDirectionTowardsPos", 1, kgetPosFromScript, kNone, kNone, kNone, kNone }, 
/* 0x2c */	{ "OC_turnCharacterTowardsAnother", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x2d */	{ "OC_sub17F4F", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x2e */	{ "OC_scrollAwayFromCharacter", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x2f */	{ "OC_skipNextVal", 1, kImmediateValue, kNone, kNone, kNone, kNone },
/* 0x30 */	{ "OC_setCurrentCharacterVar6", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x31 */	{ "OC_sub17FDD", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x32 */	{ "OC_setCharacterScriptEnabled", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x33 */	{ "OC_setCurrentCharacterVar2", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x34 */	{ "OC_SetCurrentCharacterVar2ToZero", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x35 */	{ "OC_setCharacterProperties", 5, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue }, 
/* 0x36 */	{ "OC_sub1805D", 5, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue }, 
/* 0x37 */	{ "OC_sub18074", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, 
/* 0x38 */	{ "OC_setCurrentCharacterDirection", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x39 */	{ "OC_setInterfaceHotspot", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, 
/* 0x3a */	{ "OC_scrollViewPort", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x3b */	{ "OC_setViewPortPos", 1, kgetPosFromScript, kNone, kNone, kNone, kNone }, 
/* 0x3c */	{ "OC_setCurrentCharacterAltitude", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x3d */	{ "OC_sub1817F", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone }, 
/* 0x3e */	{ "OC_sub181BB", 4, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, 
/* 0x3f */	{ "OC_sub18213", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x40 */	{ "OC_sub18252", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x41 */	{ "OC_sub18260", 2, kGetValue1, kgetPosFromScript, kNone, kNone, kNone }, // TODO
/* 0x42 */	{ "OC_CharacterVariableAddOrRemoveFlag", 4, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, 
/* 0x43 */	{ "OC_PaletteFadeOut", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x44 */	{ "OC_PaletteFadeIn", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x45 */	{ "OC_loadAndDisplayCUBESx_GFX", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x46 */	{ "OC_setCurrentCharacterVar3", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x47 */	{ "OC_setArray122C1", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x48 */	{ "OC_sub18367", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x49 */	{ "OC_enableCharacterScript", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone }, 
/* 0x4a */	{ "OC_sub18387", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone }, 
/* 0x4b */	{ "OC_setDebugFlag", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x4c */	{ "OC_setByte14837", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x4d */	{ "OC_waitForEvent", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x4e */	{ "OC_disableInterfaceHotspot", 2, kImmediateValue, kImmediateValue, kNone, kNone, kNone },  // TODO
/* 0x4f */	{ "OC_loadFileAerial", 1, kNone, kNone, kNone, kNone, kNone }, 
/* 0x50 */	{ "OC_startSpeechIfSoundOff", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x51 */	{ "OC_sub1844A", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone }, 
/* 0x52 */	{ "OC_displayNumericCharacterVariable", 5, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue }, 
/* 0x53 */	{ "OC_displayVGAFile", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x54 */	{ "OC_startSpeechWithoutSpeeker", 1, kImmediateValue, kNone, kNone, kNone, kNone },   // TODO
/* 0x55 */	{ "OC_displayTitleScreen", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x56 */	{ "OC_initGameAreaDisplay", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x57 */	{ "OC_displayCharacterStatBar", 6, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue}, 
/* 0x58 */	{ "OC_initSmallAnim", 11, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue, kImmediateValue }, 
/* 0x59 */	{ "OC_setCharacterHeroismBar", 4, kGetValue1, kImmediateValue, kImmediateValue, kImmediateValue, kNone }, 
/* 0x5a */	{ "OC_sub18690", 2, kGetValue1, kgetPosFromScript, kNone, kNone, kNone },  //TODO
/* 0x5b */	{ "OC_setViewPortCharacterTarget", 1, kGetValue1, kNone, kNone, kNone, kNone },
/* 0x5c */	{ "OC_sub186A1", 3, kGetValue1, kImmediateValue, kImmediateValue, kNone, kNone },  //TODO
/* 0x5d */	{ "OC_sub186E5_snd", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone }, 
/* 0x5e */	{ "OC_sub1870A_snd", 2, kgetPosFromScript, kImmediateValue, kNone, kNone, kNone }, 
/* 0x5f */	{ "OC_sub18725_snd", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x60 */	{ "OC_sub18733_snd", 1, kGetValue1, kNone, kNone, kNone, kNone }, 
/* 0x61 */	{ "OC_sub1873F_snd", 1, kgetPosFromScript, kNone, kNone, kNone, kNone }, 
/* 0x62 */	{ "OC_sub18746_snd", 1, kImmediateValue, kNone, kNone, kNone, kNone }, 
/* 0x63 */	{ "OC_sub1875D_snd", 0, kNone, kNone, kNone, kNone, kNone }, 
/* 0x64 */	{ "OC_setCharacterMapColor", 2, kGetValue1, kImmediateValue, kNone, kNone, kNone }, 
/* 0x65 */	{ "OC_initGameAreaDisplay", 0, kNone, kNone, kNone, kNone, kNone }
};

Common::String LilliputScript::getArgumentString(KValueType type, ScriptStream& script) {
	
	Common::String str;
	if (type == kImmediateValue) {
		str =  Common::String::format("0x%x", script.readUint16LE());
	} else if (type == kGetValue1) {
		int val = script.readUint16LE();
		if (val < 1000) { 
			str = Common::String::format("0x%x", val);
		} else if (val > 1004) { 
			str = Common::String::format("getValue1(0x%x)", val);
		} else if (val == 1000) {
			str = Common::String("_selectedCharacterId");
		} else if (val == 1001) {
			str = Common::String("characterIndex");
		} else if (val == 1002) {
			str = Common::String("_word16F00_characterId");
		} else if (val == 1003) {
			str = Common::String("_currentCharacterVariables[6]");
		} else if (val == 1004) {
			str = Common::String("_word10804");
		}
	} else if (type == kgetPosFromScript) {
		int curWord = script.readUint16LE();
		int tmpVal = curWord >> 8;
		switch(tmpVal) {
	case 0xFF:
		str = "(_rulesBuffer2_13[currentCharacter],_rulesBuffer2_14[currentCharacter])";
		break;
	case 0xFE: {
		int index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		str = Common::String::format("_vm->_rulesBuffer2_13[%d],_vm->_rulesBuffer2_14[%d]", index, index);
		break;
			   }
	case 0xFD:
		str = "_currentScriptCharacterPosition";
		break;
	case 0xFC: {
		int index = curWord & 0xFF;
		assert(index < 40);
		str = Common::String::format("(characterPositionTileX[%d], characterPositionTileY[%d])", index, index);
		break;
			   }
	case 0xFB: {
		str = "(characterPositionTileX[_word16F00_characterId], characterPositionTileY[_word16F00_characterId])";
		break;
			   }
	case 0xFA:
		str = Common::String::format("(_array10999PosX[currentCharacter], _array109C1PosY[currentCharacter])");
		break;
	case 0xF9:
		str = Common::String::format("(_currentCharacterVariables[4], _currentCharacterVariables[5])");
		break;
	case 0xF8: {
		int index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		str = Common::String::format("_vm->_rulesBuffer12Pos3[%d]", index);
	}
	case 0xF7: {
		str = Common::String::format("(_characterPositionTileX[_currentCharacterVariables[6]], _characterPositionTileY[_currentCharacterVariables[6]])");
		break;			 
	}
	case 0xF6:
		str = "_savedMousePosDivided";
		break;
	default:
		str = Common::String::format("(0x%x,0x%x)", curWord >> 8, curWord & 0xFF);
		break;
		}
	} else if (type == kCompareOperation) {
		int comp = script.readUint16LE();
		if (comp != '<' && comp != '>')
			comp = '=';
		str = Common::String::format("%c", comp);
	}
	else if (type == kComputeOperation) {
		int comp = script.readUint16LE();
		str = Common::String::format("%c", comp);
	}
	return str;
}

void LilliputScript::disasmScript(ScriptStream script) {
	
	while (!script.eos()) {
		uint16 val = script.readUint16LE();
		if (val == 0xFFF6) // end of script
			return;

		bool hasIf = false;

		if (val != 0xFFF8) {
			hasIf = true;
		}

		bool firstIf = true; 


		// check the conditions.
		while (val != 0xFFF8) {

			bool neg = false;

			if (val >= 1000) {
				val -= 1000;
				// negative condition
				neg = true;
			}

			// op code type 1
			assert(val < sizeof(opCodes1)/sizeof(OpCode));
			const OpCode *opCode = &opCodes1[val];
			const KValueType *opArgType = &opCode->_arg1;

			Common::String str;

			if (firstIf) {
				str = "if (";
				firstIf = false;
			} else {
				str = "    ";
			}
			if (neg)
				str += "not ";
			str += Common::String(opCode->_opName);
			str += "(";

			for (int p = 0; p < opCode->_numArgs; p++) {
				
				str += getArgumentString(*opArgType, script);
				if (p != opCode->_numArgs - 1)
					str += ", ";

				opArgType++;
			}
			str += ")";

			

			val = script.readUint16LE();

			if (val == 0xFFF8) {
				str += ")";
			}

			debugC(2, kDebugScript, "%s", str.c_str());
		}

		debugC(2, kDebugScript, "{ ");

		val = script.readUint16LE();

		while (val != 0xFFF7) {
			
			// op code type 2 
			assert(val < sizeof(opCodes2) / sizeof(OpCode));
			const OpCode *opCode = &opCodes2[val];
			const KValueType *opArgType = &opCode->_arg1;

			Common::String str;

			str = "    ";
			str += Common::String(opCode->_opName);
			str += "(";

			for (int p = 0; p < opCode->_numArgs; p++) {
				str += getArgumentString(*opArgType, script);

				if (p != opCode->_numArgs - 1)
					str += ", ";

				if (p < 4)
					opArgType++;
			}
			str += ");";

			debugC(2, kDebugScript, "%s", str.c_str());

			val = script.readUint16LE();
		}

		debugC(2, kDebugScript, "} ");
		debugC(2, kDebugScript, " ");
	}
}

int LilliputScript::handleOpcode(ScriptStream *script) {
	debugC(2, kDebugScript, "handleOpcode");

	_currScript = script;
	uint16 curWord = _currScript->readUint16LE();
	if (curWord == 0xFFF6)
		return 0xFF;

	for (; curWord != 0xFFF8; curWord = _currScript->readUint16LE()) {
		byte mask = 0; 
		if (curWord > 1000) {
			curWord -= 1000;
			mask = 1;
		}
		byte result = handleOpcodeType1(curWord);
		if ((result ^ mask) == 0) {
			do {
				curWord = _currScript->readUint16LE();
			} while (curWord != 0xFFF7);
			return 0;
		}
	}

	_vm->_handleOpcodeReturnCode = 1;

	for (;;) {
		curWord = _currScript->readUint16LE();
		if (curWord == 0xFFF7)
			return _vm->_handleOpcodeReturnCode;

		handleOpcodeType2(curWord);
	}
}

void LilliputScript::runScript(ScriptStream script) {
	debugC(1, kDebugScript, "runScript");

	_byte16F05_ScriptHandler = 1;
	
	while (handleOpcode(&script) != 0xFF)
		_vm->update();
}

void LilliputScript::runMenuScript(ScriptStream script) {
	debugC(1, kDebugScriptTBC, "runMenuScript");

	_byte16F05_ScriptHandler = 0;
	
	while (handleOpcode(&script) == 0)
		_vm->update();
}

byte LilliputScript::compareValues(int16 var1, uint16 oper, int16 var2) {
	debugC(2, kDebugScript, "compareValues(%d, %c, %d)", var1, oper & 0xFF, var2);

	switch (oper & 0xFF) {
	case '<':
		if (var1 < var2)
			return 1;
		break;
	case '>':
		if (var1 > var2)
			return 1;
		break;
	default:
		if (var1 == var2)
			return 1;
		break;
	}

	return 0;
}

void LilliputScript::computeOperation(byte *bufPtr, uint16 oper, int16 var3) {
	debugC(1, kDebugScript, "computeOperation(bufPtr, %c, %d)", oper & 0xFF, var3 & 0xFF);

	switch (oper & 0xFF) {
	case '=':
		bufPtr[0] = var3 & 0xFF;
		break;
	case '+': {
		int tmpVal = bufPtr[0] + var3;
		if (tmpVal > 0xFF)
			bufPtr[0] = 0xFF;
		else
			bufPtr[0] = (byte)tmpVal;
		}
		break;
	case '-': {
		int tmpVal = bufPtr[0] - var3;
		if (tmpVal < 0)
			bufPtr[0] = 0;
		else
			bufPtr[0] = (byte)tmpVal;
		}
		break;
	case '*': {
		int tmpVal = bufPtr[0] * var3;
		bufPtr[0] = tmpVal & 0xFF;
		}
		break;
	case '/': {
		if (var3 != 0)
			bufPtr[0] /= var3;
		}
		break;
	default: {
		warning("computeOperation : oper %d", oper);
		if (var3 != 0) {
			int tmpVal = bufPtr[0] / var3;
			if (tmpVal < 0)
				bufPtr[0] = 0xFF;
			else 
				bufPtr[0] = 0;
		}
		break;
		}
	}
}

void LilliputScript::enableCharacterScript(byte index, byte var1, byte *curBufPtr) {
	debugC(1, kDebugScript, "enableCharacterScript(%d, %d, curBufPtr)", index, var1);

	assert (index < 40);
	_characterScriptEnabled[index] = 1;
	curBufPtr[0] = var1;
	curBufPtr[1] = 0;
	curBufPtr[2] = 0;
	curBufPtr[3] = 0;
}

void LilliputScript::sub17B6C(int var1) {
	debugC(1, kDebugScript, "sub17B6C(%d)", var1);

	if (var1 == 0) {
		int curWord = 0;
		while (curWord != 0xFFF6)
			curWord = _currScript->readUint16LE();
		
		_currScript->seek(_currScript->pos() - 4);
		return;
	}

	++var1;
	int curVal = 0;
	int tmpVal;
	while (curVal < var1) {
		tmpVal = _currScript->readUint16LE();
		if (tmpVal == 0xFFF7)
			++curVal;
	}

	_currScript->seek(_currScript->pos() - 2);
}

void LilliputScript::sub16C86(int index, byte *buf) {
	debugC(1, kDebugScript, "sub16C86()");

	_array12811[index] = 0;

	for (int i = 0; i < 16; i++) {
		_array12311[(index * 16) + i] = Common::Point(buf[(2 * i) + 1], buf[2 * i]);
	}
}

void LilliputScript::sub16C5C(int index, int8 var3) {
	debugC(1, kDebugScript, "sub16C5C(%d, %d)", index, var3);

	assert(index < 40);
	_array12839[index] = var3;

	byte *buf = _vm->_rulesChunk1;
	if (var3 != 0) {
		int count = 0;
		while (count < var3) {
			if ((buf[0] == 0xFF) && (buf[1] == 0xFF))
				++count;
			buf += 2;
		}
	}

	sub16C86(index, buf);
}

void LilliputScript::checkSpeechAllowed(bool &forceReturnFl) {
	debugC(1, kDebugScript, "checkSpeechAllowed()");

	forceReturnFl = false;
	if ((!_vm->_displayMap) && (_vm->_characterRelativePositionX[_vm->_currentScriptCharacter] != -1))
		return;

	forceReturnFl = true;
	return;
}

void LilliputScript::formatSpeechString() {
	debugC(2, kDebugScript, "formatSpeechString()");

	int index = 0;
	int var2 = 0x100;
	int var1;

	for (;;) {
		var1 = _vm->_displayStringBuf[index++];
		if (var1 == 0)
			break;

		if (var1 == '|') {
			var2 &= 0xFF;
			++var2;
			continue;
		}

		var2 += 0x100;
		if ((var2 >> 8) < 61)
			continue;

		if ((var2 & 0xFF) == 1) {
			_vm->_displayStringBuf[index - 1] = 0;
			break;
		}

		--index;
		while (_vm->_displayStringBuf[index] != ' ')
			--index;

		_vm->_displayStringBuf[index] = '|';
		++var2;
		var2 &= 0xFF;
		++index;
	}
}

void LilliputScript::sub189B8() {
	debugC(2, kDebugScript, "sub189B8()");

	formatSpeechString();
	int index = 0;

	for (;;) {
		if (_vm->_displayStringBuf[index] == 0)
			break;
		++index;
	}

	index /= _speechDisplaySpeed;
	index += 4;
	_speechTimer = index;
	_vm->displaySpeechBubble();
	_vm->displaySpeech(_vm->_displayStringBuf);
}

void LilliputScript::decodePackedText(char *buf) {
	debugC(2, kDebugScript, "decodePackedText(buf)");

	// All the languages use the English dictionnary
	static const char *nounsArrayPtr = "I am |You are |you are |hou art |in the |is the |is a |in a |To the |to the |by |going |here |The|the|and |some |build|not |way|I |a |an |from |of |him|her|by |his |ing |tion|have |you|I've |can't |up |to |he |she |down |what|What|with|are |and|ent|ian|ome|ed |me|my|ai|it|is|of|oo|ea|er|es|th|we|ou|ow|or|gh|go|er|st|ee|th|sh|ch|ct|on|ly|ng|nd|nt|ty|ll|le|de|as|ie|in|ss|'s |'t |re|gg|tt|pp|nn|ay|ar|wh|";

	_vm->_displayStringIndex = 0;
	_byte1881A = 0;
	int index = 0;
	byte var1 = 0;
	for (;;) {
		var1 = buf[index];
		++index;
		if (var1 == ']')
			var1 = 0;

		if (var1 < 0x80) {
			if (var1 == '@') {
				var1 = buf[index];
				++index;
				if (var1 == '#') {
					_vm->numberToString(_byte18823);
				}
			} else {
				_vm->addCharToBuf(var1);
				if (var1 == 0)
					break;
			}
		} else {
			int nounIndex = 0;
			byte var3 = 0xFF - var1;
			for (int i = 0; i < var3; i++) {
				for (;;) {
					var1 = nounsArrayPtr[nounIndex];
					++nounIndex;

					if (var1 == '|')
						break;
				}
			}

			for (;;) {
				var1 = nounsArrayPtr[nounIndex];
				++nounIndex;

				if (var1 == '|')
					break;

				_vm->addCharToBuf(var1);
			}
		}
	}

	sub189B8();
}

int LilliputScript::getPackedStringStartRelativeIndex(int index) {
	debugC(2, kDebugScript, "getPackedStringStartRelativeIndex(%d)", index);
	
	int chunk4Index = _vm->_packedStringIndex[index];
	int result = 0;
	while (_vm->_packedStrings[chunk4Index + result] == 0x5B)
		++result;
	
	return result + 1;
}

// Part of the script decompiler
void LilliputScript::listAllTexts() {
	debugC(1, kDebugScript, "listAllTexts");

	for (int i = 0; i < _vm->_packedStringNumb; i++) {
		int index = _vm->_packedStringIndex[i];
		int variantCount = 0;
		while (_vm->_packedStrings[index + variantCount] == 0x5B)
			++variantCount ;
		int it = 0;
	
		/*if (variantCount != 0) {
			for (int j = 0; j < variantCount; j++) {
				decodePackedText(&_vm->_packedStrings[index + variantCount + it]);
				warning("Text 0x%x variant %d : %s", i, j, _vm->_displayStringBuf);
				do {
					++it;
				}
				while (_vm->_packedStrings[index + variantCount + it] != 0x5B);
			}
		} else {*/
			decodePackedText(&_vm->_packedStrings[index + variantCount]);
			debugC(1, kDebugScriptTBC, "Text 0x%x variant 0 : %s", i, _vm->_displayStringBuf);
		/* }*/ 
	}
}

void LilliputScript::startSpeech(int speechId) {
	debugC(2, kDebugScript, "startSpeech(%d)", speechId);

	if (speechId == -1)
		return;

	_currentSpeechId = speechId;

	int index = _vm->_packedStringIndex[speechId];
	int count = 0;
	while (_vm->_packedStrings[index + count] == '[')
		++count;

	int i = 0;
	if (count != 0) {
		int tmpVal = _vm->_rnd->getRandomNumber(count);
		if (tmpVal != 0) {
			for (int j = 0; j < tmpVal; j++) {
				do
					++i;
				while (_vm->_packedStrings[index + count + i] != ']');
				++i;
			}
		}
	}

	decodePackedText(&_vm->_packedStrings[index + count + i]);
}

int16 LilliputScript::getValue1() {
	debugC(2, kDebugScript, "getValue1()");

	int16 curWord = _currScript->readUint16LE();
	if (curWord < 1000)
		return curWord;

	switch (curWord) {
	case 1000:
		return _vm->_selectedCharacterId;
	case 1001:
		return _vm->_currentScriptCharacter;
	case 1002:
		return _word16F00_characterId;
	case 1003:
		return (int16)_vm->_currentCharacterVariables[6];
	case 1004:
		return _vm->_word10804;
	default:
		warning("getValue1: Unexpected large value %d", curWord);
		return curWord;
	}
}

Common::Point LilliputScript::getPosFromScript() {
	debugC(2, kDebugScript, "getPosFromScript()");

	int curWord = _currScript->readUint16LE();
	int tmpVal = curWord >> 8;
	switch(tmpVal) {
	case 0xFF:
		assert((_vm->_currentScriptCharacter >= 0) && (_vm->_currentScriptCharacter < 40));
		return Common::Point(_vm->_rulesBuffer2_13_posX[_vm->_currentScriptCharacter], _vm->_rulesBuffer2_14_posY[_vm->_currentScriptCharacter]);
	case 0xFE: {
		int8 index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		return Common::Point(_vm->_rulesBuffer2_13_posX[index], _vm->_rulesBuffer2_14_posY[index]);
		}
	case 0xFD:
		return _vm->_currentScriptCharacterPos;
	case 0xFC: {
		int8 index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		int16 x = _vm->_characterPositionX[index] >> 3;
		int16 y = _vm->_characterPositionY[index] >> 3;

		return Common::Point(x, y);
		}
	case 0xFB: {
		int index = _word16F00_characterId;
		assert((index >= 0) && (index < 40));
		int16 x = _vm->_characterPositionX[index] >> 3;
		int16 y = _vm->_characterPositionY[index] >> 3;

		return Common::Point(x, y);
		}
	case 0xFA:
		return Common::Point(_vm->_array10999PosX[_vm->_currentScriptCharacter], _vm->_array109C1PosY[_vm->_currentScriptCharacter]);
	case 0xF9:
		return Common::Point(_vm->_currentCharacterVariables[4], _vm->_currentCharacterVariables[5]);
	case 0xF8: {
		int8 index = curWord & 0xFF;
		assert((index >= 0) && (index < 40));
		return _vm->_rulesBuffer12Pos3[index];
		}
	case 0xF7: {
		int8 index = _vm->_currentCharacterVariables[6];
		assert((index >= 0) && (index < 40));
		int16 x = _vm->_characterPositionX[index] >> 3;
		int16 y = _vm->_characterPositionY[index] >> 3;

		return Common::Point(x, y);
		}
	case 0xF6:
		return _vm->_savedMousePosDivided;
	default:
		Common::Point pos = Common::Point(curWord >> 8, curWord & 0xFF);
		// warning("getPosFromScript - High value %d -> %d %d", curWord, pos.x, pos.y);
		return pos;
	}
}

byte *LilliputScript::getCharacterVariablePtr() {
	debugC(2, kDebugScript, "getCharacterVariablePtr()");

	int8 tmpVal = (int8) (getValue1() & 0xFF);
	int index = tmpVal * 32;
	index += _currScript->readUint16LE();

	return _vm->getCharacterVariablesPtr(index);
}

byte LilliputScript::OC_checkCharacterGoalPos() {
	debugC(2, kDebugScript, "OC_checkCharacterGoalPos()");

	if (_vm->_currentScriptCharacterPos == getPosFromScript()) {
		return 1;
	}
	return 0;
}

byte LilliputScript::OC_comparePos() {
	debugC(2, kDebugScript, "OC_comparePos()");

	int index = getValue1();
	int8 d1 = _characterTilePosX[index];
	int8 d2 = _characterTilePosY[index];
	Common::Point var1 = getPosFromScript();

	if (var1 == Common::Point(d1, d2))
		return 1;

	warning("OC_comparePos - debug: %d-%d %d-%d", var1.x, var1.y, d1, d2);
	return 0;
}

byte LilliputScript::OC_checkIsoMap3() {
	debugC(1, kDebugScript, "OC_checkIsoMap3()");

	Common::Point var = _vm->_currentScriptCharacterPos;
	if (var == Common::Point(-1, -1)) {
		_currScript->readUint16LE();
		return 0;
	}

	byte *isoMapBuf = getMapPtr(var);
	byte var2 = isoMapBuf[3];

	int16 var3 = _currScript->readUint16LE();
	byte var4 = 8 >> var3;

	if ((var2 & var4) != 0) {
		return 1;
	}

	return 0;
}

byte LilliputScript::OC_compareCharacterVariable() {
	debugC(1, kDebugScript, "OC_compareCharacterVariable()");

	byte *tmpArr = getCharacterVariablePtr();
	byte var1 = tmpArr[0];
	uint16 oper = _currScript->readUint16LE();
	int16 var2 = _currScript->readUint16LE();

	return compareValues(var1, oper, var2);
}

byte LilliputScript::OC_CompareLastRandomValue() {
	debugC(1, kDebugScript, "OC_CompareLastRandomValue()");

	uint16 operation = _currScript->readUint16LE();
	int16 val2 = _currScript->readSint16LE();
	return compareValues(_lastRandomValue, operation, val2);
}

byte LilliputScript::OC_getRandom() {
	debugC(1, kDebugScript, "OC_getRandom()");

	int maxVal = _currScript->readUint16LE();
	int rand = _vm->_rnd->getRandomNumber(maxVal);
	_lastRandomValue = (rand & 0xFF);
	
	if (rand == 0)
		return 1;

	return 0;
}

byte LilliputScript::OC_for() {
	debugC(1, kDebugScript, "OC_for()");

	int var1 = _currScript->readUint16LE();
	int tmpVal = _currScript->readUint16LE() + 1;
	// no need to seek later, the move is already done
	_currScript->writeUint16LE(tmpVal, -2);
	// overwrite the recently used "variable" in the script
	if (tmpVal < var1)
		return 0;

	_currScript->writeUint16LE(0, -2);
	return 1;
}

byte LilliputScript::OC_compCurrentSpeechId() {
	debugC(1, kDebugScriptTBC, "OC_compCurrentSpeechId()");

	int var1 = _currScript->readUint16LE();

	if (var1 == _currentSpeechId)
		return 1;

	return 0;
}

byte LilliputScript::OC_checkSaveFlag() {
	debugC(1, kDebugScript, "OC_checkSaveFlag()");

	if (_vm->_saveFlag)
		return 1;

	return 0;
}

byte LilliputScript::OC_compScriptForVal() {
	debugC(1, kDebugScriptTBC, "OC_compScriptForVal()");

	uint16 oper = _currScript->readUint16LE();
	int16 var2 = _currScript->readUint16LE();

	return compareValues(_scriptForVal, oper, var2);
}

byte LilliputScript::OC_sub174D8() {
	debugC(1, kDebugScript, "OC_sub174D8()");

	int8 tmpVal = getValue1() & 0xFF;
	uint16 curWord = _currScript->readUint16LE();
	
	if (curWord == 3000) {
		for (int index = 0; index < _vm->_numCharacters; index++) {
			if (_vm->_rulesBuffer2_5[index] == tmpVal) {
				_word16F00_characterId = index;
				return 1;
			}
		}
	} else {
		_currScript->seek(_currScript->pos() - 2);
		int index = getValue1();
		assert(index < 40);
		if (_vm->_rulesBuffer2_5[index] == tmpVal) {
			_word16F00_characterId = index;
			return 1;
		}
	}

	return 0;
}

byte LilliputScript::OC_CompareCharacterVariables() {
	debugC(1, kDebugScript, "OC_CompareCharacterVariables()");

	byte* buf1 = getCharacterVariablePtr();
	int var1 = *buf1;

	int operation = _currScript->readUint16LE();

	byte* buf2 = getCharacterVariablePtr();
	int var2 = *buf2;

	return compareValues(var1, operation, var2);
}

// TODO Rename function to "Check if character pos in rectangle"
byte LilliputScript::OC_compareCoords_1() {
	debugC(1, kDebugScript, "OC_compareCoords_1()");

	int index = _currScript->readUint16LE();
	assert(index < 40);

	int16 var3 = _vm->_rectXMinMax[index];
	int16 var4 = _vm->_rectYMinMax[index];
	Common::Point var1 = _vm->_currentScriptCharacterPos;

	if ((var1.x < (var3 >> 8)) || (var1.x > (var3 & 0xFF)) || (var1.y < (var4 >> 8)) || (var1.y > (var4 & 0xFF)))
		return 0;

	return 1;
}

// TODO Rename function to "Check if character pos in rectangle"
byte LilliputScript::OC_compareCoords_2() {
	debugC(1, kDebugScript, "OC_compareCoords_2()");

	int16 index = getValue1();
	Common::Point var1 = Common::Point(_characterTilePosX[index], _characterTilePosY[index]);
	index = _currScript->readUint16LE();
	uint16 var3 = _vm->_rectXMinMax[index];
	uint16 var4 = _vm->_rectYMinMax[index];

	if ((var1.x < (var3 >> 8)) || (var1.x > (var3 & 0xFF)) || (var1.y < (var4 >> 8)) || (var1.y > (var4 & 0xFF)))
		return 0;
	return 1;
}

byte LilliputScript::OC_CompareDistanceFromCharacterToPositionWith() {
	debugC(1, kDebugScript, "OC_CompareDistanceFromCharacterToPositionWith()");

	Common::Point var1 = getPosFromScript();
	Common::Point pos = _vm->_currentScriptCharacterPos;
	
	int dx = var1.x - pos.x;
	if (dx < 0)
		dx = -dx;

	int dy = var1.y - pos.y;
	if (dy < 0)
		dy = -dy;

	int16 dist = dx + dy;
		
	uint16 operation = _currScript->readUint16LE();
	int16 var2 = _currScript->readSint16LE();
	
	return compareValues(dist, operation, var2);
}

byte LilliputScript::OC_compareRandomCharacterId() {
	debugC(1, kDebugScriptTBC, "OC_compareRandomCharacterId()");

	byte *tmpArr = getCharacterVariablePtr();
	_lastRandomValue = _vm->_rnd->getRandomNumber(tmpArr[0] + 1);
	uint16 oper = _currScript->readUint16LE();
	int16 var2 = _currScript->readSint16LE();

	return compareValues(_lastRandomValue, oper, var2);
}

byte LilliputScript::OC_IsCurrentCharacterIndex() {
	debugC(1, kDebugScript, "OC_IsCurrentCharacterIndex()");
	
	int tmpVal = getValue1();
	if (tmpVal == _vm->_currentScriptCharacter)
		return 1;
	return 0;
}

byte LilliputScript::OC_sub175C8() {
	debugC(1, kDebugScript, "OC_sub175C8()");
	
	byte var4 = _currScript->readUint16LE() & 0xFF;
	int tmpVal = _currScript->readUint16LE();
	
	if (tmpVal < 2000) {
		_currScript->seek(_currScript->pos() - 2);
		int index = getValue1();
		int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + index];
		if ((var1 & 0xFF) < var4)
			return 0;
		
		_word16F00_characterId = index;
		return 1;
	}

	if (tmpVal == 3000) {
		for (int i = 0; i < _vm->_numCharacters; i++) {
			int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + i];
			if ((var1 & 0xFF) >= var4) {
				_word16F00_characterId = i;
				return 1;
			}
		}
		return 0;
	}
	
	tmpVal -= 2000;
	byte var4b = tmpVal & 0xFF;
	for (int i = 0; i < _vm->_numCharacters; i++) {
		int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + i];
		if (((var1 & 0xFF) >= var4) && (_vm->_rulesBuffer2_12[i] == var4b)) {
			_word16F00_characterId = i;
			return 1;
		}
	}	
	
	return 0;
}

byte LilliputScript::OC_sub17640() {
	debugC(1, kDebugScript, "OC_sub17640()");

	uint16 var4 = _currScript->readUint16LE();
	int index = _vm->_currentScriptCharacter * 40;
	uint16 tmpVal = _currScript->readUint16LE();

	if (tmpVal < 2000) {
		_currScript->seek(_currScript->pos() - 2);
		int subIndex = getValue1();
		tmpVal = _array10B51[index + subIndex];
		byte v1 = tmpVal & 0xFF;
		byte v2 = tmpVal >> 8;
		if ((v1 < (var4 & 0xFF)) || (v2 >= (var4 & 0xFF)))
			return 0;
		_word16F00_characterId = subIndex;
		return 1;
	}

	int var1 = tmpVal;
	if (var1 == 3000) {
		for (int i = 0; i < _vm->_numCharacters; i++) {
			tmpVal = _array10B51[index + i];
			byte v1 = tmpVal & 0xFF;
			byte v2 = tmpVal >> 8;
			if ((v1 >= (var4 & 0xFF)) && (v2 < (var4 & 0xFF))) {
				_word16F00_characterId = i;
				return 1;
			}
		}
		return 0;
	} 

	var1 -= 2000;
	var4 = ((var1 & 0xFF) << 8) + (var4 & 0xFF);
	for (int i = 0; i < _vm->_numCharacters; i++) {
		tmpVal = _array10B51[index + i];
		byte v1 = tmpVal & 0xFF;
		byte v2 = tmpVal >> 8;
		if ((v1 >= (var4 & 0xFF)) && (v2 < (var4 & 0xFF)) && (_vm->_rulesBuffer2_12[i] == (var4 >> 8))) {
			_word16F00_characterId = i;
			return 1;
		}
	}
	return 0;
}

byte LilliputScript::OC_sub176C4() {
	debugC(1, kDebugScript, "OC_sub176C4()");
	
	byte var4 = _currScript->readUint16LE() & 0xFF;

	int tmpVal = _currScript->readUint16LE();
	
	if (tmpVal < 2000) {
		_currScript->seek(_currScript->pos() - 2);
		int index = getValue1();
		int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + index];
		if (((var1 & 0xFF) >= var4) || ((var1 >> 8) < var4))
			return 0;
		
		_word16F00_characterId = index;
		return 1;
	}

	if (tmpVal == 3000) {
		for (int i = 0; i < _vm->_numCharacters; i++) {
			int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + i];
			if (((var1 & 0xFF) < var4) && ((var1 >> 8) >= var4)) {
				_word16F00_characterId = i;
				return 1;
			}
		}
		return 0;
	}
	
	tmpVal -= 2000;
	byte var4b = tmpVal & 0xFF;
	for (int i = 0; i < _vm->_numCharacters; i++) {
		int var1 = _array10B51[(_vm->_currentScriptCharacter * 40) + i];
		if (((var1 & 0xFF) < var4) && ((var1 >> 8) >= var4)) {
			if (_vm->_rulesBuffer2_12[i] == var4b) {
				_word16F00_characterId = i;
				return 1;
			}
		}
	}	
	
	return 0;
}

byte LilliputScript::OC_compWord10804() {
	debugC(1, kDebugScript, "OC_compWord10804()");
	
	int tmpVal = getValue1();
	if (tmpVal == _vm->_word10804)
		return 1;

	return 0;
}

byte LilliputScript::OC_sub17766() {
	debugC(1, kDebugScript, "OC_sub17766()");

	int8 var1 = (_currScript->readUint16LE() & 0xFF);
	if ((var1 == _array12839[_vm->_currentScriptCharacter]) && (_array12811[_vm->_currentScriptCharacter] != 16))
		return 1;

	return 0;
}

byte LilliputScript::OC_sub17782() {
	debugC(1, kDebugScript, "OC_sub17782()");

	int8 var1 = (_currScript->readUint16LE() & 0xFF);
	if ((var1 == _array12839[_vm->_currentScriptCharacter]) && (_array12811[_vm->_currentScriptCharacter] == 16))
		return 1;

	return 0;
}

byte *LilliputScript::getMapPtr(Common::Point val) {
	debugC(1, kDebugScript, "getMapPtr(%d %d)", val.x, val.y);

	return &_vm->_bufferIsoMap[(val.y * 64 + val.x) << 2];
}

byte LilliputScript::OC_CompareMapValueWith() {
	debugC(1, kDebugScript, "OC_CompareMapValueWith()");

	Common::Point tmpVal = getPosFromScript();

	if (tmpVal == Common::Point(-1, -1)) {
		_currScript->seek(_currScript->pos() + 6);
		return 0;
	}
	int16 var2 = _currScript->readUint16LE();
	byte *buf = getMapPtr(tmpVal);
	int16 var1 = buf[var2];
	uint16 oper = _currScript->readUint16LE();
	var2 = _currScript->readSint16LE();
	
	return compareValues(var1, oper, var2);
}

byte LilliputScript::OC_IsCharacterValid() {
	debugC(1, kDebugScript, "OC_IsCharacterValid()");

	int index = getValue1();
	if (_vm->_characterPositionX[index] == -1)
		return 0;

	return 1;
}

byte LilliputScript::OC_compWord16EFE() {
	debugC(1, kDebugScript, "OC_compWord16EFE()");

	byte curByte = _currScript->readUint16LE() & 0xFF;
	byte tmpVal = _vm->_word16EFEh;

	if (curByte != tmpVal)
		return 0;

	_word16F00_characterId = _vm->_word16EFEl_characterId;
	return 1;
}

byte LilliputScript::OC_AreCurrentCharacterVar0AndVar1EqualsTo() {
	debugC(1, kDebugScript, "OC_AreCurrentCharacterVar0AndVar1EqualsTo()");

	byte var1 = _currScript->readUint16LE() & 0xFF;
	byte var2 = _currScript->readUint16LE() & 0xFF;

	assert(_vm->_currentCharacterVariables != NULL);

	if ((var1 == _vm->_currentCharacterVariables[0]) && (var2 == _vm->_currentCharacterVariables[1]))
		return 1;

	return 0;
}

byte LilliputScript::OC_CurrentCharacterVar0Equals() {
	debugC(1, kDebugScript, "OC_CurrentCharacterVar0Equals()");

	byte curByte = (_currScript->readUint16LE() & 0xFF);
	assert(_vm->_currentCharacterVariables != NULL);
	if (_vm->_currentCharacterVariables[0] == curByte)
		return 1;
	return 0;
}

byte LilliputScript::OC_checkLastInterfaceHotspotIndexMenu13() {
	debugC(1, kDebugScript, "OC_checkLastInterfaceHotspotIndexMenu13()");

	byte tmpVal = (_currScript->readUint16LE() & 0xFF);
	
	if ((_vm->_actionType != 1) && (_vm->_actionType != kActionTalk))
		return 0;

	if (tmpVal == _vm->_lastInterfaceHotspotIndex)
		return 1;

	return 0;
}

byte LilliputScript::OC_checkLastInterfaceHotspotIndexMenu2() {
	debugC(1, kDebugScript, "OC_checkLastInterfaceHotspotIndexMenu2()");

	int8 hotspotIndex = (_currScript->readUint16LE() & 0xFF);

	if ((_vm->_actionType == 2) && (hotspotIndex == _vm->_lastInterfaceHotspotIndex))
		return 1;

	return 0;
}

byte LilliputScript::OC_CompareNumberOfCharacterWithVar0Equals() {
	debugC(1, kDebugScript, "OC_CompareNumberOfCharacterWithVar0Equals()");

	byte curByte = (_currScript->readUint16LE() & 0xFF);
	int16 count = 0;

	for (int i = 0; i < _vm->_numCharacters; i++) {
		if (curByte == *_vm->getCharacterVariablesPtr(32 * i))
			++count;
	}

	uint16 oper = _currScript->readUint16LE();
	int16 var2 = _currScript->readSint16LE();

	return compareValues(count, oper, var2);
}

byte LilliputScript::OC_IsPositionInViewport() {
	debugC(1, kDebugScript, "OC_IsPositionInViewport()");

	Common::Point var1 = getPosFromScript();

	int16 dx = var1.x - _viewportPos.x;
	int16 dy = var1.y - _viewportPos.y;

	if ((dx >= 0) && (dx < 8) && (dy >= 0) && (dy < 8))
		return 1;
	return 0;
}

byte LilliputScript::OC_CompareGameVariables() {
	debugC(1, kDebugScript, "OC_CompareGameVariables()");

	int16 var1 = getValue1();
	int16 var2 = getValue1();
	if (var1 == var2)
		return 1;
	return 0;
}

byte LilliputScript::OC_skipNextOpcode() {
	debugC(1, kDebugScriptTBC, "OC_skipNextOpcode()");

	_currScript->readUint16LE();
	return 1;
}

byte LilliputScript::OC_CurrentCharacterVar2Equals1() {
	debugC(1, kDebugScript, "OC_CurrentCharacterVar2Equals1()");

	assert(_vm->_currentCharacterVariables != NULL);
	if (_vm->_currentCharacterVariables[2] == 1)
		return 1;
	return 0;
}

byte LilliputScript::OC_sub178D2() {
	debugC(1, kDebugScript, "OC_sub178D2()");

	int index = getValue1();
	assert (index < 40);

	byte curByte = (_currScript->readUint16LE() & 0xFF);
	if (curByte == _vm->_rulesBuffer2_12[index])
		return 1;

	return 0;
}

byte LilliputScript::OC_CharacterVariableAnd() {
	debugC(1, kDebugScript, "OC_CharacterVariableAnd()");

	byte *bufPtr = getCharacterVariablePtr();
	byte var1 = bufPtr[0];
	byte curByte = (_currScript->readUint16LE() & 0xFF);

	if (var1 & curByte)
		return 1;

	return 0;
}

byte LilliputScript::OC_IsCurrentCharacterVar0LessEqualThan() {
	debugC(1, kDebugScript, "OC_IsCurrentCharacterVar0LessEqualThan()");

	assert(_vm->_currentCharacterVariables != NULL);
	byte curByte = (_currScript->readUint16LE() & 0xFF);

	if (curByte <= _vm->_currentCharacterVariables[0])
		return 1;
	return 0;
}

byte LilliputScript::OC_sub1790F() {
	debugC(1, kDebugScript, "OC_sub1790F()");

	int16 index = getValue1();
	assert((index >= 0) && (index < 40));
	if (_vm->_rulesBuffer2_5[index] == -1)
		return 0;

	_word16F00_characterId = _vm->_rulesBuffer2_5[index];

	return 1;
}

byte LilliputScript::OC_CurrentCharacterVar1Equals() {
	debugC(1, kDebugScript, "OC_CurrentCharacterVar1Equals()");

	assert(_vm->_currentCharacterVariables != NULL);
	byte curByte = (_currScript->readUint16LE() & 0xFF);

	if (_vm->_currentCharacterVariables[1] == curByte)
		return 1;

	return 0;
}

byte LilliputScript::OC_isCurrentCharacterActive() {
	debugC(1, kDebugScript, "OC_isCurrentCharacterActive()");

	if (_vm->_currentScriptCharacterPos == Common::Point(-1, -1))
		return 0;

	if (_vm->_array16E94[_vm->_currentScriptCharacter] == 0)
		return 0;

	return 1;
}

byte LilliputScript::OC_CurrentCharacterVar3Equals1() {
	debugC(1, kDebugScript, "OC_CurrentCharacterVar3Equals1()");

	assert(_vm->_currentCharacterVariables != NULL);
	if (_vm->_currentCharacterVariables[3] == 1)
		return 1;

	return 0;
}

byte LilliputScript::OC_checkCharacterDirection() {
	debugC(1, kDebugScript, "OC_checkCharacterDirection()");

	int16 index = getValue1();
	byte expectedVal = (_currScript->readUint16LE() & 0xFF);

	if (_vm->_characterDirectionArray[index] == expectedVal)
		return 1;
	return 0;
}

byte LilliputScript::OC_checkLastInterfaceHotspotIndex() {
	debugC(1, kDebugScript, "OC_checkLastInterfaceHotspotIndex()");

	uint16 index = _currScript->readUint16LE();
	int8 var2 = (_currScript->readUint16LE() & 0xFF);

	assert(index < 20);

	if (_interfaceHotspotStatus[index] == var2)
		return 1;

	return 0;
}

byte LilliputScript::OC_checkSelectedCharacter() {
	debugC(1, kDebugScript, "OC_checkSelectedCharacter()");

	if ((_vm->_selectedCharacterId != -1) || (_vm->_savedMousePosDivided == Common::Point(-1, -1)))
		return 0;

	return 1;
}

byte LilliputScript::OC_checkDelayedReactivation() {
	debugC(1, kDebugScript, "OC_checkDelayedReactivation()");

	if (_vm->_delayedReactivationAction || (_vm->_selectedCharacterId == -1))
		return 0;

	return 1;
}

byte LilliputScript::OC_sub179C2() {
	debugC(1, kDebugScriptTBC, "OC_sub179C2()");
	Common::Point var1 = getPosFromScript();

	if ((_vm->_array10999PosX[_vm->_currentScriptCharacter] == var1.x)
		 && (_vm->_array109C1PosY[_vm->_currentScriptCharacter] == var1.y))
		return 1;

	return 0;
}
byte LilliputScript::OC_checkFunctionKeyPressed() {
	debugC(1, kDebugScript, "OC_checkFunctionKeyPressed()");

	static const Common::KeyCode specialKeys[10] = {
		Common::KEYCODE_F10, Common::KEYCODE_F1, Common::KEYCODE_F2, Common::KEYCODE_F3, Common::KEYCODE_F4,
		Common::KEYCODE_F5,  Common::KEYCODE_F6, Common::KEYCODE_F7, Common::KEYCODE_F8, Common::KEYCODE_F9};

	int8 index = (_currScript->readUint16LE() & 0xFF) - 0x30;
	
	if (specialKeys[index] == _vm->_lastKeyPressed.kbd.keycode)
		return 1;

	return 0;
}

byte LilliputScript::OC_checkCodeEntered() {
	debugC(1, kDebugScript, "OC_checkCodeEntered()");

	static const byte solutionArr[10] = {11, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	if (_vm->_actionType == kCodeEntered) {
		uint16 index = _currScript->readUint16LE();
		if (solutionArr[index] == _vm->_codeEntered[0]) {
			index = _currScript->readUint16LE();
			if (solutionArr[index] == _vm->_codeEntered[1]) {
				index = _currScript->readUint16LE();
				if (solutionArr[index] == _vm->_codeEntered[2]) {
					return 1;
				}
			} else
				// skip last index check
				_currScript->seek(_currScript->pos() + 2);
		} else
			// skip two last index checks
			_currScript->seek(_currScript->pos() + 4);
	} else
		// skip the three index checks
		_currScript->seek(_currScript->pos() + 6);

	return 0;
}

byte LilliputScript::OC_checkViewPortCharacterTarget() {
	debugC(1, kDebugScript, "OC_checkViewPortCharacterTarget()");

	int var1 = getValue1();
	if (var1 == _viewportCharacterTarget)
		return 1;
	
	return 0;
}

void LilliputScript::OC_setWord18821() {
	debugC(1, kDebugScriptTBC, "OC_setWord18821()");

	_word18821 = getValue1();
}

void LilliputScript::OC_ChangeIsoMap() {
	debugC(1, kDebugScriptTBC, "OC_ChangeIsoMap()");
	Common::Point var1 = getPosFromScript();
	int var2 = _currScript->readUint16LE();
	int var3 = _currScript->readUint16LE();

	byte* mapPtr = getMapPtr(var1);
	
	int mask = 8 >> var2;
	mask = ~mask;
	mapPtr[3] &= mask;

	if (var3 > 0) {
		mask = ~mask;
		mapPtr[3] |= mask;
	}
}

void LilliputScript::OC_startSpeech() {
	debugC(1, kDebugScript, "OC_startSpeech()");

	int curWord = _currScript->readUint16LE();

	bool forceReturnFl = false;
	checkSpeechAllowed(forceReturnFl);
	if (forceReturnFl)
		return;

	_talkingCharacter = _vm->_currentScriptCharacter;
	startSpeech(curWord);
}

void LilliputScript::getSpeechVariant(int speechIndex, int speechVariant) {
	debugC(2, kDebugScript, "getSpeechVariant(%d, %d)", speechIndex, speechVariant);

	// The packed strings are stored by variants, enclosed by imbricated brackets.
	// So the different possibilities are:
	// text
	// [text1]text2
	// [[text1]text2]text3
	// etc etc

	if (speechIndex == -1) 
		return;

	_currentSpeechId = speechIndex;
	int index = _vm->_packedStringIndex[speechIndex];

	// Skip the speech variant opening characters
	while (_vm->_packedStrings[index] == '[')
		++index;

	for (int i = 0; i < speechVariant; i++) {
		byte tmpVal = ' ';
		// Skip a speech variant
		while (tmpVal != ']') {
			tmpVal = _vm->_packedStrings[index];
			++index;
		}
	}
	
	if (_vm->_packedStrings[index] == 0)
		return;

	decodePackedText(&_vm->_packedStrings[index]);
}

void LilliputScript::OC_getComputedVariantSpeech() {
	debugC(1, kDebugScriptTBC, "OC_getComputedVariantSpeech()");

	int tmpVal1 = getCharacterVariablePtr()[0];
	int tmpVal2 = (_currScript->readUint16LE() & 0xFF);
	int speechVariant = tmpVal1 / tmpVal2;
	
	int speechIndex = _currScript->readUint16LE();

	bool forceReturnFl = false;
	checkSpeechAllowed(forceReturnFl);
	if (forceReturnFl)
		return;

	_talkingCharacter = _vm->_currentScriptCharacter;
	getSpeechVariant(speechIndex, speechVariant);
}

void LilliputScript::OC_getRotatingVariantSpeech() {
	debugC(1, kDebugScript, "OC_getRotatingVariantSpeech()");

	int index = _currScript->readUint16LE();
	int maxValue = getPackedStringStartRelativeIndex(index);
	
	int currVariant = _currScript->readUint16LE();
	int nextVariant = currVariant + 1;
	if (nextVariant >= maxValue)
		nextVariant = 0;
	_currScript->writeUint16LE(nextVariant, -2); 

	bool forceReturnFl = false;
	checkSpeechAllowed(forceReturnFl);
	if (forceReturnFl)
		return;

	_talkingCharacter = _vm->_currentScriptCharacter;

	getSpeechVariant(index, currVariant);
	
}

void LilliputScript::OC_startSpeechIfMute() {
	debugC(1, kDebugScript, "OC_startSpeechIfMute()");

	if (_talkingCharacter == -1) {
		OC_startSpeech();
		return;
	}

	_currScript->readUint16LE();
}

void LilliputScript::OC_getComputedVariantSpeechIfMute() {
	debugC(1, kDebugScriptTBC, "OC_getComputedVariantSpeechIfMute()");

	if (_talkingCharacter == -1) {
		OC_getComputedVariantSpeech();
		return;
	}
	_currScript->readUint16LE();
	_currScript->readUint16LE();
	_currScript->readUint16LE();
	_currScript->readUint16LE();

}

void LilliputScript::OC_startSpeechIfSilent() {
	debugC(1, kDebugScript, "OC_startSpeechIfSilent()");

	if (_talkingCharacter == -1) {
		OC_getRotatingVariantSpeech();
		return;
	}
	_currScript->readUint16LE();
	_currScript->readUint16LE();
}

void LilliputScript::OC_ComputeCharacterVariable() {
	debugC(1, kDebugScript, "OC_ComputeCharacterVariable()");

	byte *bufPtr = getCharacterVariablePtr();
	uint16 oper = _currScript->readUint16LE();
	int16 var3 = _currScript->readSint16LE();

	computeOperation(bufPtr, oper, var3);
}

void LilliputScript::OC_getRandom_type2() {
	debugC(1, kDebugScript, "OC_getRandom_type2()");

	byte *bufPtr = getCharacterVariablePtr();
	int maxVal = _currScript->readUint16LE(); 
	int randomVal = _vm->_rnd->getRandomNumber(maxVal);
	*bufPtr = randomVal;
}

void LilliputScript::OC_setCharacterPosition() {
	debugC(1, kDebugScript, "OC_setCharacterPosition()");
	
	int index = getValue1();
	Common::Point tmpVal = getPosFromScript();

	int var2 = (tmpVal.x << 3) + 4;
	int var4 = (tmpVal.y << 3) + 4;

	assert((index >= 0) && (index < 40));
	_vm->_characterPositionX[index] = var2;
	_vm->_characterPositionY[index] = var4;
}

void LilliputScript::OC_DisableCharacter() {
	debugC(1, kDebugScript, "OC_DisableCharacter()");

	int characterIndex = getValue1();
	assert(characterIndex < 40);

	if (characterIndex == _vm->_word10804)
		_viewportCharacterTarget = -1;

	_vm->_characterPositionX[characterIndex] = -1;
	_vm->_characterPositionY[characterIndex] = -1;
}

void LilliputScript::OC_saveAndQuit() {
	warning("OC_saveAndQuit");
	_vm->_shouldQuit = true;
}

void LilliputScript::OC_sub17B93() {
	debugC(1, kDebugScript, "OC_sub17B93()");

	int var1 = _currScript->readUint16LE();
	sub17B6C(var1);
}

void LilliputScript::OC_startSpeech5() {
	debugC(1, kDebugScriptTBC, "OC_startSpeech5()");

	bool forceReturnFl = false;
	checkSpeechAllowed(forceReturnFl);
	if (forceReturnFl)
		return;

	_talkingCharacter = _vm->_currentScriptCharacter;
	startSpeech(5);
}

void LilliputScript::OC_resetByte1714E() {
	debugC(1, kDebugScriptTBC, "OC_resetByte1714E()");

	_vm->_handleOpcodeReturnCode = 0;
}

void LilliputScript::OC_deleteSavegameAndQuit() {
	warning("OC_deleteSavegameAndQuit");
	_vm->_shouldQuit = true;
}

void LilliputScript::OC_incScriptForVal() {
	debugC(1, kDebugScriptTBC, "OC_incScriptForVal()");

	++_scriptForVal;
}

void LilliputScript::OC_sub17BA5() {
	debugC(1, kDebugScript, "OC_sub17BA5()");
	
	byte *tmpArr = getCharacterVariablePtr();
	uint16 oper = _currScript->readUint16LE();
	int16 var3 = getCharacterVariablePtr()[0];
	computeOperation(tmpArr, oper, var3);
}

void LilliputScript::OC_setByte18823() {
	debugC(1, kDebugScriptTBC, "OC_setByte18823()");

	byte *tmpArr = getCharacterVariablePtr();
	_byte18823 = *tmpArr;
}

void LilliputScript::OC_callScript() {
	debugC(1, kDebugScript, "OC_callScript()");

	int index = _currScript->readUint16LE();

	int charIndex = getValue1();
	_vm->setCurrentCharacter(charIndex);

	int tmpIndex = _vm->_currentScriptCharacter;

	assert(index < _vm->_gameScriptIndexSize);
	int scriptIndex = _vm->_arrayGameScriptIndex[index];

	_scriptStack.push(_currScript);

	if (_byte16F05_ScriptHandler == 0) {
		_vm->_handleOpcodeReturnCode = 0;
		debugC(1, kDebugScript, "========================== Menu Script %d==============================", scriptIndex);
		runMenuScript(ScriptStream(&_vm->_arrayGameScripts[scriptIndex], _vm->_arrayGameScriptIndex[index + 1] - _vm->_arrayGameScriptIndex[index]));
		debugC(1, kDebugScript, "========================== End of Menu Script==============================");
	} else {
		runScript(ScriptStream(&_vm->_arrayGameScripts[scriptIndex], _vm->_arrayGameScriptIndex[index + 1] - _vm->_arrayGameScriptIndex[index]));
	}

	_currScript = _scriptStack.pop();

	_vm->setCurrentCharacter(tmpIndex);
}

void LilliputScript::OC_callScriptAndReturn() {
	debugC(1, kDebugScript, "OC_callScriptAndReturn()");

	OC_callScript();
	sub17B6C(0);
}

void LilliputScript::OC_setCurrentScriptCharacterPos() {
	debugC(1, kDebugScript, "OC_setCurrentScriptCharacterPos()");

	Common::Point pos = getPosFromScript();
	_vm->_array10999PosX[_vm->_currentScriptCharacter] = pos.x;
	_vm->_array109C1PosY[_vm->_currentScriptCharacter] = pos.y;
	_vm->_array109E9PosX[_vm->_currentScriptCharacter] = -1;
}

void LilliputScript::OC_initScriptFor() {
	debugC(1, kDebugScriptTBC, "OC_initScriptFor()");

	_scriptForVal = 0;
}

void LilliputScript::OC_sub17AE1() {
	debugC(1, kDebugScript, "OC_sub17AE1()");

	int8 var3 = (_currScript->readUint16LE() & 0xFF);
	sub16C5C(_vm->_currentScriptCharacter, var3);
}

void LilliputScript::OC_sub17AEE() {
	debugC(1, kDebugScript, "OC_sub17AEE()");

	int8 var3 = (_currScript->readUint16LE() & 0xFF);
	sub16C5C(_vm->_currentScriptCharacter + 1, var3);
}

void LilliputScript::OC_setWord10804() {
	debugC(1, kDebugScript, "OC_setWord10804()");

	_vm->_word10804 = getValue1();
}

void LilliputScript::OC_sub17C0E() {
	debugC(1, kDebugScript, "OC_sub17C0E()");

	assert(_vm->_currentCharacterVariables != NULL);
	Common::Point var1 = Common::Point(_vm->_currentCharacterVariables[4], _vm->_currentCharacterVariables[5]);
	byte var2 = _vm->_currentCharacterVariables[6];

	byte *mapPtr = getMapPtr(var1);
	mapPtr[var2] = _vm->_currentCharacterVariables[7];
	mapPtr[3] = _vm->_currentCharacterVariables[8];

	if (var2 == 0) {
		_vm->_refreshScreenFlag = true;
		_vm->displayLandscape();
		_vm->_refreshScreenFlag = false;
	}
}

void LilliputScript::OC_sub17C55() {
	debugC(1, kDebugScript, "OC_sub17C55()");

	int8 var1 = (getValue1() & 0xFF);
	int16 index = getValue1();

	int8 var3 = (_currScript->readSint16LE() & 0xFF);
	byte var4 = (_currScript->readUint16LE() & 0xFF);

	assert((index >= 0) && (index < 40));
	_vm->_rulesBuffer2_5[index] = var1;
	_vm->_rulesBuffer2_6[index] = var3;
	_vm->_rulesBuffer2_7[index] = var4;

	warning("debug - OC_sub17C55 index %d, var1 0x%x var3 0x%x var4 0x%x", index, var1, var3, var4);
}

void LilliputScript::OC_sub17C76() {
	debugC(1, kDebugScript, "OC_sub17C76()");
	
	int index = getValue1();
	_vm->_rulesBuffer2_5[index] = -1;
	_vm->_characterPositionAltitude[index] = 0;
	_characterScriptEnabled[index] = 1;

}

void LilliputScript::OC_setCurrentCharacter() {
	debugC(1, kDebugScript, "OC_setCurrentCharacter()");
	int index = getValue1();
	_vm->setCurrentCharacter(index);
}

void LilliputScript::sub171AF(int16 var1, byte var2h, byte characterId, int16 var4) {
	debugC(2, kDebugScript, "sub171AF(%d, %d, %d, %d)", var1, var2h, characterId, var4);

	int index = 0;
	for (int i = 0; i < 10; i++) {
		if (_vm->_array12861[index + 1] == -1) {
			_vm->_array12861[index + 1] = var1;
			_vm->_array12861[index + 2] = (var2h << 8) + characterId;
			_vm->_array12861[index + 0] = _vm->_word1289D + var4;
			return;
		}
		index += 3;
	}
}

void LilliputScript::OC_sub17C8B() {
	debugC(1, kDebugScript, "OC_sub17C8B()");

	int16 type = 2 << 8;
	int16 var4 = _currScript->readSint16LE();
	byte var2h = (_currScript->readUint16LE() & 0xFF);
	
	sub171AF(type, var2h, _vm->_currentScriptCharacter, var4);
}

void LilliputScript::OC_sub17CA2() {
	debugC(1, kDebugScript, "OC_sub17CA2()");

	int16 type = 1 << 8;
	int16 var4 = _currScript->readSint16LE();
	byte var2h = (_currScript->readUint16LE() & 0xFF);
	
	sub171AF(type, var2h, _vm->_currentScriptCharacter, var4);
}

void LilliputScript::OC_sub17CB9() {
	debugC(1, kDebugScriptTBC, "OC_sub17CB9()");

	int16 var4 = _currScript->readSint16LE();
	int16 type = getValue1();
	byte var2h = (_currScript->readUint16LE() & 0xFF);
	
	sub171AF(type, var2h, _vm->_currentScriptCharacter, var4);
}

void LilliputScript::OC_sub17CD1() {
	debugC(1, kDebugScript, "OC_sub17CD1()");

	int16 type = 3 << 8;
	int16 var4 = _currScript->readSint16LE();
	byte var2h = (_currScript->readUint16LE() & 0xFF);
	
	sub171AF(type, var2h, _vm->_currentScriptCharacter, var4);
}

void LilliputScript::OC_resetWord16EFE() {
	debugC(1, kDebugScript, "OC_resetWord16EFE()");

	_vm->_word16EFEh = -1;
	_vm->_word16EFEl_characterId = -1;
}

void LilliputScript::OC_enableCurrentCharacterScript() {
	debugC(1, kDebugScript, "OC_enableCurrentCharacterScript()");

	uint8 var1 = (_currScript->readUint16LE() & 0xFF);
	enableCharacterScript(_vm->_currentScriptCharacter , var1, _vm->_currentCharacterVariables);
	sub17B6C(0);
}

void LilliputScript::OC_IncCurrentCharacterVar1() {
	debugC(1, kDebugScript, "OC_IncCurrentCharacterVar1()");

	assert(_vm->_currentCharacterVariables != NULL);
	++_vm->_currentCharacterVariables[1];
}

void LilliputScript::OC_sub17D23() {
	debugC(1, kDebugScript, "OC_sub17D23()");

	uint16 oper = _currScript->readUint16LE();
	Common::Point var1 = getPosFromScript();
	byte* buf = _vm->_currentCharacterVariables + 4;
	computeOperation(buf, oper, var1.x);
	computeOperation(buf + 1, oper, var1.y);
}

void LilliputScript::OC_sub17E6D() {
	debugC(1, kDebugScriptTBC, "OC_sub17E6D()");
	
	uint16 var1 = _currScript->readUint16LE();
	_vm->_rulesBuffer2_12[_vm->_currentScriptCharacter] = (var1 - 2000) & 0xFF;
}

void LilliputScript::OC_changeCurrentCharacterSprite() {
	debugC(2, kDebugScript, "OC_changeCurrentCharacterSprite()");

	int16 var1 = _currScript->readSint16LE();
	byte var2 = (_currScript->readUint16LE() & 0xFF);
	_vm->_characterFrameArray[_vm->_currentScriptCharacter] = var1;
	_vm->_spriteSizeArray[_vm->_currentScriptCharacter] = var2;

}

byte *LilliputScript::getCurrentCharacterVarFromScript() {
	debugC(2, kDebugScript, "getCurrentCharacterVarFromScript()");

	int index = _currScript->readUint16LE();	
	return &_vm->_currentCharacterVariables[index];
}

void LilliputScript::OC_sub17E99() {
	debugC(1, kDebugScript, "OC_sub17E99()");

	byte *compBuf = getCurrentCharacterVarFromScript();
	uint16 oper = _currScript->readUint16LE();
	int index = _currScript->readUint16LE();	

	byte *buf = getCurrentCharacterVarFromScript();
	byte var1 = buf[0];
	byte var3 = _vm->_rulesChunk11[var1 + _vm->_rulesChunk10[index]];

	computeOperation(compBuf, oper, var3);
}

void LilliputScript::OC_sub17EC5() {
	debugC(1, kDebugScriptTBC, "OC_sub17EC5()");

	int indexChunk10 = _currScript->readUint16LE();

	byte *compBuf = getCurrentCharacterVarFromScript();
	int indexChunk11 = _vm->_rulesChunk10[indexChunk10] + compBuf[0];

	uint16 oper = _currScript->readUint16LE();

	byte *tmpBuf = getCurrentCharacterVarFromScript();
	int16 var3 = tmpBuf[0];
	
	computeOperation(&_vm->_rulesChunk11[indexChunk11], oper, var3);
}

Common::Point LilliputScript::getCharacterTilePos(int index) {
	debugC(2, kDebugScript, "getCharacterTilePos(%d)", index);

	return Common::Point(_vm->_characterPositionX[index] / 8, _vm->_characterPositionY[index] / 8);
}

void LilliputScript::OC_setCharacterDirectionTowardsPos() {
	debugC(1, kDebugScript, "OC_setCharacterDirectionTowardsPos()");

	Common::Point pos1 = getPosFromScript();
	Common::Point tilePos = getCharacterTilePos(_vm->_currentScriptCharacter);

	_vm->_characterDirectionArray[_vm->_currentScriptCharacter] = _vm->getDirection(tilePos, pos1);
}

void LilliputScript::OC_turnCharacterTowardsAnother() {
	debugC(1, kDebugScript, "OC_turnCharacterTowardsAnother()");
	
	int index = getValue1();

	static const byte _directionsArray[] = { 0, 2, 0, 1, 3, 2, 3, 1 };

	int dx = _vm->_characterPositionX[index] - _vm->_characterPositionX[_vm->_currentScriptCharacter];
	int dy = _vm->_characterPositionY[index] - _vm->_characterPositionY[_vm->_currentScriptCharacter];

	int flag = 0;
	if (dx < 0) {
		dx = -dx;
		flag |= 4;
	}
	if (dy < 0) {
		dy = -dy;
		flag |= 2;
	} 
	if (dx < dy) {
		flag |= 1;
	}

	_vm->_characterDirectionArray[_vm->_currentScriptCharacter] = _directionsArray[flag];
}

void LilliputScript::OC_sub17F4F() {
	debugC(1, kDebugScript, "OC_sub17F4F()");

	int16 var = getValue1();
	_array10A39[_vm->_currentScriptCharacter] = (byte)(var & 0xFF);
	warning("debug - OC_sub17F4F: _array10A39[%d] = %d", _vm->_currentScriptCharacter, var);
	_vm->_array109E9PosX[_vm->_currentScriptCharacter] = -1;
}

void LilliputScript::OC_scrollAwayFromCharacter() {
	debugC(1, kDebugScript, "OC_scrollAwayFromCharacter()");

	if (_vm->_currentScriptCharacter != _viewportCharacterTarget)
		return;

	static const char speedX[] = {-1, -3, -3, -6};
	static const char speedY[] = {-3, -6, -1, -3};
	
	int cx = speedX[_vm->_characterDirectionArray[_vm->_currentScriptCharacter]];
	int cy = speedY[_vm->_characterDirectionArray[_vm->_currentScriptCharacter]];

	Common::Point pos = getCharacterTilePos(_vm->_currentScriptCharacter);

	int newPosX = pos.x + cx;
	int newPosY = pos.y + cy;

	if (newPosX < 0)
		newPosX = 0;

	if (newPosX > 56)
		newPosX = 56;

	if (newPosY < 0)
		newPosY = 0;

	if (newPosY > 56)
		newPosY = 56;

	_vm->_refreshScreenFlag = true;
	_vm->viewportScrollTo(Common::Point(newPosX, newPosY));
	_vm->_refreshScreenFlag = false;

}

void LilliputScript::OC_skipNextVal() {
	debugC(1, kDebugScriptTBC, "OC_skipNextVal()");

	 _currScript->readUint16LE();
}

void LilliputScript::OC_setCurrentCharacterVar6() {
	debugC(1, kDebugScript, "OC_setCurrentCharacterVar6()");
	
	uint16 var1 = (uint16)getValue1();
	warning("debug - OC_setCurrentCharacterVar6 %d", var1);
	_vm->_currentCharacterVariables[6] = var1 & 0xFF;
}

void LilliputScript::OC_sub17FDD() {
	debugC(1, kDebugScript, "OC_sub17FDD()");

	int index = _currScript->readUint16LE();
	
	int tmpVal = (_vm->_currentScriptCharacter * 32) + index;
	assert (tmpVal < 40 * 32);
	_array10AB1[_vm->_currentScriptCharacter] = _vm->_rulesBuffer2_16[tmpVal];
	_array12811[_vm->_currentScriptCharacter] = 16;
}

void LilliputScript::OC_setCharacterScriptEnabled() {
	debugC(1, kDebugScript, "OC_setCharacterScriptEnabled()");

	int16 index = getValue1();
	_characterScriptEnabled[index] = 1;
}

void LilliputScript::OC_setCurrentCharacterVar2() {
	debugC(1, kDebugScript, "OC_setCurrentCharacterVar2()");

	int curWord = _currScript->readUint16LE();
	assert(_vm->_currentCharacterVariables != NULL);
	_vm->_currentCharacterVariables[2] = curWord & 0xFF;
}

void LilliputScript::OC_SetCurrentCharacterVar2ToZero() {
	debugC(1, kDebugScript, "OC_SetCurrentCharacterVar2ToZero()");

	assert(_vm->_currentCharacterVariables != NULL);
	_vm->_currentCharacterVariables[2] = 0;
}

void LilliputScript::OC_setCharacterProperties() {
	debugC(1, kDebugScript, "OC_setCharacterProperties()");

	int16 index = getValue1();

	int16 x = _vm->_characterPositionX[index] & 0xFFF8;
	x += _currScript->readSint16LE();
	_vm->_characterPositionX[index] = x;

	int16 y = _vm->_characterPositionY[index] & 0xFFF8;
	y += _currScript->readSint16LE();
	_vm->_characterPositionY[index] = y;

	_vm->_characterPositionAltitude[index]  = (int8)(_currScript->readUint16LE() & 0xFF);
	_vm->_characterDirectionArray[index] = _currScript->readUint16LE() & 0xFF;
}

void LilliputScript::OC_sub1805D() {
	debugC(1, kDebugScriptTBC, "OC_sub1805D()");

	_word129A3 = getValue1();
	for (int i = 0; i < 4; i++)
		_array129A5[i] = _currScript->readUint16LE() & 0xFF;
}

void LilliputScript::OC_sub18074() {
	debugC(1, kDebugScriptTBC, "OC_sub18074()");

	int var2 = _currScript->readUint16LE();
	byte var1 = (_currScript->readUint16LE() & 0xFF);

	_vm->_rulesBuffer2_16[(_vm->_currentScriptCharacter * 32) + var2] = var1;
}

void LilliputScript::OC_setCurrentCharacterDirection() {
	debugC(1, kDebugScript, "OC_setCurrentCharacterDirection()");

	_vm->_characterDirectionArray[_vm->_currentScriptCharacter] = (_currScript->readUint16LE() & 0xFF);
}

void LilliputScript::OC_setInterfaceHotspot() {
	debugC(1, kDebugScript, "OC_setInterfaceHotspot()");

	int16 index = _currScript->readSint16LE();
	assert((index >= 0) && (index < 20));

	uint16 curWord = _currScript->readUint16LE();
	_interfaceHotspotStatus[index] = (curWord & 0xFF);
	_interfaceButtonActivationDelay[index] = (curWord >> 8);

	_vm->displayInterfaceHotspots();
}

void LilliputScript::OC_scrollViewPort() {
	debugC(1, kDebugScriptTBC, "OC_scrollViewPort()");

	_viewportCharacterTarget = -1;

	int direction = _currScript->readUint16LE();
	
	static const char scrollValX[] = { 6, 0, 0, -6 };
	static const char scrollValY[] = { 0, -6, 6, 0 };
	
	int x = _viewportPos.x + scrollValX[direction];
	int y = _viewportPos.y + scrollValY[direction];

	CLIP(x, 0, 56);
	CLIP(y, 0, 56);

	_vm->_refreshScreenFlag = true;
	_vm->viewportScrollTo(Common::Point(x, y));
	_vm->_refreshScreenFlag = false;
}

void LilliputScript::OC_setViewPortPos() {
	debugC(1, kDebugScript, "OC_setViewPortPos()");

	_viewportCharacterTarget = -1;
	_viewportPos = getPosFromScript();

	_vm->displayLandscape();
	_vm->prepareGameArea();
}

void LilliputScript::OC_setCurrentCharacterAltitude() {
	debugC(1, kDebugScript, "OC_setCurrentCharacterAltitude()");

	_vm->_characterPositionAltitude[_vm->_currentScriptCharacter] = (_currScript->readUint16LE() & 0xFF);
}

void LilliputScript::OC_sub1817F() {
	debugC(1, kDebugScript, "OC_sub1817F()");

	int8 x = (int8)(_currScript->readUint16LE() & 0xFF);
	int8 y = (int8)(_currScript->readUint16LE() & 0xFF);

	sub1818B(Common::Point(x, y));
}

void LilliputScript::sub1818B(Common::Point point) {
	debugC(2, kDebugScript, "sub1818B(%d - %d)", point.x, point.y);

	Common::Point pos = point;
	for (int i = 0; i <  _vm->_word1817B; i++) {
		if (_array1813BPos[i].x == pos.x) {
			pos.y += _array1813BPos[i].y;
			if (pos.y > 0xFF)
				pos.y = 0xFF;

			_array1813BPos[i] = pos;
			return;
		}
	}

	_array1813BPos[_vm->_word1817B] = pos;
	++_vm->_word1817B;
}

void LilliputScript::OC_sub181BB() {
	debugC(1, kDebugScript, "OC_sub181BB()");
	
	int8 x = (int8)(_currScript->readUint16LE() & 0xFF);
	byte oper = _currScript->readUint16LE() & 0xFF;
	uint16 index = _currScript->readUint16LE();
	int16 c = _vm->_currentCharacterVariables[index];

	switch (oper) {
	case '-':
		c = - 1 - c;
		break;
	case '>':
		c -= 128;
		if (c < 0) 
			c = 0;
		c *= 2;
		break;
	case '<':
		c = -1 - c - 128;
		if (c < 0)
			c = 0;
		c *= 2;
		break;
	case '+':
		break;
	default:
		warning("OC_sub181BB: skipped oper %c", oper);
		break;
	}
	if (c > 0xFF)
		warning("OC_sub181BB- Abnormal value c = %d, should put back c &= 0xFF;", c);

	int y = (_currScript->readSint16LE() * c) + c;
	y >>= 8;
	sub1818B(Common::Point(x, y));
}

void LilliputScript::OC_sub18213() {
	debugC(1, kDebugScript, "OC_sub18213()");

	uint16 var1 = _currScript->readUint16LE();

	int maxValue = 0;
	int maxItem = var1 & 0xFF;

	for (int i = 0; i < _vm->_word1817B; i++) {
		if (_array1813BPos[i].y > maxValue) {
			maxValue = _array1813BPos[i].y;
			maxItem = _array1813BPos[i].x;
		}
	}
	enableCharacterScript(_vm->_currentScriptCharacter, maxItem, _vm->_currentCharacterVariables);
}

void LilliputScript::OC_sub18252() {
	debugC(1, kDebugScript, "OC_sub18252()");

	int16 index = getValue1();
	assert((index >0) && (index < 40));

	_vm->_array12299[index] = 4;
}

void LilliputScript::OC_sub18260() {
	debugC(1, kDebugScriptTBC, "OC_sub18260()");
	
	int index = getValue1();
	Common::Point var4 = getPosFromScript();

	Common::Point pt = var4 + _viewportPos;
	byte *isoMapBuf = getMapPtr(pt);

	if (isoMapBuf[1] != 0xFF) {
		int byte1825D = 255;
		for (int var2 = 7; var2 >= 0; var2--) {
			for (int var3 = 7; var3 >= 0; var3--) {
				Common::Point(_viewportPos.x + var2, _viewportPos.y + var3);
				isoMapBuf = getMapPtr(pt);

				if (isoMapBuf[1] == 0xFF) {
					int x = abs(var2 - var4.x);
					int y = abs(var3 - var4.y);
					if (x + y < byte1825D) {
						byte1825D = x + y;
						_word1825E = Common::Point(var2, var3);
					}
				}
			}
		}
		var4 = _word1825E;
	}

	_vm->_characterPositionX[index] = (var4.x + _viewportPos.x) * 8;
	_vm->_characterPositionY[index] = (var4.y + _viewportPos.y) * 8;
}

void LilliputScript::OC_CharacterVariableAddOrRemoveFlag() {
	debugC(1, kDebugScript, "OC_CharacterVariableAddOrRemoveFlag()");

	byte *tmpArr = getCharacterVariablePtr();

	byte var1 = (_currScript->readUint16LE() & 0xFF);
	byte var2 = (_currScript->readUint16LE() & 0xFF);

	if (var2 == 0)
		tmpArr[0] &= ~var1;
	else
		tmpArr[0] |= var1;
}

void LilliputScript::OC_PaletteFadeOut() {
	debugC(1, kDebugScript, "OC_PaletteFadeOut()");

	_vm->_refreshScreenFlag = true;
	_vm->paletteFadeOut();
	_vm->_refreshScreenFlag = false;
}

void LilliputScript::OC_PaletteFadeIn() {
	debugC(1, kDebugScript, "OC_PaletteFadeIn()");

	_vm->_refreshScreenFlag = true;
	_vm->paletteFadeIn();
	_vm->_refreshScreenFlag = false;
}

void LilliputScript::OC_loadAndDisplayCUBESx_GFX() {
	debugC(1, kDebugScript, "OC_loadAndDisplayCUBESx_GFX()");

	int curWord = (_currScript->readUint16LE() & 0xFF);
	assert((curWord >= 0) && (curWord <= 9));
	Common::String fileName = Common::String::format("CUBES%d.GFX", curWord);
	_byte10806 = curWord + 0x30; // Useless?

	_vm->_bufferCubegfx = _vm->loadVGA(fileName, 61440, false);
	_vm->displayLandscape();
	_vm->prepareGameArea();
}

void LilliputScript::OC_setCurrentCharacterVar3() {
	debugC(1, kDebugScript, "OC_setCurrentCharacterVar3()");

	byte var1 = _currScript->readUint16LE() & 0xFF;
	assert(_vm->_currentCharacterVariables != NULL);

	_vm->_currentCharacterVariables[3] = var1;
}

void LilliputScript::OC_setArray122C1() {
	debugC(1, kDebugScript, "OC_setArray122C1()");

	byte var1 = (_currScript->readUint16LE() & 0xFF);
	_array122C1[_vm->_currentScriptCharacter] = var1;
}

void LilliputScript::OC_sub18367() {
	debugC(1, kDebugScriptTBC, "OC_sub18367()");

	_characterScriptEnabled[_vm->_currentScriptCharacter] = 1;
	_vm->_currentCharacterVariables[0] = _array122C1[_vm->_currentScriptCharacter];
	_vm->_currentCharacterVariables[1] = 0;
	_vm->_currentCharacterVariables[2] = 0;
	_vm->_currentCharacterVariables[3] = 0;
}

void LilliputScript::OC_enableCharacterScript() {
	debugC(1, kDebugScript, "OC_enableCharacterScript()");

	int16 index = getValue1();
	byte var2 = _currScript->readUint16LE() & 0xFF;
	
	enableCharacterScript(index, var2, _vm->getCharacterVariablesPtr(index * 32));
}

void LilliputScript::OC_sub18387() {
	debugC(1, kDebugScriptTBC, "OC_sub18387()");

	int index = getValue1();
	byte var1 = _currScript->readUint16LE() & 0xFF;

	assert(index < 40);
	_vm->_rulesBuffer2_10[index] = var1;
}

void LilliputScript::OC_setDebugFlag() {
	debugC(1, kDebugScriptTBC, "OC_setDebugFlag()");

	_vm->_debugFlag = 1;
}

void LilliputScript::OC_setByte14837() {
	debugC(1, kDebugScriptTBC, "OC_setByte14837()");

	_vm->_byte14837 = 1;
}

void LilliputScript::OC_waitForEvent() {
	debugC(1, kDebugScriptTBC, "OC_waitForEvent()");

	_vm->_refreshScreenFlag = true;
	while (true) {
		if (_vm->_keyboard_checkKeyboard()) {
			_vm->_keyboard_getch();
			break;;
		}
		if (_vm->_mouseButton == 1)
			break;

		_vm->update();
	}

	_vm->_mouseButton = 0;
	_vm->_refreshScreenFlag = false;
}

void LilliputScript::OC_disableInterfaceHotspot() {
	debugC(1, kDebugScript, "OC_disableInterfaceHotspot()");

	int index = _currScript->readUint16LE();
	_interfaceButtonActivationDelay[index] = (_currScript->readUint16LE() & 0xFF);
	_interfaceHotspotStatus[index] = kHotspotDisabled;

	_vm->displayInterfaceHotspots();
}

void LilliputScript::OC_loadFileAerial() {
	debugC(1, kDebugScript, "OC_loadFileAerial()");

	// Unused variable, and the script position is restored afterwards
	// TODO: Check if this part of the code is present in Rome, else remove it
	// int var1 = _currScript->readUint16LE() & 0xFF;
	// byte _byte15EAD = var1;

	_vm->_refreshScreenFlag = true;
	_talkingCharacter = -1;
	OC_PaletteFadeOut();
	_vm->_bool15AC2 = true;
	_vm->displayVGAFile("AERIAL.GFX");
	OC_PaletteFadeIn();

	_vm->displayCharactersOnMap();
	_vm->_displayMap = true;

	_vm->_keyboard_resetKeyboardBuffer();

	_vm->_refreshScreenFlag = false;
}

void LilliputScript::OC_startSpeechIfSoundOff() {
	debugC(1, kDebugScriptTBC, "OC_startSpeechIfSoundOff()");

	// HACK: In the original, OC_startSpeechIfSoundOff() only calls 
	// OC_startSpeech if sound is off. For the moment, it's always called

	OC_startSpeech();
}

void LilliputScript::OC_sub1844A() {
	debugC(1, kDebugScriptTBC, "OC_sub1844A()");
	
	int var1 = getValue1();
	int var2 = _currScript->readUint16LE();

	_vm->_rulesBuffer2_11[var1] = (var2 & 0xFF);

	for (int i = 0; i < 40; i++) {
		_array10B51[40 * var1 + i] = 0;
		_array10B51[var1 + 40 * i] = 0;
	}
}

void LilliputScript::OC_displayNumericCharacterVariable() {
	debugC(1, kDebugScript, "OC_displayNumericCharacterVariable()");

	byte *buf215Ptr = getCharacterVariablePtr();
	byte tmpVal = buf215Ptr[0];
	int curWord = _currScript->readUint16LE();
	assert(curWord != 0);
	int displayVal = tmpVal / (curWord & 0xFF);
	int posX = _currScript->readSint16LE();
	int posY = _currScript->readSint16LE();

	if (!_vm->_displayMap) {
		_vm->restoreSurfaceUnderMousePointer();
		displayNumber(displayVal, Common::Point(posX, posY));
		_vm->displayMousePointer();
	}
}

void LilliputScript::displayNumber(byte var1, Common::Point pos) {
	debugC(1, kDebugScript, "displayNumber(%d, %d - %d)", var1, pos.x, pos.y);

	_vm->_displayStringIndex = 0;
	_vm->_displayStringBuf[0] = 32;
	_vm->_displayStringBuf[1] = 32;
	_vm->_displayStringBuf[2] = 32;
	_vm->_displayStringBuf[3] = 0;

	_vm->numberToString(var1);
	_vm->displayString(_vm->_displayStringBuf, pos);
}

void LilliputScript::OC_displayVGAFile() {
	debugC(1, kDebugScript, "OC_displayVGAFile()");

	_vm->_refreshScreenFlag = true;
	_vm->paletteFadeOut();
	int curWord = _currScript->readUint16LE();
	int index = _vm->_packedStringIndex[curWord];
	Common::String fileName = Common::String((const char *)&_vm->_packedStrings[index]);
	_talkingCharacter = -1;
	_vm->displayVGAFile(fileName);
	_vm->paletteFadeIn();
}

void LilliputScript::OC_startSpeechWithoutSpeeker() {
	debugC(1, kDebugScript, "OC_startSpeechWithoutSpeeker()");

	int16 speechId = _currScript->readUint16LE();
	startSpeech(speechId);
}

void LilliputScript::OC_displayTitleScreen() {
	debugC(1, kDebugScript, "OC_displayTitleScreen()");

	_vm->_byte184F4 = (_currScript->readUint16LE() & 0xFF);
	_vm->_sound_byte16F06 = _vm->_byte184F4;

	_vm->_keyboard_resetKeyboardBuffer();

	_vm->_mouseButton = 0;
	_vm->_lastKeyPressed = Common::Event();

	while (!_vm->_shouldQuit) {
		_vm->displaySmallAnims();
		_vm->update();
		_vm->pollEvent();
		if (_vm->_keyboard_checkKeyboard()) {
			Common::Event event = _vm->_keyboard_getch();
			_vm->_lastKeyPressed = event;
			if (event.type == Common::EVENT_KEYDOWN)
				_vm->_keyboard_getch();
			break;
		}
		
		if (_vm->_mouseButton == 1)
			break;
		
		if ((_vm->_byte184F4 != 0) && (_vm->_sound_byte16F06 == 0))
			break;
	}
	
	_vm->_mouseButton = 0;
}

void LilliputScript::OC_initGameAreaDisplay() {
	debugC(1, kDebugScript, "OC_initGameAreaDisplay()");

	OC_PaletteFadeOut();
	_vm->_displayMap = false;
	_heroismLevel = 0;
	_vm->unselectInterfaceHotspots();

	_vm->initGameAreaDisplay();

	OC_PaletteFadeIn();
	_vm->_refreshScreenFlag = false;
	
	_vm->_soundHandler->contentFct5();
}

void LilliputScript::OC_displayCharacterStatBar() {
	debugC(1, kDebugScript, "OC_displayCharacterStatBar()");

	byte *tmpArr = getCharacterVariablePtr();
	int8 type = (_currScript->readUint16LE() & 0xFF);
	int8 score = (((70 * tmpArr[0]) / (_currScript->readUint16LE() & 0xFF)) & 0xFF);
	int16 posX = _currScript->readSint16LE();
	int16 posY = _currScript->readSint16LE();
	
	_vm->displayCharacterStatBar(type, posX, score, posY);
}

void LilliputScript::OC_initSmallAnim() {
	debugC(1, kDebugScript, "OC_initSmallAnim()");

	int index = _currScript->readUint16LE();
	assert (index < 4);
	_vm->_smallAnims[index]._active = true;
	_vm->_smallAnims[index]._pos.x = _currScript->readSint16LE();
	_vm->_smallAnims[index]._pos.y = _currScript->readSint16LE();

	for (int i = 0; i < 8; i++)
		_vm->_smallAnims[index]._frameIndex[i] = _currScript->readUint16LE();
}

void LilliputScript::OC_setCharacterHeroismBar() {
	debugC(1, kDebugScript, "OC_setCharacterHeroismBar()");

	_savedBuffer215Ptr = getCharacterVariablePtr();
	_heroismBarX = _currScript->readUint16LE();
	_heroismBarBottomY = _currScript->readUint16LE();
}

void LilliputScript::OC_sub18690() {
	debugC(1, kDebugScriptTBC, "OC_sub18690()");

	int index = getValue1();
	Common::Point pos = getPosFromScript();
	_vm->_rulesBuffer2_13_posX[index] = pos.x;
	_vm->_rulesBuffer2_14_posY[index] = pos.y;
}

void LilliputScript::OC_setViewPortCharacterTarget() {
	debugC(1, kDebugScript, "OC_setViewPortCharacterTarget()");

	_viewportCharacterTarget = getValue1();
}

void LilliputScript::OC_sub186A1() {
	debugC(1, kDebugScriptTBC, "OC_sub186A1()");

	int var1 = getValue1();
	int posX = _currScript->readUint16LE();
	int posY = _currScript->readUint16LE();
	Common::Point pos = Common::Point(posX, posY);

	_vm->fill16x16Rect(16, pos);

	int frame = _vm->_characterFrameArray[var1];
	byte* buf = _vm->_bufferMen;

	if (frame > 0xF0) {
		buf = _vm->_bufferMen2;
		frame -= 0xF0;
	}

	_vm->display16x16IndexedBuf(buf, frame, pos);
}

void LilliputScript::OC_sub186E5_snd() {
	debugC(1, kDebugScript, "OC_sub186E5_snd()");
	int index = getValue1();
	assert(index < 40);

	Common::Point var4 = Common::Point(0xFF, index & 0xFF);
	int var1 = (_currScript->readUint16LE() & 0xFF);
	
	_vm->_soundHandler->contentFct2(var1, _viewportPos, Common::Point(_characterTilePosX[index], _characterTilePosY[index]), var4);
}

void LilliputScript::OC_sub1870A_snd() {
	debugC(1, kDebugScript, "OC_sub1870A_snd()");

	Common::Point var3 = getPosFromScript();
	Common::Point var4 = var3;
	Common::Point var2 = _viewportPos;
	int var1 = (_currScript->readUint16LE() & 0xFF);

	_vm->_soundHandler->contentFct2(var1, var2, var3, var4);
}

void LilliputScript::OC_sub18725_snd() {
	debugC(1, kDebugScript, "OC_sub18725_snd()");

	Common::Point var4 = Common::Point(-1, getValue1() & 0xFF);

	_vm->_soundHandler->contentFct3(var4);
}

void LilliputScript::OC_sub18733_snd() {
	debugC(1, kDebugScript, "OC_sub18733_snd()");

	Common::Point var4 = getPosFromScript();

	_vm->_soundHandler->contentFct3(var4);
}

void LilliputScript::OC_sub1873F_snd() {
	debugC(1, kDebugScriptTBC, "OC_sub1873F_snd()");

	_vm->_soundHandler->contentFct4();
}

void LilliputScript::OC_sub18746_snd() {
	debugC(1, kDebugScript, "OC_sub18746_snd()");

	Common::Point var4 = Common::Point(-1, -1);
	Common::Point var2 = _viewportPos;
	int var1 = _currScript->readSint16LE() & 0xFF;
	warning("OC_sub18746_snd: unknown value for var3");
	Common::Point var3 = Common::Point(-1, -1);

	_vm->_soundHandler->contentFct2(var1, var2, var3, var4);
}

void LilliputScript::OC_sub1875D_snd() {
	debugC(1, kDebugScript, "OC_sub1875D_snd()");

	_vm->_soundHandler->contentFct6();
}

void LilliputScript::OC_setCharacterMapColor() {
	debugC(1, kDebugScript, "OC_setCharacterMapColor()");

	byte index = (getValue1() & 0xFF);
	int color = _currScript->readUint16LE();

	assert(index < 40);
	_characterMapPixelColor[index] = (color & 0xFF);
}

} // End of namespace
