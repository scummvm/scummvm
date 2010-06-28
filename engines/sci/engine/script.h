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

#ifndef SCI_ENGINE_SCRIPT_H
#define SCI_ENGINE_SCRIPT_H

#include "common/str.h"
#include "sci/engine/segment.h"

namespace Sci {

struct EngineState;
class ResourceManager;

enum ScriptObjectTypes {
	SCI_OBJ_TERMINATOR,
	SCI_OBJ_OBJECT,
	SCI_OBJ_CODE,
	SCI_OBJ_SYNONYMS,
	SCI_OBJ_SAID,
	SCI_OBJ_STRINGS,
	SCI_OBJ_CLASS,
	SCI_OBJ_EXPORTS,
	SCI_OBJ_POINTERS,
	SCI_OBJ_PRELOAD_TEXT, /* This is really just a flag. */
	SCI_OBJ_LOCALVARS
};

typedef Common::HashMap<uint16, Object> ObjMap;

class Script : public SegmentObj {
private:
	int _nr; /**< Script number */
	byte *_buf; /**< Static data buffer, or NULL if not used */
	byte *_heapStart; /**< Start of heap if SCI1.1, NULL otherwise */

	int _lockers; /**< Number of classes and objects that require this script */
	size_t _scriptSize;
	size_t _heapSize;
	uint16 _bufSize;

	const uint16 *_exportTable; /**< Abs. offset of the export table or 0 if not present */
	uint16 _numExports; /**< Number of entries in the exports table */

	const byte *_synonyms; /**< Synonyms block or 0 if not present*/
	uint16 _numSynonyms; /**< Number of entries in the synonyms block */

	int _localsOffset;
	uint16 _localsCount;

	bool _markedAsDeleted;

public:
	/**
	 * Table for objects, contains property variables.
	 * Indexed by the TODO offset.
	 */
	ObjMap _objects;
	SegmentId _localsSegment; /**< The local variable segment */
	LocalVariables *_localsBlock;

public:
	int getLocalsOffset() const { return _localsOffset; }
	uint16 getLocalsCount() const { return _localsCount; }

	uint32 getScriptSize() const { return _scriptSize; }
	uint32 getHeapSize() const { return _heapSize; }
	uint32 getBufSize() const { return _bufSize; }
	const byte *getBuf(uint offset = 0) const { return _buf + offset; }

	int getScriptNumber() const { return _nr; }

public:
	Script();
	~Script();

	void freeScript();
	void init(int script_nr, ResourceManager *resMan);
	void load(ResourceManager *resMan);

	virtual bool isValidOffset(uint16 offset) const;
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) const;
	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const;
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	Common::Array<reg_t> listObjectReferences() const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

	Object *allocateObject(uint16 offset);
	Object *getObject(uint16 offset);
	const Object *getObject(uint16 offset) const;

	/**
	 * Initializes an object within the segment manager
	 * @param obj_pos	Location (segment, offset) of the object. It must
	 * 					point to the beginning of the script/class block
	 * 					(as opposed to what the VM considers to be the
	 * 					object location)
	 * @returns			A newly created Object describing the object,
	 * 					stored within the relevant script
	 */
	Object *scriptObjInit(reg_t obj_pos, bool fullObjectInit = true);

	/**
	 * Removes a script object
	 * @param obj_pos	Location (segment, offset) of the object.
	 */
	void scriptObjRemove(reg_t obj_pos);

	/**
	 * Initializes the script's local variables
	 * @param segMan	A reference to the segment manager
	 */
	void initialiseLocals(SegManager *segMan);

	/**
	 * Adds the script's classes to the segment manager's class table
	 * @param segMan	A reference to the segment manager
	 */
	void initialiseClasses(SegManager *segMan);

	/**
	 * Initializes the script's objects (SCI0)
	 * @param segMan	A reference to the segment manager
	 * @param segmentId	The script's segment id
	 */
	void initialiseObjectsSci0(SegManager *segMan, SegmentId segmentId);

	/**
	 * Initializes the script's objects (SCI1.1+)
	 * @param segMan	A reference to the segment manager
	 * @param segmentId	The script's segment id
	 */
	void initialiseObjectsSci11(SegManager *segMan, SegmentId segmentId);

	// script lock operations

	/** Increments the number of lockers of this script by one. */
	void incrementLockers();

	/** Decrements the number of lockers of this script by one. */
	void decrementLockers();

	/**
	 * Retrieves the number of locks held on this script.
	 * @return the number of locks held on the previously identified script
	 */
	int getLockers() const;

	/** Sets the number of locks held on this script. */
	void setLockers(int lockers);

	/**
	 * Retrieves a pointer to the exports of this script
	 * @return	pointer to the exports.
	 */
	const uint16 *getExportTable() const { return _exportTable; }

	/**
	 * Retrieves the number of exports of script.
	 * @return	the number of exports of this script
	 */
	uint16 getExportsNr() const { return _numExports; }

	/**
	 * Retrieves a pointer to the synonyms associated with this script
	 * @return	pointer to the synonyms, in non-parsed format.
	 */
	const byte *getSynonyms() const { return _synonyms; }

	/**
	 * Retrieves the number of synonyms associated with this script.
	 * @return	the number of synonyms associated with this script
	 */
	uint16 getSynonymsNr() const { return _numSynonyms; }

	/**
	 * Validate whether the specified public function is exported by
	 * the script in the specified segment.
	 * @param pubfunct		Index of the function to validate
	 * @return				NULL if the public function is invalid, its
	 * 						offset into the script's segment otherwise
	 */
	uint16 validateExportFunc(int pubfunct);

	/**
	 * Marks the script as deleted.
	 * This will not actually delete the script.  If references remain present on the
	 * heap or the stack, the script will stay in memory in a quasi-deleted state until
	 * either unreachable (resulting in its eventual deletion) or reloaded (resulting
	 * in its data being updated).
	 */
	void markDeleted() {
		_markedAsDeleted = true;
	}

	/**
	 * Determines whether the script is marked as being deleted.
	 */
	bool isMarkedAsDeleted() const {
		return _markedAsDeleted;
	}

	/**
	 * Copies a byte string into a script's heap representation.
	 * @param dst	script-relative offset of the destination area
	 * @param src	pointer to the data source location
	 * @param n		number of bytes to copy
	 */
	void mcpyInOut(int dst, const void *src, size_t n);

	/**
	 * Finds the pointer where a block of a specific type starts from
	 */
	byte *findBlock(int type);

private:
	/**
	 * Processes a relocation block witin a script
	 *  This function is idempotent, but it must only be called after all
	 *  objects have been instantiated, or a run-time error will occur.
	 * @param obj_pos	Location (segment, offset) of the block
	 * @return			Location of the relocation block
	 */
	void relocate(reg_t block);

	bool relocateLocal(SegmentId segment, int location);
};

} // End of namespace Sci

#endif // SCI_ENGINE_SCRIPT_H
