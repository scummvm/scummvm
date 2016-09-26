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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/savefile.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/func.h"
#include "common/serializer.h"
#include "graphics/thumbnail.h"

#include "sci/sci.h"
#include "sci/event.h"

#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/message.h"
#include "sci/engine/savegame.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm_types.h"
#include "sci/engine/script.h"	// for SCI_OBJ_EXPORTS and SCI_OBJ_SYNONYMS
#include "sci/graphics/helpers.h"
#include "sci/graphics/menu.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/screen.h"
#include "sci/parser/vocabulary.h"
#include "sci/sound/audio.h"
#include "sci/sound/music.h"

#ifdef ENABLE_SCI32
#include "sci/graphics/cursor32.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap32.h"
#endif

namespace Sci {


#define VER(x) Common::Serializer::Version(x)


#pragma mark -

// These are serialization functions for various objects.

void syncWithSerializer(Common::Serializer &s, Common::Serializable &obj) {
	obj.saveLoadWithSerializer(s);
}

// FIXME: Object could implement Serializable to make use of the function
// above.
void syncWithSerializer(Common::Serializer &s, Object &obj) {
	obj.saveLoadWithSerializer(s);
}

void syncWithSerializer(Common::Serializer &s, reg_t &obj) {
	// Segment and offset are accessed directly here
	s.syncAsUint16LE(obj._segment);
	s.syncAsUint16LE(obj._offset);
}

void syncWithSerializer(Common::Serializer &s, synonym_t &obj) {
	s.syncAsUint16LE(obj.replaceant);
	s.syncAsUint16LE(obj.replacement);
}

void syncWithSerializer(Common::Serializer &s, Class &obj) {
	s.syncAsSint32LE(obj.script);
	syncWithSerializer(s, obj.reg);
}

void syncWithSerializer(Common::Serializer &s, List &obj) {
	syncWithSerializer(s, obj.first);
	syncWithSerializer(s, obj.last);
}

void syncWithSerializer(Common::Serializer &s, Node &obj) {
	syncWithSerializer(s, obj.pred);
	syncWithSerializer(s, obj.succ);
	syncWithSerializer(s, obj.key);
	syncWithSerializer(s, obj.value);
}

#pragma mark -

// By default, sync using syncWithSerializer, which in turn can easily be overloaded.
template<typename T>
struct DefaultSyncer : Common::BinaryFunction<Common::Serializer, T, void> {
	void operator()(Common::Serializer &s, T &obj, int) const {
		syncWithSerializer(s, obj);
	}
};

// Syncer for entries in a segment obj table
template<typename T>
struct SegmentObjTableEntrySyncer : Common::BinaryFunction<Common::Serializer, typename T::Entry &, void> {
	void operator()(Common::Serializer &s, typename T::Entry &entry, int index) const {
		s.syncAsSint32LE(entry.next_free);

		bool hasData;
		if (s.getVersion() >= 37) {
			if (s.isSaving()) {
				hasData = entry.data != nullptr;
			}
			s.syncAsByte(hasData);
		} else {
			hasData = (entry.next_free == index);
		}

		if (hasData) {
			if (s.isLoading()) {
				entry.data = new typename T::value_type;
			}
			syncWithSerializer(s, *entry.data);
		} else if (s.isLoading()) {
			if (s.getVersion() < 37) {
				typename T::value_type dummy;
				syncWithSerializer(s, dummy);
			}
			entry.data = nullptr;
		}
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
template<typename T, class Syncer = DefaultSyncer<T> >
struct ArraySyncer : Common::BinaryFunction<Common::Serializer, T, void> {
	void operator()(Common::Serializer &s, Common::Array<T> &arr) const {
		uint len = arr.size();
		s.syncAsUint32LE(len);
		Syncer sync;

		// Resize the array if loading.
		if (s.isLoading())
			arr.resize(len);

		for (uint i = 0; i < len; ++i) {
			sync(s, arr[i], i);
		}
	}
};

// Convenience wrapper
template<typename T>
void syncArray(Common::Serializer &s, Common::Array<T> &arr) {
	ArraySyncer<T> sync;
	sync(s, arr);
}

template<typename T, class Syncer>
void syncArray(Common::Serializer &s, Common::Array<T> &arr) {
	ArraySyncer<T, Syncer> sync;
	sync(s, arr);
}

void SegManager::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.isLoading()) {
		resetSegMan();

		// Reset _scriptSegMap, to be restored below
		_scriptSegMap.clear();
	}

	s.skip(4, VER(14), VER(18));		// OBSOLETE: Used to be _exportsAreWide

