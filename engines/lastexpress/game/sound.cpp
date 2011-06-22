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

#include "lastexpress/game/sound.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/helpers.h"
#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

#define SOUNDCACHE_ENTRY_SIZE 92160
#define SOUNDCACHE_MAX_SIZE   6

// Letters & messages
const char *messages[24] = {
	"",
	"TXT1001",  // 1
	"TXT1001A", // 2
	"TXT1011",  // 3
	"TXT1012",  // 4
	"TXT1013",  // 5
	"TXT1014",  // 6
	"TXT1020",  // 7
	"TXT1030",  // 8
	"END1009B", // 50
	"END1046",  // 51
	"END1047",  // 52
	"END1112",  // 53
	"END1112A", // 54
	"END1503",  // 55
	"END1505A", // 56
	"END1505B", // 57
	"END1610",  // 58
	"END1612A", // 59
	"END1612C", // 61
	"END1612D", // 62
	"ENDALRM1", // 63
	"ENDALRM2", // 64
	"ENDALRM3"  // 65
};

const char *cities[17] = {
	"EPERNAY",
	"CHALONS",
	"BARLEDUC",
	"NANCY",
	"LUNEVILL",
	"AVRICOUR",
	"DEUTSCHA",
	"STRASBOU",
	"BADENOOS",
	"SALZBURG",
	"ATTNANG",
	"WELS",
	"LINZ",
	"VIENNA",
	"POZSONY",
	"GALANTA",
	"POLICE"
};

const char *locomotiveSounds[5] = {
	"ZFX1005",
	"ZFX1006",
	"ZFX1007",
	"ZFX1007A",
	"ZFX1007B"
};

static const SoundManager::FlagType soundFlags[32] = {
	SoundManager::kFlagDefault, SoundManager::kFlag15, SoundManager::kFlag14, SoundManager::kFlag13, SoundManager::kFlag12,
	SoundManager::kFlag11,      SoundManager::kFlag11, SoundManager::kFlag10, SoundManager::kFlag10, SoundManager::kFlag9,  SoundManager::kFlag9, SoundManager::kFlag8, SoundManager::kFlag8,
	SoundManager::kFlag7,       SoundManager::kFlag7,  SoundManager::kFlag7,  SoundManager::kFlag6,  SoundManager::kFlag6,  SoundManager::kFlag6,
	SoundManager::kFlag5,       SoundManager::kFlag5,  SoundManager::kFlag5,  SoundManager::kFlag5,  SoundManager::kFlag4,  SoundManager::kFlag4, SoundManager::kFlag4, SoundManager::kFlag4,
	SoundManager::kFlag3,       SoundManager::kFlag3,  SoundManager::kFlag3,  SoundManager::kFlag3,  SoundManager::kFlag3
};

SoundManager::SoundManager(LastExpressEngine *engine) : _engine(engine), _state(0), _currentType(kSoundType16), _flag(0) {
	// Initialize unknown data
	_data0 = 0;
	_data1 = 0;
	_data2 = 0;

	memset(&_buffer, 0, sizeof(_buffer));
	memset(&_lastWarning, 0, sizeof(_lastWarning));

	// Sound cache
	_soundCacheData = malloc(6 * SOUNDCACHE_ENTRY_SIZE);

	_drawSubtitles = 0;
	_currentSubtitle = NULL;
}

SoundManager::~SoundManager() {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		SAFE_DELETE(*i);
	_soundList.clear();

	// Entries in the cache are just pointers to sound list entries
	_soundCache.clear();

	for (Common::List<SubtitleEntry *>::iterator i = _subtitles.begin(); i != _subtitles.end(); ++i)
		SAFE_DELETE(*i);
	_subtitles.clear();

	_currentSubtitle = NULL;

	free(_soundCacheData);

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Timer
//////////////////////////////////////////////////////////////////////////
void SoundManager::handleTimer() {
	Common::StackLock locker(_mutex);

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		SoundEntry *entry = (*i);
		if (entry->stream == NULL) {
			SAFE_DELETE(*i);
			i = _soundList.reverse_erase(i);
			continue;
		} else if (!entry->soundStream) {
			entry->soundStream = new StreamedSound();

			// TODO: stream any sound in the queue after filtering
			entry->soundStream->load(entry->stream);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Sound queue management
//////////////////////////////////////////////////////////////////////////
void SoundManager::updateQueue() {
	// TODO add mutex lock!
	warning("Sound::updateQueue: not implemented!");
}

void SoundManager::resetQueue(SoundType type1, SoundType type2) {
	if (!type2)
		type2 = type1;

	Common::StackLock locker(_mutex);

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->type != type1 && (*i)->type != type2)
			resetEntry(*i);
	}
}

void SoundManager::removeFromQueue(EntityIndex entity) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(entity);
	if (entry)
		resetEntry(entry);
}

void SoundManager::removeFromQueue(Common::String filename) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(filename);
	if (entry)
		resetEntry(entry);
}

void SoundManager::clearQueue() {
	_flag |= 4;

	// FIXME: Wait a while for a flag to be set
	//for (int i = 0; i < 3000000; i++)
	//	if (_flag & 8)
	//		break;

	_flag |= 8;

	Common::StackLock locker(_mutex);

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		SoundEntry *entry = (*i);

		// Delete entry
		removeEntry(entry);
		SAFE_DELETE(entry);

		i = _soundList.reverse_erase(i);
	}

	updateSubtitles();
}

bool SoundManager::isBuffered(EntityIndex entity) {
	Common::StackLock locker(_mutex);

	return (getEntry(entity) != NULL);
}

bool SoundManager::isBuffered(Common::String filename, bool testForEntity) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(filename);

	if (testForEntity)
		return entry != NULL && !entry->entity;

	return (entry != NULL);
}

//////////////////////////////////////////////////////////////////////////
// Entry
//////////////////////////////////////////////////////////////////////////
void SoundManager::setupEntry(SoundEntry *entry, Common::String name, FlagType flag, int a4) {
	if (!entry)
		error("SoundManager::setupEntry: Invalid entry!");

	entry->field_4C = a4;
	setEntryType(entry, flag);
	setEntryStatus(entry, flag);

	// Add entry to sound list
	_soundList.push_back(entry);

	// TODO Add entry to cache and load sound data
	//setupCache(entry);
	loadSoundData(entry, name);
}

void SoundManager::setEntryType(SoundEntry *entry, FlagType flag) {
	switch (flag & kFlagType9) {
	default:
	case kFlagNone:
		entry->type = _currentType;
		_currentType = (SoundType)(_currentType + 1);
		break;

	case kFlagType1_2: {
		SoundEntry *previous2 = getEntry(kSoundType2);
		if (previous2)
			updateEntry(previous2, 0);

		SoundEntry *previous = getEntry(kSoundType1);
		if (previous) {
			previous->type = kSoundType2;
			updateEntry(previous, 0);
		}

		entry->type = kSoundType1;
		}
		break;

	case kFlagType3: {
		SoundEntry *previous = getEntry(kSoundType3);
		if (previous) {
			previous->type = kSoundType4;
			updateEntry(previous, 0);
		}

		entry->type = kSoundType11;
		}
		break;

	case kFlagType7: {
		SoundEntry *previous = getEntry(kSoundType7);
		if (previous)
			previous->type = kSoundType8;

		entry->type = kSoundType7;
		}
		break;

	case kFlagType9: {
		SoundEntry *previous = getEntry(kSoundType9);
		if (previous)
			previous->type = kSoundType10;

		entry->type = kSoundType9;
		}
		break;

	case kFlagType11: {
		SoundEntry *previous = getEntry(kSoundType11);
		if (previous)
			previous->type = kSoundType14;

		entry->type = kSoundType11;
		}
		break;

	case kFlagType13: {
		SoundEntry *previous = getEntry(kSoundType13);
		if (previous)
			previous->type = kSoundType14;

		entry->type = kSoundType13;
		}
		break;
	}
}

void SoundManager::setEntryStatus(SoundEntry *entry, FlagType flag) const {
	SoundStatus status = (SoundStatus)flag;
	if (!((status & 0xFF) & kSoundStatusClear1))
		status = (SoundStatus)(status | kSoundStatusClear2);

	if (((status & 0xFF00) >> 8) & kSoundStatusClear0)
		entry->status.status = (uint32)status;
	else
		entry->status.status = (status | kSoundStatusClear4);
}

void SoundManager::setInCache(SoundEntry *entry) {
	entry->status.status |= kSoundStatusClear2;
}

bool SoundManager::setupCache(SoundEntry *entry) {
	if (entry->soundData)
		return true;

	if (_soundCache.size() >= SOUNDCACHE_MAX_SIZE) {

		SoundEntry *cacheEntry = NULL;
		uint32 size = 1000;

		for (Common::List<SoundEntry *>::iterator i = _soundCache.begin(); i != _soundCache.end(); ++i) {
			if (!((*i)->status.status & kSoundStatus_180)) {
				uint32 newSize = (*i)->field_4C + ((*i)->status.status & kSoundStatusClear1);

				if (newSize < size) {
					cacheEntry = (*i);
					size = newSize;
				}
			}
		}

		if (entry->field_4C <= size)
			return false;

		if (cacheEntry)
			setInCache(cacheEntry);

		// TODO: Wait until the cache entry is ready to be removed
		while (!(cacheEntry->status.status1 & 1))
			;

		if (cacheEntry->soundData)
			removeFromCache(cacheEntry);

		_soundCache.push_back(entry);
		entry->soundData = (char *)_soundCacheData + SOUNDCACHE_ENTRY_SIZE * (_soundCache.size() - 1);
	} else {
		_soundCache.push_back(entry);
		entry->soundData = (char *)_soundCacheData + SOUNDCACHE_ENTRY_SIZE * (_soundCache.size() - 1);
	}

	return true;
}

