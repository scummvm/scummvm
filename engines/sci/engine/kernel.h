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

#include "sci/sci.h"	// for USE_OLD_MUSIC_FUNCTIONS
#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/engine/vm.h"

namespace Sci {

struct Node;	// from segment.h
struct List;	// from segment.h

/**
 * @defgroup VocabularyResources	Vocabulary resources in SCI
 *
 * vocab.999 / 999.voc (unneeded) contains names of the kernel functions which
 * are implemented by the interpreter. In Sierra SCI, they are used exclusively
 * by the debugger, which is why keeping the file up to date was less important.
 * This resource is notoriously unreliable, and should not be used. Fortunately,
 * kernel names are the same in major SCI revisions, which is why we have them
 * hardcoded.
 *
 * vocab.998 / 998.voc (unneeded) contains opcode names. Opcodes have remained
 * the same from SCI0 to SCI2.1, and have changed in SCI3, so this is only used
 * on demand for debugging purposes, for showing the opcode names
 *
 * vocab.997 / 997.voc (usually needed) contains the names of every selector in
 * the class hierarchy. Each method and property '''name''' consumes one id, but
 * if a name is shared between classes, one id will do. Some demos do not contain
 * a selector vocabulary, but the selectors used by the engine have stayed more or
 * less static, so we add the selectors we need inside static_selectors.cpp
 * The SCI engine loads vocab.997 on startup, and fills in an internal structure
 * that allows interpreter code to access these selectors via #defined macros. It
 * does not use the file after this initial stage.
 *
 * vocab.996 / 996.voc (required) contains the classes which are used in each
 * script, and is required by the segment manager
 *
 * vocab.995 / 995.voc (unneeded) contains strings for the embedded SCI debugger
 *
 * vocab.994 / 994.voc (unneeded) contains offsets into certain classes of certain
 * properties. This enables the interpreter to update these properties in O(1) time,
 * which was important in the era when SCI was initially conceived. In SCI, we
 * figured out what '''property''' a certain offset refers to (which requires one to
 * guess what class a pointer points to) and then simply use the property name and
 * vocab.997. This results in much more readable code. Thus, this vocabulary isn't
 * used at all.
 *
 * SCI0 parser vocabularies:
 * - vocab.901 / 901.voc - suffix vocabulary
 * - vocab.900 / 900.vo1 - parse tree branches
 * - vocab.0 / 0.voc - main vocabulary, containing words and their attributes
 *                     (e.g. "onto" - "position")
 *
 * SCI01 parser vocabularies:
 * - vocab.902 / 902.voc - suffix vocabulary
 * - vocab.901 / 901.vo1 - parse tree branches
 * - vocab.900 / 900.voc - main vocabulary, containing words and their attributes
 *                        (e.g. "onto" - "position")
 *
 */
//@{

//#define DEBUG_PARSER	// enable for parser debugging
//#define DISABLE_VALIDATIONS	// enable to stop validation checks

/* Generic description: */
typedef reg_t KernelFunc(EngineState *s, int argc, reg_t *argv);

struct KernelFuncWithSignature {
	KernelFunc *fun; /**< The actual function */
	const char *signature;  /**< KernelFunc signature */
	Common::String orig_name; /**< Original name, in case we couldn't map it */
	bool isDummy;
};

enum AutoDetectedFeatures {
	kFeatureOldScriptHeader = 1 << 0
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
	 * Loads the kernel function names.
	 *
	 * This function reads the kernel function name table from resource_map,
	 * and fills the _kernelNames array with them.
	 * The resulting list has the same format regardless of the format of the
	 * name table of the resource (the format changed between version 0 and 1).
	 * @return true on success, false on failure
	 */
	bool loadKernelNames(Common::String gameId, EngineState *s);

	/**
	 * Determines the selector ID of a selector by its name
	 * @param selectorName Name of the selector to look up
	 * @return The appropriate selector ID, or -1 on error
	 */
	int findSelector(const char *selectorName) const;

	// Script dissection/dumping functions
	void dissectScript(int scriptNumber, Vocabulary *vocab);
	void dumpScriptObject(char *data, int seeker, int objsize);
	void dumpScriptClass(char *data, int seeker, int objsize);

	SelectorCache _selectorCache; /**< Shortcut list for important selectors */
	typedef Common::Array<KernelFuncWithSignature> KernelFuncsContainer;
	KernelFuncsContainer _kernelFuncs; /**< Table of kernel functions */

private:
	/**
	 * Sets the default kernel function names, based on the SCI version used
	 */
	void setDefaultKernelNames(Common::String gameId);

#ifdef ENABLE_SCI32
	/**
	 * Sets the default kernel function names to the SCI2 kernel functions
	 */
	void setKernelNamesSci2();