	uint sync_heap_size = _heap.size();
	s.syncAsUint32LE(sync_heap_size);
	_heap.resize(sync_heap_size);
	for (uint i = 0; i < sync_heap_size; ++i) {
		SegmentObj *&mobj = _heap[i];

		// Sync the segment type
		SegmentType type = (s.isSaving() && mobj) ? mobj->getType() : SEG_TYPE_INVALID;
		s.syncAsUint32LE(type);

		if (type == SEG_TYPE_HUNK) {
			// Don't save or load HunkTable segments
			continue;
		} else if (type == SEG_TYPE_INVALID) {
			// If we were saving and mobj == 0, or if we are loading and this is an
			// entry marked as empty -> skip to next
			continue;
		} else if (type == 5) {
			// Don't save or load the obsolete system string segments
			if (s.isSaving()) {
				continue;
			} else {
				// Old saved game. Skip the data.
				Common::String tmp;
				for (int j = 0; j < 4; j++) {
					s.syncString(tmp);	// OBSOLETE: name
					s.skip(4);			// OBSOLETE: maxSize
					s.syncString(tmp);	// OBSOLETE: value
				}
				_heap[i] = NULL;	// set as freed
				continue;
			}
#ifdef ENABLE_SCI32
		} else if (type == SEG_TYPE_ARRAY) {
			// Set the correct segment for SCI32 arrays
			_arraysSegId = i;
		} else if (s.getVersion() >= 36 && type == SEG_TYPE_BITMAP) {
			_bitmapSegId = i;
#endif
		}

		if (s.isLoading())
			mobj = SegmentObj::createSegmentObj(type);

		assert(mobj);

		// Let the object sync custom data. Scripts are loaded at this point.
		mobj->saveLoadWithSerializer(s);

		if (type == SEG_TYPE_SCRIPT) {
			Script *scr = (Script *)mobj;

			// If we are loading a script, perform some extra steps
			if (s.isLoading()) {
				// Hook the script up in the script->segment map
				_scriptSegMap[scr->getScriptNumber()] = i;

				ObjMap objects = scr->getObjectMap();
				for (ObjMap::iterator it = objects.begin(); it != objects.end(); ++it)
					it->_value.syncBaseObject(scr->getBuf(it->_value.getPos().getOffset()));

			}

			// Sync the script's string heap
			if (s.getVersion() >= 28)
				scr->syncStringHeap(s);
		}
	}

	s.syncAsSint32LE(_clonesSegId);
	s.syncAsSint32LE(_listsSegId);
	s.syncAsSint32LE(_nodesSegId);

	syncArray<Class>(s, _classTable);

	// Now that all scripts are loaded, init their objects.
	// Just like in Script::initializeObjectsSci0, we do two passes
	// in case an object is loaded before its base.
	int passes = getSciVersion() < SCI_VERSION_1_1 ? 2 : 1;
	for (int pass = 1; pass <= passes; ++pass) {
		for (uint i = 0; i < _heap.size(); i++) {
			if (!_heap[i] ||  _heap[i]->getType() != SEG_TYPE_SCRIPT)
				continue;

			Script *scr = (Script *)_heap[i];
			scr->syncLocalsBlock(this);

			ObjMap objects = scr->getObjectMap();
			for (ObjMap::iterator it = objects.begin(); it != objects.end(); ++it) {
				reg_t addr = it->_value.getPos();
				Object *obj = scr->scriptObjInit(addr, false);

				if (pass == 2) {
					if (!obj->initBaseObject(this, addr, false)) {
						// TODO/FIXME: This should not be happening at all. It might indicate a possible issue
						// with the garbage collector. It happens for example in LSL5 (German, perhaps English too).
						warning("Failed to locate base object for object at %04X:%04X; skipping", PRINT_REG(addr));
						objects.erase(addr.toUint16());
					}
				}
			}
		}
	}
}


static void sync_SavegameMetadata(Common::Serializer &s, SavegameMetadata &obj) {
	s.syncString(obj.name);
	s.syncVersion(CURRENT_SAVEGAME_VERSION);
	obj.version = s.getVersion();
	s.syncString(obj.gameVersion);
	s.syncAsSint32LE(obj.saveDate);
	s.syncAsSint32LE(obj.saveTime);
	if (s.getVersion() < 22) {
		obj.gameObjectOffset = 0;
		obj.script0Size = 0;
	} else {
		s.syncAsUint16LE(obj.gameObjectOffset);
		s.syncAsUint16LE(obj.script0Size);
	}

	// Playtime
	obj.playTime = 0;
	if (s.isLoading()) {
		if (s.getVersion() >= 26)
			s.syncAsUint32LE(obj.playTime);
	} else {
		if (s.getVersion() >= 34) {
			obj.playTime = g_sci->getTickCount();
		} else {
			obj.playTime = g_engine->getTotalPlayTime() / 1000;
		}
		s.syncAsUint32LE(obj.playTime);
	}

	if (s.getVersion() >= 38) {
		if (s.isSaving()) {
			obj.score = g_sci->getEngineState()->variables[VAR_GLOBAL][kScore].toUint16();
			if (g_sci->getGameId() == GID_SHIVERS) {
				obj.score |= g_sci->getEngineState()->variables[VAR_GLOBAL][kShivers1Score].toUint16() << 16;
			}
		}

		s.syncAsUint32LE(obj.score);
	}
}

void EngineState::saveLoadWithSerializer(Common::Serializer &s) {
	Common::String tmp;
	s.syncString(tmp, VER(14), VER(23));			// OBSOLETE: Used to be gameVersion

	if (getSciVersion() <= SCI_VERSION_1_1) {
		// Save/Load picPort as well for SCI0-SCI1.1. Necessary for Castle of Dr. Brain,
		// as the picPort has been changed when loading during the intro
		int16 picPortTop, picPortLeft;
		Common::Rect picPortRect;

		if (s.isSaving())
			picPortRect = g_sci->_gfxPorts->kernelGetPicWindow(picPortTop, picPortLeft);

		s.syncAsSint16LE(picPortRect.top);
		s.syncAsSint16LE(picPortRect.left);
		s.syncAsSint16LE(picPortRect.bottom);
		s.syncAsSint16LE(picPortRect.right);
		s.syncAsSint16LE(picPortTop);
		s.syncAsSint16LE(picPortLeft);

		if (s.isLoading())
			g_sci->_gfxPorts->kernelSetPicWindow(picPortRect, picPortTop, picPortLeft, false);
	}

	_segMan->saveLoadWithSerializer(s);

	g_sci->_soundCmd->syncPlayList(s);

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		g_sci->_gfxPalette32->saveLoadWithSerializer(s);
		g_sci->_gfxRemap32->saveLoadWithSerializer(s);
		g_sci->_gfxCursor32->saveLoadWithSerializer(s);
	} else
#endif
		g_sci->_gfxPalette16->saveLoadWithSerializer(s);
}

