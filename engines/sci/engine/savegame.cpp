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
#include "graphics/thumbnail.h"

#include "sci/sci.h"
#include "sci/event.h"
#ifdef INCLUDE_OLDGFX
#include "sci/gfx/menubar.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxPort, GfxContainer
#endif

#include "sci/engine/state.h"
#include "sci/engine/message.h"
#include "sci/engine/savegame.h"
#include "sci/engine/vm_types.h"
#include "sci/gui/gui.h"
#include "sci/sfx/audio.h"
#ifdef USE_OLD_MUSIC_FUNCTIONS
#include "sci/sfx/iterator/core.h"
#include "sci/sfx/iterator/iterator.h"
#else
#include "sci/sfx/music.h"
#endif

namespace Sci {


#define VER(x) Common::Serializer::Version(x)


// OBSOLETE: This const is used for backward compatibility only.
const uint32 INTMAPPER_MAGIC_KEY = 0xDEADBEEF;


#ifdef USE_OLD_MUSIC_FUNCTIONS
// from ksound.cpp:
SongIterator *build_iterator(ResourceManager *resMan, int song_nr, SongIteratorType type, songit_id_t id);
#endif


#pragma mark -

// TODO: Many of the following sync_*() methods should be turned into member funcs
// of the classes they are syncing.

#ifdef USE_OLD_MUSIC_FUNCTIONS
static void sync_songlib(Common::Serializer &s, SongLibrary &obj);
#endif

static void sync_reg_t(Common::Serializer &s, reg_t &obj) {
	s.syncAsUint16LE(obj.segment);
	s.syncAsUint16LE(obj.offset);
}

#ifdef USE_OLD_MUSIC_FUNCTIONS
static void syncSong(Common::Serializer &s, Song &obj) {
	s.syncAsSint32LE(obj._handle);
	s.syncAsSint32LE(obj._resourceNum);
	s.syncAsSint32LE(obj._priority);
	s.syncAsSint32LE(obj._status);
	s.syncAsSint32LE(obj._restoreBehavior);
	s.syncAsSint32LE(obj._restoreTime);
	s.syncAsSint32LE(obj._loops);
	s.syncAsSint32LE(obj._hold);

	if (s.isLoading()) {
		obj._it = 0;
		obj._delay = 0;
		obj._next = 0;
		obj._nextPlaying = 0;
		obj._nextStopping = 0;
	}
}
#else

#define DEFROBNICATE_HANDLE(handle) (make_reg((handle >> 16) & 0xffff, handle & 0xffff))

static void syncSong(Common::Serializer &s, MusicEntry *song) {
	if (s.getVersion() < 14) {
		// Old sound system data. This data is only loaded, never saved (as we're never
		// saving in the older version format)
		uint32 handle = 0;
		s.syncAsSint32LE(handle);
		song->soundObj = DEFROBNICATE_HANDLE(handle);
		s.syncAsSint32LE(song->resnum);
		s.syncAsSint32LE(song->prio);
		s.syncAsSint32LE(song->status);
		s.skip(4);	// restoreBehavior
		uint32 restoreTime = 0;
		s.syncAsSint32LE(restoreTime);
		song->ticker = restoreTime * 60 / 1000;
		s.syncAsSint32LE(song->loop);
		s.skip(4);	// hold
		// volume and dataInc will be synced from the sound objects
		// when the sound list is reconstructed in gamestate_restore()
		song->volume = 100;
		song->dataInc = 0;
		// No fading info
		song->fadeTo = 0;
		song->fadeStep = 0;
		song->fadeTicker = 0;
		song->fadeTickerStep = 0;
	} else {
		// A bit more optimized saving
		sync_reg_t(s, song->soundObj);
		s.syncAsSint16LE(song->resnum);
		s.syncAsSint16LE(song->dataInc);
		s.syncAsSint16LE(song->ticker);
		s.syncAsByte(song->prio);
		s.syncAsByte(song->loop);
		s.syncAsByte(song->volume);
		s.syncAsByte(song->fadeTo);
		s.syncAsSint16LE(song->fadeStep);
		s.syncAsSint32LE(song->fadeTicker);
		s.syncAsSint32LE(song->fadeTickerStep);
		s.syncAsByte(song->status);
	}

	// pMidiParser and pStreamAud will be initialized when the
	// sound list is reconstructed in gamestate_restore()
	if (s.isLoading()) {
		song->soundRes = 0;
		song->pMidiParser = 0;
		song->pStreamAud = 0;
	}
}
#endif

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


template <>
void syncWithSerializer(Common::Serializer &s, reg_t &obj) {
	sync_reg_t(s, obj);
}


#ifdef INCLUDE_OLDGFX

void MenuItem::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_type);
	s.syncString(_keytext);
	s.skip(4, VER(9), VER(9)); 	// OBSOLETE: Used to be keytext_size

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

