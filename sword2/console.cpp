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

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"

#include "common/debugger.cpp"

namespace Sword2 {

Debugger::Debugger(Sword2Engine *vm)
	: Common::Debugger<Debugger>() {
	_vm = vm;

	memset(_debugTextBlocks, 0, sizeof(_debugTextBlocks));
	memset(_showVar, 0, sizeof(_showVar));

	_displayDebugText = false;	// "INFO"
	_displayWalkGrid = false;	// "WALKGRID"
	_displayMouseMarker = false;	// "MOUSE"
	_displayTime = false;		// "TIME"
	_displayPlayerMarker = false;	// "PLAYER"
	_displayTextNumbers = false;	// "TEXT"

	_definingRectangles = false;	// "RECT"
	_draggingRectangle = 0;		// 0 = waiting to start new rect
					// 1 = currently dragging a rectangle

	_rectX1 = _rectY1 = 0;
	_rectX2 = _rectY2 = 0;
	_rectFlicker = false;

	_testingSnR = false;		// "SAVEREST" - for system to kill all
					// object resources (except player) in
					// fnAddHuman()

	_startTime = 0;			// "TIMEON" & "TIMEOFF" - system start
					// time

	_textNumber = 0;		// Current system text line number

	_playerGraphicNoFrames = 0;	// No. of frames in currently displayed
					// anim

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

void Debugger::varGet(int var) {
	Debug_Printf("%d\n", VAR(var));
}

void Debugger::varSet(int var, int val) {
	Debug_Printf("was %d, ", VAR(var));
	VAR(var) = val;
	Debug_Printf("now %d\n", VAR(var));
}

void Debugger::preEnter() {
	// Pause sound output
	_vm->_sound->pauseFx();
	_vm->_sound->pauseSpeech();
	_vm->_sound->pauseMusic();
}

void Debugger::postEnter() {
	// Resume previous sound state
	_vm->_sound->unpauseFx();
	_vm->_sound->unpauseSpeech();
	_vm->_sound->unpauseMusic();

	// Restore old mouse cursor
	_vm->_graphics->drawMouse();
}

///////////////////////////////////////////////////
// Now the fun stuff:

// Commands

bool Debugger::Cmd_Exit(int argc, const char **argv) {
	_detach_now = true;
	return false;
}

bool Debugger::Cmd_Help(int argc, const char **argv) {
	// console normally has 39 line width
	// wrap around nicely
	int width = 0, size, i;

	DebugPrintf("Commands are:\n");
	for (i = 0 ; i < _dcmd_count ; i++) {
		size = strlen(_dcmds[i].name) + 1;

		if ((width + size) >= 39) {
			DebugPrintf("\n");
			width = size;
		} else
			width += size;

		DebugPrintf("%s ", _dcmds[i].name);
	}

	DebugPrintf("\n");
	return true;
}

bool Debugger::Cmd_Mem(int argc, const char **argv) {
	_vm->_memory->displayMemory();
	return true;
}

bool Debugger::Cmd_Tony(int argc, const char **argv) {
	DebugPrintf("What about him?\n");
	return true;
}

bool Debugger::Cmd_Res(int argc, const char **argv) {
	_vm->_resman->printConsoleClusters();
	return true;
}

bool Debugger::Cmd_Starts(int argc, const char **argv) {
	_vm->_logic->conPrintStartMenu();
	return true;
}

bool Debugger::Cmd_Start(int argc, const char **argv) {
	uint8 pal[4] = { 255, 255, 255, 0 };

	if (argc != 2) {
		DebugPrintf("Usage: %s number\n", argv[0]);
		return true;
	}

	_vm->_logic->conStart(atoi(argv[1]));
	_vm->_graphics->setPalette(187, 1, pal, RDPAL_INSTANT);
	return true;
}

bool Debugger::Cmd_Info(int argc, const char **argv) {
	_displayDebugText = !_displayDebugText;

	if (_displayDebugText)
		DebugPrintf("Info text on\n");
	else
		DebugPrintf("Info Text off\n");

	return true;
}

bool Debugger::Cmd_WalkGrid(int argc, const char **argv) {
	_displayWalkGrid = !_displayWalkGrid;

	if (_displayWalkGrid)
		DebugPrintf("Walk-grid display on\n");
	else
		DebugPrintf("Walk-grid display off\n");

	return true;
}

bool Debugger::Cmd_Mouse(int argc, const char **argv) {
	_displayMouseMarker = !_displayMouseMarker;

	if (_displayMouseMarker)
		DebugPrintf("Mouse marker on\n");
	else
		DebugPrintf("Mouse marker off\n");

	return true;
}

bool Debugger::Cmd_Player(int argc, const char **argv) {
	_displayPlayerMarker = !_displayPlayerMarker;

	if (_displayPlayerMarker)
		DebugPrintf("Player feet marker on\n");
	else
		DebugPrintf("Player feet marker off\n");

	return true;
}

bool Debugger::Cmd_ResLook(int argc, const char **argv) {
	if (argc != 2)
		DebugPrintf("Usage: %s number\n", argv[0]);
	else
		_vm->_resman->examine(atoi(argv[1]));
	return true;
}

bool Debugger::Cmd_CurrentInfo(int argc, const char **argv) {
	printCurrentInfo();
	return true;
}

bool Debugger::Cmd_RunList(int argc, const char **argv) {
	_vm->_logic->examineRunList();
	return true;
}

bool Debugger::Cmd_Kill(int argc, const char **argv) {
	if (argc != 2)
		DebugPrintf("Usage: %s number\n", argv[0]);
	else
		_vm->_resman->kill(atoi(argv[1]));
	return true;
}

bool Debugger::Cmd_Nuke(int argc, const char **argv) {
	DebugPrintf("Killing all resources except variable file and player object\n");
	_vm->_resman->killAll(true);
	return true;
}

bool Debugger::Cmd_Var(int argc, const char **argv) {
	switch (argc) {
	case 2:
		varGet(atoi(argv[1]));
		break;
	case 3:
		varSet(atoi(argv[1]), atoi(argv[2]));
		break;
	default:
		DebugPrintf("Usage: %s number value\n", argv[0]);
		break;
	}

	return true;
}

bool Debugger::Cmd_Rect(int argc, const char **argv) {
	_definingRectangles = !_definingRectangles;

	if (_definingRectangles)
		DebugPrintf("Mouse rectangles enabled\n");
	else
		DebugPrintf("Mouse rectangles disabled\n");

	_draggingRectangle = 0;
	return true;
}

bool Debugger::Cmd_Clear(int argc, const char **argv) {
	_vm->_resman->killAllObjects(true);
	return true;
}

bool Debugger::Cmd_DebugOn(int argc, const char **argv) {
	_displayDebugText = true;
	_displayWalkGrid = true;
	_displayMouseMarker = true;
	_displayPlayerMarker = true;
	_displayTextNumbers = true;
	DebugPrintf("Enabled all on-screen debug info\n");
	return true;
}

bool Debugger::Cmd_DebugOff(int argc, const char **argv) {
	_displayDebugText = false;
	_displayWalkGrid = false;
	_displayMouseMarker = false;
	_displayPlayerMarker = false;
	_displayTextNumbers = false;
	DebugPrintf("Disabled all on-screen debug info\n");
	return true;
}

bool Debugger::Cmd_SaveRest(int argc, const char **argv) {
	_testingSnR = !_testingSnR;

	if (_testingSnR)
		DebugPrintf("Enabled S&R logic_script stability checking\n");
	else
		DebugPrintf("Disabled S&R logic_script stability checking\n");

	return true;
}

bool Debugger::Cmd_ListSaveGames(int argc, const char **argv) {
	DebugPrintf("Savegames:\n");

	for (int i = 0; i < 100; i++) {
		uint8 description[SAVE_DESCRIPTION_LEN];

		// if there is a save game print the name
		if (_vm->getSaveDescription(i, description) == SR_OK)
			DebugPrintf("%d: \"%s\"\n", i, description);
	}

	return true;
}

bool Debugger::Cmd_SaveGame(int argc, const char **argv) {
	char description[SAVE_DESCRIPTION_LEN];
	int len = 0;
	uint16 slotNo;
	uint32 rv;

	if (argc < 3) {
		DebugPrintf("Usage: %s slot description\n", argv[0]);
		return true;
	}

	// if mouse if off, or system menu is locked off
	if (_vm->_mouseStatus || _vm->_mouseModeLocked) {
		DebugPrintf("WARNING: Cannot save game while control menu unavailable!\n");
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
	rv = _vm->saveGame(slotNo, (uint8 *) description);

	if (rv == SR_OK)
		DebugPrintf("Saved game \"%s\" to file \"savegame.%.3d\"\n", description, slotNo);
	else if (rv == SR_ERR_FILEOPEN)
		DebugPrintf("ERROR: Cannot open file \"savegame.%.3d\"\n", slotNo);
	else	// SR_ERR_WRITEFAIL
		DebugPrintf("ERROR: Write error on file \"savegame.%.3d\"\n", slotNo);

	return true;
}

bool Debugger::Cmd_RestoreGame(int argc, const char **argv) {
	uint16 slotNo;
	uint8 description[SAVE_DESCRIPTION_LEN];
	uint32 rv;

	if (argc != 2) {
		DebugPrintf("Usage: %s slot\n", argv[0]);
		return true;
	}

	// if mouse if off, or system menu is locked off
	if (_vm->_mouseStatus || _vm->_mouseModeLocked) {
		DebugPrintf("WARNING: Cannot restore game while control menu unavailable!\n");
		return true;
	}

	slotNo = atoi(argv[1]);
	rv = _vm->restoreGame(slotNo);

	if (rv == SR_OK) {
		_vm->getSaveDescription(slotNo, description);
		DebugPrintf("Restored game \"%s\" from file \"savegame.%.3d\"\n", description, slotNo);
	} else if (rv == SR_ERR_FILEOPEN)
		DebugPrintf("ERROR: Cannot open file \"savegame.%.3d\"\n", slotNo);
	else if (rv == SR_ERR_INCOMPATIBLE)
		DebugPrintf("ERROR: \"savegame.%.3d\" is no longer compatible with current player/variable resources\n", slotNo);
	else	// SR_ERR_READFAIL
		DebugPrintf("ERROR: Read error on file \"savegame.%.3d\"\n", slotNo);

	return true;
}

// FIXME: Replace these with a command to modify the graphics detail setting

bool Debugger::Cmd_BltFxOn(int argc, const char **argv) {
	// _vm->_graphics->setBltFx();
	// DebugPrintf("Blit fx enabled\n");
	DebugPrintf("FIXME: The setBltFx() function no longer exists\n");
	return true;
}

bool Debugger::Cmd_BltFxOff(int argc, const char **argv) {
	// _vm->_graphics->clearBltFx();
	// DebugPrintf("Blit fx disabled\n");
	DebugPrintf("FIXME: The clearBltFx() function no longer exists\n");
	return true;
}

bool Debugger::Cmd_TimeOn(int argc, const char **argv) {
	if (argc == 2)
		_startTime = _vm->_system->get_msecs() - atoi(argv[1]) * 1000;
	else if (_startTime == 0)
		_startTime = _vm->_system->get_msecs();
	_displayTime = true;
	DebugPrintf("Timer display on\n");
	return true;
}

bool Debugger::Cmd_TimeOff(int argc, const char **argv) {
	_displayTime = false;
	DebugPrintf("Timer display off\n");
	return true;
}

bool Debugger::Cmd_Text(int argc, const char **argv) {
	_displayTextNumbers = !_displayTextNumbers;

	if (_displayTextNumbers)
		DebugPrintf("Text numbers on\n");
	else
		DebugPrintf("Text numbers off\n");

	return true;
}

bool Debugger::Cmd_ShowVar(int argc, const char **argv) {
	int32 showVarNo = 0;
	int32 varNo;

	if (argc != 2) {
		DebugPrintf("Usage: %s number\n", argv[0]);
		return true;
	}

	varNo = atoi(argv[1]);
	
	// search for a spare slot in the watch-list, but also watch out for
	// this variable already being in the list

	while (showVarNo < MAX_SHOWVARS && _showVar[showVarNo] != 0 && _showVar[showVarNo] != varNo)
		showVarNo++;

	// if we've found a spare slot or the variable's already there
	if (showVarNo < MAX_SHOWVARS) {
		if (_showVar[showVarNo] == 0) {
			// empty slot - add it to the list at this slot
			_showVar[showVarNo] = varNo;
			DebugPrintf("var(%d) added to the watch-list\n", varNo);
		} else
			DebugPrintf("var(%d) already in the watch-list!\n", varNo);
	} else
		DebugPrintf("Sorry - no more allowed - hide one or extend the system watch-list\n");

	return true;
}

bool Debugger::Cmd_HideVar(int argc, const char **argv) {
	int32 showVarNo = 0;
	int32 varNo;

	if (argc != 2) {
		DebugPrintf("Usage: %s number\n", argv[0]);
		return true;
	}

	varNo = atoi(argv[1]);
	
	// search for 'varNo' in the watch-list
	while (showVarNo < MAX_SHOWVARS && _showVar[showVarNo] != varNo)
		showVarNo++;

	if (showVarNo < MAX_SHOWVARS) {
		// We've found 'varNo' in the list - clear this slot
		_showVar[showVarNo] = 0;
		DebugPrintf("var(%d) removed from watch-list\n", varNo);
	} else
		DebugPrintf("Sorry - can't find var(%d) in the list\n", varNo);

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

	DebugPrintf("\"Broken Sword II\" (c) Revolution Software 1997.\n");

#if 0
	DebugPrintf("v%s created on %s for %s\n", version, dateStamp, unencoded_name + HEAD_LEN);
#endif

#if 0
	// THE FOLLOWING LINES ARE TO BE COMMENTED OUT OF THE FINAL VERSION
	DebugPrintf("This program has a personalised fingerprint encrypted into the code.\n");
	DebugPrintf("If this CD was not sent directly to you by Virgin Interactive or Revolution Software\n");
	DebugPrintf("then please contact James Long at Revolution on (+44) 1904 639698.\n");
#endif

	return true;
}

bool Debugger::Cmd_SoftHard(int argc, const char **argv) {
	DebugPrintf("ScummVM doesn't distinguish between software and hardware rendering.\n");
	return true;
}

bool Debugger::Cmd_AnimTest(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s value\n", argv[0]);
		return true;
	}

	// Automatically do "s 32" to run the animation testing start script
	_vm->_logic->conStart(32);

	// Same as typing "VAR 912 <value>" at the console
	varSet(912, atoi(argv[1]));

	DebugPrintf("Setting flag 'system_testing_anims'\n");
	return true;
}

bool Debugger::Cmd_TextTest(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s value\n", argv[0]);
		return true;
	}

	// Automatically do "s 33" to run the text/speech testing start script
	_vm->_logic->conStart(33);

	// Same as typing "VAR 1230 <value>" at the console
	varSet(1230, atoi(argv[1]));

	_displayTextNumbers = true;

	DebugPrintf("Setting flag 'system_testing_text'\n");
	DebugPrintf("Text numbers on\n");
	return true;
}

bool Debugger::Cmd_LineTest(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: %s value1 value2\n", argv[0]);
		return true;
	}

