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
#include "sci/tools.h"

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
	case SEG_TYPE_STRING_FRAG:
		mem = new StringFrag();
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

#ifdef DEBUG_segMan
	printf("_buf = %p ", _buf);
#endif
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

Object *Script::allocateObject(uint16 offset) {
	return &_objects[offset];
}

Object *Script::getObject(uint16 offset) {
	if (_objects.contains(offset))
		return &_objects[offset];
	else
		return 0;
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

byte *SegmentObj::dereference(reg_t pointer, int *size) {
	error("Error: Trying to dereference pointer %04x:%04x to inappropriate segment",
		          PRINT_REG(pointer));
	return NULL;
}

bool Script::isValidOffset(uint16 offset) const {
	return offset < _bufSize;
}

byte *Script::dereference(reg_t pointer, int *size) {
	if (pointer.offset > _bufSize) {
		warning("Attempt to dereference invalid pointer %04x:%04x into script segment (script size=%d)\n",
				  PRINT_REG(pointer), (uint)_bufSize);
		return NULL;
	}
	if (size)
		*size = _bufSize - pointer.offset;
	return _buf + pointer.offset;
}

bool LocalVariables::isValidOffset(uint16 offset) const {
	return offset < _locals.size() * sizeof(reg_t);
}

byte *LocalVariables::dereference(reg_t pointer, int *size) {
	if (size)
		*size = _locals.size() * sizeof(reg_t);

	// FIXME: The following doesn't seem to be endian safe.
	// To fix this, we'd have to always treat the reg_t
	// values stored here as in the little endian format.
	byte *base = (byte *)&_locals[0];
	return base + pointer.offset;
}

bool DataStack::isValidOffset(uint16 offset) const {
	return offset < _capacity * sizeof(reg_t);
}

byte *DataStack::dereference(reg_t pointer, int *size) {
	if (size)
		*size = _capacity * sizeof(reg_t);

	byte *base = (byte *)_entries;
	return base + pointer.offset;
}

bool DynMem::isValidOffset(uint16 offset) const {
	return offset < _size;
}

byte *DynMem::dereference(reg_t pointer, int *size) {
	if (size)
		*size = _size;

	byte *base = (byte *)_buf;
	return base + pointer.offset;
}

bool SystemStrings::isValidOffset(uint16 offset) const {
	return offset < SYS_STRINGS_MAX && strings[offset].name;
}

byte *SystemStrings::dereference(reg_t pointer, int *size) {
	if (size)
		*size = strings[pointer.offset].max_size;
	if (pointer.offset < SYS_STRINGS_MAX && strings[pointer.offset].name)
		return (byte *)(strings[pointer.offset].value);

	// This occurs in KQ5CD when interacting with certain objects
	warning("Attempt to dereference invalid pointer %04x:%04x", PRINT_REG(pointer));
	return NULL;
}


//-------------------- script --------------------
reg_t Script::findCanonicAddress(SegManager *segMan, reg_t addr) {
	addr.offset = 0;
	return addr;
}

void Script::freeAtAddress(SegManager *segMan, reg_t addr) {
	/*
		debugC(2, kDebugLevelGC, "[GC] Freeing script %04x:%04x\n", PRINT_REG(addr));
		if (_localsSegment)
			debugC(2, kDebugLevelGC, "[GC] Freeing locals %04x:0000\n", _localsSegment);
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

			for (uint i = 0; i < obj->_variables.size(); i++)
				(*note)(param, obj->_variables[i]);
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
	for (uint i = 0; i < clone->_variables.size(); i++)
		(*note)(param, clone->_variables[i]);

	// Note that this also includes the 'base' object, which is part of the script and therefore also emits the locals.
	(*note)(param, clone->_pos);
	//debugC(2, kDebugLevelGC, "[GC] Reporting clone-pos %04x:%04x\n", PRINT_REG(clone->pos));
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


} // End of namespace Sci
