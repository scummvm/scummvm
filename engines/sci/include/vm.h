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

#ifndef _SCI_VM_H
#define _SCI_VM_H

/* VM and kernel declarations */

#include "sci/include/versions.h"	// for sci_version_t
#include "sci/include/vm_types.h"	// for reg_t
#include "sci/include/sys_strings.h"
#include "sci/include/heapmgr.h"

#include "sci/engine/intmap.h"

namespace Sci {

#define VM_STACK_SIZE 0x1000
/* Number of bytes to be allocated for the stack */

#define SCRIPT_MAX_EXEC_STACK 256
/* Maximum number of calls residing on the stack */
#define SCRIPT_MAX_CLASSTABLE_SIZE 256
/* Maximum number of entries in the class table */
#define SCRIPT_MAX_CLONES 256
/* Maximum number of cloned objects on the heap */


#define SCRIPT_SELECTOR_OFFSET 8 -8
/* Object-relative offset of the selector area inside a script */

#define SCRIPT_LOCALVARPTR_OFFSET 2 -8
/* Object-relative offset of the pointer to the underlying script's local variables */

#define SCRIPT_SELECTORCTR_OFFSET 6 -8
/* Object-relative offset of the selector counter */

#define SCRIPT_FUNCTAREAPTR_OFFSET 4 -8
/* Object-relative offset of the offset of the function area */

#define SCRIPT_FUNCTAREAPTR_MAGIC 8 -8
/* Offset that has to be added to the function area pointer */

#define SCRIPT_NAME_OFFSET (s->version < SCI_VERSION(1,001,000) ? 14 -8 : 16)
/* Offset of the name pointer */
#define SCRIPT_NAME_SELECTOR (s->version < SCI_VERSION(1,001,000) ? 3 : 8)

#define SCRIPT_INFO_OFFSET (s->version < SCI_VERSION(1,001,000) ? 12 -8 : 14)
/* Object-relative offset of the -info- selector */
#define SCRIPT_INFO_SELECTOR (s->version < SCI_VERSION(1,001,000) ? 2 : 7)

#define SCRIPT_INFO_CLONE 0x0001
/* Flag fo the -info- selector */

#define SCRIPT_INFO_CLASS 0x8000
/* Flag for the -info- selector */


#define SCRIPT_OBJECT_MAGIC_NUMBER 0x1234
/* Magical object identifier */
#define SCRIPT_OBJECT_MAGIC_OFFSET (s->version < SCI_VERSION(1,001,000) ? -8 : 0)
/* Offset of this identifier */

#define SCRIPT_SPECIES_OFFSET 8 -8
/* Script-relative offset of the species ID */

#define SCRIPT_SUPERCLASS_OFFSET (s->version < SCI_VERSION(1,001,000) ? 10 -8 : 12)

/*---------------------------------*/
/* Script selector index variables */
/*---------------------------------*/
#define SCRIPT_SPECIES_SELECTOR (s->version < SCI_VERSION(1,001,000) ? 0 : 5)
#define SCRIPT_SUPERCLASS_SELECTOR (s->version < SCI_VERSION(1,001,000) ? 1 : 6)

#define SCRIPT_LOFS_MAGIC 3
/* Magic adjustment value for lofsa and lofss */


#define CALL_SP_CARRY NULL /* Stack pointer value: Use predecessor's value */


#define SELECTOR_NONE 0
#define SELECTOR_VARIABLE 1
#define SELECTOR_METHOD 2
/* Types of selectors as returned by grep_selector() below */

struct class_t {
	int script; /* number of the script the class is in, -1 for non-existing */
	reg_t reg; /* offset; script-relative offset, segment: 0 if not instantiated */
};

#define RAW_GET_CLASS_INDEX(scr, reg) ((scr)->obj_indices->checkKey(reg.offset, false))
#define RAW_IS_OBJECT(datablock) (getUInt16(((byte *) datablock) + SCRIPT_OBJECT_MAGIC_OFFSET) == SCRIPT_OBJECT_MAGIC_NUMBER)

#define IS_CLASS(obj) (obj->variables[SCRIPT_INFO_SELECTOR].offset & SCRIPT_INFO_CLASS)

/* This struct is used to buffer the list of send calls in send_selector() */
struct calls_struct_t {
	union {
		reg_t func;
		reg_t *var;
	} address;
	stack_ptr_t argp;
	int argc;
	selector_t selector;
	stack_ptr_t sp; /* Stack pointer */
	int type; /* Same as exec_stack_t.type */
};

struct local_variables_t {
	int script_id; /* Script ID this local variable block belongs to */
	reg_t *locals;
	int nr;
};

#define OBJECT_FLAG_FREED (0x1 << 0)	/* Clone has been marked as 'freed' */

struct object_t {
	int flags;
	reg_t pos; /* Object offset within its script; for clones, this is their base */
	int variables_nr;
	int variable_names_nr; /* Number of variable names, may be less than variables_nr */
	int methods_nr;
	byte *base; /* Points to a buffer all relative references (code, strings) point to */
	byte *base_obj; /* base + object offset within base */
	uint16 *base_method; /* Pointer to the method selector area for this object */
	uint16 *base_vars; /* Pointer to the varselector area for this object */
	reg_t *variables;
};

struct code_block_t {
	reg_t pos;
	int size;
};

#define VM_OBJECT_GET_VARSELECTOR(obj, i)  \
	(s->version < SCI_VERSION(1,001,000) ? \
	 getUInt16(obj->base_obj + obj->variables_nr * 2 + i*2) : \
	 *(obj->base_vars + i))
#define VM_OBJECT_READ_PROPERTY(obj, i) (obj->variables[i])
#define VM_OBJECT_GET_FUNCSELECTOR(obj, i) \
	(s->version < SCI_VERSION(1,001,000) ? \
	 getUInt16((byte *) (obj->base_method + i)) : \
	 getUInt16((byte *) (obj->base_method + i*2 + 1)))
#define VM_OBJECT_READ_FUNCTION(obj, i) \
	(s->version < SCI_VERSION(1,001,000) ? \
	 make_reg(obj->pos.segment, \
		 getUInt16((byte *) (obj->base_method \
				 + obj->methods_nr + 1 \
				 + i))) : \
	 make_reg(obj->pos.segment, \
		 getUInt16((byte *) (obj->base_method \
				 + i * 2 + 2))))




struct script_t {
	int nr; /* Script number */
	byte* buf; /* Static data buffer, or NULL if not used */
	size_t buf_size;
	size_t script_size;
	size_t heap_size;

