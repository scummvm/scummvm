/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
#include "saga/yslib.h"

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

#define SFUNC_NUM 78

typedef unsigned int SDataWord_T;

enum SCRIPT_VERBS {
	S_VERB_WALKTO = 0,
	S_VERB_LOOKAT = 2,
	S_VERB_PICKUP = 1,
	S_VERB_TALKTO,
	S_VERB_OPEN = 5,
	S_VERB_CLOSE = 6,
	S_VERB_USE = 8,
	S_VERB_GIVE
};

#define STHREAD_TIMESLICE 8

struct SEMAPHORE {
	int hold_count;
};

enum {
	kVarObject = 0,
	kVarWithObject,
	kVarAction,
	kVarActor
};

enum {
	kTFlagNone = 0,
	kTFlagWaiting = 1,	// wait for even denoted in waitType
	kTFlagFinished = 2,
	kTFlagAborted = 4,
	kTFlagAsleep = 7	// Combination of all flags which can halt a thread
};

enum {
	kTWaitNone = 0,		// waiting for nothing
	kTWaitDelay,		// waiting for a timer
	kTWaitSpeech,		// waiting for speech to finish
	kTWaitDialogEnd,	// waiting for my dialog to finish
	kTWaitDialogBegin,	// waiting for other dialog to finish
	kTWaitWalk,			// waiting to finish walking
	kTWaitRequest,		// a request is up
	kTWaitPause
};

struct SCRIPT_THREAD {
	int flags;
	int waitType;

	uint sleepTime;
	int ep_num; // Entrypoint number
	unsigned long ep_offset; // Entrypoint offset
	unsigned long i_offset; // Instruction offset

	SEMAPHORE sem; // FIXME: no equivalent. should be replaced with flags

	// The scripts are allowed to access the stack like any other memory
	// area. It's therefore probably quite important that our stacks work
	// the same as in the original interpreter.

	SDataWord_T stackBuf[64];

	int stackPtr;
	int framePtr;

	SDataWord_T threadVars[4];

	SDataWord_T retVal;

	SDataWord_T stackTop() {
		return stackBuf[stackPtr];
	}

	int stackSize() {
		return ARRAYSIZE(stackBuf) - stackPtr - 1;
	}

	void push(SDataWord_T n) {
		assert(stackPtr > 0);
		stackBuf[--stackPtr] = n;
	}

	SDataWord_T pop() {
		assert(stackPtr < ARRAYSIZE(stackBuf));
		return stackBuf[stackPtr++];
	}
};

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

struct DIALOGUE_LIST {
	unsigned int n_dialogue;
	const char **str;
	size_t *str_off;
};

struct VOICE_LUT {
	int n_voices;
	int *voices;
};

struct SCRIPTDATA {
	int loaded;
	SCRIPT_BYTECODE *bytecode;
	DIALOGUE_LIST *diag;
	VOICE_LUT *voice;
};

struct SCRIPT_LUT_ENTRY {
	int script_rn;
	int diag_list_rn;
	int voice_lut_rn;
};

struct SCRIPT_DATABUF {
	SDataWord_T *data;
	int len;
};

#define SCRIPTFUNC_PARAMS SCRIPT_THREAD *thread, int nArgs

class Script {
public:
	Script();
	~Script();
	
	int reg(void);
	int loadScript(int scriptNum);
	int freeScript();
	SCRIPT_BYTECODE *loadBytecode(byte *bytecode_p, size_t bytecode_len);
	DIALOGUE_LIST *loadDialogue(const byte *dialogue_p, size_t dialogue_len);
	VOICE_LUT *loadVoiceLUT(const byte *voicelut_p, size_t voicelut_len, SCRIPTDATA *script);
	int disassemble(SCRIPT_BYTECODE *script_list, DIALOGUE_LIST *diag_list);

