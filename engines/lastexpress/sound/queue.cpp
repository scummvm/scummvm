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

#include "lastexpress/sound/queue.h"

#include "lastexpress/game/logic.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/entry.h"
#include "lastexpress/sound/sound.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"

namespace LastExpress {

SoundQueue::SoundQueue(LastExpressEngine *engine) : _engine(engine) {
	 _ambientState = 0;
	 _currentTag = kSoundTagFirstNormal;
	 _flag = 0;

	_subtitlesFlag = 0;
	_currentSubtitle = NULL;
	//_soundCacheData = NULL;
}

SoundQueue::~SoundQueue() {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		SAFE_DELETE(*i);
	_soundList.clear();

	for (Common::List<SubtitleEntry *>::iterator i = _subtitles.begin(); i != _subtitles.end(); ++i)
		SAFE_DELETE(*i);
	_subtitles.clear();

	_currentSubtitle = NULL;
	//SAFE_DELETE(_soundCacheData);

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Sound queue management
//////////////////////////////////////////////////////////////////////////
void SoundQueue::addToQueue(SoundEntry *entry) {
	_soundList.push_back(entry);
}

void SoundQueue::stop(EntityIndex entity) {
	SoundEntry *entry = getEntry(entity);
	if (entry)
		entry->kill();
}

void SoundQueue::stop(Common::String filename) {
	SoundEntry *entry = getEntry(filename);
	if (entry)
		entry->kill();
}

void SoundQueue::updateQueue() {
	++_flag;

	if (getAmbientState() & kAmbientSoundEnabled) {
		SoundEntry *entry = getEntry(kSoundTagAmbient);
		if (!entry || getFlags()->flag_3 || (entry && entry->getTime() > getSound()->getAmbientSoundDuration())) {
			getSound()->playAmbientSound(0x45);
		} else {
			if (getSound()->needToChangeAmbientVolume()) {
				entry->setVolumeSmoothly(getSound()->getChangedAmbientVolume());
				getSound()->clearAmbientVolumeChange();
			}
		}
	}

	for (Common::List<SoundEntry *>::iterator it = _soundList.begin(); it != _soundList.end(); ++it) {
		SoundEntry *entry = *it;
		if (entry == NULL)
			error("[SoundQueue::updateQueue] Invalid entry found in sound queue");

		// Original removes the entry data from the cache and sets the archive as not loaded
		// and if the sound data buffer is not full, loads a new entry to be played based on
		// its priority and filter id

		if (!entry->update() && !(entry->getStatus() & kSoundFlagKeepAfterFinish)) {
			entry->close();
			SAFE_DELETE(entry);
			it = _soundList.reverse_erase(it);
			continue;
		}

		// When the entry has stopped playing, we remove his buffer
		if (entry->isFinished()) {
			entry->close();
			SAFE_DELETE(entry);
			it = _soundList.reverse_erase(it);
			continue;
		}
	}

	// Original update the current entry, loading another set of samples to be decoded

	getFlags()->flag_3 = false;

	--_flag;
}

void SoundQueue::stopAmbient() {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getTag() == kSoundTagAmbient) {
			(*i)->kill();
			break;
		}
	}

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getTag() == kSoundTagOldAmbient) {
			(*i)->kill();
			break;
		}
	}
}

void SoundQueue::stopAllExcept(SoundTag type1, SoundTag type2) {
	if (!type2)
		type2 = type1;

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getTag() != type1 && (*i)->getTag() != type2)
			(*i)->kill();
	}
}

void SoundQueue::destroyAllSound() {
	_flag |= 8;

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		SoundEntry *entry = (*i);
		if (entry == NULL)
			error("[SoundQueue::destroyAllSound] Invalid entry found in sound queue");

		// Delete entry
		entry->close();
		SAFE_DELETE(entry);

		i = _soundList.reverse_erase(i);
	}

	updateSubtitles();
}

//////////////////////////////////////////////////////////////////////////
// State
//////////////////////////////////////////////////////////////////////////
void SoundQueue::stopAll() {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		(*i)->addStatusFlag(kSoundFlagClosed);
}

//////////////////////////////////////////////////////////////////////////
// Entry management
//////////////////////////////////////////////////////////////////////////
void SoundQueue::assignNISLink(EntityIndex index) {
	SoundEntry *entry = getEntry(kSoundTagLink);
	if (entry)
		entry->setEntity(index);
}

