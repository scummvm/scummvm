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

#ifndef SCI_ENGINE_SEGMENT_H
#define SCI_ENGINE_SEGMENT_H

#include "common/serializer.h"
#include "common/str.h"
#include "sci/engine/object.h"
#include "sci/engine/vm.h"
#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/util.h"
#ifdef ENABLE_SCI32
#include "sci/graphics/palette32.h"
#endif

namespace Sci {

struct SegmentRef {
	bool isRaw;	///< true if data is raw, false if it is a reg_t sequence
	union {
		byte *raw;
		reg_t *reg;
	};
	int maxSize;	///< number of available bytes

	// FIXME: Perhaps a generic 'offset' is more appropriate here
	bool skipByte; ///< true if referencing the 2nd data byte of *reg, false otherwise

	// TODO: Add this?
	//reg_t pointer;	// Original pointer

	// TODO: Add this?
	//SegmentType type;

	SegmentRef() : isRaw(true), raw(0), maxSize(0), skipByte(false) {}

	bool isValid() const { return (isRaw ? raw != 0 : reg != 0); }
};


enum SegmentType {
	SEG_TYPE_INVALID = 0,
	SEG_TYPE_SCRIPT = 1,
	SEG_TYPE_CLONES = 2,
	SEG_TYPE_LOCALS = 3,
	SEG_TYPE_STACK = 4,
	// 5 used to be system strings,	now obsolete
	SEG_TYPE_LISTS = 6,
	SEG_TYPE_NODES = 7,
	SEG_TYPE_HUNK = 8,
	SEG_TYPE_DYNMEM = 9,
	// 10 used to be string fragments, now obsolete

#ifdef ENABLE_SCI32
	SEG_TYPE_ARRAY = 11,
	// 12 used to be string, now obsolete
	SEG_TYPE_BITMAP = 13,
#endif

	SEG_TYPE_MAX // For sanity checking
};

struct SegmentObj : public Common::Serializable {
	SegmentType _type;

public:
	static SegmentObj *createSegmentObj(SegmentType type);

public:
	SegmentObj(SegmentType type) : _type(type) {}
	~SegmentObj() override {}

	inline SegmentType getType() const { return _type; }

	/**
	 * Check whether the given offset into this memory object is valid,
	 * i.e., suitable for passing to dereference.
	 */
	virtual bool isValidOffset(uint32 offset) const = 0;

	/**
	 * Dereferences a raw memory pointer.
	 * @param reg	reference to dereference
	 * @return		the data block referenced
	 */
	virtual SegmentRef dereference(reg_t pointer);

	/**
	 * Finds the canonic address associated with sub_reg.
	 * Used by the garbage collector.
	 *
	 * For each valid address a, there exists a canonic address c(a) such that c(a) = c(c(a)).
	 * This address "governs" a in the sense that deallocating c(a) will deallocate a.
	 *
	 * @param sub_addr		base address whose canonic address is to be found
	 */
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) const { return sub_addr; }

	/**
	 * Deallocates all memory associated with the specified address.
	 * Used by the garbage collector.
	 * @param sub_addr		address (within the given segment) to deallocate
	 */
	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr) {}

	/**
	 * Iterates over and reports all addresses within the segment.
	 * Used by the garbage collector.
	 * @return a list of addresses within the segment
	 */
	virtual Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const {
		return Common::Array<reg_t>();
	}

	/**
	 * Iterates over all references reachable from the specified object.
	 * Used by the garbage collector.
	 * @param  object	object (within the current segment) to analyze
	 * @return a list of outgoing references within the object
	 *
	 * @note This function may also choose to report numbers (segment 0) as adresses
	 */
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const {
		return Common::Array<reg_t>();
	}
};

struct LocalVariables : public SegmentObj {
	int script_id; /**< Script ID this local variable block belongs to */
	Common::Array<reg_t> _locals;

public:
	LocalVariables(): SegmentObj(SEG_TYPE_LOCALS), script_id(0) { }

	bool isValidOffset(uint32 offset) const override {
		return offset < _locals.size() * 2;
	}
	SegmentRef dereference(reg_t pointer) override;
	reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) const override;
	Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const override;

	void saveLoadWithSerializer(Common::Serializer &ser) override;
};