#endif

void SegManager::saveLoadWithSerializer(Common::Serializer &s) {
	s.skip(4, VER(9), VER(9));	// OBSOLETE: Used to be reserved_id
	s.syncAsSint32LE(_exportsAreWide);
	s.skip(4, VER(9), VER(9));	// OBSOLETE: Used to be gc_mark_bits

	if (s.isLoading()) {
		// Reset _scriptSegMap, to be restored below
		_scriptSegMap.clear();

		if (s.getVersion() <= 9) {
			// OBSOLETE: Skip over the old id_seg_map when loading (we now
			// regenerate the equivalent data, in _scriptSegMap, from scratch).

			s.skip(4);	// base_value
			while (true) {
				uint32 key = 0;
				s.syncAsSint32LE(key);
				if (key == INTMAPPER_MAGIC_KEY)
					break;
				s.skip(4);	// idx
			}
		}
	}


	uint sync_heap_size = _heap.size();
	s.syncAsUint32LE(sync_heap_size);
	_heap.resize(sync_heap_size);
	for (uint i = 0; i < sync_heap_size; ++i) {
		SegmentObj *&mobj = _heap[i];

		// Sync the segment type
		SegmentType type = (s.isSaving() && mobj) ? mobj->getType() : SEG_TYPE_INVALID;
		s.syncAsUint32LE(type);

		// If we were saving and mobj == 0, or if we are loading and this is an
		// entry marked as empty -> skip to next
		if (type == SEG_TYPE_INVALID) {
			continue;
		}

		s.skip(4, VER(9), VER(9));	// OBSOLETE: Used to be _segManagerId

		// Don't save or load HunkTable segments
		if (type == SEG_TYPE_HUNK) {
			continue;
		}

		// Handle the OBSOLETE type SEG_TYPE_STRING_FRAG -- just ignore it
		if (s.isLoading() && type == SEG_TYPE_STRING_FRAG) {
			continue;
		}


		if (s.isLoading()) {
			mobj = SegmentObj::createSegmentObj(type);
		}
		assert(mobj);

		// Let the object sync custom data
		mobj->saveLoadWithSerializer(s);

		// If we are loading a script, hook it up in the script->segment map.
		if (s.isLoading() && type == SEG_TYPE_SCRIPT) {
			_scriptSegMap[((Script *)mobj)->_nr] = i;
		}
	}

	s.syncAsSint32LE(Clones_seg_id);
	s.syncAsSint32LE(Lists_seg_id);
	s.syncAsSint32LE(Nodes_seg_id);
}

static void sync_SegManagerPtr(Common::Serializer &s, ResourceManager *&resMan, SegManager *&obj) {
	s.skip(1, VER(9), VER(9));	// obsolete: used to be a flag indicating if we got sci11 or not

	if (s.isLoading())
		obj->resetSegMan();

	obj->saveLoadWithSerializer(s);
}



template <>
void syncWithSerializer(Common::Serializer &s, Class &obj) {
	s.syncAsSint32LE(obj.script);
	sync_reg_t(s, obj.reg);
}

static void sync_SavegameMetadata(Common::Serializer &s, SavegameMetadata &obj) {
	// TODO: It would be a good idea to store a magic number & a header size here,
	// so that we can implement backward compatibility if the savegame format changes.

	s.syncString(obj.savegame_name);
	s.syncVersion(CURRENT_SAVEGAME_VERSION);
	obj.savegame_version = s.getVersion();
	s.syncString(obj.game_version);
	s.skip(4, VER(9), VER(9));	// obsolete: used to be game version
	s.syncAsSint32LE(obj.savegame_date);
	s.syncAsSint32LE(obj.savegame_time);
}

