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

#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"
#include "sci/engine/intmap.h"

namespace Sci {


/**
 * Verify the the given condition is true, output the message if condition is false, and exit.
 * @param cond	condition to be verified
 * @param msg	the message to be printed if condition fails
 */
#define VERIFY( cond, msg ) if (!(cond)) {\
		error("%s, line, %d, %s", __FILE__, __LINE__, msg); \
	}


#define DEFAULT_SCRIPTS 32
#define DEFAULT_OBJECTS 8	    // default # of objects per script
#define DEFAULT_OBJECTS_INCREMENT 4 // Number of additional objects to instantiate if we're running out of them

//#define GC_DEBUG // Debug garbage collection
//#define GC_DEBUG_VERBOSE // Debug garbage verbosely

#undef DEBUG_SEG_MANAGER // Define to turn on debugging

#define INVALID_SCRIPT_ID -1

int SegManager::findFreeId(int *id) {
	bool was_added = false;
	int retval = 0;

	while (!was_added) {
		retval = id_seg_map->checkKey(reserved_id, true, &was_added);
		*id = reserved_id--;
		if (reserved_id < -1000000)
			reserved_id = -10;
		// Make sure we don't underflow
	}

	return retval;
}

MemObject *SegManager::allocNonscriptSegment(MemObjectType type, SegmentId *segid) {
	// Allocates a non-script segment
	int id;

	*segid = findFreeId(&id);
	return memObjAllocate(*segid, id, type);
}

SegManager::SegManager(bool sci1_1) {
	// Initialise memory count
	mem_allocated = 0;

	id_seg_map = new IntMapper();
	reserved_id = INVALID_SCRIPT_ID;
	id_seg_map->checkKey(reserved_id, true);	// reserve 0 for seg_id
	reserved_id--; // reserved_id runs in the reversed direction to make sure no one will use it.

	_heap.resize(DEFAULT_SCRIPTS);

	Clones_seg_id = 0;
	Lists_seg_id = 0;
	Nodes_seg_id = 0;
	Hunks_seg_id = 0;

	exports_wide = 0;
	isSci1_1 = sci1_1;

	// gc initialisation
	gc_mark_bits = 0;
}

// Destroy the object, free the memorys if allocated before
SegManager::~SegManager() {
	// Free memory
	for (uint i = 0; i < _heap.size(); i++) {
		if (_heap[i])
			deallocate(i, false);
	}

	delete id_seg_map;
}

// allocate a memory for script from heap
// Parameters: (EngineState *) s: The state to operate on
//             (int) script_nr: The script number to load
// Returns   : 0 - allocation failure
//             1 - allocated successfully
//             seg_id - allocated segment id
Script *SegManager::allocateScript(EngineState *s, int script_nr, int* seg_id) {
	int seg;
	bool was_added;
	MemObject* mem;

	seg = id_seg_map->checkKey(script_nr, true, &was_added);
	if (!was_added) {
		*seg_id = seg;
		return (Script *)_heap[*seg_id];
	}

	// allocate the MemObject
	mem = memObjAllocate(seg, script_nr, MEM_OBJ_SCRIPT);
	if (!mem) {
		sciprintf("%s, %d, Not enough memory, ", __FILE__, __LINE__);
		return NULL;
	}

	*seg_id = seg;
	return (Script *)mem;
}

void SegManager::setScriptSize(Script &scr, EngineState *s, int script_nr) {
	Resource *script = s->resmgr->findResource(kResourceTypeScript, script_nr, 0);
	Resource *heap = s->resmgr->findResource(kResourceTypeHeap, script_nr, 0);

	scr.script_size = script->size;
	scr.heap_size = 0; // Set later

	if (!script || (s->version >= SCI_VERSION(1, 001, 000) && !heap)) {
		sciprintf("%s: failed to load %s\n", __FUNCTION__, !script ? "script" : "heap");
		return;
	}
	if (s->version < SCI_VERSION_FTU_NEW_SCRIPT_HEADER) {
		scr.buf_size = script->size + READ_LE_UINT16(script->data) * 2;
		//locals_size = READ_LE_UINT16(script->data) * 2;
	} else if (s->version < SCI_VERSION(1, 001, 000)) {
		scr.buf_size = script->size;
	} else {
		scr.buf_size = script->size + heap->size;
		scr.heap_size = heap->size;

		// Ensure that the start of the heap resource can be word-aligned.
		if (script->size & 2) {
			scr.buf_size++;
			scr.script_size++;
		}

		if (scr.buf_size > 65535) {
			sciprintf("Script and heap sizes combined exceed 64K.\n"
			          "This means a fundamental design bug was made in SCI\n"
			          "regarding SCI1.1 games.\nPlease report this so it can be"
			          "fixed in the next major version!\n");
			return;
		}
	}
}

int SegManager::initialiseScript(Script &scr, EngineState *s, int script_nr) {
	// allocate the script.buf

	setScriptSize(scr, s, script_nr);
	scr.buf = (byte *)sci_malloc(scr.buf_size);

	dbgPrint("scr.buf ", scr.buf);
	if (!scr.buf) {
		scr.freeScript();
		sciprintf("SegManager: Not enough memory space for script size");
		scr.buf_size = 0;
		return 0;
	}

	// Initialize objects
	scr.objects = NULL;
	scr.objects_allocated = 0;
	scr.objects_nr = 0; // No objects recorded yet

	scr.locals_offset = 0;
	scr.locals_block = NULL;

	scr.code = NULL;
	scr.code_blocks_nr = 0;
	scr.code_blocks_allocated = 0;

	scr.nr = script_nr;
	scr.marked_as_deleted = 0;
	scr.relocated = 0;

	scr.obj_indices = new IntMapper();

	if (s->version >= SCI_VERSION(1, 001, 000))
		scr.heap_start = scr.buf + scr.script_size;
	else
		scr.heap_start = scr.buf;

	return 1;
}

