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

#include "sstack.h"
#include "sdata.h"

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

struct R_SCRIPT_MODULE {
	int initialized;
	R_RSCFILE_CONTEXT *script_ctxt;
	int voice_lut_present;
	R_SCRIPT_LUT_ENTRY *script_lut;
	int script_lut_max;
	uint16 script_lut_entrylen;
	R_SCRIPTDATA *current_script;
	YS_DL_LIST *thread_list;
	R_SCRIPT_DATABUF *data_buf[R_SCRIPT_DATABUF_NUM];
	int dbg_singlestep;
	int dbg_dostep;
	R_SCRIPT_THREAD *dbg_thread;
	R_TEXTLIST_ENTRY *dbg_txtentry;
};

extern R_SCRIPT_MODULE ScriptModule;

R_SCRIPT_BYTECODE *SCRIPT_LoadBytecode(byte *bytecode_p, size_t bytecode_len);
R_DIALOGUE_LIST *SCRIPT_LoadDialogue(const byte *dialogue_p, size_t dialogue_len);
R_VOICE_LUT *SCRIPT_LoadVoiceLUT(const byte *voicelut_p, size_t voicelut_len, R_SCRIPTDATA *script);
int SCRIPT_Disassemble(R_SCRIPT_BYTECODE *script_list, R_DIALOGUE_LIST *diag_list);
void CF_script_info(int argc, char *argv[]);
void CF_script_exec(int argc, char *argv[]);
void CF_script_togglestep(int argc, char *argv[]);

} // End of namespace Saga

#endif