void Vocabulary::saveLoadWithSerializer(Common::Serializer &s) {
	syncArray<synonym_t>(s, _synonyms);
}

void LocalVariables::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(script_id);
	syncArray<reg_t>(s, _locals);
}

void Object::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_flags);
	syncWithSerializer(s, _pos);
	s.syncAsSint32LE(_methodCount);		// that's actually a uint16

	syncArray<reg_t>(s, _variables);
}


template<typename T>
void sync_Table(Common::Serializer &s, T &obj) {
	s.syncAsSint32LE(obj.first_free);
	s.syncAsSint32LE(obj.entries_used);

	syncArray<typename T::Entry, SegmentObjTableEntrySyncer<T> >(s, obj._table);
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

void Script::syncStringHeap(Common::Serializer &s) {
	if (getSciVersion() < SCI_VERSION_1_1) {
		// Sync all of the SCI_OBJ_STRINGS blocks
		byte *buf = _buf;
		bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

		if (oldScriptHeader)
			buf += 2;

		do {
			int blockType = READ_LE_UINT16(buf);
			int blockSize;
			if (blockType == 0)
				break;

			blockSize = READ_LE_UINT16(buf + 2);
			assert(blockSize > 0);

			if (blockType == SCI_OBJ_STRINGS)
				s.syncBytes(buf, blockSize);

			buf += blockSize;

			if (_buf - buf == 0)
				break;
		} while (1);

	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE){
		// Strings in SCI1.1 come after the object instances
		byte *buf = _heapStart + 4 + READ_SCI11ENDIAN_UINT16(_heapStart + 2) * 2;

		// Skip all of the objects
		while (READ_SCI11ENDIAN_UINT16(buf) == SCRIPT_OBJECT_MAGIC_NUMBER)
			buf += READ_SCI11ENDIAN_UINT16(buf + 2) * 2;

		// Now, sync everything till the end of the buffer
		s.syncBytes(buf, _heapSize - (buf - _heapStart));
	} else if (getSciVersion() == SCI_VERSION_3) {
		warning("TODO: syncStringHeap(): Implement SCI3 variant");
	}
}

void Script::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_nr);

	if (s.isLoading())
		load(_nr, g_sci->getResMan(), g_sci->getScriptPatcher());
	s.skip(4, VER(14), VER(22));		// OBSOLETE: Used to be _bufSize
	s.skip(4, VER(14), VER(22));		// OBSOLETE: Used to be _scriptSize
	s.skip(4, VER(14), VER(22));		// OBSOLETE: Used to be _heapSize

	s.skip(4, VER(14), VER(19));		// OBSOLETE: Used to be _numExports
	s.skip(4, VER(14), VER(19));		// OBSOLETE: Used to be _numSynonyms
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
			syncWithSerializer(s, tmp);
			_objects[tmp.getPos().getOffset()] = tmp;
		}
	} else {
		ObjMap::iterator it;
		const ObjMap::iterator end = _objects.end();
		for (it = _objects.begin(); it != end; ++it) {
			syncWithSerializer(s, it->_value);
		}
	}

	s.skip(4, VER(14), VER(20));		// OBSOLETE: Used to be _localsOffset
	s.syncAsSint32LE(_localsSegment);

	s.syncAsSint32LE(_markedAsDeleted);
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
		free(_entries);
		_entries = (reg_t *)calloc(_capacity, sizeof(reg_t));
	}
}

#pragma mark -

void SciMusic::saveLoadWithSerializer(Common::Serializer &s) {
	// Sync song lib data. When loading, the actual song lib will be initialized
	// afterwards in gamestate_restore()
	int songcount = 0;
	byte masterVolume = soundGetMasterVolume();
	byte reverb = _pMidiDrv->getReverb();

	if (s.isSaving()) {
		s.syncAsByte(_soundOn);
		s.syncAsByte(masterVolume);
		s.syncAsByte(reverb, VER(17));
	} else if (s.isLoading()) {
		if (s.getVersion() >= 15) {
			s.syncAsByte(_soundOn);
			s.syncAsByte(masterVolume);
			reverb = 0;
			s.syncAsByte(reverb, VER(17));
		} else {
			_soundOn = true;
			masterVolume = 15;
			reverb = 0;
		}

		soundSetSoundOn(_soundOn);
		soundSetMasterVolume(masterVolume);
		setGlobalReverb(reverb);
	}

	if (s.isSaving())
		songcount = _playList.size();
	s.syncAsUint32LE(songcount);

	if (s.isLoading())
		clearPlayList();

	Common::StackLock lock(_mutex);

	if (s.isLoading()) {
		for (int i = 0; i < songcount; i++) {
			MusicEntry *curSong = new MusicEntry();
			curSong->saveLoadWithSerializer(s);
			_playList.push_back(curSong);
		}
	} else {
		for (int i = 0; i < songcount; i++) {
			_playList[i]->saveLoadWithSerializer(s);
		}
	}
}

