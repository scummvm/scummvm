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

#include "common/endian.h"

#include "sci/sci.h"
#include "sci/engine/features.h"
#include "sci/engine/script.h"	// for SCI_OBJ_EXPORTS and SCI_OBJ_SYNONYMS
#include "sci/engine/segment.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"

namespace Sci {

//#define GC_DEBUG // Debug garbage collection
//#define GC_DEBUG_VERBOSE // Debug garbage verbosely


SegmentObj *SegmentObj::createSegmentObj(SegmentType type) {
	SegmentObj *mem = 0;
	switch (type) {
	case SEG_TYPE_SCRIPT:
		mem = new Script();
		break;
	case SEG_TYPE_CLONES:
		mem = new CloneTable();
		break;
	case SEG_TYPE_LOCALS:
		mem = new LocalVariables();
		break;
	case SEG_TYPE_SYS_STRINGS:
		mem = new SystemStrings();
		break;
	case SEG_TYPE_STACK:
		mem = new DataStack();
		break;
	case SEG_TYPE_HUNK:
		mem = new HunkTable();
		break;
	case SEG_TYPE_LISTS:
		mem = new ListTable();
		break;
	case SEG_TYPE_NODES:
		mem = new NodeTable();
		break;
	case SEG_TYPE_DYNMEM:
		mem = new DynMem();
		break;
#ifdef ENABLE_SCI32
	case SEG_TYPE_ARRAY:
		mem = new ArrayTable();
		break;
	case SEG_TYPE_STRING:
		mem = new StringTable();
		break;
#endif
	default:
		error("Unknown SegmentObj type %d", type);
		break;
	}

	assert(mem);
	assert(mem->_type == type);
	return mem;
}

Script::Script() : SegmentObj(SEG_TYPE_SCRIPT) {
	_nr = 0;
	_buf = NULL;
	_bufSize = 0;
	_scriptSize = 0;
	_heapSize = 0;

	_synonyms = NULL;
	_heapStart = NULL;
	_exportTable = NULL;

	_localsOffset = 0;
	_localsSegment = 0;
	_localsBlock = NULL;
	_localsCount = 0;

	_markedAsDeleted = false;
}

Script::~Script() {
	freeScript();
}

void Script::freeScript() {
	free(_buf);
	_buf = NULL;
	_bufSize = 0;

	_objects.clear();
	_codeBlocks.clear();
}

void Script::init(int script_nr, ResourceManager *resMan) {
	Resource *script = resMan->findResource(ResourceId(kResourceTypeScript, script_nr), 0);

	_localsOffset = 0;
	_localsBlock = NULL;
	_localsCount = 0;

	_codeBlocks.clear();

	_markedAsDeleted = false;

	_nr = script_nr;
	_buf = 0;
	_heapStart = 0;

	_scriptSize = script->size;
	_bufSize = script->size;
	_heapSize = 0;

	_lockers = 1;

	if (getSciVersion() == SCI_VERSION_0_EARLY) {
		_bufSize += READ_LE_UINT16(script->data) * 2;
	} else if (getSciVersion() >= SCI_VERSION_1_1) {
		/**
		 * In SCI11, the heap was in a separate space from the script.
		 * We append it to the end of the script, and adjust addressing accordingly.
		 * However, since we address the heap with a 16-bit pointer, the combined
		 * size of the stack and the heap must be 64KB. So far this has worked
		 * for SCI11, SCI2 and SCI21 games. SCI3 games use a different script format,
		 * and theoretically they can exceed the 64KB boundary using relocation.
		 */
		Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, script_nr), 0);
		_bufSize += heap->size;
		_heapSize = heap->size;

		// Ensure that the start of the heap resource can be word-aligned.
		if (script->size & 2) {
			_bufSize++;
			_scriptSize++;
		}

		// As mentioned above, the script and the heap together should not exceed 64KB
		if (_bufSize > 65535)
			error("Script and heap sizes combined exceed 64K. This means a fundamental "
					"design bug was made regarding SCI1.1 and newer games.\nPlease "
					"report this error to the ScummVM team");
	}
}

