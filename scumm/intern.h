/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef INTERN_H
#define INTERN_H

#include "scumm.h"


class Scumm_v5 : public Scumm
{
protected:
	typedef void (Scumm_v5::*OpcodeProcV5)();
	struct OpcodeEntryV5 {
		OpcodeProcV5 proc;
		const char *desc;
	};
	
	const OpcodeEntryV5 *_opcodesV5;
	
public:
	Scumm_v5(GameDetector *detector, OSystem *syst) : Scumm(detector, syst) {}

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(int i);
	virtual const char *getOpcodeDesc(int i);

	void decodeParseString();
	int getWordVararg(int16 *ptr);
	int getVarOrDirectWord(byte mask);
	int getVarOrDirectByte(byte mask);

	/* Version 5 script opcodes */
	void o5_actorFollowCamera();
	void o5_actorFromPos();
	void o5_actorSet();
	void o5_add();
	void o5_and();
	void o5_animateActor();
	void o5_badOpcode();
	void o5_breakHere();
	void o5_chainScript();
	void o5_cursorCommand();
	void o5_cutscene();
	void o5_debug();
	void o5_decrement();
	void o5_delay();
	void o5_delayVariable();
	void o5_divide();
	void o5_doSentence();
	void o5_drawBox();
	void o5_drawObject();
	void o5_dummy();
	void o5_endCutscene();
	void o5_equalZero();
	void o5_expression();
	void o5_faceActor();
	void o5_findInventory();
	void o5_findObject();
	void o5_freezeScripts();
	void o5_getActorCostume();
	void o5_getActorElevation();
	void o5_getActorFacing();
	void o5_getActorMoving();
	void o5_getActorRoom();
	void o5_getActorScale();
	void o5_getActorWalkBox();
	void o5_getActorWidth();
	void o5_getActorX();
	void o5_getActorY();
	void o5_getAnimCounter();
	void o5_getClosestObjActor();
	void o5_getDist();
	void o5_getInventoryCount();
	void o5_getObjectOwner();
	void o5_getObjectState();
	void o5_getRandomNr();
	void o5_isScriptRunning();
	void o5_getVerbEntrypoint();
	void o5_ifClassOfIs();
	void o5_increment();
	void o5_isActorInBox();
	void o5_isEqual();
	void o5_isGreater();
	void o5_isGreaterEqual();
	void o5_isLess();
	void o5_isNotEqual();
	void o5_ifState();
	void o5_ifNotState();
	void o5_isSoundRunning();
	void o5_jumpRelative();
	void o5_lessOrEqual();
	void o5_lights();
	void o5_loadRoom();
	void o5_loadRoomWithEgo();
	void o5_matrixOps();
	void o5_move();
	void o5_multiply();
	void o5_notEqualZero();
	void o5_or();
	void o5_overRide();
	void o5_panCameraTo();
	void o5_pickupObject();
	void o5_print();
	void o5_printEgo();
	void o5_pseudoRoom();
	void o5_putActor();
	void o5_putActorAtObject();
	void o5_putActorInRoom();
	void o5_quitPauseRestart();
	void o5_resourceRoutines();
	void o5_roomOps();
	void o5_saveRestoreVerbs();
	void o5_setCameraAt();
	void o5_setClass();
	void o5_setObjectName();
	void o5_setOwnerOf();
	void o5_setState();
	void o5_setVarRange();
	void o5_soundKludge();
	void o5_startMusic();
	void o5_startObject();
	void o5_startScript();
	void o5_startSound();
	void o5_stopMusic();
	void o5_stopObjectCode();
	void o5_stopObjectScript();
	void o5_stopScript();
	void o5_stopSound();
	void o5_stringOps();
	void o5_subtract();
	void o5_verbOps();
	void o5_wait();
	void o5_walkActorTo();
	void o5_walkActorToActor();
	void o5_walkActorToObject();
    void o5_oldRoomEffect();
	void o5_pickupObjectOld();
};

