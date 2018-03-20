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

#ifndef LILLIPUT_SCRIPT_H
#define LILLIPUT_SCRIPT_H

#include "common/memstream.h"
#include "common/stack.h"
#include "common/random.h"
#include "common/rect.h"

#include "lilliput/stream.h"

namespace Lilliput {

class LilliputEngine;

enum kActionType {
	kActionNone = 0,
	kActionTalk = 3,
	kActionGoto = 4,
	kCodeEntered = 6
};

enum kValueType {
	kNone,
	kImmediateValue,
	kCompareOperation,
	kComputeOperation,
	kGetValue1,
	kgetPosFromScript
};


struct OpCode {
	const char* _opName;
	int _numArgs;
	kValueType _arg1;
	kValueType _arg2;
	kValueType _arg3;
	kValueType _arg4;
	kValueType _arg5;
};

class LilliputScript {
public:
	byte _heroismLevel;
	byte _speechTimer;

	byte _characterScriptEnabled[40];
	int8 _interfaceHotspotStatus[20];
	int8 _characterTilePosX[40];
	int8 _characterTilePosY[40];
	int8 _array12811[40];
	int8 _array10AB1[40];
	byte _interfaceButtonActivationDelay[20];
	byte _array122C1[40];
	byte _array10A39[40];
	int16 _interactions[40 * 40];

	byte *_savedBuffer215Ptr;

	Common::Point _viewportPos;
	int16 _viewportCharacterTarget;
	int16 _talkingCharacter;
	int _heroismBarX;
	int _heroismBarBottomY;

	Common::Point _array12311[640];
	byte _characterMapPixelColor[40];
	int8 _array12839[40];
	Common::Point _array1813BPos[32];

	LilliputScript(LilliputEngine *vm);
	~LilliputScript();

	void disasmScript(ScriptStream script);
	void listAllTexts();
	static Common::String getArgumentString(kValueType type, ScriptStream& script);
	void runScript(ScriptStream script);
	void runMenuScript(ScriptStream script);
private:
	LilliputEngine *_vm;

	ScriptStream *_currScript;
	Common::Stack<ScriptStream *> _scriptStack;

	byte _byte16F05_ScriptHandler;
	byte _byte10806;
	byte _lastRandomValue;
	byte _scriptForVal;
	byte _byte1881A;
	byte _byte18823;
	byte _speechDisplaySpeed;

	int16 _word16F00_characterId;
	int _currentSpeechId;
	int _word18821;
	int _word129A3;
	Common::Point _word1825E;

	char _array129A5[4];

	int handleOpcode(ScriptStream *script);
	byte handleOpcodeType1(int curWord);
	void handleOpcodeType2(int curWord);

	void enableCharacterScript(byte index, byte var1, byte *curBufPtr);
	void sub17B6C(int var1);
	void sub16C86(int index, byte *buf);
	void sub16C5C(int index, int8 var3);
	void checkSpeechAllowed(bool &forceReturnFl);
	void decodePackedText(char *buf);
	void startSpeech(int var);
	void displayNumber(byte var1, Common::Point pos);
	byte *getMapPtr(Common::Point val);
	byte *getCurrentCharacterVarFromScript();
	void sub171AF(int16 var1, byte var2h, byte characterId, int16 var4);
	void getSpeechVariant(int speechIndex, int speechVariant);
	void sub189B8();
	void formatSpeechString();
	Common::Point getCharacterTilePos(int index);
	int getPackedStringStartRelativeIndex(int index);

	int16 getValue1();
	Common::Point getPosFromScript();

	byte *getCharacterVariablePtr();
	byte compareValues(int16 var1, uint16 oper, int16 var2);
	void computeOperation(byte *bufPtr, uint16 oper, int16 var2);

	//Opcodes Type 1
	byte OC_checkCharacterGoalPos();
	byte OC_comparePos();
	byte OC_checkIsoMap3();
	byte OC_compareCharacterVariable();
	byte OC_CompareLastRandomValue();
	byte OC_getRandom();
	byte OC_for();
	byte OC_compCurrentSpeechId();
	byte OC_checkSaveFlag();
	byte OC_compScriptForVal();
	byte OC_isCarrying();
	byte OC_CompareCharacterVariables();
	byte OC_compareCoords_1();
	byte OC_compareCoords_2();
	byte OC_CompareDistanceFromCharacterToPositionWith();
	byte OC_compareRandomCharacterId();
	byte OC_IsCurrentCharacterIndex();
	byte OC_hasVisibilityLevel();
	byte OC_hasGainedVisibilityLevel();
	byte OC_hasReducedVisibilityLevel();
	byte OC_isHost();
	byte OC_sub17766();
	byte OC_sub17782();
	byte OC_CompareMapValueWith();
	byte OC_IsCharacterValid();
	byte OC_compWord16EFE();
	byte OC_AreCurrentCharacterVar0AndVar1EqualsTo();
	byte OC_CurrentCharacterVar0Equals();
	byte OC_checkLastInterfaceHotspotIndexMenu13();
	byte OC_checkLastInterfaceHotspotIndexMenu2();
	byte OC_CompareNumberOfCharacterWithVar0Equals();
	byte OC_IsPositionInViewport();
	byte OC_CompareGameVariables();
	byte OC_skipNextOpcode();
	byte OC_CurrentCharacterVar2Equals1();
	byte OC_sub178D2();
	byte OC_CharacterVariableAnd();
	byte OC_IsCurrentCharacterVar0LessEqualThan();
	byte OC_sub1790F();
	byte OC_CurrentCharacterVar1Equals();
	byte OC_isCurrentCharacterActive();
	byte OC_CurrentCharacterVar3Equals1();
	byte OC_checkCharacterDirection();
	byte OC_checkLastInterfaceHotspotIndex();
	byte OC_checkSelectedCharacter();
	byte OC_checkDelayedReactivation();
	byte OC_sub179C2();
	byte OC_checkFunctionKeyPressed();
	byte OC_checkCodeEntered();
	byte OC_checkViewPortCharacterTarget();

