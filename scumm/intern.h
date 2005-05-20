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
	ScummEngine_v5(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void initScummVars();
	virtual void decodeParseString();

	virtual void saveOrLoad(Serializer *s, uint32 savegameVersion);

	virtual void readMAXS(int blockSize);

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

/**
 * Engine for version 4 SCUMM games; GF_SMALL_HEADER is always set for these.
 */
class ScummEngine_v4 : public ScummEngine_v5 {
public:
	ScummEngine_v4(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);

protected:
	virtual void readIndexFile();
	virtual void loadCharset(int no);
	virtual void loadRoomObjects();
	virtual void readMAXS(int blockSize);
	virtual void readGlobalObjects();

	virtual void setupRoomObject(ObjectData *od, const byte *room, const byte *searchptr = NULL);
};

/**
 * Engine for version 3 SCUMM games; GF_SMALL_NAMES is always set for these.
 */
class ScummEngine_v3 : public ScummEngine_v4 {
public:
	ScummEngine_v3(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);

protected:
	virtual void readRoomsOffsets();
	virtual void loadCharset(int no);
};

/**
 * Engine for old format version 3 SCUMM games; GF_OLD_BUNDLE is always set for these.
 */
class ScummEngine_v3old : public ScummEngine_v3 {
public:
	ScummEngine_v3old(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);

protected:
	virtual void readResTypeList(int id, uint32 tag, const char *name);
	virtual void readIndexFile();
	virtual void loadRoomSubBlocks();
	virtual void initRoomSubBlocks();
	virtual void loadRoomObjects();
};

/**
 * Engine for version 2 SCUMM games.
 */
class ScummEngine_v2 : public ScummEngine_v3old {
protected:
	typedef void (ScummEngine_v2::*OpcodeProcV2)();
	struct OpcodeEntryV2 {
		OpcodeProcV2 proc;
		const char *desc;
	};

	const OpcodeEntryV2 *_opcodesV2;

	V2MouseoverBox _mouseOverBoxesV2[7];
	int8 _mouseOverBoxV2;

public:
	ScummEngine_v2(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);

	virtual void scummInit();

	void checkV2MouseOver(Common::Point pos);
	void checkV2Inventory(int x, int y);
	void redrawV2Inventory();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void initScummVars();
	virtual void decodeParseString();

	virtual void readIndexFile();
	void readClassicIndexFile();	// V1
	void readEnhancedIndexFile();	// V2
	virtual void readGlobalObjects();
	virtual void loadCharset(int no);

	virtual void runInventoryScript(int i);

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

	virtual void handleMouseOver(bool updateInventory);
	void initV2MouseOver();
	void initNESMouseOver();

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

/**
 * Engine for Commodore 64 version of Maniac Mansion
 */
class ScummEngine_c64 : public ScummEngine_v2 {
protected:
	typedef void (ScummEngine_c64::*OpcodeProcC64)();
	struct OpcodeEntryC64 {
		OpcodeProcC64 proc;
		const char *desc;
	};

	const OpcodeEntryC64 *_opcodesC64;

public:
	ScummEngine_c64(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void ifStateCommon(byte type);
	virtual void ifNotStateCommon(byte type);
	virtual void setStateCommon(byte type);
	virtual void clearStateCommon(byte type);

	/* Version 2 script opcodes */
	void o_isGreaterEqual();
	void o_stopScript();
	void o_getDist();
	void o_isNotEqual();
	void o_loadSound();
	void o_putActorAtObject();
	void o_unfreezeScripts();
	void o_unknown13();
	void o_clearState08();
	void o_move();
	void o_loadActor();
	void o_freezeScript();
	void o_walkActorToObject();
	void o_isLessEqual();
	void o_subtract();
	void o_isLess();
	void o_increment();
	void o_isEqual();
	void o_loadRoom();
	void o_loadScript();
	void o_unknown4D();
	void o_unknown53();
	void o_add();
	void o_cursorCommand();
	void o_lights();
	void o_getObjectOwner();
	void o_isGreater();
	void o_pickupObject();
	void o_unknown93();
	void o_setState08();
	void o_unfreezeScript();
	void o_decrement();
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

	/** BlastObjects to draw */
	struct BlastObject {
		uint16 number;
		Common::Rect rect;
		uint16 scaleX, scaleY;
		uint16 image;
		uint16 mode;
	};

	int _blastObjectQueuePos; 
	BlastObject _blastObjectQueue[128];

	struct BlastText {
		int16 xpos, ypos;
		Common::Rect rect;
		byte color;
		byte charset;
		bool center;
		byte text[256];
	};

	int _blastTextQueuePos;
	BlastText _blastTextQueue[50];


public:
	ScummEngine_v6(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);

	virtual void scummInit();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void decodeParseString(int a, int b);
	virtual void readArrayFromIndexFile();

	virtual byte *getStringAddress(int i);
	virtual void readMAXS(int blockSize);

	virtual void palManipulateInit(int resID, int start, int end, int time);
	virtual void drawDirtyScreenParts();

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

	void enqueueText(const byte *text, int x, int y, byte color, byte charset, bool center);
	void drawBlastTexts();
	void removeBlastTexts();

	void enqueueObject(int objectNumber, int objectX, int objectY, int objectWidth,
	                   int objectHeight, int scaleX, int scaleY, int image, int mode);
	void drawBlastObjects();
	void drawBlastObject(BlastObject *eo);
	void removeBlastObjects();
	void removeBlastObject(BlastObject *eo);

	virtual void clearDrawQueues();


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

	Common::File _hFileTable[17];
	
public:
	ScummEngine_v60he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex) : ScummEngine_v6(detector, syst, gs, md5sum, substResFileNameIndex) {}

	virtual void scummInit();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	void localizeArray(int slot, byte scriptSlot);
	void redimArray(int arrayId, int newX, int newY, int d);
	int readFileToArray(int slot, int32 size);
	void writeFileFromArray(int slot, int resID);
	int virtScreenSave(byte *dst, int x1, int y1, int x2, int y2);
	void virtScreenLoad(int resIdx, int x1, int y1, int x2, int y2);
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

#ifndef DISABLE_HE
class ScummEngine_v70he : public ScummEngine_v60he {
	friend class ResExtractor;

protected:
	typedef void (ScummEngine_v70he::*OpcodeProcv70he)();
	struct OpcodeEntryv70he {
		OpcodeProcv70he proc;
		const char *desc;
	};
	
	const OpcodeEntryv70he *_opcodesv70he;

	ResExtractor *_resExtractor;

	byte *_heV7RoomOffsets;

	int32 _heSndSoundId, _heSndOffset, _heSndChannel, _heSndFlags, _heSndSoundFreq;

	bool _skipProcessActors;

public:
	ScummEngine_v70he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);
	~ScummEngine_v70he();

	Wiz _wiz;

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);
	
	virtual void setupScummVars();
	virtual void initScummVars();

	virtual void saveOrLoad(Serializer *s, uint32 savegameVersion);

	virtual void readRoomsOffsets();
	virtual void readGlobalObjects();
	virtual void readIndexBlock(uint32 blocktype, uint32 itemsize);

	int getStringCharWidth(byte chr);
	virtual int setupStringArray(int size);
	void appendSubstring(int dst, int src, int len2, int len);

	virtual void setCursorFromImg(uint img, uint room, uint imgindex);

	virtual void clearDrawQueues();

	/* HE version 70 script opcodes */
	void o70_startSound();
	void o70_pickupObject();
	void o70_getActorRoom();
	void o70_resourceRoutines();
	void o70_quitPauseRestart();
	void o70_kernelSetFunctions();
	void o70_seekFilePos();
	void o70_copyString();
	void o70_getStringWidth();
	void o70_getStringLen();
	void o70_appendString();
	void o70_concatString();
	void o70_compareString();
	void o70_readINI();
	void o70_writeINI();
	void o70_getStringLenForWidth();
	void o70_getCharIndexInString();
	void o70_setFilePath();
	void o70_setWindowCaption();
	void o70_polygonOps();
	void o70_polygonHit();

	byte VAR_NUM_SOUND_CHANNELS;
};

class ScummEngine_v71he : public ScummEngine_v70he {
public:
	ScummEngine_v71he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);

protected:
	virtual void redrawBGAreas();

	virtual void processActors();
	void preProcessAuxQueue();
	void postProcessAuxQueue();

public:
	/* Actor AuxQueue stuff (HE) */	
	AuxBlock _auxBlocks[16];
	uint16 _auxBlocksNum;
	AuxEntry _auxEntries[16];
	uint16 _auxEntriesNum;

	void queueAuxBlock(Actor *a);
	void queueAuxEntry(int actorNum, int subIndex);
};

