/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Scripting module private header

#ifndef SAGA_SCRIPT_H
#define SAGA_SCRIPT_H

#include "common/endian.h"

#include "saga/font.h"
#include "saga/list.h"

namespace Saga {

#define COMMON_BUFFER_SIZE 1024   // Why 1024?

#define S_LUT_ENTRYLEN_ITECD 22
#define S_LUT_ENTRYLEN_ITEDISK 16

#define SCRIPT_TBLENTRY_LEN 4

#define SCRIPT_MAX 5000

#define ITE_SCRIPT_FUNCTION_MAX 78
#define IHNM_SCRIPT_FUNCTION_MAX 105
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

enum VerbTypeIds {
	kVerbITENone = 0,
	kVerbITEPickUp = 1,
	kVerbITELookAt = 2,
	kVerbITEWalkTo = 3,
	kVerbITETalkTo = 4,
	kVerbITEOpen = 5,
	kVerbITEClose = 6,
	kVerbITEGive = 7,
	kVerbITEUse = 8,
	kVerbITEOptions = 9,
	kVerbITEEnter = 10,
	kVerbITELeave = 11,
	kVerbITEBegin = 12,
	kVerbITEWalkOnly = 13,
	kVerbITELookOnly = 14,


	kVerbIHNMNone = 0,
	kVerbIHNMWalk = 1,
	kVerbIHNMLookAt = 2,
	kVerbIHNMTake = 3,
	kVerbIHNMUse = 4,
	kVerbIHNMTalkTo = 5,
	kVerbIHNMSwallow = 6,
	kVerbIHNMGive = 7,
	kVerbIHNMPush = 8,
	kVerbIHNMOptions = 9,
	kVerbIHNMEnter = 10,
	kVerbIHNMLeave = 11,
	kVerbIHNMBegin = 12,
	kVerbIHNMWalkOnly = 13,
	kVerbIHNMLookOnly = 14,