void SoundManager::removeFromCache(SoundEntry *entry) {
	for (Common::List<SoundEntry *>::iterator i = _soundCache.begin(); i != _soundCache.end(); ++i) {
		if ((*i) == entry) {
			// Remove sound buffer
			entry->soundData = NULL;

			// Remove entry from sound cache
			i = _soundCache.reverse_erase(i);
		}
	}
}

void SoundManager::clearStatus() {
	Common::StackLock locker(_mutex);

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		(*i)->status.status |= kSoundStatusClear3;
}

void SoundManager::loadSoundData(SoundEntry *entry, Common::String name) {
	entry->name2 = name;

	// Load sound data
	entry->stream = getArchive(name);

	if (!entry->stream)
		entry->stream = getArchive("DEFAULT.SND");

	if (entry->stream) {
		warning("Sound::loadSoundData: not implemented!");
	} else {
		entry->status.status = kSoundStatusRemoved;
	}
}

void SoundManager::resetEntry(SoundEntry *entry) const {
	entry->status.status |= kSoundStatusRemoved;
	entry->entity = kEntityPlayer;

	if (entry->stream) {
		if (!entry->soundStream) {
			SAFE_DELETE(entry->stream);
		} else {
			entry->soundStream->stop();
			SAFE_DELETE(entry->soundStream);
		}

		entry->stream = NULL;
	}
}


void SoundManager::removeEntry(SoundEntry *entry) {
	entry->status.status |= kSoundStatusRemoved;

	// Loop until ready
	while (!(entry->status.status1 & 4) && !(_flag & 8) && (_flag & 1))
		;	// empty loop body

	// The original game remove the entry from the cache here,
	// but since we are called from within an iterator loop
	// we will remove the entry there
	// removeFromCache(entry);

	if (entry->subtitle) {
		drawSubtitle(entry->subtitle);
		SAFE_DELETE(entry->subtitle);
	}

	if (entry->entity) {
		if (entry->entity == kEntitySteam)
			playLoopingSound();
		else if (entry->entity != kEntityTrain)
			getSavePoints()->push(kEntityPlayer, entry->entity, kActionEndSound);
	}
}

void SoundManager::updateEntry(SoundEntry *entry, uint value) const {
	if (!(entry->status.status3 & 64)) {
		int value2 = value;

		entry->status.status |= kSoundStatus_100000;

		if (value) {
			if (_flag & 32) {
				entry->field_40 = value;
				value2 = value * 2 + 1;
			}

			entry->field_3C = value2;
		} else {
			entry->field_3C = 0;
			entry->status.status |= kSoundStatus_40000000;
		}
	}
}

void SoundManager::updateEntryState(SoundEntry *entry) const {
	if (_flag & 32) {
		if (entry->type != kSoundType9 && entry->type != kSoundType7 && entry->type != kSoundType5) {
			uint32 status = entry->status.status & kSoundStatusClear1;

			entry->status.status &= kSoundStatusClearAll;

			entry->field_40 = status;
			entry->status.status |= status * 2 + 1;
		}
	}

	entry->status.status |= kSoundStatus_20;
}

void SoundManager::processEntry(EntityIndex entity) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(entity);
	if (entry) {
		updateEntry(entry, 0);
		entry->entity = kEntityPlayer;
	}
}

void SoundManager::processEntry(SoundType type) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(type);
	if (entry)
		updateEntry(entry, 0);
}

void SoundManager::setupEntry(SoundType type, EntityIndex index) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(type);
	if (entry)
		entry->entity = index;
}

void SoundManager::processEntry(Common::String filename) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(filename);
	if (entry) {
		updateEntry(entry, 0);
		entry->entity = kEntityPlayer;
	}
}

void SoundManager::processEntries() {
	_state = 0;

	processEntry(kSoundType1);
	processEntry(kSoundType2);
}

uint32 SoundManager::getEntryTime(EntityIndex index) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(index);
	if (entry)
		return entry->time;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////

void SoundManager::unknownFunction4() {
	// TODO: Add mutex ?
	warning("Sound::unknownFunction4: not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Entry search
//////////////////////////////////////////////////////////////////////////
SoundManager::SoundEntry *SoundManager::getEntry(EntityIndex index) {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->entity == index)
			return *i;
	}

	return NULL;
}

SoundManager::SoundEntry *SoundManager::getEntry(Common::String name) {
	if (!name.contains('.'))
		name += ".SND";

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->name2 == name)
			return *i;
	}

	return NULL;
}

SoundManager::SoundEntry *SoundManager::getEntry(SoundType type) {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->type == type)
			return *i;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// Savegame
//////////////////////////////////////////////////////////////////////////
void SoundManager::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsUint32LE(_state);
	s.syncAsUint32LE(_currentType);

	// Compute the number of entries to save
	uint32 numEntries = count();
	s.syncAsUint32LE(numEntries);

	Common::StackLock locker(_mutex);

	// Save or load each entry data
	if (s.isSaving()) {
		for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
			SoundEntry *entry = *i;
			if (entry->name2.matchString("NISSND?") && (entry->status.status & kFlagType7) != kFlag3) {
				s.syncAsUint32LE(entry->status.status); // status;
				s.syncAsUint32LE(entry->type); // type;
				s.syncAsUint32LE(entry->blockCount); // field_8;
				s.syncAsUint32LE(entry->time); // time;
				s.syncAsUint32LE(entry->field_34); // field_10;
				s.syncAsUint32LE(entry->field_38); // field_14;
				s.syncAsUint32LE(entry->entity); // entity;

				uint32 blockCount = (uint32)entry->field_48 - _data2;
				if (blockCount > kFlag8)
					blockCount = 0;
				s.syncAsUint32LE(blockCount); // blockCount;

				s.syncAsUint32LE(entry->field_4C); // field_20;

				char name1[16];
				strcpy((char *)&name1, entry->name1.c_str());
				s.syncBytes((byte *)&name1, 16);

				char name2[16];
				strcpy((char *)&name2, entry->name2.c_str());
				s.syncBytes((byte *)&name2, 16);
			}
		}
	} else {
		warning("Sound::saveLoadWithSerializer: not implemented!");
		s.skip(numEntries * 64);
	}
}


// FIXME: We probably need another mutex here to protect during the whole savegame process
// as we could have removed an entry between the time we check the count and the time we
// save the entries
uint32 SoundManager::count() {
	Common::StackLock locker(_mutex);

	uint32 numEntries = 0;
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		if ((*i)->name2.matchString("NISSND?"))
			++numEntries;

	return numEntries;
}

//////////////////////////////////////////////////////////////////////////
// Game-related functions
//////////////////////////////////////////////////////////////////////////
void SoundManager::playSound(EntityIndex entity, Common::String filename, FlagType flag, byte a4) {
	if (isBuffered(entity) && entity)
		removeFromQueue(entity);

	FlagType currentFlag = (flag == -1) ? getSoundFlag(entity) : (FlagType)(flag | 0x80000);

	// Add .SND at the end of the filename if needed
	if (!filename.contains('.'))
		filename += ".SND";

	if (!playSoundWithSubtitles(filename, currentFlag, entity, a4))
		if (entity)
			getSavePoints()->push(kEntityPlayer, entity, kActionEndSound);
}

bool SoundManager::playSoundWithSubtitles(Common::String filename, FlagType flag, EntityIndex entity, byte a4) {
	SoundEntry *entry = new SoundEntry();

	Common::StackLock locker(_mutex);

	setupEntry(entry, filename, flag, 30);
	entry->entity = entity;

	if (a4) {
		entry->field_48 = _data2 + 2 * a4;
		entry->status.status |= kSoundStatus_8000;
	} else {
		// Get subtitles name
		while (filename.size() > 4)
			filename.deleteLastChar();

		showSubtitle(entry, filename);
		updateEntryState(entry);
	}

	return (entry->type != kSoundTypeNone);
}

void SoundManager::playSoundEvent(EntityIndex entity, byte action, byte a3) {
	int values[5];

	if (getEntityData(entity)->car != getEntityData(kEntityPlayer)->car)
		return;

	if (getEntities()->isInSalon(entity) != getEntities()->isInSalon(kEntityPlayer))
		return;

	int _action = (int)action;
	FlagType flag = getSoundFlag(entity);

	switch (action) {
	case 36: {
		int _param3 = (flag <= 9) ? flag + 7 : 16;

		if (_param3 > 7) {
			_data0 = (uint)_param3;
			_data1 = _data2 + 2 * a3;
		}
		break;
		}

	case 37:
		_data0 = 7;
		_data1 = _data2 + 2 * a3;
		break;

	case 150:
	case 156:
	case 162:
	case 168:
	case 188:
	case 198:
		_action += 1 + (int)rnd(5);
		break;

	case 174:
	case 184:
	case 194:
		_action += 1 + (int)rnd(3);
		break;

	case 180:
		_action += 1 + (int)rnd(4);
		break;

	case 246:
		values[0] = 0;
		values[1] = 104;
		values[2] = 105;
		values[3] = 106;
		values[4] = 116;
		_action = values[rnd(5)];
		break;

	case 247:
		values[0] = 11;
		values[1] = 123;
		values[2] = 124;
		_action = values[rnd(3)];
		break;

	case 248:
		values[0] = 0;
		values[1] = 103;
		values[2] = 108;
		values[3] = 109;
		_action = values[rnd(4)];
		break;

	case 249:
		values[0] = 0;
		values[1] = 56;
		values[2] = 112;
		values[3] = 113;
		_action = values[rnd(4)];
		break;

	case 250:
		values[0] = 0;
		values[1] = 107;
		values[2] = 115;
		values[3] = 117;
		_action = values[rnd(4)];
		break;

	case 251:
		values[0] = 0;
		values[1] = 11;
		values[2] = 56;
		values[3] = 113;
		_action = values[rnd(4)];
		break;

	case 252:
		values[0] = 0;
		values[1] = 6;
		values[2] = 109;
		values[3] = 121;
		_action = values[rnd(4)];
		break;

	case 254:
		values[0] = 0;
		values[1] = 104;
		values[2] = 120;
		values[3] = 121;
		_action = values[rnd(4)];
		break;

	case 255:
		values[0] = 0;
		values[1] = 106;
		values[2] = 115;
		_action = values[rnd(3)];
		break;

	default:
		break;
	}

	if (_action && flag)
		playSoundWithSubtitles(Common::String::format("LIB%03d.SND", _action), flag, kEntityPlayer, a3);
}

