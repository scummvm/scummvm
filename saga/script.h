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

#include "saga/sdata.h"
#include "saga/text.h"
#include "saga/yslib.h"

namespace Saga {

#define R_S_LUT_ENTRYLEN_ITECD 22
#define R_S_LUT_ENTRYLEN_ITEDISK 16

#define R_SCRIPT_TBLENTRY_LEN 4

#define R_SCRIPT_MAX 5000
#define R_SCRIPTLIST_HDR 12
#define R_SCRIPT_STRINGLIMIT 255
#define R_TAB "    "

#define R_DEF_THREAD_STACKSIZE 16

#define S_ERROR_PREFIX "SError: "
#define S_WARN_PREFIX "SWarning: "

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
};

} // End of namespace Saga

#endif
