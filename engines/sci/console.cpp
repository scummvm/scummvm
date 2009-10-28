/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Console module

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/debug.h"
#include "sci/resource.h"
#include "sci/vocabulary.h"
#include "sci/engine/savegame.h"
#include "sci/engine/state.h"
#include "sci/engine/gc.h"
#include "sci/engine/kernel_types.h"	// for determine_reg_type
#include "sci/gfx/gfx_gui.h"	// for sciw_set_status_bar
#include "sci/gfx/gfx_state_internal.h"
#include "sci/gfx/gfx_widgets.h"	// for getPort
#include "sci/sfx/songlib.h"	// for SongLibrary
#include "sci/sfx/iterator.h"	// for SCI_SONG_ITERATOR_TYPE_SCI0
#include "sci/sfx/sci_midi.h"
#include "sci/vocabulary.h"
#include "sci/gui/gui.h"
#include "sci/gui/gui_cursor.h"

#include "graphics/video/avi_decoder.h"
#include "sci/seq_decoder.h"

#include "common/savefile.h"

namespace Sci {

int g_debug_sleeptime_factor = 1;
int g_debug_simulated_key = 0;
bool g_debug_track_mouse_clicks = false;

// Refer to the "addresses" command on how to pass address parameters
static int parse_reg_t(EngineState *s, const char *str, reg_t *dest);

Console::Console(SciEngine *vm) : GUI::Debugger() {
	_vm = vm;

	// Variables
	DVar_Register("sleeptime_factor",	&g_debug_sleeptime_factor, DVAR_INT, 0);
	DVar_Register("gc_interval",		&script_gc_interval, DVAR_INT, 0);
	DVar_Register("simulated_key",		&g_debug_simulated_key, DVAR_INT, 0);
	DVar_Register("track_mouse_clicks",	&g_debug_track_mouse_clicks, DVAR_BOOL, 0);
	DVar_Register("script_abort_flag",	&script_abort_flag, DVAR_INT, 0);

	// General
	DCmd_Register("help",				WRAP_METHOD(Console, cmdHelp));
	// Kernel
//	DCmd_Register("classes",			WRAP_METHOD(Console, cmdClasses));	// TODO
	DCmd_Register("opcodes",			WRAP_METHOD(Console, cmdOpcodes));
	DCmd_Register("selector",			WRAP_METHOD(Console, cmdSelector));
	DCmd_Register("selectors",			WRAP_METHOD(Console, cmdSelectors));
	DCmd_Register("functions",			WRAP_METHOD(Console, cmdKernelFunctions));
	DCmd_Register("class_table",		WRAP_METHOD(Console, cmdClassTable));
	// Parser
	DCmd_Register("suffixes",			WRAP_METHOD(Console, cmdSuffixes));
	DCmd_Register("parse_grammar",		WRAP_METHOD(Console, cmdParseGrammar));
	DCmd_Register("parser_nodes",		WRAP_METHOD(Console, cmdParserNodes));
	DCmd_Register("parser_words",		WRAP_METHOD(Console, cmdParserWords));
	DCmd_Register("sentence_fragments",	WRAP_METHOD(Console, cmdSentenceFragments));
	DCmd_Register("parse",				WRAP_METHOD(Console, cmdParse));
	DCmd_Register("set_parse_nodes",	WRAP_METHOD(Console, cmdSetParseNodes));
	// Resources
	DCmd_Register("hexdump",			WRAP_METHOD(Console, cmdHexDump));
	DCmd_Register("resource_id",		WRAP_METHOD(Console, cmdResourceId));
	DCmd_Register("resource_size",		WRAP_METHOD(Console, cmdResourceSize));
	DCmd_Register("resource_types",		WRAP_METHOD(Console, cmdResourceTypes));
	DCmd_Register("list",				WRAP_METHOD(Console, cmdList));
	DCmd_Register("hexgrep",			WRAP_METHOD(Console, cmdHexgrep));
	// Game
	DCmd_Register("save_game",			WRAP_METHOD(Console, cmdSaveGame));
	DCmd_Register("restore_game",		WRAP_METHOD(Console, cmdRestoreGame));
	DCmd_Register("restart_game",		WRAP_METHOD(Console, cmdRestartGame));
	DCmd_Register("version",			WRAP_METHOD(Console, cmdGetVersion));
	DCmd_Register("room",				WRAP_METHOD(Console, cmdRoomNumber));
	DCmd_Register("exit",				WRAP_METHOD(Console, cmdExit));
	// Screen
	DCmd_Register("sci0_palette",		WRAP_METHOD(Console, cmdSci0Palette));
	DCmd_Register("clear_screen",		WRAP_METHOD(Console, cmdClearScreen));
	DCmd_Register("redraw_screen",		WRAP_METHOD(Console, cmdRedrawScreen));
	DCmd_Register("fill_screen",		WRAP_METHOD(Console, cmdFillScreen));
	DCmd_Register("show_map",			WRAP_METHOD(Console, cmdShowMap));
	DCmd_Register("update_zone",		WRAP_METHOD(Console, cmdUpdateZone));
	DCmd_Register("propagate_zone",		WRAP_METHOD(Console, cmdPropagateZone));
	DCmd_Register("priority_bands",		WRAP_METHOD(Console, cmdPriorityBands));
	// Graphics
	DCmd_Register("draw_pic",			WRAP_METHOD(Console, cmdDrawPic));
	DCmd_Register("draw_rect",			WRAP_METHOD(Console, cmdDrawRect));
	DCmd_Register("draw_cel",			WRAP_METHOD(Console, cmdDrawCel));
	DCmd_Register("view_info",			WRAP_METHOD(Console, cmdViewInfo));
	DCmd_Register("undither",           WRAP_METHOD(Console, cmdUndither));
	DCmd_Register("play_video",         WRAP_METHOD(Console, cmdPlayVideo));
	// GUI
	DCmd_Register("current_port",		WRAP_METHOD(Console, cmdCurrentPort));
	DCmd_Register("print_port",			WRAP_METHOD(Console, cmdPrintPort));
	DCmd_Register("visual_state",		WRAP_METHOD(Console, cmdVisualState));
	DCmd_Register("flush_visual",		WRAP_METHOD(Console, cmdFlushPorts));
	DCmd_Register("dynamic_views",		WRAP_METHOD(Console, cmdDynamicViews));
	DCmd_Register("dropped_views",		WRAP_METHOD(Console, cmdDroppedViews));
	DCmd_Register("status_bar",			WRAP_METHOD(Console, cmdStatusBarColors));
#ifdef GFXW_DEBUG_WIDGETS
	DCmd_Register("print_widget",		WRAP_METHOD(Console, cmdPrintWidget));
#endif
	// Segments
	DCmd_Register("segment_table",		WRAP_METHOD(Console, cmdPrintSegmentTable));
	DCmd_Register("segtable",			WRAP_METHOD(Console, cmdPrintSegmentTable));	// alias
	DCmd_Register("segment_info",		WRAP_METHOD(Console, cmdSegmentInfo));
	DCmd_Register("seginfo",			WRAP_METHOD(Console, cmdSegmentInfo));			// alias
	DCmd_Register("segment_kill",		WRAP_METHOD(Console, cmdKillSegment));
	DCmd_Register("segkill",			WRAP_METHOD(Console, cmdKillSegment));			// alias
	// Garbage collection
	DCmd_Register("gc",					WRAP_METHOD(Console, cmdGCInvoke));
	DCmd_Register("gc_objects",			WRAP_METHOD(Console, cmdGCObjects));
	DCmd_Register("gc_reachable",		WRAP_METHOD(Console, cmdGCShowReachable));
	DCmd_Register("gc_freeable",		WRAP_METHOD(Console, cmdGCShowFreeable));
	DCmd_Register("gc_normalize",		WRAP_METHOD(Console, cmdGCNormalize));
	// Music/SFX
	DCmd_Register("songlib",			WRAP_METHOD(Console, cmdSongLib));
	DCmd_Register("is_sample",			WRAP_METHOD(Console, cmdIsSample));
	DCmd_Register("sfx01_header",		WRAP_METHOD(Console, cmdSfx01Header));
	DCmd_Register("sfx01_track",		WRAP_METHOD(Console, cmdSfx01Track));
	DCmd_Register("stop_sfx",			WRAP_METHOD(Console, cmdStopSfx));
	// Script
	DCmd_Register("addresses",			WRAP_METHOD(Console, cmdAddresses));
	DCmd_Register("registers",			WRAP_METHOD(Console, cmdRegisters));
	DCmd_Register("dissect_script",		WRAP_METHOD(Console, cmdDissectScript));
	DCmd_Register("set_acc",			WRAP_METHOD(Console, cmdSetAccumulator));
	DCmd_Register("backtrace",			WRAP_METHOD(Console, cmdBacktrace));
	DCmd_Register("bt",					WRAP_METHOD(Console, cmdBacktrace));	// alias
	DCmd_Register("step",				WRAP_METHOD(Console, cmdStep));
	DCmd_Register("s",					WRAP_METHOD(Console, cmdStep));			// alias
	DCmd_Register("step_event",			WRAP_METHOD(Console, cmdStepEvent));
	DCmd_Register("se",					WRAP_METHOD(Console, cmdStepEvent));	// alias
	DCmd_Register("step_ret",			WRAP_METHOD(Console, cmdStepRet));
	DCmd_Register("sret",				WRAP_METHOD(Console, cmdStepRet));		// alias
	DCmd_Register("step_global",		WRAP_METHOD(Console, cmdStepGlobal));
	DCmd_Register("sg",					WRAP_METHOD(Console, cmdStepGlobal));	// alias
	DCmd_Register("step_callk",			WRAP_METHOD(Console, cmdStepCallk));
	DCmd_Register("snk",				WRAP_METHOD(Console, cmdStepCallk));	// alias
	DCmd_Register("disasm",				WRAP_METHOD(Console, cmdDisassemble));
	DCmd_Register("disasm_addr",		WRAP_METHOD(Console, cmdDisassembleAddress));
	DCmd_Register("send",				WRAP_METHOD(Console, cmdSend));
	DCmd_Register("go",					WRAP_METHOD(Console, cmdGo));
	// Breakpoints
	DCmd_Register("bp_list",			WRAP_METHOD(Console, cmdBreakpointList));
	DCmd_Register("bplist",				WRAP_METHOD(Console, cmdBreakpointList));			// alias
	DCmd_Register("bp_del",				WRAP_METHOD(Console, cmdBreakpointDelete));
	DCmd_Register("bpdel",				WRAP_METHOD(Console, cmdBreakpointDelete));			// alias
	DCmd_Register("bp_exec_method",		WRAP_METHOD(Console, cmdBreakpointExecMethod));
	DCmd_Register("bpx",				WRAP_METHOD(Console, cmdBreakpointExecMethod));		// alias
	DCmd_Register("bp_exec_function",	WRAP_METHOD(Console, cmdBreakpointExecFunction));
	DCmd_Register("bpe",				WRAP_METHOD(Console, cmdBreakpointExecFunction));	// alias
	// VM
	DCmd_Register("script_steps",		WRAP_METHOD(Console, cmdScriptSteps));
	DCmd_Register("vm_varlist",			WRAP_METHOD(Console, cmdVMVarlist));
	DCmd_Register("vmvarlist",			WRAP_METHOD(Console, cmdVMVarlist));	// alias
	DCmd_Register("vm_vars",			WRAP_METHOD(Console, cmdVMVars));
	DCmd_Register("vmvars",				WRAP_METHOD(Console, cmdVMVars));		// alias
	DCmd_Register("stack",				WRAP_METHOD(Console, cmdStack));
	DCmd_Register("value_type",			WRAP_METHOD(Console, cmdValueType));
	DCmd_Register("view_listnode",		WRAP_METHOD(Console, cmdViewListNode));
	DCmd_Register("view_reference",		WRAP_METHOD(Console, cmdViewReference));
	DCmd_Register("vr",					WRAP_METHOD(Console, cmdViewReference));	// alias
	DCmd_Register("view_object",		WRAP_METHOD(Console, cmdViewObject));
	DCmd_Register("vo",					WRAP_METHOD(Console, cmdViewObject));		// alias
	DCmd_Register("active_object",		WRAP_METHOD(Console, cmdViewActiveObject));
	DCmd_Register("acc_object",			WRAP_METHOD(Console, cmdViewAccumulatorObject));

	g_debugState.seeking = kDebugSeekNothing;
	g_debugState.seekLevel = 0;
	g_debugState.runningStep = 0;
	g_debugState.stopOnEvent = false;
	g_debugState.debugging = false;
}

Console::~Console() {
}

void Console::preEnter() {
	if (_vm->_gamestate)
		_vm->_gamestate->_sound.sfx_suspend(true);
	_vm->_mixer->pauseAll(true);
}

void Console::postEnter() {
	if (_vm->_gamestate)
		_vm->_gamestate->_sound.sfx_suspend(false);
	_vm->_mixer->pauseAll(false);

	if (!_videoFile.empty()) {
		_vm->_gamestate->_gui->hideCursor();

		if (_videoFile.hasSuffix(".seq")) {
			SeqDecoder *seqDecoder = new SeqDecoder();
			Graphics::VideoPlayer *player = new Graphics::VideoPlayer(seqDecoder);
			if (seqDecoder->loadFile(_videoFile.c_str(), _videoFrameDelay))
				player->playVideo();
			else
				DebugPrintf("Failed to open movie file %s\n", _videoFile.c_str());
			seqDecoder->closeFile();
			delete player;
			delete seqDecoder;
		} else if (_videoFile.hasSuffix(".avi")) {
			Graphics::AviDecoder *aviDecoder = new Graphics::AviDecoder(g_system->getMixer());
			Graphics::VideoPlayer *player = new Graphics::VideoPlayer(aviDecoder);
			if (aviDecoder->loadFile(_videoFile.c_str()))
				player->playVideo();
			else
				DebugPrintf("Failed to open movie file %s\n", _videoFile.c_str());
			aviDecoder->closeFile();
			delete player;
			delete aviDecoder;
		}

		_vm->_gamestate->_gui->showCursor();

		_videoFile.clear();
		_videoFrameDelay = 0;
	}
}


#if 0
// Unused
#define LOOKUP_SPECIES(species) (\
	(species >= 1000) ? species : *(s->_classtable[species].scriptposp) \
		+ s->_classtable[species].class_offset)
#endif