	byte *synonyms; /* Synonyms block  or 0 if not present*/
	byte *heap_start; /* Start of heap if SCI1.1, NULL otherwise */
	uint16 *export_table; /* Abs. offset of the export table or 0 if not present */

	IntMapper *obj_indices;

	int exports_nr; /* Number of entries in the exports table */
	int synonyms_nr; /* Number of entries in the synonyms block */
	int lockers; /* Number of classes and objects that require this script */

	object_t *objects; /* Table for objects, contains property variables */
	/* Indexed by the value stored at SCRIPT_LOCALVARPTR_OFFSET,
	** see VM_OBJECT_[GS]ET_INDEX()  */
	int objects_nr; /* Number of objects and classes */
	int objects_allocated; /* Number of allocated objects */

	int locals_offset;
	int locals_segment; /* The local variable segment */
	local_variables_t *locals_block;

	code_block_t *code;
	int code_blocks_nr;
	int code_blocks_allocated;
	int relocated;
	int marked_as_deleted;
};

struct dstack_t {
	int nr; /* Number of stack entries */
	reg_t *entries;
}; /* Data stack */

#define CLONE_USED -1
#define CLONE_NONE -1

typedef object_t clone_t;

struct node_t {
	reg_t pred, succ; /* Predecessor, successor */
	reg_t key;
	reg_t value;
}; /* List nodes */

struct list_t {
	reg_t first;
	reg_t last;
};

struct hunk_t {
	void *mem;
	unsigned int size;
	const char *type;
};

/* clone_table_t */
DECLARE_HEAPENTRY(clone)
/* node_table_t */
DECLARE_HEAPENTRY(node)
/* list_table_t */
DECLARE_HEAPENTRY(list) /* list entries */
/* hunk_table_t */
DECLARE_HEAPENTRY(hunk)

struct dynmem_t {
	int size;
	const char *description;
	byte *buf;
}; /* Free-style memory */

enum memObjType {
	MEM_OBJ_INVALID = 0,
	MEM_OBJ_SCRIPT = 1,
	MEM_OBJ_CLONES = 2,
	MEM_OBJ_LOCALS = 3,
	MEM_OBJ_STACK = 4,
	MEM_OBJ_SYS_STRINGS = 5,
	MEM_OBJ_LISTS = 6,
	MEM_OBJ_NODES = 7,
	MEM_OBJ_HUNK = 8,
	MEM_OBJ_DYNMEM = 9,
	MEM_OBJ_RESERVED = 10,
	MEM_OBJ_MAX = MEM_OBJ_RESERVED // For sanity checking
};

struct mem_obj_t {
	memObjType type;
	int segmgr_id; /* Internal value used by the seg_manager's hash map */
	union {
		script_t script;
		clone_table_t clones;
		local_variables_t locals;
		dstack_t stack;
		sys_strings_t sys_strings;
		list_table_t lists;
		node_table_t nodes;
		hunk_table_t hunks;
		dynmem_t dynmem;
		char *reserved;
	} data;
};



struct selector_map_t {
	selector_t init; /* Init function */
	selector_t play; /* Play function (first function to be called) */
	selector_t replay; /* Replay function */
	selector_t x, y, z; /* Coordinates */
	selector_t priority;
	selector_t view, loop, cel; /* Description of a specific image */
	selector_t brLeft, brRight, brTop, brBottom; /* Bounding Rectangle */
	selector_t xStep, yStep; /* BR adjustments */
	selector_t nsLeft, nsRight, nsTop, nsBottom; /* View boundaries ('now seen') */
	selector_t text, font; /* Used by controls */
	selector_t type, state; /* Used by contols as well */
	selector_t doit; /* Called (!) by the Animate() system call */
	selector_t signal; /* Used by Animate() to control a view's behaviour */
	selector_t underBits; /* Used by the graphics subroutines to store backupped BG pic data */