/** Data stack */
struct DataStack : SegmentObj {
	uint _capacity; /**< Number of stack entries */
	reg_t *_entries;

public:
	DataStack() : SegmentObj(SEG_TYPE_STACK), _capacity(0), _entries(NULL) { }
	~DataStack() override {
		free(_entries);
		_entries = NULL;
	}

	bool isValidOffset(uint32 offset) const override {
		return offset < _capacity * 2;
	}
	SegmentRef dereference(reg_t pointer) override;
	reg_t findCanonicAddress(SegManager *segMan, reg_t addr) const override {
		return make_reg(addr.getSegment(), 0);
	}
	Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const override;

	void saveLoadWithSerializer(Common::Serializer &ser) override;
};

enum {
	CLONE_USED = -1,
	CLONE_NONE = -1
};

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

#ifdef ENABLE_SCI32
	/**
	 * The next node for each level of recursion during iteration over this list
	 * by kListEachElementDo.
	 */
	reg_t nextNodes[10];

	/**
	 * The current level of recursion of kListEachElementDo for this list.
	 */
	int numRecursions;

	List() : numRecursions(0) {}
#endif
};

struct Hunk {
	void *mem;
	uint32 size;
	const char *type;
};

template<typename T>
struct SegmentObjTable : public SegmentObj {
	typedef T value_type;
	struct Entry {
		T *data;
		int next_free; /* Only used for free entries */
	};
	enum { HEAPENTRY_INVALID = -1 };

	int first_free; /**< Beginning of a singly linked list for entries */
	int entries_used; /**< Statistical information */

	typedef Common::Array<Entry> ArrayType;
	ArrayType _table;

public:
	SegmentObjTable(SegmentType type) : SegmentObj(type) {
		initTable();
	}

	~SegmentObjTable() override {
		for (uint i = 0; i < _table.size(); i++) {
			if (isValidEntry(i)) {
				freeEntry(i);
			}
		}
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
			assert(_table[oldff].data == nullptr);
			_table[oldff].data = new T;
			return oldff;
		} else {
			uint newIdx = _table.size();
			_table.push_back(Entry());
			_table.back().data = new T;
			_table[newIdx].next_free = newIdx;	// Tag as 'valid'
			return newIdx;
		}
	}

	bool isValidOffset(uint32 offset) const override {
		return isValidEntry(offset);
	}

	bool isValidEntry(int idx) const {
		return idx >= 0 && (uint)idx < _table.size() && _table[idx].next_free == idx;
	}

	virtual void freeEntry(int idx) {
		if (idx < 0 || (uint)idx >= _table.size())
			::error("Table::freeEntry: Attempt to release invalid table index %d", idx);

		_table[idx].next_free = first_free;
		delete _table[idx].data;
		_table[idx].data = nullptr;
		first_free = idx;
		entries_used--;
	}

	Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const override {
		Common::Array<reg_t> tmp;
		for (uint i = 0; i < _table.size(); i++)
			if (isValidEntry(i))
				tmp.push_back(make_reg(segId, i));
		return tmp;
	}

	uint size() const { return _table.size(); }

	T &at(uint index) { return *_table[index].data; }
	const T &at(uint index) const { return *_table[index].data; }

	T &operator[](uint index) { return at(index); }
	const T &operator[](uint index) const { return at(index); }
};


/* CloneTable */
struct CloneTable : public SegmentObjTable<Clone> {
	CloneTable() : SegmentObjTable<Clone>(SEG_TYPE_CLONES) {}

	void freeAtAddress(SegManager *segMan, reg_t sub_addr) override;
	Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const override;

	void saveLoadWithSerializer(Common::Serializer &ser) override;
};


/* NodeTable */
struct NodeTable : public SegmentObjTable<Node> {
	NodeTable() : SegmentObjTable<Node>(SEG_TYPE_NODES) {}

	void freeAtAddress(SegManager *segMan, reg_t sub_addr) override {
		freeEntry(sub_addr.getOffset());
	}
	Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const override;

	void saveLoadWithSerializer(Common::Serializer &ser) override;
};