void Script::load(ResourceManager *resMan) {
	Resource *script = resMan->findResource(ResourceId(kResourceTypeScript, _nr), 0);
	assert(script != 0);

	_buf = (byte *)malloc(_bufSize);
	assert(_buf);

	assert(_bufSize >= script->size);
	memcpy(_buf, script->data, script->size);

	if (getSciVersion() >= SCI_VERSION_1_1) {
		Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, _nr), 0);
		assert(heap != 0);

		_heapStart = _buf + _scriptSize;

		assert(_bufSize - _scriptSize <= heap->size);
		memcpy(_heapStart, heap->data, heap->size);
	}

	_codeBlocks.clear();

	_exportTable = 0;
	_numExports = 0;
	_synonyms = 0;
	_numSynonyms = 0;
	
	if (getSciVersion() >= SCI_VERSION_1_1) {
		if (READ_LE_UINT16(_buf + 1 + 5) > 0) {	// does the script have an export table?
			_exportTable = (const uint16 *)(_buf + 1 + 5 + 2);
			_numExports = READ_SCI11ENDIAN_UINT16(_exportTable - 1);
		}
		_localsOffset = _scriptSize + 4;
		_localsCount = READ_SCI11ENDIAN_UINT16(_buf + _localsOffset - 2);
	} else {
		_exportTable = (const uint16 *)findBlock(SCI_OBJ_EXPORTS);
		if (_exportTable) {
			_numExports = READ_SCI11ENDIAN_UINT16(_exportTable + 1);
			_exportTable += 3;	// skip header plus 2 bytes (_exportTable is a uint16 pointer)
		}
		_synonyms = findBlock(SCI_OBJ_SYNONYMS);
		if (_synonyms) {
			_numSynonyms = READ_SCI11ENDIAN_UINT16(_synonyms + 2) / 4;
			_synonyms += 4;	// skip header
		}
		const byte* localsBlock = findBlock(SCI_OBJ_LOCALVARS);
		if (localsBlock) {
			_localsOffset = localsBlock - _buf + 4;
			_localsCount = (READ_LE_UINT16(_buf + _localsOffset - 2) - 4) >> 1;	// half block size
		}
	}

	if (getSciVersion() > SCI_VERSION_0_EARLY) {
		// Does the script actually have locals? If not, set the locals offset to 0
		if (!_localsCount)
			_localsOffset = 0;

		if (_localsOffset + _localsCount * 2 + 1 >= (int)_bufSize) {
			warning("Locals extend beyond end of script: offset %04x, count %x vs size %x", _localsOffset, _localsCount, _bufSize);
			_localsCount = (_bufSize - _localsOffset) >> 1;
		}
	} else {
		// Old script block. There won't be a localvar block in this case.
		// Instead, the script starts with a 16 bit int specifying the
		// number of locals we need; these are then allocated and zeroed.
		_localsCount = READ_LE_UINT16(_buf);
		_localsOffset = -_localsCount * 2; // Make sure it's invalid
	}
}

Object *Script::allocateObject(uint16 offset) {
	return &_objects[offset];
}

Object *Script::getObject(uint16 offset) {
	if (_objects.contains(offset))
		return &_objects[offset];
	else
		return 0;
}

const Object *Script::getObject(uint16 offset) const {
	if (_objects.contains(offset))
		return &_objects[offset];
	else
		return 0;
}

Object *Script::scriptObjInit(reg_t obj_pos, bool fullObjectInit) {
	Object *obj;

	if (getSciVersion() < SCI_VERSION_1_1 && fullObjectInit)
		obj_pos.offset += 8;	// magic offset (SCRIPT_OBJECT_MAGIC_OFFSET)

	VERIFY(obj_pos.offset < _bufSize, "Attempt to initialize object beyond end of script\n");

	obj = allocateObject(obj_pos.offset);

	VERIFY(obj_pos.offset + kOffsetFunctionArea < (int)_bufSize, "Function area pointer stored beyond end of script\n");

	obj->init(_buf, obj_pos, fullObjectInit);

	return obj;
}

