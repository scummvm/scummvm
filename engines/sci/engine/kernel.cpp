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

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/event.h"
#include "sci/resource.h"
#include "sci/engine/state.h"

#include "common/system.h"

namespace Sci {

/** Default kernel name table. */
static const char *s_defaultKernelNames[] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Clone",
	/*0x05*/ "DisposeClone",
	/*0x06*/ "IsObject",
	/*0x07*/ "RespondsTo",
	/*0x08*/ "DrawPic",
	/*0x09*/ "Dummy",	// Show
	/*0x0a*/ "PicNotValid",
	/*0x0b*/ "Animate",
	/*0x0c*/ "SetNowSeen",
	/*0x0d*/ "NumLoops",
	/*0x0e*/ "NumCels",
	/*0x0f*/ "CelWide",
	/*0x10*/ "CelHigh",
	/*0x11*/ "DrawCel",
	/*0x12*/ "AddToPic",
	/*0x13*/ "NewWindow",
	/*0x14*/ "GetPort",
	/*0x15*/ "SetPort",
	/*0x16*/ "DisposeWindow",
	/*0x17*/ "DrawControl",
	/*0x18*/ "HiliteControl",
	/*0x19*/ "EditControl",
	/*0x1a*/ "TextSize",
	/*0x1b*/ "Display",
	/*0x1c*/ "GetEvent",
	/*0x1d*/ "GlobalToLocal",
	/*0x1e*/ "LocalToGlobal",
	/*0x1f*/ "MapKeyToDir",
	/*0x20*/ "DrawMenuBar",
	/*0x21*/ "MenuSelect",
	/*0x22*/ "AddMenu",
	/*0x23*/ "DrawStatus",
	/*0x24*/ "Parse",
	/*0x25*/ "Said",
	/*0x26*/ "SetSynonyms",	// Portrait (KQ6 hires)
	/*0x27*/ "HaveMouse",
	/*0x28*/ "SetCursor",
	// FOpen (SCI0)
	// FPuts (SCI0)
	// FGets (SCI0)
	// FClose (SCI0)
	/*0x29*/ "SaveGame",
	/*0x2a*/ "RestoreGame",
	/*0x2b*/ "RestartGame",
	/*0x2c*/ "GameIsRestarting",
	/*0x2d*/ "DoSound",
	/*0x2e*/ "NewList",
	/*0x2f*/ "DisposeList",
	/*0x30*/ "NewNode",
	/*0x31*/ "FirstNode",
	/*0x32*/ "LastNode",
	/*0x33*/ "EmptyList",
	/*0x34*/ "NextNode",
	/*0x35*/ "PrevNode",
	/*0x36*/ "NodeValue",
	/*0x37*/ "AddAfter",
	/*0x38*/ "AddToFront",
	/*0x39*/ "AddToEnd",
	/*0x3a*/ "FindKey",
	/*0x3b*/ "DeleteKey",
	/*0x3c*/ "Random",
	/*0x3d*/ "Abs",
	/*0x3e*/ "Sqrt",
	/*0x3f*/ "GetAngle",
	/*0x40*/ "GetDistance",
	/*0x41*/ "Wait",
	/*0x42*/ "GetTime",
	/*0x43*/ "StrEnd",
	/*0x44*/ "StrCat",
	/*0x45*/ "StrCmp",
	/*0x46*/ "StrLen",
	/*0x47*/ "StrCpy",
	/*0x48*/ "Format",
	/*0x49*/ "GetFarText",
	/*0x4a*/ "ReadNumber",
	/*0x4b*/ "BaseSetter",
	/*0x4c*/ "DirLoop",
	/*0x4d*/ "CanBeHere", // CantBeHere in newer SCI versions
	/*0x4e*/ "OnControl",
	/*0x4f*/ "InitBresen",
	/*0x50*/ "DoBresen",
	/*0x51*/ "Platform", // DoAvoider (SCI0)
	/*0x52*/ "SetJump",
	/*0x53*/ "SetDebug",
	/*0x54*/ "Dummy",    // InspectObj
	/*0x55*/ "Dummy",    // ShowSends
	/*0x56*/ "Dummy",    // ShowObjs
	/*0x57*/ "Dummy",    // ShowFree
	/*0x58*/ "MemoryInfo",
	/*0x59*/ "Dummy",    // StackUsage
	/*0x5a*/ "Dummy",    // Profiler
	/*0x5b*/ "GetMenu",
	/*0x5c*/ "SetMenu",
	/*0x5d*/ "GetSaveFiles",
	/*0x5e*/ "GetCWD",
	/*0x5f*/ "CheckFreeSpace",
	/*0x60*/ "ValidPath",
	/*0x61*/ "CoordPri",
	/*0x62*/ "StrAt",
	/*0x63*/ "DeviceInfo",
	/*0x64*/ "GetSaveDir",
	/*0x65*/ "CheckSaveGame",
	/*0x66*/ "ShakeScreen",
	/*0x67*/ "FlushResources",
	/*0x68*/ "SinMult",
	/*0x69*/ "CosMult",
	/*0x6a*/ "SinDiv",
	/*0x6b*/ "CosDiv",
	/*0x6c*/ "Graph",
	/*0x6d*/ "Joystick",
	// End of kernel function table for SCI0
	/*0x6e*/ "Dummy",	// ShiftScreen
	/*0x6f*/ "Palette",
	/*0x70*/ "MemorySegment",
	/*0x71*/ "Intersections",	// MoveCursor (SCI1 late), PalVary (SCI1.1)
	/*0x72*/ "Memory",
	/*0x73*/ "Dummy",	// ListOps
	/*0x74*/ "FileIO",
	/*0x75*/ "DoAudio",
	/*0x76*/ "DoSync",
	/*0x77*/ "AvoidPath",
	/*0x78*/ "Sort",	// StrSplit (SCI01)
	/*0x79*/ "Dummy",	// ATan
	/*0x7a*/ "Lock",
	/*0x7b*/ "StrSplit",
	/*0x7c*/ "GetMessage",	// Message (SCI1.1)
	/*0x7d*/ "IsItSkip",
	/*0x7e*/ "MergePoly",
	/*0x7f*/ "ResCheck",
	/*0x80*/ "AssertPalette",
	/*0x81*/ "TextColors",
	/*0x82*/ "TextFonts",
	/*0x83*/ "Dummy",	// Record
	/*0x84*/ "Dummy",	// PlayBack
	/*0x85*/ "ShowMovie",
	/*0x86*/ "SetVideoMode",
	/*0x87*/ "SetQuitStr",
	/*0x88*/ "Dummy"	// DbugStr
};

reg_t kStub(EngineState *s, int argc, reg_t *argv) {
	Kernel *kernel = g_sci->getKernel();
	int kernelCallNr = -1;

	Common::List<ExecStack>::iterator callIterator = s->_executionStack.end();
	if (callIterator != s->_executionStack.begin()) {
		callIterator--;
		ExecStack lastCall = *callIterator;
		kernelCallNr = lastCall.debugSelector;
	}

	Common::String warningMsg = "Dummy function k" + kernel->getKernelName(kernelCallNr) +
								Common::String::printf("[%x]", kernelCallNr) +
								" invoked. Params: " +
								Common::String::printf("%d", argc) + " (";

	for (int i = 0; i < argc; i++) {
		warningMsg +=  Common::String::printf("%04x:%04x", PRINT_REG(argv[i]));
		warningMsg += (i == argc - 1 ? ")" : ", ");
	}

	warning("%s", warningMsg.c_str());
	return s->r_acc;
}

reg_t kStubNull(EngineState *s, int argc, reg_t *argv) {
	kStub(s, argc, argv);
	return NULL_REG;
}

reg_t kDummy(EngineState *s, int argc, reg_t *argv) {
	kStub(s, argc, argv);
	error("Kernel function was called, which was considered to be unused - see log for details");
}

// [io] -> either integer or object
// (io) -> optionally integer AND an object
// (i) -> optional integer
// . -> any type
// i* -> optional multiple integers
// .* -> any parameters afterwards (or none)

