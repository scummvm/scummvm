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

// Uncompiled kernel signatures are formed from a string of letters.
// each corresponding to a type of a parameter (see below).
// Use small letters to indicate end of sum type.
// Use capital letters for sum types, e.g.
// "LNoLr" for a function which takes two arguments:
// (1) list, node or object
// (2) list or ref
#define KSIG_SPEC_LIST 'l'
#define KSIG_SPEC_NODE 'n'
#define KSIG_SPEC_OBJECT 'o'
#define KSIG_SPEC_REF 'r' // Said Specs and strings
#define KSIG_SPEC_ARITHMETIC 'i'
#define KSIG_SPEC_NULL 'z'
#define KSIG_SPEC_ANY '.'
#define KSIG_SPEC_ELLIPSIS '*' // Arbitrarily more TYPED arguments

#define KSIG_SPEC_SUM_DONE ('a' - 'A')



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
	/*0x6e*/ "ShiftScreen",
	/*0x6f*/ "Palette",
	/*0x70*/ "MemorySegment",
	/*0x71*/ "Intersections",	// MoveCursor (SCI1 late), PalVary (SCI1.1)
	/*0x72*/ "Memory",
	/*0x73*/ "ListOps",
	/*0x74*/ "FileIO",
	/*0x75*/ "DoAudio",
	/*0x76*/ "DoSync",
	/*0x77*/ "AvoidPath",
	/*0x78*/ "Sort",	// StrSplit (SCI01)
	/*0x79*/ "ATan",
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

// [io] -> either integer or object
// (io) -> optionally integer AND an object
// (i) -> optional integer
// . -> any type
// i* -> optional multiple integers
// .* -> any parameters afterwards (or none)

struct SciKernelMapEntry {
	const char *name;
	KernelFunc *function;

	SciVersion fromVersion;
	SciVersion toVersion;
	byte forPlatform;

	const char *signature;
	const char *subSignatures; // placeholder
	const char *workarounds; // placeholder
};

#define SIG_SCIALL  SCI_VERSION_NONE, SCI_VERSION_NONE
#define SIG_SCI11   SCI_VERSION_1_1, SCI_VERSION_1_1
#define SIG_SCI16    SCI_VERSION_NONE, SCI_VERSION_1_1
#define SIG_SCI32    SCI_VERSION_2, SCI_VERSION_NONE

#define SIGFOR_ALL   0x4f
#define SIGFOR_PC    1 << 0
#define SIGFOR_PC98  1 << 1
#define SIGFOR_WIN   1 << 2
#define SIGFOR_MAC   1 << 3
#define SIGFOR_AMIGA 1 << 4
#define SIGFOR_ATARI 1 << 5

#define SIG_EVERYWHERE SIG_SCI32, SIGFOR_ALL

#define MAP_CALL(_name_) #_name_, k##_name_

