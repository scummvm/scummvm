/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "scumm/scumm.h"
#include "scumm/wiz_he.h"

namespace Scumm {

class NutRenderer; // V8 Font Renderer
struct ArrayHeader;

// This is to help devices with small memory (PDA, smartphones, ...)
// to save abit of memory used by opcode names in the Scumm engine.
#ifndef REDUCE_MEMORY_USAGE
#	define _OPCODE(ver, x)	{ &ver::x, #x }
#else
#	define _OPCODE(ver, x)	{ &ver::x, "" }
#endif

class ScummEngine_v5 : public ScummEngine {
protected:
	typedef void (ScummEngine_v5::*OpcodeProcV5)();
	struct OpcodeEntryV5 {
		OpcodeProcV5 proc;
		const char *desc;
	};
	
	const OpcodeEntryV5 *_opcodesV5;
	
	uint16 _cursorImages[4][16];
	byte _cursorHotspots[2 * 4];

public:
	ScummEngine_v5(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]);

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void decodeParseString();

	int getWordVararg(int *ptr);
	void saveVars();
	void loadVars();

	virtual int getVar();
	virtual int getVarOrDirectByte(byte mask);
	virtual int getVarOrDirectWord(byte mask);

	virtual void animateCursor();

	void setBuiltinCursor(int index);
	void redefineBuiltinCursorFromChar(int index, int chr);
	void redefineBuiltinCursorHotspot(int index, int x, int y);
	void saveOrLoadCursorImages(Serializer *s);

	/* Version 5 script opcodes */
	void o5_actorFollowCamera();
	void o5_actorFromPos();
	void o5_actorOps();
	void o5_add();
	void o5_and();
	void o5_animateActor();
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
	void o5_getStringWidth();
	void o5_getVerbEntrypoint();
	void o5_ifClassOfIs();
	void o5_ifNotState();
	void o5_ifState();
	void o5_increment();
	void o5_isActorInBox();
	void o5_isEqual();
	void o5_isGreater();
	void o5_isGreaterEqual();
	void o5_isLess();
	void o5_isNotEqual();
	void o5_isScriptRunning();
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
	void o5_oldRoomEffect();
	void o5_or();
	void o5_beginOverride();
	void o5_panCameraTo();
	void o5_pickupObject();
	void o5_pickupObjectOld();
	void o5_print();
	void o5_printEgo();
	void o5_pseudoRoom();
	void o5_putActor();
	void o5_putActorAtObject();
	void o5_putActorInRoom();
	void o5_quitPauseRestart();
	void o5_resourceRoutines();
	void o5_roomOps();
	void o5_saveLoadGame();
	void o5_saveLoadVars();
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
};

class ScummEngine_v3 : public ScummEngine_v5 {
public:
	ScummEngine_v3(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) : ScummEngine_v5(detector, syst, gs, md5sum) {}

protected:
	void readIndexFile();
	void loadCharset(int no);
	void readMAXS();
	
	void readGlobalObjects();
};

class ScummEngine_v2 : public ScummEngine_v3 {
protected:
	void readIndexFile();
	void readClassicIndexFile();	// V1
	void readEnhancedIndexFile();	// V2
	void loadCharset(int no);
	void readMAXS();

	typedef void (ScummEngine_v2::*OpcodeProcV2)();
	struct OpcodeEntryV2 {
		OpcodeProcV2 proc;
		const char *desc;
	};

	const OpcodeEntryV2 *_opcodesV2;

public:
	ScummEngine_v2(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) : ScummEngine_v3(detector, syst, gs, md5sum) {}

	virtual void scummInit();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void decodeParseString();

	virtual int getVar();

	void getResultPosIndirect();
	virtual void getResultPos();
	virtual int readVar(uint var);
	virtual void writeVar(uint var, int value);

	virtual void ifStateCommon(byte type);
	virtual void ifNotStateCommon(byte type);
	virtual void setStateCommon(byte type);
	virtual void clearStateCommon(byte type);
	
	void resetSentence();
	void setUserState(byte state);

	void initV2MouseOver();