void MusicEntry::saveLoadWithSerializer(Common::Serializer &s) {
	syncWithSerializer(s, soundObj);
	s.syncAsSint16LE(resourceId);
	s.syncAsSint16LE(dataInc);
	s.syncAsSint16LE(ticker);
	s.syncAsSint16LE(signal, VER(17));
	if (s.getVersion() >= 31) // FE sound/music.h -> priority
		s.syncAsSint16LE(priority);
	else
		s.syncAsByte(priority);
	s.syncAsSint16LE(loop, VER(17));
	s.syncAsByte(volume);
	s.syncAsByte(hold, VER(17));
	s.syncAsByte(fadeTo);
	s.syncAsSint16LE(fadeStep);
	s.syncAsSint32LE(fadeTicker);
	s.syncAsSint32LE(fadeTickerStep);
	s.syncAsByte(status);
	if (s.getVersion() >= 32)
		s.syncAsByte(playBed);
	else if (s.isLoading())
		playBed = false;
	if (s.getVersion() >= 33)
		s.syncAsByte(overridePriority);
	else if (s.isLoading())
		overridePriority = false;

	// pMidiParser and pStreamAud will be initialized when the
	// sound list is reconstructed in gamestate_restore()
	if (s.isLoading()) {
		soundRes = 0;
		pMidiParser = 0;
		pStreamAud = 0;
		reverb = -1;	// invalid reverb, will be initialized in processInitSound()
	}
}

void SoundCommandParser::syncPlayList(Common::Serializer &s) {
	_music->saveLoadWithSerializer(s);
}

void SoundCommandParser::reconstructPlayList() {
	Common::StackLock lock(_music->_mutex);

	// We store all songs here because starting songs may re-shuffle their order
	MusicList songs;
	for (MusicList::iterator i = _music->getPlayListStart(); i != _music->getPlayListEnd(); ++i)
		songs.push_back(*i);

	for (MusicList::iterator i = songs.begin(); i != songs.end(); ++i) {
		initSoundResource(*i);

		if ((*i)->status == kSoundPlaying) {
			// WORKAROUND: PQ3 (German?) scripts can set volume negative in the
			// sound object directly without going through DoSound.
			// Since we re-read this selector when re-playing the sound after loading,
			// this will lead to unexpected behaviour. As a workaround we
			// sync the sound object's selectors here. (See bug #5501)
			writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(loop), (*i)->loop);
			writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(priority), (*i)->priority);
			if (_soundVersion >= SCI_VERSION_1_EARLY)
				writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(vol), (*i)->volume);

			processPlaySound((*i)->soundObj, (*i)->playBed);
		}
	}
}

#ifdef ENABLE_SCI32
void ArrayTable::saveLoadWithSerializer(Common::Serializer &ser) {
	if (ser.getVersion() < 18)
		return;

	sync_Table<ArrayTable>(ser, *this);
}

void SciArray::saveLoadWithSerializer(Common::Serializer &s) {
	uint16 size;

	if (s.isSaving()) {
		size = _size;
	}

	s.syncAsByte(_type);
	s.syncAsByte(_elementSize);
	s.syncAsUint16LE(size);

	if (s.isLoading()) {
		resize(size);
	}

	switch (_type) {
	case kArrayTypeByte:
	case kArrayTypeString:
		s.syncBytes((byte *)_data, size);
		break;
	case kArrayTypeInt16:
		for (int i = 0; i < size; ++i) {
			s.syncAsUint16LE(((int16 *)_data)[i]);
		}
		break;
	case kArrayTypeID:
		for (int i = 0; i < size; ++i) {
			syncWithSerializer(s, ((reg_t *)_data)[i]);
		}
		break;
	default:
		error("Attempt to sync invalid SciArray type %d", _type);
	}
}

void BitmapTable::saveLoadWithSerializer(Common::Serializer &ser) {
	if (ser.getVersion() < 36) {
		return;
	}

	sync_Table(ser, *this);
}

void SciBitmap::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.getVersion() < 36) {
		return;
	}

	s.syncAsByte(_gc);
	s.syncAsUint32LE(_dataSize);
	if (s.isLoading()) {
		_data = (byte *)malloc(_dataSize);
	}
	s.syncBytes(_data, _dataSize);

	if (s.isLoading()) {
		_buffer = Buffer(getWidth(), getHeight(), getPixels());
	}
}
#endif

void GfxPalette::palVarySaveLoadPalette(Common::Serializer &s, Palette *palette) {
	s.syncBytes(palette->mapping, 256);
	s.syncAsUint32LE(palette->timestamp);
	for (int i = 0; i < 256; i++) {
		s.syncAsByte(palette->colors[i].used);
		s.syncAsByte(palette->colors[i].r);
		s.syncAsByte(palette->colors[i].g);
		s.syncAsByte(palette->colors[i].b);
	}
	s.syncBytes(palette->intensity, 256);
}

