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

#include "agi/agi.h"
#include "agi/opcodes.h"

namespace Agi {

// FIXME: The parameter strings in the opcode table have mistakes.
// Nothing depends on the values of the individual characters.
// Some are out of sync with how the opcode function interprets
// the parameter. Only the string lengths are used to indicate
// the parameter count for parsing.
// Consult the opcode functions for the real parameter types.

static const AgiOpCodeDefinitionEntry opCodesV1Cond[] = {
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

static const AgiOpCodeDefinitionEntry opCodesV1[] = {
	{ "return",             "",         nullptr },              // 00
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
	{ "follow.ego",         "nnv",      &cmdFollowEgo },        // 21
	{ "block",              "nnnn",     &cmdBlock },            // 22
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
	{ "get.num",            "nv",       &cmdUnknown },          // 44
	{ "get.v",              "v",        &cmdUnknown },          // 45
	{ "assign.v",           "vv",       &cmdUnknown },          // 46
	{ "printvar",           "v",        &cmdUnknown },          // 47 prints a variable (debug script)
	{ "get.priority",       "nv",       &cmdGetPriority },      // 48
	{ "ignore.objs",        "n",        &cmdIgnoreObjs },       // 49
	{ "observe.objs",       "n",        &cmdObserveObjs },      // 4A
	{ "distance",           "nnv",      &cmdDistance },         // 4B
	{ "object.on.land",     "n",        &cmdObjectOnLand },     // 4C
	{ "set.priority.v",     "nv",       &cmdUnknown },          // 4D # set.priority.v / set.priority.f
	{ "show.obj",           "n",        &cmdShowObj },          // 4E # show.obj (KQ2)
	{ "load.logics",        "n",        &cmdLoadLogic },        // 4F # load.global.logics
	{ "display",            "nnns",     &cmdDisplay },          // 50 TODO: 4 vs 3 args
	{ "prevent.input???",   "",         &cmdUnknown },          // 51
	{ "...",                "",         &cmdUnknown },          // 52 # nop
	{ "text.screen",        "n",        &cmdUnknown },          // 53
	{ "graphics",           "",         &cmdUnknown },          // 54
	{ "stop.motion",        "",         &cmdStopMotion },       // 55
	{ "discard.view",       "n",        &cmdDiscardView },      // 56
	{ "discard.pic",        "v",        &cmdDiscardPic },       // 57
	{ "set.item.view",      "nn",       &cmdSetItemView },      // 58
	{ "...",                "",         &cmdUnknown },          // 59 # reverse.cycle, unused in KQ2 or BC
	{ "last.cel",           "nv",       &cmdLastCel },          // 5A
	{ "set.cel.v",          "nv",       &cmdSetCelF },          // 5B
	{ "...",                "",         &cmdUnknown },          // 5C # normal.cycle, unused in KQ2 or BC
	{ "load.view",          "n",        &cmdLoadView },         // 5D
	{ "...",                "",         &cmdUnknown },          // 5E unused in KQ2 or BC
	{ "...",                "",         &cmdUnknown },          // 5F BC script 102 when attempting to fill flask
	{ "setbit",             "nv",       &cmdUnknown },          // 60
	{ "...",                "nv",       &cmdUnknown },          // 61 # clearbit, unused in KQ2 or BC
	{ "set.upper.left",     "nn",       &cmdSetUpperLeft }      // 62 BC Apple II
};

static const AgiOpCodeDefinitionEntry opCodesV2Cond[] = {
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

static const AgiOpCodeDefinitionEntry opCodesV2[] = {
	{ "return",             "",         nullptr },              // 00
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
	{ "reverse.cycle",      "n",        &cmdReverseCycle },     // 4A
	{ "reverse.loop",       "nn",       &cmdReverseLoop },      // 4B
	{ "cycle.time",         "nv",       &cmdCycleTime },        // 4C
	{ "stop.motion",        "n",        &cmdStopMotion },       // 4D
	{ "start.motion",       "n",        &cmdStartMotion },      // 4E
	{ "step.size",          "nv",       &cmdStepSize },         // 4F
	{ "step.time",          "nv",       &cmdStepTime },         // 50
	{ "move.obj",           "nnnnn",    &cmdMoveObj },          // 51
	{ "move.obj.v",         "nvvvv",    &cmdMoveObjF },         // 52
	{ "follow.ego",         "nnn",      &cmdFollowEgo },        // 53
	{ "wander",             "n",        &cmdWander },           // 54
	{ "normal.motion",      "n",        &cmdNormalMotion },     // 55
	{ "set.dir",            "nv",       &cmdSetDir },           // 56
	{ "get.dir",            "nv",       &cmdGetDir },           // 57
	{ "ignore.blocks",      "n",        &cmdIgnoreBlocks },     // 58
	{ "observe.blocks",     "n",        &cmdObserveBlocks },    // 59
	{ "block",              "nnnn",     &cmdBlock },            // 5A
	{ "unblock",            "",         &cmdUnblock },          // 5B
	{ "get",                "n",        &cmdGet },              // 5C
	{ "get.v",              "v",        &cmdGetF },             // 5D
	{ "drop",               "n",        &cmdDrop },             // 5E
	{ "put",                "nn",       &cmdPut },              // 5F
	{ "put.v",              "vv",       &cmdPutF },             // 60
	{ "get.room.v",         "vv",       &cmdGetRoomF },         // 61
	{ "load.sound",         "n",        &cmdLoadSound },        // 62
	{ "sound",              "nn",       &cmdSound },            // 63
	{ "stop.sound",         "",         &cmdStopSound },        // 64
	{ "print",              "s",        &cmdPrint },            // 65
	{ "print.v",            "v",        &cmdPrintF },           // 66
	{ "display",            "nns",      &cmdDisplay },          // 67
	{ "display.v",          "vvv",      &cmdDisplayF },         // 68
	{ "clear.lines",        "nns",      &cmdClearLines },       // 69
	{ "text.screen",        "",         &cmdTextScreen },       // 6A
	{ "graphics",           "",         &cmdGraphics },         // 6B
	{ "set.cursor.char",    "s",        &cmdSetCursorChar },    // 6C
	{ "set.text.attribute", "nn",       &cmdSetTextAttribute }, // 6D
	{ "shake.screen",       "n",        &cmdShakeScreen },      // 6E
	{ "configure.screen",   "nnn",      &cmdConfigureScreen },  // 6F
	{ "status.line.on",     "",         &cmdStatusLineOn },     // 70
	{ "status.line.off",    "",         &cmdStatusLineOff },    // 71
	{ "set.string",         "ns",       &cmdSetString },        // 72
	{ "get.string",         "nsnnn",    &cmdGetString },        // 73
	{ "word.to.string",     "nn",       &cmdWordToString },     // 74
	{ "parse",              "n",        &cmdParse },            // 75
	{ "get.num",            "nv",       &cmdGetNum },           // 76
	{ "prevent.input",      "",         &cmdPreventInput },     // 77
	{ "accept.input",       "",         &cmdAcceptInput },      // 78
	{ "set.key",            "nnn",      &cmdSetKey },           // 79
	{ "add.to.pic",         "nnnnnnn",  &cmdAddToPic },         // 7A
	{ "add.to.pic.v",       "vvvvvvv",  &cmdAddToPicF },        // 7B
	{ "status",             "",         &cmdStatus },           // 7C
	{ "save.game",          "",         &cmdSaveGame },         // 7D
	{ "restore.game",       "",         &cmdLoadGame },         // 7E
	{ "init.disk",          "",         &cmdInitDisk },         // 7F
	{ "restart.game",       "",         &cmdRestartGame },      // 80
	{ "show.obj",           "n",        &cmdShowObj },          // 81
	{ "random",             "nnv",      &cmdRandom },           // 82
	{ "program.control",    "",         &cmdProgramControl },   // 83
	{ "player.control",     "",         &cmdPlayerControl },    // 84
	{ "obj.status.v",       "v",        &cmdObjStatusF },       // 85
	{ "quit",               "n",        &cmdQuit },             // 86 0 args for AGI version 2.089
	{ "show.mem",           "",         &cmdShowMem },          // 87
	{ "pause",              "",         &cmdPause },            // 88
	{ "echo.line",          "",         &cmdEchoLine },         // 89
	{ "cancel.line",        "",         &cmdCancelLine },       // 8A
	{ "init.joy",           "",         &cmdInitJoy },          // 8B
	{ "toggle.monitor",     "",         &cmdToggleMonitor },    // 8C
	{ "version",            "",         &cmdVersion },          // 8D
	{ "script.size",        "n",        &cmdScriptSize },       // 8E
	{ "set.game.id",        "s",        &cmdSetGameID },        // 8F
	{ "log",                "s",        &cmdLog },              // 90
	{ "set.scan.start",     "",         &cmdSetScanStart },     // 91
	{ "reset.scan.start",   "",         &cmdResetScanStart },   // 92
	{ "reposition.to",      "nnn",      &cmdRepositionTo },     // 93
	{ "reposition.to.v",    "nvv",      &cmdRepositionToF },    // 94
	{ "trace.on",           "",         &cmdTraceOn },          // 95
	{ "trace.info",         "nnn",      &cmdTraceInfo },        // 96
	{ "print.at",           "snnn",     &cmdPrintAt }, // 3 args for AGI versions before 2.089
	{ "print.at.v",         "vnnn",     &cmdPrintAtV },         // 98
	{ "discard.view.v",     "v",        &cmdDiscardView},       // 99
	{ "clear.text.rect",    "nnnnn",    &cmdClearTextRect },    // 9A
	{ "set.upper.left",     "nn",       &cmdSetUpperLeft },     // 9B Apple II
	{ "set.menu",           "s",        &cmdSetMenu },          // 9C
	{ "set.menu.item",      "sn",       &cmdSetMenuItem },      // 9D
	{ "submit.menu",        "",         &cmdSubmitMenu },       // 9E
	{ "enable.item",        "n",        &cmdEnableItem },       // 9F
	{ "disable.item",       "n",        &cmdDisableItem },      // A0
	{ "menu.input",         "",         &cmdMenuInput },        // A1
	{ "show.obj.v",         "v",        &cmdShowObjV },         // A2
	{ "open.dialogue",      "",         &cmdOpenDialogue },     // A3
	{ "close.dialogue",     "",         &cmdCloseDialogue },    // A4
	{ "mul.n",              "vn",       &cmdMulN },             // A5
	{ "mul.v",              "vv",       &cmdMulV },             // A6
	{ "div.n",              "vn",       &cmdDivN },             // A7
	{ "div.v",              "vv",       &cmdDivV },             // A8
	{ "close.window",       "",         &cmdCloseWindow },      // A9
	{ "set.simple",         "n",        &cmdSetSimple },        // AA AGI2.425+, *BUT* not included in AGI2.440
	{ "push.script",        "",         &cmdPushScript },       // AB
	{ "pop.script",         "",         &cmdPopScript },        // AC
	{ "hold.key",           "",         &cmdHoldKey },          // AD
	{ "set.pri.base",       "n",        &cmdSetPriBase },       // AE AGI2.936+ *AND* also inside AGI2.425
	{ "discard.sound",      "n",        &cmdDiscardSound },     // AF was skip for PC
	{ "hide.mouse",         "",         &cmdHideMouse },        // B0 1 arg for AGI3 Apple IIGS and AGI 3.002.086. AGI3+ only starts here
	{ "allow.menu",         "n",        &cmdAllowMenu },        // B1
	{ "show.mouse",         "",         &cmdShowMouse },        // B2 1 arg for AGI3 Apple IIGS
	{ "fence.mouse",        "nnnn",     &cmdFenceMouse },       // B3
	{ "get.mse.posn",       "vv",       &cmdGetMousePosn },     // B4
	{ "release.key",        "",         &cmdReleaseKey },       // B5
	{ "adj.ego.move.to.x.y","",         &cmdAdjEgoMoveToXY }    // B6 2 args for Amiga/Atari ST GR, MH1, MH2
};

//
// Currently, there is no known difference between v3.002.098 -> v3.002.149
// So version emulated;
// 0x0086,
// 0x0149
//

void AgiEngine::setupOpCodes(uint16 version) {
	const AgiOpCodeDefinitionEntry *opCodesTable = nullptr;
	const AgiOpCodeDefinitionEntry *opCodesCondTable = nullptr;
	uint16 opCodesTableSize = 0;
	uint16 opCodesCondTableSize = 0;

	debug(0, "Setting up for version 0x%04X", version);

	if (version >= 0x2000) {
		opCodesTable = opCodesV2;
		opCodesCondTable = opCodesV2Cond;
		opCodesTableSize = ARRAYSIZE(opCodesV2);
		opCodesCondTableSize = ARRAYSIZE(opCodesV2Cond);
	} else {
		opCodesTable = opCodesV1;
		opCodesCondTable = opCodesV1Cond;
		opCodesTableSize = ARRAYSIZE(opCodesV1);
		opCodesCondTableSize = ARRAYSIZE(opCodesV1Cond);
	}

	// copy data over
	for (int opCodeNr = 0; opCodeNr < opCodesTableSize; opCodeNr++) {
		_opCodes[opCodeNr].name = opCodesTable[opCodeNr].name;
		_opCodes[opCodeNr].parameters = opCodesTable[opCodeNr].parameters;
		_opCodes[opCodeNr].functionPtr = opCodesTable[opCodeNr].functionPtr;
	}

	for (int opCodeNr = 0; opCodeNr < opCodesCondTableSize; opCodeNr++) {
		_opCodesCond[opCodeNr].name = opCodesCondTable[opCodeNr].name;
		_opCodesCond[opCodeNr].parameters = opCodesCondTable[opCodeNr].parameters;
		_opCodesCond[opCodeNr].functionPtr = opCodesCondTable[opCodeNr].functionPtr;
	}

	// Alter opcode parameters for specific games
	if ((version >= 0x2000) && (version < 0x3000)) {
		// AGI2 adjustments

		// 'quit' takes 0 args for 2.089
		if (version == 0x2089)
			_opCodes[0x86].parameters = "";

		// 'print.at' and 'print.at.v' take three parameters before 2.089.
		// This is documented in the specs as only < 2.440, but SQ1 1.0X (2.089)
		// and KQ3 (2.272) take four. Bug #10872. No game scripts have been
		// discovered that call either opcode with only three parameters.
		if (version < 0x2089) {
			_opCodes[0x97].parameters = "vvv";
			_opCodes[0x98].parameters = "vvv";
		}

		// TODO: Opcode B0 is used by SQ2 Apple IIgs, but its purpose is
		// currently unknown. It takes one parameter, and that parameter
 		// appears to be a variable number. It is not hide.mouse from AGI3.
		// No other AGI2 games have been discovered that call this opcode.
		// Logic 1: during the spaceship cutscene in the intro, called with 53
		// Logic 23: called twice with 39.
		_opCodes[0xb0].name = "unknown";
		_opCodes[0xb0].parameters = "v";
		_opCodes[0xb0].functionPtr = &cmdUnknown;
	}

	if (version >= 0x3000) {
		// AGI3 adjustments

		// hide.mouse and hide.key take 1 parameter for 3.002.086.
		// KQ4 is the only known game with this interpreter and
		// its scripts do not call either opcode. no game scripts
		// have been discovered that call hold.key with 1 parameter.
		if (version == 0x3086) {
			_opCodes[0xb0].parameters = "n"; // hide.mouse
			_opCodes[0xad].parameters = "n"; // hold.key
		}

		// hide.mouse and show.mouse take 1 parameter on Apple IIGS.
		// Used by Black Cauldron, Gold Rush, King's Quest IV, and Manhunter 1.
		// Fixes bugs #6161 and #5885.
		if (getPlatform() == Common::kPlatformApple2GS) {
			_opCodes[0xb0].parameters = "n";  // hide.mouse
			_opCodes[0xb2].parameters = "n";  // show.mouse
		}

		// adj.ego.move.to.x.y takes two parameters for Amiga/Atari ST
		// versions of Gold Rush, Manhunter 1, and Manhunter 2.
		// No scripts have been discovered that call adj.ego.move.to.x.y
		// with zero parameters.
		if ((getGameID() == GID_GOLDRUSH ||
			 getGameID() == GID_MH1 ||
			 getGameID() == GID_MH2) &&
			(getPlatform() == Common::kPlatformAmiga ||
			 getPlatform() == Common::kPlatformAtariST)) {
			_opCodes[0xb6].parameters = "vv";
		}
	}

	// AGIMOUSE games use a modified push.script that updates mouse state
	if (getFeatures() & GF_AGIMOUSE) {
		_opCodes[0xab].functionPtr = &cmdAgiMousePushScript;
	}

	// add invalid entries for every opcode, that is not defined at all
	for (int opCodeNr = opCodesTableSize; opCodeNr < ARRAYSIZE(_opCodes); opCodeNr++) {
		_opCodes[opCodeNr].name = "illegal";
		_opCodes[opCodeNr].parameters = "";
		_opCodes[opCodeNr].functionPtr = nullptr;
		_opCodes[opCodeNr].parameterSize = 0;
	}

	for (int opCodeNr = opCodesCondTableSize; opCodeNr < ARRAYSIZE(_opCodesCond); opCodeNr++) {
		_opCodesCond[opCodeNr].name = "illegal";
		_opCodesCond[opCodeNr].parameters = "";
		_opCodesCond[opCodeNr].functionPtr = nullptr;
		_opCodesCond[opCodeNr].parameterSize = 0;
	}

	// calculate parameter size
	for (int opCodeNr = 0; opCodeNr < opCodesTableSize; opCodeNr++) {
		_opCodes[opCodeNr].parameterSize = strlen( _opCodes[opCodeNr].parameters);
	}

	for (int opCodeNr = 0; opCodeNr < opCodesCondTableSize; opCodeNr++) {
		_opCodesCond[opCodeNr].parameterSize = strlen( _opCodesCond[opCodeNr].parameters);
	}
}

} // End of namespace Agi
