/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef SCUMM_INTERN_H
#define SCUMM_INTERN_H

#include "scumm/scumm.h"

namespace Scumm {

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

	uint16 _cursorImages[4][17];
	byte _cursorHotspots[2 * 4];

public:
	ScummEngine_v5(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], SubstResFileNames subst);

protected:
	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void initScummVars();
	virtual void decodeParseString();

	virtual void saveOrLoad(Serializer *s);

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
	void o5_systemOps();
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
	ScummEngine_v4(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], SubstResFileNames subst);

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
	ScummEngine_v3(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], SubstResFileNames subst);

protected:
	virtual void readRoomsOffsets();
	virtual void loadCharset(int no);
};

/**
 * Engine for old format version 3 SCUMM games; GF_OLD_BUNDLE is always set for these.
 */
class ScummEngine_v3old : public ScummEngine_v3 {
public:
	ScummEngine_v3old(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], SubstResFileNames subst);

protected:
	virtual void readResTypeList(int id, const char *name);
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

	struct V2MouseoverBox {
		Common::Rect rect;
		byte color;
		byte hicolor;
	};

	V2MouseoverBox _mouseOverBoxesV2[7];
	int8 _mouseOverBoxV2;

public:
	ScummEngine_v2(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], SubstResFileNames subst);

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

	int _currentAction;
	int _currentMode;
public:
	ScummEngine_c64(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], SubstResFileNames subst);

	virtual void scummInit();

protected:
	virtual void setupRoomObject(ObjectData *od, const byte *room, const byte *searchptr = NULL);

	virtual void setupOpcodes();
	virtual void executeOpcode(byte i);
	virtual const char *getOpcodeDesc(byte i);

	virtual void setupScummVars();
	virtual void decodeParseString();

	void initC64Verbs();
	virtual void checkExecVerbs();

	virtual int getVarOrDirectWord(byte mask);
	virtual uint fetchScriptWord();

	virtual void ifStateCommon(byte type);
	virtual void ifNotStateCommon(byte type);
	virtual void setStateCommon(byte type);
	virtual void clearStateCommon(byte type);

	int getObjectFlag();

	/* Version C64 script opcodes */
	void o_setState08();
	void o_clearState08();
	void o_stopCurrentScript();
	void o_loadSound();
	void o_getActorMoving();
	void o_animateActor();
	void o_putActorAtObject();
	void o_lockSound();
	void o_lockActor();
	void o_loadActor();
	void o_loadRoom();
	void o_loadRoomWithEgo();
	void o_lockScript();
	void o_loadScript();
	void o_lockRoom();
	void o_cursorCommand();
	void o_lights();
	void o_pickupObject();
	void o_unlockActor();
	void o_unlockScript();
	void o_decrement();
	void o_badOpcode();
	void o_nop();
	void o_getActorBitVar();
	void o_setActorBitVar();
	void o_doSentence();
	void o_unknown2();
	void o_unknown3();
	void o_getClosestObjActor();
	void o_printEgo_c64();
	void o_print_c64();
	void o_unlockRoom();
	void o_unlockSound();
	void o_beginOverride();
	void o_isEqual();
	void o_isGreater();
	void o_isGreaterEqual();
	void o_isLess();
	void o_isLessEqual();
	void o_isNotEqual();
	void o_notEqualZero();
	void o_equalZero();
	void o_jumpRelative();
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

	struct TextObject {
		int16 xpos, ypos;
		byte color;
		byte charset;
		byte text[256];		
	};

	/** BlastObjects to draw */
	struct BlastObject {
		uint16 number;
		Common::Rect rect;
		uint16 scaleX, scaleY;
		uint16 image;
		uint16 mode;
	};

	int _blastObjectQueuePos;
	BlastObject _blastObjectQueue[200];

	struct BlastText : TextObject {
		Common::Rect rect;
		bool center;
	};

	int _blastTextQueuePos;
	BlastText _blastTextQueue[50];


public:
	ScummEngine_v6(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], SubstResFileNames subst);

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

	void setDefaultCursor();
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
	void o6_systemOps();
	void o6_isActorInBox();
	void o6_delay();
	void o6_delaySeconds();
	void o6_delayMinutes();
	void o6_stopSentence();
	void o6_printLine();
	void o6_printText();
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

#ifndef DISABLE_SCUMM_7_8
class ScummEngine_v7 : public ScummEngine_v6 {
public:
	ScummEngine_v7(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], SubstResFileNames subst);
	~ScummEngine_v7();

	struct LangIndexNode {
		char tag[12+1];
		int32 offset;
	};
	
	bool _existLanguageFile;
	char *_languageBuffer;
	LangIndexNode *_languageIndex;
	int _languageIndexSize;
	char _lastStringTag[12+1];

#if defined(__SYMBIAN32__) || defined (_WIN32_WCE) // for some reason VC6 cannot find the base class TextObject
	struct SubtitleText {
		int16 xpos, ypos;
		byte color;
		byte charset;
		byte text[256];		
		bool actorSpeechMsg;
	};
#else
	struct SubtitleText : TextObject {
		bool actorSpeechMsg;
	};
#endif

	int _subtitleQueuePos;
	SubtitleText _subtitleQueue[20];

	void processSubtitleQueue();
	void addSubtitleToQueue(const byte *text, const Common::Point &pos, byte color, byte charset);
	void clearSubtitleQueue();

protected:
	virtual void setupScummVars();
	virtual void initScummVars();

	virtual void akos_processQueue();

	virtual void saveOrLoad(Serializer *s);

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

	virtual void drawVerb(int verb, int mode);
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
	ScummEngine_v8(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], SubstResFileNames subst);
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

	void o8_systemOps();
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