	bool isInitialized() const { return _initialized;  }
	bool isVoiceLUTPresent() const { return _voiceLUTPresent; }
	SCRIPTDATA *currentScript() { return _currentScript; }
	void setBuffer(int idx, SCRIPT_DATABUF *ptr) { _dataBuf[idx] = ptr; }
	SCRIPT_DATABUF *dataBuffer(int idx) { return _dataBuf[idx]; }
	YS_DL_LIST *threadList() { return _threadList; }

	void scriptInfo(int argc, char *argv[]);
	void scriptExec(int argc, char *argv[]);
	
protected:
	bool _initialized;
	bool _voiceLUTPresent;
	RSCFILE_CONTEXT *_scriptContext;
	SCRIPT_LUT_ENTRY *_scriptLUT;
	int _scriptLUTMax;
	uint16 _scriptLUTEntryLen;
	SCRIPTDATA *_currentScript;
	SCRIPT_DATABUF *_dataBuf[SCRIPT_DATABUF_NUM];
	YS_DL_LIST *_threadList;

	bool _skipSpeeches;
	bool _abortEnabled;

public:
	int _dbg_singlestep;
	int _dbg_dostep;
	SCRIPT_THREAD *_dbg_thread;
	TEXTLIST_ENTRY *_dbg_txtentry;

public:
	SCRIPT_THREAD *SThreadCreate();
	int SThreadExecute(SCRIPT_THREAD *thread, int ep_num);
	int SThreadExecThreads(uint msec);
	int SThreadHoldSem(SEMAPHORE *sem);
	int SThreadReleaseSem(SEMAPHORE *sem);
	int SThreadDebugStep();
	void SThreadCompleteThread(void);
	int SThreadDestroy(SCRIPT_THREAD *thread);
	void SThreadAbortAll(void);

private:
	void setFramePtr(SCRIPT_THREAD *thread, int newPtr);
	unsigned char *SThreadGetReadPtr(SCRIPT_THREAD *thread);
	unsigned long SThreadGetReadOffset(const byte *read_p);
	size_t SThreadGetReadLen(SCRIPT_THREAD *thread);
	int SThreadRun(SCRIPT_THREAD *thread, int instr_limit);
	int SThreadSetEntrypoint(SCRIPT_THREAD *thread, int ep_num);

private:
	typedef int (Script::*SFunc_T)(SCRIPTFUNC_PARAMS);

	const SFunc_T *_SFuncList;

	void setupScriptFuncList(void);
	int SDebugPrintInstr(SCRIPT_THREAD *thread);