int SegManager::deallocate(int seg, bool recursive) {
	MemObject *mobj;
	VERIFY(check(seg), "invalid seg id");

	mobj = _heap[seg];
	id_seg_map->removeKey(mobj->getSegMgrId());

	switch (mobj->getType()) {
	case MEM_OBJ_SCRIPT:
		// FIXME: Get rid of the recursive flag, so that we can move the
		// following into the destructor. The only time it is set to false
		// is in the SegManager destructor.
		if (recursive && (*(Script *)mobj).locals_segment)
			deallocate((*(Script *)mobj).locals_segment, recursive);
		break;

	case MEM_OBJ_LOCALS:
		break;

	case MEM_OBJ_DYNMEM:
		break;
	case MEM_OBJ_SYS_STRINGS: 
		break;
	case MEM_OBJ_STACK:
		break;
	case MEM_OBJ_LISTS:
		break;
	case MEM_OBJ_NODES:
		break;
	case MEM_OBJ_CLONES:
		break;
	case MEM_OBJ_HUNK:
		break;
	case MEM_OBJ_STRING_FRAG:
		break;
	default:
		error("Deallocating segment type %d not supported", mobj->getType());
	}

	delete mobj;
	_heap[seg] = NULL;

	return 1;
}

int SegManager::scriptMarkedDeleted(int script_nr) {
	Script *scr = getScript(script_nr, SCRIPT_ID);
	return scr->marked_as_deleted;
}

void SegManager::markScriptDeleted(int script_nr) {
	Script *scr = getScript(script_nr, SCRIPT_ID);
	scr->marked_as_deleted = 1;
}

void SegManager::unmarkScriptDeleted(int script_nr) {
	Script *scr = getScript(script_nr, SCRIPT_ID);
	scr->marked_as_deleted = 0;
}

int SegManager::scriptIsMarkedAsDeleted(SegmentId seg) {
	Script *scr;

	if (!check(seg))
		return 0;

	if (_heap[seg]->getType() != MEM_OBJ_SCRIPT)
		return 0;

	scr = (Script *)_heap[seg];

	return scr->marked_as_deleted;
}


int SegManager::deallocateScript(int script_nr) {
	int seg = segGet(script_nr);

	deallocate(seg, true);

	return 1;
}

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
		mem = new dstack_t();
		break;
	case MEM_OBJ_HUNK:
		mem = new HunkTable();
		break;
	case MEM_OBJ_STRING_FRAG:
		mem = new MemObject();	// FIXME: This is a temporary hack until MEM_OBJ_STRING_FRAG is implemented
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

MemObject *SegManager::memObjAllocate(SegmentId segid, int hash_id, MemObjectType type) {
	MemObject *mem = MemObject::createMemObject(type);
	if (!mem) {
		sciprintf("SegManager: invalid mobj ");
		return NULL;
	}

	if (segid >= (int)_heap.size()) {
		if (segid >= (int)_heap.size() * 2) {
			sciprintf("SegManager: hash_map error or others??");
			return NULL;
		}
		_heap.resize(_heap.size() * 2);
	}

	mem->_segmgrId = hash_id;

	// hook it to the heap
	_heap[segid] = mem;
	return mem;
}

void Script::freeScript() {
	free(buf);
	buf = NULL;
	buf_size = 0;

	if (objects) {
		for (int i = 0; i < objects_nr; i++) {
			Object *object = &objects[i];
			if (object->variables) {
				free(object->variables);
				object->variables = NULL;
				object->variables_nr = 0;
			}
		}
		free(objects);
		objects = NULL;
		objects_nr = 0;
	}

	delete obj_indices;
	obj_indices = 0;
	free(code);
	code = 0;
}

// memory operations

void SegManager::mcpyInOut(int dst, const void *src, size_t n, int id, idFlag flag) {
	Script *scr = getScript(id, flag);
	if (scr->buf) {
		memcpy(scr->buf + dst, src, n);
	}
}

int16 SegManager::getHeap(reg_t reg) {
	MemObject *mobj;
	Script *scr;

	VERIFY(check(reg.segment), "Invalid seg id");
	mobj = _heap[reg.segment];

	switch (mobj->getType()) {
	case MEM_OBJ_SCRIPT:
		scr = (Script *)mobj;
		VERIFY(reg.offset + 1 < (uint16)scr->buf_size, "invalid offset\n");
		return (scr->buf[reg.offset] | (scr->buf[reg.offset+1]) << 8);
	default:
		error("SegManager::getHeap: unsupported mem obj type %d", mobj->getType());
		break;
	}
	return 0; // never get here
}

// return the seg if script_id is valid and in the map, else -1
int SegManager::segGet(int script_id) const {
	return id_seg_map->lookupKey(script_id);
}

Script *SegManager::getScript(const int id, idFlag flag) {
	const int seg = (flag == SCRIPT_ID) ? segGet(id) : id;

	if (seg < 0 || (uint)seg >= _heap.size()) {
		error("SegManager::getScript(%d,%d): seg id %x out of bounds", id, flag, seg);
	}
	if (!_heap[seg]) {
		error("SegManager::getScript(%d,%d): seg id %x is not in memory", id, flag, seg);
	}
	if (_heap[seg]->getType() != MEM_OBJ_SCRIPT) {
		error("SegManager::getScript(%d,%d): seg id %x refers to type %d != MEM_OBJ_SCRIPT", id, flag, seg, _heap[seg]->getType());
	}
	return (Script *)_heap[seg];
}

// validate the seg
// return:
//	false - invalid seg
//	true  - valid seg
bool SegManager::check(int seg) {
	if (seg < 0 || (uint)seg >= _heap.size()) {
		return false;
	}
	if (!_heap[seg]) {
		sciprintf("SegManager: seg %x is removed from memory, but not removed from hash_map\n", seg);
		return false;
	}
	return true;
}

int SegManager::scriptIsLoaded(int id, idFlag flag) {
	if (flag == SCRIPT_ID)
		id = segGet(id);

	return check(id);
}

void SegManager::incrementLockers(int id, idFlag flag) {
	Script *scr = getScript(id, flag);
	scr->lockers++;
}

void SegManager::decrementLockers(int id, idFlag flag) {
	Script *scr = getScript(id, flag);
	if (scr->lockers > 0)
		scr->lockers--;
}

int SegManager::getLockers(int id, idFlag flag) {
	Script *scr = getScript(id, flag);
	return scr->lockers;
}

void SegManager::setLockers(int lockers, int id, idFlag flag) {
	Script *scr = getScript(id, flag);
	scr->lockers = lockers;
}

