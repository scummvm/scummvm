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

// Scripting module: Script resource handling functions
#include "saga.h"
#include "reinherit.h"

#include "yslib.h"

#include "rscfile_mod.h"
#include "game_mod.h"
#include "text_mod.h"
#include "console_mod.h"
#include "cvar_mod.h"

#include "script_mod.h"
#include "script.h"
#include "sstack.h"
#include "sthread.h"

namespace Saga {

R_SCRIPT_MODULE ScriptModule;

int SCRIPT_Register() {
	CVAR_RegisterFunc(CF_script_info, "script_info", NULL, R_CVAR_NONE, 0, 0, NULL);
	CVAR_RegisterFunc(CF_script_exec, "script_exec", "<Script number>", R_CVAR_NONE, 1, 1, NULL);
	CVAR_RegisterFunc(CF_script_togglestep, "script_togglestep", NULL, R_CVAR_NONE, 0, 0, NULL);

	return R_SUCCESS;
}

// Initializes the scripting module.
// Loads script resource look-up table, initializes script data system
int SCRIPT_Init() {
	R_RSCFILE_CONTEXT *s_lut_ctxt;
	byte *rsc_ptr;
	size_t rsc_len;
	int prevTell;
	int result;
	int i, j;

	debug(0, "Initializing scripting subsystem.");
	// Load script resource file context
	result = GAME_GetFileContext(&ScriptModule.script_ctxt, R_GAME_SCRIPTFILE, 0);
	if (result != R_SUCCESS) {
		warning("Couldn't get script file context");
		return R_FAILURE;
	}

	// Load script LUT resource
	result = GAME_GetFileContext(&s_lut_ctxt, R_GAME_RESOURCEFILE, 0);
	if (result != R_SUCCESS) {
		warning("Couldn't get resource file context");
		return R_FAILURE;
	}

	result = RSC_LoadResource(s_lut_ctxt, ITE_SCRIPT_LUT, &rsc_ptr, &rsc_len);
	if (result != R_SUCCESS) {
		warning("Error: Couldn't load script resource look-up table");
		return R_FAILURE;
	}

	// Create logical script LUT from resource
	if (rsc_len % R_S_LUT_ENTRYLEN_ITECD == 0) {
		ScriptModule.script_lut_entrylen = R_S_LUT_ENTRYLEN_ITECD;
	} else if (rsc_len % R_S_LUT_ENTRYLEN_ITEDISK == 0) {
		ScriptModule.script_lut_entrylen = R_S_LUT_ENTRYLEN_ITEDISK;
	} else {
		warning("Error: Invalid script lookup table length");
		return R_FAILURE;
	}

	// Calculate number of entries
	ScriptModule.script_lut_max = rsc_len / ScriptModule.script_lut_entrylen;

	// Allocate space for logical LUT
	ScriptModule.script_lut = (R_SCRIPT_LUT_ENTRY *)malloc(ScriptModule.script_lut_max * sizeof(R_SCRIPT_LUT_ENTRY));
	if (ScriptModule.script_lut == NULL) {
		warning("Error: Couldn't allocate memory for script resource look-up table");
		return R_MEM;
	}

	// Convert LUT resource to logical LUT
	MemoryReadStream *readS = new MemoryReadStream(rsc_ptr, rsc_len);
	for (i = 0; i < ScriptModule.script_lut_max; i++) {
		prevTell = readS->pos();
		ScriptModule.script_lut[i].script_rn = readS->readUint16LE();
		ScriptModule.script_lut[i].diag_list_rn = readS->readUint16LE();
		ScriptModule.script_lut[i].voice_lut_rn = readS->readUint16LE();
		// Skip the unused portion of the structure
		for (j = readS->pos(); j < prevTell + ScriptModule.script_lut_entrylen; j++)
			readS->readByte();
	}

	RSC_FreeResource(rsc_ptr);

	// Any voice lookup table resources present?
	for (i = 0; i < ScriptModule.script_lut_max; i++) {
		if (ScriptModule.script_lut[i].voice_lut_rn) {
			ScriptModule.voice_lut_present = 1;
			break;
		}
	}

	// Initialize script submodules
	ScriptModule.thread_list = ys_dll_create();

	if (SDATA_Init() != R_SUCCESS) {
		free(ScriptModule.script_lut);
		return R_FAILURE;
	}

	ScriptModule.initialized = 1;
	return R_SUCCESS;
}

// Shut down script module gracefully; free all allocated module resources
int SCRIPT_Shutdown() {
	YS_DL_NODE *thread_node;
	R_SCRIPT_THREAD *thread;

	if (!ScriptModule.initialized) {
		return R_FAILURE;
	}

	debug(0, "Shutting down scripting subsystem.");

	// Free script lookup table
	free(ScriptModule.script_lut);

	// Stop all threads and destroy them

	for (thread_node = ys_dll_head(ScriptModule.thread_list); thread_node != NULL;
				thread_node = ys_dll_next(thread_node)) {
		thread = (R_SCRIPT_THREAD *)ys_dll_get_data(thread_node);
		STHREAD_Destroy(thread);
	}

	ScriptModule.initialized = 0;

	return R_SUCCESS;
}

// Loads a script; including script bytecode and dialogue list 
int SCRIPT_Load(int script_num) {
	R_SCRIPTDATA *script_data;
	byte *bytecode_p;
	size_t bytecode_len;
	uint32 scriptl_rn;
	byte *diagl_p;
	size_t diagl_len;
	uint32 diagl_rn;
	byte *voicelut_p;
	size_t voicelut_len;
	uint32 voicelut_rn;
	int result;

	if (GAME_GetGameType() == R_GAMETYPE_IHNM) {
		return R_SUCCESS;
	}

	// Validate script number
	if ((script_num < 0) || (script_num > ScriptModule.script_lut_max)) {
		warning("SCRIPT_Load(): Invalid script number");
		return R_FAILURE;
	}

	// Release old script data if present
	SCRIPT_Free();

	// Initialize script data structure
	debug(0, "Loading script data for script #%d", script_num);

	script_data = (R_SCRIPTDATA *)malloc(sizeof *script_data);
	if (script_data == NULL) {
		warning("Memory allocation failed");
		return R_MEM;
	}

	script_data->loaded = 0;

	// Initialize script pointers
	script_data->diag = NULL;
	script_data->bytecode = NULL;
	script_data->voice = NULL;

	// Load script bytecode
	scriptl_rn = ScriptModule.script_lut[script_num].script_rn;

	result = RSC_LoadResource(ScriptModule.script_ctxt, scriptl_rn, &bytecode_p, &bytecode_len);
	if (result != R_SUCCESS) {
		warning("Error loading script bytecode resource");
		free(script_data);
		return R_FAILURE;
	}

	script_data->bytecode = SCRIPT_LoadBytecode(bytecode_p, bytecode_len);

	if (script_data->bytecode == NULL) {
		warning("Error interpreting script bytecode resource");
		free(script_data);
		RSC_FreeResource(bytecode_p);
		return R_FAILURE;
	}

	// Load script dialogue list
	diagl_rn = ScriptModule.script_lut[script_num].diag_list_rn;

	// Load dialogue list resource
	result = RSC_LoadResource(ScriptModule.script_ctxt, diagl_rn, &diagl_p, &diagl_len);
	if (result != R_SUCCESS) {
		warning("Error loading dialogue list resource");
		free(script_data);
		RSC_FreeResource(bytecode_p);
		return R_FAILURE;
	}

	// Convert dialogue list resource to logical dialogue list
	script_data->diag = SCRIPT_LoadDialogue(diagl_p, diagl_len);
	if (script_data->diag == NULL) {
		warning("Error interpreting dialogue list resource");
		free(script_data);
		RSC_FreeResource(bytecode_p);
		RSC_FreeResource(diagl_p);
		return R_FAILURE;
	}

	// Load voice resource lookup table
	if (ScriptModule.voice_lut_present) {
		voicelut_rn = ScriptModule.script_lut[script_num].voice_lut_rn;

		// Load voice LUT resource
		result = RSC_LoadResource(ScriptModule.script_ctxt, voicelut_rn, &voicelut_p, &voicelut_len);
		if (result != R_SUCCESS) {
			warning("Error loading voice LUT resource");
			free(script_data);
			RSC_FreeResource(bytecode_p);
			RSC_FreeResource(diagl_p);
			return R_FAILURE;
		}

		// Convert voice LUT resource to logical voice LUT
		script_data->voice = SCRIPT_LoadVoiceLUT(voicelut_p, voicelut_len, script_data);
		if (script_data->voice == NULL) {
			warning("Error interpreting voice LUT resource");
			free(script_data);
			RSC_FreeResource(bytecode_p);
			RSC_FreeResource(diagl_p);
			RSC_FreeResource(voicelut_p);
			return R_FAILURE;
		}
	}

	// Finish initialization
	script_data->loaded = 1;
	ScriptModule.current_script = script_data;

	return R_SUCCESS;
}

// Frees all resources associated with current script.
int SCRIPT_Free() {
	if (ScriptModule.current_script == NULL) {
		return R_FAILURE;
	}

	if (!ScriptModule.current_script->loaded) {
		return R_FAILURE;
	}

	debug(0, "Releasing script data.");

	// Finish initialization
	if (ScriptModule.current_script->diag != NULL) {
		free(ScriptModule.current_script->diag->str);
		free(ScriptModule.current_script->diag->str_off);
	}
	free(ScriptModule.current_script->diag);

	if (ScriptModule.current_script->bytecode != NULL) {
		free(ScriptModule.current_script->bytecode->entrypoints);
		RSC_FreeResource(ScriptModule.current_script->bytecode->bytecode_p);
	}

	free(ScriptModule.current_script->bytecode);

	if (ScriptModule.voice_lut_present) {
		free(ScriptModule.current_script->voice->voices);
		free(ScriptModule.current_script->voice);
	}

	free(ScriptModule.current_script);

	ScriptModule.current_script = NULL;

	return R_SUCCESS;
}

// Reads the entrypoint table from a script bytecode resource in memory. 
// Returns NULL on failure.
R_SCRIPT_BYTECODE *SCRIPT_LoadBytecode(byte *bytecode_p, size_t bytecode_len) {
	R_PROC_TBLENTRY *bc_ep_tbl = NULL;
	R_SCRIPT_BYTECODE *bc_new_data = NULL;

	unsigned long n_entrypoints; // Number of entrypoints
	size_t ep_tbl_offset; // Offset of bytecode entrypoint table
	unsigned long i;

	debug(0, "Loading script bytecode...");

	MemoryReadStream *readS = new MemoryReadStream(bytecode_p, bytecode_len);

	// The first two uint32 values are the number of entrypoints, and the
	// offset to the entrypoint table, respectively.
	n_entrypoints = readS->readUint32LE();
	ep_tbl_offset = readS->readUint32LE();

	// Check that the entrypoint table offset is valid.
	if ((bytecode_len - ep_tbl_offset) < (n_entrypoints * R_SCRIPT_TBLENTRY_LEN)) {
		warning("Invalid table offset");
		return NULL;
	}

	if (n_entrypoints > R_SCRIPT_MAX) {
		warning("Script limit exceeded");
		return NULL;
	}

	// Allocate a new bytecode resource information structure and table of
	// entrypoints

	bc_new_data = (R_SCRIPT_BYTECODE *)malloc(sizeof *bc_new_data);
	if (bc_new_data == NULL) {
		warning("Memory allocation failure loading script bytecode");
		return NULL;
	}

	bc_ep_tbl = (R_PROC_TBLENTRY *)malloc(n_entrypoints * sizeof *bc_ep_tbl);
	if (bc_ep_tbl == NULL) {
		warning("Memory allocation failure creating script entrypoint table");
		free(bc_new_data);
		return NULL;
	}

	// Read in the entrypoint table

	while (readS->pos() < ep_tbl_offset)
		readS->readByte();

	for (i = 0; i < n_entrypoints; i++) {
		// First uint16 is the offset of the entrypoint name from the start
		// of the bytecode resource, second uint16 is the offset of the 
		// bytecode itself for said entrypoint
		bc_ep_tbl[i].name_offset = readS->readUint16LE();
		bc_ep_tbl[i].offset = readS->readUint16LE();

		// Perform a simple range check on offset values
		if ((bc_ep_tbl[i].name_offset > bytecode_len) || (bc_ep_tbl[i].offset > bytecode_len)) {
			warning("Invalid offset encountered in script entrypoint table");
			free(bc_new_data);
			free(bc_ep_tbl);
			return NULL;
		}
	}

	bc_new_data->bytecode_p = (byte *) bytecode_p;
	bc_new_data->bytecode_len = bytecode_len;

	bc_new_data->n_entrypoints = n_entrypoints;
	bc_new_data->entrypoints = bc_ep_tbl;
	bc_new_data->ep_tbl_offset = ep_tbl_offset;

	return bc_new_data;
}

// Reads a logical dialogue list from a dialogue list resource in memory.
// Returns NULL on failure.
R_DIALOGUE_LIST *SCRIPT_LoadDialogue(const byte *dialogue_p, size_t dialogue_len) {
	R_DIALOGUE_LIST *dialogue_list;
	uint16 n_dialogue;
	uint16 i;
	size_t offset;

	debug(0, "Loading dialogue list...");

	// Allocate dialogue list structure
	dialogue_list = (R_DIALOGUE_LIST *)malloc(sizeof *dialogue_list);
	if (dialogue_list == NULL) {
		return NULL;
	}

	MemoryReadStream *readS = new MemoryReadStream(dialogue_p, dialogue_len);

	// First uint16 is the offset of the first string
	offset = readS->readUint16LE();
	if (offset > dialogue_len) {
		warning("Error, invalid string offset");
		return NULL;
	}

	// Calculate table length
	n_dialogue = offset / 2;
	dialogue_list->n_dialogue = n_dialogue;

	// Allocate table of string pointers
	dialogue_list->str = (const char **)malloc(n_dialogue * sizeof(const char *));
	if (dialogue_list->str == NULL) {
		free(dialogue_list);
		return NULL;
	}

	// Allocate table of string offsets
	dialogue_list->str_off = (size_t *)malloc(n_dialogue * sizeof(size_t));
	if (dialogue_list->str_off == NULL) {
		free(dialogue_list->str);
		free(dialogue_list);
		return NULL;
	}

	// Read in tables from dialogue list resource
	readS->seek(0);
	for (i = 0; i < n_dialogue; i++) {
		offset = readS->readUint16LE();
		if (offset > dialogue_len) {
			warning("Error, invalid string offset");
			free(dialogue_list->str);
			free(dialogue_list->str_off);
			free(dialogue_list);
			return NULL;
		}
		dialogue_list->str[i] = (const char *)dialogue_p + offset;
		dialogue_list->str_off[i] = offset;
	}

	return dialogue_list;
}

// Reads a logical voice LUT from a voice LUT resource in memory.
// Returns NULL on failure.
R_VOICE_LUT *SCRIPT_LoadVoiceLUT(const byte *voicelut_p, size_t voicelut_len, R_SCRIPTDATA *script) {
	R_VOICE_LUT *voice_lut;

	uint16 n_voices;
	uint16 i;

	voice_lut = (R_VOICE_LUT *)malloc(sizeof *voice_lut);
	if (voice_lut == NULL) {
		return NULL;
	}

	n_voices = voicelut_len / 2;
	if (n_voices != script->diag->n_dialogue) {
		warning("Error: Voice LUT entries do not match dialogue entries");
		return NULL;
	}

	voice_lut->voices = (int *)malloc(n_voices * sizeof *voice_lut->voices);
	if (voice_lut->voices == NULL) {

		return NULL;
	}

	MemoryReadStream *readS = new MemoryReadStream(voicelut_p, voicelut_len);

	for (i = 0; i < n_voices; i++) {
		voice_lut->voices[i] = readS->readUint16LE();
	}

	return voice_lut;
}

void CF_script_info(int argc, char *argv[], void *refCon) {
	uint32 n_entrypoints;
	uint32 i;
	char *name_ptr;

	if (ScriptModule.current_script == NULL) {
		return;
	}

	if (!ScriptModule.current_script->loaded) {
		return;
	}

	n_entrypoints = ScriptModule.current_script->bytecode->n_entrypoints;

	CON_Print("Current script contains %d entrypoints:", n_entrypoints);

	for (i = 0; i < n_entrypoints; i++) {
		name_ptr = (char *)ScriptModule.current_script->bytecode->bytecode_p +
							ScriptModule.current_script->bytecode->entrypoints[i].name_offset;
		CON_Print("%lu: %s", i, name_ptr);
	}
}

void CF_script_exec(int argc, char *argv[], void *refCon) {
	uint16 ep_num;

	if (argc < 1) {
		return;
	}

	ep_num = atoi(argv[0]);

	if (ScriptModule.dbg_thread == NULL) {
		CON_Print("Creating debug thread...");
		ScriptModule.dbg_thread = STHREAD_Create();
		if (ScriptModule.dbg_thread == NULL) {
			CON_Print("Thread creation failed.");
			return;
		}
	}

	if (ep_num >= ScriptModule.current_script->bytecode->n_entrypoints) {
		CON_Print("Invalid entrypoint.");
		return;
	}

	STHREAD_Execute(ScriptModule.dbg_thread, ep_num);
}

void CF_script_togglestep(int argc, char *argv[], void *refCon) {
	ScriptModule.dbg_singlestep = !ScriptModule.dbg_singlestep;
}

} // End of namespace Saga