	/* Version 2 script opcodes */
	void o2_actorFromPos();
	void o2_actorOps();
	void o2_add();
	void o2_addIndirect();
	void o2_animateActor();
	void o2_assignVarByte();
	void o2_assignVarWordIndirect();
	void o2_beginOverride();
	void o2_chainScript();
	void o2_clearState01();
	void o2_clearState02();
	void o2_clearState04();
	void o2_clearState08();
	void o2_cursorCommand();
	void o2_cutscene();
	void o2_delay();
	void o2_doSentence();
	void o2_drawObject();
	void o2_drawSentence();
	void o2_dummy();
	void o2_endCutscene();
	void o2_findObject();
	void o2_getActorWalkBox();
	void o2_getActorX();
	void o2_getActorY();
	void o2_getBitVar();
	void o2_getObjPreposition();
	void o2_ifClassOfIs();
	void o2_ifNotState01();
	void o2_ifNotState02();
	void o2_ifNotState04();
	void o2_ifNotState08();
	void o2_ifState01();
	void o2_ifState02();
	void o2_ifState04();
	void o2_ifState08();
	void o2_isGreater();
	void o2_isGreaterEqual();
	void o2_isLess();
	void o2_isLessEqual();
	void o2_lights();
	void o2_loadRoomWithEgo();
	void o2_setBoxFlags();
	void o2_panCameraTo();
	void o2_pickupObject();
	void o2_putActor();
	void o2_putActorAtObject();
	void o2_resourceRoutines();
	void o2_restart();
	void o2_roomOps();
	void o2_getActorElevation();
	void o2_setActorElevation();
	void o2_setBitVar();
	void o2_setCameraAt();
	void o2_setObjPreposition();
	void o2_setOwnerOf();
	void o2_setState01();
	void o2_setState02();
	void o2_setState04();
	void o2_setState08();
	void o2_startScript();
	void o2_stopScript();
	void o2_subtract();
	void o2_subIndirect();
	void o2_switchCostumeSet();
	void o2_verbOps();
	void o2_waitForActor();
	void o2_waitForMessage();
	void o2_waitForSentence();
	void o2_walkActorTo();
	void o2_walkActorToObject();

	byte VAR_SENTENCE_VERB;
	byte VAR_SENTENCE_OBJECT1;
	byte VAR_SENTENCE_OBJECT2;
	byte VAR_SENTENCE_PREPOSITION;
	byte VAR_BACKUP_VERB;
};

class ScummEngine_v4 : public ScummEngine_v3 {
public:
	ScummEngine_v4(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) : ScummEngine_v3(detector, syst, gs, md5sum) {}

protected:
	void loadCharset(int no);
};

class ScummEngine_v6 : public ScummEngine {
	friend class Insane;

protected:
	typedef void (ScummEngine_v6::*OpcodeProcV6)();
	struct OpcodeEntryV6 {
		OpcodeProcV6 proc;
		const char *desc;
	};
	
	enum ArrayType {
		kBitArray = 1,
		kNibbleArray = 2,
		kByteArray = 3,
		kStringArray = 4,
		kIntArray = 5,
		kDwordArray = 6
	};
	
	#if !defined(__GNUC__)
		#pragma START_PACK_STRUCTS
	#endif	
	
	struct ArrayHeader {
		int16 dim1;
		int16 type;
		int16 dim2;
		byte data[1];
	} GCC_PACK;
	
	#if !defined(__GNUC__)
		#pragma END_PACK_STRUCTS
	#endif

	const OpcodeEntryV6 *_opcodesV6;

	int _smushFrameRate;

public:
	ScummEngine_v6(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]);

	virtual void scummInit();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void decodeParseString(int a, int b);
	virtual void readArrayFromIndexFile();

	virtual void palManipulateInit(int resID, int start, int end, int time);

	int getStackList(int *args, uint maxnum);
	int popRoomAndObj(int *room);

	ArrayHeader *getArray(int array);
	ArrayHeader *defineArray(int array, int type, int dim2, int dim1);
	int findFreeArrayId();
	void nukeArray(int array);
	virtual int readArray(int array, int index, int base);
	virtual void writeArray(int array, int index, int base, int value);
	void shuffleArray(int num, int minIdx, int maxIdx);

	void setCursorTransparency(int a);
	void setCursorHotspot(int x, int y);

	virtual void setCursorFromImg(uint img, uint room, uint imgindex);
	void useIm01Cursor(const byte *im, int w, int h);
	void useBompCursor(const byte *im, int w, int h);
	void grabCursor(int x, int y, int w, int h);

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
	void o6_if();
	void o6_ifNot();
	void o6_jump();
	void o6_startScript();
	void o6_startScriptQuick();
	void o6_startObject();
	void o6_drawObject();
	void o6_drawObjectAt();
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
	void o6_putActorAtXY();
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
	void o6_dimArray();
	void o6_dummy();
	void o6_startObjectQuick();
	void o6_startScriptQuick2();
	void o6_dim2dimArray();
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
	void o6_stampObject();
	void o6_bor();
	void o6_band();
	void o6_stopTalking();
	void o6_findAllObjects();
	void o6_pickVarRandom();
	void o6_getDateTime();
	void o6_getPixel();
	void o6_setBoxSet();
	void o6_shuffle();

	byte VAR_VIDEONAME;
	byte VAR_RANDOM_NR;
	byte VAR_STRING2DRAW;

	byte VAR_TIMEDATE_YEAR;
	byte VAR_TIMEDATE_MONTH;
	byte VAR_TIMEDATE_DAY;
	byte VAR_TIMEDATE_HOUR;
	byte VAR_TIMEDATE_MINUTE;
	byte VAR_TIMEDATE_SECOND;
};