void SegManager::setExportTableOffset(int offset, int id, idFlag flag) {
	Script *scr = getScript(id, flag);

	if (offset) {
		scr->export_table = (uint16 *)(scr->buf + offset + 2);
		scr->exports_nr = READ_LE_UINT16((byte *)(scr->export_table - 1));
	} else {
		scr->export_table = NULL;
		scr->exports_nr = 0;
	}
}

void SegManager::setExportWidth(int flag) {
	exports_wide = flag;
}

void SegManager::setSynonymsOffset(int offset, int id, idFlag flag) {
	Script *scr = getScript(id, flag);
	scr->synonyms = scr->buf + offset;
}

byte *SegManager::getSynonyms(int id, idFlag flag) {
	Script *scr = getScript(id, flag);
	return scr->synonyms;
}

void SegManager::setSynonymsNr(int nr, int id, idFlag flag) {
	Script *scr = getScript(id, flag);
	scr->synonyms_nr = nr;
}

int SegManager::getSynonymsNr(int id, idFlag flag) {
	Script *scr = getScript(id, flag);
	return scr->synonyms_nr;
}

int SegManager::relocateBlock(reg_t *block, int block_location, int block_items, SegmentId segment, int location) {
	int rel = location - block_location;
	int index;

	if (rel < 0)
		return 0;

	index = rel >> 1;

	if (index >= block_items)
		return 0;

	if (rel & 1) {
		sciprintf("Error: Attempt to relocate odd variable #%d.5e (relative to %04x)\n", index, block_location);
		return 0;
	}
	block[index].segment = segment; // Perform relocation
	if (isSci1_1)
		block[index].offset += getScript(segment, SEG_ID)->script_size;

	return 1;
}

int SegManager::relocateLocal(Script *scr, SegmentId segment, int location) {
	if (scr->locals_block)
		return relocateBlock(scr->locals_block->locals, scr->locals_offset, scr->locals_block->nr, segment, location);
	else
		return 0; // No hands, no cookies
}

int SegManager::relocateObject(Object *obj, SegmentId segment, int location) {
	return relocateBlock(obj->variables, obj->pos.offset, obj->variables_nr, segment, location);
}

void SegManager::scriptAddCodeBlock(reg_t location) {
	Script *scr = getScript(location.segment, SEG_ID);

	if (++scr->code_blocks_nr > scr->code_blocks_allocated) {
		scr->code_blocks_allocated += DEFAULT_OBJECTS_INCREMENT;
		scr->code = (CodeBlock *)sci_realloc(scr->code, scr->code_blocks_allocated * sizeof(CodeBlock));
	}

	int index = scr->code_blocks_nr - 1;
	scr->code[index].pos = location;
	scr->code[index].size = READ_LE_UINT16(scr->buf + location.offset - 2);
}

void SegManager::scriptRelocate(reg_t block) {
	Script *scr = getScript(block.segment, SEG_ID);

	VERIFY(block.offset < (uint16)scr->buf_size && READ_LE_UINT16(scr->buf + block.offset) * 2 + block.offset < (uint16)scr->buf_size,
	       "Relocation block outside of script\n");

	int count = READ_LE_UINT16(scr->buf + block.offset);

	for (int i = 0; i <= count; i++) {
		int pos = READ_LE_UINT16(scr->buf + block.offset + 2 + (i * 2));
		if (!pos)
			continue; // FIXME: A hack pending investigation

		if (!relocateLocal(scr, block.segment, pos)) {
			int k, done = 0;

			for (k = 0; !done && k < scr->objects_nr; k++) {
				if (relocateObject(scr->objects + k, block.segment, pos))
					done = 1;
			}

			for (k = 0; !done && k < scr->code_blocks_nr; k++) {
				if (pos >= scr->code[k].pos.offset &&
				        pos < scr->code[k].pos.offset + scr->code[k].size)
					done = 1;
			}

			if (!done) {
				sciprintf("While processing relocation block "PREG":\n", PRINT_REG(block));
				sciprintf("Relocation failed for index %04x (%d/%d)\n", pos, i + 1, count);
				if (scr->locals_block)
					sciprintf("- locals: %d at %04x\n", scr->locals_block->nr, scr->locals_offset);
				else
					sciprintf("- No locals\n");
				for (k = 0; k < scr->objects_nr; k++)
					sciprintf("- obj#%d at %04x w/ %d vars\n", k, scr->objects[k].pos.offset, scr->objects[k].variables_nr);
// SQ3 script 71 has broken relocation entries.
// Since this is mainstream, we can't break out as we used to do.
				sciprintf("Trying to continue anyway...\n");
//				BREAKPOINT();
			}
		}
	}
}

void SegManager::heapRelocate(EngineState *s, reg_t block) {
	Script *scr = getScript(block.segment, SEG_ID);

	VERIFY(block.offset < (uint16)scr->heap_size && READ_LE_UINT16(scr->heap_start + block.offset) * 2 + block.offset < (uint16)scr->buf_size,
	       "Relocation block outside of script\n");

	if (scr->relocated)
		return;
	scr->relocated = 1;
	int count = READ_LE_UINT16(scr->heap_start + block.offset);

	for (int i = 0; i < count; i++) {
		int pos = READ_LE_UINT16(scr->heap_start + block.offset + 2 + (i * 2)) + scr->script_size;

		if (!relocateLocal(scr, block.segment, pos)) {
			int k, done = 0;

			for (k = 0; !done && k < scr->objects_nr; k++) {
				if (relocateObject(scr->objects + k, block.segment, pos))
					done = 1;
			}

			if (!done) {
				sciprintf("While processing relocation block "PREG":\n", PRINT_REG(block));
				sciprintf("Relocation failed for index %04x (%d/%d)\n", pos, i + 1, count);
				if (scr->locals_block)
					sciprintf("- locals: %d at %04x\n", scr->locals_block->nr, scr->locals_offset);
				else
					sciprintf("- No locals\n");
				for (k = 0; k < scr->objects_nr; k++)
					sciprintf("- obj#%d at %04x w/ %d vars\n", k, scr->objects[k].pos.offset, scr->objects[k].variables_nr);
				sciprintf("Triggering breakpoint...\n");
				BREAKPOINT();
			}
		}
	}
}

#define INST_LOOKUP_CLASS(id) ((id == 0xffff) ? NULL_REG : get_class_address(s, id, SCRIPT_GET_LOCK, NULL_REG))

reg_t get_class_address(EngineState *s, int classnr, int lock, reg_t caller);

