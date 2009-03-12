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

#define DEFAULT_SCRIPTS 32
#define DEFAULT_OBJECTS 8	    // default # of objects per script
#define DEFAULT_OBJECTS_INCREMENT 4 // Number of additional objects to instantiate if we're running out of them

//#define GC_DEBUG // Debug garbage collection
//#define GC_DEBUG_VERBOSE // Debug garbage verbosely

#undef DEBUG_SEG_MANAGER // Define to turn on debugging

#define GET_SEGID()	\
	if (flag == SCRIPT_ID) \
		id = segGet(id); \
		VERIFY(check(id), "invalid seg id");

#if 0
// Unreferenced - removed
#define VERIFY_MEM(mem_ptr, ret) \
	if (!(mem_ptr)) {\
		sciprintf( "%s, *d, no enough memory", __FILE__, __LINE__ ); \
		return ret; \
	}
#endif

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

MemObject *SegManager::allocNonscriptSegment(memObjType type, SegmentId *segid) {
	// Allocates a non-script segment
	int id;

	*segid = findFreeId(&id);
	return memObjAllocate(*segid, id, type);
}

SegManager::SegManager(bool sci1_1) {
	int i;

	// Initialise memory count
	mem_allocated = 0;

	id_seg_map = new IntMapper();
	reserved_id = INVALID_SCRIPT_ID;
	id_seg_map->checkKey(reserved_id, true);	// reserve 0 for seg_id
	reserved_id--; // reserved_id runs in the reversed direction to make sure no one will use it.

	heap_size = DEFAULT_SCRIPTS;
	heap = (MemObject **)sci_calloc(heap_size, sizeof(MemObject *));

	Clones_seg_id = 0;
	Lists_seg_id = 0;
	Nodes_seg_id = 0;
	Hunks_seg_id = 0;

	exports_wide = 0;
	isSci1_1 = sci1_1;

	// initialize the heap pointers
	for (i = 0; i < heap_size; i++) {
		heap[i] = NULL;
	}

	// gc initialisation
	gc_mark_bits = 0;
}

// Destroy the object, free the memorys if allocated before
SegManager::~SegManager() {
	int i;

	// Free memory
	for (i = 0; i < heap_size; i++) {
		if (heap[i])
			deallocate(i, false);
	}

	delete id_seg_map;

	free(heap);
}

// allocate a memory for script from heap
// Parameters: (EngineState *) s: The state to operate on
//             (int) script_nr: The script number to load
// Returns   : 0 - allocation failure
//             1 - allocated successfully
//             seg_id - allocated segment id
MemObject *SegManager::allocateScript(EngineState *s, int script_nr, int* seg_id) {
	int seg;
	bool was_added;
	MemObject* mem;

	seg = id_seg_map->checkKey(script_nr, true, &was_added);
	if (!was_added) {
		*seg_id = seg;
		return heap[*seg_id];
	}

	// allocate the MemObject
	mem = memObjAllocate(seg, script_nr, MEM_OBJ_SCRIPT);
	if (!mem) {
		sciprintf("%s, %d, Not enough memory, ", __FILE__, __LINE__);
		return NULL;
	}

	*seg_id = seg;
	return mem;
}

void SegManager::setScriptSize(MemObject *mem, EngineState *s, int script_nr) {
	Resource *script = s->resmgr->findResource(kResourceTypeScript, script_nr, 0);
	Resource *heap = s->resmgr->findResource(kResourceTypeHeap, script_nr, 0);

	mem->data.script.script_size = script->size;
	mem->data.script.heap_size = 0; // Set later

	if (!script || (s->version >= SCI_VERSION(1, 001, 000) && !heap)) {
		sciprintf("%s: failed to load %s\n", __FUNCTION__, !script ? "script" : "heap");
		return;
	}
	if (s->version < SCI_VERSION_FTU_NEW_SCRIPT_HEADER) {
		mem->data.script.buf_size = script->size + READ_LE_UINT16(script->data) * 2;
		//locals_size = READ_LE_UINT16(script->data) * 2;
	} else if (s->version < SCI_VERSION(1, 001, 000)) {
		mem->data.script.buf_size = script->size;
	} else {
		mem->data.script.buf_size = script->size + heap->size;
		mem->data.script.heap_size = heap->size;

		// Ensure that the start of the heap resource can be word-aligned.
		if (script->size & 2) {
			mem->data.script.buf_size++;
			mem->data.script.script_size++;
		}

		if (mem->data.script.buf_size > 65535) {
			sciprintf("Script and heap sizes combined exceed 64K.\n"
			          "This means a fundamental design bug was made in FreeSCI\n"
			          "regarding SCI1.1 games.\nPlease report this so it can be"
			          "fixed in the next major version!\n");
			return;
		}
	}
}

int SegManager::initialiseScript(MemObject *mem, EngineState *s, int script_nr) {
	// allocate the script.buf
	Script *scr;

	setScriptSize(mem, s, script_nr);
	mem->data.script.buf = (byte*) sci_malloc(mem->data.script.buf_size);

	dbgPrint("mem->data.script.buf ", mem->data.script.buf);
	if (!mem->data.script.buf) {
		freeScript(mem);
		sciprintf("SegManager: Not enough memory space for script size");
		mem->data.script.buf_size = 0;
		return 0;
	}

	// Initialize objects
	scr = &(mem->data.script);
	scr->objects = NULL;
	scr->objects_allocated = 0;
	scr->objects_nr = 0; // No objects recorded yet

	scr->locals_offset = 0;
	scr->locals_block = NULL;

	scr->code = NULL;
	scr->code_blocks_nr = 0;
	scr->code_blocks_allocated = 0;

	scr->nr = script_nr;
	scr->marked_as_deleted = 0;
	scr->relocated = 0;

	scr->obj_indices = new IntMapper();

	if (s->version >= SCI_VERSION(1, 001, 000))
		scr->heap_start = scr->buf + scr->script_size;
	else
		scr->heap_start = scr->buf;

	return 1;
}