	// Opcodes Type 2
	void OC_setWord18821();
	void OC_ChangeIsoMap();
	void OC_startSpeech();
	void OC_getComputedVariantSpeech();
	void OC_getRotatingVariantSpeech();
	void OC_startSpeechIfMute();
	void OC_getComputedVariantSpeechIfMute();
	void OC_startSpeechIfSilent();
	void OC_ComputeCharacterVariable();
	void OC_getRandom_type2();
	void OC_setCharacterPosition();
	void OC_DisableCharacter();
	void OC_saveAndQuit();
	void OC_sub17B93();
	void OC_startSpeech5();
	void OC_resetByte1714E();
	void OC_deleteSavegameAndQuit();
	void OC_incScriptForVal();
	void OC_sub17BA5();
	void OC_setByte18823();
	void OC_callScript();
	void OC_callScriptAndReturn();
	void OC_setCurrentScriptCharacterPos();
	void OC_initScriptFor();
	void OC_sub17AE1();
	void OC_sub17AEE();
	void OC_setWord10804();
	void OC_sub17C0E();
	void OC_sub17C55();
	void OC_sub17C76();
	void OC_setCurrentCharacter();
	void OC_sub17C8B();
	void OC_sub17CA2();
	void OC_sub17CB9();
	void OC_sub17CD1();
	void OC_resetWord16EFE();
	void OC_enableCurrentCharacterScript();
	void OC_IncCurrentCharacterVar1();
	void OC_sub17D23();
	void OC_sub17E6D();
	void OC_changeCurrentCharacterSprite();
	void OC_sub17E99();
	void OC_sub17EC5();
	void OC_setCharacterDirectionTowardsPos();
	void OC_turnCharacterTowardsAnother();
	void OC_sub17F4F();
	void OC_scrollAwayFromCharacter();
	void OC_skipNextVal();
	void OC_setCurrentCharacterVar6();
	void OC_sub17FDD();
	void OC_setCharacterScriptEnabled();
	void OC_setCurrentCharacterVar2();
	void OC_SetCurrentCharacterVar2ToZero();
	void OC_setCharacterProperties();
	void OC_sub1805D();
	void OC_sub18074();
	void OC_setCurrentCharacterDirection();
	void OC_setInterfaceHotspot();
	void OC_scrollViewPort();
	void OC_setViewPortPos();
	void OC_setCurrentCharacterAltitude();
	void OC_sub1817F();
	void sub1818B(Common::Point point);
	void OC_sub181BB();
	void OC_sub18213();
	void OC_sub18252();
	void OC_sub18260();
	void OC_CharacterVariableAddOrRemoveFlag();
	void OC_PaletteFadeOut();
	void OC_PaletteFadeIn();
	void OC_loadAndDisplayCUBESx_GFX();
	void OC_setCurrentCharacterVar3();
	void OC_setArray122C1();
	void OC_sub18367();
	void OC_enableCharacterScript();
	void OC_setRulesBuffer2Element();
	void OC_setDebugFlag();
	void OC_setByte14837();
	void OC_waitForEvent();
	void OC_disableInterfaceHotspot();
	void OC_loadFileAerial();
	void OC_startSpeechIfSoundOff();
	void OC_sub1844A();
	void OC_displayNumericCharacterVariable();
	void OC_displayVGAFile();
	void OC_startSpeechWithoutSpeeker();
	void OC_displayTitleScreen();
	void OC_initGameAreaDisplay();
	void OC_displayCharacterStatBar();
	void OC_initSmallAnim();
	void OC_setCharacterHeroismBar();
	void OC_sub18690();
	void OC_setViewPortCharacterTarget();
	void OC_sub186A1();
	void OC_sub186E5_snd();
	void OC_sub1870A_snd();
	void OC_sub18725_snd();
	void OC_sub18733_snd();
	void OC_sub1873F_snd();
	void OC_sub18746_snd();
	void OC_sub1875D_snd();
	void OC_setCharacterMapColor();
};

} // End of namespace Lilliput

#endif