void EngineState::saveLoadWithSerializer(Common::Serializer &s) {
	s.skip(4, VER(9), VER(9));	// OBSOLETE: Used to be savegame_version

	Common::String tmp;
	s.syncString(tmp);			// OBSOLETE: Used to be game_version
	s.skip(4, VER(9), VER(9));	// OBSOLETE: Used to be version

#ifdef INCLUDE_OLDGFX
	if (s.isLoading()) {
		//free(menubar);
		_menubar = new Menubar();
	} else
		assert(_menubar);
	_menubar->saveLoadWithSerializer(s);
#else
	// FIXME: This code goes out of sync when loading. Find out why

	// OBSOLETE: Saved menus. Skip all of the saved data
	if (s.getVersion() < 14) {
		int totalMenus = 0;
		s.syncAsUint32LE(totalMenus);

		// Now iterate through the obsolete saved menu data
		for (int i = 0; i < totalMenus; i++) {
			s.syncString(tmp);				// OBSOLETE: Used to be _title
			s.skip(4, VER(12), VER(12));	// OBSOLETE: Used to be _titleWidth
			s.skip(4, VER(12), VER(12));	// OBSOLETE: Used to be _width

			int menuLength = 0;
			s.syncAsUint32LE(menuLength);

			for (int j = 0; j < menuLength; j++) {
				s.skip(4, VER(12), VER(12));		// OBSOLETE: Used to be _type
				s.syncString(tmp);					// OBSOLETE: Used to be _keytext
				s.skip(4, VER(9), VER(9)); 			// OBSOLETE: Used to be keytext_size

				s.skip(4, VER(12), VER(12));		// OBSOLETE: Used to be _flags
				s.skip(64, VER(12), VER(12));		// OBSOLETE: Used to be MENU_SAID_SPEC_SIZE
				s.skip(4, VER(12), VER(12));		// OBSOLETE: Used to be _saidPos
				s.syncString(tmp);					// OBSOLETE: Used to be _text
				s.skip(4, VER(12), VER(12));		// OBSOLETE: Used to be _textPos
				s.skip(4 * 4, VER(12), VER(12));	// OBSOLETE: Used to be _modifiers, _key, _enabled and _tag
			}
		}
	}
#endif

	s.skip(4, VER(12), VER(12));	// obsolete: used to be status_bar_foreground
	s.skip(4, VER(12), VER(12));	// obsolete: used to be status_bar_background

	if (s.getVersion() >= 13) {
		// Save/Load picPort as well (cause sierra sci also does this)
		int16 picPortTop, picPortLeft;
		Common::Rect picPortRect;
		if (s.isSaving())
			picPortRect = _gui->getPortPic(picPortTop, picPortLeft);

		s.syncAsSint16LE(picPortRect.top);
		s.syncAsSint16LE(picPortRect.left);
		s.syncAsSint16LE(picPortRect.bottom);
		s.syncAsSint16LE(picPortRect.right);
		s.syncAsSint16LE(picPortTop);
		s.syncAsSint16LE(picPortLeft);

		#ifndef USE_OLDGFX
			if (s.isLoading())
				_gui->setPortPic(picPortRect, picPortTop, picPortLeft, true);
		#endif
	}

	sync_SegManagerPtr(s, resMan, _segMan);

	syncArray<Class>(s, _segMan->_classtable);

#ifdef USE_OLD_MUSIC_FUNCTIONS
	sync_songlib(s, _sound._songlib);
#else
	_soundCmd->_music->saveLoadWithSerializer(s);
#endif
}

void LocalVariables::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(script_id);
	syncArray<reg_t>(s, _locals);
}


void Object::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_flags);
	sync_reg_t(s, _pos);
	s.skip(4, VER(9), VER(12));			// OBSOLETE: Used to be variable_names_nr
	s.syncAsSint32LE(_methodCount);		// that's actually a uint16

	syncArray<reg_t>(s, _variables);
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
	s.syncAsSint32LE(obj.first_free);
	s.syncAsSint32LE(obj.entries_used);

	syncArray<typename T::Entry>(s, obj._table);
}

void CloneTable::saveLoadWithSerializer(Common::Serializer &s) {
	sync_Table<CloneTable>(s, *this);
}

