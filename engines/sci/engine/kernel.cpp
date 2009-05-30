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


enum KernelFunctionType {
	KF_NEW = 1,
	KF_NONE = -1, /**< No mapping, but name is known */
	KF_TERMINATOR = -42 /**< terminates kfunct_mappers */
};

struct SciKernelFunction {
	KernelFunctionType type;
	const char *name;
	kfunct *fun; /* The actual function */
	const char *signature;  /* kfunct signature */
};

static const int sci_max_allowed_unknown_kernel_functions[] = {
	0,
	0x72, // SCI0
	0x7b, // SCI01/EGA
	0x7b, // SCI01/VGA
	0x7b, // SCI01/VGA ODD
	0x7b, // SCI1/EARLY
	0x7b, // SCI1/LATE
	0x7b, // SCI1.1
#ifdef ENABLE_SCI32
	0x0, // SCI32
#endif
};

#define DEFUN(nm, cname, sig) {KF_NEW, nm, cname, sig}
#define NOFUN(nm) {KF_NONE, nm, NULL, NULL}

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
	/*72*/	NOFUN(NULL),
	/*73*/	NOFUN(NULL),

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
	{KF_NEW, NULL, k_Unknown, NULL},

	{KF_TERMINATOR, NULL, NULL, NULL} // Terminator
};

static const char *argtype_description[] = { "Undetermined (WTF?)", "List", "Node", "Object", "Reference", "Arithmetic" };

int kernel_oops(EngineState *s, const char *file, int line, const char *reason) {
	sciprintf("Kernel Oops in file %s, line %d: %s\n", file, line, reason);
	warning("Kernel Oops in file %s, line %d: %s", file, line, reason);
	script_debug_flag = script_error_flag = 1;
	return 0;
}

// Allocates a set amount of memory for a specified use and returns a handle to it.
reg_t kalloc(EngineState *s, const char *type, int space) {
	reg_t reg;

	s->seg_manager->alloc_hunk_entry(type, space, &reg);
	debugC(2, kDebugLevelMemory, "Allocated %d at hunk %04x:%04x (%s)\n", space, PRINT_REG(reg), type);

	return reg;
}

bool has_kernel_function(EngineState *s, const char *kname) {
	Common::StringList::const_iterator it
		= Common::find(s->_vocabulary->_kernelNames.begin(), s->_vocabulary->_kernelNames.end(), kname);

	return (it != s->_vocabulary->_kernelNames.end());
}

// Returns a pointer to the memory indicated by the specified handle
byte *kmem(EngineState *s, reg_t handle) {
	HunkTable *ht = (HunkTable *)GET_SEGMENT(*s->seg_manager, handle.segment, MEM_OBJ_HUNK);

	if (!ht || !ht->isValidEntry(handle.offset)) {
		warning("Error: kmem() with invalid handle\n");
		return NULL;
	}

	return (byte *)ht->_table[handle.offset].mem;
}

// Frees the specified handle. Returns 0 on success, 1 otherwise.
int kfree(EngineState *s, reg_t handle) {
	s->seg_manager->free_hunk_entry(handle);

	return 0;
}

