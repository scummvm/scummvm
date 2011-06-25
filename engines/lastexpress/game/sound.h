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
 */

#ifndef LASTEXPRESS_SOUND_H
#define LASTEXPRESS_SOUND_H

#include "lastexpress/shared.h"

#include "lastexpress/helpers.h"

#include "common/list.h"
#include "common/mutex.h"
#include "common/system.h"
#include "common/serializer.h"

namespace LastExpress {

class LastExpressEngine;
class SubtitleManager;
class SoundEntry;
class SubtitleEntry;

class SoundManager : Common::Serializable {
public:
	SoundManager(LastExpressEngine *engine);
	~SoundManager();

	// Timer
	void handleTimer();

	// State
	void resetState() { _state |= kSoundType1; }

	// Sound queue
	void updateQueue();
	void resetQueue(SoundType type1, SoundType type2 = kSoundTypeNone);
	void clearQueue();

	// Subtitles
	void updateSubtitles();

	// Entry
	bool isBuffered(Common::String filename, bool testForEntity = false);
	bool isBuffered(EntityIndex entity);
	void setupEntry(SoundType type, EntityIndex index);
	void processEntry(EntityIndex entity);
	void processEntry(SoundType type);
	void processEntry(Common::String filename);
	void processEntries();
	void removeFromQueue(Common::String filename);
	void removeFromQueue(EntityIndex entity);
	uint32 getEntryTime(EntityIndex index);

	// Misc
	void unknownFunction4();
	void clearStatus();

	// Sound playing
	void playSound(EntityIndex entity, Common::String filename, SoundFlag flag = kFlagInvalid, byte a4 = 0);
	bool playSoundWithSubtitles(Common::String filename, SoundFlag flag, EntityIndex entity, byte a4 = 0);
	void playSoundEvent(EntityIndex entity, byte action, byte a3 = 0);
	void playDialog(EntityIndex entity, EntityIndex entityDialog, SoundFlag flag, byte a4);
	void playSteam(CityIndex index);
	void playFightSound(byte action, byte a4);
	void playLocomotiveSound();
	void playWarningCompartment(EntityIndex entity, ObjectIndex compartment);

	// Dialog & Letters
	void readText(int id);
	const char *getDialogName(EntityIndex entity) const;

	// Sound bites
	void excuseMe(EntityIndex entity, EntityIndex entity2 = kEntityPlayer, SoundFlag flag = kFlagNone);
	void excuseMeCath();
	const char *justCheckingCath() const;
	const char *wrongDoorCath() const;
	const char *justAMinuteCath() const;

	// FLags
	SoundFlag getSoundFlag(EntityIndex index) const;

	// Debug
	void stopAllSound();

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &ser);
	uint32 count();

	// Accessors
	uint32 getFlag() { return _flag; }
	int getSubtitleFlag() { return _subtitlesFlag; }
	void setSubtitleFlag(int flag) { _subtitlesFlag = flag; }

	// Subtitles
	void addSubtitle(SubtitleEntry *entry) { _subtitles.push_back(entry); }
	void removeSubtitle(SubtitleEntry *entry) { _subtitles.remove(entry); }
	void setCurrentSubtitle(SubtitleEntry *entry) { _currentSubtitle = entry; }
	SubtitleEntry *getCurrentSubtitle() { return _currentSubtitle; }

private:
	typedef int32 *SoundBuffer;

	enum SoundState {
		kSoundState0 = 0,
		kSoundState1 = 1,
		kSoundState2 = 2
	};

	// Engine
	LastExpressEngine *_engine;

	// State flag
	int _state;
	SoundType _currentType;

	Common::Mutex _mutex;

	// Unknown data
	uint32 _data0;
	uint32 _data1;
	uint32 _data2;

	// TODO: this seems to be a synchronization flag for the sound timer
	uint32 _flag;

	// Filters
	int32 _buffer[2940];    ///< Static sound buffer

	// Compartment warnings by Mertens or Coudert
	uint32 _lastWarning[12];

	// Looping sound
	void playLoopingSound(int param);
	int _loopingSoundDuration;

	// Sound entries
	Common::List<SoundEntry *> _soundList;    ///< List of all sound entries
	Common::List<SoundEntry *> _soundCache;   ///< List of entries with a data buffer
	void *_soundCacheData;

	SoundEntry *getEntry(EntityIndex index);
	SoundEntry *getEntry(Common::String name);
	SoundEntry *getEntry(SoundType type);

	void setupEntry(SoundEntry *entry, Common::String name, SoundFlag flag, int priority);
	void setEntryType(SoundEntry *entry, SoundFlag flag);
	bool setupCache(SoundEntry *entry);
	void removeFromCache(SoundEntry *entry);
	void loadSoundData(SoundEntry *entry, Common::String name);
	void removeEntry(SoundEntry *entry);

	// Subtitles
	int _subtitlesFlag;
	Common::List<SubtitleEntry *> _subtitles;
	SubtitleEntry *_currentSubtitle;

	// Sound filter
	void applyFilter(SoundEntry *entry, int16 *buffer);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SOUND_H
