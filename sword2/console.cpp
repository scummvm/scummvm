/* Copyright (C) 1994-2003 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "bs2/sword2.h"
#include "bs2/console.h"
#include "bs2/debug.h"
#include "bs2/defs.h"
#include "bs2/events.h"
#include "bs2/logic.h"
#include "bs2/maketext.h"
#include "bs2/mouse.h"
#include "bs2/protocol.h"
#include "bs2/resman.h"
#include "bs2/save_rest.h"
#include "bs2/startup.h"

// FIXME: Much of this is copied from scumm/debugger.cpp which is a pretty
// lousy form of code reuse.

#ifdef _WIN32_WCE
extern void force_keyboard(bool);
#endif

namespace Sword2 {

bool wantSfxDebug = false;	// sfx debug enabled/disabled from console

static void Var_check(int var) {
	Debug_Printf("%d\n", VAR(var));
}

static void Var_set(int var, int val) {
	Debug_Printf("was %d, ", VAR(var));
	VAR(var) = val;
	Debug_Printf("now %d\n", VAR(var));
}

Debugger::Debugger(Sword2Engine *s) {
	_vm = s;
	_frame_countdown = 0;
	_dcmd_count = 0;
	_detach_now = false;
	_isAttached = false;
	_errStr = NULL;

	// Register commands

	DCmd_Register("continue", &Debugger::Cmd_Exit);
	DCmd_Register("exit", &Debugger::Cmd_Exit);
	DCmd_Register("quit", &Debugger::Cmd_Exit);
	DCmd_Register("q", &Debugger::Cmd_Exit);
	DCmd_Register("help", &Debugger::Cmd_Help);
	DCmd_Register("mem", &Debugger::Cmd_Mem);
	DCmd_Register("tony", &Debugger::Cmd_Tony);
	DCmd_Register("res", &Debugger::Cmd_Res);
	DCmd_Register("starts", &Debugger::Cmd_Starts);
	DCmd_Register("start", &Debugger::Cmd_Start);
	DCmd_Register("s", &Debugger::Cmd_Start);
	DCmd_Register("info", &Debugger::Cmd_Info);
	DCmd_Register("walkgrid", &Debugger::Cmd_WalkGrid);
	DCmd_Register("mouse", &Debugger::Cmd_Mouse);
	DCmd_Register("player", &Debugger::Cmd_Player);
	DCmd_Register("reslook", &Debugger::Cmd_ResLook);
	DCmd_Register("cur", &Debugger::Cmd_CurrentInfo);
	DCmd_Register("runlist", &Debugger::Cmd_RunList);
	DCmd_Register("kill", &Debugger::Cmd_Kill);
	DCmd_Register("nuke", &Debugger::Cmd_Nuke);
	DCmd_Register("var", &Debugger::Cmd_Var);
	DCmd_Register("rect", &Debugger::Cmd_Rect);
	DCmd_Register("clear", &Debugger::Cmd_Clear);
	DCmd_Register("debugon", &Debugger::Cmd_DebugOn);
	DCmd_Register("debugoff", &Debugger::Cmd_DebugOn);
	DCmd_Register("saverest", &Debugger::Cmd_SaveRest);
	DCmd_Register("saves", &Debugger::Cmd_ListSaveGames);
	DCmd_Register("save", &Debugger::Cmd_SaveGame);
	DCmd_Register("restore", &Debugger::Cmd_RestoreGame);
	DCmd_Register("bltfxon", &Debugger::Cmd_BltFxOn);
	DCmd_Register("bltfxoff", &Debugger::Cmd_BltFxOff);
	DCmd_Register("timeon", &Debugger::Cmd_TimeOn);
	DCmd_Register("timeoff", &Debugger::Cmd_TimeOff);
	DCmd_Register("text", &Debugger::Cmd_Text);
	DCmd_Register("showvar", &Debugger::Cmd_ShowVar);
	DCmd_Register("hidevar", &Debugger::Cmd_HideVar);
	DCmd_Register("version", &Debugger::Cmd_Version);
	DCmd_Register("soft", &Debugger::Cmd_SoftHard);
	DCmd_Register("hard", &Debugger::Cmd_SoftHard);
	DCmd_Register("animtest", &Debugger::Cmd_AnimTest);
	DCmd_Register("texttest", &Debugger::Cmd_TextTest);
	DCmd_Register("linetest", &Debugger::Cmd_LineTest);
	DCmd_Register("grab", &Debugger::Cmd_Grab);
	DCmd_Register("events", &Debugger::Cmd_Events);
	DCmd_Register("sfx", &Debugger::Cmd_Sfx);
	DCmd_Register("english", &Debugger::Cmd_English);
	DCmd_Register("finnish", &Debugger::Cmd_Finnish);
	DCmd_Register("polish", &Debugger::Cmd_Polish);
}

void Debugger::attach(const char *entry) {

#ifdef _WIN32_WCE
	force_keyboard(true);
#endif

	if (entry) {
		_errStr = strdup(entry);
	}

	_frame_countdown = 1;
	_detach_now = false;
	_isAttached = true;
}

void Debugger::detach() {
#if USE_CONSOLE
	if (_vm->_debuggerDialog) {
		_vm->_debuggerDialog->setInputeCallback(0, 0);
		_vm->_debuggerDialog->setCompletionCallback(0, 0);
	}
#endif

#ifdef _WIN32_WCE
	force_keyboard(false);
#endif

	_detach_now = false;
	_isAttached = false;
}

// Temporary execution handler
void Debugger::onFrame() {
	if (_frame_countdown == 0)
		return;
	--_frame_countdown;

	if (!_frame_countdown) {
		// Pause sound output

		g_sound->pauseFx();
		g_sound->pauseSpeech();
		g_sound->pauseMusic();

		// Enter debugger
		enter();

		// Resume previous sound state

		g_sound->unpauseFx();
		g_sound->unpauseSpeech();
		g_sound->unpauseMusic();

		// Restore old mouse cursor
		g_display->drawMouse();

		// Detach if we're finished with the debugger
		if (_detach_now)
			detach();
	}
}

// Console handler
#if USE_CONSOLE
bool Debugger::debuggerInputCallback(ConsoleDialog *console, const char *input, void *refCon) {
	Debugger *debugger = (Debugger *) refCon;

	return debugger->RunCommand(input);
}

bool Debugger::debuggerCompletionCallback(ConsoleDialog *console, const char *input, char*& completion, void *refCon) {
	Debugger *debugger = (Debugger *) refCon;

	return debugger->TabComplete(input, completion);
}
#endif

///////////////////////////////////////////////////
// Now the fun stuff:

void Debugger::DCmd_Register(const char *cmdname, DebugProc pointer) {
	assert(_dcmd_count < (int) sizeof(_dcmds));
	strcpy(_dcmds[_dcmd_count].name, cmdname);
	_dcmds[_dcmd_count].function = pointer;

	_dcmd_count++;
}

// Main Debugger Loop
void Debugger::enter() {
#if USE_CONSOLE
	if (!_vm->_debuggerDialog) {
		_vm->_debuggerDialog = new ConsoleDialog(_vm->_newgui, 1.0, 0.67F);

		Debug_Printf("Debugger started, type 'exit' to return to the game.\n");
		Debug_Printf("Type 'help' to see a little list of commands and variables.\n");
	}

	if (_errStr) {
		Debug_Printf("ERROR: %s\n\n", _errStr);
		free(_errStr);
		_errStr = NULL;
	}

	_vm->_debuggerDialog->setInputeCallback(debuggerInputCallback, this);
	_vm->_debuggerDialog->setCompletionCallback(debuggerCompletionCallback, this);
	_vm->_debuggerDialog->runModal();
#else
	// TODO: compared to the console input, this here is very bare bone.
	// For example, no support for tab completion and no history. At least
	// we should re-add (optional) support for the readline library.
	// Or maybe instead of choosing between a console dialog and stdio,
	// we should move that choice into the ConsoleDialog class - that is,
	// the console dialog code could be #ifdef'ed to not print to the dialog
	// but rather to stdio. This way, we could also reuse the command history
	// and tab completion of the console. It would still require a lot of
	// work, but at least no dependency on a 3rd party library...

	printf("Debugger entered, please switch to this console for input.\n");

	int i;
	char buf[256];

	do {
		printf("debug> ");
		if (!fgets(buf, sizeof(buf), stdin))
			return;

		i = strlen(buf);
		while (i > 0 && buf[i - 1] == '\n')
			buf[--i] = 0;

		if (i == 0)
			continue;
	} while (RunCommand(buf));

#endif
}

// Command execution loop
bool Debugger::RunCommand(const char *inputOrig) {
	int i = 0, num_params = 0;
	const char *param[256];
	char *input = strdup(inputOrig);	// One of the rare occasions using strdup is OK (although avoiding strtok might be more elegant here).

	// Parse out any params
	char *tok = strtok(input, " ");
	if (tok) {
		do {
			param[num_params++] = tok;
		} while ((tok = strtok(NULL, " ")) != NULL);
	} else {
		param[num_params++] = input;
	}

	for (i = 0; i < _dcmd_count; i++) {
		if (!strcmp(_dcmds[i].name, param[0])) {
			bool result = (this->*_dcmds[i].function)(num_params, param);
			free(input);
			return result;
		}
	}

	Debug_Printf("Unknown command\n");
	free(input);
	return true;
}

// Commands

bool Debugger::Cmd_Exit(int argc, const char **argv) {
	_detach_now = true;
	return false;
}

bool Debugger::Cmd_Help(int argc, const char **argv) {
	// console normally has 39 line width
	// wrap around nicely
	int width = 0, size, i;

	Debug_Printf("Commands are:\n");
	for (i = 0 ; i < _dcmd_count ; i++) {
		size = strlen(_dcmds[i].name) + 1;

		if ((width + size) >= 39) {
			Debug_Printf("\n");
			width = size;
		} else
			width += size;

		Debug_Printf("%s ", _dcmds[i].name);
	}

	Debug_Printf("\n");
	return true;
}

bool Debugger::Cmd_Mem(int argc, const char **argv) {
	memory.displayMemory();
	return true;
}

bool Debugger::Cmd_Tony(int argc, const char **argv) {
	Debug_Printf("What about him?\n");
	return true;
}

bool Debugger::Cmd_Res(int argc, const char **argv) {
	res_man.printConsoleClusters();
	return true;
}

bool Debugger::Cmd_Starts(int argc, const char **argv) {
	Con_print_start_menu();
	return true;
}

bool Debugger::Cmd_Start(int argc, const char **argv) {
	uint8 pal[4] = { 255, 255, 255, 0 };

	if (argc != 2) {
		Debug_Printf("Usage: %s number\n", argv[0]);
		return true;
	}

	Con_start(atoi(argv[1]));
	g_display->setPalette(187, 1, pal, RDPAL_INSTANT);
	return true;
}

bool Debugger::Cmd_Info(int argc, const char **argv) {
	displayDebugText = !displayDebugText;

	if (displayDebugText)
		Debug_Printf("Info text on\n");
	else
		Debug_Printf("Info Text off\n");

	return true;
}

bool Debugger::Cmd_WalkGrid(int argc, const char **argv) {
	displayWalkGrid = !displayWalkGrid;

	if (displayWalkGrid)
		Debug_Printf("Walk-grid display on\n");
	else
		Debug_Printf("Walk-grid display off\n");

	return true;
}

bool Debugger::Cmd_Mouse(int argc, const char **argv) {
	displayMouseMarker = !displayMouseMarker;

	if (displayMouseMarker)
		Debug_Printf("Mouse marker on\n");
	else
		Debug_Printf("Mouse marker off\n");

	return true;
}

bool Debugger::Cmd_Player(int argc, const char **argv) {
	displayPlayerMarker = !displayPlayerMarker;

	if (displayPlayerMarker)
		Debug_Printf("Player feet marker on\n");
	else
		Debug_Printf("Player feet marker off\n");

	return true;
}

bool Debugger::Cmd_ResLook(int argc, const char **argv) {
	if (argc != 2)
		Debug_Printf("Usage: %s number\n", argv[0]);
	else
		res_man.examine(atoi(argv[1]));
	return true;
}

bool Debugger::Cmd_CurrentInfo(int argc, const char **argv) {
	Print_current_info();
	return true;
}

bool Debugger::Cmd_RunList(int argc, const char **argv) {
	g_logic.examineRunList();
	return true;
}

bool Debugger::Cmd_Kill(int argc, const char **argv) {
	if (argc != 2)
		Debug_Printf("Usage: %s number\n", argv[0]);
	else
		res_man.kill(atoi(argv[1]));
	return true;
}

bool Debugger::Cmd_Nuke(int argc, const char **argv) {
	Debug_Printf("Killing all resources except variable file and player object\n");
	res_man.killAll(true);
	return true;
}

bool Debugger::Cmd_Var(int argc, const char **argv) {
	switch (argc) {
	case 2:
		Var_check(atoi(argv[1]));
		break;
	case 3:
		Var_set(atoi(argv[1]), atoi(argv[2]));
		break;
	default:
		Debug_Printf("Usage: %s number value\n", argv[0]);
		break;
	}

	return true;
}

bool Debugger::Cmd_Rect(int argc, const char **argv) {
	definingRectangles = !definingRectangles;

	if (definingRectangles)
		Debug_Printf("Mouse rectangles enabled\n");
	else
		Debug_Printf("Mouse rectangles disabled\n");

	draggingRectangle = 0;
	return true;
}

bool Debugger::Cmd_Clear(int argc, const char **argv) {
	res_man.killAllObjects(true);
	return true;
}

bool Debugger::Cmd_DebugOn(int argc, const char **argv) {
	displayDebugText = true;
	displayWalkGrid = true;
	displayMouseMarker = true;
	displayPlayerMarker = true;
	displayTextNumbers = true;
	Debug_Printf("Enabled all on-screen debug info\n");
	return true;
}

bool Debugger::Cmd_DebugOff(int argc, const char **argv) {
	displayDebugText = false;
	displayWalkGrid = false;
	displayMouseMarker = false;
	displayPlayerMarker = false;
	displayTextNumbers = false;
	Debug_Printf("Disabled all on-screen debug info\n");
	return true;
}

bool Debugger::Cmd_SaveRest(int argc, const char **argv) {
	testingSnR = !testingSnR;

	if (testingSnR)
		Debug_Printf("Enabled S&R logic_script stability checking\n");
	else
		Debug_Printf("Disabled S&R logic_script stability checking\n");

	return true;
}

bool Debugger::Cmd_ListSaveGames(int argc, const char **argv) {
	Debug_Printf("Savegames:\n");

	for (int i = 0; i < 100; i++) {
		uint8 description[SAVE_DESCRIPTION_LEN];

		// if there is a save game print the name
		if (GetSaveDescription(i, description) == SR_OK)
			Debug_Printf("%d: \"%s\"\n", i, description);
	}

	return true;
}

bool Debugger::Cmd_SaveGame(int argc, const char **argv) {
	char description[SAVE_DESCRIPTION_LEN];
	int len = 0;
	uint16 slotNo;
	uint32 rv;

	if (argc < 3) {
		Debug_Printf("Usage: %s slot description\n", argv[0]);
		return true;
	}

	// if mouse if off, or system menu is locked off
	if (mouse_status || mouse_mode_locked) {
		Debug_Printf("WARNING: Cannot save game while control menu unavailable!\n");
		return true;
	}

	description[0] = 0;

	// FIXME: Strange things seem to happen if use too long savegame names,
	// even when they're shorter than the maximum allowed length

	for (int i = 2; i < argc; i++) {
		if (len + strlen(argv[i]) + 1 > SAVE_DESCRIPTION_LEN)
			break;

		if (i == 2) {
			strcpy(description, argv[i]);
			len = strlen(argv[i]);
		} else {
			strcat(description, " ");
			strcat(description, argv[i]);
			len += (strlen(argv[i]) + 1);
		}
	}

	slotNo = atoi(argv[1]);
	rv = SaveGame(slotNo, (uint8 *) description);

	if (rv == SR_OK)
		Debug_Printf("Saved game \"%s\" to file \"savegame.%.3d\"\n", description, slotNo);
	else if (rv == SR_ERR_FILEOPEN)
		Debug_Printf("ERROR: Cannot open file \"savegame.%.3d\"\n", slotNo);
	else	// SR_ERR_WRITEFAIL
		Debug_Printf("ERROR: Write error on file \"savegame.%.3d\"\n", slotNo);

	return true;
}

bool Debugger::Cmd_RestoreGame(int argc, const char **argv) {
	uint16 slotNo;
	uint8 description[SAVE_DESCRIPTION_LEN];
	uint32 rv;

	if (argc != 2) {
		Debug_Printf("Usage: %s slot\n", argv[0]);
		return true;
	}

	// if mouse if off, or system menu is locked off
	if (mouse_status || mouse_mode_locked) {
		Debug_Printf("WARNING: Cannot restore game while control menu unavailable!\n");
		return true;
	}

	slotNo = atoi(argv[1]);
	rv = RestoreGame(slotNo);

	if (rv == SR_OK) {
		GetSaveDescription(slotNo, description);
		Debug_Printf("Restored game \"%s\" from file \"savegame.%.3d\"\n", description, slotNo);
	} else if (rv == SR_ERR_FILEOPEN)
		Debug_Printf("ERROR: Cannot open file \"savegame.%.3d\"\n", slotNo);
	else if (rv == SR_ERR_INCOMPATIBLE)
		Debug_Printf("ERROR: \"savegame.%.3d\" is no longer compatible with current player/variable resources\n", slotNo);
	else	// SR_ERR_READFAIL
		Debug_Printf("ERROR: Read error on file \"savegame.%.3d\"\n", slotNo);

	return true;
}

// FIXME: Replace these with a command to modify the graphics detail setting

bool Debugger::Cmd_BltFxOn(int argc, const char **argv) {
	// g_display->setBltFx();
	// Debug_Printf("Blit fx enabled\n");
	Debug_Printf("FIXME: The setBltFx() function no longer exists\n");
	return true;
}

bool Debugger::Cmd_BltFxOff(int argc, const char **argv) {
	// g_display->clearBltFx();
	// Debug_Printf("Blit fx disabled\n");
	Debug_Printf("FIXME: The clearBltFx() function no longer exists\n");
	return true;
}

bool Debugger::Cmd_TimeOn(int argc, const char **argv) {
	if (argc == 2)
		startTime = SVM_timeGetTime() - atoi(argv[1]) * 1000;
	else if (startTime == 0)
		startTime = SVM_timeGetTime();
	displayTime = true;
	Debug_Printf("Timer display on\n");
	return true;
}

bool Debugger::Cmd_TimeOff(int argc, const char **argv) {
	displayTime = false;
	Debug_Printf("Timer display off\n");
	return true;
}

bool Debugger::Cmd_Text(int argc, const char **argv) {
	displayTextNumbers = !displayTextNumbers;

	if (displayTextNumbers)
		Debug_Printf("Text numbers on\n");
	else
		Debug_Printf("Text numbers off\n");

	return true;
}

bool Debugger::Cmd_ShowVar(int argc, const char **argv) {
	int32 showVarNo = 0;
	int32 varNo;

	if (argc != 2) {
		Debug_Printf("Usage: %s number\n", argv[0]);
		return true;
	}

	varNo = atoi(argv[1]);
	
	// search for a spare slot in the watch-list, but also watch out for
	// this variable already being in the list

	while (showVarNo < MAX_SHOWVARS && showVar[showVarNo] != 0 && showVar[showVarNo] != varNo)
		showVarNo++;

	// if we've found a spare slot or the variable's already there
	if (showVarNo < MAX_SHOWVARS) {
		if (showVar[showVarNo] == 0) {
			// empty slot - add it to the list at this slot
			showVar[showVarNo] = varNo;
			Debug_Printf("var(%d) added to the watch-list\n", varNo);
		} else
			Debug_Printf("var(%d) already in the watch-list!\n", varNo);
	} else
		Debug_Printf("Sorry - no more allowed - hide one or extend the system watch-list\n");

	return true;
}

bool Debugger::Cmd_HideVar(int argc, const char **argv) {
	int32 showVarNo = 0;
	int32 varNo;

	if (argc != 2) {
		Debug_Printf("Usage: %s number\n", argv[0]);
		return true;
	}

	varNo = atoi(argv[1]);
	
	// search for 'varNo' in the watch-list
	while (showVarNo < MAX_SHOWVARS && showVar[showVarNo] != varNo)
		showVarNo++;

	if (showVarNo < MAX_SHOWVARS) {
		// We've found 'varNo' in the list - clear this slot
		showVar[showVarNo] = 0;
		Debug_Printf("var(%d) removed from watch-list\n", varNo);
	} else
		Debug_Printf("Sorry - can't find var(%d) in the list\n", varNo);

	return true;
}

bool Debugger::Cmd_Version(int argc, const char **argv) {

	// The version string is incomplete, so we may as well remove the code
	// to extract information from it.

#if 0

	#define HEAD_LEN 8

	// version & owner details

	// So version string is 18 bytes long :
	// Version String =  <8 byte header,5 character version, \0, INT32 time>
	uint8 version_string[HEAD_LEN + 10] = { 1, 255, 37, 22, 45, 128, 34, 67 };
	uint8 unencoded_name[HEAD_LEN + 48] = {
		76, 185, 205, 23, 44, 34, 24, 34,
		'R','e','v','o','l','u','t','i','o','n',' ',
		'S','o','f','t','w','a','r','e',' ','L','t','d',
		0 };

	struct tm *time;
	time_t t;
	char dateStamp[255];
	char version[6];

	strcpy(version, (char*) version_string + HEAD_LEN);
	*(((unsigned char *) &t)) = *(version_string + 14);
	*(((unsigned char *) &t) + 1) = *(version_string + 15);
	*(((unsigned char *) &t) + 2) = *(version_string + 16);
	*(((unsigned char *) &t) + 3) = *(version_string + 17);

	time = localtime(&t);
	sprintf(dateStamp, "%s", asctime(time));
	dateStamp[24] = 0;	// fudge over the newline character!
#endif

	Debug_Printf("\"Broken Sword II\" (c) Revolution Software 1997.\n");

#if 0
	Debug_Printf("v%s created on %s for %s\n", version, dateStamp, unencoded_name + HEAD_LEN);
#endif

#if 0
	// THE FOLLOWING LINES ARE TO BE COMMENTED OUT OF THE FINAL VERSION
	Debug_Printf("This program has a personalised fingerprint encrypted into the code.\n");
	Debug_Printf("If this CD was not sent directly to you by Virgin Interactive or Revolution Software\n");
	Debug_Printf("then please contact James Long at Revolution on (+44) 1904 639698.\n");
#endif

	return true;
}

bool Debugger::Cmd_SoftHard(int argc, const char **argv) {
	Debug_Printf("ScummVM doesn't distinguish between software and hardware rendering.\n");
	return true;
}

bool Debugger::Cmd_AnimTest(int argc, const char **argv) {
	if (argc != 2) {
		Debug_Printf("Usage: %s value\n", argv[0]);
		return true;
	}

	// Automatically do "s 32" to run the animation testing start script
	Con_start(32);

	// Same as typing "VAR 912 <value>" at the console
	Var_set(912, atoi(argv[1]));

	Debug_Printf("Setting flag 'system_testing_anims'\n");
	return true;
}

bool Debugger::Cmd_TextTest(int argc, const char **argv) {
	if (argc != 2) {
		Debug_Printf("Usage: %s value\n", argv[0]);
		return true;
	}

	// Automatically do "s 33" to run the text/speech testing start script
	Con_start(33);

	// Same as typing "VAR 1230 <value>" at the console
	Var_set(1230, atoi(argv[1]));

	displayTextNumbers = true;

	Debug_Printf("Setting flag 'system_testing_text'\n");
	Debug_Printf("Text numbers on\n");
	return true;
}

bool Debugger::Cmd_LineTest(int argc, const char **argv) {
	if (argc != 3) {
		Debug_Printf("Usage: %s value1 value2\n", argv[0]);
		return true;
	}

	// Automatically do "s 33" to run the text/speech testing start script
	Con_start(33);

	// Same as typing "VAR 1230 <value>" at the console
	Var_set(1230, atoi(argv[1]));

	// Same as typing "VAR 1264 <value>" at the console
	Var_set(1264, atoi(argv[2]));

	displayTextNumbers = true;

	Debug_Printf("Setting flag 'system_testing_text'\n");
	Debug_Printf("Setting flag 'system_test_line_no'\n");
	Debug_Printf("Text numbers on\n");
	return true;
}

bool Debugger::Cmd_Grab(int argc, const char **argv) {
	Debug_Printf("FIXME: Continuous screen-grabbing not implemented\n");

#if 0
	grabbingSequences = !grabbingSequences;

	if (grabbingSequences)
		Debug_Printf("PCX-grabbing enabled\n");
	else
		Debug_Printf("PCX-grabbing disabled\n");
#endif

	return true;
}

bool Debugger::Cmd_Events(int argc, const char **argv) {
	Debug_Printf("EVENT LIST:\n");

	for (uint32 i = 0; i < MAX_events; i++) {
		if (event_list[i].id) {
			uint32 target = event_list[i].id;
			uint32 script = event_list[i].interact_id;

			Debug_Printf("slot %d: id = %s (%d)\n", i, FetchObjectName(target), target);
			Debug_Printf("         script = %s (%d) pos %d\n", FetchObjectName(script / 65536), script / 65536, script % 65536);
		}
	}

	return true;
}

bool Debugger::Cmd_Sfx(int argc, const char **argv) {
	wantSfxDebug = !wantSfxDebug;

	if (wantSfxDebug)
		Debug_Printf("SFX logging activated\n");
	else
		Debug_Printf("SFX logging deactivated\n");

	return true;
}

bool Debugger::Cmd_English(int argc, const char **argv) {
	g_sword2->initialiseFontResourceFlags(DEFAULT_TEXT);
	Debug_Printf("Default fonts selected\n");
	return true;
}

bool Debugger::Cmd_Finnish(int argc, const char **argv) {
	g_sword2->initialiseFontResourceFlags(FINNISH_TEXT);
	Debug_Printf("Finnish fonts selected\n");
	return true;
}

bool Debugger::Cmd_Polish(int argc, const char **argv) {
	g_sword2->initialiseFontResourceFlags(POLISH_TEXT);
	Debug_Printf("Polish fonts selected\n");
	return true;
}

// returns true if something has been completed
// completion has to be delete[]-ed then
bool Debugger::TabComplete(const char *input, char*& completion) {
	// very basic tab completion
	// for now it just supports command completions

	// adding completions of command parameters would be nice (but hard) :-)
	// maybe also give a list of possible command completions?
	//   (but this will require changes to console)

	if (strchr(input, ' '))
		return false; // already finished the first word

	unsigned int inputlen = strlen(input);

	unsigned int matchlen = 0;
	char match[30]; // the max. command name is 30 chars

	for (int i = 0; i < _dcmd_count; i++) {
		if (!strncmp(_dcmds[i].name, input, inputlen)) {
			unsigned int commandlen = strlen(_dcmds[i].name);
			if (commandlen == inputlen) { // perfect match
				return false;
			}
			if (commandlen > inputlen) { // possible match
				// no previous match
				if (matchlen == 0) {
					strcpy(match, _dcmds[i].name + inputlen);
					matchlen = commandlen - inputlen;
				} else {
					// take common prefix of previous match and this command
					unsigned int j;
					for (j = 0; j < matchlen; j++) {
						if (match[j] != _dcmds[i].name[inputlen + j]) break;
					}
					matchlen = j;
				}
				if (matchlen == 0)
					return false;
			}
		}
	}
	if (matchlen == 0)
		return false;

	completion = new char[matchlen + 1];
	memcpy(completion, match, matchlen);
	completion[matchlen] = 0;
	return true;
}

} // End of namespace Sword2
