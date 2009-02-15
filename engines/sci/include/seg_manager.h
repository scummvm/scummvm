/***************************************************************************
 seg_manager.h Copyright (C) 2002 Xiaojun Chen, Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/


#ifndef _SCI_SEG_MANAGER_H
#define _SCI_SEG_MANAGER_H

#include "sci/include/int_hashmap.h"
#include "sci/include/sys_strings.h"
#include "sci/include/vm.h"

#define DEFAULT_SCRIPTS 32
#define DEFAULT_OBJECTS 8	    /* default # of objects per script */
#define DEFAULT_OBJECTS_INCREMENT 4 /* Number of additional objects to
				    ** instantiate if we're running out of them  */

/* SCRIPT_ID must be 0 */
typedef enum {
	SCRIPT_ID,
	SEG_ID
} id_flag;

//void dbg_print( const char* msg, void *i );	/* for debug only */

/* verify the the given condition is true, output the message if condition is false, and exit
** Parameters:
**   cond - condition to be verified
**   msg  - the message to be printed if condition fails
** return:
**   none, terminate the program if fails
*/
#define VERIFY( cond, msg ) if (! ( cond ) ) {\
	sciprintf( "%s, line, %d, %s\n", __FILE__, __LINE__, msg ); \
	BREAKPOINT(); \
	}


#define MEM_OBJ_INVALID		0
#define MEM_OBJ_SCRIPT		1
#define MEM_OBJ_CLONES		2
#define MEM_OBJ_LOCALS		3
#define MEM_OBJ_STACK		4
#define MEM_OBJ_SYS_STRINGS	5
#define MEM_OBJ_LISTS		6
#define MEM_OBJ_NODES		7
#define MEM_OBJ_HUNK		8
#define MEM_OBJ_DYNMEM		9
#define MEM_OBJ_RESERVED	10
#define MEM_OBJ_MAX		MEM_OBJ_RESERVED /* For sanity checking */
typedef int mem_obj_enum;

struct _mem_obj;

#define GET_SEGMENT(mgr, index, rtype) ((index) > 0 && (mgr).heap_size > index)?		\
		(((mgr).heap[index] && (mgr).heap[index]->type == rtype)? (mgr).heap[index]	\
		: NULL) /* Type does not match */						\
	: NULL /* Invalid index */

#define GET_SEGMENT_ANY(mgr, index) ((index) > 0 && (mgr).heap_size > index)?			\
		(((mgr).heap[index])? (mgr).heap[index]						\
		: NULL) /* Type does not match */						\
	: NULL /* Invalid index */

#define GET_OBJECT_SEGMENT(mgr, index) ((index) > 0 && (mgr).heap_size > index)?		\
		(((mgr).heap[index]								\
                    && ((mgr).heap[index]->type == MEM_OBJ_SCRIPT				\
		        || (mgr).heap[index]->type == MEM_OBJ_CLONES))? (mgr).heap[index]	\
		: NULL) /* Type does not match */						\
	: NULL /* Invalid index */


typedef struct _seg_manager_t {
	int_hash_map_t* id_seg_map; /* id - script id; seg - index of heap */
	struct _mem_obj** heap;
	int heap_size;		/* size of the heap */
	int reserved_id;
	int exports_wide;
	int sci1_1;

	int gc_mark_bits; /* For standard Mark&Sweep:
			  ** 1 or 0, depending on what unreachable/freshly allocated
			  ** memory is tagged as  */
	size_t mem_allocated; /* Total amount of memory allocated */

	seg_id_t clones_seg_id; /* ID of the (a) clones segment */
	seg_id_t lists_seg_id; /* ID of the (a) list segment */
	seg_id_t nodes_seg_id; /* ID of the (a) node segment */
	seg_id_t hunks_seg_id; /* ID of the (a) hunk segment */
} seg_manager_t;



/*==============================================================*/
/* Toplevel functionality					*/
/*==============================================================*/
void
sm_init (seg_manager_t* self, int sci1_1);
/* Initialize the segment manager
*/

void
sm_destroy (seg_manager_t* self);
/* Deallocate all memory associated with the segment manager
*/