class ScummEngine_v72he : public ScummEngine_v71he {
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

	int _stringLength;
	byte _stringBuffer[4096];

	WizParameters _wizParams;

public:
	ScummEngine_v72he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);

	virtual void scummInit();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void initScummVars();
	virtual void readArrayFromIndexFile();

	virtual byte *getStringAddress(int i);
	virtual void readMAXS(int blockSize);

	virtual void redrawBGAreas();

	ArrayHeader *defineArray(int array, int type, int dim2start, int dim2end, int dim1start, int dim1end);
	virtual int readArray(int array, int idx2, int idx1);
	virtual void writeArray(int array, int idx2, int idx1, int value);
	void redimArray(int arrayId, int newDim2start, int newDim2end, 
					int newDim1start, int newDim1end, int type);
	void checkArrayLimits(int array, int dim2start, int dim2end, int dim1start, int dim1end);
	void copyArray(int array1, int a1_dim2start, int a1_dim2end, int a1_dim1start, int a1_dim1end, 
					int array2, int a2_dim2start, int a2_dim2end, int a2_dim1start, int a2_dim1end);
	void copyArrayHelper(ArrayHeader *ah, int idx2, int idx1, int len1, byte **data, int *size, int *num);
	virtual int setupStringArray(int size);
	int readFileToArray(int slot, int32 size);
	void writeFileFromArray(int slot, int resID);

	void remapHEPalette(const uint8 *src, uint8 *dst);

	void displayWizImage(WizImage *pwi);
	void displayWizComplexImage(const WizParameters *params);
	void drawWizComplexPolygon(int resNum, int state, int po_x, int po_y, int xmapNum, int angle, int zoom, const Common::Rect *r, int flags, int dstResNum, int paletteNum);
	void captureWizImage(int resNum, const Common::Rect& r, bool frontBuffer, int compType);
	void getWizImageDim(int resNum, int state,  int32 &w, int32 &h);
	uint8 *drawWizImage(int resNum, int state, int x1, int y1, int zorder, int xmapNum, int field_390, const Common::Rect *clipBox, int flags, int dstResNum, int paletteNum);
	void drawWizPolygon(int resNum, int state, int id, int flags, int xmapNum, int dstResNum, int paletteNum);
	void flushWizBuffer();

	virtual void decodeParseString(int a, int b);
	void decodeScriptString(byte *dst, bool scriptString = false);
	void copyScriptString(byte *dst, int dstSize);
	int convertFilePath(byte *dst, bool setFilePath = 0);

	byte *heFindResourceData(uint32 tag, byte *ptr);
	byte *heFindResource(uint32 tag, byte *ptr);
	byte *findWrappedBlock(uint32 tag, byte *ptr, int state, bool flagError);
	int findObject(int x, int y, int num, int *args);
	int getSoundResourceSize(int id);

	/* HE version 72 script opcodes */
	void o72_pushDWord();
	void o72_getScriptString();
	void o72_isAnyOf();
	void o72_resetCutscene();
	void o72_findObjectWithClassOf();
	void o72_getObjectImageX();
	void o72_getObjectImageY();
	void o72_captureWizImage();
	void o72_getTimer();
	void o72_setTimer();
	void o72_getSoundElapsedTime();
	void o72_startScript();
	void o72_startObject();
	void o72_drawObject();
	void o72_printWizImage();
	void o72_getArrayDimSize();
	void o72_getNumFreeArrays();
	void o72_roomOps();
	void o72_actorOps();
	void o72_verbOps();
	void o72_findObject();
	void o72_arrayOps();
	void o72_talkActor();
	void o72_talkEgo();
	void o72_dimArray();
	void o72_dim2dimArray();
	void o72_traceStatus();
	void o72_debugInput();
	void o72_drawWizImage();
	void o72_kernelGetFunctions();
	void o72_jumpToScript();
	void o72_openFile();
	void o72_readFile();
	void o72_writeFile();
	void o72_findAllObjects();
	void o72_deleteFile();
	void o72_rename();
	void o72_getPixel();
	void o72_pickVarRandom();
	void o72_redimArray();
	void o72_isResourceLoaded();
	void o72_readINI();
	void o72_writeINI();
	void o72_getResourceSize();
	void o72_setFilePath();
	void o72_setWindowCaption();

	byte VAR_NUM_ROOMS;
	byte VAR_NUM_SCRIPTS;
	byte VAR_NUM_SOUNDS;
	byte VAR_NUM_COSTUMES;
	byte VAR_NUM_IMAGES;
	byte VAR_NUM_CHARSETS;
	byte VAR_NUM_SPRITE_GROUPS;
	byte VAR_NUM_SPRITES;
	byte VAR_NUM_PALETTES;
	byte VAR_NUM_UNK;
	byte VAR_POLYGONS_ONLY;
	byte VAR_PLATFORM;
	byte VAR_WINDOWS_VERSION;
	byte VAR_CURRENT_CHARSET;
	byte VAR_U32_VERSION;
	byte VAR_U32_ARRAY_UNK;
	byte VAR_WIZ_TCOLOR;
};