void Script::scriptObjRemove(reg_t obj_pos) {
	if (getSciVersion() < SCI_VERSION_1_1)
		obj_pos.offset += 8;

	_objects.erase(obj_pos.toUint16());
}

// This helper function is used by Script::relocateLocal and Object::relocate
static bool relocateBlock(Common::Array<reg_t> &block, int block_location, SegmentId segment, int location, size_t scriptSize) {
	int rel = location - block_location;

	if (rel < 0)
		return false;

	uint idx = rel >> 1;

	if (idx >= block.size())
		return false;

	if (rel & 1) {
		warning("Attempt to relocate odd variable #%d.5e (relative to %04x)\n", idx, block_location);
		return false;
	}
	block[idx].segment = segment; // Perform relocation
	if (getSciVersion() >= SCI_VERSION_1_1)
		block[idx].offset += scriptSize;

	return true;
}

bool Script::relocateLocal(SegmentId segment, int location) {
	if (_localsBlock)
		return relocateBlock(_localsBlock->_locals, _localsOffset, segment, location, _scriptSize);
	else
		return false;
}

void Script::scriptAddCodeBlock(reg_t location) {
	CodeBlock cb;
	cb.pos = location;
	cb.size = READ_SCI11ENDIAN_UINT16(_buf + location.offset - 2);
	_codeBlocks.push_back(cb);
}

void Script::relocate(reg_t block) {
	byte *heap = _buf;
	uint16 heapSize = (uint16)_bufSize;
	uint16 heapOffset = 0;

	if (getSciVersion() >= SCI_VERSION_1_1) {
		heap = _heapStart;
		heapSize = (uint16)_heapSize;
		heapOffset = _scriptSize;
	}

	VERIFY(block.offset < (uint16)heapSize && READ_SCI11ENDIAN_UINT16(heap + block.offset) * 2 + block.offset < (uint16)heapSize,
	       "Relocation block outside of script\n");

	int count = READ_SCI11ENDIAN_UINT16(heap + block.offset);
	int exportIndex = 0;

	for (int i = 0; i < count; i++) {
		int pos = READ_SCI11ENDIAN_UINT16(heap + block.offset + 2 + (exportIndex * 2)) + heapOffset;
		// This occurs in SCI01/SCI1 games where every usually one export
		// value is zero. It seems that in this situation, we should skip
		// the export and move to the next one, though the total count
		// of valid exports remains the same
		if (!pos) {
			exportIndex++;
			pos = READ_SCI11ENDIAN_UINT16(heap + block.offset + 2 + (exportIndex * 2)) + heapOffset;
			if (!pos)
				error("Script::relocate(): Consecutive zero exports found");
		}

		if (!relocateLocal(block.segment, pos)) {
			bool done = false;
			uint k;

			ObjMap::iterator it;
			const ObjMap::iterator end = _objects.end();
			for (it = _objects.begin(); !done && it != end; ++it) {
				if (it->_value.relocate(block.segment, pos, _scriptSize))
					done = true;
			}

			// Sanity check for SCI0-SCI1
			if (getSciVersion() < SCI_VERSION_1_1) {
				for (k = 0; !done && k < _codeBlocks.size(); k++) {
					if (pos >= _codeBlocks[k].pos.offset &&
							pos < _codeBlocks[k].pos.offset + _codeBlocks[k].size)
						done = true;
				}
			}

			if (!done) {
				debug("While processing relocation block %04x:%04x:\n", PRINT_REG(block));
				debug("Relocation failed for index %04x (%d/%d)\n", pos, exportIndex + 1, count);
				if (_localsBlock)
					debug("- locals: %d at %04x\n", _localsBlock->_locals.size(), _localsOffset);
				else
					debug("- No locals\n");
				for (it = _objects.begin(), k = 0; it != end; ++it, ++k)
					debug("- obj#%d at %04x w/ %d vars\n", k, it->_value.getPos().offset, it->_value.getVarCount());
				debug("Trying to continue anyway...\n");
			}
		}

		exportIndex++;
	}
}

void Script::incrementLockers() {
	_lockers++;
}