void
sm_gc(seg_manager_t *self, struct _state *s);
/* Perform garbage collection
** Parameters: (state_t *) s: The state to operate on
** Effects   : Unreachable objects in 's' are deallocated
*/



/*==============================================================*/
/* 1. Scripts							*/
/*==============================================================*/


void
sm_free_script ( mem_obj_t* mem );

mem_obj_t*
sm_allocate_script(struct _seg_manager_t* self, struct _state *s, int script_nr, int* seg_id);
/* Allocate a script into the segment manager
** Parameters: (int) script_nr: number of the script to load
**	       (state_t *) s: The state containing resource manager handlers to load the
**			      script data
** Returns   : (int) 0 on failure, 1 on success
**	       (int) *seg_id: The segment ID of the newly allocated segment, on success

** The script must then be initialised; see section (1b.), below.
*/

int
sm_deallocate_script(struct _seg_manager_t* self, int script_nr);
/* Forcefully deallocate a previously allocated script
** Parameters: (int) script_nr: number of the script to deallocate
** Returns   : (int) 1 on success, 0 on failure
*/

int
sm_script_is_loaded(struct _seg_manager_t* self, int id, id_flag flag);
/* Determines whether a script has been loaded yet
** Parameters: (int) id: number of the script or ID of the script segment to check for
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
*/ 

guint16
sm_validate_export_func(struct _seg_manager_t* self, int pubfunct, int seg);
/* Validate whether the specified public function is exported by the script in the specified segment
** Parameters:	(int) pubfunct: Index of the function to validate
**		(int) seg: Segment ID of the script the check is to be performed for
** Returns   :  (guint16) 0 if the public function is invalid, its offset into the script's segment
**			  otherwise
**/

int
sm_seg_get (seg_manager_t* self, int script_nr);
/* Get the segment ID associated with a script number
** Parameters: (int) script_nr: Number of the script to look up
** Returns   : (int) The associated segment ID, or -1 if no matching segment exists
** This function is "pure" (i.e, it doesn't modify anything).
*/


/**************************/
/* script lock operations */
/**************************/

void
sm_increment_lockers(struct _seg_manager_t* self, int id, id_flag flag);
/* Increments the number of lockers of the script in question by one
** Parameters: (int) id: ID of the script or script segment to modify
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
*/ 

void
sm_decrement_lockers(struct _seg_manager_t* self, int id, id_flag flag);
/* Decrements the number of lockers of the script in question by one
** Parameters: (int) id: ID of the script or script segment to modify
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
*/ 

int
sm_get_lockers(struct _seg_manager_t* self, int id, id_flag flag);
/* Retrieves the number of locks held on this script
** Parameters: (int) id: ID of the script or script segment to read from
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
** Returns   : (int) The number of locks held on the previously identified script
*/ 

void
sm_set_lockers(struct _seg_manager_t* self, int lockers, int id, id_flag flag);
/* Sets the number of locks held on the specified script
** Parameters: (int) id: ID of the script or script segment to modify
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
*/ 

byte *
sm_get_synonyms (struct _seg_manager_t* self, int id, id_flag flag);
/* Retrieves a pointer to the synonyms associated with the specified script
** Parameters: (int) id: ID of the script or script segment to read from
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
** Returns   : (byte *) Pointer to the synonyms, in non-parsed format.
** A dynamic failure is issued if the specified ID does not reference a proper script.
*/

int
sm_get_synonyms_nr (struct _seg_manager_t* self, int id, id_flag flag);
/* Retrieves the number of synonyms associated with the specified script
** Parameters: (int) id: ID of the script or script segment to read from
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
** Returns   : (int) The number of synonyms associated with the specified script
** A dynamic failure is issued if the specified ID does not reference a proper script.
*/


/*==============================================================*/
/* 1b. Script Initialisation					*/
/*==============================================================*/

/*******************************************/
/* The set of functions below are intended */
/* to be used during script instantiation, */
/* i.e. loading and linking.		   */
/*******************************************/

void
sm_script_initialise_locals_zero(struct _seg_manager_t *self, seg_id_t seg, int nr);
/* Initializes a script's local variable block
** Parameters: (seg_id_t) seg: Segment containing the script to initialize
**             (int) nr: Number of local variables to allocate
** All variables are initialized to zero.
*/