bool Console::cmdHelp(int argc, const char **argv) {
	DebugPrintf("\n");
	DebugPrintf("Variables\n");
	DebugPrintf("---------\n");
	DebugPrintf("sleeptime_factor: Factor to multiply with wait times in kWait()\n");
	DebugPrintf("gc_interval: Number of kernel calls in between garbage collections\n");
	DebugPrintf("simulated_key: Add a key with the specified scan code to the event list\n");
	DebugPrintf("track_mouse_clicks: Toggles mouse click tracking to the console\n");
	DebugPrintf("weak_validations: Turns some validation errors into warnings\n");
	DebugPrintf("script_abort_flag: Set to 1 to abort script execution. Set to 2 to force a replay afterwards\n");
	DebugPrintf("\n");
	DebugPrintf("Debug flags\n");
	DebugPrintf("-----------\n");
	DebugPrintf("debugflag_list - Lists the available debug flags and their status\n");
	DebugPrintf("debugflag_enable - Enables a debug flag\n");
	DebugPrintf("debugflag_disable - Disables a debug flag\n");
	DebugPrintf("\n");
	DebugPrintf("Commands\n");
	DebugPrintf("--------\n");
	DebugPrintf("Kernel:\n");
	DebugPrintf(" opcodes - Lists the opcode names\n");
	DebugPrintf(" selectors - Lists the selector names\n");
	DebugPrintf(" selector - Attempts to find the requested selector by name\n");
	DebugPrintf(" functions - Lists the kernel functions\n");
	DebugPrintf(" class_table - Shows the available classes\n");
	DebugPrintf("\n");
	DebugPrintf("Parser:\n");
	DebugPrintf(" suffixes - Lists the vocabulary suffixes\n");
	DebugPrintf(" parse_grammar - Shows the parse grammar, in strict GNF\n");
	DebugPrintf(" parser_nodes - Shows the specified number of nodes from the parse node tree\n");
	DebugPrintf(" parser_words - Shows the words from the parse node tree\n");
	DebugPrintf(" sentence_fragments - Shows the sentence fragments (used to build Parse trees)\n");
	DebugPrintf(" parse - Parses a sequence of words and prints the resulting parse tree\n");
	DebugPrintf(" set_parse_nodes - Sets the contents of all parse nodes\n");
	DebugPrintf("\n");
	DebugPrintf("Resources:\n");
	DebugPrintf(" hexdump - Dumps the specified resource to standard output\n");
	DebugPrintf(" resource_id - Identifies a resource number by splitting it up in resource type and resource number\n");
	DebugPrintf(" resource_size - Shows the size of a resource\n");
	DebugPrintf(" resource_types - Shows the valid resource types\n");
	DebugPrintf(" list - Lists all the resources of a given type\n");
	DebugPrintf(" hexgrep - Searches some resources for a particular sequence of bytes, represented as hexadecimal numbers\n");
	DebugPrintf("\n");
	DebugPrintf("Game:\n");
	DebugPrintf(" save_game - Saves the current game state to the hard disk\n");
	DebugPrintf(" restore_game - Restores a saved game from the hard disk\n");
	DebugPrintf(" restart_game - Restarts the game\n");
	DebugPrintf(" version - Shows the resource and interpreter versions\n");
	DebugPrintf(" room - Shows the current room number\n");
	DebugPrintf(" exit - Exits the game\n");
	DebugPrintf("\n");
	DebugPrintf("Screen:\n");
	DebugPrintf(" sci0_palette - Sets the SCI0 palette to use (EGA, Amiga or grayscale)\n");
	DebugPrintf(" clear_screen - Clears the screen\n");
	DebugPrintf(" redraw_screen - Redraws the screen\n");
	DebugPrintf(" fill_screen - Fills the screen with one of the EGA colors\n");
	DebugPrintf(" show_map - Shows one of the screen maps (visual, priority or control)\n");
	DebugPrintf(" update_zone - Propagates a rectangular area from the back buffer to the front buffer\n");
	DebugPrintf(" propagate_zone - Propagates a rectangular area from a lower graphics buffer to a higher one\n");
	DebugPrintf(" priority_bands - Shows information about priority bands\n");
	DebugPrintf("\n");
	DebugPrintf("Graphics:\n");
	DebugPrintf(" draw_pic - Draws a pic resource\n");
	DebugPrintf(" draw_rect - Draws a rectangle to the screen with one of the EGA colors\n");
	DebugPrintf(" draw_cel - Draws a single view cel to the center of the screen\n");
	DebugPrintf(" view_info - Displays information for the specified view\n");
	DebugPrintf(" undither - Enable/disable undithering\n");
	DebugPrintf("\n");
	DebugPrintf("GUI:\n");
	DebugPrintf(" current_port - Shows the ID of the currently active port\n");
	DebugPrintf(" print_port - Prints information about a port\n");
	DebugPrintf(" visual_state - Shows the state of the current visual widget\n");
	DebugPrintf(" flush_visual - Flushes dynamically allocated ports (for memory profiling)\n");
	DebugPrintf(" dynamic_views - Lists active dynamic views\n");
	DebugPrintf(" dropped_views - Lists dropped dynamic views\n");
	DebugPrintf(" status_bar - Sets the colors of the status bar\n");
#ifdef GFXW_DEBUG_WIDGETS
	DebugPrintf(" print_widget - Shows active widgets (no params) or information on the specified widget indices\n");
#endif
	DebugPrintf("\n");
	DebugPrintf("Segments:\n");
	DebugPrintf(" segment_table / segtable - Lists all segments\n");
	DebugPrintf(" segment_info / seginfo - Provides information on the specified segment\n");
	DebugPrintf(" segment_kill / segkill - Deletes the specified segment\n");
	DebugPrintf("\n");
	DebugPrintf("Garbage collection:\n");
	DebugPrintf(" gc - Invokes the garbage collector\n");
	DebugPrintf(" gc_objects - Lists all reachable objects, normalized\n");
	DebugPrintf(" gc_reachable - Lists all addresses directly reachable from a given memory object\n");
	DebugPrintf(" gc_freeable - Lists all addresses freeable in a given segment\n");
	DebugPrintf(" gc_normalize - Prints the \"normal\" address of a given address\n");
	DebugPrintf("\n");
	DebugPrintf("Music/SFX:\n");
	DebugPrintf(" songlib - Shows the song library\n");
	DebugPrintf(" is_sample - Shows information on a given sound resource, if it's a PCM sample\n");
	DebugPrintf(" sfx01_header - Dumps the header of a SCI01 song\n");
	DebugPrintf(" sfx01_track - Dumps a track of a SCI01 song\n");
	DebugPrintf(" stop_sfx - Stops a playing sound\n");
	DebugPrintf("\n");
	DebugPrintf("Script:\n");
	DebugPrintf(" addresses - Provides information on how to pass addresses\n");
	DebugPrintf(" registers - Shows the current register values\n");
	DebugPrintf(" dissect_script - Examines a script\n");
	DebugPrintf(" set_acc - Sets the accumulator\n");
	DebugPrintf(" backtrace / bt - Dumps the send/self/super/call/calle/callb stack\n");
	DebugPrintf(" step / s - Executes one operation (no parameters) or several operations (specified as a parameter) \n");
	DebugPrintf(" step_event / se - Steps forward until a SCI event is received.\n");
	DebugPrintf(" step_ret / sret - Steps forward until ret is called on the current execution stack level.\n");
	DebugPrintf(" step_global / sg - Steps until the global variable with the specified index is modified.\n");
	DebugPrintf(" step_callk / snk - Steps forward until it hits the next callk operation, or a specific callk (specified as a parameter)\n");
	DebugPrintf(" disasm - Disassembles a method by name\n");
	DebugPrintf(" disasm_addr - Disassembles one or more commands\n");
	DebugPrintf(" send - Sends a message to an object\n");
	DebugPrintf(" go - Executes the script\n");
	DebugPrintf("\n");
	DebugPrintf("Breakpoints:\n");
	DebugPrintf(" bp_list / bplist - Lists the current breakpoints\n");
	DebugPrintf(" bp_del / bpdel - Deletes a breakpoint with the specified index\n");
	DebugPrintf(" bp_exec_method / bpx - Sets a breakpoint on the execution of the specified method\n");
	DebugPrintf(" bp_exec_function / bpe - Sets a breakpoint on the execution of the specified exported function\n");
	DebugPrintf("\n");
	DebugPrintf("VM:\n");
	DebugPrintf(" script_steps - Shows the number of executed SCI operations\n");
	DebugPrintf(" vm_varlist / vmvarlist - Shows the addresses of variables in the VM\n");
	DebugPrintf(" vm_vars / vmvars - Displays or changes variables in the VM\n");
	DebugPrintf(" stack - Lists the specified number of stack elements\n");
	DebugPrintf(" value_type - Determines the type of a value\n");
	DebugPrintf(" view_listnode - Examines the list node at the given address\n");
	DebugPrintf(" view_reference / vr - Examines an arbitrary reference\n");
	DebugPrintf(" view_object / vo - Examines the object at the given address\n");
	DebugPrintf(" active_object - Shows information on the currently active object or class\n");
	DebugPrintf(" acc_object - Shows information on the object or class at the address indexed by the accumulator\n");
	DebugPrintf("\n");
	return true;
}

ResourceType parseResourceType(const char *resid) {
	// Gets the resource number of a resource string, or returns -1
	ResourceType res = kResourceTypeInvalid;

	for (int i = 0; i < kResourceTypeInvalid; i++)
		if (strcmp(getResourceTypeName((ResourceType)i), resid) == 0)
			res = (ResourceType)i;

	return res;
}

const char *selector_name(EngineState *s, int selector) {
	if (selector >= 0 && selector < (int)s->_kernel->getSelectorNamesSize())
		return s->_kernel->getSelectorName(selector).c_str();
	else
		return "--INVALID--";
}

bool Console::cmdGetVersion(int argc, const char **argv) {
	DebugPrintf("Emulated interpreter version: %s\n", getSciVersionDesc(getSciVersion()).c_str());

	return true;
}

