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

#ifndef SCI_ENGINE_KERNEL_H
#define SCI_ENGINE_KERNEL_H

#include "common/scummsys.h"
#include "common/debug.h"
#include "common/rect.h"

#include "sci/uinput.h"
#include "sci/vocabulary.h"

namespace Sci {

struct Node;	// from vm.h
struct List;	// from vm.h

#define AVOIDPATH_DYNMEM_STRING "AvoidPath polyline"
//#define DEBUG_PARSER	// enable for parser debugging

struct opcode {
	int type;
	Common::String name;
};

/* Generic description: */
typedef reg_t KernelFunc(EngineState *s, int funct_nr, int argc, reg_t *argv);

struct KernelFuncWithSignature {
	KernelFunc *fun; /**< The actual function */
	const char *signature;  /**< KernelFunc signature */
	Common::String orig_name; /**< Original name, in case we couldn't map it */
};

class Kernel {
public:
	Kernel(ResourceManager *resmgr, bool isOldSci0);
	~Kernel();

	uint getOpcodesSize() const { return _opcodes.size(); }
	const opcode &getOpcode(uint opcode) const { return _opcodes[opcode]; }

	uint getSelectorNamesSize() const { return _selectorNames.size(); }
	const Common::String &getSelectorName(uint selector) const { return _selectorNames[selector]; }

	uint getKernelNamesSize() const { return _kernelNames.size(); }
	const Common::String &getKernelName(uint number) const { return _kernelNames[number]; }

	/**
	 * Determines the selector ID of a selector by its name
	 * @param selectorName Name of the selector to look up
	 * @return The appropriate selector ID, or -1 on error
	 */
	int findSelector(const char *selectorName) const;

	/**
	 * Detects whether a particular kernel function is required in the game
	 * @param functionName The name of the desired kernel function
	 * @return True if the kernel function is listed in the kernel table, false otherwise
	*/
	bool hasKernelFunction(const char *functionName) const;

	/**
	 * Applies to all versions before 0.000.502
	 * Old SCI versions used to interpret the third DrawPic() parameter inversely,
	 * with the opposite default value (obviously).
	 * Also, they used 15 priority zones from 42 to 200 instead of 14 priority
	 * zones from 42 to 190.
	 */
	bool usesOldGfxFunctions() const { return _oldGfxFunctions; }

	// Script dissection/dumping functions
	void dissectScript(int scriptNumber, Vocabulary *vocab);
	void dumpScriptObject(char *data, int seeker, int objsize);
	void dumpScriptClass(char *data, int seeker, int objsize);

	selector_map_t _selectorMap; /**< Shortcut list for important selectors */
	Common::Array<KernelFuncWithSignature> _kernelFuncs; /**< Table of kernel functions */

private:
	/**
	 * Loads the kernel function names.
	 *
	 * This function reads the kernel function name table from resource_map,
	 * and fills the _kernelNames array with them.
	 * The resulting list has the same format regardless of the format of the
	 * name table of the resource (the format changed between version 0 and 1).
	 * @return true on success, false on failure
	 */
	bool loadKernelNames();

	/**
	 * Sets the default kernel function names, based on the SCI version used
	 */
	void setDefaultKernelNames();

	/**
	* Loads the kernel selector names.
	*/
	void loadSelectorNames(bool isOldSci0);

	/**
	 * Maps special selectors
	 */
	void mapSelectors();

	/**
	 * Maps kernel functions
	 */
	void mapFunctions();

	/**
	 * Loads the opcode names (only used for debugging).
	 * @return true on success, false on failure
	 */
	bool loadOpcodes();

	ResourceManager *_resmgr;
	bool _oldGfxFunctions;