	kVerbTypeIdsMax = kVerbITELookOnly + 1
};

enum VerbTypes {
	kVerbNone,
	kVerbWalkTo,
	kVerbGive,
	kVerbUse,
	kVerbEnter,
	kVerbLookAt,
	kVerbPickUp,
	kVerbOpen,
	kVerbClose,
	kVerbTalkTo,
	kVerbWalkOnly,
	kVerbLookOnly,
	kVerbOptions
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
	kWaitTypePlacard = 8,
	kWaitTypeStatusTextInput = 9,
	kWaitTypeWaitFrames = 10    // IHNM. waiting for a frame count
};

enum OpCodes {
	opNextBlock = 0x01,
	opDup = 0x02,
	opDrop = 0x03,
	opZero = 0x04,
	opOne = 0x05,
	opConstint = 0x06,
//...
	opStrlit = 0x08,
//...
	opGetFlag = 0x0B,
	opGetInt = 0x0C,
//...
	opPutFlag = 0x0F,
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
	opJmp = 0x1D,
	opJmpTrueV = 0x1E,
	opJmpFalseV = 0x1F,
	opJmpTrue = 0x20,
	opJmpFalse = 0x21,
	opJmpSwitch = 0x22,
//...
	opJmpRandom = 0x24,
	opNegate = 0x25,
	opNot = 0x26,
	opCompl = 0x27,
	opIncV = 0x28,
	opDecV = 0x29,
	opPostInc = 0x2A,
	opPostDec = 0x2B,
	opAdd = 0x2C,
	opSub = 0x2D,
	opMul = 0x2E,
	opDiv = 0x2F,
	opMod = 0x30,
//...
	opEq = 0x33,
	opNe = 0x34,
	opGt = 0x35,
	opLt = 0x36,
	opGe = 0x37,
	opLe = 0x38,
//...
	opRsh = 0x3F,
	opLsh = 0x40,
	opAnd = 0x41,
	opOr = 0x42,
	opXor = 0x43,
	opLAnd = 0x44,
	opLOr = 0x45,
	opLXor = 0x46,
//...
	opSpeak = 0x53,
	opDialogBegin = 0x54,
	opDialogEnd = 0x55,
	opReply = 0x56,
	opAnimate = 0x57
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
		voicesCount = 0;
		free(voices);
	}
	VoiceLUT() {
		memset(this, 0, sizeof(*this));
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

	int16 _returnValue;

	uint16 _instructionOffset;			// Instruction offset

	int32 _frameWait;

public:
	byte *baseAddress(byte addrMode) {
		switch (addrMode) {
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
		return _stackSize - _stackTopIndex - 2;
	}

	void push(int16 value) {
		if (_stackTopIndex <= 0) {
			error("ScriptThread::push() stack overflow");
		}
		_stackBuf[--_stackTopIndex] = (uint16)value;
	}

	int16 pop() {
		if (_stackTopIndex >= _stackSize) {
			error("ScriptThread::pop() stack underflow");
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


#define SCRIPTFUNC_PARAMS ScriptThread *thread, int nArgs, bool &disContinue

class Script {
public:
	StringsTable _mainStrings;

	Script(SagaEngine *vm);
	~Script();

	void loadModule(int scriptModuleNumber);
	void freeModules();

	void doVerb();
	void showVerb(int statusColor = -1);
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
		setRightButtonVerb(getVerbType(kVerbNone));
		_pointerObject = ID_NOTHING;
		_currentObject[_firstObjectSet ? 1 : 0] = ID_NOTHING;
	}
	void setNoPendingVerb() {
		_pendingVerb = getVerbType(kVerbNone);
		_currentObject[0] = _currentObject[0] = ID_NOTHING;
		setPointerVerb();
	}
	int getVerbType(VerbTypes verbType);

private:
	// When reading or writing data to the common buffer, we have to use a
	// well-defined byte order since it's stored in savegames. Otherwise,
	// we use native byte ordering since that data may be accessed in other
	// ways than through these functions.
	//
	// The "module" area is a special case, which possibly never ever
	// happens. But if it does, we need well-defined byte ordering.

	uint16 readUint16(byte *addr, byte addrMode) {
		switch (addrMode) {
		case kAddressCommon:
		case kAddressStatic:
		case kAddressModule:
			return READ_LE_UINT16(addr);
		default:
			return READ_UINT16(addr);
		}
	}

	void writeUint16(byte *addr, uint16 value, byte addrMode) {
		switch (addrMode) {
		case kAddressCommon:
		case kAddressStatic:
		case kAddressModule:
			WRITE_LE_UINT16(addr, value);
			break;
		default:
			WRITE_UINT16(addr, value);
			break;
		}
	}

	SagaEngine *_vm;
	ResourceContext *_scriptContext;

	uint16 _modulesLUTEntryLen;
	ModuleData *_modules;
	int _modulesCount;
	TextListEntry *_placardTextEntry;

protected:
	friend class SagaEngine;
	byte *_commonBuffer;
	uint _commonBufferSize;

private:
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

	VoiceLUT _globalVoiceLUT;

public:
	ScriptThread *createThread(uint16 scriptModuleNumber, uint16 scriptEntryPointNumber);
	int executeThread(ScriptThread *thread, int entrypointNumber);
	void executeThreads(uint msec);
	void completeThread(void);
	void abortAllThreads(void);

	void wakeUpActorThread(int waitType, void *threadObj);
	void wakeUpThreads(int waitType);
	void wakeUpThreadsDelayed(int waitType, int sleepTime);

	void loadVoiceLUT(VoiceLUT &voiceLUT, const byte *resourcePointer, size_t resourceLength);

private:
	void loadModuleBase(ModuleData &module, const byte *resourcePointer, size_t resourceLength);

	// runThread returns true if we should break running of other threads
	bool runThread(ScriptThread *thread, uint instructionLimit);
	void setThreadEntrypoint(ScriptThread *thread, int entrypointNumber);

public:
	void finishDialog(int replyID, int flags, int bitOffset);

private:

	typedef void (Script::*ScriptFunctionType)(SCRIPTFUNC_PARAMS);

	struct ScriptFunctionDescription {
		ScriptFunctionType scriptFunction;
		const char *scriptFunctionName;
	};
	const ScriptFunctionDescription *_scriptFunctionsList;

	void setupScriptFuncList(void);

	void sfPutString(SCRIPTFUNC_PARAMS);
	void sfWait(SCRIPTFUNC_PARAMS);
	void sfTakeObject(SCRIPTFUNC_PARAMS);
	void sfIsCarried(SCRIPTFUNC_PARAMS);
	void sfStatusBar(SCRIPTFUNC_PARAMS);
	void sfMainMode(SCRIPTFUNC_PARAMS);
	void sfScriptWalkTo(SCRIPTFUNC_PARAMS);
	void sfScriptDoAction(SCRIPTFUNC_PARAMS);
	void sfSetActorFacing(SCRIPTFUNC_PARAMS);
	void sfStartBgdAnim(SCRIPTFUNC_PARAMS);
	void sfStopBgdAnim(SCRIPTFUNC_PARAMS);
	void sfLockUser(SCRIPTFUNC_PARAMS);
	void sfPreDialog(SCRIPTFUNC_PARAMS);
	void sfKillActorThreads(SCRIPTFUNC_PARAMS);
	void sfFaceTowards(SCRIPTFUNC_PARAMS);
	void sfSetFollower(SCRIPTFUNC_PARAMS);
	void sfScriptGotoScene(SCRIPTFUNC_PARAMS);

	void sfSetObjImage(SCRIPTFUNC_PARAMS);
	void sfSetObjName(SCRIPTFUNC_PARAMS);
	void sfGetObjImage(SCRIPTFUNC_PARAMS);
	void sfGetNumber(SCRIPTFUNC_PARAMS);
	void sfScriptOpenDoor(SCRIPTFUNC_PARAMS);
	void sfScriptCloseDoor(SCRIPTFUNC_PARAMS);
	void sfSetBgdAnimSpeed(SCRIPTFUNC_PARAMS);
	void SF_cycleColors(SCRIPTFUNC_PARAMS);
	void sfDoCenterActor(SCRIPTFUNC_PARAMS);
	void sfStartBgdAnimSpeed(SCRIPTFUNC_PARAMS);
	void sfScriptWalkToAsync(SCRIPTFUNC_PARAMS);
	void sfEnableZone(SCRIPTFUNC_PARAMS);
	void sfSetActorState(SCRIPTFUNC_PARAMS);
	void sfScriptMoveTo(SCRIPTFUNC_PARAMS);
	void sfSceneEq(SCRIPTFUNC_PARAMS);
	void sfDropObject(SCRIPTFUNC_PARAMS);
	void sfFinishBgdAnim(SCRIPTFUNC_PARAMS);
	void sfSwapActors(SCRIPTFUNC_PARAMS);
	void sfSimulSpeech(SCRIPTFUNC_PARAMS);
	void sfScriptWalk(SCRIPTFUNC_PARAMS);
	void sfCycleFrames(SCRIPTFUNC_PARAMS);
	void sfSetFrame(SCRIPTFUNC_PARAMS);
	void sfSetPortrait(SCRIPTFUNC_PARAMS);
	void sfSetProtagPortrait(SCRIPTFUNC_PARAMS);
	void sfChainBgdAnim(SCRIPTFUNC_PARAMS);
	void sfScriptSpecialWalk(SCRIPTFUNC_PARAMS);
	void sfPlaceActor(SCRIPTFUNC_PARAMS);
	void sfCheckUserInterrupt(SCRIPTFUNC_PARAMS);
	void sfScriptWalkRelative(SCRIPTFUNC_PARAMS);
	void sfScriptMoveRelative(SCRIPTFUNC_PARAMS);
	void sfSimulSpeech2(SCRIPTFUNC_PARAMS);
	void sfPlacard(SCRIPTFUNC_PARAMS);
	void sfPlacardOff(SCRIPTFUNC_PARAMS);
	void sfSetProtagState(SCRIPTFUNC_PARAMS);
	void sfResumeBgdAnim(SCRIPTFUNC_PARAMS);
	void sfThrowActor(SCRIPTFUNC_PARAMS);
	void sfWaitWalk(SCRIPTFUNC_PARAMS);
	void sfScriptSceneID(SCRIPTFUNC_PARAMS);
	void sfChangeActorScene(SCRIPTFUNC_PARAMS);
	void sfScriptClimb(SCRIPTFUNC_PARAMS);
	void sfSetDoorState(SCRIPTFUNC_PARAMS);
	void sfSetActorZ(SCRIPTFUNC_PARAMS);
	void sfScriptText(SCRIPTFUNC_PARAMS);
	void sfGetActorX(SCRIPTFUNC_PARAMS);
	void sfGetActorY(SCRIPTFUNC_PARAMS);
	void sfEraseDelta(SCRIPTFUNC_PARAMS);
	void sfPlayMusic(SCRIPTFUNC_PARAMS);
	void sfPickClimbOutPos(SCRIPTFUNC_PARAMS);
	void sfTossRif(SCRIPTFUNC_PARAMS);
	void sfShowControls(SCRIPTFUNC_PARAMS);
	void sfShowMap(SCRIPTFUNC_PARAMS);
	void sfPuzzleWon(SCRIPTFUNC_PARAMS);
	void sfEnableEscape(SCRIPTFUNC_PARAMS);
	void sfPlaySound(SCRIPTFUNC_PARAMS);
	void sfPlayLoopedSound(SCRIPTFUNC_PARAMS);
	void sfGetDeltaFrame(SCRIPTFUNC_PARAMS);
	void sfShowProtect(SCRIPTFUNC_PARAMS);
	void sfProtectResult(SCRIPTFUNC_PARAMS);
	void sfRand(SCRIPTFUNC_PARAMS);
	void sfFadeMusic(SCRIPTFUNC_PARAMS);
	void sfScriptStartCutAway(SCRIPTFUNC_PARAMS);
	void sfReturnFromCutAway(SCRIPTFUNC_PARAMS);
	void sfEndCutAway(SCRIPTFUNC_PARAMS);
	void sfGetMouseClicks(SCRIPTFUNC_PARAMS);
	void sfResetMouseClicks(SCRIPTFUNC_PARAMS);
	void sfWaitFrames(SCRIPTFUNC_PARAMS);
	void sfScriptFade(SCRIPTFUNC_PARAMS);
	void sfPlayVoice(SCRIPTFUNC_PARAMS);
	void sfVstopFX(SCRIPTFUNC_PARAMS);
	void sfVstopLoopedFX(SCRIPTFUNC_PARAMS);
	void sfDemoIsInteractive(SCRIPTFUNC_PARAMS);
	void sfVsetTrack(SCRIPTFUNC_PARAMS);
	void sfDebugShowData(SCRIPTFUNC_PARAMS);
	void sfNull(SCRIPTFUNC_PARAMS);
	void sfWaitFramesEsc(SCRIPTFUNC_PARAMS);
	void sfPsychicProfile(SCRIPTFUNC_PARAMS);
	void sfPsychicProfileOff(SCRIPTFUNC_PARAMS);
	void sfSetSpeechBox(SCRIPTFUNC_PARAMS);
	void sfSetChapterPoints(SCRIPTFUNC_PARAMS);
	void sfSetPortraitBgColor(SCRIPTFUNC_PARAMS);
	void sfScriptStartVideo(SCRIPTFUNC_PARAMS);
	void sfScriptReturnFromVideo(SCRIPTFUNC_PARAMS);
	void sfScriptEndVideo(SCRIPTFUNC_PARAMS);
	void sf87(SCRIPTFUNC_PARAMS);
	void sf88(SCRIPTFUNC_PARAMS);
	void sf89(SCRIPTFUNC_PARAMS);
	void sfGetPoints(SCRIPTFUNC_PARAMS);
	void sfSetGlobalFlag(SCRIPTFUNC_PARAMS);
	void sfClearGlobalFlag(SCRIPTFUNC_PARAMS);
	void sfTestGlobalFlag(SCRIPTFUNC_PARAMS);
	void sfSetPoints(SCRIPTFUNC_PARAMS);
	void sf103(SCRIPTFUNC_PARAMS);
	void sfDisableAbortSpeeches(SCRIPTFUNC_PARAMS);

	void SF_stub(const char *name, ScriptThread *thread, int nArgs);
};

} // End of namespace Saga

#endif