class ScummEngine_v80he : public ScummEngine_v72he {
protected:
	typedef void (ScummEngine_v80he::*OpcodeProcV80he)();
	struct OpcodeEntryV80he {
		OpcodeProcV80he proc;
		const char *desc;
	};
	
	const OpcodeEntryV80he *_opcodesV80he;

	int32 _heSBNGId;

public:
	ScummEngine_v80he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void initScummVars();

	virtual void initCharset(int charset);

	virtual void clearDrawQueues();

	void loadImgSpot(int resId, int state, int16 &x, int16 &y);
	void loadWizCursor(int resId);
	void drawLine(int x1, int y1, int x, int unk1, int unk2, int type, int id);
	void drawPixel(int x, int y, int flags);
	
	/* HE version 80 script opcodes */
	void o80_loadSBNG();
	void o80_getFileSize();
	void o80_stringToInt();
	void o80_getSoundVar();
	void o80_localizeArrayToRoom();
	void o80_readConfigFile();
	void o80_writeConfigFile();
	void o80_cursorCommand();
	void o80_setState();
	void o80_drawBox();
	void o80_drawWizPolygon();
	void o80_drawLine();
	void o80_pickVarRandom();
	void o80_getResourceSize();
};

struct SpriteInfo;
struct SpriteGroup;

