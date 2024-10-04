/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Console module

#include "common/md5.h"
#include "sci/sci.h"
#include "sci/console.h"
#include "sci/debug.h"
#include "sci/event.h"
#include "sci/resource/resource.h"
#include "sci/version.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/savegame.h"
#include "sci/engine/gc.h"
#include "sci/engine/features.h"
#include "sci/engine/scriptdebug.h"
#include "sci/sound/midiparser_sci.h"
#include "sci/sound/music.h"
#include "sci/sound/drivers/mididriver.h"
#include "sci/sound/drivers/map-mt32-to-gm.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/view.h"

#include "sci/parser/vocabulary.h"

#include "video/avi_decoder.h"
#include "sci/video/seq_decoder.h"
#ifdef ENABLE_SCI32
#include "common/memstream.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/palette32.h"
#include "sci/sound/decoders/sol.h"
#include "video/coktel_decoder.h"
#endif

#include "common/file.h"
#include "common/savefile.h"

#include "engines/util.h"

namespace Sci {

int g_debug_sleeptime_factor = 1;
int g_debug_simulated_key = 0;
bool g_debug_track_mouse_clicks = false;

// Refer to the "addresses" command on how to pass address parameters
static int parse_reg_t(EngineState *s, const char *str, reg_t *dest);

Console::Console(SciEngine *engine) : GUI::Debugger(),
	_engine(engine), _debugState(engine->_debugState), _videoFrameDelay(0),
	_gameFlagsGlobal(_engine->_features->getGameFlagsGlobal()) {

	assert(_engine);
	assert(_engine->_gamestate);

	// Variables
	registerVar("sleeptime_factor",	&g_debug_sleeptime_factor);
	registerVar("gc_interval",		&engine->_gamestate->scriptGCInterval);
	registerVar("simulated_key",		&g_debug_simulated_key);
	registerVar("track_mouse_clicks",	&g_debug_track_mouse_clicks);
	registerCmd("speed_throttle",   WRAP_METHOD(Console, cmdSpeedThrottle));

	// General
	registerCmd("help",				WRAP_METHOD(Console, cmdHelp));
	// Kernel
	registerCmd("opcodes",			WRAP_METHOD(Console, cmdOpcodes));
	registerCmd("selector",			WRAP_METHOD(Console, cmdSelector));
	registerCmd("selectors",			WRAP_METHOD(Console, cmdSelectors));
	registerCmd("kernfunctions",		WRAP_METHOD(Console, cmdKernelFunctions));
	registerCmd("functions",		WRAP_METHOD(Console, cmdKernelFunctions));	// alias
	registerCmd("kerncall", 		WRAP_METHOD(Console, cmdKernelCall));
	registerCmd("kc",				WRAP_METHOD(Console, cmdKernelCall));	// alias
	registerCmd("class_table",		WRAP_METHOD(Console, cmdClassTable));
	// Parser
	registerCmd("suffixes",			WRAP_METHOD(Console, cmdSuffixes));
	registerCmd("parse_grammar",		WRAP_METHOD(Console, cmdParseGrammar));
	registerCmd("parser_nodes",		WRAP_METHOD(Console, cmdParserNodes));
	registerCmd("parser_words",		WRAP_METHOD(Console, cmdParserWords));
	registerCmd("sentence_fragments",	WRAP_METHOD(Console, cmdSentenceFragments));
	registerCmd("parse",				WRAP_METHOD(Console, cmdParse));
	registerCmd("set_parse_nodes",	WRAP_METHOD(Console, cmdSetParseNodes));
	registerCmd("said",				WRAP_METHOD(Console, cmdSaid));
	// Resources
	registerCmd("diskdump",			WRAP_METHOD(Console, cmdDiskDump));
	registerCmd("hexdump",			WRAP_METHOD(Console, cmdHexDump));
	registerCmd("resource_id",		WRAP_METHOD(Console, cmdResourceId));
	registerCmd("resource_info",		WRAP_METHOD(Console, cmdResourceInfo));
	registerCmd("resource_types",		WRAP_METHOD(Console, cmdResourceTypes));
	registerCmd("list",				WRAP_METHOD(Console, cmdList));
	registerCmd("alloc_list",				WRAP_METHOD(Console, cmdAllocList));
	registerCmd("hexgrep",			WRAP_METHOD(Console, cmdHexgrep));
	registerCmd("verify_scripts",		WRAP_METHOD(Console, cmdVerifyScripts));
	registerCmd("integrity_dump",	WRAP_METHOD(Console, cmdResourceIntegrityDump));
	// Game
	registerCmd("save_game",			WRAP_METHOD(Console, cmdSaveGame));
	registerCmd("restore_game",		WRAP_METHOD(Console, cmdRestoreGame));
	registerCmd("restart_game",		WRAP_METHOD(Console, cmdRestartGame));
	registerCmd("version",			WRAP_METHOD(Console, cmdGetVersion));
	registerCmd("room",				WRAP_METHOD(Console, cmdRoomNumber));
	registerCmd("quit",				WRAP_METHOD(Console, cmdQuit));
	registerCmd("list_saves",			WRAP_METHOD(Console, cmdListSaves));
	// Graphics
	registerCmd("show_map",			WRAP_METHOD(Console, cmdShowMap));
	registerCmd("set_palette",		WRAP_METHOD(Console, cmdSetPalette));
	registerCmd("draw_pic",			WRAP_METHOD(Console, cmdDrawPic));
	registerCmd("draw_cel",			WRAP_METHOD(Console, cmdDrawCel));
	registerCmd("undither",           WRAP_METHOD(Console, cmdUndither));
	registerCmd("pic_visualize",		WRAP_METHOD(Console, cmdPicVisualize));
	registerCmd("play_video",         WRAP_METHOD(Console, cmdPlayVideo));
	registerCmd("animate_list",       WRAP_METHOD(Console, cmdAnimateList));
	registerCmd("al",                 WRAP_METHOD(Console, cmdAnimateList));	// alias
	registerCmd("window_list",        WRAP_METHOD(Console, cmdWindowList));
	registerCmd("wl",                 WRAP_METHOD(Console, cmdWindowList));	// alias
	registerCmd("plane_list",         WRAP_METHOD(Console, cmdPlaneList));
	registerCmd("pl",                 WRAP_METHOD(Console, cmdPlaneList));	// alias
	registerCmd("visible_plane_list", WRAP_METHOD(Console, cmdVisiblePlaneList));
	registerCmd("vpl",                WRAP_METHOD(Console, cmdVisiblePlaneList));	// alias
	registerCmd("plane_items",        WRAP_METHOD(Console, cmdPlaneItemList));
	registerCmd("pi",                 WRAP_METHOD(Console, cmdPlaneItemList));	// alias
	registerCmd("visible_plane_items", WRAP_METHOD(Console, cmdVisiblePlaneItemList));
	registerCmd("vpi",                WRAP_METHOD(Console, cmdVisiblePlaneItemList));	// alias
	registerCmd("saved_bits",         WRAP_METHOD(Console, cmdSavedBits));
	registerCmd("show_saved_bits",    WRAP_METHOD(Console, cmdShowSavedBits));
	// Segments
	registerCmd("segment_table",		WRAP_METHOD(Console, cmdPrintSegmentTable));
	registerCmd("segtable",			WRAP_METHOD(Console, cmdPrintSegmentTable));	// alias
	registerCmd("segment_info",		WRAP_METHOD(Console, cmdSegmentInfo));
	registerCmd("seginfo",			WRAP_METHOD(Console, cmdSegmentInfo));			// alias
	registerCmd("segment_kill",		WRAP_METHOD(Console, cmdKillSegment));
	registerCmd("segkill",			WRAP_METHOD(Console, cmdKillSegment));			// alias
	// Garbage collection
	registerCmd("gc",					WRAP_METHOD(Console, cmdGCInvoke));
	registerCmd("gc_objects",			WRAP_METHOD(Console, cmdGCObjects));
	registerCmd("gc_reachable",		WRAP_METHOD(Console, cmdGCShowReachable));
	registerCmd("gc_freeable",		WRAP_METHOD(Console, cmdGCShowFreeable));
	registerCmd("gc_normalize",		WRAP_METHOD(Console, cmdGCNormalize));
	// Music/SFX
	registerCmd("songlib",			WRAP_METHOD(Console, cmdSongLib));
	registerCmd("songinfo",			WRAP_METHOD(Console, cmdSongInfo));
	registerCmd("is_sample",			WRAP_METHOD(Console, cmdIsSample));
	registerCmd("startsound",			WRAP_METHOD(Console, cmdStartSound));
	registerCmd("togglesound",		WRAP_METHOD(Console, cmdToggleSound));
	registerCmd("stopallsounds",		WRAP_METHOD(Console, cmdStopAllSounds));
	registerCmd("sfx01_header",		WRAP_METHOD(Console, cmdSfx01Header));
	registerCmd("sfx01_track",		WRAP_METHOD(Console, cmdSfx01Track));
	registerCmd("show_instruments",	WRAP_METHOD(Console, cmdShowInstruments));
	registerCmd("map_instrument",		WRAP_METHOD(Console, cmdMapInstrument));
	registerCmd("audio_list",		WRAP_METHOD(Console, cmdAudioList));
	registerCmd("audio_dump",		WRAP_METHOD(Console, cmdAudioDump));
	// Script
	registerCmd("addresses",			WRAP_METHOD(Console, cmdAddresses));
	registerCmd("registers",			WRAP_METHOD(Console, cmdRegisters));
	registerCmd("reg",					WRAP_METHOD(Console, cmdRegisters));
	registerCmd("dissect_script",		WRAP_METHOD(Console, cmdDissectScript));
	registerCmd("backtrace",			WRAP_METHOD(Console, cmdBacktrace));
	registerCmd("bt",					WRAP_METHOD(Console, cmdBacktrace));	// alias
	registerCmd("trace",				WRAP_METHOD(Console, cmdTrace));
	registerCmd("t",					WRAP_METHOD(Console, cmdTrace));		// alias
	registerCmd("s",					WRAP_METHOD(Console, cmdTrace));		// alias
	registerCmd("stepover",			WRAP_METHOD(Console, cmdStepOver));
	registerCmd("p",					WRAP_METHOD(Console, cmdStepOver));		// alias
	registerCmd("step_ret",			WRAP_METHOD(Console, cmdStepRet));
	registerCmd("pret",				WRAP_METHOD(Console, cmdStepRet));		// alias
	registerCmd("step_event",			WRAP_METHOD(Console, cmdStepEvent));
	registerCmd("se",					WRAP_METHOD(Console, cmdStepEvent));	// alias
	registerCmd("step_global",		WRAP_METHOD(Console, cmdStepGlobal));
	registerCmd("sg",					WRAP_METHOD(Console, cmdStepGlobal));	// alias
	registerCmd("step_callk",			WRAP_METHOD(Console, cmdStepCallk));
	registerCmd("snk",				WRAP_METHOD(Console, cmdStepCallk));	// alias
	registerCmd("disasm",				WRAP_METHOD(Console, cmdDisassemble));
	registerCmd("disasm_addr",		WRAP_METHOD(Console, cmdDisassembleAddress));
	registerCmd("find_callk",			WRAP_METHOD(Console, cmdFindKernelFunctionCall));
	registerCmd("send",				WRAP_METHOD(Console, cmdSend));
	registerCmd("go",					WRAP_METHOD(Console, cmdGo));
	registerCmd("logkernel",          WRAP_METHOD(Console, cmdLogKernel));
	registerCmd("vocab994",          WRAP_METHOD(Console, cmdMapVocab994));
	registerCmd("gameflags_init",    WRAP_METHOD(Console, cmdGameFlagsInit));
	registerCmd("gameflags_test",    WRAP_METHOD(Console, cmdGameFlagsTest));
	registerCmd("tf",                WRAP_METHOD(Console, cmdGameFlagsTest));
	registerCmd("gameflags_set",     WRAP_METHOD(Console, cmdGameFlagsSet));
	registerCmd("sf",                WRAP_METHOD(Console, cmdGameFlagsSet));
	registerCmd("gameflags_clear",   WRAP_METHOD(Console, cmdGameFlagsClear));
	registerCmd("cf",                WRAP_METHOD(Console, cmdGameFlagsClear));
	// Breakpoints
	registerCmd("bp_list",			WRAP_METHOD(Console, cmdBreakpointList));
	registerCmd("bplist",				WRAP_METHOD(Console, cmdBreakpointList));			// alias
	registerCmd("bl",					WRAP_METHOD(Console, cmdBreakpointList));			// alias
	registerCmd("bp_del",				WRAP_METHOD(Console, cmdBreakpointDelete));
	registerCmd("bpdel",				WRAP_METHOD(Console, cmdBreakpointDelete));			// alias
	registerCmd("bc",					WRAP_METHOD(Console, cmdBreakpointDelete));			// alias
	registerCmd("bp_action",				WRAP_METHOD(Console, cmdBreakpointAction));
	registerCmd("bpact",				WRAP_METHOD(Console, cmdBreakpointAction));			// alias
	registerCmd("bp_address",			WRAP_METHOD(Console, cmdBreakpointAddress));
	registerCmd("bpa",					WRAP_METHOD(Console, cmdBreakpointAddress));		// alias
	registerCmd("bp_method",			WRAP_METHOD(Console, cmdBreakpointMethod));
	registerCmd("bpx",				WRAP_METHOD(Console, cmdBreakpointMethod));			// alias
	registerCmd("bp_read",			WRAP_METHOD(Console, cmdBreakpointRead));
	registerCmd("bpr",				WRAP_METHOD(Console, cmdBreakpointRead));			// alias
	registerCmd("bp_write",			WRAP_METHOD(Console, cmdBreakpointWrite));
	registerCmd("bpw",				WRAP_METHOD(Console, cmdBreakpointWrite));			// alias
	registerCmd("bp_kernel",			WRAP_METHOD(Console, cmdBreakpointKernel));
	registerCmd("bpk",				WRAP_METHOD(Console, cmdBreakpointKernel));			// alias
	registerCmd("bp_function",		WRAP_METHOD(Console, cmdBreakpointFunction));
	registerCmd("bpe",				WRAP_METHOD(Console, cmdBreakpointFunction));		// alias
	// VM
	registerCmd("script_steps",		WRAP_METHOD(Console, cmdScriptSteps));
	registerCmd("script_objects",   WRAP_METHOD(Console, cmdScriptObjects));
	registerCmd("scro",             WRAP_METHOD(Console, cmdScriptObjects));
	registerCmd("script_strings",   WRAP_METHOD(Console, cmdScriptStrings));
	registerCmd("scrs",             WRAP_METHOD(Console, cmdScriptStrings));
	registerCmd("script_said",      WRAP_METHOD(Console, cmdScriptSaid));
	registerCmd("vm_varlist",			WRAP_METHOD(Console, cmdVMVarlist));
	registerCmd("vmvarlist",			WRAP_METHOD(Console, cmdVMVarlist));				// alias
	registerCmd("vl",					WRAP_METHOD(Console, cmdVMVarlist));				// alias
	registerCmd("vm_vars",			WRAP_METHOD(Console, cmdVMVars));
	registerCmd("vmvars",				WRAP_METHOD(Console, cmdVMVars));					// alias
	registerCmd("vv",					WRAP_METHOD(Console, cmdVMVars));					// alias
	registerCmd("locals",				WRAP_METHOD(Console, cmdLocalVars));
	registerCmd("l",					WRAP_METHOD(Console, cmdLocalVars));				// alias
	registerCmd("stack",				WRAP_METHOD(Console, cmdStack));
	registerCmd("st",					WRAP_METHOD(Console, cmdStack));					// alias
	registerCmd("value_type",			WRAP_METHOD(Console, cmdValueType));
	registerCmd("view_listnode",		WRAP_METHOD(Console, cmdViewListNode));
	registerCmd("view_reference",		WRAP_METHOD(Console, cmdViewReference));
	registerCmd("vr",					WRAP_METHOD(Console, cmdViewReference));			// alias
	registerCmd("dump_reference",		WRAP_METHOD(Console, cmdDumpReference));
	registerCmd("dr",					WRAP_METHOD(Console, cmdDumpReference));			// alias
	registerCmd("view_object",		WRAP_METHOD(Console, cmdViewObject));
	registerCmd("vo",					WRAP_METHOD(Console, cmdViewObject));				// alias
	registerCmd("active_object",		WRAP_METHOD(Console, cmdViewActiveObject));
	registerCmd("acc_object",			WRAP_METHOD(Console, cmdViewAccumulatorObject));

	_debugState.seeking = kDebugSeekNothing;
	_debugState.seekLevel = 0;
	_debugState.runningStep = 0;
	_debugState.stopOnEvent = false;
	_debugState.debugging = false;
	_debugState.breakpointWasHit = false;
	_debugState._breakpoints.clear(); // No breakpoints defined
	_debugState._activeBreakpointTypes = 0;
}

Console::~Console() {
}

void Console::attach(const char *entry) {
	if (entry) {
		// Attaching to display a severe error, let the engine know
		_engine->severeError();
	}

	GUI::Debugger::attach(entry);
}

void Console::preEnter() {
	GUI::Debugger::preEnter();
}

extern void playVideo(Video::VideoDecoder &videoDecoder);

void Console::postEnter() {
	if (!_videoFile.empty()) {
		Common::ScopedPtr<Video::VideoDecoder> videoDecoder;

		if (_videoFile.baseName().hasSuffix(".seq")) {
			videoDecoder.reset(new SEQDecoder(_videoFrameDelay));
		} else if (_videoFile.baseName().hasSuffix(".avi")) {
			videoDecoder.reset(new Video::AVIDecoder());
		} else {
			warning("Unrecognized video type");
		}

		if (videoDecoder && videoDecoder->loadFile(_videoFile)) {
			_engine->_gfxCursor->kernelHide();
			playVideo(*videoDecoder);
			_engine->_gfxCursor->kernelShow();
		} else
			warning("Could not play video %s", _videoFile.toString(Common::Path::kNativeSeparator).c_str());

		_videoFile.clear();
		_videoFrameDelay = 0;
	}

	GUI::Debugger::postEnter();
}

bool Console::cmdHelp(int argc, const char **argv) {
	debugPrintf("\n");
	debugPrintf("Variables\n");
	debugPrintf("---------\n");
	debugPrintf("sleeptime_factor: Factor to multiply with wait times in kWait()\n");
	debugPrintf("gc_interval: Number of kernel calls in between garbage collections\n");
	debugPrintf("simulated_key: Add a key with the specified scan code to the event list\n");
	debugPrintf("track_mouse_clicks: Toggles mouse click tracking to the console\n");
	debugPrintf("speed_throttle: Displays or changes kGameIsRestarting maximum delay\n");
	debugPrintf("\n");
	debugPrintf("Debug flags\n");
	debugPrintf("-----------\n");
	debugPrintf("debugflag_list - Lists the available debug flags and their status\n");
	debugPrintf("debugflag_enable - Enables a debug flag\n");
	debugPrintf("debugflag_disable - Disables a debug flag\n");
	debugPrintf("debuglevel - Shows or sets debug level\n");
	debugPrintf("\n");
	debugPrintf("Commands\n");
	debugPrintf("--------\n");
	debugPrintf("Kernel:\n");
	debugPrintf(" opcodes - Lists the opcode names\n");
	debugPrintf(" selectors - Lists the selector names\n");
	debugPrintf(" selector - Attempts to find the requested selector by name\n");
	debugPrintf(" functions - Lists the kernel functions\n");
	debugPrintf(" class_table - Shows the available classes\n");
	debugPrintf("\n");
	debugPrintf("Parser:\n");
	debugPrintf(" suffixes - Lists the vocabulary suffixes\n");
	debugPrintf(" parse_grammar - Shows the parse grammar, in strict GNF\n");
	debugPrintf(" parser_nodes - Shows the specified number of nodes from the parse node tree\n");
	debugPrintf(" parser_words - Shows the words from the parse node tree\n");
	debugPrintf(" sentence_fragments - Shows the sentence fragments (used to build Parse trees)\n");
	debugPrintf(" parse - Parses a sequence of words and prints the resulting parse tree\n");
	debugPrintf(" set_parse_nodes - Sets the contents of all parse nodes\n");
	debugPrintf(" said - Match a string against a said spec\n");
	debugPrintf("\n");
	debugPrintf("Resources:\n");
	debugPrintf(" diskdump - Dumps the specified resource to disk as a patch file\n");
	debugPrintf(" hexdump - Dumps the specified resource to standard output\n");
	debugPrintf(" resource_id - Identifies a resource number by splitting it up in resource type and resource number\n");
	debugPrintf(" resource_info - Shows info about a resource\n");
	debugPrintf(" resource_types - Shows the valid resource types\n");
	debugPrintf(" list - Lists all the resources of a given type\n");
	debugPrintf(" alloc_list - Lists all allocated resources\n");
	debugPrintf(" hexgrep - Searches some resources for a particular sequence of bytes, represented as hexadecimal numbers\n");
	debugPrintf(" verify_scripts - Performs sanity checks on SCI1.1-SCI2.1 game scripts (e.g. if they're up to 64KB in total)\n");
	debugPrintf(" integrity_dump - Dumps integrity data about resources in the current game to disk\n");
	debugPrintf("\n");
	debugPrintf("Game:\n");
	debugPrintf(" save_game - Saves the current game state to the hard disk\n");
	debugPrintf(" restore_game - Restores a saved game from the hard disk\n");
	debugPrintf(" list_saves - List all saved games including filenames\n");
	debugPrintf(" restart_game - Restarts the game\n");
	debugPrintf(" version - Shows the resource and interpreter versions\n");
	debugPrintf(" room - Gets or sets the current room number\n");
	debugPrintf(" quit - Quits the game\n");
	debugPrintf("\n");
	debugPrintf("Graphics:\n");
	debugPrintf(" show_map - Switches to visual, priority, control or display screen\n");
	debugPrintf(" set_palette - Sets a palette resource\n");
	debugPrintf(" draw_pic - Draws a pic resource\n");
	debugPrintf(" draw_cel - Draws a cel from a view resource\n");
	debugPrintf(" pic_visualize - Enables visualization of the drawing process of EGA pictures\n");
	debugPrintf(" undither - Enable/disable undithering\n");
	debugPrintf(" play_video - Plays a SEQ, AVI, VMD, RBT or DUK video\n");
	debugPrintf(" animate_list / al - Shows the current list of objects in kAnimate's draw list (SCI0 - SCI1.1)\n");
	debugPrintf(" window_list / wl - Shows a list of all the windows (ports) in the draw list (SCI0 - SCI1.1)\n");
	debugPrintf(" plane_list / pl - Shows a list of all the planes in the draw list (SCI2+)\n");
	debugPrintf(" visible_plane_list / vpl - Shows a list of all the planes in the visible draw list (SCI2+)\n");
	debugPrintf(" plane_items / pi - Shows a list of all items for a plane (SCI2+)\n");
	debugPrintf(" visible_plane_items / vpi - Shows a list of all items for a plane in the visible draw list (SCI2+)\n");
	debugPrintf(" saved_bits - List saved bits on the hunk\n");
	debugPrintf(" show_saved_bits - Display saved bits\n");
	debugPrintf("\n");
	debugPrintf("Segments:\n");
	debugPrintf(" segment_table / segtable - Lists all segments\n");
	debugPrintf(" segment_info / seginfo - Provides information on the specified segment\n");
	debugPrintf(" segment_kill / segkill - Deletes the specified segment\n");
	debugPrintf("\n");
	debugPrintf("Garbage collection:\n");
	debugPrintf(" gc - Invokes the garbage collector\n");
	debugPrintf(" gc_objects - Lists all reachable objects, normalized\n");
	debugPrintf(" gc_reachable - Lists all addresses directly reachable from a given memory object\n");
	debugPrintf(" gc_freeable - Lists all addresses freeable in a given segment\n");
	debugPrintf(" gc_normalize - Prints the \"normal\" address of a given address\n");
	debugPrintf("\n");
	debugPrintf("Music/SFX:\n");
	debugPrintf(" songlib - Shows the song library\n");
	debugPrintf(" songinfo - Shows information about a specified song in the song library\n");
	debugPrintf(" togglesound - Starts/stops a sound in the song library\n");
	debugPrintf(" stopallsounds - Stops all sounds in the playlist\n");
	debugPrintf(" startsound - Starts the specified sound resource, replacing the first song in the song library\n");
	debugPrintf(" is_sample - Shows information on a given sound resource, if it's a PCM sample\n");
	debugPrintf(" sfx01_header - Dumps the header of a SCI01 song\n");
	debugPrintf(" sfx01_track - Dumps a track of a SCI01 song\n");
	debugPrintf(" show_instruments - Shows the instruments of a specific song, or all songs\n");
	debugPrintf(" map_instrument - Dynamically maps an MT-32 instrument to a GM instrument\n");
	debugPrintf(" audio_list - Lists currently active digital audio samples (SCI2+)\n");
	debugPrintf(" audio_dump - Dumps the requested audio resource as an uncompressed wave file (SCI2+)\n");
	debugPrintf("\n");
	debugPrintf("Script:\n");
	debugPrintf(" addresses - Provides information on how to pass addresses\n");
	debugPrintf(" registers / reg - Shows the current register values\n");
	debugPrintf(" dissect_script - Examines a script\n");
	debugPrintf(" backtrace / bt - Dumps the send/self/super/call/calle/callb stack\n");
	debugPrintf(" trace / t / s - Executes one operation (no parameters) or several operations (specified as a parameter) \n");
	debugPrintf(" stepover / p - Executes one operation, skips over call/send\n");
	debugPrintf(" step_ret / pret - Steps forward until ret is called on the current execution stack level.\n");
	debugPrintf(" step_event / se - Steps forward until a SCI event is received.\n");
	debugPrintf(" step_global / sg - Steps until the global variable with the specified index is modified.\n");
	debugPrintf(" step_callk / snk - Steps forward until it hits the next callk operation, or a specific callk (specified as a parameter)\n");
	debugPrintf(" disasm - Disassembles a method by name\n");
	debugPrintf(" disasm_addr - Disassembles one or more commands\n");
	debugPrintf(" send - Sends a message to an object\n");
	debugPrintf(" go - Executes the script\n");
	debugPrintf(" logkernel - Logs kernel calls\n");
	debugPrintf(" gameflags_init - Initialize gameflag commands if necessary\n");
	debugPrintf(" gameflags_test / tf - Test game flags\n");
	debugPrintf(" gameflags_set / sf - Sets game flags\n");
	debugPrintf(" gameflags_clear / cf - Clears game flags\n");
	debugPrintf("\n");
	debugPrintf("Breakpoints:\n");
	debugPrintf(" bp_list / bplist / bl - Lists the current breakpoints\n");
	debugPrintf(" bp_del / bpdel / bc - Deletes a breakpoint with the specified index\n");
	debugPrintf(" bp_action / bpact - Set action to be performed when breakpoint is triggered\n");
	debugPrintf(" bp_address / bpa - Sets a breakpoint on a script address\n");
	debugPrintf(" bp_method / bpx - Sets a breakpoint on the execution of a specified method/selector\n");
	debugPrintf(" bp_read / bpr - Sets a breakpoint on reading of a specified selector\n");
	debugPrintf(" bp_write / bpw - Sets a breakpoint on writing to a specified selector\n");
	debugPrintf(" bp_kernel / bpk - Sets a breakpoint on execution of a kernel function\n");
	debugPrintf(" bp_function / bpe - Sets a breakpoint on the execution of the specified exported function\n");
	debugPrintf("\n");
	debugPrintf("VM:\n");
	debugPrintf(" script_steps - Shows the number of executed SCI operations\n");
	debugPrintf(" script_objects / scro - Shows all objects inside a specified script\n");
	debugPrintf(" script_strings / scrs - Shows all strings inside a specified script\n");
	debugPrintf(" script_said - Shows all said - strings inside a specified script\n");
	debugPrintf(" vm_varlist / vmvarlist / vl - Shows the addresses of variables in the VM\n");
	debugPrintf(" vm_vars / vmvars / vv - Displays or changes variables in the VM\n");
	debugPrintf(" locals / l - Displays or changes local variables in the VM\n");
	debugPrintf(" stack / st - Lists the specified number of stack elements\n");
	debugPrintf(" value_type - Determines the type of a value\n");
	debugPrintf(" view_listnode - Examines the list node at the given address\n");
	debugPrintf(" view_reference / vr - Examines an arbitrary reference\n");
	debugPrintf(" dump_reference / dr - Dumps an arbitrary reference to disk\n");
	debugPrintf(" view_object / vo - Examines the object at the given address\n");
	debugPrintf(" active_object - Shows information on the currently active object or class\n");
	debugPrintf(" acc_object - Shows information on the object or class at the address indexed by the accumulator\n");
	debugPrintf("\n");
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

bool Console::cmdGetVersion(int argc, const char **argv) {
	const char *viewTypeDesc[] = { "Unknown", "EGA", "Amiga ECS 32 colors", "Amiga AGA 64 colors", "VGA", "VGA SCI1.1" };

	bool hasVocab997 = g_sci->getResMan()->testResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SELECTORS)) ? true : false;
	Common::String gameVersion = "N/A";

