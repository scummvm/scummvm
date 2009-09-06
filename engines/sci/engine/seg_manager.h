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

#ifndef SCI_ENGINE_segMan_H
#define SCI_ENGINE_segMan_H

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

/**
 * Parameters for getScriptSegment().
 */
enum ScriptLoadType {
	SCRIPT_GET_DONT_LOAD = 0, /**< Fail if not loaded */
	SCRIPT_GET_LOAD = 1, /**< Load, if neccessary */
	SCRIPT_GET_LOCK = 3 /**< Load, if neccessary, and lock */
};


class SegManager : public Common::Serializable {
public:
	/**
	 * Initialize the segment manager.
	 */
	SegManager(ResourceManager *resMan);

	/**
	 * Deallocate all memory associated with the segment manager.
	 */
	~SegManager();

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

	// 1. Scripts

	/**
	 * Allocate a script into the segment manager.
	 * @param script_nr		The number of the script to load
	 * @param seg_id		The segment ID of the newly allocated segment,
	 * 						on success
	 * @return				0 on failure, 1 on success
	 */
	Script *allocateScript(int script_nr, SegmentId *seg_id);

	// The script must then be initialised; see section (1b.), below.

	/**
	 * Forcefully deallocate a previously allocated script.
	 * @param script_nr		number of the script to deallocate
	 * @return				1 on success, 0 on failure
	 */
	int deallocateScript(int script_nr);

	/**
	 * Determines whether a script has been loaded yet.
	 * @param seg	ID of the script segment to check for
	 */
	bool scriptIsLoaded(SegmentId seg);

	/**
	 * Validate whether the specified public function is exported by 
	 * the script in the specified segment.
	 * @param pubfunct		Index of the function to validate
	 * @param seg			Segment ID of the script the check is to 
	 * 						be performed for
	 * @return				NULL if the public function is invalid, its 
	 * 						offset into the script's segment otherwise
	 */
	uint16 validateExportFunc(int pubfunct, SegmentId seg);

	/**
	 * Determines the segment occupied by a certain script, if any.
	 * @param script_nr		Number of the script to look up
	 * @return				The script's segment ID, or -1 on failure
	 */
	SegmentId getScriptSegment(int script_nr) const;

	/**
	 * Determines the segment occupied by a certain script. Optionally
	 * load it, or load & lock it.
	 * @param[in] script_nr	Number of the script to look up
	 * @param[in] load		flag determining whether to load/lock the script
	 * @return				The script's segment ID, or -1 on failure
	 */
	SegmentId getScriptSegment(int script_nr, ScriptLoadType load);

	// TODO: document this
	reg_t getClassAddress(int classnr, ScriptLoadType lock, reg_t caller);

	/**
	 * Return a pointer to the specified script.
	 * If the id is invalid, does not refer to a script or the script is 
	 * not loaded, this will invoke error().
	 * @param seg	ID of the script segment to check for
	 * @return		A pointer to the Script object
	 */
	Script *getScript(SegmentId seg);

	/**
	 * Return a pointer to the specified script.
	 * If the id is invalid, does not refer to a script, or 
	 * the script is not loaded, this will return NULL
	 * @param seg	ID of the script segment to check for
	 * @return		A pointer to the Script object, or NULL
	 */
	Script *getScriptIfLoaded(SegmentId seg);

	/**
	 * Finds a unique segment by type
	 * @param type	The type of the segment to find
	 * @return		The segment number, or -1 if the segment wasn't found
	 */
	SegmentId findSegmentByType(int type);


	// 1b. Script Initialisation

	// The set of functions below are intended
	// to be used during script instantiation,
	// i.e. loading and linking.

	/**
	 * Initializes a script's local variable block
	 * All variables are initialized to zero.
	 * @param seg	Segment containing the script to initialize
	 * @param nr	Number of local variables to allocate
	 */
	void scriptInitialiseLocalsZero(SegmentId seg, int nr);

	/**
	 * Initializes a script's local variable block according to a prototype
	 * @param location	Location to initialize from
	 */
	void scriptInitialiseLocals(reg_t location);

	/**
	 * Initializes an object within the segment manager
	 * @param obj_pos	Location (segment, offset) of the object. It must
	 * 					point to the beginning of the script/class block 
	 * 					(as opposed to what the VM considers to be the 
	 * 					object location)
	 * @returns			A newly created Object describing the object,
	 * 					stored within the relevant script
	 */
	Object *scriptObjInit(reg_t obj_pos);

	/**
	 * Informs the segment manager that a code block must be relocated
	 * @param location	Start of block to relocate
	 */
	void scriptAddCodeBlock(reg_t location);

	/**
	 * Tells the segment manager whether exports are wide (32-bit) or not.
	 * @param flag	1 if exports are wide, 0 otherwise
	 */
	void setExportWidth(int flag);

	/**
	 * Processes a relocation block witin a script
	 *  This function is idempotent, but it must only be called after all
	 *  objects have been instantiated, or a run-time error will occur.
	 * @param obj_pos	Location (segment, offset) of the block
	 * @return			Location of the relocation block
	 */
	void scriptRelocate(reg_t block);

	/**
	 * Determines whether the script referenced by the indicated segment 
	 * is marked as being deleted.
	 * Will return 0 when applied to an invalid or non-script seg.
	 * @param seg	Segment ID of the script to investigate
	 * @return		1 iff seg points to a script and the segment is 
	 * 				deleted, 0 otherwise
	 */
	bool scriptIsMarkedAsDeleted(SegmentId seg);


	// 2. Clones

	/**
	 * Allocate a fresh clone
	 * @param addr The offset of the freshly allocated clone
	 * @return	Reference to the memory allocated for the clone
	 */
	Clone *allocateClone(reg_t *addr);


