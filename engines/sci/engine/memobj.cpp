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
#include "sci/engine/memobj.h"
#include "sci/engine/intmap.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"
#include "sci/tools.h"

namespace Sci {

MemObject *MemObject::createMemObject(MemObjectType type) {
	MemObject *mem = 0;
	switch (type) {
	case MEM_OBJ_SCRIPT:
		mem = new Script();
		break;
	case MEM_OBJ_CLONES:
		mem = new CloneTable();
		break;
	case MEM_OBJ_LOCALS:
		mem = new LocalVariables();
		break;
	case MEM_OBJ_SYS_STRINGS:
		mem = new SystemStrings();
		break;
	case MEM_OBJ_STACK:
		mem = new DataStack();
		break;
	case MEM_OBJ_HUNK:
		mem = new HunkTable();
		break;
	case MEM_OBJ_STRING_FRAG:
		mem = new StringFrag();
		break;
	case MEM_OBJ_LISTS:
		mem = new ListTable();
		break;
	case MEM_OBJ_NODES:
		mem = new NodeTable();
		break;
	case MEM_OBJ_DYNMEM:
		mem = new DynMem();
		break;
	default:
		error("Unknown MemObject type %d", type);
		break;
	}

	assert(mem);
	mem->_type = type;
	return mem;
}

void Script::freeScript() {
	free(buf);
	buf = NULL;
	buf_size = 0;

	_objects.clear();

	delete obj_indices;
	obj_indices = 0;
	_codeBlocks.clear();
}

void Script::init() {
	locals_offset = 0;
	locals_block = NULL;

	_codeBlocks.clear();

	_markedAsDeleted = false;
	relocated = 0;

	obj_indices = new IntMapper();
}

Object *Script::allocateObject(uint16 offset) {
	int idx = obj_indices->checkKey(offset, true);
	if ((uint)idx == _objects.size())
		_objects.push_back(Object());

	return &_objects[idx];
}

Object *Script::getObject(uint16 offset) {
	int idx = obj_indices->checkKey(offset, false);
	if (idx >= 0 && (uint)idx < _objects.size())
		return &_objects[idx];
	else
		return 0;
}

void Script::incrementLockers() {
	lockers++;
}

void Script::decrementLockers() {
	if (lockers > 0)
		lockers--;
}

int Script::getLockers() const {
	return lockers;
}

void Script::setLockers(int lockers_) {
	lockers = lockers_;
}

void Script::setExportTableOffset(int offset) {
	if (offset) {
		export_table = (uint16 *)(buf + offset + 2);
		exports_nr = READ_LE_UINT16((byte *)(export_table - 1));
	} else {
		export_table = NULL;
		exports_nr = 0;
	}
}

void Script::setSynonymsOffset(int offset) {
	synonyms = buf + offset;
}

byte *Script::getSynonyms() const {
	return synonyms;
}

void Script::setSynonymsNr(int n) {
	synonyms_nr = n;
}

int Script::getSynonymsNr() const {
	return synonyms_nr;
}

// memory operations

void Script::mcpyInOut(int dst, const void *src, size_t n) {
	if (buf) {
		assert(dst + n <= buf_size);
		memcpy(buf + dst, src, n);
	}
}

int16 Script::getHeap(uint16 offset) const {
	assert(offset + 1 < (int)buf_size);
	return READ_LE_UINT16(buf + offset);
//	return (buf[offset] | (buf[offset+1]) << 8);
}

byte *MemObject::dereference(reg_t pointer, int *size) {
	error("Error: Trying to dereference pointer %04x:%04x to inappropriate segment",
		          PRINT_REG(pointer));
	return NULL;
}

bool Script::isValidOffset(uint16 offset) const {
	return offset < buf_size;
}

byte *Script::dereference(reg_t pointer, int *size) {
	if (pointer.offset > buf_size) {
		warning("Attempt to dereference invalid pointer %04x:%04x into script segment (script size=%d)\n",
				  PRINT_REG(pointer), (uint)buf_size);
		return NULL;
	}
	if (size)
		*size = buf_size - pointer.offset;
	return buf + pointer.offset;
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
	return offset < nr * sizeof(reg_t);
}

byte *DataStack::dereference(reg_t pointer, int *size) {
	if (size)
		*size = nr * sizeof(reg_t);

	byte *base = (byte *)entries;
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
		if (locals_segment)
			debugC(2, kDebugLevelGC, "[GC] Freeing locals %04x:0000\n", locals_segment);
	*/

	if (_markedAsDeleted)
		segMan->deallocateScript(nr);
}

void Script::listAllDeallocatable(SegmentId segId, void *param, NoteCallback note) {
	(*note)(param, make_reg(segId, 0));
}

void Script::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note, SciVersion version) {
	Script *script = this;

	if (addr.offset <= script->buf_size && addr.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET && RAW_IS_OBJECT(script->buf + addr.offset)) {
		Object *obj = getObject(addr.offset);
		if (obj) {
			// Note all local variables, if we have a local variable environment
			if (script->locals_segment)
				(*note)(param, make_reg(script->locals_segment, 0));

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

void CloneTable::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note, SciVersion version) {
	CloneTable *clone_table = this;
	Clone *clone;

//	assert(addr.segment == _segId);

	if (!clone_table->isValidEntry(addr.offset)) {
		warning("Unexpected request for outgoing references from clone at %04x:%04x", PRINT_REG(addr));
//		BREAKPOINT();
		return;
	}

	clone = &(clone_table->_table[addr.offset]);

	// Emit all member variables (including references to the 'super' delegate)
	for (uint i = 0; i < clone->_variables.size(); i++)
		(*note)(param, clone->_variables[i]);

	// Note that this also includes the 'base' object, which is part of the script and therefore also emits the locals.
	(*note)(param, clone->pos);
	//debugC(2, kDebugLevelGC, "[GC] Reporting clone-pos %04x:%04x\n", PRINT_REG(clone->pos));
}

void CloneTable::freeAtAddress(SegManager *segMan, reg_t addr) {
	CloneTable *clone_table = this;
	Object *victim_obj;

//	assert(addr.segment == _segId);

	victim_obj = &(clone_table->_table[addr.offset]);

#ifdef GC_DEBUG
	if (!(victim_obj->flags & OBJECT_FLAG_FREED))
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
	clone_table->freeEntry(addr.offset);
}


//-------------------- locals --------------------
reg_t LocalVariables::findCanonicAddress(SegManager *segMan, reg_t addr) {
	// Reference the owning script
	SegmentId owner_seg = segMan->getScriptSegment(script_id);

	assert(owner_seg > 0);

	return make_reg(owner_seg, 0);
}

void LocalVariables::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note, SciVersion version) {
//	assert(addr.segment == _segId);

	for (uint i = 0; i < _locals.size(); i++)
		(*note)(param, _locals[i]);
}


//-------------------- stack --------------------
reg_t DataStack::findCanonicAddress(SegManager *segMan, reg_t addr) {
	addr.offset = 0;
	return addr;
}

void DataStack::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note, SciVersion version) {
	fprintf(stderr, "Emitting %d stack entries\n", nr);
	for (int i = 0; i < nr; i++)
		(*note)(param, entries[i]);
	fprintf(stderr, "DONE");
}


//-------------------- lists --------------------
void ListTable::freeAtAddress(SegManager *segMan, reg_t sub_addr) {
	freeEntry(sub_addr.offset);
}

void ListTable::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note, SciVersion version) {
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

void NodeTable::listAllOutgoingReferences(reg_t addr, void *param, NoteCallback note, SciVersion version) {
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
