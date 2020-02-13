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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCI_ENGINE_SCRIPT_H
#define SCI_ENGINE_SCRIPT_H

#include "common/str.h"
#include "sci/util.h"
#include "sci/engine/segment.h"
#include "sci/engine/script_patches.h"

namespace Sci {

struct EngineState;
class ResourceManager;
struct SciScriptPatcherEntry;

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

typedef Common::HashMap<uint32, Object> ObjMap;

enum ScriptOffsetEntryTypes {
	SCI_SCR_OFFSET_TYPE_OBJECT = 0, // classes are handled by this type as well
	SCI_SCR_OFFSET_TYPE_STRING,
	SCI_SCR_OFFSET_TYPE_SAID
};

enum {
	kNoRelocation = 0xFFFFFFFF
};

struct offsetLookupArrayEntry {
	uint16    type;       // type of entry
	uint16    id;         // id of this type, first item inside script data is 1, second item is 2, etc.
	uint32    offset;     // offset of entry within script resource data
	uint16    stringSize; // size of string, including terminating [NUL]
};

typedef Common::Array<offsetLookupArrayEntry> offsetLookupArrayType;

class Script : public SegmentObj {
private:
	int _nr; /**< Script number */
	Common::SpanOwner<SciSpan<byte> > _buf; /**< Static data buffer, or NULL if not used */
	SciSpan<byte> _script; /**< Script size includes alignment byte */
	SciSpan<byte> _heap; /**< Start of heap if SCI1.1, NULL otherwise */

	int _lockers; /**< Number of classes and objects that require this script */

	SciSpan<const uint16> _exports; /**< Exports block or 0 if not present */
	uint16 _numExports; /**< Number of export entries */
	SciSpan<const byte> _synonyms; /**< Synonyms block or 0 if not present */
	uint16 _numSynonyms; /**< Number of synonym entries */

	int _codeOffset; /**< The absolute offset of the VM code block */

	int _localsOffset;
	uint16 _localsCount;

	bool _markedAsDeleted;
	SegmentId _localsSegment; /**< The local variable segment */
	LocalVariables *_localsBlock;

	ObjMap _objects;	/**< Table for objects, contains property variables */

protected:
	offsetLookupArrayType _offsetLookupArray; // Table of all elements of currently loaded script, that may get pointed to

private:
	uint16 _offsetLookupObjectCount;
	uint16 _offsetLookupStringCount;
	uint16 _offsetLookupSaidCount;

public:
	int getLocalsOffset() const { return _localsOffset; }
	uint16 getLocalsCount() const { return _localsCount; }

	uint32 getScriptSize() const { return _script.size(); }
	uint32 getHeapSize() const { return _heap.size(); }
	uint32 getBufSize() const { return _buf->size(); }
	inline uint32 getHeapOffset() const {
		if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
			return _script.size();
		}

		return 0;
	}

	const byte *getBuf(uint offset = 0) const { return _buf->getUnsafeDataAt(offset); }
	SciSpan<const byte> getSpan(uint offset) const { return _buf->subspan(offset); }

	int getScriptNumber() const { return _nr; }
	SegmentId getLocalsSegment() const { return _localsSegment; }
	reg_t *getLocalsBegin() { return _localsBlock ? _localsBlock->_locals.begin() : NULL; }
	void syncLocalsBlock(SegManager *segMan);
	ObjMap &getObjectMap() { return _objects; }
	const ObjMap &getObjectMap() const { return _objects; }
	bool offsetIsObject(uint32 offset) const;

public:
	Script();
	~Script() override;

	void freeScript(const bool keepLocalsSegment = false);
	void load(int script_nr, ResourceManager *resMan, ScriptPatcher *scriptPatcher, bool applyScriptPatches = true);