Object *SegManager::scriptObjInit0(EngineState *s, reg_t obj_pos) {
	Object *obj;
	int id;
	unsigned int base = obj_pos.offset - SCRIPT_OBJECT_MAGIC_OFFSET;
	reg_t temp;

	Script *scr = getScript(obj_pos.segment, SEG_ID);

	VERIFY(base < scr->buf_size, "Attempt to initialize object beyond end of script\n");

	if (!scr->objects) {
		scr->objects_allocated = DEFAULT_OBJECTS;
		scr->objects = (Object *)sci_malloc(sizeof(Object) * scr->objects_allocated);
	}
	if (scr->objects_nr == scr->objects_allocated) {
		scr->objects_allocated += DEFAULT_OBJECTS_INCREMENT;
		scr->objects = (Object *)sci_realloc(scr->objects, sizeof(Object) * scr->objects_allocated);
	}

	temp = make_reg(obj_pos.segment, base);
	id = scr->obj_indices->checkKey(base, true);
	scr->objects_nr++;

	obj = scr->objects + id;

	VERIFY(base + SCRIPT_FUNCTAREAPTR_OFFSET  < scr->buf_size, "Function area pointer stored beyond end of script\n");

	{
		byte *data = (byte *)(scr->buf + base);
		int funct_area = READ_LE_UINT16(data + SCRIPT_FUNCTAREAPTR_OFFSET);
		int variables_nr;
		int functions_nr;
		int is_class;
		int i;

		obj->flags = 0;
		obj->pos = temp;

		VERIFY(base + funct_area < scr->buf_size, "Function area pointer references beyond end of script");

		variables_nr = READ_LE_UINT16(data + SCRIPT_SELECTORCTR_OFFSET);
		functions_nr = READ_LE_UINT16(data + funct_area - 2);
		is_class = READ_LE_UINT16(data + SCRIPT_INFO_OFFSET) & SCRIPT_INFO_CLASS;

		VERIFY(base + funct_area + functions_nr * 2
		       // add again for classes, since those also store selectors
		       + (is_class ? functions_nr * 2 : 0) < scr->buf_size, "Function area extends beyond end of script");

		obj->variables_nr = variables_nr;
		obj->variables = (reg_t *)sci_malloc(sizeof(reg_t) * variables_nr);

		obj->methods_nr = functions_nr;
		obj->base = scr->buf;
		obj->base_obj = data;
		obj->base_method = (uint16 *)(data + funct_area);
		obj->base_vars = NULL;

		for (i = 0; i < variables_nr; i++)
			obj->variables[i] = make_reg(0, READ_LE_UINT16(data + (i * 2)));
	}

	return obj;
}

Object *SegManager::scriptObjInit11(EngineState *s, reg_t obj_pos) {
	Object *obj;
	int id;
	int base = obj_pos.offset;

	Script *scr = getScript(obj_pos.segment, SEG_ID);

	VERIFY(base < (uint16)scr->buf_size, "Attempt to initialize object beyond end of script\n");

	if (!scr->objects) {
		scr->objects_allocated = DEFAULT_OBJECTS;
		scr->objects = (Object *)sci_malloc(sizeof(Object) * scr->objects_allocated);
	}
	if (scr->objects_nr == scr->objects_allocated) {
		scr->objects_allocated += DEFAULT_OBJECTS_INCREMENT;
		scr->objects = (Object *)sci_realloc(scr->objects, sizeof(Object) * scr->objects_allocated);
	}

	id = scr->obj_indices->checkKey(obj_pos.offset, true);
	scr->objects_nr++;

	obj = scr->objects + id;

	VERIFY(base + SCRIPT_FUNCTAREAPTR_OFFSET < (uint16)scr->buf_size, "Function area pointer stored beyond end of script\n");

	{
		byte *data = (byte *)(scr->buf + base);
		uint16 *funct_area = (uint16 *)(scr->buf + READ_LE_UINT16(data + 6));
		uint16 *prop_area = (uint16 *)(scr->buf + READ_LE_UINT16(data + 4));
		int variables_nr;
		int functions_nr;
		int is_class;
		int i;

		obj->flags = 0;
		obj->pos = obj_pos;

		VERIFY((byte *) funct_area < scr->buf + scr->buf_size, "Function area pointer references beyond end of script");

		variables_nr = READ_LE_UINT16(data + 2);
		functions_nr = READ_LE_UINT16(funct_area);
		is_class = READ_LE_UINT16(data + 14) & SCRIPT_INFO_CLASS;

		obj->base_method = funct_area;
		obj->base_vars = prop_area;

		VERIFY(((byte *) funct_area + functions_nr) < scr->buf + scr->buf_size, "Function area extends beyond end of script");

		obj->variables_nr = variables_nr;
		obj->variable_names_nr = variables_nr;
		obj->variables = (reg_t *)sci_malloc(sizeof(reg_t) * variables_nr);

		obj->methods_nr = functions_nr;
		obj->base = scr->buf;
		obj->base_obj = data;

		for (i = 0; i < variables_nr; i++)
			obj->variables[i] = make_reg(0, READ_LE_UINT16(data + (i * 2)));
	}

	return obj;
}

Object *SegManager::scriptObjInit(EngineState *s, reg_t obj_pos) {
	if (!isSci1_1)
		return scriptObjInit0(s, obj_pos);
	else
		return scriptObjInit11(s, obj_pos);
}

LocalVariables *SegManager::allocLocalsSegment(Script *scr, int count) {
	if (!count) { // No locals
		scr->locals_segment = 0;
		scr->locals_block = NULL;
		return NULL;
	} else {
		MemObject *mobj;
		LocalVariables *locals;

		if (scr->locals_segment) {
			mobj = _heap[scr->locals_segment];
			VERIFY(mobj != NULL, "Re-used locals segment was NULL'd out");
			VERIFY(mobj->getType() == MEM_OBJ_LOCALS, "Re-used locals segment did not consist of local variables");
			VERIFY((*(LocalVariables *)mobj).script_id == scr->nr, "Re-used locals segment belonged to other script");
		} else
			mobj = allocNonscriptSegment(MEM_OBJ_LOCALS, &scr->locals_segment);

		locals = scr->locals_block = (LocalVariables *)mobj;
		locals->script_id = scr->nr;
		locals->locals = (reg_t *)sci_calloc(count, sizeof(reg_t));
		locals->nr = count;

		return locals;
	}
}

