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
#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/engine/vm.h"

namespace Sci {

struct Node;	// from vm.h
struct List;	// from vm.h

#define AVOIDPATH_DYNMEM_STRING "AvoidPath polyline"
//#define DEBUG_PARSER	// enable for parser debugging

/* Generic description: */
typedef reg_t KernelFunc(EngineState *s, int, int argc, reg_t *argv);

struct KernelFuncWithSignature {
	KernelFunc *fun; /**< The actual function */
	const char *signature;  /**< KernelFunc signature */
	Common::String orig_name; /**< Original name, in case we couldn't map it */
};

enum AutoDetectedFeatures {
	kFeatureOldScriptHeader = 1 << 0,
	kFeatureOldGfxFunctions = 1 << 1
};

class Kernel {
public:
	/**
	 * Initializes the SCI kernel
	 */
	Kernel(ResourceManager *resMan);
	~Kernel();

	uint getSelectorNamesSize() const;
	const Common::String &getSelectorName(uint selector) const;

	uint getKernelNamesSize() const;
	const Common::String &getKernelName(uint number) const;

	/**
	 * Determines the selector ID of a selector by its name
	 * @param selectorName Name of the selector to look up
	 * @return The appropriate selector ID, or -1 on error
	 */
	int findSelector(const char *selectorName) const;

	/**
	 * Applies to all versions before 0.000.502
	 * Old SCI versions used to interpret the third DrawPic() parameter inversely,
	 * with the opposite default value (obviously).
	 * Also, they used 15 priority zones from 42 to 200 instead of 14 priority
	 * zones from 42 to 190.
	 */
	bool usesOldGfxFunctions() const { return (features & kFeatureOldGfxFunctions); }

	// Script dissection/dumping functions
	void dissectScript(int scriptNumber, Vocabulary *vocab);
	void dumpScriptObject(char *data, int seeker, int objsize);
	void dumpScriptClass(char *data, int seeker, int objsize);

	SelectorCache _selectorCache; /**< Shortcut list for important selectors */
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
	
#ifdef ENABLE_SCI32
	/**
	 * Sets the default kernel function names to the SCI2 kernel functions
	 */
	void setKernelNamesSci2();
	
	/**
	 * Sets the default kernel function names to the SCI2.1 kernel functions
	 */
	void setKernelNamesSci21();
#endif

	/**
	 * Loads the kernel selector names.
	 */
	void loadSelectorNames();
	
	/**
	 * Check for any hardcoded selector table we might have that can be used
	 * if a game is missing the selector names.
	 */
	Common::StringList checkStaticSelectorNames();

	/**
	 * Maps special selectors
	 */
	void mapSelectors();

	/**
	 * Detects SCI features based on the existence of certain selectors
	 */
	void detectSciFeatures();

	/**
	 * Maps kernel functions
	 */
	void mapFunctions();

	ResourceManager *_resMan;
	uint32 features;

