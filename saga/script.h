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

#define R_SCRIPT_DATABUF_NUM 5
#define R_SCRIPT_DATABUF_LEN 1024

#define R_S_LUT_ENTRYLEN_ITECD 22
#define R_S_LUT_ENTRYLEN_ITEDISK 16

#define R_SCRIPT_TBLENTRY_LEN 4

#define R_SCRIPT_MAX 5000
#define R_SCRIPTLIST_HDR 12
#define R_SCRIPT_STRINGLIMIT 255
#define R_TAB "    "

#define S_ERROR_PREFIX "SError: "
#define S_WARN_PREFIX "SWarning: "

#define R_SFUNC_NUM 78

typedef unsigned int SDataWord_T;

enum R_SCRIPT_VERBS {
	S_VERB_WALKTO = 0,
	S_VERB_LOOKAT = 2,
	S_VERB_PICKUP = 1,
	S_VERB_TALKTO,
	S_VERB_OPEN = 5,
	S_VERB_CLOSE = 6,
	S_VERB_USE = 8,
	S_VERB_GIVE
};

#define STHREAD_DEF_INSTR_COUNT 8

struct R_SEMAPHORE {
	int hold_count;
};

enum {
	kVarObject = 0,
	kVarWithObject,
	kVarAction,
	kVarActor
};

struct R_SCRIPT_THREAD {
	int executing;

	int sleep_time;
	int ep_num; // Entrypoint number
	unsigned long ep_offset; // Entrypoint offset
	unsigned long i_offset; // Instruction offset

	R_SEMAPHORE sem;

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

struct R_PROC_TBLENTRY {
	size_t name_offset;
	size_t offset;
};

struct R_SCRIPT_BYTECODE {
	unsigned char *bytecode_p;
	size_t bytecode_len;
	size_t ep_tbl_offset;
	unsigned long n_entrypoints;
	R_PROC_TBLENTRY *entrypoints;
};

struct R_DIALOGUE_LIST {
	unsigned int n_dialogue;
	const char **str;
	size_t *str_off;
};

struct R_VOICE_LUT {
	int n_voices;
	int *voices;
};

struct R_SCRIPTDATA {
	int loaded;
	R_SCRIPT_BYTECODE *bytecode;
	R_DIALOGUE_LIST *diag;
	R_VOICE_LUT *voice;
};

struct R_SCRIPT_LUT_ENTRY {
	int script_rn;
	int diag_list_rn;
	int voice_lut_rn;
};

struct R_SCRIPT_DATABUF {
	SDataWord_T *data;
	int len;
};

#define R_SCRIPTFUNC_PARAMS R_SCRIPT_THREAD *thread

class Script {
public:
	Script();
	~Script();
	
	int reg(void);
	int loadScript(int scriptNum);
	int freeScript();
	R_SCRIPT_BYTECODE *loadBytecode(byte *bytecode_p, size_t bytecode_len);
	R_DIALOGUE_LIST *loadDialogue(const byte *dialogue_p, size_t dialogue_len);
	R_VOICE_LUT *loadVoiceLUT(const byte *voicelut_p, size_t voicelut_len, R_SCRIPTDATA *script);
	int disassemble(R_SCRIPT_BYTECODE *script_list, R_DIALOGUE_LIST *diag_list);

	bool isInitialized() const { return _initialized;  }
	bool isVoiceLUTPresent() const { return _voiceLUTPresent; }
	R_SCRIPTDATA *currentScript() { return _currentScript; }
	void setBuffer(int idx, R_SCRIPT_DATABUF *ptr) { _dataBuf[idx] = ptr; }
	R_SCRIPT_DATABUF *dataBuffer(int idx) { return _dataBuf[idx]; }
	YS_DL_LIST *threadList() { return _threadList; }

	void scriptInfo(int argc, char *argv[]);
	void scriptExec(int argc, char *argv[]);
	
protected:
	bool _initialized;
	bool _voiceLUTPresent;
	R_RSCFILE_CONTEXT *_scriptContext;
	R_SCRIPT_LUT_ENTRY *_scriptLUT;
	int _scriptLUTMax;
	uint16 _scriptLUTEntryLen;
	R_SCRIPTDATA *_currentScript;
	R_SCRIPT_DATABUF *_dataBuf[R_SCRIPT_DATABUF_NUM];
	YS_DL_LIST *_threadList;

public:
	int _dbg_singlestep;
	int _dbg_dostep;
	R_SCRIPT_THREAD *_dbg_thread;
	R_TEXTLIST_ENTRY *_dbg_txtentry;

public:
	R_SCRIPT_THREAD *SThreadCreate();
	int SThreadExecute(R_SCRIPT_THREAD *thread, int ep_num);
	int SThreadExecThreads(int msec);
	int SThreadHoldSem(R_SEMAPHORE *sem);
	int SThreadReleaseSem(R_SEMAPHORE *sem);
	int SThreadDebugStep();
	void SThreadCompleteThread(void);
	int SThreadDestroy(R_SCRIPT_THREAD *thread);

private:
	void setFramePtr(R_SCRIPT_THREAD *thread, int newPtr);
	unsigned char *SThreadGetReadPtr(R_SCRIPT_THREAD *thread);
	unsigned long SThreadGetReadOffset(const byte *read_p);
	size_t SThreadGetReadLen(R_SCRIPT_THREAD *thread);
	int SThreadRun(R_SCRIPT_THREAD *thread, int instr_limit, int msec);
	int SThreadSetEntrypoint(R_SCRIPT_THREAD *thread, int ep_num);

private:
	typedef int (Script::*SFunc_T)(R_SCRIPTFUNC_PARAMS);