void kernel_compile_signature(const char **s) {
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
			error("Failed compiling kernel function signature '%s': non-terminal ellipsis '%c'\n", *s, *src);
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

int script_map_kernel(EngineState *s) {
	int mapped = 0;
	int ignored = 0;
	uint functions_nr = s->_vocabulary->_kernelNames.size();
	uint max_functions_nr = sci_max_allowed_unknown_kernel_functions[s->resmgr->_sciVersion];

	if (functions_nr < max_functions_nr) {
		warning("SCI version believed to have %d kernel"
		        " functions, but only %d reported-- filling up remaining %d",
		          max_functions_nr, functions_nr, max_functions_nr - functions_nr);

		functions_nr = max_functions_nr;
	}

	s->_kfuncTable.resize(functions_nr);

	for (uint functnr = 0; functnr < functions_nr; functnr++) {
		int seeker, found = -1;
		Common::String sought_name;

		if (functnr < s->_vocabulary->_kernelNames.size())
			sought_name = s->_vocabulary->_kernelNames[functnr];

		if (!sought_name.empty())
			for (seeker = 0; (found == -1) && kfunct_mappers[seeker].type != KF_TERMINATOR; seeker++)
				if (kfunct_mappers[seeker].name && sought_name == kfunct_mappers[seeker].name)
					found = seeker; // Found a kernel function with the same name!

		if (found == -1) {
			if (!sought_name.empty()) {
				warning("Kernel function %s[%x] unmapped", s->_vocabulary->_kernelNames[functnr].c_str(), functnr);
				s->_kfuncTable[functnr].fun = kNOP;
			} else {
				warning("Flagging kernel function %x as unknown", functnr);
				s->_kfuncTable[functnr].fun = k_Unknown;
			}

			s->_kfuncTable[functnr].signature = NULL;
			s->_kfuncTable[functnr].orig_name = sought_name;
		} else
			switch (kfunct_mappers[found].type) {
			case KF_NONE:
				s->_kfuncTable[functnr].signature = NULL;
				++ignored;
				break;

			case KF_NEW:
				s->_kfuncTable[functnr].fun = kfunct_mappers[found].fun;
				s->_kfuncTable[functnr].signature = kfunct_mappers[found].signature;
				s->_kfuncTable[functnr].orig_name.clear();
				kernel_compile_signature(&(s->_kfuncTable[functnr].signature));
				++mapped;
				break;
			case KF_TERMINATOR:
				error("Unexpectedly encountered KF_TERMINATOR");
				break;
			}

	} // for all functions requesting to be mapped

	sciprintf("Handled %d/%d kernel functions, mapping %d", mapped + ignored, s->_vocabulary->_kernelNames.size(), mapped);
	if (ignored)
		sciprintf(" and ignoring %d", ignored);
	sciprintf(".\n");

	return 0;
}

int determine_reg_type(EngineState *s, reg_t reg, int allow_invalid) {
	MemObject *mobj;

	if (!reg.segment) {
		if (!reg.offset)
			return KSIG_ARITHMETIC | KSIG_NULL;

		return KSIG_ARITHMETIC;
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
		if (allow_invalid || ((CloneTable *)mobj)->isValidEntry(reg.offset))
			return KSIG_OBJECT;
		else
			return KSIG_OBJECT | KSIG_INVALID;

	case MEM_OBJ_LOCALS:
		if (allow_invalid || reg.offset < (*(LocalVariables *)mobj)._locals.size() * sizeof(reg_t))
			return KSIG_REF;
		else
			return KSIG_REF | KSIG_INVALID;

	case MEM_OBJ_STACK:
		if (allow_invalid || reg.offset < (*(DataStack *)mobj).nr * sizeof(reg_t))
			return KSIG_REF;
		else
			return KSIG_REF | KSIG_INVALID;

	case MEM_OBJ_SYS_STRINGS:
		if (allow_invalid || (reg.offset < SYS_STRINGS_MAX
		                      && (*(SystemStrings *)mobj).strings[reg.offset].name))
			return KSIG_REF;
		else
			return KSIG_REF | KSIG_INVALID;

	case MEM_OBJ_LISTS:
		if (allow_invalid || ((ListTable *)mobj)->isValidEntry(reg.offset))
			return KSIG_LIST;
		else
			return KSIG_LIST | KSIG_INVALID;

	case MEM_OBJ_NODES:
		if (allow_invalid || ((NodeTable *)mobj)->isValidEntry(reg.offset))
			return KSIG_NODE;
		else
			return KSIG_NODE | KSIG_INVALID;

	case MEM_OBJ_DYNMEM:
		if (allow_invalid || reg.offset < (*(DynMem *)mobj)._size)
			return KSIG_REF;
		else
			return KSIG_REF | KSIG_INVALID;

	default:
		return 0;

	}
}

const char *kernel_argtype_description(int type) {
	type &= ~KSIG_INVALID;

	return argtype_description[sci_ffs(type)];
}

int kernel_matches_signature(EngineState *s, const char *sig, int argc, reg_t *argv) {
	if (!sig)
		return 1;

	while (*sig && argc) {
		if ((*sig & KSIG_ANY) != KSIG_ANY) {
			int type = determine_reg_type(s, *argv, *sig & KSIG_ALLOW_INV);

			if (!type) {
				sciprintf("[KERN] Could not determine type of ref %04x:%04x; failing signature check\n", PRINT_REG(*argv));
				return 0;
			}

			if (type & KSIG_INVALID) {
				sciprintf("[KERN] ref %04x:%04x was determined to be a %s, but the reference itself is invalid\n",
				          PRINT_REG(*argv), kernel_argtype_description(type));
				return 0;
			}

			if (!(type & *sig))
				return 0;

		}
		if (!(*sig & KSIG_ELLIPSIS))
			++sig;
		++argv;
		--argc;
	}

	if (argc)
		return 0; // Too many arguments
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

} // End of namespace Sci