class ScummEngine_v60he : public ScummEngine_v6 {
protected:
	typedef void (ScummEngine_v60he::*OpcodeProcv60he)();
	struct OpcodeEntryv60he {
		OpcodeProcv60he proc;
		const char *desc;
	};

	const OpcodeEntryv60he *_opcodesv60he;

	File _hFileTable[17];
	
public:
	ScummEngine_v60he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) : ScummEngine_v6(detector, syst, gs, md5sum) {}

	virtual void scummInit();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	void localizeArray(int slot, byte script);
	void redimArray(int arrayId, int newX, int newY, int d);
	int readFileToArray(int slot, int32 size);
	void writeFileFromArray(int slot, int resID);
	int virtScreenSave(byte *dst, int x1, int y1, int x2, int y2);
	void virtScreenLoad(int resIdx, int x1, int y1, int x2, int y2);
	void seekFilePos(int slot, int offset, int mode);
	virtual void decodeParseString(int a, int b);
	void swapObjects(int object1, int object2);

	/* HE version 60 script opcodes */
	void o60_setState();
	void o60_roomOps();
	void o60_actorOps();
	void o60_wait();
	void o60_kernelSetFunctions();
	void o60_kernelGetFunctions();
	void o60_openFile();
	void o60_closeFile();
	void o60_deleteFile();
	void o60_readFile();
	void o60_rename();
	void o60_writeFile();
	void o60_soundOps();
	void o60_seekFilePos();
	void o60_localizeArrayToScript();
	void o60_redimArray();
	void o60_readFilePos();
};

class ScummEngine_v70he : public ScummEngine_v60he {
	friend class Win32ResExtractor;
	friend class MacResExtractor;

protected:
	typedef void (ScummEngine_v70he::*OpcodeProcv70he)();
	struct OpcodeEntryv70he {
		OpcodeProcv70he proc;
		const char *desc;
	};
	
	const OpcodeEntryv70he *_opcodesv70he;

	Win32ResExtractor *_win32ResExtractor;
	MacResExtractor *_macResExtractor;

	int _heSndSoundFreq, _heSndOffset, _heSndChannel, _heSndSoundId, _heSndFlags;

public:
	ScummEngine_v70he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]);

	Wiz _wiz;

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);
	
	virtual void redrawBGAreas();

	void appendSubstring(int dst, int src, int len2, int len);

	int findObject(int x, int y, int num, int *args);

	virtual void setCursorFromImg(uint img, uint room, uint imgindex);

	/* HE version 70 script opcodes */
	void o70_startSound();
	void o70_pickupObject();
	void o70_getActorRoom();
	void o70_resourceRoutines();
	void o70_findObject();
	void o70_quitPauseRestart();
	void o70_kernelSetFunctions();
	void o70_getStringWidth();
	void o70_getStringLen();
	void o70_appendString();
	void o70_compareString();
	void o70_readINI();
	void o70_writeINI();
	void o70_getStringLenForWidth();
	void o70_getCharIndexInString();
	void o70_setFilePath();
	void o70_setWindowCaption();
	void o70_polygonOps();
	void o70_polygonHit();
};

class ScummEngine_v72he : public ScummEngine_v70he {
protected:
	typedef void (ScummEngine_v72he::*OpcodeProcV72he)();
	struct OpcodeEntryV72he {
		OpcodeProcV72he proc;
		const char *desc;
	};
	
#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif	

	struct ArrayHeader {
		int32 type;      //0
		int32 dim1start; //4
		int32 dim1end;   //8
		int32 dim2start; //0C
		int32 dim2end;   //10
		byte data[1];    //14
	} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

	const OpcodeEntryV72he *_opcodesV72he;

public:
	ScummEngine_v72he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) : ScummEngine_v70he(detector, syst, gs, md5sum) {}