	// Kernel-related lists
	Common::StringList _selectorNames;
	Common::StringList _kernelNames;
};

/******************** Selector functionality ********************/

enum SelectorInvocation {
	kStopOnInvalidSelector = 0,
	kContinueOnInvalidSelector = 1
};

#define GET_SEL32(_o_, _slc_) read_selector(segMan, _o_, ((SciEngine*)g_engine)->getKernel()->_selectorCache._slc_, __FILE__, __LINE__)
#define GET_SEL32V(_o_, _slc_) (GET_SEL32(_o_, _slc_).offset)
/* Retrieves a selector from an object
** Parameters: (reg_t) object: The address of the object which the selector should be read from
**             (selector_name) selector: The selector to read
** Returns   : (int16/uint16/reg_t) The selector value
** This macro halts on error. 'selector' must be a selector name registered in vm.h's
** SelectorCache and mapped in script.cpp.
*/

#define PUT_SEL32(_o_, _slc_, _val_) write_selector(segMan, _o_, ((SciEngine*)g_engine)->getKernel()->_selectorCache._slc_, _val_, __FILE__, __LINE__)
#define PUT_SEL32V(_o_, _slc_, _val_) write_selector(segMan, _o_, ((SciEngine*)g_engine)->getKernel()->_selectorCache._slc_, make_reg(0, _val_), __FILE__, __LINE__)
/* Writes a selector value to an object
** Parameters: (reg_t) object: The address of the object which the selector should be written to
**             (selector_name) selector: The selector to read
**             (int16) value: The value to write
** Returns   : (void)
** This macro halts on error. 'selector' must be a selector name registered in vm.h's
** SelectorCache and mapped in script.cpp.
*/


enum {
	// FIXME: FAKE_FUNCT_NR is a hack used to substitute for an opcode number in certain places
	FAKE_FUNCT_NR = -1
};

#define INV_SEL(_object_, _selector_, _noinvalid_) \
	s, _object_,  s->_kernel->_selectorCache._selector_, _noinvalid_, FAKE_FUNCT_NR, argv, argc, __FILE__, __LINE__
/* Kludge for use with invoke_selector(). Used for compatibility with compilers that can't
** handle vararg macros.
*/


reg_t read_selector(SegManager *segMan, reg_t object, Selector selector_id, const char *fname, int line);
void write_selector(SegManager *segMan, reg_t object, Selector selector_id, reg_t value, const char *fname, int line);
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
 * @return The referenced text, or empty string on error.
 */
Common::String kernel_lookup_text(EngineState *s, reg_t address, int index);

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
enum {
	_K_VIEW_SIG_FLAG_STOP_UPDATE    = 0x0001,
	_K_VIEW_SIG_FLAG_UPDATED        = 0x0002,
	_K_VIEW_SIG_FLAG_NO_UPDATE      = 0x0004,
	_K_VIEW_SIG_FLAG_HIDDEN         = 0x0008,
	_K_VIEW_SIG_FLAG_FIX_PRI_ON     = 0x0010,
	_K_VIEW_SIG_FLAG_ALWAYS_UPDATE  = 0x0020,
	_K_VIEW_SIG_FLAG_FORCE_UPDATE   = 0x0040,
	_K_VIEW_SIG_FLAG_REMOVE         = 0x0080,
	_K_VIEW_SIG_FLAG_FROZEN         = 0x0100,
	_K_VIEW_SIG_FLAG_IS_EXTRA       = 0x0200,
	_K_VIEW_SIG_FLAG_HIT_OBSTACLE   = 0x0400,
	_K_VIEW_SIG_FLAG_DOESNT_TURN    = 0x0800,
	_K_VIEW_SIG_FLAG_NO_CYCLER      = 0x1000,
	_K_VIEW_SIG_FLAG_IGNORE_HORIZON = 0x2000,
	_K_VIEW_SIG_FLAG_IGNORE_ACTOR   = 0x4000,
	_K_VIEW_SIG_FLAG_DISPOSE_ME     = 0x8000,

