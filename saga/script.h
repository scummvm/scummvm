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

#define COMMON_BUFFER_SIZE 1024

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
#define DEFAULT_THREAD_STACK_SIZE 256

enum AddressTypes {
	kAddressCommon     = 0,	// offset from global variables
	kAddressStatic     = 1,	// offset from global variables
	kAddressModule     = 2,	// offset from start of module
	kAddressStack      = 3,	// offset from stack
	kAddressThread     = 4	// offset from thread structure
/*	kAddressId         = 5,	// offset from const id object
	kAddressIdIndirect = 6,	// offset from stack id object
	kAddressIndex      = 7	// index from id*/
};

enum VerbTypes {
//todo: LUT for drawing
	kVerbNone = 0,
	kVerbPickUp = 1,
	kVerbLookAt = 2,
	kVerbWalkTo = 3,
	kVerbTalkTo = 4,
	kVerbOpen = 5,
	kVerbClose = 6,
	kVerbGive = 7,
	kVerbUse = 8,
	kVerbOptions = 9,
	kVerbEnter = 10,
	kVerbLeave = 11,
	kVerbBegin = 12,
	kVerbWalkOnly = 13,
	kVerbLookOnly = 14,

	kVerbTypesMax = kVerbLookOnly + 1
};

#define STHREAD_TIMESLICE 8

enum ThreadVarTypes {
	kThreadVarObject = 0,
	kThreadVarWithObject = 1,
	kThreadVarAction = 2,
	kThreadVarActor = 3,

	kThreadVarMax = kThreadVarActor + 1
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
	opNextBlock = 1,
	opDup = 2,
	opDrop = 3,
	opZero = 4,
	opOne = 5,
	opConstint = 6,
//...
	opStrlit = 8,
//...
	opGetFlag = 0xB,
	opGetInt = 0xC,
//...
	opPutFlag = 0xf,
	opPutInt = 0x10,
	//...
	opPutFlagV = 0x13,
	opPutIntV = 0x14,
//...
	opCall = 0x17,
	opCcall = 0x18,
	opCcallV = 0x19,
	opEnter = 0x1A,
	opReturn = 0x1B,
	opReturnV = 0x1C,
//...
	opSpeak = 0x53,
	opDialogBegin = 0x54,
	opDialogEnd = 0x55,
	opReply = 0x56
};

enum CycleFlags {
	kCyclePong    = 1 << 0,
	kCycleOnce    = 1 << 1,
	kCycleRandom  = 1 << 2,
	kCycleReverse = 1 << 3
};

enum WalkFlags {
	kWalkBackPedal = 1 << 0,
	kWalkAsync     = 1 << 1,
	kWalkUseAngle  = 1 << 2,
	kWalkFace      = 1 << 5
};

enum ReplyFlags {
	kReplyOnce      = 1 << 0,
	kReplySummary   = 1 << 1,
	kReplyCondition = 1 << 2
};

struct EntryPoint {
	uint16 nameOffset;
	uint16 offset;
};

struct VoiceLUT {
	uint16 voicesCount;
	uint16 *voices;
	void freeMem() {
		free(voices);
	}
};

struct ModuleData {
	bool loaded;			// is it loaded or not?
	int scriptResourceId;
	int stringsResourceId;
	int voicesResourceId;

	byte *moduleBase;				// all base module
	uint16 moduleBaseSize;			// base module size
	uint16 staticSize;				// size of static data
	uint staticOffset;				// offset of static data begining in _commonBuffer

	uint16 entryPointsTableOffset;	// offset of entrypoint table in moduleBase
	uint16 entryPointsCount;
	EntryPoint *entryPoints;

	StringsTable strings;
	VoiceLUT voiceLUT;
	void freeMem() {
		strings.freeMem();
		voiceLUT.freeMem();
		free(moduleBase);
		free(entryPoints);
		memset(this, 0x0, sizeof(*this)); 
	}
};

class ScriptThread {
public:
	uint16 *_stackBuf;
	uint16 _stackSize;					// stack size in uint16
	
	uint16 _stackTopIndex;
	uint16 _frameIndex;

	uint16 _threadVars[kThreadVarMax];

	byte *_moduleBase;					//
	uint16 _moduleBaseSize;

	byte *_commonBase;					// 
	byte *_staticBase;					// 
	VoiceLUT *_voiceLUT;				//
	StringsTable *_strings;				//

	int _flags;							// ThreadFlags
	int _waitType;						// ThreadWaitTypes
	uint _sleepTime;
	void *_threadObj;					// which object we're handling

	uint16 _returnValue;

	uint16 _instructionOffset;			// Instruction offset


public:
	byte *baseAddress(byte addrMode) {
		switch(addrMode) {
			case kAddressCommon:
				return _commonBase;
			case kAddressStatic:
				return _staticBase;
			case kAddressModule:
				return _moduleBase;
			case kAddressStack:
				return (byte*)&_stackBuf[_frameIndex];
			case kAddressThread:
				return (byte*)_threadVars;
			default:
				return _commonBase;
		}
	}

	int16 stackTop() {
		return (int16)_stackBuf[_stackTopIndex];
	}

	uint pushedSize() {
		return _stackSize - _stackTopIndex - 1;
	}

	void push(int16 value) {
		if (_stackTopIndex <= 0) {
			error("ScriptThread::push() stack overflow");
		}
		_stackBuf[--_stackTopIndex] = (uint16)value;
	}

