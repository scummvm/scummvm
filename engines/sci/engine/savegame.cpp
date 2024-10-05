/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/savefile.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/func.h"
#include "common/serializer.h"
#include "common/translation.h"
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
#include "common/config-manager.h"
#include "common/gui_options.h"
#include "sci/engine/guest_additions.h"
#include "sci/graphics/cursor32.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap32.h"
#include "sci/graphics/video32.h"
#endif

namespace Sci {

// These are serialization functions for various objects.

void syncWithSerializer(Common::Serializer &s, Common::Serializable &obj) {
	obj.saveLoadWithSerializer(s);
}

void syncWithSerializer(Common::Serializer &s, ResourceId &obj) {
	s.syncAsByte(obj._type);
	s.syncAsUint16LE(obj._number);
	s.syncAsUint32LE(obj._tuple);
}

void syncWithSerializer(Common::Serializer &s, reg_t &obj) {
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

		bool hasData = false;
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

			if (s.isLoading()) {
				_scriptSegMap[scr->getScriptNumber()] = i;
			}

			if (s.getVersion() >= 28)
				scr->syncStringHeap(s);
		}
	}

	s.syncAsSint32LE(_clonesSegId);
	s.syncAsSint32LE(_listsSegId);
	s.syncAsSint32LE(_nodesSegId);

	syncArray<Class>(s, _classTable);

	if (s.isLoading()) {
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

				ObjMap &objects = scr->getObjectMap();
				for (ObjMap::iterator it = objects.begin(); it != objects.end(); ++it) {
					reg_t addr = it->_value.getPos();
					if (pass == 1) {
						scr->scriptObjInit(addr, false);
					} else {
						Object *obj = scr->getObject(addr.getOffset());
						// When a game disposes a script with kDisposeScript,
						// the script is marked as deleted and its lockers are
						// set to 0, which makes the GC stop using the script
						// as a retainer of its own objects. Most of the time,
						// this means that the script and all of its objects are
						// cleaned up on the next GC cycle, but occasionally a
						// game will retain a reference to an object within a
						// disposed script somewhere else, which keeps the
						// script (and all of its objects) alive. This does not
						// prevent the GC from safely collecting other objects
						// that had only been retained by now-unreachable script
						// objects, so references held by these unreachable
						// objects may be invalidated. If the superclass of one
						// of these objects is GC'd (because it was the only
						// retainer of the superclass), and a save game is
						// created after kDisposeScript is called but before
						// the script actually becomes collectable, it will
						// cause the `initBaseObject` call to fail on restore,
						// but this is fine because the object isn't reachable
						// anyway (it is just waiting to be GC'd).
						//
						// For example, in EcoQuest floppy, after opening the
						// gate for Delphineus at the beginning of the game,
						// the game calls to dispose script 380, but there are
						// still reachable references to the script 380 object
						// `outsideGateLever` at `CueObj::client` and
						// `OnMeAndLowY::theObj`, so script 380 (and all of its
						// objects) are retained. However, the now-unreachable
						// `fJump` object had been the only retainer of its
						// superclass `JumpTo`, so the `JumpTo` class gets
						// GC'd, and the `fJump` object is left with no valid
						// superclass. If the game is saved and restored at this
						// point, `initBaseObject` will fail on `fJump` because
						// it has no superclass (but, again, this is fine
						// because this is an unreachable object). Later,
						// `outsideGateLever` becomes unreachable as the
						// `CueObj::client` and `OnMeAndLowY::theObj` properties
						// are changed, which means that all script 380 objects
						// are finally unreachable and the script and its
						// objects get fully disposed.
						//
						// All that said, if a script has lockers and the base
						// object necessary for restoring the object is still
						// missing, that is probably a real bug.
						if (!obj->initBaseObject(this, addr, false) && scr->getLockers()) {
							warning("Failed to locate base object %04x:%04x for object %04x:%04x (%s); skipping", PRINT_REG(obj->getSpeciesSelector()), PRINT_REG(addr), getObjectName(addr));
						}
					}
				}

#ifdef ENABLE_SCI32
				if (pass == passes) {
					g_sci->_guestAdditions->segManSaveLoadScriptHook(*scr);
				}
#endif
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

	// Some games require additional metadata to display their restore screens
	// correctly
	if (s.getVersion() >= 39) {
		if (s.isSaving()) {
			const reg_t *globals = g_sci->getEngineState()->variables[VAR_GLOBAL];
			if (g_sci->getGameId() == GID_SHIVERS) {
				obj.lowScore = globals[kGlobalVarScore].toUint16();
				obj.highScore = globals[kGlobalVarShivers1Score].toUint16();
				obj.avatarId = 0;
			} else if (g_sci->getGameId() == GID_MOTHERGOOSEHIRES) {
				obj.lowScore = obj.highScore = 0;
				obj.avatarId = readSelectorValue(g_sci->getEngineState()->_segMan, globals[kGlobalVarEgo], SELECTOR(view));
			} else {
				obj.lowScore = obj.highScore = obj.avatarId = 0;
			}
		}

		s.syncAsUint16LE(obj.lowScore);
		s.syncAsUint16LE(obj.highScore);
		s.syncAsByte(obj.avatarId);
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

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		g_sci->_video32->beforeSaveLoadWithSerializer(s);
	}

	if (getSciVersion() >= SCI_VERSION_2 &&
		s.isLoading() &&
		g_sci->getPlatform() == Common::kPlatformMacintosh) {
		g_sci->_gfxFrameout->deletePlanesForMacRestore();
	}
#endif

	_segMan->saveLoadWithSerializer(s);

	g_sci->_soundCmd->syncPlayList(s);

	if (getSciVersion() >= SCI_VERSION_2) {
#ifdef ENABLE_SCI32
		g_sci->_gfxPalette32->saveLoadWithSerializer(s);
		g_sci->_gfxRemap32->saveLoadWithSerializer(s);
		g_sci->_gfxCursor32->saveLoadWithSerializer(s);
		g_sci->_audio32->saveLoadWithSerializer(s);
		g_sci->_video32->saveLoadWithSerializer(s);
#endif
	} else {
		g_sci->_gfxPalette16->saveLoadWithSerializer(s);
	}

	// Stop any currently playing audio when loading.
	// Loading is not normally allowed while audio is being played in SCI games.
	// Stopping sounds is needed in ScummVM, as the player may load via
	// Control - F5 at any point, even while a sound is playing.
	if (s.isLoading()) {
		if (getSciVersion() >= SCI_VERSION_2) {
#ifdef ENABLE_SCI32
			g_sci->_audio32->stop(kAllChannels);
#endif
		} else {
			g_sci->_audio->stopAllAudio();
		}
	}
}