	Common::File versionFile;
	if (versionFile.open("VERSION")) {
		gameVersion = versionFile.readLine();
		versionFile.close();
	}

	debugPrintf("Game ID: %s\n", _engine->getGameIdStr());
	debugPrintf("Emulated interpreter version: %s\n", getSciVersionDesc(getSciVersion()));
	debugPrintf("\n");
	debugPrintf("Detected features:\n");
	debugPrintf("------------------\n");
	debugPrintf("Sound type: %s\n", getSciVersionDesc(_engine->_features->detectDoSoundType()));
	debugPrintf("Graphics functions type: %s\n", getSciVersionDesc(_engine->_features->detectGfxFunctionsType()));
	debugPrintf("Lofs type: %s\n", getSciVersionDesc(_engine->_features->detectLofsType()));
	debugPrintf("Move count type: %s\n", (_engine->_features->handleMoveCount()) ? "increment" : "ignore");
	debugPrintf("SetCursor type: %s\n", getSciVersionDesc(_engine->_features->detectSetCursorType()));
	debugPrintf("PseudoMouse ability: %s\n", _engine->_features->detectPseudoMouseAbility() == kPseudoMouseAbilityTrue ? "yes" : "no");
#ifdef ENABLE_SCI32
	if ((getSciVersion() >= SCI_VERSION_2_1_EARLY) && (getSciVersion() <= SCI_VERSION_2_1_LATE))
		debugPrintf("SCI2.1 kernel table: %s\n", (_engine->_features->detectSci21KernelType() == SCI_VERSION_2) ? "modified SCI2 (old)" : "SCI2.1 (new)");
#endif
	debugPrintf("View type: %s\n", viewTypeDesc[g_sci->getResMan()->getViewType()]);
	if (getSciVersion() <= SCI_VERSION_1_1) {
		debugPrintf("kAnimate fastCast enabled: %s\n", g_sci->_gfxAnimate->isFastCastEnabled() ? "yes" : "no");
	}
	if (getSciVersion() < SCI_VERSION_2) {
		debugPrintf("Uses palette merging: %s\n", g_sci->_gfxPalette16->isMerging() ? "yes" : "no");
		debugPrintf("Uses 16 bit color matching: %s\n", g_sci->_gfxPalette16->isUsing16bitColorMatch() ? "yes" : "no");
	}
	debugPrintf("Resource volume version: %s\n", g_sci->getResMan()->getVolVersionDesc());
	debugPrintf("Resource map version: %s\n", g_sci->getResMan()->getMapVersionDesc());
	debugPrintf("Contains selector vocabulary (vocab.997): %s\n", hasVocab997 ? "yes" : "no");
	debugPrintf("Has CantBeHere selector: %s\n", g_sci->getKernel()->_selectorCache.cantBeHere != -1 ? "yes" : "no");
	if (getSciVersion() >= SCI_VERSION_2) {
		debugPrintf("Plane id base: %d\n", g_sci->_features->detectPlaneIdBase());
	}
	debugPrintf("Game version (VERSION file): %s\n", gameVersion.c_str());
	debugPrintf("\n");

	return true;
}

bool Console::cmdOpcodes(int argc, const char **argv) {
	// Load the opcode table from vocab.998 if it exists, to obtain the opcode names
	Resource *r = _engine->getResMan()->findResource(ResourceId(kResourceTypeVocab, 998), 0);

	// If the resource couldn't be loaded, leave
	if (!r) {
		debugPrintf("unable to load vocab.998\n");
		return true;
	}

	int count = r->getUint16LEAt(0);

	debugPrintf("Opcode names in numeric order [index: type name]:\n");

	for (int i = 0; i < count; i++) {
		int offset = r->getUint16LEAt(2 + i * 2);
		int len = r->getUint16LEAt(offset) - 2;
		int type = r->getUint16LEAt(offset + 2);
		// QFG3 has empty opcodes
		Common::String name = len > 0 ? r->getStringAt(offset + 4, len) : "Dummy";
		debugPrintf("%03x: %03x %15s | ", i, type, name.c_str());
		if ((i % 3) == 2)
			debugPrintf("\n");
	}

	debugPrintf("\n");

	return true;
}

bool Console::cmdSelector(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Attempts to find the requested selector by name.\n");
		debugPrintf("Usage: %s <selector name>\n", argv[0]);
		return true;
	}

	Common::String name = argv[1];
	int seeker = _engine->getKernel()->findSelector(name.c_str());
	if (seeker >= 0) {
		debugPrintf("Selector %s found at %03x (%d)\n", name.c_str(), seeker, seeker);
		return true;
	}

	debugPrintf("Selector %s wasn't found\n", name.c_str());

	return true;
}

bool Console::cmdSelectors(int argc, const char **argv) {
	debugPrintf("Selector names in numeric order:\n");
	Common::String selectorName;
	for (uint seeker = 0; seeker < _engine->getKernel()->getSelectorNamesSize(); seeker++) {
		selectorName = _engine->getKernel()->getSelectorName(seeker);
		if (selectorName != "BAD SELECTOR")
			debugPrintf("%03x: %20s | ", seeker, selectorName.c_str());
		else
			continue;
		if ((seeker % 3) == 2)
			debugPrintf("\n");
	}

	debugPrintf("\n");

#if 0
	// For debug/development

	// If we ever need to modify static_selectors.cpp, this code will print the selectors
	// in a ready to use format
	Common::DumpFile *outFile = new Common::DumpFile();
	outFile->open("selectors.txt");
	char buf[50];
	Common::String selName;
	uint totalSize = _engine->getKernel()->getSelectorNamesSize();
	uint seeker = 0;
	while (seeker < totalSize) {
		selName = "\"" + _engine->getKernel()->getSelectorName(seeker) + "\"";
		Common::sprintf_s(buf, "%15s, ", selName.c_str());
		outFile->writeString(buf);

		if (!((seeker + 1) % 5) && seeker)
			outFile->writeByte('\n');
		seeker++;
	}
	outFile->finalize();
	outFile->close();
#endif

	return true;
}

bool Console::cmdKernelFunctions(int argc, const char **argv) {
	debugPrintf("Kernel function names in numeric order:\n");
	debugPrintf("+ denotes Kernel functions with subcommands\n");
	uint column = 0;
	for (uint seeker = 0; seeker <  _engine->getKernel()->getKernelNamesSize(); seeker++) {
		const Common::String &kernelName = _engine->getKernel()->getKernelName(seeker);
		if (kernelName == "Dummy")
			continue;

		const KernelFunction &kernelCall = _engine->getKernel()->_kernelFuncs[seeker];
		const char *subCmdNote = kernelCall.subFunctionCount ? "+" : "";
		
		if (argc == 1) {
			debugPrintf("%03x: %20s | ", seeker, (kernelName + subCmdNote).c_str());
			if ((column++ % 3) == 2)
				debugPrintf("\n");
		} else {
			for (int i = 1; i < argc; ++i) {
				if (kernelName.equalsIgnoreCase(argv[i])) {
					debugPrintf("%03x: %s\n", seeker, (kernelName + subCmdNote).c_str());
				}
			}
		}
	}

	debugPrintf("\n");

	return true;
}

bool Console::cmdKernelCall(int argc, const char **argv) {
	const int MAX_ARGS_ALLOWED = 20;

	if (argc <= 1) {
		debugPrintf("Calls a kernel function by name.\n");
		debugPrintf("(You must ensure you invoke the kernel function with the correct signature.)\n");
		debugPrintf("Usage: %s <kernel-func-name> <param1> <param2> ... <paramn>\n", argv[0]);
		debugPrintf("Example 1: %s GameIsRestarting\n", argv[0]);
		debugPrintf("Example 2: %s Random 3 7\n", argv[0]);
		debugPrintf("Example 3: %s Memory 6 002a:0012 0x6566\n", argv[0]);
		return true;
	}

	const int kern_argc = argc - 2;

	if (kern_argc > MAX_ARGS_ALLOWED) {
		debugPrintf("No more than %d args allowed for a kernel call, you gave: %d.\n", (int)MAX_ARGS_ALLOWED, kern_argc);
		return true;
	}

	Kernel *kernel = _engine->getKernel();

	// Identify kernel call code by name.
	int kernIdx = kernel->findKernelFuncPos(argv[1]);
	if (kernIdx == -1) {
		debugPrintf("No kernel function with name - see command \"kernfunctions\" for a list: %s\n", argv[1]);
		return true;
	}

	const KernelFunction &kernelCall = kernel->_kernelFuncs[kernIdx];
	
	reg_t kernArgArr[MAX_ARGS_ALLOWED];

	for (int i = 0; i < kern_argc; i++) {
		if (parse_reg_t(_engine->_gamestate, argv[2+i], &kernArgArr[i])) {
			debugPrintf("Invalid address \"%s\" passed.\n", argv[2+i]);
			debugPrintf("Check the \"addresses\" command on how to use addresses\n");
			return true;
		}
	}

	reg_t kernResult;

	if (!kernelCall.subFunctionCount) {
		// Must be a regular kernel function.
		kernResult = kernelCall.function(_engine->_gamestate, kern_argc, kernArgArr);
	} else {
		// Must be a kernel function that supports sub commands.

		// Pull out the first argument register as the sub function id.
		uint subId = kernArgArr[0].getOffset();

		const KernelSubFunction &kernelSubCall = kernelCall.subFunctions[subId];
		if (!kernelSubCall.function) {
			debugPrintf("Kernel sub function with id:%d does not exist\n", 0);
			return true;
		}

		// Pass a pointer to the remaining reg_t args.
		kernResult = kernelSubCall.function(_engine->_gamestate, kern_argc-1, &(kernArgArr[1]));
	}	

	debugPrintf("kernel call result is: %04x:%04x\n", PRINT_REG(kernResult));

	return true;
}

bool Console::cmdSuffixes(int argc, const char **argv) {
	_engine->getVocabulary()->printSuffixes();

	return true;
}

bool Console::cmdParserWords(int argc, const char **argv) {
	_engine->getVocabulary()->printParserWords();

	return true;
}

bool Console::cmdSetParseNodes(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Sets the contents of all parse nodes.\n");
		debugPrintf("Usage: %s <parse node1> <parse node2> ... <parse noden>\n", argv[0]);
		debugPrintf("Tokens should be separated by blanks and enclosed in parentheses\n");
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
		nextValue = strtol(token, nullptr, 0);
		nextToken = kParseNumber;
	}

	if (_engine->getVocabulary()->parseNodes(&i, &pos, nextToken, nextValue, argc, argv) == -1)
		return 1;

	_engine->getVocabulary()->dumpParseTree();

	return true;
}

bool Console::cmdRegisters(int argc, const char **argv) {
	EngineState *s = _engine->_gamestate;
	debugPrintf("Current register values:\n");
	debugPrintf("acc=%04x:%04x prev=%04x:%04x &rest=%x\n", PRINT_REG(s->r_acc), PRINT_REG(s->r_prev), s->r_rest);

	if (!s->_executionStack.empty()) {
		debugPrintf("pc=%04x:%04x obj=%04x:%04x fp=ST:%04x sp=ST:%04x\n",
					PRINT_REG(s->xs->addr.pc), PRINT_REG(s->xs->objp),
					(unsigned)(s->xs->fp - s->stack_base), (unsigned)(s->xs->sp - s->stack_base));
	} else
		debugPrintf("<no execution stack: pc,obj,fp omitted>\n");

	return true;
}

bool Console::parseResourceNumber36(const char *userParameter, uint16 &resourceNumber, uint32 &resourceTuple) {
	int userParameterLen = strlen(userParameter);

	if (userParameterLen != 10) {
		debugPrintf("Audio36/Sync36 resource numbers must be specified as RRRNNVVCCS\n");
		debugPrintf("where RRR is the resource number/map\n");
		debugPrintf("      NN is the noun\n");
		debugPrintf("      VV is the verb\n");
		debugPrintf("      CC is the cond\n");
		debugPrintf("      S  is the seq\n");
		return false;
	}

	// input: RRRNNVVCCS
	resourceNumber = strtol(Common::String(userParameter, 3).c_str(), nullptr, 36);
	uint16 noun = strtol(Common::String(userParameter + 3, 2).c_str(), nullptr, 36);
	uint16 verb = strtol(Common::String(userParameter + 5, 2).c_str(), nullptr, 36);
	uint16 cond = strtol(Common::String(userParameter + 7, 2).c_str(), nullptr, 36);
	uint16 seq = strtol(Common::String(userParameter + 9, 1).c_str(), nullptr, 36);
	resourceTuple = ((noun & 0xff) << 24) | ((verb & 0xff) << 16) | ((cond & 0xff) << 8) | (seq & 0xff);
	return true;
}

bool Console::cmdDiskDump(int argc, const char **argv) {
	bool resourceAll = false;
	uint16 resourceNumber = 0;
	uint32 resourceTuple = 0;

	if (argc != 3) {
		debugPrintf("Dumps the specified resource to disk as a patch file\n");
		debugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		debugPrintf("       <resource number> may be '*' to dump all resources of given type\n");
		cmdResourceTypes(argc, argv);
		return true;
	}

	ResourceType resourceType = parseResourceType(argv[1]);
	if (resourceType == kResourceTypeInvalid) {
		debugPrintf("Resource type '%s' is not valid\n", argv[1]);
		return true;
	}

	if (strcmp(argv[2], "*") == 0) {
		resourceAll = true;
	} else {
		switch (resourceType) {
		case kResourceTypeAudio36:
		case kResourceTypeSync36:
			if (!parseResourceNumber36(argv[2], resourceNumber, resourceTuple)) {
				return true;
			}
			break;
		default:
			resourceNumber = atoi(argv[2]);
			break;
		}
	}

	if (resourceAll) {
		// "*" used, dump everything of that type
		Common::List<ResourceId> resources = _engine->getResMan()->listResources(resourceType, -1);
		Common::sort(resources.begin(), resources.end());

		Common::List<ResourceId>::iterator itr;
		for (itr = resources.begin(); itr != resources.end(); ++itr) {
			resourceNumber = itr->getNumber();
			resourceTuple = itr->getTuple();
			cmdDiskDumpWorker(resourceType, resourceNumber, resourceTuple);
		}
	} else {
		// id was given, dump only this resource
		cmdDiskDumpWorker(resourceType, resourceNumber, resourceTuple);
	}

	return true;
}

void Console::cmdDiskDumpWorker(ResourceType resourceType, int resourceNumber, uint32 resourceTuple) {
	const char *resourceTypeName = getResourceTypeName(resourceType);
	ResourceId resourceId;
	Resource *resource = nullptr;
	char outFileName[50];

	switch (resourceType) {
	case kResourceTypeAudio36:
	case kResourceTypeSync36: {
		resourceId = ResourceId(resourceType, resourceNumber, resourceTuple);
		resource = _engine->getResMan()->findResource(resourceId, 0);
		Common::sprintf_s(outFileName, "%s", resourceId.toPatchNameBase36().c_str());
		// patch filename is: [type:1 char] [map:3 chars] [noun:2 chars] [verb:2 chars] "." [cond: 2 chars] [seq:1 char]
		//  e.g. "@5EG0000.014"
		break;
	}
	default:
		resourceId = ResourceId(resourceType, resourceNumber);
		resource = _engine->getResMan()->findResource(resourceId, 0);
		Common::sprintf_s(outFileName, "%s.%03d", resourceTypeName, resourceNumber);
		// patch filename is: [resourcetype].[resourcenumber]
		//  e.g. "Script.0"
		break;
	}

	if (resource) {
		Common::DumpFile *outFile = new Common::DumpFile();
		outFile->open(outFileName);
		resource->writeToStream(outFile);
		outFile->finalize();
		outFile->close();
		delete outFile;
		debugPrintf("Resource %s (located in %s) has been dumped to disk\n", outFileName, resource->getResourceLocation().toString(Common::Path::kNativeSeparator).c_str());
	} else {
		debugPrintf("Resource %s not found\n", outFileName);
	}
}

bool Console::cmdHexDump(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Dumps the specified resource to standard output\n");
		debugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		cmdResourceTypes(argc, argv);
		return true;
	}

	int resNum = atoi(argv[2]);
	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		debugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _engine->getResMan()->findResource(ResourceId(res, resNum), 0);
		if (resource) {
			Common::hexdump(resource->getUnsafeDataAt(0), resource->size(), 16, 0);
			debugPrintf("Resource %s.%03d has been dumped to standard output\n", argv[1], resNum);
		} else {
			debugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdResourceId(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Identifies a resource number by splitting it up in resource type and resource number\n");
		debugPrintf("Usage: %s <resource number>\n", argv[0]);
		return true;
	}

	int id = atoi(argv[1]);
	debugPrintf("%s.%d (0x%x)\n", getResourceTypeName((ResourceType)(id >> 11)), id & 0x7ff, id & 0x7ff);

	return true;
}

bool Console::cmdList(int argc, const char **argv) {
	int selectedMapNumber = -1;
	Common::List<ResourceId> resources;
	Common::List<ResourceId>::iterator itr;
	int displayCount = 0;
	int currentMap = -1;

	if (argc < 2) {
		debugPrintf("Lists all the resources of a given type\n");
		cmdResourceTypes(argc, argv);
		return true;
	}

	ResourceType resourceType = parseResourceType(argv[1]);
	if (resourceType == kResourceTypeInvalid) {
		debugPrintf("Unknown resource type: '%s'\n", argv[1]);
		return true;
	}

	switch (resourceType) {
	case kResourceTypeAudio36:
	case kResourceTypeSync36:
		if (argc != 3) {
			debugPrintf("Please specify map number (-1: all maps)\n");
			return true;
		}
		selectedMapNumber = atoi(argv[2]);
		resources = _engine->getResMan()->listResources(resourceType, selectedMapNumber);
		Common::sort(resources.begin(), resources.end());

		for (itr = resources.begin(); itr != resources.end(); ++itr) {
			const uint16 map = itr->getNumber();
			const uint32 resourceTuple = itr->getTuple();
			const uint16 noun = (resourceTuple >> 24) & 0xff;
			const uint16 verb = (resourceTuple >> 16) & 0xff;
			const uint16 cond = (resourceTuple >> 8) & 0xff;
			const uint16 seq = resourceTuple & 0xff;

			if (currentMap != map) {
				if (displayCount % 3)
					debugPrintf("\n");
				debugPrintf("Map %04x (%i):\n", map, map);
				currentMap = map;
				displayCount = 0;
			}

			if (displayCount % 3 == 0)
				debugPrintf("  ");

			debugPrintf("%02x %02x %02x %02x (%3i %3i %3i %3i)    ", noun, verb, cond, seq, noun, verb, cond, seq);

			if (++displayCount % 3 == 0)
				debugPrintf("\n");
		}
		break;
	default:
		resources = _engine->getResMan()->listResources(resourceType);
		Common::sort(resources.begin(), resources.end());

		for (itr = resources.begin(); itr != resources.end(); ++itr) {
			debugPrintf("%8i", itr->getNumber());
			if (++displayCount % 10 == 0)
				debugPrintf("\n");
		}
		break;
	}

	debugPrintf("\n");
	return true;
}