class ScummEngine_v90he : public ScummEngine_v80he {
	friend class LogicHE;

protected:
	typedef void (ScummEngine_v90he::*OpcodeProcV90he)();
	struct OpcodeEntryV90he {
		OpcodeProcV90he proc;
		const char *desc;
	};

	const OpcodeEntryV90he *_opcodesV90he;

	struct FloodStateParameters {
		Common::Rect box;
		int32 field_10;
		int32 field_14;
		int32 field_18;
		int32 field_1C;
	};

	FloodStateParameters _floodStateParams;

	int32 _heObject, _heObjectNum;
	int32 _hePaletteNum;

	int32 _curMaxSpriteId;
	int32 _curSpriteId;
	int32 _curSpriteGroupId;
	int32 _numSpritesToProcess;
	int32 _varNumSpriteGroups;
	int32 _varNumSprites;
	int32 _varMaxSprites;
	SpriteInfo *_spriteTable;
	SpriteGroup *_spriteGroups;
	SpriteInfo **_activeSpritesTable;

public:
	ScummEngine_v90he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex) : ScummEngine_v80he(detector, syst, gs, md5sum, substResFileNameIndex) {}

	~ScummEngine_v90he();

	virtual void scummInit();

	LogicHE *_logicHE;

protected:
	virtual void allocateArrays();
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void initScummVars();

	virtual void saveOrLoad(Serializer *s, uint32 savegameVersion);

	virtual void readMAXS(int blockSize);

	virtual void processActors();

	void createWizEmptyImage(const WizParameters *params);
	void fillWizRect(const WizParameters *params);
	void fillWizParallelogram(const WizParameters *params);
	void processWizImage(const WizParameters *params);
	int getWizImageStates(int resnum);	
	int isWizPixelNonTransparent(int resnum, int state, int x, int y, int flags);
	uint8 getWizPixelColor(int resnum, int state, int x, int y, int flags);
	int computeWizHistogram(int resnum, int state, int x, int y, int w, int h);

	void getArrayDim(int array, int *dim2start, int *dim2end, int *dim1start, int *dim1end);
	void sortArray(int array, int dim2start, int dim2end, int dim1start, int dim1end, int sortOrder);
	
	uint8 *getHEPaletteIndex(int palSlot);
	int getHEPaletteColor(int palSlot, int color);
	int getHEPaletteSimilarColor(int palSlot, int red, int green, int start, int end);
	int getHEPaletteColorComponent(int palSlot, int color, int component);
	void setHEPaletteColor(int palSlot, uint8 color, uint8 r, uint8 g, uint8 b);
	void setHEPaletteFromPtr(int palSlot, const uint8 *palData);
	void setHEPaletteFromCostume(int palSlot, int resId);
	void setHEPaletteFromImage(int palSlot, int resId, int state);
	void setHEPaletteFromRoom(int palSlot, int resId, int state);
	void restoreHEPalette(int palSlot);
	void copyHEPalette(int dstPalSlot, int srcPalSlot);
	void copyHEPaletteColor(int palSlot, uint8 dstColor, uint8 srcColor);

	void getSpriteBounds(int spriteId, bool checkGroup, Common::Rect &bound);
	int findSpriteWithClassOf(int x, int y, int spriteGroupId, int d, int num, int *args);
	int spriteInfoGet_classFlags(int spriteId, int num, int *args);
	int spriteInfoGet_flagDoubleBuffered(int spriteId);
	int spriteInfoGet_flagYFlipped(int spriteId);
	int spriteInfoGet_flagXFlipped(int spriteId);
	int spriteInfoGet_flagActive(int spriteId);
	int spriteInfoGet_flagNeedPaletteRemap(int spriteId);
	int spriteInfoGet_flagDelayed(int spriteId);
	int spriteInfoGet_flagMarkDirty(int spriteId);	
	int spriteInfoGet_flagHasImage(int spriteId);
	int spriteInfoGet_resId(int spriteId);
	int spriteInfoGet_resState(int spriteId);
	int spriteInfoGet_groupNum(int spriteId);
	int spriteInfoGet_paletteNum(int spriteId);
	int spriteInfoGet_zorderPriority(int spriteId);
	int spriteInfoGet_grp_tx(int spriteId);
	int spriteInfoGet_grp_ty(int spriteId);
	int spriteInfoGet_field_44(int spriteId);
	int spriteInfoGet_xmapNum(int spriteId);
	int spriteInfoGet_wizSize(int spriteId);
	int spriteInfoGet_zoom(int spriteId);
	int spriteInfoGet_delayAmount(int spriteId);
	int spriteInfoGet_maskImgResNum(int spriteId);
	int spriteInfoGet_field_80(int spriteId);
	int spriteInfoGet_field_8C_90(int spriteId, int type);
	void getSpriteImageDim(int spriteId, int32 &w, int32 &h);
	void spriteInfoGet_tx_ty(int spriteId, int32 &tx, int32 &ty);
	void spriteInfoGet_dx_dy(int spriteId, int32 &dx, int32 &dy);

	int spriteGroupGet_allocateGroupSpritesList(int spriteGroupId);
	int spriteGroupGet_zorderPriority(int spriteGroupId);
	int spriteGroupGet_dstResNum(int spriteGroupId);
	int spriteGroupGet_scale_x_ratio_mul(int spriteGroupId);
	int spriteGroupGet_scale_x_ratio_div(int spriteGroupId);
	int spriteGroupGet_scale_y_ratio_mul(int spriteGroupId);
	int spriteGroupGet_scale_y_ratio_div(int spriteGroupId);
	void spriteGroupGet_tx_ty(int spriteGroupId, int32 &tx, int32 &ty);

	void spriteInfoSet_paletteNum(int spriteId, int value);
	void spriteInfoSet_maskImgResNum(int spriteId, int value);
	void spriteInfoSet_field_80(int spriteId, int value);
	void spriteInfoSet_resetSprite(int spriteId);
	void spriteInfoSet_resState(int spriteId, int value);
	void spriteInfoSet_tx_ty(int spriteId, int value1, int value2);
	void spriteInfoSet_groupNum(int spriteId, int value);
	void spriteInfoSet_dx_dy(int spriteId, int value1, int value2);
	void spriteInfoSet_xmapNum(int spriteId, int value);
	void spriteInfoSet_field_44(int spriteId, int value1, int value2);
	void spriteInfoSet_zorderPriority(int spriteId, int value);
	void spriteInfoSet_Inc_tx_ty(int spriteId, int value1, int value2);
	void spriteInfoSet_zoom(int spriteId, int value);
	void spriteInfoSet_angle(int spriteId, int value);
	void spriteInfoSet_flagDoubleBuffered(int spriteId, int value);
	void spriteInfoSet_flagYFlipped(int spriteId, int value);
	void spriteInfoSet_flagXFlipped(int spriteId, int value);
	void spriteInfoSet_flagActive(int spriteId, int value);
	void spriteInfoSet_flagNeedPaletteRemap(int spriteId, int value);
	void spriteInfoSet_flagDelayed(int spriteId, int value);
	void spriteInfoSet_flagMarkDirty(int spriteId, int value);
	void spriteInfoSet_flagHasImage(int spriteId, int value);
	void spriteInfoSet_delay(int spriteId, int value);
	void spriteInfoSet_setClassFlag(int spriteId, int classId, int toggle);
	void spriteInfoSet_resetClassFlags(int spriteId);
	void spriteInfoSet_field_84(int spriteId, int value);
	void spriteInfoSet_field_8C_90(int spriteId, int type, int value);

	void redrawSpriteGroup(int spriteGroupId);
	void spriteGroupSet_case0_0(int spriteGroupId, int value1, int value2);
	void spriteGroupSet_case0_1(int spriteGroupId, int value);
	void spriteGroupSet_case0_2(int spriteGroupId, int value);
	void spriteGroupSet_case0_3(int spriteGroupId, int value);
	void spriteGroupSet_case0_4(int spriteGroupId);
	void spriteGroupSet_case0_5(int spriteGroupId, int value);
	void spriteGroupSet_case0_6(int spriteGroupId, int value);
	void spriteGroupSet_case0_7(int spriteGroupId, int value);
	void spriteGroupSet_bbox(int spriteGroupId, int x1, int y1, int x2, int y2);
	void spriteGroupSet_zorderPriority(int spriteGroupId, int value);
	void spriteGroupSet_tx_ty(int spriteGroupId, int value1, int value2);
	void spriteGroupSet_inc_tx_ty(int spriteGroupId, int value1, int value2);
	void spriteGroupSet_dstResNum(int spriteGroupId, int value);
	void spriteGroupSet_scaling(int spriteGroupId);
	void spriteGroupSet_scale_x_ratio_mul(int spriteGroupId, int value);
	void spriteGroupSet_scale_x_ratio_div(int spriteGroupId, int value);
	void spriteGroupSet_scale_y_ratio_mul(int spriteGroupId, int value);
	void spriteGroupSet_scale_y_ratio_div(int spriteGroupId, int value);
	void spriteGroupSet_flagClipBoxAnd(int spriteGroupId);

	void spritesAllocTables(int numSprites, int numGroups, int numMaxSprites);
	void spritesResetGroup(int spriteGroupId);
	void spritesResetTables(bool refreshScreen);
	void spriteAddImageToList(int spriteId, int imageNum, int *spriteIdptr);

	void setDefaultCursor();

