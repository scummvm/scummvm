/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Scripting module private header

#ifndef SAGA_SCRIPT_H
#define SAGA_SCRIPT_H

#include "saga/text.h"
#include "saga/list.h"

namespace Saga {

#define SCRIPT_DATABUF_NUM 5
#define SCRIPT_DATABUF_LEN 1024

#define S_LUT_ENTRYLEN_ITECD 22
#define S_LUT_ENTRYLEN_ITEDISK 16

#define SCRIPT_TBLENTRY_LEN 4

#define SCRIPT_MAX 5000
#define SCRIPTLIST_HDR 12
#define SCRIPT_STRINGLIMIT 255
#define TAB "    "

#define S_ERROR_PREFIX "SError: "
#define S_WARN_PREFIX "SWarning: "

#define SCRIPT_FUNCTION_MAX 78

typedef unsigned int ScriptDataWord;

enum VerbTypes {
//todo: LUT for drawing
	kVerbNone = 0,
	kVerbPickup = 1,
	kVerbLookAt = 2,
	kVerbWalkTo = 3,
	kVerbSpeakTo = 4,
	kVerbOpen = 5,
	kVerbClose = 6,
	kVerbGive = 7,
	kVerbUse = 8,
	kVerbOptions = 9,
	kVerbEnter = 10,
	kVerbLeave = 11,
	kVerbBegin = 12,
	kVerbWalkOnly = 13,
	kVerbLookOnly = 14
};

#define STHREAD_TIMESLICE 8

enum {
	kVarObject = 0,
	kVarWithObject,
	kVarAction,
	kVarActor
};

enum ThreadFlags {
	kTFlagNone = 0,
	kTFlagWaiting = 1,	// wait for even denoted in waitType
	kTFlagFinished = 2,
	kTFlagAborted = 4,
	kTFlagAsleep = kTFlagWaiting | kTFlagFinished | kTFlagAborted	// Combination of all flags which can halt a thread
};

enum ThreadWaitTypes {
	kWaitTypeNone = 0,			// waiting for nothing
	kWaitTypeDelay = 1,			// waiting for a timer
	kWaitTypeSpeech = 2,		// waiting for speech to finish
	kWaitTypeDialogEnd = 3,		// waiting for my dialog to finish
	kWaitTypeDialogBegin = 4,	// waiting for other dialog to finish
	kWaitTypeWalk = 5,			// waiting to finish walking
	kWaitTypeRequest = 6,		// a request is up
	kWaitTypePause = 7,
	kWaitTypePlacard = 8
};

enum OpCodes {

//...
	opCcall = 0x18,
	opCcallV = 0x19,
	opEnter = 0x1A,
	opReturn = 0x1B,
	opReturnV = 0x1C,
//...
	opSpeak = 0x53
};

enum CycleFlags {
	kCyclePong = (1 << 0),
	kCycleOnce = (1 << 1),
	kCycleRandom = (1 << 2),
	kCycleReverse = (1 << 3)
};

enum WalkFlags {
	kWalkBackPedal = (1<<0),
	kWalkAsync = (1<<1),
	kWalkUseAngle = (1<<2),
	kWalkFace = (1<<5)
};

struct SCRIPT_THREAD {
	int flags;				// ThreadFlags
	int waitType;			// ThreadWaitTypes
	void *threadObj;		// which object we're handling

	uint sleepTime;
	int ep_num; // Entrypoint number
	unsigned long ep_offset; // Entrypoint offset
	unsigned long i_offset; // Instruction offset

	// The scripts are allowed to access the stack like any other memory
	// area. It's therefore probably quite important that our stacks work
	// the same as in the original interpreter.

	ScriptDataWord stackBuf[64];

	int stackPtr;
	int framePtr;

	ScriptDataWord threadVars[4];

	ScriptDataWord retVal;

	ScriptDataWord stackTop() {
		return stackBuf[stackPtr];
	}