	// Kernel-related lists
	/**
	 * List of opcodes, loaded from vocab.998. This list is only used for debugging
	 * purposes, as we hardcode the list of opcodes in the sci_opcodes enum (script.h)
	 */
	Common::Array<opcode> _opcodes;
	Common::StringList _selectorNames;
	Common::StringList _kernelNames;
};

/******************** Selector functionality ********************/

enum SelectorInvocation {
	kStopOnInvalidSelector = 0,
	kContinueOnInvalidSelector = 1
};

#define GET_SEL32(_o_, _slc_) read_selector(s, _o_, s->_kernel->_selectorMap._slc_, __FILE__, __LINE__)
#define GET_SEL32V(_o_, _slc_) (GET_SEL32(_o_, _slc_).offset)
#define GET_SEL32SV(_o_, _slc_) ((int16)(GET_SEL32(_o_, _slc_).offset))
/* Retrieves a selector from an object
** Parameters: (reg_t) object: The address of the object which the selector should be read from
**             (selector_name) selector: The selector to read
** Returns   : (int16/uint16/reg_t) The selector value
** This macro halts on error. 'selector' must be a selector name registered in vm.h's
** selector_map_t and mapped in script.c.
*/

#define PUT_SEL32(_o_, _slc_, _val_) write_selector(s, _o_, s->_kernel->_selectorMap._slc_, _val_, __FILE__, __LINE__)
#define PUT_SEL32V(_o_, _slc_, _val_) write_selector(s, _o_, s->_kernel->_selectorMap._slc_, make_reg(0, _val_), __FILE__, __LINE__)
/* Writes a selector value to an object
** Parameters: (reg_t) object: The address of the object which the selector should be written to
**             (selector_name) selector: The selector to read
**             (int16) value: The value to write
** Returns   : (void)
** This macro halts on error. 'selector' must be a selector name registered in vm.h's
** selector_map_t and mapped in script.c.
*/


#define INV_SEL(_object_, _selector_, _noinvalid_) \
	s, _object_,  s->_kernel->_selectorMap._selector_, _noinvalid_, funct_nr, argv, argc, __FILE__, __LINE__
/* Kludge for use with invoke_selector(). Used for compatibility with compilers that can't
** handle vararg macros.
*/


reg_t read_selector(EngineState *s, reg_t object, Selector selector_id, const char *fname, int line);
void write_selector(EngineState *s, reg_t object, Selector selector_id, reg_t value, const char *fname, int line);
int invoke_selector(EngineState *s, reg_t object, int selector_id, SelectorInvocation noinvalid, int kfunct,
	StackPtr k_argp, int k_argc, const char *fname, int line, int argc, ...);


/******************** Text functionality ********************/
/**
 * Looks up text referenced by scripts
 * SCI uses two values to reference to text: An address, and an index. The address
 * determines whether the text should be read from a resource file, or from the heap,
 * while the index either refers to the number of the string in the specified source,
 * or to a relative position inside the text.
 *
 * @param s The current state
 * @param address The address to look up
 * @param index The relative index
 * @return The referenced text, or NULL on error.
 */
char *kernel_lookup_text(EngineState *s, reg_t address, int index);


/******************** Debug functionality ********************/
/**
 * Checks whether a heap address contains an object
 * @param s The current state
 * @parm obj The address to check
 * @return True if it is an object, false otherwise
 */
bool is_object(EngineState *s, reg_t obj);

/******************** Kernel function parameter macros ********************/

/* Returns the parameter value or (alt) if not enough parameters were supplied */
/**
 * Dereferences a heap pointer
 * @param s The state to operate on
 * @param pointer The pointer to dereference
 * @parm entries The number of values expected (for checking; use 0 for strings)
 * @return A physical reference to the address pointed to, or NULL on error or
 * if not enugh entries were available.
 * reg_t dereferenciation also assures alignedness of data.
 */
reg_t *kernel_dereference_reg_pointer(EngineState *s, reg_t pointer, int entries);
byte *kernel_dereference_bulk_pointer(EngineState *s, reg_t pointer, int entries);
#define kernel_dereference_char_pointer(state, pointer, entries) (char*)kernel_dereference_bulk_pointer(state, pointer, entries)

/******************** Priority macros/functions ********************/
/**
 * Finds the position of the priority band specified
 * Parameters: (EngineState *) s: State to search in
 * (int) band: Band to look for
 * Returns   : (int) Offset at which the band starts
 */
int _find_priority_band(EngineState *s, int band);

/**
 * Does the opposite of _find_priority_band
 * @param s Engine state
 * @param y Coordinate to check
 * @return The priority band y belongs to
 */
int _find_view_priority(EngineState *s, int y);

#define SCI0_VIEW_PRIORITY_14_ZONES(y) (((y) < s->priority_first)? 0 : (((y) >= s->priority_last)? 14 : 1\
	+ ((((y) - s->priority_first) * 14) / (s->priority_last - s->priority_first))))