/* ListTable */
struct ListTable : public SegmentObjTable<List> {
	ListTable() : SegmentObjTable<List>(SEG_TYPE_LISTS) {}

	void freeAtAddress(SegManager *segMan, reg_t sub_addr) override {
		freeEntry(sub_addr.getOffset());
	}
	Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const override;

	void saveLoadWithSerializer(Common::Serializer &ser) override;
};


/* HunkTable */
struct HunkTable : public SegmentObjTable<Hunk> {
	HunkTable() : SegmentObjTable<Hunk>(SEG_TYPE_HUNK) {}
	~HunkTable() override {
		for (uint i = 0; i < _table.size(); i++) {
			if (isValidEntry(i))
				freeEntryContents(i);
		}
	}

	void freeEntryContents(int idx) {
		free(at(idx).mem);
		at(idx).mem = 0;
	}

	void freeEntry(int idx) override {
		freeEntryContents(idx);
		SegmentObjTable<Hunk>::freeEntry(idx);
	}

	void freeAtAddress(SegManager *segMan, reg_t sub_addr) override {
		freeEntry(sub_addr.getOffset());
	}

	void saveLoadWithSerializer(Common::Serializer &ser) override;
};


// Free-style memory
struct DynMem : public SegmentObj {
	uint _size;
	Common::String _description;
	byte *_buf;

public:
	DynMem() : SegmentObj(SEG_TYPE_DYNMEM), _size(0), _buf(0) {}
	~DynMem() override {
		free(_buf);
		_buf = NULL;
	}

	bool isValidOffset(uint32 offset) const override {
		return offset < _size;
	}
	SegmentRef dereference(reg_t pointer) override;
	reg_t findCanonicAddress(SegManager *segMan, reg_t addr) const override {
		return make_reg(addr.getSegment(), 0);
	}
	Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const override {
		const reg_t r = make_reg(segId, 0);
		return Common::Array<reg_t>(&r, 1);
	}

	void saveLoadWithSerializer(Common::Serializer &ser) override;
};

#ifdef ENABLE_SCI32

#pragma mark -
#pragma mark Arrays

enum SciArrayType {
	kArrayTypeInt16   = 0,
	kArrayTypeID      = 1,
	kArrayTypeByte    = 2,
	kArrayTypeString  = 3,
	// Type 4 was for 32-bit integers; never used
	kArrayTypeInvalid = 5
};

enum SciArrayTrim {
	kArrayTrimRight  = 1, ///< Trim whitespace after the last non-whitespace character
	kArrayTrimCenter = 2, ///< Trim whitespace between non-whitespace characters
	kArrayTrimLeft   = 4  ///< Trim whitespace before the first non-whitespace character
};

class SciArray : public Common::Serializable {
public:
	SciArray() :
		_type(kArrayTypeInvalid),
		_size(0),
		_data(nullptr) {}

	SciArray(const SciArray &array) {
		_type = array._type;
		_size = array._size;
		_elementSize = array._elementSize;
		_data = malloc(_elementSize * _size);
		assert(_data);
		memcpy(_data, array._data, _elementSize * _size);
	}

	SciArray &operator=(const SciArray &array) {
		if (this == &array)
			return *this;

		free(_data);
		_type = array._type;
		_size = array._size;
		_elementSize = array._elementSize;
		_data = malloc(_elementSize * _size);
		assert(_data);
		memcpy(_data, array._data, _elementSize * _size);

		return *this;
	}

	~SciArray() override {
		free(_data);
		_size = 0;
		_type = kArrayTypeInvalid;
	}

	void saveLoadWithSerializer(Common::Serializer &s) override;

	/**
	 * Returns the type of this array.
	 */
	SciArrayType getType() const {
		return _type;
	}

	/**
	 * Sets the type of this array. The type of the array may only be set once.
	 */
	void setType(const SciArrayType type) {
		assert(_type == kArrayTypeInvalid);
		switch(type) {
		case kArrayTypeInt16:
		case kArrayTypeID:
			_elementSize = sizeof(reg_t);
			break;
		case kArrayTypeString:
			_elementSize = sizeof(char);
			break;
		case kArrayTypeByte:
			_elementSize = sizeof(byte);
			break;
		default:
			error("Invalid array type %d", type);
		}
		_type = type;
	}