	struct R_SFUNC_ENTRY {
		int sfunc_num;
		int sfunc_argc;
		SFunc_T sfunc_fp;
	};

	const R_SFUNC_ENTRY *_SFuncList;

	void setupScriptFuncList(void);
	int SDebugPrintInstr(R_SCRIPT_THREAD *thread);

	int SF_sleep(R_SCRIPTFUNC_PARAMS);
	int SF_takeObject(R_SCRIPTFUNC_PARAMS);
	int SF_objectIsCarried(R_SCRIPTFUNC_PARAMS);
	int SF_setStatusText(R_SCRIPTFUNC_PARAMS);
	int SF_commandMode(R_SCRIPTFUNC_PARAMS);
	int SF_actorWalkTo(R_SCRIPTFUNC_PARAMS);
	int SF_setFacing(R_SCRIPTFUNC_PARAMS);
	int SF_startBgdAnim(R_SCRIPTFUNC_PARAMS);
	int SF_freezeInterface(R_SCRIPTFUNC_PARAMS);
	int SF_dialogMode(R_SCRIPTFUNC_PARAMS);
	int SF_startAnim(R_SCRIPTFUNC_PARAMS);
	int SF_stopBgdAnim(R_SCRIPTFUNC_PARAMS);
	int SF_actorWalkToAsync(R_SCRIPTFUNC_PARAMS);
	int SF_moveTo(R_SCRIPTFUNC_PARAMS);
	int SF_actorWalk(R_SCRIPTFUNC_PARAMS);
	int SF_cycleActorFrames(R_SCRIPTFUNC_PARAMS);
	int SF_setFrame(R_SCRIPTFUNC_PARAMS);
	int SF_setRightPortrait(R_SCRIPTFUNC_PARAMS);
	int SF_setLeftPortrait(R_SCRIPTFUNC_PARAMS);
	int SF_linkAnim(R_SCRIPTFUNC_PARAMS);
	int SF_placeActor(R_SCRIPTFUNC_PARAMS);
	int SF_checkUserInterrupt(R_SCRIPTFUNC_PARAMS);
	int SF_moveRelative(R_SCRIPTFUNC_PARAMS);
	int SF_doAction(R_SCRIPTFUNC_PARAMS);
	int SF_faceTowards(R_SCRIPTFUNC_PARAMS);
	int SF_setFollower(R_SCRIPTFUNC_PARAMS);
	int SF_setBgdAnimSpeed(R_SCRIPTFUNC_PARAMS);
	int SF_centerActor(R_SCRIPTFUNC_PARAMS);
	int SF_setActorState(R_SCRIPTFUNC_PARAMS);
	int SF_finishBgdAnim(R_SCRIPTFUNC_PARAMS);
	int SF_swapActors(R_SCRIPTFUNC_PARAMS);
	int SF_scriptSpecialWalk(R_SCRIPTFUNC_PARAMS);
	int SF_walkRelative(R_SCRIPTFUNC_PARAMS);
	int SF_throwActor(R_SCRIPTFUNC_PARAMS);
	int SF_waitWalk(R_SCRIPTFUNC_PARAMS);
	int SF_changeActorScene(R_SCRIPTFUNC_PARAMS);
	int SF_climb(R_SCRIPTFUNC_PARAMS);
	int SF_setActorZ(R_SCRIPTFUNC_PARAMS);
	int SF_getActorX(R_SCRIPTFUNC_PARAMS);
	int SF_getActorY(R_SCRIPTFUNC_PARAMS);
	int SF_playMusic(R_SCRIPTFUNC_PARAMS);
	int SF_enableEscape(R_SCRIPTFUNC_PARAMS);
	int SF_playSound(R_SCRIPTFUNC_PARAMS);
};

} // End of namespace Saga

#endif