void SegManager::scriptInitialiseLocalsZero(SegmentId seg, int count) {
	Script *scr = getScript(seg, SEG_ID);

	scr->locals_offset = -count * 2; // Make sure it's invalid

	allocLocalsSegment(scr, count);
}

void SegManager::scriptInitialiseLocals(reg_t location) {
	Script *scr = getScript(location.segment, SEG_ID);
	unsigned int count;

	VERIFY(location.offset + 1 < (uint16)scr->buf_size, "Locals beyond end of script\n");

	if (isSci1_1)
		count = READ_LE_UINT16(scr->buf + location.offset - 2);
	else
		count = (READ_LE_UINT16(scr->buf + location.offset - 2) - 4) >> 1;
	// half block size

	scr->locals_offset = location.offset;

	if (!(location.offset + count * 2 + 1 < scr->buf_size)) {
		sciprintf("Locals extend beyond end of script: offset %04x, count %x vs size %x\n", location.offset, count, (uint)scr->buf_size);
		count = (scr->buf_size - location.offset) >> 1;
	}

	LocalVariables *locals = allocLocalsSegment(scr, count);
	if (locals) {
		uint i;
		byte *base = (byte *)(scr->buf + location.offset);

		for (i = 0; i < count; i++)
			locals->locals[i].offset = READ_LE_UINT16(base + i * 2);
	}
}

void SegManager::scriptRelocateExportsSci11(int seg) {
	Script *scr = getScript(seg, SEG_ID);
	for (int i = 0; i < scr->exports_nr; i++) {
		/* We are forced to use an ugly heuristic here to distinguish function
		   exports from object/class exports. The former kind points into the
		   script resource, the latter into the heap resource.  */
		int location = READ_LE_UINT16((byte *)(scr->export_table + i));
		if (READ_LE_UINT16(scr->heap_start + location) == SCRIPT_OBJECT_MAGIC_NUMBER) {
			WRITE_LE_UINT16((byte *)(scr->export_table + i), location + scr->heap_start - scr->buf);
		} else {
			// Otherwise it's probably a function export,
			// and we don't need to do anything.
		}
	}
}

void SegManager::scriptInitialiseObjectsSci11(EngineState *s, int seg) {
	Script *scr = getScript(seg, SEG_ID);
	byte *seeker = scr->heap_start + 4 + READ_LE_UINT16(scr->heap_start + 2) * 2;

	while (READ_LE_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		if (READ_LE_UINT16(seeker + 14) & SCRIPT_INFO_CLASS) {
			int classpos = seeker - scr->buf;
			int species = READ_LE_UINT16(seeker + 10);

			if (species < 0 || species >= (int)s->_classtable.size()) {
				sciprintf("Invalid species %d(0x%x) not in interval [0,%d) while instantiating script %d\n",
				          species, species, s->_classtable.size(), scr->nr);
				script_debug_flag = script_error_flag = 1;
				return;
			}

			s->_classtable[species].script = scr->nr;
			s->_classtable[species].reg.segment = seg;
			s->_classtable[species].reg.offset = classpos;
		}
		seeker += READ_LE_UINT16(seeker + 2) * 2;
	}

	seeker = scr->heap_start + 4 + READ_LE_UINT16(scr->heap_start + 2) * 2;
	while (READ_LE_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		reg_t reg;
		Object *obj;

		reg.segment = seg;
		reg.offset = seeker - scr->buf;
		obj = scriptObjInit(s, reg);

#if 0
		if (obj->variables[5].offset != 0xffff) {
			obj->variables[5] = INST_LOOKUP_CLASS(obj->variables[5].offset);
			base_obj = obj_get(s, obj->variables[5]);
			obj->variable_names_nr = base_obj->variables_nr;
			obj->base_obj = base_obj->base_obj;
		}
#endif

		// Copy base from species class, as we need its selector IDs
		obj->variables[6] = INST_LOOKUP_CLASS(obj->variables[6].offset);

		seeker += READ_LE_UINT16(seeker + 2) * 2;
	}
}

void SegManager::scriptFreeUnusedObjects(SegmentId seg) {
	Script *scr = getScript(seg, SEG_ID);
	if (scr->objects_allocated > scr->objects_nr) {
		if (scr->objects_nr)
			scr->objects = (Object *)sci_realloc(scr->objects, sizeof(Object) * scr->objects_nr);
		else {
			if (scr->objects_allocated)
				free(scr->objects);
			scr->objects = NULL;
		}
		scr->objects_allocated = scr->objects_nr;
	}
}

/*
static char *SegManager::dynprintf(char *msg, ...) {
	va_list argp;
	char *buf = (char *)sci_malloc(strlen(msg) + 100);

	va_start(argp, msg);
	vsprintf(buf, msg, argp);
	va_end(argp);

	return buf;
}
*/

dstack_t *SegManager::allocateStack(int size, SegmentId *segid) {
	MemObject *mobj = allocNonscriptSegment(MEM_OBJ_STACK, segid);
	dstack_t *retval = (dstack_t *)mobj;

	retval->entries = (reg_t *)sci_calloc(size, sizeof(reg_t));
	retval->nr = size;

	return retval;
}

SystemStrings *SegManager::allocateSysStrings(SegmentId *segid) {
	return (SystemStrings *)allocNonscriptSegment(MEM_OBJ_SYS_STRINGS, segid);
}

SegmentId SegManager::allocateStringFrags() {
	SegmentId segid;
	
	allocNonscriptSegment(MEM_OBJ_STRING_FRAG, &segid);

	return segid;
}

uint16 SegManager::validateExportFunc(int pubfunct, int seg) {
	Script *scr = getScript(seg, SEG_ID);
	if (scr->exports_nr <= pubfunct) {
		sciprintf("pubfunct is invalid");
		return 0;
	}

	if (exports_wide)
		pubfunct *= 2;
	uint16 offset = READ_LE_UINT16((byte *)(scr->export_table + pubfunct));
	VERIFY(offset < scr->buf_size, "invalid export function pointer");

	return offset;
}

