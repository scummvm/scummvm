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

#ifndef SCI_ENGINE_SEG_MANAGER_H
#define SCI_ENGINE_SEG_MANAGER_H

#include "sci/engine/vm.h"

namespace Sci {

struct IntMapper;	// from intmap.h

// SCRIPT_ID must be 0
enum idFlag {
	SCRIPT_ID,
	SEG_ID
};

// Verify the the given condition is true, output the message if condition is false, and exit
// Parameters:
//   cond - condition to be verified
//   msg  - the message to be printed if condition fails
// return:
//   none, terminate the program if fails
#define VERIFY( cond, msg ) if (!(cond)) {\
		sciprintf("%s, line, %d, %s\n", __FILE__, __LINE__, msg); \
		BREAKPOINT(); \
	}

#define GET_SEGMENT(mgr, index, rtype) ((index) > 0 && (mgr).heap_size > index) ?		\
		(((mgr).heap[index] && (mgr).heap[index]->type == rtype)? (mgr).heap[index]	: NULL) : NULL

#define GET_SEGMENT_ANY(mgr, index) ((index) > 0 && (mgr).heap_size > index) ?			\
		(((mgr).heap[index])? (mgr).heap[index]	: NULL) : NULL

#define GET_OBJECT_SEGMENT(mgr, index) ((index) > 0 && (mgr).heap_size > index) ?		\
		(((mgr).heap[index]	&& ((mgr).heap[index]->type == MEM_OBJ_SCRIPT || (mgr).heap[index]->type == MEM_OBJ_CLONES))? (mgr).heap[index]	\
		: NULL): NULL

class SegInterface;

class SegManager {
public:
	// Initialize the segment manager
	SegManager(bool sci1_1);

	// Deallocate all memory associated with the segment manager
	~SegManager();

	// 1. Scripts

	void freeScript(mem_obj_t* mem);

	// Allocate a script into the segment manager
	// Parameters: (int) script_nr: number of the script to load
	//	       (state_t *) s: The state containing resource manager handlers to load the
	//			      script data
	// Returns   : (int) 0 on failure, 1 on success
	//	       (int) *seg_id: The segment ID of the newly allocated segment, on success
	mem_obj_t *allocateScript(EngineState *s, int script_nr, int* seg_id);

	// The script must then be initialised; see section (1b.), below.

	// Forcefully deallocate a previously allocated script
	// Parameters: (int) script_nr: number of the script to deallocate
	// Returns   : (int) 1 on success, 0 on failure
	int deallocateScript(int script_nr);

	// Determines whether a script has been loaded yet
	// Parameters: (int) id: number of the script or ID of the script segment to check for
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	int scriptIsLoaded(int id, idFlag flag);

	// Validate whether the specified public function is exported by the script in the specified segment
	// Parameters:	(int) pubfunct: Index of the function to validate
	//		(int) seg: Segment ID of the script the check is to be performed for
	// Returns   :  (uint16) 0 if the public function is invalid, its offset into the script's segment
	//			 otherwise
	uint16 validateExportFunc(int pubfunct, int seg);

	// Get the segment ID associated with a script number
	// Parameters: (int) script_nr: Number of the script to look up
	// Returns   : (int) The associated segment ID, or -1 if no matching segment exists
	// This function is "pure" (i.e, it doesn't modify anything).
	int segGet(int script_nr);


	// script lock operations

	// Increments the number of lockers of the script in question by one
	// Parameters: (int) id: ID of the script or script segment to modify
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	void incrementLockers(int id, idFlag flag);

	// Decrements the number of lockers of the script in question by one
	// Parameters: (int) id: ID of the script or script segment to modify
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	void decrementLockers(int id, idFlag flag);

	// Retrieves the number of locks held on this script
	// Parameters: (int) id: ID of the script or script segment to read from
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	// Returns   : (int) The number of locks held on the previously identified script
	int getLockers(int id, idFlag flag);

	// Sets the number of locks held on the specified script
	// Parameters: (int) id: ID of the script or script segment to modify
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	void setLockers(int lockers, int id, idFlag flag);

	// Retrieves a pointer to the synonyms associated with the specified script
	// Parameters: (int) id: ID of the script or script segment to read from
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	// Returns   : (byte *) Pointer to the synonyms, in non-parsed format.
	// A dynamic failure is issued if the specified ID does not reference a proper script.
	byte *getSynonyms(int id, idFlag flag);

	// Retrieves the number of synonyms associated with the specified script
	// Parameters: (int) id: ID of the script or script segment to read from
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	// Returns   : (int) The number of synonyms associated with the specified script
	// A dynamic failure is issued if the specified ID does not reference a proper script.
	int getSynonymsNr(int id, idFlag flag);


