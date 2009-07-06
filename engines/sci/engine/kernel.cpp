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
#include "sci/engine/intmap.h"
#include "sci/engine/kernel.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/gfx/operations.h"
#include "sci/engine/kernel_types.h"

namespace Sci {

/** The string used to identify the "unknown" SCI0 function for each game */
#define SCRIPT_UNKNOWN_FUNCTION_STRING "[Unknown]"

// Default kernel name table
#define SCI0_KNAMES_WELL_DEFINED 0x6e
#define SCI_KNAMES_DEFAULT_ENTRIES_NR 0x89

static const char *sci_default_knames[SCI_KNAMES_DEFAULT_ENTRIES_NR] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Clone",
	/*0x05*/ "DisposeClone",
	/*0x06*/ "IsObject",
	/*0x07*/ "RespondsTo",
	/*0x08*/ "DrawPic",
	/*0x09*/ "Show",
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
	/*0x26*/ "SetSynonyms",
	/*0x27*/ "HaveMouse",
	/*0x28*/ "SetCursor",
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
	/*0x4d*/ "CanBeHere",
	/*0x4e*/ "OnControl",
	/*0x4f*/ "InitBresen",
	/*0x50*/ "DoBresen",
	/*0x51*/ "Platform",
	/*0x52*/ "SetJump",
	/*0x53*/ "SetDebug",
	/*0x54*/ "InspectObj",
	/*0x55*/ "ShowSends",
	/*0x56*/ "ShowObjs",
	/*0x57*/ "ShowFree",
	/*0x58*/ "MemoryInfo",
	/*0x59*/ "StackUsage",
	/*0x5a*/ "Profiler",
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
	/*0x6e*/ "ShiftScreen",
	/*0x6f*/ "Palette",
	/*0x70*/ "MemorySegment",
	/*0x71*/ "MoveCursor",
	/*0x72*/ "Memory",
	/*0x73*/ "ListOps",
	/*0x74*/ "FileIO",
	/*0x75*/ "DoAudio",
	/*0x76*/ "DoSync",
	/*0x77*/ "AvoidPath",
	/*0x78*/ "Sort",
	/*0x79*/ "ATan",
	/*0x7a*/ "Lock",
	/*0x7b*/ "StrSplit",
	/*0x7c*/ "Message",
	/*0x7d*/ "IsItSkip",
	/*0x7e*/ "MergePoly",
	/*0x7f*/ "ResCheck",
	/*0x80*/ "AssertPalette",
	/*0x81*/ "TextColors",
	/*0x82*/ "TextFonts",
	/*0x83*/ "Record",
	/*0x84*/ "PlayBack",
	/*0x85*/ "ShowMovie",
	/*0x86*/ "SetVideoMode",
	/*0x87*/ "SetQuitStr",
	/*0x88*/ "DbugStr"
};

struct SciKernelFunction {
	const char *name;
	KernelFunc *fun; /* The actual function */
	const char *signature;  /* kfunct signature */
};

#define DEFUN(name, fun, sig) {name, fun, sig}
#define NOFUN(name) {name, NULL, NULL}