void Vocabulary::saveLoadWithSerializer(Common::Serializer &s) {
	syncArray<synonym_t>(s, _synonyms);
}

void LocalVariables::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(script_id);
	syncArray<reg_t>(s, _locals);
}

void Object::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_isFreed);
	syncWithSerializer(s, _pos);
	s.syncAsSint32LE(_methodCount);		// that's actually a uint16

	syncArray<reg_t>(s, _variables);

#ifdef ENABLE_SCI32
	if (s.getVersion() >= 42 && getSciVersion() == SCI_VERSION_3) {
		// Obsolete mustSetViewVisible array
		if (s.getVersion() == 42 && s.isLoading()) {
			uint32 len;
			s.syncAsUint32LE(len);
			s.skip(len);
		}
		syncWithSerializer(s, _superClassPosSci3);
		syncWithSerializer(s, _speciesSelectorSci3);
		syncWithSerializer(s, _infoSelectorSci3);
	}
#endif
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
		SciSpan<byte> buf = *_buf;
		bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

		if (oldScriptHeader)
			buf += 2;

		for (;;) {
			int blockType = buf.getUint16LEAt(0);
			int blockSize;
			if (blockType == 0)
				break;

			blockSize = buf.getUint16LEAt(2);
			assert(blockSize > 0);

			if (blockType == SCI_OBJ_STRINGS)
				s.syncBytes(buf.getUnsafeDataAt(0, blockSize), blockSize);

			buf += blockSize;
		}

	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE){
		// Strings in SCI1.1 come after the object instances
		SciSpan<byte> buf = _heap.subspan(4 + _heap.getUint16SEAt(2) * 2);

		// Skip all of the objects
		while (buf.getUint16SEAt(0) == SCRIPT_OBJECT_MAGIC_NUMBER)
			buf += buf.getUint16SEAt(2) * 2;

		// Now, sync everything till the end of the buffer
		const int length = _heap.size() - (buf - _heap);
		s.syncBytes(buf.getUnsafeDataAt(0, length), length);
	} else if (getSciVersion() == SCI_VERSION_3) {
		const int stringOffset = _buf->getInt32SEAt(4);
		const int length = _buf->getInt32SEAt(8) - stringOffset;
		s.syncBytes(_buf->getUnsafeDataAt(stringOffset, length), length);
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
	s.syncAsUint32LE(_lockers);

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
	s.syncAsByte(fadeSetVolume, VER(46));
	s.syncAsByte(fadeCompleted, VER(46));
	s.syncAsByte(stopAfterFading, VER(45));
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
		soundRes = nullptr;
		pMidiParser = nullptr;
		pStreamAud = nullptr;
		reverb = -1;	// invalid reverb, will be initialized in processInitSound()
	}
}