	// 1b. Script Initialisation

	// The set of functions below are intended
	// to be used during script instantiation,
	// i.e. loading and linking.

	// Initializes a script's local variable block
	// Parameters: (seg_id_t) seg: Segment containing the script to initialize
	//             (int) nr: Number of local variables to allocate
	// All variables are initialized to zero.
	void scriptInitialiseLocalsZero(seg_id_t seg, int nr);

	// Initializes a script's local variable block according to a prototype
	// Parameters: (reg_t) location: Location to initialize from
	void scriptInitialiseLocals(reg_t location);

	// Initializes an object within the segment manager
	// Parameters: (reg_t) obj_pos: Location (segment, offset) of the object
	// Returns   : (object_t *) A newly created object_t describing the object
	// obj_pos must point to the beginning of the script/class block (as opposed
	// to what the VM considers to be the object location)
	// The corresponding object_t is stored within the relevant script.
	object_t *scriptObjInit(EngineState *s, reg_t obj_pos);

	// Informs the segment manager that a code block must be relocated
	// Parameters: (reg_t) location: Start of block to relocate
	void scriptAddCodeBlock(reg_t location);

	// Tells the segment manager whether exports are wide (32-bit) or not.
	// Parameters: (int) flag: 1 if exports are wide, 0 otherwise
	void setExportWidth(int flag);

	// Processes a relocation block witin a script
	// Parameters: (reg_t) obj_pos: Location (segment, offset) of the block
	// Returns   : (object_t *) Location of the relocation block
	// This function is idempotent, but it must only be called after all
	// objects have been instantiated, or a run-time error will occur.
	void scriptRelocate(reg_t block);

	// Deallocates all unused but allocated entries for objects
	// Parameters: (seg_id_t) segid: segment of the script to prune in this way
	// These entries are created during script instantiation; deallocating them
	// frees up some additional memory.
	void scriptFreeUnusedObjects(seg_id_t segid);

	// Sets the script-relative offset of the exports table
	// Parameters: (int) offset: The script-relative exports table offset
	//	       (int) id: ID of the script or script segment to write to
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	// A dynamic failure is issued if the specified ID does not reference a proper script.
	void setExportTableOffset(int offset, int id, idFlag flag);

	// Sets the script-relative offset of the synonyms associated with the specified script
	// Parameters: (int) offset: The script-relative offset of the synonyms block
	//	       (int) id: ID of the script or script segment to write to
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	// A dynamic failure is issued if the specified ID does not reference a proper script.
	void setSynonymsOffset(int offset, int id, idFlag flag);

	// Sets the number of synonyms associated with the specified script
	// Parameters: (int) nr: The number of synonyms, as to be stored within the script
	//	       (int) id: ID of the script or script segment to write to
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	// A dynamic failure is issued if the specified ID does not reference a proper script.
	void setSynonymsNr(int nr, int id, idFlag flag);

	// Marks the script identified by its script number as deleted
	// Parameters: (int) script_nr: Script number to mark as deleted
	// This will not actually delete the script.  If references remain present on the
	// heap or the stack, the script will stay in memory in a quasi-deleted state until
	// either unreachable (resulting in its eventual deletion) or reloaded (resulting
	// in its data being updated).
	void markScriptDeleted(int script_nr);

	// Marks the script identified by its script number as not deleted
	// Parameters: (int) script_nr: Script number to mark as not deleted
	void unmarkScriptDeleted(int script_nr);

	// Determines whether the script referenced by the indicated segment is marked as being deleted.
	// Parameters: (seg_id_t) Segment ID of the script to investigate
	// Returns   : (int) 1 iff seg points to a script and the segment is deleted, 0 otherwise
	// Will return 0 when applied to an invalid or non-script seg.
	int scriptIsMarkedAsDeleted(seg_id_t seg);


	// 2. Clones

	// Allocate a fresh clone
	// Returns : (clone_t*): Reference to the memory allocated for the clone
	//           (reg_t) *addr: The offset of the freshly allocated clone
	clone_t *alloc_clone(reg_t *addr);

	// Deallocates a clone
	// Parameters: (reg_t) addr: Offset of the clone scheduled for termination
	void free_clone(reg_t addr);


	// 3. Objects (static, from Scripts, and dynmic, from Clones)

	// Not all of these functions are fully operational for clones ATM

	// Retrieves a 16 bit value from within a script's heap representation
	// Parameters: (reg_t) reg: The address to read from
	// Returns   : (int16) The value read from the specified location
	int16 getHeap(reg_t reg);