void SegManager::free_hunk_entry(reg_t addr) {
	HunkTable *ht = (HunkTable *)GET_SEGMENT(*this, addr.segment, MEM_OBJ_HUNK);

	if (!ht) {
		sciprintf("Attempt to free Hunk from address "PREG": Invalid segment type\n", PRINT_REG(addr));
		return;
	}

	ht->freeEntry(addr.offset);
}

Hunk *SegManager::alloc_hunk_entry(const char *hunk_type, int size, reg_t *reg) {
	Hunk *h = alloc_Hunk(reg);

	if (!h)
		return NULL;

	h->mem = sci_malloc(size);
	h->size = size;
	h->type = hunk_type;

	return h;
}

Clone *SegManager::alloc_Clone(reg_t *addr) {
	CloneTable *table;
	int offset;

	if (!Clones_seg_id) {
		table = (CloneTable *)allocNonscriptSegment(MEM_OBJ_CLONES, &(Clones_seg_id));
		table->initTable();
	} else
		table = (CloneTable *)_heap[Clones_seg_id];

	offset = table->allocEntry();

	*addr = make_reg(Clones_seg_id, offset);
	return &(table->table[offset]);
}

List *SegManager::alloc_List(reg_t *addr) {
	ListTable *table;
	int offset;

	if (!Lists_seg_id) {
		table = (ListTable *)allocNonscriptSegment(MEM_OBJ_LISTS, &(Lists_seg_id));
		table->initTable();
	} else
		table = (ListTable *)_heap[Lists_seg_id];

	offset = table->allocEntry();

	*addr = make_reg(Lists_seg_id, offset);
	return &(table->table[offset]);
}

Node *SegManager::alloc_Node(reg_t *addr) {
	NodeTable *table;
	int offset;

	if (!Nodes_seg_id) {
		table = (NodeTable *)allocNonscriptSegment(MEM_OBJ_NODES, &(Nodes_seg_id));
		table->initTable();
	} else
		table = (NodeTable *)_heap[Nodes_seg_id];

	offset = table->allocEntry();

	*addr = make_reg(Nodes_seg_id, offset);
	return &(table->table[offset]);
}

Hunk *SegManager::alloc_Hunk(reg_t *addr) {
	HunkTable *table;
	int offset;

	if (!Hunks_seg_id) {
		table = (HunkTable *)allocNonscriptSegment(MEM_OBJ_HUNK, &(Hunks_seg_id));
		table->initTable();
	} else
		table = (HunkTable *)_heap[Hunks_seg_id];

	offset = table->allocEntry();

	*addr = make_reg(Hunks_seg_id, offset);
	return &(table->table[offset]);
}

byte *MemObject::dereference(reg_t pointer, int *size) {
	error("Error: Trying to dereference pointer "PREG" to inappropriate segment",
		          PRINT_REG(pointer));
	return NULL;
}

byte *Script::dereference(reg_t pointer, int *size) {
	if (pointer.offset > buf_size) {
		sciprintf("Error: Attempt to dereference invalid pointer "PREG" into script segment (script size=%d)\n",
				  PRINT_REG(pointer), (uint)buf_size);
		return NULL;
	}
	if (size)
		*size = buf_size - pointer.offset;
	return (byte *)(buf + pointer.offset);
}

byte *LocalVariables::dereference(reg_t pointer, int *size) {
	int count = nr * sizeof(reg_t);
	byte *base = (byte *)locals;

	if (size)
		*size = count;

	return base + pointer.offset;
}

byte *dstack_t::dereference(reg_t pointer, int *size) {
	int count = nr * sizeof(reg_t);
	byte *base = (byte *)entries;

	if (size)
		*size = count;

	return base + pointer.offset;
}

byte *DynMem::dereference(reg_t pointer, int *size) {
	int count = _size;
	byte *base = (byte *)_buf;

	if (size)
		*size = count;

	return base + pointer.offset;
}

byte *SystemStrings::dereference(reg_t pointer, int *size) {
	if (size)
		*size = strings[pointer.offset].max_size;
	if (pointer.offset < SYS_STRINGS_MAX && strings[pointer.offset].name)
		return (byte *)(strings[pointer.offset].value);

	error("Attempt to dereference invalid pointer "PREG"", PRINT_REG(pointer));
	return NULL;
}

byte *SegManager::dereference(reg_t pointer, int *size) {
	if (!pointer.segment || (pointer.segment >= _heap.size()) || !_heap[pointer.segment]) {
		error("Attempt to dereference invalid pointer "PREG"", PRINT_REG(pointer));
		return NULL; /* Invalid */
	}

	MemObject *mobj = _heap[pointer.segment];
	return mobj->dereference(pointer, size);
}

unsigned char *SegManager::allocDynmem(int size, const char *descr, reg_t *addr) {
	SegmentId seg;
	MemObject *mobj = allocNonscriptSegment(MEM_OBJ_DYNMEM, &seg);
	*addr = make_reg(seg, 0);

	DynMem &d = *(DynMem *)mobj;

	d._size = size;

	if (size == 0)
		d._buf = NULL;
	else
		d._buf = (byte *)sci_malloc(size);

	d._description = sci_strdup(descr);

	return (unsigned char *)(d._buf);
}

const char *SegManager::getDescription(reg_t addr) {
	MemObject *mobj = _heap[addr.segment];

	if (addr.segment >= _heap.size())
		return "";

	switch (mobj->getType()) {
	case MEM_OBJ_DYNMEM:
		return (*(DynMem *)mobj)._description;
	default:
		return "";
	}
}

int SegManager::freeDynmem(reg_t addr) {
	if (addr.segment <= 0 || addr.segment >= _heap.size() || !_heap[addr.segment] || _heap[addr.segment]->getType() != MEM_OBJ_DYNMEM)
		return 1; // error

	deallocate(addr.segment, true);

	return 0; // OK
}

void SegManager::dbgPrint(const char* msg, void *i) {
#ifdef DEBUG_SEG_MANAGER
	char buf[1000];
	sprintf(buf, "%s = [0x%x], dec:[%d]", msg, i, i);
	perror(buf);
#endif
}


// ------------------- Segment interface ------------------
SegInterface::SegInterface(SegManager *segmgr, MemObject *mobj, SegmentId segId, MemObjectType typeId) :
	_segmgr(segmgr), _mobj(mobj), _segId(segId), _typeId(typeId) {
	VERIFY(_mobj->getType() == _typeId, "Invalid MemObject type");
}

