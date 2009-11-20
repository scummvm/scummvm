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

#ifndef SCI_ENGINE_VM_H
#define SCI_ENGINE_VM_H

/* VM and kernel declarations */

#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/resource.h"	// for SciVersion

#include "common/util.h"

namespace Sci {

class SegManager;
struct EngineState;
class Object;
class ResourceManager;

/** Number of bytes to be allocated for the stack */
#define VM_STACK_SIZE 0x1000

/** Maximum number of calls residing on the stack */
#define SCRIPT_MAX_EXEC_STACK 256
/** Maximum number of entries in the class table */
#define SCRIPT_MAX_CLASSTABLE_SIZE 256
/** Maximum number of cloned objects on the heap */
#define SCRIPT_MAX_CLONES 256


/** Object-relative offset of the selector area inside a script */
#define SCRIPT_SELECTOR_OFFSET 8 -8

/** Object-relative offset of the pointer to the underlying script's local variables */
#define SCRIPT_LOCALVARPTR_OFFSET 2 -8

/** Object-relative offset of the selector counter */
#define SCRIPT_SELECTORCTR_OFFSET 6 -8

/** Object-relative offset of the offset of the function area */
#define SCRIPT_FUNCTAREAPTR_OFFSET 4 -8

/** Offset that has to be added to the function area pointer */
#define SCRIPT_FUNCTAREAPTR_MAGIC 8 -8

/** Offset of the name pointer */
#define SCRIPT_NAME_OFFSET (getSciVersion() < SCI_VERSION_1_1 ? 14 -8 : 16)

/** Object-relative offset of the -info- selector */
#define SCRIPT_INFO_OFFSET (getSciVersion() < SCI_VERSION_1_1 ? 12 -8 : 14)

/** Flag fo the -info- selector */
#define SCRIPT_INFO_CLONE 0x0001

/** Flag for the -info- selector */
#define SCRIPT_INFO_CLASS 0x8000


/** Magical object identifier */
#define SCRIPT_OBJECT_MAGIC_NUMBER 0x1234
/** Offset of this identifier */
#define SCRIPT_OBJECT_MAGIC_OFFSET (getSciVersion() < SCI_VERSION_1_1 ? -8 : 0)

/** Script-relative offset of the species ID */
#define SCRIPT_SPECIES_OFFSET 8 -8

#define SCRIPT_SUPERCLASS_OFFSET (getSciVersion() < SCI_VERSION_1_1 ? 10 -8 : 12)

/** Magic adjustment value for lofsa and lofss */
#define SCRIPT_LOFS_MAGIC 3

/** Stack pointer value: Use predecessor's value */
#define CALL_SP_CARRY NULL

/** Types of selectors as returned by lookup_selector() below. */
enum SelectorType {
	kSelectorNone = 0,
	kSelectorVariable,
	kSelectorMethod
};

struct Class {
	int script; // number of the script the class is in, -1 for non-existing
	reg_t reg; // offset; script-relative offset, segment: 0 if not instantiated
};

#define RAW_IS_OBJECT(datablock) (READ_LE_UINT16(((byte *) datablock) + SCRIPT_OBJECT_MAGIC_OFFSET) == SCRIPT_OBJECT_MAGIC_NUMBER)

/** Contains selector IDs for a few selected selectors */
struct SelectorCache {
	SelectorCache() {
		memset(this, 0, sizeof(*this));
	}