SciKernelFunction kfunct_mappers[] = {
	/*00*/	DEFUN("Load", kLoad, "iii*"),
	/*01*/	DEFUN("UnLoad", kUnLoad, "i.*"),
	/*02*/	DEFUN("ScriptID", kScriptID, "Ioi*"),
	/*03*/	DEFUN("DisposeScript", kDisposeScript, "Oi"), // Work around QfG1 bug
	/*04*/	DEFUN("Clone", kClone, "o"),
	/*05*/	DEFUN("DisposeClone", kDisposeClone, "o"),
	/*06*/	DEFUN("IsObject", kIsObject, "."),
	/*07*/	DEFUN("RespondsTo", kRespondsTo, ".i"),
	/*08*/	DEFUN("DrawPic", kDrawPic, "i*"),
	/*09*/	DEFUN("Show", kShow, "i"),
	/*0a*/	DEFUN("PicNotValid", kPicNotValid, "i*"),
	/*0b*/	DEFUN("Animate", kAnimate, "LI*"), // More like (li?)?
	/*0c*/	DEFUN("SetNowSeen", kSetNowSeen, "oi*"), // The second parameter is ignored
	/*0d*/	DEFUN("NumLoops", kNumLoops, "o"),
	/*0e*/	DEFUN("NumCels", kNumCels, "o"),
	/*0f*/	DEFUN("CelWide", kCelWide, "iOiOi"),
	/*10*/	DEFUN("CelHigh", kCelHigh, "iOiOi"),
	/*11*/	DEFUN("DrawCel", kDrawCel, "iiiiii*"),
	/*12*/	DEFUN("AddToPic", kAddToPic, "Il*"),
	/*13*/	DEFUN("NewWindow", kNewWindow, "iiiiZRi*"),
	/*14*/	DEFUN("GetPort", kGetPort, ""),
	/*15*/	DEFUN("SetPort", kSetPort, "ii*"),
	/*16*/	DEFUN("DisposeWindow", kDisposeWindow, "ii*"),
	/*17*/	DEFUN("DrawControl", kDrawControl, "o"),
	/*18*/	DEFUN("HiliteControl", kHiliteControl, "o"),
	/*19*/	DEFUN("EditControl", kEditControl, "ZoZo"),
	/*1a*/	DEFUN("TextSize", kTextSize, "rZrii*r*"),
	/*1b*/	DEFUN("Display", kDisplay, ".*"),
	/*1c*/	DEFUN("GetEvent", kGetEvent, "io"),
	/*1d*/	DEFUN("GlobalToLocal", kGlobalToLocal, "o"),
	/*1e*/	DEFUN("LocalToGlobal", kLocalToGlobal, "o"),
	/*1f*/	DEFUN("MapKeyToDir", kMapKeyToDir, "o"),
	/*20*/	DEFUN("DrawMenuBar", kDrawMenuBar, "i"),
	/*21*/	DEFUN("MenuSelect", kMenuSelect, "oi*"),
	/*22*/	DEFUN("AddMenu", kAddMenu, "rr"),
	/*23*/	DEFUN("DrawStatus", kDrawStatus, "Zri*"),
	/*24*/	DEFUN("Parse", kParse, "ro"),
	/*25*/	DEFUN("Said", kSaid, "Zr"),
	/*26*/	DEFUN("SetSynonyms", kSetSynonyms, "o"),
	/*27*/	DEFUN("HaveMouse", kHaveMouse, ""),
	/*28*/	DEFUN("SetCursor", kSetCursor, "i*"),
	// FIXME: The number 0x28 occurs twice :-)
	/*28*/	DEFUN("MoveCursor", kMoveCursor, "ii*"),
	/*29*/	DEFUN("FOpen", kFOpen, "ri"),
	/*2a*/	DEFUN("FPuts", kFPuts, "ir"),
	/*2b*/	DEFUN("FGets", kFGets, "rii"),
	/*2c*/	DEFUN("FClose", kFClose, "i"),
	/*2d*/	DEFUN("SaveGame", kSaveGame, "rirr*"),
	/*2e*/	DEFUN("RestoreGame", kRestoreGame, "rir*"),
	/*2f*/	DEFUN("RestartGame", kRestartGame, ""),
	/*30*/	DEFUN("GameIsRestarting", kGameIsRestarting, "i*"),
	/*31*/	DEFUN("DoSound", kDoSound, "iIo*"),
	/*32*/	DEFUN("NewList", kNewList, ""),
	/*33*/	DEFUN("DisposeList", kDisposeList, "l"),
	/*34*/	DEFUN("NewNode", kNewNode, ".."),
	/*35*/	DEFUN("FirstNode", kFirstNode, "Zl"),
	/*36*/	DEFUN("LastNode", kLastNode, "l"),
	/*37*/	DEFUN("EmptyList", kEmptyList, "l"),
	/*38*/	DEFUN("NextNode", kNextNode, "n!"),
	/*39*/	DEFUN("PrevNode", kPrevNode, "n"),
	/*3a*/	DEFUN("NodeValue", kNodeValue, "Zn!"),
	/*3b*/	DEFUN("AddAfter", kAddAfter, "lnn"),
	/*3c*/	DEFUN("AddToFront", kAddToFront, "ln"),
	/*3d*/	DEFUN("AddToEnd", kAddToEnd, "ln"),
	/*3e*/	DEFUN("FindKey", kFindKey, "l."),
	/*3f*/	DEFUN("DeleteKey", kDeleteKey, "l."),
	/*40*/	DEFUN("Random", kRandom, "i*"),
	/*41*/	DEFUN("Abs", kAbs, "Oi"),
	/*42*/	DEFUN("Sqrt", kSqrt, "i"),
	/*43*/	DEFUN("GetAngle", kGetAngle, "iiii"),
	/*44*/	DEFUN("GetDistance", kGetDistance, "iiiii*"),
	/*45*/	DEFUN("Wait", kWait, "i"),
	/*46*/	DEFUN("GetTime", kGetTime, "i*"),
	/*47*/	DEFUN("StrEnd", kStrEnd, "r"),
	/*48*/	DEFUN("StrCat", kStrCat, "rr"),
	/*49*/	DEFUN("StrCmp", kStrCmp, "rri*"),
	/*4a*/	DEFUN("StrLen", kStrLen, "Zr"),
	/*4b*/	DEFUN("StrCpy", kStrCpy, "rZri*"),
	/*4c*/	DEFUN("Format", kFormat, "r.*"),
	/*4d*/	DEFUN("GetFarText", kGetFarText, "iir"),
	/*4e*/	DEFUN("ReadNumber", kReadNumber, "r"),
	/*4f*/	DEFUN("BaseSetter", kBaseSetter, "o"),
	/*50*/	DEFUN("DirLoop", kDirLoop, "oi"),
	// Opcode 51 is defined twice for a reason. Older SCI versions
	// call CanBeHere, whereas newer ones its inverse, CantBeHere
	/*51*/	DEFUN("CanBeHere", kCanBeHere, "ol*"),
	/*51*/	DEFUN("CantBeHere", kCanBeHere, "ol*"),
	/*52*/	DEFUN("OnControl", kOnControl, "i*"),
	/*53*/	DEFUN("InitBresen", kInitBresen, "oi*"),
	/*54*/	DEFUN("DoBresen", kDoBresen, "o"),
	/*55*/	DEFUN("DoAvoider", kDoAvoider, "o"),
	/*56*/	DEFUN("SetJump", kSetJump, "oiii"),
	/*57*/	DEFUN("SetDebug", kSetDebug, "i*"),
	/*58*/	NOFUN("InspectObj"),
	/*59*/	NOFUN("ShowSends"),
	/*5a*/	NOFUN("ShowObjs"),
	/*5b*/	NOFUN("ShowFree"),
	/*5c*/	DEFUN("MemoryInfo", kMemoryInfo, "i"),
	/*5d*/	NOFUN("StackUsage"),
	/*5e*/	NOFUN("Profiler"),
	/*5f*/	DEFUN("GetMenu", kGetMenu, "i."),
	/*60*/	DEFUN("SetMenu", kSetMenu, "i.*"),
	/*61*/	DEFUN("GetSaveFiles", kGetSaveFiles, "rrr"),
	/*62*/	DEFUN("GetCWD", kGetCWD, "r"),
	/*63*/	DEFUN("CheckFreeSpace", kCheckFreeSpace, "r"),
	/*64*/	DEFUN("ValidPath", kValidPath, "r"),
	/*65*/	DEFUN("CoordPri", kCoordPri, "i"),
	/*66*/	DEFUN("StrAt", kStrAt, "rii*"),
	/*67*/	DEFUN("DeviceInfo", kDeviceInfo, "i.*"),
	/*68*/	DEFUN("GetSaveDir", kGetSaveDir, ""),
	/*69*/	DEFUN("CheckSaveGame", kCheckSaveGame, ".*"),
	/*6a*/	DEFUN("ShakeScreen", kShakeScreen, "ii*"),
	/*6b*/	DEFUN("FlushResources", kFlushResources, "i"),
	/*6c*/	DEFUN("TimesSin", kTimesSin, "ii"),
	/*6d*/	DEFUN("TimesCos", kTimesCos, "ii"),
	/*6e*/	DEFUN("6e", kTimesSin, "ii"),
	/*6f*/	DEFUN("6f", kTimesCos, "ii"),
	/*70*/	DEFUN("Graph", kGraph, ".*"),
	/*71*/	DEFUN("Joystick", kJoystick, ".*"),
	/*72*/	NOFUN("unknown72"),
	/*73*/	NOFUN("unknown73"),

	// Experimental functions
	/*74*/	DEFUN("FileIO", kFileIO, "i.*"),
	/*(?)*/	DEFUN("Memory", kMemory, "i.*"),
	/*(?)*/	DEFUN("Sort", kSort, "ooo"),
	/*(?)*/	DEFUN("AvoidPath", kAvoidPath, "ii.*"),
	/*(?)*/	DEFUN("Lock", kLock, "iii*"),
	/*(?)*/	DEFUN("Palette", kPalette, "i.*"),
	/*(?)*/	DEFUN("IsItSkip", kIsItSkip, "iiiii"),

	// Non-experimental Functions without a fixed ID

	DEFUN("CosMult", kTimesCos, "ii"),
	DEFUN("SinMult", kTimesSin, "ii"),
	/*(?)*/	DEFUN("CosDiv", kCosDiv, "ii"),
	/*(?)*/	DEFUN("PriCoord", kPriCoord, "i"),
	/*(?)*/	DEFUN("SinDiv", kSinDiv, "ii"),
	/*(?)*/	DEFUN("TimesCot", kTimesCot, "ii"),
	/*(?)*/	DEFUN("TimesTan", kTimesTan, "ii"),
	DEFUN("Message", kMessage, ".*"),
	DEFUN("DoAudio", kDoAudio, ".*"),
	DEFUN("DoSync", kDoSync, ".*"),
	DEFUN("ResCheck", kResCheck, "iii*"),
	DEFUN("SetQuitStr", kSetQuitStr, "r"),
	DEFUN("ShowMovie", kShowMovie, "ri"),
	DEFUN("SetVideoMode", kSetVideoMode, "i"),

	// Special and NOP stuff
	{NULL, k_Unknown, NULL},

	{NULL, NULL, NULL} // Terminator
};