	int stackSize() {
		return ARRAYSIZE(stackBuf) - stackPtr - 1;
	}

	void push(ScriptDataWord n) {
		assert(stackPtr > 0);
		stackBuf[--stackPtr] = n;
	}

	ScriptDataWord pop() {
		assert(stackPtr < ARRAYSIZE(stackBuf));
		return stackBuf[stackPtr++];
	}
	
	void wait(int aWaitType) {
		waitType = aWaitType;
		flags |= kTFlagWaiting;
	}

	void waitWalk(void *aThreadObj) {
		wait(kWaitTypeWalk);
		threadObj = aThreadObj;
	}

	void waitDelay(int aSleepTime) {
		wait(kWaitTypeDelay);
		sleepTime = aSleepTime;
	}

	SCRIPT_THREAD() { memset(this, 0, sizeof(*this)); }
};

typedef SortedList<SCRIPT_THREAD> ScriptThreadList;

struct PROC_TBLENTRY {
	size_t name_offset;
	size_t offset;
};

struct SCRIPT_BYTECODE {
	unsigned char *bytecode_p;
	size_t bytecode_len;
	size_t ep_tbl_offset;
	unsigned long n_entrypoints;
	PROC_TBLENTRY *entrypoints;
};

struct VOICE_LUT {
	int n_voices;
	int *voices;
};

struct ScriptData {
	int loaded;
	SCRIPT_BYTECODE *bytecode;
	StringsTable strings;
	VOICE_LUT *voice;
};

struct SCRIPT_LUT_ENTRY {
	int script_rn;
	int diag_list_rn;
	int voice_lut_rn;
};

struct ScriptDataBuf {
	ScriptDataWord *data;
	int length;
};

#define SCRIPTFUNC_PARAMS SCRIPT_THREAD *thread, int nArgs

class Script {
public:
	Script();
	~Script();
	
	void CF_script_togglestep();

	int loadScript(int scriptNum);
	int freeScript();
	SCRIPT_BYTECODE *loadBytecode(byte *bytecode_p, size_t bytecode_len);
	VOICE_LUT *loadVoiceLUT(const byte *voicelut_p, size_t voicelut_len, ScriptData *script);

	bool isInitialized() const { return _initialized;  }
	bool isVoiceLUTPresent() const { return _voiceLUTPresent; }
	ScriptData *currentScript() { return _currentScript; }
	ScriptDataBuf *dataBuffer(int idx) { return &_dataBuf[idx]; }
	int getWord(int bufNumber, int wordNumber, ScriptDataWord *data);
	int putWord(int bufNumber, int wordNumber, ScriptDataWord data);
	int setBit(int bufNumber, ScriptDataWord bitNumber, int bitState);
	int getBit(int bufNumber, ScriptDataWord bitNumber, int *bitState);	
	const char * getScriptString(int index) const { return _currentScript->strings.getString(index); }

	void doVerb();
	void showVerb();
	void setVerb(int verb);
	void setLeftButtonVerb(int verb);

	void scriptInfo();
	void scriptExec(int argc, const char **argv);
	
protected:
	bool _initialized;
	bool _voiceLUTPresent;
	RSCFILE_CONTEXT *_scriptContext;
	SCRIPT_LUT_ENTRY *_scriptLUT;
	int _scriptLUTMax;
	uint16 _scriptLUTEntryLen;
	ScriptData *_currentScript;
	ScriptDataBuf _dataBuf[SCRIPT_DATABUF_NUM];
	ScriptThreadList _threadList;
	StringsTable _mainStrings;

//verb	
	bool _firstObjectSet;
	bool _secondObjectNeeded;
	uint16 _currentObject[2];
	uint16 _pendingObject[2];
	int _currentVerb;
	int _stickyVerb;
	int _leftButtonVerb;
	int _rightButtonVerb;
	int _pendingVerb;
	

public:
	bool _skipSpeeches;
	bool _abortEnabled;

