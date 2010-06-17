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

struct SciKernelFunction {
	const char *name;
	KernelFunc *fun; /* The actual function */
	const char *signature;  /* kfunct signature */
};

SciKernelFunction kfunct_mappers[] = {
	/*00*/	{ "Load", kLoad, "iii*" },
	/*01*/	{ "UnLoad", kUnLoad, "i.*" },	// Work around SQ1 bug, when exiting the Ulence flats bar
	/*02*/	{ "ScriptID", kScriptID, "Ioi*" },
	/*03*/	{ "DisposeScript", kDisposeScript, "Oii*" }, // Work around QfG1 bug
	/*04*/	{ "Clone", kClone, "o" },
	/*05*/	{ "DisposeClone", kDisposeClone, "o" },
	/*06*/	{ "IsObject", kIsObject, "." },
	/*07*/	{ "RespondsTo", kRespondsTo, ".i" },
	/*08*/	{ "DrawPic", kDrawPic, "i*" },

	/*0a*/	{ "PicNotValid", kPicNotValid, "i*" },
	/*0b*/	{ "Animate", kAnimate, "LI*" }, // More like (li?)?
	/*0c*/	{ "SetNowSeen", kSetNowSeen, "oi*" }, // The second parameter is ignored
	/*0d*/	{ "NumLoops", kNumLoops, "o" },
	/*0e*/	{ "NumCels", kNumCels, "o" },
	/*0f*/	{ "CelWide", kCelWide, "iOi*" },
	/*10*/	{ "CelHigh", kCelHigh, "iOi*" },
	/*11*/	{ "DrawCel", kDrawCel, "iiiiii*i*r*" },
	/*12*/	{ "AddToPic", kAddToPic, "Il*" },
	// FIXME: signature check removed (set to .*) as kNewWindow is different in Mac versions
	/*13*/	{ "NewWindow", kNewWindow, "*." },
	///*13*/	{ "NewWindow", kNewWindow, "iiiiZRi*" },
	/*14*/	{ "GetPort", kGetPort, "" },
	/*15*/	{ "SetPort", kSetPort, "ii*" },
	/*16*/	{ "DisposeWindow", kDisposeWindow, "ii*" },
	/*17*/	{ "DrawControl", kDrawControl, "o" },
	/*18*/	{ "HiliteControl", kHiliteControl, "o" },
	/*19*/	{ "EditControl", kEditControl, "ZoZo" },
	/*1a*/	{ "TextSize", kTextSize, "rZrii*r*" },
	/*1b*/	{ "Display", kDisplay, ".*" },
	/*1c*/	{ "GetEvent", kGetEvent, "ioi*" },	// Mac versions pass an extra 3rd parameter (ignored - always 0?)
	/*1d*/	{ "GlobalToLocal", kGlobalToLocal, "oo*" },
	/*1e*/	{ "LocalToGlobal", kLocalToGlobal, "oo*" },
	/*1f*/	{ "MapKeyToDir", kMapKeyToDir, "o" },
	/*20*/	{ "DrawMenuBar", kDrawMenuBar, "i" },
	/*21*/	{ "MenuSelect", kMenuSelect, "oi*" },
	/*22*/	{ "AddMenu", kAddMenu, "rr" },
	/*23*/	{ "DrawStatus", kDrawStatus, "Zri*" },
	/*24*/	{ "Parse", kParse, "ro" },
	/*25*/	{ "Said", kSaid, "Zr" },
	/*26*/	{ "SetSynonyms", kSetSynonyms, "o" },
	/*27*/	{ "HaveMouse", kHaveMouse, "" },
	/*28*/	{ "SetCursor", kSetCursor, "i*" },
	// FIXME: The number 0x28 occurs twice :-)
	/*28*/	{ "MoveCursor", kMoveCursor, "ii" },
	/*29*/	{ "FOpen", kFOpen, "ri" },
	/*2a*/	{ "FPuts", kFPuts, "ir" },
	/*2b*/	{ "FGets", kFGets, "rii" },
	/*2c*/	{ "FClose", kFClose, "i" },
	/*2d*/	{ "SaveGame", kSaveGame, "rirr*" },
	/*2e*/	{ "RestoreGame", kRestoreGame, "rir*" },
	/*2f*/	{ "RestartGame", kRestartGame, "" },
	/*30*/	{ "GameIsRestarting", kGameIsRestarting, "i*" },
	/*31*/	{ "DoSound", kDoSound, "iIo*" },
	/*32*/	{ "NewList", kNewList, "" },
	/*33*/	{ "DisposeList", kDisposeList, "l" },
	/*34*/	{ "NewNode", kNewNode, ".." },
	/*35*/	{ "FirstNode", kFirstNode, "Zl" },
	/*36*/	{ "LastNode", kLastNode, "l" },
	/*37*/	{ "EmptyList", kEmptyList, "l" },
	/*38*/	{ "NextNode", kNextNode, "n" },
	/*39*/	{ "PrevNode", kPrevNode, "n" },
	/*3a*/	{ "NodeValue", kNodeValue, "Zn" },
	/*3b*/	{ "AddAfter", kAddAfter, "lnn" },
	/*3c*/	{ "AddToFront", kAddToFront, "ln" },
	/*3d*/	{ "AddToEnd", kAddToEnd, "ln" },
	/*3e*/	{ "FindKey", kFindKey, "l." },
	/*3f*/	{ "DeleteKey", kDeleteKey, "l." },
	/*40*/	{ "Random", kRandom, "i*" },
	/*41*/	{ "Abs", kAbs, "Oi" },
	/*42*/	{ "Sqrt", kSqrt, "i" },
	/*43*/	{ "GetAngle", kGetAngle, "iiiii*" }, // occasionally KQ6 passes a 5th argument by mistake
	/*44*/	{ "GetDistance", kGetDistance, "iiiii*" },
	/*45*/	{ "Wait", kWait, "i" },
	/*46*/	{ "GetTime", kGetTime, "i*" },
	/*47*/	{ "StrEnd", kStrEnd, "r" },
	/*48*/	{ "StrCat", kStrCat, "rr" },
	/*49*/	{ "StrCmp", kStrCmp, "rri*" },
	/*4a*/	{ "StrLen", kStrLen, "Zr" },
	/*4b*/	{ "StrCpy", kStrCpy, "rZri*" },
	/*4c*/	{ "Format", kFormat, "r.*" },
	/*4d*/	{ "GetFarText", kGetFarText, "iiZr" },
	/*4e*/	{ "ReadNumber", kReadNumber, "r" },
	/*4f*/	{ "BaseSetter", kBaseSetter, "o" },
	/*50*/	{ "DirLoop", kDirLoop, "oi" },
	// Opcode 51 is defined twice for a reason: In older SCI versions
	// it is CanBeHere, whereas in newer version it is CantBeHere
	/*51*/	{ "CanBeHere", kCanBeHere, "ol*" },
	/*51*/	{ "CantBeHere", kCantBeHere, "ol*" },
	/*52*/	{ "OnControl", kOnControl, "i*" },
	/*53*/	{ "InitBresen", kInitBresen, "oi*" },
	/*54*/	{ "DoBresen", kDoBresen, "o" },
	/*55*/	{ "DoAvoider", kDoAvoider, "o" },
	/*56*/	{ "SetJump", kSetJump, "oiii" },
	/*57*/	{ "SetDebug", kSetDebug, "i*" },
	/*5c*/	{ "MemoryInfo", kMemoryInfo, "i" },
	/*5f*/	{ "GetMenu", kGetMenu, "i." },
	/*60*/	{ "SetMenu", kSetMenu, "i.*" },
	/*61*/	{ "GetSaveFiles", kGetSaveFiles, "rrr" },
	/*62*/	{ "GetCWD", kGetCWD, "r" },
	/*63*/	{ "CheckFreeSpace", kCheckFreeSpace, "r.*" },
	/*64*/	{ "ValidPath", kValidPath, "r" },
	/*65*/	{ "CoordPri", kCoordPri, "ii*" },
	/*66*/	{ "StrAt", kStrAt, "rii*" },
	/*67*/	{ "DeviceInfo", kDeviceInfo, "i.*" },
	/*68*/	{ "GetSaveDir", kGetSaveDir, ".*" },	// accepts a parameter in SCI2+ games
	/*69*/	{ "CheckSaveGame", kCheckSaveGame, ".*" },
	/*6a*/	{ "ShakeScreen", kShakeScreen, "ii*" },
	/*6b*/	{ "FlushResources", kFlushResources, "i" },
	/*6c*/	{ "TimesSin", kTimesSin, "ii" },
	/*6d*/	{ "TimesCos", kTimesCos, "ii" },
	/*6e*/	{ "6e", kTimesSin, "ii" },
	/*6f*/	{ "6f", kTimesCos, "ii" },
	/*70*/	{ "Graph", kGraph, ".*" },
	/*71*/	{ "Joystick", kJoystick, ".*" },

	// Experimental functions
	/*74*/	{ "FileIO", kFileIO, "i.*" },
	/*(?)*/	{ "Memory", kMemory, "i.*" },
	/*(?)*/	{ "Sort", kSort, "ooo" },
	/*(?)*/	{ "AvoidPath", kAvoidPath, "ii.*" },
	/*(?)*/	{ "Lock", kLock, "iii*" },
	/*(?)*/	{ "Palette", kPalette, "i.*" },
	/*(?)*/	{ "IsItSkip", kIsItSkip, "iiiii" },
	/*7b*/	{ "StrSplit", kStrSplit, "rrZr" },

	// Non-experimental functions without a fixed ID
	{ "CosMult", kTimesCos, "ii" },
	{ "SinMult", kTimesSin, "ii" },

	// Misc functions
	/*(?)*/	{ "CosDiv", kCosDiv, "ii" },
	/*(?)*/	{ "PriCoord", kPriCoord, "i" },
	/*(?)*/	{ "SinDiv", kSinDiv, "ii" },
	/*(?)*/	{ "TimesCot", kTimesCot, "ii" },
	/*(?)*/	{ "TimesTan", kTimesTan, "ii" },
	{ "Message", kMessage, ".*" },
	{ "GetMessage", kGetMessage, "iiir" },
	{ "DoAudio", kDoAudio, ".*" },
	{ "DoSync", kDoSync, ".*" },
	{ "MemorySegment", kMemorySegment, "iri*" },
	{ "Intersections", kIntersections, "iiiiriiiri" },
	{ "MergePoly", kMergePoly, "rli" },
	{ "ResCheck", kResCheck, "iii*" },
	{ "SetQuitStr", kSetQuitStr, "r" },
	{ "ShowMovie", kShowMovie, ".*" },
	{ "SetVideoMode", kSetVideoMode, "i" },
	{ "Platform", kPlatform, ".*" },
	{ "TextColors", kTextColors, ".*" },
	{ "TextFonts", kTextFonts, ".*" },
	{ "Portrait", kPortrait, ".*" },
	{ "Empty", kEmpty, ".*" },

#ifdef ENABLE_SCI32
	// SCI2 Kernel Functions
	{ "IsHiRes", kIsHiRes, "" },
	{ "Array", kArray, ".*" },
	{ "ListAt", kListAt, "li" },
	{ "String", kString, ".*" },
	{ "AddScreenItem", kAddScreenItem, "o" },
	{ "UpdateScreenItem", kUpdateScreenItem, "o" },
	{ "DeleteScreenItem", kDeleteScreenItem, "o" },
	{ "AddPlane", kAddPlane, "o" },
	{ "DeletePlane", kDeletePlane, "o" },
	{ "UpdatePlane", kUpdatePlane, "o" },
	{ "RepaintPlane", kRepaintPlane, "o" },
	{ "GetHighPlanePri", kGetHighPlanePri, "" },
	{ "FrameOut", kFrameOut, "" },
	{ "ListEachElementDo", kListEachElementDo, "li.*" },
	{ "ListFirstTrue", kListFirstTrue, "li.*" },
	{ "ListAllTrue", kListAllTrue, "li.*" },
	{ "ListIndexOf", kListIndexOf, "lZo" },
	{ "OnMe", kOnMe, "iio.*" },
	{ "InPolygon", kInPolygon, "iio" },
	{ "CreateTextBitmap", kCreateTextBitmap, "i.*" },

	// SCI2.1 Kernel Functions
	{ "Save", kSave, ".*" },
	{ "List", kList, ".*" },
	{ "Robot", kRobot, ".*" },
	{ "PlayVMD", kPlayVMD, ".*" },
	{ "IsOnMe", kIsOnMe, "iio.*" },
	{ "MulDiv", kMulDiv, "iii" },

#endif

	// its a stub, but its needed for Pharkas to work
	{ "PalVary", kPalVary, "ii*" },
	{ "AssertPalette", kAssertPalette, "i" },

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

	{NULL, NULL, NULL} // Terminator
};