void Script::decrementLockers() {
	if (_lockers > 0)
		_lockers--;
}

int Script::getLockers() const {
	return _lockers;
}

void Script::setLockers(int lockers) {
	_lockers = lockers;
}

uint16 Script::validateExportFunc(int pubfunct) {
	bool exportsAreWide = (g_sci->_features->detectLofsType() == SCI_VERSION_1_MIDDLE);

	if (_numExports <= pubfunct) {
		warning("validateExportFunc(): pubfunct is invalid");
		return 0;
	}

	if (exportsAreWide)
		pubfunct *= 2;
	uint16 offset = READ_SCI11ENDIAN_UINT16(_exportTable + pubfunct);
	VERIFY(offset < _bufSize, "invalid export function pointer");

	return offset;
}

byte *Script::findBlock(int type) {
	byte *buf = _buf;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (oldScriptHeader)
		buf += 2;

	do {
		int seekerType = READ_LE_UINT16(buf);

		if (seekerType == 0)
			break;
		if (seekerType == type)
			return buf;

		int seekerSize = READ_LE_UINT16(buf + 2);
		assert(seekerSize > 0);
		buf += seekerSize;
	} while (1);

	return NULL;
}


// memory operations

void Script::mcpyInOut(int dst, const void *src, size_t n) {
	if (_buf) {
		assert(dst + n <= _bufSize);
		memcpy(_buf + dst, src, n);
	}
}

int16 Script::getHeap(uint16 offset) const {
	assert(offset + 1 < (int)_bufSize);
	return READ_SCI11ENDIAN_UINT16(_buf + offset);
//	return (_buf[offset] | (_buf[offset+1]) << 8);
}

SegmentRef SegmentObj::dereference(reg_t pointer) {
	error("Error: Trying to dereference pointer %04x:%04x to inappropriate segment",
		          PRINT_REG(pointer));
	return SegmentRef();
}

bool Script::isValidOffset(uint16 offset) const {
	return offset < _bufSize;
}

SegmentRef Script::dereference(reg_t pointer) {
	if (pointer.offset > _bufSize) {
		warning("Script::dereference(): Attempt to dereference invalid pointer %04x:%04x into script segment (script size=%d)",
				  PRINT_REG(pointer), (uint)_bufSize);
		return SegmentRef();
	}

	SegmentRef ret;
	ret.isRaw = true;
	ret.maxSize = _bufSize - pointer.offset;
	ret.raw = _buf + pointer.offset;
	return ret;
}

bool LocalVariables::isValidOffset(uint16 offset) const {
	return offset < _locals.size() * 2;
}

SegmentRef LocalVariables::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = false;	// reg_t based data!
	ret.maxSize = (_locals.size() - pointer.offset / 2) * 2;

	if (pointer.offset & 1) {
		ret.maxSize -= 1;
		ret.skipByte = true;
	}

	if (ret.maxSize > 0) {
		ret.reg = &_locals[pointer.offset / 2];
	} else {
		warning("LocalVariables::dereference: Offset at end or out of bounds %04x:%04x", PRINT_REG(pointer));
		ret.reg = 0;
	}
	return ret;
}

bool DataStack::isValidOffset(uint16 offset) const {
	return offset < _capacity * 2;
}

SegmentRef DataStack::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = false;	// reg_t based data!
	ret.maxSize = (_capacity - pointer.offset / 2) * 2;

	if (pointer.offset & 1) {
		ret.maxSize -= 1;
		ret.skipByte = true;
	}

	ret.reg = &_entries[pointer.offset / 2];
	return ret;
}

bool DynMem::isValidOffset(uint16 offset) const {
	return offset < _size;
}

SegmentRef DynMem::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = true;
	ret.maxSize = _size - pointer.offset;
	ret.raw = _buf + pointer.offset;
	return ret;
}

bool SystemStrings::isValidOffset(uint16 offset) const {
	return offset < SYS_STRINGS_MAX && !_strings[offset]._name.empty();
}