bool Console::cmdResourceIntegrityDump(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Dumps integrity data about resources in the current game to disk.\n");
		debugPrintf("Usage: %s <filename> [<skip video file hashing>] [<skip video files altogether>]\n", argv[0]);
		return true;
	}

	Common::DumpFile outFile;
	if (!outFile.open(argv[1])) {
		debugPrintf("Failed to open output file %s.\n", argv[1]);
		return true;
	}

	const bool hashVideoFiles = argc < 3;
	const bool videoFiles = argc < 4;

	for (int i = 0; i < kResourceTypeInvalid; ++i) {
		const ResourceType resType = (ResourceType)i;

		// This will list video resources inside of resource bundles even if
		// video files are skipped, but this seems fine since those files are
		// small because they were intended to load into memory. (This happens
		// with VMDs in GK2.)
		Common::List<ResourceId> resources = _engine->getResMan()->listResources(resType);

		const char *extension = "";
		if (videoFiles) {
			switch (resType) {
			case kResourceTypeRobot:
			case kResourceTypeVMD:
			case kResourceTypeDuck:
			case kResourceTypeClut: {
				extension = getResourceTypeExtension(resType);
				assert(*extension != '\0');

				const Common::Path filesGlob(Common::String::format("*.%s", extension));
				Common::ArchiveMemberList files;
				const int numMatches = SearchMan.listMatchingMembers(files, filesGlob);
				if (numMatches > 0) {
					Common::ArchiveMemberList::const_iterator it;
					for (it = files.begin(); it != files.end(); ++it) {
						const uint resNo = atoi((*it)->getName().c_str());
						resources.push_back(ResourceId(resType, resNo));
					}
				}

				break;
			}
			default:
				break;
			}
		}

		if (resources.size()) {
			Common::sort(resources.begin(), resources.end());
			Common::List<ResourceId>::const_iterator it;
			debugPrintf("%s: ", getResourceTypeName(resType));
			for (it = resources.begin(); it != resources.end(); ++it) {
				Common::String statusName;
				if (resType == kResourceTypeAudio36 || resType == kResourceTypeSync36) {
					statusName = it->toPatchNameBase36();
				} else {
					statusName = Common::String::format("%d", it->getNumber());
				}

				const Common::String resourceName = it->toString();

				Resource *resource = _engine->getResMan()->findResource(*it, false);
				if (resource) {
					Common::MemoryReadStream stream = resource->toStream();
					writeIntegrityDumpLine(statusName, resourceName, outFile, &stream, resource->size(), true);
				} else if (videoFiles && *extension != '\0') {
					const Common::Path fileName(Common::String::format("%u.%s", it->getNumber(), extension));
					Common::File file;
					Common::ReadStream *stream = nullptr;
					if (file.open(fileName)) {
						stream = &file;
					}
					writeIntegrityDumpLine(statusName, resourceName, outFile, stream, file.size(), hashVideoFiles);
				}
			}

			debugPrintf("\n");
		}
	}

	const char *otherVideoFiles[] = { "avi", "seq" };
	for (uint i = 0; i < ARRAYSIZE(otherVideoFiles); ++i) {
		const char *extension = otherVideoFiles[i];

		Common::ArchiveMemberList files;
		if (SearchMan.listMatchingMembers(files, Common::String::format("*.%s", extension).c_str()) > 0) {
			debugPrintf("%s: ", extension);
			Common::sort(files.begin(), files.end(), Common::ArchiveMemberListComparator());
			Common::ArchiveMemberList::const_iterator it;
			for (it = files.begin(); it != files.end(); ++it) {
				const Common::ArchiveMember &file = **it;
				Common::ScopedPtr<Common::SeekableReadStream> stream(file.createReadStream());
				writeIntegrityDumpLine(file.getName(), file.getName(), outFile, stream.get(), stream->size(), hashVideoFiles);
			}
			debugPrintf("\n");
		}
	}

	return true;
}

bool Console::cmdAllocList(int argc, const char **argv) {
	ResourceManager *resMan = _engine->getResMan();

	for (int i = 0; i < kResourceTypeInvalid; ++i) {
		Common::List<ResourceId> resources = _engine->getResMan()->listResources((ResourceType)i);
		if (resources.size()) {
			Common::sort(resources.begin(), resources.end());
			bool hasAlloc = false;
			Common::List<ResourceId>::const_iterator it;
			for (it = resources.begin(); it != resources.end(); ++it) {
				Resource *resource = resMan->testResource(*it);
				if (resource != nullptr && resource->data() != nullptr) {
					if (hasAlloc) {
						debugPrintf(", ");
					} else {
						debugPrintf("%s: ", getResourceTypeName((ResourceType)i));
					}
					hasAlloc = true;
					debugPrintf("%u (%u locks)", resource->getNumber(), resource->getNumLockers());
				}
			}
			if (hasAlloc) {
				debugPrintf("\n");
			}
		}
	}

	return true;
}

bool Console::cmdDissectScript(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Examines a script\n");
		debugPrintf("Usage: %s <script number>\n", argv[0]);
		return true;
	}

	_engine->getKernel()->dissectScript(atoi(argv[1]), _engine->getVocabulary());

	return true;
}

bool Console::cmdRoomNumber(int argc, const char **argv) {
	// The room number is stored in global var 13
	// The same functionality is provided by "vmvars g 13" (but this one is more straighforward)

	if (argc != 2) {
		debugPrintf("Current room number is %d\n", _engine->_gamestate->currentRoomNumber());
		debugPrintf("Calling this command with the room number (in decimal or hexadecimal) changes the room\n");
	} else {
		Common::String roomNumberStr = argv[1];
		int roomNumber = strtol(roomNumberStr.c_str(), nullptr, roomNumberStr.hasSuffix("h") ? 16 : 10);
		_engine->_gamestate->setRoomNumber(roomNumber);
		debugPrintf("Room number changed to %d (%x in hex)\n", roomNumber, roomNumber);
	}

	return true;
}

bool Console::cmdResourceInfo(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Shows information about a resource\n");
		debugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		return true;
	}

	int resNum = atoi(argv[2]);
	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		debugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _engine->getResMan()->findResource(ResourceId(res, resNum), 0);
		if (resource) {
			debugPrintf("Resource size: %u\n", resource->size());
			debugPrintf("Resource location: %s\n", resource->getResourceLocation().toString().c_str());
			Common::MemoryReadStream stream = resource->toStream();
			const Common::String hash = Common::computeStreamMD5AsString(stream);
			debugPrintf("Resource hash (decompressed): %s\n", hash.c_str());
		} else {
			debugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdResourceTypes(int argc, const char **argv) {
	debugPrintf("The %d valid resource types are:\n", kResourceTypeInvalid);
	for (int i = 0; i < kResourceTypeInvalid; i++) {
		debugPrintf("%s", getResourceTypeName((ResourceType) i));
		debugPrintf((i < kResourceTypeInvalid - 1) ? ", " : "\n");
	}

	return true;
}

bool Console::cmdHexgrep(int argc, const char **argv) {
	if (argc < 4) {
		debugPrintf("Searches some resources for a particular sequence of bytes, represented as decimal or hexadecimal numbers.\n");
		debugPrintf("Usage: %s <resource type> <resource number> <search string>\n", argv[0]);
		debugPrintf("<resource number> can be a specific resource number, or \"all\" for all of the resources of the specified type\n");
		debugPrintf("EXAMPLES:\n  hexgrep script all 0xe8 0x03 0xc8 0x00\n  hexgrep pic 0x42 0xfe\n");
		cmdResourceTypes(argc, argv);
		return true;
	}

	ResourceType restype = parseResourceType(argv[1]);
	int resNumber = 0, resMax = 0;
	Resource *script = nullptr;

	if (restype == kResourceTypeInvalid) {
		debugPrintf("Resource type '%s' is not valid\n", argv[1]);
		return true;
	}

	if (!scumm_stricmp(argv[2], "all")) {
		resNumber = 0;
		resMax = 65535;
	} else {
		resNumber = resMax = atoi(argv[2]);
	}

	// Convert the bytes
	Common::Array<int> byteString;
	byteString.resize(argc - 3);

	for (uint i = 0; i < byteString.size(); i++)
		if (!parseInteger(argv[i + 3], byteString[i]))
			return true;

	for (; resNumber <= resMax; resNumber++) {
		script = _engine->getResMan()->findResource(ResourceId(restype, resNumber), 0);
		if (script) {
			uint32 seeker = 0, seekerold = 0;
			uint32 comppos = 0;
			int output_script_name = 0;

			while (seeker < script->size()) {
				if (script->getUint8At(seeker) == byteString[comppos]) {
					if (comppos == 0)
						seekerold = seeker;

					comppos++;

					if (comppos == byteString.size()) {
						comppos = 0;
						seeker = seekerold + 1;

						if (!output_script_name) {
							debugPrintf("\nIn %s.%03d:\n", getResourceTypeName((ResourceType)restype), resNumber);
							output_script_name = 1;
						}
						debugPrintf("   0x%04x\n", seekerold);
					}
				} else
					comppos = 0;

				seeker++;
			}
		}
	}

	return true;
}

bool Console::cmdVerifyScripts(int argc, const char **argv) {
	if (getSciVersion() < SCI_VERSION_1_1) {
		debugPrintf("This script check is only meant for SCI1.1-SCI3 games\n");
		return true;
	}

	Common::List<ResourceId> resources = _engine->getResMan()->listResources(kResourceTypeScript);
	Common::sort(resources.begin(), resources.end());

	debugPrintf("%d SCI1.1-SCI3 scripts found, performing sanity checks...\n", resources.size());

	Common::List<ResourceId>::iterator itr;
	for (itr = resources.begin(); itr != resources.end(); ++itr) {
		Resource *script = _engine->getResMan()->findResource(*itr, false);
		if (!script)
			debugPrintf("Error: script %d couldn't be loaded\n", itr->getNumber());

		if (getSciVersion() <= SCI_VERSION_2_1_LATE) {
			Resource *heap = _engine->getResMan()->findResource(ResourceId(kResourceTypeHeap, itr->getNumber()), false);
			if (!heap)
				debugPrintf("Error: script %d doesn't have a corresponding heap\n", itr->getNumber());

			if (script && heap && (script->size() + heap->size() > 65535))
				debugPrintf("Error: script and heap %d together are larger than 64KB (%u bytes)\n",
				itr->getNumber(), script->size() + heap->size());
		} else {	// SCI3
			if (script && script->size() > 0x3FFFF)
				debugPrintf("Error: script %d is larger than 256KB (%u bytes)\n",
				itr->getNumber(), script->size());
		}
	}

	debugPrintf("SCI1.1-SCI2.1 script check finished\n");

	return true;
}

// Same as in sound/drivers/midi.cpp
uint8 getGmInstrument(const Mt32ToGmMap &Mt32Ins) {
	if (Mt32Ins.gmInstr == MIDI_MAPPED_TO_RHYTHM)
		return Mt32Ins.gmRhythmKey + 0x80;
	else
		return Mt32Ins.gmInstr;
}

bool Console::cmdShowInstruments(int argc, const char **argv) {
	int songNumber = -1;

	if (argc == 2)
		songNumber = atoi(argv[1]);

	SciVersion doSoundVersion = _engine->_features->detectDoSoundType();
	MidiPlayer *player = MidiPlayer_Midi_create(doSoundVersion);
	MidiParser_SCI *parser = new MidiParser_SCI(doSoundVersion, nullptr);
	parser->setMidiDriver(player);

	Common::List<ResourceId> resources = _engine->getResMan()->listResources(kResourceTypeSound);
	Common::sort(resources.begin(), resources.end());
	int instruments[128];
	bool instrumentsSongs[128][1000];

	for (int i = 0; i < 128; i++)
		instruments[i] = 0;

	for (int i = 0; i < 128; i++)
		for (int j = 0; j < 1000; j++)
			instrumentsSongs[i][j] = false;

	if (songNumber == -1) {
		debugPrintf("%d sounds found, checking their instrument mappings...\n", resources.size());
		debugPrintf("Instruments:\n");
		debugPrintf("============\n");
	}

	Common::List<ResourceId>::iterator itr;
	for (itr = resources.begin(); itr != resources.end(); ++itr) {
		if (songNumber >= 0 && itr->getNumber() != songNumber)
			continue;

		SoundResource sound(itr->getNumber(), _engine->getResMan(), doSoundVersion);
		int channelFilterMask = sound.getChannelFilterMask(player->getPlayId(), player->hasRhythmChannel());
		SoundResource::Track *track = sound.getTrackByType(player->getPlayId());
		if (!track || track->digitalChannelNr != -1) {
			// Skip digitized sound effects
			continue;
		}

		parser->loadMusic(track, nullptr, channelFilterMask, doSoundVersion);
		SciSpan<const byte> channelData = parser->getMixedData();

		byte curEvent = 0, prevEvent = 0, command = 0;
		bool endOfTrack = false;
		bool firstOneShown = false;

		debugPrintf("Song %d: ", itr->getNumber());

		do {
			while (*channelData == 0xF8)
				channelData++;

			channelData++;	// delta

			if ((*channelData & 0xF0) >= 0x80)
				curEvent = *(channelData++);
			else
				curEvent = prevEvent;
			if (curEvent < 0x80)
				continue;

			prevEvent = curEvent;
			command = curEvent >> 4;

			byte channel;

			switch (command) {
			case 0xC:	// program change
				channel = curEvent & 0x0F;
				if (channel != 15) {	// SCI special
					byte instrument = *channelData++;
					if (!firstOneShown)
						firstOneShown = true;
					else
						debugPrintf(",");

					debugPrintf(" %d", instrument);
					instruments[instrument]++;
					instrumentsSongs[instrument][itr->getNumber()] = true;
				} else {
					channelData++;
				}
				break;
			case 0xD:
				channelData++;	// param1
				break;
			case 0xB:
			case 0x8:
			case 0x9:
			case 0xA:
			case 0xE:
				channelData++;	// param1
				channelData++;	// param2
				break;
			case 0xF:
				if ((curEvent & 0x0F) == 0x2) {
					channelData++;	// param1
					channelData++;	// param2
				} else if ((curEvent & 0x0F) == 0x3) {
					channelData++;	// param1
				} else if ((curEvent & 0x0F) == 0xF) {	// META
					byte type = *channelData++;
					if (type == 0x2F) {// end of track reached
						endOfTrack = true;
					} else {
						// no further processing necessary
					}
				}
				break;
			default:
				break;
			}
		} while (!endOfTrack);

		debugPrintf("\n");
	}

	delete parser;
	delete player;

	debugPrintf("\n");

	if (songNumber == -1) {
		debugPrintf("Used instruments: ");
		for (int i = 0; i < 128; i++) {
			if (instruments[i] > 0)
				debugPrintf("%d, ", i);
		}
		debugPrintf("\n\n");
	}

	debugPrintf("Instruments not mapped in the MT32->GM map: ");
	for (int i = 0; i < 128; i++) {
		if (instruments[i] > 0 && getGmInstrument(Mt32MemoryTimbreMaps[i]) == MIDI_UNMAPPED)
			debugPrintf("%d, ", i);
	}
	debugPrintf("\n\n");

	if (songNumber == -1) {
		debugPrintf("Used instruments in songs:\n");
		for (int i = 0; i < 128; i++) {
			if (instruments[i] > 0) {
				debugPrintf("Instrument %d: ", i);
				for (int j = 0; j < 1000; j++) {
					if (instrumentsSongs[i][j])
						debugPrintf("%d, ", j);
				}
				debugPrintf("\n");
			}
		}

		debugPrintf("\n\n");
	}

	return true;
}

bool Console::cmdMapInstrument(int argc, const char **argv) {
	if (argc != 4) {
		debugPrintf("Maps an MT-32 custom instrument to a GM instrument on the fly\n\n");
		debugPrintf("Usage %s <MT-32 instrument name> <GM instrument> <GM rhythm key>\n", argv[0]);
		debugPrintf("Each MT-32 instrument is always 10 characters and is mapped to either a GM instrument, or a GM rhythm key\n");
		debugPrintf("A value of 255 (0xff) signifies an unmapped instrument\n");
		debugPrintf("Please replace the spaces in the instrument name with underscores (\"_\"). They'll be converted to spaces afterwards\n\n");
		debugPrintf("Example: %s test_0__XX 1 255\n", argv[0]);
		debugPrintf("The above example will map the MT-32 instrument \"test 0  XX\" to GM instrument 1\n\n");
	} else {
		if (Mt32dynamicMappings != nullptr) {
			Mt32ToGmMap newMapping;
			char *instrumentName = new char[11];
			Common::strlcpy(instrumentName, argv[1], 11);

			for (uint16 i = 0; i < Common::strnlen(instrumentName, 11); i++)
				if (instrumentName[i] == '_')
					instrumentName[i] = ' ';

			newMapping.name = instrumentName;
			newMapping.gmInstr = atoi(argv[2]);
			newMapping.gmRhythmKey = atoi(argv[3]);
			Mt32dynamicMappings->push_back(newMapping);
		}
	}

	debugPrintf("Current dynamic mappings:\n");
	if (Mt32dynamicMappings != nullptr) {
		const Mt32ToGmMapList::iterator end = Mt32dynamicMappings->end();
		for (Mt32ToGmMapList::iterator it = Mt32dynamicMappings->begin(); it != end; ++it) {
			debugPrintf("\"%s\" -> %d / %d\n", (*it).name, (*it).gmInstr, (*it).gmRhythmKey);
		}
	}

	return true;
}

bool Console::cmdAudioList(int argc, const char **argv) {
#ifdef ENABLE_SCI32
	if (_engine->_audio32) {
		debugPrintf("Audio list (%d active channels):\n", _engine->_audio32->getNumActiveChannels());
		_engine->_audio32->printAudioList(this);
	} else {
		debugPrintf("This SCI version does not have a software digital audio mixer\n");
	}
#else
	debugPrintf("SCI32 isn't included in this compiled executable\n");
#endif

	return true;
}

bool Console::cmdAudioDump(int argc, const char **argv) {
#ifdef ENABLE_SCI32
	if (argc != 2 && argc != 6) {
		debugPrintf("Dumps the requested audio resource as an uncompressed wave file.\n");
		debugPrintf("Usage (audio): %s <audio resource id>\n", argv[0]);
		debugPrintf("Usage (audio36): %s <audio map id> <noun> <verb> <cond> <seq>\n", argv[0]);
		return true;
	}

	ResourceId id;
	if (argc == 2) {
		id = ResourceId(kResourceTypeAudio, atoi(argv[1]));
	} else {
		id = ResourceId(kResourceTypeAudio36, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
	}

	Resource *resource = _engine->_resMan->findResource(id, false);
	if (!resource) {
		debugPrintf("Not found.\n");
		return true;
	}

	Common::MemoryReadStream stream = resource->toStream();

	Common::DumpFile outFile;
	const Common::Path fileName(Common::String::format("%s.wav", id.toString().c_str()));
	if (!outFile.open(fileName)) {
		debugPrintf("Could not open dump file %s.\n", fileName.toString(Common::Path::kNativeSeparator).c_str());
		return true;
	}

	const bool isSol = detectSolAudio(stream);
	const bool isWave = !isSol && detectWaveAudio(stream);
	const bool isRaw = !isSol && !isWave;

	if (isSol || isRaw) {
		uint16 sampleRate = 11025;
		int numChannels = 1;
		int bytesPerSample = 1;
		bool sourceIs8Bit = true;
		uint32 compressedSize = 0;
		uint32 decompressedSize;

		if (isSol) {
			stream.seek(6, SEEK_SET);
			sampleRate = stream.readUint16LE();
			const byte flags = stream.readByte();
			compressedSize = stream.readUint32LE();

			// All AudioStreams must output 16-bit samples
			bytesPerSample = 2;

			decompressedSize = compressedSize;

			if (flags & kCompressed) {
				decompressedSize *= 2;
			}
			if (flags & k16Bit) {
				sourceIs8Bit = false;
			} else {
				// 8-bit is implicitly up-converted by AudioStream to 16-bit
				decompressedSize *= 2;
			}
			if (flags & kStereo) {
				numChannels = 2;
			}
		} else {
			decompressedSize = resource->size();
		}

		enum {
			kWaveHeaderSize = 36
		};

		outFile.writeString("RIFF");
		outFile.writeUint32LE(kWaveHeaderSize + decompressedSize);
		outFile.writeString("WAVEfmt ");
		outFile.writeUint32LE(16);
		outFile.writeUint16LE(1);
		outFile.writeUint16LE(numChannels);
		outFile.writeUint32LE(sampleRate);
		outFile.writeUint32LE(sampleRate * bytesPerSample * numChannels);
		outFile.writeUint16LE(bytesPerSample * numChannels);
		outFile.writeUint16LE(bytesPerSample * 8);
		outFile.writeString("data");
		outFile.writeUint32LE(decompressedSize);

		if (isSol) {
			stream.seek(0, SEEK_SET);
			Common::ScopedPtr<Audio::SeekableAudioStream> audioStream(makeSOLStream(&stream, DisposeAfterUse::NO));

			if (!audioStream) {
				debugPrintf("Could not create SOL stream.\n");
				return true;
			}

			int16 buffer[2048];
			const int samplesToRead = ARRAYSIZE(buffer);
			uint bytesWritten = 0;
			int samplesRead;
			while ((samplesRead = audioStream->readBuffer(buffer, samplesToRead))) {
				uint bytesToWrite = samplesRead * bytesPerSample;
				outFile.write(buffer, bytesToWrite);
				bytesWritten += bytesToWrite;
			}

			if (bytesWritten != decompressedSize) {
				debugPrintf("WARNING: Should have written %u bytes but wrote %u bytes!\n", decompressedSize, bytesWritten);
				while (bytesWritten < decompressedSize) {
					outFile.writeByte(0);
					++bytesWritten;
				}
			}

			const char *bits;
			if (sourceIs8Bit) {
				bits = "upconverted 16";
			} else {
				bits = "16";
			}

			debugPrintf("%s-bit %uHz %d-channel SOL audio, %u -> %u bytes\n", bits, sampleRate, numChannels, compressedSize, decompressedSize);
		} else {
			outFile.write(resource->data(), resource->size());
			debugPrintf("%d-bit %uHz %d-channel raw audio, %u bytes\n", bytesPerSample * 8, sampleRate, numChannels, decompressedSize);
		}
	} else if (isWave) {
		outFile.write(resource->data(), resource->size());
		debugPrintf("Raw wave file\n");
	} else {
		error("Impossible situation");
	}

	debugPrintf("Written to %s successfully.\n", fileName.toString(Common::Path::kNativeSeparator).c_str());
#else
	debugPrintf("SCI32 isn't included in this compiled executable\n");
#endif
	return true;
}

bool Console::cmdSaveGame(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Saves the current game state to the hard disk\n");
		debugPrintf("Usage: %s <filename>\n", argv[0]);
		return true;
	}

	int result = 0;
	for (uint i = 0; i < _engine->_gamestate->_fileHandles.size(); i++)
		if (_engine->_gamestate->_fileHandles[i].isOpen())
			result++;

	if (result)
		debugPrintf("Note: Game state has %d open file handles.\n", result);

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out = saveFileMan->openForSaving(argv[1]);
	const char *version = "";
	if (!out) {
		debugPrintf("Error opening savegame \"%s\" for writing\n", argv[1]);
		return true;
	}

	// TODO: enable custom descriptions? force filename into a specific format?
	if (!gamestate_save(_engine->_gamestate, out, "debugging", version)) {
		debugPrintf("Saving the game state to '%s' failed\n", argv[1]);
	} else {
		out->finalize();
		if (out->err()) {
			warning("Writing the savegame failed");
		}
		delete out;
	}

	return true;
}

bool Console::cmdRestoreGame(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Restores a saved game from the hard disk\n");
		debugPrintf("Usage: %s <filename>\n", argv[0]);
		return true;
	}

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::SeekableReadStream *in = saveFileMan->openForLoading(argv[1]);
	if (in) {
		// found a savegame file
		gamestate_restore(_engine->_gamestate, in);
		delete in;
	}

	if (_engine->_gamestate->r_acc == make_reg(0, 1)) {
		debugPrintf("Restoring gamestate '%s' failed.\n", argv[1]);
		return true;
	}

	return cmdExit(0, nullptr);
}