public:
	void saveOrLoadSpriteData(Serializer *s, uint32 savegameVersion);
	void spritesBlitToScreen();
	void spritesMarkDirty(bool checkZOrder);
	void spritesSortActiveSprites();
	void spritesProcessWiz(bool arg);
	void spritesUpdateImages();

protected:
	/* HE version 90 script opcodes */
	void o90_dup_n();
	void o90_min();
	void o90_max();
	void o90_sin();
	void o90_cos();
	void o90_sqrt();
	void o90_atan2();
	void o90_getSegmentAngle();
	void o90_getActorData();
	void o90_startScriptUnk();
	void o90_jumpToScriptUnk();
	void o90_smackerOps();
	void o90_getSmackerData();
	void o90_wizImageOps();
	void o90_getDistanceBetweenPoints();
	void o90_getSpriteInfo();
	void o90_setSpriteInfo();
	void o90_getSpriteGroupInfo();
	void o90_setSpriteGroupInfo();
	void o90_getWizData();
	void o90_floodState();
	void o90_mod();
	void o90_shl();
	void o90_shr();
	void o90_xor();
	void o90_findAllObjectsWithClassOf();
	void o90_getPolygonOverlap();
	void o90_cond();
	void o90_dim2dim2Array();
	void o90_redim2dimArray();
	void o90_getLinesIntersectionPoint();
	void o90_sortArray();
	void o90_getObjectData();
	void o90_getPaletteData();
	void o90_paletteOps();
	void o90_fontUnk();
	void o90_getActorAnimProgress();
	void o90_kernelGetFunctions();
	void o90_kernelSetFunctions();
};