static const char *argtype_description[] = {
	"Undetermined",
	"List",
	"Node",
	"Object",
	"Reference",
	"Arithmetic"
};

Kernel::Kernel(ResourceManager *resmgr, bool isOldSci0) : _resmgr(resmgr) {
	memset(&_selectorMap, 0, sizeof(_selectorMap));	// FIXME: Remove this once/if we C++ify selector_map_t

	loadSelectorNames(isOldSci0);
	mapSelectors();     // Map a few special selectors for later use
	loadOpcodes();
	loadKernelNames();
	mapFunctions();     // Map the kernel functions

	// SCI0 games using old graphics functions (before version 0.000.502) did not have a
	// motionCue selector
	_oldGfxFunctions = (_selectorMap.motionCue == -1 && _resmgr->_sciVersion == SCI_VERSION_0);

	// SCI1 games which use absolute lofs have the egoMoveSpeed selector
	_hasLofsAbsolute = (_selectorMap.egoMoveSpeed != -1 && _resmgr->_sciVersion < SCI_VERSION_1_1);

	printAutoDetectedFeatures();
}

Kernel::~Kernel() {
}

void Kernel::printAutoDetectedFeatures() {
	if (_oldGfxFunctions)
		printf("Kernel auto-detection: game found to be using old graphics functions\n");
	else
		printf("Kernel auto-detection: game found to be using newer graphics functions\n");

	if (_hasLofsAbsolute)
		printf("Kernel auto-detection: game found to be using absolute parameters for lofs\n");
	else
		printf("Kernel auto-detection: game found to be using relative parameters for lofs\n");

	if (_selectorMap.setVol != -1)
		printf("Kernel auto-detection: using SCI1 sound functions\n");
	else if (_selectorMap.nodePtr != -1)
		printf("Kernel auto-detection: using SCI01 sound functions\n");
	else
		printf("Kernel auto-detection: using SCI0 sound functions\n");

	if (_resmgr->_sciVersion == SCI_VERSION_0 && _selectorMap.sightAngle != -1)
		printf("Kernel auto-detection: found SCI0 game using a SCI1 kernel table\n");
}

