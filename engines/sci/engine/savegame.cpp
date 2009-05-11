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

#include "common/stream.h"
#include "common/system.h"
#include "common/func.h"
#include "common/serializer.h"

#include <time.h>	// FIXME: For struct tm


#include "sci/sci_memory.h"
#include "sci/gfx/operations.h"
#include "sci/gfx/menubar.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxPort, GfxContainer
#include "sci/sfx/core.h"
#include "sci/sfx/iterator.h"
#include "sci/engine/state.h"
#include "sci/engine/intmap.h"
#include "sci/engine/savegame.h"

namespace Sci {

// from ksound.cpp:
SongIterator *build_iterator(EngineState *s, int song_nr, int type, songit_id_t id);

#pragma mark -

// TODO: Many of the following sync_*() methods should be turned into member funcs
// of the classes they are syncing.

static void sync_MemObjPtr(Common::Serializer &s, MemObject *&obj);
static void sync_songlib_t(Common::Serializer &s, songlib_t &obj);

static void sync_reg_t(Common::Serializer &s, reg_t &obj) {
	s.syncAsUint16LE(obj.segment);
	s.syncAsUint16LE(obj.offset);
}

// FIXME: Sync a C string, using malloc/free storage.
// Much better to replace all of these by Common::String
static void syncCStr(Common::Serializer &s, char **str) {
	Common::String tmp;
	if (s.isSaving() && *str)
		tmp = *str;
	s.syncString(tmp);
	if (s.isLoading()) {
		//free(*str);
		*str = strdup(tmp.c_str());
	}
}


static void sync_song_t(Common::Serializer &s, song_t &obj) {
	s.syncAsSint32LE(obj.handle);
	s.syncAsSint32LE(obj.resource_num);
	s.syncAsSint32LE(obj.priority);
	s.syncAsSint32LE(obj.status);
	s.syncAsSint32LE(obj.restore_behavior);
	s.syncAsSint32LE(obj.restore_time);
	s.syncAsSint32LE(obj.loops);
	s.syncAsSint32LE(obj.hold);
	
	if (s.isLoading()) {
		obj._delay = 0;
		obj.it = 0;
		obj.next_playing = 0;
		obj.next_stopping = 0;
		obj.next = 0;
	}
}


// Experimental hack: Use syncWithSerializer to sync. By default, this assume
// the object to be synced is a subclass of Serializable and thus tries to invoke
// the saveLoadWithSerializer() method. But it is possible to specialize this
// template function to handle stuff that is not implementing that interface.
template<typename T>
void syncWithSerializer(Common::Serializer &s, T &obj) {
	obj.saveLoadWithSerializer(s);
}

// By default, sync using syncWithSerializer, which in turn can easily be overloaded.
template <typename T>
struct DefaultSyncer : Common::BinaryFunction<Common::Serializer, T, void> {
	void operator()(Common::Serializer &s, T &obj) const {
		//obj.saveLoadWithSerializer(s);
		syncWithSerializer(s, obj);
	}
};

/**
 * Sync a Common::Array using a Common::Serializer.
 * When saving, this writes the length of the array, then syncs (writes) all entries.
 * When loading, it loads the length of the array, then resizes it accordingly, before
 * syncing all entries.
 *
 * Note: This shouldn't be in common/array.h nor in common/serializer.h, after
 * all, not all code using arrays wants to use the serializer, and vice versa.
 * But we could put this into a separate header file in common/ at some point.
 * Something like common/serializer-extras.h or so.
 *
 * TODO: Add something like this for lists, queues....
 */
template <typename T, class Syncer = DefaultSyncer<T> >
struct ArraySyncer : Common::BinaryFunction<Common::Serializer, T, void> {
	void operator()(Common::Serializer &s, Common::Array<T> &arr) const {
		uint len = arr.size();
		s.syncAsUint32LE(len);
		Syncer sync;

		// Resize the array if loading.
		if (s.isLoading())
			arr.resize(len);

		typename Common::Array<T>::iterator i;
		for (i = arr.begin(); i != arr.end(); ++i) {
			sync(s, *i);
		}
	}
};

// Convenience wrapper
template<typename T>
void syncArray(Common::Serializer &s, Common::Array<T> &arr) {
	ArraySyncer<T> sync;
	sync(s, arr);
}



void MenuItem::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_type);
	s.syncString(_keytext);
	s.skip(4); 	// Used to be keytext_size (an already unused field)

	s.syncAsSint32LE(_flags);
	s.syncBytes(_said, MENU_SAID_SPEC_SIZE);
	sync_reg_t(s, _saidPos);
	s.syncString(_text);
	sync_reg_t(s, _textPos);
	s.syncAsSint32LE(_modifiers);
	s.syncAsSint32LE(_key);
	s.syncAsSint32LE(_enabled);
	s.syncAsSint32LE(_tag);
}