Kernel::Kernel(ResourceManager *resMan, SegManager *segMan)
	: _resMan(resMan), _segMan(segMan), _invalid("<invalid>") {
	loadSelectorNames();
	mapSelectors();      // Map a few special selectors for later use
}

Kernel::~Kernel() {
	for (KernelFuncsContainer::iterator i = _kernelFuncs.begin(); i != _kernelFuncs.end(); ++i)
		// TODO: Doing a const_cast is not that nice actually... But since KernelFuncWithSignature
		// keeps the signature member as "const char *" there is no way around it.
		// Think of a clever way to avoid this.
		free(const_cast<char *>(i->signature));
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
	// FIXME: The following check is a temporary workaround for
	// an issue leading to crashes when using the debugger's backtrace
	// command.
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

static void kernel_compile_signature(const char **s) {
	const char *src = *s;
	char *result;
	bool ellipsis = false;
	int index = 0;

	if (!src)
		return; // NULL signature: Nothing to do

	result = (char *)malloc(strlen(*s) + 1);

	while (*src) {
		char c;
		char v = 0;

		if (ellipsis) {
			error("Failed compiling kernel function signature '%s': non-terminal ellipsis '%c'", *s, *src);
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
				error("ERROR compiling kernel function signature '%s': (%02x / '%c') not understood", *s, c, c);
			}
		} while (*src && (*src == KSIG_SPEC_ELLIPSIS || (c < 'a' && c != KSIG_SPEC_ANY)));

		// To handle sum types
		result[index++] = v;
	}

	result[index] = 0;
	*s = result; // Write back
}

