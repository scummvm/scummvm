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

#include "common/scummsys.h"
#include "common/serializer.h"
#include "sci/engine/vm.h"
#include "sci/engine/memobj.h"

namespace Sci {

#define GET_SEGMENT(mgr, index, rtype) (((index) > 0 && (int)(mgr)._heap.size() > index) ?		\
		(((mgr)._heap[index] && (mgr)._heap[index]->getType() == rtype)? (mgr)._heap[index]	: NULL) : NULL)

#define GET_SEGMENT_ANY(mgr, index) (((index) > 0 && (int)(mgr)._heap.size() > index) ?			\
		(((mgr)._heap[index])? (mgr)._heap[index]	: NULL) : NULL)

#define GET_OBJECT_SEGMENT(mgr, index) (((index) > 0 && (int)(mgr)._heap.size() > index) ?		\
		(((mgr)._heap[index]	&& ((mgr)._heap[index]->getType() == MEM_OBJ_SCRIPT || (mgr)._heap[index]->getType() == MEM_OBJ_CLONES))? (mgr)._heap[index]	\
		: NULL): NULL)

class SegManager : public Common::Serializable {
public:
	// Initialize the segment manager
	SegManager(bool sci1_1);

	// Deallocate all memory associated with the segment manager
	~SegManager();

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

	// 1. Scripts

	// Allocate a script into the segment manager
	// Parameters: (int) script_nr: number of the script to load
	//	       (state_t *) s: The state containing resource manager handlers to load the
	//			      script data
	// Returns   : (int) 0 on failure, 1 on success
	//	       (int) *seg_id: The segment ID of the newly allocated segment, on success
	Script *allocateScript(EngineState *s, int script_nr, SegmentId *seg_id);

	// The script must then be initialised; see section (1b.), below.

	// Forcefully deallocate a previously allocated script
	// Parameters: (int) script_nr: number of the script to deallocate
	// Returns   : (int) 1 on success, 0 on failure
	int deallocateScript(int script_nr);

	/**
	 * Determines whether a script has been loaded yet.
	 * @param seg	ID of the script segment to check for
	 */
	bool scriptIsLoaded(SegmentId seg);

	// Validate whether the specified public function is exported by the script in the specified segment
	// Parameters:	(int) pubfunct: Index of the function to validate
	//		(int) seg: Segment ID of the script the check is to be performed for
	// Returns   :  (uint16) 0 if the public function is invalid, its offset into the script's segment
	//			 otherwise
	uint16 validateExportFunc(int pubfunct, SegmentId seg);

	// Get the segment ID associated with a script number
	// Parameters: (int) script_nr: Number of the script to look up
	// Returns   : (int) The associated segment ID, or -1 if no matching segment exists
	SegmentId segGet(int script_nr) const;

	/**
	 * Return a pointer to the specified script. If the id is invalid, does not refer
	 * to a script or the script is not loaded, this will invoke error().
	 * @param seg	ID of the script segment to check for
	 * @return pointer to the Script object
	 */
	Script *getScript(SegmentId seg);

	/**
	 * Return a pointer to the specified script. If the id is invalid, does not refer
	 * @param seg	ID of the script segment to check for
	 * @return pointer to the Script object, or NULL
	 */
	Script *getScriptIfLoaded(SegmentId seg);




	// 1b. Script Initialisation

	// The set of functions below are intended
	// to be used during script instantiation,
	// i.e. loading and linking.

	// Initializes a script's local variable block
	// Parameters: (SegmentId) seg: Segment containing the script to initialize
	//             (int) nr: Number of local variables to allocate
	// All variables are initialized to zero.
	void scriptInitialiseLocalsZero(SegmentId seg, int nr);

	// Initializes a script's local variable block according to a prototype
	// Parameters: (reg_t) location: Location to initialize from
	void scriptInitialiseLocals(reg_t location);

	// Initializes an object within the segment manager
	// Parameters: (reg_t) obj_pos: Location (segment, offset) of the object
	// Returns   : (Object *) A newly created Object describing the object
	// obj_pos must point to the beginning of the script/class block (as opposed
	// to what the VM considers to be the object location)
	// The corresponding Object is stored within the relevant script.
	Object *scriptObjInit(EngineState *s, reg_t obj_pos);

	// Informs the segment manager that a code block must be relocated
	// Parameters: (reg_t) location: Start of block to relocate
	void scriptAddCodeBlock(reg_t location);

	// Tells the segment manager whether exports are wide (32-bit) or not.
	// Parameters: (int) flag: 1 if exports are wide, 0 otherwise
	void setExportWidth(int flag);

	// Processes a relocation block witin a script
	// Parameters: (reg_t) obj_pos: Location (segment, offset) of the block
	// Returns   : (Object *) Location of the relocation block
	// This function is idempotent, but it must only be called after all
	// objects have been instantiated, or a run-time error will occur.
	void scriptRelocate(reg_t block);