	_K_VIEW_SIG_FLAG_STOPUPD        = 0x20000000 /* View has been stop-updated */
};


/******************** Kernel functions ********************/

// New kernel functions
reg_t kStrLen(EngineState *s, int, int argc, reg_t *argv);
reg_t kGetFarText(EngineState *s, int, int argc, reg_t *argv);
reg_t kReadNumber(EngineState *s, int, int argc, reg_t *argv);
reg_t kStrCat(EngineState *s, int, int argc, reg_t *argv);
reg_t kStrCmp(EngineState *s, int, int argc, reg_t *argv);
reg_t kSetSynonyms(EngineState *s, int, int argc, reg_t *argv);
reg_t kLock(EngineState *s, int, int argc, reg_t *argv);
reg_t kPalette(EngineState *s, int, int argc, reg_t *argv);
reg_t kNumCels(EngineState *s, int, int argc, reg_t *argv);
reg_t kNumLoops(EngineState *s, int, int argc, reg_t *argv);
reg_t kDrawCel(EngineState *s, int, int argc, reg_t *argv);
reg_t kCoordPri(EngineState *s, int, int argc, reg_t *argv);
reg_t kPriCoord(EngineState *s, int, int argc, reg_t *argv);
reg_t kShakeScreen(EngineState *s, int, int argc, reg_t *argv);
reg_t kSetCursor(EngineState *s, int, int argc, reg_t *argv);
reg_t kMoveCursor(EngineState *s, int, int argc, reg_t *argv);
reg_t kShow(EngineState *s, int, int argc, reg_t *argv);
reg_t kPicNotValid(EngineState *s, int, int argc, reg_t *argv);
reg_t kOnControl(EngineState *s, int, int argc, reg_t *argv);
reg_t kDrawPic(EngineState *s, int, int argc, reg_t *argv);
reg_t kGetPort(EngineState *s, int, int argc, reg_t *argv);
reg_t kSetPort(EngineState *s, int, int argc, reg_t *argv);
reg_t kNewWindow(EngineState *s, int, int argc, reg_t *argv);
reg_t kDisposeWindow(EngineState *s, int, int argc, reg_t *argv);
reg_t kCelWide(EngineState *s, int, int argc, reg_t *argv);
reg_t kCelHigh(EngineState *s, int, int argc, reg_t *argv);
reg_t kSetJump(EngineState *s, int, int argc, reg_t *argv);
reg_t kDirLoop(EngineState *s, int, int argc, reg_t *argv);
reg_t kDoAvoider(EngineState *s, int, int argc, reg_t *argv);
reg_t kGetAngle(EngineState *s, int, int argc, reg_t *argv);
reg_t kGetDistance(EngineState *s, int, int argc, reg_t *argv);
reg_t kRandom(EngineState *s, int, int argc, reg_t *argv);
reg_t kAbs(EngineState *s, int, int argc, reg_t *argv);
reg_t kSqrt(EngineState *s, int, int argc, reg_t *argv);
reg_t kTimesSin(EngineState *s, int, int argc, reg_t *argv);
reg_t kTimesCos(EngineState *s, int, int argc, reg_t *argv);
reg_t kCosMult(EngineState *s, int, int argc, reg_t *argv);
reg_t kSinMult(EngineState *s, int, int argc, reg_t *argv);
reg_t kTimesTan(EngineState *s, int, int argc, reg_t *argv);
reg_t kTimesCot(EngineState *s, int, int argc, reg_t *argv);
reg_t kCosDiv(EngineState *s, int, int argc, reg_t *argv);
reg_t kSinDiv(EngineState *s, int, int argc, reg_t *argv);
reg_t kValidPath(EngineState *s, int, int argc, reg_t *argv);
reg_t kFOpen(EngineState *s, int, int argc, reg_t *argv);
reg_t kFPuts(EngineState *s, int, int argc, reg_t *argv);
reg_t kFGets(EngineState *s, int, int argc, reg_t *argv);
reg_t kFClose(EngineState *s, int, int argc, reg_t *argv);
reg_t kMapKeyToDir(EngineState *s, int, int argc, reg_t *argv);
reg_t kGlobalToLocal(EngineState *s, int, int argc, reg_t *argv);
reg_t kLocalToGlobal(EngineState *s, int, int argc, reg_t *argv);
reg_t kWait(EngineState *s, int, int argc, reg_t *argv);
reg_t kRestartGame(EngineState *s, int, int argc, reg_t *argv);
reg_t kDeviceInfo(EngineState *s, int, int argc, reg_t *argv);
reg_t kGetEvent(EngineState *s, int, int argc, reg_t *argv);
reg_t kCheckFreeSpace(EngineState *s, int, int argc, reg_t *argv);
reg_t kFlushResources(EngineState *s, int, int argc, reg_t *argv);
reg_t kGetSaveFiles(EngineState *s, int, int argc, reg_t *argv);
reg_t kSetDebug(EngineState *s, int, int argc, reg_t *argv);
reg_t kCheckSaveGame(EngineState *s, int, int argc, reg_t *argv);
reg_t kSaveGame(EngineState *s, int, int argc, reg_t *argv);
reg_t kRestoreGame(EngineState *s, int, int argc, reg_t *argv);
reg_t kFileIO(EngineState *s, int, int argc, reg_t *argv);
reg_t kGetTime(EngineState *s, int, int argc, reg_t *argv);
reg_t kHaveMouse(EngineState *s, int, int argc, reg_t *argv);
reg_t kJoystick(EngineState *s, int, int argc, reg_t *argv);
reg_t kGameIsRestarting(EngineState *s, int, int argc, reg_t *argv);
reg_t kGetCWD(EngineState *s, int, int argc, reg_t *argv);
reg_t kSort(EngineState *s, int, int argc, reg_t *argv);
reg_t kStrEnd(EngineState *s, int, int argc, reg_t *argv);
reg_t kMemory(EngineState *s, int, int argc, reg_t *argv);
reg_t kAvoidPath(EngineState *s, int, int argc, reg_t *argv);
reg_t kParse(EngineState *s, int, int argc, reg_t *argv);
reg_t kSaid(EngineState *s, int, int argc, reg_t *argv);
reg_t kStrCpy(EngineState *s, int, int argc, reg_t *argv);
reg_t kStrAt(EngineState *s, int, int argc, reg_t *argv);
reg_t kEditControl(EngineState *s, int, int argc, reg_t *argv);
reg_t kDrawControl(EngineState *s, int, int argc, reg_t *argv);
reg_t kHiliteControl(EngineState *s, int, int argc, reg_t *argv);
reg_t kClone(EngineState *s, int, int argc, reg_t *argv);
reg_t kDisposeClone(EngineState *s, int, int argc, reg_t *argv);
reg_t kCanBeHere(EngineState *s, int, int argc, reg_t *argv);
reg_t kCantBeHere(EngineState *s, int, int argc, reg_t *argv);
reg_t kSetNowSeen(EngineState *s, int, int argc, reg_t *argv);
reg_t kInitBresen(EngineState *s, int, int argc, reg_t *argv);
reg_t kDoBresen(EngineState *s, int, int argc, reg_t *argv);
reg_t kBaseSetter(EngineState *s, int, int argc, reg_t *argv);
reg_t kAddToPic(EngineState *s, int, int argc, reg_t *argv);
reg_t kAnimate(EngineState *s, int, int argc, reg_t *argv);
reg_t kDisplay(EngineState *s, int, int argc, reg_t *argv);
reg_t kGraph(EngineState *s, int, int argc, reg_t *argv);
reg_t kFormat(EngineState *s, int, int argc, reg_t *argv);
reg_t kDoSound(EngineState *s, int, int argc, reg_t *argv);
reg_t kAddMenu(EngineState *s, int, int argc, reg_t *argv);
reg_t kSetMenu(EngineState *s, int, int argc, reg_t *argv);
reg_t kGetMenu(EngineState *s, int, int argc, reg_t *argv);
reg_t kDrawStatus(EngineState *s, int, int argc, reg_t *argv);
reg_t kDrawMenuBar(EngineState *s, int, int argc, reg_t *argv);
reg_t kMenuSelect(EngineState *s, int, int argc, reg_t *argv);

reg_t kLoad(EngineState *s, int, int argc, reg_t *argv);
reg_t kUnLoad(EngineState *s, int, int argc, reg_t *argv);
reg_t kScriptID(EngineState *s, int, int argc, reg_t *argv);
reg_t kDisposeScript(EngineState *s, int, int argc, reg_t *argv);
reg_t kIsObject(EngineState *s, int, int argc, reg_t *argv);
reg_t kRespondsTo(EngineState *s, int, int argc, reg_t *argv);
reg_t kNewList(EngineState *s, int, int argc, reg_t *argv);
reg_t kDisposeList(EngineState *s, int, int argc, reg_t *argv);
reg_t kNewNode(EngineState *s, int, int argc, reg_t *argv);
reg_t kFirstNode(EngineState *s, int, int argc, reg_t *argv);
reg_t kLastNode(EngineState *s, int, int argc, reg_t *argv);
reg_t kEmptyList(EngineState *s, int, int argc, reg_t *argv);
reg_t kNextNode(EngineState *s, int, int argc, reg_t *argv);
reg_t kPrevNode(EngineState *s, int, int argc, reg_t *argv);
reg_t kNodeValue(EngineState *s, int, int argc, reg_t *argv);
reg_t kAddAfter(EngineState *s, int, int argc, reg_t *argv);
reg_t kAddToFront(EngineState *s, int, int argc, reg_t *argv);
reg_t kAddToEnd(EngineState *s, int, int argc, reg_t *argv);
reg_t kFindKey(EngineState *s, int, int argc, reg_t *argv);
reg_t kDeleteKey(EngineState *s, int, int argc, reg_t *argv);
reg_t kMemoryInfo(EngineState *s, int, int argc, reg_t *argv);
reg_t kGetSaveDir(EngineState *s, int, int argc, reg_t *argv);
reg_t kTextSize(EngineState *s, int, int argc, reg_t *argv);
reg_t kIsItSkip(EngineState *s, int, int argc, reg_t *argv);
reg_t kMessage(EngineState *s, int, int argc, reg_t *argv);
reg_t kDoAudio(EngineState *s, int, int argc, reg_t *argv);
reg_t kDoSync(EngineState *s, int, int argc, reg_t *argv);
reg_t kResCheck(EngineState *s, int, int argc, reg_t *argv);
reg_t kSetQuitStr(EngineState *s, int, int argc, reg_t *argv);
reg_t kShowMovie(EngineState *s, int, int argc, reg_t *argv);
reg_t kSetVideoMode(EngineState *s, int, int argc, reg_t *argv);
reg_t kStrSplit(EngineState *s, int, int argc, reg_t *argv);
reg_t kPlatform(EngineState *s, int, int argc, reg_t *argv);
reg_t kPalVary(EngineState *s, int, int argc, reg_t *argv);

// for unknown/unnamed kernel function
reg_t kUnknown(EngineState *s, int, int argc, reg_t *argv);

// for named but unimplemented kernel functions
reg_t kStub(EngineState *s, int, int argc, reg_t *argv);


} // End of namespace Sci

#endif // SCI_ENGIENE_KERNEL_H