	int16 pop() {
		if (_stackTopIndex >= _stackSize) {
			error("ScriptThread::push() stack underflow");
		}
		return (int16)_stackBuf[_stackTopIndex++];
	}
	

// wait stuff
	void wait(int waitType) {
		_waitType = waitType;
		_flags |= kTFlagWaiting;
	}

	void waitWalk(void *threadObj) {
		wait(kWaitTypeWalk);
		_threadObj = threadObj;
	}

	void waitDelay(int sleepTime) {
		wait(kWaitTypeDelay);
		_sleepTime = sleepTime;
	}

	ScriptThread() {
		memset(this, 0xFE, sizeof(*this)); 
		_stackBuf = NULL;
	}
	~ScriptThread() {
		free(_stackBuf);
	}
};

typedef SortedList<ScriptThread> ScriptThreadList;


#define SCRIPTFUNC_PARAMS ScriptThread *thread, int nArgs

class Script {
public:
	StringsTable _mainStrings;

	Script();
	~Script();
	
	void CF_script_togglestep();

	void loadModule(int scriptModuleNumber);
	void freeModules();

	bool isInitialized() const { return _initialized;  }
	bool isVoiceLUTPresent() const { return _voiceLUTPresent; }
/*	ScriptData *currentScript() { return _currentScript; }
	int getWord(int bufNumber, int wordNumber, ScriptDataWord *data);
	int putWord(int bufNumber, int wordNumber, ScriptDataWord data);
	int setBit(int bufNumber, ScriptDataWord bitNumber, int bitState);
	int getBit(int bufNumber, ScriptDataWord bitNumber, int *bitState);	*/
//	const char * getScriptString(int index) const { return _currentScript->strings.getString(index); }

	void doVerb();
	void showVerb(int statuscolor = -1);
	void setVerb(int verb);
	int getCurrentVerb() const { return _currentVerb; }
	void setPointerVerb();
	void whichObject(const Point& mousePoint);
	void hitObject(bool leftButton);
	void playfieldClick(const Point& mousePoint, bool leftButton);
	
	void setLeftButtonVerb(int verb);
	int getLeftButtonVerb() const { return _leftButtonVerb; }
	void setRightButtonVerb(int verb);
	int getRightButtonVerb() const { return _rightButtonVerb; }
	void setNonPlayfieldVerb() {
		setRightButtonVerb(kVerbNone);
		_pointerObject = ID_NOTHING;
		_currentObject[_firstObjectSet ? 1 : 0] = ID_NOTHING;
	}
	void setNoPendingVerb() {
		_pendingVerb = kVerbNone;
		_currentObject[0] = _currentObject[0] = ID_NOTHING;
		setPointerVerb();
	}

	void scriptInfo();
	void scriptExec(int argc, const char **argv);
	
private:
	bool _initialized;
	bool _voiceLUTPresent;
	RSCFILE_CONTEXT *_scriptContext;
	
	uint16 _modulesLUTEntryLen;
	ModuleData *_modules;
	int _modulesCount;
	
	byte* _commonBuffer;
	uint _commonBufferSize;
	uint _staticSize;

	ScriptThreadList _threadList;
	
	ScriptThread *_conversingThread;

//verb	
	bool _firstObjectSet;
	bool _secondObjectNeeded;
	uint16 _currentObject[2];
	int16 _currentObjectFlags[2];
	int _currentVerb;
	int _stickyVerb;
	int _leftButtonVerb;
	int _rightButtonVerb;

public:	
	uint16 _pendingObject[2];
	int _pendingVerb;
	uint16 _pointerObject;	

	bool _skipSpeeches;
	bool _abortEnabled;

	int _dbg_singlestep;
	int _dbg_dostep;
	ScriptThread *_dbg_thread;
	TEXTLIST_ENTRY *_dbg_txtentry;

public:
	ScriptThread *createThread(uint16 scriptModuleNumber, uint16 scriptEntryPointNumber);
	int executeThread(ScriptThread *thread, int entrypointNumber);
	int executeThreads(uint msec);
	int SThreadDebugStep();
	void completeThread(void);

	void wakeUpActorThread(int waitType, void *threadObj);
	void wakeUpThreads(int waitType);
	void wakeUpThreadsDelayed(int waitType, int sleepTime);

private:
	void loadModuleBase(ModuleData &module, const byte *resourcePointer, size_t resourceLength);
	void loadModuleVoiceLUT(ModuleData &module, const byte *resourcePointer, size_t resourceLength);

	void runThread(ScriptThread *thread, uint instructionLimit);
	void setThreadEntrypoint(ScriptThread *thread, int entrypointNumber);

public:
	void finishDialog(int replyID, int flags, int bitOffset);

private:

	typedef int (Script::*ScriptFunctionType)(SCRIPTFUNC_PARAMS);

	struct ScriptFunctionDescription {
		ScriptFunctionType scriptFunction;
		const char *scriptFunctionName;
	};
	const ScriptFunctionDescription *_scriptFunctionsList;

	void setupScriptFuncList(void);
	void scriptError(ScriptThread *thread, const char *format, ...);
	int SDebugPrintInstr(ScriptThread *thread);

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

/*inline int getSWord(ScriptDataWord word) {
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
*/

} // End of namespace Saga

#endif
