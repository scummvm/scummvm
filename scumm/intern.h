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
class NutRenderer; // V8 Font Renderer

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
	int getWordVararg(int *ptr);
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

	virtual void decodeParseString(int a, int b);
	int getStackList(int *args, uint maxnum);

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
	void o6_pop();
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
	void o6_startObjectEx();
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
	void o6_actorOps();
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
	void o6_delaySeconds();
	void o6_delayMinutes();
	void o6_stopSentence();
	void o6_printLine();
	void o6_printCursor();
	void o6_printDebug();
	void o6_printSystem();
	void o6_printActor();
	void o6_printEgo();
	void o6_talkActor();
	void o6_talkEgo();
	void o6_dim();
	void o6_dummy();
	void o6_startObjectQuick();
	void o6_startScriptQuick();
	void o6_dim2();
	void o6_abs();
	void o6_distObjectObject();
	void o6_distObjectPt();
	void o6_distPtPt();
	void o6_kernelSetFunctions();
	void o6_delayFrames();
	void o6_pickOneOf();
	void o6_pickOneOfDefault();
	void o6_jumpToScript();
	void o6_isRoomScriptRunning();
	void o6_kernelGetFunctions();
	void o6_getAnimateVariable();
	void o6_drawBlastObject();
	void o6_getActorLayer();
	void o6_unknownCD();
	void o6_bor();
	void o6_band();
	void o6_stopTalking();
	void o6_openFile();
	void o6_closeFile();
	void o6_deleteFile();
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

	virtual void setupScummVars();

	virtual uint fetchScriptWord();
	virtual int fetchScriptWordSigned();
	
	virtual int readVar(uint var);
	virtual void writeVar(uint var, int value);

	virtual void decodeParseString(int m, int n);

	/* Version 8 script opcodes */
	void o8_mod();
	void o8_wait();

	void o8_dim();
	void o8_dim2();
	void o8_arrayOps();
	void o8_blastText();

	void o8_cursorCommand();
	void o8_createBoxMatrix();
	void o8_resourceRoutines();
	void o8_roomOps();
	void o8_actorOps();
	void o8_cameraOps();
	void o8_verbOps();
	
	void o8_soundKludge();
	void o8_system();
	void o8_startVideo();
	void o8_kernelSetFunctions();
	void o8_kernelGetFunctions();
	void o8_getDateTime();

	void o8_getActorChore();

	void o8_drawObject();
	void o8_getObjectImageX();
	void o8_getObjectImageY();
	void o8_getObjectImageWidth();
	void o8_getObjectImageHeight();
	
	void o8_getStringWidth();

	/* V8 Scumm Vars */
	byte VAR_TIMEDATE_YEAR;
	byte VAR_TIMEDATE_MONTH;
	byte VAR_TIMEDATE_DAY;
	byte VAR_TIMEDATE_HOUR;
	byte VAR_TIMEDATE_MINUTE;
	byte VAR_TIMEDATE_SECOND;
};


#endif