	int _dbg_singlestep;
	int _dbg_dostep;
	SCRIPT_THREAD *_dbg_thread;
	TEXTLIST_ENTRY *_dbg_txtentry;

public:
	SCRIPT_THREAD *SThreadCreate();
	int SThreadExecute(SCRIPT_THREAD *thread, int ep_num);
	int executeThreads(uint msec);
	int SThreadDebugStep();
	void SThreadCompleteThread(void);

	void wakeUpActorThread(int waitType, void *threadObj);
	void wakeUpThreads(int waitType);
	void wakeUpThreadsDelayed(int waitType, int sleepTime);

private:
	void setFramePtr(SCRIPT_THREAD *thread, int newPtr);
	unsigned char *SThreadGetReadPtr(SCRIPT_THREAD *thread);
	unsigned long SThreadGetReadOffset(const byte *read_p);
	size_t SThreadGetReadLen(SCRIPT_THREAD *thread);
	void runThread(SCRIPT_THREAD *thread, int instr_limit);
	int SThreadSetEntrypoint(SCRIPT_THREAD *thread, int ep_num);

private:
	typedef int (Script::*ScriptFunctionType)(SCRIPTFUNC_PARAMS);

	struct ScriptFunctionDescription {
		ScriptFunctionType scriptFunction;
		const char *scriptFunctionName;
	};
	const ScriptFunctionDescription *_scriptFunctionsList;

	void setupScriptFuncList(void);
	void scriptError(SCRIPT_THREAD *thread, const char *format, ...);
	int SDebugPrintInstr(SCRIPT_THREAD *thread);

