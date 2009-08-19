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

#ifndef SCI_ENGINE_MEMOBJ_H
#define SCI_ENGINE_MEMOBJ_H

#include "common/serializer.h"
#include "sci/engine/vm.h"
#include "sci/engine/vm_types.h"	// for reg_t

//#include "common/util.h"

namespace Sci {

enum MemObjectType {
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
	MEM_OBJ_STRING_FRAG = 10,

	MEM_OBJ_MAX // For sanity checking
};

struct MemObject : public Common::Serializable {
	MemObjectType _type;
	int _segmgrId; /**< Internal value used by the seg_manager's hash map */

	typedef void (*NoteCallback)(void *param, reg_t addr);	// FIXME: Bad choice of name

public:
	static MemObject *createMemObject(MemObjectType type);

public:
	virtual ~MemObject() {}

	inline MemObjectType getType() const { return _type; }
	inline int getSegMgrId() const { return _segmgrId; }

	/**
	 * Check whether the given offset into this memory object is valid,
	 * i.e., suitable for passing to dereference.
	 */
	virtual bool isValidOffset(uint16 offset) const = 0;

	/**
	 * Dereferences a raw memory pointer.
	 * @param reg	reference to dereference
	 * @param size	if not NULL, set to the theoretical maximum size of the referenced data block
	 * @return		the data block referenced
	 */
	virtual byte *dereference(reg_t pointer, int *size);

	/**
	 * Finds the canonic address associated with sub_reg.
	 *
	 * For each valid address a, there exists a canonic address c(a) such that c(a) = c(c(a)).
	 * This address "governs" a in the sense that deallocating c(a) will deallocate a.
	 *
	 * @param sub_addr		base address whose canonic address is to be found
	 */
	virtual reg_t findCanonicAddress(SegManager *segmgr, reg_t sub_addr) { return sub_addr; }

	/**
	 * Deallocates all memory associated with the specified address.
	 * @param sub_addr		address (within the given segment) to deallocate
	 */
	virtual void freeAtAddress(SegManager *segmgr, reg_t sub_addr) {}

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
	virtual void listAllOutgoingReferences(EngineState *s, reg_t object, void *param, NoteCallback note) {}
};


// TODO: Implement the following class
struct StringFrag : public MemObject {
	virtual bool isValidOffset(uint16 offset) const { return false; }

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

struct IntMapper;

enum {
	SYS_STRINGS_MAX = 4,

	SYS_STRING_SAVEDIR = 0,
	SYS_STRING_PARSER_BASE = 1,