//    name,                        version/platform,         signature,              sub-signatures,  workarounds
static SciKernelMapEntry s_kernelMap[] = {
    { MAP_CALL(Load),              SIG_EVERYWHERE,           "iii*",                 NULL,            NULL },
    { MAP_CALL(UnLoad),            SIG_EVERYWHERE,           "i.*",                  NULL,            NULL },
	// ^^ FIXME - Work around SQ1 bug, when exiting the Ulence flats bar
    { MAP_CALL(ScriptID),          SIG_EVERYWHERE,           "Ioi*",                 NULL,            NULL },
    { MAP_CALL(DisposeScript),     SIG_EVERYWHERE,           "Oii*",                 NULL,            NULL },
	// ^^ FIXME - Work around QfG1 bug
    { MAP_CALL(Clone),             SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(DisposeClone),      SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(IsObject),          SIG_EVERYWHERE,           ".",                    NULL,            NULL },
    { MAP_CALL(RespondsTo),        SIG_EVERYWHERE,           ".i",                   NULL,            NULL },
    { MAP_CALL(DrawPic),           SIG_EVERYWHERE,           "i*",                   NULL,            NULL },
    { MAP_CALL(PicNotValid),       SIG_EVERYWHERE,           "i*",                   NULL,            NULL },
    { MAP_CALL(Animate),           SIG_EVERYWHERE,           "LI*",                  NULL,            NULL },
	// ^^ FIXME - More like (li?)?
    { MAP_CALL(SetNowSeen),        SIG_EVERYWHERE,           "oi*",                  NULL,            NULL },
	// ^^ FIXME - The second parameter is ignored
    { MAP_CALL(NumLoops),          SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(NumCels),           SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(CelWide),           SIG_EVERYWHERE,           "iOi*",                 NULL,            NULL },
    { MAP_CALL(CelHigh),           SIG_EVERYWHERE,           "iOi*",                 NULL,            NULL },
    { MAP_CALL(DrawCel),           SIG_EVERYWHERE,           "iiiiii*i*r*",          NULL,            NULL },
    { MAP_CALL(AddToPic),          SIG_EVERYWHERE,           "Il*",                  NULL,            NULL },
    { MAP_CALL(NewWindow),         SIG_SCIALL, SIGFOR_MAC,   "*.",                   NULL,            NULL },
    { MAP_CALL(NewWindow),         SIG_EVERYWHERE,           "iiiiZRi*",             NULL,            NULL },
    { MAP_CALL(GetPort),           SIG_EVERYWHERE,           "",                     NULL,            NULL },
    { MAP_CALL(SetPort),           SIG_EVERYWHERE,           "ii*",                  NULL,            NULL },
    { MAP_CALL(DisposeWindow),     SIG_EVERYWHERE,           "ii*",                  NULL,            NULL },
    { MAP_CALL(DrawControl),       SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(HiliteControl),     SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(EditControl),       SIG_EVERYWHERE,           "ZoZo",                 NULL,            NULL },
    { MAP_CALL(TextSize),          SIG_EVERYWHERE,           "rZrii*r*",             NULL,            NULL },
    { MAP_CALL(Display),           SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(GetEvent),          SIG_EVERYWHERE,           "ioi*",                 NULL,            NULL },
	// ^^ FIXME Mac versions pass an extra 3rd parameter (ignored - always 0?)
    { MAP_CALL(GlobalToLocal),     SIG_EVERYWHERE,           "oo*",                  NULL,            NULL },
    { MAP_CALL(LocalToGlobal),     SIG_EVERYWHERE,           "oo*",                  NULL,            NULL },
    { MAP_CALL(MapKeyToDir),       SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(DrawMenuBar),       SIG_EVERYWHERE,           "i",                    NULL,            NULL },
    { MAP_CALL(MenuSelect),        SIG_EVERYWHERE,           "oi*",                  NULL,            NULL },
    { MAP_CALL(AddMenu),           SIG_EVERYWHERE,           "rr",                   NULL,            NULL },
    { MAP_CALL(DrawStatus),        SIG_EVERYWHERE,           "Zri*",                 NULL,            NULL },
    { MAP_CALL(Parse),             SIG_EVERYWHERE,           "ro",                   NULL,            NULL },
    { MAP_CALL(Said),              SIG_EVERYWHERE,           "Zr",                   NULL,            NULL },
    { MAP_CALL(SetSynonyms),       SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(HaveMouse),         SIG_EVERYWHERE,           "",                     NULL,            NULL },
    { MAP_CALL(SetCursor),         SIG_EVERYWHERE,           "i*",                   NULL,            NULL },
    { MAP_CALL(MoveCursor),        SIG_EVERYWHERE,           "ii",                   NULL,            NULL },
    { MAP_CALL(FOpen),             SIG_EVERYWHERE,           "ri",                   NULL,            NULL },
    { MAP_CALL(FPuts),             SIG_EVERYWHERE,           "ir",                   NULL,            NULL },
    { MAP_CALL(FGets),             SIG_EVERYWHERE,           "rii",                  NULL,            NULL },
    { MAP_CALL(FClose),            SIG_EVERYWHERE,           "i",                    NULL,            NULL },
    { MAP_CALL(SaveGame),          SIG_EVERYWHERE,           "rirr*",                NULL,            NULL },
    { MAP_CALL(RestoreGame),       SIG_EVERYWHERE,           "rir*",                 NULL,            NULL },
    { MAP_CALL(RestartGame),       SIG_EVERYWHERE,           "",                     NULL,            NULL },
    { MAP_CALL(GameIsRestarting),  SIG_EVERYWHERE,           "i*",                   NULL,            NULL },
    { MAP_CALL(DoSound),           SIG_EVERYWHERE,           "iIo*",                 NULL,            NULL },
    { MAP_CALL(NewList),           SIG_EVERYWHERE,           "",                     NULL,            NULL },
    { MAP_CALL(DisposeList),       SIG_EVERYWHERE,           "l",                    NULL,            NULL },
    { MAP_CALL(NewNode),           SIG_EVERYWHERE,           "..",                   NULL,            NULL },
    { MAP_CALL(FirstNode),         SIG_EVERYWHERE,           "Zl",                   NULL,            NULL },
    { MAP_CALL(LastNode),          SIG_EVERYWHERE,           "l",                    NULL,            NULL },
    { MAP_CALL(EmptyList),         SIG_EVERYWHERE,           "l",                    NULL,            NULL },
    { MAP_CALL(NextNode),          SIG_EVERYWHERE,           "n",                    NULL,            NULL },
    { MAP_CALL(PrevNode),          SIG_EVERYWHERE,           "n",                    NULL,            NULL },
    { MAP_CALL(NodeValue),         SIG_EVERYWHERE,           "Zn",                   NULL,            NULL },
    { MAP_CALL(AddAfter),          SIG_EVERYWHERE,           "lnn",                  NULL,            NULL },
    { MAP_CALL(AddToFront),        SIG_EVERYWHERE,           "ln",                   NULL,            NULL },
    { MAP_CALL(AddToEnd),          SIG_EVERYWHERE,           "ln",                   NULL,            NULL },
    { MAP_CALL(FindKey),           SIG_EVERYWHERE,           "l.",                   NULL,            NULL },
    { MAP_CALL(DeleteKey),         SIG_EVERYWHERE,           "l.",                   NULL,            NULL },
    { MAP_CALL(Random),            SIG_EVERYWHERE,           "i*",                   NULL,            NULL },
    { MAP_CALL(Abs),               SIG_EVERYWHERE,           "Oi",                   NULL,            NULL },
    { MAP_CALL(Sqrt),              SIG_EVERYWHERE,           "i",                    NULL,            NULL },
    { MAP_CALL(GetAngle),          SIG_EVERYWHERE,           "iiiii*",               NULL,            NULL },
	 // ^^ FIXME - occasionally KQ6 passes a 5th argument by mistake
    { MAP_CALL(GetDistance),       SIG_EVERYWHERE,           "iiiii*",               NULL,            NULL },
    { MAP_CALL(Wait),              SIG_EVERYWHERE,           "i",                    NULL,            NULL },
    { MAP_CALL(GetTime),           SIG_EVERYWHERE,           "i*",                   NULL,            NULL },
    { MAP_CALL(StrEnd),            SIG_EVERYWHERE,           "r",                    NULL,            NULL },
    { MAP_CALL(StrCat),            SIG_EVERYWHERE,           "rr",                   NULL,            NULL },
    { MAP_CALL(StrCmp),            SIG_EVERYWHERE,           "rri*",                 NULL,            NULL },
    { MAP_CALL(StrLen),            SIG_EVERYWHERE,           "Zr",                   NULL,            NULL },
    { MAP_CALL(StrCpy),            SIG_EVERYWHERE,           "rZri*",                NULL,            NULL },
    { MAP_CALL(Format),            SIG_EVERYWHERE,           "r.*",                  NULL,            NULL },
    { MAP_CALL(GetFarText),        SIG_EVERYWHERE,           "iiZr",                 NULL,            NULL },
    { MAP_CALL(ReadNumber),        SIG_EVERYWHERE,           "r",                    NULL,            NULL },
    { MAP_CALL(BaseSetter),        SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(DirLoop),           SIG_EVERYWHERE,           "oi",                   NULL,            NULL },
    { MAP_CALL(CanBeHere),         SIG_EVERYWHERE,           "ol*",                  NULL,            NULL },
    { MAP_CALL(CantBeHere),        SIG_EVERYWHERE,           "ol*",                  NULL,            NULL },
    { MAP_CALL(OnControl),         SIG_EVERYWHERE,           "i*",                   NULL,            NULL },
    { MAP_CALL(InitBresen),        SIG_EVERYWHERE,           "oi*",                  NULL,            NULL },
    { MAP_CALL(DoBresen),          SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(DoAvoider),         SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(SetJump),           SIG_EVERYWHERE,           "oiii",                 NULL,            NULL },
    { MAP_CALL(SetDebug),          SIG_EVERYWHERE,           "i*",                   NULL,            NULL },
    { MAP_CALL(MemoryInfo),        SIG_EVERYWHERE,           "i",                    NULL,            NULL },
    { MAP_CALL(GetMenu),           SIG_EVERYWHERE,           "i.",                   NULL,            NULL },
    { MAP_CALL(SetMenu),           SIG_EVERYWHERE,           "i.*",                  NULL,            NULL },
    { MAP_CALL(GetSaveFiles),      SIG_EVERYWHERE,           "rrr",                  NULL,            NULL },
    { MAP_CALL(GetCWD),            SIG_EVERYWHERE,           "r",                    NULL,            NULL },
    { MAP_CALL(CheckFreeSpace),    SIG_EVERYWHERE,           "r.*",                  NULL,            NULL },
    { MAP_CALL(ValidPath),         SIG_EVERYWHERE,           "r",                    NULL,            NULL },
    { MAP_CALL(CoordPri),          SIG_EVERYWHERE,           "ii*",                  NULL,            NULL },
    { MAP_CALL(StrAt),             SIG_EVERYWHERE,           "rii*",                 NULL,            NULL },
    { MAP_CALL(DeviceInfo),        SIG_EVERYWHERE,           "i.*",                  NULL,            NULL },
    { MAP_CALL(GetSaveDir),        SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
	// ^^ FIXME accepts a parameter in SCI2+ games
    { MAP_CALL(CheckSaveGame),     SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(ShakeScreen),       SIG_EVERYWHERE,           "ii*",                  NULL,            NULL },
    { MAP_CALL(FlushResources),    SIG_EVERYWHERE,           "i",                    NULL,            NULL },
    { MAP_CALL(TimesSin),          SIG_EVERYWHERE,           "ii",                   NULL,            NULL },
    { MAP_CALL(TimesCos),          SIG_EVERYWHERE,           "ii",                   NULL,            NULL },
    { MAP_CALL(Graph),             SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(Joystick),          SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(FileIO),            SIG_EVERYWHERE,           "i.*",                  NULL,            NULL },
    { MAP_CALL(Memory),            SIG_EVERYWHERE,           "i.*",                  NULL,            NULL },
    { MAP_CALL(Sort),              SIG_EVERYWHERE,           "ooo",                  NULL,            NULL },
    { MAP_CALL(AvoidPath),         SIG_EVERYWHERE,           "ii.*",                 NULL,            NULL },
    { MAP_CALL(Lock),              SIG_EVERYWHERE,           "iii*",                 NULL,            NULL },
    { MAP_CALL(Palette),           SIG_EVERYWHERE,           "i.*",                  NULL,            NULL },
    { MAP_CALL(IsItSkip),          SIG_EVERYWHERE,           "iiiii",                NULL,            NULL },
    { MAP_CALL(StrSplit),          SIG_EVERYWHERE,           "rrZr",                 NULL,            NULL },
    { "CosMult", kTimesCos,        SIG_EVERYWHERE,           "ii",                   NULL,            NULL },
    { "SinMult", kTimesSin,        SIG_EVERYWHERE,           "ii",                   NULL,            NULL },
    { MAP_CALL(CosDiv),            SIG_EVERYWHERE,           "ii",                   NULL,            NULL },
    { MAP_CALL(PriCoord),          SIG_EVERYWHERE,           "i",                    NULL,            NULL },
    { MAP_CALL(SinDiv),            SIG_EVERYWHERE,           "ii",                   NULL,            NULL },
    { MAP_CALL(TimesCot),          SIG_EVERYWHERE,           "ii",                   NULL,            NULL },
    { MAP_CALL(TimesTan),          SIG_EVERYWHERE,           "ii",                   NULL,            NULL },
    { MAP_CALL(Message),           SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(GetMessage),        SIG_EVERYWHERE,           "iiir",                 NULL,            NULL },
    { MAP_CALL(DoAudio),           SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(DoSync),            SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(MemorySegment),     SIG_EVERYWHERE,           "iri*",                 NULL,            NULL },
    { MAP_CALL(Intersections),     SIG_EVERYWHERE,           "iiiiriiiri",           NULL,            NULL },
    { MAP_CALL(MergePoly),         SIG_EVERYWHERE,           "rli",                  NULL,            NULL },
    { MAP_CALL(ResCheck),          SIG_EVERYWHERE,           "iii*",                 NULL,            NULL },
    { MAP_CALL(SetQuitStr),        SIG_EVERYWHERE,           "r",                    NULL,            NULL },
    { MAP_CALL(ShowMovie),         SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(SetVideoMode),      SIG_EVERYWHERE,           "i",                    NULL,            NULL },
    { MAP_CALL(Platform),          SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(TextColors),        SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(TextFonts),         SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(Portrait),          SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(PalVary),           SIG_EVERYWHERE,           "ii*",                  NULL,            NULL },
    { MAP_CALL(AssertPalette),     SIG_EVERYWHERE,           "i",                    NULL,            NULL },
    { MAP_CALL(Empty),             SIG_EVERYWHERE,           ".*",                   NULL,            NULL },

#ifdef ENABLE_SCI32
    // SCI2 Kernel Functions
    { MAP_CALL(IsHiRes),           SIG_EVERYWHERE,           "",                     NULL,            NULL },
    { MAP_CALL(Array),             SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(ListAt),            SIG_EVERYWHERE,           "li",                   NULL,            NULL },
    { MAP_CALL(String),            SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(AddScreenItem),     SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(UpdateScreenItem),  SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(DeleteScreenItem),  SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(AddPlane),          SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(DeletePlane),       SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(UpdatePlane),       SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(RepaintPlane),      SIG_EVERYWHERE,           "o",                    NULL,            NULL },
    { MAP_CALL(GetHighPlanePri),   SIG_EVERYWHERE,           "",                     NULL,            NULL },
    { MAP_CALL(FrameOut),          SIG_EVERYWHERE,           "",                     NULL,            NULL },
    { MAP_CALL(ListEachElementDo), SIG_EVERYWHERE,           "li.*",                 NULL,            NULL },
    { MAP_CALL(ListFirstTrue),     SIG_EVERYWHERE,           "li.*",                 NULL,            NULL },
    { MAP_CALL(ListAllTrue),       SIG_EVERYWHERE,           "li.*",                 NULL,            NULL },
    { MAP_CALL(ListIndexOf),       SIG_EVERYWHERE,           "lZo",                  NULL,            NULL },
    { MAP_CALL(OnMe),              SIG_EVERYWHERE,           "iio.*",                NULL,            NULL },
    { MAP_CALL(InPolygon),         SIG_EVERYWHERE,           "iio",                  NULL,            NULL },
    { MAP_CALL(CreateTextBitmap),  SIG_EVERYWHERE,           "i.*",                  NULL,            NULL },

    // SCI2.1 Kernel Functions
    { MAP_CALL(Save),              SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(List),              SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(Robot),             SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(PlayVMD),           SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { MAP_CALL(IsOnMe),            SIG_EVERYWHERE,           "iio.*",                NULL,            NULL },
    { MAP_CALL(MulDiv),            SIG_EVERYWHERE,           "iii",                  NULL,            NULL },
    { MAP_CALL(Text),              SIG_EVERYWHERE,           ".*",                   NULL,            NULL },
    { NULL, NULL,                  SIG_EVERYWHERE,           NULL,                   NULL,            NULL }
#endif
};

#if 0
	// Stub functions
	/*09*/	{ "Show", kShow, "i" },
	{ "ShiftScreen", kShiftScreen, ".*" },
	{ "ListOps", kListOps, ".*" },
	{ "ATan", kATan, ".*" },
	{ "Record", kRecord, ".*" },
	{ "PlayBack", kPlayBack, ".*" },
	{ "DbugStr", kDbugStr, ".*" },
#endif

Kernel::Kernel(ResourceManager *resMan, SegManager *segMan)
	: _resMan(resMan), _segMan(segMan), _invalid("<invalid>") {
	loadSelectorNames();
	mapSelectors();      // Map a few special selectors for later use
}

Kernel::~Kernel() {
	for (KernelFuncsContainer::iterator i = _kernelFuncs.begin(); i != _kernelFuncs.end(); ++i)
		free(i->signature);
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

static char *compileKernelSignature(const char *s) {
	const char *src = s;
	char *result;
	bool ellipsis = false;
	int index = 0;

	if (!src)
		return 0; // NULL signature: Nothing to do

	result = (char *)malloc(strlen(s) + 1);

	while (*src) {
		char c;
		char v = 0;

		if (ellipsis) {
			error("Failed compiling kernel function signature '%s': non-terminal ellipsis '%c'", s, *src);
		}

		do {
			char cc;
			cc = c = *src++;
			if (c >= 'A' || c <= 'Z')
				cc = c | KSIG_SPEC_SUM_DONE;

			switch (cc) {
			case KSIG_SPEC_LIST:
				v |= KSIG_LIST;
				break;

			case KSIG_SPEC_NODE:
				v |= KSIG_NODE;
				break;

			case KSIG_SPEC_REF:
				v |= KSIG_REF;
				break;

			case KSIG_SPEC_OBJECT:
				v |= KSIG_OBJECT;
				break;

			case KSIG_SPEC_ARITHMETIC:
				v |= KSIG_ARITHMETIC;
				break;

			case KSIG_SPEC_NULL:
				v |= KSIG_NULL;
				break;

			case KSIG_SPEC_ANY:
				v |= KSIG_ANY;
				break;

			case KSIG_SPEC_ELLIPSIS:
				v |= KSIG_ELLIPSIS;
				ellipsis = true;
				break;

			default:
				error("ERROR compiling kernel function signature '%s': (%02x / '%c') not understood", s, c, c);
			}
		} while (*src && (*src == KSIG_SPEC_ELLIPSIS || (c < 'a' && c != KSIG_SPEC_ANY)));

		// To handle sum types
		result[index++] = v;
	}

	result[index] = 0;

	return result;
}

void Kernel::mapFunctions() {
	int mapped = 0;
	int ignored = 0;
	uint functionCount = _kernelNames.size();
	byte platformMask = 0;

	switch (g_sci->getPlatform()) {
	case Common::kPlatformPC:
		platformMask = SIGFOR_PC;
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

	for (uint functNr = 0; functNr < functionCount; functNr++) {
		// First, get the name, if known, of the kernel function with number functnr
		Common::String sought_name = _kernelNames[functNr];

		// Reset the table entry
		_kernelFuncs[functNr].func = NULL;
		_kernelFuncs[functNr].signature = NULL;
		_kernelFuncs[functNr].origName = sought_name;
		_kernelFuncs[functNr].isDummy = true;

		if (sought_name.empty()) {
			// No name was given -> must be an unknown opcode
			warning("Kernel function %x unknown", functNr);
			continue;
		}

		// Don't map dummy functions - they will never be called
		if (sought_name == "Dummy")
			continue;

		// If the name is known, look it up in s_kernelMap. This table
		// maps kernel func names to actual function (pointers).
		SciKernelMapEntry *kernelMap = s_kernelMap;
		bool nameMatch = false;
		while (kernelMap->name) {
			if (sought_name == kernelMap->name) {
				if (platformMask & kernelMap->forPlatform)
					break;
				nameMatch = true;
			}
			kernelMap++;
		}

		if (kernelMap->name) {
			// A match was found
			if (kernelMap->function) {
				_kernelFuncs[functNr].func = kernelMap->function;
				_kernelFuncs[functNr].signature = compileKernelSignature(kernelMap->signature);
				_kernelFuncs[functNr].isDummy = false;
				++mapped;
			} else {
				//warning("Ignoring function %s\n", s_kernelFuncMap[found].name);
				++ignored;
			}
		} else {
			if (nameMatch)
				error("kernel function %s[%x] not found for this version/platform", sought_name.c_str(), functNr);
			// No match but a name was given -> stub
			warning("Kernel function %s[%x] unmapped", sought_name.c_str(), functNr);
		}
	} // for all functions requesting to be mapped

	debugC(2, kDebugLevelVM, "Handled %d/%d kernel functions, mapping %d and ignoring %d.",
				mapped + ignored, _kernelNames.size(), mapped, ignored);

	return;
}

int Kernel::findRegType(reg_t reg) {
	// No segment? Must be arithmetic
	if (!reg.segment)
		return reg.offset ? KSIG_ARITHMETIC : KSIG_ARITHMETIC | KSIG_NULL;

	// Otherwise it's an object
	SegmentObj *mobj = _segMan->getSegmentObj(reg.segment);
	if (!mobj)
		return 0; // Invalid

	if (!mobj->isValidOffset(reg.offset))
		error("[KERN] ref %04x:%04x is invalid", PRINT_REG(reg));

	switch (mobj->getType()) {
	case SEG_TYPE_SCRIPT:
		if (reg.offset <= (*(Script *)mobj).getBufSize() &&
			reg.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET &&
		    RAW_IS_OBJECT((*(Script *)mobj).getBuf(reg.offset)) ) {
			return ((Script *)mobj)->getObject(reg.offset) ? KSIG_OBJECT : KSIG_REF;
		} else
			return KSIG_REF;
	case SEG_TYPE_CLONES:
		return KSIG_OBJECT;
	case SEG_TYPE_LOCALS:
	case SEG_TYPE_STACK:
	case SEG_TYPE_SYS_STRINGS:
	case SEG_TYPE_DYNMEM:
	case SEG_TYPE_HUNK:
#ifdef ENABLE_SCI32
	case SEG_TYPE_ARRAY:
	case SEG_TYPE_STRING:
#endif
		return KSIG_REF;
	case SEG_TYPE_LISTS:
		return KSIG_LIST;
	case SEG_TYPE_NODES:
		return KSIG_NODE;
	default:
		return 0;
	}
}

struct SignatureDebugType {
	char typeCheck;
	const char *text;
};

static const SignatureDebugType signatureDebugTypeList[] = {
	{ KSIG_NULL,       "null" },
	{ KSIG_ARITHMETIC, "value" },
	{ KSIG_OBJECT,     "object" },
	{ KSIG_REF,        "reference" },
	{ KSIG_LIST,       "list" },
	{ KSIG_NODE,       "node" },
	{ 0,               NULL }
};

static void kernelSignatureDebugType(const char type) {
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
void Kernel::signatureDebug(const char *sig, int argc, const reg_t *argv) {
	int argnr = 0;
	while (*sig || argc) {
		printf("parameter %d: ", argnr++);
		if (argc) {
			reg_t parameter = *argv;
			printf("%04x:%04x (", PRINT_REG(parameter));
			kernelSignatureDebugType(findRegType(parameter));
			printf(")");
			argv++;
			argc--;
		} else {
			printf("not passed");
		}
		if (*sig) {
			const char signature = *sig;
			if ((signature & KSIG_ANY) == KSIG_ANY) {
				printf(", may be any");
			} else {
				printf(", should be ");
				kernelSignatureDebugType(signature);
			}
			if (signature & KSIG_ELLIPSIS)
				printf(" (optional)");
			sig++;
		}
		printf("\n");
	}
}

bool Kernel::signatureMatch(const char *sig, int argc, const reg_t *argv) {
	const char *checkSig = sig;
	const reg_t *checkParam = argv;
	int checkCount = argc;
	// Always "match" if no signature is given
	if (!sig)
		return true;

	while (*checkSig && checkCount) {
		if ((*checkSig & KSIG_ANY) != KSIG_ANY) {
			int type = findRegType(*checkParam);

			if (!type) {
				warning("[KERNEL] call signature: couldn't determine type of ref %04x:%04x", PRINT_REG(*argv));
				signatureDebug(sig, argc, argv);
				return false;
			}

			if (!(type & *checkSig)) {
				warning("[KERNEL] call signature: %d args left, is %d, should be %d", argc, type, *sig);
				signatureDebug(sig, argc, argv);
				return false;
			}

		}
		if (!(*checkSig & KSIG_ELLIPSIS))
			++checkSig;
		++checkParam;
		--checkCount;
	}

	if (checkCount) {
		warning("[KERNEL] call signature: too many arguments");
		signatureDebug(sig, argc, argv);
		return false; // Too many arguments
	}
	if (*checkSig == 0 || (*checkSig & KSIG_ELLIPSIS))
		return true;

	warning("[KERNEL] call signature: too few arguments");
	signatureDebug(sig, argc, argv);
	return false;
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