	int SF_putString(SCRIPTFUNC_PARAMS);
	int sfWait(SCRIPTFUNC_PARAMS);
	int SF_takeObject(SCRIPTFUNC_PARAMS);
	int SF_objectIsCarried(SCRIPTFUNC_PARAMS);
	int sfStatusBar(SCRIPTFUNC_PARAMS);
	int SF_mainMode(SCRIPTFUNC_PARAMS);
	int sfScriptWalkTo(SCRIPTFUNC_PARAMS);
	int SF_doAction(SCRIPTFUNC_PARAMS);
	int sfSetActorFacing(SCRIPTFUNC_PARAMS);
	int sfStartBgdAnim(SCRIPTFUNC_PARAMS);
	int sfStopBgdAnim(SCRIPTFUNC_PARAMS);
	int sfLockUser(SCRIPTFUNC_PARAMS);
	int SF_preDialog(SCRIPTFUNC_PARAMS);
	int SF_killActorThreads(SCRIPTFUNC_PARAMS);
	int sfFaceTowards(SCRIPTFUNC_PARAMS);
	int sfSetFollower(SCRIPTFUNC_PARAMS);
	int SF_gotoScene(SCRIPTFUNC_PARAMS);
	int SF_setObjImage(SCRIPTFUNC_PARAMS);
	int SF_setObjName(SCRIPTFUNC_PARAMS);
	int SF_getObjImage(SCRIPTFUNC_PARAMS);
	int SF_getNumber(SCRIPTFUNC_PARAMS);
	int sfScriptOpenDoor(SCRIPTFUNC_PARAMS);
	int sfScriptCloseDoor(SCRIPTFUNC_PARAMS);
	int sfSetBgdAnimSpeed(SCRIPTFUNC_PARAMS);
	int SF_cycleColors(SCRIPTFUNC_PARAMS);
	int sfDoCenterActor(SCRIPTFUNC_PARAMS);
	int sfStartBgdAnimSpeed(SCRIPTFUNC_PARAMS);
	int sfScriptWalkToAsync(SCRIPTFUNC_PARAMS);
	int SF_enableZone(SCRIPTFUNC_PARAMS);
	int sfSetActorState(SCRIPTFUNC_PARAMS);
	int scriptMoveTo(SCRIPTFUNC_PARAMS);
	int SF_sceneEq(SCRIPTFUNC_PARAMS);
	int SF_dropObject(SCRIPTFUNC_PARAMS);
	int sfFinishBgdAnim(SCRIPTFUNC_PARAMS);
	int sfSwapActors(SCRIPTFUNC_PARAMS);
	int sfSimulSpeech(SCRIPTFUNC_PARAMS);
	int sfScriptWalk(SCRIPTFUNC_PARAMS);
	int sfCycleFrames(SCRIPTFUNC_PARAMS);
	int sfSetFrame(SCRIPTFUNC_PARAMS);
	int sfSetPortrait(SCRIPTFUNC_PARAMS);
	int sfSetProtagPortrait(SCRIPTFUNC_PARAMS);
	int sfChainBgdAnim(SCRIPTFUNC_PARAMS);
	int SF_scriptSpecialWalk(SCRIPTFUNC_PARAMS);
	int sfPlaceActor(SCRIPTFUNC_PARAMS);
	int SF_checkUserInterrupt(SCRIPTFUNC_PARAMS);
	int SF_walkRelative(SCRIPTFUNC_PARAMS);
	int SF_moveRelative(SCRIPTFUNC_PARAMS);
	int SF_simulSpeech2(SCRIPTFUNC_PARAMS);
	int sfPlacard(SCRIPTFUNC_PARAMS);
	int sfPlacardOff(SCRIPTFUNC_PARAMS);
	int SF_setProtagState(SCRIPTFUNC_PARAMS);
	int sfResumeBgdAnim(SCRIPTFUNC_PARAMS);
	int SF_throwActor(SCRIPTFUNC_PARAMS);
	int SF_waitWalk(SCRIPTFUNC_PARAMS);
	int SF_sceneID(SCRIPTFUNC_PARAMS);
	int SF_changeActorScene(SCRIPTFUNC_PARAMS);
	int SF_climb(SCRIPTFUNC_PARAMS);
	int sfSetDoorState(SCRIPTFUNC_PARAMS);
	int SF_setActorZ(SCRIPTFUNC_PARAMS);
	int SF_text(SCRIPTFUNC_PARAMS);
	int SF_getActorX(SCRIPTFUNC_PARAMS);
	int SF_getActorY(SCRIPTFUNC_PARAMS);
	int SF_eraseDelta(SCRIPTFUNC_PARAMS);
	int sfPlayMusic(SCRIPTFUNC_PARAMS);
	int SF_pickClimbOutPos(SCRIPTFUNC_PARAMS);
	int SF_tossRif(SCRIPTFUNC_PARAMS);
	int SF_showControls(SCRIPTFUNC_PARAMS);
	int SF_showMap(SCRIPTFUNC_PARAMS);
	int SF_puzzleWon(SCRIPTFUNC_PARAMS);
	int sfEnableEscape(SCRIPTFUNC_PARAMS);
	int sfPlaySound(SCRIPTFUNC_PARAMS);
	int SF_playLoopedSound(SCRIPTFUNC_PARAMS);
	int SF_getDeltaFrame(SCRIPTFUNC_PARAMS);
	int SF_showProtect(SCRIPTFUNC_PARAMS);
	int SF_protectResult(SCRIPTFUNC_PARAMS);
	int sfRand(SCRIPTFUNC_PARAMS);
	int SF_fadeMusic(SCRIPTFUNC_PARAMS);
	int SF_playVoice(SCRIPTFUNC_PARAMS);
};

inline int getSWord(ScriptDataWord word) {
	uint16 uInt = word;
	int sInt;

	if (uInt & 0x8000U) {
		sInt = (int)(uInt - 0x8000U) - 0x7FFF - 1;
	} else {
		sInt = uInt;
	}

	return sInt;
}

inline uint getUWord(ScriptDataWord word) {
	return (uint16) word;
}


} // End of namespace Saga

#endif