void SoundCommandParser::syncPlayList(Common::Serializer &s) {
	_music->saveLoadWithSerializer(s);
}

void SoundCommandParser::reconstructPlayList() {
	_music->_mutex.lock();

	// We store all songs here because starting songs may re-shuffle their order
	MusicList songs;
	for (MusicList::iterator i = _music->getPlayListStart(); i != _music->getPlayListEnd(); ++i)
		songs.push_back(*i);

	// Done with main playlist, so release lock
	_music->_mutex.unlock();

	for (MusicList::iterator i = songs.begin(); i != songs.end(); ++i) {
		MusicEntry *entry = *i;
		initSoundResource(entry);

#ifdef ENABLE_SCI32
		if (_soundVersion >= SCI_VERSION_2 && entry->isSample) {
			const reg_t &soundObj = entry->soundObj;

			if (readSelectorValue(_segMan, soundObj, SELECTOR(loop)) == 0xFFFF &&
				readSelector(_segMan, soundObj, SELECTOR(handle)) != NULL_REG) {

				writeSelector(_segMan, soundObj, SELECTOR(handle), NULL_REG);
				processPlaySound(soundObj, entry->playBed);
			}
		} else
#endif
		if (entry->status == kSoundPlaying) {
			// WORKAROUND: PQ3 (German?) scripts can set volume negative in the
			// sound object directly without going through DoSound.
			// Since we re-read this selector when re-playing the sound after loading,
			// this will lead to unexpected behaviour. As a workaround we
			// sync the sound object's selectors here. (See bug #5501)
			writeSelectorValue(_segMan, entry->soundObj, SELECTOR(loop), entry->loop);
			writeSelectorValue(_segMan, entry->soundObj, SELECTOR(priority), entry->priority);
			if (_soundVersion >= SCI_VERSION_1_EARLY)
				writeSelectorValue(_segMan, entry->soundObj, SELECTOR(vol), entry->volume);

			processPlaySound(entry->soundObj, entry->playBed, true);
		}
	}

	// Emulate the original SCI0 behavior: If no sound with status kSoundPlaying was found we
	// look for the first sound with status kSoundPaused and start that. It relies on a correctly
	// sorted playlist, but we have that...
	if (_soundVersion <= SCI_VERSION_0_LATE && !_music->getFirstSlotWithStatus(kSoundPlaying)) {
		if (MusicEntry *pSnd = _music->getFirstSlotWithStatus(kSoundPaused)) {
			writeSelectorValue(_segMan, pSnd->soundObj, SELECTOR(loop), pSnd->loop);
			writeSelectorValue(_segMan, pSnd->soundObj, SELECTOR(priority), pSnd->priority);
			processPlaySound(pSnd->soundObj, pSnd->playBed, true);
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
	uint16 savedSize;

	if (s.isSaving()) {
		savedSize = _size;
	}

	s.syncAsByte(_type);
	s.syncAsByte(_elementSize);
	s.syncAsUint16LE(savedSize);

	if (s.isLoading()) {
		resize(savedSize);
	}

	switch (_type) {
	case kArrayTypeInt16:
	case kArrayTypeID:
		for (int i = 0; i < savedSize; ++i) {
			syncWithSerializer(s, ((reg_t *)_data)[i]);
		}
		break;
	case kArrayTypeByte:
	case kArrayTypeString:
		s.syncBytes((byte *)_data, savedSize);
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
		_buffer.init(getWidth(), getHeight(), getWidth(), getPixels(), Graphics::PixelFormat::createFormatCLUT8());
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
	if (s.isLoading()) {
		// Palette cycling schedules must be reset on load because we persist the engine tick count.
		// Otherwise, loading during cycling leaves the animation stuck until the new lower tick count
		// reaches the stale scheduled values. (Example: SQ5 Kiz Urazgubi waterfalls)
		// SSCI didn't persist or reset engine tick count so it didn't need to reset the schedules.
		_schedules.clear();
	}
	if (s.getVersion() >= 25) {
		// We need to save intensity of the _sysPalette at least for kq6 when entering the dark cave (room 390)
		//  from room 340. scripts will set intensity to 60 for this room and restore them when leaving.
		//  Sierra SCI is also doing this (although obviously not for SCI0->SCI01 games, still it doesn't hurt
		//  to save it everywhere). Refer to bug #5383
		s.syncBytes(_sysPalette.intensity, 256);
	}
	if (s.getVersion() >= 24) {
		if (s.isLoading() && _palVaryResourceId != -1)
			palVaryRemoveTimer();

		s.syncAsSint32LE(_palVaryResourceId);
		if (_palVaryResourceId != -1 || s.getVersion() >= 40) {
			if (_palVaryResourceId != -1) {
				palVarySaveLoadPalette(s, &_palVaryOriginPalette);
				palVarySaveLoadPalette(s, &_palVaryTargetPalette);
			}
			s.syncAsSint16LE(_palVaryStep);
			s.syncAsSint16LE(_palVaryStepStop);
			s.syncAsSint16LE(_palVaryDirection);
			s.syncAsUint16LE(_palVaryTicks);
			s.syncAsSint32LE(_palVaryPaused);
			if (s.getVersion() >= 40)
				s.syncAsSint32LE(_palVarySignal);
		}

		if (s.isLoading() && s.getVersion() < 40) {
			// Reset _palVaryPaused to 0 when loading an old savegame.
			// Before version 40, we didn't restore or reset _palVaryPaused.
			// In QfG3 this could get it stuck at positive values (bug #9674).
			//
			// Other SCI11 games don't appear to use palVaryPaused at all.
			// (Looked at eq2, freddy, kq6, lb2, mgoose11, pq1, qg1, sq4, sq5)
			_palVaryPaused = 0;

			// Clear any pending updates, since _palVarySignal also wasn't saved
			// before version 40.
			_palVarySignal = 0;
		}

		if (s.isLoading() && _palVaryResourceId != -1) {
			palVaryInstallTimer();
		}
	}
}

#ifdef ENABLE_SCI32
static void saveLoadPalette32(Common::Serializer &s, Palette &palette) {
	s.syncAsUint32LE(palette.timestamp);
	for (int i = 0; i < ARRAYSIZE(palette.colors); ++i) {
		s.syncAsByte(palette.colors[i].used);
		s.syncAsByte(palette.colors[i].r);
		s.syncAsByte(palette.colors[i].g);
		s.syncAsByte(palette.colors[i].b);
	}
}

static void saveLoadOptionalPalette32(Common::Serializer &s, Common::ScopedPtr<Palette> &palette) {
	bool hasPalette = false;
	if (s.isSaving()) {
		hasPalette = palette;
	}
	s.syncAsByte(hasPalette);
	if (hasPalette) {
		if (s.isLoading()) {
			palette.reset(new Palette);
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

		for (int i = 0; i < kNumCyclers; ++i) {
			_cyclers[i].reset();
		}

		_varyTargetPalette.reset();
		_varyStartPalette.reset();
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

	if (g_sci->_features->hasLatePaletteCode() && s.getVersion() >= 41) {
		s.syncAsSint16LE(_gammaLevel);
		saveLoadPalette32(s, _sourcePalette);
		++_version;
		_needsUpdate = true;
		_gammaChanged = true;
	}

	saveLoadOptionalPalette32(s, _varyTargetPalette);
	saveLoadOptionalPalette32(s, _varyStartPalette);

	// _nextPalette is not saved by SSCI

	for (int i = 0; i < ARRAYSIZE(_cyclers); ++i) {
		PalCycler *cycler = nullptr;

		bool hasCycler = false;
		if (s.isSaving()) {
			cycler = _cyclers[i].get();
			hasCycler = (cycler != nullptr);
		}
		s.syncAsByte(hasCycler);

		if (hasCycler) {
			if (s.isLoading()) {
				cycler = new PalCycler;
				_cyclers[i].reset(cycler);
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

void Audio32::saveLoadWithSerializer(Common::Serializer &s) {
	if (!g_sci->_features->hasSci3Audio() || s.getVersion() < 44) {
		return;
	}

	syncArray(s, _lockedResourceIds);
}

void Video32::beforeSaveLoadWithSerializer(Common::Serializer &s) {
	if (getSciVersion() < SCI_VERSION_3 || s.isSaving()) {
		return;
	}

	_robotPlayer.close();
}

void Video32::saveLoadWithSerializer(Common::Serializer &s) {
	if (getSciVersion() < SCI_VERSION_3) {
		return;
	}

	bool robotExists = _robotPlayer.getStatus() != RobotDecoder::kRobotStatusUninitialized;
	s.syncAsByte(robotExists);
	if (robotExists) {
		GuiResourceId robotId;
		reg_t planeId;
		Common::Point position;
		int16 priority, scale;
		int frameNo;

		if (s.isSaving()) {
			robotId = _robotPlayer.getResourceId();
			planeId = _robotPlayer.getPlaneId();
			priority = _robotPlayer.getPriority();
			position = _robotPlayer.getPosition();
			scale = _robotPlayer.getScale();
			frameNo = _robotPlayer.getFrameNo();
		}

		s.syncAsUint16LE(robotId);
		syncWithSerializer(s, planeId);
		s.syncAsSint16LE(priority);
		s.syncAsSint16LE(position.x);
		s.syncAsSint16LE(position.y);
		s.syncAsSint16LE(scale);
		s.syncAsSint32LE(frameNo);

		if (s.isLoading()) {
			_robotPlayer.open(robotId, planeId, priority, position.x, position.y, scale);
			_robotPlayer.showFrame(frameNo, position.x, position.y, priority);
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

bool gamestate_save(EngineState *s, int saveId, const Common::String &savename, const Common::String &version) {
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	const Common::String filename = g_sci->getSavegameName(saveId);

	Common::OutSaveFile *saveStream = saveFileMan->openForSaving(filename);
	if (saveStream == nullptr) {
		warning("Error opening savegame \"%s\" for writing", filename.c_str());
		return false;
	}

	if (!gamestate_save(s, saveStream, savename, version)) {
		warning("Saving the game failed");
		saveStream->finalize();
		delete saveStream;
		return false;
	}

	saveStream->finalize();
	if (saveStream->err()) {
		warning("Writing the savegame failed");
		delete saveStream;
		return false;
	}

	delete saveStream;
	return true;
}

bool gamestate_save(EngineState *s, Common::WriteStream *fh, const Common::String &savename, const Common::String &version) {
	Common::Serializer ser(nullptr, fh);
	Common::String ver = version;

	// If the game version is empty, we are probably saving from the GMM, so read it
	// from the version global and then the VERSION file
	if (ver.empty()) {
		ver = s->getGameVersionFromGlobal();
		if (ver.empty()) {
			Common::ScopedPtr<Common::SeekableReadStream> versionFile(SearchMan.createReadStreamForMember("VERSION"));
			ver = versionFile ? versionFile->readLine() : "";
		}
	}

	set_savegame_metadata(ser, fh, savename, ver);
	s->saveLoadWithSerializer(ser);		// FIXME: Error handling?
	if (g_sci->_gfxPorts)
		g_sci->_gfxPorts->saveLoadWithSerializer(ser);
	Vocabulary *voc = g_sci->getVocabulary();
	if (voc)
		voc->saveLoadWithSerializer(ser);

	// TODO: SSCI (at least JonesCD, presumably more) also stores the Menu state

	return true;
}

extern int showScummVMDialog(const Common::U32String &message, const Common::U32String &altButton = Common::U32String(), bool alignCenter = true);

void gamestate_afterRestoreFixUp(EngineState *s, int savegameId) {
	switch (g_sci->getGameId()) {
	case GID_CAMELOT: {
		// WORKAROUND: CAMELOT depends on its dynamic menu state persisting. The menu items'
		//  enabled states determines when the player can draw or sheathe their sword and
		//  open a purse. If these aren't updated then the player may be unable to perform
		//  necessary actions, or may be able to perform unexpected ones that break the game.
		//  Since we don't persist menu state (yet) we need to recreate it from game state.
		//
		// - Action \ Open Purse: Enabled while one of the purses is in inventory.
		// - Action \ Draw Sword: Enabled while flag 3 is set, unless disabled by room scripts.
		// * The text "Draw Sword" toggles to "Sheathe Sword" depending on global 124,
		//   but this is only cosmetic. Exported proc #1 in script 997 refreshes this
		//   when the sword status or room changes.
		//
		// After evaluating all the scripts that disable the sword, we enforce the few
		//  that prevent breaking the game: room 50 under the aqueduct and sitting with
		//  the scholar while in room 82 (ego view 84).
		//
		// FIXME: Save and restore full menu state as SSCI did and don't apply these
		//  workarounds when restoring saves that contain menu state.

		// Action \ Open Purse
		reg_t enablePurse = NULL_REG;
		Common::Array<reg_t> purses = s->_segMan->findObjectsByName("purse");
		reg_t ego = s->variables[VAR_GLOBAL][0];
		for (uint i = 0; i < purses.size(); ++i) {
			reg_t purseOwner = readSelector(s->_segMan, purses[i], SELECTOR(owner));
			if (purseOwner == ego) {
				enablePurse = TRUE_REG;
				break;
			}
		}
		g_sci->_gfxMenu->kernelSetAttribute(1281 >> 8, 1281 & 0xFF, SCI_MENU_ATTRIBUTE_ENABLED, enablePurse);

		// Action \ Draw Sword
		bool hasSword = (s->variables[VAR_GLOBAL][250].getOffset() & 0x1000); // flag 3
		bool underAqueduct = (s->variables[VAR_GLOBAL][11].getOffset() == 50);
		bool sittingWithScholar = (readSelectorValue(s->_segMan, ego, SELECTOR(view)) == 84);
		reg_t enableSword = (hasSword && !underAqueduct && !sittingWithScholar) ? TRUE_REG : NULL_REG;
		g_sci->_gfxMenu->kernelSetAttribute(1283 >> 8, 1283 & 0xFF, SCI_MENU_ATTRIBUTE_ENABLED, enableSword);
		break;
	}
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
		// We also set this in kSaveGame so that the global is correct even if no restoring occurs,
		//  otherwise the auto-delete script at the end of the SCI1.1 floppy version breaks if
		//  the game is played from start to finish. (bug #5294)
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
	case GID_KQ5:
		// WORKAROUND: We allow users to choose if they want the older KQ5 CD Windows cursors. These
		//  are black and white Cursor resources instead of the color View resources in the DOS version.
		//  This setting affects how KQCursor objects are initialized and might have changed since the
		//  game was saved. The scripts don't expect this since it wasn't an option in the original.
		//  In order for the cursors to correctly use the current setting, we need to clear the "number"
		//  property of every KQCursor when restoring when Windows cursors are disabled.
		if (g_sci->isCD() && !g_sci->_features->useWindowsCursors()) {
			Common::Array<reg_t> cursors = s->_segMan->findObjectsBySuperClass("KQCursor");
			for (uint i = 0; i < cursors.size(); ++i) {
				writeSelector(s->_segMan, cursors[i], SELECTOR(number), NULL_REG);
			}
		}
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
			if ((g_sci->getPlatform() == Common::kPlatformWindows) || (g_sci->useHiresGraphics())) {
				s->variables[VAR_GLOBAL][0xA9].setOffset(1);
			} else {
				s->variables[VAR_GLOBAL][0xA9].setOffset(0);
			}
		}
		break;
	case GID_PQ2:
		// HACK: Same as in Jones - enable the save game menu option when loading in
		// PQ2 (bug #6875). It gets disabled in the game's death screen.
		g_sci->_gfxMenu->kernelSetAttribute(2, 1, SCI_MENU_ATTRIBUTE_ENABLED, TRUE_REG);	// Game -> Save Game
		break;
#ifdef ENABLE_SCI32
	case GID_KQ7:
		if (Common::checkGameGUIOption(GAMEOPTION_UPSCALE_VIDEOS, ConfMan.get("guioptions"))) {
			uint16 value = ConfMan.getBool("enable_video_upscale") ? 32 : 0;
			s->variables[VAR_GLOBAL][kGlobalVarKQ7UpscaleVideos] = make_reg(0, value);
		}
		break;
	case GID_PHANTASMAGORIA2:
		if (Common::checkGameGUIOption(GAMEOPTION_ENABLE_CENSORING, ConfMan.get("guioptions"))) {
			s->variables[VAR_GLOBAL][kGlobalVarPhant2CensorshipFlag] = make_reg(0, ConfMan.getBool("enable_censoring"));
		}
		break;
	case GID_SHIVERS:
		// WORKAROUND: When loading a saved game from the GMM in the same scene in
		// Shivers, we end up with the same draw list, but the scene palette is not
		// set properly. Normally, Shivers does a room change when showing the saved
		// game list, which does not occur when loading directly from the GMM. When
		// loading from the GMM, at this point all of the visible planes and items
		// are deleted, so calling frameOut here helps reset the game palette
		// properly, like when changing a room.
		g_sci->_gfxFrameout->frameOut(true);
		break;
#endif
	default:
		break;
	}
}

bool gamestate_restore(EngineState *s, int saveId) {
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	const Common::String filename = g_sci->getSavegameName(saveId);
	Common::SeekableReadStream *saveStream = saveFileMan->openForLoading(filename);

	if (saveStream == nullptr) {
		warning("Savegame #%d not found", saveId);
		return false;
	}

	gamestate_restore(s, saveStream);
	delete saveStream;

	gamestate_afterRestoreFixUp(s, saveId);
	return true;
}

void gamestate_restore(EngineState *s, Common::SeekableReadStream *fh) {
	SavegameMetadata meta;

	Common::Serializer ser(fh, nullptr);
	sync_SavegameMetadata(ser, meta);

	if (fh->eos()) {
		s->r_acc = TRUE_REG;	// signal failure
		return;
	}

	// In SCI32 these checks are all in kCheckSaveGame32
	if (getSciVersion() < SCI_VERSION_2) {
		if ((meta.version < MINIMUM_SAVEGAME_VERSION) || (meta.version > CURRENT_SAVEGAME_VERSION)) {
			if (meta.version < MINIMUM_SAVEGAME_VERSION) {
				showScummVMDialog(_("The format of this saved game is obsolete, unable to load it"));
			} else {
				Common::U32String msg = Common::U32String::format(_("Savegame version is %d, maximum supported is %0d"), meta.version, CURRENT_SAVEGAME_VERSION);
				showScummVMDialog(msg);
			}

			s->r_acc = TRUE_REG;	// signal failure
			return;
		}

		if (meta.gameObjectOffset > 0 && meta.script0Size > 0) {
			Resource *script0 = g_sci->getResMan()->findResource(ResourceId(kResourceTypeScript, 0), false);
			assert(script0);
			if (script0->size() != meta.script0Size || g_sci->getGameObject().getOffset() != meta.gameObjectOffset) {
				showScummVMDialog(_("This saved game was created with a different version of the game, unable to load it"));

				s->r_acc = TRUE_REG;	// signal failure
				return;
			}
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

	Vocabulary *voc = g_sci->getVocabulary();
	if (ser.getVersion() >= 30 && voc)
		voc->saveLoadWithSerializer(ser);

	g_sci->_soundCmd->reconstructPlayList();

	// Message state:
	s->initMessageState();

	// System strings:
	s->_segMan->initSysStrings();

	s->abortScriptProcessing = kAbortLoadGame;

	// signal restored game to game scripts
	s->gameIsRestarting = GAMEISRESTARTING_RESTORE;
}

void set_savegame_metadata(Common::Serializer &ser, Common::WriteStream *fh, const Common::String &savename, const Common::String &version) {
	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	SavegameMetadata meta;
	meta.version = CURRENT_SAVEGAME_VERSION;
	meta.name = savename;
	meta.gameVersion = version;
	meta.saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	meta.saveTime = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);

	Resource *script0 = g_sci->getResMan()->findResource(ResourceId(kResourceTypeScript, 0), false);
	assert(script0);
	meta.script0Size = script0->size();
	meta.gameObjectOffset = g_sci->getGameObject().getOffset();

	sync_SavegameMetadata(ser, meta);
	Graphics::saveThumbnail(*fh);
}

void set_savegame_metadata(Common::WriteStream *fh, const Common::String &savename, const Common::String &version) {
	Common::Serializer ser(nullptr, fh);
	set_savegame_metadata(ser, fh, savename, version);
}

bool get_savegame_metadata(Common::SeekableReadStream *stream, SavegameMetadata &meta) {
	assert(stream);

	Common::Serializer ser(stream, nullptr);
	sync_SavegameMetadata(ser, meta);

	if (stream->eos())
		return false;

	if ((meta.version < MINIMUM_SAVEGAME_VERSION) ||
	    (meta.version > CURRENT_SAVEGAME_VERSION)) {
		if (meta.version < MINIMUM_SAVEGAME_VERSION)
			warning("Old savegame version detected- can't load");
		else
			warning("Savegame version is %d- maximum supported is %0d", meta.version, CURRENT_SAVEGAME_VERSION);

		return false;
	}

	return true;
}

} // End of namespace Sci