	/* The following selectors are used by the Bresenham syscalls: */
	selector_t canBeHere; /* Funcselector: Checks for movement validity */
	selector_t client; /* The object that wants to be moved */
	selector_t cycler; /* The cycler of the client */
	selector_t dx, dy; /* Deltas */
	selector_t edgeHit;
	selector_t b_movCnt, b_i1, b_i2, b_di, b_xAxis, b_incr; /* Various Bresenham vars */
	selector_t completed;

	selector_t illegalBits; /* Used by CanBeHere */
	selector_t dispose;

	selector_t prevSignal; /* Used by DoSound */

	selector_t message, modifiers; /* Used by GetEvent */

	selector_t owner, handle;
	selector_t cue;
	selector_t number;

	selector_t max, cursor; /* Used by EditControl */
	selector_t mode; /* Used by text controls (-> DrawControl()) */

	selector_t wordFail, syntaxFail, semanticFail; /* Used by Parse() */

	selector_t claimed; /* Used generally by the event mechanism */

	selector_t elements; /* Used by SetSynonyms() */

	selector_t lsTop, lsBottom, lsRight, lsLeft; /* Used by Animate() subfunctions and scroll list controls */

	selector_t baseSetter; /* Alternative baseSetter */

	selector_t who, distance; /* Used for 'chasing' movers */

	selector_t looper, mover, isBlocked, heading; /* Used in DoAvoider */

	selector_t caller, moveDone, moveSpeed; /* Used for DoBresen */