// FIXME - subclassing V2 from Scumm_v5 is a hack: V2 should have its own opcode table
class Scumm_v2 : public Scumm_v5
{
public:
	Scumm_v2(GameDetector *detector, OSystem *syst) : Scumm_v5(detector, syst) {}

	virtual void readIndexFile();
};

// FIXME - maybe we should move the opcodes from v5 to v3, and change the inheritance 
// accordingly - that would be more logical I guess. However, if you do so, take care
// of preserving the right readIndexFile / loadCharset !!!
class Scumm_v3 : public Scumm_v5
{
public:
	Scumm_v3(GameDetector *detector, OSystem *syst) : Scumm_v5(detector, syst) {}

	void readIndexFile();
	virtual void loadCharset(int no);
};

class Scumm_v4 : public Scumm_v3
{
public:
	Scumm_v4(GameDetector *detector, OSystem *syst) : Scumm_v3(detector, syst) {}

	void loadCharset(int no);
};

class Scumm_v6 : public Scumm
{
protected:
	typedef void (Scumm_v6::*OpcodeProcV6)();
	struct OpcodeEntryV6 {
		OpcodeProcV6 proc;
		const char *desc;
	};
	
	const OpcodeEntryV6 *_opcodesV6;
	
public:
	Scumm_v6(GameDetector *detector, OSystem *syst) : Scumm(detector, syst) {}

protected:
	virtual void setupOpcodes();

	virtual void executeOpcode(int i);
	virtual const char *getOpcodeDesc(int i);

	int popRoomAndObj(int *room);

	void decodeParseString2(int a, int b);
	int getStackList(int16 *args, uint maxnum);

