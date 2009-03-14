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

#include "engines/tinsel/serializer.h"	// FIXME: Hack

#include <time.h>	// FIXME: For struct tm
#include "common/stream.h"
#include "common/system.h"
#include "sci/sci_memory.h"
#include "sci/gfx/operations.h"
#include "sci/gfx/menubar.h"
#include "sci/gfx/gfx_state_internal.h"	// required for gfxw_port_t, gfxw_container_t
#include "sci/sfx/core.h"
#include "sci/sfx/iterator.h"
#include "sci/engine/state.h"
#include "sci/engine/intmap.h"
#include "sci/engine/savegame.h"

namespace Sci {


int gamestate_save(EngineState *s, Common::WriteStream *fh, const char* savename) {
	tm curTime;
	g_system->getTimeAndDate(curTime);

	SavegameMetadata meta;
	meta.savegame_version = FREESCI_CURRENT_SAVEGAME_VERSION;
	meta.savegame_name = savename;
	meta.version = s->version;
	meta.game_version = s->game_version;
	meta.savegame_date = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	meta.savegame_time = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);

	s->savegame_version = FREESCI_CURRENT_SAVEGAME_VERSION;
	s->dyn_views_list_serial = (s->dyn_views)? s->dyn_views->serial : -2;
	s->drop_views_list_serial = (s->drop_views)? s->drop_views->serial : -2;
	s->port_serial = (s->port)? s->port->serial : -2;

	if (s->execution_stack_base) {
		sciprintf("Cannot save from below kernel function\n");
		return 1;
	}

/*
	if (s->sound_server) {
		if ((s->sound_server->save)(s, dirname)) {
			sciprintf("Saving failed for the sound subsystem\n");
			chdir("..");
			return 1;
		}
	}
*/
	// Calculate the time spent with this game
	s->game_time = (g_system->getMillis() - s->game_start_time) / 1000;

	%CFSMLWRITE SavegameMetadata (&meta) INTO fh;
	%CFSMLWRITE EngineState s INTO fh;

	return 0;
}

static SegmentId find_unique_seg_by_type(SegManager *self, int type) {
	int i;

	for (i = 0; i < self->heap_size; i++)
		if (self->heap[i] &&
		    self->heap[i]->type == type)
			return i;
	return -1;
}

static byte *find_unique_script_block(EngineState *s, byte *buf, int type) {
	int magic_pos_adder = s->version >= SCI_VERSION_FTU_NEW_SCRIPT_HEADER ? 0 : 2;

	buf += magic_pos_adder;
	do {
		int seeker_type = READ_LE_UINT16(buf);
		int seeker_size;

		if (seeker_type == 0) break;
		if (seeker_type == type) return buf;

		seeker_size = READ_LE_UINT16(buf + 2);
		buf += seeker_size;
	} while(1);

	return NULL;
}

static void reconstruct_stack(EngineState *retval) {
	SegmentId stack_seg = find_unique_seg_by_type(retval->seg_manager, MEM_OBJ_STACK);
	dstack_t *stack = &(retval->seg_manager->heap[stack_seg]->data.stack);

	retval->stack_segment = stack_seg;
	retval->stack_base = stack->entries;
	retval->stack_top = retval->stack_base + VM_STACK_SIZE;
}

static int clone_entry_used(CloneTable *table, int n) {
	int backup;
	int seeker = table->first_free;
	CloneEntry *entries = table->table;

	if (seeker == HEAPENTRY_INVALID) return 1;

	do {
		if (seeker == n) return 0;
		backup = seeker;
		seeker = entries[seeker].next_free;
	} while (entries[backup].next_free != HEAPENTRY_INVALID);

	return 1;
}

static void load_script(EngineState *s, SegmentId seg) {
	Resource *script, *heap = NULL;
	Script *scr = &(s->seg_manager->heap[seg]->data.script);

	scr->buf = (byte *)malloc(scr->buf_size);

	script = s->resmgr->findResource(kResourceTypeScript, scr->nr, 0);
	if (s->version >= SCI_VERSION(1,001,000))
		heap = s->resmgr->findResource(kResourceTypeHeap, scr->nr, 0);

	switch (s->seg_manager->isSci1_1) {
	case 0 :
		s->seg_manager->mcpyInOut(0, script->data, script->size, seg, SEG_ID);
		break;
	case 1 :
		s->seg_manager->mcpyInOut(0, script->data, script->size, seg, SEG_ID);
		s->seg_manager->mcpyInOut(scr->script_size, heap->data, heap->size, seg, SEG_ID);
		break;
	}
}