void NodeTable::saveLoadWithSerializer(Common::Serializer &s) {
	sync_Table<NodeTable>(s, *this);
}

void ListTable::saveLoadWithSerializer(Common::Serializer &s) {
	sync_Table<ListTable>(s, *this);
}

void HunkTable::saveLoadWithSerializer(Common::Serializer &s) {
	// Do nothing, hunk tables are not actually saved nor loaded.
}

void Script::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_nr);
	s.syncAsUint32LE(_bufSize);
	s.syncAsUint32LE(_scriptSize);
	s.syncAsUint32LE(_heapSize);

	if (s.getVersion() <= 10) {
		assert((s.isLoading()));
		// OBSOLETE: Skip over the old _objIndices data when loading
		s.skip(4);	// base_value
		while (true) {
			uint32 key = 0;
			s.syncAsSint32LE(key);
			if (key == INTMAPPER_MAGIC_KEY)
				break;
			s.skip(4);	// idx
		}
	}

	s.syncAsSint32LE(_numExports);
	s.syncAsSint32LE(_numSynonyms);
	s.syncAsSint32LE(_lockers);

	// Sync _objects. This is a hashmap, and we use the following on disk format:
	// First we store the number of items in the hashmap, then we store each
	// object (which is an 'Object' instance). For loading, we take advantage
	// of the fact that the key of each Object obj is just obj._pos.offset !
	// By "chance" this format is identical to the format used to sync Common::Array<>,
	// hence we can still old savegames with identical code :).

	uint numObjs = _objects.size();
	s.syncAsUint32LE(numObjs);

	if (s.isLoading()) {
		_objects.clear();
		Object tmp;
		for (uint i = 0; i < numObjs; ++i) {
			syncWithSerializer<Object>(s, tmp);
			_objects[tmp.getPos().offset] = tmp;
		}
	} else {
		ObjMap::iterator it;
		const ObjMap::iterator end = _objects.end();
		for (it = _objects.begin(); it != end; ++it) {
			syncWithSerializer<Object>(s, it->_value);
		}
	}

	s.syncAsSint32LE(_localsOffset);
	s.syncAsSint32LE(_localsSegment);

	s.syncAsSint32LE(_markedAsDeleted);
}

static void sync_SystemString(Common::Serializer &s, SystemString &obj) {
	s.syncString(obj._name);
	s.syncAsSint32LE(obj._maxSize);

	// Sync obj._value. We cannot use syncCStr as we must make sure that
	// the allocated buffer has the correct size, i.e., obj._maxSize
	Common::String tmp;
	if (s.isSaving() && obj._value)
		tmp = obj._value;
	s.syncString(tmp);
	if (s.isLoading()) {
		//free(*str);
		obj._value = (char *)calloc(obj._maxSize, sizeof(char));
		strncpy(obj._value, tmp.c_str(), obj._maxSize);
	}
}

void SystemStrings::saveLoadWithSerializer(Common::Serializer &s) {
	for (int i = 0; i < SYS_STRINGS_MAX; ++i)
		sync_SystemString(s, _strings[i]);
}

void DynMem::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_size);
	s.syncString(_description);
	if (!_buf && _size) {
		_buf = (byte *)calloc(_size, 1);
	}
	if (_size)
		s.syncBytes(_buf, _size);
}

void DataStack::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsUint32LE(_capacity);
	if (s.isLoading()) {
		//free(entries);
		_entries = (reg_t *)calloc(_capacity, sizeof(reg_t));
	}
}

#pragma mark -

#ifdef USE_OLD_MUSIC_FUNCTIONS
static void sync_songlib(Common::Serializer &s, SongLibrary &obj) {
	int songcount = 0;
	if (s.isSaving())
		songcount = obj.countSongs();
	s.syncAsUint32LE(songcount);

	if (s.isLoading()) {
		obj._lib = 0;
		while (songcount--) {
			Song *newsong = new Song;
			syncSong(s, *newsong);
			obj.addSong(newsong);
		}
	} else {
		Song *seeker = obj._lib;
		while (seeker) {
			seeker->_restoreTime = seeker->_it->getTimepos();
			syncSong(s, *seeker);
			seeker = seeker->_next;
		}
	}
}
#else
void SciMusic::saveLoadWithSerializer(Common::Serializer &s) {
	// Sync song lib data. When loading, the actual song lib will be initialized
	// afterwards in gamestate_restore()
	_mutex.lock();

	int songcount = 0;
	if (s.isSaving())
		songcount = _playList.size();
	s.syncAsUint32LE(songcount);

	if (s.isLoading()) {
		stopAll();

		for (int i = 0; i < songcount; i++) {
			MusicEntry *curSong = new MusicEntry();
			syncSong(s, curSong);
			_playList.push_back(curSong);
		}
	} else {
		for (int i = 0; i < songcount; i++) {
			syncSong(s, _playList[i]);
		}
	}

	_mutex.unlock();
}
#endif