bool Console::cmdOpcodes(int argc, const char **argv) {
	// Load the opcode table from vocab.998 if it exists, to obtain the opcode names
	Resource* r = _vm->getResourceManager()->findResource(ResourceId(kResourceTypeVocab, 998), 0);

	// If the resource couldn't be loaded, leave
	if (!r) {
		DebugPrintf("unable to load vocab.998");
		return true;
	}

	int count = READ_LE_UINT16(r->data);

	DebugPrintf("Opcode names in numeric order [index: type name]:\n");

	for (int i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset) - 2;
		int type = READ_LE_UINT16(r->data + offset + 2);
		// QFG3 has empty opcodes
		Common::String name = len > 0 ? Common::String((char *)r->data + offset + 4, len) : "Dummy";
		DebugPrintf("%03x: %03x %20s | ", i, type, name.c_str());
		if ((i % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdSelector(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Attempts to find the requested selector by name.\n");
		DebugPrintf("Usage: %s <selector name>\n", argv[0]);
		return true;
	}

	for (uint seeker = 0; seeker < _vm->getKernel()->getSelectorNamesSize(); seeker++) {
		if (!scumm_stricmp(_vm->getKernel()->getSelectorName(seeker).c_str(), argv[1])) {
			DebugPrintf("Selector %s found at %03x (%d)\n", _vm->getKernel()->getSelectorName(seeker).c_str(), seeker, seeker);
			return true;
		}
	}

	DebugPrintf("Selector %s wasn't found\n", argv[1]);

	return true;
}

bool Console::cmdSelectors(int argc, const char **argv) {
	DebugPrintf("Selector names in numeric order:\n");
	Common::String selectorName;
	for (uint seeker = 0; seeker < _vm->getKernel()->getSelectorNamesSize(); seeker++) {
		selectorName = _vm->getKernel()->getSelectorName(seeker);
		if (selectorName != "BAD SELECTOR")
			DebugPrintf("%03x: %20s | ", seeker, selectorName.c_str());
		else
			continue;
		if ((seeker % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdKernelFunctions(int argc, const char **argv) {
	DebugPrintf("Kernel function names in numeric order:\n");
	for (uint seeker = 0; seeker <  _vm->getKernel()->getKernelNamesSize(); seeker++) {
		DebugPrintf("%03x: %20s | ", seeker, _vm->getKernel()->getKernelName(seeker).c_str());
		if ((seeker % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdSuffixes(int argc, const char **argv) {
	_vm->getVocabulary()->printSuffixes();

	return true;
}

bool Console::cmdParserWords(int argc, const char **argv) {
	_vm->getVocabulary()->printParserWords();

	return true;
}

bool Console::cmdSetParseNodes(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Sets the contents of all parse nodes.\n");
		DebugPrintf("Usage: %s <parse node1> <parse node2> ... <parse noden>\n", argv[0]);
		DebugPrintf("Tokens should be separated by blanks and enclosed in parentheses\n");
		return true;
	}

	int i = 0;
	int pos = -1;
	int nextToken = 0, nextValue = 0;

	const char *token = argv[i++];

	if (!strcmp(token, "(")) {
		nextToken = kParseOpeningParenthesis;
	} else if (!strcmp(token, ")")) {
		nextToken = kParseClosingParenthesis;
	} else if (!strcmp(token, "nil")) {
		nextToken = kParseNil;
	} else {
		nextValue = strtol(token, NULL, 0);
		nextToken = kParseNumber;
	}

	if (_vm->getVocabulary()->parseNodes(&i, &pos, nextToken, nextValue, argc, argv) == -1)
		return 1;

	_vm->getVocabulary()->dumpParseTree();

	return true;
}

bool Console::cmdRegisters(int argc, const char **argv) {
	DebugPrintf("Current register values:\n");
	DebugPrintf("acc=%04x:%04x prev=%04x:%04x &rest=%x\n", PRINT_REG(_vm->_gamestate->r_acc), PRINT_REG(_vm->_gamestate->r_prev), scriptState.restAdjust);

	if (!_vm->_gamestate->_executionStack.empty()) {
		EngineState *s = _vm->_gamestate;	// for PRINT_STK
		DebugPrintf("pc=%04x:%04x obj=%04x:%04x fp=ST:%04x sp=ST:%04x\n",
					PRINT_REG(scriptState.xs->addr.pc), PRINT_REG(scriptState.xs->objp),
					(unsigned)(scriptState.xs->fp - s->stack_base), (unsigned)(scriptState.xs->sp - s->stack_base));
	} else
		DebugPrintf("<no execution stack: pc,obj,fp omitted>\n");

	return true;
}

bool Console::cmdHexDump(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Dumps the specified resource to standard output\n");
		DebugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		cmdResourceTypes(argc, argv);
		return true;
	}

	int resNum = atoi(argv[2]);
	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _vm->getResourceManager()->findResource(ResourceId(res, resNum), 0);
		if (resource) {
			Common::hexdump(resource->data, resource->size, 16, 0);
			DebugPrintf("Resource %s.%03d has been dumped to standard output\n", argv[1], resNum);
		} else {
			DebugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdResourceId(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Identifies a resource number by splitting it up in resource type and resource number\n");
		DebugPrintf("Usage: %s <resource number>\n", argv[0]);
		return true;
	}

	int id = atoi(argv[1]);
	DebugPrintf("%s.%d (0x%x)\n", getResourceTypeName((ResourceType)(id >> 11)), id & 0x7ff, id & 0x7ff);

	return true;
}

bool Console::cmdDissectScript(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Examines a script\n");
		DebugPrintf("Usage: %s <script number>\n", argv[0]);
		return true;
	}

	_vm->getKernel()->dissectScript(atoi(argv[1]), _vm->getVocabulary());

	return true;
}

bool Console::cmdRoomNumber(int argc, const char **argv) {
	DebugPrintf("Current room number is %d\n", _vm->_gamestate->currentRoomNumber());

	return true;
}

bool Console::cmdResourceSize(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Shows the size of a resource\n");
		DebugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		return true;
	}

	int resNum = atoi(argv[2]);
	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _vm->getResourceManager()->findResource(ResourceId(res, resNum), 0);
		if (resource) {
			DebugPrintf("Resource size: %d\n", resource->size);
		} else {
			DebugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdResourceTypes(int argc, const char **argv) {
	DebugPrintf("The %d valid resource types are:\n", kResourceTypeInvalid);
	for (int i = 0; i < kResourceTypeInvalid; i++) {
		DebugPrintf("%s", getResourceTypeName((ResourceType) i));
		DebugPrintf((i < kResourceTypeInvalid - 1) ? ", " : "\n");
	}

	return true;
}

extern int sci0_palette;

bool Console::cmdSci0Palette(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Sets the SCI0 palette to use - 0: EGA, 1: AGI/Amiga, 2: Grayscale\n");
		return true;
	}

#ifdef INCLUDE_OLDGFX
	sci0_palette = atoi(argv[1]);
	cmdRedrawScreen(argc, argv);
#endif

	return false;
}

bool Console::cmdHexgrep(int argc, const char **argv) {
	if (argc < 4) {
		DebugPrintf("Searches some resources for a particular sequence of bytes, represented as hexadecimal numbers.\n");
		DebugPrintf("Usage: %s <resource type> <resource number> <search string>\n", argv[0]);
		DebugPrintf("<resource number> can be a specific resource number, or \"all\" for all of the resources of the specified type\n", argv[0]);
		DebugPrintf("EXAMPLES:\n  hexgrep script all e8 03 c8 00\n  hexgrep pic 042 fe");
		cmdResourceTypes(argc, argv);
		return true;
	}

	ResourceType restype = parseResourceType(argv[1]);
	int resNumber = 0, resMax = 0;
	char seekString[500];
	Resource *script = NULL;

	if (restype == kResourceTypeInvalid) {
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
		return true;
	}

	if (!scumm_stricmp(argv[2], "all")) {
		resNumber = 0;
		resMax = 999;
	} else {
		resNumber = resMax = atoi(argv[2]);
	}

	strcpy(seekString, argv[3]);

	// Construct the seek string
	for (int i = 4; i < argc; i++) {
		strcat(seekString, argv[i]);
	}

	for (; resNumber <= resMax; resNumber++) {
		if ((script = _vm->getResourceManager()->findResource(ResourceId(restype, resNumber), 0))) {
			unsigned int seeker = 0, seekerold = 0;
			uint32 comppos = 0;
			int output_script_name = 0;

			while (seeker < script->size) {
				if (script->data[seeker] == seekString[comppos]) {
					if (comppos == 0)
						seekerold = seeker;

					comppos++;

					if (comppos == strlen(seekString)) {
						comppos = 0;
						seeker = seekerold + 1;

						if (!output_script_name) {
							DebugPrintf("\nIn %s.%03d:\n", getResourceTypeName((ResourceType)restype), resNumber);
							output_script_name = 1;
						}
						DebugPrintf("   0x%04x\n", seekerold);
					}
				} else
					comppos = 0;

				seeker++;
			}
		}
	}

	return true;
}

bool Console::cmdList(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Lists all the resources of a given type\n");
		cmdResourceTypes(argc, argv);
		return true;
	}


	ResourceType res = parseResourceType(argv[1]);
	if (res == kResourceTypeInvalid)
		DebugPrintf("Unknown resource type: '%s'\n", argv[1]);
	else {
		int number = -1;

		if ((res == kResourceTypeAudio36) || (res == kResourceTypeSync36)) {
			if (argc != 3) {
				DebugPrintf("Please specify map number\n");
				return true;
			}
			number = atoi(argv[2]);
		}

		Common::List<ResourceId> *resources = _vm->getResourceManager()->listResources(res, number);
		sort(resources->begin(), resources->end(), ResourceIdLess());
		Common::List<ResourceId>::iterator itr = resources->begin();

		int cnt = 0;
		while (itr != resources->end()) {
			if (number == -1) {
				DebugPrintf("%8i", itr->number);
				if (++cnt % 10 == 0)
					DebugPrintf("\n");
			}
			else if (number == (int)itr->number) {
				DebugPrintf("(%3i, %3i, %3i, %3i)   ", (itr->tuple >> 24) & 0xff, (itr->tuple >> 16) & 0xff,
							(itr->tuple >> 8) & 0xff, itr->tuple & 0xff);
				if (++cnt % 4 == 0)
					DebugPrintf("\n");
			}
			itr++;
		}
		DebugPrintf("\n");

		delete resources;
	}

	return true;
}

bool Console::cmdClearScreen(int argc, const char **argv) {
#ifdef INCLUDE_OLDGFX
	gfxop_clear_box(_vm->_gamestate->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update_box(_vm->_gamestate->gfx_state, gfx_rect(0, 0, 320, 200));
#endif
	return false;
}

bool Console::cmdRedrawScreen(int argc, const char **argv) {
#ifdef INCLUDE_OLDGFX
	_vm->_gamestate->visual->draw(Common::Point(0, 0));
	gfxop_update_box(_vm->_gamestate->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update(_vm->_gamestate->gfx_state);
	gfxop_sleep(_vm->_gamestate->gfx_state, 0);
#endif
	return false;
}

bool Console::cmdSaveGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Saves the current game state to the hard disk\n");
		DebugPrintf("Usage: %s <filename>\n", argv[0]);
		return true;
	}

	int result = 0;
	for (uint i = 0; i < _vm->_gamestate->_fileHandles.size(); i++)
		if (_vm->_gamestate->_fileHandles[i].isOpen())
			result++;

	if (result)
		DebugPrintf("Note: Game state has %d open file handles.\n", result);

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out = saveFileMan->openForSaving(argv[1]);
	const char *version = "";
	if (!out) {
		DebugPrintf("Error opening savegame \"%s\" for writing\n", argv[1]);
		return true;
	}

	// TODO: enable custom descriptions? force filename into a specific format?
	if (gamestate_save(_vm->_gamestate, out, "debugging", version)) {
		DebugPrintf("Saving the game state to '%s' failed\n", argv[1]);
	}

	return true;
}

bool Console::cmdRestoreGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Restores a saved game from the hard disk\n");
		DebugPrintf("Usage: %s <filename>\n", argv[0]);
		return true;
	}

	EngineState *newstate = NULL;

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::SeekableReadStream *in = saveFileMan->openForLoading(argv[1]);
	if (in) {
		// found a savegame file
		newstate = gamestate_restore(_vm->_gamestate, in);
		delete in;
	}

	if (newstate) {
		_vm->_gamestate->successor = newstate; // Set successor

		script_abort_flag = 2; // Abort current game with replay

		shrink_execution_stack(_vm->_gamestate, _vm->_gamestate->execution_stack_base + 1);
		return 0;
	} else {
		DebugPrintf("Restoring gamestate '%s' failed.\n", argv[1]);
		return 1;
	}

	return false;
}

bool Console::cmdRestartGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Restarts the game. There are two ways to restart a SCI game:\n");
		DebugPrintf("%s play - calls the game object's play() method\n", argv[0]);
		DebugPrintf("%s replay - calls the replay() methody\n", argv[0]);
		return true;
	}

	if (!scumm_stricmp(argv[1], "play")) {
		_vm->_gamestate->restarting_flags |= SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE;
	} else if (!scumm_stricmp(argv[1], "replay")) {
		_vm->_gamestate->restarting_flags &= ~SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE;
	} else {
		DebugPrintf("Invalid usage of %s\n", argv[0]);
		return true;
	}

	_vm->_gamestate->restarting_flags |= SCI_GAME_IS_RESTARTING_NOW;
	script_abort_flag = 1;

	return false;
}

bool Console::cmdClassTable(int argc, const char **argv) {
	DebugPrintf("Available classes:\n");
	for (uint i = 0; i < _vm->_gamestate->_segMan->_classtable.size(); i++) {
		if (_vm->_gamestate->_segMan->_classtable[i].reg.segment) {
			DebugPrintf(" Class 0x%x at %04x:%04x (script 0x%x)\n", i,
					PRINT_REG(_vm->_gamestate->_segMan->_classtable[i].reg),
					_vm->_gamestate->_segMan->_classtable[i].script);
		}
	}

	return true;
}

bool Console::cmdSentenceFragments(int argc, const char **argv) {
	DebugPrintf("Sentence fragments (used to build Parse trees)\n");

	for (uint i = 0; i < _vm->getVocabulary()->getParserBranchesSize(); i++) {
		int j = 0;

		const parse_tree_branch_t &branch = _vm->getVocabulary()->getParseTreeBranch(i);
		DebugPrintf("R%02d: [%x] ->", i, branch.id);
		while ((j < 10) && branch.data[j]) {
			int dat = branch.data[j++];

			switch (dat) {
			case VOCAB_TREE_NODE_COMPARE_TYPE:
				dat = branch.data[j++];
				DebugPrintf(" C(%x)", dat);
				break;

			case VOCAB_TREE_NODE_COMPARE_GROUP:
				dat = branch.data[j++];
				DebugPrintf(" WG(%x)", dat);
				break;

			case VOCAB_TREE_NODE_FORCE_STORAGE:
				dat = branch.data[j++];
				DebugPrintf(" FORCE(%x)", dat);
				break;

			default:
				if (dat > VOCAB_TREE_NODE_LAST_WORD_STORAGE) {
					int dat2 = branch.data[j++];
					DebugPrintf(" %x[%x]", dat, dat2);
				} else
					DebugPrintf(" ?%x?", dat);
			}
		}
		DebugPrintf("\n");
	}

	DebugPrintf("%d rules.\n", _vm->getVocabulary()->getParserBranchesSize());

	return true;
}

bool Console::cmdParse(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Parses a sequence of words with a GNF rule set and prints the resulting parse tree\n");
		DebugPrintf("Usage: %s <word1> <word2> ... <wordn>\n", argv[0]);
		return true;
	}

	ResultWordList words;
	char *error;
	char string[1000];

	// Construct the string
	strcpy(string, argv[2]);
	for (int i = 2; i < argc; i++) {
		strcat(string, argv[i]);
	}

	DebugPrintf("Parsing '%s'\n", string);
	bool res = _vm->getVocabulary()->tokenizeString(words, string, &error);
	if (res && !words.empty()) {
		int syntax_fail = 0;

		_vm->getVocabulary()->synonymizeTokens(words);

		DebugPrintf("Parsed to the following blocks:\n");

		for (ResultWordList::const_iterator i = words.begin(); i != words.end(); ++i)
			DebugPrintf("   Type[%04x] Group[%04x]\n", i->_class, i->_group);

		if (_vm->getVocabulary()->parseGNF(words, true))
			syntax_fail = 1; // Building a tree failed

		if (syntax_fail)
			DebugPrintf("Building a tree failed.\n");
		else
			_vm->getVocabulary()->dumpParseTree();

	} else {
		DebugPrintf("Unknown word: '%s'\n", error);
		free(error);
	}

	return true;
}

bool Console::cmdParserNodes(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Shows the specified number of nodes from the parse node tree\n");
		DebugPrintf("Usage: %s <nr>\n", argv[0]);
		DebugPrintf("where <nr> is the number of nodes to show from the parse node tree\n");
		return true;
	}

	int end = MIN<int>(atoi(argv[1]), VOCAB_TREE_NODES);

	_vm->getVocabulary()->printParserNodes(end);

	return true;
}

bool Console::cmdDrawPic(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Draws a pic resource\n");
		DebugPrintf("Usage: %s <nr> [<pal>] [<fl>]\n", argv[0]);
		DebugPrintf("where <nr> is the number of the pic resource to draw\n");
		DebugPrintf("<pal> is the optional default palette for the pic (default: 0)\n");
		DebugPrintf("<fl> are any pic draw flags (default: 1)\n");
		return true;
	}

	int flags = 1, default_palette = 0;

	if (argc > 2)
		default_palette = atoi(argv[2]);

	if (argc == 4)
		flags = atoi(argv[3]);

#ifdef INCLUDE_OLDGFX
	gfxop_new_pic(_vm->_gamestate->gfx_state, atoi(argv[1]), flags, default_palette);
	gfxop_clear_box(_vm->_gamestate->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update(_vm->_gamestate->gfx_state);
	gfxop_sleep(_vm->_gamestate->gfx_state, 0);
#endif

	return false;
}

bool Console::cmdDrawRect(int argc, const char **argv) {
	if (argc != 6) {
		DebugPrintf("Draws a rectangle to the screen with one of the EGA colors\n");
		DebugPrintf("Usage: %s <x> <y> <width> <height> <color>\n", argv[0]);
		DebugPrintf("where <color> is the EGA color to use (0-15)\n");
		return true;
	}

	int col = CLIP<int>(atoi(argv[5]), 0, 15);

#ifdef INCLUDE_OLDGFX
	gfxop_set_clip_zone(_vm->_gamestate->gfx_state, gfx_rect_fullscreen);
	gfxop_fill_box(_vm->_gamestate->gfx_state, gfx_rect(atoi(argv[1]), atoi(argv[2]),
										atoi(argv[3]), atoi(argv[4])), _vm->_gamestate->ega_colors[col]);
	gfxop_update(_vm->_gamestate->gfx_state);
#endif

	return false;
}

bool Console::cmdDrawCel(int argc, const char **argv) {
	if (argc != 4) {
		DebugPrintf("Draws a single view cel to the center of the screen\n");
		DebugPrintf("Usage: %s <view> <loop> <cel> <palette>\n", argv[0]);
		return true;
	}

	int view = atoi(argv[1]);
	int loop = atoi(argv[2]);
	int cel = atoi(argv[3]);
	int palette = atoi(argv[4]);


#ifdef INCLUDE_OLDGFX
	gfxop_set_clip_zone(_vm->_gamestate->gfx_state, gfx_rect_fullscreen);
	gfxop_draw_cel(_vm->_gamestate->gfx_state, view, loop, cel, Common::Point(160, 100), _vm->_gamestate->ega_colors[0], palette);
	gfxop_update(_vm->_gamestate->gfx_state);
#endif

	return false;
}

bool Console::cmdViewInfo(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Displays the number of loops and cels of each loop\n");
		DebugPrintf("for the specified view resource and palette.");
		DebugPrintf("Usage: %s <view> <palette>\n", argv[0]);
		return true;
	}

#ifdef INCLUDE_OLDGFX
	int view = atoi(argv[1]);
	int palette = atoi(argv[2]);
	int loops, i;
	gfxr_view_t *view_pixmaps = NULL;
	gfx_color_t transparent = { PaletteEntry(), 0, -1, -1, 0 };

	DebugPrintf("Resource view.%d ", view);

	loops = _vm->_gamestate->_gui->getLoopCount(view);

	if (loops < 0)
		DebugPrintf("does not exist.\n");
	else {
		DebugPrintf("has %d loops:\n", loops);

		for (i = 0; i < loops; i++) {
			int j, cels = _vm->_gamestate->_gui->getCelCount(view, i);

			DebugPrintf("Loop %d: %d cels.\n", i, cels);
			for (j = 0; j < cels; j++) {
				int width;
				int height;
				Common::Point mod;

				// Show pixmap on screen
				view_pixmaps = _vm->_gamestate->gfx_state->gfxResMan->getView(view, &i, &j, palette);
				gfxop_draw_cel(_vm->_gamestate->gfx_state, view, i, j, Common::Point(0,0), transparent, palette);

				gfxop_get_cel_parameters(_vm->_gamestate->gfx_state, view, i, j, &width, &height, &mod);

				DebugPrintf("   cel %d: size %dx%d, adj+(%d,%d)\n", j, width, height, mod.x, mod.y);
			}
		}
	}
#endif

	return true;
}