SegmentRef SystemStrings::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = true;
	ret.maxSize = _strings[pointer.offset]._maxSize;
	if (isValidOffset(pointer.offset))
		ret.raw = (byte *)(_strings[pointer.offset]._value);
	else {
		// This occurs in KQ5CD when interacting with certain objects
		warning("SystemStrings::dereference(): Attempt to dereference invalid pointer %04x:%04x", PRINT_REG(pointer));
	}

	return ret;
}


//-------------------- script --------------------
reg_t Script::findCanonicAddress(SegManager *segMan, reg_t addr) const {
	addr.offset = 0;
	return addr;
}

void Script::freeAtAddress(SegManager *segMan, reg_t addr) {
	/*
		debugC(2, kDebugLevelGC, "[GC] Freeing script %04x:%04x", PRINT_REG(addr));
		if (_localsSegment)
			debugC(2, kDebugLevelGC, "[GC] Freeing locals %04x:0000", _localsSegment);
	*/

	if (_markedAsDeleted)
		segMan->deallocateScript(_nr);
}

void Script::listAllDeallocatable(SegmentId segId, void *param, NoteCallback note) const {
	(*note)(param, make_reg(segId, 0));
}

void Script::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) const {
	if (addr.offset <= _bufSize && addr.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET && RAW_IS_OBJECT(_buf + addr.offset)) {
		const Object *obj = getObject(addr.offset);
		if (obj) {
			// Note all local variables, if we have a local variable environment
			if (_localsSegment)
				(*note)(param, make_reg(_localsSegment, 0));

			for (uint i = 0; i < obj->getVarCount(); i++)
				(*note)(param, obj->getVariable(i));
		} else {
			warning("Request for outgoing script-object reference at %04x:%04x failed", PRINT_REG(addr));
		}
	} else {
		/*		warning("Unexpected request for outgoing script-object references at %04x:%04x", PRINT_REG(addr));*/
		/* Happens e.g. when we're looking into strings */
	}
}


//-------------------- clones --------------------

void CloneTable::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) const {
//	assert(addr.segment == _segId);

	if (!isValidEntry(addr.offset)) {
		error("Unexpected request for outgoing references from clone at %04x:%04x", PRINT_REG(addr));
	}

	const Clone *clone = &(_table[addr.offset]);

	// Emit all member variables (including references to the 'super' delegate)
	for (uint i = 0; i < clone->getVarCount(); i++)
		(*note)(param, clone->getVariable(i));

	// Note that this also includes the 'base' object, which is part of the script and therefore also emits the locals.
	(*note)(param, clone->getPos());
	//debugC(2, kDebugLevelGC, "[GC] Reporting clone-pos %04x:%04x", PRINT_REG(clone->pos));
}

void CloneTable::freeAtAddress(SegManager *segMan, reg_t addr) {
#ifdef GC_DEBUG
	Object *victim_obj;

//	assert(addr.segment == _segId);

	victim_obj = &(_table[addr.offset]);

	if (!(victim_obj->_flags & OBJECT_FLAG_FREED))
		warning("[GC] Clone %04x:%04x not reachable and not freed (freeing now)", PRINT_REG(addr));
#ifdef GC_DEBUG_VERBOSE
	else
		warning("[GC-DEBUG] Clone %04x:%04x: Freeing", PRINT_REG(addr));
#endif
#endif
	/*
		warning("[GC] Clone %04x:%04x: Freeing", PRINT_REG(addr));
		warning("[GC] Clone had pos %04x:%04x", PRINT_REG(victim_obj->pos));
	*/
	freeEntry(addr.offset);
}


//-------------------- locals --------------------
reg_t LocalVariables::findCanonicAddress(SegManager *segMan, reg_t addr) const {
	// Reference the owning script
	SegmentId owner_seg = segMan->getScriptSegment(script_id);

	assert(owner_seg > 0);

	return make_reg(owner_seg, 0);
}

void LocalVariables::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) const {
//	assert(addr.segment == _segId);

	for (uint i = 0; i < _locals.size(); i++)
		(*note)(param, _locals[i]);
}