	/**
	 * Returns the size of the array, in elements.
	 */
	uint16 size() const {
		return _size;
	}

	/**
	 * Returns the maximum number of bytes that can be stored in the array.
	 */
	uint16 byteSize() const {
		uint16 size1 = _size;
		if (_type == kArrayTypeID || _type == kArrayTypeInt16) {
			size1 *= sizeof(uint16);
		}
		return size1;
	}

	/**
	 * Ensures the array is large enough to store at least the given number of
	 * values given in `newSize`. If `force` is true, the array will be resized
	 * to store exactly `newSize` values. New values are initialized to zero.
	 */
	void resize(uint16 newSize, const bool force = false) {
		if (force || newSize > _size) {
			_data = realloc(_data, _elementSize * newSize);
			if (newSize > _size) {
				memset((byte *)_data + _elementSize * _size, 0, (newSize - _size) * _elementSize);
			}
			_size = newSize;
		}
	}

	/**
	 * Returns a pointer to the array's raw data storage.
	 */
	void *getRawData() { return _data; }
	const void *getRawData() const { return _data; }

	/**
	 * Gets the value at the given index as a reg_t.
	 */
	reg_t getAsID(const uint16 index) {
		if (getSciVersion() >= SCI_VERSION_3) {
			// SCI3 resizes arrays automatically when out-of-bounds indices are
			// passed, but it has an off-by-one error, always passing the index
			// instead of `index + 1` on a read. This happens to work in SSCI
			// only because the resize method there actually allocates memory
			// for `index + 25` elements when growing the array, and it always
			// grows the array on its first resize because it decides whether to
			// grow based on byte size including an extra array header.
			resize(index + 1);
		} else {
			assert(index < _size);
		}

		switch(_type) {
		case kArrayTypeInt16:
		case kArrayTypeID:
			return ((reg_t *)_data)[index];
		case kArrayTypeByte:
		case kArrayTypeString: {
			int16 value;

			if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
				value = ((int8 *)_data)[index];
			} else {
				value = ((uint8 *)_data)[index];
			}

			return make_reg(0, value);
		}
		default:
			error("Invalid array type %d", _type);
		}
	}

	/**
	 * Sets the value at the given index from a reg_t.
	 */
	void setFromID(const uint16 index, const reg_t value) {
		if (getSciVersion() >= SCI_VERSION_3) {
			// This code is different from SSCI; see getAsID for an explanation
			resize(index + 1);
		} else {
			assert(index < _size);
		}

		switch(_type) {
		case kArrayTypeInt16:
		case kArrayTypeID:
			((reg_t *)_data)[index] = value;
			break;
		case kArrayTypeByte:
		case kArrayTypeString:
			((byte *)_data)[index] = value.toSint16();
			break;
		default:
			error("Invalid array type %d", _type);
		}
	}

	/**
	 * Gets the value at the given index as an int16.
	 */
	int16 getAsInt16(const uint16 index) {
		assert(_type == kArrayTypeInt16);

		if (getSciVersion() >= SCI_VERSION_3) {
			// This code is different from SSCI; see getAsID for an explanation
			resize(index + 1);
		} else {
			assert(index < _size);
		}

		const reg_t value = ((reg_t *)_data)[index];
		assert(value.isNumber());
		return value.toSint16();
	}

	/**
	 * Sets the value at the given index from an int16.
	 */
	void setFromInt16(const uint16 index, const int16 value) {
		assert(_type == kArrayTypeInt16);

		if (getSciVersion() >= SCI_VERSION_3) {
			// This code is different from SSCI; see getAsID for an explanation
			resize(index + 1);
		} else {
			assert(index < _size);
		}

		((reg_t *)_data)[index] = make_reg(0, value);
	}

	/**
	 * Returns a reference to the byte at the given index. Only valid for
	 * string and byte arrays.
	 */
	byte &byteAt(const uint16 index) {
		assert(_type == kArrayTypeString || _type == kArrayTypeByte);

		if (getSciVersion() >= SCI_VERSION_3) {
			// This code is different from SSCI; see getAsID for an explanation
			resize(index + 1);
		} else {
			assert(index < _size);
		}

		return ((byte *)_data)[index];
	}

	/**
	 * Returns a reference to the char at the given index. Only valid for
	 * string and byte arrays.
	 */
	char &charAt(const uint16 index) {
		assert(_type == kArrayTypeString || _type == kArrayTypeByte);

		if (getSciVersion() >= SCI_VERSION_3) {
			// This code is different from SSCI; see getAsID for an explanation
			resize(index + 1);
		} else {
			assert(index < _size);
		}

		return ((char *)_data)[index];
	}

	/**
	 * Returns a reference to the reg_t at the given index. Only valid for ID
	 * and int16 arrays.
	 */
	reg_t &IDAt(const uint16 index) {
		assert(_type == kArrayTypeID || _type == kArrayTypeInt16);

		if (getSciVersion() >= SCI_VERSION_3) {
			// This code is different from SSCI; see getAsID for an explanation
			resize(index + 1);
		} else {
			assert(index < _size);
		}

		return ((reg_t *)_data)[index];
	}

	/**
	 * Reads values from the given reg_t pointer and sets them in the array,
	 * growing the array if needed to store all values.
	 */
	void setElements(const uint16 index, uint16 count, const reg_t *values) {
		resize(index + count);

		switch (_type) {
		case kArrayTypeInt16:
		case kArrayTypeID: {
			const reg_t *source = values;
			reg_t *target = (reg_t *)_data + index;
			while (count--) {
				*target++ = *source++;
			}
			break;
		}
		case kArrayTypeByte:
		case kArrayTypeString: {
			const reg_t *source = values;
			byte *target = (byte *)_data + index;
			while (count--) {
				if (!source->isNumber()) {
					error("Non-number %04x:%04x sent to byte or string array", PRINT_REG(*source));
				}
				*target++ = source->getOffset();
				++source;
			}
			break;
		}
		default:
			error("Attempted write to SciArray with invalid type %d", _type);
		}
	}

	/**
	 * Fills the array with the given value. Existing values will be
	 * overwritten. The array will be grown if needed to store all values.
	 */
	void fill(const uint16 index, uint16 count, const reg_t value) {
		if (count == 65535 /* -1 */) {
			count = size() - index;
		}

		if (!count) {
			return;
		}

		resize(index + count);

		switch (_type) {
		case kArrayTypeInt16:
		case kArrayTypeID: {
			reg_t *target = (reg_t *)_data + index;
			while (count--) {
				*target++ = value;
			}
			break;
		}
		case kArrayTypeByte:
		case kArrayTypeString: {
			byte *target = (byte *)_data + index;
			const byte fillValue = value.getOffset();
			while (count--) {
				*target++ = fillValue;
			}
			break;
		}

		case kArrayTypeInvalid:
		default:
			error("Attempted write to uninitialized SciArray");
			break;
		}
	}

	/**
	 * Copies values from the source array. Both arrays will be grown if needed
	 * to prevent out-of-bounds reads/writes.
	 */
	void copy(SciArray &source, const uint16 sourceIndex, const uint16 targetIndex, int16 count) {
		if (count == -1) {
			count = source.size() - sourceIndex;
		}

		if (count < 1) {
			return;
		}

		resize(targetIndex + count);
		source.resize(sourceIndex + count);

		assert(source._elementSize == _elementSize);

		const byte *sourceData = (byte *)source._data + sourceIndex * source._elementSize;
		byte *targetData = (byte *)_data + targetIndex * _elementSize;
		memmove(targetData, sourceData, count * _elementSize);
	}

	void byteCopy(const SciArray &source, const uint16 sourceOffset, const uint16 targetOffset, const uint16 count) {
		error("SciArray::byteCopy not implemented");
	}

	/**
	 * Removes whitespace from string data held in this array.
	 */
	void trim(const int8 flags, const char showChar) {
		enum {
			kWhitespaceBoundary = 32,
			kAsciiBoundary = 128
		};

		byte *data = (byte *)_data;
		byte *end = data + _size;
		byte *source;
		byte *target;

		if (flags & kArrayTrimLeft) {
			target = data;
			source = data;
			while (source < end && *source != '\0' && *source != showChar && *source <= kWhitespaceBoundary) {
				++source;
			}
			memmove(target, source, Common::strnlen((char *)source, _size - 1) + 1);
		}

		if (flags & kArrayTrimRight) {
			source = data + Common::strnlen((char *)data, _size) - 1;
			while (source > data && *source != showChar && *source <= kWhitespaceBoundary) {
				*source = '\0';
				--source;
			}
		}

		if (flags & kArrayTrimCenter) {
			target = data;
			while (target < end && *target != '\0' && *target <= kWhitespaceBoundary && *target != showChar) {
				++target;
			}

			if (*target != '\0') {
				while (target < end && *target != '\0' && (*target > kWhitespaceBoundary || *target == showChar)) {
					++target;
				}

				if (*target != '\0') {
					source = target;
					while (*source != '\0') {
						while (source < end && *source != '\0' && *source <= kWhitespaceBoundary && *source != showChar) {
							++source;
						}

						while (source < end && *source != '\0' && (*source > kWhitespaceBoundary || *source == showChar)) {
							*target++ = *source++;
						}
					}

					--source;
					while (source >= data && source > target && (*source <= kWhitespaceBoundary || *source >= kAsciiBoundary) && *source != showChar) {
						--source;
					}
					++source;

					memmove(target, source, Common::strnlen((char *)source, _size - 1) + 1);
				}
			}
		}
	}

	/**
	 * Copies the string data held by this array into a new Common::String.
	 */
	Common::String toString() const {
		assert(_type == kArrayTypeString);
		return Common::String((char *)_data);
	}

	/**
	 * Copies the string from the given Common::String into this array.
	 */
	void fromString(const Common::String &string) {
		// At least LSL6hires uses a byte-type array to hold string data
		assert(_type == kArrayTypeString || _type == kArrayTypeByte);
		resize(string.size() + 1, true);
		Common::strlcpy((char *)_data, string.c_str(), string.size() + 1);
	}

	Common::String toDebugString() const {
		const char *type;
		switch(_type) {
		case kArrayTypeID:
			type = "reg_t";
			break;
		case kArrayTypeByte:
			type = "byte";
			break;
		case kArrayTypeInt16:
			type = "int16";
			break;
		case kArrayTypeString:
			type = "string";
			break;
		case kArrayTypeInvalid:
		default:
			type = "invalid";
			break;
		}

		return Common::String::format("type %s; %u entries", type, size());
	}