#pragma mark -


int gamestate_save(EngineState *s, Common::WriteStream *fh, const char* savename, const char *version) {
	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	SavegameMetadata meta;
	meta.savegame_version = CURRENT_SAVEGAME_VERSION;
	meta.savegame_name = savename;
	meta.game_version = version;
	meta.savegame_date = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	meta.savegame_time = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);

	if (s->execution_stack_base) {
		warning("Cannot save from below kernel function");
		return 1;
	}

/*
	if (s->sound_server) {
		if ((s->sound_server->save)(s, dirname)) {
			warning("Saving failed for the sound subsystem");
			//chdir("..");
			return 1;
		}
	}
*/
	Common::Serializer ser(0, fh);
	sync_SavegameMetadata(ser, meta);
	Graphics::saveThumbnail(*fh);
	s->saveLoadWithSerializer(ser);		// FIXME: Error handling?

	return 0;
}

static byte *find_unique_script_block(EngineState *s, byte *buf, int type) {
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (oldScriptHeader)
		buf += 2;

	do {
		int seeker_type = READ_LE_UINT16(buf);

		if (seeker_type == 0) break;
		if (seeker_type == type) return buf;

		int seeker_size = READ_LE_UINT16(buf + 2);
		assert(seeker_size > 0);
		buf += seeker_size;
	} while (1);

	return NULL;
}

// TODO: This should probably be turned into an EngineState or DataStack method.
static void reconstruct_stack(EngineState *retval) {
	SegmentId stack_seg = retval->_segMan->findSegmentByType(SEG_TYPE_STACK);
	DataStack *stack = (DataStack *)(retval->_segMan->_heap[stack_seg]);

	retval->stack_segment = stack_seg;
	retval->stack_base = stack->_entries;
	retval->stack_top = stack->_entries + stack->_capacity;
}

static void load_script(EngineState *s, Script *scr) {
	Resource *script, *heap = NULL;

	scr->_buf = (byte *)malloc(scr->_bufSize);
	assert(scr->_buf);

	script = s->resMan->findResource(ResourceId(kResourceTypeScript, scr->_nr), 0);
	if (getSciVersion() >= SCI_VERSION_1_1)
		heap = s->resMan->findResource(ResourceId(kResourceTypeHeap, scr->_nr), 0);

	memcpy(scr->_buf, script->data, script->size);
	if (getSciVersion() >= SCI_VERSION_1_1) {
		scr->_heapStart = scr->_buf + scr->_scriptSize;
		memcpy(scr->_heapStart, heap->data, heap->size);
	}
}