	// Determines whether the script referenced by the indicated segment is marked as being deleted.
	// Parameters: (SegmentId) Segment ID of the script to investigate
	// Returns   : (int) 1 iff seg points to a script and the segment is deleted, 0 otherwise
	// Will return 0 when applied to an invalid or non-script seg.
	bool scriptIsMarkedAsDeleted(SegmentId seg);


	// 2. Clones

	// Allocate a fresh clone
	// Returns : (Clone*): Reference to the memory allocated for the clone
	//           (reg_t) *addr: The offset of the freshly allocated clone
	Clone *alloc_Clone(reg_t *addr);


	// 3. Objects (static, from Scripts, and dynmic, from Clones)



	// 4. Stack

	// Allocates a data stack
	// Parameters: (int) size: Number of stack entries to reserve
	// Returns   : (DataStack *): The physical stack
	//             (SegmentId) segid: Segment ID of the stack
	DataStack *allocateStack(int size, SegmentId *segid);


	// 5. System Strings

	// Allocates a system string table
	// Returns   : (DataStack *): The physical stack
	//             (SegmentId) segid: Segment ID of the stack
	// See also sys_string_acquire();
	SystemStrings *allocateSysStrings(SegmentId *segid);


	// 5. System Strings

	// Allocates a string fragments segment
	// Returns   : (SegmentId): Segment ID to use for string fragments
	// See also stringfrag.h
	SegmentId allocateStringFrags();


	// 6, 7. Lists and Nodes

	// Allocate a fresh list
	// Returns : (listY_t*): Reference to the memory allocated for the list
	//           (reg_t) *addr: The offset of the freshly allocated list
	List *alloc_List(reg_t *addr);

	// Allocate a fresh node
	// Returns : (node_t*): Reference to the memory allocated for the node
	//           (reg_t) *addr: The offset of the freshly allocated node
	Node *alloc_Node(reg_t *addr);


	// 8. Hunk Memory

	// Allocate a fresh chunk of the hunk
	// Parameters: (int) size: Number of bytes to allocate for the hunk entry
	//             (const char *) hunk_type: A descriptive string for the hunk entry,
	//				for debugging purposes
	// Returns   : (Hunk *): Reference to the memory allocated for the hunk piece
	//             (reg_t) *addr: The offset of the freshly allocated hunk entry
	Hunk *alloc_hunk_entry(const char *hunk_type, int size, reg_t *addr);

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
	//SegmentId sm_allocate_reserved_segment(char *name);


	// Generic Operations on Segments and Addresses

	// Dereferences a raw memory pointer
	// Parameters: (reg_t) reg: The reference to dereference
	// Returns   : (byte *) The data block referenced
	//             (int) size: (optionally) the theoretical maximum size of it
	byte *dereference(reg_t reg, int *size);




	void heapRelocate(reg_t block);
	void scriptRelocateExportsSci11(SegmentId seg);
	void scriptInitialiseObjectsSci11(EngineState *s, SegmentId seg);
	int initialiseScript(Script &scr, EngineState *s, int script_nr);

private:
	IntMapper *id_seg_map; // id - script id; seg - index of heap
public: // TODO: make private
	Common::Array<MemObject *> _heap;
	int reserved_id;
	int exports_wide;
	bool isSci1_1;

	SegmentId Clones_seg_id; // ID of the (a) clones segment
	SegmentId Lists_seg_id; // ID of the (a) list segment
	SegmentId Nodes_seg_id; // ID of the (a) node segment
	SegmentId Hunks_seg_id; // ID of the (a) hunk segment

private:
	MemObject *allocNonscriptSegment(MemObjectType type, SegmentId *segid);
	LocalVariables *allocLocalsSegment(Script *scr, int count);
	MemObject *memObjAllocate(SegmentId segid, int hash_id, MemObjectType type);
	int deallocate(SegmentId seg, bool recursive);

	Hunk *alloc_Hunk(reg_t *);

	int relocateLocal(Script *scr, SegmentId segment, int location);
	int relocateBlock(Common::Array<reg_t> &block, int block_location, SegmentId segment, int location);
	int relocateObject(Object *obj, SegmentId segment, int location);

	int findFreeId(int *id);
	static void setScriptSize(Script &scr, EngineState *s, int script_nr);
	Object *scriptObjInit0(EngineState *s, reg_t obj_pos);
	Object *scriptObjInit11(EngineState *s, reg_t obj_pos);

	/* Check segment validity
	** Parameters: (int) seg: The segment to validate
	** Returns   : (bool)	false if 'seg' is an invalid segment
	**			true  if 'seg' is a valid segment
	*/
	bool check(SegmentId seg);

	void dbgPrint(const char* msg, void *i);	// for debug only

	// Perform garbage collection
	// Parameters: (EngineState *) s: The state to operate on
	// Effects   : Unreachable objects in 's' are deallocated
	//void sm_gc(EngineState *s);
};

} // End of namespace Sci

#endif // SCI_ENGINE_SEG_MANAGER