	MAX_PARSER_BASE = 64
};

struct SystemString {
	char *name;
	int max_size;
	reg_t *value;
};

struct SystemStrings : public MemObject {
	SystemString strings[SYS_STRINGS_MAX];

public:
	SystemStrings() {
		memset(strings, 0, sizeof(strings));
	}
	~SystemStrings() {
		for (int i = 0; i < SYS_STRINGS_MAX; i++) {
			SystemString *str = &strings[i];
			if (str->name) {
				free(str->name);
				str->name = NULL;

				free(str->value);
				str->value = NULL;

				str->max_size = 0;
			}
		}
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual byte *dereference(reg_t pointer, int *size);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

/** This struct is used to buffer the list of send calls in send_selector() */
struct CallsStruct {
	reg_t addr_func;
	reg_t varp_objp;
	union {
		reg_t func;
		ObjVarRef var;
	} address;
	StackPtr argp;
	int argc;
	Selector selector;
	StackPtr sp; /**< Stack pointer */
	int type; /**< Same as ExecStack.type */
};

struct LocalVariables : public MemObject {
	int script_id; /**< Script ID this local variable block belongs to */
	Common::Array<reg_t> _locals;

public:
	LocalVariables() {
		script_id = 0;
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual byte *dereference(reg_t pointer, int *size);
	virtual reg_t findCanonicAddress(SegManager *segmgr, reg_t sub_addr);
	virtual void listAllOutgoingReferences(EngineState *s, reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

/** Clone has been marked as 'freed' */
#define OBJECT_FLAG_FREED (0x1 << 0)

struct Object {
	int flags;
	reg_t pos; /**< Object offset within its script; for clones, this is their base */
	int variable_names_nr; /**< Number of variable names, may be less than variables_nr */
	int methods_nr;
	byte *base; /**< Points to a buffer all relative references (code, strings) point to */
	byte *base_obj; /**< base + object offset within base */
	uint16 *base_method; /**< Pointer to the method selector area for this object */
	uint16 *base_vars; /**< Pointer to the varselector area for this object */
	Common::Array<reg_t> _variables;
};

struct CodeBlock {
	reg_t pos;
	int size;
};

#define VM_OBJECT_GET_VARSELECTOR(obj, i)  \
	(s->_version < SCI_VERSION_1_1 ? \
	 READ_LE_UINT16(obj->base_obj + obj->_variables.size() * 2 + i*2) : \
	 *(obj->base_vars + i))
#define VM_OBJECT_READ_PROPERTY(obj, i) (obj->_variables[i])
#define VM_OBJECT_GET_FUNCSELECTOR(obj, i) \
	(s->_version < SCI_VERSION_1_1 ? \
	 READ_LE_UINT16((byte *) (obj->base_method + i)) : \
	 READ_LE_UINT16((byte *) (obj->base_method + i*2 + 1)))
#define VM_OBJECT_READ_FUNCTION(obj, i) \
	(s->_version < SCI_VERSION_1_1 ? \
	 make_reg(obj->pos.segment, \
		 READ_LE_UINT16((byte *) (obj->base_method \
				 + obj->methods_nr + 1 \
				 + i))) : \
	 make_reg(obj->pos.segment, \
		 READ_LE_UINT16((byte *) (obj->base_method \
				 + i * 2 + 2))))




struct Script : public MemObject {
	int nr; /**< Script number */
	byte *buf; /**< Static data buffer, or NULL if not used */
	size_t buf_size;
	size_t script_size;
	size_t heap_size;

	byte *synonyms; /**< Synonyms block or 0 if not present*/
	byte *heap_start; /**< Start of heap if SCI1.1, NULL otherwise */
	uint16 *export_table; /**< Abs. offset of the export table or 0 if not present */

	IntMapper *obj_indices;

	int exports_nr; /**< Number of entries in the exports table */
	int synonyms_nr; /**< Number of entries in the synonyms block */
	int lockers; /**< Number of classes and objects that require this script */

	/**
	 * Table for objects, contains property variables.
	 * Indexed by the value stored at SCRIPT_LOCALVARPTR_OFFSET,
	 * see VM_OBJECT_[GS]ET_INDEX()
	 */
	Common::Array<Object> _objects;

	int locals_offset;
	int locals_segment; /**< The local variable segment */
	LocalVariables *locals_block;

	Common::Array<CodeBlock> _codeBlocks;
	int relocated;
	bool _markedAsDeleted;

public:
	Script() {
		nr = 0;
		buf = NULL;
		buf_size = 0;
		script_size = 0;
		heap_size = 0;

		synonyms = NULL;
		heap_start = NULL;
		export_table = NULL;

		obj_indices = NULL;

		locals_offset = 0;
		locals_segment = 0;
		locals_block = NULL;

		relocated = 0;
		_markedAsDeleted = 0;
	}

	~Script() {
		freeScript();
	}

	void freeScript();

	virtual bool isValidOffset(uint16 offset) const;
	virtual byte *dereference(reg_t pointer, int *size);
	virtual reg_t findCanonicAddress(SegManager *segmgr, reg_t sub_addr);
	virtual void freeAtAddress(SegManager *segmgr, reg_t sub_addr);
	virtual void listAllDeallocatable(SegmentId segId, void *param, NoteCallback note);
	virtual void listAllOutgoingReferences(EngineState *s, reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

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
	 * Marks the script as not deleted.
	 */
	void unmarkDeleted() {
		_markedAsDeleted = false;
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
};

/** Data stack */
struct DataStack : MemObject {
	int nr; /**< Number of stack entries */
	reg_t *entries;

public:
	DataStack() {
		nr = 0;
		entries = NULL;
	}
	~DataStack() {
		free(entries);
		entries = NULL;
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual byte *dereference(reg_t pointer, int *size);
	virtual reg_t findCanonicAddress(SegManager *segmgr, reg_t sub_addr);
	virtual void listAllOutgoingReferences(EngineState *s, reg_t object, void *param, NoteCallback note);

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
struct Table : public MemObject {
	typedef T value_type;
	struct Entry : public T {
		int next_free; /* Only used for free entries */
	};
	enum { HEAPENTRY_INVALID = -1 };


	int first_free; /**< Beginning of a singly linked list for entries */
	int entries_used; /**< Statistical information */

	Common::Array<Entry> _table;

public:
	Table() {
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
	virtual void freeAtAddress(SegManager *segmgr, reg_t sub_addr);
	virtual void listAllOutgoingReferences(EngineState *s, reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* NodeTable */
struct NodeTable : public Table<Node> {
	virtual void freeAtAddress(SegManager *segmgr, reg_t sub_addr);
	virtual void listAllOutgoingReferences(EngineState *s, reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* ListTable */
struct ListTable : public Table<List> {
	virtual void freeAtAddress(SegManager *segmgr, reg_t sub_addr);
	virtual void listAllOutgoingReferences(EngineState *s, reg_t object, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* HunkTable */
struct HunkTable : public Table<Hunk> {
	virtual void freeEntry(int idx) {
		Table<Hunk>::freeEntry(idx);

		free(_table[idx].mem);
	}

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


// Free-style memory
struct DynMem : public MemObject {
	int _size;
	char *_description;
	byte *_buf;

public:
	DynMem() : _size(0), _description(0), _buf(0) {}
	~DynMem() {
		free(_description);
		_description = NULL;
		free(_buf);
		_buf = NULL;
	}

	virtual bool isValidOffset(uint16 offset) const;
	virtual byte *dereference(reg_t pointer, int *size);
	virtual reg_t findCanonicAddress(SegManager *segmgr, reg_t sub_addr);
	virtual void listAllDeallocatable(SegmentId segId, void *param, NoteCallback note);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


} // End of namespace Sci

#endif // SCI_ENGINE_VM_H