bool Console::cmdRestartGame(int argc, const char **argv) {
#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		debugPrintf("This SCI version does not support this command\n");
		return true;
	}
#endif
	_engine->_gamestate->abortScriptProcessing = kAbortRestartGame;

	return cmdExit(0, nullptr);
}

// The scripts get IDs ranging from 100->199, because the scripts require us to assign unique ids THAT EVEN STAY BETWEEN
//  SAVES and the scripts also use "saves-count + 1" to create a new savedgame slot.
//  SCI1.1 actually recycles ids, in that case we will currently get "0".
// This behavior is required especially for LSL6. In this game, it's possible to quick save. The scripts will use
//  the last-used id for that feature. If we don't assign sticky ids, the feature will overwrite different saves all the
//  time. And sadly we can't just use the actual filename ids directly, because of the creation method for new slots.

bool Console::cmdListSaves(int argc, const char **argv) {
	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	for (uint i = 0; i < saves.size(); i++) {
		Common::String filename = g_sci->getSavegameName(saves[i].id);
		debugPrintf("%s: '%s'\n", filename.c_str(), saves[i].name);
	}

	return true;
}

bool Console::cmdClassTable(int argc, const char **argv) {
	debugPrintf("Available classes (pass a parameter to filter the table by a specific class):\n");

	for (uint i = 0; i < _engine->_gamestate->_segMan->classTableSize(); i++) {
		Class temp = _engine->_gamestate->_segMan->_classTable[i];
		if (temp.reg.getSegment()) {
			const char *className = _engine->_gamestate->_segMan->getObjectName(temp.reg);
			if (argc == 1 || (argc == 2 && !strcmp(className, argv[1]))) {
				debugPrintf(" Class 0x%x (%s) at %04x:%04x (script %d)\n", i,
						className,
						PRINT_REG(temp.reg),
						temp.script);
			}
		} else if (argc == 1) {
			debugPrintf(" Class 0x%x (not loaded; can't get name) (script %d)\n", i, temp.script);
		}
	}

	return true;
}

bool Console::cmdSentenceFragments(int argc, const char **argv) {
	debugPrintf("Sentence fragments (used to build Parse trees)\n");

	for (uint i = 0; i < _engine->getVocabulary()->getParserBranchesSize(); i++) {
		int j = 0;

		const parse_tree_branch_t &branch = _engine->getVocabulary()->getParseTreeBranch(i);
		debugPrintf("R%02d: [%x] ->", i, branch.id);
		while ((j < 10) && branch.data[j]) {
			int dat = branch.data[j++];

			switch (dat) {
			case VOCAB_TREE_NODE_COMPARE_TYPE:
				dat = branch.data[j++];
				debugPrintf(" C(%x)", dat);
				break;

			case VOCAB_TREE_NODE_COMPARE_GROUP:
				dat = branch.data[j++];
				debugPrintf(" WG(%x)", dat);
				break;

			case VOCAB_TREE_NODE_FORCE_STORAGE:
				dat = branch.data[j++];
				debugPrintf(" FORCE(%x)", dat);
				break;

			default:
				if (dat > VOCAB_TREE_NODE_LAST_WORD_STORAGE) {
					int dat2 = branch.data[j++];
					debugPrintf(" %x[%x]", dat, dat2);
				} else
					debugPrintf(" ?%x?", dat);
			}
		}
		debugPrintf("\n");
	}

	debugPrintf("%d rules.\n", _engine->getVocabulary()->getParserBranchesSize());

	return true;
}

bool Console::cmdParse(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Parses a sequence of words with a GNF rule set and prints the resulting parse tree\n");
		debugPrintf("Usage: %s <word1> <word2> ... <wordn>\n", argv[0]);
		return true;
	}

	char *error;
	Common::String string = argv[1];

	// Construct the string
	for (int i = 2; i < argc; i++) {
		string += " ";
		string += argv[i];
	}

	debugPrintf("Parsing '%s'\n", string.c_str());

	ResultWordListList words;
	bool res = _engine->getVocabulary()->tokenizeString(words, string.c_str(), &error);
	if (res && !words.empty()) {
		int syntax_fail = 0;

		_engine->getVocabulary()->synonymizeTokens(words);

		debugPrintf("Parsed to the following blocks:\n");

		for (ResultWordListList::const_iterator i = words.begin(); i != words.end(); ++i) {
			debugPrintf("   ");
			for (ResultWordList::const_iterator j = i->begin(); j != i->end(); ++j) {
				debugPrintf("%sType[%04x] Group[%04x]", j == i->begin() ? "" : " / ", j->_class, j->_group);
			}
			debugPrintf("\n");
		}

		if (_engine->getVocabulary()->parseGNF(words, true))
			syntax_fail = 1; // Building a tree failed

		if (syntax_fail)
			debugPrintf("Building a tree failed.\n");
		else
			_engine->getVocabulary()->dumpParseTree();

	} else {
		debugPrintf("Unknown word: '%s'\n", error);
		free(error);
	}

	return true;
}

bool Console::cmdSaid(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Matches a string against a said spec\n");
		debugPrintf("Usage: %s <string> > & <said spec>\n", argv[0]);
		debugPrintf("<string> is a sequence of actual words.\n");
		debugPrintf("<said spec> is a sequence of hex tokens.\n");
		return true;
	}

	char *error;
	Common::String string = argv[1];
	byte spec[1000];

	int p;
	// Construct the string
	for (p = 2; p < argc && strcmp(argv[p],"&") != 0; p++) {
		string += " ";
		string += argv[p];
	}

	if (p >= argc-1) {
		debugPrintf("Matches a string against a said spec\n");
		debugPrintf("Usage: %s <string> > & <said spec>\n", argv[0]);
		debugPrintf("<string> is a sequence of actual words.\n");
		debugPrintf("<said spec> is a sequence of hex tokens.\n");
		return true;
	}

	// TODO: Maybe turn this into a proper said spec compiler
	uint32 len = 0;
	for (p++; p < argc; p++) {
		if (strcmp(argv[p], ",") == 0) {
			spec[len++] = 0xf0;
		} else if (strcmp(argv[p], "&") == 0) {
			spec[len++] = 0xf1;
		} else if (strcmp(argv[p], "/") == 0) {
			spec[len++] = 0xf2;
		} else if (strcmp(argv[p], "(") == 0) {
			spec[len++] = 0xf3;
		} else if (strcmp(argv[p], ")") == 0) {
			spec[len++] = 0xf4;
		} else if (strcmp(argv[p], "[") == 0) {
			spec[len++] = 0xf5;
		} else if (strcmp(argv[p], "]") == 0) {
			spec[len++] = 0xf6;
		} else if (strcmp(argv[p], "#") == 0) {
			spec[len++] = 0xf7;
		} else if (strcmp(argv[p], "<") == 0) {
			spec[len++] = 0xf8;
		} else if (strcmp(argv[p], ">") == 0) {
			spec[len++] = 0xf9;
		} else if (strcmp(argv[p], "[<") == 0) {
			spec[len++] = 0xf5;
			spec[len++] = 0xf8;
		} else if (strcmp(argv[p], "[/") == 0) {
			spec[len++] = 0xf5;
			spec[len++] = 0xf2;
		} else if (strcmp(argv[p], "!*") == 0) {
			spec[len++] = 0x0f;
			spec[len++] = 0xfe;
		} else if (strcmp(argv[p], "[!*]") == 0) {
			spec[len++] = 0xf5;
			spec[len++] = 0x0f;
			spec[len++] = 0xfe;
			spec[len++] = 0xf6;
		} else {
			uint32 s = strtol(argv[p], nullptr, 16);
			if (s >= 0xf0 && s <= 0xff) {
				spec[len++] = s;
			} else {
				spec[len++] = s >> 8;
				spec[len++] = s & 0xFF;
			}
		}
	}
	spec[len++] = 0xFF;

	debugN("Matching '%s' against:", string.c_str());
	_engine->getVocabulary()->debugDecipherSaidBlock(SciSpan<const byte>(spec, len));
	debugN("\n");

	ResultWordListList words;
	bool res = _engine->getVocabulary()->tokenizeString(words, string.c_str(), &error);
	if (res && !words.empty()) {
		int syntax_fail = 0;

		_engine->getVocabulary()->synonymizeTokens(words);

		debugPrintf("Parsed to the following blocks:\n");

		for (ResultWordListList::const_iterator i = words.begin(); i != words.end(); ++i) {
			debugPrintf("   ");
			for (ResultWordList::const_iterator j = i->begin(); j != i->end(); ++j) {
				debugPrintf("%sType[%04x] Group[%04x]", j == i->begin() ? "" : " / ", j->_class, j->_group);
			}
			debugPrintf("\n");
		}



		if (_engine->getVocabulary()->parseGNF(words, true))
			syntax_fail = 1; // Building a tree failed

		if (syntax_fail)
			debugPrintf("Building a tree failed.\n");
		else {
			_engine->getVocabulary()->dumpParseTree();
			_engine->getVocabulary()->parserIsValid = true;

			int ret = said((byte *)spec, true);
			debugPrintf("kSaid: %s\n", (ret == SAID_NO_MATCH ? "No match" : "Match"));
		}

	} else {
		debugPrintf("Unknown word: '%s'\n", error);
		free(error);
	}

	return true;
}


bool Console::cmdParserNodes(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Shows the specified number of nodes from the parse node tree\n");
		debugPrintf("Usage: %s <nr>\n", argv[0]);
		debugPrintf("where <nr> is the number of nodes to show from the parse node tree\n");
		return true;
	}

	int end = MIN<int>(atoi(argv[1]), VOCAB_TREE_NODES);

	_engine->getVocabulary()->printParserNodes(end);

	return true;
}

bool Console::cmdSetPalette(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Sets a palette resource (SCI16)\n");
		debugPrintf("Usage: %s <resourceId>\n", argv[0]);
		debugPrintf("where <resourceId> is the number of the palette resource to set\n");
		return true;
	}

	uint16 resourceId = atoi(argv[1]);

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		debugPrintf("This SCI version does not support this command\n");
		return true;
	}
#endif

	_engine->_gfxPalette16->kernelSetFromResource(resourceId, true);
	return true;
}

bool Console::cmdDrawPic(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Draws a pic resource\n");
		debugPrintf("Usage: %s <resourceId>\n", argv[0]);
		debugPrintf("where <resourceId> is the number of the pic resource to draw\n");
		return true;
	}

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	// If a graphical debugger overlay is used, hide it here, so that the
	// results can be drawn.
	g_system->hideOverlay();
#endif

	uint16 resourceId = atoi(argv[1]);
	_engine->_gfxPaint16->kernelDrawPicture(resourceId, 100, false, false, false, 0);
	_engine->_gfxScreen->copyToScreen();
	_engine->sleep(2000);

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	// Show the graphical debugger overlay
	g_system->showOverlay();
#endif

	return true;
}

bool Console::cmdDrawCel(int argc, const char **argv) {
	if (argc < 4) {
		debugPrintf("Draws a cel from a view resource\n");
		debugPrintf("Usage: %s <resourceId> <loopNr> <celNr> \n", argv[0]);
		debugPrintf("where <resourceId> is the number of the view resource to draw\n");
		return true;
	}

	uint16 resourceId = atoi(argv[1]);
	uint16 loopNo = atoi(argv[2]);
	uint16 celNo = atoi(argv[3]);

	if (_engine->_gfxPaint16) {
		_engine->_gfxPaint16->kernelDrawCel(resourceId, loopNo, celNo, 50, 50, 0, 0, 128, 128, false, NULL_REG);
	} else {
		GfxView *view = _engine->_gfxCache->getView(resourceId);
		Common::Rect celRect(50, 50, 50 + view->getWidth(loopNo, celNo), 50 + view->getHeight(loopNo, celNo));
		view->draw(celRect, celRect, celRect, loopNo, celNo, 255, 0, false);
		_engine->_gfxScreen->copyRectToScreen(celRect);
	}
	return true;
}

bool Console::cmdUndither(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Enable/disable undithering.\n");
		debugPrintf("Usage: %s <0/1>\n", argv[0]);
		return true;
	}

	bool flag = atoi(argv[1]) ? true : false;
	_engine->_gfxScreen->enableUndithering(flag);
	if (flag)
		debugPrintf("undithering ENABLED\n");
	else
		debugPrintf("undithering DISABLED\n");
	return true;
}

bool Console::cmdPicVisualize(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Enable/disable picture visualization (EGA only)\n");
		debugPrintf("Usage: %s <0/1>\n", argv[0]);
		return true;
	}

	bool state = atoi(argv[1]) ? true : false;

	if (_engine->_resMan->getViewType() == kViewEga) {
		_engine->_gfxPaint16->debugSetEGAdrawingVisualize(state);
		if (state)
			debugPrintf("picture visualization ENABLED\n");
		else
			debugPrintf("picture visualization DISABLED\n");
	} else {
		debugPrintf("picture visualization only available for EGA games\n");
	}
	return true;
}

bool Console::cmdPlayVideo(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Plays a SEQ or AVI video.\n");
		debugPrintf("Usage: %s <video file name> <delay>\n", argv[0]);
		debugPrintf("The video file name should include the extension\n");
		debugPrintf("Delay is only used in SEQ videos and is measured in ticks (default: 10)\n");
		return true;
	}

	Common::String filename = argv[1];
	filename.toLowercase();

	if (filename.hasSuffix(".seq") || filename.hasSuffix(".avi")) {
		_videoFile = filename;
		_videoFrameDelay = (argc == 2) ? 10 : atoi(argv[2]);
		return cmdExit(0, nullptr);
	} else {
		debugPrintf("Unknown video file type\n");
		return true;
	}
}

bool Console::cmdAnimateList(int argc, const char **argv) {
	if (_engine->_gfxAnimate) {
		debugPrintf("Animate list:\n");
		_engine->_gfxAnimate->printAnimateList(this);
	} else {
		debugPrintf("This SCI version does not have an animate list\n");
	}
	return true;
}

bool Console::cmdWindowList(int argc, const char **argv) {
	if (_engine->_gfxPorts) {
		debugPrintf("Window list:\n");
		_engine->_gfxPorts->printWindowList(this);
	} else {
		debugPrintf("This SCI version does not have a list of ports\n");
	}
	return true;
}

bool Console::cmdPlaneList(int argc, const char **argv) {
#ifdef ENABLE_SCI32
	if (_engine->_gfxFrameout) {
		debugPrintf("Plane list:\n");
		_engine->_gfxFrameout->printPlaneList(this);
	} else {
		debugPrintf("This SCI version does not have a list of planes\n");
	}
#else
	debugPrintf("SCI32 isn't included in this compiled executable\n");
#endif
	return true;
}

bool Console::cmdVisiblePlaneList(int argc, const char **argv) {
#ifdef ENABLE_SCI32
	if (_engine->_gfxFrameout) {
		debugPrintf("Visible plane list:\n");
		_engine->_gfxFrameout->printVisiblePlaneList(this);
	} else {
		debugPrintf("This SCI version does not have a list of planes\n");
	}
#else
	debugPrintf("SCI32 isn't included in this compiled executable\n");
#endif
	return true;
}


bool Console::cmdPlaneItemList(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Shows the list of items for a plane\n");
		debugPrintf("Usage: %s <plane address>\n", argv[0]);
		return true;
	}

	reg_t planeObject = NULL_REG;

	if (parse_reg_t(_engine->_gamestate, argv[1], &planeObject)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

#ifdef ENABLE_SCI32
	if (_engine->_gfxFrameout) {
		debugPrintf("Plane item list:\n");
		_engine->_gfxFrameout->printPlaneItemList(this, planeObject);
	} else {
		debugPrintf("This SCI version does not have a list of plane items\n");
	}
#else
	debugPrintf("SCI32 isn't included in this compiled executable\n");
#endif
	return true;
}

bool Console::cmdVisiblePlaneItemList(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Shows the list of items for a plane\n");
		debugPrintf("Usage: %s <plane address>\n", argv[0]);
		return true;
	}

	reg_t planeObject = NULL_REG;

	if (parse_reg_t(_engine->_gamestate, argv[1], &planeObject)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

#ifdef ENABLE_SCI32
	if (_engine->_gfxFrameout) {
		debugPrintf("Visible plane item list:\n");
		_engine->_gfxFrameout->printVisiblePlaneItemList(this, planeObject);
	} else {
		debugPrintf("This SCI version does not have a list of plane items\n");
	}
#else
	debugPrintf("SCI32 isn't included in this compiled executable\n");
#endif
	return true;
}

bool Console::cmdSavedBits(int argc, const char **argv) {
	SegManager *segman = _engine->_gamestate->_segMan;
	SegmentId id = segman->findSegmentByType(SEG_TYPE_HUNK);
	HunkTable* hunks = (HunkTable *)segman->getSegmentObj(id);
	if (!hunks) {
		debugPrintf("No hunk segment found.\n");
		return true;
	}

	Common::Array<reg_t> entries = hunks->listAllDeallocatable(id);

	for (uint i = 0; i < entries.size(); ++i) {
		uint32 offset = entries[i].getOffset();
		const Hunk& h = hunks->at(offset);
		if (strcmp(h.type, "SaveBits()") == 0) {
			byte* memoryPtr = (byte *)h.mem;

			if (memoryPtr) {
				debugPrintf("%04x:%04x:", PRINT_REG(entries[i]));

				Common::Rect rect;
				byte mask;
				assert(h.size >= sizeof(rect) + sizeof(mask));

				memcpy((void *)&rect, memoryPtr, sizeof(rect));
				memcpy((void *)&mask, memoryPtr + sizeof(rect), sizeof(mask));

				debugPrintf(" %d,%d - %d,%d", rect.top, rect.left,
				                              rect.bottom, rect.right);
				if (mask & GFX_SCREEN_MASK_VISUAL)
					debugPrintf(" visual");
				if (mask & GFX_SCREEN_MASK_PRIORITY)
					debugPrintf(" priority");
				if (mask & GFX_SCREEN_MASK_CONTROL)
					debugPrintf(" control");
				debugPrintf("\n");
			}
		}
	}


	return true;
}

