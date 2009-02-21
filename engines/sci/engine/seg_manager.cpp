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
#include "sci/include/sciresource.h"
#include "sci/include/versions.h"
#include "sci/include/engine.h"

namespace Sci {

//#define GC_DEBUG*/ // Debug garbage collection
//#define GC_DEBUG_VERBOSE*/ // Debug garbage verbosely

#define SM_MEMORY_POISON	// Poison memory upon deallocation

mem_obj_t* mem_obj_allocate(seg_manager_t *self, seg_id_t segid, int hash_id, mem_obj_enum type);

#undef DEBUG_SEG_MANAGER // Define to turn on debugging

#define GET_SEGID() 	\
	if (flag == SCRIPT_ID) \
		id = sm_seg_get (self, id); \
		VERIFY(sm_check(self, id), "invalid seg id");

#define VERIFY_MEM(mem_ptr, ret) \
	if (!(mem_ptr)) {\
		sciprintf( "%s, *d, no enough memory", __FILE__, __LINE__ ); \
		return ret; \
	}

#define INVALID_SCRIPT_ID -1

void dbg_print(const char* msg, void *i) {
#ifdef DEBUG_SEG_MANAGER
	char buf[1000];
	sprintf(buf, "%s = [0x%x], dec:[%d]", msg, i, i);
	perror(buf);
#endif
}

//-- forward declarations --

void sm_script_initialise_locals_zero(seg_manager_t *self, seg_id_t seg, int count);
void sm_script_initialise_locals(seg_manager_t *self, reg_t location);
static int _sm_deallocate(seg_manager_t* self, int seg, int recursive);
static hunk_t *sm_alloc_hunk(seg_manager_t *self, reg_t *);
static void sm_free_hunk(seg_manager_t *self, reg_t addr);
static int sm_check(seg_manager_t* self, int seg);
/* Check segment validity
** Parameters: (int) seg: The segment to validate
** Returns   : (int)	0 if 'seg' is an invalid segment
**			1 if 'seg' is a valid segment
*/

// End of Memory Management

static inline int find_free_id(seg_manager_t *self, int *id) {
	char was_added = 0;
	int retval = 0;

	while (!was_added) {
		retval = self->id_seg_map->check_value(self->reserved_id, true, &was_added);
		*id = self->reserved_id--;
		if (self->reserved_id < -1000000)
			self->reserved_id = -10;
		// Make sure we don't underflow
	}

	return retval;
}

static mem_obj_t * alloc_nonscript_segment(seg_manager_t *self, mem_obj_enum type, seg_id_t *segid) {
	// Allocates a non-script segment
	int id;

	*segid = find_free_id(self, &id);
	return mem_obj_allocate(self, *segid, id, type);
}

void sm_init(seg_manager_t* self, int sci1_1) {
	int i;

	self->mem_allocated = 0; // Initialise memory count

	self->id_seg_map = new int_hash_map_t();
	self->reserved_id = INVALID_SCRIPT_ID;
	self->id_seg_map->check_value(self->reserved_id, true);	// reserve 0 for seg_id
	self->reserved_id--; // reserved_id runs in the reversed direction to make sure no one will use it.

	self->heap_size = DEFAULT_SCRIPTS;
	self->heap = (mem_obj_t**)sci_calloc(self->heap_size, sizeof(mem_obj_t *));

	self->clones_seg_id = 0;
	self->lists_seg_id = 0;
	self->nodes_seg_id = 0;
	self->hunks_seg_id = 0;

	self->exports_wide = 0;
	self->sci1_1 = sci1_1;

	// initialize the heap pointers
	for (i = 0; i < self->heap_size; i++) {
		self->heap[i] = NULL;
	}

	// gc initialisation
	self->gc_mark_bits = 0;
}

// destroy the object, free the memorys if allocated before
void sm_destroy(seg_manager_t* self) {
	int i;

	// free memory
	for (i = 0; i < self->heap_size; i++) {
		if (self->heap[i])
			_sm_deallocate(self, i, 0);
	}

	delete self->id_seg_map;

	free(self->heap);
	self->heap = NULL;
}

// allocate a memory for script from heap
// Parameters: (EngineState *) s: The state to operate on
//             (int) script_nr: The script number to load
// Returns   : 0 - allocation failure
//             1 - allocated successfully
//             seg_id - allocated segment id
mem_obj_t* sm_allocate_script(seg_manager_t* self, EngineState *s, int script_nr, int* seg_id) {
	int seg;
	char was_added;
	mem_obj_t* mem;

	seg = self->id_seg_map->check_value(script_nr, true, &was_added);
	if (!was_added) {
		*seg_id = seg;
		return self->heap[*seg_id];
	}

	// allocate the mem_obj_t
	mem = mem_obj_allocate(self, seg, script_nr, MEM_OBJ_SCRIPT);
	if (!mem) {
		sciprintf("%s, %d, Not enough memory, ", __FILE__, __LINE__);
		return NULL;
	}

	*seg_id = seg;
	return mem;
}

static void sm_set_script_size(mem_obj_t *mem, EngineState *s, int script_nr) {
	resource_t *script = scir_find_resource(s->resmgr, sci_script, script_nr, 0);
	resource_t *heap = scir_find_resource(s->resmgr, sci_heap, script_nr, 0);

	mem->data.script.script_size = script->size;
	mem->data.script.heap_size = 0; // Set later

	if (!script || (s->version >= SCI_VERSION(1, 001, 000) && !heap)) {
		sciprintf("%s: failed to load %s\n", __FUNCTION__, !script ? "script" : "heap");
		return;
	}
	if (s->version < SCI_VERSION_FTU_NEW_SCRIPT_HEADER) {
		mem->data.script.buf_size = script->size + getUInt16(script->data) * 2;
		//locals_size = getUInt16(script->data) * 2;
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
			          "fixed in the next major version");
			return;
		}
	}
}

