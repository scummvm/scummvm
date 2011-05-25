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
 */

#include "sci/sci.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"
#include "sci/engine/script.h"

namespace Sci {


enum {
	DEFAULT_SCRIPTS = 32,
	DEFAULT_OBJECTS = 8,			///< default number of objects per script
	DEFAULT_OBJECTS_INCREMENT = 4	///< Number of additional objects to instantiate if we're running out of them
};

SegManager::SegManager(ResourceManager *resMan) {
	_heap.push_back(0);

	_clonesSegId = 0;
	_listsSegId = 0;
	_nodesSegId = 0;
	_hunksSegId = 0;

#ifdef ENABLE_SCI32
	_arraysSegId = 0;
	_stringSegId = 0;
#endif

	_resMan = resMan;

	createClassTable();
}

SegManager::~SegManager() {
	resetSegMan();
}

void SegManager::resetSegMan() {
	// Free memory
	for (uint i = 0; i < _heap.size(); i++) {
		if (_heap[i])
			deallocate(i);
	}

	_heap.clear();

	// And reinitialize
	_heap.push_back(0);

	_clonesSegId = 0;
	_listsSegId = 0;
	_nodesSegId = 0;
	_hunksSegId = 0;

#ifdef ENABLE_SCI32
	_arraysSegId = 0;
	_stringSegId = 0;
#endif

	// Reinitialize class table
	_classTable.clear();
	createClassTable();
}

void SegManager::initSysStrings() {
	if (getSciVersion() <= SCI_VERSION_1_1) {
		// We need to allocate system strings in one segment, for compatibility reasons
		allocDynmem(512, "system strings", &_saveDirPtr);
		_parserPtr = make_reg(_saveDirPtr.segment, _saveDirPtr.offset + 256);
#ifdef ENABLE_SCI32
	} else {
		SciString *saveDirString = allocateString(&_saveDirPtr);
		saveDirString->setSize(256);
		saveDirString->setValue(0, 0);

		_parserPtr = NULL_REG;	// no SCI2 game had a parser
#endif
	}
}

SegmentId SegManager::findFreeSegment() const {
	// The following is a very crude approach: We find a free segment id by
	// scanning from the start. This can be slow if the number of segments
	// becomes large. Optimizations are possible and easy, but I'll refrain
	// from attempting any until we determine we actually need it.
	uint seg = 1;
	while (seg < _heap.size() && _heap[seg]) {
		++seg;
	}
	assert(seg < 65536);
	return seg;
}

SegmentObj *SegManager::allocSegment(SegmentObj *mem, SegmentId *segid) {
	// Find a free segment
	SegmentId id = findFreeSegment();
	if (segid)
		*segid = id;

	if (!mem)
		error("SegManager: invalid mobj");

	// ... and put it into the (formerly) free segment.
	if (id >= (int)_heap.size()) {
		assert(id == (int)_heap.size());
		_heap.push_back(0);
	}
	_heap[id] = mem;

	return mem;
}

Script *SegManager::allocateScript(int script_nr, SegmentId *segid) {
	// Check if the script already has an allocated segment. If it
	// does, return that segment.
	*segid = _scriptSegMap.getVal(script_nr, 0);
	if (*segid > 0) {
		return (Script *)_heap[*segid];
	}

	// allocate the SegmentObj
	SegmentObj *mem = allocSegment(new Script(), segid);

	// Add the script to the "script id -> segment id" hashmap
	_scriptSegMap[script_nr] = *segid;

	return (Script *)mem;
}

void SegManager::deallocate(SegmentId seg) {
	if (!check(seg))
		error("SegManager::deallocate(): invalid segment ID");

	SegmentObj *mobj = _heap[seg];

	if (mobj->getType() == SEG_TYPE_SCRIPT) {
		Script *scr = (Script *)mobj;
		_scriptSegMap.erase(scr->getScriptNumber());
		if (scr->_localsSegment)
			deallocate(scr->_localsSegment);
	}

	delete mobj;
	_heap[seg] = NULL;
}

bool SegManager::isHeapObject(reg_t pos) const {
	const Object *obj = getObject(pos);
	if (obj == NULL || (obj && obj->isFreed()))
		return false;
	Script *scr = getScriptIfLoaded(pos.segment);
	return !(scr && scr->isMarkedAsDeleted());
}

void SegManager::deallocateScript(int script_nr) {
	deallocate(getScriptSegment(script_nr));
}

Script *SegManager::getScript(const SegmentId seg) {
	if (seg < 1 || (uint)seg >= _heap.size()) {
		error("SegManager::getScript(): seg id %x out of bounds", seg);
	}
	if (!_heap[seg]) {
		error("SegManager::getScript(): seg id %x is not in memory", seg);
	}
	if (_heap[seg]->getType() != SEG_TYPE_SCRIPT) {
		error("SegManager::getScript(): seg id %x refers to type %d != SEG_TYPE_SCRIPT", seg, _heap[seg]->getType());
	}
	return (Script *)_heap[seg];
}

Script *SegManager::getScriptIfLoaded(const SegmentId seg) const {
	if (seg < 1 || (uint)seg >= _heap.size() || !_heap[seg] || _heap[seg]->getType() != SEG_TYPE_SCRIPT)
		return 0;
	return (Script *)_heap[seg];
}

SegmentId SegManager::findSegmentByType(int type) const {
	for (uint i = 0; i < _heap.size(); i++)
		if (_heap[i] && _heap[i]->getType() == type)
			return i;
	return 0;
}

SegmentObj *SegManager::getSegmentObj(SegmentId seg) const {
	if (seg < 1 || (uint)seg >= _heap.size() || !_heap[seg])
		return 0;
	return _heap[seg];
}

SegmentType SegManager::getSegmentType(SegmentId seg) const {
	if (seg < 1 || (uint)seg >= _heap.size() || !_heap[seg])
		return SEG_TYPE_INVALID;
	return _heap[seg]->getType();
}

SegmentObj *SegManager::getSegment(SegmentId seg, SegmentType type) const {
	return getSegmentType(seg) == type ? _heap[seg] : NULL;
}

Object *SegManager::getObject(reg_t pos) const {
	SegmentObj *mobj = getSegmentObj(pos.segment);
	Object *obj = NULL;

	if (mobj != NULL) {
		if (mobj->getType() == SEG_TYPE_CLONES) {
			CloneTable *ct = (CloneTable *)mobj;
			if (ct->isValidEntry(pos.offset))
				obj = &(ct->_table[pos.offset]);
			else
				warning("getObject(): Trying to get an invalid object");
		} else if (mobj->getType() == SEG_TYPE_SCRIPT) {
			Script *scr = (Script *)mobj;
			if (pos.offset <= scr->getBufSize() && pos.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET
			        && RAW_IS_OBJECT(scr->getBuf(pos.offset))) {
				obj = scr->getObject(pos.offset);
			}
		}
	}

	return obj;
}

const char *SegManager::getObjectName(reg_t pos) {
	const Object *obj = getObject(pos);
	if (!obj)
		return "<no such object>";

	reg_t nameReg = obj->getNameSelector();
	if (nameReg.isNull())
		return "<no name>";

	const char *name = 0;
	if (nameReg.segment)
		name  = derefString(nameReg);
	if (!name)
		return "<invalid name>";

	return name;
}

reg_t SegManager::findObjectByName(const Common::String &name, int index) {
	Common::Array<reg_t> result;
	uint i;

	// Now all values are available; iterate over all objects.
	for (i = 0; i < _heap.size(); i++) {
		const SegmentObj *mobj = _heap[i];

		if (!mobj)
			continue;

		reg_t objpos = make_reg(i, 0);

		if (mobj->getType() == SEG_TYPE_SCRIPT) {
			// It's a script, scan all objects in it
			const Script *scr = (const Script *)mobj;
			for (ObjMap::const_iterator it = scr->_objects.begin(); it != scr->_objects.end(); ++it) {
				objpos.offset = it->_value.getPos().offset;
				if (name == getObjectName(objpos))
					result.push_back(objpos);
			}
		} else  if (mobj->getType() == SEG_TYPE_CLONES) {
			// It's clone table, scan all objects in it
			const CloneTable *ct = (const CloneTable *)mobj;
			for (uint idx = 0; idx < ct->_table.size(); ++idx) {
				if (!ct->isValidEntry(idx))
					continue;

				objpos.offset = idx;
				if (name == getObjectName(objpos))
					result.push_back(objpos);
			}
		}
	}

	if (result.empty())
		return NULL_REG;

	if (result.size() > 1 && index < 0) {
		debug("findObjectByName(%s): multiple matches:", name.c_str());
		for (i = 0; i < result.size(); i++)
			debug("  %3x: [%04x:%04x]", i, PRINT_REG(result[i]));
		return NULL_REG; // Ambiguous
	}

	if (index < 0)
		return result[0];
	else if (result.size() <= (uint)index)
		return NULL_REG; // Not found
	return result[index];
}

// validate the seg
// return:
//	false - invalid seg
//	true  - valid seg
bool SegManager::check(SegmentId seg) {
	if (seg < 1 || (uint)seg >= _heap.size()) {
		return false;
	}
	if (!_heap[seg]) {
		warning("SegManager: seg %x is removed from memory, but not removed from hash_map", seg);
		return false;
	}
	return true;
}

// return the seg if script_id is valid and in the map, else 0
SegmentId SegManager::getScriptSegment(int script_id) const {
	return _scriptSegMap.getVal(script_id, 0);
}

SegmentId SegManager::getScriptSegment(int script_nr, ScriptLoadType load) {
	SegmentId segment;

	if ((load & SCRIPT_GET_LOAD) == SCRIPT_GET_LOAD)
		instantiateScript(script_nr);

	segment = getScriptSegment(script_nr);

	if (segment > 0) {
		if ((load & SCRIPT_GET_LOCK) == SCRIPT_GET_LOCK)
			getScript(segment)->incrementLockers();
	}
	return segment;
}

LocalVariables *SegManager::allocLocalsSegment(Script *scr) {
	if (!scr->getLocalsCount()) { // No locals
		scr->_localsSegment = 0;
		scr->_localsBlock = NULL;
		return NULL;
	} else {
		LocalVariables *locals;

		if (scr->_localsSegment) {
			locals = (LocalVariables *)_heap[scr->_localsSegment];
			if (!locals || locals->getType() != SEG_TYPE_LOCALS || locals->script_id != scr->getScriptNumber())
				error("Invalid script locals segment while allocating locals");
		} else
			locals = (LocalVariables *)allocSegment(new LocalVariables(), &scr->_localsSegment);

		scr->_localsBlock = locals;
		locals->script_id = scr->getScriptNumber();
		locals->_locals.resize(scr->getLocalsCount());

		return locals;
	}
}

DataStack *SegManager::allocateStack(int size, SegmentId *segid) {
	SegmentObj *mobj = allocSegment(new DataStack(), segid);
	DataStack *retval = (DataStack *)mobj;

	retval->_entries = (reg_t *)calloc(size, sizeof(reg_t));
	retval->_capacity = size;

	// SSCI initializes the stack with "S" characters (uppercase S in SCI0-SCI1,
	// lowercase s in SCI0 and SCI11) - probably stands for "stack"
	byte filler = (getSciVersion() >= SCI_VERSION_01 && getSciVersion() <= SCI_VERSION_1_LATE) ? 'S' : 's';
	for (int i = 0; i < size; i++)
		retval->_entries[i] = make_reg(0, filler);

	return retval;
}

void SegManager::freeHunkEntry(reg_t addr) {
	if (addr.isNull()) {
		warning("Attempt to free a Hunk from a null address");
		return;
	}

	HunkTable *ht = (HunkTable *)getSegment(addr.segment, SEG_TYPE_HUNK);

	if (!ht) {
		warning("Attempt to free Hunk from address %04x:%04x: Invalid segment type", PRINT_REG(addr));
		return;
	}

	ht->freeEntryContents(addr.offset);
}

reg_t SegManager::allocateHunkEntry(const char *hunk_type, int size) {
	HunkTable *table;
	int offset;

	if (!_hunksSegId)
		allocSegment(new HunkTable(), &(_hunksSegId));
	table = (HunkTable *)_heap[_hunksSegId];

	offset = table->allocEntry();

	reg_t addr = make_reg(_hunksSegId, offset);
	Hunk *h = &(table->_table[offset]);

	if (!h)
		return NULL_REG;

	h->mem = malloc(size);
	h->size = size;
	h->type = hunk_type;

	return addr;
}

byte *SegManager::getHunkPointer(reg_t addr) {
	HunkTable *ht = (HunkTable *)getSegment(addr.segment, SEG_TYPE_HUNK);

	if (!ht || !ht->isValidEntry(addr.offset)) {
		// Valid SCI behavior, e.g. when loading/quitting
		return NULL;
	}

	return (byte *)ht->_table[addr.offset].mem;
}

Clone *SegManager::allocateClone(reg_t *addr) {
	CloneTable *table;
	int offset;

	if (!_clonesSegId)
		table = (CloneTable *)allocSegment(new CloneTable(), &(_clonesSegId));
	else
		table = (CloneTable *)_heap[_clonesSegId];

	offset = table->allocEntry();

	*addr = make_reg(_clonesSegId, offset);
	return &(table->_table[offset]);
}

List *SegManager::allocateList(reg_t *addr) {
	ListTable *table;
	int offset;

	if (!_listsSegId)
		allocSegment(new ListTable(), &(_listsSegId));
	table = (ListTable *)_heap[_listsSegId];

	offset = table->allocEntry();

	*addr = make_reg(_listsSegId, offset);
	return &(table->_table[offset]);
}

Node *SegManager::allocateNode(reg_t *addr) {
	NodeTable *table;
	int offset;

	if (!_nodesSegId)
		allocSegment(new NodeTable(), &(_nodesSegId));
	table = (NodeTable *)_heap[_nodesSegId];

	offset = table->allocEntry();

	*addr = make_reg(_nodesSegId, offset);
	return &(table->_table[offset]);
}

reg_t SegManager::newNode(reg_t value, reg_t key) {
	reg_t nodeRef;
	Node *n = allocateNode(&nodeRef);
	n->pred = n->succ = NULL_REG;
	n->key = key;
	n->value = value;

	return nodeRef;
}

List *SegManager::lookupList(reg_t addr) {
	if (getSegmentType(addr.segment) != SEG_TYPE_LISTS) {
		error("Attempt to use non-list %04x:%04x as list", PRINT_REG(addr));
		return NULL;
	}

	ListTable *lt = (ListTable *)_heap[addr.segment];

	if (!lt->isValidEntry(addr.offset)) {
		error("Attempt to use non-list %04x:%04x as list", PRINT_REG(addr));
		return NULL;
	}

	return &(lt->_table[addr.offset]);
}

Node *SegManager::lookupNode(reg_t addr, bool stopOnDiscarded) {
	if (addr.isNull())
		return NULL; // Non-error null

	SegmentType type = getSegmentType(addr.segment);

	if (type != SEG_TYPE_NODES) {
		error("Attempt to use non-node %04x:%04x (type %d) as list node", PRINT_REG(addr), type);
		return NULL;
	}

	NodeTable *nt = (NodeTable *)_heap[addr.segment];

	if (!nt->isValidEntry(addr.offset)) {
		if (!stopOnDiscarded)
			return NULL;

		error("Attempt to use invalid or discarded reference %04x:%04x as list node", PRINT_REG(addr));
		return NULL;
	}

	return &(nt->_table[addr.offset]);
}

SegmentRef SegManager::dereference(reg_t pointer) {
	SegmentRef ret;

	if (!pointer.segment || (pointer.segment >= _heap.size()) || !_heap[pointer.segment]) {
		// This occurs in KQ5CD when interacting with certain objects
		warning("SegManager::dereference(): Attempt to dereference invalid pointer %04x:%04x", PRINT_REG(pointer));
		return ret; /* Invalid */
	}

	SegmentObj *mobj = _heap[pointer.segment];
	return mobj->dereference(pointer);
}

static void *derefPtr(SegManager *segMan, reg_t pointer, int entries, bool wantRaw) {
	SegmentRef ret = segMan->dereference(pointer);

	if (!ret.isValid())
		return NULL;

	if (ret.isRaw != wantRaw) {
		warning("Dereferencing pointer %04x:%04x (type %d) which is %s, but expected %s", PRINT_REG(pointer),
			segMan->getSegmentType(pointer.segment),
			ret.isRaw ? "raw" : "not raw",
			wantRaw ? "raw" : "not raw");
	}

	if (!wantRaw && ret.skipByte) {
		warning("Unaligned pointer read: %04x:%04x expected with word alignment", PRINT_REG(pointer));
		return NULL;
	}

	if (entries > ret.maxSize) {
		warning("Trying to dereference pointer %04x:%04x beyond end of segment", PRINT_REG(pointer));
		return NULL;
	}

	if (ret.isRaw)
		return ret.raw;
	else
		return ret.reg;
}

byte *SegManager::derefBulkPtr(reg_t pointer, int entries) {
	return (byte *)derefPtr(this, pointer, entries, true);
}

reg_t *SegManager::derefRegPtr(reg_t pointer, int entries) {
	return (reg_t *)derefPtr(this, pointer, 2*entries, false);
}

char *SegManager::derefString(reg_t pointer, int entries) {
	return (char *)derefPtr(this, pointer, entries, true);
}

// Helper functions for getting/setting characters in string fragments
static inline char getChar(const SegmentRef &ref, uint offset) {
	if (ref.skipByte)
		offset++;

	reg_t val = ref.reg[offset / 2];

	// segment 0xFFFF means that the scripts are using uninitialized temp-variable space
	//  we can safely ignore this, if it isn't one of the first 2 chars.
	//  foreign lsl3 uses kFileIO(readraw) and then immediately uses kReadNumber right at the start
	if (val.segment != 0)
		if (!((val.segment == 0xFFFF) && (offset > 1)))
			warning("Attempt to read character from non-raw data");

	bool oddOffset = offset & 1;
	if (g_sci->isBE())
		oddOffset = !oddOffset;

	return (oddOffset ? val.offset >> 8 : val.offset & 0xff);
}

static inline void setChar(const SegmentRef &ref, uint offset, byte value) {
	if (ref.skipByte)
		offset++;

	reg_t *val = ref.reg + offset / 2;

	val->segment = 0;

	bool oddOffset = offset & 1;
	if (g_sci->isBE())
		oddOffset = !oddOffset;

	if (oddOffset)
		val->offset = (val->offset & 0x00ff) | (value << 8);
	else
		val->offset = (val->offset & 0xff00) | value;
}

// TODO: memcpy, strcpy and strncpy could maybe be folded into a single function
void SegManager::strncpy(reg_t dest, const char* src, size_t n) {
	SegmentRef dest_r = dereference(dest);
	if (!dest_r.isValid()) {
		warning("Attempt to strncpy to invalid pointer %04x:%04x", PRINT_REG(dest));
		return;
	}


	if (dest_r.isRaw) {
		// raw -> raw
		if (n == 0xFFFFFFFFU)
			::strcpy((char *)dest_r.raw, src);
		else
			::strncpy((char *)dest_r.raw, src, n);
	} else {
		// raw -> non-raw
		for (uint i = 0; i < n; i++) {
			setChar(dest_r, i, src[i]);
			if (!src[i])
				break;
		}
		// Put an ending NUL to terminate the string
		if ((size_t)dest_r.maxSize > n)
			setChar(dest_r, n, 0);
	}
}

void SegManager::strncpy(reg_t dest, reg_t src, size_t n) {
	if (src.isNull()) {
		// Clear target string instead.
		if (n > 0)
			strcpy(dest, "");

		return;	// empty text
	}

	SegmentRef dest_r = dereference(dest);
	const SegmentRef src_r = dereference(src);
	if (!src_r.isValid()) {
		warning("Attempt to strncpy from invalid pointer %04x:%04x", PRINT_REG(src));

		// Clear target string instead.
		if (n > 0)
			strcpy(dest, "");
		return;
	}

	if (!dest_r.isValid()) {
		warning("Attempt to strncpy to invalid pointer %04x:%04x", PRINT_REG(dest));
		return;
	}


	if (src_r.isRaw) {
		// raw -> *
		strncpy(dest, (const char*)src_r.raw, n);
	} else if (dest_r.isRaw && !src_r.isRaw) {
		// non-raw -> raw
		for (uint i = 0; i < n; i++) {
			char c = getChar(src_r, i);
			dest_r.raw[i] = c;
			if (!c)
				break;
		}
	} else {
		// non-raw -> non-raw
		for (uint i = 0; i < n; i++) {
			char c = getChar(src_r, i);
			setChar(dest_r, i, c);
			if (!c)
				break;
		}
	}
}

void SegManager::strcpy(reg_t dest, const char* src) {
	strncpy(dest, src, 0xFFFFFFFFU);
}

void SegManager::strcpy(reg_t dest, reg_t src) {
	strncpy(dest, src, 0xFFFFFFFFU);
}

void SegManager::memcpy(reg_t dest, const byte* src, size_t n) {
	SegmentRef dest_r = dereference(dest);
	if (!dest_r.isValid()) {
		warning("Attempt to memcpy to invalid pointer %04x:%04x", PRINT_REG(dest));
		return;
	}
	if ((int)n > dest_r.maxSize) {
		warning("Trying to dereference pointer %04x:%04x beyond end of segment", PRINT_REG(dest));
		return;
	}

	if (dest_r.isRaw) {
		// raw -> raw
		::memcpy((char*)dest_r.raw, src, n);
	} else {
		// raw -> non-raw
		for (uint i = 0; i < n; i++)
			setChar(dest_r, i, src[i]);
	}
}

void SegManager::memcpy(reg_t dest, reg_t src, size_t n) {
	SegmentRef dest_r = dereference(dest);
	const SegmentRef src_r = dereference(src);
	if (!dest_r.isValid()) {
		warning("Attempt to memcpy to invalid pointer %04x:%04x", PRINT_REG(dest));
		return;
	}
	if ((int)n > dest_r.maxSize) {
		warning("Trying to dereference pointer %04x:%04x beyond end of segment", PRINT_REG(dest));
		return;
	}
	if (!src_r.isValid()) {
		warning("Attempt to memcpy from invalid pointer %04x:%04x", PRINT_REG(src));
		return;
	}
	if ((int)n > src_r.maxSize) {
		warning("Trying to dereference pointer %04x:%04x beyond end of segment", PRINT_REG(src));
		return;
	}

	if (src_r.isRaw) {
		// raw -> *
		memcpy(dest, src_r.raw, n);
	} else if (dest_r.isRaw) {
		// * -> raw
		memcpy(dest_r.raw, src, n);
	} else {
		// non-raw -> non-raw
		for (uint i = 0; i < n; i++) {
			char c = getChar(src_r, i);
			setChar(dest_r, i, c);
		}
	}
}

void SegManager::memcpy(byte *dest, reg_t src, size_t n) {
	const SegmentRef src_r = dereference(src);
	if (!src_r.isValid()) {
		warning("Attempt to memcpy from invalid pointer %04x:%04x", PRINT_REG(src));
		return;
	}
	if ((int)n > src_r.maxSize) {
		warning("Trying to dereference pointer %04x:%04x beyond end of segment", PRINT_REG(src));
		return;
	}

	if (src_r.isRaw) {
		// raw -> raw
		::memcpy(dest, src_r.raw, n);
	} else {
		// non-raw -> raw
		for (uint i = 0; i < n; i++) {
			char c = getChar(src_r, i);
			dest[i] = c;
		}
	}
}

size_t SegManager::strlen(reg_t str) {
	if (str.isNull())
		return 0;	// empty text

	SegmentRef str_r = dereference(str);
	if (!str_r.isValid()) {
		warning("Attempt to call strlen on invalid pointer %04x:%04x", PRINT_REG(str));
		return 0;
	}

	if (str_r.isRaw) {
		return ::strlen((const char *)str_r.raw);
	} else {
		int i = 0;
		while (getChar(str_r, i))
			i++;
		return i;
	}
}


Common::String SegManager::getString(reg_t pointer, int entries) {
	Common::String ret;
	if (pointer.isNull())
		return ret;	// empty text

	SegmentRef src_r = dereference(pointer);
	if (!src_r.isValid()) {
		warning("SegManager::getString(): Attempt to dereference invalid pointer %04x:%04x", PRINT_REG(pointer));
		return ret;
	}
	if (entries > src_r.maxSize) {
		warning("Trying to dereference pointer %04x:%04x beyond end of segment", PRINT_REG(pointer));
		return ret;
	}
	if (src_r.isRaw)
		ret = (char *)src_r.raw;
	else {
		uint i = 0;
		for (;;) {
			char c = getChar(src_r, i);

			if (!c)
				break;

			i++;
			ret += c;
		};
	}
	return ret;
}

byte *SegManager::allocDynmem(int size, const char *descr, reg_t *addr) {
	SegmentId seg;
	SegmentObj *mobj = allocSegment(new DynMem(), &seg);
	*addr = make_reg(seg, 0);

	DynMem &d = *(DynMem *)mobj;

	d._size = size;

	if (size == 0)
		d._buf = NULL;
	else
		d._buf = (byte *)malloc(size);

	d._description = descr;

	return (byte *)(d._buf);
}

bool SegManager::freeDynmem(reg_t addr) {
	if (addr.segment < 1 || addr.segment >= _heap.size() || !_heap[addr.segment] || _heap[addr.segment]->getType() != SEG_TYPE_DYNMEM)
		return false; // error

	deallocate(addr.segment);

	return true; // OK
}

#ifdef ENABLE_SCI32
SciArray<reg_t> *SegManager::allocateArray(reg_t *addr) {
	ArrayTable *table;
	int offset;

	if (!_arraysSegId) {
		table = (ArrayTable *)allocSegment(new ArrayTable(), &(_arraysSegId));
	} else
		table = (ArrayTable *)_heap[_arraysSegId];

	offset = table->allocEntry();

	*addr = make_reg(_arraysSegId, offset);
	return &(table->_table[offset]);
}

SciArray<reg_t> *SegManager::lookupArray(reg_t addr) {
	if (_heap[addr.segment]->getType() != SEG_TYPE_ARRAY)
		error("Attempt to use non-array %04x:%04x as array", PRINT_REG(addr));

	ArrayTable *arrayTable = (ArrayTable *)_heap[addr.segment];

	if (!arrayTable->isValidEntry(addr.offset))
		error("Attempt to use non-array %04x:%04x as array", PRINT_REG(addr));

	return &(arrayTable->_table[addr.offset]);
}

void SegManager::freeArray(reg_t addr) {
	if (_heap[addr.segment]->getType() != SEG_TYPE_ARRAY)
		error("Attempt to use non-array %04x:%04x as array", PRINT_REG(addr));

	ArrayTable *arrayTable = (ArrayTable *)_heap[addr.segment];

	if (!arrayTable->isValidEntry(addr.offset))
		error("Attempt to use non-array %04x:%04x as array", PRINT_REG(addr));

	arrayTable->_table[addr.offset].destroy();
	arrayTable->freeEntry(addr.offset);
}

SciString *SegManager::allocateString(reg_t *addr) {
	StringTable *table;
	int offset;

	if (!_stringSegId) {
		table = (StringTable *)allocSegment(new StringTable(), &(_stringSegId));
	} else
		table = (StringTable *)_heap[_stringSegId];

	offset = table->allocEntry();

	*addr = make_reg(_stringSegId, offset);
	return &(table->_table[offset]);
}

SciString *SegManager::lookupString(reg_t addr) {
	if (_heap[addr.segment]->getType() != SEG_TYPE_STRING)
		error("lookupString: Attempt to use non-string %04x:%04x as string", PRINT_REG(addr));

	StringTable *stringTable = (StringTable *)_heap[addr.segment];

	if (!stringTable->isValidEntry(addr.offset))
		error("lookupString: Attempt to use non-string %04x:%04x as string", PRINT_REG(addr));

	return &(stringTable->_table[addr.offset]);
}

void SegManager::freeString(reg_t addr) {
	if (_heap[addr.segment]->getType() != SEG_TYPE_STRING)
		error("freeString: Attempt to use non-string %04x:%04x as string", PRINT_REG(addr));

	StringTable *stringTable = (StringTable *)_heap[addr.segment];

	if (!stringTable->isValidEntry(addr.offset))
		error("freeString: Attempt to use non-string %04x:%04x as string", PRINT_REG(addr));

	stringTable->_table[addr.offset].destroy();
	stringTable->freeEntry(addr.offset);
}

#endif

void SegManager::createClassTable() {
	Resource *vocab996 = _resMan->findResource(ResourceId(kResourceTypeVocab, 996), 1);

	if (!vocab996)
		error("SegManager: failed to open vocab 996");

	int totalClasses = vocab996->size >> 2;
	_classTable.resize(totalClasses);

	for (uint16 classNr = 0; classNr < totalClasses; classNr++) {
		uint16 scriptNr = READ_SCI11ENDIAN_UINT16(vocab996->data + classNr * 4 + 2);

		_classTable[classNr].reg = NULL_REG;
		_classTable[classNr].script = scriptNr;
	}

	_resMan->unlockResource(vocab996);
}

reg_t SegManager::getClassAddress(int classnr, ScriptLoadType lock, reg_t caller) {
	if (classnr == 0xffff)
		return NULL_REG;

	if (classnr < 0 || (int)_classTable.size() <= classnr || _classTable[classnr].script < 0) {
		error("[VM] Attempt to dereference class %x, which doesn't exist (max %x)", classnr, _classTable.size());
		return NULL_REG;
	} else {
		Class *the_class = &_classTable[classnr];
		if (!the_class->reg.segment) {
			getScriptSegment(the_class->script, lock);

			if (!the_class->reg.segment) {
				error("[VM] Trying to instantiate class %x by instantiating script 0x%x (%03d) failed;", classnr, the_class->script, the_class->script);
				return NULL_REG;
			}
		} else
			if (caller.segment != the_class->reg.segment)
				getScript(the_class->reg.segment)->incrementLockers();

		return the_class->reg;
	}
}

int SegManager::instantiateScript(int scriptNum) {
	SegmentId segmentId = getScriptSegment(scriptNum);
	Script *scr = getScriptIfLoaded(segmentId);
	if (scr) {
		if (!scr->isMarkedAsDeleted()) {
			scr->incrementLockers();
			return segmentId;
		} else {
			scr->freeScript();
		}
	} else {
		scr = allocateScript(scriptNum, &segmentId);
	}

	scr->init(scriptNum, _resMan);
	scr->load(_resMan);
	scr->initializeLocals(this);
	scr->initializeClasses(this);
	scr->initializeObjects(this, segmentId);

	return segmentId;
}

void SegManager::uninstantiateScript(int script_nr) {
	SegmentId segmentId = getScriptSegment(script_nr);
	Script *scr = getScriptIfLoaded(segmentId);

	if (!scr || scr->isMarkedAsDeleted()) {   // Is it already unloaded?
		//warning("unloading script 0x%x requested although not loaded", script_nr);
		// This is perfectly valid SCI behavior
		return;
	}

	scr->decrementLockers();   // One less locker

	if (scr->getLockers() > 0)
		return;

	// Free all classtable references to this script
	for (uint i = 0; i < classTableSize(); i++)
		if (getClass(i).reg.segment == segmentId)
			setClassOffset(i, NULL_REG);

	if (getSciVersion() < SCI_VERSION_1_1)
		uninstantiateScriptSci0(script_nr);
	// FIXME: Add proper script uninstantiation for SCI 1.1

	if (!scr->getLockers()) {
		// The actual script deletion seems to be done by SCI scripts themselves
		scr->markDeleted();
		debugC(kDebugLevelScripts, "Unloaded script 0x%x.", script_nr);
	}
}

void SegManager::uninstantiateScriptSci0(int script_nr) {
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);
	SegmentId segmentId = getScriptSegment(script_nr);
	Script *scr = getScript(segmentId);
	reg_t reg = make_reg(segmentId, oldScriptHeader ? 2 : 0);
	int objType, objLength = 0;