bool Console::cmdShowSavedBits(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Display saved bits.\n");
		debugPrintf("Usage: %s <address>\n", argv[0]);
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t memoryHandle = NULL_REG;

	if (parse_reg_t(_engine->_gamestate, argv[1], &memoryHandle)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	if (memoryHandle.isNull()) {
		debugPrintf("Invalid address.\n");
		return true;
	}

	SegManager *segman = _engine->_gamestate->_segMan;
	SegmentId id = segman->findSegmentByType(SEG_TYPE_HUNK);
	HunkTable* hunks = (HunkTable *)segman->getSegmentObj(id);
	if (!hunks) {
		debugPrintf("No hunk segment found.\n");
		return true;
	}

	if (memoryHandle.getSegment() != id || !hunks->isValidOffset(memoryHandle.getOffset())) {
		debugPrintf("Invalid address.\n");
		return true;
	}

	const Hunk& h = hunks->at(memoryHandle.getOffset());

	if (strcmp(h.type, "SaveBits()") != 0) {
		debugPrintf("Invalid address.\n");
		return true;
	}

	byte *memoryPtr = segman->getHunkPointer(memoryHandle);

	if (!memoryPtr) {
		debugPrintf("Invalid or freed bits.\n");
		return true;
	}

	// Now we _finally_ know these are valid saved bits

	Common::Rect rect;
	byte mask;
	assert(h.size >= sizeof(rect) + sizeof(mask));

	memcpy((void *)&rect, memoryPtr, sizeof(rect));
	memcpy((void *)&mask, memoryPtr + sizeof(rect), sizeof(mask));

	Common::Point tl(rect.left, rect.top);
	Common::Point tr(rect.right-1, rect.top);
	Common::Point bl(rect.left, rect.bottom-1);
	Common::Point br(rect.right-1, rect.bottom-1);

	debugPrintf(" %d,%d - %d,%d", rect.top, rect.left,
	                              rect.bottom, rect.right);
	if (mask & GFX_SCREEN_MASK_VISUAL)
		debugPrintf(" visual");
	if (mask & GFX_SCREEN_MASK_PRIORITY)
		debugPrintf(" priority");
	if (mask & GFX_SCREEN_MASK_CONTROL)
		debugPrintf(" control");
	debugPrintf("\n");

	if (!_engine->_gfxPaint16 || !_engine->_gfxScreen)
		return true;

	// We backup all planes, and then flash the saved bits
	// FIXME: This probably won't work well with hi-res games

	byte bakMask = GFX_SCREEN_MASK_VISUAL | GFX_SCREEN_MASK_PRIORITY | GFX_SCREEN_MASK_CONTROL;
	int bakSize = _engine->_gfxScreen->bitsGetDataSize(rect, bakMask);
	reg_t bakScreen = segman->allocateHunkEntry("show_saved_bits backup", bakSize);
	byte* bakMemory = segman->getHunkPointer(bakScreen);
	assert(bakMemory);
	_engine->_gfxScreen->bitsSave(rect, bakMask, bakMemory);

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	// If a graphical debugger overlay is used, hide it here, so that the
	// results can be drawn.
	g_system->hideOverlay();
#endif

	const int paintCount = 3;
	for (int i = 0; i < paintCount; ++i) {
		_engine->_gfxScreen->bitsRestore(memoryPtr);
		_engine->_gfxScreen->drawLine(tl, tr, 0, 255, 255);
		_engine->_gfxScreen->drawLine(tr, br, 0, 255, 255);
		_engine->_gfxScreen->drawLine(br, bl, 0, 255, 255);
		_engine->_gfxScreen->drawLine(bl, tl, 0, 255, 255);
		_engine->_gfxScreen->copyRectToScreen(rect);
		g_system->updateScreen();
		g_sci->sleep(500);
		_engine->_gfxScreen->bitsRestore(bakMemory);
		_engine->_gfxScreen->copyRectToScreen(rect);
		g_system->updateScreen();
		if (i < paintCount - 1)
			g_sci->sleep(500);
	}

	_engine->_gfxPaint16->bitsFree(bakScreen);

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	// Show the graphical debugger overlay
	g_system->showOverlay();
#endif

	return true;
}


bool Console::cmdParseGrammar(int argc, const char **argv) {
	debugPrintf("Parse grammar, in strict GNF:\n");

	_engine->getVocabulary()->buildGNF(true);

	return true;
}

bool Console::cmdPrintSegmentTable(int argc, const char **argv) {
	debugPrintf("Segment table:\n");

	for (uint i = 0; i < _engine->_gamestate->_segMan->_heap.size(); i++) {
		SegmentObj *mobj = _engine->_gamestate->_segMan->_heap[i];
		if (mobj && mobj->getType()) {
			debugPrintf(" [%04x] ", i);

			switch (mobj->getType()) {
			case SEG_TYPE_SCRIPT:
				debugPrintf("S  script.%03d l:%d ", (*(Script *)mobj).getScriptNumber(), (*(Script *)mobj).getLockers());
				break;

			case SEG_TYPE_CLONES:
				debugPrintf("C  clones (%d allocd)", (*(CloneTable *)mobj).entries_used);
				break;

			case SEG_TYPE_LOCALS:
				debugPrintf("V  locals %03d", (*(LocalVariables *)mobj).script_id);
				break;

			case SEG_TYPE_STACK:
				debugPrintf("D  data stack (%d)", (*(DataStack *)mobj)._capacity);
				break;

			case SEG_TYPE_LISTS:
				debugPrintf("L  lists (%d)", (*(ListTable *)mobj).entries_used);
				break;

			case SEG_TYPE_NODES:
				debugPrintf("N  nodes (%d)", (*(NodeTable *)mobj).entries_used);
				break;

			case SEG_TYPE_HUNK:
				debugPrintf("H  hunk (%d)", (*(HunkTable *)mobj).entries_used);
				break;

			case SEG_TYPE_DYNMEM:
				debugPrintf("M  dynmem: %d bytes", (*(DynMem *)mobj)._size);
				break;

#ifdef ENABLE_SCI32
			case SEG_TYPE_ARRAY:
				debugPrintf("A  SCI32 arrays (%d)", (*(ArrayTable *)mobj).entries_used);
				break;

			case SEG_TYPE_BITMAP:
				debugPrintf("T  SCI32 bitmaps (%d)", (*(BitmapTable *)mobj).entries_used);
				break;
#endif

			default:
				debugPrintf("I  Invalid (type = %x)", mobj->getType());
				break;
			}

			debugPrintf("  \n");
		}
	}
	debugPrintf("\n");

	return true;
}

bool Console::segmentInfo(int nr) {
	debugPrintf("[%04x] ", nr);

	if ((nr < 0) || ((uint)nr >= _engine->_gamestate->_segMan->_heap.size()) || !_engine->_gamestate->_segMan->_heap[nr])
		return false;

	SegmentObj *mobj = _engine->_gamestate->_segMan->_heap[nr];

	switch (mobj->getType()) {

	case SEG_TYPE_SCRIPT: {
		Script *scr = (Script *)mobj;
		debugPrintf("script.%03d locked by %d, bufsize=%d (%x)\n", scr->getScriptNumber(), scr->getLockers(), (uint)scr->getBufSize(), (uint)scr->getBufSize());
		if (scr->getExportsNr()) {
			const uint location = scr->getExportsOffset();
			debugPrintf("  Exports: %4d at %d\n", scr->getExportsNr(), location);
		} else
			debugPrintf("  Exports: none\n");

		debugPrintf("  Synonyms: %4d\n", scr->getSynonymsNr());

		if (scr->getLocalsCount() > 0)
			debugPrintf("  Locals : %4d in segment 0x%x\n", scr->getLocalsCount(), scr->getLocalsSegment());
		else
			debugPrintf("  Locals : none\n");

		const ObjMap &objects = scr->getObjectMap();
		debugPrintf("  Objects: %4d\n", objects.size());

		ObjMap::const_iterator it;
		const ObjMap::const_iterator end = objects.end();
		for (it = objects.begin(); it != end; ++it) {
			debugPrintf("    ");
			// Object header
			const Object *obj = _engine->_gamestate->_segMan->getObject(it->_value.getPos());
			if (obj)
				debugPrintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(it->_value.getPos()),
							_engine->_gamestate->_segMan->getObjectName(it->_value.getPos()),
							obj->getVarCount(), obj->getMethodCount());
		}
	}
	break;

	case SEG_TYPE_LOCALS: {
		LocalVariables *locals = (LocalVariables *)mobj;
		debugPrintf("locals for script.%03d\n", locals->script_id);
		debugPrintf("  %d (0x%x) locals\n", locals->_locals.size(), locals->_locals.size());
	}
	break;

	case SEG_TYPE_STACK: {
		DataStack *stack = (DataStack *)mobj;
		debugPrintf("stack\n");
		debugPrintf("  %d (0x%x) entries\n", stack->_capacity, stack->_capacity);
	}
	break;

	case SEG_TYPE_CLONES: {
		CloneTable &ct = *(CloneTable *)mobj;

		debugPrintf("clones\n");

		for (uint i = 0; i < ct.size(); i++)
			if (ct.isValidEntry(i)) {
				reg_t objpos = make_reg(nr, i);
				debugPrintf("  [%04x] %s; copy of ", i, _engine->_gamestate->_segMan->getObjectName(objpos));
				// Object header
				const Object *obj = _engine->_gamestate->_segMan->getObject(ct[i].getPos());
				if (obj)
					debugPrintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(ct[i].getPos()),
								_engine->_gamestate->_segMan->getObjectName(ct[i].getPos()),
								obj->getVarCount(), obj->getMethodCount());
			}
	}
	break;

	case SEG_TYPE_LISTS: {
		ListTable &lt = *(ListTable *)mobj;

		debugPrintf("lists\n");
		for (uint i = 0; i < lt.size(); i++)
			if (lt.isValidEntry(i)) {
				debugPrintf("  [%04x]: ", i);
				printList(lt[i]);
			}
	}
	break;

	case SEG_TYPE_NODES: {
		debugPrintf("nodes (total %d)\n", (*(NodeTable *)mobj).entries_used);
		break;
	}

	case SEG_TYPE_HUNK: {
		HunkTable &ht = *(HunkTable *)mobj;

		debugPrintf("hunk  (total %d)\n", ht.entries_used);
		for (uint i = 0; i < ht.size(); i++)
			if (ht.isValidEntry(i)) {
				debugPrintf("    [%04x] %d bytes at %p, type=%s\n",
				          i, ht[i].size, ht[i].mem, ht[i].type);
			}
	}
	break;

	case SEG_TYPE_DYNMEM: {
		debugPrintf("dynmem (%s): %d bytes\n",
		          (*(DynMem *)mobj)._description.c_str(), (*(DynMem *)mobj)._size);

		Common::hexdump((*(DynMem *)mobj)._buf, (*(DynMem *)mobj)._size, 16, 0);
	}
	break;

#ifdef ENABLE_SCI32
	case SEG_TYPE_ARRAY: {
		ArrayTable &table = *(ArrayTable *)mobj;
		debugPrintf("SCI32 arrays\n");
		for (uint i = 0; i < table.size(); ++i) {
			if (table.isValidEntry(i)) {
				debugPrintf("    [%04x] %s\n", i, table[i].toDebugString().c_str());
			}
		}
		break;
	}

	case SEG_TYPE_BITMAP: {
		BitmapTable &table = *(BitmapTable *)mobj;
		debugPrintf("SCI32 bitmaps (total %d)\n", table.entries_used);
		for (uint i = 0; i < table.size(); ++i) {
			if (table.isValidEntry(i)) {
				debugPrintf("    [%04x] %s\n", i, table[i].toString().c_str());
			}
		}
		break;
	}
#endif

	default :
		debugPrintf("Invalid type %d\n", mobj->getType());
		break;
	}

	debugPrintf("\n");
	return true;
}

bool Console::cmdSegmentInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Provides information on the specified segment(s)\n");
		debugPrintf("Usage: %s <segment number>\n", argv[0]);
		debugPrintf("<segment number> can be a number, which shows the information of the segment with\n");
		debugPrintf("the specified number, or \"all\" to show information on all active segments\n");
		return true;
	}

	if (!scumm_stricmp(argv[1], "all")) {
		for (uint i = 0; i < _engine->_gamestate->_segMan->_heap.size(); i++)
			segmentInfo(i);
	} else {
		int segmentNr;
		if (!parseInteger(argv[1], segmentNr))
			return true;
		if (!segmentInfo(segmentNr))
			debugPrintf("Segment %04xh does not exist\n", segmentNr);
	}

	return true;
}


bool Console::cmdKillSegment(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Deletes the specified segment\n");
		debugPrintf("Usage: %s <segment number>\n", argv[0]);
		return true;
	}
	int segmentNumber;
	if (!parseInteger(argv[1], segmentNumber))
		return true;
	_engine->_gamestate->_segMan->getScript(segmentNumber)->setLockers(0);

	return true;
}

bool Console::cmdShowMap(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Switches to one of the following screen maps\n");
		debugPrintf("Usage: %s <screen map>\n", argv[0]);
		debugPrintf("Screen maps:\n");
		debugPrintf("- 0: visual map\n");
		debugPrintf("- 1: priority map\n");
		debugPrintf("- 2: control map\n");
		debugPrintf("- 3: display screen\n");
		return true;
	}

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		debugPrintf("Command not available / implemented for SCI32 games.\n");
		return true;
	}
#endif

	int map = atoi(argv[1]);

	switch (map) {
	case 0:
	case 1:
	case 2:
	case 3:
		if (_engine->_gfxScreen) {
			_engine->_gfxScreen->debugShowMap(map);
		}
		break;

	default:
		debugPrintf("Map %d is not available.\n", map);
		return true;
	}
	return cmdExit(0, nullptr);
}

bool Console::cmdSongLib(int argc, const char **argv) {
	debugPrintf("Song library:\n");
	g_sci->_soundCmd->printPlayList(this);

	return true;
}

bool Console::cmdSongInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Shows information about a given song in the playlist\n");
		debugPrintf("Usage: %s <song object>\n", argv[0]);
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	g_sci->_soundCmd->printSongInfo(addr, this);

	return true;
}

bool Console::cmdStartSound(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Adds the requested sound resource to the playlist, and starts playing it\n");
		debugPrintf("Usage: %s <sound resource id>\n", argv[0]);
		return true;
	}

	int16 number = atoi(argv[1]);

	if (!_engine->getResMan()->testResource(ResourceId(kResourceTypeSound, number))) {
		debugPrintf("Unable to load this sound resource, most probably it has an equivalent audio resource (SCI1.1)\n");
		return true;
	}

	// TODO: Maybe also add a playBed option.
	g_sci->_soundCmd->startNewSound(number);
	return cmdExit(0, nullptr);
}