void Menu::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncString(_title);
	s.syncAsSint32LE(_titleWidth);
	s.syncAsSint32LE(_width);

	syncArray<MenuItem>(s, _items);
}


void Menubar::saveLoadWithSerializer(Common::Serializer &s) {
	syncArray<Menu>(s, _menus);
}

void SegManager::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(reserved_id);
	s.syncAsSint32LE(exports_wide);
	s.syncAsSint32LE(gc_mark_bits);

	id_seg_map->saveLoadWithSerializer(s);

	uint sync_heap_size = _heap.size();
	s.syncAsUint32LE(sync_heap_size);
	_heap.resize(sync_heap_size);
	for (uint i = 0; i < sync_heap_size; ++i)
		sync_MemObjPtr(s, _heap[i]);

	s.syncAsSint32LE(Clones_seg_id);
	s.syncAsSint32LE(Lists_seg_id);
	s.syncAsSint32LE(Nodes_seg_id);
}

static void sync_SegManagerPtr(Common::Serializer &s, SegManager *&obj) {
	bool sci11 = false;

	if (s.isSaving()) {
		assert(obj);
		sci11 = obj->isSci1_1;
	}

	s.syncAsByte(sci11);

	if (s.isLoading()) {
		// FIXME: Do in-place loading at some point, instead of creating a new EngineState instance from scratch.
		delete obj;
		obj = new SegManager(sci11);
	}
	
	obj->saveLoadWithSerializer(s);
}



template <>
void syncWithSerializer(Common::Serializer &s, Class &obj) {
	s.syncAsSint32LE(obj.script);
	sync_reg_t(s, obj.reg);
}

static void sync_sfx_state_t(Common::Serializer &s, sfx_state_t &obj) {
	sync_songlib_t(s, obj.songlib);
}

static void sync_SavegameMetadata(Common::Serializer &s, SavegameMetadata &obj) {
	// TODO: It would be a good idea to store a magic number & a header size here,
	// so that we can implement backward compatibility if the savegame format changes.

	s.syncString(obj.savegame_name);
	s.syncAsSint32LE(obj.savegame_version);
	s.syncString(obj.game_version);
	s.syncAsSint32LE(obj.version);
	s.syncAsSint32LE(obj.savegame_date);
	s.syncAsSint32LE(obj.savegame_time);
}

void EngineState::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(savegame_version);

	syncCStr(s, &game_version);
	s.syncAsSint32LE(version);

	// FIXME: Do in-place loading at some point, instead of creating a new EngineState instance from scratch.
	if (s.isLoading()) {
		//free(menubar);
		_menubar = new Menubar();
	} else
		assert(_menubar);
	_menubar->saveLoadWithSerializer(s);

	s.syncAsSint32LE(status_bar_foreground);
	s.syncAsSint32LE(status_bar_background);

	sync_SegManagerPtr(s, seg_manager);


	syncArray<Class>(s, _classtable);

	sync_sfx_state_t(s, sound);
}

static void sync_LocalVariables(Common::Serializer &s, LocalVariables &obj) {
	s.syncAsSint32LE(obj.script_id);

	s.syncAsSint32LE(obj.nr);
	if (!obj.locals && obj.nr)
		obj.locals = (reg_t *)sci_calloc(obj.nr, sizeof(reg_t));
	for (int i = 0; i < obj.nr; ++i)
		sync_reg_t(s, obj.locals[i]);
}

template <>
void syncWithSerializer(Common::Serializer &s, Object &obj) {
	s.syncAsSint32LE(obj.flags);
	sync_reg_t(s, obj.pos);
	s.syncAsSint32LE(obj.variable_names_nr);
	s.syncAsSint32LE(obj.methods_nr);

	s.syncAsSint32LE(obj.variables_nr);
	if (!obj.variables && obj.variables_nr)
		obj.variables = (reg_t *)sci_calloc(obj.variables_nr, sizeof(reg_t));
	for (int i = 0; i < obj.variables_nr; ++i)
		sync_reg_t(s, obj.variables[i]);
}