	// Writes a 16 bit value into a script's heap representation
	// Parameters: (reg_t) reg: The address to write to
	//	       (int16) value: The value to write
	//void putHeap(reg_t reg, int16 value);

	// Copies a byte string into a script's heap representation
	// Parameters: (int) dst: The script-relative offset of the destination area
	//	       (const void *) src: Pointer to the data source location
	//	       (size_t) n: Number of bytes to copy
	//	       (int) id: ID of the script or script segment to write to
	//             (idFlag) flag: Whether to address the script by script number (SCRIPT_ID) or
	//				by its segment (SEG_ID). SEG_ID is faster than SCRIPT_ID,
	//				but less convenient.
	// A dynamic failure is issued if the specified ID does not reference a proper script.
	void mcpyInOut(int dst, const void *src, size_t n, int id, int flag);


	// 4. Stack

	// Allocates a data stack
	// Parameters: (int) size: Number of stack entries to reserve
	// Returns   : (dstack_t *): The physical stack
	//             (seg_id_t) segid: Segment ID of the stack
	dstack_t *allocateStack(int size, seg_id_t *segid);


	// 5. System Strings

	// Allocates a system string table
	// Returns   : (dstack_t *): The physical stack
	//             (seg_id_t) segid: Segment ID of the stack
	// See also sys_string_acquire();
	sys_strings_t *allocateSysStrings(seg_id_t *segid);


	// 6, 7. Lists and Nodes

	// Allocate a fresh list
	// Returns : (listY_t*): Reference to the memory allocated for the list
	//           (reg_t) *addr: The offset of the freshly allocated list
	list_t *alloc_list(reg_t *addr);

	// Deallocates a list
	// Parameters: (reg_t) addr: Offset of the list scheduled for termination
	void free_list(reg_t addr);

	// Allocate a fresh node
	// Returns : (node_t*): Reference to the memory allocated for the node
	//           (reg_t) *addr: The offset of the freshly allocated node
	node_t *alloc_node(reg_t *addr);

	// Deallocates a list node
	// Parameters: (reg_t) addr: Offset of the node scheduled for termination
	void free_node(reg_t addr);


	// 8. Hunk Memory

	// Allocate a fresh chunk of the hunk
	// Parameters: (int) size: Number of bytes to allocate for the hunk entry
	//             (const char *) hunk_type: A descriptive string for the hunk entry,
	//				for debugging purposes
	// Returns   : (hunk_t*): Reference to the memory allocated for the hunk piece
	//             (reg_t) *addr: The offset of the freshly allocated hunk entry
	hunk_t *alloc_hunk_entry(const char *hunk_type, int size, reg_t *addr);

	// Deallocates a hunk entry
	// Parameters: (reg_t) addr: Offset of the hunk entry to delete
	void free_hunk_entry(reg_t addr);


	// 9. Dynamic Memory

	// Allocate some dynamic memory
	// Parameters: (int) size: Number of bytes to allocate
	//             (const char_ *) description: A descriptive string,
	//				for debugging purposes
	// Returns   : (unsigned char*): Raw pointer into the allocated dynamic memory
	//             (reg_t) *addr: The offset of the freshly allocated X
	unsigned char *allocDynmem(int size, const char *description, reg_t *addr);

	// Deallocates a piece of dynamic memory
	// Parameters: (reg_t) addr: Offset of the dynmem chunk to free
	int freeDynmem(reg_t addr);

	// Gets the description of a dynmem segment
	// Parameters: (reg_t) addr: Segment to describe
	// Returns   : (const char *): Pointer to the descriptive string set in
	// allocDynmem
	const char *getDescription(reg_t addr);


	// 10. Reserved segments

	// Reserves a special-purpose segment
	// Parameters: (char *) name: A string name identifying the segment (the string is cloned and retained)
	// Returns   : A fresh segment ID for the segment in question
	// Reserved segments are never used by the segment manager.  They can be used to tag special-purpose addresses.
	// Segment 0 is implicitly reserved for numbers.
	//seg_id_t sm_allocate_reserved_segment(char *name);


	// Generic Operations on Segments and Addresses

	// Dereferences a raw memory pointer
	// Parameters: (reg_t) reg: The reference to dereference
	// Returns   : (byte *) The data block referenced
	//             (int) size: (optionally) the theoretical maximum size of it
	byte *dereference(reg_t reg, int *size);


	// Segment Interface

	// Retrieves the segment interface to the specified segment
	// Parameters: (seg_id_t) segid: ID of the segment to look up
	// Returns   : (SegInterface *): An interface to the specified segment ID, or NULL on error
	// The returned interface must be deleted after use
	SegInterface *getSegInterface(seg_id_t segid);