static void reconstruct_scripts(EngineState *s, SegManager *self) {
	int i;
	MemObject *mobj;
	for (i = 0; i < self->heap_size; i++) {
		if (self->heap[i]) {
			mobj = self->heap[i];
			switch (mobj->type)  {
			case MEM_OBJ_SCRIPT: {
				int j;
				Script *scr = &mobj->data.script;

				load_script(s, i);
				scr->locals_block = scr->locals_segment == 0 ? NULL : &s->seg_manager->heap[scr->locals_segment]->data.locals;
				scr->export_table = (uint16 *) find_unique_script_block(s, scr->buf, sci_obj_exports);
				scr->synonyms = find_unique_script_block(s, scr->buf, sci_obj_synonyms);
				scr->code = NULL;
				scr->code_blocks_nr = 0;
				scr->code_blocks_allocated = 0;

				if (!self->isSci1_1)
					scr->export_table += 3;

				for (j = 0; j < scr->objects_nr; j++) {
					byte *data = scr->buf + scr->objects[j].pos.offset;
					scr->objects[j].base = scr->buf;
					scr->objects[j].base_obj = data;
				}
				break;
			}
			default:
				break;
			}
		}
	}

	for (i = 0; i < self->heap_size; i++) {
		if (self->heap[i]) {
			mobj = self->heap[i];
			switch (mobj->type)  {
			case MEM_OBJ_SCRIPT: {
				int j;
				Script *scr = &mobj->data.script;

				for (j = 0; j < scr->objects_nr; j++) {
					byte *data = scr->buf + scr->objects[j].pos.offset;

					if (self->isSci1_1) {
						uint16 *funct_area = (uint16 *) (scr->buf + READ_LE_UINT16( data + 6 ));
						uint16 *prop_area = (uint16 *) (scr->buf + READ_LE_UINT16( data + 4 ));

						scr->objects[j].base_method = funct_area;
						scr->objects[j].base_vars = prop_area;
					} else {
						int funct_area = READ_LE_UINT16( data + SCRIPT_FUNCTAREAPTR_OFFSET );
						Object *base_obj;

						base_obj = obj_get(s, scr->objects[j].variables[SCRIPT_SPECIES_SELECTOR]);

						if (!base_obj) {
							sciprintf("Object without a base class: Script %d, index %d (reg address "PREG"\n",
								  scr->nr, j, PRINT_REG(scr->objects[j].variables[SCRIPT_SPECIES_SELECTOR]));
							continue;
						}
						scr->objects[j].variable_names_nr = base_obj->variables_nr;
						scr->objects[j].base_obj = base_obj->base_obj;

						scr->objects[j].base_method = (uint16 *) (data + funct_area);
						scr->objects[j].base_vars = (uint16 *) (data + scr->objects[j].variable_names_nr * 2 + SCRIPT_SELECTOR_OFFSET);
					}
				}
				break;
			}
			default:
				break;
			}
		}
	}
}

void reconstruct_clones(EngineState *s, SegManager *self) {
	int i;
	MemObject *mobj;

	for (i = 0; i < self->heap_size; i++) {
		if (self->heap[i]) {
			mobj = self->heap[i];
			switch (mobj->type) {
			case MEM_OBJ_CLONES: {
				int j;
				CloneEntry *seeker = mobj->data.clones.table;

				sciprintf("Free list: ");
				for (j = mobj->data.clones.first_free; j != HEAPENTRY_INVALID; j = mobj->data.clones.table[j].next_free) {
					sciprintf("%d ", j);
				}
				sciprintf("\n");

				sciprintf("Entries w/zero vars: ");
				for (j = 0; j < mobj->data.clones.max_entry; j++) {
					if (mobj->data.clones.table[j].entry.variables == NULL)
						sciprintf("%d ", j);
				}
				sciprintf("\n");

				for (j = 0; j < mobj->data.clones.max_entry; j++) {
					Object *base_obj;

					if (!clone_entry_used(&mobj->data.clones, j)) {
						seeker++;
						continue;
					}
					base_obj = obj_get(s, seeker->entry.variables[SCRIPT_SPECIES_SELECTOR]);
					if (!base_obj) {
						sciprintf("Clone entry without a base class: %d\n", j);
						seeker->entry.base = seeker->entry.base_obj = NULL;
						seeker->entry.base_vars = seeker->entry.base_method = NULL;
						continue;
					}
					seeker->entry.base = base_obj->base;
					seeker->entry.base_obj = base_obj->base_obj;
					seeker->entry.base_vars = base_obj->base_vars;
					seeker->entry.base_method = base_obj->base_method;

					seeker++;
				}

				break;
			}
			default:
				break;
			}
		}
	}
}