void SegManager::reconstructScripts(EngineState *s) {
	uint i;
	SegmentObj *mobj;

	for (i = 0; i < _heap.size(); i++) {
		mobj = _heap[i];
		if (!mobj ||  mobj->getType() != SEG_TYPE_SCRIPT)
			continue;

		Script *scr = (Script *)mobj;

		// FIXME: Unify this code with script_instantiate_* ?
		load_script(s, scr);
		scr->_localsBlock = (scr->_localsSegment == 0) ? NULL : (LocalVariables *)(_heap[scr->_localsSegment]);
		if (getSciVersion() >= SCI_VERSION_1_1) {
			scr->_exportTable = 0;
			scr->_synonyms = 0;
			if (READ_LE_UINT16(scr->_buf + 6) > 0) {
				scr->setExportTableOffset(6);
				s->_segMan->scriptRelocateExportsSci11(i);
			}
		} else {
			scr->_exportTable = (uint16 *) find_unique_script_block(s, scr->_buf, SCI_OBJ_EXPORTS);
			scr->_synonyms = find_unique_script_block(s, scr->_buf, SCI_OBJ_SYNONYMS);
			scr->_exportTable += 3;
		}
		scr->_codeBlocks.clear();

		ObjMap::iterator it;
		const ObjMap::iterator end = scr->_objects.end();
		for (it = scr->_objects.begin(); it != end; ++it) {
			byte *data = scr->_buf + it->_value.getPos().offset;
			it->_value._baseObj = data;
		}
	}

	for (i = 0; i < _heap.size(); i++) {
		mobj = _heap[i];
		if (!mobj ||  mobj->getType() != SEG_TYPE_SCRIPT)
			continue;

		Script *scr = (Script *)mobj;

		// FIXME: Unify this code with Script::scriptObjInit ?
		ObjMap::iterator it;
		const ObjMap::iterator end = scr->_objects.end();
		for (it = scr->_objects.begin(); it != end; ++it) {
			byte *data = scr->_buf + it->_value.getPos().offset;

			if (getSciVersion() >= SCI_VERSION_1_1) {
				uint16 *funct_area = (uint16 *)(scr->_buf + READ_LE_UINT16( data + 6 ));
				uint16 *prop_area = (uint16 *)(scr->_buf + READ_LE_UINT16( data + 4 ));

				it->_value._baseMethod = funct_area;
				it->_value._baseVars = prop_area;
			} else {
				int funct_area = READ_LE_UINT16(data + SCRIPT_FUNCTAREAPTR_OFFSET);
				Object *_baseObj;

				_baseObj = s->_segMan->getObject(it->_value.getSpeciesSelector());

				if (!_baseObj) {
					warning("Object without a base class: Script %d, index %d (reg address %04x:%04x",
						  scr->_nr, i, PRINT_REG(it->_value.getSpeciesSelector()));
					continue;
				}
				it->_value.setVarCount(_baseObj->getVarCount());
				it->_value._baseObj = _baseObj->_baseObj;

				it->_value._baseMethod = (uint16 *)(data + funct_area);
				it->_value._baseVars = (uint16 *)(data + it->_value.getVarCount() * 2 + SCRIPT_SELECTOR_OFFSET);
			}
		}
	}
}
#ifdef INCLUDE_OLDGFX
int _reset_graphics_input(EngineState *s);
#endif

#ifdef USE_OLD_MUSIC_FUNCTIONS
static void reconstruct_sounds(EngineState *s) {
	Song *seeker;
	SongIteratorType it_type;

	if (getSciVersion() > SCI_VERSION_01)
		it_type = SCI_SONG_ITERATOR_TYPE_SCI1;
	else
		it_type = SCI_SONG_ITERATOR_TYPE_SCI0;

	seeker = s->_sound._songlib._lib;

	while (seeker) {
		SongIterator *base, *ff = 0;
		int oldstatus;
		SongIterator::Message msg;

		base = ff = build_iterator(s->resMan, seeker->_resourceNum, it_type, seeker->_handle);
		if (seeker->_restoreBehavior == RESTORE_BEHAVIOR_CONTINUE)
			ff = new_fast_forward_iterator(base, seeker->_restoreTime);
		ff->init();

		msg = SongIterator::Message(seeker->_handle, SIMSG_SET_LOOPS(seeker->_loops));
		songit_handle_message(&ff, msg);
		msg = SongIterator::Message(seeker->_handle, SIMSG_SET_HOLD(seeker->_hold));
		songit_handle_message(&ff, msg);

		oldstatus = seeker->_status;
		seeker->_status = SOUND_STATUS_STOPPED;
		seeker->_it = ff;
		s->_sound.sfx_song_set_status(seeker->_handle, oldstatus);
		seeker = seeker->_next;
	}
}
#endif