protected:
	virtual void setupScummVars();
	virtual void readArrayFromIndexFile();

	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);
	
	virtual void redrawBGAreas();

	ArrayHeader *defineArray(int array, int type, int dim2start, int dim2end, int dim1start, int dim1end);
	virtual int readArray(int array, int idx2, int idx1);
	virtual void writeArray(int array, int idx2, int idx1, int value);
	void redimArray(int arrayId, int newDim2start, int newDim2end, 
					int newDim1start, int newDim1end, int type);
	int readFileToArray(int slot, int32 size);
	void writeFileFromArray(int slot, int resID);

	void captureWizImage(int restype, int resnum, const Common::Rect& r, bool frontBuffer, int compType);
	void displayWizImage(const WizImage *pwi);
	void getWizImageDim(int resnum, int state,  int32 &w, int32 &h);
	uint8 *drawWizImage(int restype, const WizImage *pwi);
	void drawWizPolygon(int resnum, int state, int id, int flags);
	void flushWizBuffer();

	virtual void decodeParseString(int a, int b);
	void decodeScriptString(byte *dst, bool scriptString = false);
	void copyScriptString(byte *dst);

	const byte *findWrappedBlock(uint32 tag, const byte *ptr, int state, bool flagError);

	/* HE version 72 script opcodes */
	void o72_pushDWord();
	void o72_addMessageToStack();
	void o72_isAnyOf();
	void o72_resetCutscene();
	void o72_findObjectWithClassOf();
	void o72_getObjectImageX();
	void o72_getObjectImageY();
	void o72_captureWizImage();
	void o72_getTimer();
	void o72_setTimer();
	void o72_getSoundElapsedTimeOfSound();
	void o72_startScript();
	void o72_startObject();
	void o72_drawObject();
	void o72_printWizImage();
	void o72_getArrayDimSize();
	void o72_getNumFreeArrays();
	void o72_roomOps();
	void o72_actorOps();
	void o72_verbOps();
	void o72_arrayOps();
	void o72_dimArray();
	void o72_dim2dimArray();
	void o72_traceStatus();
	void o72_unknownCF();
	void o72_drawWizImage();
	void o72_kernelGetFunctions();
	void o72_jumpToScript();
	void o72_openFile();
	void o72_readFile();
	void o72_writeFile();
	void o72_findAllObjects();
	void o72_deleteFile();
	void o72_getPixel();
	void o72_pickVarRandom();
	void o72_redimArray();
	void o72_copyString();
	void o72_appendString();
	void o72_concatString();
	void o72_checkGlobQueue();
	void o72_readINI();
	void o72_writeINI();
	void o72_unknownF5();
	void o72_unknownF6();
	void o72_getResourceSize();
	void o72_setFilePath();
	void o72_setWindowCaption();
};

class ScummEngine_v80he : public ScummEngine_v72he {
protected:
	typedef void (ScummEngine_v80he::*OpcodeProcV80he)();
	struct OpcodeEntryV80he {
		OpcodeProcV80he proc;
		const char *desc;
	};
	
	const OpcodeEntryV80he *_opcodesV80he;

public:
	ScummEngine_v80he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) : ScummEngine_v72he(detector, syst, gs, md5sum) {}

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	void loadImgSpot(int resId, int state, int16 &x, int16 &y);
	void loadWizCursor(int resId, int resType, bool state);
	
	/* HE version 80 script opcodes */
	void o80_loadSBNG();
	void o80_stringToInt();
	void o80_getSoundVar();
	void o80_localizeArrayToRoom();
	void o80_readConfigFile();
	void o80_writeConfigFile();
	void o80_cursorCommand();
	void o80_setState();
	void o80_drawBox();
	void o80_drawWizPolygon();
	void o80_unknownE0();
	void o80_pickVarRandom();
};

class ScummEngine_v90he : public ScummEngine_v80he {
protected:
	typedef void (ScummEngine_v90he::*OpcodeProcV90he)();
	struct OpcodeEntryV90he {
		OpcodeProcV90he proc;
		const char *desc;
	};

	int _heObject, _heObjectNum;
	int _hePaletteNum;
	
