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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "agi/agi.h"
#include "agi/opcodes.h"

namespace Agi {

AgiInstruction *logicNamesTest;
AgiInstruction *logicNamesCmd;

AgiInstruction insV1Test[] = {
	{ "",                   "",         &condUnknown },     // 00
	{ "equaln",             "vn",       &condEqual },       // 01
	{ "equalv",             "vv",       &condEqualV },      // 02
	{ "lessn",              "vn",       &condLess },        // 03
	{ "lessv",              "vv",       &condLessV },       // 04
	{ "greatern",           "vn",       &condGreater },     // 05
	{ "greaterv",           "vv",       &condGreaterV },    // 06
	{ "isset",              "v",        &condIsSetV1 },     // 07
	{ "has",                "n",        &condHasV1 },       // 08
	{ "said",               "nnnn",     &condSaid2 },       // 09
	{ "posn",               "nnnnn",    &condPosn },        // 0A
	{ "controller",         "n",        &condController },  // 0B
	{ "obj.in.room",        "nv",       &condObjInRoom },   // 0C
	{ "said",               "nnnnnn",   &condSaid3 },       // 0D
	{ "have.key",           "",         &condHaveKey },     // 0E
	{ "said",               "nn",       &condSaid1 },       // 0F
	{ "bit",                "nv",       &condBit },         // 10
};

AgiInstruction insV1[] = {
	{ "return",             "",         NULL },                 // 00
	{ "increment",          "v",        &cmdIncrement },        // 01
	{ "decrement",          "v",        &cmdDecrement },        // 02
	{ "assignn",            "vn",       &cmdAssignN },          // 03
	{ "assignv",            "vv",       &cmdAssignV },          // 04
	{ "addn",               "vn",       &cmdAddN },             // 05
	{ "addv",               "vv",       &cmdAddV },             // 06
	{ "subn",               "vn",       &cmdSubN },             // 07
	{ "subv",               "vv",       &cmdSubV },             // 08
	{ "load.view",          "n",        &cmdLoadView },         // 09
	{ "animate.obj",        "n",        &cmdAnimateObj },       // 0A
	{ "new.room",           "n",        &cmdNewRoom },          // 0B
	{ "draw.pic",           "v",        &cmdDrawPicV1 },        // 0C
	{ "print",              "s",        &cmdPrint },            // 0D TODO
	{ "status",             "",         &cmdStatus },           // 0E TODO
	{ "save.game",          "",         &cmdSaveGame },         // 0F TODO
	{ "restore.game",       "",         &cmdLoadGame },         // 10 TODO
	{ "init.disk",          "",         &cmdInitDisk },         // 11 TODO
	{ "restart.game",       "",         &cmdRestartGame },      // 12 TODO
	{ "random",             "v",        &cmdRandomV1 },         // 13
	{ "get",                "n",        &cmdGetV1 },            // 14
	{ "drop",               "n",        &cmdDrop },             // 15
	{ "draw",               "n",        &cmdDraw },             // 16 TODO
	{ "erase",              "n",        &cmdErase },            // 17 TODO
	{ "position",           "nnn",      &cmdPositionV1 },       // 18
	{ "position.v",         "nvv",      &cmdPositionFV1 },      // 19
	{ "get.posn",           "nvv",      &cmdGetPosn },          // 1A
	{ "set.cel",            "nn",       &cmdSetCel },           // 1B
	{ "set.loop",           "nn",       &cmdSetLoop },          // 1C
	{ "end.of.loop",        "nn",       &cmdEndOfLoopV1 },      // 1D
	{ "reverse.loop",       "nn",       &cmdReverseLoopV1 },    // 1E
	{ "move.obj",           "nnnnn",    &cmdMoveObj },          // 1F
	{ "set.view",           "nn",       &cmdSetView },          // 20
	{ "follow.ego",         "nnn",      &cmdFollowEgo },        // 21
	{ "block",              "",         &cmdBlock },            // 22
	{ "unblock",            "",         &cmdUnblock },          // 23
	{ "ignore.blocks",      "n",        &cmdIgnoreBlocks },     // 24
	{ "observe.blocks",     "n",        &cmdObserveBlocks },    // 25
	{ "wander",             "n",        &cmdWander },           // 26
	{ "reposition",         "nvv",      &cmdRepositionV1 },     // 27
	{ "stop.motion",        "n",        &cmdStopMotionV1 },     // 28
	{ "start.motion",       "n",        &cmdStartMotionV1 },    // 29
	{ "stop.cycling",       "n",        &cmdStopCycling },      // 2A
	{ "start.cycling",      "n",        &cmdStartCycling },     // 2B
	{ "stop.update",        "n",        &cmdStopUpdate },       // 2C
	{ "start.update",       "n",        &cmdStartUpdate },      // 2D
	{ "program.control",    "",         &cmdProgramControl },   // 2E
	{ "player.control",     "",         &cmdPlayerControl },    // 2F
	{ "set.priority",       "nn",       &cmdSetPriority },      // 30
	{ "release.priority",   "n",        &cmdReleasePriority },  // 31
	{ "add.to.pic",         "nnnnnn",   &cmdAddToPicV1 },       // 32
	{ "set.horizon",        "n",        &cmdSetHorizon },       // 33
	{ "ignore.horizon",     "n",        &cmdIgnoreHorizon },    // 34
	{ "observe.horizon",    "n",        &cmdObserveHorizon },   // 35
	{ "load.logics",        "n",        &cmdLoadLogic },        // 36 TODO
	{ "object.on.water",    "n",        &cmdObjectOnWater },    // 37
	{ "load.pic",           "v",        &cmdLoadPicV1 },        // 38
	{ "load.sound",         "n",        &cmdLoadSound },        // 39
	{ "sound",              "nn",       &cmdSound },            // 3A
	{ "stop.sound",         "",         &cmdStopSound },        // 3B
	{ "set.v",              "v",        &cmdSetV },             // 3C
	{ "reset.v",            "v",        &cmdResetV },           // 3D
	{ "toggle.v",           "v",        &cmdToggleV },          // 3E
	{ "new.room.v",         "v",        &cmdNewRoom },          // 3F
	{ "call",               "n",        &cmdCallV1 },           // 40 TODO
	{ "quit",               "",         &cmdQuitV1 },           // 41
	{ "set.speed",          "v",        &cmdSetSpeed },         // 42
	{ "move.obj.v",         "nvvvv",    &cmdMoveObjF },         // 43
	{ "...",                "nn",       &cmdUnknown },          // 44
	{ "get.v",              "v",        &cmdUnknown },          // 45
	{ "assign.v",           "vv",       &cmdUnknown },          // 46
	{ "...",                "n",        &cmdUnknown },          // 47 # printvar.v
	{ "get.priority",       "nv",       &cmdGetPriority },      // 48
	{ "ignore.objs",        "n",        &cmdIgnoreObjs },       // 49
	{ "observe.objs",       "n",        &cmdObserveObjs },      // 4A
	{ "distance",           "nnv",      &cmdDistance },         // 4B
	{ "object.on.land",     "n",        &cmdObjectOnLand },     // 4C
	{ "...",                "nv",       &cmdUnknown },          // 4D # set.priority.f
	{ "...",                "",         &cmdUnknown },          // 4E  # show.obj
	{ "load.logics",        "n",        &cmdLoadLogic },        // 4F # load.global.logics
	{ "display",            "nnns",     &cmdDisplay },          // 50 TODO: 4 vs 3 args
	{ "prevent.input???",   "",         &cmdUnknown },          // 51
	{ "...",                "",         &cmdUnknown },          // 52 # nop
	{ "...",                "n",        &cmdUnknown },          // 53 # text.screen
	{ "...",                "",         &cmdUnknown },          // 54 ???
	{ "stop.motion",        "",         &cmdStopMotion },       // 55 or force.update??
	{ "discard.view",       "n",        &cmdDiscardView },      // 56
	{ "discard.pic",        "v",        &cmdDiscardPic },       // 57
	{ "set.item.view",      "nn",       &cmdSetItemView },      // 58
	{ "...",                "",         &cmdUnknown },          // 59 # reverse.cycle
	{ "last.cel",           "nv",       &cmdLastCel },          // 5A
	{ "set.cel.v",          "nv",       &cmdSetCelF },          // 5B
	{ "...",                "",         &cmdUnknown },          // 5C # normal.cycle
	{ "load.view",          "n",        &cmdLoadView },         // 5D
	{ "...",                "",         &cmdUnknown },          // 5E
	{ "...",                "",         &cmdUnknown },          // 5F
	{ "setbit",             "nv",       &cmdUnknown },          // 60
	{ "...",                "nv",       &cmdUnknown },          // 61 # clearbit
};

AgiInstruction insV2Test[] = {
	{ "",                   "",         &condUnknown },         // 00
	{ "equaln",             "vn",       &condEqual },           // 01
	{ "equalv",             "vv",       &condEqualV },          // 02
	{ "lessn",              "vn",       &condLess },            // 03
	{ "lessv",              "vv",       &condLessV },           // 04
	{ "greatern",           "vn",       &condGreater },         // 05
	{ "greaterv",           "vv",       &condGreaterV },        // 06
	{ "isset",              "n",        &condIsSet },           // 07
	{ "issetv",             "v",        &condIsSetV },          // 08
	{ "has",                "n",        &condHas },             // 09
	{ "obj.in.room",        "nv",       &condObjInRoom},        // 0A
	{ "posn",               "nnnnn",    &condPosn },            // 0B
	{ "controller",         "n",        &condController },      // 0C
	{ "have.key",           "",         &condHaveKey},          // 0D
	{ "said",               "",         &condSaid },            // 0E
	{ "compare.strings",    "ss",       &condCompareStrings },  // 0F
	{ "obj.in.box",         "nnnnn",    &condObjInBox },        // 10
	{ "center.posn",        "nnnnn",    &condCenterPosn },      // 11
	{ "right.posn",         "nnnnn",    &condRightPosn },       // 12
	{ "in.motion.using.mouse", "",      &condUnknown13 }        // 13
};

AgiInstruction insV2[] = {
	{ "return",             "",         NULL },                 // 00
	{ "increment",          "v",        &cmdIncrement },        // 01
	{ "decrement",          "v",        &cmdDecrement },        // 02
	{ "assignn",            "vn",       &cmdAssignN },          // 03
	{ "assignv",            "vv",       &cmdAssignV },          // 04
	{ "addn",               "vn",       &cmdAddN },             // 05
	{ "addv",               "vv",       &cmdAddV },             // 06
	{ "subn",               "vn",       &cmdSubN },             // 07
	{ "subv",               "vv",       &cmdSubV },             // 08
	{ "lindirectv",         "vv",       &cmdLindirectV },       // 09
	{ "lindirect",          "vv",       &cmdRindirect },        // 0A
	{ "lindirectn",         "vn",       &cmdLindirectN },       // 0B
	{ "set",                "n",        &cmdSet },              // 0C
	{ "reset",              "n",        &cmdReset },            // 0D
	{ "toggle",             "n",        &cmdToggle },           // 0E
	{ "set.v",              "v",        &cmdSetV },             // 0F
	{ "reset.v",            "v",        &cmdResetV },           // 10
	{ "toggle.v",           "v",        &cmdToggleV },          // 11
	{ "new.room",           "n",        &cmdNewRoom },          // 12
	{ "new.room.v",         "v",        &cmdNewRoomF },         // 13
	{ "load.logics",        "n",        &cmdLoadLogic },        // 14
	{ "load.logics.v",      "v",        &cmdLoadLogicF },       // 15
	{ "call",               "n",        &cmdCall },             // 16
	{ "call.v",             "v",        &cmdCallF },            // 17
	{ "load.pic",           "v",        &cmdLoadPic },          // 18
	{ "draw.pic",           "v",        &cmdDrawPic },          // 19
	{ "show.pic",           "",         &cmdShowPic },          // 1A
	{ "discard.pic",        "v",        &cmdDiscardPic },       // 1B
	{ "overlay.pic",        "v",        &cmdOverlayPic },       // 1C
	{ "show.pri.screen",    "",         &cmdShowPriScreen },    // 1D
	{ "load.view",          "n",        &cmdLoadView },         // 1E
	{ "load.view.v",        "v",        &cmdLoadViewF },        // 1F
	{ "discard.view",       "n",        &cmdDiscardView },      // 20
	{ "animate.obj",        "n",        &cmdAnimateObj },       // 21
	{ "unanimate.all",      "",         &cmdUnanimateAll },     // 22
	{ "draw",               "n",        &cmdDraw },             // 23
	{ "erase",              "n",        &cmdErase },            // 24
	{ "position",           "nnn",      &cmdPosition },         // 25
	{ "position.v",         "nvv",      &cmdPositionF },        // 26
	{ "get.posn",           "nvv",      &cmdGetPosn },          // 27
	{ "reposition",         "nvv",      &cmdReposition },       // 28
	{ "set.view",           "nn",       &cmdSetView },          // 29
	{ "set.view.v",         "nv",       &cmdSetViewF },         // 2A
	{ "set.loop",           "nn",       &cmdSetLoop },          // 2B
	{ "set.loop.v",         "nv",       &cmdSetLoopF },         // 2C
	{ "fix.loop",           "n",        &cmdFixLoop },          // 2D
	{ "release.loop",       "n",        &cmdReleaseLoop },      // 2E
	{ "set.cel",            "nn",       &cmdSetCel },           // 2F
	{ "set.cel.v",          "nv",       &cmdSetCelF },          // 30
	{ "last.cel",           "nv",       &cmdLastCel },          // 31
	{ "current.cel",        "nv",       &cmdCurrentCel },       // 32
	{ "current.loop",       "nv",       &cmdCurrentLoop },      // 33
	{ "current.view",       "nv",       &cmdCurrentView },      // 34
	{ "number.of.loops",    "nv",       &cmdNumberOfLoops },    // 35
	{ "set.priority",       "nn",       &cmdSetPriority },      // 36
	{ "set.priority.v",     "nv",       &cmdSetPriorityF },     // 37
	{ "release.priority",   "n",        &cmdReleasePriority },  // 38
	{ "get.priority",       "nn",       &cmdGetPriority },      // 39
	{ "stop.update",        "n",        &cmdStopUpdate },       // 3A
	{ "start.update",       "n",        &cmdStartUpdate },      // 3B
	{ "force.update",       "n",        &cmdForceUpdate },      // 3C
	{ "ignore.horizon",     "n",        &cmdIgnoreHorizon },    // 3D
	{ "observe.horizon",    "n",        &cmdObserveHorizon },   // 3E
	{ "set.horizon",        "n",        &cmdSetHorizon },       // 3F
	{ "object.on.water",    "n",        &cmdObjectOnWater },    // 40
	{ "object.on.land",     "n",        &cmdObjectOnLand },     // 41
	{ "object.on.anything", "n",        &cmdObjectOnAnything }, // 42
	{ "ignore.objs",        "n",        &cmdIgnoreObjs },       // 43
	{ "observe.objs",       "n",        &cmdObserveObjs },      // 44
	{ "distance",           "nnv",      &cmdDistance },         // 45
	{ "stop.cycling",       "n",        &cmdStopCycling },      // 46
	{ "start.cycling",      "n",        &cmdStartCycling },     // 47
	{ "normal.cycle",       "n",        &cmdNormalCycle },      // 48
	{ "end.of.loop",        "nn",       &cmdEndOfLoop },        // 49
	{ "reverse.cycle",      "n",        &cmdReverseCycle },     // 5A
	{ "reverse.loop",       "nn",       &cmdReverseLoop },      // 5B
	{ "cycle.time",         "nv",       &cmdCycleTime },        // 5C
	{ "stop.motion",        "n",        &cmdStopMotion },       // 5D
	{ "start.motion",       "n",        &cmdStartMotion },      // 5E
	{ "step.size",          "nv",       &cmdStepSize },         // 5F
	{ "step.time",          "nv",       &cmdStepTime },         // 60
	{ "move.obj",           "nnnnn",    &cmdMoveObj },          // 61
	{ "move.obj.v",         "nvvvv",    &cmdMoveObjF },         // 62
	{ "follow.ego",         "nnn",      &cmdFollowEgo },        // 63
	{ "wander",             "n",        &cmdWander },           // 64
	{ "normal.motion",      "n",        &cmdNormalMotion },     // 65
	{ "set.dir",            "nv",       &cmdSetDir },           // 66
	{ "get.dir",            "nv",       &cmdGetDir },           // 67
	{ "ignore.blocks",      "n",        &cmdIgnoreBlocks },     // 68
	{ "observe.blocks",     "n",        &cmdObserveBlocks },    // 69
	{ "block",              "nnnn",     &cmdBlock },            // 6A
	{ "unblock",            "",         &cmdUnblock },          // 6B
	{ "get",                "n",        &cmdGet },              // 6C
	{ "get.v",              "v",        &cmdGetF },             // 6D
	{ "drop",               "n",        &cmdDrop },             // 6E
	{ "put",                "nn",       &cmdPut },              // 6F
	{ "put.v",              "vv",       &cmdPutF },             // 70
	{ "get.room.v",         "vv",       &cmdGetRoomF },         // 71
	{ "load.sound",         "n",        &cmdLoadSound },        // 72
	{ "sound",              "nn",       &cmdSound },            // 73
	{ "stop.sound",         "",         &cmdStopSound },        // 74
	{ "print",              "s",        &cmdPrint },            // 75
	{ "print.v",            "v",        &cmdPrintF },           // 76
	{ "display",            "nns",      &cmdDisplay },          // 77
	{ "display.v",          "vvv",      &cmdDisplayF },         // 78
	{ "clear.lines",        "nns",      &cmdClearLines },       // 79
	{ "text.screen",        "",         &cmdTextScreen },       // 7A
	{ "graphics",           "",         &cmdGraphics },         // 7B
	{ "set.cursor.char",    "s",        &cmdSetCursorChar },    // 7C
	{ "set.text.attribute", "nn",       &cmdSetTextAttribute }, // 7D
	{ "shake.screen",       "n",        &cmdShakeScreen },      // 7E
	{ "configure.screen",   "nnn",      &cmdConfigureScreen },  // 7F
	{ "status.line.on",     "",         &cmdStatusLineOn },     // 80
	{ "status.line.off",    "",         &cmdStatusLineOff },    // 81
	{ "set.string",         "ns",       &cmdSetString },        // 82
	{ "get.string",         "nsnnn",    &cmdGetString },        // 83
	{ "word.to.string",     "nn",       &cmdWordToString },     // 84
	{ "parse",              "n",        &cmdParse },            // 85
	{ "get.num",            "nv",       &cmdGetNum },           // 86
	{ "prevent.input",      "",         &cmdPreventInput },     // 87
	{ "accept.input",       "",         &cmdAcceptInput },      // 88
	{ "set.key",            "nnn",      &cmdSetKey },           // 89
	{ "add.to.pic",         "nnnnnnn",  &cmdAddToPic },         // 8A
	{ "add.to.pic.v",       "vvvvvvv",  &cmdAddToPicF },        // 8B
	{ "status",             "",         &cmdStatus },           // 8C
	{ "save.game",          "",         &cmdSaveGame },         // 8D
	{ "restore.game",       "",         &cmdLoadGame },         // 8E
	{ "init.disk",          "",         &cmdInitDisk },         // 8F
	{ "restart.game",       "",         &cmdRestartGame },      // 90
	{ "show.obj",           "n",        &cmdShowObj },          // 91
	{ "random",             "nnv",      &cmdRandom },           // 92
	{ "program.control",    "",         &cmdProgramControl },   // 93
	{ "player.control",     "",         &cmdPlayerControl },    // 94
	{ "obj.status.v",       "v",        &cmdObjStatusF },       // 95
	{ "quit",               "n",        &cmdQuit },             // 96 0 args for AGI version 2.089
	{ "show.mem",           "",         &cmdShowMem },          // 97
	{ "pause",              "",         &cmdPause },            // 98
	{ "echo.line",          "",         &cmdEchoLine },         // 99
	{ "cancel.line",        "",         &cmdCancelLine },       // 9A
	{ "init.joy",           "",         &cmdInitJoy },          // 9B
	{ "toggle.monitor",     "",         &cmdToggleMonitor },    // 9C
	{ "version",            "",         &cmdVersion },          // 9D
	{ "script.size",        "n",        &cmdScriptSize },       // 9E
	{ "set.game.id",        "s",        &cmdSetGameID },        // 9F
	{ "log",                "s",        &cmdLog },              // A0
	{ "set.scan.start",     "",         &cmdSetScanStart },     // A1
	{ "reset.scan.start",   "",         &cmdResetScanStart },   // A2
	{ "reposition.to",      "nnn",      &cmdRepositionTo },     // A3
	{ "reposition.to.v",    "nvv",      &cmdRepositionToF },    // A4
	{ "trace.on",           "",         &cmdTraceOn },          // A5
	{ "trace.info",         "nnn",      &cmdTraceInfo },        // A6
	{ "print.at",           "snnn",     &cmdPrintAt }, // 3 args for AGI versions before 2.440
	{ "print.at.v",         "vnnn",     &cmdPrintAtV },         // A8
	{ "discard.view.v",     "v",        &cmdDiscardView},       // A9
	{ "clear.text.rect",    "nnnnn",    &cmdClearTextRect },    // AA
	{ "set.upper.left",     "nn",       &cmdSetUpperLeft },     // AB
	{ "set.menu",           "s",        &cmdSetMenu },          // AC
	{ "set.menu.item",      "sn",       &cmdSetMenuItem },      // AD
	{ "submit.menu",        "",         &cmdSubmitMenu },       // AE
	{ "enable.item",        "n",        &cmdEnableItem },       // AF
	{ "disable.item",       "n",        &cmdDisableItem },      // B0
	{ "menu.input",         "",         &cmdMenuInput },        // B1
	{ "show.obj.v",         "v",        &cmdShowObjV },         // B2
	{ "open.dialogue",      "",         &cmdOpenDialogue },     // B3
	{ "close.dialogue",     "",         &cmdCloseDialogue },    // B4
	{ "mul.n",              "vn",       &cmdMulN },             // B5
	{ "mul.v",              "vv",       &cmdMulV },             // B6
	{ "div.n",              "vn",       &cmdDivN },             // B7
	{ "div.v",              "vv",       &cmdDivV },             // B8
	{ "close.window",       "",         &cmdCloseWindow },      // B9
	{ "set.simple",         "n",        &cmdSetSimple },        // BA AGI2.425+, *BUT* not included in AGI2.440
	{ "push.script",        "",         &cmdPushScript },       // BB
	{ "pop.script",         "",         &cmdPopScript },        // BC
	{ "hold.key",           "",         &cmdHoldKey },          // BD
	{ "set.pri.base",       "n",        &cmdSetPriBase },       // BE AGI2.936+ *AND* also inside AGI2.425
	{ "discard.sound",      "n",        &cmdDiscardSound },     // BF was skip for PC
	{ "hide.mouse",         "",         &cmdHideMouse },        // C0 1 arg for AGI version 3.002.086 AGI3+ only starts here
	{ "allow.menu",         "n",        &cmdAllowMenu },        // C1
	{ "show.mouse",         "",         &cmdShowMouse },        // C2
	{ "fence.mouse",        "nnnn",     &cmdFenceMouse },       // C3
	{ "mouse.posn",         "vv",       &cmdMousePosn },        // C4
	{ "release.key",        "",         &cmdReleaseKey },       // C5 2 args for at least the Amiga GR (v2.05 1989-03-09) using AGI 2.316
	{ "adj.ego.move.to.xy", "",         &cmdAdjEgoMoveToXY }    // C6
};

void AgiEngine::setupOpcodes() {
	if (getVersion() >= 0x2000) {
		for (int i = 0; i < ARRAYSIZE(insV2Test); ++i)
			_agiCondCommands[i] = insV2Test[i].func;
		for (int i = 0; i < ARRAYSIZE(insV2); ++i)
			_agiCommands[i] = insV2[i].func;

		logicNamesTest = insV2Test;
		logicNamesCmd = insV2;

		// Alter opcode parameters for specific games
		// TODO: This could be either turned into a game feature, or a version
		// specific check, instead of a game version check

		// The Apple IIGS versions of MH1 and Goldrush both have a parameter for
		// show.mouse and hide.mouse. Fixes bugs #3577754 and #3426946.
		if ((getGameID() == GID_MH1 || getGameID() == GID_GOLDRUSH) &&
		        getPlatform() == Common::kPlatformApple2GS) {
			logicNamesCmd[176].args = "n";  // hide.mouse
			logicNamesCmd[178].args = "n";  // show.mouse
		}
	} else {
		for (int i = 0; i < ARRAYSIZE(insV1Test); ++i)
			_agiCondCommands[i] = insV1Test[i].func;
		for (int i = 0; i < ARRAYSIZE(insV1); ++i)
			_agiCommands[i] = insV1[i].func;

		logicNamesTest = insV1Test;
		logicNamesCmd = insV1;
	}
}

}