void SoundManager::playSteam(CityIndex index) {
	if (index >= ARRAYSIZE(cities))
		error("SoundManager::playSteam: invalid city index (was %d, max %d)", index, ARRAYSIZE(cities));

	_state |= kSoundState2;

	if (!getEntry(kSoundType1))
		playSoundWithSubtitles("STEAM.SND", kFlagSteam, kEntitySteam);

	// Get the new sound entry and show subtitles
	SoundEntry *entry = getEntry(kSoundType1);
	if (entry)
		showSubtitle(entry, cities[index]);
}

void SoundManager::playFightSound(byte action, byte a4) {
	int _action = (int)action;
	int values[5];

	switch (action) {
	default:
		break;

	case 174:
	case 184:
	case 194:
		values[0] = action + 1;
		values[1] = action + 2;
		values[2] = action + 3;
		_action = values[rnd(3)];
		break;

	case 180:
		values[0] = action + 1;
		values[1] = action + 2;
		values[2] = action + 3;
		values[3] = action + 4;
		_action = values[rnd(4)];
		break;

	case 150:
	case 156:
	case 162:
	case 168:
	case 188:
	case 198:
		values[0] = action + 1;
		values[1] = action + 2;
		values[2] = action + 3;
		values[3] = action + 4;
		values[4] = action + 5;
		_action = values[rnd(5)];
		break;
	}

	if (_action)
		playSound(kEntityTrain, Common::String::format("LIB%03d.SND", _action), kFlagDefault, a4);
}

void SoundManager::playDialog(EntityIndex entity, EntityIndex entityDialog, FlagType flag, byte a4) {
	if (isBuffered(getDialogName(entityDialog)))
		removeFromQueue(getDialogName(entityDialog));

	playSound(entity, getDialogName(entityDialog), flag, a4);
}

void SoundManager::playLocomotiveSound() {
	playSound(kEntityPlayer, locomotiveSounds[rnd(5)], (FlagType)(rnd(15) + 2));
}