int SegManager::deallocate(int seg, bool recursive) {
	MemObject *mobj;
	VERIFY(check(seg), "invalid seg id");
	int i;

	mobj = heap[seg];
	id_seg_map->removeKey(mobj->segmgr_id);

	switch (mobj->type) {
	case MEM_OBJ_SCRIPT:
		freeScript(mobj);

		mobj->data.script.buf = NULL;
		if (recursive && mobj->data.script.locals_segment)
			deallocate(mobj->data.script.locals_segment, recursive);
		break;

	case MEM_OBJ_LOCALS:
		free(mobj->data.locals.locals);
		mobj->data.locals.locals = NULL;
		break;

	case MEM_OBJ_DYNMEM:
		free(mobj->data.dynmem.description);
		mobj->data.dynmem.description = NULL;
		free(mobj->data.dynmem.buf);
		mobj->data.dynmem.buf = NULL;
		break;
	case MEM_OBJ_SYS_STRINGS: 
		for (i = 0; i < SYS_STRINGS_MAX; i++) {
			SystemString *str = &mobj->data.sys_strings.strings[i];
			if (str->name) {
				free(str->name);
				str->name = NULL;

				free(str->value);
				str->value = NULL;

				str->max_size = 0;
			}
		}
		break;
	case MEM_OBJ_STACK:
		free(mobj->data.stack.entries);
		mobj->data.stack.entries = NULL;
		break;
	case MEM_OBJ_LISTS:
		free(mobj->data.lists.table);
		mobj->data.lists.table = NULL;
		mobj->data.lists.entries_nr = mobj->data.lists.max_entry = 0;
		break;
	case MEM_OBJ_NODES:
		free(mobj->data.nodes.table);
		mobj->data.nodes.table = NULL;
		mobj->data.nodes.entries_nr = mobj->data.nodes.max_entry = 0;
		break;
	case MEM_OBJ_CLONES:
		free(mobj->data.clones.table);
		mobj->data.clones.table = NULL;
		mobj->data.clones.entries_nr = mobj->data.clones.max_entry = 0;
		break;
	case MEM_OBJ_HUNK:
		free(mobj->data.hunks.table);
		mobj->data.hunks.table = NULL;
		mobj->data.hunks.entries_nr = mobj->data.hunks.max_entry = 0;
		break;
	case MEM_OBJ_RESERVED:
		free(mobj->data.reserved);
		break;
	case MEM_OBJ_STRING_FRAG:
		break;
	default:
		fprintf(stderr, "Deallocating segment type %d not supported!\n",
		        mobj->type);
		BREAKPOINT();
	}

	free(mobj);
	heap[seg] = NULL;

	return 1;
}

int SegManager::scriptMarkedDeleted(int script_nr) {
	Script *scr;
	int seg = segGet(script_nr);
	VERIFY(check(seg), "invalid seg id");

	scr = &(heap[seg]->data.script);
	return scr->marked_as_deleted;
}

void SegManager::markScriptDeleted(int script_nr) {
	Script *scr;
	int seg = segGet(script_nr);
	VERIFY(check(seg), "invalid seg id");

	scr = &(heap[seg]->data.script);
	scr->marked_as_deleted = 1;
}

void SegManager::unmarkScriptDeleted(int script_nr) {
	Script *scr;
	int seg = segGet(script_nr);
	VERIFY(check(seg), "invalid seg id");

	scr = &(heap[seg]->data.script);
	scr->marked_as_deleted = 0;
}

int SegManager::scriptIsMarkedAsDeleted(SegmentId seg) {
	Script *scr;

	if (!check(seg))
		return 0;

	if (heap[seg]->type != MEM_OBJ_SCRIPT)
		return 0;

	scr = &(heap[seg]->data.script);

	return scr->marked_as_deleted;
}


int SegManager::deallocateScript(int script_nr) {
	int seg = segGet(script_nr);

	deallocate(seg, true);

	return 1;
}

MemObject *SegManager::memObjAllocate(SegmentId segid, int hash_id, memObjType type) {
	MemObject *mem = (MemObject *)sci_calloc(sizeof(MemObject), 1);
	if (!mem) {
		sciprintf("SegManager: invalid mem_obj ");
		return NULL;
	}

	if (segid >= heap_size) {
		void *temp;
		int oldhs = heap_size;

		if (segid >= heap_size * 2) {
			sciprintf("SegManager: hash_map error or others??");
			return NULL;
		}
		heap_size *= 2;
		temp = sci_realloc((void *)heap, heap_size * sizeof(MemObject *));
		if (!temp) {
			sciprintf("SegManager: Not enough memory space for script size");
			return NULL;
		}
		heap = (MemObject **)temp;

		// Clear pointers
		memset(heap + oldhs, 0, sizeof(MemObject *) * (heap_size - oldhs));
	}

	mem->segmgr_id = hash_id;
	mem->type = type;

	// hook it to the heap
	heap[segid] = mem;
	return mem;
}