	int SF_putString(SCRIPTFUNC_PARAMS);
	int SF_sleep(SCRIPTFUNC_PARAMS);
	int SF_takeObject(SCRIPTFUNC_PARAMS);
	int SF_objectIsCarried(SCRIPTFUNC_PARAMS);
	int SF_setStatusText(SCRIPTFUNC_PARAMS);
	int SF_commandMode(SCRIPTFUNC_PARAMS);
	int SF_actorWalkTo(SCRIPTFUNC_PARAMS);
	int SF_doAction(SCRIPTFUNC_PARAMS);
	int SF_setFacing(SCRIPTFUNC_PARAMS);
	int SF_startBgdAnim(SCRIPTFUNC_PARAMS);
	int SF_stopBgdAnim(SCRIPTFUNC_PARAMS);
	int SF_freezeInterface(SCRIPTFUNC_PARAMS);
	int SF_dialogMode(SCRIPTFUNC_PARAMS);
	int SF_killActorThreads(SCRIPTFUNC_PARAMS);
	int SF_faceTowards(SCRIPTFUNC_PARAMS);
	int SF_setFollower(SCRIPTFUNC_PARAMS);
	int SF_gotoScene(SCRIPTFUNC_PARAMS);
	int SF_setObjImage(SCRIPTFUNC_PARAMS);
	int SF_setObjName(SCRIPTFUNC_PARAMS);
	int SF_getObjName(SCRIPTFUNC_PARAMS);
	int SF_getNumber(SCRIPTFUNC_PARAMS);
	int SF_openDoor(SCRIPTFUNC_PARAMS);
	int SF_closeDoor(SCRIPTFUNC_PARAMS);
	int SF_setBgdAnimSpeed(SCRIPTFUNC_PARAMS);
	int SF_cycleColors(SCRIPTFUNC_PARAMS);
	int SF_centerActor(SCRIPTFUNC_PARAMS);
	int SF_startAnim(SCRIPTFUNC_PARAMS);
	int SF_actorWalkToAsync(SCRIPTFUNC_PARAMS);
	int SF_enableZone(SCRIPTFUNC_PARAMS);
	int SF_setActorState(SCRIPTFUNC_PARAMS);
	int SF_moveTo(SCRIPTFUNC_PARAMS);
	int SF_sceneEq(SCRIPTFUNC_PARAMS);
	int SF_dropObject(SCRIPTFUNC_PARAMS);
	int SF_finishBgdAnim(SCRIPTFUNC_PARAMS);
	int SF_swapActors(SCRIPTFUNC_PARAMS);
	int SF_simulSpeech(SCRIPTFUNC_PARAMS);
	int SF_actorWalk(SCRIPTFUNC_PARAMS);
	int SF_cycleActorFrames(SCRIPTFUNC_PARAMS);
	int SF_setFrame(SCRIPTFUNC_PARAMS);
	int SF_setRightPortrait(SCRIPTFUNC_PARAMS);
	int SF_setLeftPortrait(SCRIPTFUNC_PARAMS);
	int SF_linkAnim(SCRIPTFUNC_PARAMS);
	int SF_scriptSpecialWalk(SCRIPTFUNC_PARAMS);
	int SF_placeActor(SCRIPTFUNC_PARAMS);
	int SF_checkUserInterrupt(SCRIPTFUNC_PARAMS);
	int SF_walkRelative(SCRIPTFUNC_PARAMS);
	int SF_moveRelative(SCRIPTFUNC_PARAMS);
	int SF_simulSPeech2(SCRIPTFUNC_PARAMS);
	int SF_placard(SCRIPTFUNC_PARAMS);
	int SF_placardOff(SCRIPTFUNC_PARAMS);
	int SF_setProtagState(SCRIPTFUNC_PARAMS);
	int SF_resumeBgdAnim(SCRIPTFUNC_PARAMS);
	int SF_throwActor(SCRIPTFUNC_PARAMS);
	int SF_waitWalk(SCRIPTFUNC_PARAMS);
	int SF_sceneID(SCRIPTFUNC_PARAMS);
	int SF_changeActorScene(SCRIPTFUNC_PARAMS);
	int SF_climb(SCRIPTFUNC_PARAMS);
	int SF_setDoorState(SCRIPTFUNC_PARAMS);
	int SF_setActorZ(SCRIPTFUNC_PARAMS);
	int SF_text(SCRIPTFUNC_PARAMS);
	int SF_getActorX(SCRIPTFUNC_PARAMS);
	int SF_getActorY(SCRIPTFUNC_PARAMS);
	int SF_eraseDelta(SCRIPTFUNC_PARAMS);
	int SF_playMusic(SCRIPTFUNC_PARAMS);
	int SF_pickClimbOutPos(SCRIPTFUNC_PARAMS);
	int SF_tossRif(SCRIPTFUNC_PARAMS);
	int SF_showControls(SCRIPTFUNC_PARAMS);
	int SF_showMap(SCRIPTFUNC_PARAMS);
	int SF_puzzleWon(SCRIPTFUNC_PARAMS);
	int SF_enableEscape(SCRIPTFUNC_PARAMS);
	int SF_playSound(SCRIPTFUNC_PARAMS);
	int SF_playLoopedSound(SCRIPTFUNC_PARAMS);
	int SF_getDeltaFrame(SCRIPTFUNC_PARAMS);
	int SF_showProtect(SCRIPTFUNC_PARAMS);
	int SF_protectResult(SCRIPTFUNC_PARAMS);
	int SF_rand(SCRIPTFUNC_PARAMS);
	int SF_fadeMusic(SCRIPTFUNC_PARAMS);
	int SF_playVoice(SCRIPTFUNC_PARAMS);
};

} // End of namespace Saga

#endif