template <>
void syncWithSerializer(Common::Serializer &s, Table<Clone>::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	syncWithSerializer<Object>(s, obj);
}

template <>
void syncWithSerializer(Common::Serializer &s, Table<List>::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	sync_reg_t(s, obj.first);
	sync_reg_t(s, obj.last);
}

template <>
void syncWithSerializer(Common::Serializer &s, Table<Node>::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	sync_reg_t(s, obj.pred);
	sync_reg_t(s, obj.succ);
	sync_reg_t(s, obj.key);
	sync_reg_t(s, obj.value);
}

template <typename T>
void sync_Table(Common::Serializer &s, T &obj) {
	// TODO: Change this to use syncArray. This involves breaking the savegame format compatibility.
	s.syncAsSint32LE(obj.first_free);
	s.syncAsSint32LE(obj.entries_used);

	syncArray<typename T::Entry>(s, obj._table);
}

static void sync_Script(Common::Serializer &s, Script &obj) {
	s.syncAsSint32LE(obj.nr);
	s.syncAsUint32LE(obj.buf_size);
	s.syncAsUint32LE(obj.script_size);
	s.syncAsUint32LE(obj.heap_size);

	// FIXME: revamp obj_indices handling
	if (!obj.obj_indices) {
		assert(s.isLoading());
		obj.obj_indices = new IntMapper();
	}

	obj.obj_indices->saveLoadWithSerializer(s);

	s.syncAsSint32LE(obj.exports_nr);
	s.syncAsSint32LE(obj.synonyms_nr);
	s.syncAsSint32LE(obj.lockers);

	syncArray<Object>(s, obj._objects);

	s.syncAsSint32LE(obj.locals_offset);
	s.syncAsSint32LE(obj.locals_segment);

	s.syncAsSint32LE(obj.marked_as_deleted);
}

static void sync_SystemString(Common::Serializer &s, SystemString &obj) {
	syncCStr(s, &obj.name);
	s.syncAsSint32LE(obj.max_size);
	
	// FIXME: This is a *WEIRD* hack: We sync a reg_t* as if it was a string.
	// No idea why, but this mimicks what the old save/load code used to do.
	syncCStr(s, (char **)&obj.value);
}

static void sync_SystemStrings(Common::Serializer &s, SystemStrings &obj) {
	for (int i = 0; i < SYS_STRINGS_MAX; ++i)
		sync_SystemString(s, obj.strings[i]);
}

static void sync_DynMem(Common::Serializer &s, DynMem &obj) {
	s.syncAsSint32LE(obj._size);
	syncCStr(s, &obj._description);
	if (!obj._buf && obj._size) {
		obj._buf = (byte *)sci_calloc(obj._size, 1);
	}
	if (obj._size)
		s.syncBytes(obj._buf, obj._size);
}

static void sync_DataStack(Common::Serializer &s, DataStack &obj) {
	s.syncAsUint32LE(obj.nr);
	if (s.isLoading()) {
		//free(obj.entries);
		obj.entries = (reg_t *)sci_calloc(obj.nr, sizeof(reg_t));
	}
}

#pragma mark -

static void sync_songlib_t(Common::Serializer &s, songlib_t &obj) {
	int songcount = 0;
	if (s.isSaving())
		songcount = song_lib_count(obj);
	s.syncAsUint32LE(songcount);
	
	if (s.isLoading()) {
		song_lib_init(&obj);
		while (songcount--) {
			song_t *newsong = (song_t *)calloc(1, sizeof(song_t));
			sync_song_t(s, *newsong);
			song_lib_add(obj, newsong);
		}
	} else {
		song_t *seeker = *(obj.lib);
		while (seeker) {
			seeker->restore_time = seeker->it->getTimepos();
			sync_song_t(s, *seeker);
			seeker = seeker->next;
		}
	}
}