void Kernel::loadSelectorNames(bool isOldSci0) {
	int count;

	Resource *r = _resmgr->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SNAMES), 0);

	if (!r) // No such resource?
		error("Kernel: Could not retrieve selector names");

	count = READ_LE_UINT16(r->data) + 1; // Counter is slightly off

	for (int i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset);

		Common::String tmp((const char *)r->data + offset + 2, len);
		_selectorNames.push_back(tmp);
		//printf("%s\n", tmp.c_str());	// debug

		// Early SCI versions used the LSB in the selector ID as a read/write
		// toggle. To compensate for that, we add every selector name twice.
		if (isOldSci0)
			_selectorNames.push_back(tmp);
	}
}

bool Kernel::loadOpcodes() {
	int count, i = 0;
	Resource* r = _resmgr->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_OPCODES), 0);

	_opcodes.clear();

	// if the resource couldn't be loaded, leave
	if (r == NULL) {
		warning("unable to load vocab.%03d", VOCAB_RESOURCE_OPCODES);
		return false;
	}

	count = READ_LE_UINT16(r->data);

	_opcodes.resize(count);
	for (i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset) - 2;
		_opcodes[i].type = READ_LE_UINT16(r->data + offset + 2);
		// QFG3 has empty opcodes
		_opcodes[i].name = len > 0 ? Common::String((char *)r->data + offset + 4, len) : "Dummy";
	}

	return true;
}