void GfxPalette::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.getVersion() >= 25) {
		// We need to save intensity of the _sysPalette at least for kq6 when entering the dark cave (room 390)
		//  from room 340. scripts will set intensity to 60 for this room and restore them when leaving.
		//  Sierra SCI is also doing this (although obviously not for SCI0->SCI01 games, still it doesn't hurt
		//  to save it everywhere). Refer to bug #3072868
		s.syncBytes(_sysPalette.intensity, 256);
	}
	if (s.getVersion() >= 24) {
		if (s.isLoading() && _palVaryResourceId != -1)
			palVaryRemoveTimer();

		s.syncAsSint32LE(_palVaryResourceId);
		if (_palVaryResourceId != -1) {
			palVarySaveLoadPalette(s, &_palVaryOriginPalette);
			palVarySaveLoadPalette(s, &_palVaryTargetPalette);
			s.syncAsSint16LE(_palVaryStep);
			s.syncAsSint16LE(_palVaryStepStop);
			s.syncAsSint16LE(_palVaryDirection);
			s.syncAsUint16LE(_palVaryTicks);
			s.syncAsSint32LE(_palVaryPaused);
		}

		_palVarySignal = 0;

		if (s.isLoading() && _palVaryResourceId != -1) {
			palVaryInstallTimer();
		}
	}
}

#ifdef ENABLE_SCI32
void saveLoadPalette32(Common::Serializer &s, Palette *const palette) {
	s.syncAsUint32LE(palette->timestamp);
	for (int i = 0; i < ARRAYSIZE(palette->colors); ++i) {
		s.syncAsByte(palette->colors[i].used);
		s.syncAsByte(palette->colors[i].r);
		s.syncAsByte(palette->colors[i].g);
		s.syncAsByte(palette->colors[i].b);
	}
}

void saveLoadOptionalPalette32(Common::Serializer &s, Palette **const palette) {
	bool hasPalette;
	if (s.isSaving()) {
		hasPalette = (*palette != nullptr);
	}
	s.syncAsByte(hasPalette);
	if (hasPalette) {
		if (s.isLoading()) {
			*palette = new Palette;
		}
		saveLoadPalette32(s, *palette);
	}
}

void GfxPalette32::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.getVersion() < 34) {
		return;
	}

	if (s.isLoading()) {
		++_version;
	}

	s.syncAsSint16LE(_varyDirection);
	s.syncAsSint16LE(_varyPercent);
	s.syncAsSint16LE(_varyTargetPercent);
	s.syncAsSint16LE(_varyFromColor);
	s.syncAsSint16LE(_varyToColor);
	s.syncAsUint16LE(_varyNumTimesPaused);
	s.syncAsByte(_needsUpdate);
	s.syncAsSint32LE(_varyTime);
	s.syncAsUint32LE(_varyLastTick);

	for (int i = 0; i < ARRAYSIZE(_fadeTable); ++i) {
		s.syncAsByte(_fadeTable[i]);
	}
	for (int i = 0; i < ARRAYSIZE(_cycleMap); ++i) {
		s.syncAsByte(_cycleMap[i]);
	}

	saveLoadOptionalPalette32(s, &_varyTargetPalette);
	saveLoadOptionalPalette32(s, &_varyStartPalette);
	// NOTE: _sourcePalette and _nextPalette are not saved
	// by SCI engine

	for (int i = 0; i < ARRAYSIZE(_cyclers); ++i) {
		PalCycler *cycler = nullptr;

		bool hasCycler;
		if (s.isSaving()) {
			cycler = _cyclers[i];
			hasCycler = (cycler != nullptr);
		}
		s.syncAsByte(hasCycler);

		if (hasCycler) {
			if (s.isLoading()) {
				_cyclers[i] = cycler = new PalCycler;
			}

			s.syncAsByte(cycler->fromColor);
			s.syncAsUint16LE(cycler->numColorsToCycle);
			s.syncAsByte(cycler->currentCycle);
			s.syncAsByte(cycler->direction);
			s.syncAsUint32LE(cycler->lastUpdateTick);
			s.syncAsSint16LE(cycler->delay);
			s.syncAsUint16LE(cycler->numTimesPaused);
		}
	}
}

void GfxRemap32::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.getVersion() < 35) {
		return;
	}

	s.syncAsByte(_numActiveRemaps);
	s.syncAsByte(_blockedRangeStart);
	s.syncAsSint16LE(_blockedRangeCount);

	for (uint i = 0; i < _remaps.size(); ++i) {
		SingleRemap &singleRemap = _remaps[i];
		s.syncAsByte(singleRemap._type);
		if (s.isLoading() && singleRemap._type != kRemapNone) {
			singleRemap.reset();
		}
		s.syncAsByte(singleRemap._from);
		s.syncAsByte(singleRemap._to);
		s.syncAsByte(singleRemap._delta);
		s.syncAsByte(singleRemap._percent);
		s.syncAsByte(singleRemap._gray);
	}

	if (s.isLoading()) {
		_needsUpdate = true;
	}
}

void GfxCursor32::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.getVersion() < 38) {
		return;
	}

	int32 hideCount;
	if (s.isSaving()) {
		hideCount = _hideCount;
	}
	s.syncAsSint32LE(hideCount);
	s.syncAsSint16LE(_restrictedArea.left);
	s.syncAsSint16LE(_restrictedArea.top);
	s.syncAsSint16LE(_restrictedArea.right);
	s.syncAsSint16LE(_restrictedArea.bottom);
	s.syncAsUint16LE(_cursorInfo.resourceId);
	s.syncAsUint16LE(_cursorInfo.loopNo);
	s.syncAsUint16LE(_cursorInfo.celNo);

	if (s.isLoading()) {
		hide();
		setView(_cursorInfo.resourceId, _cursorInfo.loopNo, _cursorInfo.celNo);
		if (!hideCount) {
			show();
		} else {
			_hideCount = hideCount;
		}
	}
}
#endif