EngineState *gamestate_restore(EngineState *s, Common::SeekableReadStream *fh) {
	EngineState *retval;
#ifdef USE_OLD_MUSIC_FUNCTIONS
	SongLibrary temp;
#endif

/*
	if (s->sound_server) {
		if ((s->sound_server->restore)(s, dirname)) {
			warning("Restoring failed for the sound subsystem");
			return NULL;
		}
	}
*/

	SavegameMetadata meta;

	Common::Serializer ser(fh, 0);
	sync_SavegameMetadata(ser, meta);

	if (fh->eos())
		return false;

	if ((meta.savegame_version < MINIMUM_SAVEGAME_VERSION) ||
	    (meta.savegame_version > CURRENT_SAVEGAME_VERSION)) {
		if (meta.savegame_version < MINIMUM_SAVEGAME_VERSION)
			warning("Old savegame version detected- can't load");
		else
			warning("Savegame version is %d- maximum supported is %0d", meta.savegame_version, CURRENT_SAVEGAME_VERSION);

		return NULL;
	}

	if (meta.savegame_version >= 12) {
		// We don't need the thumbnail here, so just read it and discard it
		Graphics::Surface *thumbnail = new Graphics::Surface();
		assert(thumbnail);
		Graphics::loadThumbnail(*fh, *thumbnail);
		delete thumbnail;
		thumbnail = 0;
	}

	// Create a new EngineState object
	retval = new EngineState(s->resMan, s->_kernel, s->_voc, s->_segMan, s->_gui, s->_audio);
	retval->_event = new SciEvent();

	// Copy some old data
	retval->gfx_state = s->gfx_state;
	retval->_soundCmd = s->_soundCmd;

	retval->saveLoadWithSerializer(ser);	// FIXME: Error handling?

#ifdef USE_OLD_MUSIC_FUNCTIONS
	s->_sound.sfx_exit();
#endif

	// Set exec stack base to zero
	retval->execution_stack_base = 0;

	// Now copy all current state information
#ifdef INCLUDE_OLDGFX
	// Graphics and input state:
	retval->old_screen = 0;
#endif

#ifdef USE_OLD_MUSIC_FUNCTIONS
	temp = retval->_sound._songlib;
	retval->_sound.sfx_init(retval->resMan, s->sfx_init_flags, s->detectDoSoundType());
	retval->sfx_init_flags = s->sfx_init_flags;
	retval->_sound._songlib.freeSounds();
	retval->_sound._songlib = temp;
	retval->_soundCmd->updateSfxState(&retval->_sound);
#endif

	reconstruct_stack(retval);
	retval->_segMan->reconstructScripts(retval);
	retval->_segMan->reconstructClones();
	retval->_gameObj = s->_gameObj;
	retval->script_000 = retval->_segMan->getScript(retval->_segMan->getScriptSegment(0, SCRIPT_GET_DONT_LOAD));
	retval->gc_countdown = GC_INTERVAL - 1;
	retval->sys_strings_segment = retval->_segMan->findSegmentByType(SEG_TYPE_SYS_STRINGS);
	retval->sys_strings = (SystemStrings *)GET_SEGMENT(*retval->_segMan, retval->sys_strings_segment, SEG_TYPE_SYS_STRINGS);
#ifdef INCLUDE_OLDGFX
	_reset_graphics_input(retval);
#endif

	// Time state:
	retval->last_wait_time = g_system->getMillis();
	retval->game_start_time = g_system->getMillis();

	// static parser information:

	retval->parser_base = make_reg(s->sys_strings_segment, SYS_STRING_PARSER_BASE);

	// Copy breakpoint information from current game instance
	retval->have_bp = s->have_bp;
	retval->bp_list = s->bp_list;

	retval->successor = NULL;
#ifdef INCLUDE_OLDGFX
	retval->pic_priority_table = (int *)(retval->gfx_state->pic) ? retval->gfx_state->pic->priorityTable : NULL;
#endif
	retval->_gameId = s->_gameId;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	retval->_sound._it = NULL;
	retval->_sound._flags = s->_sound._flags;
	retval->_sound._song = NULL;
	retval->_sound._suspended = s->_sound._suspended;
	reconstruct_sounds(retval);
#else
	retval->_soundCmd->_music->reconstructSounds(meta.savegame_version);
#endif

	// Message state:
	retval->_msgState = new MessageState(retval->_segMan);

	retval->_gui->resetEngineState(retval);

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
			warning("Old savegame version detected- can't load");
		else
			warning("Savegame version is %d- maximum supported is %0d", meta->savegame_version, CURRENT_SAVEGAME_VERSION);

		return false;
	}

	return true;
}

} // End of namespace Sci