bool Console::cmdToggleSound(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Plays or stops the specified sound in the playlist\n");
		debugPrintf("Usage: %s <address> <state>\n", argv[0]);
		debugPrintf("Where:\n");
		debugPrintf("- <address> is the address of the sound to play or stop.\n");
		debugPrintf("- <state> is the new state (play or stop).\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t id;

	if (parse_reg_t(_engine->_gamestate, argv[1], &id)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	Common::String newState = argv[2];
	newState.toLowercase();

	if (newState == "play") {
		// Maybe also have a 'playbed' option. (Second argument to processPlaySound.)
		g_sci->_soundCmd->processPlaySound(id, false);
	} else if (newState == "stop")
		g_sci->_soundCmd->processStopSound(id, false);
	else
		debugPrintf("New state can either be 'play' or 'stop'\n");

	return true;
}

bool Console::cmdStopAllSounds(int argc, const char **argv) {
	g_sci->_soundCmd->stopAllSounds();

	debugPrintf("All sounds have been stopped\n");
	return true;
}

bool Console::cmdIsSample(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Tests whether a given sound resource is a PCM sample, \n");
		debugPrintf("and displays information on it if it is.\n");
		debugPrintf("Usage: %s <sample id>\n", argv[0]);
		return true;
	}

	int16 number = atoi(argv[1]);

	if (!_engine->getResMan()->testResource(ResourceId(kResourceTypeSound, number))) {
		debugPrintf("Unable to load this sound resource, most probably it has an equivalent audio resource (SCI1.1)\n");
		return true;
	}

	SoundResource soundRes(number, _engine->getResMan(), _engine->_features->detectDoSoundType());

	if (!soundRes.exists()) {
		debugPrintf("Not a sound resource!\n");
		return true;
	}

	SoundResource::Track *track = soundRes.getDigitalTrack();
	if (!track || track->digitalChannelNr == -1) {
		debugPrintf("Valid song, but not a sample.\n");
		return true;
	}

	debugPrintf("Sample size: %d, sample rate: %d, channels: %d, digital channel number: %d\n",
			track->digitalSampleSize, track->digitalSampleRate, track->channelCount, track->digitalChannelNr);

	return true;
}

bool Console::cmdGCInvoke(int argc, const char **argv) {
	debugPrintf("Performing garbage collection...\n");
	run_gc(_engine->_gamestate);
	return true;
}

bool Console::cmdGCObjects(int argc, const char **argv) {
	AddrSet *use_map = findAllActiveReferences(_engine->_gamestate);

	debugPrintf("Reachable object references (normalised):\n");
	for (AddrSet::iterator i = use_map->begin(); i != use_map->end(); ++i) {
		debugPrintf(" - %04x:%04x\n", PRINT_REG(i->_key));
	}

	delete use_map;

	return true;
}

bool Console::cmdGCShowReachable(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Prints all addresses directly reachable from the memory object specified as parameter.\n");
		debugPrintf("Usage: %s <address>\n", argv[0]);
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegmentObj(addr.getSegment());
	if (!mobj) {
		debugPrintf("Unknown segment : %x\n", addr.getSegment());
		return 1;
	}

	debugPrintf("Reachable from %04x:%04x:\n", PRINT_REG(addr));
	const Common::Array<reg_t> tmp = mobj->listAllOutgoingReferences(addr);
	for (Common::Array<reg_t>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
		if (it->getSegment())
			g_sci->getSciDebugger()->debugPrintf("  %04x:%04x\n", PRINT_REG(*it));

	return true;
}

bool Console::cmdGCShowFreeable(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Prints all addresses freeable in the segment associated with the\n");
		debugPrintf("given address (offset is ignored).\n");
		debugPrintf("Usage: %s <address>\n", argv[0]);
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegmentObj(addr.getSegment());
	if (!mobj) {
		debugPrintf("Unknown segment : %x\n", addr.getSegment());
		return true;
	}

	debugPrintf("Freeable in segment %04x:\n", addr.getSegment());
	const Common::Array<reg_t> tmp = mobj->listAllDeallocatable(addr.getSegment());
	for (Common::Array<reg_t>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
		if (it->getSegment())
			g_sci->getSciDebugger()->debugPrintf("  %04x:%04x\n", PRINT_REG(*it));

	return true;
}

bool Console::cmdGCNormalize(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Prints the \"normal\" address of a given address,\n");
		debugPrintf("i.e. the address we would free in order to free\n");
		debugPrintf("the object associated with the original address.\n");
		debugPrintf("Usage: %s <address>\n", argv[0]);
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegmentObj(addr.getSegment());
	if (!mobj) {
		debugPrintf("Unknown segment : %x\n", addr.getSegment());
		return true;
	}

	addr = mobj->findCanonicAddress(_engine->_gamestate->_segMan, addr);
	debugPrintf(" %04x:%04x\n", PRINT_REG(addr));

	return true;
}

bool Console::cmdVMVarlist(int argc, const char **argv) {
	EngineState *s = _engine->_gamestate;
	const char *varnames[] = {"global", "local", "temp", "param"};

	debugPrintf("Addresses of variables in the VM:\n");

	for (int i = 0; i < 4; i++) {
		debugPrintf("%s vars at %04x:%04x ", varnames[i], PRINT_REG(make_reg(s->variablesSegment[i], s->variables[i] - s->variablesBase[i])));
		debugPrintf("  total %d", s->variablesMax[i]);
		debugPrintf("\n");
	}

	return true;
}

bool Console::cmdVMVars(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Displays or changes variables in the VM\n");
		debugPrintf("Usage: %s <type> <varnum> [<value>]\n", argv[0]);
		debugPrintf("First parameter is either g(lobal), l(ocal), t(emp), p(aram) or a(cc).\n");
		debugPrintf("Second parameter is the var number (not specified on acc)\n");
		debugPrintf("Third parameter (if specified) is the value to set the variable to, in address form\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	EngineState *s = _engine->_gamestate;
	const char *varNames[] = {"global", "local", "temp", "param", "acc"};
	const char *varAbbrev = "gltpa";
	const char *varType_pre = strchr(varAbbrev, *argv[1]);
	int varType;
	int varIndex = 0;
	reg_t *curValue = nullptr;
	const char *setValue = nullptr;

	if (!varType_pre) {
		debugPrintf("Invalid variable type '%c'\n", *argv[1]);
		return true;
	}

	varType = varType_pre - varAbbrev;

	switch (varType) {
	case 0:
	case 1:
	case 2:
	case 3: {
		if (argc < 3) {
			for (int i = 0; i < s->variablesMax[varType]; ++i) {
				curValue = &s->variables[varType][i];
				debugPrintf("%s var %d == %04x:%04x", varNames[varType], i, PRINT_REG(*curValue));
				printBasicVarInfo(*curValue);
				debugPrintf("\n");
			}

			return true;
		}
		if (argc > 4) {
			debugPrintf("Too many arguments\n");
			return true;
		}

		if (!parseInteger(argv[2], varIndex))
			return true;

		if (varIndex < 0) {
			debugPrintf("Variable number may not be negative\n");
			return true;
		}

		if (s->variablesMax[varType] <= varIndex) {
			debugPrintf("Maximum variable number for this type is %d (0x%x)\n", s->variablesMax[varType], s->variablesMax[varType]);
			return true;
		}
		curValue = &s->variables[varType][varIndex];
		if (argc == 4)
			setValue = argv[3];
		break;
	}

	case 4:
		// acc
		if (argc > 3) {
			debugPrintf("Too many arguments\n");
			return true;
		}
		curValue = &s->r_acc;
		if (argc == 3)
			setValue = argv[2];
		break;

	default:
		break;
	}

	if (!setValue) {
		if (varType == 4)
			debugPrintf("%s == %04x:%04x", varNames[varType], PRINT_REG(*curValue));
		else
			debugPrintf("%s var %d == %04x:%04x", varNames[varType], varIndex, PRINT_REG(*curValue));
		printBasicVarInfo(*curValue);
		debugPrintf("\n");
	} else {
		if (parse_reg_t(s, setValue, curValue)) {
			debugPrintf("Invalid value/address passed.\n");
			debugPrintf("Check the \"addresses\" command on how to use addresses\n");
			debugPrintf("Or pass a decimal or hexadecimal value directly (e.g. 12, 1Ah)\n");
			return true;
		}
	}
	return true;
}

bool Console::cmdLocalVars(int argc, const char **argv) {
	if (!(2 <= argc && argc <= 4)) {
		debugPrintf("Displays or changes local variables in the VM\n");
		debugPrintf("Usage: %s <script> <varnum> [<value>]\n", argv[0]);
		return true;
	}

	int scriptNumber;
	if (!parseInteger(argv[1], scriptNumber) || scriptNumber < 0) {
		debugPrintf("Invalid script: %s\n", argv[1]);
		return true;
	}

	// search segment table for script locals
	Common::Array<reg_t> *locals = nullptr;
	for (uint i = 0; i < _engine->_gamestate->_segMan->_heap.size(); i++) {
		SegmentObj *segmentObj = _engine->_gamestate->_segMan->_heap[i];
		if (segmentObj != nullptr && segmentObj->getType() == SEG_TYPE_LOCALS) {
			LocalVariables *localVariables = (LocalVariables *)segmentObj;
			if (localVariables->script_id == scriptNumber) {
				locals = &localVariables->_locals;
				break;
			}
		}
	}
	if (locals == nullptr) {
		debugPrintf("No locals for script: %d\n", scriptNumber);
		return true;
	}

	int varIndex = -1;
	if (argc >= 3) {
		if (!parseInteger(argv[2], varIndex) || varIndex < 0) {
			debugPrintf("Variable number may not be negative\n");
			return true;
		}
		if (varIndex >= (int)locals->size()) {
			debugPrintf("Maximum variable number for this type is %d (0x%x)\n", locals->size(), locals->size());
			return true;
		}
	}

	if (argc <= 3) {
		// print script local(s)
		for (uint i = 0; i < locals->size(); i++) {
			if (varIndex == -1 || varIndex == (int)i) {
				reg_t value = (*locals)[i];
				debugPrintf("local var %d == %04x:%04x", i, PRINT_REG(value));
				printBasicVarInfo(value);
				debugPrintf("\n");
			}
		}
	} else {
		// change script local
		reg_t *value = &(*locals)[varIndex];
		if (parse_reg_t(_engine->_gamestate, argv[3], value)) {
			debugPrintf("Invalid value/address passed.\n");
			debugPrintf("Check the \"addresses\" command on how to use addresses\n");
			debugPrintf("Or pass a decimal or hexadecimal value directly (e.g. 12, 1Ah)\n");
		}
	}

	return true;
}

bool Console::cmdStack(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Lists the specified number of stack elements.\n");
		debugPrintf("Usage: %s <elements>\n", argv[0]);
		return true;
	}

	if (_engine->_gamestate->_executionStack.empty()) {
		debugPrintf("No exec stack!\n");
		return true;
	}

	const ExecStack &xs = _engine->_gamestate->_executionStack.back();
	int nr = atoi(argv[1]);

	for (int i = nr; i > 0; i--) {
		bool isArgc = (xs.sp - xs.variables_argp - i == 0);
		if (isArgc)
			debugPrintf("-- parameters --\n");
		if (xs.tempCount && ((xs.sp - xs.fp - i) == 0))
			debugPrintf("-- temp variables --\n");
		if (xs.sp - xs.fp - xs.tempCount - i == 0)
			debugPrintf("-- local stack --\n");
		if (xs.sp - i >= _engine->_gamestate->stack_base)
			debugPrintf("ST:%04x = %04x:%04x%s\n",
				(unsigned)(xs.sp - i - _engine->_gamestate->stack_base),
				PRINT_REG(xs.sp[-i]),
				(isArgc ? "  argc" : ""));
	}

	return true;
}

bool Console::cmdValueType(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Determines the type of a value.\n");
		debugPrintf("The type can be one of the following:\n");
		debugPrintf("Invalid, list, object, reference or arithmetic\n");
		debugPrintf("Usage: %s <address>\n", argv[0]);
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t val;

	if (parse_reg_t(_engine->_gamestate, argv[1], &val)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	int t = g_sci->getKernel()->findRegType(val);

	switch (t) {
	case SIG_TYPE_LIST:
		debugPrintf("List\n");
		break;
	case SIG_TYPE_OBJECT:
		debugPrintf("Object\n");
		break;
	case SIG_TYPE_REFERENCE:
		debugPrintf("Reference\n");
		break;
	case SIG_TYPE_INTEGER:
		debugPrintf("Integer\n");
		break;
	case SIG_TYPE_INTEGER | SIG_TYPE_NULL:
		debugPrintf("Null\n");
		break;
	default:
		debugPrintf("Erroneous unknown type 0x%02x (%d decimal)\n", t, t);
	}

	return true;
}

bool Console::cmdViewListNode(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Examines the list node at the given address.\n");
		debugPrintf("Usage: %s <address>\n", argv[0]);
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	printNode(addr);
	return true;
}

bool Console::cmdViewReference(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Examines an arbitrary reference.\n");
		debugPrintf("Usage: %s <start address> [<end address>]\n", argv[0]);
		debugPrintf("Where <start address> is the starting address to examine\n");
		debugPrintf("<end address>, if provided, is the address where examining ends at\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t reg = NULL_REG;
	reg_t reg_end = NULL_REG;

	if (parse_reg_t(_engine->_gamestate, argv[1], &reg)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	if (argc > 2) {
		if (parse_reg_t(_engine->_gamestate, argv[2], &reg_end)) {
			debugPrintf("Invalid address passed.\n");
			debugPrintf("Check the \"addresses\" command on how to use addresses\n");
			return true;
		}
	}

	printReference(reg, reg_end);
	return true;
}

bool Console::cmdDumpReference(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Dumps an arbitrary reference to disk.\n");
		debugPrintf("Usage: %s <start address> [<end address>]\n", argv[0]);
		debugPrintf("Where <start address> is the starting address to dump\n");
		debugPrintf("<end address>, if provided, is the address where the dump ends\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t reg = NULL_REG;
	reg_t reg_end = NULL_REG;

	if (parse_reg_t(_engine->_gamestate, argv[1], &reg)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	if (argc > 2) {
		if (parse_reg_t(_engine->_gamestate, argv[2], &reg_end)) {
			debugPrintf("Invalid address passed.\n");
			debugPrintf("Check the \"addresses\" command on how to use addresses\n");
			return true;
		}
	}

	if (reg.getSegment() == 0 && reg.getOffset() == 0) {
		debugPrintf("Register is null.\n");
		return true;
	}

	if (g_sci->getKernel()->findRegType(reg) != SIG_TYPE_REFERENCE) {
		debugPrintf("%04x:%04x is not a reference\n", PRINT_REG(reg));
		return true;
	}

	if (reg_end.getSegment() != reg.getSegment() && reg_end != NULL_REG) {
		debugPrintf("Ending segment different from starting segment. Assuming no bound on dump.\n");
		reg_end = NULL_REG;
	}

	Common::DumpFile out;
	Common::Path outFileName;
	uint32 bytesWritten;

	switch (_engine->_gamestate->_segMan->getSegmentType(reg.getSegment())) {
#ifdef ENABLE_SCI32
	case SEG_TYPE_BITMAP: {
		outFileName = Common::Path(Common::String::format("%04x_%04x.tga", PRINT_REG(reg)));
		out.open(outFileName);
		SciBitmap &bitmap = *_engine->_gamestate->_segMan->lookupBitmap(reg);
		const Color *color = g_sci->_gfxPalette32->getCurrentPalette().colors;
		const uint16 numColors = ARRAYSIZE(g_sci->_gfxPalette32->getCurrentPalette().colors);

		out.writeByte(0); // image id length
		out.writeByte(1); // color map type (present)
		out.writeByte(1); // image type (uncompressed color-mapped)
		out.writeSint16LE(0);         // index of first color map entry
		out.writeSint16LE(numColors); // number of color map entries
		out.writeByte(24);            // number of bits per color entry (RGB24)
		out.writeSint16LE(0);                      // bottom-left x-origin
		out.writeSint16LE(bitmap.getHeight() - 1); // bottom-left y-origin
		out.writeSint16LE(bitmap.getWidth());  // width
		out.writeSint16LE(bitmap.getHeight()); // height
		out.writeByte(8); // bits per pixel
		out.writeByte(1 << 5); // origin of pixel data (top-left)

		bytesWritten = 18;

		for (int i = 0; i < numColors; ++i) {
			out.writeByte(color->b);
			out.writeByte(color->g);
			out.writeByte(color->r);
			++color;
		}

		bytesWritten += numColors * 3;
		bytesWritten += out.write(bitmap.getPixels(), bitmap.getWidth() * bitmap.getHeight());
		break;
	}
#endif

	default: {
		const SegmentRef block = _engine->_gamestate->_segMan->dereference(reg);
		uint32 size = block.maxSize;

		if (size == 0) {
			debugPrintf("Size of reference is zero.\n");
			return true;
		}

		if (reg_end.getSegment() != 0 && (size < reg_end.getOffset() - reg.getOffset())) {
			debugPrintf("Block end out of bounds (size %d). Resetting.\n", size);
			reg_end = NULL_REG;
		}

		if (reg_end.getSegment() != 0 && (size >= reg_end.getOffset() - reg.getOffset())) {
			size = reg_end.getOffset() - reg.getOffset();
		}

		if (reg_end.getSegment() != 0) {
			debugPrintf("Block size less than or equal to %d\n", size);
		}

		outFileName = Common::Path(Common::String::format("%04x_%04x.dmp", PRINT_REG(reg)));
		out.open(outFileName);
		bytesWritten = out.write(block.raw, size);
		break;
	}
	}

	out.finalize();
	out.close();

	debugPrintf("Wrote %u bytes to %s\n", bytesWritten, outFileName.toString(Common::Path::kNativeSeparator).c_str());
	return true;
}

bool Console::cmdViewObject(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Examines the object at the given address.\n");
		debugPrintf("Usage: %s <address> [<selector name> ...]\n", argv[0]);
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	if (argc >= 3) {
		for (int i = 2; i < argc; ++i) {
			const Object *obj = _engine->_gamestate->_segMan->getObject(addr);
			if (!obj) {
				debugPrintf("%04x:%04x is not an object.\n", PRINT_REG(addr));
				break;
			}

			const Selector selector = _engine->getKernel()->findSelector(argv[i]);
			if (selector == -1) {
				debugPrintf("Invalid selector '%s'.\n", argv[i]);
				break;
			}

			const int index = obj->locateVarSelector(_engine->_gamestate->_segMan, selector);
			if (index == -1) {
				debugPrintf("Selector '%s' is not valid for object %04x:%04x.\n", argv[i], PRINT_REG(addr));
				break;
			}

			const reg_t value = obj->getVariable(index);
			if (i == argc - 1) {
				if (value.isPointer()) {
					printReference(value);
				} else {
					debugPrintf("%04x:%04x (%u)\n", PRINT_REG(value), value.toUint16());
				}
			} else if (!value.isPointer()) {
				debugPrintf("Selector '%s' on object %04x:%04x is not a pointer to an object.\n", argv[i], PRINT_REG(addr));
				debugPrintf("Value is %04x:%04x (%u).\n", PRINT_REG(value), value.toUint16());
				break;
			} else {
				addr = value;
			}
		}
	} else {
		debugPrintf("Information on the object at the given address:\n");
		printObject(addr);
	}

	return true;
}

bool Console::cmdViewActiveObject(int argc, const char **argv) {
	debugPrintf("Information on the currently active object or class:\n");
	printObject(_engine->_gamestate->xs->objp);

	return true;
}

bool Console::cmdViewAccumulatorObject(int argc, const char **argv) {
	debugPrintf("Information on the currently active object or class at the address indexed by the accumulator:\n");
	printObject(_engine->_gamestate->r_acc);

	return true;
}

bool Console::cmdScriptSteps(int argc, const char **argv) {
	debugPrintf("Number of executed SCI operations: %d\n", _engine->_gamestate->scriptStepCounter);
	return true;
}

bool Console::cmdScriptObjects(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Shows all objects inside a specified script.\n");
		debugPrintf("Usage: %s <script number>\n", argv[0]);
		debugPrintf("Example: %s 999\n", argv[0]);
		debugPrintf("<script number> may be * to show objects inside all loaded scripts\n");
		return true;
	}

	int curScriptNr;
	if (strcmp(argv[1], "*") == 0) {
		// get said-strings of all currently loaded scripts
		curScriptNr = -1;
	} else {
		curScriptNr = atoi(argv[1]);
	}

	printOffsets(curScriptNr, SCI_SCR_OFFSET_TYPE_OBJECT);
	return true;
}

bool Console::cmdScriptStrings(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Shows all strings inside a specified script.\n");
		debugPrintf("Usage: %s <script number>\n", argv[0]);
		debugPrintf("Example: %s 999\n", argv[0]);
		debugPrintf("<script number> may be * to show strings inside all loaded scripts\n");
		return true;
	}

	int curScriptNr;
	if (strcmp(argv[1], "*") == 0) {
		// get strings of all currently loaded scripts
		curScriptNr = -1;
	} else {
		curScriptNr = atoi(argv[1]);
	}

	printOffsets(curScriptNr, SCI_SCR_OFFSET_TYPE_STRING);
	return true;
}

bool Console::cmdScriptSaid(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Shows all said-strings inside a specified script.\n");
		debugPrintf("Usage: %s <script number>\n", argv[0]);
		debugPrintf("Example: %s 999\n", argv[0]);
		debugPrintf("<script number> may be * to show said-strings inside all loaded scripts\n");
		return true;
	}

	int curScriptNr;
	if (strcmp(argv[1], "*") == 0) {
		// get said-strings of all currently loaded scripts
		curScriptNr = -1;
	} else {
		curScriptNr = atoi(argv[1]);
	}

	printOffsets(curScriptNr, SCI_SCR_OFFSET_TYPE_SAID);
	return true;
}

void Console::printOffsets(int scriptNr, uint16 showType) {
	SegManager *segMan = _engine->_gamestate->_segMan;
	Vocabulary *vocab = _engine->_vocabulary;
	SegmentId curSegmentNr;
	Common::List<SegmentId> segmentNrList;

	SegmentType curSegmentType = SEG_TYPE_INVALID;
	SegmentObj *curSegmentObj = nullptr;
	Script *curScriptObj = nullptr;
	const byte *curScriptData = nullptr;

	segmentNrList.clear();
	if (scriptNr < 0) {
		// get offsets of all currently loaded scripts
		for (curSegmentNr = 0; curSegmentNr < segMan->_heap.size(); curSegmentNr++) {
			curSegmentObj = segMan->_heap[curSegmentNr];
			if (curSegmentObj && curSegmentObj->getType() == SEG_TYPE_SCRIPT) {
				segmentNrList.push_back(curSegmentNr);
			}
		}

	} else {
		curSegmentNr = segMan->getScriptSegment(scriptNr);
		if (!curSegmentNr) {
			debugPrintf("Script %d is currently not loaded/available\n", scriptNr);
			return;
		}
		segmentNrList.push_back(curSegmentNr);
	}

	const offsetLookupArrayType *scriptOffsetLookupArray;
	offsetLookupArrayType::const_iterator arrayIterator;
	int showTypeCount = 0;

	reg_t objectPos;
	const char *objectNamePtr = nullptr;
	const byte *stringPtr = nullptr;
	const byte *saidPtr = nullptr;

	Common::List<SegmentId>::iterator it;
	const Common::List<SegmentId>::iterator end = segmentNrList.end();

	for (it = segmentNrList.begin(); it != end; it++) {
		curSegmentNr = *it;
		// get object of this segment
		curSegmentObj = segMan->getSegmentObj(curSegmentNr);
		if (!curSegmentObj)
			continue;

		curSegmentType = curSegmentObj->getType();
		if (curSegmentType != SEG_TYPE_SCRIPT) // safety check
			continue;

		curScriptObj = (Script *)curSegmentObj;
		debugPrintf("=== SCRIPT %d inside Segment %04x (%dd) ===\n", curScriptObj->getScriptNumber(), curSegmentNr, curSegmentNr);
		debugN("=== SCRIPT %d inside Segment %04x (%dd) ===\n", curScriptObj->getScriptNumber(), curSegmentNr, curSegmentNr);

		// now print the list
		scriptOffsetLookupArray = curScriptObj->getOffsetArray();
		curScriptData = curScriptObj->getBuf();
		showTypeCount = 0;

		for (arrayIterator = scriptOffsetLookupArray->begin(); arrayIterator != scriptOffsetLookupArray->end(); arrayIterator++) {
			if (arrayIterator->type == showType) {
				switch (showType) {
				case SCI_SCR_OFFSET_TYPE_OBJECT:
					objectPos = make_reg(curSegmentNr, arrayIterator->offset);
					objectNamePtr = segMan->getObjectName(objectPos);
					debugPrintf(" %03d:%04x: %s\n", arrayIterator->id, arrayIterator->offset, objectNamePtr);
					debugN(" %03d:%04x: %s\n", arrayIterator->id, arrayIterator->offset, objectNamePtr);
					break;
				case SCI_SCR_OFFSET_TYPE_STRING:
					stringPtr = curScriptData + arrayIterator->offset;
					debugPrintf(" %03d:%04x: '%s' (size %d)\n", arrayIterator->id, arrayIterator->offset, stringPtr, arrayIterator->stringSize);
					debugN(" %03d:%04x: '%s' (size %d)\n", arrayIterator->id, arrayIterator->offset, stringPtr, arrayIterator->stringSize);
					break;
				case SCI_SCR_OFFSET_TYPE_SAID:
					saidPtr = curScriptData + arrayIterator->offset;
					debugPrintf(" %03d:%04x:\n", arrayIterator->id, arrayIterator->offset);
					debugN(" %03d:%04x: ", arrayIterator->id, arrayIterator->offset);
					vocab->debugDecipherSaidBlock(SciSpan<const byte>(saidPtr, (arrayIterator + 1)->offset - arrayIterator->offset));
					debugN("\n");
					break;
				default:
					break;
				}
				showTypeCount++;
			}
		}

		if (showTypeCount == 0) {
			switch (showType) {
			case SCI_SCR_OFFSET_TYPE_OBJECT:
				debugPrintf(" no objects\n");
				debugN(" no objects\n");
				break;
			case SCI_SCR_OFFSET_TYPE_STRING:
				debugPrintf(" no strings\n");
				debugN(" no strings\n");
				break;
			case SCI_SCR_OFFSET_TYPE_SAID:
				debugPrintf(" no said-strings\n");
				debugN(" no said-strings\n");
				break;
			default:
				break;
			}
		}

		debugPrintf("\n");
		debugN("\n");
	}
}

bool Console::cmdBacktrace(int argc, const char **argv) {
	logBacktrace();

	return true;
}

bool Console::cmdTrace(int argc, const char **argv) {
	if (argc == 2 && atoi(argv[1]) > 0)
		_debugState.runningStep = atoi(argv[1]) - 1;
	_debugState.debugging = true;

	return cmdExit(0, nullptr);
}

bool Console::cmdStepOver(int argc, const char **argv) {
	_debugState.seeking = kDebugSeekStepOver;
	_debugState.seekLevel = _engine->_gamestate->_executionStack.size();
	return cmdTrace(argc, argv);
}

bool Console::cmdStepEvent(int argc, const char **argv) {
	_debugState.stopOnEvent = true;
	_debugState.debugging = true;

	return cmdExit(0, nullptr);
}

bool Console::cmdStepRet(int argc, const char **argv) {
	_debugState.seeking = kDebugSeekLevelRet;
	_debugState.seekLevel = _engine->_gamestate->_executionStack.size() - 1;
	_debugState.debugging = true;

	return cmdExit(0, nullptr);
}

bool Console::cmdStepGlobal(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Steps until the global variable with the specified index is modified.\n");
		debugPrintf("Usage: %s <global variable index>\n", argv[0]);
		return true;
	}

	_debugState.seeking = kDebugSeekGlobal;
	_debugState.seekSpecial = atoi(argv[1]);
	_debugState.debugging = true;

	return cmdExit(0, nullptr);
}

bool Console::cmdStepCallk(int argc, const char **argv) {
	if (argc == 2) {
		/* Try to convert the parameter to a number. If the conversion stops
		   before end of string, assume that the parameter is a function name
		   and scan the function table to find out the index. */
		char *endptr;
		int callk_index = strtoul(argv[1], &endptr, 0);
		if (*endptr != '\0') {
			callk_index = -1;
			for (uint i = 0; i < _engine->getKernel()->getKernelNamesSize(); i++)
				if (argv[1] == _engine->getKernel()->getKernelName(i)) {
					callk_index = i;
					break;
				}

			if (callk_index == -1) {
				debugPrintf("Unknown kernel function '%s'\n", argv[1]);
				return true;
			}
		}

		_debugState.seeking = kDebugSeekSpecialCallk;
		_debugState.seekSpecial = callk_index;
	} else {
		_debugState.seeking = kDebugSeekCallk;
	}
	_debugState.debugging = true;

	return cmdExit(0, nullptr);
}

bool Console::cmdDisassemble(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Disassembles a method by name.\n");
		debugPrintf("Usage: %s <object> <method> <options>\n", argv[0]);
		debugPrintf("Valid options are:\n");
		debugPrintf(" bwt  : Print byte/word tag\n");
		debugPrintf(" bc   : Print bytecode\n");
		debugPrintf(" bcc  : Print bytecode, formatted to use in C code\n");
		return true;
	}

	reg_t objAddr = NULL_REG;
	bool printBytecode = false;
	bool printBWTag = false;
	bool printCSyntax = false;

	if (parse_reg_t(_engine->_gamestate, argv[1], &objAddr)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	const Object *obj = _engine->_gamestate->_segMan->getObject(objAddr);
	int selectorId = _engine->getKernel()->findSelector(argv[2]);
	reg_t addr = NULL_REG;

	if (!obj) {
		debugPrintf("Not an object.\n");
		return true;
	}

	if (selectorId < 0) {
		debugPrintf("Not a valid selector name.\n");
		return true;
	}

	if (lookupSelector(_engine->_gamestate->_segMan, objAddr, selectorId, nullptr, &addr) != kSelectorMethod) {
		debugPrintf("Not a method.\n");
		return true;
	}

	for (int i = 3; i < argc; i++) {
		if (!scumm_stricmp(argv[i], "bwt"))
			printBWTag = true;
		else if (!scumm_stricmp(argv[i], "bc"))
			printBytecode = true;
		else if (!scumm_stricmp(argv[i], "bcc")) {
			printBytecode = true;
			printCSyntax = true;
		}
	}

	reg_t farthestTarget = addr;
	do {
		reg_t prevAddr = addr;
		reg_t jumpTarget;
		if (isJumpOpcode(_engine->_gamestate, addr, jumpTarget)) {
			if (jumpTarget > farthestTarget)
				farthestTarget = jumpTarget;
		}
		addr = disassemble(_engine->_gamestate, make_reg32(addr.getSegment(), addr.getOffset()), obj, printBWTag, printBytecode, printCSyntax);
		if (addr.isNull() && prevAddr < farthestTarget)
			addr = prevAddr + 1; // skip past the ret
	} while (addr.getOffset() > 0);

	return true;
}

bool Console::cmdDisassembleAddress(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Disassembles one or more commands.\n");
		debugPrintf("Usage: %s [startaddr] <options>\n", argv[0]);
		debugPrintf("Valid options are:\n");
		debugPrintf(" bwt  : Print byte/word tag\n");
		debugPrintf(" c<x> : Disassemble <x> bytes\n");
		debugPrintf(" bc   : Print bytecode\n");
		debugPrintf(" bcc  : Print bytecode, formatted to use in C code\n");
		return true;
	}

	reg_t vpc = NULL_REG;
	uint opCount = 1;
	bool printBWTag = false;
	bool printBytes = false;
	bool printCSyntax = false;
	uint32 size;

	if (parse_reg_t(_engine->_gamestate, argv[1], &vpc)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentRef ref = _engine->_gamestate->_segMan->dereference(vpc);
	size = ref.maxSize + vpc.getOffset(); // total segment size

	for (int i = 2; i < argc; i++) {
		if (!scumm_stricmp(argv[i], "bwt"))
			printBWTag = true;
		else if (!scumm_stricmp(argv[i], "bc"))
			printBytes = true;
		else if (!scumm_stricmp(argv[i], "bcc")) {
			printBytes = true;
			printCSyntax = true;
		} else if (toupper(argv[i][0]) == 'C')
			opCount = atoi(argv[i] + 1);
		else {
			debugPrintf("Invalid option '%s'\n", argv[i]);
			return true;
		}
	}

	do {
		vpc = disassemble(_engine->_gamestate, make_reg32(vpc.getSegment(), vpc.getOffset()), nullptr, printBWTag, printBytes, printCSyntax);
	} while ((vpc.getOffset() > 0) && (vpc.getOffset() + 6 < size) && (--opCount));

	return true;
}

void Console::printKernelCallsFound(int kernelFuncNum, bool showFoundScripts) {
	Common::List<ResourceId> resources = _engine->getResMan()->listResources(kResourceTypeScript);
	Common::sort(resources.begin(), resources.end());

	if (showFoundScripts)
		debugPrintf("%d scripts found, disassembling...\n", resources.size());

	int scriptSegment;
	Script *script;
	// Create a custom segment manager here, so that the game's segment
	// manager won't be affected by loading and unloading scripts here.
	SegManager *customSegMan = new SegManager(_engine->getResMan(), _engine->getScriptPatcher());

	Common::List<ResourceId>::iterator itr;
	for (itr = resources.begin(); itr != resources.end(); ++itr) {
		// Ignore specific leftover scripts, which require other non-existing scripts
		if ((_engine->getGameId() == GID_HOYLE3         && itr->getNumber() == 995) ||
		    (_engine->getGameId() == GID_KQ5            && itr->getNumber() == 980) ||
			(_engine->getGameId() == GID_KQ7            && itr->getNumber() == 111) ||
			(_engine->getGameId() == GID_MOTHERGOOSE256 && itr->getNumber() == 980) ||
		    (_engine->getGameId() == GID_SLATER         && itr->getNumber() == 947)) {
			continue;
		}

		// Load script
		scriptSegment = customSegMan->instantiateScript(itr->getNumber());
		script = customSegMan->getScript(scriptSegment);

		// Iterate through all the script's objects
		const ObjMap &objects = script->getObjectMap();
		ObjMap::const_iterator it;
		const ObjMap::const_iterator end = objects.end();
		for (it = objects.begin(); it != end; ++it) {
			const Object *obj = customSegMan->getObject(it->_value.getPos());
			const char *objName = customSegMan->getObjectName(it->_value.getPos());

			// Now dissassemble each method of the script object
			for (uint16 i = 0; i < obj->getMethodCount(); i++) {
				reg_t fptr = obj->getFunction(i);
				uint32 offset = fptr.getOffset();
				int16 opparams[4];
				byte extOpcode;
				uint16 maxJmpOffset = 0;

				for (;;) {
					offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
					byte opcode = extOpcode >> 1;

					if (opcode == op_callk) {
						uint16 kFuncNum = opparams[0];
						uint16 argc2 = opparams[1];

						if (kFuncNum == kernelFuncNum) {
							debugPrintf("Called from script %d, object %s, method %s(%d) with %d bytes for arguments\n",
								itr->getNumber(), objName,
								_engine->getKernel()->getSelectorName(obj->getFuncSelector(i)).c_str(), i, argc2);
						}
					}

					// Monitor all jump opcodes (bt, bnt and jmp), so that if
					// there is a jump after a ret, we don't stop processing
					if (opcode == op_bt || opcode == op_bnt || opcode == op_jmp) {
						uint16 curJmpOffset = offset + (uint16)opparams[0];
						// QFG2 has invalid jumps outside the script buffer in script 260
						if (curJmpOffset > maxJmpOffset && curJmpOffset < script->getScriptSize())
							maxJmpOffset = curJmpOffset;
					}

					// Check for end of function/script
					if (offset >= script->getBufSize())
						break;
					if (opcode == op_ret && offset >= maxJmpOffset)
						break;
				}	// while (true)
			}	// for (uint16 i = 0; i < obj->getMethodCount(); i++)
		}	// for (it = script->_objects.begin(); it != end; ++it)

		customSegMan->uninstantiateScript(itr->getNumber());
	}

	delete customSegMan;
}

bool Console::cmdFindKernelFunctionCall(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Finds the scripts and methods that call a specific kernel function.\n");
		debugPrintf("Usage: %s <kernel function>\n", argv[0]);
		debugPrintf("Example: %s Display\n", argv[0]);
		debugPrintf("Special usage:\n");
		debugPrintf("%s Dummy - find all calls to actual dummy functions "
					"(mapped to kDummy, and dummy in the kernel table). "
					"There shouldn't be calls to these (apart from a known "
					"one in Shivers)\n", argv[0]);
		debugPrintf("%s Unused - find all calls to unused functions (mapped to "
					"kDummy - i.e. mapped in SSCI but dummy in ScummVM, thus "
					"they'll error out when called). Only debug scripts should "
					"be calling these\n", argv[0]);
		debugPrintf("%s Unmapped - find all calls to currently unmapped or "
					"unimplemented functions (mapped to kStub/kStubNull)\n", argv[0]);
		return true;
	}

	Kernel *kernel = _engine->getKernel();
	Common::String funcName(argv[1]);

	if (funcName != "Dummy" && funcName != "Unused" && funcName != "Unmapped") {
		// Find the number of the kernel function call
		int kernelFuncNum = kernel->findKernelFuncPos(argv[1]);

		if (kernelFuncNum < 0) {
			debugPrintf("Invalid kernel function requested\n");
			return true;
		}

		printKernelCallsFound(kernelFuncNum, true);
	} else if (funcName == "Dummy") {
		// Find all actual dummy kernel functions (mapped to kDummy, and dummy
		// in the kernel table)
		for (uint i = 0; i < kernel->_kernelFuncs.size(); i++) {
			if (kernel->_kernelFuncs[i].function == &kDummy && kernel->getKernelName(i) == "Dummy") {
				debugPrintf("Searching for kernel function %d (%s)...\n", i, kernel->getKernelName(i).c_str());
				printKernelCallsFound(i, false);
			}
		}
	} else if (funcName == "Unused") {
		// Find all actual dummy kernel functions (mapped to kDummy - i.e.
		// mapped in SSCI but dummy in ScummVM, thus they'll error out when
		// called)
		for (uint i = 0; i < kernel->_kernelFuncs.size(); i++) {
			if (kernel->_kernelFuncs[i].function == &kDummy && kernel->getKernelName(i) != "Dummy") {
				debugPrintf("Searching for kernel function %d (%s)...\n", i, kernel->getKernelName(i).c_str());
				printKernelCallsFound(i, false);
			}
		}
	} else if (funcName == "Unmapped") {
		// Find all unmapped kernel functions (mapped to kStub/kStubNull)
		for (uint i = 0; i < kernel->_kernelFuncs.size(); i++) {
			if (kernel->_kernelFuncs[i].function == &kStub ||
				kernel->_kernelFuncs[i].function == &kStubNull) {
				debugPrintf("Searching for kernel function %d (%s)...\n", i, kernel->getKernelName(i).c_str());
				printKernelCallsFound(i, false);
			}
		}
	}

	return true;
}

bool Console::cmdSend(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Sends a message to an object.\n");
		debugPrintf("Usage: %s <object> <selector name> <param1> <param2> ... <paramn>\n", argv[0]);
		debugPrintf("Example: %s ?fooScript cue\n", argv[0]);
		return true;
	}

	reg_t object;

	if (parse_reg_t(_engine->_gamestate, argv[1], &object)) {
		debugPrintf("Invalid address \"%s\" passed.\n", argv[1]);
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	const char *selectorName = argv[2];
	int selectorId = _engine->getKernel()->findSelector(selectorName);

	if (selectorId < 0) {
		debugPrintf("Unknown selector: \"%s\"\n", selectorName);
		return true;
	}

	const Object *o = _engine->_gamestate->_segMan->getObject(object);
	if (o == nullptr) {
		debugPrintf("Address \"%04x:%04x\" is not an object\n", PRINT_REG(object));
		return true;
	}

	SelectorType selector_type = lookupSelector(_engine->_gamestate->_segMan, object, selectorId, nullptr, nullptr);

	if (selector_type == kSelectorNone) {
		debugPrintf("Object does not support selector: \"%s\"\n", selectorName);
		return true;
	}

	// everything after the selector name is passed as an argument to the send
	int send_argc = argc - 3;

	// Create the data block for send_selector() at the top of the stack:
	// [selector_number][argument_counter][arguments...]
	StackPtr stackframe = _engine->_gamestate->_executionStack.back().sp;
	stackframe[0] = make_reg(0, selectorId);
	stackframe[1] = make_reg(0, send_argc);
	for (int i = 0; i < send_argc; i++) {
		if (parse_reg_t(_engine->_gamestate, argv[3+i], &stackframe[2+i])) {
			debugPrintf("Invalid address \"%s\" passed.\n", argv[3+i]);
			debugPrintf("Check the \"addresses\" command on how to use addresses\n");
			return true;
		}
	}

	reg_t old_acc = _engine->_gamestate->r_acc;

	// Now commit the actual function:
	ExecStack *old_xstack, *xstack;
	old_xstack = &_engine->_gamestate->_executionStack.back();
	xstack = send_selector(_engine->_gamestate, object, object,
	                       stackframe + 2 + send_argc,
	                       2 + send_argc, stackframe);

	bool restore_acc = old_xstack != xstack || argc == 3;

	if (old_xstack != xstack) {
		_engine->_gamestate->_executionStackPosChanged = true;
		debugPrintf("Message scheduled for execution\n");

		// We call run_engine explicitly so we can restore the value of r_acc
		// after execution.
		run_vm(_engine->_gamestate);
		_engine->_gamestate->xs = old_xstack;

	}

	if (restore_acc) {
		// varselector read or message executed
		debugPrintf("Message completed. Value returned: %04x:%04x\n", PRINT_REG(_engine->_gamestate->r_acc));
		_engine->_gamestate->r_acc = old_acc;
	}

	return true;
}

bool Console::cmdGo(int argc, const char **argv) {
	// CHECKME: is this necessary?
	_debugState.seeking = kDebugSeekNothing;

	return cmdExit(argc, argv);
}

bool Console::cmdLogKernel(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Logs calls to specified kernel function.\n");
		debugPrintf("Usage: %s <kernel function/*>\n", argv[0]);
		debugPrintf("Example: %s StrCpy\n", argv[0]);
		debugPrintf("This is an alias for: bpk <kernel function> log\n");
		return true;
	}

	const char *bpk_argv[] = { "bpk", argv[1], "log" };
	cmdBreakpointKernel(3, bpk_argv);

	return true;
}

void Console::printBreakpoint(int index, const Breakpoint &bp) {
	debugPrintf("  #%i: ", index);
	const char *bpaction;

	switch (bp._action) {
	case BREAK_LOG:
		bpaction = " (action: log only)";
		break;
	case BREAK_BACKTRACE:
		bpaction = " (action: show backtrace)";
		break;
	case BREAK_INSPECT:
		bpaction = " (action: show object)";
		break;
	case BREAK_NONE:
		bpaction = " (action: ignore)";
		break;
	default:
		bpaction = "";
	}
	switch (bp._type) {
	case BREAK_SELECTOREXEC:
		debugPrintf("Execute %s%s\n", bp._name.c_str(), bpaction);
		break;
	case BREAK_SELECTORREAD:
		debugPrintf("Read %s%s\n", bp._name.c_str(), bpaction);
		break;
	case BREAK_SELECTORWRITE:
		debugPrintf("Write %s%s\n", bp._name.c_str(), bpaction);
		break;
	case BREAK_EXPORT: {
		int bpdata = bp._address;
		debugPrintf("Execute script %d, export %d%s\n", bpdata >> 16, bpdata & 0xFFFF, bpaction);
		break;
	}
	case BREAK_ADDRESS:
		debugPrintf("Execute address %04x:%04x%s\n", PRINT_REG(bp._regAddress), bpaction);
		break;
	case BREAK_KERNEL:
		debugPrintf("Kernel call k%s%s\n", bp._name.c_str(), bpaction);
		break;
	default:
		debugPrintf("UNKNOWN TYPE\n");
		break;
	}
}

bool Console::cmdBreakpointList(int argc, const char **argv) {
	int i = 0;

	debugPrintf("Breakpoint list:\n");

	Common::List<Breakpoint>::const_iterator bp = _debugState._breakpoints.begin();
	Common::List<Breakpoint>::const_iterator end = _debugState._breakpoints.end();
	for (; bp != end; ++bp)
		printBreakpoint(i++, *bp);

	if (!i)
		debugPrintf("  No breakpoints defined.\n");

	return true;
}

bool Console::cmdBreakpointDelete(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Deletes a breakpoint with the specified index.\n");
		debugPrintf("Usage: %s <breakpoint index>\n", argv[0]);
		debugPrintf("<index> * will remove all breakpoints\n");
		return true;
	}

	if (strcmp(argv[1], "*") == 0) {
		_debugState._breakpoints.clear();
		_debugState._activeBreakpointTypes = 0;
		return true;
	}

	const int idx = atoi(argv[1]);

	// Find the breakpoint at index idx.
	Common::List<Breakpoint>::iterator bp = _debugState._breakpoints.begin();
	const Common::List<Breakpoint>::iterator end = _debugState._breakpoints.end();
	for (int i = 0; bp != end && i < idx; ++bp, ++i) {
		// do nothing
	}

	if (bp == end) {
		debugPrintf("Invalid breakpoint index %i\n", idx);
		return true;
	}

	// Delete it
	_debugState._breakpoints.erase(bp);

	_debugState.updateActiveBreakpointTypes();

	return true;
}