	// Statically defined selectors, (almost the) same in all SCI versions
	Selector y;
	Selector x;
	Selector view, loop, cel; // Description of a specific image
	Selector underBits; // Used by the graphics subroutines to store backupped BG pic data
	Selector nsTop, nsLeft, nsBottom, nsRight; // View boundaries ('now seen')
	Selector lsTop, lsLeft, lsBottom, lsRight; // Used by Animate() subfunctions and scroll list controls
	Selector signal; // Used by Animate() to control a view's behaviour
	Selector illegalBits; // Used by CanBeHere
	Selector brTop, brLeft, brBottom, brRight; // Bounding Rectangle
	// name, key, time
	Selector text; // Used by controls
	Selector elements; // Used by SetSynonyms()
	// color, back
	Selector mode; // Used by text controls (-> DrawControl())
	// style
	Selector state, font, type;	// Used by controls
	// window
	Selector cursor, max; // Used by EditControl
	// mark, who
	Selector message; // Used by GetEvent
	// edit
	Selector play; // Play function (first function to be called)
	Selector number;
	Selector handle;	// Replaced by nodePtr in SCI1+
	Selector nodePtr;	// Replaces handle in SCI1+
	Selector client; // The object that wants to be moved
	Selector dx, dy; // Deltas
	Selector b_movCnt, b_i1, b_i2, b_di, b_xAxis, b_incr; // Various Bresenham vars
	Selector xStep, yStep; // BR adjustments
	Selector moveSpeed; // Used for DoBresen
	Selector canBeHere; // Funcselector: Checks for movement validity in SCI0
	Selector heading, mover; // Used in DoAvoider
	Selector doit; // Called (!) by the Animate() system call
	Selector isBlocked, looper;	// Used in DoAvoider
	Selector priority;
	Selector modifiers; // Used by GetEvent
	Selector replay; // Replay function
	// setPri, at, next, done, width
	Selector wordFail, syntaxFail; // Used by Parse()
	// semanticFail, pragmaFail
	// said
	Selector claimed; // Used generally by the event mechanism
	// value, save, restore, title, button, icon, draw
	Selector delete_; // Called by Animate() to dispose a view object
	Selector z;

	// SCI1+ static selectors
	Selector parseLang;
	Selector printLang; // Used for i18n
	Selector subtitleLang;
	Selector size;
	Selector points; // Used by AvoidPath()
	Selector palette;
	Selector dataInc;
	// handle (in SCI1)
	Selector min; // SMPTE time format
	Selector sec;
	Selector frame;
	Selector vol;
	Selector pri;
	// perform
	Selector moveDone;	// used for DoBresen

	// SCI1 selectors which have been moved a bit in SCI1.1, but otherwise static
	Selector cantBeHere; // Checks for movement avoidance in SCI1+. Replaces canBeHere
	Selector topString; // SCI1 scroll lists use this instead of lsTop
	Selector flags;

	// SCI1+ audio sync related selectors, not static. They're used for lip syncing in
	// CD talkie games
	Selector syncCue; // Used by DoSync()
	Selector syncTime;

	// SCI1.1 specific selectors
	Selector scaleX, scaleY;	// SCI1.1 view scaling

	// Used for auto detection purposes
	Selector overlay;	// Used to determine if a game is using old gfx functions or not
	Selector setCursor; // For autodetection
};

// A reference to an object's variable.
// The object is stored as a reg_t, the variable as an index into _variables
struct ObjVarRef {
	reg_t obj;
	int varindex;

	reg_t* getPointer(SegManager *segMan) const;
};


struct ViewObject {
	reg_t obj;
	ObjVarRef signalp;    /* Used only indirectly */
	ObjVarRef underBitsp; /* The same goes for the handle storage */
	int underBits; /* Copy of the underbits: Needed for cleanup */

	int x, y;
	int priority;
	byte *view;
	int view_nr, loop, cel; /* view_nr is ised for save/restore */
	int nsTop, nsLeft, nsRight, nsBottom;
	int real_y, z, index_nr; /* Used for sorting */
};

enum ExecStackType {
	EXEC_STACK_TYPE_CALL = 0,
	EXEC_STACK_TYPE_KERNEL = 1,
	EXEC_STACK_TYPE_VARSELECTOR = 2
};

struct ExecStack {
	reg_t objp;  // Pointer to the beginning of the current object
	reg_t sendp; // Pointer to the object containing the invoked method

	union {
		ObjVarRef varp; // Variable pointer for r/w access
		reg_t pc;       // Pointer to the initial program counter. Not accurate for the TOS element
	} addr;