//-------------------- stack --------------------
reg_t DataStack::findCanonicAddress(SegManager *segMan, reg_t addr) const {
	addr.offset = 0;
	return addr;
}

void DataStack::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) const {
	fprintf(stderr, "Emitting %d stack entries\n", _capacity);
	for (int i = 0; i < _capacity; i++)
		(*note)(param, _entries[i]);
	fprintf(stderr, "DONE");
}


//-------------------- lists --------------------
void ListTable::freeAtAddress(SegManager *segMan, reg_t sub_addr) {
	freeEntry(sub_addr.offset);
}

void ListTable::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) const {
	if (!isValidEntry(addr.offset)) {
		warning("Invalid list referenced for outgoing references: %04x:%04x", PRINT_REG(addr));
		return;
	}

	const List *list = &(_table[addr.offset]);

	note(param, list->first);
	note(param, list->last);
	// We could probably get away with just one of them, but
	// let's be conservative here.
}


//-------------------- nodes --------------------
void NodeTable::freeAtAddress(SegManager *segMan, reg_t sub_addr) {
	freeEntry(sub_addr.offset);
}

void NodeTable::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) const {
	if (!isValidEntry(addr.offset)) {
		warning("Invalid node referenced for outgoing references: %04x:%04x", PRINT_REG(addr));
		return;
	}
	const Node *node = &(_table[addr.offset]);

	// We need all four here. Can't just stick with 'pred' OR 'succ' because node operations allow us
	// to walk around from any given node
	note(param, node->pred);
	note(param, node->succ);
	note(param, node->key);
	note(param, node->value);
}


//-------------------- hunk --------------------

//-------------------- object ----------------------------

void Object::init(byte *buf, reg_t obj_pos, bool initVariables) {
	byte *data = buf + obj_pos.offset;
	_baseObj = data;
	_pos = obj_pos;

	if (getSciVersion() < SCI_VERSION_1_1) {
		_variables.resize(READ_LE_UINT16(data + kOffsetSelectorCounter));
		_baseVars = (const uint16 *)(_baseObj + _variables.size() * 2);
		_baseMethod = (const uint16 *)(data + READ_LE_UINT16(data + kOffsetFunctionArea));
		_methodCount = READ_LE_UINT16(_baseMethod - 1);
	} else {
		_variables.resize(READ_SCI11ENDIAN_UINT16(data + 2));
		_baseVars = (const uint16 *)(buf + READ_SCI11ENDIAN_UINT16(data + 4));
		_baseMethod = (const uint16 *)(buf + READ_SCI11ENDIAN_UINT16(data + 6));
		_methodCount = READ_SCI11ENDIAN_UINT16(_baseMethod);
	}

	if (initVariables) {
		for (uint i = 0; i < _variables.size(); i++)
			_variables[i] = make_reg(0, READ_SCI11ENDIAN_UINT16(data + (i * 2)));
	}
}

const Object *Object::getClass(SegManager *segMan) const {
	return isClass() ? this : segMan->getObject(getSuperClassSelector());
}

int Object::locateVarSelector(SegManager *segMan, Selector slc) const {
	const byte *buf;
	uint varnum;

	if (getSciVersion() < SCI_VERSION_1_1) {
		varnum = getVarCount();
		int selector_name_offset = varnum * 2 + kOffsetSelectorSegment;
		buf = _baseObj + selector_name_offset;
	} else {
		const Object *obj = getClass(segMan);
		varnum = obj->getVariable(1).toUint16();
		buf = (byte *)obj->_baseVars;
	}

	for (uint i = 0; i < varnum; i++)
		if (READ_SCI11ENDIAN_UINT16(buf + (i << 1)) == slc) // Found it?
			return i; // report success

	return -1; // Failed
}

bool Object::relocate(SegmentId segment, int location, size_t scriptSize) {
	return relocateBlock(_variables, getPos().offset, segment, location, scriptSize);
}