void
sm_script_initialise_locals(struct _seg_manager_t *self, reg_t location);
/* Initializes a script's local variable block according to a prototype
** Parameters: (reg_t) location: Location to initialize from
*/

object_t *
sm_script_obj_init(seg_manager_t *self, struct _state *s, reg_t obj_pos);
/* Initializes an object within the segment manager
** Parameters: (reg_t) obj_pos: Location (segment, offset) of the object
** Returns   : (object_t *) A newly created object_t describing the object
** obj_pos must point to the beginning of the script/class block (as opposed
** to what the VM considers to be the object location)
** The corresponding object_t is stored within the relevant script.
*/

void
sm_script_add_code_block(struct _seg_manager_t* self, reg_t location);
/* Informs the segment manager that a code block must be relocated
** Parameters: (reg_t) location: Start of block to relocate
*/

void
sm_set_export_width(struct _seg_manager_t* self, int flag);
/* Tells the segment manager whether exports are wide (32-bit) or not.
** Parameters: (int) flag: 1 if exports are wide, 0 otherwise */

void
sm_script_relocate(struct _seg_manager_t* self, reg_t block);
/* Processes a relocation block witin a script
** Parameters: (reg_t) obj_pos: Location (segment, offset) of the block
** Returns   : (object_t *) Location of the relocation block
** This function is idempotent, but it must only be called after all
** objects have been instantiated, or a run-time error will occur.
*/

void
sm_script_free_unused_objects(struct _seg_manager_t *self, seg_id_t segid);
/* Deallocates all unused but allocated entries for objects
** Parameters: (seg_id_t) segid: segment of the script to prune in this way
** These entries are created during script instantiation; deallocating them
** frees up some additional memory.
*/

void
sm_set_export_table_offset (struct _seg_manager_t* self, int offset, int id, id_flag flag);
/* Sets the script-relative offset of the exports table
** Parameters: (int) offset: The script-relative exports table offset
**	       (int) id: ID of the script or script segment to write to
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
** A dynamic failure is issued if the specified ID does not reference a proper script.
*/

void
sm_set_synonyms_offset (struct _seg_manager_t* self, int offset, int id, id_flag flag);
/* Sets the script-relative offset of the synonyms associated with the specified script
** Parameters: (int) offset: The script-relative offset of the synonyms block
**	       (int) id: ID of the script or script segment to write to
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
** A dynamic failure is issued if the specified ID does not reference a proper script.
*/

void
sm_set_synonyms_nr (struct _seg_manager_t* self, int nr, int id, id_flag flag);
/* Sets the number of synonyms associated with the specified script
** Parameters: (int) nr: The number of synonyms, as to be stored within the script
**	       (int) id: ID of the script or script segment to write to
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
** A dynamic failure is issued if the specified ID does not reference a proper script.
*/

void
sm_mark_script_deleted(seg_manager_t* self, int script_nr);
/* Marks the script identified by its script number as deleted
** Parameters: (int) script_nr: Script number to mark as deleted
** This will not actually delete the script.  If references remain present on the
** heap or the stack, the script will stay in memory in a quasi-deleted state until
** either unreachable (resulting in its eventual deletion) or reloaded (resulting
** in its data being updated).
*/

void
sm_unmark_script_deleted(seg_manager_t* self, int script_nr);
/* Marks the script identified by its script number as not deleted
** Parameters: (int) script_nr: Script number to mark as not deleted
*/

int
sm_script_is_marked_as_deleted(seg_manager_t* self, seg_id_t seg);
/* Determines whether the script referenced by the indicated segment is marked as being deleted.
** Parameters: (seg_id_t) Segment ID of the script to investigate
** Returns   : (int) 1 iff seg points to a script and the segment is deleted, 0 otherwise
** Will return 0 when applied to an invalid or non-script seg.
*/



/*==============================================================*/
/* 2. Clones							*/
/*==============================================================*/

clone_t*
sm_alloc_clone(struct _seg_manager_t *self, reg_t *addr);
/* Allocate a fresh clone
** Returns : (clone_t*): Reference to the memory allocated for the clone
**           (reg_t) *addr: The offset of the freshly allocated clone
*/