#define SCI0_PRIORITY_BAND_FIRST_14_ZONES(nr) ((((nr) == 0)? 0 :  \
	((s->priority_first) + (((nr)-1) * (s->priority_last - s->priority_first)) / 14)))

#define SCI0_VIEW_PRIORITY(y) (((y) < s->priority_first)? 0 : (((y) >= s->priority_last)? 14 : 1\
	+ ((((y) - s->priority_first) * 15) / (s->priority_last - s->priority_first))))

#define SCI0_PRIORITY_BAND_FIRST(nr) ((((nr) == 0)? 0 :  \
	((s->priority_first) + (((nr)-1) * (s->priority_last - s->priority_first)) / 15)))



/******************** Dynamic view list functions ********************/
/**
 * Determines the base rectangle of the specified view object
 * @param s The state to use
 * @param object The object to set
 * @return The absolute base rectangle
 */
Common::Rect set_base(EngineState *s, reg_t object);

/**
 * Determines the now-seen rectangle of a view object
 * @param s The state to use
 * @param object The object to check
 * @param clip Flag to determine wheter priority band clipping 
 * should be performed
 * @return The absolute rectangle describing the now-seen area.
 */
extern Common::Rect get_nsrect(EngineState *s, reg_t object, byte clip);

/**
 * Removes all views in anticipation of a new window or text 
 */
void _k_dyn_view_list_prepare_change(EngineState *s);

/**
 * Redraws all views after a new window or text was added 
 */
void _k_dyn_view_list_accept_change(EngineState *s);




/******************** Misc functions ********************/

/**
 * Get all sound events, apply their changes to the heap 
 */
void process_sound_events(EngineState *s);

/**
 * Resolves an address into a list node
 * @param s The state to operate on
 * @param addr The address to resolve
 * @return The list node referenced, or NULL on error
 */
Node *lookup_node(EngineState *s, reg_t addr);

/**
 * Resolves a list pointer to a list
 * @param s The state to operate on
 * @param addr The address to resolve
 * @return The list referenced, or NULL on error
 */
List *lookup_list(EngineState *s, reg_t addr);


/******************** Constants ********************/

/* Maximum length of a savegame name (including terminator character) */
#define SCI_MAX_SAVENAME_LENGTH 0x24

/* Flags for the signal selector */
#define _K_VIEW_SIG_FLAG_STOP_UPDATE    0x0001
#define _K_VIEW_SIG_FLAG_UPDATED        0x0002
#define _K_VIEW_SIG_FLAG_NO_UPDATE      0x0004
#define _K_VIEW_SIG_FLAG_HIDDEN         0x0008
#define _K_VIEW_SIG_FLAG_FIX_PRI_ON     0x0010
#define _K_VIEW_SIG_FLAG_ALWAYS_UPDATE  0x0020
#define _K_VIEW_SIG_FLAG_FORCE_UPDATE   0x0040
#define _K_VIEW_SIG_FLAG_REMOVE         0x0080
#define _K_VIEW_SIG_FLAG_FROZEN         0x0100
#define _K_VIEW_SIG_FLAG_IS_EXTRA       0x0200
#define _K_VIEW_SIG_FLAG_HIT_OBSTACLE   0x0400
#define _K_VIEW_SIG_FLAG_DOESNT_TURN    0x0800
#define _K_VIEW_SIG_FLAG_NO_CYCLER      0x1000
#define _K_VIEW_SIG_FLAG_IGNORE_HORIZON 0x2000
#define _K_VIEW_SIG_FLAG_IGNORE_ACTOR   0x4000
#define _K_VIEW_SIG_FLAG_DISPOSE_ME     0x8000