int Object::propertyOffsetToId(SegManager *segMan, int propertyOffset) const {
	int selectors = getVarCount();

	if (propertyOffset < 0 || (propertyOffset >> 1) >= selectors) {
		warning("Applied propertyOffsetToId to invalid property offset %x (property #%d not in [0..%d])",
		          propertyOffset, propertyOffset >> 1, selectors - 1);
		return -1;
	}

	if (getSciVersion() < SCI_VERSION_1_1) {
		const byte *selectoroffset = ((const byte *)(_baseObj)) + kOffsetSelectorSegment + selectors * 2;
		return READ_SCI11ENDIAN_UINT16(selectoroffset + propertyOffset);
	} else {
		const Object *obj = this;
		if (!isClass())
			obj = segMan->getObject(getSuperClassSelector());

		return READ_SCI11ENDIAN_UINT16((const byte *)obj->_baseVars + propertyOffset);
	}
}

void Object::initSpecies(SegManager *segMan, reg_t addr) {
	uint16 speciesOffset = getSpeciesSelector().offset;

	if (speciesOffset == 0xffff)		// -1
		setSpeciesSelector(NULL_REG);	// no species
	else
		setSpeciesSelector(segMan->getClassAddress(speciesOffset, SCRIPT_GET_LOCK, addr));
}

void Object::initSuperClass(SegManager *segMan, reg_t addr) {
	uint16 superClassOffset = getSuperClassSelector().offset;

	if (superClassOffset == 0xffff)			// -1
		setSuperClassSelector(NULL_REG);	// no superclass
	else
		setSuperClassSelector(segMan->getClassAddress(superClassOffset, SCRIPT_GET_LOCK, addr));
}

bool Object::initBaseObject(SegManager *segMan, reg_t addr, bool doInitSuperClass) {
	const Object *baseObj = segMan->getObject(getSpeciesSelector());

	if (baseObj) {
		_variables.resize(baseObj->getVarCount());
		// Copy base from species class, as we need its selector IDs
		_baseObj = baseObj->_baseObj;
		if (doInitSuperClass)
			initSuperClass(segMan, addr);
		return true;
	}

	return false;
}

//-------------------- dynamic memory --------------------

reg_t DynMem::findCanonicAddress(SegManager *segMan, reg_t addr) const {
	addr.offset = 0;
	return addr;
}

void DynMem::listAllDeallocatable(SegmentId segId, void *param, NoteCallback note) const {
	(*note)(param, make_reg(segId, 0));
}

#ifdef ENABLE_SCI32

SegmentRef ArrayTable::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = false;
	ret.maxSize = _table[pointer.offset].getSize() * 2;
	ret.reg = _table[pointer.offset].getRawData();
	return ret;
}

void ArrayTable::freeAtAddress(SegManager *segMan, reg_t sub_addr) { 
	_table[sub_addr.offset].destroy();
	freeEntry(sub_addr.offset);
}

void ArrayTable::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) const {
	if (!isValidEntry(addr.offset)) {
		warning("Invalid array referenced for outgoing references: %04x:%04x", PRINT_REG(addr));
		return;
	}

	const SciArray<reg_t> *array = &(_table[addr.offset]);

	for (uint32 i = 0; i < array->getSize(); i++) {
		reg_t value = array->getValue(i);
		if (value.segment != 0)
			note(param, value);
	}
}

Common::String SciString::toString() const {
	if (_type != 3)
		error("SciString::toString(): Array is not a string");

	Common::String string;
	for (uint32 i = 0; i < _size && _data[i] != 0; i++)
		string += _data[i];

	return string;
}

void SciString::fromString(const Common::String &string) {
	if (_type != 3)
		error("SciString::fromString(): Array is not a string");

	if (string.size() > _size)
		setSize(string.size());

	for (uint32 i = 0; i < string.size(); i++)
		_data[i] = string[i];
}

SegmentRef StringTable::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = true;
	ret.maxSize = _table[pointer.offset].getSize();
	ret.raw = (byte*)_table[pointer.offset].getRawData();
	return ret;
}

void StringTable::freeAtAddress(SegManager *segMan, reg_t sub_addr) { 
	_table[sub_addr.offset].destroy();
	freeEntry(sub_addr.offset);
}

#endif

} // End of namespace Sci