int _reset_graphics_input(EngineState *s);

static void reconstruct_sounds(EngineState *s) {
	song_t *seeker;
	int it_type = s->resmgr->_sciVersion >= SCI_VERSION_01 ? SCI_SONG_ITERATOR_TYPE_SCI1 : SCI_SONG_ITERATOR_TYPE_SCI0;

	if (s->sound.songlib.lib)
		seeker = *(s->sound.songlib.lib);
	else {
		song_lib_init(&s->sound.songlib);
		seeker = NULL;
	}

	while (seeker) {
		SongIterator *base, *ff;
		int oldstatus;
		SongIterator::Message msg;

		base = ff = build_iterator(s, seeker->resource_num, it_type, seeker->handle);
		if (seeker->restore_behavior == RESTORE_BEHAVIOR_CONTINUE)
			ff = new_fast_forward_iterator(base, seeker->restore_time);
		ff->init();

		msg = SongIterator::Message(seeker->handle, SIMSG_SET_LOOPS(seeker->loops));
		songit_handle_message(&ff, msg);
		msg = SongIterator::Message(seeker->handle, SIMSG_SET_HOLD(seeker->hold));
		songit_handle_message(&ff, msg);

		oldstatus = seeker->status;
		seeker->status = SOUND_STATUS_STOPPED;
		seeker->it = ff;
		sfx_song_set_status(&s->sound, seeker->handle, oldstatus);
		seeker = seeker->next;
	}
}

void internal_stringfrag_strncpy(EngineState *s, reg_t *dest, reg_t *src, int len);

