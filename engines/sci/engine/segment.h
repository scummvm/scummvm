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

#ifndef SCI_ENGINE_SEGMENT_H
#define SCI_ENGINE_SEGMENT_H

#include "common/serializer.h"
#include "sci/engine/vm.h"
#include "sci/engine/vm_types.h"	// for reg_t

namespace Sci {

struct SegmentRef {
	bool isRaw;	///< true if data is raw, false if it is a reg_t sequence
	union {
		byte *raw;
		reg_t *reg;
	};
	int maxSize;	///< number of available bytes
	// TODO: Add this?
	//reg_t pointer;	// Original pointer

	// TODO: Add this?
	//SegmentType type;

	SegmentRef() : isRaw(true), raw(0), maxSize(0) {}

	bool isValid() const { return raw != 0; }
};


enum SegmentType {
	SEG_TYPE_INVALID = 0,
	SEG_TYPE_SCRIPT = 1,
	SEG_TYPE_CLONES = 2,
	SEG_TYPE_LOCALS = 3,
	SEG_TYPE_STACK = 4,
	SEG_TYPE_SYS_STRINGS = 5,
	SEG_TYPE_LISTS = 6,
	SEG_TYPE_NODES = 7,
	SEG_TYPE_HUNK = 8,
	SEG_TYPE_DYNMEM = 9,
	SEG_TYPE_STRING_FRAG = 10,	// obsolete, we keep it to be able to load old saves
	
#ifdef ENABLE_SCI32
	SEG_TYPE_ARRAY = 11,
	SEG_TYPE_STRING = 12,
#endif

	SEG_TYPE_MAX // For sanity checking
};

struct SegmentObj : public Common::Serializable {
	SegmentType _type;

	typedef void (*NoteCallback)(void *param, reg_t addr);	// FIXME: Bad choice of name

public:
	static SegmentObj *createSegmentObj(SegmentType type);

public:
	SegmentObj(SegmentType type) : _type(type) {}
	virtual ~SegmentObj() {}

	inline SegmentType getType() const { return _type; }

	/**
	 * Check whether the given offset into this memory object is valid,
	 * i.e., suitable for passing to dereference.
	 */
	virtual bool isValidOffset(uint16 offset) const = 0;

	/**
	 * Dereferences a raw memory pointer.
	 * @param reg	reference to dereference
	 * @return		the data block referenced
	 */
	virtual SegmentRef dereference(reg_t pointer);

	/**
	 * Finds the canonic address associated with sub_reg.
	 *
	 * For each valid address a, there exists a canonic address c(a) such that c(a) = c(c(a)).
	 * This address "governs" a in the sense that deallocating c(a) will deallocate a.
	 *
	 * @param sub_addr		base address whose canonic address is to be found
	 */
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) { return sub_addr; }

	/**
	 * Deallocates all memory associated with the specified address.
	 * @param sub_addr		address (within the given segment) to deallocate
	 */
	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr) {}

	/**
	 * Iterates over and reports all addresses within the current segment.
	 * @param note		Invoked for each address on which free_at_address() makes sense
	 * @param param		parameter passed to 'note'
	 */
	virtual void listAllDeallocatable(SegmentId segId, void *param, NoteCallback note) {}

	/**
	 * Iterates over all references reachable from the specified object.
	 * @param object	object (within the current segment) to analyse
	 * @param param		parameter passed to 'note'
	 * @param note		Invoked for each outgoing reference within the object
	 * Note: This function may also choose to report numbers (segment 0) as adresses
	 */
	virtual void listAllOutgoingReferences(reg_t object, void *param, NoteCallback note) {}
};


struct IntMapper;

enum {
	SYS_STRINGS_MAX = 4,

	SYS_STRING_SAVEDIR = 0,
	SYS_STRING_PARSER_BASE = 1,

	MAX_PARSER_BASE = 64
};

struct SystemString {
	Common::String _name;
	int _maxSize;
	char *_value;
};