	/**
	 * Sets the default kernel function names to the SCI2.1 kernel functions
	 */
	void setKernelNamesSci21(EngineState *s);
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
	 * Maps kernel functions
	 */
	void mapFunctions();

	ResourceManager *_resMan;
	uint32 features;

	// Kernel-related lists
	Common::StringList _selectorNames;
	Common::StringList _kernelNames;
};

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


/******************** Dynamic view list functions ********************/

/**
 * Determines the now-seen rectangle of a view object
 * @param s The state to use
 * @param object The object to check
 * @param clip Flag to determine wheter priority band clipping
 * should be performed
 * @return The absolute rectangle describing the now-seen area.
 */
extern Common::Rect get_nsrect(EngineState *s, reg_t object, byte clip);


#ifdef USE_OLD_MUSIC_FUNCTIONS
/******************** Misc functions ********************/

/**
 * Get all sound events, apply their changes to the heap
 */
void process_sound_events(EngineState *s);

/******************** Constants ********************/
#endif

/* Maximum length of a savegame name (including terminator character) */
#define SCI_MAX_SAVENAME_LENGTH 0x24

/******************** Kernel functions ********************/

// New kernel functions
reg_t kStrLen(EngineState *s, int argc, reg_t *argv);
reg_t kGetFarText(EngineState *s, int argc, reg_t *argv);
reg_t kReadNumber(EngineState *s, int argc, reg_t *argv);
reg_t kStrCat(EngineState *s, int argc, reg_t *argv);
reg_t kStrCmp(EngineState *s, int argc, reg_t *argv);
reg_t kSetSynonyms(EngineState *s, int argc, reg_t *argv);
reg_t kLock(EngineState *s, int argc, reg_t *argv);
reg_t kPalette(EngineState *s, int argc, reg_t *argv);
reg_t kPalVary(EngineState *s, int argc, reg_t *argv);
reg_t kAssertPalette(EngineState *s, int argc, reg_t *argv);
reg_t kPortrait(EngineState *s, int argc, reg_t *argv);
reg_t kNumCels(EngineState *s, int argc, reg_t *argv);
reg_t kNumLoops(EngineState *s, int argc, reg_t *argv);
reg_t kDrawCel(EngineState *s, int argc, reg_t *argv);
reg_t kCoordPri(EngineState *s, int argc, reg_t *argv);
reg_t kPriCoord(EngineState *s, int argc, reg_t *argv);
reg_t kShakeScreen(EngineState *s, int argc, reg_t *argv);
reg_t kSetCursor(EngineState *s, int argc, reg_t *argv);
reg_t kMoveCursor(EngineState *s, int argc, reg_t *argv);
reg_t kPicNotValid(EngineState *s, int argc, reg_t *argv);
reg_t kOnControl(EngineState *s, int argc, reg_t *argv);
reg_t kDrawPic(EngineState *s, int argc, reg_t *argv);
reg_t kGetPort(EngineState *s, int argc, reg_t *argv);
reg_t kSetPort(EngineState *s, int argc, reg_t *argv);
reg_t kNewWindow(EngineState *s, int argc, reg_t *argv);
reg_t kDisposeWindow(EngineState *s, int argc, reg_t *argv);
reg_t kCelWide(EngineState *s, int argc, reg_t *argv);
reg_t kCelHigh(EngineState *s, int argc, reg_t *argv);
reg_t kSetJump(EngineState *s, int argc, reg_t *argv);
reg_t kDirLoop(EngineState *s, int argc, reg_t *argv);
reg_t kDoAvoider(EngineState *s, int argc, reg_t *argv);
reg_t kGetAngle(EngineState *s, int argc, reg_t *argv);
reg_t kGetDistance(EngineState *s, int argc, reg_t *argv);
reg_t kRandom(EngineState *s, int argc, reg_t *argv);
reg_t kAbs(EngineState *s, int argc, reg_t *argv);
reg_t kSqrt(EngineState *s, int argc, reg_t *argv);
reg_t kTimesSin(EngineState *s, int argc, reg_t *argv);
reg_t kTimesCos(EngineState *s, int argc, reg_t *argv);
reg_t kCosMult(EngineState *s, int argc, reg_t *argv);
reg_t kSinMult(EngineState *s, int argc, reg_t *argv);
reg_t kTimesTan(EngineState *s, int argc, reg_t *argv);
reg_t kTimesCot(EngineState *s, int argc, reg_t *argv);
reg_t kCosDiv(EngineState *s, int argc, reg_t *argv);
reg_t kSinDiv(EngineState *s, int argc, reg_t *argv);
reg_t kValidPath(EngineState *s, int argc, reg_t *argv);
reg_t kFOpen(EngineState *s, int argc, reg_t *argv);
reg_t kFPuts(EngineState *s, int argc, reg_t *argv);
reg_t kFGets(EngineState *s, int argc, reg_t *argv);
reg_t kFClose(EngineState *s, int argc, reg_t *argv);
reg_t kMapKeyToDir(EngineState *s, int argc, reg_t *argv);
reg_t kGlobalToLocal(EngineState *s, int argc, reg_t *argv);
reg_t kLocalToGlobal(EngineState *s, int argc, reg_t *argv);
reg_t kWait(EngineState *s, int argc, reg_t *argv);
reg_t kRestartGame(EngineState *s, int argc, reg_t *argv);
reg_t kDeviceInfo(EngineState *s, int argc, reg_t *argv);
reg_t kGetEvent(EngineState *s, int argc, reg_t *argv);
reg_t kCheckFreeSpace(EngineState *s, int argc, reg_t *argv);
reg_t kFlushResources(EngineState *s, int argc, reg_t *argv);
reg_t kGetSaveFiles(EngineState *s, int argc, reg_t *argv);
reg_t kSetDebug(EngineState *s, int argc, reg_t *argv);
reg_t kCheckSaveGame(EngineState *s, int argc, reg_t *argv);
reg_t kSaveGame(EngineState *s, int argc, reg_t *argv);
reg_t kRestoreGame(EngineState *s, int argc, reg_t *argv);
reg_t kFileIO(EngineState *s, int argc, reg_t *argv);
reg_t kGetTime(EngineState *s, int argc, reg_t *argv);
reg_t kHaveMouse(EngineState *s, int argc, reg_t *argv);
reg_t kJoystick(EngineState *s, int argc, reg_t *argv);
reg_t kGameIsRestarting(EngineState *s, int argc, reg_t *argv);
reg_t kGetCWD(EngineState *s, int argc, reg_t *argv);
reg_t kSort(EngineState *s, int argc, reg_t *argv);
reg_t kStrEnd(EngineState *s, int argc, reg_t *argv);
reg_t kMemory(EngineState *s, int argc, reg_t *argv);
reg_t kAvoidPath(EngineState *s, int argc, reg_t *argv);
reg_t kParse(EngineState *s, int argc, reg_t *argv);
reg_t kSaid(EngineState *s, int argc, reg_t *argv);
reg_t kStrCpy(EngineState *s, int argc, reg_t *argv);
reg_t kStrAt(EngineState *s, int argc, reg_t *argv);
reg_t kEditControl(EngineState *s, int argc, reg_t *argv);
reg_t kDrawControl(EngineState *s, int argc, reg_t *argv);
reg_t kHiliteControl(EngineState *s, int argc, reg_t *argv);
reg_t kClone(EngineState *s, int argc, reg_t *argv);
reg_t kDisposeClone(EngineState *s, int argc, reg_t *argv);
reg_t kCanBeHere(EngineState *s, int argc, reg_t *argv);
reg_t kCantBeHere(EngineState *s, int argc, reg_t *argv);
reg_t kSetNowSeen(EngineState *s, int argc, reg_t *argv);
reg_t kInitBresen(EngineState *s, int argc, reg_t *argv);
reg_t kDoBresen(EngineState *s, int argc, reg_t *argv);
reg_t kBaseSetter(EngineState *s, int argc, reg_t *argv);
reg_t kAddToPic(EngineState *s, int argc, reg_t *argv);
reg_t kAnimate(EngineState *s, int argc, reg_t *argv);
reg_t kDisplay(EngineState *s, int argc, reg_t *argv);
reg_t kGraph(EngineState *s, int argc, reg_t *argv);
reg_t kFormat(EngineState *s, int argc, reg_t *argv);
reg_t kDoSound(EngineState *s, int argc, reg_t *argv);
reg_t kAddMenu(EngineState *s, int argc, reg_t *argv);
reg_t kSetMenu(EngineState *s, int argc, reg_t *argv);
reg_t kGetMenu(EngineState *s, int argc, reg_t *argv);
reg_t kDrawStatus(EngineState *s, int argc, reg_t *argv);
reg_t kDrawMenuBar(EngineState *s, int argc, reg_t *argv);
reg_t kMenuSelect(EngineState *s, int argc, reg_t *argv);

reg_t kLoad(EngineState *s, int argc, reg_t *argv);
reg_t kUnLoad(EngineState *s, int argc, reg_t *argv);
reg_t kScriptID(EngineState *s, int argc, reg_t *argv);
reg_t kDisposeScript(EngineState *s, int argc, reg_t *argv);
reg_t kIsObject(EngineState *s, int argc, reg_t *argv);
reg_t kRespondsTo(EngineState *s, int argc, reg_t *argv);
reg_t kNewList(EngineState *s, int argc, reg_t *argv);
reg_t kDisposeList(EngineState *s, int argc, reg_t *argv);
reg_t kNewNode(EngineState *s, int argc, reg_t *argv);
reg_t kFirstNode(EngineState *s, int argc, reg_t *argv);
reg_t kLastNode(EngineState *s, int argc, reg_t *argv);
reg_t kEmptyList(EngineState *s, int argc, reg_t *argv);
reg_t kNextNode(EngineState *s, int argc, reg_t *argv);
reg_t kPrevNode(EngineState *s, int argc, reg_t *argv);
reg_t kNodeValue(EngineState *s, int argc, reg_t *argv);
reg_t kAddAfter(EngineState *s, int argc, reg_t *argv);
reg_t kAddToFront(EngineState *s, int argc, reg_t *argv);
reg_t kAddToEnd(EngineState *s, int argc, reg_t *argv);
reg_t kFindKey(EngineState *s, int argc, reg_t *argv);
reg_t kDeleteKey(EngineState *s, int argc, reg_t *argv);
reg_t kMemoryInfo(EngineState *s, int argc, reg_t *argv);
reg_t kGetSaveDir(EngineState *s, int argc, reg_t *argv);
reg_t kTextSize(EngineState *s, int argc, reg_t *argv);
reg_t kIsItSkip(EngineState *s, int argc, reg_t *argv);
reg_t kGetMessage(EngineState *s, int argc, reg_t *argv);
reg_t kMessage(EngineState *s, int argc, reg_t *argv);
reg_t kDoAudio(EngineState *s, int argc, reg_t *argv);
reg_t kDoSync(EngineState *s, int argc, reg_t *argv);
reg_t kMemorySegment(EngineState *s, int argc, reg_t *argv);
reg_t kIntersections(EngineState *s, int argc, reg_t *argv);
reg_t kResCheck(EngineState *s, int argc, reg_t *argv);
reg_t kSetQuitStr(EngineState *s, int argc, reg_t *argv);
reg_t kShowMovie(EngineState *s, int argc, reg_t *argv);
reg_t kSetVideoMode(EngineState *s, int argc, reg_t *argv);
reg_t kStrSplit(EngineState *s, int argc, reg_t *argv);
reg_t kPlatform(EngineState *s, int argc, reg_t *argv);
reg_t kTextColors(EngineState *s, int argc, reg_t *argv);
reg_t kTextFonts(EngineState *s, int argc, reg_t *argv);

#ifdef ENABLE_SCI32
// SCI2 Kernel Functions
reg_t kIsHiRes(EngineState *s, int argc, reg_t *argv);
reg_t kArray(EngineState *s, int argc, reg_t *argv);
reg_t kListAt(EngineState *s, int argc, reg_t *argv);
reg_t kString(EngineState *s, int argc, reg_t *argv);
// "Screen items" in SCI32 are views
reg_t kAddScreenItem(EngineState *s, int argc, reg_t *argv);
reg_t kUpdateScreenItem(EngineState *s, int argc, reg_t *argv);
reg_t kDeleteScreenItem(EngineState *s, int argc, reg_t *argv);
// "Planes" in SCI32 are pictures
reg_t kAddPlane(EngineState *s, int argc, reg_t *argv);
reg_t kDeletePlane(EngineState *s, int argc, reg_t *argv);
reg_t kUpdatePlane(EngineState *s, int argc, reg_t *argv);
reg_t kRepaintPlane(EngineState *s, int argc, reg_t *argv);
reg_t kFrameOut(EngineState *s, int argc, reg_t *argv);
reg_t kListIndexOf(EngineState *s, int argc, reg_t *argv);
reg_t kListEachElementDo(EngineState *s, int argc, reg_t *argv);
reg_t kListFirstTrue(EngineState *s, int argc, reg_t *argv);
reg_t kListAllTrue(EngineState *s, int argc, reg_t *argv);
reg_t kOnMe(EngineState *s, int argc, reg_t *argv);
reg_t kInPolygon(EngineState *s, int argc, reg_t *argv);

// SCI2.1 Kernel Functions
reg_t kSave(EngineState *s, int argc, reg_t *argv);
reg_t kList(EngineState *s, int argc, reg_t *argv);
reg_t kRobot(EngineState *s, int argc, reg_t *argv);

#endif

//@}

} // End of namespace Sci

#endif // SCI_ENGINE_KERNEL_H
