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

#include "common/util.h"

namespace Sci {

class SegManager;
struct EngineState;
typedef int sci_version_t;
struct IntMapper;
struct Object;

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
#define SCRIPT_NAME_OFFSET (s->_version < SCI_VERSION_1_1 ? 14 -8 : 16)
#define SCRIPT_NAME_SELECTOR (s->_version < SCI_VERSION_1_1 ? 3 : 8)

/** Object-relative offset of the -info- selector */
#define SCRIPT_INFO_OFFSET (s->_version < SCI_VERSION_1_1 ? 12 -8 : 14)
#define SCRIPT_INFO_SELECTOR (s->_version < SCI_VERSION_1_1 ? 2 : 7)

/** Flag fo the -info- selector */
#define SCRIPT_INFO_CLONE 0x0001

/** Flag for the -info- selector */
#define SCRIPT_INFO_CLASS 0x8000


/** Magical object identifier */
#define SCRIPT_OBJECT_MAGIC_NUMBER 0x1234
/** Offset of this identifier */
#define SCRIPT_OBJECT_MAGIC_OFFSET (s->_version < SCI_VERSION_1_1 ? -8 : 0)

/** Script-relative offset of the species ID */
#define SCRIPT_SPECIES_OFFSET 8 -8

#define SCRIPT_SUPERCLASS_OFFSET (s->_version < SCI_VERSION_1_1 ? 10 -8 : 12)

/*---------------------------------*/
/* Script selector index variables */
/*---------------------------------*/
#define SCRIPT_SPECIES_SELECTOR (s->_version < SCI_VERSION_1_1 ? 0 : 5)
#define SCRIPT_SUPERCLASS_SELECTOR (s->_version < SCI_VERSION_1_1 ? 1 : 6)
#define SCRIPT_CLASSSCRIPT_SELECTOR 4

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
	int script; /**< number of the script the class is in, -1 for non-existing */
	reg_t reg; /**< offset; script-relative offset, segment: 0 if not instantiated */
};

#define RAW_GET_CLASS_INDEX(scr, reg) ((scr)->obj_indices->checkKey(reg.offset, false))
#define RAW_IS_OBJECT(datablock) (READ_LE_UINT16(((byte *) datablock) + SCRIPT_OBJECT_MAGIC_OFFSET) == SCRIPT_OBJECT_MAGIC_NUMBER)

#define IS_CLASS(obj) (obj->_variables[SCRIPT_INFO_SELECTOR].offset & SCRIPT_INFO_CLASS)


/** Contains selector IDs for a few selected selectors */
struct selector_map_t {
	Selector init; /**< Init function */
	Selector play; /**< Play function (first function to be called) */
	Selector replay; /**< Replay function */
	Selector x, y, z; /**< Coordinates */
	Selector priority;
	Selector view, loop, cel; /**< Description of a specific image */
	Selector brLeft, brRight, brTop, brBottom; /**< Bounding Rectangle */
	Selector xStep, yStep; /**< BR adjustments */
	Selector nsLeft, nsRight, nsTop, nsBottom; /**< View boundaries ('now seen') */
	Selector text, font; /**< Used by controls */
	Selector type, state; /**< Used by contols as well */
	Selector doit; /**< Called (!) by the Animate() system call */
	Selector signal; /**< Used by Animate() to control a view's behaviour */
	Selector underBits; /**< Used by the graphics subroutines to store backupped BG pic data */

	/* The following selectors are used by the Bresenham syscalls: */
	Selector canBeHere; /**< Funcselector: Checks for movement validity */
	Selector client; /**< The object that wants to be moved */
	Selector cycler; /**< The cycler of the client */
	Selector dx, dy; /**< Deltas */
	Selector edgeHit;
	Selector b_movCnt, b_i1, b_i2, b_di, b_xAxis, b_incr; /**< Various Bresenham vars */
	Selector completed;

	Selector illegalBits; /**< Used by CanBeHere */
	Selector dispose;

	Selector prevSignal; /**< Used by DoSound */

