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

#include "lastexpress/game/sound.h"

#include "lastexpress/data/snd.h"
#include "lastexpress/data/subtitle.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

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
	_soundStream = new StreamedSound();

	// Initialize unknown data
	_data0 = 0;
	_data1 = 0;
	_data2 = 0;

	memset(&_buffer, 0, sizeof(_buffer));
	memset(&_lastWarning, 0, sizeof(_lastWarning));
}

SoundManager::~SoundManager() {
	for (Common::List<SoundEntry *>::iterator i = _cache.begin(); i != _cache.end(); ++i)
		SAFE_DELETE(*i);

	_cache.clear();

	SAFE_DELETE(_soundStream);

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Timer
//////////////////////////////////////////////////////////////////////////
void SoundManager::handleTimer() {
	_mutex.lock();

	for (Common::List<SoundEntry *>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		SoundEntry *entry = (*i);
		if (entry->stream == NULL) {
			SAFE_DELETE(*i);
			i = _cache.reverse_erase(i);
			continue;
		} else if (!entry->isStreamed) {
			entry->isStreamed = true;

			// TODO: stream any sound in the queue after filtering
			_soundStream->load(entry->stream);
		}
	}

	_mutex.unlock();
}

//////////////////////////////////////////////////////////////////////////
// Sound queue management
//////////////////////////////////////////////////////////////////////////
void SoundManager::updateQueue() {
	// TODO add mutex lock!
	//warning("Sound::unknownFunction1: not implemented!");
}

void SoundManager::resetQueue(SoundType type1, SoundType type2) {
	if (!type2)
		type2 = type1;

	_mutex.lock();

	for (Common::List<SoundEntry *>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		if ((*i)->type != type1 && (*i)->type != type2)
			resetEntry(*i);
	}

	_mutex.unlock();
}

void SoundManager::removeFromQueue(EntityIndex entity) {
	_mutex.lock();

	SoundEntry *entry = getEntry(entity);
	if (entry)
		resetEntry(entry);

	_mutex.unlock();
}

void SoundManager::removeFromQueue(Common::String filename) {
	_mutex.lock();

	SoundEntry *entry = getEntry(filename);
	if (entry)
		resetEntry(entry);

	_mutex.unlock();
}

void SoundManager::clearQueue() {
	_flag |= 4;

	// FIXME: Wait a while for a flag to be set
	//for (int i = 0; i < 3000000; i++)
	//	if (_flag & 8)
	//		break;

	_flag |= 8;

	_mutex.lock();

	for (Common::List<SoundEntry *>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		SoundEntry *entry = (*i);

		// Delete entry
		removeEntry(entry);
		SAFE_DELETE(entry);

		i = _cache.reverse_erase(i);
	}

	_mutex.unlock();

	updateSubtitles();
}

bool SoundManager::isBuffered(EntityIndex entity) {
	_mutex.lock();

	bool buffered = (getEntry(entity) != NULL);	

	_mutex.unlock();

	return buffered;
}

bool SoundManager::isBuffered(Common::String filename, bool testForEntity) {
	_mutex.lock();

	SoundEntry *entry = getEntry(filename);

	bool ret = (entry != NULL);
	if (testForEntity)
		ret = ret && !entry->entity;

	_mutex.unlock();

	return ret;
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

	// Add entry to cache
	_cache.push_back(entry);

	setupCache(entry);
	loadSoundData(entry, name);
}

void SoundManager::setEntryType(SoundEntry *entry, FlagType flag) {
	switch (flag & kFlagType7) {
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

bool SoundManager::setupCache(SoundEntry *entry) {
	warning("Sound::setupCache: not implemented!");
	return true;
}

void SoundManager::clearStatus() {
	_mutex.lock();

	for (Common::List<SoundEntry *>::iterator i = _cache.begin(); i != _cache.end(); ++i)
		(*i)->status.status |= kSoundStatusClear3;

	_mutex.unlock();
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

void SoundManager::resetEntry(SoundEntry *entry) {
	entry->status.status |= kSoundStatusRemoved;
	entry->entity = kEntityPlayer;

	if (entry->stream) {
		if (!entry->isStreamed)
			SAFE_DELETE(entry->stream);

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
		drawSubtitles(entry->subtitle);
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
	_mutex.lock();

	SoundEntry *entry = getEntry(entity);
	if (entry) {
		updateEntry(entry, 0);
		entry->entity = kEntityPlayer;
	}

	_mutex.unlock();
}

void SoundManager::processEntry(SoundType type) {
	_mutex.lock();

	SoundEntry *entry = getEntry(type);
	if (entry)
		updateEntry(entry, 0);

	_mutex.unlock();
}

void SoundManager::setupEntry(SoundType type, EntityIndex index) {
	_mutex.lock();

	SoundEntry *entry = getEntry(type);
	if (entry)
		entry->entity = index;

	_mutex.unlock();
}

void SoundManager::processEntry(Common::String filename) {
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
	_mutex.lock();

	uint32 time = 0;
	SoundEntry *entry = getEntry(index);
	if (entry)
		time = entry->time;

	_mutex.unlock();

	return time;
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
	for (Common::List<SoundEntry *>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		if ((*i)->entity == index)
			return *i;
	}

	return NULL;
}

SoundManager::SoundEntry *SoundManager::getEntry(Common::String name) {
	if (!name.contains('.'))
		name += ".SND";

	for (Common::List<SoundEntry *>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		if ((*i)->name2 == name)
			return *i;
	}

	return NULL;
}

SoundManager::SoundEntry *SoundManager::getEntry(SoundType type) {
	for (Common::List<SoundEntry *>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
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

	_mutex.lock();

	// Save or load each entry data
	if (s.isSaving()) {
		for (Common::List<SoundEntry *>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
			SoundEntry *entry = *i;
			if (entry->name2.matchString("NISSND?") && (entry->status.status & kFlagType7) != kFlag3) {
				s.syncAsUint32LE(entry->status.status); // status;
				s.syncAsUint32LE(entry->type); // type;
				s.syncAsUint32LE(entry->field_1C); // field_8;
				s.syncAsUint32LE(entry->time); // time;
				s.syncAsUint32LE(entry->field_34); // field_10;
				s.syncAsUint32LE(entry->field_38); // field_14;
				s.syncAsUint32LE(entry->entity); // entity;

				uint32 field_1C = (uint32)entry->field_48 - _data2;
				if (field_1C > kFlag8)
					field_1C = 0;
				s.syncAsUint32LE(field_1C); // field_1C;

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

	_mutex.unlock();
}


// FIXME: We probably need another mutex here to protect during the whole savegame process
// as we could have removed an entry between the time we check the count and the time we
// save the entries
uint32 SoundManager::count() {
	_mutex.lock();

	uint32 numEntries = 0;
	for (Common::List<SoundEntry *>::iterator i = _cache.begin(); i != _cache.end(); ++i)
		if ((*i)->name2.matchString("NISSND?"))
			++numEntries;

	_mutex.unlock();

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

	_mutex.lock();

	setupEntry(entry, filename, flag, 30);
	entry->entity = entity;

	if (a4) {
		entry->field_48 = _data2 + 2 * a4;
		entry->status.status |= kSoundStatus_8000;
	} else {
		// Get subtitles name
		while (filename.size() > 4)
			filename.deleteLastChar();

		showSubtitles(entry, filename);
		updateEntryState(entry);
	}

	bool isPlaying = (entry->type != kSoundTypeNone);

	_mutex.unlock();

	return isPlaying;
}

void SoundManager::playSoundEvent(EntityIndex entity, byte action, byte a3) {
	char filename[12];
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

	if (_action) {
		sprintf((char *)&filename, "LIB%03d.SND", _action);

		if (flag)
			playSoundWithSubtitles((char*)&filename, flag, kEntityPlayer, a3);
	}
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
		showSubtitles(entry, cities[index]);
}

void SoundManager::playFightSound(byte action, byte a4) {
	int _action = (int)action;
	char filename[12];
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

	if (_action) {
		sprintf((char *)&filename, "LIB%03d.SND", _action);
		playSound(kEntityTrain, (char*)&filename, kFlagDefault, a4);
	}
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
	// TODO: Add mutex ?
	//warning("SoundManager::updateSubtitles: not implemented!");
}

void SoundManager::showSubtitles(SoundEntry *entry, Common::String filename) {
	warning("SoundManager::showSubtitles: not implemented!");
}

void SoundManager::drawSubtitles(SubtitleManager *subtitle) {
	warning("SoundManager::drawSubtitles: not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
void SoundManager::playLoopingSound() {
	warning("SoundManager::playLoopingSound: not implemented!");
}

void SoundManager::stopAllSound() const {
	_soundStream->stop();
}

} // End of namespace LastExpress