void
sm_free_clone(struct _seg_manager_t *self, reg_t addr);
/* Deallocates a clone
** Parameters: (reg_t) addr: Offset of the clone scheduled for termination
*/



/*==============================================================*/
/* Objects (static, from Scripts, and dynmic, from Clones)	*/
/*==============================================================*/

/* Not all of these functions are fully operational for clones ATM */

gint16 
sm_get_heap(struct _seg_manager_t* self, reg_t reg );
/* Retrieves a 16 bit value from within a script's heap representation
** Parameters: (reg_t) reg: The address to read from
** Returns   : (gint16) The value read from the specified location
*/

void
sm_put_heap(struct _seg_manager_t* self, reg_t reg, gint16 value );
/* Writes a 16 bit value into a script's heap representation
** Parameters: (reg_t) reg: The address to write to
**	       (gint16) value: The value to write
*/

void
sm_mcpy_in_out (seg_manager_t* self, int dst, const void* src, size_t n, int id, int flag);
/* Copies a byte string into a script's heap representation
** Parameters: (int) dst: The script-relative offset of the destination area
**	       (const void *) src: Pointer to the data source location
**	       (size_t) n: Number of bytes to copy
**	       (int) id: ID of the script or script segment to write to
**             (id_flag) flag: Whether to address the script by script number (SCRIPT_ID) or
**				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
**				but less convenient.
** A dynamic failure is issued if the specified ID does not reference a proper script.
*/


/*==============================================================*/
/* 4. Stack							*/
/*==============================================================*/

dstack_t *
sm_allocate_stack(struct _seg_manager_t *self, int size, seg_id_t *segid);
/* Allocates a data stack
** Parameters: (int) size: Number of stack entries to reserve
** Returns   : (dstack_t *): The physical stack
**             (seg_id_t) segid: Segment ID of the stack
*/



/*==============================================================*/
/* 5. System Strings						*/
/*==============================================================*/

sys_strings_t *
sm_allocate_sys_strings(struct _seg_manager_t *self, seg_id_t *segid);
/* Allocates a system string table
** Returns   : (dstack_t *): The physical stack
**             (seg_id_t) segid: Segment ID of the stack
** See also sys_string_acquire();
*/



/*==============================================================*/
/* 6, 7. Lists and Nodes					*/
/*==============================================================*/

list_t*
sm_alloc_list(struct _seg_manager_t *self, reg_t *addr);
/* Allocate a fresh list
** Returns : (listY_t*): Reference to the memory allocated for the list
**           (reg_t) *addr: The offset of the freshly allocated list
*/

void
sm_free_list(struct _seg_manager_t *self, reg_t addr);
/* Deallocates a list
** Parameters: (reg_t) addr: Offset of the list scheduled for termination
*/


node_t*
sm_alloc_node(struct _seg_manager_t *self, reg_t *addr);
/* Allocate a fresh node
** Returns : (node_t*): Reference to the memory allocated for the node
**           (reg_t) *addr: The offset of the freshly allocated node
*/

void
sm_free_node(struct _seg_manager_t *self, reg_t addr);
/* Deallocates a list node
** Parameters: (reg_t) addr: Offset of the node scheduled for termination
*/



/*==============================================================*/
/* 8. Hunk Memory						*/
/*==============================================================*/

hunk_t*
sm_alloc_hunk_entry(struct _seg_manager_t *self, const char *hunk_type, int size, reg_t *addr);
/* Allocate a fresh chunk of the hunk
** Parameters: (int) size: Number of bytes to allocate for the hunk entry
**             (const char *) hunk_type: A descriptive string for the hunk entry,
**				for debugging purposes
** Returns   : (hunk_t*): Reference to the memory allocated for the hunk piece
**             (reg_t) *addr: The offset of the freshly allocated hunk entry
*/

void
sm_free_hunk_entry(struct _seg_manager_t *self, reg_t addr);
/* Deallocates a hunk eentry
** Parameters: (reg_t) addr: Offset of the hunk entry to delete
*/



/*==============================================================*/
/* 9. Dynamic Memory						*/
/*==============================================================*/