	Selector message, modifiers; /**< Used by GetEvent */

	Selector owner, handle;
	Selector cue;
	Selector number;

	Selector max, cursor; /**< Used by EditControl */
	Selector mode; /**< Used by text controls (-> DrawControl()) */

	Selector wordFail, syntaxFail, semanticFail; /**< Used by Parse() */

	Selector claimed; /**< Used generally by the event mechanism */

	Selector elements; /**< Used by SetSynonyms() */

	Selector lsTop, lsBottom, lsRight, lsLeft; /**< Used by Animate() subfunctions and scroll list controls */

	Selector baseSetter; /**< Alternative baseSetter */

	Selector who, distance; /**< Used for 'chasing' movers */

	Selector looper, mover, isBlocked, heading; /**< Used in DoAvoider */

	Selector caller, moveDone, moveSpeed; /**< Used for DoBresen */

	Selector delete_; /**< Called by Animate() to dispose a view object */

	Selector vol;
	Selector pri;

	Selector min; /**< SMPTE time format */
	Selector sec;
	Selector frame;

	Selector dataInc;
	Selector size;
	Selector palette;
	Selector cantBeHere;
	Selector nodePtr;
	Selector flags;

	Selector points; /**< Used by AvoidPath() */

	Selector syncCue; /**< Used by DoSync() */
	Selector syncTime; /**< Used by DoSync() */
};

// A reference to an object's variable.
// The object is stored as a reg_t, the variable as an index into _variables
struct ObjVarRef {
	reg_t obj;
	int varindex;

	reg_t* getPointer(EngineState *s) const;
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

enum {
	VAR_GLOBAL = 0,
	VAR_LOCAL = 1,
	VAR_TEMP = 2,
	VAR_PARAM = 3
};

enum ExecStackType {
	EXEC_STACK_TYPE_CALL = 0,
	EXEC_STACK_TYPE_KERNEL = 1,
	EXEC_STACK_TYPE_VARSELECTOR = 2
};

struct ExecStack {
	reg_t objp;
	reg_t sendp; /**< Pointer to the object containing the invoked method */

	union {
		ObjVarRef varp; /**< Variable pointer for r/w access */
		reg_t pc; /**< Not accurate for the TOS element */
	} addr;

	StackPtr fp; /**< Frame pointer */
	StackPtr sp; /**< Stack pointer */
	int argc;

	/* former variables[4]: [all other values are derived] */
	StackPtr variables_argp; /**< Argument pointer */
	SegmentId local_segment; /**< local variables etc. */

	Selector selector; /**< The selector which was used to call or -1 if not applicable */
	int origin;   /**< The stack frame position the call was made from, or -1 if it was the initial call.  */
	ExecStackType type;

	reg_t* getVarPointer(EngineState *s) const;
};


enum BreakpointType {
	/**
	 * Break when selector is executed. data contains (char *) selector name
	 * (in the format Object::Method)
	 */
	BREAK_SELECTOR,