protected:
	void *_data;
	SciArrayType _type;
	uint16 _size;
	uint8 _elementSize;
};

struct ArrayTable : public SegmentObjTable<SciArray> {
	ArrayTable() : SegmentObjTable<SciArray>(SEG_TYPE_ARRAY) {}

	Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const override;

	void saveLoadWithSerializer(Common::Serializer &ser) override;
	SegmentRef dereference(reg_t pointer) override;
};

#pragma mark -
#pragma mark Bitmaps

enum {
	kDefaultSkipColor = 250
};

#define BITMAP_PROPERTY(size, property, offset)\
inline uint##size get##property() const {\
	return READ_SCI11ENDIAN_UINT##size(_data + (offset));\
}\
inline void set##property(uint##size value) {\
	WRITE_SCI11ENDIAN_UINT##size(_data + (offset), (value));\
}

struct BitmapTable;

/**
 * A convenience class for creating and modifying in-memory
 * bitmaps.
 */
class SciBitmap : public Common::Serializable {
	byte *_data;
	int _dataSize;
	Buffer _buffer;
	bool _gc;

public:
	enum BitmapFlags {
		kBitmapRemap = 2
	};

	/**
	 * Gets the size of the bitmap header for the current
	 * engine version.
	 */
	static inline uint16 getBitmapHeaderSize() {
		// TODO: These values are accurate for each engine, but there may be no reason
		// to not simply just always use size 40, since SCI2.1mid does not seem to
		// actually store any data above byte 40, and SCI2 did not allow bitmaps with
		// scaling resolutions other than the default (320x200). Perhaps SCI3 used
		// the extra bytes, or there is some reason why they tried to align the header
		// size with other headers like pic headers?
//		uint32 bitmapHeaderSize;
//		if (getSciVersion() >= SCI_VERSION_2_1_MIDDLE) {
//			bitmapHeaderSize = 46;
//		} else if (getSciVersion() == SCI_VERSION_2_1_EARLY) {
//			bitmapHeaderSize = 40;
//		} else {
//			bitmapHeaderSize = 36;
//		}
//		return bitmapHeaderSize;
		return 46;
	}