class ScummEngine_v99he : public ScummEngine_v90he {
public:
	ScummEngine_v99he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex) : ScummEngine_v90he(detector, syst, gs, md5sum, substResFileNameIndex) {}

	virtual void scummInit();

protected:
	virtual void initScummVars();

	virtual void readMAXS(int blockSize);

	virtual void copyPalColor(int dst, int src);
	virtual void darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor);
	virtual void setPaletteFromPtr(const byte *ptr, int numcolor = -1);
	virtual void setPalColor(int index, int r, int g, int b);
	virtual void updatePalette();
};

class ScummEngine_v100he : public ScummEngine_v99he {
protected:
	typedef void (ScummEngine_v100he::*OpcodeProcV100he)();
	struct OpcodeEntryV100he {
		OpcodeProcV100he proc;
		const char *desc;
	};

	int32 _heResId, _heResType;

	const OpcodeEntryV100he *_opcodesV100he;

public:
	ScummEngine_v100he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex) : ScummEngine_v99he(detector, syst, gs, md5sum, substResFileNameIndex) {}

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);
	
	virtual void saveOrLoad(Serializer *s, uint32 savegameVersion);

	virtual void decodeParseString(int a, int b);

	/* HE version 100 script opcodes */
	void o100_actorOps();
	void o100_arrayOps();
	void o100_dim2dimArray();
	void o100_redim2dimArray();
	void o100_dimArray();
	void o100_drawLine();
	void o100_drawObject();
	void o100_setSpriteGroupInfo();
	void o100_resourceRoutines();
	void o100_wizImageOps();
	void o100_jumpToScript();
	void o100_loadSBNG();
	void o100_dim2dim2Array();
	void o100_paletteOps();
	void o100_jumpToScriptUnk();
	void o100_startScriptUnk();
	void o100_redimArray();
	void o100_roomOps();
	void o100_startSound();
	void o100_setSpriteInfo();
	void o100_startScript();
	void o100_quitPauseRestart();
	void o100_cursorCommand();
	void o100_wait();
	void o100_writeFile();
	void o100_isResourceLoaded();
	void o100_getResourceSize();
	void o100_getSpriteGroupInfo();
	void o100_getPaletteData();
	void o100_readFile();
	void o100_getSpriteInfo();
	void o100_getWizData();
};
#endif

