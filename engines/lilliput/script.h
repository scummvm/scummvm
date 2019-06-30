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
	kButtonPressed = 1,
	kButtonReleased = 2,
	kActionTalk = 3,
	kActionGoto = 4,
	kCubeSelected = 5,
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

struct EvaluatedMode {
	int _mode;
	int _priority;
};

class LilliputScript {
public:
	byte _heroismLevel;
	byte _speechTimer;

	byte _characterScriptEnabled[40];
	int8 _interfaceHotspotStatus[20];
	Common::Point _characterTilePos[40];
	int8 _characterNextSequence[40];
	int8 _characterPose[40];
	byte _interfaceButtonActivationDelay[20];
	byte _array122C1[40];
	byte _characterSeek[40];
	int16 _interactions[40 * 40];

	byte *_barAttrPtr;

	Common::Point _viewportPos;
	int16 _viewportCharacterTarget;
	int16 _talkingCharacter;
	int _heroismBarX;
	int _heroismBarBottomY;

	Common::Point _sequenceArr[640];
	byte _characterMapPixelColor[40];
	int8 _characterLastSequence[40];
	EvaluatedMode _newEvaluatedModes[32];

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
	byte _cubeSet;
	byte _lastRandomValue;
	byte _scriptForVal;
	byte _textVarNumber;
	byte _speechDisplaySpeed;

	int16 _word16F00_characterId;
	int _currentSpeechId;
	int _word18821;
	int _monitoredCharacter;
	Common::Point _word1825E;

	char _monitoredAttr[4];

	int handleOpcode(ScriptStream *script);
	byte handleOpcodeType1(int curWord);
	void handleOpcodeType2(int curWord);

	void enableCharacterScript(byte index, byte var1, byte *curBufPtr);
	void skipOpcodes(int var1);
	void copySequence(int index, byte *buf);
	void setSequence(int charIdx, int8 seqIdx);
	void checkSpeechAllowed(bool &forceReturnFl);
	void decodePackedText(char *buf);
	void startSpeech(int var);
	void displayNumber(byte var1, Common::Point pos);
	byte *getMapPtr(Common::Point val);
	byte *getCurrentCharacterVarFromScript();
	void sendSignal(int16 var1, byte var2h, byte characterId, int16 var4);
	void getSpeechVariant(int speechIndex, int speechVariant);
	void showSpeech();
	void formatSpeechString();
	Common::Point getCharacterTilePos(int index);
	int getPackedStringStartRelativeIndex(int index);

	int16 getValue1();
	Common::Point getPosFromScript();

	byte *getCharacterAttributesPtr();
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
	byte OC_isSequenceActive();
	byte OC_isSequenceFinished();
	byte OC_CompareMapValueWith();
	byte OC_IsCharacterValid();
	byte OC_CheckWaitingSignal();
	byte OC_CurrentCharacterVar0AndVar1Equals();
	byte OC_CurrentCharacterVar0Equals();
	byte OC_checkLastInterfaceHotspotIndexMenu13();
	byte OC_checkLastInterfaceHotspotIndexMenu2();
	byte OC_CompareNumberOfCharacterWithVar0Equals();
	byte OC_IsPositionInViewport();
	byte OC_CompareGameVariables();
	byte OC_skipNextOpcode();
	byte OC_CheckCurrentCharacterAttr2();
	byte OC_CheckCurrentCharacterType();
	byte OC_CheckCurrentCharacterAttr0And();
	byte OC_IsCurrentCharacterAttr0LessEqualThan();
	byte OC_isCarried();
	byte OC_CheckCurrentCharacterAttr1();
	byte OC_isCurrentCharacterSpecial();
	byte OC_CurrentCharacterAttr3Equals1();
	byte OC_checkCharacterDirection();
	byte OC_checkLastInterfaceHotspotIndex();
	byte OC_checkSelectedCharacter();
	byte OC_checkDelayedReactivation();
	byte OC_checkTargetReached();
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
	void OC_setAttributeToRandom();
	void OC_setCharacterPosition();
	void OC_DisableCharacter();
	void OC_saveAndQuit();
	void OC_nSkipOpcodes();
	void OC_startSpeech5();
	void OC_resetHandleOpcodeFlag();
	void OC_deleteSavegameAndQuit();
	void OC_incScriptForVal();
	void OC_computeChararacterAttr();
	void OC_setTextVarNumber();
	void OC_callScript();
	void OC_callScriptAndReturn();
	void OC_setCurrentScriptCharacterPos();
	void OC_initScriptFor();
	void OC_setCurrentCharacterSequence();
	void OC_setNextCharacterSequence();
	void OC_setHost();
	void OC_changeMapCube();
	void OC_setCharacterCarry();
	void OC_dropCarried();
	void OC_setCurrentCharacter();
	void OC_sendSeeSignal();
	void OC_sendHearSignal();
	void OC_sendVarSignal();
	void OC_sendBroadcastSignal();
	void OC_resetWaitingSignal();
	void OC_enableCurrentCharacterScript();
	void OC_IncCurrentCharacterVar1();
	void OC_setCurrentCharacterPos();
	void OC_setCurrentCharacterBehavior();
	void OC_changeCurrentCharacterSprite();
	void OC_getList();
	void OC_setList();
	void OC_setCharacterDirectionTowardsPos();
	void OC_turnCharacterTowardsAnother();
	void OC_setSeek();
	void OC_scrollAwayFromCharacter();
	void OC_skipNextVal();
	void OC_setCurrentCharacterAttr6();
	void OC_setCurrentCharacterPose();
	void OC_setCharacterScriptEnabled();
	void OC_setCurrentCharacterAttr2();
	void OC_clearCurrentCharacterAttr2();
	void OC_setCharacterProperties();
	void OC_setMonitoredCharacter();
	void OC_setNewPose();
	void OC_setCurrentCharacterDirection();
	void OC_setInterfaceHotspot();
	void OC_scrollViewPort();
	void OC_setViewPortPos();
	void OC_setCurrentCharacterAltitude();
	void OC_setModePriority();
	void setMode(EvaluatedMode newMode);
	void OC_setComputedModePriority();
	void OC_selectBestMode();
	void OC_magicPuffEntrance();
	void OC_spawnCharacterAtPos();
	void OC_CharacterVariableAddOrRemoveFlag();
	void OC_PaletteFadeOut();
	void OC_PaletteFadeIn();
	void OC_loadAndDisplayCubesGfx();
	void OC_setCurrentCharacterAttr3();
	void OC_setArray122C1();
	void OC_sub18367();
	void OC_enableCharacterScript();
	void OC_setRulesBuffer2Element();
	void OC_setDebugFlag();
	void OC_setDebugFlag2();
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
	void OC_setCharacterHome();
	void OC_setViewPortCharacterTarget();
	void OC_showObject();
	void OC_playObjectSound();
	void OC_startLocationSound();
	void OC_stopObjectSound();
	void OC_stopLocationSound();
	void OC_toggleSound();
	void OC_playMusic();
	void OC_stopMusic();
	void OC_setCharacterMapColor();
};

} // End of namespace Lilliput

#endif