	bool isValidOffset(uint32 offset) const override;
	SegmentRef dereference(reg_t pointer) override;
	reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) const override;
	void freeAtAddress(SegManager *segMan, reg_t sub_addr) override;
	Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const override;
	Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const override;

	/**
	 * Return a list of all references to objects in this script
	 * (and also to the locals segment, if any).
	 * Used by the garbage collector.
	 * @return a list of outgoing references within the object
	 */
	Common::Array<reg_t> listObjectReferences() const;

	void saveLoadWithSerializer(Common::Serializer &ser) override;

	Object *getObject(uint32 offset);
	const Object *getObject(uint32 offset) const;

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
	 * Initializes the script's local variables
	 * @param segMan	A reference to the segment manager
	 */
	void initializeLocals(SegManager *segMan);

	/**
	 * Adds the script's classes to the segment manager's class table
	 * @param segMan	A reference to the segment manager
	 */
	void initializeClasses(SegManager *segMan);

	/**
	 * Initializes the script's objects (SCI0)
	 * @param segMan	          A reference to the segment manager
	 * @param segmentId	          The script's segment id
	 * @param applyScriptPatches  Apply patches for the script, if available
	 */
	void initializeObjects(SegManager *segMan, SegmentId segmentId, bool applyScriptPatches);

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
	 * Retrieves the offset of the export table in the script
	 * @return	the exports offset.
	 */
	uint getExportsOffset() const { return _exports.sourceByteOffset(); }

	/**
	 * Retrieves the number of exports of script.
	 * @return	the number of exports of this script
	 */
	uint16 getExportsNr() const { return _numExports; }

	/**
	 * Retrieves a pointer to the synonyms associated with this script
	 * @return	pointer to the synonyms, in non-parsed format.
	 */
	const SciSpan<const byte> &getSynonyms() const { return _synonyms; }

	/**
	 * Retrieves the number of synonyms associated with this script.
	 * @return	the number of synonyms associated with this script
	 */
	uint16 getSynonymsNr() const { return _numSynonyms; }

	/**
	 * Validate whether the specified public function is exported by
	 * the script in the specified segment.
	 * @param pubfunct		Index of the function to validate
	 * @param relocSci3     Decide whether to relocate this SCI3 public function or not
	 * @return				NULL if the public function is invalid, its
	 * 						offset into the script's segment otherwise
	 */
	uint32 validateExportFunc(int pubfunct, bool relocSci3);

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
	 * Finds the pointer where a block of a specific type starts from,
	 * in SCI0 - SCI1 games
	 */
	SciSpan<const byte> findBlockSCI0(ScriptObjectTypes type, bool findLastBlock = false) const;

	/**
	 * Syncs the string heap of a script. Used when saving/loading.
	 */
	void syncStringHeap(Common::Serializer &ser);

#ifdef ENABLE_SCI32
	/**
	 * Resolve a relocation in an SCI3 script
	 * @param offset        The offset to relocate from
	 */
	int relocateOffsetSci3(uint32 offset) const;
#endif

	/**
	 * Gets an offset to the beginning of the code block in a SCI1.1 or later
	 * script
	 */
	int getCodeBlockOffset() { return _codeOffset; }

	/**
	 * Get the offset array
	 */
	const offsetLookupArrayType *getOffsetArray() { return &_offsetLookupArray; };
	uint16 getOffsetObjectCount() { return _offsetLookupObjectCount; };
	uint16 getOffsetStringCount() { return _offsetLookupStringCount; };
	uint16 getOffsetSaidCount() { return _offsetLookupSaidCount; };

	/**
	 * @returns kNoRelocation if no relocation exists for the given offset,
	 * otherwise returns a delta for the offset to its relocated position.
	 */
	uint32 getRelocationOffset(const uint32 offset) const;

private:
	/**
	 * Returns a Span containing the relocation table for a SCI0-SCI2.1 script.
	 * (The SCI0-SCI2.1 relocation table is simply a list of all of the
	 * offsets in the script heap whose values should be treated as pointers to
	 * objects (vs just being numbers).)
	 */
	const SciSpan<const uint16> getRelocationTableSci0Sci21() const;

	/**
	 * Processes a relocation block within a SCI0-SCI2.1 script
	 *  This function is idempotent, but it must only be called after all
	 *  objects have been instantiated, or a run-time error will occur.
	 */
	void relocateSci0Sci21(const SegmentId segmentId);

#ifdef ENABLE_SCI32
	/**
	 * Processes a relocation block within a SCI3 script
	 *  This function is idempotent, but it must only be called after all
	 *  objects have been instantiated, or a run-time error will occur.
	 */
	void relocateSci3(const SegmentId segmentId);
#endif

	bool relocateLocal(SegmentId segment, int location, uint32 offset);

#ifdef ENABLE_SCI32
	/**
	 * Gets a pointer to the beginning of the objects in a SCI3 script
	 */
	SciSpan<const byte> getSci3ObjectsPointer();
#endif

	/**
	 * Initializes the script's objects (SCI0)
	 * @param segMan	A reference to the segment manager
	 * @param segmentId	The script's segment id
	 * @applyScriptPatches  Apply patches for the script, if available
	 */
	void initializeObjectsSci0(SegManager *segMan, SegmentId segmentId, bool applyScriptPatches);

	/**
	 * Initializes the script's objects (SCI1.1 - SCI2.1)
	 * @param segMan	    A reference to the segment manager
	 * @param segmentId	    The script's segment id
	 * @applyScriptPatches  Apply patches for the script, if available
	 */
	void initializeObjectsSci11(SegManager *segMan, SegmentId segmentId, bool applyScriptPatches);

#ifdef ENABLE_SCI32
	/**
	 * Initializes the script's objects (SCI3)
	 * @param segMan	A reference to the segment manager
	 * @param segmentId	The script's segment id
	 */
	void initializeObjectsSci3(SegManager *segMan, SegmentId segmentId, bool applyScriptPatches);
#endif

	LocalVariables *allocLocalsSegment(SegManager *segMan);

	/**
	 * Identifies certain offsets within script data and set up lookup-table
	 */
	void identifyOffsets();
};

} // End of namespace Sci

#endif // SCI_ENGINE_SCRIPT_H