	selector_t delete_; /* Called by Animate() to dispose a view object */

	selector_t vol;
	selector_t pri;

	selector_t min;	/* SMPTE time format */
	selector_t sec;
	selector_t frame;

	selector_t dataInc;
	selector_t size;
	selector_t palette;
	selector_t cantBeHere;
	selector_t nodePtr;
	selector_t flags;

	selector_t points; /* Used by AvoidPath() */
}; /* Contains selector IDs for a few selected selectors */

struct view_object_t {
	reg_t obj;
	reg_t *signalp;    /* Used only indirectly */
	reg_t *underBitsp; /* The same goes for the handle storage */
	int underBits; /* Copy of the underbits: Needed for cleanup */

	int x, y;
	int priority;
	byte *view;
	int view_nr, loop, cel; /* view_nr is ised for save/restore */
	int nsTop, nsLeft, nsRight, nsBottom;
	int real_y, z, index_nr; /* Used for sorting */
};

#define VAR_GLOBAL 0
#define VAR_LOCAL 1
#define VAR_TEMP 2
#define VAR_PARAM 3

#define EXEC_STACK_TYPE_CALL 0
#define EXEC_STACK_TYPE_KERNEL 1
#define EXEC_STACK_TYPE_VARSELECTOR 2

struct exec_stack_t {
	reg_t objp;
	reg_t sendp; /* Pointer to the object containing the invoked method */
	union {
		reg_t *varp; /* Variable pointer for read/write access */
		reg_t pc; /* Not accurate for the TOS element */
	} addr;
	stack_ptr_t fp; /* Frame pointer */
	stack_ptr_t sp; /* Stack pointer */
	int argc;

	/* former variables[4]: [all other values are derived] */
	stack_ptr_t variables_argp; /* Argument pointer */
	seg_id_t local_segment; /* local variables etc. */