void SoundQueue::fade(EntityIndex entity) {
	SoundEntry *entry = getEntry(entity);
	if (entry) {
		entry->fade();
		entry->setEntity(kEntityPlayer);
	}
}

void SoundQueue::fade(SoundTag type) {
	SoundEntry *entry = getEntry(type);
	if (entry)
		entry->fade();
}

void SoundQueue::fade(Common::String filename) {
	SoundEntry *entry = getEntry(filename);
	if (entry) {
		entry->fade();
		entry->setEntity(kEntityPlayer);
	}
}

void SoundQueue::endAmbient() {
	_ambientState = 0;

	fade(kSoundTagAmbient);
	fade(kSoundTagOldAmbient);
}

SoundEntry *SoundQueue::getEntry(EntityIndex index) {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getEntity() == index)
			return *i;
	}

	return NULL;
}

SoundEntry *SoundQueue::getEntry(Common::String name) {
	if (!name.contains('.'))
		name += ".SND";

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getName().equalsIgnoreCase(name))
			return *i;
	}

	return NULL;
}

SoundEntry *SoundQueue::getEntry(SoundTag type) {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getTag() == type)
			return *i;
	}

	return NULL;
}

uint32 SoundQueue::getEntryTime(EntityIndex index) {
	SoundEntry *entry = getEntry(index);
	if (entry)
		return entry->getTime();

	return 0;
}

bool SoundQueue::isBuffered(EntityIndex entity) {
	return (getEntry(entity) != NULL);
}

bool SoundQueue::isBuffered(Common::String filename, bool testForEntity) {
	SoundEntry *entry = getEntry(filename);

	if (testForEntity)
		return entry != NULL && entry->getEntity() != kEntityPlayer;

	return (entry != NULL);
}

//////////////////////////////////////////////////////////////////////////
// Subtitles
//////////////////////////////////////////////////////////////////////////
void SoundQueue::updateSubtitles() {
	uint32 index = 0;
	SubtitleEntry *subtitle = NULL;

	for (Common::List<SubtitleEntry *>::iterator i = _subtitles.begin(); i != _subtitles.end(); ++i) {
		uint32 current_index = 0;
		SoundEntry *soundEntry = (*i)->getSoundEntry();
		SoundFlag status = (SoundFlag)soundEntry->getStatus();

		if (!(status & kSoundFlagPlaying)
		 || status & kSoundFlagMute
		 || soundEntry->getTime() == 0
		 || (status & kSoundVolumeMask) < kVolume6
		 || ((getFlags()->nis & 0x8000) && soundEntry->getPriority() < 90)) {
			 current_index = 0;
		} else {
			current_index = soundEntry->getPriority() + (status & kSoundVolumeMask);

			if (_currentSubtitle == (*i))
				current_index += 4;
		}

		if (index < current_index) {
			index = current_index;
			subtitle = (*i);
		}
	}

	if (_currentSubtitle == subtitle) {
		if (subtitle)
			subtitle->setupAndDraw();

		return;
	}

	if (!subtitle)
		return;

	if (_subtitlesFlag & 1)
		subtitle->drawOnScreen();

	subtitle->loadData();
	subtitle->setupAndDraw();
}

//////////////////////////////////////////////////////////////////////////
// Savegame
//////////////////////////////////////////////////////////////////////////
void SoundQueue::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsUint32LE(_ambientState);
	s.syncAsUint32LE(_currentTag);

	// Compute the number of entries to save
	uint32 numEntries = count();
	s.syncAsUint32LE(numEntries);

	// Save or load each entry data
	if (s.isSaving()) {
		for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
			(*i)->saveLoadWithSerializer(s);
	} else {
		warning("[Sound::saveLoadWithSerializer] Loading not implemented");

		uint32 unusedDataSize = numEntries * 64;
		if (s.isLoading()) {
			byte *empty = (byte *)malloc(unusedDataSize);
			s.syncBytes(empty, unusedDataSize);
			free(empty);
		} else {
			s.skip(unusedDataSize);
		}
	}
}


uint32 SoundQueue::count() {
	uint32 numEntries = 0;
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		if ((*i)->getName().matchString("NISSND?"))
			++numEntries;

	return numEntries;
}

//////////////////////////////////////////////////////////////////////////
// Debug
//////////////////////////////////////////////////////////////////////////
void SoundQueue::stopAllSound() {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		(*i)->getSoundStream()->stop();
}

} // End of namespace LastExpress
