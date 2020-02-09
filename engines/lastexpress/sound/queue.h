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

#ifndef LASTEXPRESS_SOUND_QUEUE_H
#define LASTEXPRESS_SOUND_QUEUE_H

#include "lastexpress/shared.h"

#include "common/array.h"
#include "common/mutex.h"
#include "common/serializer.h"

namespace LastExpress {

class LastExpressEngine;
class SoundEntry;
class SubtitleEntry;

class SoundQueue : Common::Serializable {
public:
	SoundQueue(LastExpressEngine *engine);
	~SoundQueue() override;

	// Queue
	void addToQueue(SoundEntry *entry);
	void stop(Common::String filename);
	void stop(EntityIndex entity);
	void updateQueue();
	void stopAmbient();
	void stopAllExcept(SoundTag tag1, SoundTag tag2 = kSoundTagNone);
	void destroyAllSound();

	// State
	void stopAll();
	int getAmbientState() { return _ambientState; }
	void startAmbient() { _ambientState |= kAmbientSoundEnabled; }
	void setAmbientToSteam() { _ambientState |= kAmbientSoundSteam; }

	// Entries
	void assignNISLink(EntityIndex index);
	void fade(EntityIndex entity);
	void fade(SoundTag tag);
	void fade(Common::String filename);
	void endAmbient();
	SoundEntry *getEntry(SoundTag tag);
	SoundEntry *getEntry(EntityIndex index);
	SoundEntry *getEntry(Common::String name);
	uint32 getEntryTime(EntityIndex index);
	bool isBuffered(Common::String filename, bool testForEntity = false);
	bool isBuffered(EntityIndex entity);

	// Subtitles
	void updateSubtitles();
	void addSubtitle(SubtitleEntry *entry) { _subtitles.push_back(entry); }
	void removeSubtitle(SubtitleEntry *entry) { _subtitles.remove(entry); }
	void setCurrentSubtitle(SubtitleEntry *entry) { _currentSubtitle = entry; }
	SubtitleEntry *getCurrentSubtitle() { return _currentSubtitle; }

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &ser) override;
	uint32 count();

	// Accessors
	uint32 getFlag() { return _flag; }
	int getSubtitleFlag() { return _subtitlesFlag; }
	void setSubtitleFlag(int flag) { _subtitlesFlag = flag; }

	int32 generateNextTag() { return _currentTag++; }

protected:
	// Debug
	void stopAllSound();

private:
	LastExpressEngine *_engine;

	// State & shared data
	int _ambientState;
	int32 _currentTag;
	uint32 _flag;

	// Entries
	Common::List<SoundEntry *> _soundList;    ///< List of all sound entries
	//void *_soundCacheData;

	// Subtitles
	int _subtitlesFlag;
	Common::List<SubtitleEntry *> _subtitles;
	SubtitleEntry *_currentSubtitle;

	friend class Debugger;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SOUND_QUEUE_H