#ifndef DISABLE_SCUMM_7_8
class ScummEngine_v7 : public ScummEngine_v6 {
public:
	ScummEngine_v7(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);
	~ScummEngine_v7();

	bool _existLanguageFile;
	char *_languageBuffer;
	LangIndexNode *_languageIndex;
	int _languageIndexSize;
	char _lastStringTag[12+1];

protected:
	virtual void setupScummVars();
	virtual void initScummVars();

	virtual void akos_processQueue();

	virtual void saveOrLoad(Serializer *s, uint32 savegameVersion);

	virtual void readMAXS(int blockSize);
	virtual void readGlobalObjects();
	virtual void readIndexBlock(uint32 blocktype, uint32 itemsize);

	virtual void setCameraAt(int pos_x, int pos_y);
	virtual void setCameraFollows(Actor *a);
	virtual void moveCamera();
	virtual void panCameraTo(int x, int y);

	virtual int getObjectIdFromOBIM(const byte *obim);

	virtual void actorTalk(const byte *msg);
	virtual void translateText(const byte *text, byte *trans_buff);
	virtual void loadLanguageBundle();
	void playSpeech(const byte *ptr);
};

class ScummEngine_v8 : public ScummEngine_v7 {
protected:
	typedef void (ScummEngine_v8::*OpcodeProcV8)();
	struct OpcodeEntryV8 {
		OpcodeProcV8 proc;
		const char *desc;
	};
	
	const OpcodeEntryV8 *_opcodesV8;

	struct ObjectNameId {
		char name[40];
		int id;
	};
	int _objectIDMapSize;
	ObjectNameId *_objectIDMap;

public:
	ScummEngine_v8(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);
	~ScummEngine_v8();

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void decodeParseString(int m, int n);
	virtual void readArrayFromIndexFile();

	virtual void readMAXS(int blockSize);
	virtual void readGlobalObjects();

	virtual uint fetchScriptWord();
	virtual int fetchScriptWordSigned();
	
	virtual int readVar(uint var);
	virtual void writeVar(uint var, int value);

	virtual int getObjectIdFromOBIM(const byte *obim);


	void desaturatePalette(int hueScale, int satScale, int lightScale, int startColor, int endColor);


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

#endif

} // End of namespace Scumm

#endif