// Allocates a set amount of memory for a specified use and returns a handle to it.
reg_t kalloc(EngineState *s, const char *type, int space) {
	reg_t reg;

	s->seg_manager->alloc_hunk_entry(type, space, &reg);
	debugC(2, kDebugLevelMemory, "Allocated %d at hunk %04x:%04x (%s)\n", space, PRINT_REG(reg), type);

	return reg;
}

// Returns a pointer to the memory indicated by the specified handle
byte *kmem(EngineState *s, reg_t handle) {
	HunkTable *ht = (HunkTable *)GET_SEGMENT(*s->seg_manager, handle.segment, MEM_OBJ_HUNK);

	if (!ht || !ht->isValidEntry(handle.offset)) {
		warning("Error: kmem() with invalid handle");
		return NULL;
	}

	return (byte *)ht->_table[handle.offset].mem;
}

// Frees the specified handle. Returns 0 on success, 1 otherwise.
int kfree(EngineState *s, reg_t handle) {
	s->seg_manager->free_hunk_entry(handle);

	return 0;
}

static void kernel_compile_signature(const char **s) {
	const char *src = *s;
	char *result;
	int ellipsis = 0;
	char v;
	int index = 0;

	if (!src)
		return; // NULL signature: Nothing to do

	result = (char*)malloc(strlen(*s) + 1);

	while (*src) {
		char c;
		v = 0;

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

			case KSIG_SPEC_ALLOW_INV:
				v |= KSIG_ALLOW_INV;
				break;

			case KSIG_SPEC_ELLIPSIS:
				v |= KSIG_ELLIPSIS;
				ellipsis = 1;
				break;

			default: {
				error("INTERNAL ERROR when compiling kernel function signature '%s': (%02x) not understood (aka"
				          " '%c')\n", *s, c, c);
			}
			}
		} while (*src && (*src == KSIG_SPEC_ALLOW_INV || *src == KSIG_SPEC_ELLIPSIS || (c < 'a' && c != KSIG_SPEC_ANY)));

		// To handle sum types
		result[index++] = v;
	}

	result[index] = 0;
	*s = result; // Write back
}