static bool stringToBreakpointAction(const Common::String &str, BreakpointAction &action) {
	if (str == "break")
		action = BREAK_BREAK;
	else if (str == "log")
		action = BREAK_LOG;
	else if (str == "bt")
		action = BREAK_BACKTRACE;
	else if (str == "inspect")
		action = BREAK_INSPECT;
	else if (str == "ignore")
		action = BREAK_NONE;
	else
		return false;
	return true;
}

bool Console::cmdBreakpointAction(int argc, const char **argv) {
	bool usage = false;

	if (argc != 3) {
		usage = true;
	}

	Common::String arg;
	if (argc >= 3)
		arg = argv[2];

	BreakpointAction bpaction;
	if (!stringToBreakpointAction(arg, bpaction))
		usage = true;

	if (usage) {
		debugPrintf("Change the action for the breakpoint with the specified index.\n");
		debugPrintf("Usage: %s <breakpoint index> break|log|bt|inspect|ignore\n", argv[0]);
		debugPrintf("<index> * will process all breakpoints\n");
		debugPrintf("Actions: break  : break into debugger\n");
		debugPrintf("         log    : log without breaking\n");
		debugPrintf("         bt     : show backtrace without breaking\n");
		debugPrintf("         inspect: show object (only for bpx/bpr/bpw)\n");
		debugPrintf("         ignore : ignore breakpoint\n");
		return true;
	}

	Common::List<Breakpoint>::iterator bp = _debugState._breakpoints.begin();
	const Common::List<Breakpoint>::iterator end = _debugState._breakpoints.end();
	if (strcmp(argv[1], "*") == 0) {
		for (; bp != end; ++bp)
			bp->_action = bpaction;
		_debugState.updateActiveBreakpointTypes();
		return true;
	}

	const int idx = atoi(argv[1]);

	// Find the breakpoint at index idx.
	for (int i = 0; bp != end && i < idx; ++bp, ++i) {
		// do nothing
	}

	if (bp == end) {
		debugPrintf("Invalid breakpoint index %i\n", idx);
		return true;
	}

	bp->_action = bpaction;

	_debugState.updateActiveBreakpointTypes();

	printBreakpoint(idx, *bp);

	return true;
}


bool Console::cmdBreakpointMethod(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Sets a breakpoint on execution of a specified method/selector.\n");
		debugPrintf("Usage: %s <name> [<action>]\n", argv[0]);
		debugPrintf("Example: %s ego::doit\n", argv[0]);
		debugPrintf("         %s ego::doit log\n", argv[0]);
		debugPrintf("May also be used to set a breakpoint that applies whenever an object\n");
		debugPrintf("of a specific type is touched: %s foo::\n", argv[0]);
		debugPrintf("See bp_action usage for possible actions.\n");
		return true;
	}

	BreakpointAction action = BREAK_BREAK;
	if (argc == 3) {
		if (!stringToBreakpointAction(argv[2], action)) {
			debugPrintf("Invalid breakpoint action %s.\n", argv[2]);
			debugPrintf("See bp_action usage for possible actions.\n");
			return true;
		}
	}

	/* Note: We can set a breakpoint on a method that has not been loaded yet.
	   Thus, we can't check whether the command argument is a valid method name.
	   A breakpoint set on an invalid method name will just never trigger. */
	Breakpoint bp;
	bp._type = BREAK_SELECTOREXEC;
	bp._name = argv[1];
	bp._action = action;

	_debugState._breakpoints.push_back(bp);

	if (action != BREAK_NONE)
		_debugState._activeBreakpointTypes |= BREAK_SELECTOREXEC;

	printBreakpoint(_debugState._breakpoints.size() - 1, bp);

	return true;
}

bool Console::cmdBreakpointRead(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Sets a breakpoint on reading of a specified selector.\n");
		debugPrintf("Usage: %s <name> [<action>]\n", argv[0]);
		debugPrintf("Example: %s ego::view\n", argv[0]);
		debugPrintf("         %s ego::view log\n", argv[0]);
		debugPrintf("See bp_action usage for possible actions.\n");
		return true;
	}

	BreakpointAction action = BREAK_BREAK;
	if (argc == 3) {
		if (!stringToBreakpointAction(argv[2], action)) {
			debugPrintf("Invalid breakpoint action %s.\n", argv[2]);
			debugPrintf("See bp_action usage for possible actions.\n");
			return true;
		}
	}

	Breakpoint bp;
	bp._type = BREAK_SELECTORREAD;
	bp._name = argv[1];
	bp._action = action;

	_debugState._breakpoints.push_back(bp);

	if (action != BREAK_NONE)
		_debugState._activeBreakpointTypes |= BREAK_SELECTORREAD;

	printBreakpoint(_debugState._breakpoints.size() - 1, bp);

	return true;
}

bool Console::cmdBreakpointWrite(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Sets a breakpoint on writing of a specified selector.\n");
		debugPrintf("Usage: %s <name> [<action>]\n", argv[0]);
		debugPrintf("Example: %s ego::view\n", argv[0]);
		debugPrintf("         %s ego::view log\n", argv[0]);
		debugPrintf("See bp_action usage for possible actions.\n");
		return true;
	}

	BreakpointAction action = BREAK_BREAK;
	if (argc == 3) {
		if (!stringToBreakpointAction(argv[2], action)) {
			debugPrintf("Invalid breakpoint action %s.\n", argv[2]);
			debugPrintf("See bp_action usage for possible actions.\n");
			return true;
		}
	}

	Breakpoint bp;
	bp._type = BREAK_SELECTORWRITE;
	bp._name = argv[1];
	bp._action = action;

	_debugState._breakpoints.push_back(bp);

	if (action != BREAK_NONE)
		_debugState._activeBreakpointTypes |= BREAK_SELECTORWRITE;

	printBreakpoint(_debugState._breakpoints.size() - 1, bp);

	return true;
}

bool Console::cmdBreakpointKernel(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Sets a breakpoint on execution of a kernel function.\n");
		debugPrintf("Usage: %s <name> [<action>]\n", argv[0]);
		debugPrintf("Example: %s DrawPic\n", argv[0]);
		debugPrintf("         %s DoSoundPlay,DoSoundStop\n", argv[0]);
		debugPrintf("         %s DoSound*\n", argv[0]);
		debugPrintf("         %s DoSound*,!DoSoundUpdateCues\n", argv[0]);
		debugPrintf("         %s DrawPic log\n", argv[0]);
		debugPrintf("See bp_action usage for possible actions.\n");
		return true;
	}

	BreakpointAction action = BREAK_BREAK;
	if (argc == 3) {
		if (!stringToBreakpointAction(argv[2], action)) {
			debugPrintf("Invalid breakpoint action %s.\n", argv[2]);
			debugPrintf("See bp_action usage for possible actions.\n");
			return true;
		}
	}

	// Check if any kernel functions match, to catch typos
	Common::String pattern = argv[1];
	bool found = false;
	const Kernel::KernelFunctionArray &kernelFuncs = _engine->getKernel()->_kernelFuncs;
	for (uint id = 0; id < kernelFuncs.size() && !found; id++) {
		if (kernelFuncs[id].name) {
			const KernelSubFunction *kernelSubCall = kernelFuncs[id].subFunctions;
			if (!kernelSubCall) {
				Common::String kname = kernelFuncs[id].name;
				if (matchKernelBreakpointPattern(pattern, kname))
					found = true;
			} else {
				uint kernelSubCallCount = kernelFuncs[id].subFunctionCount;
				for (uint subId = 0; subId < kernelSubCallCount; subId++) {
					if (kernelSubCall->name) {
						Common::String kname = kernelSubCall->name;
						if (matchKernelBreakpointPattern(pattern, kname))
							found = true;
					}
					kernelSubCall++;
				}
			}
		}
	}
	if (!found) {
		debugPrintf("No kernel functions match %s.\n", pattern.c_str());
		return true;
	}

	Breakpoint bp;
	bp._type = BREAK_KERNEL;
	bp._name = pattern;
	bp._action = action;

	_debugState._breakpoints.push_back(bp);

	if (action != BREAK_NONE)
		_debugState._activeBreakpointTypes |= BREAK_KERNEL;

	printBreakpoint(_debugState._breakpoints.size() - 1, bp);

	return true;
}

bool Console::cmdBreakpointFunction(int argc, const char **argv) {
	if (argc < 3 || argc > 4) {
		debugPrintf("Sets a breakpoint on the execution of the specified exported function.\n");
		debugPrintf("Usage: %s <script number> <export number> [<action>]\n", argv[0]);
		debugPrintf("See bp_action usage for possible actions.\n");
		return true;
	}

	BreakpointAction action = BREAK_BREAK;
	if (argc == 4) {
		if (!stringToBreakpointAction(argv[3], action)) {
			debugPrintf("Invalid breakpoint action %s.\n", argv[3]);
			debugPrintf("See bp_action usage for possible actions.\n");
			return true;
		}
	}

	/* Note: We can set a breakpoint on a method that has not been loaded yet.
	   Thus, we can't check whether the command argument is a valid method name.
	   A breakpoint set on an invalid method name will just never trigger. */
	Breakpoint bp;
	bp._type = BREAK_EXPORT;
	// script number, export number
	bp._address = (atoi(argv[1]) << 16 | atoi(argv[2]));
	bp._action = action;

	_debugState._breakpoints.push_back(bp);
	_debugState._activeBreakpointTypes |= BREAK_EXPORT;

	printBreakpoint(_debugState._breakpoints.size() - 1, bp);

	return true;
}

bool Console::cmdBreakpointAddress(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Sets a breakpoint on the execution of the specified code address.\n");
		debugPrintf("Usage: %s <address> [<action>]\n", argv[0]);
		debugPrintf("See bp_action usage for possible actions.\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	BreakpointAction action = BREAK_BREAK;
	if (argc == 3) {
		if (!stringToBreakpointAction(argv[2], action)) {
			debugPrintf("Invalid breakpoint action %s.\n", argv[2]);
			debugPrintf("See bp_action usage for possible actions.\n");
			return true;
		}
	}

	Breakpoint bp;
	bp._type = BREAK_ADDRESS;
	bp._regAddress = make_reg32(addr.getSegment(), addr.getOffset());
	bp._action = action;

	_debugState._breakpoints.push_back(bp);
	_debugState._activeBreakpointTypes |= BREAK_ADDRESS;

	printBreakpoint(_debugState._breakpoints.size() - 1, bp);

	return true;
}

bool Console::cmdSfx01Header(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Dumps the header of a SCI01 song\n");
		debugPrintf("Usage: %s <track>\n", argv[0]);
		return true;
	}

	Resource *song = _engine->getResMan()->findResource(ResourceId(kResourceTypeSound, atoi(argv[1])), false);

	if (!song) {
		debugPrintf("Doesn't exist\n");
		return true;
	}

	uint32 offset = 0;

	debugPrintf("SCI01 song track mappings:\n");

	if (song->getUint8At(0) == 0xf0) // SCI1 priority spec
		offset = 8;

	if (song->size() <= 0)
		return 1;

	while (song->getUint8At(offset) != 0xff) {
		byte device_id = song->getUint8At(offset);
		debugPrintf("* Device %02x:\n", device_id);
		offset++;

		if (offset + 1 >= song->size())
			return 1;

		while (song->getUint8At(offset) != 0xff) {
			int track_offset;
			int end;
			byte header1, header2;

			if (offset + 7 >= song->size())
				return 1;

			offset += 2;

			track_offset = song->getUint16LEAt(offset);
			header1 = song->getUint8At(track_offset);
			header2 = song->getUint8At(track_offset + 1);
			track_offset += 2;

			end = song->getUint16LEAt(offset + 2);
			debugPrintf("  - %04x -- %04x", track_offset, track_offset + end);

			if (track_offset == 0xfe)
				debugPrintf(" (PCM data)\n");
			else
				debugPrintf(" (channel %d, special %d, %d playing notes, %d foo)\n",
				          header1 & 0xf, header1 >> 4, header2 & 0xf, header2 >> 4);
			offset += 4;
		}
		offset++;
	}

	return true;
}

static int _parse_ticks(const byte *data, int *offset_p, int size) {
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
static void midi_hexdump(const byte *data, int size, int notational_offset) {
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
		debugN("  [%04x] %d\t",
		        old_offset + notational_offset, time);

		cmd = data[offset];
		if (!(cmd & 0x80)) {
			cmd = prev;
			if (prev < 0x80) {
				debugN("Track broken at %x after"
				        " offset mod of %d\n",
				        offset + notational_offset, offset_mod);
				Common::hexdump(data, size, 16, notational_offset);
				return;
			}
			debugN("(rs %02x) ", cmd);
			blanks += 8;
		} else {
			++offset;
			debugN("%02x ", cmd);
			blanks += 3;
		}
		prev = cmd;

		pleft = MIDI_cmdlen[cmd >> 4];
		if (SCI_MIDI_CONTROLLER(cmd) && data[offset] == SCI_MIDI_CUMULATIVE_CUE)
			--pleft; // This is SCI(0)1 specific

		for (i = 0; i < pleft; i++) {
			if (i == 0)
				firstarg = data[offset];
			debugN("%02x ", data[offset++]);
			blanks += 3;
		}

		while (blanks < 16) {
			blanks += 4;
			debugN("    ");
		}

		while (blanks < 20) {
			++blanks;
			debugN(" ");
		}

		if (cmd == SCI_MIDI_EOT)
			debugN(";; EOT");
		else if (cmd == SCI_MIDI_SET_SIGNAL) {
			if (firstarg == SCI_MIDI_SET_SIGNAL_LOOP)
				debugN(";; LOOP point");
			else
				debugN(";; CUE (%d)", firstarg);
		} else if (SCI_MIDI_CONTROLLER(cmd)) {
			if (firstarg == SCI_MIDI_CUMULATIVE_CUE)
				debugN(";; CUE (cumulative)");
			else if (firstarg == SCI_MIDI_RESET_ON_SUSPEND)
				debugN(";; RESET-ON-SUSPEND flag");
		}
		debugN("\n");

		if (old_offset >= offset) {
			debugN("-- Not moving forward anymore,"
			        " aborting (%x/%x)\n", offset, old_offset);
			return;
		}
	}
}

bool Console::cmdSfx01Track(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Dumps a track of a SCI01 song\n");
		debugPrintf("Usage: %s <track> <offset>\n", argv[0]);
		return true;
	}

	Resource *song = _engine->getResMan()->findResource(ResourceId(kResourceTypeSound, atoi(argv[1])), 0);

	int offset = atoi(argv[2]);

	if (!song) {
		debugPrintf("Doesn't exist\n");
		return true;
	}

	midi_hexdump(song->getUnsafeDataAt(offset), song->size() - offset, offset);

	return true;
}