	StackPtr fp; // Frame pointer
	StackPtr sp; // Stack pointer
	int argc;

	StackPtr variables_argp; // Argument pointer
	SegmentId local_segment; // local variables etc

	Selector selector;      // The selector which was used to call or -1 if not applicable
	int origin;             // The stack frame position the call was made from, or -1 if it was the initial call
	ExecStackType type;

	reg_t* getVarPointer(SegManager *segMan) const;
};

enum {
	VAR_GLOBAL = 0,
	VAR_LOCAL = 1,
	VAR_TEMP = 2,
	VAR_PARAM = 3
};

/**
 * Structure for storing the current internal state of the VM.
 */
struct ScriptState {
	ExecStack *xs;
	int16 restAdjust;
	reg_t *variables[4];		// global, local, temp, param, as immediate pointers
	reg_t *variables_base[4];	// Used for referencing VM ops
	SegmentId variables_seg[4];	// Same as above, contains segment IDs
	int variables_max[4];		// Max. values for all variables
};

/**
 * The current internal state of the VM.
 */
extern ScriptState scriptState;


// These types are used both as identifiers and as elements of bitfields
enum BreakpointType {
	/**
	 * Break when selector is executed. data contains (char *) selector name
	 * (in the format Object::Method)
	 */
	BREAK_SELECTOR = 1,