bool Console::cmdUndither(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Enable/disable undithering.\n");
		DebugPrintf("Usage: %s <0/1>\n", argv[0]);
		return true;
	}

	bool flag = atoi(argv[1]) ? true : false;

	return _vm->_gamestate->_gui->debugUndither(flag);
}

bool Console::cmdPlayVideo(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Plays a SEQ or AVI video.\n");
		DebugPrintf("Usage: %s <video file name> <delay>\n", argv[0]);
		DebugPrintf("The video file name should include the extension\n");
		DebugPrintf("Delay is only used in SEQ videos and is measured in ticks (default: 10)\n");
		return true;
	}

	Common::String filename = argv[1];
	filename.toLowercase();

	if (filename.hasSuffix(".seq") || filename.hasSuffix(".avi")) {
		_videoFile = filename;
		_videoFrameDelay = (argc == 2) ? 10 : atoi(argv[2]);
		return false;
	} else {
		DebugPrintf("Unknown video file type\n");
		return true;
	}
}

bool Console::cmdUpdateZone(int argc, const char **argv) {
	if (argc != 4) {
		DebugPrintf("Propagates a rectangular area from the back buffer to the front buffer\n");
		DebugPrintf("Usage: %s <x> <y> <width> <height>\n", argv[0]);
		return true;
	}

	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	int width = atoi(argv[3]);
	int height = atoi(argv[4]);

#ifdef INCLUDE_OLDGFX
	_vm->_gamestate->gfx_state->driver->update(gfx_rect(x, y, width, height), Common::Point(x, y), GFX_BUFFER_FRONT);
#endif

	return false;
}

bool Console::cmdPropagateZone(int argc, const char **argv) {
	if (argc != 5) {
		DebugPrintf("Propagates a rectangular area from a lower graphics buffer to a higher one\n");
		DebugPrintf("Usage: %s <x> <y> <width> <height> <map>\n", argv[0]);
		DebugPrintf("Where <map> can be 0 or 1\n");
		return true;
	}

#ifdef INCLUDE_OLDGFX
	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	int width = atoi(argv[3]);
	int height = atoi(argv[4]);
	int map = CLIP<int>(atoi(argv[5]), 0, 1);
	rect_t rect = gfx_rect(x, y, width, height);

	gfxop_set_clip_zone(_vm->_gamestate->gfx_state, gfx_rect_fullscreen);

	if (map == 1)
		gfxop_clear_box(_vm->_gamestate->gfx_state, rect);
	else
		gfxop_update_box(_vm->_gamestate->gfx_state, rect);
	gfxop_update(_vm->_gamestate->gfx_state);
	gfxop_sleep(_vm->_gamestate->gfx_state, 0);
#endif

	return false;
}

bool Console::cmdFillScreen(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Fills the screen with one of the EGA colors\n");
		DebugPrintf("Usage: %s <color>\n", argv[0]);
		DebugPrintf("where <color> is the EGA color to use (0-15)\n");
		return true;
	}

	int col = CLIP<int>(atoi(argv[1]), 0, 15);

#ifdef INCLUDE_OLDGFX
	gfxop_set_clip_zone(_vm->_gamestate->gfx_state, gfx_rect_fullscreen);
	gfxop_fill_box(_vm->_gamestate->gfx_state, gfx_rect_fullscreen, _vm->_gamestate->ega_colors[col]);
	gfxop_update(_vm->_gamestate->gfx_state);
#endif

	return false;
}

bool Console::cmdCurrentPort(int argc, const char **argv) {
#ifdef INCLUDE_OLDGFX
	if (!_vm->_gamestate->port)
		DebugPrintf("There is no port active currently.\n");
	else
		DebugPrintf("Current port ID: %d\n", _vm->_gamestate->port->_ID);
#endif

	return true;
}

bool Console::cmdPrintPort(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints information about a port\n");
		DebugPrintf("%s current - prints information about the current port\n", argv[0]);
		DebugPrintf("%s <ID> - prints information about the port with the specified ID\n", argv[0]);
		return true;
	}

#ifdef INCLUDE_OLDGFX
	GfxPort *port;
	
	if (!scumm_stricmp(argv[1], "current")) {
		port = _vm->_gamestate->port;
		if (!port)
			DebugPrintf("There is no active port currently\n");
		else
			port->print(0);
	} else {
		if (!_vm->_gamestate->visual) {
			DebugPrintf("Visual is uninitialized\n");
		} else {
			port = _vm->_gamestate->visual->getPort(atoi(argv[1]));
			if (!port)
				DebugPrintf("No such port\n");
			else
				port->print(0);
		}
	}
#endif

	return true;
}

bool Console::cmdParseGrammar(int argc, const char **argv) {
	DebugPrintf("Parse grammar, in strict GNF:\n");

	_vm->getVocabulary()->buildGNF(true);

	return true;
}

bool Console::cmdVisualState(int argc, const char **argv) {
	DebugPrintf("State of the current visual widget:\n");

#ifdef INCLUDE_OLDGFX
	if (_vm->_gamestate->visual)
		_vm->_gamestate->visual->print(0);
	else
		DebugPrintf("The visual widget is uninitialized.\n");
#endif

	return true;
}

bool Console::cmdFlushPorts(int argc, const char **argv) {
#ifdef INCLUDE_OLDGFX
	_vm->_gamestate->_gui->hideCursor();
	DebugPrintf("Flushing dynamically allocated ports (for memory profiling)...\n");
	delete _vm->_gamestate->visual;
	_vm->_gamestate->gfx_state->gfxResMan->freeAllResources();
	_vm->_gamestate->visual = NULL;
#endif

	return true;
}

bool Console::cmdDynamicViews(int argc, const char **argv) {
#ifdef INCLUDE_OLDGFX
	DebugPrintf("List of active dynamic views:\n");

	if (_vm->_gamestate->dyn_views)
		_vm->_gamestate->dyn_views->print(0);
	else
		DebugPrintf("The list is empty.\n");
#endif

	return true;
}

bool Console::cmdDroppedViews(int argc, const char **argv) {
#ifdef INCLUDE_OLDGFX
	DebugPrintf("List of dropped dynamic views:\n");

	if (_vm->_gamestate->drop_views)
		_vm->_gamestate->drop_views->print(0);
	else
		DebugPrintf("The list is empty.\n");
#endif

	return true;
}

bool Console::cmdPriorityBands(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Priority bands start at y=%d. They end at y=%d\n", _vm->_gamestate->priority_first, _vm->_gamestate->priority_last);
		DebugPrintf("Use %s <priority band> to print the start of priority for the specified priority band (0 - 15)\n", argv[0]);
		return true;
	}

	int zone = CLIP<int>(atoi(argv[1]), 0, 15);
#ifdef INCLUDE_OLDGFX
	DebugPrintf("Zone %x starts at y=%d\n", zone, _find_priority_band(_vm->_gamestate, zone));
#endif

	return true;
}

bool Console::cmdStatusBarColors(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Sets the colors of the status bar\n");
		DebugPrintf("Usage: %s <foreground color> <background color>\n", argv[0]);
		return true;
	}

#ifdef INCLUDE_OLDGFX
	_vm->_gamestate->titlebar_port->_color = _vm->_gamestate->ega_colors[atoi(argv[1])];
	_vm->_gamestate->titlebar_port->_bgcolor = _vm->_gamestate->ega_colors[atoi(argv[2])];

	_vm->_gamestate->status_bar_foreground = atoi(argv[1]);
	_vm->_gamestate->status_bar_background = atoi(argv[2]);

	sciw_set_status_bar(_vm->_gamestate, _vm->_gamestate->titlebar_port, _vm->_gamestate->_statusBarText,
							_vm->_gamestate->status_bar_foreground, _vm->_gamestate->status_bar_background);
	gfxop_update(_vm->_gamestate->gfx_state);
#endif

	return false;
}

bool Console::cmdPrintSegmentTable(int argc, const char **argv) {
	DebugPrintf("Segment table:\n");

	for (uint i = 0; i < _vm->_gamestate->_segMan->_heap.size(); i++) {
		SegmentObj *mobj = _vm->_gamestate->_segMan->_heap[i];
		if (mobj && mobj->getType()) {
			DebugPrintf(" [%04x] ", i);

			switch (mobj->getType()) {
			case SEG_TYPE_SCRIPT:
				DebugPrintf("S  script.%03d l:%d ", (*(Script *)mobj)._nr, (*(Script *)mobj).getLockers());
				break;

			case SEG_TYPE_CLONES:
				DebugPrintf("C  clones (%d allocd)", (*(CloneTable *)mobj).entries_used);
				break;

			case SEG_TYPE_LOCALS:
				DebugPrintf("V  locals %03d", (*(LocalVariables *)mobj).script_id);
				break;

			case SEG_TYPE_STACK:
				DebugPrintf("D  data stack (%d)", (*(DataStack *)mobj)._capacity);
				break;

			case SEG_TYPE_SYS_STRINGS:
				DebugPrintf("Y  system string table");
				break;

			case SEG_TYPE_LISTS:
				DebugPrintf("L  lists (%d)", (*(ListTable *)mobj).entries_used);
				break;

			case SEG_TYPE_NODES:
				DebugPrintf("N  nodes (%d)", (*(NodeTable *)mobj).entries_used);
				break;

			case SEG_TYPE_HUNK:
				DebugPrintf("H  hunk (%d)", (*(HunkTable *)mobj).entries_used);
				break;

			case SEG_TYPE_DYNMEM:
				DebugPrintf("M  dynmem: %d bytes", (*(DynMem *)mobj)._size);
				break;

			case SEG_TYPE_STRING_FRAG:
				DebugPrintf("F  string fragments");
				break;

			default:
				DebugPrintf("I  Invalid (type = %x)", mobj->getType());
				break;
			}

			DebugPrintf("  \n");
		}
	}
	DebugPrintf("\n");

	return true;
}