	void heapRelocate(EngineState *s, reg_t block);
	void scriptRelocateExportsSci11(int seg);
	void scriptInitialiseObjectsSci11(EngineState *s, int seg);
	int scriptMarkedDeleted(int script_nr);
	int initialiseScript(mem_obj_t *mem, EngineState *s, int script_nr);

public: // TODO: make private
	IntMapper *id_seg_map; // id - script id; seg - index of heap
	mem_obj_t **heap;
	int heap_size;		// size of the heap
	int reserved_id;
	int exports_wide;
	bool isSci1_1;

	int gc_mark_bits;
	// For standard Mark&Sweep:
	// 1 or 0, depending on what unreachable/freshly allocated
	// memory is tagged as
	size_t mem_allocated; // Total amount of memory allocated

	seg_id_t clones_seg_id; // ID of the (a) clones segment
	seg_id_t lists_seg_id; // ID of the (a) list segment
	seg_id_t nodes_seg_id; // ID of the (a) node segment
	seg_id_t hunks_seg_id; // ID of the (a) hunk segment

private:
	mem_obj_t *allocNonscriptSegment(memObjType type, seg_id_t *segid);
	local_variables_t *allocLocalsSegment(script_t *scr, int count);
	mem_obj_t *memObjAllocate(seg_id_t segid, int hash_id, memObjType type);
	int deallocate(int seg, bool recursive);

	hunk_t *alloc_hunk(reg_t *);
	void free_hunk(reg_t addr);

	inline int relocateLocal(script_t *scr, seg_id_t segment, int location);
	inline int relocateBlock(reg_t *block, int block_location, int block_items, seg_id_t segment, int location);
	inline int relocateObject(object_t *obj, seg_id_t segment, int location);

	inline int findFreeId(int *id);
	static void setScriptSize(mem_obj_t *mem, EngineState *s, int script_nr);
	object_t *scriptObjInit0(EngineState *s, reg_t obj_pos);
	object_t *scriptObjInit11(EngineState *s, reg_t obj_pos);

	/* Check segment validity
	** Parameters: (int) seg: The segment to validate
	** Returns   : (bool)	false if 'seg' is an invalid segment
	**			true  if 'seg' is a valid segment
	*/
	bool check(int seg);

	void dbgPrint(const char* msg, void *i);	// for debug only

	// Perform garbage collection
	// Parameters: (EngineState *) s: The state to operate on
	// Effects   : Unreachable objects in 's' are deallocated
	//void sm_gc(EngineState *s);
};


// 11. Segment interface, primarily for GC

class SegInterface {
protected:
	SegInterface(SegManager *segmgr, mem_obj_t *mobj, seg_id_t segId, memObjType typeId);

public:
	// Deallocates the segment interface
	virtual ~SegInterface() {}

	// Finds the canonic address associated with sub_reg
	// Parameters: (reg_t) sub_addr: The base address whose canonic address is to be found
	// For each valid address a, there exists a canonic address c(a) such that c(a) = c(c(a)).
	// This address "governs" a in the sense that deallocating c(a) will deallocate a.
	virtual reg_t findCanonicAddress(reg_t sub_addr);

	// Deallocates all memory associated with the specified address
	// Parameters: (reg_t) sub_addr: The address (within the given segment) to deallocate
	virtual void freeAtAddress(reg_t sub_addr);

	// Iterates over and reports all addresses within the current segment
	// Parameters: note : (voidptr * addr) -> (): Invoked for each address on which free_at_address()
	//                                makes sense
	//             (void *) param: Parameter passed to 'note'
	virtual void listAllDeallocatable(void *param, void (*note)(void *param, reg_t addr));

	// Iterates over all references reachable from the specified object
	// Parameters: (reg_t) object: The object (within the current segment) to analyse
	//             (void *) param: Parameter passed to 'note'
	//             note : (voidptr * addr) -> (): Invoked for each outgoing reference within the object
	// Note: This function may also choose to report numbers (segment 0) as adresses
	virtual void listAllOutgoingReferences(EngineState *s, reg_t object, void *param, void (*note)(void *param, reg_t addr));

	// Get the memory object
	mem_obj_t *getMobj() { return _mobj; }

	// Get the segment type
	memObjType getType() { return _typeId; }

protected:
	SegManager *_segmgr;
	mem_obj_t *_mobj;
	seg_id_t _segId;

private:
	memObjType _typeId; // Segment type
};

} // End of namespace Sci

#endif // SCI_ENGINE_SEG_MANAGER
