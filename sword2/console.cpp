/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
#include "sword2/console.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/resman.h"
#include "sword2/sound.h"
#include "sword2/driver/d_draw.h"

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

	_speechScriptWaiting = 0;	// The id of whoever we're waiting for
					// in a speech script. See fnTheyDo(),
					// fnTheyDoWeWait(), fnWeWait(), and
					// fnTimedWait().

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
	DCmd_Register("reslist", &Debugger::Cmd_ResList);
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
	DCmd_Register("debugoff", &Debugger::Cmd_DebugOff);
	DCmd_Register("saverest", &Debugger::Cmd_SaveRest);
	DCmd_Register("timeon", &Debugger::Cmd_TimeOn);
	DCmd_Register("timeoff", &Debugger::Cmd_TimeOff);
	DCmd_Register("text", &Debugger::Cmd_Text);
	DCmd_Register("showvar", &Debugger::Cmd_ShowVar);
	DCmd_Register("hidevar", &Debugger::Cmd_HideVar);
	DCmd_Register("version", &Debugger::Cmd_Version);
	DCmd_Register("animtest", &Debugger::Cmd_AnimTest);
	DCmd_Register("texttest", &Debugger::Cmd_TextTest);
	DCmd_Register("linetest", &Debugger::Cmd_LineTest);
	DCmd_Register("events", &Debugger::Cmd_Events);
	DCmd_Register("sfx", &Debugger::Cmd_Sfx);
	DCmd_Register("english", &Debugger::Cmd_English);
	DCmd_Register("finnish", &Debugger::Cmd_Finnish);
	DCmd_Register("polish", &Debugger::Cmd_Polish);
}

void Debugger::varGet(int var) {
	DebugPrintf("%d\n", Logic::_scriptVars[var]);
}

void Debugger::varSet(int var, int val) {
	DebugPrintf("was %d, ", Logic::_scriptVars[var]);
	Logic::_scriptVars[var] = val;
	DebugPrintf("now %d\n", Logic::_scriptVars[var]);
}

void Debugger::preEnter() {
	// Pause sound output
	if (_vm->_sound) {
		_vm->_sound->pauseFx();
		_vm->_sound->pauseSpeech();
		_vm->_sound->pauseMusic();
	}
}

void Debugger::postEnter() {
	if (_vm->_sound) {
		// Resume previous sound state
		_vm->_sound->unpauseFx();
		_vm->_sound->unpauseSpeech();
		_vm->_sound->unpauseMusic();
	}

	if (_vm->_graphics) {
		// Restore old mouse cursor
		_vm->_graphics->drawMouse();
	}
}

// Now the fun stuff: Commands

bool Debugger::Cmd_Exit(int argc, const char **argv) {
	_detach_now = true;
	return false;
}

bool Debugger::Cmd_Help(int argc, const char **argv) {
	// console normally has 78 line width
	// wrap around nicely
	int width = 0;

	DebugPrintf("Commands are:\n");
	for (int i = 0 ; i < _dcmd_count ; i++) {
		int size = strlen(_dcmds[i].name) + 1;

		if (width + size >= 75) {
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
	_vm->_memory->memDisplay();
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

bool Debugger::Cmd_ResList(int argc, const char **argv) {
	// By default, list only resources that are being held open.
	uint minCount = 1;

	if (argc > 1)
		minCount = atoi(argv[1]);

	_vm->_resman->listResources(minCount);
	return true;
}

bool Debugger::Cmd_Starts(int argc, const char **argv) {
	_vm->conPrintStartMenu();
	return true;
}

bool Debugger::Cmd_Start(int argc, const char **argv) {
	uint8 pal[4] = { 255, 255, 255, 0 };

	if (argc != 2) {
		DebugPrintf("Usage: %s number\n", argv[0]);
		return true;
	}

	_vm->conStart(atoi(argv[1]));
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
	uint32 filter = _vm->setEventFilter(0);

	_definingRectangles = !_definingRectangles;

	if (_definingRectangles) {
		_vm->setEventFilter(filter & ~(RD_LEFTBUTTONUP | RD_RIGHTBUTTONUP));
		DebugPrintf("Mouse rectangles enabled\n");
	} else {
		_vm->setEventFilter(filter | RD_LEFTBUTTONUP | RD_RIGHTBUTTONUP);
		DebugPrintf("Mouse rectangles disabled\n");
	}

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

bool Debugger::Cmd_TimeOn(int argc, const char **argv) {
	if (argc == 2)
		_startTime = _vm->_system->getMillis() - atoi(argv[1]) * 1000;
	else if (_startTime == 0)
		_startTime = _vm->_system->getMillis();
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
	// This function used to print more information, but nothing we
	// particularly care about.

	DebugPrintf("\"Broken Sword II\" (c) Revolution Software 1997.\n");
	return true;
}

bool Debugger::Cmd_AnimTest(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s value\n", argv[0]);
		return true;
	}

	// Automatically do "s 32" to run the animation testing start script
	_vm->conStart(32);

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
	_vm->conStart(33);

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
	_vm->conStart(33);

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

bool Debugger::Cmd_Events(int argc, const char **argv) {
	_vm->_logic->printEventList();
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