	// Automatically do "s 33" to run the text/speech testing start script
	_vm->_logic->conStart(33);

	// Same as typing "VAR 1230 <value>" at the console
	varSet(1230, atoi(argv[1]));

	// Same as typing "VAR 1264 <value>" at the console
	varSet(1264, atoi(argv[2]));

	_displayTextNumbers = true;

	DebugPrintf("Setting flag 'system_testing_text'\n");
	DebugPrintf("Setting flag 'system_test_line_no'\n");
	DebugPrintf("Text numbers on\n");
	return true;
}

bool Debugger::Cmd_Grab(int argc, const char **argv) {
	DebugPrintf("FIXME: Continuous screen-grabbing not implemented\n");

#if 0
	_vm->_grabbingSequences = !_vm->_grabbingSequences;

	if (_vm->_grabbingSequences)
		DebugPrintf("PCX-grabbing enabled\n");
	else
		DebugPrintf("PCX-grabbing disabled\n");
#endif

	return true;
}

bool Debugger::Cmd_Events(int argc, const char **argv) {
	DebugPrintf("EVENT LIST:\n");

	for (uint32 i = 0; i < MAX_events; i++) {
		if (_vm->_logic->_eventList[i].id) {
			uint32 target = _vm->_logic->_eventList[i].id;
			uint32 script = _vm->_logic->_eventList[i].interact_id;

			DebugPrintf("slot %d: id = %s (%d)\n", i, _vm->fetchObjectName(target), target);
			DebugPrintf("         script = %s (%d) pos %d\n", _vm->fetchObjectName(script / 65536), script / 65536, script % 65536);
		}
	}

	return true;
}

bool Debugger::Cmd_Sfx(int argc, const char **argv) {
	_vm->_wantSfxDebug = !_vm->_wantSfxDebug;

	if (_vm->_wantSfxDebug)
		DebugPrintf("SFX logging activated\n");
	else
		DebugPrintf("SFX logging deactivated\n");

	return true;
}

bool Debugger::Cmd_English(int argc, const char **argv) {
	_vm->initialiseFontResourceFlags(DEFAULT_TEXT);
	DebugPrintf("Default fonts selected\n");
	return true;
}

bool Debugger::Cmd_Finnish(int argc, const char **argv) {
	_vm->initialiseFontResourceFlags(FINNISH_TEXT);
	DebugPrintf("Finnish fonts selected\n");
	return true;
}

bool Debugger::Cmd_Polish(int argc, const char **argv) {
	_vm->initialiseFontResourceFlags(POLISH_TEXT);
	DebugPrintf("Polish fonts selected\n");
	return true;
}

} // End of namespace Sword2
