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
#include "saga/saga.h"
#include "saga/yslib.h"

#include "saga/gfx.h"
#include "saga/rscfile_mod.h"
#include "saga/game_mod.h"
#include "saga/console.h"
#include "saga/cvar_mod.h"

#include "saga/script.h"

namespace Saga {

static void CF_script_info(int argc, char *argv[], void *refCon); 
static void CF_script_exec(int argc, char *argv[], void *refCon);
static void CF_script_togglestep(int argc, char *argv[], void *refCon);

int Script::reg() {
	CVAR_RegisterFunc(CF_script_info, "script_info", NULL, CVAR_NONE, 0, 0, this);
	CVAR_RegisterFunc(CF_script_exec, "script_exec", "<Script number>", CVAR_NONE, 1, 1, this);
	CVAR_RegisterFunc(CF_script_togglestep, "script_togglestep", NULL, CVAR_NONE, 0, 0, this);

	return SUCCESS;
}

// Initializes the scripting module.
// Loads script resource look-up table, initializes script data system
Script::Script() {
	GAME_RESOURCEDESC gr_desc;
	RSCFILE_CONTEXT *s_lut_ctxt;
	byte *rsc_ptr;
	size_t rsc_len;
	int prevTell;
	int result;
	int i, j;

	//initialize member variables
	_dbg_thread = 0;
	_scriptContext = 0;
	_voiceLUTPresent = false;
	_scriptLUTEntryLen = 0;
	_currentScript = 0;
	_threadList = 0;
	_abortEnabled = true;
	_skipSpeeches = false;
	memset(_dataBuf, 0, sizeof(_dataBuf));

	GAME_GetResourceInfo(&gr_desc);
	
	debug(0, "Initializing scripting subsystem");
	// Load script resource file context
	_scriptContext = GAME_GetFileContext(GAME_SCRIPTFILE, 0);
	if (_scriptContext == NULL) {
		error("Couldn't get script file context");
	}

	// Load script LUT resource
	s_lut_ctxt = GAME_GetFileContext(GAME_RESOURCEFILE, 0);
	if (s_lut_ctxt == NULL) {
		error("Couldn't get resource file context");
	}

	debug(0, "Loading script LUT from resource %u.", gr_desc.script_lut_rn);
	result = RSC_LoadResource(s_lut_ctxt, gr_desc.script_lut_rn, &rsc_ptr, &rsc_len);
	if (result != SUCCESS) {
		error("Error: Couldn't load script resource look-up table");
	}

	// Create logical script LUT from resource
	if (rsc_len % S_LUT_ENTRYLEN_ITECD == 0) {
		_scriptLUTEntryLen = S_LUT_ENTRYLEN_ITECD;
	} else if (rsc_len % S_LUT_ENTRYLEN_ITEDISK == 0) {
		_scriptLUTEntryLen = S_LUT_ENTRYLEN_ITEDISK;
	} else {
		error("Error: Invalid script lookup table length");
	}

	// Calculate number of entries
	_scriptLUTMax = rsc_len / _scriptLUTEntryLen;

	debug(0, "LUT has %d entries.", _scriptLUTMax);

	// Allocate space for logical LUT
	_scriptLUT = (SCRIPT_LUT_ENTRY *)malloc(_scriptLUTMax * sizeof(SCRIPT_LUT_ENTRY));
	if (_scriptLUT == NULL) {
		error("Error: Couldn't allocate memory for script resource look-up table");
	}

	// Convert LUT resource to logical LUT
	MemoryReadStream scriptS(rsc_ptr, rsc_len);
	for (i = 0; i < _scriptLUTMax; i++) {
		prevTell = scriptS.pos();
		_scriptLUT[i].script_rn = scriptS.readUint16LE();
		_scriptLUT[i].diag_list_rn = scriptS.readUint16LE();
		_scriptLUT[i].voice_lut_rn = scriptS.readUint16LE();
		
		// Skip the unused portion of the structure
		for (j = scriptS.pos(); j < prevTell + _scriptLUTEntryLen; j++) {
			if (scriptS.readByte() != 0)
				warning("Unused scriptLUT part isn't really unused for LUT %d (pos: %d)", i, j);
		}
	}

	RSC_FreeResource(rsc_ptr);

	// Any voice lookup table resources present?
	for (i = 0; i < _scriptLUTMax; i++) {
		if (_scriptLUT[i].voice_lut_rn) {
			_voiceLUTPresent = true;
			break;
		}
	}

	// Initialize script submodules
	_threadList = ys_dll_create();

	setupScriptFuncList();

	_initialized = true;
}

// Shut down script module gracefully; free all allocated module resources
Script::~Script() {
	YS_DL_NODE *thread_node;
	SCRIPT_THREAD *thread;

	if (!_initialized) {
		error("Script not initialized");
	}

	debug(0, "Shutting down scripting subsystem.");

	// Free script lookup table
	free(_scriptLUT);

	// Stop all threads and destroy them

	for (thread_node = ys_dll_head(_threadList); thread_node != NULL;
				thread_node = ys_dll_next(thread_node)) {
		thread = (SCRIPT_THREAD *)ys_dll_get_data(thread_node);
		SThreadDestroy(thread);
	}

	_initialized = false;
}

// Loads a script; including script bytecode and dialogue list 
int Script::loadScript(int script_num) {
	SCRIPTDATA *script_data;
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

	if (GAME_GetGameType() == GID_IHNM) {
		return SUCCESS;
	}

	// Validate script number
	if ((script_num < 0) || (script_num > _scriptLUTMax)) {
		warning("Script::loadScript(): Invalid script number");
		return FAILURE;
	}

	// Release old script data if present
	freeScript();

	// Initialize script data structure
	debug(0, "Loading script data for script #%d", script_num);

	script_data = (SCRIPTDATA *)malloc(sizeof *script_data);
	if (script_data == NULL) {
		warning("Memory allocation failed");
		return MEM;
	}

	script_data->loaded = 0;

	// Initialize script pointers
	script_data->diag = NULL;
	script_data->bytecode = NULL;
	script_data->voice = NULL;

	// Load script bytecode
	scriptl_rn = _scriptLUT[script_num].script_rn;

	result = RSC_LoadResource(_scriptContext, scriptl_rn, &bytecode_p, &bytecode_len);
	if (result != SUCCESS) {
		warning("Error loading script bytecode resource");
		free(script_data);
		return FAILURE;
	}

	script_data->bytecode = loadBytecode(bytecode_p, bytecode_len);

	if (script_data->bytecode == NULL) {
		warning("Error interpreting script bytecode resource");
		free(script_data);
		RSC_FreeResource(bytecode_p);
		return FAILURE;
	}

	// Load script dialogue list
	diagl_rn = _scriptLUT[script_num].diag_list_rn;

	// Load dialogue list resource
	result = RSC_LoadResource(_scriptContext, diagl_rn, &diagl_p, &diagl_len);
	if (result != SUCCESS) {
		warning("Error loading dialogue list resource");
		free(script_data);
		RSC_FreeResource(bytecode_p);
		return FAILURE;
	}

	// Convert dialogue list resource to logical dialogue list
	script_data->diag = loadDialogue(diagl_p, diagl_len);
	if (script_data->diag == NULL) {
		warning("Error interpreting dialogue list resource");
		free(script_data);
		RSC_FreeResource(bytecode_p);
		RSC_FreeResource(diagl_p);
		return FAILURE;
	}

	// Load voice resource lookup table
	if (_voiceLUTPresent) {
		voicelut_rn = _scriptLUT[script_num].voice_lut_rn;

		// Load voice LUT resource
		result = RSC_LoadResource(_scriptContext, voicelut_rn, &voicelut_p, &voicelut_len);
		if (result != SUCCESS) {
			warning("Error loading voice LUT resource");
			free(script_data);
			RSC_FreeResource(bytecode_p);
			RSC_FreeResource(diagl_p);
			return FAILURE;
		}

		// Convert voice LUT resource to logical voice LUT
		script_data->voice = loadVoiceLUT(voicelut_p, voicelut_len, script_data);
		if (script_data->voice == NULL) {
			warning("Error interpreting voice LUT resource");
			free(script_data);
			RSC_FreeResource(bytecode_p);
			RSC_FreeResource(diagl_p);
			RSC_FreeResource(voicelut_p);
			return FAILURE;
		}
	}

	// Finish initialization
	script_data->loaded = 1;
	_currentScript = script_data;

	return SUCCESS;
}

// Frees all resources associated with current script.
int Script::freeScript() {
	if (_currentScript == NULL) {
		return FAILURE;
	}

	if (!_currentScript->loaded) {
		return FAILURE;
	}

	debug(0, "Releasing script data.");

	// Finish initialization
	if (_currentScript->diag != NULL) {
		free(_currentScript->diag->str);
		free(_currentScript->diag->str_off);
	}
	free(_currentScript->diag);

	if (_currentScript->bytecode != NULL) {
		free(_currentScript->bytecode->entrypoints);
		RSC_FreeResource(_currentScript->bytecode->bytecode_p);
	}

	free(_currentScript->bytecode);

	if (_voiceLUTPresent) {
		free(_currentScript->voice->voices);
		free(_currentScript->voice);
	}

	free(_currentScript);

	_currentScript = NULL;

	return SUCCESS;
}

// Reads the entrypoint table from a script bytecode resource in memory. 
// Returns NULL on failure.
SCRIPT_BYTECODE *Script::loadBytecode(byte *bytecode_p, size_t bytecode_len) {
	PROC_TBLENTRY *bc_ep_tbl = NULL;
	SCRIPT_BYTECODE *bc_new_data = NULL;

	unsigned long n_entrypoints; // Number of entrypoints
	size_t ep_tbl_offset; // Offset of bytecode entrypoint table
	unsigned long i;

	debug(0, "Loading script bytecode...");

	MemoryReadStream scriptS(bytecode_p, bytecode_len);

	// The first two uint32 values are the number of entrypoints, and the
	// offset to the entrypoint table, respectively.
	n_entrypoints = scriptS.readUint32LE();
	ep_tbl_offset = scriptS.readUint32LE();

	// Check that the entrypoint table offset is valid.
	if ((bytecode_len - ep_tbl_offset) < (n_entrypoints * SCRIPT_TBLENTRY_LEN)) {
		warning("Invalid table offset");
		return NULL;
	}

	if (n_entrypoints > SCRIPT_MAX) {
		warning("Script limit exceeded");
		return NULL;
	}

	// Allocate a new bytecode resource information structure and table of
	// entrypoints

	bc_new_data = (SCRIPT_BYTECODE *)malloc(sizeof *bc_new_data);
	if (bc_new_data == NULL) {
		warning("Memory allocation failure loading script bytecode");
		return NULL;
	}

	bc_ep_tbl = (PROC_TBLENTRY *)malloc(n_entrypoints * sizeof *bc_ep_tbl);
	if (bc_ep_tbl == NULL) {
		warning("Memory allocation failure creating script entrypoint table");
		free(bc_new_data);
		return NULL;
	}

	// Read in the entrypoint table

	while (scriptS.pos() < ep_tbl_offset)
		scriptS.readByte();

	for (i = 0; i < n_entrypoints; i++) {
		// First uint16 is the offset of the entrypoint name from the start
		// of the bytecode resource, second uint16 is the offset of the 
		// bytecode itself for said entrypoint
		bc_ep_tbl[i].name_offset = scriptS.readUint16LE();
		bc_ep_tbl[i].offset = scriptS.readUint16LE();

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
DIALOGUE_LIST *Script::loadDialogue(const byte *dialogue_p, size_t dialogue_len) {
	DIALOGUE_LIST *dialogue_list;
	uint16 n_dialogue;
	uint16 i;
	size_t offset;

	debug(0, "Loading dialogue list...");

	// Allocate dialogue list structure
	dialogue_list = (DIALOGUE_LIST *)malloc(sizeof *dialogue_list);
	if (dialogue_list == NULL) {
		return NULL;
	}

	MemoryReadStream scriptS(dialogue_p, dialogue_len);

	// First uint16 is the offset of the first string
	offset = scriptS.readUint16LE();
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
	scriptS.seek(0);
	for (i = 0; i < n_dialogue; i++) {
		offset = scriptS.readUint16LE();
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
VOICE_LUT *Script::loadVoiceLUT(const byte *voicelut_p, size_t voicelut_len, SCRIPTDATA *script) {
	VOICE_LUT *voice_lut;

	uint16 n_voices;
	uint16 i;

	voice_lut = (VOICE_LUT *)malloc(sizeof *voice_lut);
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

	MemoryReadStream scriptS(voicelut_p, voicelut_len);

	for (i = 0; i < n_voices; i++) {
		voice_lut->voices[i] = scriptS.readUint16LE();
	}

	return voice_lut;
}

void Script::scriptInfo(int argc, char *argv[]) {
	uint32 n_entrypoints;
	uint32 i;
	char *name_ptr;

	if (currentScript() == NULL) {
		return;
	}

	if (!currentScript()->loaded) {
		return;
	}

	n_entrypoints = currentScript()->bytecode->n_entrypoints;

	_vm->_console->print("Current script contains %d entrypoints:", n_entrypoints);

	for (i = 0; i < n_entrypoints; i++) {
		name_ptr = (char *)currentScript()->bytecode->bytecode_p +
							currentScript()->bytecode->entrypoints[i].name_offset;
		_vm->_console->print("%lu: %s", i, name_ptr);
	}
}

void Script::scriptExec(int argc, char *argv[]) {
	uint16 ep_num;

	if (argc < 1) {
		return;
	}

	ep_num = atoi(argv[0]);

	if (_dbg_thread == NULL) {
		_vm->_console->print("Creating debug thread...");
		_dbg_thread = SThreadCreate();
		if (_dbg_thread == NULL) {
			_vm->_console->print("Thread creation failed.");
			return;
		}
	}

	if (ep_num >= currentScript()->bytecode->n_entrypoints) {
		_vm->_console->print("Invalid entrypoint.");
		return;
	}

	SThreadExecute(_dbg_thread, ep_num);
}

void CF_script_info(int argc, char *argv[], void *refCon) {
	((Script *)refCon)->scriptInfo(argc, argv);
}

void CF_script_exec(int argc, char *argv[], void *refCon) {
	((Script *)refCon)->scriptExec(argc, argv);
}

void CF_script_togglestep(int argc, char *argv[], void *refCon) {
	((Script *)refCon)->_dbg_singlestep = !((Script *)refCon)->_dbg_singlestep;
}

} // End of namespace Saga