unsigned char *
sm_alloc_dynmem(struct _seg_manager_t *self, int size, const char *description, reg_t *addr);
/* Allocate some dynamic memory
** Parameters: (int) size: Number of bytes to allocate
**             (const char_ *) description: A descriptive string,
**				for debugging purposes
** Returns   : (unsigned char*): Raw pointer into the allocated dynamic memory
**             (reg_t) *addr: The offset of the freshly allocated X
*/

int
sm_free_dynmem(struct _seg_manager_t *self, reg_t addr);
/* Deallocates a piece of dynamic memory
** Parameters: (reg_t) addr: Offset of the dynmem chunk to free
*/

const char *
sm_get_description(struct _seg_manager_t *self, reg_t addr);
/* Gets the description of a dynmem segment
** Parameters: (reg_t) addr: Segment to describe
** Returns   : (const char *): Pointer to the descriptive string set in
** sm_alloc_dynmem 
*/

/*==============================================================*/
/* 10. Reserved segments					*/
/*==============================================================*/

seg_id_t
sm_allocate_reserved_segment(struct _seg_manager_t *self, char *name);
/* Reserves a special-purpose segment
** Parameters: (char *) name: A string name identifying the segment (the string is cloned and retained)
** Returns   : A fresh segment ID for the segment in question
** Reserved segments are never used by the segment manager.  They can be used to tag special-purpose addresses.
** Segment 0 is implicitly reserved for numbers.
*/

/*==============================================================*/
/* Generic Operations on Segments and Addresses			*/
/*==============================================================*/

byte *
sm_dereference(struct _seg_manager_t *self, reg_t reg, int *size);
/* Dereferences a raw memory pointer
** Parameters: (reg_t) reg: The reference to dereference
** Returns   : (byte *) The data block referenced
**             (int) size: (optionally) the theoretical maximum size of it
*/


/*==============================================================*/
/* 11. Segment interface, primarily for GC			*/
/*==============================================================*/

typedef struct _seg_interface {
	seg_manager_t *segmgr;
	mem_obj_t *mobj;
	seg_id_t seg_id;
	mem_obj_enum type_id;	/* Segment type */
	const char *type;	/* String description of the segment type */

	reg_t
	(*find_canonic_address)(struct _seg_interface *self, reg_t sub_addr);
	/* Finds the canonic address associated with sub_reg
	** Parameters: (reg_t) sub_addr: The base address whose canonic address is to be found
	** For each valid address a, there exists a canonic address c(a) such that c(a) = c(c(a)).
	** This address "governs" a in the sense that deallocating c(a) will deallocate a.
	*/

	void
	(*free_at_address)(struct _seg_interface *self, reg_t sub_addr);
	/* Deallocates all memory associated with the specified address
	** Parameters: (reg_t) sub_addr: The address (within the given segment) to deallocate
	*/

	void
	(*list_all_deallocatable)(struct _seg_interface *self, void *param, void (*note)(void *param, reg_t addr));
	/* Iterates over and reports all addresses within the current segment
	** Parameters: note : (voidptr * addr) -> (): Invoked for each address on which free_at_address()
	**                                makes sense
	**             (void *) param: Parameter passed to 'note'
	*/

	void
	(*list_all_outgoing_references)(struct _seg_interface *self, struct _state *s, reg_t object, void *param, void (*note)(void *param, reg_t addr));
	/* Iterates over all references reachable from the specified object
	** Parameters: (reg_t) object: The object (within the current segment) to analyse
	**             (void *) param: Parameter passed to 'note'
	**             note : (voidptr * addr) -> (): Invoked for each outgoing reference within the object
	** Note: This function may also choose to report numbers (segment 0) as adresses
	*/

	void
	(*deallocate_self)(struct _seg_interface *self);
	/* Deallocates the segment interface
	 */

} seg_interface_t;

seg_interface_t *
get_seg_interface(seg_manager_t *self, seg_id_t segid);
/* Retrieves the segment interface to the specified segment
** Parameters: (seg_id_t) segid: ID of the segment to look up
** Returns   : (seg_interface_t *): An interface to the specified segment ID, or NULL on error
** The returned interface 'si' must be freed after use by calling 'si->dealloc_self(si)';
*/



#endif