	// Make a pass over the object in order to uninstantiate all superclasses

	do {
		reg.offset += objLength; // Step over the last checked object

		objType = READ_SCI11ENDIAN_UINT16(scr->getBuf(reg.offset));
		if (!objType)
			break;
		objLength = READ_SCI11ENDIAN_UINT16(scr->getBuf(reg.offset + 2));

		reg.offset += 4; // Step over header

		if ((objType == SCI_OBJ_OBJECT) || (objType == SCI_OBJ_CLASS)) { // object or class?
			reg.offset += 8;	// magic offset (SCRIPT_OBJECT_MAGIC_OFFSET)
			int16 superclass = READ_SCI11ENDIAN_UINT16(scr->getBuf(reg.offset + 2));

			if (superclass >= 0) {
				int superclass_script = getClass(superclass).script;

				if (superclass_script == script_nr) {
					if (scr->getLockers())
						scr->decrementLockers();  // Decrease lockers if this is us ourselves
				} else {
					uninstantiateScript(superclass_script);
				}
				// Recurse to assure that the superclass lockers number gets decreased
			}

			reg.offset += SCRIPT_OBJECT_MAGIC_OFFSET;
		} // if object or class

		reg.offset -= 4; // Step back on header

	} while (objType != 0);
}

} // End of namespace Sci