EngineState *gamestate_restore(EngineState *s, Common::SeekableReadStream *fh) {
	int read_eof = 0;
	EngineState *retval;
	songlib_t temp;

/*
	if (s->sound_server) {
		if ((s->sound_server->restore)(s, dirname)) {
			sciprintf("Restoring failed for the sound subsystem\n");
			return NULL;
		}
	}
*/

	retval = new EngineState();

	retval->savegame_version = -1;
	retval->gfx_state = s->gfx_state;

	SavegameMetadata meta;

%CFSMLREAD-ATOMIC SavegameMetadata (&meta) FROM fh ERRVAR read_eof;

	if (read_eof)
		return false;

	if ((meta.savegame_version < FREESCI_MINIMUM_SAVEGAME_VERSION) ||
	    (meta.savegame_version > FREESCI_CURRENT_SAVEGAME_VERSION)) {
		if (meta.savegame_version < FREESCI_MINIMUM_SAVEGAME_VERSION)
			sciprintf("Old savegame version detected- can't load\n");
		else
			sciprintf("Savegame version is %d- maximum supported is %0d\n", meta.savegame_version, FREESCI_CURRENT_SAVEGAME_VERSION);

		return NULL;
	}

	// Backwards compatibility settings
	retval->dyn_views = NULL;
	retval->drop_views = NULL;
	retval->port = NULL;
	retval->save_dir_copy_buf = NULL;

	retval->sound_mute = s->sound_mute;
	retval->sound_volume = s->sound_volume;

	%CFSMLREAD-ATOMIC EngineState retval FROM fh ERRVAR read_eof;

	sfx_exit(&s->sound);

	// Set exec stack base to zero
	retval->execution_stack_base = 0;
	retval->execution_stack_pos = 0;

	// Now copy all current state information
	// Graphics and input state:
	retval->animation_delay = s->animation_delay;
	retval->animation_granularity = s->animation_granularity;
	retval->gfx_state = s->gfx_state;

	retval->resmgr = s->resmgr;

	temp = retval->sound.songlib;
	sfx_init(&retval->sound, retval->resmgr, s->sfx_init_flags);
	retval->sfx_init_flags = s->sfx_init_flags;
	song_lib_free(retval->sound.songlib);
	retval->sound.songlib = temp;

	_reset_graphics_input(retval);
	reconstruct_stack(retval);
	reconstruct_scripts(retval, retval->seg_manager);
	reconstruct_clones(retval, retval->seg_manager);
	retval->game_obj = s->game_obj;
	retval->script_000 = &retval->seg_manager->heap[script_get_segment(s, 0, SCRIPT_GET_DONT_LOAD)]->data.script;
	retval->gc_countdown = GC_INTERVAL - 1;
	retval->save_dir_copy = make_reg(s->sys_strings_segment, SYS_STRING_SAVEDIR);
	retval->save_dir_edit_offset = 0;
	retval->sys_strings_segment = find_unique_seg_by_type(retval->seg_manager, MEM_OBJ_SYS_STRINGS);
	retval->sys_strings = &(((MemObject *)(GET_SEGMENT(*retval->seg_manager, retval->sys_strings_segment, MEM_OBJ_SYS_STRINGS)))->data.sys_strings);

	// Restore system strings
	SystemString *str;

	// First, pad memory
	for (int i = 0; i < SYS_STRINGS_MAX; i++) {
		str = &retval->sys_strings->strings[i];
		char *data = (char *) str->value;
		if (data) {
			str->value = (reg_t *)sci_malloc(str->max_size + 1);
			strcpy((char *)str->value, data);
			free(data);
		}
	}

	str = &retval->sys_strings->strings[SYS_STRING_SAVEDIR];
	internal_stringfrag_strncpy(s, str->value, s->sys_strings->strings[SYS_STRING_SAVEDIR].value, str->max_size);
	str->value[str->max_size].segment = s->string_frag_segment; // Make sure to terminate
	str->value[str->max_size].offset &= 0xff00; // Make sure to terminate
	
	// Time state:
	retval->last_wait_time = g_system->getMillis();
	retval->game_start_time = g_system->getMillis() - retval->game_time * 1000;

	// static parser information:
	retval->parser_rules = s->parser_rules;
	retval->parser_words_nr = s->parser_words_nr;
	retval->parser_words = s->parser_words;
	retval->_parserSuffixes = s->_parserSuffixes;
	retval->parser_branches_nr = s->parser_branches_nr;
	retval->parser_branches = s->parser_branches;

	// static VM/Kernel information:
	retval->_selectorNames = s->_selectorNames;
	retval->kernel_names_nr = s->kernel_names_nr;
	retval->kernel_names = s->kernel_names;
	retval->kfunct_table = s->kfunct_table;
	retval->kfunct_nr = s->kfunct_nr;
	retval->opcodes = s->opcodes;

	memcpy(&(retval->selector_map), &(s->selector_map), sizeof(selector_map_t));

	retval->max_version = retval->version;
	retval->min_version = retval->version;
	retval->parser_base = make_reg(s->sys_strings_segment, SYS_STRING_PARSER_BASE);

	// Copy breakpoint information from current game instance
	retval->have_bp = s->have_bp;
	retval->bp_list = s->bp_list;

	retval->debug_mode = s->debug_mode;

	retval->kernel_opt_flags = 0;
	retval->have_mouse_flag = 1;

	retval->successor = NULL;
	retval->pic_priority_table = (int*)gfxop_get_pic_metainfo(retval->gfx_state);
	retval->game_name = sci_strdup(obj_get_name(retval, retval->game_obj));

	retval->sound.it = NULL;
	retval->sound.flags = s->sound.flags;
	retval->sound.song = NULL;
	retval->sound.suspended = s->sound.suspended;
	retval->sound.debug = s->sound.debug;
	reconstruct_sounds(retval);

	return retval;
}

bool get_savegame_metadata(Common::SeekableReadStream* stream, SavegameMetadata* meta) {
	int read_eof = 0;

	%CFSMLREAD-ATOMIC SavegameMetadata meta FROM stream ERRVAR read_eof;

	if (read_eof)
		return false;

	if ((meta->savegame_version < FREESCI_MINIMUM_SAVEGAME_VERSION) ||
	    (meta->savegame_version > FREESCI_CURRENT_SAVEGAME_VERSION)) {
		if (meta->savegame_version < FREESCI_MINIMUM_SAVEGAME_VERSION)
			sciprintf("Old savegame version detected- can't load\n");
		else
			sciprintf("Savegame version is %d- maximum supported is %0d\n", meta->savegame_version, FREESCI_CURRENT_SAVEGAME_VERSION);

		return false;
	}

	return true;
}

} // End of namespace Sci