void GfxPorts::saveLoadWithSerializer(Common::Serializer &s) {
	// reset() is called directly way earlier in gamestate_restore()
	if (s.getVersion() >= 27) {
		uint windowCount = 0;
		uint id = PORTS_FIRSTSCRIPTWINDOWID;
		if (s.isSaving()) {
			while (id < _windowsById.size()) {
				if (_windowsById[id])
					windowCount++;
				id++;
			}
		}
		// Save/Restore window count
		s.syncAsUint32LE(windowCount);

		if (s.isSaving()) {
			id = PORTS_FIRSTSCRIPTWINDOWID;
			while (id < _windowsById.size()) {
				if (_windowsById[id]) {
					Window *window = (Window *)_windowsById[id];
					window->saveLoadWithSerializer(s);
				}
				id++;
			}
		} else {
			id = PORTS_FIRSTSCRIPTWINDOWID;
			while (windowCount) {
				Window *window = new Window(0);
				window->saveLoadWithSerializer(s);

				// add enough entries inside _windowsById as needed
				while (id <= window->id) {
					_windowsById.push_back(0);
					id++;
				}
				_windowsById[window->id] = window;
				// _windowList may not be 100% correct using that way of restoring
				//  saving/restoring ports won't work perfectly anyway, because the contents
				//  of the window can only get repainted by the scripts and they dont do that
				//  so we will get empty, transparent windows instead. So perfect window order
				//  shouldn't really matter
				if (window->counterTillFree) {
					_freeCounter++;
				} else {
					// we don't put the saved script windows into _windowList[], so that they aren't used
					//  by kernel functions. This is important and would cause issues otherwise.
					//  see Conquests of Camelot - bug #6744 - when saving on the map screen (room 103),
					//                                restoring would result in a black window in place
					//                                where the area name was displayed before
					//                                In Sierra's SCI the behaviour is identical to us
					//                                 Sierra's SCI won't show those windows after restoring
					// If this should cause issues in another game, we would have to add a flag to simply
					//  avoid any drawing operations for such windows
					// We still have to restore script windows, because for example Conquests of Camelot
					//  will immediately delete windows, that were created before saving the game.
				}

				windowCount--;
			}
		}
	}
}

void SegManager::reconstructStack(EngineState *s) {
	DataStack *stack = (DataStack *)(_heap[findSegmentByType(SEG_TYPE_STACK)]);
	s->stack_base = stack->_entries;
	s->stack_top = s->stack_base + stack->_capacity;
}

void SegManager::reconstructClones() {
	for (uint i = 0; i < _heap.size(); i++) {
		SegmentObj *mobj = _heap[i];
		if (mobj && mobj->getType() == SEG_TYPE_CLONES) {
			CloneTable *ct = (CloneTable *)mobj;

			for (uint j = 0; j < ct->_table.size(); j++) {
				// Check if the clone entry is used
				uint entryNum = (uint)ct->first_free;
				bool isUsed = true;
				while (entryNum != ((uint) CloneTable::HEAPENTRY_INVALID)) {
					if (entryNum == j) {
						isUsed = false;
						break;
					}
					entryNum = ct->_table[entryNum].next_free;
				}

				if (!isUsed)
					continue;

				CloneTable::value_type &seeker = ct->at(j);
				const Object *baseObj = getObject(seeker.getSpeciesSelector());
				seeker.cloneFromObject(baseObj);
				if (!baseObj) {
					// Can happen when loading some KQ6 savegames
					warning("Clone entry without a base class: %d", j);
				}
			}	// end for
		}	// end if
	}	// end for
}


#pragma mark -


bool gamestate_save(EngineState *s, Common::WriteStream *fh, const Common::String &savename, const Common::String &version) {
	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	SavegameMetadata meta;
	meta.version = CURRENT_SAVEGAME_VERSION;
	meta.name = savename;
	meta.gameVersion = version;
	meta.saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	meta.saveTime = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);

	Resource *script0 = g_sci->getResMan()->findResource(ResourceId(kResourceTypeScript, 0), false);
	meta.script0Size = script0->size;
	meta.gameObjectOffset = g_sci->getGameObject().getOffset();

	// Checking here again
// TODO: This breaks Torin autosave, is there actually any reason for it?
//	if (s->executionStackBase) {
//		warning("Cannot save from below kernel function");
//		return false;
//	}

	Common::Serializer ser(0, fh);
	sync_SavegameMetadata(ser, meta);
	Graphics::saveThumbnail(*fh);
	s->saveLoadWithSerializer(ser);		// FIXME: Error handling?
	if (g_sci->_gfxPorts)
		g_sci->_gfxPorts->saveLoadWithSerializer(ser);
	Vocabulary *voc = g_sci->getVocabulary();
	if (voc)
		voc->saveLoadWithSerializer(ser);

	// TODO: SSCI (at least JonesCD, presumably more) also stores the Menu state

	return true;
}

extern void showScummVMDialog(const Common::String &message);