bool Console::segmentInfo(int nr) {
	DebugPrintf("[%04x] ", nr);

	if ((nr < 0) || ((uint)nr >= _vm->_gamestate->_segMan->_heap.size()) || !_vm->_gamestate->_segMan->_heap[nr])
		return false;

	SegmentObj *mobj = _vm->_gamestate->_segMan->_heap[nr];

	switch (mobj->getType()) {

	case SEG_TYPE_SCRIPT: {
		Script *scr = (Script *)mobj;
		DebugPrintf("script.%03d locked by %d, bufsize=%d (%x)\n", scr->_nr, scr->getLockers(), (uint)scr->_bufSize, (uint)scr->_bufSize);
		if (scr->_exportTable)
			DebugPrintf("  Exports: %4d at %d\n", scr->_numExports, (int)(((byte *)scr->_exportTable) - ((byte *)scr->_buf)));
		else
			DebugPrintf("  Exports: none\n");

		DebugPrintf("  Synonyms: %4d\n", scr->_numSynonyms);

		if (scr->_localsBlock)
			DebugPrintf("  Locals : %4d in segment 0x%x\n", scr->_localsBlock->_locals.size(), scr->_localsSegment);
		else
			DebugPrintf("  Locals : none\n");

		DebugPrintf("  Objects: %4d\n", scr->_objects.size());

		ObjMap::iterator it;
		const ObjMap::iterator end = scr->_objects.end();
		for (it = scr->_objects.begin(); it != end; ++it) {
			DebugPrintf("    ");
			// Object header
			Object *obj = _vm->_gamestate->_segMan->getObject(it->_value.getPos());
			if (obj)
				DebugPrintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(it->_value.getPos()),
							_vm->_gamestate->_segMan->getObjectName(it->_value.getPos()),
							obj->getVarCount(), obj->getMethodCount());
		}
	}
	break;

	case SEG_TYPE_LOCALS: {
		LocalVariables *locals = (LocalVariables *)mobj;
		DebugPrintf("locals for script.%03d\n", locals->script_id);
		DebugPrintf("  %d (0x%x) locals\n", locals->_locals.size(), locals->_locals.size());
	}
	break;

	case SEG_TYPE_STACK: {
		DataStack *stack = (DataStack *)mobj;
		DebugPrintf("stack\n");
		DebugPrintf("  %d (0x%x) entries\n", stack->_capacity, stack->_capacity);
	}
	break;

	case SEG_TYPE_SYS_STRINGS: {
		DebugPrintf("system string table - viewing currently disabled\n");
#if 0
		SystemStrings *strings = &(mobj->data.sys_strings);

		for (int i = 0; i < SYS_STRINGS_MAX; i++)
			if (strings->strings[i].name)
				DebugPrintf("  %s[%d]=\"%s\"\n", strings->strings[i].name, strings->strings[i].max_size, strings->strings[i].value);
#endif
	}
	break;

	case SEG_TYPE_CLONES: {
		CloneTable *ct = (CloneTable *)mobj;

		DebugPrintf("clones\n");

		for (uint i = 0; i < ct->_table.size(); i++)
			if (ct->isValidEntry(i)) {
				reg_t objpos;
				objpos.offset = i;
				objpos.segment = nr;
				DebugPrintf("  [%04x] %s; copy of ", i, _vm->_gamestate->_segMan->getObjectName(objpos));
				// Object header
				Object *obj = _vm->_gamestate->_segMan->getObject(ct->_table[i].getPos());
				if (obj)
					DebugPrintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(ct->_table[i].getPos()),
								_vm->_gamestate->_segMan->getObjectName(ct->_table[i].getPos()),
								obj->getVarCount(), obj->getMethodCount());
			}
	}
	break;

	case SEG_TYPE_LISTS: {
		ListTable *lt = (ListTable *)mobj;

		DebugPrintf("lists\n");
		for (uint i = 0; i < lt->_table.size(); i++)
			if (lt->isValidEntry(i)) {
				DebugPrintf("  [%04x]: ", i);
				printList(&(lt->_table[i]));
			}
	}
	break;

	case SEG_TYPE_NODES: {
		DebugPrintf("nodes (total %d)\n", (*(NodeTable *)mobj).entries_used);
		break;
	}

	case SEG_TYPE_HUNK: {
		HunkTable *ht = (HunkTable *)mobj;

		DebugPrintf("hunk  (total %d)\n", ht->entries_used);
		for (uint i = 0; i < ht->_table.size(); i++)
			if (ht->isValidEntry(i)) {
				DebugPrintf("    [%04x] %d bytes at %p, type=%s\n",
				          i, ht->_table[i].size, ht->_table[i].mem, ht->_table[i].type);
			}
	}
	break;

	case SEG_TYPE_DYNMEM: {
		DebugPrintf("dynmem (%s): %d bytes\n",
		          (*(DynMem *)mobj)._description.c_str(), (*(DynMem *)mobj)._size);

		Common::hexdump((*(DynMem *)mobj)._buf, (*(DynMem *)mobj)._size, 16, 0);
	}
	break;

	case SEG_TYPE_STRING_FRAG: {
		DebugPrintf("string frags\n");
		break;
	}

	default :
		DebugPrintf("Invalid type %d\n", mobj->getType());
		break;
	}

	DebugPrintf("\n");
	return true;
}

bool Console::cmdSegmentInfo(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Provides information on the specified segment(s)\n");
		DebugPrintf("Usage: %s <segment number>\n", argv[0]);
		DebugPrintf("<segment number> can be a number, which shows the information of the segment with\n");
		DebugPrintf("the specified number, or \"all\" to show information on all active segments");
		return true;
	}

	if (!scumm_stricmp(argv[1], "all")) {
		for (uint i = 0; i < _vm->_gamestate->_segMan->_heap.size(); i++)
			segmentInfo(i);
	} else {
		int nr = atoi(argv[1]);
		if (!segmentInfo(nr))
			DebugPrintf("Segment %04x does not exist\n", nr);
	}

	return true;
}


bool Console::cmdKillSegment(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Deletes the specified segment\n");
		DebugPrintf("Usage: %s <segment number>\n", argv[0]);
		return true;
	}

	_vm->_gamestate->_segMan->getScript(atoi(argv[1]))->setLockers(0);

	return true;
}

bool Console::cmdShowMap(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Shows one of the screen maps\n");
		DebugPrintf("Usage: %s <screen map>\n", argv[0]);
		DebugPrintf("Screen maps:\n");
		DebugPrintf("- 0: visual map (back buffer)\n");
		DebugPrintf("- 1: priority map (back buffer)\n");
		DebugPrintf("- 2: control map (static buffer)\n");
		DebugPrintf("- 3: display screen (newgui only)\n");
		return true;
	}

	int map = atoi(argv[1]);

	switch (map) {
	case 0:
	case 1:
	case 2:
	case 3:
		return _vm->_gamestate->_gui->debugShowMap(map);
		break;

	default:
		DebugPrintf("Map %d is not available.\n", map);
		return true;
	}
	return false;
}

bool Console::cmdSongLib(int argc, const char **argv) {
	DebugPrintf("Song library:\n");

	Song *seeker = _vm->_gamestate->_sound._songlib._lib;

	do {
		DebugPrintf("    %p", (void *)seeker);

		if (seeker) {
			DebugPrintf("[%04lx,p=%d,s=%d]->", seeker->_handle, seeker->_priority, seeker->_status);
			seeker = seeker->_next;
		}
		DebugPrintf("\n");
	} while (seeker);
	DebugPrintf("\n");

	return true;
}

bool Console::cmdGCInvoke(int argc, const char **argv) {
	DebugPrintf("Performing garbage collection...\n");
	run_gc(_vm->_gamestate);
	return true;
}

bool Console::cmdGCObjects(int argc, const char **argv) {
	reg_t_hash_map *use_map = find_all_used_references(_vm->_gamestate);

	DebugPrintf("Reachable object references (normalised):\n");
	for (reg_t_hash_map::iterator i = use_map->begin(); i != use_map->end(); ++i) {
		DebugPrintf(" - %04x:%04x\n", PRINT_REG(i->_key));
	}

	delete use_map;

	return true;
}

void _print_address(void * _, reg_t addr) {
	if (addr.segment)
		((SciEngine *)g_engine)->getSciDebugger()->DebugPrintf("  %04x:%04x\n", PRINT_REG(addr));
}