void Kernel::mapFunctions() {
	int mapped = 0;
	int ignored = 0;
	uint functions_nr = getKernelNamesSize();
	uint max_functions_nr = (_resmgr->_sciVersion == SCI_VERSION_0) ? 0x72 : 0x7b;

	if (functions_nr < max_functions_nr) {
		warning("SCI version believed to have %d kernel"
		        " functions, but only %d reported-- filling up remaining %d",
		          max_functions_nr, functions_nr, max_functions_nr - functions_nr);

		functions_nr = max_functions_nr;
	}

	_kernelFuncs.resize(functions_nr);

	for (uint functnr = 0; functnr < functions_nr; functnr++) {
		int found = -1;

		// First, get the name, if known, of the kernel function with number functnr
		Common::String sought_name;
		if (functnr < getKernelNamesSize())
			sought_name = getKernelName(functnr);

		// If the name is known, look it up in kfunct_mappers. This table
		// maps kernel func names to actual function (pointers).
		if (!sought_name.empty()) {
			for (uint seeker = 0; (found == -1) && kfunct_mappers[seeker].name; seeker++)
				if (sought_name == kfunct_mappers[seeker].name)
					found = seeker; // Found a kernel function with the correct name!
		}

		// Reset the table entry
		_kernelFuncs[functnr].fun = NULL;
		_kernelFuncs[functnr].signature = NULL;
		_kernelFuncs[functnr].orig_name = sought_name;

		if (found == -1) {
			if (!sought_name.empty()) {
				// No match but a name was given -> NOP
				warning("Kernel function %s[%x] unmapped", sought_name.c_str(), functnr);
				_kernelFuncs[functnr].fun = kNOP;
			} else {
				// No match and no name was given -> must be an unknown opcode
				warning("Flagging kernel function %x as unknown", functnr);
				_kernelFuncs[functnr].fun = k_Unknown;
			}
		} else {
			// A match in kfunct_mappers was found
			if (kfunct_mappers[found].fun) {
				_kernelFuncs[functnr].fun = kfunct_mappers[found].fun;
				_kernelFuncs[functnr].signature = kfunct_mappers[found].signature;
				kernel_compile_signature(&(_kernelFuncs[functnr].signature));
				++mapped;
			} else {
				//warning("Ignoring function %s\n", kfunct_mappers[found].name);
				++ignored;
			}
		}
	} // for all functions requesting to be mapped

	debugC(2, kDebugLevelVM, "Handled %d/%d kernel functions, mapping %d and ignoring %d.\n", 
				mapped + ignored, getKernelNamesSize(), mapped, ignored);

	return;
}

int determine_reg_type(EngineState *s, reg_t reg, bool allow_invalid) {
	MemObject *mobj;
	int type = 0;

	if (!reg.segment) {
		type = KSIG_ARITHMETIC;
		if (!reg.offset)
			type |= KSIG_NULL;

		return type;
	}

	if ((reg.segment >= s->seg_manager->_heap.size()) || !s->seg_manager->_heap[reg.segment])
		return 0; // Invalid

	mobj = s->seg_manager->_heap[reg.segment];

	switch (mobj->getType()) {
	case MEM_OBJ_SCRIPT:
		if (reg.offset <= (*(Script *)mobj).buf_size && reg.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET
		        && RAW_IS_OBJECT((*(Script *)mobj).buf + reg.offset)) {
			int idx = RAW_GET_CLASS_INDEX((Script *)mobj, reg);
			if (idx >= 0 && (uint)idx < (*(Script *)mobj)._objects.size())
				return KSIG_OBJECT;
			else
				return KSIG_REF;
		} else
			return KSIG_REF;

	case MEM_OBJ_CLONES:
		type = KSIG_OBJECT;
		break;

	case MEM_OBJ_LOCALS:
	case MEM_OBJ_STACK:
	case MEM_OBJ_SYS_STRINGS:
	case MEM_OBJ_DYNMEM:
		type = KSIG_REF;
		break;

	case MEM_OBJ_LISTS:
		type = KSIG_LIST;
		break;

	case MEM_OBJ_NODES:
		type = KSIG_NODE;
		break;

	default:
		return 0;
	}

	if (!allow_invalid && !mobj->isValidOffset(reg.offset))
		type |= KSIG_INVALID;
	return type;
}

const char *kernel_argtype_description(int type) {
	type &= ~KSIG_INVALID;

	return argtype_description[sci_ffs(type)];
}

bool kernel_matches_signature(EngineState *s, const char *sig, int argc, const reg_t *argv) {
	// Always "match" if no signature is given
	if (!sig)
		return true;

	while (*sig && argc) {
		if ((*sig & KSIG_ANY) != KSIG_ANY) {
			int type = determine_reg_type(s, *argv, *sig & KSIG_ALLOW_INV);

			if (!type) {
				warning("[KERN] Could not determine type of ref %04x:%04x; failing signature check", PRINT_REG(*argv));
				return false;
			}

			if (type & KSIG_INVALID) {
				warning("[KERN] ref %04x:%04x was determined to be a %s, but the reference itself is invalid",
				          PRINT_REG(*argv), kernel_argtype_description(type));
				return false;
			}

			if (!(type & *sig))
				return false;

		}
		if (!(*sig & KSIG_ELLIPSIS))
			++sig;
		++argv;
		--argc;
	}

	if (argc)
		return false; // Too many arguments
	else
		return (*sig == 0 || (*sig & KSIG_ELLIPSIS));
}