void gamestate_delayedrestore(EngineState *s) {
	int savegameId = s->_delayedRestoreGameId; // delayedRestoreGameId gets destroyed within gamestate_restore()!
	Common::String fileName = g_sci->getSavegameName(savegameId);
	Common::SeekableReadStream *in = g_sci->getSaveFileManager()->openForLoading(fileName);

	if (in) {
		// found a savegame file
		gamestate_restore(s, in);
		delete in;
		if (s->r_acc != make_reg(0, 1)) {
			gamestate_afterRestoreFixUp(s, savegameId);
			return;
		}
	}

	error("Restoring gamestate '%s' failed", fileName.c_str());
}

void gamestate_afterRestoreFixUp(EngineState *s, int savegameId) {
	switch (g_sci->getGameId()) {
	case GID_MOTHERGOOSE:
		// WORKAROUND: Mother Goose SCI0
		//  Script 200 / rm200::newRoom will set global C5h directly right after creating a child to the
		//   current number of children plus 1.
		//  We can't trust that global, that's why we set the actual savedgame id right here directly after
		//   restoring a saved game.
		//  If we didn't, the game would always save to a new slot
		s->variables[VAR_GLOBAL][0xC5].setOffset(SAVEGAMEID_OFFICIALRANGE_START + savegameId);
		break;
	case GID_MOTHERGOOSE256:
		// WORKAROUND: Mother Goose SCI1/SCI1.1 does some weird things for
		//  saving a previously restored game.
		// We set the current savedgame-id directly and remove the script
		//  code concerning this via script patch.
		s->variables[VAR_GLOBAL][0xB3].setOffset(SAVEGAMEID_OFFICIALRANGE_START + savegameId);
		break;
	case GID_JONES:
		// HACK: The code that enables certain menu items isn't called when a game is restored from the
		// launcher, or the "Restore game" option in the game's main menu - bugs #6537 and #6723.
		// These menu entries are disabled when the game is launched, and are enabled when a new game is
		// started. The code for enabling these entries is is all in script 1, room1::init, but that code
		// path is never followed in these two cases (restoring game from the menu, or restoring a game
		// from the ScummVM launcher). Thus, we perform the calls to enable the menus ourselves here.
		// These two are needed when restoring from the launcher
		// FIXME: The original interpreter saves and restores the menu state, so these attributes
		// are automatically reset there. We may want to do the same.
		g_sci->_gfxMenu->kernelSetAttribute(257 >> 8, 257 & 0xFF, SCI_MENU_ATTRIBUTE_ENABLED, TRUE_REG);    // Sierra -> About Jones
		g_sci->_gfxMenu->kernelSetAttribute(258 >> 8, 258 & 0xFF, SCI_MENU_ATTRIBUTE_ENABLED, TRUE_REG);    // Sierra -> Help
		// The rest are normally enabled from room1::init
		g_sci->_gfxMenu->kernelSetAttribute(769 >> 8, 769 & 0xFF, SCI_MENU_ATTRIBUTE_ENABLED, TRUE_REG);    // Options -> Delete current player
		g_sci->_gfxMenu->kernelSetAttribute(513 >> 8, 513 & 0xFF, SCI_MENU_ATTRIBUTE_ENABLED, TRUE_REG);    // Game -> Save Game
		g_sci->_gfxMenu->kernelSetAttribute(515 >> 8, 515 & 0xFF, SCI_MENU_ATTRIBUTE_ENABLED, TRUE_REG);    // Game -> Restore Game
		g_sci->_gfxMenu->kernelSetAttribute(1025 >> 8, 1025 & 0xFF, SCI_MENU_ATTRIBUTE_ENABLED, TRUE_REG);  // Status -> Statistics
		g_sci->_gfxMenu->kernelSetAttribute(1026 >> 8, 1026 & 0xFF, SCI_MENU_ATTRIBUTE_ENABLED, TRUE_REG);  // Status -> Goals
		break;
	case GID_KQ6:
		if (g_sci->isCD()) {
			// WORKAROUND:
			// For the CD version of King's Quest 6, set global depending on current hires/lowres state
			// The game sets a global at the start depending on it and some things check that global
			// instead of checking platform like for example the game action menu.
			// This never happened in the original interpreter, because the original DOS interpreter
			// was only capable of lowres graphics and the original Windows 3.11 interpreter was only capable
			// of hires graphics. Saved games were not compatible between those two.
			// Which means saving during lowres mode, then going into hires mode and restoring that saved game,
			// will result in some graphics being incorrect (lowres).
			// That's why we are setting the global after restoring a saved game depending on hires/lowres state.
			// The CD demo of KQ6 does the same and uses the exact same global.
			if ((g_sci->getPlatform() == Common::kPlatformWindows) || (g_sci->forceHiresGraphics())) {
				s->variables[VAR_GLOBAL][0xA9].setOffset(1);
			} else {
				s->variables[VAR_GLOBAL][0xA9].setOffset(0);
			}
		}
		break;
	case GID_PQ2:
		// HACK: Same as above - enable the save game menu option when loading in PQ2 (bug #6875).
		// It gets disabled in the game's death screen.
		g_sci->_gfxMenu->kernelSetAttribute(2, 1, SCI_MENU_ATTRIBUTE_ENABLED, TRUE_REG);	// Game -> Save Game
		break;
	default:
		break;
	}
}