reg_t SegInterface::findCanonicAddress(reg_t addr) {
	return addr;
}

void SegInterface::freeAtAddress(reg_t sub_addr) {
}

void SegInterface::listAllDeallocatable(void *param, NoteCallback note) {
}

void SegInterface::listAllOutgoingReferences(EngineState *s, reg_t object, void *param, NoteCallback note) {
}


//-------------------- base --------------------
class SegInterfaceBase : public SegInterface {
protected:
	SegInterfaceBase(SegManager *segmgr, MemObject *mobj, SegmentId segId, MemObjectType typeId) :
		SegInterface(segmgr, mobj, segId, typeId) {}
public:
	reg_t findCanonicAddress(reg_t addr);
	void listAllDeallocatable(void *param, NoteCallback note);
};

reg_t SegInterfaceBase::findCanonicAddress(reg_t addr) {
	addr.offset = 0;
	return addr;
}

void SegInterfaceBase::listAllDeallocatable(void *param, NoteCallback note) {
	(*note)(param, make_reg(_segId, 0));
}


//-------------------- script --------------------
class SegInterfaceScript : public SegInterfaceBase {
public:
	SegInterfaceScript(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterfaceBase(segmgr, mobj, segId, MEM_OBJ_SCRIPT) {}
	void freeAtAddress(reg_t addr);
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note);
};

void SegInterfaceScript::freeAtAddress(reg_t addr) {
	Script *script = (Script *)_mobj;
	/*
		sciprintf("[GC] Freeing script "PREG"\n", PRINT_REG(addr));
		if (script->locals_segment)
			sciprintf("[GC] Freeing locals %04x:0000\n", script->locals_segment);
	*/

	if (script->marked_as_deleted)
		_segmgr->deallocateScript(script->nr);
}

void SegInterfaceScript::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note) {
	Script *script = (Script *)_mobj;

	if (addr.offset <= script->buf_size && addr.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET && RAW_IS_OBJECT(script->buf + addr.offset)) {
		int idx = RAW_GET_CLASS_INDEX(script, addr);
		if (idx >= 0 && idx < script->objects_nr) {
			Object *obj = script->objects + idx;
			int i;

			// Note all local variables, if we have a local variable environment
			if (script->locals_segment)
				(*note)(param, make_reg(script->locals_segment, 0));

			for (i = 0; i < obj->variables_nr; i++)
				(*note)(param, obj->variables[i]);
		} else {
			fprintf(stderr, "Request for outgoing script-object reference at "PREG" yielded invalid index %d\n", PRINT_REG(addr), idx);
		}
	} else {
		/*		fprintf(stderr, "Unexpected request for outgoing script-object references at "PREG"\n", PRINT_REG(addr));*/
		/* Happens e.g. when we're looking into strings */
	}
}


//-------------------- clones --------------------
class SegInterfaceClones : public SegInterface {
public:
	SegInterfaceClones(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_CLONES) {}
	void freeAtAddress(reg_t addr);
	void listAllDeallocatable(void *param, NoteCallback note);
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note);
};

void SegInterfaceClones::listAllDeallocatable(void *param, NoteCallback note) {
	CloneTable *table = (CloneTable *)_mobj;
	for (int i = 0; i < table->max_entry; i++)
		if (ENTRY_IS_VALID(table, i))
			(*note)(param, make_reg(_segId, i));


}

void SegInterfaceClones::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note) {
	CloneTable *clone_table = (CloneTable *)_mobj;
	Clone *clone;
	int i;

	assert(addr.segment == _segId);

	if (!(ENTRY_IS_VALID(clone_table, addr.offset))) {
		fprintf(stderr, "Unexpected request for outgoing references from clone at "PREG"\n", PRINT_REG(addr));
//		BREAKPOINT();
		return;
	}

	clone = &(clone_table->table[addr.offset]);

	// Emit all member variables (including references to the 'super' delegate)
	for (i = 0; i < clone->variables_nr; i++)
		(*note)(param, clone->variables[i]);

	// Note that this also includes the 'base' object, which is part of the script and therefore also emits the locals.
	(*note)(param, clone->pos);
	//sciprintf("[GC] Reporting clone-pos "PREG"\n", PRINT_REG(clone->pos));
}

void SegInterfaceClones::freeAtAddress(reg_t addr) {
	CloneTable *clone_table = (CloneTable *)_mobj;
	Object *victim_obj;

	assert(addr.segment == _segId);

	victim_obj = &(clone_table->table[addr.offset]);

#ifdef GC_DEBUG
	if (!(victim_obj->flags & OBJECT_FLAG_FREED))
		sciprintf("[GC] Warning: Clone "PREG" not reachable and not freed (freeing now)\n", PRINT_REG(addr));
#ifdef GC_DEBUG_VERBOSE
	else
		sciprintf("[GC-DEBUG] Clone "PREG": Freeing\n", PRINT_REG(addr));
#endif
#endif
	/*
		sciprintf("[GC] Clone "PREG": Freeing\n", PRINT_REG(addr));
		sciprintf("[GC] Clone had pos "PREG"\n", PRINT_REG(victim_obj->pos));
	*/
	free(victim_obj->variables);
	victim_obj->variables = NULL;
	clone_table->freeEntry(addr.offset);
}


//-------------------- locals --------------------
class SegInterfaceLocals : public SegInterface {
public:
	SegInterfaceLocals(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_LOCALS) {}
	reg_t findCanonicAddress(reg_t addr);
	void freeAtAddress(reg_t addr);
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note);
};

reg_t SegInterfaceLocals::findCanonicAddress(reg_t addr) {
	LocalVariables *locals = (LocalVariables *)_mobj;
	// Reference the owning script
	SegmentId owner_seg = _segmgr->segGet(locals->script_id);

	assert(owner_seg >= 0);

	return make_reg(owner_seg, 0);
}

void SegInterfaceLocals::freeAtAddress(reg_t sub_addr) {
	//sciprintf("  Request to free "PREG"\n", PRINT_REG(sub_addr));
	// STUB
}

void SegInterfaceLocals::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note) {
	LocalVariables *locals = (LocalVariables *)_mobj;

	assert(addr.segment == _segId);

	for (int i = 0; i < locals->nr; i++)
		(*note)(param, locals->locals[i]);
}