	selector_t selector; /* The selector which was used to call or -1 if not applicable */
	int origin;   /* The stack frame position the call was made from, or -1 if it
		      ** was the initial call.  */
	byte type; /* EXEC_STACK_TYPE* */

};

struct breakpoint_t {
	int type;
	union {
		uint32 address;  /* Breakpoints on exports */
		char *name; /* Breakpoints on selector names */
	} data;
	breakpoint_t *next;
};

#define BREAK_SELECTOR 1
/* Break when selector is executed. data contains (char *) selector name
   (in the format Object::Method) */

#define BREAK_EXPORT 2
/* Break when an exported function is called. data contains script_no << 16 |
   export_no. */

extern int script_debug_flag;
/* Set this to 1 to activate script debugging */

extern int script_error_flag;
/* Set to 1 to move pc back to last position, even though action is executed */

extern int script_checkloads_flag;
/* Displays the numbers of scripts when they are (un)loaded */

#define SCRIPT_ABORT_WITH_REPLAY 1025
extern int script_abort_flag;
/* Set this to 1 to abort script execution immediately. Aborting will leave the
** debug exec stack intact.
** Set it to SCRIPT_ABORT_WITH_REPLAY to force a replay afterwards.
*/

#define GC_INTERVAL 32768	/* Number of kernel calls in between gcs; should be < 50000 */

extern int script_gc_interval;
/* Initially GC_DELAY, can be set at runtime */

extern int script_step_counter;
/* Number of steps executed */


extern const char *(*_debug_get_input)(void);
/* The function used to get input for debugging */

extern int _debugstate_valid;
extern int _debug_seeking;
extern int _debug_step_running;


typedef int kernel_function(struct EngineState *s);

extern kernel_function* kfuncs[];
extern int max_instance;

/*inline*/
exec_stack_t *execute_method(EngineState *s, uint16 script, uint16 pubfunct, stack_ptr_t sp, reg_t calling_obj,
	uint16 argc, stack_ptr_t argp);
/* Executes function pubfunct of the specified script.
** Parameters: (EngineState *) s: The state which is to be executed with
**             (uint16) script: The script which is called
**             (uint16) pubfunct: The exported script function which is to be called
**             (stack_ptr_t) sp: Stack pointer position
**             (reg_t) calling_obj: The heap address of the object which executed the call
**             (uint16) argc: Number of arguments supplied
**             (stack_ptr_t) argp: Pointer to the first supplied argument
** Returns   : (exec_stack_t *): A pointer to the new exec stack TOS entry
*/


exec_stack_t *send_selector(EngineState *s, reg_t send_obj, reg_t work_obj,
	stack_ptr_t sp, int framesize, stack_ptr_t argp);
/* Executes a "send" or related operation to a selector
** Parameters: (EngineState *) s: The EngineState to operate on
**             (reg_t) send_obj: Heap address of the object to send to
**             (reg_t) work_obj: Heap address of the object initiating the send
**             (stack_ptr_t) sp: Stack pointer position
**             (int) framesize: Size of the send as determined by the "send" operation
**             (stack_ptr_t) argp: Pointer to the beginning of the heap block containing the
**                              data to be send. This area is a succession of one or more
**                              sequences of [selector_number][argument_counter] and then
**                              "argument_counter" word entries with the parameter values.
** Returns   : (exec_stack_t *): A pointer to the new execution stack TOS entry
*/


#define SCI_XS_CALLEE_LOCALS -1

exec_stack_t *add_exec_stack_entry(EngineState *s, reg_t pc, stack_ptr_t sp, reg_t objp, int argc,
	stack_ptr_t argp, selector_t selector, reg_t sendp, int origin, seg_id_t local_segment);
/* Adds an entry to the top of the execution stack
** Parameters: (EngineState *) s: The state with which to execute
**             (reg_t) pc: The initial program counter
**             (stack_ptr_t) sp: The initial stack pointer
**             (reg_t) objp: Pointer to the beginning of the current object
**             (int) argc: Number of parameters to call with
**             (stack_ptr_t) argp: Heap pointer to the first parameter
**             (selector_t) selector: The selector by which it was called or
**			 NULL_SELECTOR if n.a. For debugging.
**             (reg_t) sendp: Pointer to the object which the message was sent to.
**                       Equal to objp for anything but super.
**             (int) origin: Number of the execution stack element this entry was created by
**                       (usually the current TOS number, except for multiple sends).
**             (seg_id_t) local_segment: The segment to use for local variables,
**                        or SCI_XS_CALLEE_LOCALS to use obj's segment.
** Returns   : (exec_stack_t *): A pointer to the new exec stack TOS entry
*/


exec_stack_t *add_exec_stack_varselector(EngineState *s, reg_t objp, int argc, stack_ptr_t argp,
	selector_t selector, reg_t *address, int origin);
/* Adds one varselector access to the execution stack
** Parameters: (EngineState *) s: The EngineState to use
**             (reg_t) objp: Pointer to the object owning the selector
**             (int) argc: 1 for writing, 0 for reading
**             (stack_ptr_t) argp: Pointer to the address of the data to write -2
**             (int) selector: Selector name
**             (reg_t *) address: Heap address of the selector
**             (int) origin: Stack frame which the access originated from
** Returns   : (exec_stack_t *): Pointer to the new exec-TOS element
** This function is called from send_selector only.
*/


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


void script_debug(EngineState *s, reg_t *pc, stack_ptr_t *sp, stack_ptr_t *pp, reg_t *objp,
	int *restadjust, seg_id_t *segids, reg_t **variables, reg_t **variables_base,
	int *variables_nr, int bp);
/* Debugger functionality
** Parameters: (EngineState *) s: The state at which debugging should take place
**             (reg_t *) pc: Pointer to the program counter
**             (stack_ptr_t *) sp: Pointer to the stack pointer
**             (stack_ptr_t *) pp: Pointer to the frame pointer
**             (reg_t *) objp: Pointer to the object base pointer
**             (int *) restadjust: Pointer to the &rest adjustment value
**	       (seg_id_t *) segids: four-element array containing segment IDs for locals etc.
**	       (reg_t **) variables: four-element array referencing registers for globals etc.
**	       (reg_t **) variables_base: four-element array referencing
**                                        register bases for temps etc.
**	       (int *) variables_nr: four-element array giving sizes for params etc. (may be NULL)
**             (int) bp: Flag, set to 1 when a breakpoint is triggered
** Returns   : (void)
*/

int script_init_engine(EngineState *s, sci_version_t version);
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


int lookup_selector(EngineState *s, reg_t obj, selector_t selectorid, reg_t **vptr, reg_t *fptr);
/* Looks up a selector and returns its type and value
** Parameters: (EngineState *) s: The EngineState to use
**             (reg_t) obj: Address of the object to look the selector up in
**             (selector_t) selectorid: The selector to look up
** Returns   : (int) SELECTOR_NONE if the selector was not found in the object or its superclasses.
**                   SELECTOR_VARIABLE if the selector represents an object-relative variable
**                   SELECTOR_METHOD if the selector represents a method
**             (reg_t *) *vptr: A pointer to the storage space associated with the selector, if
**                              it is a variable
**             (reg_t) *fptr: A reference to the function described by that selector, if it is
**                            a valid function selector.
** *vptr is written to iff it is non-NULL and the selector indicates a property of the object.
** *fptr is written to iff it is non-NULL and the selector indicates a member function of that object.
*/


#define SCRIPT_GET_DONT_LOAD 0 /* Fail if not loaded */
#define SCRIPT_GET_LOAD 1 /* Load, if neccessary */
#define SCRIPT_GET_LOCK 3 /* Load, if neccessary, and lock */

seg_id_t script_get_segment(EngineState *s, int script_id, int load);
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


int game_save_state(EngineState *s, char *name, int coredump);
/* Saves the game state to the harddisk
** Parameters: (EngineState *) s: The game state to save
**             (char *) name: Name of the subdirectory (relative to s->save_dir)
**             (int) coredump: Set to non-zero in order to write additional debug information
** Returns   : (int) 0 on success, 1 otherwise
*/


EngineState *game_restore_state(char *name);
/* Restores the game state from a file
** Parameters: (char *) name: Name of the saved game state to restore
** Returns   : (EngineState *): The restored game state, or NULL on failure
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

void script_map_selectors(EngineState *s, selector_map_t *map);
/* Maps special selectors
** Parameters: (EngineState *) s: The state from which the selector information should be taken
**             (selector_map_t *) map: Pointer to the selector map to map
** Returns   : (void)
** Called by script_run();
*/

int script_map_kernel(EngineState *s);
/* Maps kernel functions
** Parameters: (EngineState *) s: The state which the kernel_names are retrieved from
** Returns   : (void)
** This function reads from and writes to s. It is called by script_run().
*/


void script_detect_versions(EngineState *s);
/* Detects SCI versions by their different script header
** Parameters: (EngineState *) s: The state to operate on
** Returns   : (void)
*/

reg_t kalloc(EngineState *s, const char *type, int space);
/* Allocates "kernel" memory and returns a handle suitable to be passed on to SCI scripts
** Parameters: (EngineState *) s: Pointer to the EngineState to operate on
**             (const char *) type: A free-form type description string (static)
**             (int) space: The space to allocate
** Returns   : (reg_t) The handle
*/

int has_kernel_function(EngineState *s, const char *kname);
/* Detects whether a particular kernel function is required in the game
** Parameters: (EngineState *) s: Pointer to the EngineState to operate on
**             (const char *) kname: The name of the desired kernel function
** Returns   : (int) 1 if the kernel function is listed in the kernel table,
**                   0 otherwise
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

object_t *obj_get(EngineState *s, reg_t offset);
/* Retreives an object from the specified location
** Parameters: (EngineState *) s: Pointer to the EngineState to operate on
**             (reg_t) offset: The object's offset
** Returns   : (object_t *) The object in question, or NULL if there is none
*/

} // End of namespace Sci

#endif /* !_SCI_VM_H */