	/* Version 6 script opcodes */
	void o6_setBlastObjectWindow();
	void o6_pushByte();
	void o6_pushWord();
	void o6_pushByteVar();
	void o6_pushWordVar();
	void o6_invalid();
	void o6_byteArrayRead();
	void o6_wordArrayRead();
	void o6_byteArrayIndexedRead();
	void o6_wordArrayIndexedRead();
	void o6_dup();
	void o6_not();
	void o6_eq();
	void o6_neq();
	void o6_gt();
	void o6_lt();
	void o6_le();
	void o6_ge();
	void o6_add();
	void o6_sub();
	void o6_mul();
	void o6_div();
	void o6_land();
	void o6_lor();
	void o6_kill();
	void o6_writeByteVar();
	void o6_writeWordVar();
	void o6_byteArrayWrite();
	void o6_wordArrayWrite();
	void o6_byteArrayIndexedWrite();
	void o6_wordArrayIndexedWrite();
	void o6_byteVarInc();
	void o6_wordVarInc();
	void o6_byteArrayInc();
	void o6_wordArrayInc();
	void o6_byteVarDec();
	void o6_wordVarDec();
	void o6_byteArrayDec();
	void o6_wordArrayDec();
	void o6_jumpTrue();
	void o6_jumpFalse();
	void o6_jump();
	void o6_startScriptEx();
	void o6_startScript();
	void o6_startObject();
	void o6_setObjectState();
	void o6_setObjectXY();
	void o6_stopObjectCode();
	void o6_endCutscene();
	void o6_cutscene();
	void o6_stopMusic();
	void o6_freezeUnfreeze();
	void o6_cursorCommand();
	void o6_breakHere();
	void o6_ifClassOfIs();
	void o6_setClass();
	void o6_getState();
	void o6_setState();
	void o6_setOwner();
	void o6_getOwner();
	void o6_startSound();
	void o6_stopSound();
	void o6_startMusic();
	void o6_stopObjectScript();
	void o6_panCameraTo();
	void o6_actorFollowCamera();
	void o6_setCameraAt();
	void o6_loadRoom();
	void o6_stopScript();
	void o6_walkActorToObj();
	void o6_walkActorTo();
	void o6_putActorInRoom();
	void o6_putActorAtObject();
	void o6_faceActor();
	void o6_animateActor();
	void o6_doSentence();
	void o6_pickupObject();
	void o6_loadRoomWithEgo();
	void o6_getRandomNumber();
	void o6_getRandomNumberRange();
	void o6_getActorMoving();
	void o6_isScriptRunning();
	void o6_getActorRoom();
	void o6_getObjectX();
	void o6_getObjectY();
	void o6_getObjectOldDir();
	void o6_getObjectNewDir();
	void o6_getActorWalkBox();
	void o6_getActorCostume();
	void o6_findInventory();
	void o6_getInventoryCount();
	void o6_getVerbFromXY();
	void o6_beginOverride();
	void o6_endOverride();
	void o6_setObjectName();
	void o6_isSoundRunning();
	void o6_setBoxFlags();
	void o6_createBoxMatrix();
	void o6_resourceRoutines();
	void o6_roomOps();
	void o6_actorSet();
	void o6_verbOps();
	void o6_getActorFromXY();
	void o6_findObject();
	void o6_pseudoRoom();
	void o6_getActorElevation();
	void o6_getVerbEntrypoint();
	void o6_arrayOps();
	void o6_saveRestoreVerbs();
	void o6_drawBox();
	void o6_getActorWidth();
	void o6_wait();
	void o6_getActorScaleX();
	void o6_getActorAnimCounter1();
	void o6_soundKludge();
	void o6_isAnyOf();
	void o6_quitPauseRestart();
	void o6_isActorInBox();
	void o6_delay();
	void o6_delayLonger();
	void o6_delayVeryLong();
	void o6_stopSentence();
	void o6_print_0();
	void o6_print_1();
	void o6_print_2();
	void o6_print_3();
	void o6_printActor();
	void o6_printEgo();
	void o6_talkActor();
	void o6_talkEgo();
	void o6_dim();
	void o6_dummy();
	void o6_runVerbCodeQuick();
	void o6_runScriptQuick();
	void o6_dim2();
	void o6_abs();
	void o6_distObjectObject();
	void o6_distObjectPt();
	void o6_distPtPt();
	void o6_miscOps();
	void o6_delayFrames();
	void o6_pickOneOf();
	void o6_pickOneOfDefault();
	void o6_jumpToScript();
	void o6_isRoomScriptRunning();
	void o6_kernelFunction();
	void o6_getAnimateVariable();
	void o6_drawBlastObject();
	void o6_getActorPriority();
	void o6_unknownCD();
	void o6_bor();
	void o6_band();
};

class Scumm_v7 : public Scumm_v6
{
public:
	Scumm_v7(GameDetector *detector, OSystem *syst) : Scumm_v6(detector, syst) {}

protected:
	virtual void setupScummVars();
};

class Scumm_v8 : public Scumm_v6
{
protected:
	typedef void (Scumm_v8::*OpcodeProcV8)();
	struct OpcodeEntryV8 {
		OpcodeProcV8 proc;
		const char *desc;
	};
	
	const OpcodeEntryV8 *_opcodesV8;
	
public:
	Scumm_v8(GameDetector *detector, OSystem *syst) : Scumm_v6(detector, syst) {}

protected:
	virtual void setupOpcodes();

	virtual void executeOpcode(int i);
	virtual const char *getOpcodeDesc(int i);

	virtual uint32 fetchScriptUint32();
	virtual int fetchScriptInt32();

	/* Version 8 script opcodes */
	void o8_unknown();
	void o8_invalid();