static void *_kernel_dereference_pointer(EngineState *s, reg_t pointer, int entries, int align) {
	int maxsize;
	void *retval = s->seg_manager->dereference(pointer, &maxsize);

	if (!retval)
		return NULL;

	if (pointer.offset & (align - 1)) {
		warning("Unaligned pointer read: %04x:%04x expected with %d alignment", PRINT_REG(pointer), align);
		return NULL;
	}

	if (entries > maxsize) {
		warning("Trying to dereference pointer %04x:%04x beyond end of segment", PRINT_REG(pointer));
		return NULL;
	}
	return retval;

}

byte *kernel_dereference_bulk_pointer(EngineState *s, reg_t pointer, int entries) {
	return (byte*)_kernel_dereference_pointer(s, pointer, entries, 1);
}

reg_t *kernel_dereference_reg_pointer(EngineState *s, reg_t pointer, int entries) {
	return (reg_t*)_kernel_dereference_pointer(s, pointer, entries, sizeof(reg_t));
}

void Kernel::setDefaultKernelNames() {
	bool isSci0 = (_resmgr->_sciVersion == SCI_VERSION_0);
	int offset = 0;

	// Check if we have a SCI01 game which uses a SCI1 kernel table (e.g. the KQ1 demo
	// and full version). We do this by checking if the sightAngle selector exists, as no
	// SCI0 game seems to have it
	if (_selectorMap.sightAngle != -1 && isSci0)
		isSci0 = false;

	_kernelNames.resize(SCI_KNAMES_DEFAULT_ENTRIES_NR + (isSci0 ? 4 : 0));
	for (int i = 0; i < SCI_KNAMES_DEFAULT_ENTRIES_NR; i++) {
		// In SCI0, Platform was DoAvoider
		if (!strcmp(sci_default_knames[i], "Platform") && isSci0) {
			_kernelNames[i + offset] = "DoAvoider";
			continue;
		}

		_kernelNames[i + offset] = sci_default_knames[i];

		// SCI0 has 4 extra functions between SetCursor (0x28) and Savegame
		if (!strcmp(sci_default_knames[i], "SetCursor") && isSci0) {
			_kernelNames[i + 1] = "FOpen";
			_kernelNames[i + 2] = "FPuts";
			_kernelNames[i + 3] = "FGets";
			_kernelNames[i + 4] = "FClose";
			offset = 4;
		}
	}

	if (_resmgr->_sciVersion == SCI_VERSION_1_1) {
		// KQ6CD calls unimplemented function 0x26
		_kernelNames[0x26] = "Dummy";
	}
}

#ifdef ENABLE_SCI32
static void vocab_get_knames11(ResourceManager *resmgr, Common::StringList &names) {
/*
 999.voc format for SCI1.1 games:
	[b] # of kernel functions
	[w] unknown
	[offset to function name info]
		...
    {[w name-len][function name]}
		...
*/
	//unsigned int size = 64, pos = 3;
	int len;
	Resource *r = resmgr->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_KNAMES), 0);
	if(r == NULL) // failed to open vocab.999 (happens with SCI1 demos)
		return; // FIXME: should return a default table for this engine
	const byte nCnt = *r->data;

	names.resize(nCnt);
	for (int i = 0; i < nCnt; i++) {
		int off = READ_LE_UINT16(r->data + 2 * i + 2);
		len = READ_LE_UINT16(r->data + off);
		names[i] = Common::String((char *)r->data + off + 2, len);
	}
}
#endif

bool Kernel::loadKernelNames() {
	_kernelNames.clear();

	switch (_resmgr->_sciVersion) {
	case SCI_VERSION_0:
	case SCI_VERSION_01:
	case SCI_VERSION_01_VGA:
	case SCI_VERSION_01_VGA_ODD:
	case SCI_VERSION_1:
	case SCI_VERSION_1_1:
		setDefaultKernelNames();
		break;
#ifdef ENABLE_SCI32
	case SCI_VERSION_32:
		vocab_get_knames11(_resmgr, _kernelNames);
#endif
		break;
	default:
		break;
	}

	return true;
}

} // End of namespace Sci