	const OpcodeEntryV90he *_opcodesV90he;
	WizParameters _wizParams;

public:
	ScummEngine_v90he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) : ScummEngine_v80he(detector, syst, gs, md5sum) {}

	virtual void scummInit();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);
	
	void drawWizComplexPolygon(int resnum, int state, int po_x, int po_y, int arg14, int angle, int zoom, const Common::Rect *r);
	void displayWizComplexImage(const WizParameters *params);
	void processWizImage(const WizParameters *params);
	int getWizImageStates(int resnum);	
	int isWizPixelNonTransparent(int restype, int resnum, int state, int x, int y, int flags);
	uint8 getWizPixelColor(int restype, int resnum, int state, int x, int y, int flags);
	int computeWizHistogram(int resnum, int state, int x, int y, int w, int h);
	
	/* HE version 90 script opcodes */
	void o90_dup();
	void o90_min();
	void o90_max();
	void o90_sin();
	void o90_cos();
	void o90_sqrt();
	void o90_atan2();
	void o90_getSegmentAngle();
	void o90_startScriptUnk();
	void o90_jumpToScriptUnk();
	void o90_wizImageOps();
	void o90_getDistanceBetweenPoints();
	void o90_unknown25();
	void o90_unknown26();
	void o90_unknown27();
	void o90_unknown28();
	void o90_getWizData();
	void o90_unknown2F();
	void o90_mod();
	void o90_shl();
	void o90_shr();
	void o90_findAllObjectsWithClassOf();
	void o90_getPolygonOverlap();
	void o90_cond();
	void o90_dim2dim2Array();
	void o90_sortArray();
	void o90_getObjectData();
	void o90_getPaletteData();
	void o90_paletteOps();
	void o90_unknownA5();
	void o90_getActorAnimProgress();
};

class ScummEngine_v100he : public ScummEngine_v90he {
protected:
	typedef void (ScummEngine_v100he::*OpcodeProcV100he)();
	struct OpcodeEntryV100he {
		OpcodeProcV100he proc;
		const char *desc;
	};

	int _heResId, _heResType;

	const OpcodeEntryV100he *_opcodesV100he;

public:
	ScummEngine_v100he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) : ScummEngine_v90he(detector, syst, gs, md5sum) {}

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);
	
	virtual void decodeParseString(int a, int b);

	/* HE version 100 script opcodes */
	void o100_actorOps();
	void o100_arrayOps();
	void o100_dim2dimArray();
	void o100_dimArray();
	void o100_drawObject();
	void o100_unknown28();
	void o100_resourceRoutines();
	void o100_wizImageOps();
	void o100_loadSBNG();
	void o100_dim2dim2Array();
	void o100_paletteOps();
	void o100_redimArray();
	void o100_roomOps();
	void o100_startSound();
	void o100_unknown26();
	void o100_quitPauseRestart();
	void o100_cursorCommand();
	void o100_wait();
	void o100_writeFile();
	void o100_getResourceSize();
	void o100_unknown27();
	void o100_getPaletteData();
	void o100_readFile();
	void o100_unknown25();
	void o100_unknown29();
};

class ScummEngine_v7 : public ScummEngine_v6 {
public:
	ScummEngine_v7(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) : ScummEngine_v6(detector, syst, gs, md5sum) {}

protected:
	virtual void setupScummVars();

	virtual void setCameraAt(int pos_x, int pos_y);
	virtual void setCameraFollows(Actor *a);
	virtual void moveCamera();
	virtual void panCameraTo(int x, int y);
};

class ScummEngine_v8 : public ScummEngine_v7 {
protected:
	typedef void (ScummEngine_v8::*OpcodeProcV8)();
	struct OpcodeEntryV8 {
		OpcodeProcV8 proc;
		const char *desc;
	};
	
	const OpcodeEntryV8 *_opcodesV8;
	
public:
	ScummEngine_v8(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) : ScummEngine_v7(detector, syst, gs, md5sum) {}

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void decodeParseString(int m, int n);
	virtual void readArrayFromIndexFile();

	virtual uint fetchScriptWord();
	virtual int fetchScriptWordSigned();
	
	virtual int readVar(uint var);
	virtual void writeVar(uint var, int value);

	/* Version 8 script opcodes */
	void o8_mod();
	void o8_wait();

	void o8_dimArray();
	void o8_dim2dimArray();
	void o8_arrayOps();
	void o8_blastText();

	void o8_cursorCommand();
	void o8_resourceRoutines();
	void o8_roomOps();
	void o8_actorOps();
	void o8_cameraOps();
	void o8_verbOps();
	
	void o8_system();
	void o8_startVideo();
	void o8_kernelSetFunctions();
	void o8_kernelGetFunctions();

	void o8_getActorChore();
	void o8_getActorZPlane();

	void o8_drawObject();
	void o8_getObjectImageX();
	void o8_getObjectImageY();
	void o8_getObjectImageWidth();
	void o8_getObjectImageHeight();
	
	void o8_getStringWidth();

};

} // End of namespace Scumm

#endif