/* No longer in use?
void SegManager::sm_object_init(Object *object) {
	if (!object)
		return;
	object->variables_nr = 0;
	object->variables = NULL;
};*/

void SegManager::freeScript(MemObject *mem) {
	if (!mem)
		return;
	if (mem->data.script.buf) {
		free(mem->data.script.buf);
		mem->data.script.buf = NULL;
		mem->data.script.buf_size = 0;
	}
	if (mem->data.script.objects) {
		int i;

		for (i = 0; i < mem->data.script.objects_nr; i++) {
			Object* object = &mem->data.script.objects[i];
			if (object->variables) {
				free(object->variables);
				object->variables = NULL;
				object->variables_nr = 0;
			}
		}
		free(mem->data.script.objects);
		mem->data.script.objects = NULL;
		mem->data.script.objects_nr = 0;
	}

	delete mem->data.script.obj_indices;
	free(mem->data.script.code);
}

// memory operations
#if 0
// Unreferenced - removed
static void SegManager::sm_mset(int offset, int c, size_t n, int id, int flag) {
	MemObject *mem_obj;
	GET_SEGID();
	mem_obj = heap[id];
	switch (mem_obj->type) {
	case MEM_OBJ_SCRIPT:
		if (mem_obj->data.script.buf) {
			memset(mem_obj->data.script.buf + offset, c, n);
		}
		break;
	case MEM_OBJ_CLONES:
		sciprintf("memset for clones haven't been implemented\n");
		break;
	default:
		sciprintf("unknown mem obj type\n");
		break;
	}
}
#endif

#if 0
// Unreferenced - removed
static void SegManager::sm_mcpy_in_in(int dst, const int src, size_t n, int id, int flag) {
	MemObject *mem_obj;
	GET_SEGID();
	mem_obj = heap[id];
	switch (mem_obj->type) {
	case MEM_OBJ_SCRIPT:
		if (mem_obj->data.script.buf) {
			memcpy(mem_obj->data.script.buf + dst, mem_obj->data.script.buf + src, n);
		}
		break;
	case MEM_OBJ_CLONES:
		sciprintf("memcpy for clones haven't been implemented\n");
		break;
	default:
		sciprintf("unknown mem obj type\n");
		break;
	}
}
#endif

void SegManager::mcpyInOut(int dst, const void *src, size_t n, int id, int flag) {
	MemObject *mem_obj;
	GET_SEGID();
	mem_obj = heap[id];
	switch (mem_obj->type) {
	case MEM_OBJ_SCRIPT:
		if (mem_obj->data.script.buf) {
			memcpy(mem_obj->data.script.buf + dst, src, n);
		}
		break;
	case MEM_OBJ_CLONES:
		sciprintf("memcpy for clones hasn't been implemented yet\n");
		break;
	default:
		sciprintf("unknown mem obj type\n");
		break;
	}
}

#if 0
// Unreferenced - removed
static void SegManager::sm_mcpy_out_in(void *dst, const int src, size_t n, int id, int flag) {
	MemObject *mem_obj;
	GET_SEGID();
	mem_obj = heap[id];
	switch (mem_obj->type) {
	case MEM_OBJ_SCRIPT:
		if (mem_obj->data.script.buf) {
			memcpy(dst, mem_obj->data.script.buf + src, n);
		}
		break;
	case MEM_OBJ_CLONES:
		sciprintf("memcpy for clones hasn't been implemented yet\n");
		break;
	default:
		sciprintf("unknown mem obj type\n");
		break;
	}
}
#endif

int16 SegManager::getHeap(reg_t reg) {
	MemObject *mem_obj;
	memObjType mem_type;

	VERIFY(check(reg.segment), "Invalid seg id");
	mem_obj = heap[reg.segment];
	mem_type = mem_obj->type;

	switch (mem_type) {
	case MEM_OBJ_SCRIPT:
		VERIFY(reg.offset + 1 < (uint16)mem_obj->data.script.buf_size, "invalid offset\n");
		return (mem_obj->data.script.buf[reg.offset] | (mem_obj->data.script.buf[reg.offset+1]) << 8);
	case MEM_OBJ_CLONES:
		sciprintf("memcpy for clones hasn't been implemented yet\n");
		break;
	default:
		sciprintf("unknown mem obj type\n");
		break;
	}
	return 0; // never get here
}

#if 0
// Unreferenced - removed
void SegManager::sm_put_heap(reg_t reg, int16 value) {
	MemObject *mem_obj;
	memObjType mem_type;

	VERIFY(check(reg.segment), "Invalid seg id");
	mem_obj = heap[reg.segment];
	mem_type = mem_obj->type;

	switch (mem_type) {
	case MEM_OBJ_SCRIPT:
		VERIFY(reg.offset + 1 < (uint16)mem_obj->data.script.buf_size, "invalid offset");
		mem_obj->data.script.buf[reg.offset] = value & 0xff;
		mem_obj->data.script.buf[reg.offset + 1] = value >> 8;
		break;
	case MEM_OBJ_CLONES:
		sciprintf("memcpy for clones haven't been implemented\n");
		break;
	default:
		sciprintf("unknown mem obj type\n");
		break;
	}
}
#endif