const char *SoundManager::getDialogName(EntityIndex entity) const {
	switch (entity) {
	case kEntityAnna:
		if (getEvent(kEventAnnaDialogGoToJerusalem))
			return "XANN12";

		if (getEvent(kEventLocomotiveRestartTrain))
			return "XANN11";

		if (getEvent(kEventAnnaBaggageTies) || getEvent(kEventAnnaBaggageTies2) || getEvent(kEventAnnaBaggageTies3) || getEvent(kEventAnnaBaggageTies4))
			return "XANN10";

		if (getEvent(kEventAnnaTired) || getEvent(kEventAnnaTiredKiss))
			return "XANN9";

		if (getEvent(kEventAnnaBaggageArgument))
			return "XANN8";

		if (getEvent(kEventKronosVisit))
			return "XANN7";

		if (getEvent(kEventAbbotIntroduction))
			return "XANN6A";

		if (getEvent(kEventVassiliSeizure))
			return "XANN6";

		if (getEvent(kEventAugustPresentAnna) || getEvent(kEventAugustPresentAnnaFirstIntroduction))
			return "XANN5";

		if (getProgress().field_60)
			return "XANN4";

		if (getEvent(kEventAnnaGiveScarf) || getEvent(kEventAnnaGiveScarfDiner) || getEvent(kEventAnnaGiveScarfSalon)
		 || getEvent(kEventAnnaGiveScarfMonogram) || getEvent(kEventAnnaGiveScarfDinerMonogram) || getEvent(kEventAnnaGiveScarfSalonMonogram))
			return "XANN3";

		if (getEvent(kEventDinerMindJoin))
			return "XANN2";

		if (getEvent(kEventGotALight) || getEvent(kEventGotALightD))
			return "XANN1";

		break;

	case kEntityAugust:
		if (getEvent(kEventAugustTalkCigar))
			return "XAUG6";

		if (getEvent(kEventAugustBringBriefcase))
			return "XAUG5";

		// Getting closer to Vienna...
		if (getState()->time > kTime2200500 && !getEvent(kEventAugustMerchandise))
			return "XAUG4A";

		if (getEvent(kEventAugustMerchandise))
			return "XAUG4";

		if (getEvent(kEventDinerAugust) || getEvent(kEventDinerAugustAlexeiBackground) || getEvent(kEventMeetAugustTylerCompartment)
		 || getEvent(kEventMeetAugustTylerCompartmentBed) || getEvent(kEventMeetAugustHisCompartment) || getEvent(kEventMeetAugustHisCompartmentBed))
			return "XAUG3";

		if (getEvent(kEventAugustPresentAnnaFirstIntroduction))
			return "XAUG2";

		if (getProgress().eventMertensAugustWaiting)
			return "XAUG1";

		break;

	case kEntityTatiana:
		if (getEvent(kEventTatianaTylerCompartment))
			return "XTAT6";

		if (getEvent(kEventTatianaCompartmentStealEgg))
			return "XTAT5";

		if (getEvent(kEventTatianaGivePoem))
			return "XTAT3";

		if (getProgress().field_64)
			return "XTAT1";

		break;

	case kEntityVassili:
		if (getEvent(kEventCathFreePassengers))
			return "XVAS4";

		if (getEvent(kEventVassiliCompartmentStealEgg))
			return "XVAS3";

		if (getEvent(kEventAbbotIntroduction))
			return "XVAS2";

		if (getEvent(kEventVassiliSeizure))
			return "XVAS1A";

		if (getProgress().field_64)
			return "XVAS1";

		break;

	case kEntityAlexei:
		if (getProgress().field_88)
			return "XALX6";

		if (getProgress().field_8C)
			return "XALX5";

		if (getProgress().field_90)
			return "XALX4A";

		if (getProgress().field_68)
			return "XALX4";

		if (getEvent(kEventAlexeiSalonPoem))
			return "XALX3";

		if (getEvent(kEventAlexeiSalonVassili))
			return "XALX2";

		if (getEvent(kEventAlexeiDiner) || getEvent(kEventAlexeiDinerOriginalJacket))
			return "XALX1";

		break;

	case kEntityAbbot:
		if (getEvent(kEventAbbotDrinkDefuse))
			return "XABB4";

		if (getEvent(kEventAbbotInvitationDrink) || getEvent(kEventDefuseBomb))
			return "XABB3";

		if (getEvent(kEventAbbotWrongCompartment) || getEvent(kEventAbbotWrongCompartmentBed))
			return "XABB2";

		if (getEvent(kEventAbbotIntroduction))
			return "XABB1";

		break;

	case kEntityMilos:
		if (getEvent(kEventLocomotiveMilosDay) || getEvent(kEventLocomotiveMilosNight))
			return "XMIL5";

		if (getEvent(kEventMilosCompartmentVisitTyler) && (getProgress().chapter == kChapter3 || getProgress().chapter == kChapter4))
			return "XMIL4";

		if (getEvent(kEventMilosCorridorThanks) || getProgress().chapter == kChapter5)
			return "XMIL3";

		if (getEvent(kEventMilosCompartmentVisitAugust))
			return "XMIL2";

		if (getEvent(kEventMilosTylerCompartmentDefeat))
			return "XMIL1";

		break;

	case kEntityVesna:
		if (getProgress().field_94)
			return "XVES2";

		if (getProgress().field_98)
			return "XVES1";

		break;

	case kEntityKronos:
		if (getEvent(kEventKronosReturnBriefcase))
			return "XKRO6";

		if (getEvent(kEventKronosBringEggCeiling) || getEvent(kEventKronosBringEgg))
			return "XKRO5";

		if (getEvent(kEventKronosConversation) || getEvent(kEventKronosConversationFirebird)) {
			ObjectLocation location = getInventory()->get(kItemFirebird)->location;
			if (location != kObjectLocation6 && location != kObjectLocation5 && location != kObjectLocation2 && location != kObjectLocation1)
				return "XKRO4A";
		}

		if (getEvent(kEventKronosConversationFirebird))
			return "XKRO4";

		if (getEvent(kEventKronosConversation)) {
			if (!getEvent(kEventMilosCompartmentVisitAugust))
				return "XKRO3";
			else
				return "XKRO2";
		}

		if (getProgress().eventMertensKronosInvitation)
			return "XKRO1";

		break;

	case kEntityFrancois:
		if (getProgress().field_9C)
			return "XFRA3";

		if (getProgress().field_A0
		 || getEvent(kEventFrancoisWhistle) || getEvent(kEventFrancoisWhistleD)
		 || getEvent(kEventFrancoisWhistleNight) || getEvent(kEventFrancoisWhistleNightD))
			return "XFRA2";

		if (getState()->time > kTimeParisEpernay) // Between Paris and Epernay
			return "XFRA1";

		break;

	case kEntityMmeBoutarel:
		if (getProgress().field_A4)
			return "XMME4";

		if (getProgress().field_A8)
			return "XMME3";

		if (getProgress().field_A0)
			return "XMME2";

		if (getProgress().field_AC)
			return "XMME1";

		break;

	case kEntityBoutarel:
		if (getProgress().eventMetBoutarel)
			return "XMRB1";

		break;

	case kEntityRebecca:
		if (getProgress().field_B4)
			return "XREB1A";

		if (getProgress().field_B8)
			return "XREB1";

		break;

	case kEntitySophie:
		if (getProgress().field_B0)
			return "XSOP2";

		if (getProgress().field_BC)
			return "XSOP1B";

		if (getProgress().field_B4)
			return "XSOP1A";

		if (getProgress().field_B8)
			return "XSOP1";

		break;

	case kEntityMahmud:
		if (getProgress().field_C4)
			return "XMAH1";

		break;

	case kEntityYasmin:
		if (getProgress().eventMetYasmin)
			return "XHAR2";

		break;

	case kEntityHadija:
		if (getProgress().eventMetHadija)
			return "XHAR1";

		break;

	case kEntityAlouan:
		if (getProgress().field_DC)
			return "XHAR3";

		break;

	case kEntityGendarmes:
		if (getProgress().field_E0)
			return "XHAR4";

		break;

	case kEntityChapters:
		if (getEvent(kEventCathDream) || getEvent(kEventCathWakingUp))
			return "XTYL3";

		return "XTYL1";

	default:
		break;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// Letters & Messages
//////////////////////////////////////////////////////////////////////////
void SoundManager::readText(int id){
	if (!isBuffered(kEntityTables4))
		return;

	if (id < 0 || (id > 8 && id < 50) || id > 64)
		error("Sound::readText - attempting to use invalid id. Valid values [1;8] - [50;64], was %d", id);

	// Get proper message file (names are stored in sequence in the array but id is [1;8] - [50;64])
	const char *text = messages[id <= 8 ? id : id - 41];

	// Check if file is in cache for id [1;8]
	if (id <= 8)
		if (isBuffered(text))
			removeFromQueue(text);

	playSound(kEntityTables4, text, kFlagDefault);
}

//////////////////////////////////////////////////////////////////////////
// Sound bites
//////////////////////////////////////////////////////////////////////////
void SoundManager::playWarningCompartment(EntityIndex entity, ObjectIndex compartment) {

#define PLAY_WARNING(index, sound1, sound2, sound3, sound4, sound5, sound6) { \
	if (_lastWarning[index] + 450 >= getState()->timeTicks) { \
		if (rnd(2)) \
			playSound(kEntityMertens, sound1, kFlagDefault); \
		else \
			playSound(kEntityMertens, rnd(2) ? sound2 : sound3, kFlagDefault); \
	} else { \
		if (rnd(2)) \
			playSound(kEntityMertens, sound4, kFlagDefault); \
		else \
			playSound(kEntityMertens, rnd(2) ? sound5 : sound6, kFlagDefault); \
	} \
	_lastWarning[index] = getState()->timeTicks; \
}

	if (entity != kEntityMertens && entity != kEntityCoudert)
		return;

	//////////////////////////////////////////////////////////////////////////
	// Mertens
	if (entity == kEntityMertens) {

		switch (compartment) {
		default:
			break;

		case kObjectCompartment2:
			PLAY_WARNING(0, "Con1502A", "Con1500B", "Con1500C", "Con1502", "Con1500", "Con1500A");
			break;

		case kObjectCompartment3:
			PLAY_WARNING(1, "Con1501A", "Con1500B", "Con1500C", "Con1501", "Con1500", "Con1500A");
			break;

		case kObjectCompartment4:
			PLAY_WARNING(2, "Con1503", "Con1500B", "Con1500C", "Con1503", "Con1500", "Con1500A");
			break;

		case kObjectCompartment5:
		case kObjectCompartment6:
		case kObjectCompartment7:
		case kObjectCompartment8:
			++_lastWarning[3];

			switch (_lastWarning[3]) {
			default:
				break;

			case 1:
				getSound()->playSound(kEntityMertens, "Con1503C", kFlagDefault);
				break;

			case 2:
				getSound()->playSound(kEntityMertens, rnd(2) ? "Con1503E" : "Con1503A", kFlagDefault);
				break;

			case 3:
				getSound()->playSound(kEntityMertens, rnd(2) ? "Con1503B" : "Con1503D", kFlagDefault);
				_lastWarning[3] = 0;
				break;
			}
		}

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Coudert
	switch (compartment) {
		default:
			break;

		case kObjectCompartmentA:
			if (_lastWarning[4] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1500" : "Jac1500A", kFlagDefault);
				break;
			}

			getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1508" : "Jac1508A", kFlagDefault);
			break;

		case kObjectCompartmentB:
			if (_lastWarning[5] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1500" : "Jac1500A", kFlagDefault);
				break;
			}

			if (getProgress().field_40 || (getState()->time > kTimeCityLinz && getState()->time < kTime2133000))
				getSound()->playSound(kEntityCoudert, "Jac1507A", kFlagDefault);
			else
				getSound()->playSound(kEntityCoudert, "Jac1507", kFlagDefault);
			break;

		case kObjectCompartmentC:
			if (_lastWarning[6] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1500" : "Jac1500A", kFlagDefault);
				break;
			}

			if (getProgress().chapter < kChapter3)
				getSound()->playSound(kEntityCoudert, "Jac1506", kFlagDefault);
			else
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1506A" : "Jac1506B", kFlagDefault);
			break;

		case kObjectCompartmentD:
			if (_lastWarning[7] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1500" : "Jac1500A", kFlagDefault);
				break;
			}

			getSound()->playSound(kEntityCoudert, "Jac1505", kFlagDefault);
			break;

		case kObjectCompartmentE:
			if (_lastWarning[8] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1500" : "Jac1500A", kFlagDefault);
				break;
			}

			if (getProgress().field_40 || (getState()->time > kTime2115000 && getState()->time < kTime2133000)) {
				getSound()->playSound(kEntityCoudert, "Jac1504B", kFlagDefault);
				break;
			}

			if (getEntities()->isInsideCompartment(kEntityRebecca, kCarRedSleeping, kPosition_4840))
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1500" : "Jac1500A", kFlagDefault);
			else
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1504" : "Jac1504A", kFlagDefault);
			break;

		case kObjectCompartmentF:
			if (_lastWarning[9] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1500" : "Jac1500A", kFlagDefault);
				break;
			}

			if (getProgress().field_40 || (getState()->time > kTime2083500 && getState()->time < kTime2133000)) {
				getSound()->playSound(kEntityCoudert, "Jac1503B", kFlagDefault);
				break;
			}

			if (rnd(2) || getEntities()->isInsideCompartment(kEntityAnna, kCarRedSleeping, kPosition_4070))
				getSound()->playSound(kEntityCoudert, "Jac1503", kFlagDefault);
			else
				getSound()->playSound(kEntityCoudert, "Jac1503A", kFlagDefault);
			break;

		case kObjectCompartmentG:
			if (_lastWarning[10] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1500" : "Jac1500A", kFlagDefault);
				break;
			}

			if (rnd(2) || getEntities()->isInsideCompartment(kEntityMilos, kCarRedSleeping, kPosition_3050))
				getSound()->playSound(kEntityCoudert, "Jac1502", kFlagDefault);
			else
				getSound()->playSound(kEntityCoudert, "Jac1502A", kFlagDefault);
			break;

		case kObjectCompartmentH:
			if (_lastWarning[11] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1500" : "Jac1500A", kFlagDefault);
				break;
			}

			if (getEntities()->isInsideCompartment(kEntityIvo, kCarRedSleeping, kPosition_2740))
				getSound()->playSound(kEntityCoudert, rnd(2) ? "Jac1500" : "Jac1500A", kFlagDefault);
			else
				getSound()->playSound(kEntityCoudert, "Jac1501", kFlagDefault);
			break;
	}

	// Update ticks (Compartments A - H are indexes 4 - 11)
	_lastWarning[compartment - 28] = getState()->timeTicks;
}

void SoundManager::excuseMe(EntityIndex entity, EntityIndex entity2, FlagType flag) {
	if (isBuffered(entity) && entity != kEntityPlayer && entity != kEntityChapters && entity != kEntityTrain)
		return;

	if (entity2 == kEntityFrancois || entity2 == kEntityMax)
		return;

	if (entity == kEntityFrancois && getEntityData(kEntityFrancois)->field_4A3 != 30)
		return;

	if (flag == kFlagNone)
		flag = getSoundFlag(entity);

	switch (entity) {
	default:
		break;

	case kEntityAnna:
		playSound(kEntityPlayer, "ANN1107A", flag);
		break;

	case kEntityAugust:
		switch(rnd(4)) {
		default:
			break;

		case 0:
			playSound(kEntityPlayer, "AUG1100A", flag);
			break;

		case 1:
			playSound(kEntityPlayer, "AUG1100B", flag);
			break;

		case 2:
			playSound(kEntityPlayer, "AUG1100C", flag);
			break;

		case 3:
			playSound(kEntityPlayer, "AUG1100D", flag);
			break;
		}
		break;

	case kEntityMertens:
		if (Entities::isFemale(entity2)) {
			playSound(kEntityPlayer, (rnd(2) ? "CON1111" : "CON1111A"), flag);
		} else {
			if (entity2 || getProgress().jacket != kJacketGreen || !rnd(2)) {
				switch(rnd(3)) {
				default:
					break;

				case 0:
					playSound(kEntityPlayer, "CON1110A", flag);
					break;

				case 1:
					playSound(kEntityPlayer, "CON1110C", flag);
					break;

				case 2:
					playSound(kEntityPlayer, "CON1110", flag);
					break;
				}
			} else {
				if (isNight()) {
					playSound(kEntityPlayer, (getProgress().field_18 == 2 ? "CON1110F" : "CON1110E"));
				} else {
					playSound(kEntityPlayer, "CON1110D");
				}
			}
		}
		break;

	case kEntityCoudert:
		if (Entities::isFemale(entity2)) {
			playSound(kEntityPlayer, "JAC1111D", flag);
		} else {
			if (entity2 || getProgress().jacket != kJacketGreen || !rnd(2)) {
				switch(rnd(4)) {
				default:
					break;

				case 0:
					playSound(kEntityPlayer, "JAC1111", flag);
					break;

				case 1:
					playSound(kEntityPlayer, "JAC1111A", flag);
					break;

				case 2:
					playSound(kEntityPlayer, "JAC1111B", flag);
					break;

				case 3:
					playSound(kEntityPlayer, "JAC1111C", flag);
					break;
				}
			} else {
				playSound(kEntityPlayer, "JAC1113B", flag);
			}
		}
		break;

	case kEntityPascale:
		playSound(kEntityPlayer, (rnd(2) ? "HDE1002" : "HED1002A"), flag);
		break;

	case kEntityServers0:
	case kEntityServers1:
		switch(rnd(3)) {
		default:
			break;

		case 0:
			playSound(kEntityPlayer, (entity == kEntityServers0) ? "WAT1002" : "WAT1003", flag);
			break;

		case 1:
			playSound(kEntityPlayer, (entity == kEntityServers0) ? "WAT1002A" : "WAT1003A", flag);
			break;

		case 2:
			playSound(kEntityPlayer, (entity == kEntityServers0) ? "WAT1002B" : "WAT1003B", flag);
			break;
		}
		break;

	case kEntityVerges:
		if (Entities::isFemale(entity2)) {
			playSound(kEntityPlayer, (rnd(2) ? "TRA1113A" : "TRA1113B"));
		} else {
			playSound(kEntityPlayer, "TRA1112", flag);
		}
		break;

	case kEntityTatiana:
		playSound(kEntityPlayer, (rnd(2) ? "TAT1102A" : "TAT1102B"), flag);
		break;

	case kEntityAlexei:
		playSound(kEntityPlayer, (rnd(2) ? "ALX1099C" : "ALX1099D"), flag);
		break;

	case kEntityAbbot:
		if (Entities::isFemale(entity2)) {
			playSound(kEntityPlayer, "ABB3002C", flag);
		} else {
			switch(rnd(3)) {
			default:
				break;

			case 0:
				playSound(kEntityPlayer, "ABB3002", flag);
				break;

			case 1:
				playSound(kEntityPlayer, "ABB3002A", flag);
				break;

			case 2:
				playSound(kEntityPlayer, "ABB3002B", flag);
				break;
			}
		}
		break;

	case kEntityVesna:
		switch(rnd(3)) {
		default:
			break;

		case 0:
			playSound(kEntityPlayer, "VES1109A", flag);
			break;

		case 1:
			playSound(kEntityPlayer, "VES1109B", flag);
			break;

		case 2:
			playSound(kEntityPlayer, "VES1109C", flag);
			break;
		}
		break;

	case kEntityKahina:
		playSound(kEntityPlayer, (rnd(2) ? "KAH1001" : "KAH1001A"), flag);
		break;

	case kEntityFrancois:
	case kEntityMmeBoutarel:
		switch(rnd(4)) {
		default:
			break;

		case 0:
			playSound(kEntityPlayer, (entity == kEntityFrancois) ? "FRA1001" : "MME1103A", flag);
			break;

		case 1:
			playSound(kEntityPlayer, (entity == kEntityFrancois) ? "FRA1001A" : "MME1103B", flag);
			break;

		case 2:
			playSound(kEntityPlayer, (entity == kEntityFrancois) ? "FRA1001B" : "MME1103C", flag);
			break;

		case 3:
			playSound(kEntityPlayer, (entity == kEntityFrancois) ? "FRA1001C" : "MME1103D", flag);
			break;
		}
		break;

	case kEntityBoutarel:
		playSound(kEntityPlayer, "MRB1104", flag);
		if (flag > 2)
			getProgress().eventMetBoutarel = true;
		break;

	case kEntityRebecca:
		playSound(kEntityPlayer, (rnd(2) ? "REB1106" : "REB110A"), flag);
		break;

	case kEntitySophie: {
		switch(rnd(3)) {
		default:
			break;

		case 0:
			playSound(kEntityPlayer, "SOP1105", flag);
			break;

		case 1:
			playSound(kEntityPlayer, Entities::isFemale(entity2) ? "SOP1105C" : "SOP1105A", flag);
			break;

		case 2:
			playSound(kEntityPlayer, Entities::isFemale(entity2) ? "SOP1105D" : "SOP1105B", flag);
			break;
		}
		break;
	}

	case kEntityMahmud:
		playSound(kEntityPlayer, "MAH1101", flag);
		break;

	case kEntityYasmin:
		playSound(kEntityPlayer, "HAR1002", flag);
		if (flag > 2)
			getProgress().eventMetYasmin = true;
		break;

	case kEntityHadija:
		playSound(kEntityPlayer, (rnd(2) ? "HAR1001" : "HAR1001A"), flag);
		if (flag > 2)
			getProgress().eventMetHadija = true;
		break;

	case kEntityAlouan:
		playSound(kEntityPlayer, "HAR1004", flag);
		break;
	}
}

void SoundManager::excuseMeCath() {
	switch(rnd(3)) {
	default:
		playSound(kEntityPlayer, "CAT1126B");
		break;

	case 1:
		playSound(kEntityPlayer, "CAT1126C");
		break;

	case 2:
		playSound(kEntityPlayer, "CAT1126D");
		break;
	}
}

const char *SoundManager::justCheckingCath() const {
	switch(rnd(4)) {
	default:
		break;

	case 0:
		return "CAT5001";

	case 1:
		return "CAT5001A";

	case 2:
		return "CAT5001B";

	case 3:
		return "CAT5001C";
	}

	return "CAT5001";
}

const char *SoundManager::wrongDoorCath() const {
	switch(rnd(5)) {
	default:
		break;

	case 0:
		return "CAT1125";

	case 1:
		return "CAT1125A";

	case 2:
		return "CAT1125B";

	case 3:
		return "CAT1125C";

	case 4:
		return "CAT1125D";
	}

	return "CAT1125";
}

const char *SoundManager::justAMinuteCath() const {
	switch(rnd(3)) {
	default:
		break;

	case 0:
		return "CAT1520";

	case 1:
		return "CAT1521";

	case 2:
		return "CAT1125";    // ?? is this a bug in the original?
	}

	return "CAT1520";
}

//////////////////////////////////////////////////////////////////////////
// Sound flags
//////////////////////////////////////////////////////////////////////////
SoundManager::FlagType SoundManager::getSoundFlag(EntityIndex entity) const {
	if (entity == kEntityPlayer)
		return kFlagDefault;

	if (getEntityData(entity)->car != getEntityData(kEntityPlayer)->car)
		return kFlagNone;

	// Compute sound value
	FlagType ret = kFlag2;

	// Get default value if valid
	int index = ABS(getEntityData(entity)->entityPosition - getEntityData(kEntityPlayer)->entityPosition) / 230;
	if (index < 32)
		ret = soundFlags[index];

	if (getEntityData(entity)->location == kLocationOutsideTrain) {
		if (getEntityData(entity)->car != kCarKronos
		&& !getEntities()->isOutsideAlexeiWindow()
		&& !getEntities()->isOutsideAnnaWindow())
			return kFlagNone;

		return (FlagType)(ret / 6);
	}

	switch (getEntityData(entity)->car) {
	default:
		break;

	case kCarKronos:
		if (getEntities()->isInKronosSalon(entity) != getEntities()->isInKronosSalon(kEntityPlayer))
			ret = (FlagType)(ret * 2);
		break;

	case kCarGreenSleeping:
	case kCarRedSleeping:
		if (getEntities()->isInGreenCarEntrance(kEntityPlayer) && !getEntities()->isInKronosSalon(entity))
			ret = (FlagType)(ret * 2);

		if (getEntityData(kEntityPlayer)->location
		&& (getEntityData(entity)->entityPosition != kPosition_1 || !getEntities()->isDistanceBetweenEntities(kEntityPlayer, entity, 400)))
			ret = (FlagType)(ret * 2);
		break;

	case kCarRestaurant:
		if (getEntities()->isInSalon(entity) == getEntities()->isInSalon(kEntityPlayer)
		&& (getEntities()->isInRestaurant(entity) != getEntities()->isInRestaurant(kEntityPlayer)))
			ret = (FlagType)(ret * 2);
		else
			ret = (FlagType)(ret * 4);
		break;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
// Subtitles
//////////////////////////////////////////////////////////////////////////
void SoundManager::updateSubtitles() {
	Common::StackLock locker(_mutex);

	uint32 index = 0;
	SubtitleEntry *subtitle = NULL;

	for (Common::List<SubtitleEntry *>::iterator i = _subtitles.begin(); i != _subtitles.end(); ++i) {
		uint32 current_index = 0;
		SoundEntry *soundEntry = (*i)->sound;
		SoundStatus status = (SoundStatus)soundEntry->status.status;

		if (!(status & kSoundStatus_40)
		 || status & 0x180
		 || soundEntry->time == 0
		 || (status & 0x1F) < 6
		 || ((getFlags()->nis & 0x8000) && soundEntry->field_4C < 90)) {
			 current_index = 0;
		} else {
			current_index = soundEntry->field_4C + (status & 0x1F);

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
			setupSubtitleAndDraw(subtitle);

		return;
	}

	if (_drawSubtitles & 1)
		drawSubtitleOnScreen(subtitle);

	if (subtitle) {
		loadSubtitleData(subtitle);
		setupSubtitleAndDraw(subtitle);
	}
}

void SoundManager::showSubtitle(SoundEntry *entry, Common::String filename) {
	entry->subtitle = loadSubtitle(filename, entry);

	if (entry->subtitle->status.status2 & 4) {
		drawSubtitle(entry->subtitle);
		SAFE_DELETE(entry->subtitle);
	} else {
		entry->status.status |= kSoundStatus_20000;
	}
}

SoundManager::SubtitleEntry *SoundManager::loadSubtitle(Common::String filename, SoundEntry *soundEntry) {
	SubtitleEntry *entry = new SubtitleEntry();
	_subtitles.push_back(entry);

	// Set sound entry and filename
	entry->filename = filename + ".SBE";
	entry->sound = soundEntry;

	// Load subtitle data
	if (_engine->getResourceManager()->hasFile(filename)) {
		if (_drawSubtitles & 2)
			return entry;

		loadSubtitleData(entry);
	} else {
		entry->status.status = kSoundStatus_400;
	}

	return entry;
}

void SoundManager::loadSubtitleData(SubtitleEntry * entry) {
	entry->data = new SubtitleManager(_engine->getFont());
	entry->data->load(getArchive(entry->filename));

	_drawSubtitles |= 2;
	_currentSubtitle = entry;
}

void SoundManager::setupSubtitleAndDraw(SubtitleEntry *subtitle) {
	if (!subtitle->data) {
		subtitle->data = new SubtitleManager(_engine->getFont());
		subtitle->data->load(getArchive(subtitle->filename));
	}

	if (subtitle->data->getMaxTime() > subtitle->sound->time) {
		subtitle->status.status = kSoundStatus_400;
	} else {
		subtitle->data->setTime((uint16)subtitle->sound->time);

		if (_drawSubtitles & 1)
			drawSubtitleOnScreen(subtitle);
	}

	_currentSubtitle = subtitle;
}

void SoundManager::drawSubtitle(SubtitleEntry *subtitle) {
	// Remove subtitle from queue
	_subtitles.remove(subtitle);

	if (subtitle == _currentSubtitle) {
		drawSubtitleOnScreen(subtitle);

		_currentSubtitle = NULL;
		_drawSubtitles = 0;
	}
}

void SoundManager::drawSubtitleOnScreen(SubtitleEntry *subtitle) {
	if (!subtitle)
		error("SoundManager::drawSubtitleOnScreen: Invalid subtitle entry!");

	_drawSubtitles &= ~1;

	if (subtitle->data == NULL)
		return;

	if (_drawSubtitles & 1)
		_engine->getGraphicsManager()->draw(subtitle->data, GraphicsManager::kBackgroundOverlay);
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
void SoundManager::playLoopingSound() {
	warning("SoundManager::playLoopingSound: not implemented!");
}

void SoundManager::stopAllSound() {
	Common::StackLock locker(_mutex);

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		(*i)->soundStream->stop();
}

//////////////////////////////////////////////////////////////////////////
// Sound filter
//////////////////////////////////////////////////////////////////////////

static int filterData[1424] = {
	0, 0, 0, 0, 128, 256, 384, 512, 0, 0, 0, 0, 128, 256,
	384, 512, 0, 0, 0, 0, 192, 320, 448, 576, 0, 0, 0, 0,
	192, 320, 448, 576, 64, 64, 64, 64, 256, 384, 512, 640,
	64, 64, 64, 64, 256, 384, 512, 640, 128, 128, 128, 128,
	320, 448, 576, 704, 128, 128, 128, 128, 320, 448, 576,
	704, 192, 192, 192, 192, 384, 512, 640, 768, 192, 192,
	192, 192, 384, 512, 640, 768, 256, 256, 256, 256, 448,
	576, 704, 832, 256, 256, 256, 256, 448, 576, 704, 832,
	320, 320, 320, 320, 512, 640, 768, 896, 320, 320, 320,
	320, 512, 640, 768, 896, 384, 384, 384, 384, 576, 704,
	832, 960, 384, 384, 384, 384, 576, 704, 832, 960, 448,
	448, 448, 448, 640, 768, 896, 1024, 448, 448, 448, 448,
	640, 768, 896, 1024, 512, 512, 512, 512, 704, 832, 960,
	1088, 512, 512, 512, 512, 704, 832, 960, 1088, 576,
	576, 576, 576, 768, 896, 1024, 1152, 576, 576, 576,
	576, 768, 896, 1024, 1152, 640, 640, 640, 640, 832,
	960, 1088, 1216, 640, 640, 640, 640, 832, 960, 1088,
	1216, 704, 704, 704, 704, 896, 1024, 1152, 1280, 704,
	704, 704, 704, 896, 1024, 1152, 1280, 768, 768, 768,
	768, 960, 1088, 1216, 1344, 768, 768, 768, 768, 960,
	1088, 1216, 1344, 832, 832, 832, 832, 1024, 1152, 1280,
	1408, 832, 832, 832, 832, 1024, 1152, 1280, 1408, 896,
	896, 896, 896, 1088, 1216, 1344, 1472, 896, 896, 896,
	896, 1088, 1216, 1344, 1472, 960, 960, 960, 960, 1152,
	1280, 1408, 1536, 960, 960, 960, 960, 1152, 1280, 1408,
	1536, 1024, 1024, 1024, 1024, 1216, 1344, 1472, 1600,
	1024, 1024, 1024, 1024, 1216, 1344, 1472, 1600, 1088,
	1088, 1088, 1088, 1280, 1408, 1536, 1664, 1088, 1088,
	1088, 1088, 1280, 1408, 1536, 1664, 1152, 1152, 1152,
	1152, 1344, 1472, 1600, 1728, 1152, 1152, 1152, 1152,
	1344, 1472, 1600, 1728, 1216, 1216, 1216, 1216, 1408,
	1536, 1664, 1792, 1216, 1216, 1216, 1216, 1408, 1536,
	1664, 1792, 1280, 1280, 1280, 1280, 1472, 1600, 1728,
	1856, 1280, 1280, 1280, 1280, 1472, 1600, 1728, 1856,
	1344, 1344, 1344, 1344, 1536, 1664, 1792, 1920, 1344,
	1344, 1344, 1344, 1536, 1664, 1792, 1920, 1408, 1408,
	1408, 1408, 1600, 1728, 1856, 1984, 1408, 1408, 1408,
	1408, 1600, 1728, 1856, 1984, 1472, 1472, 1472, 1472,
	1664, 1792, 1920, 2048, 1472, 1472, 1472, 1472, 1664,
	1792, 1920, 2048, 1536, 1536, 1536, 1536, 1728, 1856,
	1984, 2112, 1536, 1536, 1536, 1536, 1728, 1856, 1984,
	2112, 1600, 1600, 1600, 1600, 1792, 1920, 2048, 2176,
	1600, 1600, 1600, 1600, 1792, 1920, 2048, 2176, 1664,
	1664, 1664, 1664, 1856, 1984, 2112, 2240, 1664, 1664,
	1664, 1664, 1856, 1984, 2112, 2240, 1728, 1728, 1728,
	1728, 1920, 2048, 2176, 2304, 1728, 1728, 1728, 1728,
	1920, 2048, 2176, 2304, 1792, 1792, 1792, 1792, 1984,
	2112, 2240, 2368, 1792, 1792, 1792, 1792, 1984, 2112,
	2240, 2368, 1856, 1856, 1856, 1856, 2048, 2176, 2304,
	2432, 1856, 1856, 1856, 1856, 2048, 2176, 2304, 2432,
	1920, 1920, 1920, 1920, 2112, 2240, 2368, 2496, 1920,
	1920, 1920, 1920, 2112, 2240, 2368, 2496, 1984, 1984,
	1984, 1984, 2176, 2304, 2432, 2560, 1984, 1984, 1984,
	1984, 2176, 2304, 2432, 2560, 2048, 2048, 2048, 2048,
	2240, 2368, 2496, 2624, 2048, 2048, 2048, 2048, 2240,
	2368, 2496, 2624, 2112, 2112, 2112, 2112, 2304, 2432,
	2560, 2688, 2112, 2112, 2112, 2112, 2304, 2432, 2560,
	2688, 2176, 2176, 2176, 2176, 2368, 2496, 2624, 2752,
	2176, 2176, 2176, 2176, 2368, 2496, 2624, 2752, 2240,
	2240, 2240, 2240, 2432, 2560, 2688, 2816, 2240, 2240,
	2240, 2240, 2432, 2560, 2688, 2816, 2304, 2304, 2304,
	2304, 2496, 2624, 2752, 2880, 2304, 2304, 2304, 2304,
	2496, 2624, 2752, 2880, 2368, 2368, 2368, 2368, 2560,
	2688, 2816, 2944, 2368, 2368, 2368, 2368, 2560, 2688,
	2816, 2944, 2432, 2432, 2432, 2432, 2624, 2752, 2880,
	3008, 2432, 2432, 2432, 2432, 2624, 2752, 2880, 3008,
	2496, 2496, 2496, 2496, 2688, 2816, 2944, 3072, 2496,
	2496, 2496, 2496, 2688, 2816, 2944, 3072, 2560, 2560,
	2560, 2560, 2752, 2880, 3008, 3136, 2560, 2560, 2560,
	2560, 2752, 2880, 3008, 3136, 2624, 2624, 2624, 2624,
	2816, 2944, 3072, 3200, 2624, 2624, 2624, 2624, 2816,
	2944, 3072, 3200, 2688, 2688, 2688, 2688, 2880, 3008,
	3136, 3264, 2688, 2688, 2688, 2688, 2880, 3008, 3136,
	3264, 2752, 2752, 2752, 2752, 2944, 3072, 3200, 3328,
	2752, 2752, 2752, 2752, 2944, 3072, 3200, 3328, 2816,
	2816, 2816, 2816, 3008, 3136, 3264, 3392, 2816, 2816,
	2816, 2816, 3008, 3136, 3264, 3392, 2880, 2880, 2880,
	2880, 3072, 3200, 3328, 3456, 2880, 2880, 2880, 2880,
	3072, 3200, 3328, 3456, 2944, 2944, 2944, 2944, 3136,
	3264, 3392, 3520, 2944, 2944, 2944, 2944, 3136, 3264,
	3392, 3520, 3008, 3008, 3008, 3008, 3200, 3328, 3456,
	3584, 3008, 3008, 3008, 3008, 3200, 3328, 3456, 3584,
	3072, 3072, 3072, 3072, 3264, 3392, 3520, 3648, 3072,
	3072, 3072, 3072, 3264, 3392, 3520, 3648, 3136, 3136,
	3136, 3136, 3328, 3456, 3584, 3712, 3136, 3136, 3136,
	3136, 3328, 3456, 3584, 3712, 3200, 3200, 3200, 3200,
	3392, 3520, 3648, 3776, 3200, 3200, 3200, 3200, 3392,
	3520, 3648, 3776, 3264, 3264, 3264, 3264, 3456, 3584,
	3712, 3840, 3264, 3264, 3264, 3264, 3456, 3584, 3712,
	3840, 3328, 3328, 3328, 3328, 3520, 3648, 3776, 3904,
	3328, 3328, 3328, 3328, 3520, 3648, 3776, 3904, 3392,
	3392, 3392, 3392, 3584, 3712, 3840, 3968, 3392, 3392,
	3392, 3392, 3584, 3712, 3840, 3968, 3456, 3456, 3456,
	3456, 3648, 3776, 3904, 4032, 3456, 3456, 3456, 3456,
	3648, 3776, 3904, 4032, 3520, 3520, 3520, 3520, 3712,
	3840, 3968, 4096, 3520, 3520, 3520, 3520, 3712, 3840,
	3968, 4096, 3584, 3584, 3584, 3584, 3776, 3904, 4032,
	4160, 3584, 3584, 3584, 3584, 3776, 3904, 4032, 4160,
	3648, 3648, 3648, 3648, 3840, 3968, 4096, 4224, 3648,
	3648, 3648, 3648, 3840, 3968, 4096, 4224, 3712, 3712,
	3712, 3712, 3904, 4032, 4160, 4288, 3712, 3712, 3712,
	3712, 3904, 4032, 4160, 4288, 3776, 3776, 3776, 3776,
	3968, 4096, 4224, 4352, 3776, 3776, 3776, 3776, 3968,
	4096, 4224, 4352, 3840, 3840, 3840, 3840, 4032, 4160,
	4288, 4416, 3840, 3840, 3840, 3840, 4032, 4160, 4288,
	4416, 3904, 3904, 3904, 3904, 4096, 4224, 4352, 4480,
	3904, 3904, 3904, 3904, 4096, 4224, 4352, 4480, 3968,
	3968, 3968, 3968, 4160, 4288, 4416, 4544, 3968, 3968,
	3968, 3968, 4160, 4288, 4416, 4544, 4032, 4032, 4032,
	4032, 4224, 4352, 4480, 4608, 4032, 4032, 4032, 4032,
	4224, 4352, 4480, 4608, 4096, 4096, 4096, 4096, 4288,
	4416, 4544, 4672, 4096, 4096, 4096, 4096, 4288, 4416,
	4544, 4672, 4160, 4160, 4160, 4160, 4352, 4480, 4608,
	4736, 4160, 4160, 4160, 4160, 4352, 4480, 4608, 4736,
	4224, 4224, 4224, 4224, 4416, 4544, 4672, 4800, 4224,
	4224, 4224, 4224, 4416, 4544, 4672, 4800, 4288, 4288,
	4288, 4288, 4480, 4608, 4736, 4864, 4288, 4288, 4288,
	4288, 4480, 4608, 4736, 4864, 4352, 4352, 4352, 4352,
	4544, 4672, 4800, 4928, 4352, 4352, 4352, 4352, 4544,
	4672, 4800, 4928, 4416, 4416, 4416, 4416, 4608, 4736,
	4864, 4992, 4416, 4416, 4416, 4416, 4608, 4736, 4864,
	4992, 4480, 4480, 4480, 4480, 4672, 4800, 4928, 5056,
	4480, 4480, 4480, 4480, 4672, 4800, 4928, 5056, 4544,
	4544, 4544, 4544, 4736, 4864, 4992, 5120, 4544, 4544,
	4544, 4544, 4736, 4864, 4992, 5120, 4608, 4608, 4608,
	4608, 4800, 4928, 5056, 5184, 4608, 4608, 4608, 4608,
	4800, 4928, 5056, 5184, 4672, 4672, 4672, 4672, 4864,
	4992, 5120, 5248, 4672, 4672, 4672, 4672, 4864, 4992,
	5120, 5248, 4736, 4736, 4736, 4736, 4928, 5056, 5184,
	5312, 4736, 4736, 4736, 4736, 4928, 5056, 5184, 5312,
	4800, 4800, 4800, 4800, 4992, 5120, 5248, 5376, 4800,
	4800, 4800, 4800, 4992, 5120, 5248, 5376, 4864, 4864,
	4864, 4864, 5056, 5184, 5312, 5440, 4864, 4864, 4864,
	4864, 5056, 5184, 5312, 5440, 4928, 4928, 4928, 4928,
	5120, 5248, 5376, 5504, 4928, 4928, 4928, 4928, 5120,
	5248, 5376, 5504, 4992, 4992, 4992, 4992, 5184, 5312,
	5440, 5568, 4992, 4992, 4992, 4992, 5184, 5312, 5440,
	5568, 5056, 5056, 5056, 5056, 5248, 5376, 5504, 5632,
	5056, 5056, 5056, 5056, 5248, 5376, 5504, 5632, 5120,
	5120, 5120, 5120, 5312, 5440, 5568, 5632, 5120, 5120,
	5120, 5120, 5312, 5440, 5568, 5632, 5184, 5184, 5184,
	5184, 5376, 5504, 5632, 5632, 5184, 5184, 5184, 5184,
	5376, 5504, 5632, 5632, 5248, 5248, 5248, 5248, 5440,
	5568, 5632, 5632, 5248, 5248, 5248, 5248, 5440, 5568,
	5632, 5632, 5312, 5312, 5312, 5312, 5504, 5632, 5632,
	5632, 5312, 5312, 5312, 5312, 5504, 5632, 5632, 5632,
	5376, 5376, 5376, 5376, 5568, 5632, 5632, 5632, 5376,
	5376, 5376, 5376, 5568, 5632, 5632, 5632, 5440, 5440,
	5440, 5440, 5632, 5632, 5632, 5632, 5440, 5440, 5440,
	5440, 5632, 5632, 5632, 5632, 5504, 5504, 5504, 5504,
	5632, 5632, 5632, 5632, 5504, 5504, 5504, 5504, 5632,
	5632, 5632, 5632, 5568, 5568, 5568, 5568, 5632, 5632,
	5632, 5632, 5568, 5568, 5568, 5568, 5632, 5632, 5632,
	5632
};

static int filterData2[1424] = {
	0, 2, 4, 6, 7, 9, 11, 13, 0, -2, -4, -6, -7, -9, -11,
	-13, 1, 3, 5, 7, 9, 11, 13, 15, -1, -3, -5, -7, -9,
	-11, -13, -15, 1, 3, 5, 7, 10, 12, 14, 16, -1, -3, -5,
	-7, -10, -12, -14, -16, 1, 3, 6, 8, 11, 13, 16, 18,
	-1, -3, -6, -8, -11, -13, -16, -18, 1, 4, 6, 9, 12,
	15, 17, 20, -1, -4, -6, -9, -12, -15, -17, -20, 1, 4,
	7, 10, 13, 16, 19, 22, -1, -4, -7, -10, -13, -16, -19,
	-22, 1, 4, 8, 11, 14, 17, 21, 24, -1, -4, -8, -11, -14,
	-17, -21, -24, 1, 5, 8, 12, 15, 19, 22, 26, -1, -5,
	-8, -12, -15, -19, -22, -26, 2, 6, 10, 14, 18, 22, 26,
	30, -2, -6, -10, -14, -18, -22, -26, -30, 2, 6, 10,
	14, 19, 23, 27, 31, -2, -6, -10, -14, -19, -23, -27,
	-31, 2, 7, 11, 16, 21, 26, 30, 35, -2, -7, -11, -16,
	-21, -26, -30, -35, 2, 7, 13, 18, 23, 28, 34, 39, -2,
	-7, -13, -18, -23, -28, -34, -39, 2, 8, 14, 20, 25,
	31, 37, 43, -2, -8, -14, -20, -25, -31, -37, -43, 3,
	9, 15, 21, 28, 34, 40, 46, -3, -9, -15, -21, -28, -34,
	-40, -46, 3, 10, 17, 24, 31, 38, 45, 52, -3, -10, -17,
	-24, -31, -38, -45, -52, 3, 11, 19, 27, 34, 42, 50,
	58, -3, -11, -19, -27, -34, -42, -50, -58, 4, 12, 21,
	29, 38, 46, 55, 63, -4, -12, -21, -29, -38, -46, -55,
	-63, 4, 13, 23, 32, 41, 50, 60, 69, -4, -13, -23, -32,
	-41, -50, -60, -69, 5, 15, 25, 35, 46, 56, 66, 76, -5,
	-15, -25, -35, -46, -56, -66, -76, 5, 16, 28, 39, 50,
	61, 73, 84, -5, -16, -28, -39, -50, -61, -73, -84, 6,
	18, 31, 43, 56, 68, 81, 93, -6, -18, -31, -43, -56,
	-68, -81, -93, 6, 20, 34, 48, 61, 75, 89, 103, -6, -20,
	-34, -48, -61, -75, -89, -103, 7, 22, 37, 52, 67, 82,
	97, 112, -7, -22, -37, -52, -67, -82, -97, -112, 8,
	24, 41, 57, 74, 90, 107, 123, -8, -24, -41, -57, -74,
	-90, -107, -123, 9, 27, 45, 63, 82, 100, 118, 136, -9,
	-27, -45, -63, -82, -100, -118, -136, 10, 30, 50, 70,
	90, 110, 130, 150, -10, -30, -50, -70, -90, -110, -130,
	-150, 11, 33, 55, 77, 99, 121, 143, 165, -11, -33, -55,
	-77, -99, -121, -143, -165, 12, 36, 60, 84, 109, 133,
	157, 181, -12, -36, -60, -84, -109, -133, -157, -181,
	13, 40, 66, 93, 120, 147, 173, 200, -13, -40, -66, -93,
	-120, -147, -173, -200, 14, 44, 73, 103, 132, 162, 191,
	221, -14, -44, -73, -103, -132, -162, -191, -221, 16,
	48, 81, 113, 146, 178, 211, 243, -16, -48, -81, -113,
	-146, -178, -211, -243, 17, 53, 89, 125, 160, 196, 232,
	268, -17, -53, -89, -125, -160, -196, -232, -268, 19,
	58, 98, 137, 176, 215, 255, 294, -19, -58, -98, -137,
	-176, -215, -255, -294, 21, 64, 108, 151, 194, 237,
	281, 324, -21, -64, -108, -151, -194, -237, -281, -324,
	23, 71, 118, 166, 213, 261, 308, 356, -23, -71, -118,
	-166, -213, -261, -308, -356, 26, 78, 130, 182, 235,
	287, 339, 391, -26, -78, -130, -182, -235, -287, -339,
	-391, 28, 86, 143, 201, 258, 316, 373, 431, -28, -86,
	-143, -201, -258, -316, -373, -431, 31, 94, 158, 221,
	284, 347, 411, 474, -31, -94, -158, -221, -284, -347,
	-411, -474, 34, 104, 174, 244, 313, 383, 453, 523, -34,
	-104, -174, -244, -313, -383, -453, -523, 38, 115, 191,
	268, 345, 422, 498, 575, -38, -115, -191, -268, -345,
	-422, -498, -575, 42, 126, 210, 294, 379, 463, 547,
	631, -42, -126, -210, -294, -379, -463, -547, -631,
	46, 139, 231, 324, 417, 510, 602, 695, -46, -139, -231,
	-324, -417, -510, -602, -695, 51, 153, 255, 357, 459,
	561, 663, 765, -51, -153, -255, -357, -459, -561, -663,
	-765, 56, 168, 280, 392, 505, 617, 729, 841, -56, -168,
	-280, -392, -505, -617, -729, -841, 61, 185, 308, 432,
	555, 679, 802, 926, -61, -185, -308, -432, -555, -679,
	-802, -926, 68, 204, 340, 476, 612, 748, 884, 1020,
	-68, -204, -340, -476, -612, -748, -884, -1020, 74,
	224, 373, 523, 672, 822, 971, 1121, -74, -224, -373,
	-523, -672, -822, -971, -1121, 82, 246, 411, 575, 740,
	904, 1069, 1233, -82, -246, -411, -575, -740, -904,
	-1069, -1233, 90, 271, 452, 633, 814, 995, 1176, 1357,
	-90, -271, -452, -633, -814, -995, -1176, -1357, 99,
	298, 497, 696, 895, 1094, 1293, 1492, -99, -298, -497,
	-696, -895, -1094, -1293, -1492, 109, 328, 547, 766,
	985, 1204, 1423, 1642, -109, -328, -547, -766, -985,
	-1204, -1423, -1642, 120, 361, 601, 842, 1083, 1324,
	1564, 1805, -120, -361, -601, -842, -1083, -1324, -1564,
	-1805, 132, 397, 662, 927, 1192, 1457, 1722, 1987, -132,
	-397, -662, -927, -1192, -1457, -1722, -1987, 145, 437,
	728, 1020, 1311, 1603, 1894, 2186, -145, -437, -728,
	-1020, -1311, -1603, -1894, -2186, 160, 480, 801, 1121,
	1442, 1762, 2083, 2403, -160, -480, -801, -1121, -1442,
	-1762, -2083, -2403, 176, 529, 881, 1234, 1587, 1940,
	2292, 2645, -176, -529, -881, -1234, -1587, -1940, -2292,
	-2645, 194, 582, 970, 1358, 1746, 2134, 2522, 2910,
	-194, -582, -970, -1358, -1746, -2134, -2522, -2910,
	213, 640, 1066, 1493, 1920, 2347, 2773, 3200, -213,
	-640, -1066, -1493, -1920, -2347, -2773, -3200, 234,
	704, 1173, 1643, 2112, 2582, 3051, 3521, -234, -704,
	-1173, -1643, -2112, -2582, -3051, -3521, 258, 774,
	1291, 1807, 2324, 2840, 3357, 3873, -258, -774, -1291,
	-1807, -2324, -2840, -3357, -3873, 284, 852, 1420, 1988,
	2556, 3124, 3692, 4260, -284, -852, -1420, -1988, -2556,
	-3124, -3692, -4260, 312, 937, 1561, 2186, 2811, 3436,
	4060, 4685, -312, -937, -1561, -2186, -2811, -3436,
	-4060, -4685, 343, 1030, 1718, 2405, 3092, 3779, 4467,
	5154, -343, -1030, -1718, -2405, -3092, -3779, -4467,
	-5154, 378, 1134, 1890, 2646, 3402, 4158, 4914, 5670,
	-378, -1134, -1890, -2646, -3402, -4158, -4914, -5670,
	415, 1247, 2079, 2911, 3742, 4574, 5406, 6238, -415,
	-1247, -2079, -2911, -3742, -4574, -5406, -6238, 457,
	1372, 2287, 3202, 4117, 5032, 5947, 6862, -457, -1372,
	-2287, -3202, -4117, -5032, -5947, -6862, 503, 1509,
	2516, 3522, 4529, 5535, 6542, 7548, -503, -1509, -2516,
	-3522, -4529, -5535, -6542, -7548, 553, 1660, 2767,
	3874, 4981, 6088, 7195, 8302, -553, -1660, -2767, -3874,
	-4981, -6088, -7195, -8302, 608, 1826, 3044, 4262, 5479,
	6697, 7915, 9133, -608, -1826, -3044, -4262, -5479,
	-6697, -7915, -9133, 669, 2009, 3348, 4688, 6027, 7367,
	8706, 10046, -669, -2009, -3348, -4688, -6027, -7367,
	-8706, -10046, 736, 2210, 3683, 5157, 6630, 8104, 9577,
	11051, -736, -2210, -3683, -5157, -6630, -8104, -9577,
	-11051, 810, 2431, 4052, 5673, 7294, 8915, 10536, 12157,
	-810, -2431, -4052, -5673, -7294, -8915, -10536, -12157,
	891, 2674, 4457, 6240, 8023, 9806, 11589, 13372, -891,
	-2674, -4457, -6240, -8023, -9806, -11589, -13372, 980,
	2941, 4903, 6864, 8825, 10786, 12748, 14709, -980, -2941,
	-4903, -6864, -8825, -10786, -12748, -14709, 1078, 3236,
	5393, 7551, 9708, 11866, 14023, 16181, -1078, -3236,
	-5393, -7551, -9708, -11866, -14023, -16181, 1186, 3559,
	5933, 8306, 10679, 13052, 15426, 17799, -1186, -3559,
	-5933, -8306, -10679, -13052, -15426, -17799, 1305,
	3915, 6526, 9136, 11747, 14357, 16968, 19578, -1305,
	-3915, -6526, -9136, -11747, -14357, -16968, -19578,
	1435, 4307, 7179, 10051, 12922, 15794, 18666, 21538,
	-1435, -4307, -7179, -10051, -12922, -15794, -18666,
	-21538, 1579, 4738, 7896, 11055, 14214, 17373, 20531,
	23690, -1579, -4738, -7896, -11055, -14214, -17373,
	-20531, -23690, 1737, 5212, 8686, 12161, 15636, 19111,
	22585, 26060, -1737, -5212, -8686, -12161, -15636, -19111,
	-22585, -26060, 1911, 5733, 9555, 13377, 17200, 21022,
	24844, 28666, -1911, -5733, -9555, -13377, -17200, -21022,
	-24844, -28666, 2102, 6306, 10511, 14715, 18920, 23124,
	27329, 31533, -2102, -6306, -10511, -14715, -18920,
	-23124, -27329, -31533, 2312, 6937, 11562, 16187, 20812,
	25437, 30062, 32767, -2312, -6937, -11562, -16187, -20812,
	-25437, -30062, -32767, 2543, 7631, 12718, 17806, 22893,
	27981, 32767, 32767, -2543, -7631, -12718, -17806, -22893,
	-27981, -32767, -32767, 2798, 8394, 13990, 19586, 25183,
	30779, 32767, 32767, -2798, -8394, -13990, -19586, -25183,
	-30779, -32767, -32767, 3077, 9233, 15389, 21545, 27700,
	32767, 32767, 32767, -3077, -9233, -15389, -21545, -27700,
	-32767, -32767, -32767, 3385, 10157, 16928, 23700, 30471,
	32767, 32767, 32767, -3385, -10157, -16928, -23700,
	-30471, -32767, -32767, -32767, 3724, 11172, 18621,
	26069, 32767, 32767, 32767, 32767, -3724, -11172, -18621,
	-26069, -32767, -32767, -32767, -32767, 4095, 12287,
	20479, 28671, 32767, 32767, 32767, 32767, -4095, -12287,
	-20479, -28671, -32767, -32767, -32767, -32767
};

static const int p1s[17] = { 0, 4, 3, 4, 2, 4, 3, 4, 1, 4, 3, 4,  2, 4,  3, 4,  0};
static const int p2s[17] = { 0, 1, 1, 3, 1, 5, 3, 7, 1, 9, 5, 11, 3, 13, 7, 15, 1};

static void soundFilter(byte *data, int16 *buffer, int p1, int p2);

void SoundManager::applyFilter(SoundEntry *entry, int16 *buffer) {
	if ((((byte *)entry->soundData)[1] << 6) > 0x1600) {
    entry->status.status |= 0x20000000;
  } else {
	  int variant = entry->status.status & 0x1f;

	  soundFilter((byte *)entry->soundData, buffer, p1s[variant], p2s[variant]);
  }
}


static void soundFilter(byte *data, int16 *buffer, int p1, int p2) {
	int data1, data2, data1p, data2p;
	byte idx;

	data2 = data[0];
	data1 = data[1] << 6;

	data += 2;

	for (int count = 0; count < 735; count++) {
		idx = data[count] >> 4;
		data1p = filterData[idx + data1];
		data2p = CLIP(filterData2[idx + data1] + data2, -32767, 32767);

		buffer[2 * count] = (p2 * data2p) >> p1;

		idx = data[count] & 0xF;

		data1 = filterData[idx + data1p];
		data2 = CLIP(filterData2[idx + data1p] + data2p, -32767, 32767);
		buffer[2 * count + 1] = (p2 * data2) >> p1;
	}
}


} // End of namespace LastExpress