	/**
	 * Gets the byte size of a bitmap with the given width
	 * and height.
	 */
	static inline uint32 getBitmapSize(const uint16 width, const uint16 height) {
		return width * height + getBitmapHeaderSize();
	}

	inline SciBitmap() : _data(nullptr), _dataSize(0), _gc(true) {}

	inline SciBitmap(const SciBitmap &other) {
		_dataSize = other._dataSize;
		_data = (byte *)malloc(other._dataSize);
		memcpy(_data, other._data, other._dataSize);
		if (_dataSize) {
			_buffer.init(getWidth(), getHeight(), getWidth(), getPixels(), Graphics::PixelFormat::createFormatCLUT8());
		}
		_gc = other._gc;
	}

	inline ~SciBitmap() override {
		free(_data);
		_data = nullptr;
		_dataSize = 0;
	}

	inline SciBitmap &operator=(const SciBitmap &other) {
		if (this == &other) {
			return *this;
		}

		free(_data);
		_dataSize = other._dataSize;
		_data = (byte *)malloc(other._dataSize);
		memcpy(_data, other._data, _dataSize);
		if (_dataSize) {
			_buffer.init(getWidth(), getHeight(), getWidth(), getPixels(), Graphics::PixelFormat::createFormatCLUT8());
		}
		_gc = other._gc;

		return *this;
	}