struct SystemStrings : public SegmentObj {
	SystemString _strings[SYS_STRINGS_MAX];

public:
	SystemStrings() : SegmentObj(SEG_TYPE_SYS_STRINGS) {
		for (int i = 0; i < SYS_STRINGS_MAX; i++) {
			_strings[i]._maxSize = 0;
			_strings[i]._value = 0;
		}
	}
	~SystemStrings() {
		for (int i = 0; i < SYS_STRINGS_MAX; i++) {
			SystemString *str = &_strings[i];
			if (!str->_name.empty()) {
				free(str->_value);
				str->_value = NULL;

				str->_maxSize = 0;
			}
		}
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual SegmentRef dereference(reg_t pointer);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

struct LocalVariables : public SegmentObj {
	int script_id; /**< Script ID this local variable block belongs to */
	Common::Array<reg_t> _locals;

public:
	LocalVariables(): SegmentObj(SEG_TYPE_LOCALS) {
		script_id = 0;
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr);
	virtual void listAllOutgoingReferences(reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

/** Clone has been marked as 'freed' */
#define OBJECT_FLAG_FREED (1 << 0)

class Object {
public:
	Object() {
		_flags = 0;
		_offset = getSciVersion() < SCI_VERSION_1_1 ? 0 : 5;
	}

	~Object() { }

	reg_t getSpeciesSelector() { return _variables[_offset]; }
	void setSpeciesSelector(reg_t value) { _variables[_offset] = value; }

	reg_t getSuperClassSelector() {	return _variables[_offset + 1];	}
	void setSuperClassSelector(reg_t value) { _variables[_offset + 1] = value; }

	reg_t getInfoSelector() { return _variables[_offset + 2]; }
	void setInfoSelector(reg_t value) {	_variables[_offset + 2] = value; }

	reg_t getNameSelector() { return _variables[_offset + 3]; }
	void setNameSelector(reg_t value) {	_variables[_offset + 3] = value; }

	reg_t getClassScriptSelector() { return _variables[4]; }
	void setClassScriptSelector(reg_t value) { _variables[4] = value; }

	Selector getVarSelector(uint16 i) { return *(_baseVars + i); }

	reg_t getFunction(uint16 i) {
		uint16 offset = (getSciVersion() < SCI_VERSION_1_1) ? _methodCount + 1 + i : i * 2 + 2;
		return make_reg(_pos.segment, READ_LE_UINT16((byte *) (_baseMethod + offset)));
	}

	Selector getFuncSelector(uint16 i) {
		uint16 offset = (getSciVersion() < SCI_VERSION_1_1) ? i : i * 2 + 1;
		return READ_LE_UINT16((byte *) (_baseMethod + offset));
	}

	/**
	 * Determines if this object is a class and explicitly defines the
	 * selector as a funcselector. Does NOT say anything about the object's
	 * superclasses, i.e. failure may be returned even if one of the
	 * superclasses defines the funcselector
	 */
	int funcSelectorPosition(Selector sel) {
		for (uint i = 0; i < _methodCount; i++)
			if (getFuncSelector(i) == sel)
				return i;

		return -1;
	}

	bool isClass() { return (getInfoSelector().offset & SCRIPT_INFO_CLASS);	}

	void markAsFreed() { _flags |= OBJECT_FLAG_FREED; }
	bool isFreed() { return _flags & OBJECT_FLAG_FREED;	}

	void setVarCount(uint size) { _variables.resize(size); }
	uint getVarCount() { return _variables.size(); }

	void init(byte *buf, reg_t obj_pos) {
		byte *data = (byte *)(buf + obj_pos.offset);
		_baseObj = data;
		_pos = obj_pos;

		if (getSciVersion() < SCI_VERSION_1_1) {
			_variables.resize(READ_LE_UINT16(data + SCRIPT_SELECTORCTR_OFFSET));
			_baseVars = (uint16 *)(_baseObj + _variables.size() * 2);
			_baseMethod = (uint16 *)(data + READ_LE_UINT16(data + SCRIPT_FUNCTAREAPTR_OFFSET));
			_methodCount = READ_LE_UINT16(_baseMethod - 1);
		} else {
			_variables.resize(READ_LE_UINT16(data + 2));
			_baseVars = (uint16 *)(buf + READ_LE_UINT16(data + 4));
			_baseMethod = (uint16 *)(buf + READ_LE_UINT16(data + 6));
			_methodCount = READ_LE_UINT16(_baseMethod);
		}

		for (uint i = 0; i < _variables.size(); i++)
			_variables[i] = make_reg(0, READ_LE_UINT16(data + (i * 2)));
	}

	reg_t getVariable(uint var) { return _variables[var]; }

	uint16 getMethodCount() { return _methodCount; }
	reg_t getPos() { return _pos; }

	void saveLoadWithSerializer(Common::Serializer &ser);

	void cloneFromObject(Object *obj) {
		_baseObj = obj ? obj->_baseObj : NULL;
		_baseMethod = obj ? obj->_baseMethod : NULL;
		_baseVars = obj ? obj->_baseVars : NULL;
	}

	// TODO: make private
	Common::Array<reg_t> _variables;
	byte *_baseObj; /**< base + object offset within base */
	uint16 *_baseVars; /**< Pointer to the varselector area for this object */
	uint16 *_baseMethod; /**< Pointer to the method selector area for this object */

private:
	uint16 _methodCount;
	int _flags;
	uint16 _offset;
	reg_t _pos; /**< Object offset within its script; for clones, this is their base */
};

struct CodeBlock {
	reg_t pos;
	int size;
};

typedef Common::HashMap<uint16, Object> ObjMap;

class Script : public SegmentObj {
public:
	int _nr; /**< Script number */
	byte *_buf; /**< Static data buffer, or NULL if not used */
	size_t _bufSize;
	size_t _scriptSize;
	size_t _heapSize;

	byte *_heapStart; /**< Start of heap if SCI1.1, NULL otherwise */

	uint16 *_exportTable; /**< Abs. offset of the export table or 0 if not present */
	int _numExports; /**< Number of entries in the exports table */

	byte *_synonyms; /**< Synonyms block or 0 if not present*/
	int _numSynonyms; /**< Number of entries in the synonyms block */

protected:
	int _lockers; /**< Number of classes and objects that require this script */

public:
	/**
	 * Table for objects, contains property variables.
	 * Indexed by the TODO offset.
	 */
	ObjMap _objects;

	int _localsOffset;
	SegmentId _localsSegment; /**< The local variable segment */
	LocalVariables *_localsBlock;

	Common::Array<CodeBlock> _codeBlocks;
	bool _relocated;
	bool _markedAsDeleted;

public:
	Script();
	~Script();

	void freeScript();
	bool init(int script_nr, ResourceManager *resMan);

	virtual bool isValidOffset(uint16 offset) const;
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr);
	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual void listAllDeallocatable(SegmentId segId, void *param, NoteCallback note);
	virtual void listAllOutgoingReferences(reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

	Object *allocateObject(uint16 offset);
	Object *getObject(uint16 offset);

	/**
	 * Informs the segment manager that a code block must be relocated
	 * @param location	Start of block to relocate
	 */
	void scriptAddCodeBlock(reg_t location);

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
	 * Processes a relocation block witin a script
	 *  This function is idempotent, but it must only be called after all
	 *  objects have been instantiated, or a run-time error will occur.
	 * @param obj_pos	Location (segment, offset) of the block
	 * @return			Location of the relocation block
	 */
	void scriptRelocate(reg_t block);

	void heapRelocate(reg_t block);

private:
	int relocateLocal(SegmentId segment, int location);
	int relocateBlock(Common::Array<reg_t> &block, int block_location, SegmentId segment, int location);
	int relocateObject(Object &obj, SegmentId segment, int location);

public:
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
	 * Retrieves a pointer to the synonyms associated with this script
	 * @return	pointer to the synonyms, in non-parsed format.
	 */
	byte *getSynonyms() const;

	/**
	 * Retrieves the number of synonyms associated with this script.
	 * @return	the number of synonyms associated with this script
	 */
	int getSynonymsNr() const;


	/**
	 * Sets the script-relative offset of the exports table.
	 * @param offset	script-relative exports table offset
	 */
	void setExportTableOffset(int offset);

	/**
	 * Sets the script-relative offset of the synonyms associated with this script.
	 * @param offset	script-relative offset of the synonyms block
	 */
	void setSynonymsOffset(int offset);

	/**
	 * Sets the number of synonyms associated with this script,
	 * @param nr		number of synonyms, as to be stored within the script
	 */
	void setSynonymsNr(int nr);


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
	 * Retrieves a 16 bit value from within a script's heap representation.
	 * @param offset	offset to read from
	 * @return the value read from the specified location
	 */
	int16 getHeap(uint16 offset) const;

private:
	void setScriptSize(int script_nr, ResourceManager *resMan);
};

/** Data stack */
struct DataStack : SegmentObj {
	int _capacity; /**< Number of stack entries */
	reg_t *_entries;

public:
	DataStack() : SegmentObj(SEG_TYPE_STACK) {
		_capacity = 0;
		_entries = NULL;
	}
	~DataStack() {
		free(_entries);
		_entries = NULL;
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr);
	virtual void listAllOutgoingReferences(reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

#define CLONE_USED -1
#define CLONE_NONE -1

typedef Object Clone;

struct Node {
	reg_t pred; /**< Predecessor node */
	reg_t succ; /**< Successor node */
	reg_t key;
	reg_t value;
}; /* List nodes */

struct List {
	reg_t first;
	reg_t last;
};

struct Hunk {
	void *mem;
	unsigned int size;
	const char *type;
};

template<typename T>
struct Table : public SegmentObj {
	typedef T value_type;
	struct Entry : public T {
		int next_free; /* Only used for free entries */
	};
	enum { HEAPENTRY_INVALID = -1 };


	int first_free; /**< Beginning of a singly linked list for entries */
	int entries_used; /**< Statistical information */

	Common::Array<Entry> _table;

public:
	Table(SegmentType type) : SegmentObj(type) {
		initTable();
	}

	void initTable() {
		entries_used = 0;
		first_free = HEAPENTRY_INVALID;
		_table.clear();
	}

	int allocEntry() {
		entries_used++;
		if (first_free != HEAPENTRY_INVALID) {
			int oldff = first_free;
			first_free = _table[oldff].next_free;

			_table[oldff].next_free = oldff;
			return oldff;
		} else {
			uint newIdx = _table.size();
			_table.push_back(Entry());
			_table[newIdx].next_free = newIdx;	// Tag as 'valid'
			return newIdx;
		}
	}

	virtual bool isValidOffset(uint16 offset) const {
		return isValidEntry(offset);
	}

	bool isValidEntry(int idx) const {
		return idx >= 0 && (uint)idx < _table.size() && _table[idx].next_free == idx;
	}

	virtual void freeEntry(int idx) {
		if (idx < 0 || (uint)idx >= _table.size())
			::error("Table::freeEntry: Attempt to release invalid table index %d", idx);

		_table[idx].next_free = first_free;
		first_free = idx;
		entries_used--;
	}

	virtual void listAllDeallocatable(SegmentId segId, void *param, NoteCallback note) {
		for (uint i = 0; i < _table.size(); i++)
			if (isValidEntry(i))
				(*note)(param, make_reg(segId, i));
	}
};


/* CloneTable */
struct CloneTable : public Table<Clone> {
	CloneTable() : Table<Clone>(SEG_TYPE_CLONES) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual void listAllOutgoingReferences(reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* NodeTable */
struct NodeTable : public Table<Node> {
	NodeTable() : Table<Node>(SEG_TYPE_NODES) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual void listAllOutgoingReferences(reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* ListTable */
struct ListTable : public Table<List> {
	ListTable() : Table<List>(SEG_TYPE_LISTS) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual void listAllOutgoingReferences(reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* HunkTable */
struct HunkTable : public Table<Hunk> {
	HunkTable() : Table<Hunk>(SEG_TYPE_HUNK) {}

	virtual void freeEntry(int idx) {
		Table<Hunk>::freeEntry(idx);

		free(_table[idx].mem);
	}

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


// Free-style memory
struct DynMem : public SegmentObj {
	int _size;
	Common::String _description;
	byte *_buf;

public:
	DynMem() : SegmentObj(SEG_TYPE_DYNMEM), _size(0), _buf(0) {}
	~DynMem() {
		free(_buf);
		_buf = NULL;
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr);
	virtual void listAllDeallocatable(SegmentId segId, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

#ifdef ENABLE_SCI32

template <typename T>
class SciArray {
public:
	SciArray() {
		_type = -1;
		_data = NULL;
		_size = 0;
		_actualSize = 0;
	}

	~SciArray() {
		delete[] _data;
	}

	void setType(byte type) {
		if (_type >= 0)
			error("SciArray::setType(): Type already set");

		_type = type;
	}
	
	void setSize(uint32 size) {
		if (_type < 0)
			error("SciArray::setSize(): No type set");

		// Check if we don't have to do anything
		if (_size == size)
			return;

		// Check if we don't have to expand the array
		if (size <= _actualSize) {
			_size = size;
			return;
		}

		// So, we're going to have to create an array of some sort
		T *newArray = new T[size];

		// Check if we never created an array before
		if (!_data) {
			_size = _actualSize = size;
			_data = newArray;
			return;
		}

		// Copy data from the old array to the new
		memcpy(newArray, _data, _size * sizeof(T));

		// Now set the new array to the old and set the sizes
		delete[] _data;
		_data = newArray;
		_size = _actualSize = size;
	}
	
	T getValue(uint16 index) {
		if (index >= _size)
			error("SciArray::getValue(): %d is out of bounds (%d)", index, _size);

		return _data[index];
	}
	
	void setValue(uint16 index, T value) {
		if (index >= _size)
			error("SciArray::setValue(): %d is out of bounds (%d)", index, _size);

		_data[index] = value;
	}
	
	byte getType() { return _type; }
	uint32 getSize() { return _size; }
	T *getRawData() { return _data; }

protected:
	int8 _type;
	T *_data;
	uint32 _size; // _size holds the number of entries that the scripts have requested
	uint32 _actualSize; // _actualSize is the actual numbers of entries allocated
};

class SciString : public SciArray<char> {
public:
	SciString() : SciArray<char>() { setType(3); }
	
	Common::String toString();
	void fromString(Common::String string);
};

struct ArrayTable : public Table<SciArray<reg_t> > {
	ArrayTable() : Table<SciArray<reg_t> >(SEG_TYPE_ARRAY) {}

	virtual void saveLoadWithSerializer(Common::Serializer &ser) {}
};

struct StringTable : public Table<SciString> {
	StringTable() : Table<SciString>(SEG_TYPE_STRING) {}

	virtual void saveLoadWithSerializer(Common::Serializer &ser) {}
	SegmentRef dereference(reg_t pointer);
};

#endif


} // End of namespace Sci

#endif // SCI_ENGINE_SEGMENT_H