	void o8_pushNumber();
	void o8_pushVariable();
	void o8_pushArrayValue();
	void o8_pushArrayValueIndexed();
	void o8_dup();
	void o8_pop();
	void o8_not();
	void o8_eq();
	void o8_neq();
	void o8_gt();
	void o8_lt();
	void o8_leq();
	void o8_geq();
	void o8_add();
	void o8_sub();
	void o8_mul();
	void o8_div();
	void o8_land();
	void o8_lor();
	void o8_band();
	void o8_bor();
	void o8_mod();
	void o8_if();
	void o8_ifNot();
	void o8_jump();
	void o8_breakHere();
	void o8_breakHereVar();
	void o8_waitForStuff();
	void o8_sleepJiffies();
	void o8_sleepSeconds();
	void o8_sleepMinutes();
	void o8_storeVariable();
	void o8_incVariable();
	void o8_decVariable();
	void o8_arrayDim();
	void o8_storeArray();
	void o8_incArray();
	void o8_decArray();
	void o8_arrayDim2();
	void o8_storeArrayIndexed();
	void o8_assignArray();
	void o8_arrayShuffle();
	void o8_arrayLocalize();
	void o8_startScript();
	void o8_startScriptQuick();
	void o8_endScript();
	void o8_stopScript();
	void o8_chainScript();
	void o8_return();
	void o8_startObject();
	void o8_stopObject();
	void o8_cutscene();
	void o8_endCutscene();
	void o8_freezeScripts();
	void o8_beginOverride();
	void o8_endOverride();
	void o8_stopSentence();
	void o8_debug();
	void o8_debugWin();
	void o8_cameraPanTo();
	void o8_cameraFollow();
	void o8_cameraAt();
	void o8_sayLine();
	void o8_sayLineDefault();
	void o8_sayLineSimple();
	void o8_sayLineSimpleDefault();
	void o8_printLine();
	void o8_printCursor();
	void o8_printDebug();
	void o8_printSystem();
	void o8_blastText();
	void o8_drawObject();
	void o8_blastObject();
	void o8_userFace();
	void o8_currentRoom();
	void o8_comeOutDoor();
	void o8_walkActorToObject();
	void o8_walkActorToXY();
	void o8_putActorAtXY();
	void o8_putActorAtObject();
	void o8_faceTowards();
	void o8_doAnimation();
	void o8_doSentence();
	void o8_pickUpObject();
	void o8_setBox();
	void o8_setBoxPath();
	void o8_setBoxSet();
	void o8_heapStuff();
	void o8_roomStuff();
	void o8_actorStuff();
	void o8_cameraStuff();
	void o8_verbStuff();
	void o8_startSfx();
	void o8_startMusic();
	void o8_stopSound();
	void o8_soundKludge();
	void o8_system();
	void o8_verbSets();
	void o8_newNameOf();
	void o8_getTimeDate();
	void o8_drawBox();
	void o8_actObjStamp();
	void o8_startVideo();
	void o8_kludge();
	void o8_pick();
	void o8_pickDefault();
	void o8_pickRandom();
	void o8_inSet();
	void o8_getRandomNumber();
	void o8_getRandomNumberRange();
	void o8_classOf();
	void o8_stateOf();
	void o8_ownerOf();
	void o8_scriptRunning();
	void o8_objectRunning();
	void o8_soundRunning();
	void o8_abs();
	void o8_pixel();
	void o8_inBox();
	void o8_validVerb();
	void o8_findActor();
	void o8_findObject();
	void o8_findVerb();
	void o8_findAllObjects();
	void o8_actorInvertory();
	void o8_actorInvertoryCount();
	void o8_actorVariable();
	void o8_actorRoom();
	void o8_actorBox();
	void o8_actorMoving();
	void o8_actorCostume();
	void o8_actorScale();
	void o8_actorDepth();
	void o8_actorElevation();
	void o8_actorWidth();
	void o8_actObjFacing();
	void o8_actObjX();
	void o8_actObjY();
	void o8_actorChore();
	void o8_proximity2ActObjs();
	void o8_proximity2Points();
	void o8_objectImageX();
	void o8_objectImageY();
	void o8_objectImageWidth();
	void o8_objectImageHeight();
	void o8_verbX();
	void o8_verbY();
	void o8_stringWidth();
	void o8_actorZPlane();
};

#endif