	/**
	 * Allocates and initialises a new bitmap.
	 */
	inline void create(const int16 width, const int16 height, const uint8 skipColor, const int16 originX, const int16 originY, const int16 xResolution, const int16 yResolution, const uint32 paletteSize, const bool remap, const bool gc) {

		_dataSize = getBitmapSize(width, height) + paletteSize;
		_data = (byte *)realloc(_data, _dataSize);
		_gc = gc;

		const uint16 bitmapHeaderSize = getBitmapHeaderSize();

		setWidth(width);
		setHeight(height);
		setOrigin(Common::Point(originX, originY));
		setSkipColor(skipColor);
		_data[9] = 0;
		WRITE_SCI11ENDIAN_UINT16(_data + 10, 0);
		setRemap(remap);
		setDataSize(width * height);
		WRITE_SCI11ENDIAN_UINT32(_data + 16, 0);
		setHunkPaletteOffset(paletteSize > 0 ? (bitmapHeaderSize + width * height) : 0);
		setDataOffset(bitmapHeaderSize);
		setUncompressedDataOffset(bitmapHeaderSize);
		setControlOffset(0);
		setXResolution(xResolution);
		setYResolution(yResolution);

		_buffer.init(getWidth(), getHeight(), getWidth(), getPixels(), Graphics::PixelFormat::createFormatCLUT8());
	}

	inline int getRawSize() const {
		return _dataSize;
	}

	inline byte *getRawData() const {
		return _data;
	}

	inline Buffer &getBuffer() {
		return _buffer;
	}

	inline bool getShouldGC() const {
		return _gc;
	}

	inline void enableGC() {
		_gc = true;
	}

	inline void disableGC() {
		_gc = false;
	}

	BITMAP_PROPERTY(16, Width, 0);
	BITMAP_PROPERTY(16, Height, 2);

	inline Common::Point getOrigin() const {
		return Common::Point(
			(int16)READ_SCI11ENDIAN_UINT16(_data + 4),
			(int16)READ_SCI11ENDIAN_UINT16(_data + 6)
		);
	}

	inline void setOrigin(const Common::Point &origin) {
		WRITE_SCI11ENDIAN_UINT16(_data + 4, (uint16)origin.x);
		WRITE_SCI11ENDIAN_UINT16(_data + 6, (uint16)origin.y);
	}