//-------------------- stack --------------------
class SegInterfaceStack : public SegInterface {
public:
	SegInterfaceStack(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_STACK) {}
	reg_t findCanonicAddress(reg_t addr);
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note);
};

reg_t SegInterfaceStack::findCanonicAddress(reg_t addr) {
	addr.offset = 0;
	return addr;
}

void SegInterfaceStack::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note) {
	int i;
	dstack_t &d = *(dstack_t *)_mobj;
	fprintf(stderr, "Emitting %d stack entries\n", d.nr);
	for (i = 0; i < d.nr; i++)
		(*note)(param, d.entries[i]);
	fprintf(stderr, "DONE");
}


//-------------------- system strings --------------------
class SegInterfaceSysStrings : public SegInterface {
public:
	SegInterfaceSysStrings(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_SYS_STRINGS) {}
};

//-------------------- string frags --------------------
class SegInterfaceStringFrag : public SegInterface {
public:
	SegInterfaceStringFrag(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_STRING_FRAG) {}
};


//-------------------- lists --------------------
class SegInterfaceLists : public SegInterface {
public:
	SegInterfaceLists(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_LISTS) {}
	void freeAtAddress(reg_t addr);
	void listAllDeallocatable(void *param, NoteCallback note);
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note);
};

void SegInterfaceLists::freeAtAddress(reg_t sub_addr) {
	ListTable *table = (ListTable *)_mobj;
	table->freeEntry(sub_addr.offset);
}

void SegInterfaceLists::listAllDeallocatable(void *param, NoteCallback note) {
	ListTable *table = (ListTable *)_mobj;
	for (int i = 0; i < table->max_entry; i++)
		if (ENTRY_IS_VALID(table, i))
			(*note) (param, make_reg(_segId, i));
}

void SegInterfaceLists::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note) {
	ListTable *table = (ListTable *)_mobj;
	List *list = &(table->table[addr.offset]);

	if (!ENTRY_IS_VALID(table, addr.offset)) {
		fprintf(stderr, "Invalid list referenced for outgoing references: "PREG"\n", PRINT_REG(addr));
		return;
	}

	note(param, list->first);
	note(param, list->last);
	// We could probably get away with just one of them, but
	// let's be conservative here.
}


//-------------------- nodes --------------------
class SegInterfaceNodes : public SegInterface {
public:
	SegInterfaceNodes(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_NODES) {}
	void freeAtAddress(reg_t addr);
	void listAllDeallocatable(void *param, NoteCallback note);
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note);
};

void SegInterfaceNodes::freeAtAddress(reg_t sub_addr) {
	NodeTable *table = (NodeTable *)_mobj;
	table->freeEntry(sub_addr.offset);
}

void SegInterfaceNodes::listAllDeallocatable(void *param, NoteCallback note) {
	NodeTable *table = (NodeTable *)_mobj;
	for (int i = 0; i < table->max_entry; i++)
		if (ENTRY_IS_VALID(table, i))
			(*note) (param, make_reg(_segId, i));
}

void SegInterfaceNodes::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, NoteCallback note) {
	NodeTable *table = (NodeTable *)_mobj;
	Node *node = &(table->table[addr.offset]);

	if (!ENTRY_IS_VALID(table, addr.offset)) {
		fprintf(stderr, "Invalid node referenced for outgoing references: "PREG"\n", PRINT_REG(addr));
		return;
	}

	// We need all four here. Can't just stick with 'pred' OR 'succ' because node operations allow us
	// to walk around from any given node
	note(param, node->pred);
	note(param, node->succ);
	note(param, node->key);
	note(param, node->value);
}


//-------------------- hunk --------------------
class SegInterfaceHunk : public SegInterface {
public:
	SegInterfaceHunk(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_HUNK) {}
	void freeAtAddress(reg_t addr);
	void listAllDeallocatable(void *param, NoteCallback note);
};

void SegInterfaceHunk::freeAtAddress(reg_t sub_addr) {
	//sciprintf("  Request to free "PREG"\n", PRINT_REG(sub_addr));
	// STUB
}

void SegInterfaceHunk::listAllDeallocatable(void *param, NoteCallback note) {
	HunkTable *table = (HunkTable *)_mobj;
	for (int i = 0; i < table->max_entry; i++)
		if (ENTRY_IS_VALID(table, i))
			(*note) (param, make_reg(_segId, i));
}


//-------------------- dynamic memory --------------------
class SegInterfaceDynMem : public SegInterfaceBase {
public:
	SegInterfaceDynMem(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterfaceBase(segmgr, mobj, segId, MEM_OBJ_DYNMEM) {}
	void freeAtAddress(reg_t addr);
};

void SegInterfaceDynMem::freeAtAddress(reg_t sub_addr) {
	//sciprintf("  Request to free "PREG"\n", PRINT_REG(sub_addr));
	// STUB
}


SegInterface *SegManager::getSegInterface(SegmentId segid) {
	if (!check(segid))
		return NULL; // Invalid segment

	SegInterface *retval = NULL;
	MemObject *mobj = _heap[segid];
	switch (mobj->getType()) {
	case MEM_OBJ_SCRIPT:
		retval = new SegInterfaceScript(this, mobj, segid);
		break;
	case MEM_OBJ_CLONES:
		retval = new SegInterfaceClones(this, mobj, segid);
		break;
	case MEM_OBJ_LOCALS:
		retval = new SegInterfaceLocals(this, mobj, segid);
		break;
	case MEM_OBJ_STACK:
		retval = new SegInterfaceStack(this, mobj, segid);
		break;
	case MEM_OBJ_SYS_STRINGS:
		retval = new SegInterfaceSysStrings(this, mobj, segid);
		break;
	case MEM_OBJ_LISTS:
		retval = new SegInterfaceLists(this, mobj, segid);
		break;
	case MEM_OBJ_NODES:
		retval = new SegInterfaceNodes(this, mobj, segid);
		break;
	case MEM_OBJ_HUNK:
		retval = new SegInterfaceHunk(this, mobj, segid);
		break;
	case MEM_OBJ_DYNMEM:
		retval = new SegInterfaceDynMem(this, mobj, segid);
		break;
	case MEM_OBJ_STRING_FRAG:
		retval = new SegInterfaceStringFrag(this, mobj, segid);
		break;
	default:
		error("Improper segment interface for %d", mobj->getType());
	}

	return retval;
}

} // End of namespace Sci