	/**
	 * Break when an exported function is called. data contains
	 * script_no << 16 | export_no.
	 */
	BREAK_EXPORT = 2
};

struct Breakpoint {
	BreakpointType type;
	union {
		uint32 address;  // Breakpoints on exports */
		char *name; // Breakpoints on selector names */
	} data;
	Breakpoint *next;
};

/**
 * Set this to 1 to abort script execution immediately. Aborting will 
 * leave the debug exec stack intact.
 * Set it to 2 to force a replay afterwards.
 */
extern int script_abort_flag;

/** Number of kernel calls in between gcs; should be < 50000 */
enum {
	GC_INTERVAL = 32768
};

/** Initially GC_DELAY, can be set at runtime */
extern int script_gc_interval;

/** Number of steps executed */
extern int script_step_counter;


/**
 * Executes function pubfunct of the specified script.
 * @param[in] s				The state which is to be executed with
 * @param[in] script		The script which is called
 * @param[in] pubfunct		The exported script function which is to
 * 							be called
 * @param[in] sp			Stack pointer position
 * @param[in] calling_obj	The heap address of the object that
 * 							executed the call
 * @param[in] argc			Number of arguments supplied
 * @param[in] argp			Pointer to the first supplied argument
 * @return					A pointer to the new exec stack TOS entry
 */
ExecStack *execute_method(EngineState *s, uint16 script, uint16 pubfunct, 
		StackPtr sp, reg_t calling_obj, uint16 argc, StackPtr argp);


/**
 * Executes a "send" or related operation to a selector.
 * @param[in] s			The EngineState to operate on
 * @param[in] send_obj	Heap address of the object to send to
 * @param[in] work_obj	Heap address of the object initiating the send
 * @param[in] sp		Stack pointer position
 * @param[in] framesize	Size of the send as determined by the "send" 
 * 						operation
 * @param[in] argp		Pointer to the beginning of the heap block 
 * 						containing the data to be sent. This area is a 
 * 						succession of one or more sequences of 
 * 						[selector_number][argument_counter] and then
 * 						"argument_counter" word entries with the 
 * 						parameter values.
 * @return				A pointer to the new execution stack TOS entry
 */
ExecStack *send_selector(EngineState *s, reg_t send_obj, reg_t work_obj,
	StackPtr sp, int framesize, StackPtr argp);


#define SCI_XS_CALLEE_LOCALS ((SegmentId)-1)

/**
 * Adds an entry to the top of the execution stack.
 *
 * @param[in] s				The state with which to execute
 * @param[in] pc			The initial program counter
 * @param[in] sp			The initial stack pointer
 * @param[in] objp			Pointer to the beginning of the current object
 * @param[in] argc			Number of parameters to call with
 * @param[in] argp			Heap pointer to the first parameter
 * @param[in] selector		The selector by which it was called or
 *							NULL_SELECTOR if n.a. For debugging.
 * @param[in] sendp			Pointer to the object which the message was 
 * 							sent to. Equal to objp for anything but super.
 * @param[in] origin		Number of the execution stack element this
 * 							entry was created by (usually the current TOS 
 * 							number, except for multiple sends).
 * @param[in] local_segment	The segment to use for local variables,
 *							or SCI_XS_CALLEE_LOCALS to use obj's segment.
 * @return 					A pointer to the new exec stack TOS entry
 */
ExecStack *add_exec_stack_entry(EngineState *s, reg_t pc, StackPtr sp, 
		reg_t objp, int argc, StackPtr argp, Selector selector, 
		reg_t sendp, int origin, SegmentId local_segment);


/**
 * Adds one varselector access to the execution stack.
 * This function is called from send_selector only.
 * @param[in] s			The EngineState to use
 * @param[in] objp		Pointer to the object owning the selector
 * @param[in] argc		1 for writing, 0 for reading
 * @param[in] argp		Pointer to the address of the data to write -2
 * @param[in] selector	Selector name
 * @param[in] address	Heap address of the selector
 * @param[in] origin	Stack frame which the access originated from
 * @return 				Pointer to the new exec-TOS element
 */
ExecStack *add_exec_stack_varselector(EngineState *s, reg_t objp, int argc,
		StackPtr argp, Selector selector, const ObjVarRef& address, 
		int origin);

/**
 * This function executes SCI bytecode
 * It executes the code on s->heap[pc] until it hits a 'ret' operation 
 * while (stack_base == stack_pos). Requires s to be set up correctly.
 * @param[in] s			The state to use
 * @param[in] restoring	1 if s has just been restored, 0 otherwise
 */
void run_vm(EngineState *s, int restoring);

/**
 * Debugger functionality
 * @param[in] s					The state at which debugging should take place
 * @param[in] bp				Flag, set to true when a breakpoint is triggered
 */
void script_debug(EngineState *s, bool bp);

/**
 * Initializes a EngineState block
 * @param[in] s	The state to initialize
 * @return		0 on success, 1 if vocab.996 (the class table) is missing
 * 				or corrupted
 */
int script_init_engine(EngineState *);

/**
 * Looks up a selector and returns its type and value
 * varindex is written to iff it is non-NULL and the selector indicates a property of the object.
 * @param[in] segMan		The Segment Manager
 * @param[in] obj			Address of the object to look the selector up in
 * @param[in] selectorid	The selector to look up
 * @param[out] varp			A reference to the selector, if it is a
 * 							variable.
 * @param[out] fptr			A reference to the function described by that
 * 							selector, if it is a valid function selector.
 * 							fptr is written to iff it is non-NULL and the
 * 							selector indicates a member function of that
 * 							object.
 * @return					kSelectorNone if the selector was not found in 
 * 							the object or its superclasses.
 * 							kSelectorVariable if the selector represents an
 * 							object-relative variable.
 * 							kSelectorMethod if the selector represents a
 * 							method
 */
SelectorType lookup_selector(SegManager *segMan, reg_t obj, Selector selectorid,
		ObjVarRef *varp, reg_t *fptr);

/**
 * Makes sure that a script and its superclasses get loaded to the heap.
 * If the script already has been loaded, only the number of lockers is 
 * increased. All scripts containing superclasses of this script are loaded
 * recursively as well, unless 'recursive' is set to zero. The 
 * complementary function is "script_uninstantiate()" below.
 * @param[in] resMan		The resource manager
 * @param[in] segMan	The segment manager
 * @param[in] script_nr		The script number to load
 * @return					The script's segment ID or 0 if out of heap
 */
int script_instantiate(ResourceManager *resMan, SegManager *segMan, int script_nr);

/**
 * Decreases the numer of lockers of a script and unloads it if that number
 * reaches zero. 
 * This function will recursively unload scripts containing its 
 * superclasses, if those aren't locked by other scripts as well.
 * @param[in] segMan	The segment manager
 * @param[in] version		The SCI version to use
 * @param[in] script_nr	The script number that is requestet to be unloaded
 */
void script_uninstantiate(SegManager *segMan, int script_nr);

/**
 * Converts the builtin Sierra game IDs to the ones we use in ScummVM
 * @param[in] gameName		The internal game name
 * @param[in] gameFlags     The game's flags, which are adjusted accordingly for demos
 * @return					The equivalent ScummVM game id
 */
const char *convertSierraGameId(const char *gameName, uint32 *gameFlags);

/**
 * Initializes an SCI game
 * This function must be run before script_run() is executed. Graphics data
 * is initialized iff s->gfx_state != NULL.
 * @param[in] s	The state to operate on
 * @return		0 on success, 1 if an error occured.
 */
int game_init(EngineState *s);

#ifdef INCLUDE_OLDGFX
/**
 * Initializes the graphics part of an SCI game
 * This function may only be called if game_init() did not initialize
 * the graphics data.
 * @param[in] s	The state to initialize the graphics in
 * @return		0 on success, 1 if an error occured
 */
int game_init_graphics(EngineState *s);
#endif

/**
 * Initializes the sound part of an SCI game
 * This function may only be called if game_init() did not initialize
 * the sound data.
 * @param[in] s				The state to initialize the sound in
 * @param[in] sound_flags	Flags to pass to the sound subsystem
 * @return					0 on success, 1 if an error occured
 */
int game_init_sound(EngineState *s, int sound_flags);

/**
 * Runs an SCI game
 * This is the main function for SCI games. It takes a valid state, loads
 * script 0 to it, finds the game object, allocates a stack, and runs the
 * init method of the game object. In layman's terms, this runs an SCI game.
 * Note that, EngineState *s may be changed during the game, e.g. if a game
 * state is restored.
 * @param[in] s	Pointer to the pointer of the state to operate on
 * @return		0 on success, 1 if an error occured.
 */
int game_run(EngineState **s);

/**
 * Restores an SCI game state and runs the game
 * This restores a savegame; otherwise, it behaves just like game_run().
 * @param[in] s				Pointer to the pointer of the state to
 * 							operate on
 * @param[in] savegame_name	Name of the savegame to restore
 * @return					0 on success, 1 if an error occured.
 */
int game_restore(EngineState **s, char *savegame_name);

/**
 * Uninitializes an initialized SCI game
 * This function should be run after each script_run() call.
 * @param[in] s	The state to operate on
 * @return		0 on success, 1 if an error occured.
 */
int game_exit(EngineState *s);

/**
 * Instructs the virtual machine to abort
 */
void quit_vm();

/**
 * Allocates "kernel" memory and returns a handle suitable to be passed on
 * to SCI scripts
 * @param[in] segMan	The Segment Manager
 * @param[in] type			A free-form type description string (static)
 * @param[in] space			The space to allocate
 * @return					The handle
 */
reg_t kalloc(SegManager *segMan, const char *type, int space);

/**
 * Returns a pointer to "kernel" memory based on the handle
 * @param[in] segMan	The Segment Manager
 * @param[in] handle		The handle to use
 * @return					A pointer to the allocated memory
 */
byte *kmem(SegManager *segMan, reg_t handle);

/**
 * Frees all "kernel" memory associated with a handle
 * @param[in] segMan	The Segment Manager
 * @param[in] handle		The handle to free
 * @return					0 on success, 1 otherwise
 */
int kfree(SegManager *segMan, reg_t handle);

/**
 * Shrink execution stack to size.
 * Contains an assert it is not already smaller.
 */
void shrink_execution_stack(EngineState *s, uint size);

} // End of namespace Sci

#endif // SCI_ENGINE_VM_H