	/**
	 * Break when an exported function is called. data contains
	 * script_no << 16 | export_no.
	 */
	BREAK_EXPORT
};

struct Breakpoint {
	BreakpointType type;
	union {
		uint32 address;  /**< Breakpoints on exports */
		char *name; /**< Breakpoints on selector names */
	} data;
	Breakpoint *next;
};

/**
 * Set this to 1 to abort script execution immediately. Aborting will leave the
 * debug exec stack intact.
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
 * Parameters: (EngineState *) s: The state which is to be executed with
 *             (uint16) script: The script which is called
 *             (uint16) pubfunct: The exported script function which is to be called
 *             (StackPtr) sp: Stack pointer position
 *             (reg_t) calling_obj: The heap address of the object which executed the call
 *             (uint16) argc: Number of arguments supplied
 *             (StackPtr) argp: Pointer to the first supplied argument
 * Returns   : (ExecStack *): A pointer to the new exec stack TOS entry
 */
ExecStack *execute_method(EngineState *s, uint16 script, uint16 pubfunct, StackPtr sp, reg_t calling_obj,
	uint16 argc, StackPtr argp);


/**
 * Executes a "send" or related operation to a selector.
 * Parameters: (EngineState *) s: The EngineState to operate on
 *             (reg_t) send_obj: Heap address of the object to send to
 *             (reg_t) work_obj: Heap address of the object initiating the send
 *             (StackPtr) sp: Stack pointer position
 *             (int) framesize: Size of the send as determined by the "send" operation
 *             (StackPtr) argp: Pointer to the beginning of the heap block containing the
 *                              data to be sent. This area is a succession of one or more
 *                              sequences of [selector_number][argument_counter] and then
 *                              "argument_counter" word entries with the parameter values.
 * Returns   : (ExecStack *): A pointer to the new execution stack TOS entry
 */
ExecStack *send_selector(EngineState *s, reg_t send_obj, reg_t work_obj,
	StackPtr sp, int framesize, StackPtr argp);


#define SCI_XS_CALLEE_LOCALS -1

/**
 * Adds an entry to the top of the execution stack.
 *
 * @param s				The state with which to execute
 * @param pc			The initial program counter
 * @param sp			The initial stack pointer
 * @param objp			Pointer to the beginning of the current object
 * @param argc			Number of parameters to call with
 * @param argp			Heap pointer to the first parameter
 * @param selector		The selector by which it was called or
 *						NULL_SELECTOR if n.a. For debugging.
 * @param sendp			Pointer to the object which the message was sent to.
 *						Equal to objp for anything but super.
 * @param origin		Number of the execution stack element this entry was created by
 *						(usually the current TOS number, except for multiple sends).
 * @param local_segment	The segment to use for local variables,
 *						or SCI_XS_CALLEE_LOCALS to use obj's segment.
 * @return a pointer to the new exec stack TOS entry
 */
ExecStack *add_exec_stack_entry(EngineState *s, reg_t pc, StackPtr sp, reg_t objp, int argc,
	StackPtr argp, Selector selector, reg_t sendp, int origin, SegmentId local_segment);


/**
 * Adds one varselector access to the execution stack.
 * Parameters: (EngineState *) s: The EngineState to use
 *             (reg_t) objp: Pointer to the object owning the selector
 *             (int) argc: 1 for writing, 0 for reading
 *             (StackPtr) argp: Pointer to the address of the data to write -2
 *             (int) selector: Selector name
 *             (ObjVarRef& ) address: Heap address of the selector
 *             (int) origin: Stack frame which the access originated from
 * Returns   : (ExecStack *): Pointer to the new exec-TOS element
 * This function is called from send_selector only.
 */
ExecStack *add_exec_stack_varselector(EngineState *s, reg_t objp, int argc, StackPtr argp,
	Selector selector, const ObjVarRef& address, int origin);


void run_vm(EngineState *s, int restoring);
/* Executes the code on s->heap[pc] until it hits a 'ret' operation while (stack_base == stack_pos)
** Parameters: (EngineState *) s: The state to use
**             (int) restoring: 1 if s has just been restored, 0 otherwise
** Returns   : (void)
** This function will execute SCI bytecode. It requires s to be set up
** correctly.
*/

void vm_handle_fatal_error(EngineState *s, int line, const char *file);
/* Handles a fatal error condition
** Parameters: (EngineState *) s: The state to recover from
**             (int) line: Source code line number the error occured in
**             (const char *) file: File the error occured in
*/


void script_debug(EngineState *s, reg_t *pc, StackPtr *sp, StackPtr *pp, reg_t *objp,
	int *restadjust, SegmentId *segids, reg_t **variables, reg_t **variables_base,
	int *variables_nr, int bp);
/* Debugger functionality
** Parameters: (EngineState *) s: The state at which debugging should take place
**             (reg_t *) pc: Pointer to the program counter
**             (StackPtr *) sp: Pointer to the stack pointer
**             (StackPtr *) pp: Pointer to the frame pointer
**             (reg_t *) objp: Pointer to the object base pointer
**             (int *) restadjust: Pointer to the &rest adjustment value
**	       (SegmentId *) segids: four-element array containing segment IDs for locals etc.
**	       (reg_t **) variables: four-element array referencing registers for globals etc.
**	       (reg_t **) variables_base: four-element array referencing
**                                        register bases for temps etc.
**	       (int *) variables_nr: four-element array giving sizes for params etc. (may be NULL)
**             (int) bp: Flag, set to 1 when a breakpoint is triggered
** Returns   : (void)
*/

int script_init_engine(EngineState *s);
/* Initializes a EngineState block
** Parameters: (EngineState *) s: The state to initialize
** Returns   : 0 on success, 1 if vocab.996 (the class table) is missing or corrupted
*/

void script_set_gamestate_save_dir(EngineState *s, const char *path);
/* Sets the gamestate's save_dir to the parameter path
** Parameters: (EngineState *) s: The state to set
**             (const char *) path: Path where save_dir will point to
** Returns   : (void)
*/

void script_free_engine(EngineState *s);
/* Frees all additional memory associated with a EngineState block
** Parameters: (EngineState *) s: The EngineState whose elements should be cleared
** Returns   : (void)
*/

void script_free_vm_memory(EngineState *s);
/* Frees all script memory (heap, hunk, and class tables).
** Parameters: (EngineState *) s: The EngineState to free
** Returns   : (void)
** This operation is implicit in script_free_engine(), but is required for restoring
** the game state.
*/


SelectorType lookup_selector(EngineState *s, reg_t obj, Selector selectorid, ObjVarRef *varp, reg_t *fptr);
/* Looks up a selector and returns its type and value
** Parameters: (EngineState *) s: The EngineState to use
**             (reg_t) obj: Address of the object to look the selector up in
**             (Selector) selectorid: The selector to look up
** Returns   : (SelectorType) kSelectorNone if the selector was not found in the object or its superclasses.
**                            kSelectorVariable if the selector represents an object-relative variable
**                            kSelectorMethod if the selector represents a method
**             (ObjVarRef *) *varp: A reference to the selector, if
**                              it is a variable
**             (reg_t) *fptr: A reference to the function described by that selector, if it is
**                            a valid function selector.
** *varindex is written to iff it is non-NULL and the selector indicates a property of the object.
** *fptr is written to iff it is non-NULL and the selector indicates a member function of that object.
*/

enum {
	SCRIPT_GET_DONT_LOAD = 0, /**< Fail if not loaded */
	SCRIPT_GET_LOAD = 1, /**< Load, if neccessary */
	SCRIPT_GET_LOCK = 3 /**< Load, if neccessary, and lock */
};

SegmentId script_get_segment(EngineState *s, int script_id, int load);
/* Determines the segment occupied by a certain script
** Parameters: (EngineState *) s: The state to operate on
**             (int) script_id: The script in question
**             (int) load: One of SCRIPT_GET_*
** Returns   : The script's segment, or 0 on failure
*/

reg_t script_lookup_export(EngineState *s, int script_nr, int export_index);
/* Looks up an entry of the exports table of a script
** Parameters: (EngineState *) s: The state to operate on
**             (int) script_nr: The script to look up in
** Returns   : (int) export_index: index of the export entry to look up
*/

int script_instantiate(EngineState *s, int script_nr);
/* Makes sure that a script and its superclasses get loaded to the heap
** Parameters: (EngineState *) s: The state to operate on
**             (int) script_nr: The script number to load
** Returns   : (int) The script's segment ID or 0 if out of heap
** If the script already has been loaded, only the number of lockers is increased.
** All scripts containing superclasses of this script aret loaded recursively as well,
** unless 'recursive' is set to zero.
** The complementary function is "script_uninstantiate()" below.
*/


void script_uninstantiate(EngineState *s, int script_nr);
/* Decreases the numer of lockers of a script and unloads it if that number reaches zero
** Parameters: (EngineState *) s: The state to operate on
**             (int) script_nr: The script number that is requestet to be unloaded
** Returns   : (void)
** This function will recursively unload scripts containing its superclasses, if those
** aren't locked by other scripts as well.
*/


int game_init(EngineState *s);
/* Initializes an SCI game
** Parameters: (EngineState *) s: The state to operate on
** Returns   : (int): 0 on success, 1 if an error occured.
** This function must be run before script_run() is executed.
** Graphics data is initialized iff s->gfx_state != NULL.
*/

int game_init_graphics(EngineState *s);
/* Initializes the graphics part of an SCI game
** Parameters: (EngineState *) s: The state to initialize the graphics in
** Returns   : (int) 0 on success, 1 if an error occured
** This function may only be called if game_init() did not initialize
** the graphics data.
*/

int game_init_sound(EngineState *s, int sound_flags);
/* Initializes the sound part of an SCI game
** Parameters: (EngineState *) s: The state to initialize the sound in
**             (int) sound_flags:  Flags to pass to the sound subsystem
** Returns   : (int) 0 on success, 1 if an error occured
** This function may only be called if game_init() did not initialize
** the graphics data.
*/


int game_run(EngineState **s);
/* Runs an SCI game
** Parameters: (EngineState **) s: Pointer to the pointer of the state to operate on
** Returns   : (int): 0 on success, 1 if an error occured.
** This is the main function for SCI games. It takes a valid state, loads script 0 to it,
** finds the game object, allocates a stack, and runs the init method of the game object.
** In layman's terms, this runs an SCI game.
** By the way, *s may be changed during the game, e.g. if a game state is restored.
*/

int game_restore(EngineState **s, char *savegame_name);
/* Restores an SCI game state and runs the game
** Parameters: (EngineState **) s: Pointer to the pointer of the state to operate on
**             (char *) savegame_name: Name of the savegame to restore
** Returns   : (int): 0 on success, 1 if an error occured.
** This restores a savegame; otherwise, it behaves just like game_run().
*/

int game_exit(EngineState *s);
/* Uninitializes an initialized SCI game
** Parameters: (EngineState *) s: The state to operate on
** Returns   : (int): 0 on success, 1 if an error occured.
** This function should be run after each script_run() call.
*/

void quit_vm();
/* Instructs the virtual machine to abort
** Paramteres: (void)
** Returns   : (void)
*/

reg_t kalloc(EngineState *s, const char *type, int space);
/* Allocates "kernel" memory and returns a handle suitable to be passed on to SCI scripts
** Parameters: (EngineState *) s: Pointer to the EngineState to operate on
**             (const char *) type: A free-form type description string (static)
**             (int) space: The space to allocate
** Returns   : (reg_t) The handle
*/

byte *kmem(EngineState *s, reg_t handle);
/* Returns a pointer to "kernel" memory based on the handle
** Parameters: (EngineState *) s: Pointer to the EngineState to operate on
**             (reg_t) handle: The handle to use
** Returns   : (byte *) A pointer to the allocated memory
*/


int kfree(EngineState *s, reg_t handle);
/* Frees all "kernel" memory associated with a handle
** Parameters: (EngineState *) s: Pointer to the EngineState to operate on
**             (reg_t) handle: The handle to free
** Returns   : (int) 0 on success, 1 otherwise
*/

const char *obj_get_name(EngineState *s, reg_t pos);
/* Determines the name of an object
** Parameters: (EngineState *) s: Pointer to the EngineState to operate on
**             (reg_t) pos: Location of the object whose name we want to
**                          inspect
** Returns   : (const char *) A name for that object, or a string describing
**                            an error that occured while looking it up
** The string is stored in a static buffer and need not be freed (neither
** may it be modified).
*/

Object *obj_get(EngineState *s, reg_t offset);
/* Retrieves an object from the specified location
** Parameters: (EngineState *) s: Pointer to the EngineState to operate on
**             (reg_t) offset: The object's offset
** Returns   : (Object *) The object in question, or NULL if there is none
*/

void shrink_execution_stack(EngineState *s, uint size);
/* Shrink execution stack to size.
** Contains an assert it is not already smaller.
*/

} // End of namespace Sci

#endif // SCI_ENGINE_VM_H