	inline uint8 getSkipColor() const {
		return _data[8];
	}

	inline void setSkipColor(const uint8 skipColor) {
		_data[8] = skipColor;
	}

	inline bool getRemap() const {
		return READ_SCI11ENDIAN_UINT16(_data + 10) & kBitmapRemap;
	}

	inline void setRemap(const bool remap) {
		uint16 flags = READ_SCI11ENDIAN_UINT16(_data + 10);
		if (remap) {
			flags |= kBitmapRemap;
		} else {
			flags &= ~kBitmapRemap;
		}
		WRITE_SCI11ENDIAN_UINT16(_data + 10, flags);
	}

	BITMAP_PROPERTY(32, DataSize, 12);

	BITMAP_PROPERTY(32, HunkPaletteOffset, 20);

	BITMAP_PROPERTY(32, DataOffset, 24);

	// This property is used as a "magic number" for validating that a block of
	// memory is a valid bitmap, and so is always set to the size of the header.
	BITMAP_PROPERTY(32, UncompressedDataOffset, 28);

	// This property always seems to be zero in SSCI
	BITMAP_PROPERTY(32, ControlOffset, 32);

	inline uint16 getXResolution() const {
		if (getDataOffset() >= 40) {
			return READ_SCI11ENDIAN_UINT16(_data + 36);
		}

		// SCI2 bitmaps did not have scaling ability
		return 320;
	}

	inline void setXResolution(uint16 xResolution) {
		if (getDataOffset() >= 40) {
			WRITE_SCI11ENDIAN_UINT16(_data + 36, xResolution);
		}
	}

	inline uint16 getYResolution() const {
		if (getDataOffset() >= 40) {
			return READ_SCI11ENDIAN_UINT16(_data + 38);
		}

		// SCI2 bitmaps did not have scaling ability
		return 200;
	}

	inline void setYResolution(uint16 yResolution) {
		if (getDataOffset() >= 40) {
			WRITE_SCI11ENDIAN_UINT16(_data + 38, yResolution);
		}
	}

	inline byte *getPixels() {
		return _data + getUncompressedDataOffset();
	}

	inline byte *getHunkPalette() {
		if (getHunkPaletteOffset() == 0) {
			return nullptr;
		}
		return _data + getHunkPaletteOffset();
	}

	inline void setPalette(const Palette &palette) {
		byte *paletteData = getHunkPalette();
		if (paletteData != nullptr) {
			SciSpan<byte> paletteSpan(paletteData, getRawSize() - getHunkPaletteOffset());
			HunkPalette::write(paletteSpan, palette);
		}
	}

	void saveLoadWithSerializer(Common::Serializer &ser) override;

	void applyRemap(SciArray &clut) {
		const int length = getWidth() * getHeight();
		uint8 *pixel = getPixels();
		for (int i = 0; i < length; ++i) {
			const int16 color = clut.getAsInt16(*pixel);
			assert(color >= 0 && color <= 255);
			*pixel++ = (uint8)color;
		}
	}

	Common::String toString() const {
		return Common::String::format("%dx%d; res %dx%d; origin %dx%d; skip color %u; %s; %s)",
			getWidth(), getHeight(),
			getXResolution(), getYResolution(),
			getOrigin().x, getOrigin().y,
			getSkipColor(),
			getRemap() ? "remap" : "no remap",
			getShouldGC() ? "GC" : "no GC");
	}
};

#undef BITMAP_PROPERTY

struct BitmapTable : public SegmentObjTable<SciBitmap> {
	BitmapTable() : SegmentObjTable<SciBitmap>(SEG_TYPE_BITMAP) {}

	SegmentRef dereference(reg_t pointer) override {
		SegmentRef ret;
		ret.isRaw = true;
		ret.maxSize = at(pointer.getOffset()).getRawSize();
		ret.raw = at(pointer.getOffset()).getRawData();
		return ret;
	}

	void saveLoadWithSerializer(Common::Serializer &ser) override;
};

#endif


} // End of namespace Sci

#endif // SCI_ENGINE_SEGMENT_H
