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

// Scripting module: Script resource handling functions
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/rscfile_mod.h"
#include "saga/console.h"

#include "saga/script.h"
#include "saga/stream.h"
#include "saga/interface.h"

namespace Saga {

// Initializes the scripting module.
// Loads script resource look-up table, initializes script data system
Script::Script() {
	RSCFILE_CONTEXT *s_lut_ctxt;
	RSCFILE_CONTEXT *resourceContext;
	byte *rsc_ptr;
	size_t rsc_len;
	int prevTell;
	int result;
	int i, j;
	byte *stringsPointer;
	size_t stringsLength;

	//initialize member variables
	_dbg_thread = 0;
	_dbg_singlestep = 0;
	_scriptContext = 0;
	_voiceLUTPresent = false;
	_scriptLUTEntryLen = 0;
	_currentScript = 0;
	_abortEnabled = true;
	_skipSpeeches = false;

	_currentVerb = kVerbNone;
	_stickyVerb = kVerbWalkTo;
	_leftButtonVerb = kVerbNone;
	_rightButtonVerb = kVerbNone;

	_dataBuf[0].data = _dataBuf[1].data = (ScriptDataWord *)calloc(SCRIPT_DATABUF_LEN, sizeof(ScriptDataWord));;
	_dataBuf[0].length = _dataBuf[1].length = SCRIPT_DATABUF_LEN;

	for (i = 2; i < SCRIPT_DATABUF_NUM; i++) {
		_dataBuf[i].length = 0;
		_dataBuf[i].data = NULL;
	}

	
	debug(0, "Initializing scripting subsystem");
	// Load script resource file context
	_scriptContext = _vm->getFileContext(GAME_SCRIPTFILE, 0);
	if (_scriptContext == NULL) {
		error("Couldn't get script file context");
	}

	// Load script LUT resource
	s_lut_ctxt = _vm->getFileContext(GAME_RESOURCEFILE, 0);
	if (s_lut_ctxt == NULL) {
		error("Couldn't get resource file context");
	}

	debug(0, "Loading script LUT from resource %u.", _vm->getResourceDescription()->script_lut_rn);
	result = RSC_LoadResource(s_lut_ctxt, _vm->getResourceDescription()->script_lut_rn, &rsc_ptr, &rsc_len);
	if (result != SUCCESS) {
		error("Error: Couldn't load script resource look-up table");
	}

	// Create logical script LUT from resource
	if (rsc_len % S_LUT_ENTRYLEN_ITECD == 0) {
		_scriptLUTEntryLen = S_LUT_ENTRYLEN_ITECD;
	} else if (rsc_len % S_LUT_ENTRYLEN_ITEDISK == 0) {
		_scriptLUTEntryLen = S_LUT_ENTRYLEN_ITEDISK;
	} else {
		error("Error: Invalid script lookup table length (%d)", rsc_len);
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
	MemoryReadStreamEndian scriptS(rsc_ptr, rsc_len, IS_BIG_ENDIAN);
	for (i = 0; i < _scriptLUTMax; i++) {
		prevTell = scriptS.pos();
		_scriptLUT[i].script_rn = scriptS.readUint16();
		_scriptLUT[i].diag_list_rn = scriptS.readUint16();
		_scriptLUT[i].voice_lut_rn = scriptS.readUint16();
		
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


	setupScriptFuncList();

	resourceContext = _vm->getFileContext(GAME_RESOURCEFILE, 0);

	result = RSC_LoadResource(resourceContext, RID_ITE_MAIN_STRINGS, &stringsPointer, &stringsLength); // fixme: IHNM
	if ((result != SUCCESS) || (stringsLength == 0)) {
		error("Error loading strings list resource");
	}

	_vm->loadStrings(_mainStrings, stringsPointer, stringsLength);
	RSC_FreeResource(stringsPointer);

	_initialized = true;
}

// Shut down script module gracefully; free all allocated module resources
Script::~Script() {

	if (!_initialized) {
		error("Script not initialized");
	}

	debug(0, "Shutting down scripting subsystem.");
	
	_mainStrings.freeMem();

	// Free script lookup table
	free(_scriptLUT);
	
	free(_dataBuf[0].data);

	_initialized = false;
}

int Script::getWord(int bufNumber, int wordNumber, ScriptDataWord *data) {
	if ((bufNumber < 0) || (bufNumber >= SCRIPT_DATABUF_NUM)) {
		return FAILURE;
	}

	if ((wordNumber < 0) || (wordNumber >= _dataBuf[bufNumber].length)) {
		return FAILURE;
	}

	if (data == NULL) {
		return FAILURE;
	}

	*data = _dataBuf[bufNumber].data[wordNumber];

	return SUCCESS;
}

int Script::putWord(int bufNumber, int wordNumber, ScriptDataWord data) {
	if ((bufNumber < 0) || (bufNumber >= SCRIPT_DATABUF_NUM)) {
		return FAILURE;
	}

	if ((wordNumber < 0) || (wordNumber >= _dataBuf[bufNumber].length)) {
		return FAILURE;
	}

	_dataBuf[bufNumber].data[wordNumber] = data;

	return SUCCESS;
}

int Script::setBit(int bufNumber, ScriptDataWord bitNumber, int bitState) {
	int wordNumber;
	int bitPos;

	ScriptDataWord bitPattern = 0x01;

	if ((bufNumber < 0) || (bufNumber >= SCRIPT_DATABUF_NUM)) {
		return FAILURE;
	}

	if (bitNumber >= (unsigned long)_dataBuf[bufNumber].length * (sizeof(ScriptDataWord) * CHAR_BIT)) {
		return FAILURE;
	}
	
	wordNumber = bitNumber / (sizeof(ScriptDataWord) * CHAR_BIT);
	bitPos = bitNumber % (sizeof(ScriptDataWord) * CHAR_BIT);

	bitPattern <<= ((sizeof(ScriptDataWord) * CHAR_BIT) - (bitPos + 1));

	if (bitState) {
		_dataBuf[bufNumber].data[wordNumber] |= bitPattern;
	} else {
		_dataBuf[bufNumber].data[wordNumber] &= ~bitPattern;
	}

	return SUCCESS;
}

int Script::getBit(int bufNumber, ScriptDataWord bitNumber, int *bitState) {
	int wordNumber;
	int bitPos;

	ScriptDataWord bitPattern = 0x01;

	if ((bufNumber < 0) || (bufNumber >= SCRIPT_DATABUF_NUM)) {
		return FAILURE;
	}

	if (bitNumber >= (unsigned long)_dataBuf[bufNumber].length * (sizeof(ScriptDataWord) * CHAR_BIT)) {
		return FAILURE;
	}

	wordNumber = bitNumber / (sizeof(ScriptDataWord) * CHAR_BIT);
	bitPos = bitNumber % (sizeof(ScriptDataWord) * CHAR_BIT);

	bitPattern <<= ((sizeof(ScriptDataWord) * CHAR_BIT) - (bitPos + 1));


	*bitState = (_dataBuf[bufNumber].data[wordNumber] & bitPattern) ? 1 : 0;

	return SUCCESS;
}

// Loads a script; including script bytecode and dialogue list 
int Script::loadScript(int script_num) {
	ScriptData *script_data;
	byte *bytecode_p;
	size_t bytecode_len;
	uint32 scriptl_rn;
	byte *stringsPointer;
	size_t stringsLength;
	uint32 stringsResourceId;
	byte *voicelut_p;
	size_t voicelut_len;
	uint32 voicelut_rn;
	int result;

	// Validate script number
	if ((script_num < 0) || (script_num > _scriptLUTMax)) {
		warning("Script::loadScript(): Invalid script number");
		return FAILURE;
	}

	// Release old script data if present
	freeScript();

	// Initialize script data structure
	debug(0, "Loading script data for script #%d", script_num);

	script_data = (ScriptData *)malloc(sizeof(*script_data));
	if (script_data == NULL) {
		error("Memory allocation failed");
	}

	script_data->loaded = 0;

	// Initialize script pointers	
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
		error("Error interpreting script bytecode resource");
	}

	// Load script strings list
	stringsResourceId = _scriptLUT[script_num].diag_list_rn;

	// Load strings list resource
	result = RSC_LoadResource(_scriptContext, stringsResourceId, &stringsPointer, &stringsLength);
	if ((result != SUCCESS) || (stringsLength == 0)) {
		error("Error loading strings list resource");
	}
	
	// Convert strings list resource to logical strings list
	_vm->loadStrings(script_data->strings, stringsPointer, stringsLength);
	RSC_FreeResource(stringsPointer);

	// Load voice resource lookup table
	if (_voiceLUTPresent) {
		voicelut_rn = _scriptLUT[script_num].voice_lut_rn;

		// Load voice LUT resource
		result = RSC_LoadResource(_scriptContext, voicelut_rn, &voicelut_p, &voicelut_len);
		if (result != SUCCESS) {
			error("Error loading voice LUT resource");
		}

		// Convert voice LUT resource to logical voice LUT
		script_data->voice = loadVoiceLUT(voicelut_p, voicelut_len, script_data);
		if (script_data->voice == NULL) {
			error("Error interpreting voice LUT resource");
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
	_currentScript->strings.freeMem();

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

	MemoryReadStreamEndian scriptS(bytecode_p, bytecode_len, IS_BIG_ENDIAN);

	// The first two uint32 values are the number of entrypoints, and the
	// offset to the entrypoint table, respectively.
	n_entrypoints = scriptS.readUint16();
	scriptS.readUint16(); //skip
	ep_tbl_offset = scriptS.readUint16();
	scriptS.readUint16(); //skip

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

	bc_new_data = (SCRIPT_BYTECODE *)malloc(sizeof(*bc_new_data));
	if (bc_new_data == NULL) {
		warning("Memory allocation failure loading script bytecode");
		return NULL;
	}

	bc_ep_tbl = (PROC_TBLENTRY *)malloc(n_entrypoints * sizeof(*bc_ep_tbl));
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
		bc_ep_tbl[i].name_offset = scriptS.readUint16();
		bc_ep_tbl[i].offset = scriptS.readUint16();

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


// Reads a logical voice LUT from a voice LUT resource in memory.
// Returns NULL on failure.
VOICE_LUT *Script::loadVoiceLUT(const byte *voicelut_p, size_t voicelut_len, ScriptData *script) {
	VOICE_LUT *voice_lut;

	uint16 i;

	voice_lut = (VOICE_LUT *)malloc(sizeof(*voice_lut));
	if (voice_lut == NULL) {
		return NULL;
	}

	voice_lut->n_voices = voicelut_len / 2;
	if (voice_lut->n_voices != script->strings.stringsCount) {
		warning("Error: Voice LUT entries do not match dialogue entries");
		return NULL;
	}

	voice_lut->voices = (int *)malloc(voice_lut->n_voices * sizeof(*voice_lut->voices));
	if (voice_lut->voices == NULL) {

		return NULL;
	}

	MemoryReadStreamEndian scriptS(voicelut_p, voicelut_len, IS_BIG_ENDIAN);

	for (i = 0; i < voice_lut->n_voices; i++) {
		voice_lut->voices[i] = scriptS.readUint16();
	}

	return voice_lut;
}

void Script::scriptError(SCRIPT_THREAD *thread, const char *format, ...) {
	char buf[STRINGBUFLEN];
	va_list	argptr;

	va_start(argptr, format);
	vsprintf(buf, format, argptr);
	va_end (argptr);

	thread->flags |= kTFlagAborted;
	debug(0, "Script::scriptError %X: %s", thread->i_offset, buf);
	_vm->_console->DebugPrintf("Script::scriptError %X: %s", thread->i_offset, buf);	
}

void Script::scriptInfo() {
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

	_vm->_console->DebugPrintf("Current script contains %d entrypoints:\n", n_entrypoints);

	for (i = 0; i < n_entrypoints; i++) {
		name_ptr = (char *)currentScript()->bytecode->bytecode_p +
							currentScript()->bytecode->entrypoints[i].name_offset;
		_vm->_console->DebugPrintf("%lu: %s\n", i, name_ptr);
	}
}

void Script::scriptExec(int argc, const char **argv) {
	uint16 ep_num;

	ep_num = atoi(argv[1]);

	if (_dbg_thread == NULL) {
		_vm->_console->DebugPrintf("Creating debug thread...\n");
		_dbg_thread = SThreadCreate();
		if (_dbg_thread == NULL) {
			_vm->_console->DebugPrintf("Thread creation failed.\n");
			return;
		}
	}

	if (ep_num >= currentScript()->bytecode->n_entrypoints) {
		_vm->_console->DebugPrintf("Invalid entrypoint.\n");
		return;
	}

	SThreadExecute(_dbg_thread, ep_num);
}

// verb
void Script::showVerb() {
	const char *verbName;
	const char *object1Name;
	const char *object2Name;
	char statusString[STATUS_TEXT_LEN];


	if (_leftButtonVerb == kVerbNone) {
		_vm->_interface->setStatusText("");
		return;
	}
	
	verbName = _mainStrings.getString(_leftButtonVerb - 1);

	if (objectIdType(_currentObject[0]) == kGameObjectNone) {
		_vm->_interface->setStatusText(verbName);
		return;
	}

	object1Name = _vm->getObjectName(_currentObject[0]);

	if (!_secondObjectNeeded) {
		snprintf(statusString, STATUS_TEXT_LEN, "%s %s", verbName, object1Name);
		_vm->_interface->setStatusText(statusString);
		return;
	}

	
	if (objectIdType(_currentObject[1]) != kGameObjectNone) {
		object2Name = _vm->getObjectName(_currentObject[1]);
	} else {
		object2Name = "";
	}

	if (_leftButtonVerb == kVerbGive) {
		snprintf(statusString, STATUS_TEXT_LEN, "Give %s to %s", object1Name, object2Name);
		_vm->_interface->setStatusText(statusString);
	} else {
		if (_leftButtonVerb == kVerbUse) {
			snprintf(statusString, STATUS_TEXT_LEN, "Use %s with %s", object1Name, object2Name);
			_vm->_interface->setStatusText(statusString);
		} else {
			snprintf(statusString, STATUS_TEXT_LEN, "%s %s", verbName, object1Name);
			_vm->_interface->setStatusText(statusString);
		}
	}
}

void Script::setVerb(int verb) {
	_pendingObject[0] = ID_NOTHING;
	_currentObject[0] = ID_NOTHING;
	_pendingObject[1] = ID_NOTHING;
	_currentObject[1] = ID_NOTHING;
	_firstObjectSet = false;
	_secondObjectNeeded = false;

	setLeftButtonVerb( verb );
	showVerb();
}

void Script::setLeftButtonVerb(int verb) {
}

void Script::doVerb() {
}

// console wrappers
void Script::CF_script_togglestep() {
	_dbg_singlestep = !_dbg_singlestep;
}

} // End of namespace Saga