#define _K_VIEW_SIG_FLAG_STOPUPD 0x20000000 /* View has been stop-updated */


/* Sound status */
#define _K_SOUND_STATUS_STOPPED 0
#define _K_SOUND_STATUS_INITIALIZED 1
#define _K_SOUND_STATUS_PAUSED 2
#define _K_SOUND_STATUS_PLAYING 3



/* Kernel optimization flags */
#define KERNEL_OPT_FLAG_GOT_EVENT (1<<0)
#define KERNEL_OPT_FLAG_GOT_2NDEVENT (1<<1)


/******************** Kernel functions ********************/

// New kernel functions
reg_t kStrLen(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetFarText(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kReadNumber(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kStrCat(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kStrCmp(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetSynonyms(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kLock(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kPalette(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kNumCels(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kNumLoops(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDrawCel(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kCoordPri(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kPriCoord(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kShakeScreen(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetCursor(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kMoveCursor(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kShow(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kPicNotValid(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kOnControl(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDrawPic(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetPort(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetPort(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kNewWindow(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDisposeWindow(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kCelWide(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kCelHigh(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetJump(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDirLoop(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDoAvoider(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetAngle(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetDistance(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kRandom(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kAbs(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSqrt(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kTimesSin(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kTimesCos(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kCosMult(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSinMult(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kTimesTan(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kTimesCot(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kCosDiv(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSinDiv(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kValidPath(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kFOpen(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kFPuts(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kFGets(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kFClose(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kMapKeyToDir(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGlobalToLocal(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kLocalToGlobal(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kWait(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kRestartGame(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDeviceInfo(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetEvent(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kCheckFreeSpace(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kFlushResources(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetSaveFiles(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetDebug(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kCheckSaveGame(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSaveGame(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kRestoreGame(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kFileIO(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetTime(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kHaveMouse(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kJoystick(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGameIsRestarting(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetCWD(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSort(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kStrEnd(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kMemory(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kAvoidPath(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kParse(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSaid(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kStrCpy(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kStrAt(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kEditControl(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDrawControl(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kHiliteControl(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kClone(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDisposeClone(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kCanBeHere(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetNowSeen(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kInitBresen(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDoBresen(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kBaseSetter(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kAddToPic(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kAnimate(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDisplay(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGraph(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kFormat(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDoSound(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kAddMenu(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetMenu(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetMenu(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDrawStatus(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDrawMenuBar(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kMenuSelect(EngineState *s, int funct_nr, int argc, reg_t *argv);

reg_t kLoad(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kUnLoad(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kScriptID(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDisposeScript(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kIsObject(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kRespondsTo(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kNewList(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDisposeList(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kNewNode(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kFirstNode(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kLastNode(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kEmptyList(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kNextNode(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kPrevNode(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kNodeValue(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kAddAfter(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kAddToFront(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kAddToEnd(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kFindKey(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDeleteKey(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kMemoryInfo(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kGetSaveDir(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kTextSize(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kIsItSkip(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kMessage(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDoAudio(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kDoSync(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kResCheck(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetQuitStr(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kShowMovie(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t kSetVideoMode(EngineState *s, int funct_nr, int argc, reg_t *argv);
reg_t k_Unknown(EngineState *s, int funct_nr, int argc, reg_t *argv);

// The Unknown/Unnamed kernel function
reg_t kstub(EngineState *s, int funct_nr, int argc, reg_t *argv);
// for unimplemented kernel functions
reg_t kNOP(EngineState *s, int funct_nr, int argc, reg_t *argv);
// for kernel functions that don't do anything
reg_t kFsciEmu(EngineState *s, int funct_nr, int argc, reg_t *argv);
// Emulating "old" kernel functions on the heap


} // End of namespace Sci

#endif // SCI_ENGIENE_KERNEL_H