//    gameID,           room,script,lvl,          object-name, method-name,    call, index,   replace
static const SciWorkaroundEntry kAbs_workarounds[] = {
    { GID_HOYLE1,          1,     1,  0,              "room1", "doit",           -1,    0, { 2, 0x3e9 } }, // crazy eights - called with objects instead of integers
    { GID_HOYLE1,          2,     2,  0,              "room2", "doit",           -1,    0, { 2, 0x3e9 } }, // old maid - called with objects instead of integers
    { GID_HOYLE1,          3,     3,  0,              "room3", "doit",           -1,    0, { 2, 0x3e9 } }, // hearts - called with objects instead of integers
    SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call, index,   replace
static const SciWorkaroundEntry kDisposeScript_workarounds[] = {
    { GID_QFG1,           64,    64,  0,               "rm64", "dispose",        -1,    0, { 1,    0 } }, // when leaving graveyard, parameter 0 is an object
    SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call, index,   replace
static const SciWorkaroundEntry kDoSoundFade_workarounds[] = {
    { GID_KQ1,            -1,   989,  0,          "gameSound", "fade",           -1,    0, { 1,    0 } }, // gets called in several scenes (e.g. graham cracker) with 0:0
    { GID_KQ6,           105,   989,  0,        "globalSound", "fade",           -1,    0, { 0,    0 } }, // floppy: during intro, parameter 4 is an object
    SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call, index,   replace
static const SciWorkaroundEntry kGraphRestoreBox_workarounds[] = {
    { GID_LSL6,           -1,    85,  0,          "rScroller", "hide",           -1,    0, { 0,    0 } }, // happens when restoring (sometimes), same as the one below
    { GID_LSL6,           -1,    85,  0,          "lScroller", "hide",           -1,    0, { 0,    0 } }, // happens when restoring (sometimes), same as the one below
    { GID_LSL6,           -1,    86,  0,             "LL6Inv", "show",           -1,    0, { 0,    0 } }, // happens when restoring, is called with hunk segment, but hunk is not allocated at that time
    // ^^ TODO: check, if this is really a script error or an issue with our restore code
    { GID_LSL6,           -1,    86,  0,             "LL6Inv", "hide",           -1,    0, { 0,    0 } }, // happens during the game, gets called with 1 extra parameter
    SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call, index,   replace
static const SciWorkaroundEntry kGraphFillBoxForeground_workarounds[] = {
    { GID_LSL6,           -1,     0,  0,               "LSL6", "hideControls",   -1,    0, { 0,    0 } }, // happens when giving the bungee key to merrily (room 240) and at least in room 650 too - gets called with additional 5th parameter
    SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call, index,   replace
static const SciWorkaroundEntry kGraphFillBoxAny_workarounds[] = {
    { GID_SQ4,            -1,   818,  0,     "iconTextSwitch", "show",           -1,    0, { 0,    0 } }, // game menu "text/speech" display - parameter 5 is missing, but the right color number is on the stack
    SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call, index,   replace
static const SciWorkaroundEntry kSetPort_workarounds[] = {
    { GID_LSL6,          740,   740,  0,              "rm740", "drawPic",        -1,    0, { 1,    0 } }, // ending scene, is called with additional 3 (!) parameters
    SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call, index,   replace
static const SciWorkaroundEntry kUnLoad_workarounds[] = {
    { GID_LSL6,          130,   130,  0,    "recruitLarryScr", "changeState",    -1,    0, { 1,    0 } }, // during intro, a 3rd parameter is passed by accident
    { GID_LSL6,          740,   740,  0,        "showCartoon", "changeState",    -1,    0, { 1,    0 } }, // during ending, 4 additional parameters are passed by accident
    { GID_SQ1,            43,   303,  0,            "slotGuy", "dispose",        -1,    0, { 1,    0 } }, // when leaving ulence flats bar, parameter 1 is not passed - script error
    SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    call, index,   replace
static const SciWorkaroundEntry kStrCpy_workarounds[] = {
    { GID_ISLANDBRAIN,   260,    45,  0,        "aWord", "addOn",                -1,    0, { 0,    0 } }, // Hominy Homonym puzzle
    SCI_WORKAROUNDENTRY_TERMINATOR
};

struct SciKernelMapSubEntry {
	SciVersion fromVersion;
	SciVersion toVersion;

	uint16 id;

	const char *name;
	KernelFunctionCall *function;

	const char *signature;
	const SciWorkaroundEntry *workarounds;
};

#define SCI_SUBOPENTRY_TERMINATOR { SCI_VERSION_NONE, SCI_VERSION_NONE, 0, NULL, NULL, NULL, NULL }


#define SIG_SCIALL         SCI_VERSION_NONE, SCI_VERSION_NONE
#define SIG_SCI0           SCI_VERSION_NONE, SCI_VERSION_01
#define SIG_SCI1           SCI_VERSION_1_EGA, SCI_VERSION_1_LATE
#define SIG_SCI11          SCI_VERSION_1_1, SCI_VERSION_1_1
#define SIG_SINCE_SCI11    SCI_VERSION_1_1, SCI_VERSION_NONE
#define SIG_SCI21          SCI_VERSION_2_1, SCI_VERSION_2_1

#define SIG_SCI16          SCI_VERSION_NONE, SCI_VERSION_1_1
#define SIG_SCI32          SCI_VERSION_2, SCI_VERSION_NONE

// SCI-Sound-Version
#define SIG_SOUNDSCI0      SCI_VERSION_0_EARLY, SCI_VERSION_0_LATE
#define SIG_SOUNDSCI1EARLY SCI_VERSION_1_EARLY, SCI_VERSION_1_EARLY
#define SIG_SOUNDSCI1LATE  SCI_VERSION_1_LATE, SCI_VERSION_1_LATE
#define SIG_SOUNDSCI21     SCI_VERSION_2_1, SCI_VERSION_2_1

#define SIGFOR_ALL   0x3f
#define SIGFOR_DOS   1 << 0
#define SIGFOR_PC98  1 << 1
#define SIGFOR_WIN   1 << 2
#define SIGFOR_MAC   1 << 3
#define SIGFOR_AMIGA 1 << 4
#define SIGFOR_ATARI 1 << 5
#define SIGFOR_PC    SIGFOR_DOS|SIGFOR_WIN

#define SIG_EVERYWHERE  SIG_SCIALL, SIGFOR_ALL

#define MAP_CALL(_name_) #_name_, k##_name_

//    version,         subId, function-mapping,                    signature,              workarounds
static const SciKernelMapSubEntry kDoSound_subops[] = {
    { SIG_SOUNDSCI0,       0, MAP_CALL(DoSoundInit),               "o",                    NULL },
    { SIG_SOUNDSCI0,       1, MAP_CALL(DoSoundPlay),               "o",                    NULL },
    { SIG_SOUNDSCI0,       2, MAP_CALL(DoSoundDummy),              "o",                    NULL },
    { SIG_SOUNDSCI0,       3, MAP_CALL(DoSoundDispose),            "o",                    NULL },
    { SIG_SOUNDSCI0,       4, MAP_CALL(DoSoundMute),               "(i)",                  NULL },
    { SIG_SOUNDSCI0,       5, MAP_CALL(DoSoundStop),               "o",                    NULL },
    { SIG_SOUNDSCI0,       6, MAP_CALL(DoSoundPause),              "i",                    NULL },
    { SIG_SOUNDSCI0,       7, MAP_CALL(DoSoundResumeAfterRestore), "",                     NULL },
    { SIG_SOUNDSCI0,       8, MAP_CALL(DoSoundMasterVolume),       "(i)",                  NULL },
    { SIG_SOUNDSCI0,       9, MAP_CALL(DoSoundUpdate),             "o",                    NULL },
    { SIG_SOUNDSCI0,      10, MAP_CALL(DoSoundFade),               "o",                    kDoSoundFade_workarounds },
    { SIG_SOUNDSCI0,      11, MAP_CALL(DoSoundGetPolyphony),       "",                     NULL },
    { SIG_SOUNDSCI0,      12, MAP_CALL(DoSoundStopAll),            "",                     NULL },
    { SIG_SOUNDSCI1EARLY,  0, MAP_CALL(DoSoundMasterVolume),       NULL,                   NULL },
    { SIG_SOUNDSCI1EARLY,  1, MAP_CALL(DoSoundMute),               NULL,                   NULL },
    { SIG_SOUNDSCI1EARLY,  2, MAP_CALL(DoSoundDummy),              NULL,                   NULL },
    { SIG_SOUNDSCI1EARLY,  3, MAP_CALL(DoSoundGetPolyphony),       NULL,                   NULL },
    { SIG_SOUNDSCI1EARLY,  4, MAP_CALL(DoSoundUpdate),             NULL,                   NULL },
    { SIG_SOUNDSCI1EARLY,  5, MAP_CALL(DoSoundInit),               NULL,                   NULL },
    { SIG_SOUNDSCI1EARLY,  6, MAP_CALL(DoSoundDispose),            NULL,                   NULL },
    { SIG_SOUNDSCI1EARLY,  7, MAP_CALL(DoSoundPlay),               "oi",                   NULL },
    // ^^ TODO: In SCI1-SCI1.1 DoSound (play) is called by 2 methods of the Sound object: play and
    //  playBed. The methods are the same, apart from the second integer parameter: it's 0 in
    //  play and 1 in playBed, to distinguish the caller. It's passed on, we should find out what
    //  it actually does internally
    { SIG_SOUNDSCI1EARLY,  8, MAP_CALL(DoSoundStop),               NULL,                   NULL },
    { SIG_SOUNDSCI1EARLY,  9, MAP_CALL(DoSoundPause),              "[o0]i",                NULL },
    { SIG_SOUNDSCI1EARLY, 10, MAP_CALL(DoSoundFade),               "oiiii",                NULL },
    { SIG_SOUNDSCI1EARLY, 11, MAP_CALL(DoSoundUpdateCues),         "o",                    NULL },
    { SIG_SOUNDSCI1EARLY, 12, MAP_CALL(DoSoundSendMidi),           "oiii",                 NULL },
    { SIG_SOUNDSCI1EARLY, 13, MAP_CALL(DoSoundReverb),             "i",                    NULL },
    { SIG_SOUNDSCI1EARLY, 14, MAP_CALL(DoSoundSetHold),            "oi",                   NULL },
    { SIG_SOUNDSCI1EARLY, 15, MAP_CALL(DoSoundDummy),              "",                     NULL },
    //  ^^ Longbow demo
    { SIG_SOUNDSCI1LATE,   0, MAP_CALL(DoSoundMasterVolume),       NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,   1, MAP_CALL(DoSoundMute),               NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,   2, MAP_CALL(DoSoundDummy),              "",                     NULL },
    { SIG_SOUNDSCI1LATE,   3, MAP_CALL(DoSoundGetPolyphony),       NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,   4, MAP_CALL(DoSoundGetAudioCapability), "",                     NULL },
    { SIG_SOUNDSCI1LATE,   5, MAP_CALL(DoSoundSuspend),            "i",                    NULL },
    { SIG_SOUNDSCI1LATE,   6, MAP_CALL(DoSoundInit),               NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,   7, MAP_CALL(DoSoundDispose),            NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,   8, MAP_CALL(DoSoundPlay),               NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,   9, MAP_CALL(DoSoundStop),               NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,  10, MAP_CALL(DoSoundPause),              NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,  11, MAP_CALL(DoSoundFade),               "oiiii(i)",             kDoSoundFade_workarounds },
    { SIG_SOUNDSCI1LATE,  12, MAP_CALL(DoSoundSetHold),            NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,  13, MAP_CALL(DoSoundDummy),              NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,  14, MAP_CALL(DoSoundSetVolume),          "oi",                   NULL },
    { SIG_SOUNDSCI1LATE,  15, MAP_CALL(DoSoundSetPriority),        "oi",                   NULL },
    { SIG_SOUNDSCI1LATE,  16, MAP_CALL(DoSoundSetLoop),            "oi",                   NULL },
    { SIG_SOUNDSCI1LATE,  17, MAP_CALL(DoSoundUpdateCues),         NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,  18, MAP_CALL(DoSoundSendMidi),           "oiii(i)",              NULL },
    { SIG_SOUNDSCI1LATE,  19, MAP_CALL(DoSoundReverb),             NULL,                   NULL },
    { SIG_SOUNDSCI1LATE,  20, MAP_CALL(DoSoundUpdate),             NULL,                   NULL },
#ifdef ENABLE_SCI32
    { SIG_SOUNDSCI21,      0, MAP_CALL(DoSoundMasterVolume),       NULL,                   NULL },
    { SIG_SOUNDSCI21,      1, MAP_CALL(DoSoundMute),               NULL,                   NULL },
    { SIG_SOUNDSCI21,      2, MAP_CALL(DoSoundDummy),              NULL,                   NULL },
    { SIG_SOUNDSCI21,      3, MAP_CALL(DoSoundGetPolyphony),       NULL,                   NULL },
    { SIG_SOUNDSCI21,      4, MAP_CALL(DoSoundGetAudioCapability), NULL,                   NULL },
    { SIG_SOUNDSCI21,      5, MAP_CALL(DoSoundSuspend),            NULL,                   NULL },
    { SIG_SOUNDSCI21,      6, MAP_CALL(DoSoundInit),               NULL,                   NULL },
    { SIG_SOUNDSCI21,      7, MAP_CALL(DoSoundDispose),            NULL,                   NULL },
    { SIG_SOUNDSCI21,      8, MAP_CALL(DoSoundPlay),               "o(i)",                 NULL },
    // ^^ TODO: if this is really the only change between SCI1LATE AND SCI21, we could rename the
    //     SIG_SOUNDSCI1LATE #define to SIG_SINCE_SOUNDSCI1LATE and make it being SCI1LATE+. Although
    //     I guess there are many more changes somewhere
    // TODO: Quest for Glory 4 (SCI2.1) uses the old scheme, we need to detect it accordingly
    //        signature for SCI21 should be "o"
    { SIG_SOUNDSCI21,      9, MAP_CALL(DoSoundStop),               NULL,                   NULL },
    { SIG_SOUNDSCI21,     10, MAP_CALL(DoSoundPause),              NULL,                   NULL },
    { SIG_SOUNDSCI21,     11, MAP_CALL(DoSoundFade),               NULL,                   NULL },
    { SIG_SOUNDSCI21,     12, MAP_CALL(DoSoundSetHold),            NULL,                   NULL },
    { SIG_SOUNDSCI21,     13, MAP_CALL(DoSoundDummy),              NULL,                   NULL },
    { SIG_SOUNDSCI21,     14, MAP_CALL(DoSoundSetVolume),          NULL,                   NULL },
    { SIG_SOUNDSCI21,     15, MAP_CALL(DoSoundSetPriority),        NULL,                   NULL },
    { SIG_SOUNDSCI21,     16, MAP_CALL(DoSoundSetLoop),            NULL,                   NULL },
    { SIG_SOUNDSCI21,     17, MAP_CALL(DoSoundUpdateCues),         NULL,                   NULL },
    { SIG_SOUNDSCI21,     18, MAP_CALL(DoSoundSendMidi),           NULL,                   NULL },
    { SIG_SOUNDSCI21,     19, MAP_CALL(DoSoundReverb),             NULL,                   NULL },
    { SIG_SOUNDSCI21,     20, MAP_CALL(DoSoundUpdate),             NULL,                   NULL },
#endif
    SCI_SUBOPENTRY_TERMINATOR
};

//    version,         subId, function-mapping,                    signature,              workarounds
static const SciKernelMapSubEntry kGraph_subops[] = {
    { SIG_SCI32,           1, MAP_CALL(StubNull),                  "",                     NULL }, // called by gk1 sci32 right at the start
    { SIG_SCIALL,          2, MAP_CALL(GraphGetColorCount),        "",                     NULL },
    // 3 - set palette via resource
    { SIG_SCIALL,          4, MAP_CALL(GraphDrawLine),             "iiiii(i)(i)",          NULL },
    // 5 - nop
    // 6 - draw pattern
    { SIG_SCIALL,          7, MAP_CALL(GraphSaveBox),              "iiiii",                NULL },
    { SIG_SCIALL,          8, MAP_CALL(GraphRestoreBox),           "[r0!]",                kGraphRestoreBox_workarounds },
    // ^ this may get called with invalid references, we check them within restoreBits() and sierra sci behaves the same
    { SIG_SCIALL,          9, MAP_CALL(GraphFillBoxBackground),    "iiii",                 NULL },
    { SIG_SCIALL,         10, MAP_CALL(GraphFillBoxForeground),    "iiii",                 kGraphFillBoxForeground_workarounds },
    { SIG_SCIALL,         11, MAP_CALL(GraphFillBoxAny),           "iiiiii(i)(i)",         kGraphFillBoxAny_workarounds },
    { SIG_SCI11,          12, MAP_CALL(GraphUpdateBox),            "iiii(i)(r0)",          NULL }, // kq6 hires
    { SIG_SCIALL,         12, MAP_CALL(GraphUpdateBox),            "iiii(i)",              NULL },
    { SIG_SCIALL,         13, MAP_CALL(GraphRedrawBox),            "iiii",                 NULL },
    { SIG_SCIALL,         14, MAP_CALL(GraphAdjustPriority),       "ii",                   NULL },
    { SIG_SCI11,          15, MAP_CALL(GraphSaveUpscaledHiresBox), "iiii",                 NULL }, // kq6 hires
    SCI_SUBOPENTRY_TERMINATOR
};

//    version,         subId, function-mapping,                    signature,              workarounds
static const SciKernelMapSubEntry kPalVary_subops[] = {
    { SIG_SCIALL,          0, MAP_CALL(PalVaryInit),               "ii(i)(i)",             NULL },
    { SIG_SCIALL,          1, MAP_CALL(PalVaryReverse),            "(i)(i)(i)",            NULL },
    { SIG_SCIALL,          2, MAP_CALL(PalVaryGetCurrentStep),     "",                     NULL },
    { SIG_SCIALL,          3, MAP_CALL(PalVaryDeinit),             "",                     NULL },
    { SIG_SCIALL,          4, MAP_CALL(PalVaryChangeTarget),       "i",                    NULL },
    { SIG_SCIALL,          5, MAP_CALL(PalVaryChangeTicks),        "i",                    NULL },
    { SIG_SCIALL,          6, MAP_CALL(PalVaryPauseResume),        "i",                    NULL },
    { SIG_SCI32,           8, MAP_CALL(PalVaryUnknown),            "",                     NULL },
    SCI_SUBOPENTRY_TERMINATOR
};

//    version,         subId, function-mapping,                    signature,              workarounds
static const SciKernelMapSubEntry kPalette_subops[] = {
    { SIG_SCIALL,          1, MAP_CALL(PaletteSetFromResource),    "i(i)",                 NULL },
    { SIG_SCIALL,          2, MAP_CALL(PaletteSetFlag),            "iii",                  NULL },
    { SIG_SCIALL,          3, MAP_CALL(PaletteUnsetFlag),          "iii",                  NULL },
    { SIG_SCIALL,          4, MAP_CALL(PaletteSetIntensity),       "iii(i)",               NULL },
    { SIG_SCIALL,          5, MAP_CALL(PaletteFindColor),          "iii",                  NULL },
    { SIG_SCIALL,          6, MAP_CALL(PaletteAnimate),            "i*",                   NULL },
    { SIG_SCIALL,          7, MAP_CALL(PaletteSave),               "",                     NULL },
    { SIG_SCIALL,          8, MAP_CALL(PaletteRestore),            "i",                    NULL },
    SCI_SUBOPENTRY_TERMINATOR
};

static const SciKernelMapSubEntry kFileIO_subops[] = {
    { SIG_SCI32,           0, MAP_CALL(FileIOOpen),                "r(i)",                 NULL },
    { SIG_SCIALL,          0, MAP_CALL(FileIOOpen),                "ri",                   NULL },
    { SIG_SCIALL,          1, MAP_CALL(FileIOClose),               "i",                    NULL },
    { SIG_SCIALL,          2, MAP_CALL(FileIOReadRaw),             "iri",                  NULL },
    { SIG_SCIALL,          3, MAP_CALL(FileIOWriteRaw),            "iri",                  NULL },
    { SIG_SCIALL,          4, MAP_CALL(FileIOUnlink),              "r",                    NULL },
    { SIG_SCIALL,          5, MAP_CALL(FileIOReadString),          "rii",                  NULL },
    { SIG_SCIALL,          6, MAP_CALL(FileIOWriteString),         "ir",                   NULL },
    { SIG_SCIALL,          7, MAP_CALL(FileIOSeek),                "iii",                  NULL },
    { SIG_SCIALL,          8, MAP_CALL(FileIOFindFirst),           "rri",                  NULL },
    { SIG_SCIALL,          9, MAP_CALL(FileIOFindNext),            "r",                    NULL },
    { SIG_SCIALL,         10, MAP_CALL(FileIOExists),              "r",                    NULL },
    { SIG_SINCE_SCI11,    11, MAP_CALL(FileIORename),              "rr",                   NULL },
#ifdef ENABLE_SCI32
    { SIG_SCI32,          13, MAP_CALL(FileIOReadByte),            "i",                    NULL },
    { SIG_SCI32,          14, MAP_CALL(FileIOWriteByte),           "ii",                   NULL },
    { SIG_SCI32,          15, MAP_CALL(FileIOReadWord),            "i",                    NULL },
    { SIG_SCI32,          16, MAP_CALL(FileIOWriteWord),           "ii",                   NULL },
    { SIG_SCI32,          19, MAP_CALL(Stub),                      "",                     NULL }, // for Torin demo
#endif
    SCI_SUBOPENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
//    version,         subId, function-mapping,                    signature,              workarounds
static const SciKernelMapSubEntry kList_subops[] = {
    { SIG_SCI21,           0, MAP_CALL(NewList),                   "",                     NULL },
    { SIG_SCI21,           1, MAP_CALL(DisposeList),               "l",                    NULL },
    { SIG_SCI21,           2, MAP_CALL(NewNode),                   ".",                    NULL },
    { SIG_SCI21,           3, MAP_CALL(FirstNode),                 "[l0]",                 NULL },
    { SIG_SCI21,           4, MAP_CALL(LastNode),                  "l",                    NULL },
    { SIG_SCI21,           5, MAP_CALL(EmptyList),                 "l",                    NULL },
    { SIG_SCI21,           6, MAP_CALL(NextNode),                  "n",                    NULL },
    { SIG_SCI21,           7, MAP_CALL(PrevNode),                  "n",                    NULL },
    { SIG_SCI21,           8, MAP_CALL(NodeValue),                 "[n0]",                 NULL },
    { SIG_SCI21,           9, MAP_CALL(AddAfter),                  "lnn.",                 NULL },
    { SIG_SCI21,          10, MAP_CALL(AddToFront),                "ln.",                  NULL },
    { SIG_SCI21,          11, MAP_CALL(AddToEnd),                  "ln.",                  NULL },
    { SIG_SCI21,          12, MAP_CALL(AddBefore),                 "ln.",                  NULL },
    { SIG_SCI21,          13, MAP_CALL(MoveToFront),               "ln",                   NULL },
    { SIG_SCI21,          14, MAP_CALL(MoveToEnd),                 "ln",                   NULL },
    { SIG_SCI21,          15, MAP_CALL(FindKey),                   "l.",                   NULL },
    { SIG_SCI21,          16, MAP_CALL(DeleteKey),                 "l.",                   NULL },
    { SIG_SCI21,          17, MAP_CALL(ListAt),                    "li",                   NULL },
    // FIXME: This doesn't seem to be ListIndexOf. In Torin demo, an index is
    // passed as a second parameter instead of an object. Thus, it seems to
    // be something like ListAt instead... If we swap the two subops though,
    // Torin demo crashes complaining that it tried to send to a non-object,
    // therefore the semantics might be different here (signature was l[o0])
    { SIG_SCI21,          18, MAP_CALL(StubNull),                  "li",                   NULL },
    { SIG_SCI21,          19, MAP_CALL(ListEachElementDo),         "li(.*)",               NULL },
    { SIG_SCI21,          20, MAP_CALL(ListFirstTrue),             "li(.*)",               NULL },
    { SIG_SCI21,          21, MAP_CALL(ListAllTrue),               "li(.*)",               NULL },
    { SIG_SCI21,          22, MAP_CALL(Sort),                      "ooo",                  NULL },
    SCI_SUBOPENTRY_TERMINATOR
};
#endif

struct SciKernelMapEntry {
	const char *name;
	KernelFunctionCall *function;

	SciVersion fromVersion;
	SciVersion toVersion;
	byte forPlatform;

	const char *signature;
	const SciKernelMapSubEntry *subFunctions;
	const SciWorkaroundEntry *workarounds;
};

//    name,                        version/platform,         signature,              sub-signatures,  workarounds
static SciKernelMapEntry s_kernelMap[] = {
    { MAP_CALL(Abs),               SIG_EVERYWHERE,           "i",                     NULL,            kAbs_workarounds },
    { MAP_CALL(AddAfter),          SIG_EVERYWHERE,           "lnn",                   NULL,            NULL },
    { MAP_CALL(AddMenu),           SIG_EVERYWHERE,           "rr",                    NULL,            NULL },
    { MAP_CALL(AddToEnd),          SIG_EVERYWHERE,           "ln",                    NULL,            NULL },
    { MAP_CALL(AddToFront),        SIG_EVERYWHERE,           "ln",                    NULL,            NULL },
    { MAP_CALL(AddToPic),          SIG_EVERYWHERE,           "[il](iiiiii)",          NULL,            NULL },
    { MAP_CALL(Animate),           SIG_EVERYWHERE,           "(l0)(i)",               NULL,            NULL },
    { MAP_CALL(AssertPalette),     SIG_EVERYWHERE,           "i",                     NULL,            NULL },
    { MAP_CALL(AvoidPath),         SIG_EVERYWHERE,           "ii(.*)",                NULL,            NULL },
    { MAP_CALL(BaseSetter),        SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(CanBeHere),         SIG_EVERYWHERE,           "o(l)",                  NULL,            NULL },
    { MAP_CALL(CantBeHere),        SIG_EVERYWHERE,           "o(l)",                  NULL,            NULL },
    { MAP_CALL(CelHigh),           SIG_EVERYWHERE,           "ii(i)",                 NULL,            NULL },
    { MAP_CALL(CelWide),           SIG_EVERYWHERE,           "ii(i)",                 NULL,            NULL },
    { MAP_CALL(CheckFreeSpace),    SIG_SCI32, SIGFOR_ALL,    "r.*",                   NULL,            NULL },
    { MAP_CALL(CheckFreeSpace),    SIG_EVERYWHERE,           "r",                     NULL,            NULL },
    { MAP_CALL(CheckSaveGame),     SIG_EVERYWHERE,           ".*",                    NULL,            NULL },
    { MAP_CALL(Clone),             SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(CoordPri),          SIG_EVERYWHERE,           "i(i)",                  NULL,            NULL },
    { MAP_CALL(CosDiv),            SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
    { MAP_CALL(DeleteKey),         SIG_EVERYWHERE,           "l.",                    NULL,            NULL },
    { MAP_CALL(DeviceInfo),        SIG_EVERYWHERE,           "i(r)(r)(i)",            NULL,            NULL }, // subop
    { MAP_CALL(Display),           SIG_EVERYWHERE,           "[ir]([ir!]*)",          NULL,            NULL },
    // ^ we allow invalid references here, because kDisplay gets called with those in e.g. pq3 during intro
    //    restoreBits() checks and skips invalid handles, so that's fine. Sierra SCI behaved the same
    { MAP_CALL(DirLoop),           SIG_EVERYWHERE,           "oi",                    NULL,            NULL },
    { MAP_CALL(DisposeClone),      SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(DisposeList),       SIG_EVERYWHERE,           "l",                     NULL,            NULL },
    { MAP_CALL(DisposeScript),     SIG_EVERYWHERE,           "i(i*)",                 NULL,            kDisposeScript_workarounds },
    { MAP_CALL(DisposeWindow),     SIG_EVERYWHERE,           "i(i)",                  NULL,            NULL },
    { MAP_CALL(DoAudio),           SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
    { MAP_CALL(DoAvoider),         SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(DoBresen),          SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(DoSound),           SIG_EVERYWHERE,           "i(.*)",                 kDoSound_subops, NULL },
    { MAP_CALL(DoSync),            SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
    { MAP_CALL(DrawCel),           SIG_SCI11, SIGFOR_PC,     "iiiii(i)(i)(r0)",       NULL,            NULL }, // for kq6 hires
    { MAP_CALL(DrawCel),           SIG_EVERYWHERE,           "iiiii(i)(i)",           NULL,            NULL },
    { MAP_CALL(DrawControl),       SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(DrawMenuBar),       SIG_EVERYWHERE,           "i",                     NULL,            NULL },
    { MAP_CALL(DrawPic),           SIG_EVERYWHERE,           "i(i)(i)(i)",            NULL,            NULL },
    { MAP_CALL(DrawStatus),        SIG_EVERYWHERE,           "[r0](i)(i)",            NULL,            NULL },
    { MAP_CALL(EditControl),       SIG_EVERYWHERE,           "[o0][o0]",              NULL,            NULL },
    { MAP_CALL(Empty),             SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
    { MAP_CALL(EmptyList),         SIG_EVERYWHERE,           "l",                     NULL,            NULL },
    { MAP_CALL(FClose),            SIG_EVERYWHERE,           "i",                     NULL,            NULL },
    { MAP_CALL(FGets),             SIG_EVERYWHERE,           "rii",                   NULL,            NULL },
    { MAP_CALL(FOpen),             SIG_EVERYWHERE,           "ri",                    NULL,            NULL },
    { MAP_CALL(FPuts),             SIG_EVERYWHERE,           "ir",                    NULL,            NULL },
    { MAP_CALL(FileIO),            SIG_EVERYWHERE,           "i(.*)",                 kFileIO_subops,  NULL },
    { MAP_CALL(FindKey),           SIG_EVERYWHERE,           "l.",                    NULL,            NULL },
    { MAP_CALL(FirstNode),         SIG_EVERYWHERE,           "[l0]",                  NULL,            NULL },
    { MAP_CALL(FlushResources),    SIG_EVERYWHERE,           "i",                     NULL,            NULL },
    { MAP_CALL(Format),            SIG_EVERYWHERE,           "r(.*)",                 NULL,            NULL },
    { MAP_CALL(GameIsRestarting),  SIG_EVERYWHERE,           "(i)",                   NULL,            NULL },
    { MAP_CALL(GetAngle),          SIG_EVERYWHERE,           "iiii",                  NULL,            NULL },
    // ^^ FIXME - occasionally KQ6 passes a 5th argument by mistake
    { MAP_CALL(GetCWD),            SIG_EVERYWHERE,           "r",                     NULL,            NULL },
    { MAP_CALL(GetDistance),       SIG_EVERYWHERE,           "ii(i)(i)(i)(i)",        NULL,            NULL },
    { MAP_CALL(GetEvent),          SIG_SCIALL, SIGFOR_MAC,   "io(i*)",                NULL,            NULL },
    { MAP_CALL(GetEvent),          SIG_EVERYWHERE,           "io",                    NULL,            NULL },
    { MAP_CALL(GetFarText),        SIG_EVERYWHERE,           "ii[r0]",                NULL,            NULL },
    { MAP_CALL(GetMenu),           SIG_EVERYWHERE,           "i.",                    NULL,            NULL },
    { MAP_CALL(GetMessage),        SIG_EVERYWHERE,           "iiir",                  NULL,            NULL },
    { MAP_CALL(GetPort),           SIG_EVERYWHERE,           "",                      NULL,            NULL },
    { MAP_CALL(GetSaveDir),        SIG_SCI32, SIGFOR_ALL,    "(r*)",                  NULL,            NULL },
    { MAP_CALL(GetSaveDir),        SIG_EVERYWHERE,           "",                      NULL,            NULL },
    { MAP_CALL(GetSaveFiles),      SIG_EVERYWHERE,           "rrr",                   NULL,            NULL },
    { MAP_CALL(GetTime),           SIG_EVERYWHERE,           "(i)",                   NULL,            NULL },
    { MAP_CALL(GlobalToLocal),     SIG_SCI32, SIGFOR_ALL,    "oo",                    NULL,            NULL },
    { MAP_CALL(GlobalToLocal),     SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(Graph),             SIG_EVERYWHERE,           NULL,                    kGraph_subops,   NULL },
    { MAP_CALL(HaveMouse),         SIG_EVERYWHERE,           "",                      NULL,            NULL },
    { MAP_CALL(HiliteControl),     SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(InitBresen),        SIG_EVERYWHERE,           "o(i)",                  NULL,            NULL },
    { MAP_CALL(Intersections),     SIG_EVERYWHERE,           "iiiiriiiri",            NULL,            NULL },
    { MAP_CALL(IsItSkip),          SIG_EVERYWHERE,           "iiiii",                 NULL,            NULL },
    { MAP_CALL(IsObject),          SIG_EVERYWHERE,           ".",                     NULL,            NULL },
    { MAP_CALL(Joystick),          SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
    { MAP_CALL(LastNode),          SIG_EVERYWHERE,           "l",                     NULL,            NULL },
    { MAP_CALL(Load),              SIG_EVERYWHERE,           "ii(i*)",                NULL,            NULL },
    { MAP_CALL(LocalToGlobal),     SIG_SCI32, SIGFOR_ALL,    "oo",                    NULL,            NULL },
    { MAP_CALL(LocalToGlobal),     SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(Lock),              SIG_EVERYWHERE,           "ii(i)",                 NULL,            NULL },
    { MAP_CALL(MapKeyToDir),       SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(Memory),            SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
    { MAP_CALL(MemoryInfo),        SIG_EVERYWHERE,           "i",                     NULL,            NULL },
    { MAP_CALL(MemorySegment),     SIG_EVERYWHERE,           "ir(i)",                 NULL,            NULL }, // subop
    { MAP_CALL(MenuSelect),        SIG_EVERYWHERE,           "o(i)",                  NULL,            NULL },
    { MAP_CALL(MergePoly),         SIG_EVERYWHERE,           "rli",                   NULL,            NULL },
    { MAP_CALL(Message),           SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
    { MAP_CALL(MoveCursor),        SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
    { MAP_CALL(NewList),           SIG_EVERYWHERE,           "",                      NULL,            NULL },
    { MAP_CALL(NewNode),           SIG_EVERYWHERE,           "..",                    NULL,            NULL },
    { MAP_CALL(NewWindow),         SIG_SCIALL, SIGFOR_MAC,   ".*",                    NULL,            NULL },
    { MAP_CALL(NewWindow),         SIG_SCI0, SIGFOR_ALL,     "iiii[r0]i(i)(i)(i)",    NULL,            NULL },
    { MAP_CALL(NewWindow),         SIG_SCI1, SIGFOR_ALL,     "iiii[ir]i(i)(i)([ir])(i)(i)(i)(i)", NULL, NULL },
    { MAP_CALL(NewWindow),         SIG_SCI11, SIGFOR_ALL,    "iiiiiiii[r0]i(i)(i)(i)", NULL,          NULL },
    { MAP_CALL(NextNode),          SIG_EVERYWHERE,           "n",                     NULL,            NULL },
    { MAP_CALL(NodeValue),         SIG_EVERYWHERE,           "[n0]",                  NULL,            NULL },
    { MAP_CALL(NumCels),           SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(NumLoops),          SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(OnControl),         SIG_EVERYWHERE,           "ii(i)(i)(i)",           NULL,            NULL },
    { MAP_CALL(PalVary),           SIG_EVERYWHERE,           "i(i*)",                 kPalVary_subops, NULL },
    { MAP_CALL(Palette),           SIG_EVERYWHERE,           "i(.*)",                 kPalette_subops, NULL },
    { MAP_CALL(Parse),             SIG_EVERYWHERE,           "ro",                    NULL,            NULL },
    { MAP_CALL(PicNotValid),       SIG_EVERYWHERE,           "(i)",                   NULL,            NULL },
    { MAP_CALL(Platform),          SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
    { MAP_CALL(Portrait),          SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL }, // subop
    { MAP_CALL(PrevNode),          SIG_EVERYWHERE,           "n",                     NULL,            NULL },
    { MAP_CALL(PriCoord),          SIG_EVERYWHERE,           "i",                     NULL,            NULL },
    { MAP_CALL(Random),            SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
    { MAP_CALL(ReadNumber),        SIG_EVERYWHERE,           "r",                     NULL,            NULL },
    { MAP_CALL(ResCheck),          SIG_EVERYWHERE,           "ii(iiii)",              NULL,            NULL },
    { MAP_CALL(RespondsTo),        SIG_EVERYWHERE,           ".i",                    NULL,            NULL },
    { MAP_CALL(RestartGame),       SIG_EVERYWHERE,           "",                      NULL,            NULL },
    { MAP_CALL(RestoreGame),       SIG_EVERYWHERE,           "rir",                   NULL,            NULL },
    { MAP_CALL(Said),              SIG_EVERYWHERE,           "[r0]",                  NULL,            NULL },
    { MAP_CALL(SaveGame),          SIG_EVERYWHERE,           "rir(r)",                NULL,            NULL },
    { MAP_CALL(ScriptID),          SIG_EVERYWHERE,           "[io](i)",               NULL,            NULL },
    { MAP_CALL(SetCursor),         SIG_EVERYWHERE,           "i(i*)",                 NULL,            NULL },
    { MAP_CALL(SetDebug),          SIG_EVERYWHERE,           "(i*)",                  NULL,            NULL },
    { MAP_CALL(SetJump),           SIG_EVERYWHERE,           "oiii",                  NULL,            NULL },
    { MAP_CALL(SetMenu),           SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL },
    { MAP_CALL(SetNowSeen),        SIG_EVERYWHERE,           "o(i)",                  NULL,            NULL },
    { MAP_CALL(SetPort),           SIG_EVERYWHERE,           "i(iiiii)(i)",           NULL,            kSetPort_workarounds },
    { MAP_CALL(SetQuitStr),        SIG_EVERYWHERE,           "r",                     NULL,            NULL },
    { MAP_CALL(SetSynonyms),       SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(SetVideoMode),      SIG_EVERYWHERE,           "i",                     NULL,            NULL },
    { MAP_CALL(ShakeScreen),       SIG_EVERYWHERE,           "(i)(i)",                NULL,            NULL },
    { MAP_CALL(ShowMovie),         SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
    { MAP_CALL(SinDiv),            SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
    { MAP_CALL(Sort),              SIG_EVERYWHERE,           "ooo",                   NULL,            NULL },
    { MAP_CALL(Sqrt),              SIG_EVERYWHERE,           "i",                     NULL,            NULL },
    { MAP_CALL(StrAt),             SIG_EVERYWHERE,           "ri(i)",                 NULL,            NULL },
    { MAP_CALL(StrCat),            SIG_EVERYWHERE,           "rr",                    NULL,            NULL },
    { MAP_CALL(StrCmp),            SIG_EVERYWHERE,           "rr(i)",                 NULL,            NULL },
    { MAP_CALL(StrCpy),            SIG_EVERYWHERE,           "[r0]r(i)",              NULL,            kStrCpy_workarounds },
    { MAP_CALL(StrEnd),            SIG_EVERYWHERE,           "r",                     NULL,            NULL },
    { MAP_CALL(StrLen),            SIG_EVERYWHERE,           "[r0]",                  NULL,            NULL },
    { MAP_CALL(StrSplit),          SIG_EVERYWHERE,           "rr[r0]",                NULL,            NULL },
    { MAP_CALL(TextColors),        SIG_EVERYWHERE,           "(i*)",                  NULL,            NULL },
    { MAP_CALL(TextFonts),         SIG_EVERYWHERE,           "(i*)",                  NULL,            NULL },
    { MAP_CALL(TextSize),          SIG_SCIALL, SIGFOR_MAC,   "r[r0]i(i)(r0)(i)",      NULL,            NULL },
    { MAP_CALL(TextSize),          SIG_EVERYWHERE,           "r[r0]i(i)(r0)",         NULL,            NULL },
    { MAP_CALL(TimesCos),          SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
    { "CosMult", kTimesCos,        SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
    { MAP_CALL(TimesCot),          SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
    { MAP_CALL(TimesSin),          SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
    { "SinMult", kTimesSin,        SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
    { MAP_CALL(TimesTan),          SIG_EVERYWHERE,           "ii",                    NULL,            NULL },
    { MAP_CALL(UnLoad),            SIG_EVERYWHERE,           "i[ri]",                 NULL,            kUnLoad_workarounds },
    { MAP_CALL(ValidPath),         SIG_EVERYWHERE,           "r",                     NULL,            NULL },
    { MAP_CALL(Wait),              SIG_EVERYWHERE,           "i",                     NULL,            NULL },

#ifdef ENABLE_SCI32
    // SCI2 Kernel Functions
    { MAP_CALL(AddPlane),          SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(AddScreenItem),     SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(Array),             SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
    { MAP_CALL(CreateTextBitmap),  SIG_EVERYWHERE,           "i(.*)",                 NULL,            NULL },
    { MAP_CALL(DeletePlane),       SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(DeleteScreenItem),  SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(FrameOut),          SIG_EVERYWHERE,           "",                      NULL,            NULL },
    { MAP_CALL(GetHighPlanePri),   SIG_EVERYWHERE,           "",                      NULL,            NULL },
    { MAP_CALL(InPolygon),         SIG_EVERYWHERE,           "iio",                   NULL,            NULL },
    { MAP_CALL(IsHiRes),           SIG_EVERYWHERE,           "",                      NULL,            NULL },
    { MAP_CALL(ListAllTrue),       SIG_EVERYWHERE,           "li(.*)",                NULL,            NULL },
    { MAP_CALL(ListAt),            SIG_EVERYWHERE,           "li",                    NULL,            NULL },
    { MAP_CALL(ListEachElementDo), SIG_EVERYWHERE,           "li(.*)",                NULL,            NULL },
    { MAP_CALL(ListFirstTrue),     SIG_EVERYWHERE,           "li(.*)",                NULL,            NULL },
    { MAP_CALL(ListIndexOf),       SIG_EVERYWHERE,           "l[o0]",                 NULL,            NULL },
    { MAP_CALL(OnMe),              SIG_EVERYWHERE,           "iio(.*)",               NULL,            NULL },
    { MAP_CALL(RepaintPlane),      SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(String),            SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
    { MAP_CALL(UpdatePlane),       SIG_EVERYWHERE,           "o",                     NULL,            NULL },
    { MAP_CALL(UpdateScreenItem),  SIG_EVERYWHERE,           "o",                     NULL,            NULL },

    // SCI2.1 Kernel Functions
    { MAP_CALL(CD),           	   SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
    { MAP_CALL(IsOnMe),            SIG_EVERYWHERE,           "iio(.*)",               NULL,            NULL },
    { MAP_CALL(List),              SIG_SCI21, SIGFOR_ALL,    "(.*)",                  kList_subops,    NULL },
    { MAP_CALL(MulDiv),            SIG_EVERYWHERE,           "iii",                   NULL,            NULL },
    { MAP_CALL(PlayVMD),           SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
    { MAP_CALL(Robot),             SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
    { MAP_CALL(Save),              SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
    { MAP_CALL(Text),              SIG_EVERYWHERE,           "(.*)",                  NULL,            NULL },
    { NULL, NULL,                  SIG_EVERYWHERE,           NULL,                    NULL,            NULL }
#endif
};

Kernel::Kernel(ResourceManager *resMan, SegManager *segMan)
	: _resMan(resMan), _segMan(segMan), _invalid("<invalid>") {
	loadSelectorNames();
	mapSelectors();      // Map a few special selectors for later use
}

Kernel::~Kernel() {
	for (KernelFunctionArray::iterator i = _kernelFuncs.begin(); i != _kernelFuncs.end(); ++i)
		delete[] i->signature;
}

uint Kernel::getSelectorNamesSize() const {
	return _selectorNames.size();
}

const Common::String &Kernel::getSelectorName(uint selector) const {
	if (selector >= _selectorNames.size())
		return _invalid;
	return _selectorNames[selector];
}

uint Kernel::getKernelNamesSize() const {
	return _kernelNames.size();
}

const Common::String &Kernel::getKernelName(uint number) const {
	// FIXME: The following check is a temporary workaround for an issue
	// leading to crashes when using the debugger's backtrace command.
	if (number >= _kernelNames.size())
		return _invalid;
	return _kernelNames[number];
}

int Kernel::findSelector(const char *selectorName) const {
	for (uint pos = 0; pos < _selectorNames.size(); ++pos) {
		if (_selectorNames[pos] == selectorName)
			return pos;
	}

	debugC(2, kDebugLevelVM, "Could not map '%s' to any selector", selectorName);

	return -1;
}

void Kernel::loadSelectorNames() {
	Resource *r = _resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SELECTORS), 0);
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (!r) { // No such resource?
		// Check if we have a table for this game
		// Some demos do not have a selector table
		Common::StringArray staticSelectorTable = checkStaticSelectorNames();

		if (staticSelectorTable.empty())
			error("Kernel: Could not retrieve selector names");
		else
			warning("No selector vocabulary found, using a static one");

		for (uint32 i = 0; i < staticSelectorTable.size(); i++) {
			_selectorNames.push_back(staticSelectorTable[i]);
			if (oldScriptHeader)
				_selectorNames.push_back(staticSelectorTable[i]);
		}

		return;
	}

	int count = READ_LE_UINT16(r->data) + 1; // Counter is slightly off

	for (int i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset);

		Common::String tmp((const char *)r->data + offset + 2, len);
		_selectorNames.push_back(tmp);
		//printf("%s\n", tmp.c_str());	// debug

		// Early SCI versions used the LSB in the selector ID as a read/write
		// toggle. To compensate for that, we add every selector name twice.
		if (oldScriptHeader)
			_selectorNames.push_back(tmp);
	}
}

// this parses a written kernel signature into an internal memory format
// [io] -> either integer or object
// (io) -> optionally integer AND an object
// (i) -> optional integer
// . -> any type
// i* -> optional multiple integers
// .* -> any parameters afterwards (or none)
static uint16 *parseKernelSignature(const char *kernelName, const char *writtenSig) {
	const char *curPos;
	char curChar;
	uint16 *result = NULL;
	uint16 *writePos = NULL;
	int size = 0;
	bool validType = false;
	bool optionalType = false;
	bool eitherOr = false;
	bool optional = false;
	bool hadOptional = false;

	// No signature given? no signature out
	if (!writtenSig)
		return NULL;

	// First, we check how many bytes the result will be
	//  we also check, if the written signature makes any sense
	curPos = writtenSig;
	while (*curPos) {
		curChar = *curPos;
		switch (curChar) {
		case '[': // either or
			if (eitherOr)
				error("signature for k%s: '[' used within '[]'", kernelName);
			eitherOr = true;
			validType = false;
			break;
		case ']': // either or end
			if (!eitherOr)
				error("signature for k%s: ']' used without leading '['", kernelName);
			if (!validType)
				error("signature for k%s: '[]' does not surround valid type(s)", kernelName);
			eitherOr = false;
			validType = false;
			size++;
			break;
		case '(': // optional
			if (optional)
				error("signature for k%s: '(' used within '()' brackets", kernelName);
			if (eitherOr)
				error("signature for k%s: '(' used within '[]' brackets", kernelName);
			optional = true;
			validType = false;
			optionalType = false;
			break;
		case ')': // optional end
			if (!optional)
				error("signature for k%s: ')' used without leading '('", kernelName);
			if (!optionalType)
				error("signature for k%s: '()' does not to surround valid type(s)", kernelName);
			optional = false;
			validType = false;
			hadOptional = true;
			break;
		case '0': // allowed types
		case 'i':
		case 'o':
		case 'r':
		case 'l':
		case 'n':
		case '.':
		case '!':
			if ((hadOptional) & (!optional))
				error("signature for k%s: non-optional type may not follow optional type", kernelName);
			validType = true;
			if (optional)
				optionalType = true;
			if (!eitherOr)
				size++;
			break;
		case '*': // accepts more of the same parameter (must be last char)
			if (!validType) {
				if ((writtenSig == curPos) || (*(curPos - 1) != ']'))
					error("signature for k%s: a valid type must be in front of '*'", kernelName);
			}
			if (eitherOr)
				error("signature for k%s: '*' may not be inside '[]'", kernelName);
			if (optional) {
				if ((*(curPos + 1) != ')') || (*(curPos + 2) != 0))
					error("signature for k%s: '*' may only be used for last type", kernelName);
			} else {
				if (*(curPos + 1) != 0)
					error("signature for k%s: '*' may only be used for last type", kernelName);
			}
			break;
		default:
			error("signature for k%s: '%c' unknown", kernelName, *curPos);
		}
		curPos++;
	}

	uint16 signature = 0;

	// Now we allocate buffer with required size and fill it
	result = new uint16[size + 1];
	writePos = result;
	curPos = writtenSig;
	do {
		curChar = *curPos;
		if (!eitherOr) {
			// not within either-or, check if next character forces output
			switch (curChar) {
			case 0:
			case '[':
			case '(':
			case ')':
			case 'i':
			case 'o':
			case 'r':
			case 'l':
			case 'n':
			case '.':
			case '!':
				// and we also got some signature pending?
				if (signature) {
					if (!(signature & SIG_MAYBE_ANY))
						error("signature for k%s: invalid ('!') may only get used in combination with a real type", kernelName);
					if ((signature & SIG_IS_INVALID) && ((signature & SIG_MAYBE_ANY) == (SIG_TYPE_NULL | SIG_TYPE_INTEGER)))
						error("signature for k%s: invalid ('!') should not be used on exclusive null/integer type", kernelName);
					if (optional) {
						signature |= SIG_IS_OPTIONAL;
						if (curChar != ')')
							signature |= SIG_NEEDS_MORE;
					}
					*writePos = signature;
					writePos++;
					signature = 0;
				}
			}
		}
		switch (curChar) {
		case '[': // either or
			eitherOr = true;
			break;
		case ']': // either or end
			eitherOr = false;
			break;
		case '(': // optional
			optional = true;
			break;
		case ')': // optional end
			optional = false;
			break;
		case '0':
			if (signature & SIG_TYPE_NULL)
				error("signature for k%s: NULL ('0') specified more than once", kernelName);
			signature |= SIG_TYPE_NULL;
			break;
		case 'i':
			if (signature & SIG_TYPE_INTEGER)
				error("signature for k%s: integer ('i') specified more than once", kernelName);
			signature |= SIG_TYPE_INTEGER | SIG_TYPE_NULL;
			break;
		case 'o':
			if (signature & SIG_TYPE_OBJECT)
				error("signature for k%s: object ('o') specified more than once", kernelName);
			signature |= SIG_TYPE_OBJECT;
			break;
		case 'r':
			if (signature & SIG_TYPE_REFERENCE)
				error("signature for k%s: reference ('r') specified more than once", kernelName);
			signature |= SIG_TYPE_REFERENCE;
			break;
		case 'l':
			if (signature & SIG_TYPE_LIST)
				error("signature for k%s: list ('l') specified more than once", kernelName);
			signature |= SIG_TYPE_LIST;
			break;
		case 'n':
			if (signature & SIG_TYPE_NODE)
				error("signature for k%s: node ('n') specified more than once", kernelName);
			signature |= SIG_TYPE_NODE;
			break;
		case '.':
			if (signature & SIG_MAYBE_ANY)
				error("signature for k%s: maybe-any ('.') shouldn't get specified with other types in front of it", kernelName);
			signature |= SIG_MAYBE_ANY;
			break;
		case '!':
			if (signature & SIG_IS_INVALID)
				error("signature for k%s: invalid ('!') specified more than once", kernelName);
			signature |= SIG_IS_INVALID;
			break;
		case '*': // accepts more of the same parameter
			signature |= SIG_MORE_MAY_FOLLOW;
			break;
		default:
			break;
		}
		curPos++;
	} while (curChar);

	// Write terminator
	*writePos = 0;

	return result;
}

uint16 Kernel::findRegType(reg_t reg) {
	// No segment? Must be integer
	if (!reg.segment)
		return SIG_TYPE_INTEGER | (reg.offset ? 0 : SIG_TYPE_NULL);

	if (reg.segment == 0xFFFF)
		return SIG_TYPE_UNINITIALIZED;

	// Otherwise it's an object
	SegmentObj *mobj = _segMan->getSegmentObj(reg.segment);
	if (!mobj)
		return SIG_TYPE_ERROR;

	uint16 result = 0;
	if (!mobj->isValidOffset(reg.offset))
		result |= SIG_IS_INVALID;

	switch (mobj->getType()) {
	case SEG_TYPE_SCRIPT:
		if (reg.offset <= (*(Script *)mobj).getBufSize() &&
			reg.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET &&
		    RAW_IS_OBJECT((*(Script *)mobj).getBuf(reg.offset)) ) {
			result |= ((Script *)mobj)->getObject(reg.offset) ? SIG_TYPE_OBJECT : SIG_TYPE_REFERENCE;
		} else
			result |= SIG_TYPE_REFERENCE;
		break;
	case SEG_TYPE_CLONES:
		result |= SIG_TYPE_OBJECT;
		break;
	case SEG_TYPE_LOCALS:
	case SEG_TYPE_STACK:
	case SEG_TYPE_SYS_STRINGS:
	case SEG_TYPE_DYNMEM:
	case SEG_TYPE_HUNK:
#ifdef ENABLE_SCI32
	case SEG_TYPE_ARRAY:
	case SEG_TYPE_STRING:
#endif
		result |= SIG_TYPE_REFERENCE;
		break;
	case SEG_TYPE_LISTS:
		result |= SIG_TYPE_LIST;
		break;
	case SEG_TYPE_NODES:
		result |= SIG_TYPE_NODE;
		break;
	default:
		return SIG_TYPE_ERROR;
	}
	return result;
}

struct SignatureDebugType {
	uint16 typeCheck;
	const char *text;
};

static const SignatureDebugType signatureDebugTypeList[] = {
	{ SIG_TYPE_NULL,          "null" },
	{ SIG_TYPE_INTEGER,       "integer" },
	{ SIG_TYPE_UNINITIALIZED, "uninitialized" },
	{ SIG_TYPE_OBJECT,        "object" },
	{ SIG_TYPE_REFERENCE,     "reference" },
	{ SIG_TYPE_LIST,          "list" },
	{ SIG_TYPE_NODE,          "node" },
	{ SIG_TYPE_ERROR,         "error" },
	{ SIG_IS_INVALID,         "invalid" },
	{ 0,                      NULL }
};

static void kernelSignatureDebugType(const uint16 type) {
	bool firstPrint = true;

	const SignatureDebugType *list = signatureDebugTypeList;
	while (list->typeCheck) {
		if (type & list->typeCheck) {
			if (!firstPrint)
				printf(", ");
			printf("%s", list->text);
			firstPrint = false;
		}
		list++;
	}
}

// Shows kernel call signature and current arguments for debugging purposes
void Kernel::signatureDebug(const uint16 *sig, int argc, const reg_t *argv) {
	int argnr = 0;
	while (*sig || argc) {
		printf("parameter %d: ", argnr++);
		if (argc) {
			reg_t parameter = *argv;
			printf("%04x:%04x (", PRINT_REG(parameter));
			int regType = findRegType(parameter);
			if (regType)
				kernelSignatureDebugType(regType);
			else
				printf("unknown type of %04x:%04x", PRINT_REG(parameter));
			printf(")");
			argv++;
			argc--;
		} else {
			printf("not passed");
		}
		if (*sig) {
			const uint16 signature = *sig;
			if ((signature & SIG_MAYBE_ANY) == SIG_MAYBE_ANY) {
				printf(", may be any");
			} else {
				printf(", should be ");
				kernelSignatureDebugType(signature);
			}
			if (signature & SIG_IS_OPTIONAL)
				printf(" (optional)");
			if (signature & SIG_NEEDS_MORE)
				printf(" (needs more)");
			if (signature & SIG_MORE_MAY_FOLLOW)
				printf(" (more may follow)");
			sig++;
		}
		printf("\n");
	}
}

bool Kernel::signatureMatch(const uint16 *sig, int argc, const reg_t *argv) {
	uint16 nextSig = *sig;
	uint16 curSig = nextSig;
	while (nextSig && argc) {
		curSig = nextSig;
		int type = findRegType(*argv);

		if ((type & SIG_IS_INVALID) && (!(curSig & SIG_IS_INVALID)))
			return false; // pointer is invalid and signature doesn't allow that?

		if (!((type & ~SIG_IS_INVALID) & curSig))
			return false; // type mismatch

		if (!(curSig & SIG_MORE_MAY_FOLLOW)) {
			sig++;
			nextSig = *sig;
		} else {
			nextSig |= SIG_IS_OPTIONAL; // more may follow -> assumes followers are optional
		}
		argv++;
		argc--;
	}

	// Too many arguments?
	if (argc)
		return false;
	// Signature end reached?
	if (nextSig == 0)
		return true;
	// current parameter is optional?
	if (curSig & SIG_IS_OPTIONAL) {
		// yes, check if nothing more is required
		if (!(curSig & SIG_NEEDS_MORE))
			return true;
	} else {
		// no, check if next parameter is optional
		if (nextSig & SIG_IS_OPTIONAL)
			return true;
	}
	// Too few arguments or more optional arguments required
	return false;
}

void Kernel::mapFunctions() {
	int mapped = 0;
	int ignored = 0;
	uint functionCount = _kernelNames.size();
	byte platformMask = 0;
	SciVersion myVersion = getSciVersion();

	switch (g_sci->getPlatform()) {
	case Common::kPlatformPC:
		platformMask = SIGFOR_DOS;
		break;
	case Common::kPlatformPC98:
		platformMask = SIGFOR_PC98;
		break;
	case Common::kPlatformWindows:
		platformMask = SIGFOR_WIN;
		break;
	case Common::kPlatformMacintosh:
		platformMask = SIGFOR_MAC;
		break;
	case Common::kPlatformAmiga:
		platformMask = SIGFOR_AMIGA;
		break;
	case Common::kPlatformAtariST:
		platformMask = SIGFOR_ATARI;
		break;
	default:
		break;
	}

	_kernelFuncs.resize(functionCount);

	for (uint id = 0; id < functionCount; id++) {
		// First, get the name, if known, of the kernel function with number functnr
		Common::String kernelName = _kernelNames[id];

		// Reset the table entry
		_kernelFuncs[id].function = NULL;
		_kernelFuncs[id].signature = NULL;
		_kernelFuncs[id].name = NULL;
		_kernelFuncs[id].workarounds = NULL;
		_kernelFuncs[id].subFunctions = NULL;
		_kernelFuncs[id].subFunctionCount = 0;
		_kernelFuncs[id].debugCalls = false;
		if (kernelName.empty()) {
			// No name was given -> must be an unknown opcode
			warning("Kernel function %x unknown", id);
			continue;
		}

		// Don't map dummy functions - they will never be called
		if (kernelName == "Dummy") {
			_kernelFuncs[id].function = kDummy;
			continue;
		}

		// If the name is known, look it up in s_kernelMap. This table
		// maps kernel func names to actual function (pointers).
		SciKernelMapEntry *kernelMap = s_kernelMap;
		bool nameMatch = false;
		while (kernelMap->name) {
			if (kernelName == kernelMap->name) {
				if ((kernelMap->fromVersion == SCI_VERSION_NONE) || (kernelMap->fromVersion <= myVersion))
					if ((kernelMap->toVersion == SCI_VERSION_NONE) || (kernelMap->toVersion >= myVersion))
						if (platformMask & kernelMap->forPlatform)
							break;
				nameMatch = true;
			}
			kernelMap++;
		}

		if (kernelMap->name) {
			// A match was found
			_kernelFuncs[id].function = kernelMap->function;
			_kernelFuncs[id].name = kernelMap->name;
			_kernelFuncs[id].signature = parseKernelSignature(kernelMap->name, kernelMap->signature);
			_kernelFuncs[id].workarounds = kernelMap->workarounds;
			if (kernelMap->subFunctions) {
				// Get version for subfunction identification
				SciVersion mySubVersion = (SciVersion)kernelMap->function(NULL, 0, NULL).offset;
				// Now check whats the highest subfunction-id for this version
				const SciKernelMapSubEntry *kernelSubMap = kernelMap->subFunctions;
				uint16 subFunctionCount = 0;
				while (kernelSubMap->function) {
					if ((kernelSubMap->fromVersion == SCI_VERSION_NONE) || (kernelSubMap->fromVersion <= mySubVersion))
						if ((kernelSubMap->toVersion == SCI_VERSION_NONE) || (kernelSubMap->toVersion >= mySubVersion))
							if (subFunctionCount <= kernelSubMap->id)
								subFunctionCount = kernelSubMap->id + 1;
					kernelSubMap++;
				}
				if (!subFunctionCount)
					error("k%s[%x]: no subfunctions found for requested version", kernelName.c_str(), id);
				// Now allocate required memory and go through it again
				_kernelFuncs[id].subFunctionCount = subFunctionCount;
				KernelSubFunction *subFunctions = new KernelSubFunction[subFunctionCount];
				_kernelFuncs[id].subFunctions = subFunctions;
				memset(subFunctions, 0, sizeof(KernelSubFunction) * subFunctionCount);
				// And fill this info out
				kernelSubMap = kernelMap->subFunctions;
				uint kernelSubNr = 0;
				while (kernelSubMap->function) {
					if ((kernelSubMap->fromVersion == SCI_VERSION_NONE) || (kernelSubMap->fromVersion <= mySubVersion))
						if ((kernelSubMap->toVersion == SCI_VERSION_NONE) || (kernelSubMap->toVersion >= mySubVersion)) {
							uint subId = kernelSubMap->id;
							if (!subFunctions[subId].function) {
								subFunctions[subId].function = kernelSubMap->function;
								subFunctions[subId].name = kernelSubMap->name;
								subFunctions[subId].workarounds = kernelSubMap->workarounds;
								if (kernelSubMap->signature) {
									subFunctions[subId].signature = parseKernelSignature(kernelSubMap->name, kernelSubMap->signature);
								} else {
									// we go back the submap to find the previous signature for that kernel call
									const SciKernelMapSubEntry *kernelSubMapBack = kernelSubMap;
									uint kernelSubLeft = kernelSubNr;
									while (kernelSubLeft) {
										kernelSubLeft--;
										kernelSubMapBack--;
										if (kernelSubMapBack->name == kernelSubMap->name) {
											if (kernelSubMapBack->signature) {
												subFunctions[subId].signature = parseKernelSignature(kernelSubMap->name, kernelSubMapBack->signature);
												break;
											}
										}
									}
									if (!subFunctions[subId].signature)
										error("k%s: no previous signatures", kernelSubMap->name);
								}
							}
						}
					kernelSubMap++;
					kernelSubNr++;
				}
			}
			++mapped;
		} else {
			if (nameMatch)
				error("k%s[%x]: not found for this version/platform", kernelName.c_str(), id);
			// No match but a name was given -> stub
			warning("k%s[%x]: unmapped", kernelName.c_str(), id);
			_kernelFuncs[id].function = kStub;
		}
	} // for all functions requesting to be mapped

	debugC(2, kDebugLevelVM, "Handled %d/%d kernel functions, mapping %d and ignoring %d.",
				mapped + ignored, _kernelNames.size(), mapped, ignored);

	return;
}

void Kernel::setDefaultKernelNames() {
	_kernelNames = Common::StringArray(s_defaultKernelNames, ARRAYSIZE(s_defaultKernelNames));

	// Some (later) SCI versions replaced CanBeHere by CantBeHere
	if (_selectorCache.cantBeHere != -1)
		_kernelNames[0x4d] = "CantBeHere";

	switch (getSciVersion()) {
	case SCI_VERSION_0_EARLY:
	case SCI_VERSION_0_LATE:
		// Insert SCI0 file functions after SetCursor (0x28)
		_kernelNames.insert_at(0x29, "FOpen");
		_kernelNames.insert_at(0x2A, "FPuts");
		_kernelNames.insert_at(0x2B, "FGets");
		_kernelNames.insert_at(0x2C, "FClose");

		// Function 0x55 is DoAvoider
		_kernelNames[0x55] = "DoAvoider";

		// Cut off unused functions
		_kernelNames.resize(0x72);
		break;

	case SCI_VERSION_01:
		// Multilingual SCI01 games have StrSplit as function 0x78
		_kernelNames[0x78] = "StrSplit";

		// Cut off unused functions
		_kernelNames.resize(0x79);
		break;

	case SCI_VERSION_1_LATE:
		_kernelNames[0x71] = "MoveCursor";
		break;

	case SCI_VERSION_1_1:
		// In SCI1.1, kSetSynonyms is an empty function
		_kernelNames[0x26] = "Empty";

		if (g_sci->getGameId() == GID_KQ6) {
			// In the Windows version of KQ6 CD, the empty kSetSynonyms
			// function has been replaced with kPortrait. In KQ6 Mac,
			// kPlayBack has been replaced by kShowMovie.
			if (g_sci->getPlatform() == Common::kPlatformWindows)
				_kernelNames[0x26] = "Portrait";
			else if (g_sci->getPlatform() == Common::kPlatformMacintosh)
				_kernelNames[0x84] = "ShowMovie";
		} else if (g_sci->getGameId() == GID_QFG4 && g_sci->isDemo()) {
			_kernelNames[0x7b] = "RemapColors"; // QFG4 Demo has this SCI2 function instead of StrSplit
		}

		_kernelNames[0x71] = "PalVary";
		_kernelNames[0x7c] = "Message";
		break;

	default:
		// Use default table for the other versions
		break;
	}
}

void Kernel::loadKernelNames(GameFeatures *features) {
	_kernelNames.clear();

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2_1)
		setKernelNamesSci21(features);
	else if (getSciVersion() == SCI_VERSION_2)
		setKernelNamesSci2();
	else
#endif
		setDefaultKernelNames();

	mapFunctions();
}

Common::String Kernel::lookupText(reg_t address, int index) {
	char *seeker;
	Resource *textres;

	if (address.segment)
		return _segMan->getString(address);

	int textlen;
	int _index = index;
	textres = _resMan->findResource(ResourceId(kResourceTypeText, address.offset), 0);

	if (!textres) {
		error("text.%03d not found", address.offset);
		return NULL; /* Will probably segfault */
	}

	textlen = textres->size;
	seeker = (char *) textres->data;

	while (index--)
		while ((textlen--) && (*seeker++))
			;

	if (textlen)
		return seeker;

	error("Index %d out of bounds in text.%03d", _index, address.offset);
	return NULL;
}

} // End of namespace Sci