void Kernel::mapFunctions() {
	int mapped = 0;
	int ignored = 0;
	uint functions_nr = _kernelNames.size();

	_kernelFuncs.resize(functions_nr);

	for (uint functnr = 0; functnr < functions_nr; functnr++) {
		int found = -1;

		// First, get the name, if known, of the kernel function with number functnr
		Common::String sought_name = _kernelNames[functnr];

		// Reset the table entry
		_kernelFuncs[functnr].fun = NULL;
		_kernelFuncs[functnr].signature = NULL;
		_kernelFuncs[functnr].orig_name = sought_name;

		if (sought_name.empty()) {
			// No name was given -> must be an unknown opcode
			warning("Kernel function %x unknown", functnr);
			_kernelFuncs[functnr].isDummy = true;
			continue;
		}

		// Don't map dummy functions - they will never be called
		if (sought_name == "Dummy") {
			_kernelFuncs[functnr].isDummy = true;
			continue;
		}

		// If the name is known, look it up in kfunct_mappers. This table
		// maps kernel func names to actual function (pointers).
		for (uint seeker = 0; (found == -1) && kfunct_mappers[seeker].name; seeker++)
			if (sought_name == kfunct_mappers[seeker].name)
				found = seeker; // Found a kernel function with the correct name!

		if (found == -1) {
			// No match but a name was given -> stub
			warning("Kernel function %s[%x] unmapped", sought_name.c_str(), functnr);
			_kernelFuncs[functnr].isDummy = true;
		} else {
			// A match in kfunct_mappers was found
			if (kfunct_mappers[found].fun) {
				_kernelFuncs[functnr].fun = kfunct_mappers[found].fun;
				_kernelFuncs[functnr].signature = kfunct_mappers[found].signature;
				_kernelFuncs[functnr].isDummy = false;
				kernel_compile_signature(&(_kernelFuncs[functnr].signature));
				++mapped;
			} else {
				//warning("Ignoring function %s\n", kfunct_mappers[found].name);
				++ignored;
			}
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
		warning("[KERN] ref %04x:%04x is invalid", PRINT_REG(reg));

	switch (mobj->getType()) {
	case SEG_TYPE_SCRIPT:
		if (reg.offset <= (*(Script *)mobj).getBufSize() &&
			reg.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET &&
		    RAW_IS_OBJECT((*(Script *)mobj)._buf + reg.offset)) {
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

bool Kernel::signatureMatch(const char *sig, int argc, const reg_t *argv) {
	// Always "match" if no signature is given
	if (!sig)
		return true;

	while (*sig && argc) {
		if ((*sig & KSIG_ANY) != KSIG_ANY) {
			int type = findRegType(*argv);

			if (!type) {
				warning("[KERN] Could not determine type of ref %04x:%04x; failing signature check", PRINT_REG(*argv));
				return false;
			}

			if (!(type & *sig)) {
				warning("kernel_matches_signature: %d args left, is %d, should be %d", argc, type, *sig);
				return false;
			}

		}
		if (!(*sig & KSIG_ELLIPSIS))
			++sig;
		++argv;
		--argc;
	}

	if (argc) {
		warning("kernel_matches_signature: too many arguments");
		return false; // Too many arguments
	}
	if (*sig == 0 || (*sig & KSIG_ELLIPSIS))
		return true;

	warning("kernel_matches_signature: too few arguments");
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

		if (!strcmp(g_sci->getGameID(), "kq6")) {
			// In the Windows version of KQ6 CD, the empty kSetSynonyms
			// function has been replaced with kPortrait. In KQ6 Mac,
			// kPlayBack has been replaced by kShowMovie.
			if (g_sci->getPlatform() == Common::kPlatformWindows)
				_kernelNames[0x26] = "Portrait";
			else if (g_sci->getPlatform() == Common::kPlatformMacintosh)
				_kernelNames[0x84] = "ShowMovie";
		} else if (!strcmp(g_sci->getGameID(), "qfg4") && g_sci->isDemo()) {
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
