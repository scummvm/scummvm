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

	_relocated = false;
	_markedAsDeleted = 0;
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

bool Script::init(int script_nr, ResourceManager *resMan) {
	setScriptSize(script_nr, resMan);

	_buf = (byte *)malloc(_bufSize);

	if (!_buf) {
		freeScript();
		warning("Not enough memory space for script size");
		_bufSize = 0;
		return false;
	}

	_localsOffset = 0;
	_localsBlock = NULL;

	_codeBlocks.clear();

	_relocated = false;
	_markedAsDeleted = false;

	_nr = script_nr;

	if (getSciVersion() >= SCI_VERSION_1_1)
		_heapStart = _buf + _scriptSize;
	else
		_heapStart = _buf;

	return true;
}

void Script::setScriptSize(int script_nr, ResourceManager *resMan) {
	Resource *script = resMan->findResource(ResourceId(kResourceTypeScript, script_nr), 0);
	Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, script_nr), 0);
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	_scriptSize = script->size;
	_heapSize = 0; // Set later

	if (!script || (getSciVersion() >= SCI_VERSION_1_1 && !heap)) {
		error("SegManager::setScriptSize: failed to load %s", !script ? "script" : "heap");
	}
	if (oldScriptHeader) {
		_bufSize = script->size + READ_LE_UINT16(script->data) * 2;
		//locals_size = READ_LE_UINT16(script->data) * 2;
	} else if (getSciVersion() < SCI_VERSION_1_1) {
		_bufSize = script->size;
	} else {
		_bufSize = script->size + heap->size;
		_heapSize = heap->size;

		// Ensure that the start of the heap resource can be word-aligned.
		if (script->size & 2) {
			_bufSize++;
			_scriptSize++;
		}

		if (_bufSize > 65535) {
			error("Script and heap sizes combined exceed 64K."
			          "This means a fundamental design bug was made in SCI\n"
			          "regarding SCI1.1 games.\nPlease report this so it can be"
			          "fixed in the next major version");
			return;
		}
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

Object *Script::scriptObjInit(reg_t obj_pos) {
	Object *obj;

	if (getSciVersion() < SCI_VERSION_1_1)
		obj_pos.offset += 8;	// magic offset (SCRIPT_OBJECT_MAGIC_OFFSET)

	VERIFY(obj_pos.offset < _bufSize, "Attempt to initialize object beyond end of script\n");

	obj = allocateObject(obj_pos.offset);

	VERIFY(obj_pos.offset + SCRIPT_FUNCTAREAPTR_OFFSET < (int)_bufSize, "Function area pointer stored beyond end of script\n");

	obj->init(_buf, obj_pos);

	return obj;
}

void Script::scriptObjRemove(reg_t obj_pos) {
	if (getSciVersion() < SCI_VERSION_1_1)
		obj_pos.offset += 8;

	_objects.erase(obj_pos.toUint16());
}

int Script::relocateBlock(Common::Array<reg_t> &block, int block_location, SegmentId segment, int location) {
	int rel = location - block_location;

	if (rel < 0)
		return 0;

	uint idx = rel >> 1;

	if (idx >= block.size())
		return 0;

	if (rel & 1) {
		warning("Attempt to relocate odd variable #%d.5e (relative to %04x)\n", idx, block_location);
		return 0;
	}
	block[idx].segment = segment; // Perform relocation
	if (getSciVersion() >= SCI_VERSION_1_1)
		block[idx].offset += _scriptSize;

	return 1;
}

int Script::relocateLocal(SegmentId segment, int location) {
	if (_localsBlock)
		return relocateBlock(_localsBlock->_locals, _localsOffset, segment, location);
	else
		return 0; // No hands, no cookies
}

int Script::relocateObject(Object &obj, SegmentId segment, int location) {
	return relocateBlock(obj._variables, obj.getPos().offset, segment, location);
}

void Script::scriptAddCodeBlock(reg_t location) {
	CodeBlock cb;
	cb.pos = location;
	cb.size = READ_LE_UINT16(_buf + location.offset - 2);
	_codeBlocks.push_back(cb);
}

void Script::scriptRelocate(reg_t block) {
	VERIFY(block.offset < (uint16)_bufSize && READ_LE_UINT16(_buf + block.offset) * 2 + block.offset < (uint16)_bufSize,
	       "Relocation block outside of script\n");

	int count = READ_LE_UINT16(_buf + block.offset);

	for (int i = 0; i <= count; i++) {
		int pos = READ_LE_UINT16(_buf + block.offset + 2 + (i * 2));
		if (!pos)
			continue; // FIXME: A hack pending investigation

		if (!relocateLocal(block.segment, pos)) {
			bool done = false;
			uint k;

			ObjMap::iterator it;
			const ObjMap::iterator end = _objects.end();
			for (it = _objects.begin(); !done && it != end; ++it) {
				if (relocateObject(it->_value, block.segment, pos))
					done = true;
			}

			for (k = 0; !done && k < _codeBlocks.size(); k++) {
				if (pos >= _codeBlocks[k].pos.offset &&
				        pos < _codeBlocks[k].pos.offset + _codeBlocks[k].size)
					done = true;
			}

			if (!done) {
				printf("While processing relocation block %04x:%04x:\n", PRINT_REG(block));
				printf("Relocation failed for index %04x (%d/%d)\n", pos, i + 1, count);
				if (_localsBlock)
					printf("- locals: %d at %04x\n", _localsBlock->_locals.size(), _localsOffset);
				else
					printf("- No locals\n");
				for (it = _objects.begin(), k = 0; it != end; ++it, ++k)
					printf("- obj#%d at %04x w/ %d vars\n", k, it->_value.getPos().offset, it->_value.getVarCount());
				// SQ3 script 71 has broken relocation entries.
				printf("Trying to continue anyway...\n");
			}
		}
	}
}

void Script::heapRelocate(reg_t block) {
	VERIFY(block.offset < (uint16)_heapSize && READ_LE_UINT16(_heapStart + block.offset) * 2 + block.offset < (uint16)_bufSize,
	       "Relocation block outside of script\n");

	if (_relocated)
		return;
	_relocated = true;
	int count = READ_LE_UINT16(_heapStart + block.offset);

	for (int i = 0; i < count; i++) {
		int pos = READ_LE_UINT16(_heapStart + block.offset + 2 + (i * 2)) + _scriptSize;

		if (!relocateLocal(block.segment, pos)) {
			bool done = false;
			uint k;

			ObjMap::iterator it;
			const ObjMap::iterator end = _objects.end();
			for (it = _objects.begin(); !done && it != end; ++it) {
				if (relocateObject(it->_value, block.segment, pos))
					done = true;
			}

			if (!done) {
				printf("While processing relocation block %04x:%04x:\n", PRINT_REG(block));
				printf("Relocation failed for index %04x (%d/%d)\n", pos, i + 1, count);
				if (_localsBlock)
					printf("- locals: %d at %04x\n", _localsBlock->_locals.size(), _localsOffset);
				else
					printf("- No locals\n");
				for (it = _objects.begin(), k = 0; it != end; ++it, ++k)
					printf("- obj#%d at %04x w/ %d vars\n", k, it->_value.getPos().offset, it->_value.getVarCount());
				error("Breakpoint in %s, line %d", __FILE__, __LINE__);
			}
		}
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

void Script::setExportTableOffset(int offset) {
	if (offset) {
		_exportTable = (uint16 *)(_buf + offset + 2);
		_numExports = READ_LE_UINT16((byte *)(_exportTable - 1));
	} else {
		_exportTable = NULL;
		_numExports = 0;
	}
}

// TODO: This method should be Script method. The only reason
// that it isn't is that it uses _exportsAreWide, which is true if
// detectLofsType() == SCI_VERSION_1_MIDDLE
// Maybe _exportsAreWide should become a Script member var, e.g. set
// by setExportTableOffset?
uint16 SegManager::validateExportFunc(int pubfunct, SegmentId seg) {
	Script *scr = getScript(seg);
	if (scr->_numExports <= pubfunct) {
		warning("validateExportFunc(): pubfunct is invalid");
		return 0;
	}

	if (_exportsAreWide)
		pubfunct *= 2;
	uint16 offset = READ_LE_UINT16((byte *)(scr->_exportTable + pubfunct));
	VERIFY(offset < scr->_bufSize, "invalid export function pointer");

	return offset;
}

void Script::setSynonymsOffset(int offset) {
	_synonyms = _buf + offset;
}

byte *Script::getSynonyms() const {
	return _synonyms;
}

void Script::setSynonymsNr(int n) {
	_numSynonyms = n;
}

int Script::getSynonymsNr() const {
	return _numSynonyms;
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
	return READ_LE_UINT16(_buf + offset);
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
reg_t Script::findCanonicAddress(SegManager *segMan, reg_t addr) {
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

void Script::listAllDeallocatable(SegmentId segId, void *param, NoteCallback note) {
	(*note)(param, make_reg(segId, 0));
}

void Script::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) {
	if (addr.offset <= _bufSize && addr.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET && RAW_IS_OBJECT(_buf + addr.offset)) {
		Object *obj = getObject(addr.offset);
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

void CloneTable::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) {
	Clone *clone;

//	assert(addr.segment == _segId);

	if (!isValidEntry(addr.offset)) {
		error("Unexpected request for outgoing references from clone at %04x:%04x", PRINT_REG(addr));
	}

	clone = &(_table[addr.offset]);

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
reg_t LocalVariables::findCanonicAddress(SegManager *segMan, reg_t addr) {
	// Reference the owning script
	SegmentId owner_seg = segMan->getScriptSegment(script_id);

	assert(owner_seg > 0);

	return make_reg(owner_seg, 0);
}

void LocalVariables::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) {
//	assert(addr.segment == _segId);

	for (uint i = 0; i < _locals.size(); i++)
		(*note)(param, _locals[i]);
}


//-------------------- stack --------------------
reg_t DataStack::findCanonicAddress(SegManager *segMan, reg_t addr) {
	addr.offset = 0;
	return addr;
}

void DataStack::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) {
	fprintf(stderr, "Emitting %d stack entries\n", _capacity);
	for (int i = 0; i < _capacity; i++)
		(*note)(param, _entries[i]);
	fprintf(stderr, "DONE");
}


//-------------------- lists --------------------
void ListTable::freeAtAddress(SegManager *segMan, reg_t sub_addr) {
	freeEntry(sub_addr.offset);
}

void ListTable::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) {
	if (!isValidEntry(addr.offset)) {
		warning("Invalid list referenced for outgoing references: %04x:%04x", PRINT_REG(addr));
		return;
	}

	List *list = &(_table[addr.offset]);

	note(param, list->first);
	note(param, list->last);
	// We could probably get away with just one of them, but
	// let's be conservative here.
}


//-------------------- nodes --------------------
void NodeTable::freeAtAddress(SegManager *segMan, reg_t sub_addr) {
	freeEntry(sub_addr.offset);
}

void NodeTable::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note) {
	if (!isValidEntry(addr.offset)) {
		warning("Invalid node referenced for outgoing references: %04x:%04x", PRINT_REG(addr));
		return;
	}
	Node *node = &(_table[addr.offset]);

	// We need all four here. Can't just stick with 'pred' OR 'succ' because node operations allow us
	// to walk around from any given node
	note(param, node->pred);
	note(param, node->succ);
	note(param, node->key);
	note(param, node->value);
}


//-------------------- hunk --------------------

//-------------------- dynamic memory --------------------

reg_t DynMem::findCanonicAddress(SegManager *segMan, reg_t addr) {
	addr.offset = 0;
	return addr;
}

void DynMem::listAllDeallocatable(SegmentId segId, void *param, NoteCallback note) {
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

Common::String SciString::toString() {
	if (_type != 3)
		error("SciString::toString(): Array is not a string");

	Common::String string;
	for (uint32 i = 0; i < _size && _data[i] != 0; i++)
		string += _data[i];

	return string;
}

void SciString::fromString(Common::String string) {
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

#endif

} // End of namespace Sci