bool Console::cmdMapVocab994(int argc, const char **argv) {
	EngineState *s = _engine->_gamestate;	// for the several defines in this function
	reg_t reg;

	if (argc != 4) {
		debugPrintf("Attempts to map a range of vocab.994 entries to a given class\n");
		debugPrintf("Usage: %s <class addr> <first> <last>\n", argv[0]);
		return true;
	}

	if (parse_reg_t(_engine->_gamestate, argv[1], &reg)) {
		debugPrintf("Invalid address passed.\n");
		debugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	Resource *resource = _engine->_resMan->findResource(ResourceId(kResourceTypeVocab, 994), false);
	if (resource == nullptr) {
		return true;
	}
	const Object *obj = s->_segMan->getObject(reg);
	SciSpan<const uint16> data = resource->subspan<const uint16>(0);
	uint32 first = atoi(argv[2]);
	uint32 last  = atoi(argv[3]);
	Common::Array<bool> markers;

	markers.resize(_engine->getKernel()->getSelectorNamesSize());
	if (!obj->isClass() && getSciVersion() != SCI_VERSION_3)
		obj = s->_segMan->getObject(obj->getSuperClassSelector());

	first = MIN<uint32>(first, resource->size() / 2 - 2);
	last =  MIN<uint32>(last, resource->size() / 2 - 2);

	for (uint32 i = first; i <= last; ++i) {
		uint16 ofs = data[i];

		if (obj && ofs < obj->getVarCount()) {
			uint16 varSelector = obj->getVarSelector(ofs);
			debugPrintf("%d: property at index %04x of %s is %s %s\n", i, ofs,
				    s->_segMan->getObjectName(reg),
				    _engine->getKernel()->getSelectorName(varSelector).c_str(),
				    markers[varSelector] ? "(repeat!)" : "");
			markers[varSelector] = true;
		}
		else {
			debugPrintf("%d: property at index %04x doesn't match up with %s\n", i, ofs,
				    s->_segMan->getObjectName(reg));
		}
	}

	return true;
}

bool Console::cmdGameFlagsInit(int argc, const char **argv) {
	if (argc == 2) {
		_gameFlagsGlobal = atoi(argv[1]);
	} else {
		debugPrintf("Sets the game flags global for tf / sf / cf commands\n");
		debugPrintf("Usage: %s global_number\n", argv[0]);
	}
	Common::String gameFlagGlobalString = "not set";
	if (_gameFlagsGlobal != 0) {
		gameFlagGlobalString = Common::String::format("%d", _gameFlagsGlobal);
	}
	debugPrintf("Base game flag global is %s\n", gameFlagGlobalString.c_str());
	return true;
}

bool Console::cmdGameFlagsTest(int argc, const char **argv) {
	return processGameFlagsOperation(kGameFlagsTest, argc, argv);
}

bool Console::cmdGameFlagsSet(int argc, const char **argv) {
	return processGameFlagsOperation(kGameFlagsSet, argc, argv);
}

bool Console::cmdGameFlagsClear(int argc, const char **argv) {
	return processGameFlagsOperation(kGameFlagsClear, argc, argv);
}

bool Console::processGameFlagsOperation(GameFlagsOperation op, int argc, const char **argv) {
	if (_gameFlagsGlobal == 0) {
		debugPrintf("Use gameflags_init to set game flags global\n");
		return true;
	}

	if (argc == 1) {
		const char *opVerb;
		if (op == kGameFlagsTest) {
			opVerb = "Tests";
		} else if (op == kGameFlagsSet) {
			opVerb = "Sets";
		} else {
			opVerb = "Clears";
		}
		debugPrintf("%s game flags\n", opVerb);
		debugPrintf("Usage: %s flag [flag ...]\n", argv[0]);
		return true;
	}

	EngineState *s = _engine->_gamestate;
	for (int i = 1; i < argc; ++i) {
		int flagNumber;
		if (!parseInteger(argv[i], flagNumber) || flagNumber < 0) {
			debugPrintf("Invalid flag: %s\n", argv[i]);
			continue;
		}
		// read the global that contains the flag
		uint16 globalNumber = _gameFlagsGlobal + (flagNumber / 16);
		if (globalNumber > s->variablesMax[VAR_GLOBAL]) {
			debugPrintf("Invalid flag: %d (global var %d is out of range)\n", flagNumber, globalNumber);
			continue;
		}
		reg_t *globalReg = &s->variables[VAR_GLOBAL][globalNumber];
		if (!globalReg->isNumber()) {
			debugPrintf("Invalid flag: %d (global var %d is not a number)\n", flagNumber, globalNumber);
			continue;
		}
		uint16 globalValue = globalReg->toUint16();
		uint16 flagMask;
		if (g_sci->_features->isGameFlagBitOrderNormal()) {
			flagMask = 0x0001 << (flagNumber % 16);
		} else {
			flagMask = 0x8000 >> (flagNumber % 16);
		}

		// set or clear the flag
		bool already = false;
		if (op == kGameFlagsSet) {
			if ((globalValue & flagMask)) {
				already = true;
			} else {
				globalValue |= flagMask;
				globalReg->setOffset(globalValue);
			}
		} else if (op == kGameFlagsClear) {
			if (!(globalValue & flagMask)) {
				already = true;
			} else {
				globalValue &= ~flagMask;
				globalReg->setOffset(globalValue);
			}
		}

		const char *result = (globalValue & flagMask) ? "set" : "clear";
		debugPrintf("Flag %d is %s%s (global var %d, flag %04x)\n",
					flagNumber, already ? "already " : "", result, globalNumber, flagMask);
	}
	return true;
}

bool Console::cmdQuit(int argc, const char **argv) {
	if (argc != 2) {
	}

	if (argc == 2 && !scumm_stricmp(argv[1], "now")) {
		// Quit ungracefully
		g_system->quit();
	} else if (argc == 1 || (argc == 2 && !scumm_stricmp(argv[1], "game"))) {

		// Quit gracefully
		_engine->_gamestate->abortScriptProcessing = kAbortQuitGame; // Terminate VM
		_debugState.seeking = kDebugSeekNothing;
		_debugState.runningStep = 0;

	} else {
		debugPrintf("%s [game] - exit gracefully\n", argv[0]);
		debugPrintf("%s now - exit ungracefully\n", argv[0]);
		return true;
	}

	return cmdExit(0, nullptr);
}

bool Console::cmdAddresses(int argc, const char **argv) {
	debugPrintf("Address parameters may be passed in one of three forms:\n");
	debugPrintf(" - ssss:oooo -- where 'ssss' denotes a segment and 'oooo' an offset.\n");
	debugPrintf("   Example: \"a:c5\" would address something in segment 0xa at offset 0xc5.\n");
	debugPrintf(" - &scr:oooo -- where 'scr' is a script number and oooo an offset within that script; will\n");
	debugPrintf("   fail if the script is not currently loaded\n");
	debugPrintf(" - $REG -- where 'REG' is one of 'PC', 'ACC', 'PREV' or 'OBJ': References the address\n");
	debugPrintf("   indicated by the register of this name.\n");
	debugPrintf(" - $REG+n (or -n) -- Like $REG, but modifies the offset part by a specific amount (which\n");
	debugPrintf("   is specified in hexadecimal).\n");
	debugPrintf(" - ?obj -- Looks up an object with the specified name, uses its address. This will abort if\n");
	debugPrintf("   the object name is ambiguous; in that case, a list of addresses and indices is provided.\n");
	debugPrintf("   ?obj.idx may be used to disambiguate 'obj' by the index 'idx'.\n");
	debugPrintf("   Underscores are used as substitute characters for spaces in object names.\n");
	debugPrintf("   For example, an object named \"Glass Jar\" can be accessed as \"Glass_Jar\".\n");

	return true;
}

bool Console::cmdSpeedThrottle(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("Displays or changes kGameIsRestarting maximum delay in milliseconds\n");
		debugPrintf("usage: %s [<delay>]\n", argv[0]);
		return true;
	}
	if (argc == 2) {
		int newDelay;
		if (!parseInteger(argv[1], newDelay)) {
			return true;
		}
		if (newDelay < 0) {
			debugPrintf("invalid delay\n");
			return true;
		}
		_engine->_speedThrottleDelay = newDelay;
	}
	debugPrintf("kGameIsRestarting maximum delay: %d ms\n", _engine->_speedThrottleDelay);
	return true;
}

// Returns 0 on success
static int parse_reg_t(EngineState *s, const char *str, reg_t *dest) {
	// Pointer to the part of str which contains a numeric offset (if any)
	const char *offsetStr = nullptr;

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
			offsetStr = nullptr;
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
		dest->setSegment(s->_segMan->getScriptSegment(script_nr));
		if (!dest->getSegment()) {
			return 1;
		}

		// Finally, after the colon comes the offset
		offsetStr = colon + 1;

	} else {
		// Now we either got an object name, or segment:offset or plain value
		//  segment:offset is recognized by the ":"
		//  plain value may be "123" or "123h" or "fffh" or "0xfff"
		//  object name is assumed if nothing else matches or a "?" is used as prefix as override
		//  object name may contain "+", "-" and "." for relative calculations, those chars are used nowhere else

		// First we cycle through the string counting special chars
		const char *strLoop = str;
		int charsCount = strlen(str);
		int charsCountObject = 0;
		int charsCountSegmentOffset = 0;
		int charsCountLetter = 0;
		int charsCountNumber = 0;
		bool charsForceHex = false;
		bool charsForceObject = false;

		while (*strLoop) {
			switch (*strLoop) {
			case '+':
			case '-':
			case '.':
				charsCountObject++;
				break;
			case '?':
				if (strLoop == str) {
					charsForceObject = true;
					str++; // skip over prefix
				}
				break;
			case ':':
				charsCountSegmentOffset++;
				break;
			case 'h':
				if (*(strLoop + 1) == 0)
					charsForceHex = true;
				else
					charsCountObject++;
				break;
			case '0':
				if (*(strLoop + 1) == 'x') {
					str += 2; // skip "0x"
					strLoop++; // skip "x"
					charsForceHex = true;
				}
				charsCountNumber++;
				break;
			default:
				if ((*strLoop >= '0') && (*strLoop <= '9'))
					charsCountNumber++;
				if ((*strLoop >= 'a') && (*strLoop <= 'f'))
					charsCountLetter++;
				if ((*strLoop >= 'A') && (*strLoop <= 'F'))
					charsCountLetter++;
				if ((*strLoop >= 'i') && (*strLoop <= 'z'))
					charsCountObject++;
				if ((*strLoop >= 'I') && (*strLoop <= 'Z'))
					charsCountObject++;
				if (*strLoop == '_')	// underscores are used as substitutes for spaces in object names
					charsCountObject++;
			}
			strLoop++;
		}

		if ((charsCountObject) && (charsCountSegmentOffset))
			return 1; // input doesn't make sense

		if (!charsForceObject) {
			// input may be values/segment:offset

			if (charsCountSegmentOffset) {
				// ':' found, so must be segment:offset
				const char *colon = strchr(str, ':');

				offsetStr = colon + 1;

				Common::String segmentStr(str, colon);
				dest->setSegment(strtol(segmentStr.c_str(), &endptr, 16));
				if (*endptr)
					return 1;
			} else {
				dest->setSegment(0);

				if (charsCountNumber == charsCount) {
					// Only numbers in input, assume decimal value
					int val = strtol(str, &endptr, 10);
					if (*endptr)
						return 1; // strtol failed?
					dest->setOffset(val);
					return 0;
				} else {
					// We also got letters, check if there were only hexadecimal letters and '0x' at the start or 'h' at the end
					if ((charsForceHex) && (!charsCountObject)) {
						int val = strtol(str, &endptr, 16);
						if ((*endptr != 'h') && (*endptr != 0))
							return 1;
						dest->setOffset(val);
						return 0;
					} else {
						// Something else was in input, assume object name
						charsForceObject = true;
					}
				}
			}
		}

		if (charsForceObject) {
			// We assume now that input is object name
			// Object by name: "?OBJ" or "?OBJ.INDEX" or "?OBJ.INDEX+OFFSET" or "?OBJ.INDEX-OFFSET"
			// The (optional) index can be used to distinguish multiple object with the same name.
			int index = -1;

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
				if (*endptr) {
					// The characters after the dot do not represent an index.
					// This can happen if an object contains a dot in its name,
					// like 'dominoes.opt' in Hoyle 3.
					index = -1;
				} else {
					// Valid index found, chop it off
					str_objname = Common::String(str_objname.c_str(), tmp);
				}
			}

			// Replace all underscores in the name with spaces
			for (uint i = 0; i < str_objname.size(); i++) {
				if (str_objname[i] == '_')
					str_objname.setChar(' ', i);
			}

			// Now all values are available; iterate over all objects.
			*dest = s->_segMan->findObjectByName(str_objname, index);
			if (dest->isNull())
				return 1;
		}

		(void)charsCountLetter; // Shut "unused variable" warning
	}
	if (offsetStr) {
		int val = strtol(offsetStr, &endptr, 16);

		if (relativeOffset)
			dest->incOffset(val);
		else
			dest->setOffset(val);

		if (*endptr)
			return 1;
	}

	return 0;
}

bool Console::parseInteger(const char *argument, int &result) {
	char *endPtr = nullptr;
	int idxLen = strlen(argument);
	const char *lastChar = argument + idxLen - (idxLen == 0 ? 0 : 1);

	if ((strncmp(argument, "0x", 2) == 0) || (*lastChar == 'h')) {
		// hexadecimal number
		result = strtol(argument, &endPtr, 16);
		if ((*endPtr != 0) && (*endPtr != 'h')) {
			debugPrintf("Invalid hexadecimal number '%s'\n", argument);
			return false;
		}
	} else {
		// decimal number
		result = strtol(argument, &endPtr, 10);
		if (*endPtr != 0) {
			debugPrintf("Invalid decimal number '%s'\n", argument);
			return false;
		}
	}
	return true;
}

void Console::printBasicVarInfo(reg_t variable) {
	int regType = g_sci->getKernel()->findRegType(variable);
	int segType = regType;
	SegManager *segMan = g_sci->getEngineState()->_segMan;

	segType &= SIG_TYPE_INTEGER | SIG_TYPE_OBJECT | SIG_TYPE_REFERENCE | SIG_TYPE_NODE | SIG_TYPE_LIST | SIG_TYPE_UNINITIALIZED | SIG_TYPE_ERROR;

	switch (segType) {
	case SIG_TYPE_INTEGER: {
		uint16 content = variable.toUint16();
		if (content >= 10)
			debugPrintf(" (%dd)", content);
		break;
	}
	case SIG_TYPE_OBJECT:
		debugPrintf(" (object '%s')", segMan->getObjectName(variable));
		break;
	case SIG_TYPE_REFERENCE:
		debugPrintf(" (reference)");
		break;
	case SIG_TYPE_NODE:
		debugPrintf(" (node)");
		break;
	case SIG_TYPE_LIST:
		debugPrintf(" (list)");
		break;
	case SIG_TYPE_UNINITIALIZED:
		debugPrintf(" (uninitialized)");
		break;
	case SIG_TYPE_ERROR:
		debugPrintf(" (error)");
		break;
	default:
		debugPrintf(" (??\?)");
	}

	if (regType & SIG_IS_INVALID)
		debugPrintf(" IS INVALID!");
}

void Console::printList(reg_t reg) {
	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegment(reg.getSegment(), SEG_TYPE_LISTS);

	if (!mobj) {
		debugPrintf("list:\nCould not find list segment.\n");
		return;
	}

	ListTable *table = static_cast<ListTable *>(mobj);

	if (!table->isValidEntry(reg.getOffset())) {
		debugPrintf("list:\nAddress does not contain a valid list.\n");
		return;
	}

	const List &list = table->at(reg.getOffset());
	debugPrintf("list:\n");
	printList(list);
}

void Console::printList(const List &list) {
	reg_t pos = list.first;
	reg_t my_prev = NULL_REG;

	debugPrintf("\t<\n");

	while (!pos.isNull()) {
		Node *node;
		NodeTable *nt = (NodeTable *)_engine->_gamestate->_segMan->getSegment(pos.getSegment(), SEG_TYPE_NODES);

		if (!nt || !nt->isValidEntry(pos.getOffset())) {
			debugPrintf("   WARNING: %04x:%04x: Doesn't contain list node!\n", PRINT_REG(pos));
			return;
		}

		node = &nt->at(pos.getOffset());

		debugPrintf("\t%04x:%04x  : %04x:%04x -> %04x:%04x\n", PRINT_REG(pos), PRINT_REG(node->key), PRINT_REG(node->value));

		if (my_prev != node->pred)
			debugPrintf("   WARNING: current node gives %04x:%04x as predecessor!\n", PRINT_REG(node->pred));

		my_prev = pos;
		pos = node->succ;
	}

	if (my_prev != list.last)
		debugPrintf("   WARNING: Last node was expected to be %04x:%04x, was %04x:%04x!\n",
				  PRINT_REG(list.last), PRINT_REG(my_prev));
	debugPrintf("\t>\n");
}

int Console::printNode(reg_t addr) {
	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegment(addr.getSegment(), SEG_TYPE_LISTS);

	if (mobj) {
		ListTable *lt = (ListTable *)mobj;
		List *list;

		if (!lt->isValidEntry(addr.getOffset())) {
			debugPrintf("Address does not contain a list\n");
			return 1;
		}

		list = &lt->at(addr.getOffset());

		debugPrintf("%04x:%04x : first x last = (%04x:%04x, %04x:%04x)\n", PRINT_REG(addr), PRINT_REG(list->first), PRINT_REG(list->last));
	} else {
		NodeTable *nt;
		Node *node;
		mobj = _engine->_gamestate->_segMan->getSegment(addr.getSegment(), SEG_TYPE_NODES);

		if (!mobj) {
			debugPrintf("Segment #%04x is not a list or node segment\n", addr.getSegment());
			return 1;
		}

		nt = (NodeTable *)mobj;

		if (!nt->isValidEntry(addr.getOffset())) {
			debugPrintf("Address does not contain a node\n");
			return 1;
		}
		node = &nt->at(addr.getOffset());

		debugPrintf("%04x:%04x : prev x next = (%04x:%04x, %04x:%04x); maps %04x:%04x -> %04x:%04x\n",
		          PRINT_REG(addr), PRINT_REG(node->pred), PRINT_REG(node->succ), PRINT_REG(node->key), PRINT_REG(node->value));
	}

	return 0;
}

void Console::printReference(reg_t reg, reg_t reg_end) {
	int type_mask = g_sci->getKernel()->findRegType(reg);
	int filter;
	int found = 0;

	debugPrintf("%04x:%04x is of type 0x%x: ", PRINT_REG(reg), type_mask);

	if (reg.getSegment() == 0 && reg.getOffset() == 0) {
		debugPrintf("Null.\n");
		return;
	}

	if (reg_end.getSegment() != reg.getSegment() && reg_end != NULL_REG) {
		debugPrintf("Ending segment different from starting segment. Assuming no bound on dump.\n");
		reg_end = NULL_REG;
	}

	for (filter = 1; filter < 0xf000; filter <<= 1) {
		int type = type_mask & filter;

		if (found && type) {
			debugPrintf("--- Alternatively, it could be a ");
		}


		switch (type) {
		case 0:
			break;
		case SIG_TYPE_LIST:
			printList(reg);
			break;
		case SIG_TYPE_NODE:
			debugPrintf("list node\n");
			printNode(reg);
			break;
		case SIG_TYPE_OBJECT:
			debugPrintf("object\n");
			printObject(reg);
			break;
		case SIG_TYPE_REFERENCE: {
			switch (_engine->_gamestate->_segMan->getSegmentType(reg.getSegment())) {
#ifdef ENABLE_SCI32
				case SEG_TYPE_ARRAY:
					printArray(reg);
					break;
				case SEG_TYPE_BITMAP:
					printBitmap(reg);
					break;
#endif
				default: {
					const SegmentRef block = _engine->_gamestate->_segMan->dereference(reg);
					uint16 size = block.maxSize;

					debugPrintf("raw data\n");

					if (reg_end.getSegment() != 0 && (size < reg_end.getOffset() - reg.getOffset())) {
						debugPrintf("Block end out of bounds (size %d). Resetting.\n", size);
						reg_end = NULL_REG;
					}

					if (reg_end.getSegment() != 0 && (size >= reg_end.getOffset() - reg.getOffset()))
						size = reg_end.getOffset() - reg.getOffset();

					if (reg_end.getSegment() != 0)
						debugPrintf("Block size less than or equal to %d\n", size);

					if (block.isRaw)
						Common::hexdump(block.raw, size, 16, 0);
					else
						hexDumpReg(block.reg, size / 2, 4, 0);
				}
			}
			break;
		}
		case SIG_TYPE_INTEGER:
			debugPrintf("arithmetic value\n  %d (%04x)\n", (int16) reg.getOffset(), reg.getOffset());
			break;
		default:
			debugPrintf("unknown type %d.\n", type);
		}

		if (type) {
			debugPrintf("\n");
			found = 1;
		}
	}
}

#ifdef ENABLE_SCI32
void Console::printArray(reg_t reg) {
	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegment(reg.getSegment(), SEG_TYPE_ARRAY);

	if (!mobj) {
		debugPrintf("SCI32 array:\nCould not find array segment.\n");
		return;
	}

	ArrayTable *table = static_cast<ArrayTable *>(mobj);

	if (!table->isValidEntry(reg.getOffset())) {
		debugPrintf("SCI32 array:\nAddress does not contain a valid array.\n");
		return;
	}

	const SciArray &array = table->at(reg.getOffset());

	const char *arrayType;
	switch (array.getType()) {
	case kArrayTypeID:
		arrayType = "reg_t";
		break;
	case kArrayTypeByte:
		arrayType = "byte";
		break;
	case kArrayTypeInt16:
		arrayType = "int16 (as reg_t)";
		break;
	case kArrayTypeString:
		arrayType = "string";
		break;
	default:
		arrayType = "invalid";
		break;
	}
	debugPrintf("SCI32 %s array (%u entries):\n", arrayType, array.size());
	switch (array.getType()) {
	case kArrayTypeInt16:
	case kArrayTypeID: {
		hexDumpReg((const reg_t *)array.getRawData(), array.size(), 4, 0, true);
		break;
	}
	case kArrayTypeByte:
	case kArrayTypeString: {
		Common::hexdump((const byte *)array.getRawData(), array.size(), 16, 0);
		break;
	}
	default:
		break;
	}
}

void Console::printBitmap(reg_t reg) {
	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegment(reg.getSegment(), SEG_TYPE_BITMAP);

	if (!mobj) {
		debugPrintf("SCI32 bitmap:\nCould not find bitmap segment.\n");
		return;
	}

	BitmapTable *table = static_cast<BitmapTable *>(mobj);

	if (!table->isValidEntry(reg.getOffset())) {
		debugPrintf("SCI32 bitmap:\nAddress does not contain a valid bitmap.\n");
		return;
	}

	const SciBitmap &bitmap = table->at(reg.getOffset());

	debugPrintf("SCI32 bitmap (%s):\n", bitmap.toString().c_str());

	Common::hexdump((const byte *) bitmap.getRawData(), bitmap.getRawSize(), 16, 0);
}

#endif

void Console::writeIntegrityDumpLine(const Common::String &statusName, const Common::String &resourceName, Common::WriteStream &out, Common::ReadStream *const data, const int size, const bool writeHash) {
	debugPrintf("%s", statusName.c_str());

	out.writeString(resourceName);
	if (!data) {
		out.writeString(" ERROR\n");
		debugPrintf("[ERR] ");
	} else {
		out.writeString(Common::String::format(" %d ", size));
		if (writeHash) {
			out.writeString(Common::computeStreamMD5AsString(*data));
		} else {
			out.writeString("disabled");
		}
		out.writeString("\n");
		debugPrintf("[OK] ");
	}
}

static void printChar(byte c) {
	if (c < 32 || c >= 127)
		c = '.';
	debugN("%c", c);
}

void Console::hexDumpReg(const reg_t *data, int len, int regsPerLine, int startOffset, bool isArray) {
	// reg_t version of Common::hexdump
	assert(1 <= regsPerLine && regsPerLine <= 8);
	int i;
	int offset = startOffset;
	while (len >= regsPerLine) {
		debugN("%06x: ", offset);
		for (i = 0; i < regsPerLine; i++) {
			debugN("%04x:%04x  ", PRINT_REG(data[i]));
		}
		debugN(" |");
		for (i = 0; i < regsPerLine; i++) {
			if (g_sci->isBE()) {
				printChar(data[i].toUint16() >> 8);
				printChar(data[i].toUint16() & 0xff);
			} else {
				printChar(data[i].toUint16() & 0xff);
				printChar(data[i].toUint16() >> 8);
			}
		}
		debugN("|\n");
		data += regsPerLine;
		len -= regsPerLine;
		offset += regsPerLine * (isArray ? 1 : 2);
	}

	if (len <= 0)
		return;

	debugN("%06x: ", offset);
	for (i = 0; i < regsPerLine; i++) {
		if (i < len)
			debugN("%04x:%04x  ", PRINT_REG(data[i]));
		else
			debugN("           ");
	}
	debugN(" |");
	for (i = 0; i < len; i++) {
		if (g_sci->isBE()) {
			printChar(data[i].toUint16() >> 8);
			printChar(data[i].toUint16() & 0xff);
		} else {
			printChar(data[i].toUint16() & 0xff);
			printChar(data[i].toUint16() >> 8);
		}
	}
	for (; i < regsPerLine; i++)
		debugN("  ");
	debugN("|\n");
}

} // End of namespace Sci