int sm_initialise_script(mem_obj_t *mem, EngineState *s, int script_nr) {
	// allocate the script.buf
	script_t *scr;

	sm_set_script_size(mem, s, script_nr);
	mem->data.script.buf = (byte*) sci_malloc(mem->data.script.buf_size);

	dbg_print("mem->data.script.buf ", mem->data.script.buf);
	if (!mem->data.script.buf) {
		sm_free_script(mem);
		sciprintf("seg_manager.c: Not enough memory space for script size");
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

	scr->obj_indices = new int_hash_map_t();

	if (s->version >= SCI_VERSION(1, 001, 000))
		scr->heap_start = scr->buf + scr->script_size;
	else
		scr->heap_start = scr->buf;

	return 1;
}

int _sm_deallocate(seg_manager_t* self, int seg, int recursive) {
	mem_obj_t *mobj;
	VERIFY(sm_check(self, seg), "invalid seg id");

	mobj = self->heap[seg];
	self->id_seg_map->remove_value(mobj->segmgr_id);

	switch (mobj->type) {
	case MEM_OBJ_SCRIPT:
		sm_free_script(mobj);

		mobj->data.script.buf = NULL;
		if (recursive && mobj->data.script.locals_segment)
			_sm_deallocate(self, mobj->data.script.locals_segment, recursive);
		break;

	case MEM_OBJ_LOCALS:
		free(mobj->data.locals.locals);
		mobj->data.locals.locals = NULL;
		break;

	case MEM_OBJ_DYNMEM:
		if (mobj->data.dynmem.buf)
			free(mobj->data.dynmem.buf);
		mobj->data.dynmem.buf = NULL;
		break;
	case MEM_OBJ_SYS_STRINGS:
		sys_string_free_all(&(mobj->data.sys_strings));
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
	default:
		error("Deallocating segment type %d not supported", mobj->type);
		BREAKPOINT();
	}

	free(mobj);
	self->heap[seg] = NULL;

	return 1;
}

int sm_script_marked_deleted(seg_manager_t* self, int script_nr) {
	script_t *scr;
	int seg = sm_seg_get(self, script_nr);
	VERIFY(sm_check(self, seg), "invalid seg id");

	scr = &(self->heap[seg]->data.script);
	return scr->marked_as_deleted;
}

void sm_mark_script_deleted(seg_manager_t* self, int script_nr) {
	script_t *scr;
	int seg = sm_seg_get(self, script_nr);
	VERIFY(sm_check(self, seg), "invalid seg id");

	scr = &(self->heap[seg]->data.script);
	scr->marked_as_deleted = 1;
}

void sm_unmark_script_deleted(seg_manager_t* self, int script_nr) {
	script_t *scr;
	int seg = sm_seg_get(self, script_nr);
	VERIFY(sm_check(self, seg), "invalid seg id");

	scr = &(self->heap[seg]->data.script);
	scr->marked_as_deleted = 0;
}

int sm_script_is_marked_as_deleted(seg_manager_t* self, seg_id_t seg) {
	script_t *scr;

	if (!sm_check(self, seg))
		return 0;

	if (self->heap[seg]->type != MEM_OBJ_SCRIPT)
		return 0;

	scr = &(self->heap[seg]->data.script);

	return scr->marked_as_deleted;
}


int sm_deallocate_script(seg_manager_t* self, int script_nr) {
	int seg = sm_seg_get(self, script_nr);

	_sm_deallocate(self, seg, 1);

	return 1;
}

mem_obj_t* mem_obj_allocate(seg_manager_t *self, seg_id_t segid, int hash_id, mem_obj_enum type) {
	mem_obj_t* mem = (mem_obj_t*) sci_calloc(sizeof(mem_obj_t), 1);
	if (!mem) {
		sciprintf("seg_manager.c: invalid mem_obj ");
		return NULL;
	}

	if (segid >= self->heap_size) {
		void *temp;
		int oldhs = self->heap_size;

		if (segid >= self->heap_size * 2) {
			sciprintf("seg_manager.c: hash_map error or others??");
			return NULL;
		}
		self->heap_size *= 2;
		temp = sci_realloc((void*)self->heap, self->heap_size * sizeof(mem_obj_t*));
		if (!temp) {
			sciprintf("seg_manager.c: Not enough memory space for script size");
			return NULL;
		}
		self->heap = (mem_obj_t**)  temp;

		// Clear pointers
		memset(self->heap + oldhs, 0, sizeof(mem_obj_t *) * (self->heap_size - oldhs));
	}

	mem->segmgr_id = hash_id;
	mem->type = type;

	// hook it to the heap
	self->heap[segid] = mem;
	return mem;
}

/* No longer in use?
void sm_object_init(object_t *object) {
 	if (!object)
		return;
 	object->variables_nr = 0;
 	object->variables = NULL;
};*/

void sm_free_script(mem_obj_t *mem) {
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
			object_t* object = &mem->data.script.objects[i];
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
	if (NULL != mem->data.script.code) {
		free(mem->data.script.code);
	}
}

// memory operations
#if 0
// Unreferenced - removed
static void sm_mset(seg_manager_t *self, int offset, int c, size_t n, int id, int flag) {
	mem_obj_t *mem_obj;
	GET_SEGID();
	mem_obj = self->heap[id];
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
static void sm_mcpy_in_in(seg_manager_t *self, int dst, const int src, size_t n, int id, int flag) {
	mem_obj_t *mem_obj;
	GET_SEGID();
	mem_obj = self->heap[id];
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

void sm_mcpy_in_out(seg_manager_t *self, int dst, const void *src, size_t n, int id, int flag) {
	mem_obj_t *mem_obj;
	GET_SEGID();
	mem_obj = self->heap[id];
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
static void sm_mcpy_out_in(seg_manager_t *self, void *dst, const int src, size_t n, int id, int flag) {
	mem_obj_t *mem_obj;
	GET_SEGID();
	mem_obj = self->heap[id];
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

gint16 sm_get_heap(seg_manager_t *self, reg_t reg) {
	mem_obj_t *mem_obj;
	mem_obj_enum mem_type;

	VERIFY(sm_check(self, reg.segment), "Invalid seg id");
	mem_obj = self->heap[reg.segment];
	mem_type = mem_obj->type;

	switch (mem_type) {
	case MEM_OBJ_SCRIPT:
		VERIFY(reg.offset + 1 < (guint16)mem_obj->data.script.buf_size, "invalid offset\n");
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

void sm_put_heap(seg_manager_t *self, reg_t reg, gint16 value) {
	mem_obj_t *mem_obj;
	mem_obj_enum mem_type;

	VERIFY(sm_check(self, reg.segment), "Invalid seg id");
	mem_obj = self->heap[reg.segment];
	mem_type = mem_obj->type;

	switch (mem_type) {
	case MEM_OBJ_SCRIPT:
		VERIFY(reg.offset + 1 < (guint16)mem_obj->data.script.buf_size, "invalid offset");
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

// return the seg if script_id is valid and in the map, else -1
int sm_seg_get(seg_manager_t *self, int script_id) {
	return self->id_seg_map->check_value(script_id, false);
}

// validate the seg
// return:
//	0 - invalid seg
//	1 - valid seg
static int sm_check(seg_manager_t *self, int seg) {
	if (seg < 0 || seg >= self->heap_size) {
		return 0;
	}
	if (!self->heap[seg]) {
		sciprintf("seg_manager.c: seg %x is removed from memory, but not removed from hash_map\n", seg);
		return 0;
	}
	return 1;
}

int sm_script_is_loaded(seg_manager_t *self, int id, id_flag flag) {
	if (flag == SCRIPT_ID)
		id = sm_seg_get(self, id);

	return sm_check(self, id);
}

void sm_increment_lockers(seg_manager_t *self, int id, id_flag flag) {
	if (flag == SCRIPT_ID)
		id = sm_seg_get(self, id);
	VERIFY(sm_check(self, id), "invalid seg id");
	self->heap[id]->data.script.lockers++;
}

void sm_decrement_lockers(seg_manager_t *self, int id, id_flag flag) {
	if (flag == SCRIPT_ID)
		id = sm_seg_get(self, id);
	VERIFY(sm_check(self, id), "invalid seg id");

	if (self->heap[id]->data.script.lockers > 0)
		self->heap[id]->data.script.lockers--;
}

int sm_get_lockers(seg_manager_t *self, int id, id_flag flag) {
	if (flag == SCRIPT_ID)
		id = sm_seg_get(self, id);
	VERIFY(sm_check(self, id), "invalid seg id");

	return self->heap[id]->data.script.lockers;
}

void sm_set_lockers(seg_manager_t *self, int lockers, int id, id_flag flag) {
	if (flag == SCRIPT_ID)
		id = sm_seg_get(self, id);
	VERIFY(sm_check(self, id), "invalid seg id");
	self->heap[id]->data.script.lockers = lockers;
}

void sm_set_export_table_offset(struct _seg_manager_t *self, int offset, int id, id_flag flag) {
	script_t *scr = &(self->heap[id]->data.script);

	GET_SEGID();
	if (offset) {
		scr->export_table = (guint16 *)(scr->buf + offset + 2);
		scr->exports_nr = getUInt16((byte *)(scr->export_table - 1));
	} else {
		scr->export_table = NULL;
		scr->exports_nr = 0;
	}
}

int sm_hash_segment_data(struct _seg_manager_t *self, int id) {
	int i, len, hash_code = 0x55555555;
	char *buf;

	if (self->heap[id]->type == MEM_OBJ_LISTS)
		return 0;
	if (self->heap[id]->type == MEM_OBJ_NODES)
		return 0;
	if (self->heap[id]->type == MEM_OBJ_CLONES) 
		return 0;
	buf = (char *)sm_dereference(self, make_reg(id, 0), &len);

	for (i = 0; i < len; i++)
		hash_code = (hash_code * 19) + *(buf + i);

	return hash_code;
}

void sm_set_export_width(struct _seg_manager_t *self, int flag) {
	self->exports_wide = flag;
}

#if 0
// Unreferenced - removed
static guint16 *sm_get_export_table_offset(struct _seg_manager_t *self, int id, int flag, int *max) {
	GET_SEGID();
	if (max)
		*max = self->heap[id]->data.script.exports_nr;

	return self->heap[id]->data.script.export_table;
}
#endif

void sm_set_synonyms_offset(struct _seg_manager_t *self, int offset, int id, id_flag flag) {
	GET_SEGID();
	self->heap[id]->data.script.synonyms = self->heap[id]->data.script.buf + offset;
}

byte *sm_get_synonyms(seg_manager_t *self, int id, id_flag flag) {
	GET_SEGID();
	return self->heap[id]->data.script.synonyms;
}

void sm_set_synonyms_nr(struct _seg_manager_t *self, int nr, int id, id_flag flag) {
	GET_SEGID();
	self->heap[id]->data.script.synonyms_nr = nr;
}

int sm_get_synonyms_nr(struct _seg_manager_t *self, int id, id_flag flag) {
	GET_SEGID();
	return self->heap[id]->data.script.synonyms_nr;
}

#if 0
// Unreferenced - removed
static int sm_get_heappos(struct _seg_manager_t *self, int id, int flag) {
	GET_SEGID();
	return 0;
}
#endif

#if 0
// Unreferenced - removed
static void sm_set_variables(struct _seg_manager_t *self, reg_t reg, int obj_index, reg_t variable_reg, int variable_index) {
	script_t *script;
	VERIFY(sm_check(self, reg.segment), "invalid seg id");
	VERIFY(self->heap[reg.segment], "invalid mem");

	script = &(self->heap[reg.segment]->data.script);

	VERIFY(obj_index < script->objects_nr, "Invalid obj_index");

	VERIFY(variable_index >= 0 && variable_index < script->objects[obj_index].variables_nr, "Attempt to write to invalid variable number");

	script->objects[obj_index].variables[variable_index] = variable_reg;
}
#endif

static inline int _relocate_block(seg_manager_t *self, reg_t *block, int block_location, int block_items, seg_id_t segment, int location) {
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
	if (self->sci1_1)
		block[index].offset += self->heap[segment]->data.script.script_size;

	return 1;
}

static inline int _relocate_local(seg_manager_t *self, script_t *scr, seg_id_t segment, int location) {
	if (scr->locals_block)
		return _relocate_block(self, scr->locals_block->locals, scr->locals_offset, scr->locals_block->nr, segment, location);
	else
		return 0; // No hands, no cookies
}

static inline int _relocate_object(seg_manager_t *self, object_t *obj, seg_id_t segment, int location) {
	return _relocate_block(self, obj->variables, obj->pos.offset, obj->variables_nr, segment, location);
}

void sm_script_add_code_block(seg_manager_t *self, reg_t location) {
	mem_obj_t *mobj = self->heap[location.segment];
	script_t *scr;
	int index;

	VERIFY(!(location.segment >= self->heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to add a code block to non-script\n");

	scr = &(mobj->data.script);

	if (++scr->code_blocks_nr > scr->code_blocks_allocated) {
		scr->code_blocks_allocated += DEFAULT_OBJECTS_INCREMENT;
		scr->code = (code_block_t *)sci_realloc(scr->code, scr->code_blocks_allocated * sizeof(code_block_t));
	}

	index = scr->code_blocks_nr - 1;
	scr->code[index].pos = location;
	scr->code[index].size = getUInt16(scr->buf + location.offset - 2);
}

void sm_script_relocate(seg_manager_t *self, reg_t block) {
	mem_obj_t *mobj = self->heap[block.segment];
	script_t *scr;
	int count;
	int i;

	VERIFY(!(block.segment >= self->heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt relocate non-script\n");

	scr = &(mobj->data.script);

	VERIFY(block.offset < (guint16)scr->buf_size && getUInt16(scr->buf + block.offset) * 2 + block.offset < (guint16)scr->buf_size,
	       "Relocation block outside of script\n");

	count = getUInt16(scr->buf + block.offset);

	for (i = 0; i <= count; i++) {
		int pos = getUInt16(scr->buf + block.offset + 2 + (i * 2));
		if (!pos)
			continue; // FIXME: A hack pending investigation

		if (!_relocate_local(self, scr, block.segment, pos)) {
			int k, done = 0;

			for (k = 0; !done && k < scr->objects_nr; k++) {
				if (_relocate_object(self, scr->objects + k, block.segment, pos))
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

void sm_heap_relocate(seg_manager_t *self, EngineState *s, reg_t block) {
	mem_obj_t *mobj = self->heap[block.segment];
	script_t *scr;
	int count;
	int i;

	VERIFY(!(block.segment >= self->heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt relocate non-script\n");

	scr = &(mobj->data.script);

	VERIFY(block.offset < (guint16)scr->heap_size && getUInt16(scr->heap_start + block.offset) * 2 + block.offset < (guint16)scr->buf_size,
	       "Relocation block outside of script\n");

	if (scr->relocated)
		return;
	scr->relocated = 1;
	count = getUInt16(scr->heap_start + block.offset);

	for (i = 0; i < count; i++) {
		int pos = getUInt16(scr->heap_start + block.offset + 2 + (i * 2)) + scr->script_size;

		if (!_relocate_local(self, scr, block.segment, pos)) {
			int k, done = 0;

			for (k = 0; !done && k < scr->objects_nr; k++) {
				if (_relocate_object(self, scr->objects + k, block.segment, pos))
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

static object_t *sm_script_obj_init0(seg_manager_t *self, EngineState *s, reg_t obj_pos) {
	mem_obj_t *mobj = self->heap[obj_pos.segment];
	script_t *scr;
	object_t *obj;
	int id;
	unsigned int base = obj_pos.offset - SCRIPT_OBJECT_MAGIC_OFFSET;
	reg_t temp;

	VERIFY(!(obj_pos.segment >= self->heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to initialize object in non-script\n");

	scr = &(mobj->data.script);

	VERIFY(base < scr->buf_size, "Attempt to initialize object beyond end of script\n");

	if (!scr->objects) {
		scr->objects_allocated = DEFAULT_OBJECTS;
		scr->objects = (object_t *)sci_malloc(sizeof(object_t) * scr->objects_allocated);
	}
	if (scr->objects_nr == scr->objects_allocated) {
		scr->objects_allocated += DEFAULT_OBJECTS_INCREMENT;
		scr->objects = (object_t *)sci_realloc(scr->objects, sizeof(object_t) * scr->objects_allocated);
	}

	temp = make_reg(obj_pos.segment, base);
	id = scr->obj_indices->check_value(base, true);
	scr->objects_nr++;

	obj = scr->objects + id;

	VERIFY(base + SCRIPT_FUNCTAREAPTR_OFFSET  < scr->buf_size, "Function area pointer stored beyond end of script\n");

	{
		byte *data = (byte *)(scr->buf + base);
		int funct_area = getUInt16(data + SCRIPT_FUNCTAREAPTR_OFFSET);
		int variables_nr;
		int functions_nr;
		int is_class;
		int i;

		obj->flags = 0;
		obj->pos = temp;

		VERIFY(base + funct_area < scr->buf_size, "Function area pointer references beyond end of script");

		variables_nr = getUInt16(data + SCRIPT_SELECTORCTR_OFFSET);
		functions_nr = getUInt16(data + funct_area - 2);
		is_class = getUInt16(data + SCRIPT_INFO_OFFSET) & SCRIPT_INFO_CLASS;

		VERIFY(base + funct_area + functions_nr * 2
		       // add again for classes, since those also store selectors
		       + (is_class ? functions_nr * 2 : 0) < scr->buf_size, "Function area extends beyond end of script");

		obj->variables_nr = variables_nr;
		obj->variables = (reg_t *)sci_malloc(sizeof(reg_t) * variables_nr);

		obj->methods_nr = functions_nr;
		obj->base = scr->buf;
		obj->base_obj = data;
		obj->base_method = (guint16 *)(data + funct_area);
		obj->base_vars = NULL;

		for (i = 0; i < variables_nr; i++)
			obj->variables[i] = make_reg(0, getUInt16(data + (i * 2)));
	}

	return obj;
}

static object_t *sm_script_obj_init11(seg_manager_t *self, EngineState *s, reg_t obj_pos) {
	mem_obj_t *mobj = self->heap[obj_pos.segment];
	script_t *scr;
	object_t *obj;
	int id;
	int base = obj_pos.offset;

	VERIFY(!(obj_pos.segment >= self->heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to initialize object in non-script\n");

	scr = &(mobj->data.script);

	VERIFY(base < (guint16)scr->buf_size, "Attempt to initialize object beyond end of script\n");

	if (!scr->objects) {
		scr->objects_allocated = DEFAULT_OBJECTS;
		scr->objects = (object_t *)sci_malloc(sizeof(object_t) * scr->objects_allocated);
	}
	if (scr->objects_nr == scr->objects_allocated) {
		scr->objects_allocated += DEFAULT_OBJECTS_INCREMENT;
		scr->objects = (object_t *)sci_realloc(scr->objects, sizeof(object_t) * scr->objects_allocated);
	}

	id = scr->obj_indices->check_value(obj_pos.offset, true);
	scr->objects_nr++;

	obj = scr->objects + id;

	VERIFY(base + SCRIPT_FUNCTAREAPTR_OFFSET < (guint16)scr->buf_size, "Function area pointer stored beyond end of script\n");

	{
		byte *data = (byte *)(scr->buf + base);
		guint16 *funct_area = (guint16 *)(scr->buf + getUInt16(data + 6));
		guint16 *prop_area = (guint16 *)(scr->buf + getUInt16(data + 4));
		int variables_nr;
		int functions_nr;
		int is_class;
		int i;

		obj->flags = 0;
		obj->pos = obj_pos;

		VERIFY((byte *) funct_area < scr->buf + scr->buf_size, "Function area pointer references beyond end of script");

		variables_nr = getUInt16(data + 2);
		functions_nr = *funct_area;
		is_class = getUInt16(data + 14) & SCRIPT_INFO_CLASS;

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
			obj->variables[i] = make_reg(0, getUInt16(data + (i * 2)));
	}

	return obj;
}

object_t *sm_script_obj_init(seg_manager_t *self, EngineState *s, reg_t obj_pos) {
	if (!self->sci1_1)
		return sm_script_obj_init0(self, s, obj_pos);
	else
		return sm_script_obj_init11(self, s, obj_pos);
}

static local_variables_t *_sm_alloc_locals_segment(seg_manager_t *self, script_t *scr, int count) {
	if (!count) { // No locals
		scr->locals_segment = 0;
		scr->locals_block = NULL;
		return NULL;
	} else {
		mem_obj_t *mobj;
		local_variables_t *locals;

		if (scr->locals_segment) {
			mobj = self->heap[scr->locals_segment];
			VERIFY(mobj != NULL, "Re-used locals segment was NULL'd out");
			VERIFY(mobj->type == MEM_OBJ_LOCALS, "Re-used locals segment did not consist of local variables");
			VERIFY(mobj->data.locals.script_id == scr->nr, "Re-used locals segment belonged to other script");
		} else
			mobj = alloc_nonscript_segment(self, MEM_OBJ_LOCALS, &scr->locals_segment);

		locals = scr->locals_block = &(mobj->data.locals);
		locals->script_id = scr->nr;
		locals->locals = (reg_t *)sci_calloc(count, sizeof(reg_t));
		locals->nr = count;

		return locals;
	}
}

void sm_script_initialise_locals_zero(seg_manager_t *self, seg_id_t seg, int count) {
	mem_obj_t *mobj = self->heap[seg];
	script_t *scr;

	VERIFY(!(seg >= self->heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to initialize locals in non-script\n");

	scr = &(mobj->data.script);

	scr->locals_offset = -count * 2; // Make sure it's invalid

	_sm_alloc_locals_segment(self, scr, count);
}

void sm_script_initialise_locals(seg_manager_t *self, reg_t location) {
	mem_obj_t *mobj = self->heap[location.segment];
	unsigned int count;
	script_t *scr;
	local_variables_t *locals;

	VERIFY(!(location.segment >= self->heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to initialize locals in non-script\n");

	scr = &(mobj->data.script);

	VERIFY(location.offset + 1 < (guint16)scr->buf_size, "Locals beyond end of script\n");

	if (self->sci1_1)
		count = getUInt16(scr->buf + location.offset - 2);
	else
		count = (getUInt16(scr->buf + location.offset - 2) - 4) >> 1;
	// half block size

	scr->locals_offset = location.offset;

	if (!(location.offset + count * 2 + 1 < scr->buf_size)) {
		sciprintf("Locals extend beyond end of script: offset %04x, count %x vs size %x\n", location.offset, count, (uint)scr->buf_size);
		count = (scr->buf_size - location.offset) >> 1;
	}

	locals = _sm_alloc_locals_segment(self, scr, count);
	if (locals) {
		uint i;
		byte *base = (byte *)(scr->buf + location.offset);

		for (i = 0; i < count; i++)
			locals->locals[i].offset = getUInt16(base + i * 2);
	}
}

void sm_script_relocate_exports_sci11(seg_manager_t *self, int seg) {
	mem_obj_t *mobj = self->heap[seg];
	script_t *scr;
	int i;
	int location;

	VERIFY(!(seg >= self->heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to relocate exports in non-script\n");

	scr = &(mobj->data.script);
	for (i = 0; i < scr->exports_nr; i++) {
		/* We are forced to use an ugly heuristic here to distinguish function
		   exports from object/class exports. The former kind points into the
		   script resource, the latter into the heap resource.  */
		location = getUInt16((byte *)(scr->export_table + i));
		if (getUInt16(scr->heap_start + location) == SCRIPT_OBJECT_MAGIC_NUMBER) {
			putInt16((byte *)(scr->export_table + i), location + scr->heap_start - scr->buf);
		} else {
			// Otherwise it's probably a function export,
			// and we don't need to do anything.
		}
	}
}

void sm_script_initialise_objects_sci11(seg_manager_t *self, EngineState *s, int seg) {
	mem_obj_t *mobj = self->heap[seg];
	script_t *scr;
	byte *seeker;

	VERIFY(!(seg >= self->heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to relocate exports in non-script\n");

	scr = &(mobj->data.script);
	seeker = scr->heap_start + 4 + getUInt16(scr->heap_start + 2) * 2;

	while (getUInt16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		if (getUInt16(seeker + 14) & SCRIPT_INFO_CLASS) {
			int classpos = seeker - scr->buf;
			int species = getUInt16(seeker + 10);

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
		seeker += getUInt16(seeker + 2) * 2;
	}

	seeker = scr->heap_start + 4 + getUInt16(scr->heap_start + 2) * 2;
	while (getUInt16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		reg_t reg;
		object_t *obj;

		reg.segment = seg;
		reg.offset = seeker - scr->buf;
		obj = sm_script_obj_init(&s->seg_manager, s, reg);

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

		seeker += getUInt16(seeker + 2) * 2;
	}
}

void sm_script_free_unused_objects(seg_manager_t *self, seg_id_t seg) {
	mem_obj_t *mobj = self->heap[seg];
	script_t *scr;

	VERIFY(!(seg >= self->heap_size || mobj->type != MEM_OBJ_SCRIPT), "Attempt to free unused objects in non-script\n");

	scr = &(mobj->data.script);
	if (scr->objects_allocated > scr->objects_nr) {
		if (scr->objects_nr)
			scr->objects = (object_t*)sci_realloc(scr->objects, sizeof(object_t) * scr->objects_nr);
		else {
			if (scr->objects_allocated)
				free(scr->objects);
			scr->objects = NULL;
		}
		scr->objects_allocated = scr->objects_nr;
	}
}

static inline char *dynprintf(char *msg, ...) {
	va_list argp;
	char *buf = (char *)sci_malloc(strlen(msg) + 100);

	va_start(argp, msg);
	vsprintf(buf, msg, argp);
	va_end(argp);

	return buf;
}

dstack_t *sm_allocate_stack(seg_manager_t *self, int size, seg_id_t *segid) {
	mem_obj_t *memobj = alloc_nonscript_segment(self, MEM_OBJ_STACK, segid);
	dstack_t *retval = &(memobj->data.stack);

	retval->entries = (reg_t*)sci_calloc(size, sizeof(reg_t));
	retval->nr = size;

	return retval;
}

sys_strings_t *sm_allocate_sys_strings(seg_manager_t *self, seg_id_t *segid) {
	mem_obj_t *memobj = alloc_nonscript_segment(self, MEM_OBJ_SYS_STRINGS, segid);
	sys_strings_t *retval = &(memobj->data.sys_strings);

	memset(retval, 0, sizeof(sys_string_t)*SYS_STRINGS_MAX);

	return retval;
}

seg_id_t sm_allocate_reserved_segment(seg_manager_t *self, char *src_name) {
	seg_id_t segid;
	mem_obj_t *memobj = alloc_nonscript_segment(self, MEM_OBJ_RESERVED, &segid);
	char *name = sci_strdup(src_name);

	memobj->data.reserved = name;

	return segid;
}

guint16 sm_validate_export_func(struct _seg_manager_t* self, int pubfunct, int seg) {
	script_t* script;
	guint16 offset;
	VERIFY(sm_check(self, seg), "invalid seg id");
	VERIFY(self->heap[seg]->type == MEM_OBJ_SCRIPT, "Can only validate exports on scripts");

	script = &self->heap[seg]->data.script;
	if (script->exports_nr <= pubfunct) {
		sciprintf("pubfunct is invalid");
		return 0;
	}

	if (self->exports_wide)
		pubfunct *= 2;
	offset = getUInt16((byte *)(script->export_table + pubfunct));
	VERIFY(offset < script->buf_size, "invalid export function pointer");

	return offset;
}

void sm_free_hunk_entry(seg_manager_t *self, reg_t addr) {
	sm_free_hunk(self, addr);
}

hunk_t *sm_alloc_hunk_entry(seg_manager_t *self, const char *hunk_type, int size, reg_t *reg) {
	hunk_t *h = sm_alloc_hunk(self, reg);

	if (!h)
		return NULL;

	h->mem = sci_malloc(size);
	h->size = size;
	h->type = hunk_type;

	return h;
}

static void _clone_cleanup(clone_t *clone) {
	if (clone->variables)
		free(clone->variables); // Free the dynamically allocated memory part
}

static void _hunk_cleanup(hunk_t *hunk) {
	if (hunk->mem)
		free(hunk->mem);
}

DEFINE_HEAPENTRY(list, 8, 4)
DEFINE_HEAPENTRY(node, 32, 16)
DEFINE_HEAPENTRY_WITH_CLEANUP(clone, 16, 4, _clone_cleanup)
DEFINE_HEAPENTRY_WITH_CLEANUP(hunk, 4, 4, _hunk_cleanup)

#define DEFINE_ALLOC_DEALLOC(STATIC, TYPE, SEGTYPE, PLURAL) \
STATIC TYPE##_t *sm_alloc_##TYPE(seg_manager_t *self, reg_t *addr) {												  \
	mem_obj_t *mobj;									  \
	TYPE##_table_t *table;									  \
	int offset;										  \
												  \
	if (!self->TYPE##s_seg_id) {								  \
		mobj = alloc_nonscript_segment(self, SEGTYPE, &(self->TYPE##s_seg_id));		  \
		init_##TYPE##_table(&(mobj->data.PLURAL));					  \
	} else											  \
		mobj = self->heap[self->TYPE##s_seg_id];					  \
												  \
	table = &(mobj->data.PLURAL);								  \
	offset = alloc_##TYPE##_entry(table);							  \
												  \
	*addr = make_reg(self->TYPE##s_seg_id, offset);						  \
	return &(mobj->data.PLURAL.table[offset].entry);					  \
}												  \
												  \
STATIC void	sm_free_##TYPE(seg_manager_t *self, reg_t addr) {												  \
	mem_obj_t *mobj = GET_SEGMENT(*self, addr.segment, SEGTYPE);				  \
												  \
	if (!mobj) {										  \
		sciprintf("Attempt to free " #TYPE " from address "PREG": Invalid segment type\n", PRINT_REG(addr));							  \
		return;										  \
	}											  \
												  \
	free_##TYPE##_entry(&(mobj->data.PLURAL), addr.offset);					  \
}

DEFINE_ALLOC_DEALLOC(, clone, MEM_OBJ_CLONES, clones)
DEFINE_ALLOC_DEALLOC(, list, MEM_OBJ_LISTS, lists)
DEFINE_ALLOC_DEALLOC(, node, MEM_OBJ_NODES, nodes)
DEFINE_ALLOC_DEALLOC(static, hunk, MEM_OBJ_HUNK, hunks)

byte *sm_dereference(seg_manager_t *self, reg_t pointer, int *size) {
	mem_obj_t *mobj;
	byte *base = NULL;
	int count;

	if (!pointer.segment || (pointer.segment >= self->heap_size) || !self->heap[pointer.segment]) {
		sciprintf("Error: Attempt to dereference invalid pointer "PREG"", PRINT_REG(pointer));
		return NULL; // Invalid
	}

	mobj = self->heap[pointer.segment];

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
			sciprintf("Error: Attempt to dereference invalid pointer "PREG"", PRINT_REG(pointer));
			return NULL;
		}

	case MEM_OBJ_RESERVED:
		sciprintf("Error: Trying to dereference pointer "PREG" to reserved segment `%s'", PRINT_REG(pointer), mobj->data.reserved);
		return NULL;
		break;

	default:
		sciprintf("Error: Trying to dereference pointer "PREG" to inappropriate segment", PRINT_REG(pointer));
		return NULL;
	}

	if (size)
		*size = count;

	return
	    base + pointer.offset;
}

unsigned char *sm_alloc_dynmem(seg_manager_t *self, int size, const char *descr, reg_t *addr) {
	seg_id_t seg;
	mem_obj_t *mobj = alloc_nonscript_segment(self, MEM_OBJ_DYNMEM, &seg);
	*addr = make_reg(seg, 0);

	mobj->data.dynmem.size = size;

	if (size == 0)
		mobj->data.dynmem.buf = NULL;
	else
		mobj->data.dynmem.buf = (byte*) sci_malloc(size);

	mobj->data.dynmem.description = descr;

	return (unsigned char *)(mobj->data.dynmem.buf);
}

const char *sm_get_description(seg_manager_t *self, reg_t addr) {
	mem_obj_t *mobj = self->heap[addr.segment];

	if (addr.segment >= self->heap_size)
		return "";

	switch (mobj->type) {
	case MEM_OBJ_DYNMEM:
		return mobj->data.dynmem.description;
	default:
		return "";
	}
}

int sm_free_dynmem(seg_manager_t *self, reg_t addr) {
	if (addr.segment <= 0 || addr.segment >= self->heap_size || !self->heap[addr.segment] || self->heap[addr.segment]->type != MEM_OBJ_DYNMEM)
		return 1; // error

	_sm_deallocate(self, addr.segment, 1);

	return 0; // OK
}

// ------------------- Segment interface ------------------

static void free_at_address_stub(seg_interface_t *self, reg_t sub_addr) {
	//sciprintf("  Request to free "PREG"\n", PRINT_REG(sub_addr));
	// STUB
}

static reg_t find_canonic_address_base(seg_interface_t *self, reg_t addr) {
	addr.offset = 0;

	return addr;
}

static reg_t find_canonic_address_id(seg_interface_t *self, reg_t addr) {
	return addr;
}

static void free_at_address_nop(seg_interface_t *self, reg_t sub_addr) {
}

static void list_all_deallocatable_nop(seg_interface_t *self, void *param, void (*note)(void*param, reg_t addr)) {
}

static void list_all_deallocatable_base(seg_interface_t *self, void *param, void (*note)(void*param, reg_t addr)) {
	(*note)(param, make_reg(self->seg_id, 0));
}

static void list_all_outgoing_references_nop(seg_interface_t *self, EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
}

static void deallocate_self(seg_interface_t *self) {
	free(self);
}

static void free_at_address_script(seg_interface_t *self, reg_t addr) {
	script_t *script;
	VERIFY(self->mobj->type == MEM_OBJ_SCRIPT, "Trying to free a non-script!");
	script = &(self->mobj->data.script);
	/*
		sciprintf("[GC] Freeing script "PREG"\n", PRINT_REG(addr));
		if (script->locals_segment)
			sciprintf("[GC] Freeing locals %04x:0000\n", script->locals_segment);
	*/

	if (script->marked_as_deleted)
		sm_deallocate_script(self->segmgr, script->nr);
}

static void list_all_outgoing_references_script(seg_interface_t *self, EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
	script_t *script = &(self->mobj->data.script);

	if (addr.offset <= script->buf_size && addr.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET && RAW_IS_OBJECT(script->buf + addr.offset)) {
		int idx = RAW_GET_CLASS_INDEX(script, addr);
		if (idx >= 0 && idx < script->objects_nr) {
			object_t *obj = script->objects + idx;
			int i;

			// Note all local variables, if we have a local variable environment
			if (script->locals_segment)
				(*note)(param, make_reg(script->locals_segment, 0));

			for (i = 0; i < obj->variables_nr; i++)
				(*note)(param, obj->variables[i]);
		} else {
			error("Request for outgoing script-object reference at "PREG" yielded invalid index %d\n", PRINT_REG(addr), idx);
		}
	} else {
		//error("Unexpected request for outgoing script-object references at "PREG"\n", PRINT_REG(addr));
		// Happens e.g. when we're looking into strings
	}
}

//-------------------- script --------------------
static seg_interface_t seg_interface_script = {
	/* segmgr = */	NULL,
	/* mobj = */	NULL,
	/* seg_id = */	0,
	/* type_id = */	MEM_OBJ_SCRIPT,
	/* type = */	"script",
	/* find_canonic_address = */		find_canonic_address_base,
	/* free_at_address = */			free_at_address_script,
	/* list_all_deallocatable = */		list_all_deallocatable_base,
	/* list_all_outgoing_references = */	list_all_outgoing_references_script,
	/* deallocate_self = */			deallocate_self
};


#define LIST_ALL_DEALLOCATABLE(kind, kind_field) \
	mem_obj_t *mobj = self->mobj;					\
	kind##_table_t * table = &(mobj->data.kind_field);		\
	int i;								\
									\
	for (i = 0; i < table->max_entry; i++)				\
		if (ENTRY_IS_VALID(table, i))				\
			(*note) (param, make_reg(self->seg_id, i));

static void list_all_deallocatable_clones(seg_interface_t *self, void *param, void (*note)(void*param, reg_t addr)) {
	LIST_ALL_DEALLOCATABLE(clone, clones);
}

static void list_all_outgoing_references_clones(seg_interface_t *self, EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
	mem_obj_t *mobj = self->mobj;
	clone_table_t *clone_table = &(mobj->data.clones);
	clone_t *clone;
	int i;

	assert(addr.segment == self->seg_id);

	if (!(ENTRY_IS_VALID(clone_table, addr.offset))) {
		error("Unexpected request for outgoing references from clone at "PREG"\n", PRINT_REG(addr));
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

void free_at_address_clones(seg_interface_t *self, reg_t addr) {
	object_t *victim_obj;

	assert(addr.segment == self->seg_id);

	victim_obj = &(self->mobj->data.clones.table[addr.offset].entry);

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
	sm_free_clone(self->segmgr, addr);
}

//-------------------- clones --------------------
static seg_interface_t seg_interface_clones = {
	/* segmgr = */	NULL,
	/* mobj = */	NULL,
	/* seg_id = */	0,
	/* type_id = */	MEM_OBJ_CLONES,
	/* type = */	"clones",
	/* find_canonic_address = */		find_canonic_address_id,
	/* free_at_address = */			free_at_address_clones,
	/* list_all_deallocatable = */		list_all_deallocatable_clones,
	/* list_all_outgoing_references = */	list_all_outgoing_references_clones,
	/* deallocate_self = */			deallocate_self
};

static reg_t find_canonic_address_locals(seg_interface_t *self, reg_t addr) {
	local_variables_t *locals = &(self->mobj->data.locals);
	// Reference the owning script
	seg_id_t owner_seg = sm_seg_get(self->segmgr, locals->script_id);

	assert(owner_seg >= 0);

	return make_reg(owner_seg, 0);
}

static void list_all_outgoing_references_locals(seg_interface_t *self, EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
	local_variables_t *locals = &(self->mobj->data.locals);
	int i;

	assert(addr.segment == self->seg_id);

	for (i = 0; i < locals->nr; i++)
		(*note)(param, locals->locals[i]);
}

//-------------------- locals --------------------
static seg_interface_t seg_interface_locals = {
	/* segmgr = */	NULL,
	/* mobj = */	NULL,
	/* seg_id = */	0,
	/* type_id = */	MEM_OBJ_LOCALS,
	/* type = */	"locals",
	/* find_canonic_address = */		find_canonic_address_locals,
	/* free_at_address = */			free_at_address_stub,
	/* list_all_deallocatable = */		list_all_deallocatable_nop,
	/* list_all_outgoing_references = */	list_all_outgoing_references_locals,
	/* deallocate_self = */			deallocate_self
};

static void list_all_outgoing_references_stack(seg_interface_t *self, EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
	int i;

	error("Emitting %d stack entries\n", self->mobj->data.stack.nr);
	for (i = 0; i < self->mobj->data.stack.nr; i++)
		(*note)(param, self->mobj->data.stack.entries[i]);

	error("DONE");
}

//-------------------- stack --------------------

static seg_interface_t seg_interface_stack = {
	/* segmgr = */	NULL,
	/* mobj = */	NULL,
	/* seg_id = */	0,
	/* type_id = */	MEM_OBJ_STACK,
	/* type = */	"stack",
	/* find_canonic_address = */		find_canonic_address_base,
	/* free_at_address = */			free_at_address_nop,
	/* list_all_deallocatable = */		list_all_deallocatable_nop,
	/* list_all_outgoing_references = */	list_all_outgoing_references_stack,
	/* deallocate_self = */			deallocate_self
};

//-------------------- system strings --------------------
static seg_interface_t seg_interface_sys_strings = {
	/* segmgr = */	NULL,
	/* mobj = */	NULL,
	/* seg_id = */	0,
	/* type_id = */	MEM_OBJ_SYS_STRINGS,
	/* type = */	"system strings",
	/* find_canonic_address = */		find_canonic_address_id,
	/* free_at_address = */			free_at_address_nop,
	/* list_all_deallocatable = */		list_all_deallocatable_nop,
	/* list_all_outgoing_references = */	list_all_outgoing_references_nop,
	/* deallocate_self = */			deallocate_self
};

static void list_all_deallocatable_list(seg_interface_t *self, void *param, void (*note)(void*param, reg_t addr)) {
	LIST_ALL_DEALLOCATABLE(list, lists);
}

static void list_all_outgoing_references_list(seg_interface_t *self, EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
	list_table_t *table = &(self->mobj->data.lists);
	list_t *list = &(table->table[addr.offset].entry);

	if (!ENTRY_IS_VALID(table, addr.offset)) {
		error("Invalid list referenced for outgoing references: "PREG"\n", PRINT_REG(addr));
		return;
	}

	note(param, list->first);
	note(param, list->last);
	// We could probably get away with just one of them, but
	// let's be conservative here.
}

static void free_at_address_lists(seg_interface_t *self, reg_t sub_addr) {
	sm_free_list(self->segmgr, sub_addr);
}

//-------------------- lists --------------------
static seg_interface_t seg_interface_lists = {
	/* segmgr = */	NULL,
	/* mobj = */	NULL,
	/* seg_id = */	0,
	/* type_id = */	MEM_OBJ_LISTS,
	/* type = */	"lists",
	/* find_canonic_address = */		find_canonic_address_id,
	/* free_at_address = */			free_at_address_lists,
	/* list_all_deallocatable = */		list_all_deallocatable_list,
	/* list_all_outgoing_references = */	list_all_outgoing_references_list,
	/* deallocate_self = */			deallocate_self
};

static void list_all_deallocatable_nodes(seg_interface_t *self, void *param, void (*note)(void*param, reg_t addr)) {
	LIST_ALL_DEALLOCATABLE(node, nodes);
}

static void list_all_outgoing_references_nodes(seg_interface_t *self, EngineState *s, reg_t addr, void *param, void (*note)(void*param, reg_t addr)) {
	node_table_t *table = &(self->mobj->data.nodes);
	node_t *node = &(table->table[addr.offset].entry);

	if (!ENTRY_IS_VALID(table, addr.offset)) {
		error("Invalid node referenced for outgoing references: "PREG"\n", PRINT_REG(addr));
		return;
	}

	// We need all four here. Can't just stick with 'pred' OR 'succ' because node operations allow us
	// to walk around from any given node
	note(param, node->pred);
	note(param, node->succ);
	note(param, node->key);
	note(param, node->value);
}

static void free_at_address_nodes(seg_interface_t *self, reg_t sub_addr) {
	sm_free_node(self->segmgr, sub_addr);
}

//-------------------- nodes --------------------
static seg_interface_t seg_interface_nodes = {
	/* segmgr = */	NULL,
	/* mobj = */	NULL,
	/* seg_id = */	0,
	/* type_id = */	MEM_OBJ_NODES,
	/* type = */	"nodes",
	/* find_canonic_address = */		find_canonic_address_id,
	/* free_at_address = */			free_at_address_nodes,
	/* list_all_deallocatable = */		list_all_deallocatable_nodes,
	/* list_all_outgoing_references = */	list_all_outgoing_references_nodes,
	/* deallocate_self = */			deallocate_self
};

static void list_all_deallocatable_hunk(seg_interface_t *self, void *param, void (*note)(void*param, reg_t addr)) {
	LIST_ALL_DEALLOCATABLE(hunk, hunks);
}

//-------------------- hunk --------------------
static seg_interface_t seg_interface_hunk = {
	/* segmgr = */	NULL,
	/* mobj = */	NULL,
	/* seg_id = */	0,
	/* type_id = */	MEM_OBJ_HUNK,
	/* type = */	"hunk",
	/* find_canonic_address = */		find_canonic_address_id,
	/* free_at_address = */			free_at_address_stub,
	/* list_all_deallocatable = */		list_all_deallocatable_hunk,
	/* list_all_outgoing_references = */	list_all_outgoing_references_nop,
	/* deallocate_self = */			deallocate_self
};

//-------------------- dynamic memory --------------------
static seg_interface_t seg_interface_dynmem = {
	/* segmgr = */	NULL,
	/* mobj = */	NULL,
	/* seg_id = */	0,
	/* type_id = */	MEM_OBJ_DYNMEM,
	/* type = */	"dynamic memory",
	/* find_canonic_address = */		find_canonic_address_base,
	/* free_at_address = */			free_at_address_stub,
	/* list_all_deallocatable = */		list_all_deallocatable_base,
	/* list_all_outgoing_references = */	list_all_outgoing_references_nop,
	/* deallocate_self = */			deallocate_self
};

//-------------------- reserved --------------------
static seg_interface_t seg_interface_reserved = {
	/* segmgr = */	NULL,
	/* mobj = */	NULL,
	/* seg_id = */	0,
	/* type_id = */	MEM_OBJ_RESERVED,
	/* type = */	"reserved",
	/* find_canonic_address = */		find_canonic_address_id,
	/* free_at_address = */			free_at_address_nop,
	/* list_all_deallocatable = */		list_all_deallocatable_nop,
	/* list_all_outgoing_references = */	list_all_outgoing_references_nop,
	/* deallocate_self = */			deallocate_self
};

static seg_interface_t* seg_interfaces[MEM_OBJ_MAX] = {
	&seg_interface_script,
	&seg_interface_clones,
	&seg_interface_locals,
	&seg_interface_stack,
	&seg_interface_sys_strings,
	&seg_interface_lists,
	&seg_interface_nodes,
	&seg_interface_hunk,
	&seg_interface_dynmem,
	&seg_interface_reserved
};

seg_interface_t *get_seg_interface(seg_manager_t *self, seg_id_t segid) {
	mem_obj_t *mobj;
	seg_interface_t *retval;

	if (!sm_check(self, segid))
		return NULL; // Invalid segment

	mobj = self->heap[segid];
	retval = (seg_interface_t *)sci_malloc(sizeof(seg_interface_t));
	memcpy(retval, seg_interfaces[mobj->type - 1], sizeof(seg_interface_t));

	if (mobj->type != retval->type_id) {
		error("Improper segment interface for %d", mobj->type);
		exit(1);
	}

	retval->segmgr = self;
	retval->mobj = mobj;
	retval->seg_id = segid;

	return retval;
}

} // End of namespace Sci