static void sync_MemObjPtr(Common::Serializer &s, MemObject *&mobj) {
	// Sync the memobj type
	MemObjectType type = (s.isSaving() && mobj) ? mobj->getType() : MEM_OBJ_INVALID;
	s.syncAsUint32LE(type);

	// If we were saving and mobj == 0, or if we are loading and this is an
	// entry marked as empty -> we are done.
	if (type == MEM_OBJ_INVALID) {
		mobj = 0;
		return;
	}

	if (s.isLoading()) {
		//assert(!mobj);
		mobj = MemObject::createMemObject(type);
	} else {
		assert(mobj);
	}
	
	s.syncAsSint32LE(mobj->_segmgrId);
	switch (type) {
	case MEM_OBJ_SCRIPT:
		sync_Script(s, *(Script *)mobj);
		break;
	case MEM_OBJ_CLONES:
		sync_Table<CloneTable>(s, *(CloneTable *)mobj);
		break;
	case MEM_OBJ_LOCALS:
		sync_LocalVariables(s, *(LocalVariables *)mobj);
		break;
	case MEM_OBJ_SYS_STRINGS:
		sync_SystemStrings(s, *(SystemStrings *)mobj);
		break;
	case MEM_OBJ_STACK:
		sync_DataStack(s, *(DataStack *)mobj);
		break;
	case MEM_OBJ_HUNK:
		if (s.isLoading()) {
			(*(HunkTable *)mobj).initTable();
		}
		break;
	case MEM_OBJ_STRING_FRAG:
		break;
	case MEM_OBJ_LISTS:
		sync_Table<ListTable>(s, *(ListTable *)mobj);
		break;
	case MEM_OBJ_NODES:
		sync_Table<NodeTable>(s, *(NodeTable *)mobj);
		break;
	case MEM_OBJ_DYNMEM:
		sync_DynMem(s, *(DynMem *)mobj);
		break;
	default:
		error("Unknown MemObject type %d", type);
		break;
	}
}


#pragma mark -


int gamestate_save(EngineState *s, Common::WriteStream *fh, const char* savename) {
	tm curTime;
	g_system->getTimeAndDate(curTime);

	SavegameMetadata meta;
	meta.savegame_version = CURRENT_SAVEGAME_VERSION;
	meta.savegame_name = savename;
	meta.version = s->version;
	meta.game_version = s->game_version;
	meta.savegame_date = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	meta.savegame_time = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);

	s->savegame_version = CURRENT_SAVEGAME_VERSION;

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

	Common::Serializer ser(0, fh);
	sync_SavegameMetadata(ser, meta);
	s->saveLoadWithSerializer(ser);		// FIXME: Error handling?

	return 0;
}