// return the seg if script_id is valid and in the map, else -1
int SegManager::segGet(int script_id) {
	return id_seg_map->checkKey(script_id, false);
}

// validate the seg
// return:
//	false - invalid seg
//	true  - valid seg
bool SegManager::check(int seg) {
	if (seg < 0 || seg >= heap_size) {
		return false;
	}
	if (!heap[seg]) {
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
	if (flag == SCRIPT_ID)
		id = segGet(id);
	VERIFY(check(id), "invalid seg id");
	heap[id]->data.script.lockers++;
}

void SegManager::decrementLockers(int id, idFlag flag) {
	if (flag == SCRIPT_ID)
		id = segGet(id);
	VERIFY(check(id), "invalid seg id");

	if (heap[id]->data.script.lockers > 0)
		heap[id]->data.script.lockers--;
}

int SegManager::getLockers(int id, idFlag flag) {
	if (flag == SCRIPT_ID)
		id = segGet(id);
	VERIFY(check(id), "invalid seg id");

	return heap[id]->data.script.lockers;
}

void SegManager::setLockers(int lockers, int id, idFlag flag) {
	if (flag == SCRIPT_ID)
		id = segGet(id);
	VERIFY(check(id), "invalid seg id");
	heap[id]->data.script.lockers = lockers;
}

void SegManager::setExportTableOffset(int offset, int id, idFlag flag) {
	Script *scr = &(heap[id]->data.script);

	GET_SEGID();
	if (offset) {
		scr->export_table = (uint16 *)(scr->buf + offset + 2);
		scr->exports_nr = READ_LE_UINT16((byte *)(scr->export_table - 1));
	} else {
		scr->export_table = NULL;
		scr->exports_nr = 0;
	}
}

#if 0
// Unreferenced - removed
int SegManager::sm_hash_segment_data(int id) {
	int i, len, hash_code = 0x55555555;
	char *buf;

	if (heap[id]->type == MEM_OBJ_LISTS)
		return 0;
	if (heap[id]->type == MEM_OBJ_NODES)
		return 0;
	if (heap[id]->type == MEM_OBJ_CLONES)
		return 0;
	buf = (char *)dereference(make_reg(id, 0), &len);

	for (i = 0; i < len; i++)
		hash_code = (hash_code * 19) + *(buf + i);

	return hash_code;
}
#endif

void SegManager::setExportWidth(int flag) {
	exports_wide = flag;
}

#if 0
// Unreferenced - removed
static uint16 *SegManager::sm_get_export_table_offset(int id, int flag, int *max) {
	GET_SEGID();
	if (max)
		*max = heap[id]->data.script.exports_nr;

	return heap[id]->data.script.export_table;
}
#endif

void SegManager::setSynonymsOffset(int offset, int id, idFlag flag) {
	GET_SEGID();
	heap[id]->data.script.synonyms = heap[id]->data.script.buf + offset;
}

byte *SegManager::getSynonyms(int id, idFlag flag) {
	GET_SEGID();
	return heap[id]->data.script.synonyms;
}

void SegManager::setSynonymsNr(int nr, int id, idFlag flag) {
	GET_SEGID();
	heap[id]->data.script.synonyms_nr = nr;
}

int SegManager::getSynonymsNr(int id, idFlag flag) {
	GET_SEGID();
	return heap[id]->data.script.synonyms_nr;
}

#if 0
// Unreferenced - removed
static int SegManager::sm_get_heappos(int id, int flag) {
	GET_SEGID();
	return 0;
}
#endif

#if 0
// Unreferenced - removed
static void SegManager::sm_set_variables(reg_t reg, int obj_index, reg_t variable_reg, int variable_index) {
	Script *script;
	VERIFY(check(reg.segment), "invalid seg id");
	VERIFY(heap[reg.segment], "invalid mem");

	script = &(heap[reg.segment]->data.script);

	VERIFY(obj_index < script->objects_nr, "Invalid obj_index");

	VERIFY(variable_index >= 0 && variable_index < script->objects[obj_index].variables_nr, "Attempt to write to invalid variable number");

	script->objects[obj_index].variables[variable_index] = variable_reg;
}
#endif

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
		block[index].offset += heap[segment]->data.script.script_size;

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
	MemObject *mobj = heap[location.segment];
	Script *scr;
	int index;

	VERIFY(!(location.segment >= heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to add a code block to non-script\n");

	scr = &(mobj->data.script);

	if (++scr->code_blocks_nr > scr->code_blocks_allocated) {
		scr->code_blocks_allocated += DEFAULT_OBJECTS_INCREMENT;
		scr->code = (CodeBlock *)sci_realloc(scr->code, scr->code_blocks_allocated * sizeof(CodeBlock));
	}

	index = scr->code_blocks_nr - 1;
	scr->code[index].pos = location;
	scr->code[index].size = READ_LE_UINT16(scr->buf + location.offset - 2);
}

void SegManager::scriptRelocate(reg_t block) {
	MemObject *mobj = heap[block.segment];
	Script *scr;
	int count;
	int i;

	VERIFY(!(block.segment >= heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt relocate non-script\n");

	scr = &(mobj->data.script);

	VERIFY(block.offset < (uint16)scr->buf_size && READ_LE_UINT16(scr->buf + block.offset) * 2 + block.offset < (uint16)scr->buf_size,
	       "Relocation block outside of script\n");

	count = READ_LE_UINT16(scr->buf + block.offset);

	for (i = 0; i <= count; i++) {
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
	MemObject *mobj = heap[block.segment];
	Script *scr;
	int count;
	int i;

	VERIFY(!(block.segment >= heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt relocate non-script\n");

	scr = &(mobj->data.script);

	VERIFY(block.offset < (uint16)scr->heap_size && READ_LE_UINT16(scr->heap_start + block.offset) * 2 + block.offset < (uint16)scr->buf_size,
	       "Relocation block outside of script\n");

	if (scr->relocated)
		return;
	scr->relocated = 1;
	count = READ_LE_UINT16(scr->heap_start + block.offset);

	for (i = 0; i < count; i++) {
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
	MemObject *mobj = heap[obj_pos.segment];
	Script *scr;
	Object *obj;
	int id;
	unsigned int base = obj_pos.offset - SCRIPT_OBJECT_MAGIC_OFFSET;
	reg_t temp;

	VERIFY(!(obj_pos.segment >= heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to initialize object in non-script\n");

	scr = &(mobj->data.script);

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
	MemObject *mobj = heap[obj_pos.segment];
	Script *scr;
	Object *obj;
	int id;
	int base = obj_pos.offset;

	VERIFY(!(obj_pos.segment >= heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to initialize object in non-script\n");

	scr = &(mobj->data.script);

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
		functions_nr = *funct_area;
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
			mobj = heap[scr->locals_segment];
			VERIFY(mobj != NULL, "Re-used locals segment was NULL'd out");
			VERIFY(mobj->type == MEM_OBJ_LOCALS, "Re-used locals segment did not consist of local variables");
			VERIFY(mobj->data.locals.script_id == scr->nr, "Re-used locals segment belonged to other script");
		} else
			mobj = allocNonscriptSegment(MEM_OBJ_LOCALS, &scr->locals_segment);

		locals = scr->locals_block = &(mobj->data.locals);
		locals->script_id = scr->nr;
		locals->locals = (reg_t *)sci_calloc(count, sizeof(reg_t));
		locals->nr = count;

		return locals;
	}
}

void SegManager::scriptInitialiseLocalsZero(SegmentId seg, int count) {
	MemObject *mobj = heap[seg];
	Script *scr;

	VERIFY(!(seg >= heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to initialize locals in non-script\n");

	scr = &(mobj->data.script);

	scr->locals_offset = -count * 2; // Make sure it's invalid

	allocLocalsSegment(scr, count);
}

void SegManager::scriptInitialiseLocals(reg_t location) {
	MemObject *mobj = heap[location.segment];
	unsigned int count;
	Script *scr;
	LocalVariables *locals;

	VERIFY(!(location.segment >= heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to initialize locals in non-script\n");

	scr = &(mobj->data.script);

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

	locals = allocLocalsSegment(scr, count);
	if (locals) {
		uint i;
		byte *base = (byte *)(scr->buf + location.offset);

		for (i = 0; i < count; i++)
			locals->locals[i].offset = READ_LE_UINT16(base + i * 2);
	}
}

void SegManager::scriptRelocateExportsSci11(int seg) {
	MemObject *mobj = heap[seg];
	Script *scr;
	int i;
	int location;

	VERIFY(!(seg >= heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to relocate exports in non-script\n");

	scr = &(mobj->data.script);
	for (i = 0; i < scr->exports_nr; i++) {
		/* We are forced to use an ugly heuristic here to distinguish function
		   exports from object/class exports. The former kind points into the
		   script resource, the latter into the heap resource.  */
		location = READ_LE_UINT16((byte *)(scr->export_table + i));
		if (READ_LE_UINT16(scr->heap_start + location) == SCRIPT_OBJECT_MAGIC_NUMBER) {
			WRITE_LE_UINT16((byte *)(scr->export_table + i), location + scr->heap_start - scr->buf);
		} else {
			// Otherwise it's probably a function export,
			// and we don't need to do anything.
		}
	}
}

void SegManager::scriptInitialiseObjectsSci11(EngineState *s, int seg) {
	MemObject *mobj = heap[seg];
	Script *scr;
	byte *seeker;

	VERIFY(!(seg >= heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to relocate exports in non-script\n");

	scr = &(mobj->data.script);
	seeker = scr->heap_start + 4 + READ_LE_UINT16(scr->heap_start + 2) * 2;

	while (READ_LE_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		if (READ_LE_UINT16(seeker + 14) & SCRIPT_INFO_CLASS) {
			int classpos = seeker - scr->buf;
			int species = READ_LE_UINT16(seeker + 10);

			if (species < 0 || species >= s->classtable_size) {
				sciprintf("Invalid species %d(0x%x) not in interval [0,%d) while instantiating script %d\n",
				          species, species, s->classtable_size, scr->nr);
				script_debug_flag = script_error_flag = 1;
				return;
			}

			s->classtable[species].script = scr->nr;
			s->classtable[species].reg.segment = seg;
			s->classtable[species].reg.offset = classpos;
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
	MemObject *mobj = heap[seg];
	Script *scr;

	VERIFY(!(seg >= heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to free unused objects in non-script\n");

	scr = &(mobj->data.script);
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
	MemObject *memobj = allocNonscriptSegment(MEM_OBJ_STACK, segid);
	dstack_t *retval = &(memobj->data.stack);

	retval->entries = (reg_t*)sci_calloc(size, sizeof(reg_t));
	retval->nr = size;

	return retval;
}

SystemStrings *SegManager::allocateSysStrings(SegmentId *segid) {
	MemObject *memobj = allocNonscriptSegment(MEM_OBJ_SYS_STRINGS, segid);
	SystemStrings *retval = &(memobj->data.sys_strings);

	memset(retval, 0, sizeof(SystemString)*SYS_STRINGS_MAX);

	return retval;
}

SegmentId SegManager::allocateStringFrags() {
	SegmentId segid;
	
	allocNonscriptSegment(MEM_OBJ_STRING_FRAG, &segid);

	return segid;
}

#if 0
// Unreferenced - removed
SegmentId SegManager::sm_allocate_reserved_segment(char *src_name) {
	SegmentId segid;
	MemObject *memobj = allocNonscriptSegment(MEM_OBJ_RESERVED, &segid);
	char *name = sci_strdup(src_name);

	memobj->data.reserved = name;

	return segid;
}
#endif

uint16 SegManager::validateExportFunc(int pubfunct, int seg) {
	Script* script;
	uint16 offset;
	VERIFY(check(seg), "invalid seg id");
	VERIFY(heap[seg]->type == MEM_OBJ_SCRIPT, "Can only validate exports on scripts");

	script = &heap[seg]->data.script;
	if (script->exports_nr <= pubfunct) {
		sciprintf("pubfunct is invalid");
		return 0;
	}

	if (exports_wide)
		pubfunct *= 2;
	offset = READ_LE_UINT16((byte *)(script->export_table + pubfunct));
	VERIFY(offset < script->buf_size, "invalid export function pointer");

	return offset;
}

void SegManager::free_hunk_entry(reg_t addr) {
	free_Hunk(addr);
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

void _clone_cleanup(Clone *clone) {
	if (clone->variables)
		free(clone->variables); // Free the dynamically allocated memory part
}

void _hunk_cleanup(Hunk *hunk) {
	if (hunk->mem)
		free(hunk->mem);
}

DEFINE_HEAPENTRY(List, 8, 4)
DEFINE_HEAPENTRY(Node, 32, 16)
DEFINE_HEAPENTRY_WITH_CLEANUP(Clone, 16, 4, _clone_cleanup)
DEFINE_HEAPENTRY_WITH_CLEANUP(Hunk, 4, 4, _hunk_cleanup)

#define DEFINE_ALLOC_DEALLOC(TYPE, SEGTYPE, PLURAL) \
TYPE *SegManager::alloc_##TYPE(reg_t *addr) {											 \
	MemObject *mobj;									  \
	TYPE##Table *table;									  \
	int offset;										  \
												  \
	if (!TYPE##s_seg_id) {								  \
		mobj = allocNonscriptSegment(SEGTYPE, &(TYPE##s_seg_id));		  \
		init_##TYPE##_table(&(mobj->data.PLURAL));					  \
	} else											  \
		mobj = heap[TYPE##s_seg_id];					  \
												  \
	table = &(mobj->data.PLURAL);								  \
	offset = Sci::alloc_##TYPE##_entry(table);							  \
												  \
	*addr = make_reg(TYPE##s_seg_id, offset);						  \
	return &(mobj->data.PLURAL.table[offset].entry);					  \
}												  \
												  \
void	SegManager::free_##TYPE(reg_t addr) {					  \
	MemObject *mobj = GET_SEGMENT(*this, addr.segment, SEGTYPE);				  \
												  \
	if (!mobj) {										  \
		sciprintf("Attempt to free " #TYPE " from address "PREG": Invalid segment type\n", PRINT_REG(addr));							  \
		return;										  \
	}											  \
												  \
	Sci::free_##TYPE##_entry(&(mobj->data.PLURAL), addr.offset);					  \
}

DEFINE_ALLOC_DEALLOC(Clone, MEM_OBJ_CLONES, clones)
DEFINE_ALLOC_DEALLOC(List, MEM_OBJ_LISTS, lists)
DEFINE_ALLOC_DEALLOC(Node, MEM_OBJ_NODES, nodes)
DEFINE_ALLOC_DEALLOC(Hunk, MEM_OBJ_HUNK, hunks)

byte *SegManager::dereference(reg_t pointer, int *size) {
	MemObject *mobj;
	byte *base = NULL;
	int count;

	if (!pointer.segment || (pointer.segment >= heap_size) || !heap[pointer.segment]) {
		sciprintf("Error: Attempt to dereference invalid pointer "PREG"!\n",
		          PRINT_REG(pointer));
		return NULL; /* Invalid */
	}

	mobj = heap[pointer.segment];

	switch (mobj->type) {
	case MEM_OBJ_SCRIPT:
		if (pointer.offset > mobj->data.script.buf_size) {
			sciprintf("Error: Attempt to dereference invalid pointer "PREG" into script segment (script size=%d)\n",
			          PRINT_REG(pointer), (uint)mobj->data.script.buf_size);
			return NULL;
		}
		if (size)
			*size = mobj->data.script.buf_size - pointer.offset;
		return (byte *)(mobj->data.script.buf + pointer.offset);
		break;

	case MEM_OBJ_LOCALS:
		count = mobj->data.locals.nr * sizeof(reg_t);
		base = (byte *)mobj->data.locals.locals;
		break;

	case MEM_OBJ_STACK:
		count = mobj->data.stack.nr * sizeof(reg_t);
		base = (byte *)mobj->data.stack.entries;
		break;

	case MEM_OBJ_DYNMEM:
		count = mobj->data.dynmem.size;
		base = (byte *)mobj->data.dynmem.buf;
		break;

	case MEM_OBJ_SYS_STRINGS:
		if (size)
			*size = mobj->data.sys_strings.strings[pointer.offset].max_size;
		if (pointer.offset < SYS_STRINGS_MAX && mobj->data.sys_strings.strings[pointer.offset].name)
			return (byte *)(mobj->data.sys_strings.strings[pointer.offset].value);
		else {
			sciprintf("Error: Attempt to dereference invalid pointer "PREG"!\n",
			          PRINT_REG(pointer));
			return NULL;
		}

	case MEM_OBJ_RESERVED:
		sciprintf("Error: Trying to dereference pointer "PREG" to reserved segment `%s'!\n",
		          PRINT_REG(pointer),
		          mobj->data.reserved);
		return NULL;
		break;

	default:
		sciprintf("Error: Trying to dereference pointer "PREG" to inappropriate"
		          " segment!\n",
		          PRINT_REG(pointer));
		return NULL;
	}

	if (size)
		*size = count;

	return
	    base + pointer.offset;
}

unsigned char *SegManager::allocDynmem(int size, const char *descr, reg_t *addr) {
	SegmentId seg;
	MemObject *mobj = allocNonscriptSegment(MEM_OBJ_DYNMEM, &seg);
	*addr = make_reg(seg, 0);

	mobj->data.dynmem.size = size;

	if (size == 0)
		mobj->data.dynmem.buf = NULL;
	else
		mobj->data.dynmem.buf = (byte*) sci_malloc(size);

	mobj->data.dynmem.description = sci_strdup(descr);

	return (unsigned char *)(mobj->data.dynmem.buf);
}

const char *SegManager::getDescription(reg_t addr) {
	MemObject *mobj = heap[addr.segment];

	if (addr.segment >= heap_size)
		return "";

	switch (mobj->type) {
	case MEM_OBJ_DYNMEM:
		return mobj->data.dynmem.description;
	default:
		return "";
	}
}

int SegManager::freeDynmem(reg_t addr) {
	if (addr.segment <= 0 || addr.segment >= heap_size || !heap[addr.segment] || heap[addr.segment]->type != MEM_OBJ_DYNMEM)
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
SegInterface::SegInterface(SegManager *segmgr, MemObject *mobj, SegmentId segId, memObjType typeId) :
	_segmgr(segmgr), _mobj(mobj), _segId(segId), _typeId(typeId) {
	VERIFY(_mobj->type == _typeId, "Invalid MemObject type");
}

reg_t SegInterface::findCanonicAddress(reg_t addr) {
	return addr;
}

void SegInterface::freeAtAddress(reg_t sub_addr) {
}

void SegInterface::listAllDeallocatable(void *param, void (*note)(void *param, reg_t addr)) {
}

void SegInterface::listAllOutgoingReferences(EngineState *s, reg_t object, void *param, void (*note)(void *param, reg_t addr)) {
}


//-------------------- base --------------------
class SegInterfaceBase : public SegInterface {
protected:
	SegInterfaceBase(SegManager *segmgr, MemObject *mobj, SegmentId segId, memObjType typeId) :
		SegInterface(segmgr, mobj, segId, typeId) {}
public:
	reg_t findCanonicAddress(reg_t addr);
	void listAllDeallocatable(void *param, void (*note)(void *param, reg_t addr));
};

reg_t SegInterfaceBase::findCanonicAddress(reg_t addr) {
	addr.offset = 0;
	return addr;
}

void SegInterfaceBase::listAllDeallocatable(void *param, void (*note)(void *param, reg_t addr)) {
	(*note)(param, make_reg(_segId, 0));
}


//-------------------- script --------------------
class SegInterfaceScript : public SegInterfaceBase {
public:
	SegInterfaceScript(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterfaceBase(segmgr, mobj, segId, MEM_OBJ_SCRIPT) {}
	void freeAtAddress(reg_t addr);
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void *param, reg_t addr));
};

void SegInterfaceScript::freeAtAddress(reg_t addr) {
	Script *script = &(_mobj->data.script);
	/*
		sciprintf("[GC] Freeing script "PREG"\n", PRINT_REG(addr));
		if (script->locals_segment)
			sciprintf("[GC] Freeing locals %04x:0000\n", script->locals_segment);
	*/

	if (script->marked_as_deleted)
		_segmgr->deallocateScript(script->nr);
}

void SegInterfaceScript::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void *param, reg_t addr)) {
	Script *script = &(_mobj->data.script);

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


#define LIST_ALL_DEALLOCATABLE(kind, kind_field) \
	kind##Table * table = &(_mobj->data.kind_field);		\
	int i;								\
									\
	for (i = 0; i < table->max_entry; i++)				\
		if (ENTRY_IS_VALID(table, i))				\
			(*note) (param, make_reg(_segId, i));


//-------------------- clones --------------------
class SegInterfaceClones : public SegInterface {
public:
	SegInterfaceClones(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_CLONES) {}
	void freeAtAddress(reg_t addr);
	void listAllDeallocatable(void *param, void (*note)(void *param, reg_t addr));
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void *param, reg_t addr));
};

void SegInterfaceClones::listAllDeallocatable(void *param, void (*note)(void *param, reg_t addr)) {
	LIST_ALL_DEALLOCATABLE(Clone, clones);
}

void SegInterfaceClones::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void *param, reg_t addr)) {
	CloneTable *clone_table = &(_mobj->data.clones);
	Clone *clone;
	int i;

	assert(addr.segment == _segId);

	if (!(ENTRY_IS_VALID(clone_table, addr.offset))) {
		fprintf(stderr, "Unexpected request for outgoing references from clone at "PREG"\n", PRINT_REG(addr));
//		BREAKPOINT();
		return;
	}

	clone = &(clone_table->table[addr.offset].entry);

	// Emit all member variables (including references to the 'super' delegate)
	for (i = 0; i < clone->variables_nr; i++)
		(*note)(param, clone->variables[i]);

	// Note that this also includes the 'base' object, which is part of the script and therefore also emits the locals.
	(*note)(param, clone->pos);
	//sciprintf("[GC] Reporting clone-pos "PREG"\n", PRINT_REG(clone->pos));
}

void SegInterfaceClones::freeAtAddress(reg_t addr) {
	Object *victim_obj;

	assert(addr.segment == _segId);

	victim_obj = &(_mobj->data.clones.table[addr.offset].entry);

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
	_segmgr->free_Clone(addr);
}


//-------------------- locals --------------------
class SegInterfaceLocals : public SegInterface {
public:
	SegInterfaceLocals(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_LOCALS) {}
	reg_t findCanonicAddress(reg_t addr);
	void freeAtAddress(reg_t addr);
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void *param, reg_t addr));
};

reg_t SegInterfaceLocals::findCanonicAddress(reg_t addr) {
	LocalVariables *locals = &(_mobj->data.locals);
	// Reference the owning script
	SegmentId owner_seg = _segmgr->segGet(locals->script_id);

	assert(owner_seg >= 0);

	return make_reg(owner_seg, 0);
}

void SegInterfaceLocals::freeAtAddress(reg_t sub_addr) {
	//sciprintf("  Request to free "PREG"\n", PRINT_REG(sub_addr));
	// STUB
}

void SegInterfaceLocals::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
	LocalVariables *locals = &(_mobj->data.locals);
	int i;

	assert(addr.segment == _segId);

	for (i = 0; i < locals->nr; i++)
		(*note)(param, locals->locals[i]);
}


//-------------------- stack --------------------
class SegInterfaceStack : public SegInterface {
public:
	SegInterfaceStack(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_STACK) {}
	reg_t findCanonicAddress(reg_t addr);
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void *param, reg_t addr));
};

reg_t SegInterfaceStack::findCanonicAddress(reg_t addr) {
	addr.offset = 0;
	return addr;
}

void SegInterfaceStack::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
	int i;
	fprintf(stderr, "Emitting %d stack entries\n", _mobj->data.stack.nr);
	for (i = 0; i < _mobj->data.stack.nr; i++)
		(*note)(param, _mobj->data.stack.entries[i]);
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
	void listAllDeallocatable(void *param, void (*note)(void *param, reg_t addr));
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void *param, reg_t addr));
};

void SegInterfaceLists::freeAtAddress(reg_t sub_addr) {
	_segmgr->free_List(sub_addr);
}

void SegInterfaceLists::listAllDeallocatable(void *param, void (*note)(void*param, reg_t addr)) {
	LIST_ALL_DEALLOCATABLE(List, lists);
}

void SegInterfaceLists::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
	ListTable *table = &(_mobj->data.lists);
	List *list = &(table->table[addr.offset].entry);

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
	void listAllDeallocatable(void *param, void (*note)(void *param, reg_t addr));
	void listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void *param, reg_t addr));
};

void SegInterfaceNodes::freeAtAddress(reg_t sub_addr) {
	_segmgr->free_Node(sub_addr);
}

void SegInterfaceNodes::listAllDeallocatable(void *param, void (*note)(void*param, reg_t addr)) {
	LIST_ALL_DEALLOCATABLE(Node, nodes);
}

void SegInterfaceNodes::listAllOutgoingReferences(EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
	NodeTable *table = &(_mobj->data.nodes);
	Node *node = &(table->table[addr.offset].entry);

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
	void listAllDeallocatable(void *param, void (*note)(void *param, reg_t addr));
};

void SegInterfaceHunk::freeAtAddress(reg_t sub_addr) {
	//sciprintf("  Request to free "PREG"\n", PRINT_REG(sub_addr));
	// STUB
}

void SegInterfaceHunk::listAllDeallocatable(void *param, void (*note)(void*param, reg_t addr)) {
	LIST_ALL_DEALLOCATABLE(Hunk, hunks);
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


//-------------------- reserved --------------------
class SegInterfaceReserved : public SegInterface {
public:
	SegInterfaceReserved(SegManager *segmgr, MemObject *mobj, SegmentId segId) :
		SegInterface(segmgr, mobj, segId, MEM_OBJ_RESERVED) {}
};


SegInterface *SegManager::getSegInterface(SegmentId segid) {
	if (!check(segid))
		return NULL; // Invalid segment

	SegInterface *retval = NULL;
	MemObject *mobj = heap[segid];
	switch (mobj->type) {
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
	case MEM_OBJ_RESERVED:
		retval = new SegInterfaceReserved(this, mobj, segid);
		break;
	default:
		error("Improper segment interface for %d", mobj->type);
	}

	return retval;
}

} // End of namespace Sci