void gamestate_restore(EngineState *s, Common::SeekableReadStream *fh) {
	SavegameMetadata meta;

	Common::Serializer ser(fh, 0);
	sync_SavegameMetadata(ser, meta);

	if (fh->eos()) {
		s->r_acc = TRUE_REG;	// signal failure
		return;
	}

	if ((meta.version < MINIMUM_SAVEGAME_VERSION) || (meta.version > CURRENT_SAVEGAME_VERSION)) {
		if (meta.version < MINIMUM_SAVEGAME_VERSION) {
			showScummVMDialog("The format of this saved game is obsolete, unable to load it");
		} else {
			Common::String msg = Common::String::format("Savegame version is %d, maximum supported is %0d", meta.version, CURRENT_SAVEGAME_VERSION);
			showScummVMDialog(msg);
		}

		s->r_acc = TRUE_REG;	// signal failure
		return;
	}

	if (meta.gameObjectOffset > 0 && meta.script0Size > 0) {
		Resource *script0 = g_sci->getResMan()->findResource(ResourceId(kResourceTypeScript, 0), false);
		if (script0->size != meta.script0Size || g_sci->getGameObject().getOffset() != meta.gameObjectOffset) {
			showScummVMDialog("This saved game was created with a different version of the game, unable to load it");

			s->r_acc = TRUE_REG;	// signal failure
			return;
		}
	}

	// We don't need the thumbnail here, so just read it and discard it
	Graphics::skipThumbnail(*fh);

	// reset ports is one of the first things we do, because that may free() some hunk memory
	//  and we don't want to do that after we read in the saved game hunk memory
	if (g_sci->_gfxPorts)
		g_sci->_gfxPorts->reset();
	// clear screen
	if (getSciVersion() <= SCI_VERSION_1_1) {
		// Only do clearing the screen for SCI16
		// Both SCI16 + SCI32 did not clear the screen.
		// We basically do it for SCI16, because of KQ6.
		// When hires portraits are shown and the user restores during that time, the portraits
		// wouldn't get fully removed. In original SCI, the user wasn't able to restore during that time,
		// so this is basically a workaround, so that ScummVM features work properly.
		// For SCI32, behavior was verified in DOSBox, that SCI32 does not clear and also not redraw the screen.
		// It only redraws elements that have changed in comparison to the state before the restore.
		// If we cleared the screen for SCI32, we would have issues because of this behavior.
		if (g_sci->_gfxScreen)
			g_sci->_gfxScreen->clearForRestoreGame();
	}
#ifdef ENABLE_SCI32
	// Delete current planes/elements of actively loaded VM, only when our ScummVM dialogs are patched in
	// We MUST NOT delete all planes/screen items. At least Space Quest 6 has a few in memory like for example
	// the options plane, which are not re-added and are in memory all the time right from the start of the
	// game. Sierra SCI32 did not clear planes, only scripts cleared the ones inside planes::elements.
	if (getSciVersion() >= SCI_VERSION_2) {
		if (!s->_delayedRestoreFromLauncher) {
			// Only do it, when we are restoring regulary and not from launcher
			// As it could result in option planes etc. on the screen (happens in gk1)
			g_sci->_gfxFrameout->syncWithScripts(false);
		}
	}
#endif

	s->reset(true);
	s->saveLoadWithSerializer(ser);	// FIXME: Error handling?

	// Now copy all current state information

	s->_segMan->reconstructStack(s);
	s->_segMan->reconstructClones();
	s->initGlobals();
	s->gcCountDown = GC_INTERVAL - 1;

	// Time state:
	s->lastWaitTime = g_system->getMillis();
	s->_screenUpdateTime = g_system->getMillis();
	if (meta.version >= 34) {
		g_sci->setTickCount(meta.playTime);
	} else {
		g_engine->setTotalPlayTime(meta.playTime * 1000);
	}

	if (g_sci->_gfxPorts)
		g_sci->_gfxPorts->saveLoadWithSerializer(ser);

	// SCI32:
	// Current planes/screen elements of freshly loaded VM are re-added by scripts in [gameID]::replay
	// We don't have to do that in here.
	// But we may have to do it ourselves in case we ever implement some soft-error handling in case
	// a saved game can't be restored. That way we can restore the game screen.
	// see _gfxFrameout->syncWithScripts()

	Vocabulary *voc = g_sci->getVocabulary();
	if (ser.getVersion() >= 30 && voc)
		voc->saveLoadWithSerializer(ser);

	g_sci->_soundCmd->reconstructPlayList();

	// Message state:
	delete s->_msgState;
	s->_msgState = new MessageState(s->_segMan);

	// System strings:
	s->_segMan->initSysStrings();

	s->abortScriptProcessing = kAbortLoadGame;

	// signal restored game to game scripts
	s->gameIsRestarting = GAMEISRESTARTING_RESTORE;

	s->_delayedRestoreFromLauncher = false;
}

bool get_savegame_metadata(Common::SeekableReadStream *stream, SavegameMetadata *meta) {
	assert(stream);
	assert(meta);

	Common::Serializer ser(stream, 0);
	sync_SavegameMetadata(ser, *meta);

	if (stream->eos())
		return false;

	if ((meta->version < MINIMUM_SAVEGAME_VERSION) ||
	    (meta->version > CURRENT_SAVEGAME_VERSION)) {
		if (meta->version < MINIMUM_SAVEGAME_VERSION)
			warning("Old savegame version detected- can't load");
		else
			warning("Savegame version is %d- maximum supported is %0d", meta->version, CURRENT_SAVEGAME_VERSION);

		return false;
	}

	return true;
}

} // End of namespace Sci