// FIXME: This should probably be turned into a SegManager method
static SegmentId find_unique_seg_by_type(SegManager *self, int type) {
	for (uint i = 0; i < self->_heap.size(); i++)
		if (self->_heap[i] &&
		    self->_heap[i]->getType() == type)
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

// FIXME: This should probably be turned into an EngineState method
static void reconstruct_stack(EngineState *retval) {
	SegmentId stack_seg = find_unique_seg_by_type(retval->seg_manager, MEM_OBJ_STACK);
	DataStack *stack = (DataStack *)(retval->seg_manager->_heap[stack_seg]);

	retval->stack_segment = stack_seg;
	retval->stack_base = stack->entries;
	retval->stack_top = retval->stack_base + VM_STACK_SIZE;
}

static bool clone_entry_used(CloneTable *table, int n) {
	int seeker = table->first_free;

	while (seeker != CloneTable::HEAPENTRY_INVALID) {
		if (seeker == n)
			return false;
		seeker = table->_table[seeker].next_free;
	}

	return true;
}

static void load_script(EngineState *s, SegmentId seg) {
	Resource *script, *heap = NULL;
	Script *scr = (Script *)(s->seg_manager->_heap[seg]);

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

// FIXME: The following should likely become a SegManager method
static void reconstruct_scripts(EngineState *s, SegManager *self) {
	uint i, j;
	MemObject *mobj;
	for (i = 0; i < self->_heap.size(); i++) {
		if (self->_heap[i]) {
			mobj = self->_heap[i];
			switch (mobj->getType())  {
			case MEM_OBJ_SCRIPT: {
				Script *scr = (Script *)mobj;

				load_script(s, i);
				scr->locals_block = scr->locals_segment == 0 ? NULL : (LocalVariables *)(s->seg_manager->_heap[scr->locals_segment]);
				scr->export_table = (uint16 *) find_unique_script_block(s, scr->buf, sci_obj_exports);
				scr->synonyms = find_unique_script_block(s, scr->buf, sci_obj_synonyms);
				scr->_codeBlocks.clear();

				if (!self->isSci1_1)
					scr->export_table += 3;

				for (j = 0; j < scr->_objects.size(); j++) {
					byte *data = scr->buf + scr->_objects[j].pos.offset;
					scr->_objects[j].base = scr->buf;
					scr->_objects[j].base_obj = data;
				}
				break;
			}
			default:
				break;
			}
		}
	}

	for (i = 0; i < self->_heap.size(); i++) {
		if (self->_heap[i]) {
			mobj = self->_heap[i];
			switch (mobj->getType())  {
			case MEM_OBJ_SCRIPT: {
				Script *scr = (Script *)mobj;

				for (j = 0; j < scr->_objects.size(); j++) {
					byte *data = scr->buf + scr->_objects[j].pos.offset;

					if (self->isSci1_1) {
						uint16 *funct_area = (uint16 *) (scr->buf + READ_LE_UINT16( data + 6 ));
						uint16 *prop_area = (uint16 *) (scr->buf + READ_LE_UINT16( data + 4 ));

						scr->_objects[j].base_method = funct_area;
						scr->_objects[j].base_vars = prop_area;
					} else {
						int funct_area = READ_LE_UINT16( data + SCRIPT_FUNCTAREAPTR_OFFSET );
						Object *base_obj;

						base_obj = obj_get(s, scr->_objects[j].variables[SCRIPT_SPECIES_SELECTOR]);

						if (!base_obj) {
							sciprintf("Object without a base class: Script %d, index %d (reg address "PREG"\n",
								  scr->nr, j, PRINT_REG(scr->_objects[j].variables[SCRIPT_SPECIES_SELECTOR]));
							continue;
						}
						scr->_objects[j].variable_names_nr = base_obj->variables_nr;
						scr->_objects[j].base_obj = base_obj->base_obj;

						scr->_objects[j].base_method = (uint16 *)(data + funct_area);
						scr->_objects[j].base_vars = (uint16 *)(data + scr->_objects[j].variable_names_nr * 2 + SCRIPT_SELECTOR_OFFSET);
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

// FIXME: The following should likely become a SegManager method
static void reconstruct_clones(EngineState *s, SegManager *self) {
	for (uint i = 0; i < self->_heap.size(); i++) {
		if (self->_heap[i]) {
			MemObject *mobj = self->_heap[i];
			switch (mobj->getType()) {
			case MEM_OBJ_CLONES: {
				CloneTable *ct = (CloneTable *)mobj;

				/*
				sciprintf("Free list: ");
				for (uint j = ct->first_free; j != HEAPENTRY_INVALID; j = ct->_table[j].next_free) {
					sciprintf("%d ", j);
				}
				sciprintf("\n");

				sciprintf("Entries w/zero vars: ");
				for (uint j = 0; j < ct->_table.size(); j++) {
					if (ct->_table[j].variables == NULL)
						sciprintf("%d ", j);
				}
				sciprintf("\n");
				*/

				for (uint j = 0; j < ct->_table.size(); j++) {
					Object *base_obj;

					if (!clone_entry_used(ct, j)) {
						continue;
					}
					CloneTable::Entry &seeker = ct->_table[j];
					base_obj = obj_get(s, seeker.variables[SCRIPT_SPECIES_SELECTOR]);
					if (!base_obj) {
						sciprintf("Clone entry without a base class: %d\n", j);
						seeker.base = seeker.base_obj = NULL;
						seeker.base_vars = seeker.base_method = NULL;
					} else {
						seeker.base = base_obj->base;
						seeker.base_obj = base_obj->base_obj;
						seeker.base_vars = base_obj->base_vars;
						seeker.base_method = base_obj->base_method;
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

	// FIXME: Do in-place loading at some point, instead of creating a new EngineState instance from scratch.
	retval = new EngineState();

	retval->savegame_version = -1;
	retval->gfx_state = s->gfx_state;

	SavegameMetadata meta;

	Common::Serializer ser(fh, 0);
	sync_SavegameMetadata(ser, meta);

	if (fh->eos())
		return false;

	if ((meta.savegame_version < MINIMUM_SAVEGAME_VERSION) ||
	    (meta.savegame_version > CURRENT_SAVEGAME_VERSION)) {
		if (meta.savegame_version < MINIMUM_SAVEGAME_VERSION)
			sciprintf("Old savegame version detected- can't load\n");
		else
			sciprintf("Savegame version is %d- maximum supported is %0d\n", meta.savegame_version, CURRENT_SAVEGAME_VERSION);

		return NULL;
	}

	// Backwards compatibility settings
	retval->dyn_views = NULL;
	retval->drop_views = NULL;
	retval->port = NULL;
	retval->save_dir_copy_buf = NULL;

	retval->sound_mute = s->sound_mute;
	retval->sound_volume = s->sound_volume;

	retval->saveLoadWithSerializer(ser);	// FIXME: Error handling?

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
	retval->script_000 = retval->seg_manager->getScript(script_get_segment(s, 0, SCRIPT_GET_DONT_LOAD), SEG_ID);
	retval->gc_countdown = GC_INTERVAL - 1;
	retval->save_dir_copy = make_reg(s->sys_strings_segment, SYS_STRING_SAVEDIR);
	retval->save_dir_edit_offset = 0;
	retval->sys_strings_segment = find_unique_seg_by_type(retval->seg_manager, MEM_OBJ_SYS_STRINGS);
	retval->sys_strings = (SystemStrings *)GET_SEGMENT(*retval->seg_manager, retval->sys_strings_segment, MEM_OBJ_SYS_STRINGS);

	// Restore system strings
	SystemString *str;

	// First, pad memory
	for (int i = 0; i < SYS_STRINGS_MAX; i++) {
		str = &retval->sys_strings->strings[i];
		char *data = (char *)str->value;
		if (data) {
			str->value = (reg_t *)calloc(str->max_size+1, sizeof(char));	// FIXME -- sizeof(char) or sizeof(reg_t) ??
			strncpy((char *)str->value, data, str->max_size+1);		// FIXME -- strncpy or internal_stringfrag_strncpy ?
			free(data);
		}
	}

	str = &retval->sys_strings->strings[SYS_STRING_SAVEDIR];
	internal_stringfrag_strncpy(s, str->value, s->sys_strings->strings[SYS_STRING_SAVEDIR].value, str->max_size);
	str->value[str->max_size - 1].segment = s->string_frag_segment; // Make sure to terminate
	str->value[str->max_size - 1].offset &= 0xff00; // Make sure to terminate
	
	// Time state:
	retval->last_wait_time = g_system->getMillis();
	retval->game_start_time = g_system->getMillis() - retval->game_time * 1000;

	// static parser information:
	retval->parser_rules = s->parser_rules;
	retval->_parserWords = s->_parserWords;
	retval->_parserSuffixes = s->_parserSuffixes;
	retval->_parserBranches = s->_parserBranches;

	// static VM/Kernel information:
	retval->_selectorNames = s->_selectorNames;
	retval->_kernelNames = s->_kernelNames;
	retval->_kfuncTable = s->_kfuncTable;
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
	retval->_gameName = obj_get_name(retval, retval->game_obj);

	retval->sound.it = NULL;
	retval->sound.flags = s->sound.flags;
	retval->sound.song = NULL;
	retval->sound.suspended = s->sound.suspended;
	retval->sound.debug = s->sound.debug;
	reconstruct_sounds(retval);

	return retval;
}

bool get_savegame_metadata(Common::SeekableReadStream *stream, SavegameMetadata *meta) {
	assert(stream);
	assert(meta);

	Common::Serializer ser(stream, 0);
	sync_SavegameMetadata(ser, *meta);

	if (stream->eos())
		return false;

	if ((meta->savegame_version < MINIMUM_SAVEGAME_VERSION) ||
	    (meta->savegame_version > CURRENT_SAVEGAME_VERSION)) {
		if (meta->savegame_version < MINIMUM_SAVEGAME_VERSION)
			sciprintf("Old savegame version detected- can't load\n");
		else
			sciprintf("Savegame version is %d- maximum supported is %0d\n", meta->savegame_version, CURRENT_SAVEGAME_VERSION);

		return false;
	}

	return true;
}

} // End of namespace Sci