	/**
	 * Reconstructs clones. Used when restoring saved games
	 */
	void reconstructClones();

	// 3. Objects (static, from Scripts, and dynmic, from Clones)



	// 4. Stack

	/**
	 * Allocates a data stack
	 * @param size	Number of stack entries to reserve
	 * @param segid	Segment ID of the stack
	 * @return		The physical stack
	 */
	DataStack *allocateStack(int size, SegmentId *segid);


	// 5. System Strings

	/**
	 * Allocates a system string table
	 * See also sys_string_acquire();
	 * @param[in] segid	Segment ID of the stack
	 * @returns			The physical stack
	 */            
	SystemStrings *allocateSysStrings(SegmentId *segid);


	// 5. System Strings

	/**
	 * Allocates a string fragments segment
	 * See also stringfrag.h
	 * @return	Segment ID to use for string fragments
	 */
	SegmentId allocateStringFrags();


	// 6, 7. Lists and Nodes

	/**
	 * Allocate a fresh list
	 * @param[in] addr	The offset of the freshly allocated list
	 * @return			Reference to the memory allocated for the list
	 */
	List *alloc_List(reg_t *addr);

	/**
	 * Allocate a fresh node
	 * @param[in] addr	The offset of the freshly allocated node
	 * @return			Reference to the memory allocated for the node
	 */
	Node *alloc_Node(reg_t *addr);


	// 8. Hunk Memory

	/**
	 * Allocate a fresh chunk of the hunk
	 * @param[in] size		Number of bytes to allocate for the hunk entry
	 * @param[in] hunk_type	A descriptive string for the hunk entry, for 
	 *	 					debugging purposes
	 * @param[out] addr		The offset of the freshly allocated hunk entry
	 * @return				Reference to the memory allocated for the hunk
	 * 						piece
	 */
	Hunk *alloc_hunk_entry(const char *hunk_type, int size, reg_t *addr);

	/**
	 * Deallocates a hunk entry
	 * @param[in] addr	Offset of the hunk entry to delete
	 */
	void free_hunk_entry(reg_t addr);


	// 9. Dynamic Memory

	/**
	 * Allocate some dynamic memory
	 * @param[in]  size			Number of bytes to allocate
	 * @param[in]  description	A descriptive string for debugging purposes
	 * @param[out] addr			The offset of the freshly allocated X
	 * @return					Raw pointer into the allocated dynamic 
	 * 							memory
	 */
	unsigned char *allocDynmem(int size, const char *description, reg_t *addr);

	/**
	 * Deallocates a piece of dynamic memory
	 * @param[in] addr	Offset of the dynmem chunk to free
	 */
	int freeDynmem(reg_t addr);

	/**
	 * Gets the description of a dynmem segment
	 * @param[in] addr	Segment to describe
	 * @return			Pointer to the descriptive string set in allocDynmem
	 */
	const char *getDescription(reg_t addr);


	// 10. Reserved segments

	// Reserves a special-purpose segment
	// Parameters: (char *) name: A string name identifying the segment (the string is cloned and retained)
	// Returns   : A fresh segment ID for the segment in question
	// Reserved segments are never used by the segment manager.  They can be used to tag special-purpose addresses.
	// Segment 0 is implicitly reserved for numbers.
	//SegmentId sm_allocate_reserved_segment(char *name);


	// Generic Operations on Segments and Addresses

	/**
	 * Dereferences a raw memory pointer
	 * @param[in]  reg	The reference to dereference
	 * @param[out] size	(optional) The theoretical maximum size
	 * @return			The data block referenced
	 */
	byte *dereference(reg_t reg, int *size);


	void heapRelocate(reg_t block);
	void scriptRelocateExportsSci11(SegmentId seg);
	void scriptInitialiseObjectsSci11(SegmentId seg);
	int initialiseScript(Script &scr, int script_nr);

	SciVersion sciVersion() { return _resMan->sciVersion(); }

private:
	IntMapper *id_seg_map; ///< id - script id; seg - index of heap
public: // TODO: make private
	Common::Array<MemObject *> _heap;
	int reserved_id;
	int exports_wide;
	Common::Array<Class> _classtable; /**< Table of all classes */
	ResourceManager *_resMan;

	SegmentId Clones_seg_id; ///< ID of the (a) clones segment
	SegmentId Lists_seg_id; ///< ID of the (a) list segment
	SegmentId Nodes_seg_id; ///< ID of the (a) node segment
	SegmentId Hunks_seg_id; ///< ID of the (a) hunk segment

private:
	MemObject *allocNonscriptSegment(MemObjectType type, SegmentId *segid);
	LocalVariables *allocLocalsSegment(Script *scr, int count);
	MemObject *memObjAllocate(SegmentId segid, int hash_id, MemObjectType type);
	int deallocate(SegmentId seg, bool recursive);
	int createClassTable();

	Hunk *alloc_Hunk(reg_t *);

	int relocateLocal(Script *scr, SegmentId segment, int location);
	int relocateBlock(Common::Array<reg_t> &block, int block_location, SegmentId segment, int location);
	int relocateObject(Object *obj, SegmentId segment, int location);

	int findFreeId(int *id);
	void setScriptSize(Script &scr, int script_nr);
	Object *scriptObjInit0(reg_t obj_pos);
	Object *scriptObjInit11(reg_t obj_pos);

	/**
	 * Check segment validity
	 * @param[in] seg	The segment to validate
	 * @return 			false if 'seg' is an invalid segment, true if
	 * 					'seg' is a valid segment
	 */
	bool check(SegmentId seg);
};

} // End of namespace Sci

#endif // SCI_ENGINE_segMan