bool Console::cmdGCShowReachable(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints all addresses directly reachable from the memory object specified as parameter.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;
	
	if (parse_reg_t(_vm->_gamestate, argv[1], &addr)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentObj *mobj = _vm->_gamestate->_segMan->getSegmentObj(addr.segment);
	if (!mobj) {
		DebugPrintf("Unknown segment : %x\n", addr.segment);
		return 1;
	}

	DebugPrintf("Reachable from %04x:%04x:\n", PRINT_REG(addr));
	mobj->listAllOutgoingReferences(addr, NULL, _print_address);

	return true;
}

bool Console::cmdGCShowFreeable(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints all addresses freeable in the segment associated with the\n");
		DebugPrintf("given address (offset is ignored).\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;
	
	if (parse_reg_t(_vm->_gamestate, argv[1], &addr)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentObj *mobj = _vm->_gamestate->_segMan->getSegmentObj(addr.segment);
	if (!mobj) {
		DebugPrintf("Unknown segment : %x\n", addr.segment);
		return true;
	}

	DebugPrintf("Freeable in segment %04x:\n", addr.segment);
	mobj->listAllDeallocatable(addr.segment, NULL, _print_address);

	return true;
}

bool Console::cmdGCNormalize(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints the \"normal\" address of a given address,\n");
		DebugPrintf("i.e. the address we would free in order to free\n");
		DebugPrintf("the object associated with the original address.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;
	
	if (parse_reg_t(_vm->_gamestate, argv[1], &addr)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentObj *mobj = _vm->_gamestate->_segMan->getSegmentObj(addr.segment);
	if (!mobj) {
		DebugPrintf("Unknown segment : %x\n", addr.segment);
		return true;
	}

	addr = mobj->findCanonicAddress(_vm->_gamestate->_segMan, addr);
	DebugPrintf(" %04x:%04x\n", PRINT_REG(addr));

	return true;
}

bool Console::cmdVMVarlist(int argc, const char **argv) {
	const char *varnames[] = {"global", "local", "temp", "param"};

	DebugPrintf("Addresses of variables in the VM:\n");

	for (int i = 0; i < 4; i++) {
		DebugPrintf("%s vars at %04x:%04x ", varnames[i], PRINT_REG(make_reg(scriptState.variables_seg[i], scriptState.variables[i] - scriptState.variables_base[i])));
		if (scriptState.variables_max)
			DebugPrintf("  total %d", scriptState.variables_max[i]);
		DebugPrintf("\n");
	}

	return true;
}

bool Console::cmdVMVars(int argc, const char **argv) {
	if (argc < 3) {
		DebugPrintf("Displays or changes variables in the VM\n");
		DebugPrintf("Usage: %s <type> <varnum> [<value>]\n", argv[0]);
		DebugPrintf("First parameter is either g(lobal), l(ocal), t(emp) or p(aram).\n");
		DebugPrintf("Second parameter is the var number\n");
		DebugPrintf("Third parameter (if specified) is the value to set the variable to, in address form\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	const char *varnames[] = {"global", "local", "temp", "param"};
	const char *varabbrev = "gltp";
	const char *vartype_pre = strchr(varabbrev, *argv[1]);
	int vartype;
	int idx = atoi(argv[2]);

	if (!vartype_pre) {
		DebugPrintf("Invalid variable type '%c'\n", *argv[1]);
		return true;
	}

	vartype = vartype_pre - varabbrev;

	if (idx < 0) {
		DebugPrintf("Invalid: negative index\n");
		return true;
	}

	if ((scriptState.variables_max) && (scriptState.variables_max[vartype] <= idx)) {
		DebugPrintf("Max. index is %d (0x%x)\n", scriptState.variables_max[vartype], scriptState.variables_max[vartype]);
		return true;
	}

	switch (argc) {
	case 3:
		DebugPrintf("%s var %d == %04x:%04x\n", varnames[vartype], idx, PRINT_REG(scriptState.variables[vartype][idx]));
		break;
	case 4:
		if (parse_reg_t(_vm->_gamestate, argv[3], &scriptState.variables[vartype][idx])) {
			DebugPrintf("Invalid address passed.\n");
			DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
			return true;
		}
		break;
	default:
		DebugPrintf("Too many arguments\n");
	}

	return true;
}

bool Console::cmdStack(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Lists the specified number of stack elements.\n");
		DebugPrintf("Usage: %s <elements>\n", argv[0]);
		return true;
	}

	if (_vm->_gamestate->_executionStack.empty()) {
		DebugPrintf("No exec stack!");
		return true;
	}

	ExecStack &xs = _vm->_gamestate->_executionStack.back();
	int nr = atoi(argv[1]);

	for (int i = nr; i > 0; i--) {
		if ((xs.sp - xs.fp - i) == 0)
			DebugPrintf("-- temp variables --\n");
		if (xs.sp - i >= _vm->_gamestate->stack_base)
			DebugPrintf("ST:%04x = %04x:%04x\n", (unsigned)(xs.sp - i - _vm->_gamestate->stack_base), PRINT_REG(xs.sp[-i]));
	}

	return true;
}

bool Console::cmdValueType(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Determines the type of a value.\n");
		DebugPrintf("The type can be one of the following:\n");
		DebugPrintf("Invalid, list, object, reference or arithmetic\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t val;
	
	if (parse_reg_t(_vm->_gamestate, argv[1], &val)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	int t = determine_reg_type(_vm->_gamestate->_segMan, val);

	switch (t) {
	case KSIG_LIST:
		DebugPrintf("List");
		break;
	case KSIG_OBJECT:
		DebugPrintf("Object");
		break;
	case KSIG_REF:
		DebugPrintf("Reference");
		break;
	case KSIG_ARITHMETIC:
		DebugPrintf("Arithmetic");
		break;
	default:
		DebugPrintf("Erroneous unknown type %02x(%d decimal)\n", t, t);
	}

	return true;
}

bool Console::cmdViewListNode(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Examines the list node at the given address.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;
	
	if (parse_reg_t(_vm->_gamestate, argv[1], &addr)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	printNode(addr);
	return true;
}

bool Console::cmdViewReference(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Examines an arbitrary reference.\n");
		DebugPrintf("Usage: %s <start address> [<end address>]\n", argv[0]);
		DebugPrintf("Where <start address> is the starting address to examine\n");
		DebugPrintf("<end address>, if provided, is the address where examining ends at\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t reg = NULL_REG;
	reg_t reg_end = NULL_REG;

	if (parse_reg_t(_vm->_gamestate, argv[1], &reg)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	if (argc > 2) {
		if (parse_reg_t(_vm->_gamestate, argv[2], &reg_end)) {
			DebugPrintf("Invalid address passed.\n");
			DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
			return true;
		}
	}

	int type_mask = determine_reg_type(_vm->_gamestate->_segMan, reg);
	int filter;
	int found = 0;

	DebugPrintf("%04x:%04x is of type 0x%x: ", PRINT_REG(reg), type_mask);

	if (reg.segment == 0 && reg.offset == 0) {
		DebugPrintf("Null.\n");
		return true;
	}

	if (reg_end.segment != reg.segment) {
		DebugPrintf("Ending segment different from starting segment. Assuming no bound on dump.\n");
		reg_end = NULL_REG;
	}

	for (filter = 1; filter < 0xf000; filter <<= 1) {
		int type = type_mask & filter;

		if (found && type) {
			DebugPrintf("--- Alternatively, it could be a ");
		}


		switch (type) {
		case 0:
			break;
		case KSIG_LIST: {
			List *l = _vm->_gamestate->_segMan->lookupList(reg);

			DebugPrintf("list\n");

			if (l)
				printList(l);
			else
				DebugPrintf("Invalid list.\n");
		}
			break;
		case KSIG_NODE:
			DebugPrintf("list node\n");
			printNode(reg);
			break;
		case KSIG_OBJECT:
			DebugPrintf("object\n");
			printObject(reg);
			break;
		case KSIG_REF: {
			int size;
			const SegmentRef block = _vm->_gamestate->_segMan->dereference(reg);
			size = block.maxSize;

			DebugPrintf("raw data\n");

			if (reg_end.segment != 0 && size < reg_end.offset - reg.offset) {
				DebugPrintf("Block end out of bounds (size %d). Resetting.\n", size);
				reg_end = NULL_REG;
			}

			if (reg_end.segment != 0 && (size >= reg_end.offset - reg.offset))
				size = reg_end.offset - reg.offset;

			if (reg_end.segment != 0)
				DebugPrintf("Block size less than or equal to %d\n", size);

			Common::hexdump(block.raw, size, 16, 0);
			}
			break;
		case KSIG_ARITHMETIC:
			DebugPrintf("arithmetic value\n  %d (%04x)\n", (int16) reg.offset, reg.offset);
			break;
		default:
			DebugPrintf("unknown type %d.\n", type);
		}

		if (type) {
			DebugPrintf("\n");
			found = 1;
		}
	}

	return true;
}

bool Console::cmdViewObject(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Examines the object at the given address.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;
	
	if (parse_reg_t(_vm->_gamestate, argv[1], &addr)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	DebugPrintf("Information on the object at the given address:\n");
	printObject(addr);

	return true;
}

bool Console::cmdViewActiveObject(int argc, const char **argv) {
	DebugPrintf("Information on the currently active object or class:\n");
	printObject(scriptState.xs->objp);

	return true;
}

bool Console::cmdViewAccumulatorObject(int argc, const char **argv) {
	DebugPrintf("Information on the currently active object or class at the address indexed by the accumulator:\n");
	printObject(_vm->_gamestate->r_acc);

	return true;
}

bool Console::cmdScriptSteps(int argc, const char **argv) {
	DebugPrintf("Number of executed SCI operations: %d\n", script_step_counter);
	return true;
}

bool Console::cmdSetAccumulator(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Sets the accumulator.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t val;
	
	if (parse_reg_t(_vm->_gamestate, argv[1], &val)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	_vm->_gamestate->r_acc = val;

	return true;
}

bool Console::cmdBacktrace(int argc, const char **argv) {
	DebugPrintf("Dumping the send/self/super/call/calle/callb stack:\n");

	printf("Call stack (current base: 0x%x):\n", _vm->_gamestate->execution_stack_base);
	Common::List<ExecStack>::iterator iter;
	uint i = 0;

	for (iter = _vm->_gamestate->_executionStack.begin();
	     iter != _vm->_gamestate->_executionStack.end(); ++iter, ++i) {
		ExecStack &call = *iter;
		const char *objname = _vm->_gamestate->_segMan->getObjectName(call.sendp);
		int paramc, totalparamc;

		switch (call.type) {

		case EXEC_STACK_TYPE_CALL: {// Normal function
			printf(" %x:[%x]  %s::%s(", i, call.origin, objname, (call.selector == -1) ? "<call[be]?>" :
			          selector_name(_vm->_gamestate, call.selector));
		}
		break;

		case EXEC_STACK_TYPE_KERNEL: // Kernel function
			printf(" %x:[%x]  k%s(", i, call.origin, _vm->getKernel()->getKernelName(call.selector).c_str());
			break;

		case EXEC_STACK_TYPE_VARSELECTOR:
			printf(" %x:[%x] vs%s %s::%s (", i, call.origin, (call.argc) ? "write" : "read",
			          objname, _vm->getKernel()->getSelectorName(call.selector).c_str());
			break;
		}

		totalparamc = call.argc;

		if (totalparamc > 16)
			totalparamc = 16;

		for (paramc = 1; paramc <= totalparamc; paramc++) {
			printf("%04x:%04x", PRINT_REG(call.variables_argp[paramc]));

			if (paramc < call.argc)
				printf(", ");
		}

		if (call.argc > 16)
			printf("...");

		printf(")\n    obj@%04x:%04x", PRINT_REG(call.objp));
		if (call.type == EXEC_STACK_TYPE_CALL) {
			printf(" pc=%04x:%04x", PRINT_REG(call.addr.pc));
			if (call.sp == CALL_SP_CARRY)
				printf(" sp,fp:carry");
			else {
				printf(" sp=ST:%04x", (unsigned)(call.sp - _vm->_gamestate->stack_base));
				printf(" fp=ST:%04x", (unsigned)(call.fp - _vm->_gamestate->stack_base));
			}
		} else
			printf(" pc:none");

		printf(" argp:ST:%04x", (unsigned)(call.variables_argp - _vm->_gamestate->stack_base));
		if (call.type == EXEC_STACK_TYPE_CALL)
			printf(" script: %d", (*(Script *)_vm->_gamestate->_segMan->_heap[call.addr.pc.segment])._nr);
		printf("\n");
	}

	return true;
}

bool Console::cmdStep(int argc, const char **argv) {
	if (argc == 2 && atoi(argv[1]) > 0)
		g_debugState.runningStep = atoi(argv[1]) - 1;
	g_debugState.debugging = true;

	return false;
}

bool Console::cmdStepEvent(int argc, const char **argv) {
	g_debugState.stopOnEvent = true;
	g_debugState.debugging = true;

	return false;
}

bool Console::cmdStepRet(int argc, const char **argv) {
	g_debugState.seeking = kDebugSeekLevelRet;
	g_debugState.seekLevel = _vm->_gamestate->_executionStack.size() - 1;
	g_debugState.debugging = true;

	return false;
}

bool Console::cmdStepGlobal(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Steps until the global variable with the specified index is modified.\n");
		DebugPrintf("Usage: %s <global variable index>\n", argv[0]);
		return true;
	}

	g_debugState.seeking = kDebugSeekGlobal;
	g_debugState.seekSpecial = atoi(argv[1]);
	g_debugState.debugging = true;

	return false;
}

bool Console::cmdStepCallk(int argc, const char **argv) {
	int callk_index;
	char *endptr;

	if (argc == 2) {
		/* Try to convert the parameter to a number. If the conversion stops
		   before end of string, assume that the parameter is a function name
		   and scan the function table to find out the index. */
		callk_index = strtoul(argv[1], &endptr, 0);
		if (*endptr != '\0') {
			callk_index = -1;
			for (uint i = 0; i < _vm->getKernel()->getKernelNamesSize(); i++)
				if (argv[1] == _vm->getKernel()->getKernelName(i)) {
					callk_index = i;
					break;
				}

			if (callk_index == -1) {
				DebugPrintf("Unknown kernel function '%s'\n", argv[1]);
				return true;
			}
		}

		g_debugState.seeking = kDebugSeekSpecialCallk;
		g_debugState.seekSpecial = callk_index;
	} else {
		g_debugState.seeking = kDebugSeekCallk;
	}
	g_debugState.debugging = true;

	return false;
}

bool Console::cmdDisassemble(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Disassembles a method by name.\n");
		DebugPrintf("Usage: %s <object> <method>\n", argv[0]);
		return true;
	}

	reg_t objAddr = NULL_REG;

	if (parse_reg_t(_vm->_gamestate, argv[1], &objAddr)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	Object *obj = _vm->_gamestate->_segMan->getObject(objAddr);
	int selector_id = _vm->getKernel()->findSelector(argv[2]);
	reg_t addr;

	if (!obj) {
		DebugPrintf("Not an object.");
		return true;
	}

	if (selector_id < 0) {
		DebugPrintf("Not a valid selector name.");
		return true;
	}

	if (lookup_selector(_vm->_gamestate->_segMan, objAddr, selector_id, NULL, &addr) != kSelectorMethod) {
		DebugPrintf("Not a method.");
		return true;
	}

	do {
		addr = disassemble(_vm->_gamestate, addr, 0, 0);
	} while (addr.offset > 0);

	return true;
}

bool Console::cmdDisassembleAddress(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Disassembles one or more commands.\n");
		DebugPrintf("Usage: %s [startaddr] <options>\n", argv[0]);
		DebugPrintf("Valid options are:\n");
		DebugPrintf(" bwt  : Print byte/word tag\n");
		DebugPrintf(" c<x> : Disassemble <x> bytes\n");
		DebugPrintf(" bc   : Print bytecode\n");
		return true;
	}

	reg_t vpc = NULL_REG;
	int op_count = 1;
	int do_bwc = 0;
	int do_bytes = 0;
	int size;

	if (parse_reg_t(_vm->_gamestate, argv[1], &vpc)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentRef ref = _vm->_gamestate->_segMan->dereference(vpc);
	size = ref.maxSize + vpc.offset; // total segment size

	for (int i = 2; i < argc; i++) {
		if (!scumm_stricmp(argv[i], "bwt"))
			do_bwc = 1;
		else if (!scumm_stricmp(argv[i], "bc"))
			do_bytes = 1;
		else if (toupper(argv[i][0]) == 'C')
			op_count = atoi(argv[i] + 1);
		else {
			DebugPrintf("Invalid option '%s'\n", argv[i]);
			return true;
		}
	}

	if (op_count < 0) {
		DebugPrintf("Invalid op_count\n");
		return true;
	}

	do {
		vpc = disassemble(_vm->_gamestate, vpc, do_bwc, do_bytes);
	} while ((vpc.offset > 0) && (vpc.offset + 6 < size) && (--op_count));

	return true;
}

bool Console::cmdSend(int argc, const char **argv) {
	if (argc < 3) {
		DebugPrintf("Sends a message to an object.\n");
		DebugPrintf("Usage: %s <object> <selector name> <param1> <param2> ... <paramn>\n", argv[0]);
		DebugPrintf("Example: send ?fooScript cue\n");
		return true;
	}

	reg_t object;
	
	if (parse_reg_t(_vm->_gamestate, argv[1], &object)) {
		DebugPrintf("Invalid address \"%s\" passed.\n", argv[1]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}
	
	const char *selector_name = argv[2];
	int selector_id = _vm->getKernel()->findSelector(selector_name);

	if (selector_id < 0) {
		DebugPrintf("Unknown selector: \"%s\"\n", selector_name);
		return true;
	}

	Object *o = _vm->_gamestate->_segMan->getObject(object);
	if (o == NULL) {
		DebugPrintf("Address \"%04x:%04x\" is not an object\n", PRINT_REG(object));
		return true;
	}

	SelectorType selector_type = lookup_selector(_vm->_gamestate->_segMan, object, selector_id, 0, 0);

	if (selector_type == kSelectorNone) {
		DebugPrintf("Object does not support selector: \"%s\"\n", selector_name);
		return true;
	}

	// everything after the selector name is passed as an argument to the send
	int send_argc = argc - 3;

	// Create the data block for send_selecor() at the top of the stack:
	// [selector_number][argument_counter][arguments...]
	StackPtr stackframe = _vm->_gamestate->_executionStack.back().sp;
	stackframe[0] = make_reg(0, selector_id);
	stackframe[1] = make_reg(0, send_argc);
	for (int i = 0; i < send_argc; i++) {
		if (parse_reg_t(_vm->_gamestate, argv[3+i], &stackframe[2+i])) {
			DebugPrintf("Invalid address \"%s\" passed.\n", argv[3+i]);
			DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
			return true;
		}
	}

	reg_t old_acc = _vm->_gamestate->r_acc;

	// Now commit the actual function:
	ExecStack *old_xstack, *xstack;
	old_xstack = &_vm->_gamestate->_executionStack.back();
	xstack = send_selector(_vm->_gamestate, object, object,
	                       stackframe + 2 + send_argc,
	                       2 + send_argc, stackframe);

	bool restore_acc = old_xstack != xstack || argc == 3;

	if (old_xstack != xstack) {
		_vm->_gamestate->_executionStackPosChanged = true;
		DebugPrintf("Message scheduled for execution\n");

		// We call run_vm explictly so we can restore the value of r_acc
		// after execution.
		run_vm(_vm->_gamestate, 0);

	}

	if (restore_acc) {
		// varselector read or message executed
		DebugPrintf("Message completed. Value returned: %04x:%04x\n", PRINT_REG(_vm->_gamestate->r_acc));
		_vm->_gamestate->r_acc = old_acc;
	}

	return true;
}

bool Console::cmdGo(int argc, const char **argv) {
	// CHECKME: is this necessary?
	g_debugState.seeking = kDebugSeekNothing;

	return Cmd_Exit(argc, argv);
}

bool Console::cmdBreakpointList(int argc, const char **argv) {
	Breakpoint *bp = _vm->_gamestate->bp_list;
	int i = 0;
	int bpdata;

	DebugPrintf("Breakpoint list:\n");

	while (bp) {
		DebugPrintf("  #%i: ", i);
		switch (bp->type) {
		case BREAK_SELECTOR:
			DebugPrintf("Execute %s\n", bp->data.name);
			break;
		case BREAK_EXPORT:
			bpdata = bp->data.address;
			DebugPrintf("Execute script %d, export %d\n", bpdata >> 16, bpdata & 0xFFFF);
			break;
		}

		bp = bp->next;
		i++;
	}

	return true;
}

bool Console::cmdBreakpointDelete(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Deletes a breakpoint with the specified index.\n");
		DebugPrintf("Usage: %s <breakpoint index>\n", argv[0]);
		return true;
	}

	Breakpoint *bp, *bp_next, *bp_prev;
	int i = 0, found = 0;
	int type;
	int idx = atoi(argv[1]);

	// Find breakpoint with given index
	bp_prev = NULL;
	bp = _vm->_gamestate->bp_list;
	while (bp && i < idx) {
		bp_prev = bp;
		bp = bp->next;
		i++;
	}
	if (!bp) {
		DebugPrintf("Invalid breakpoint index %i\n", idx);
		return true;
	}

	// Delete it
	bp_next = bp->next;
	type = bp->type;
	if (type == BREAK_SELECTOR) free(bp->data.name);
	free(bp);
	if (bp_prev)
		bp_prev->next = bp_next;
	else
		_vm->_gamestate->bp_list = bp_next;

	// Check if there are more breakpoints of the same type. If not, clear
	// the respective bit in s->have_bp.
	for (bp = _vm->_gamestate->bp_list; bp; bp = bp->next) {
		if (bp->type == type) {
			found = 1;
			break;
		}
	}

	if (!found)
		_vm->_gamestate->have_bp &= ~type;

	return true;
}

bool Console::cmdBreakpointExecMethod(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Sets a breakpoint on the execution of the specified method.\n");
		DebugPrintf("Usage: %s <method name>\n", argv[0]);
		DebugPrintf("Example: %s ego::doit\n", argv[0]);
		DebugPrintf("May also be used to set a breakpoint that applies whenever an object\n");
		DebugPrintf("of a specific type is touched: %s foo::\n", argv[0]);
		return true;
	}

	/* Note: We can set a breakpoint on a method that has not been loaded yet.
	   Thus, we can't check whether the command argument is a valid method name.
	   A breakpoint set on an invalid method name will just never trigger. */
	Breakpoint *bp;
	if (_vm->_gamestate->bp_list) {
		// List exists, append the breakpoint to the end
		bp = _vm->_gamestate->bp_list;
		while (bp->next)
			bp = bp->next;
		bp->next = (Breakpoint *)malloc(sizeof(Breakpoint));
		bp = bp->next;
	} else {
		// No list, so create the list head
		_vm->_gamestate->bp_list = (Breakpoint *)malloc(sizeof(Breakpoint));
		bp = _vm->_gamestate->bp_list;
	}
	bp->next = NULL;
	bp->type = BREAK_SELECTOR;
	bp->data.name = (char *)malloc(strlen(argv[1]) + 1);
	strcpy(bp->data.name, argv[1]);
	_vm->_gamestate->have_bp |= BREAK_SELECTOR;

	return true;
}

bool Console::cmdBreakpointExecFunction(int argc, const char **argv) {
	// TODO/FIXME: Why does this accept 2 parameters (the high and the low part of the address)?"
	if (argc != 3) {
		DebugPrintf("Sets a breakpoint on the execution of the specified exported function.\n");
		DebugPrintf("Usage: %s <addr1> <addr2>\n", argv[0]);
		return true;
	}

	/* Note: We can set a breakpoint on a method that has not been loaded yet.
	   Thus, we can't check whether the command argument is a valid method name.
	   A breakpoint set on an invalid method name will just never trigger. */
	Breakpoint *bp;
	if (_vm->_gamestate->bp_list) {
		// List exists, append the breakpoint to the end
		bp = _vm->_gamestate->bp_list;
		while (bp->next)
			bp = bp->next;
		bp->next = (Breakpoint *)malloc(sizeof(Breakpoint));
		bp = bp->next;
	} else {
		// No list, so create the list head
		_vm->_gamestate->bp_list = (Breakpoint *)malloc(sizeof(Breakpoint));
		bp = _vm->_gamestate->bp_list;
	}
	bp->next = NULL;
	bp->type = BREAK_EXPORT;
	bp->data.address = (atoi(argv[1]) << 16 | atoi(argv[2]));
	_vm->_gamestate->have_bp |= BREAK_EXPORT;

	return true;
}

bool Console::cmdIsSample(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Tests whether a given sound resource is a PCM sample, \n");
		DebugPrintf("and displays information on it if it is.\n");
		DebugPrintf("Usage: %s <sample id>\n", argv[0]);
		return true;
	}

	Resource *song = _vm->getResourceManager()->findResource(ResourceId(kResourceTypeSound, atoi(argv[1])), 0);
	SongIterator *songit;
	Audio::AudioStream *data;

	if (!song) {
		DebugPrintf("Not a sound resource!\n");
		return true;
	}

	songit = songit_new(song->data, song->size, SCI_SONG_ITERATOR_TYPE_SCI0, 0xcaffe /* What do I care about the ID? */);

	if (!songit) {
		DebugPrintf("Could not convert to song iterator!\n");
		return true;
	}

	if ((data = songit->getAudioStream())) {
		// TODO
/*
		DebugPrintf("\nIs sample (encoding %dHz/%s/%04x)", data->conf.rate, (data->conf.stereo) ?
		          ((data->conf.stereo == SFX_PCM_STEREO_LR) ? "stereo-LR" : "stereo-RL") : "mono", data->conf.format);
*/
		delete data;
	} else
		DebugPrintf("Valid song, but not a sample.\n");

	delete songit;

	return true;
}

bool Console::cmdSfx01Header(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Dumps the header of a SCI01 song\n");
		DebugPrintf("Usage: %s <track>\n", argv[0]);
		return true;
	}

	Resource *song = _vm->getResourceManager()->findResource(ResourceId(kResourceTypeSound, atoi(argv[1])), 0);

	if (!song) {
		DebugPrintf("Doesn't exist\n");
		return true;
	}

	uint32 offset = 0;

	DebugPrintf("SCI01 song track mappings:\n");

	if (*song->data == 0xf0) // SCI1 priority spec
		offset = 8;

	if (song->size <= 0)
		return 1;

	while (song->data[offset] != 0xff) {
		byte device_id = song->data[offset];
		DebugPrintf("* Device %02x:\n", device_id);
		offset++;

		if (offset + 1 >= song->size)
			return 1;

		while (song->data[offset] != 0xff) {
			int track_offset;
			int end;
			byte header1, header2;

			if (offset + 7 >= song->size)
				return 1;

			offset += 2;

			track_offset = READ_LE_UINT16(song->data + offset);
			header1 = song->data[track_offset];
			header2 = song->data[track_offset+1];
			track_offset += 2;

			end = READ_LE_UINT16(song->data + offset + 2);
			DebugPrintf("  - %04x -- %04x", track_offset, track_offset + end);

			if (track_offset == 0xfe)
				DebugPrintf(" (PCM data)\n");
			else
				DebugPrintf(" (channel %d, special %d, %d playing notes, %d foo)\n",
				          header1 & 0xf, header1 >> 4, header2 & 0xf, header2 >> 4);
			offset += 4;
		}
		offset++;
	}

	return true;
}

static int _parse_ticks(byte *data, int *offset_p, int size) {
	int ticks = 0;
	int tempticks;
	int offset = 0;

	do {
		tempticks = data[offset++];
		ticks += (tempticks == SCI_MIDI_TIME_EXPANSION_PREFIX) ? SCI_MIDI_TIME_EXPANSION_LENGTH : tempticks;
	} while (tempticks == SCI_MIDI_TIME_EXPANSION_PREFIX && offset < size);

	if (offset_p)
		*offset_p = offset;

	return ticks;
}

// Specialised for SCI01 tracks (this affects the way cumulative cues are treated)
static void midi_hexdump(byte *data, int size, int notational_offset) {
	int offset = 0;
	int prev = 0;
	const int MIDI_cmdlen[16] = {0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 2, 0};

	if (*data == 0xf0) // SCI1 priority spec
		offset = 8;

	while (offset < size) {
		int old_offset = offset;
		int offset_mod;
		int time = _parse_ticks(data + offset, &offset_mod, size);
		int cmd;
		int pleft;
		int firstarg = 0;
		int i;
		int blanks = 0;

		offset += offset_mod;
		printf("  [%04x] %d\t",
		        old_offset + notational_offset, time);

		cmd = data[offset];
		if (!(cmd & 0x80)) {
			cmd = prev;
			if (prev < 0x80) {
				printf("Track broken at %x after"
				        " offset mod of %d\n",
				        offset + notational_offset, offset_mod);
				Common::hexdump(data, size, 16, notational_offset);
				return;
			}
			printf("(rs %02x) ", cmd);
			blanks += 8;
		} else {
			++offset;
			printf("%02x ", cmd);
			blanks += 3;
		}
		prev = cmd;

		pleft = MIDI_cmdlen[cmd >> 4];
		if (SCI_MIDI_CONTROLLER(cmd) && data[offset] == SCI_MIDI_CUMULATIVE_CUE)
			--pleft; // This is SCI(0)1 specific

		for (i = 0; i < pleft; i++) {
			if (i == 0)
				firstarg = data[offset];
			printf("%02x ", data[offset++]);
			blanks += 3;
		}

		while (blanks < 16) {
			blanks += 4;
			printf("    ");
		}

		while (blanks < 20) {
			++blanks;
			printf(" ");
		}

		if (cmd == SCI_MIDI_EOT)
			printf(";; EOT");
		else if (cmd == SCI_MIDI_SET_SIGNAL) {
			if (firstarg == SCI_MIDI_SET_SIGNAL_LOOP)
				printf(";; LOOP point");
			else
				printf(";; CUE (%d)", firstarg);
		} else if (SCI_MIDI_CONTROLLER(cmd)) {
			if (firstarg == SCI_MIDI_CUMULATIVE_CUE)
				printf(";; CUE (cumulative)");
			else if (firstarg == SCI_MIDI_RESET_ON_SUSPEND)
				printf(";; RESET-ON-SUSPEND flag");
		}
		printf("\n");

		if (old_offset >= offset) {
			printf("-- Not moving forward anymore,"
			        " aborting (%x/%x)\n", offset, old_offset);
			return;
		}
	}
}

bool Console::cmdSfx01Track(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Dumps a track of a SCI01 song\n");
		DebugPrintf("Usage: %s <track> <offset>\n", argv[0]);
		return true;
	}

	Resource *song = _vm->getResourceManager()->findResource(ResourceId(kResourceTypeSound, atoi(argv[1])), 0);

	int offset = atoi(argv[2]);

	if (!song) {
		DebugPrintf("Doesn't exist\n");
		return true;
	}

	midi_hexdump(song->data + offset, song->size, offset);

	return true;
}

bool Console::cmdStopSfx(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Stops a playing sound\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Where <address> is the address of the sound to stop.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t id;
	
	if (parse_reg_t(_vm->_gamestate, argv[1], &id)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	int handle = id.segment << 16 | id.offset;	// frobnicate handle

	if (id.segment) {
		SegManager *segMan = _vm->_gamestate->_segMan;	// for PUT_SEL32V
		_vm->_gamestate->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
		_vm->_gamestate->_sound.sfx_remove_song(handle);
		PUT_SEL32V(segMan, id, signal, SIGNAL_OFFSET);
		PUT_SEL32V(segMan, id, nodePtr, 0);
		PUT_SEL32V(segMan, id, handle, 0);
	}

	return true;
}

bool Console::cmdExit(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("%s game - exit gracefully\n", argv[0]);
		DebugPrintf("%s now - exit ungracefully\n", argv[0]);
		return true;
	}

	if (!scumm_stricmp(argv[1], "game")) {
		// Quit gracefully
		script_abort_flag = 1; // Terminate VM
		g_debugState.seeking = kDebugSeekNothing;
		g_debugState.runningStep = 0;

	} else if (!scumm_stricmp(argv[1], "now")) {
		// Quit ungracefully
		exit(0);
	}

	return false;
}

bool Console::cmdAddresses(int argc, const char **argv) {
	DebugPrintf("Address parameters may be passed in one of three forms:\n");
	DebugPrintf(" - ssss:oooo -- where 'ssss' denotes a segment and 'oooo' an offset.\n");
	DebugPrintf("   Example: \"a:c5\" would address something in segment 0xa at offset 0xc5.\n");
	DebugPrintf(" - &scr:oooo -- where 'scr' is a script number and oooo an offset within that script; will\n");
	DebugPrintf("   fail if the script is not currently loaded\n");
	DebugPrintf(" - $REG -- where 'REG' is one of 'PC', 'ACC', 'PREV' or 'OBJ': References the address\n");
	DebugPrintf("   indicated by the register of this name.\n");
	DebugPrintf(" - $REG+n (or -n) -- Like $REG, but modifies the offset part by a specific amount (which\n");
	DebugPrintf("   is specified in hexadecimal).\n");
	DebugPrintf(" - ?obj -- Looks up an object with the specified name, uses its address. This will abort if\n");
	DebugPrintf("   the object name is ambiguous; in that case, a list of addresses and indices is provided.\n");
	DebugPrintf("   ?obj.idx may be used to disambiguate 'obj' by the index 'idx'.\n");

	return true;
}

// Returns 0 on success
static int parse_reg_t(EngineState *s, const char *str, reg_t *dest) {
	// Pointer to the part of str which contains a numeric offset (if any)
	const char *offsetStr = NULL;

	// Flag that tells whether the value stored in offsetStr is an absolute offset,
	// or a relative offset against dest->offset.
	bool relativeOffset = false;

	// Non-NULL: Parse end of string for relative offsets
	char *endptr;

	if (*str == '$') { // Register: "$FOO" or "$FOO+NUM" or "$FOO-NUM
		relativeOffset = true;

		if (!scumm_strnicmp(str + 1, "PC", 2)) {
			*dest = s->_executionStack.back().addr.pc;
			offsetStr = str + 3;
		} else if (!scumm_strnicmp(str + 1, "P", 1)) {
			*dest = s->_executionStack.back().addr.pc;
			offsetStr = str + 2;
		} else if (!scumm_strnicmp(str + 1, "PREV", 4)) {
			*dest = s->r_prev;
			offsetStr = str + 5;
		} else if (!scumm_strnicmp(str + 1, "ACC", 3)) {
			*dest = s->r_acc;
			offsetStr = str + 4;
		} else if (!scumm_strnicmp(str + 1, "A", 1)) {
			*dest = s->r_acc;
			offsetStr = str + 2;
		} else if (!scumm_strnicmp(str + 1, "OBJ", 3)) {
			*dest = s->_executionStack.back().objp;
			offsetStr = str + 4;
		} else if (!scumm_strnicmp(str + 1, "O", 1)) {
			*dest = s->_executionStack.back().objp;
			offsetStr = str + 2;
		} else
			return 1; // No matching register

		if (!*offsetStr)
			offsetStr = NULL;
		else if (*offsetStr != '+' && *offsetStr != '-')
			return 1;
	} else if (*str == '&') { // Script relative: "&SCRIPT-ID:OFFSET"
		// Look up by script ID. The text from start till just before the colon
		// (resp. end of string, if there is no colon) contains the script ID.
		const char *colon = strchr(str, ':');
		if (!colon)
			return 1;

		// Extract the script id and parse it
		Common::String scriptStr(str, colon);
		int script_nr = strtol(scriptStr.c_str() + 1, &endptr, 10);
		if (*endptr)
			return 1;

		// Now lookup the script's segment
		dest->segment = s->_segMan->getScriptSegment(script_nr);
		if (!dest->segment) {
			return 1;
		}

		// Finally, after the colon comes the offset
		offsetStr = colon + 1;
	} else if (*str == '?') {	// Object by name: "?OBJ" or "?OBJ.INDEX" or "?OBJ.INDEX+OFFSET" or "?OBJ.INDEX-OFFSET"
		// The (optional) index can be used to distinguish multiple object with the same name.
		int index = -1;

		// Skip over the leading question mark '?'
		str++;

		// Look for an offset. It starts with + or -
		relativeOffset = true;
		offsetStr = strchr(str, '+');
		if (!offsetStr)	// No + found, look for -
			offsetStr = strchr(str, '-');

		// Strip away the offset and the leading '?'
		Common::String str_objname;
		if (offsetStr)
			str_objname = Common::String(str, offsetStr);
		else
			str_objname = str;


		// Scan for a period, after which (if present) we'll find an index
		const char *tmp = Common::find(str_objname.begin(), str_objname.end(), '.');
		if (tmp != str_objname.end()) {
			index = strtol(tmp + 1, &endptr, 16);
			if (*endptr)
				return -1;
			// Chop of the index
			str_objname = Common::String(str_objname.c_str(), tmp);
		}

		// Now all values are available; iterate over all objects.

		*dest = s->_segMan->findObjectByName(str_objname, index);
		if (dest->isNull())
			return 1;

	} else {	// Finally, check for "SEGMENT:OFFSET" or just "OFFSET"
		const char *colon = strchr(str, ':');

		if (!colon) {
			// No segment specified
			offsetStr = str;
			dest->segment = 0;
		} else {
			offsetStr = colon + 1;

			Common::String segmentStr(str, colon);
			dest->segment = strtol(segmentStr.c_str(), &endptr, 16);
			if (*endptr)
				return 1;
		}
	}
	if (offsetStr) {
		int val = strtol(offsetStr, &endptr, 16);

		if (relativeOffset)
			dest->offset += val;
		else
			dest->offset = val;

		if (*endptr)
			return 1;
	}

	return 0;
}

void Console::printList(List *l) {
	reg_t pos = l->first;
	reg_t my_prev = NULL_REG;

	DebugPrintf("\t<\n");

	while (!pos.isNull()) {
		Node *node;
		NodeTable *nt = (NodeTable *)GET_SEGMENT(*_vm->_gamestate->_segMan, pos.segment, SEG_TYPE_NODES);

		if (!nt || !nt->isValidEntry(pos.offset)) {
			DebugPrintf("   WARNING: %04x:%04x: Doesn't contain list node!\n",
			          PRINT_REG(pos));
			return;
		}

		node = &(nt->_table[pos.offset]);

		DebugPrintf("\t%04x:%04x  : %04x:%04x -> %04x:%04x\n", PRINT_REG(pos), PRINT_REG(node->key), PRINT_REG(node->value));

		if (my_prev != node->pred)
			DebugPrintf("   WARNING: current node gives %04x:%04x as predecessor!\n",
			          PRINT_REG(node->pred));

		my_prev = pos;
		pos = node->succ;
	}

	if (my_prev != l->last)
		DebugPrintf("   WARNING: Last node was expected to be %04x:%04x, was %04x:%04x!\n",
		          PRINT_REG(l->last), PRINT_REG(my_prev));
	DebugPrintf("\t>\n");
}

int Console::printNode(reg_t addr) {
	SegmentObj *mobj = GET_SEGMENT(*_vm->_gamestate->_segMan, addr.segment, SEG_TYPE_LISTS);

	if (mobj) {
		ListTable *lt = (ListTable *)mobj;
		List *list;

		if (!lt->isValidEntry(addr.offset)) {
			DebugPrintf("Address does not contain a list\n");
			return 1;
		}

		list = &(lt->_table[addr.offset]);

		DebugPrintf("%04x:%04x : first x last = (%04x:%04x, %04x:%04x)\n", PRINT_REG(addr), PRINT_REG(list->first), PRINT_REG(list->last));
	} else {
		NodeTable *nt;
		Node *node;
		mobj = GET_SEGMENT(*_vm->_gamestate->_segMan, addr.segment, SEG_TYPE_NODES);

		if (!mobj) {
			DebugPrintf("Segment #%04x is not a list or node segment\n", addr.segment);
			return 1;
		}

		nt = (NodeTable *)mobj;

		if (!nt->isValidEntry(addr.offset)) {
			DebugPrintf("Address does not contain a node\n");
			return 1;
		}
		node = &(nt->_table[addr.offset]);

		DebugPrintf("%04x:%04x : prev x next = (%04x:%04x, %04x:%04x); maps %04x:%04x -> %04x:%04x\n",
		          PRINT_REG(addr), PRINT_REG(node->pred), PRINT_REG(node->succ), PRINT_REG(node->key), PRINT_REG(node->value));
	}

	return 0;
}

int Console::printObject(reg_t pos) {
	EngineState *s = _vm->_gamestate;	// for the several defines in this function
	Object *obj = s->_segMan->getObject(pos);
	Object *var_container = obj;
	uint i;

	if (!obj) {
		DebugPrintf("[%04x:%04x]: Not an object.", PRINT_REG(pos));
		return 1;
	}

	// Object header
	DebugPrintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(pos), s->_segMan->getObjectName(pos),
				obj->getVarCount(), obj->getMethodCount());

	if (!(obj->getInfoSelector().offset & SCRIPT_INFO_CLASS))
		var_container = s->_segMan->getObject(obj->getSuperClassSelector());
	DebugPrintf("  -- member variables:\n");
	for (i = 0; (uint)i < obj->getVarCount(); i++) {
		printf("    ");
		if (i < var_container->getVarCount()) {
			uint16 varSelector = var_container->getVarSelector(i);
			DebugPrintf("[%03x] %s = ", varSelector, selector_name(s, varSelector));
		} else
			DebugPrintf("p#%x = ", i);

		reg_t val = obj->getVariable(i);
		DebugPrintf("%04x:%04x", PRINT_REG(val));

		Object *ref = s->_segMan->getObject(val);
		if (ref)
			DebugPrintf(" (%s)", s->_segMan->getObjectName(val));

		DebugPrintf("\n");
	}
	DebugPrintf("  -- methods:\n");
	for (i = 0; i < obj->getMethodCount(); i++) {
		reg_t fptr = obj->getFunction(i);
		DebugPrintf("    [%03x] %s = %04x:%04x\n", obj->getFuncSelector(i), selector_name(s, obj->getFuncSelector(i)), PRINT_REG(fptr));
	}
	if (s->_segMan->_heap[pos.segment]->getType() == SEG_TYPE_SCRIPT)
		DebugPrintf("\nOwner script:\t%d\n", s->_segMan->getScript(pos.segment)->_nr);

	return 0;
}

#define GETRECT(ll, rr, tt, bb) \
	ll = GET_SELECTOR(pos, ll); \
	rr = GET_SELECTOR(pos, rr); \
	tt = GET_SELECTOR(pos, tt); \
	bb = GET_SELECTOR(pos, bb);

#if 0
// TODO Re-implement this
static void viewobjinfo(EngineState *s, HeapPtr pos) {
	char *signals[16] = {
		"stop_update",
		"updated",
		"no_update",
		"hidden",
		"fixed_priority",
		"always_update",
		"force_update",
		"remove",
		"frozen",
		"is_extra",
		"hit_obstacle",
		"doesnt_turn",
		"no_cycler",
		"ignore_horizon",
		"ignore_actor",
		"dispose!"
	};

	int x, y, z, priority;
	int cel, loop, view, signal;
	int nsLeft, nsRight, nsBottom, nsTop;
	int lsLeft, lsRight, lsBottom, lsTop;
	int brLeft, brRight, brBottom, brTop;
	int i;
	int have_rects = 0;
	Common::Rect nsrect, nsrect_clipped, brrect;

	if (lookup_selector(s->_segMan, pos, s->_kernel->_selectorCache.nsBottom, NULL) == kSelectorVariable) {
		GETRECT(nsLeft, nsRight, nsBottom, nsTop);
		GETRECT(lsLeft, lsRight, lsBottom, lsTop);
		GETRECT(brLeft, brRight, brBottom, brTop);
		have_rects = 1;
	}

	GETRECT(view, loop, signal, cel);

	printf("\n-- View information:\ncel %d/%d/%d at ", view, loop, cel);

	x = GET_SELECTOR(pos, x);
	y = GET_SELECTOR(pos, y);
	priority = GET_SELECTOR(pos, priority);
	if (s->_kernel->_selectorCache.z > 0) {
		z = GET_SELECTOR(pos, z);
		printf("(%d,%d,%d)\n", x, y, z);
	} else
		printf("(%d,%d)\n", x, y);

	if (priority == -1)
		printf("No priority.\n\n");
	else
		printf("Priority = %d (band starts at %d)\n\n", priority, PRIORITY_BAND_FIRST(priority));

	if (have_rects) {
		printf("nsRect: [%d..%d]x[%d..%d]\n", nsLeft, nsRight, nsTop, nsBottom);
		printf("lsRect: [%d..%d]x[%d..%d]\n", lsLeft, lsRight, lsTop, lsBottom);
		printf("brRect: [%d..%d]x[%d..%d]\n", brLeft, brRight, brTop, brBottom);
	}

	nsrect = get_nsrect(s, pos, 0);
	nsrect_clipped = get_nsrect(s, pos, 1);
	//brrect = set_base(s, pos);
	printf("new nsRect: [%d..%d]x[%d..%d]\n", nsrect.x, nsrect.xend, nsrect.y, nsrect.yend);
	printf("new clipped nsRect: [%d..%d]x[%d..%d]\n", nsrect_clipped.x, nsrect_clipped.xend, nsrect_clipped.y, nsrect_clipped.yend);
	printf("new brRect: [%d..%d]x[%d..%d]\n", brrect.x, brrect.xend, brrect.y, brrect.yend);
	printf("\n signals = %04x:\n", signal);

	for (i = 0; i < 16; i++)
		if (signal & (1 << i))
			printf("  %04x: %s\n", 1 << i, signals[i]);
}
#endif
#undef GETRECT

#define GETRECT(ll, rr, tt, bb) \
	ll = GET_SELECTOR(pos, ll); \
	rr = GET_SELECTOR(pos, rr); \
	tt = GET_SELECTOR(pos, tt); \
	bb = GET_SELECTOR(pos, bb);

#if 0
// Draws the nsRect and brRect of a dynview object. nsRect is green, brRect is blue.
// TODO: Re-implement this
static int c_gfx_draw_viewobj(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	HeapPtr pos = (HeapPtr)(cmdParams[0].val);
	int is_view;
	int x, y, priority;
	int nsLeft, nsRight, nsBottom, nsTop;
	int brLeft, brRight, brBottom, brTop;

	if (!s) {
		printf("Not in debug state!\n");
		return 1;
	}

	if ((pos < 4) || (pos > 0xfff0)) {
		printf("Invalid address.\n");
		return 1;
	}

	if (((int16)READ_LE_UINT16(s->heap + pos + SCRIPT_OBJECT_MAGIC_OFFSET)) != SCRIPT_OBJECT_MAGIC_NUMBER) {
		printf("Not an object.\n");
		return 0;
	}


	is_view = (lookup_selector(s->_segMan, pos, s->_kernel->_selectorCache.x, NULL) == kSelectorVariable) &&
	    (lookup_selector(s->_segMan, pos, s->_kernel->_selectorCache.brLeft, NULL) == kSelectorVariable) &&
	    (lookup_selector(s->_segMan, pos, s->_kernel->_selectorCache.signal, NULL) == kSelectorVariable) &&
	    (lookup_selector(s->_segMan, pos, s->_kernel->_selectorCache.nsTop, NULL) == kSelectorVariable);

	if (!is_view) {
		printf("Not a dynamic View object.\n");
		return 0;
	}

	x = GET_SELECTOR(pos, x);
	y = GET_SELECTOR(pos, y);
	priority = GET_SELECTOR(pos, priority);
	GETRECT(brLeft, brRight, brBottom, brTop);
	GETRECT(nsLeft, nsRight, nsBottom, nsTop);
	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);

	brTop += 10;
	brBottom += 10;
	nsTop += 10;
	nsBottom += 10;

	gfxop_fill_box(s->gfx_state, gfx_rect(nsLeft, nsTop, nsRight - nsLeft + 1, nsBottom - nsTop + 1), s->ega_colors[2]);
	gfxop_fill_box(s->gfx_state, gfx_rect(brLeft, brTop, brRight - brLeft + 1, brBottom - brTop + 1), s->ega_colors[1]);
	gfxop_fill_box(s->gfx_state, gfx_rect(x - 1, y - 1, 3, 3), s->ega_colors[0]);
	gfxop_fill_box(s->gfx_state, gfx_rect(x - 1, y, 3, 1), s->ega_colors[priority]);
	gfxop_fill_box(s->gfx_state, gfx_rect(x, y - 1, 1, 3), s->ega_colors[priority]);
	gfxop_update(s->gfx_state);

	return 0;
}
#endif
#undef GETRECT

#if 0
// Executes one operation skipping over sends
// TODO Re-implement this
int c_stepover(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	int opcode, opnumber;

	opcode = s->_heap[*p_pc];
	opnumber = opcode >> 1;
	if (opnumber == 0x22 /* callb */ || opnumber == 0x23 /* calle */ ||
	        opnumber == 0x25 /* send */ || opnumber == 0x2a /* self */ || opnumber == 0x2b /* super */) {
		g_debugState.seeking = kDebugSeekSO;
		g_debugState.seekLevel = s->_executionStack.size()-1;
		// Store in g_debugState.seekSpecial the offset of the next command after send
		switch (opcode) {
		case 0x46: // calle W
			g_debugState.seekSpecial = *p_pc + 5;
			break;

		case 0x44: // callb W
		case 0x47: // calle B
		case 0x56: // super W
			g_debugState.seekSpecial = *p_pc + 4;
			break;

		case 0x45: // callb B
		case 0x57: // super B
		case 0x4A: // send W
		case 0x54: // self W
			g_debugState.seekSpecial = *p_pc + 3;
			break;

		default:
			g_debugState.seekSpecial = *p_pc + 2;
		}
	}

	return 0;
}
#endif

#ifdef GFXW_DEBUG_WIDGETS
extern GfxWidget *debug_widgets[];
extern int debug_widget_pos;

// If called with no parameters, it shows which widgets are active
// With parameters, it lists the widget corresponding to the numerical index specified (for each parameter).
bool Console::cmdPrintWidget(int argc, const char **argv) {
	if (argc > 1) {
		for (int i = 0; i < argc; i++) {
			int widget_nr = atoi(argv[1]);

			DebugPrintf("===== Widget #%d:\n", widget_nr);
			debug_widgets[widget_nr]->print(0);
		}
	} else if (debug_widget_pos > 1) {
		DebugPrintf("Widgets 0-%d are active\n", debug_widget_pos - 1);
	} else if (debug_widget_pos == 1) {
		DebugPrintf("Widget 0 is active\n");
	} else {
		DebugPrintf("No widgets are active\n");
	}

	return true;
}
#endif

} // End of namespace Sci
