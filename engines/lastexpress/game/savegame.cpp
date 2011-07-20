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

#include "lastexpress/game/savegame.h"

#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/menu/menu.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/debug.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/helpers.h"

#include "common/file.h"
#include "common/system.h"

namespace LastExpress {

// Names of savegames
static const struct {
	const char *saveFile;
} gameInfo[6] = {
	{"lastexpress-blue.egg"},
	{"lastexpress-red.egg"},
	{"lastexpress-green.egg"},
	{"lastexpress-purple.egg"},
	{"lastexpress-teal.egg"},
	{"lastexpress-gold.egg"}
};

//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////

SaveLoad::SaveLoad(LastExpressEngine *engine) : _engine(engine), _savegame(NULL), _gameTicksLastSavegame(0) {
}

SaveLoad::~SaveLoad() {
	clear(true);

	// Zero passed pointers
	_engine = NULL;
}

void SaveLoad::initStream() {
	delete _savegame;
	_savegame = new SavegameStream();
}

void SaveLoad::flushStream(GameId id) {
	Common::OutSaveFile *save = openForSaving(id);
	if (!save)
		error("[SaveLoad::flushStream] Cannot open savegame (%s)", getFilename(id).c_str());

	if (!_savegame)
		error("[SaveLoad::flushStream] Savegame stream is invalid");

	save->write(_savegame->getData(), (uint32)_savegame->size());

	delete save;
}

//////////////////////////////////////////////////////////////////////////
// Init
//////////////////////////////////////////////////////////////////////////
void SaveLoad::create(GameId id) {
	initStream();

	Common::Serializer ser(NULL, _savegame);
	SavegameMainHeader header;
	header.saveLoadWithSerializer(ser);

	flushStream(id);
}

uint32 SaveLoad::init(GameId id, bool resetHeaders) {
	initStream();

	// Load game data
	loadStream(id);

	// Get the main header
	Common::Serializer ser(_savegame, NULL);
	SavegameMainHeader mainHeader;
	mainHeader.saveLoadWithSerializer(ser);
	if (!mainHeader.isValid())
		error("[SaveLoad::init] Savegame seems to be corrupted (invalid header)");

	// Reset cached entry headers if needed
	if (resetHeaders) {
		clear();

		SavegameEntryHeader *entryHeader = new SavegameEntryHeader();
		entryHeader->time = kTimeCityParis;
		entryHeader->chapter = kChapter1;

		_gameHeaders.push_back(entryHeader);
	}

	// Read the list of entry headers
	if (_savegame->size() > 32) {
		while (_savegame->pos() < _savegame->size() && !_savegame->eos() && !_savegame->err()) {

			// Update sound queue while we go through the savegame
			getSoundQueue()->updateQueue();

			SavegameEntryHeader *entry = new SavegameEntryHeader();
			entry->saveLoadWithSerializer(ser);

			if (!entry->isValid())
				break;

			_gameHeaders.push_back(entry);

			_savegame->seek(entry->offset, SEEK_CUR);
		}
	}

	// return the index to the current save game entry (we store count + 1 entries, so we're good)
	return mainHeader.count;
}

void SaveLoad::loadStream(GameId id) {
	Common::InSaveFile *save = openForLoading(id);
	if (save->size() < 32)
		error("[SaveLoad::loadStream] Savegame seems to be corrupted (not enough data: %i bytes)", save->size());

	if (!_savegame)
		error("[SaveLoad::loadStream] Savegame stream is invalid");

	// Load all savegame data
	uint8* buf = new uint8[8192];
	while (!save->eos() && !save->err()) {
		_engine->pollEvents();

		uint32 count = save->read(buf, sizeof(buf));
		if (count) {
			uint32 w = _savegame->write(buf, count);
			assert (w == count);
		}
	}

	if (save->err())
		error("SaveLoad::init - Error reading savegame");

	delete[] buf;
	delete save;

	// Move back to the beginning of the stream
	_savegame->seek(0);
}

void SaveLoad::clear(bool clearStream) {
	for (uint i = 0; i < _gameHeaders.size(); i++)
		SAFE_DELETE(_gameHeaders[i]);

	_gameHeaders.clear();

	if (clearStream)
		SAFE_DELETE(_savegame);
}

//////////////////////////////////////////////////////////////////////////
// Save & Load
//////////////////////////////////////////////////////////////////////////

// Load game
void SaveLoad::loadGame(GameId id) {
	if (!_savegame)
		error("[SaveLoad::loadGame] No savegame stream present");

	// Rewind current savegame
	_savegame->seek(0);

	// Validate main header
	SavegameMainHeader header;
	if (!loadMainHeader(_savegame, &header)) {
		debugC(2, kLastExpressDebugSavegame, "Cannot load main header: %s", getFilename(getMenu()->getGameId()).c_str());
		return;
	}

	if (!_savegame)
		error("[SaveLoad::loadGame] No savegame stream present");

	// Load the last entry
	_savegame->seek(header.offsetEntry);

	SavegameType type = kSavegameTypeIndex;
	EntityIndex entity = kEntityPlayer;
	uint32 val = 0;
	readEntry(&type, &entity, &val, header.keepIndex == 1);

	// Setup last loading time
	_gameTicksLastSavegame = getState()->timeTicks;

	if (header.keepIndex) {
		getSoundQueue()->clearQueue();

		readEntry(&type, &entity, &val, false);
	}

	getEntities()->reset();
	getEntities()->setup(false, entity);
}

// Load a specific game entry
void SaveLoad::loadGame(GameId id, uint32 index) {
	error("[SaveLoad::loadGame] Not implemented! (only loading the last entry is working for now)");
}

// Save game
void SaveLoad::saveGame(SavegameType type, EntityIndex entity, uint32 value) {
	if (getState()->scene <= kSceneIntro)
		return;

	// Validate main header
	SavegameMainHeader header;
	if (!loadMainHeader(_savegame, &header)) {
		debugC(2, kLastExpressDebugSavegame, "Cannot load main header: %s", getFilename(getMenu()->getGameId()).c_str());
		return;
	}

	if (!_savegame)
		error("[SaveLoad::saveGame] Savegame stream is invalid");

	// Validate the current entry if it exists
	if (header.count > 0) {
		_savegame->seek(header.offsetEntry);

		// Load entry header
		SavegameEntryHeader entry;
		Common::Serializer ser(_savegame, NULL);
		entry.saveLoadWithSerializer(ser);

		if (!entry.isValid()) {
			warning("[SaveLoad::saveGame] Invalid entry. This savegame might be corrupted");
			_savegame->seek(header.offset);
		} else if (getState()->time < entry.time || (type == kSavegameTypeTickInterval && getState()->time == entry.time)) {
			// Not ready to save a game, skipping!
			return;
		} else if ((type == kSavegameTypeTime || type == kSavegameTypeEvent)
			&& (entry.type == kSavegameTypeTickInterval && (getState()->time - entry.time) < 450)) {
			_savegame->seek(header.offsetEntry);
			--header.count;
		} else {
			_savegame->seek(header.offset);
		}
	} else {
		// Seek to the next savegame entry
		_savegame->seek(header.offset);
	}

	if (type != kSavegameTypeEvent2 && type != kSavegameTypeAuto)
		header.offsetEntry = (uint32)_savegame->pos();

	// Write the savegame entry
	writeEntry(type, entity, value);

	if (!header.keepIndex)
		++header.count;

	if (type == kSavegameTypeEvent2 || type == kSavegameTypeAuto) {
		header.keepIndex = 1;
	} else {
		header.keepIndex = 0;
		header.offset = (uint32)_savegame->pos();

		// Save ticks
		_gameTicksLastSavegame = getState()->timeTicks;
	}

	// Validate the main header
	if (!header.isValid())
		error("[SaveLoad::saveGame] Main game header is invalid");

	// Write the main header
	_savegame->seek(0);
	Common::Serializer ser(NULL, _savegame);
	header.saveLoadWithSerializer(ser);

	flushStream(getMenu()->getGameId());
}

void SaveLoad::saveVolumeBrightness() {
	warning("[SaveLoad::saveVolumeBrightness] Not implemented");
}

//////////////////////////////////////////////////////////////////////////
// Headers
//////////////////////////////////////////////////////////////////////////
bool SaveLoad::loadMainHeader(Common::InSaveFile *stream, SavegameMainHeader *header) {
	if (!stream)
		return false;

	// Check there is enough data (32 bytes)
	if (stream->size() < 32) {
		debugC(2, kLastExpressDebugSavegame, "Savegame seems to be corrupted (not enough data: %i bytes)", stream->size());
		return false;
	}

	// Rewind stream
	stream->seek(0);

	Common::Serializer ser(stream, NULL);
	header->saveLoadWithSerializer(ser);

	// Validate the header
	if (!header->isValid()) {
		debugC(2, kLastExpressDebugSavegame, "Cannot validate main header");
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Entries
//////////////////////////////////////////////////////////////////////////
void SaveLoad::writeEntry(SavegameType type, EntityIndex entity, uint32 value) {
#define WRITE_ENTRY(name, func, val) { \
	uint32 _prevPosition = (uint32)_savegame->pos(); \
	func; \
	uint32 _count = (uint32)_savegame->pos() - _prevPosition; \
	debugC(kLastExpressDebugSavegame, "Savegame: Writing " #name ": %d bytes", _count); \
	if (_count != val)\
		error("[SaveLoad::writeEntry] Number of bytes written (%d) differ from expected count (%d)", _count, val); \
}

	if (!_savegame)
		error("[SaveLoad::writeEntry] Savegame stream is invalid");

	SavegameEntryHeader header;

	header.type = type;
	header.time = (uint32)getState()->time;
	header.chapter = getProgress().chapter;
	header.value = value;

	// Save position
	uint32 originalPosition = (uint32)_savegame->pos();

	// Write header
	Common::Serializer ser(NULL, _savegame);
	header.saveLoadWithSerializer(ser);

	// Write game data
	WRITE_ENTRY("entity index", ser.syncAsUint32LE(entity), 4);
	WRITE_ENTRY("state", getState()->saveLoadWithSerializer(ser), 4 + 4 + 4 + 4 + 1 + 4 + 4);
	WRITE_ENTRY("selected item", getInventory()->saveSelectedItem(ser), 4);
	WRITE_ENTRY("positions", getEntities()->savePositions(ser), 4 * 1000);
	WRITE_ENTRY("compartments", getEntities()->saveCompartments(ser), 4 * 16 * 2);
	WRITE_ENTRY("progress", getProgress().saveLoadWithSerializer(ser), 4 * 128);
	WRITE_ENTRY("events", getState()->syncEvents(ser), 512);
	WRITE_ENTRY("inventory", getInventory()->saveLoadWithSerializer(ser), 7 * 32);
	WRITE_ENTRY("objects", getObjects()->saveLoadWithSerializer(ser), 5 * 128);
	WRITE_ENTRY("entities", getEntities()->saveLoadWithSerializer(ser), 1262 * 40);
	WRITE_ENTRY("sound", getSoundQueue()->saveLoadWithSerializer(ser), 3 * 4 + getSoundQueue()->count() * 64);
	WRITE_ENTRY("savepoints", getSavePoints()->saveLoadWithSerializer(ser), 128 * 16 + 4 + getSavePoints()->count() * 16);

	header.offset = (uint32)_savegame->pos() - (originalPosition + 32);

	// Add padding if necessary
	while (header.offset & 0xF) {
		_savegame->writeByte(0);
		header.offset++;
	}

	// Save end position
	uint32 endPosition = (uint32)_savegame->pos();

	// Validate entry header
	if (!header.isValid())
		error("[SaveLoad::writeEntry] Entry header is invalid");

	// Save the header with the updated info
	_savegame->seek(originalPosition);
	header.saveLoadWithSerializer(ser);

	// Move back to the end of the entry
	_savegame->seek(endPosition);
}

void SaveLoad::readEntry(SavegameType *type, EntityIndex *entity, uint32 *val, bool keepIndex) {
#define LOAD_ENTRY(name, func, val) { \
	uint32 _prevPosition = (uint32)_savegame->pos(); \
	func; \
	uint32 _count = (uint32)_savegame->pos() - _prevPosition; \
	debugC(kLastExpressDebugSavegame, "Savegame: Reading " #name ": %d bytes", _count); \
	if (_count != val) \
		error("[SaveLoad::readEntry] Number of bytes read (%d) differ from expected count (%d)", _count, val); \
}

#define LOAD_ENTRY_ONLY(name, func) { \
	uint32 _prevPosition = (uint32)_savegame->pos(); \
	func; \
	uint32 _count = (uint32)_savegame->pos() - _prevPosition; \
	debugC(kLastExpressDebugSavegame, "Savegame: Reading " #name ": %d bytes", _count); \
}

	if (!type || !entity || !val)
		error("[SaveLoad::readEntry] Invalid parameters passed");

	if (!_savegame)
		error("[SaveLoad::readEntry] No savegame stream present");

	// Load entry header
	SavegameEntryHeader entry;
	Common::Serializer ser(_savegame, NULL);
	entry.saveLoadWithSerializer(ser);

	if (!entry.isValid())
		error("[SaveLoad::readEntry] Entry header is invalid");

	// Init type, entity & value
	*type = entry.type;
	*val = entry.value;

	// Save position
	uint32 originalPosition = (uint32)_savegame->pos();

	// Load game data
	LOAD_ENTRY("entity index", ser.syncAsUint32LE(*entity), 4);
	LOAD_ENTRY("state", getState()->saveLoadWithSerializer(ser), 4 + 4 + 4 + 4 + 1 + 4 + 4);
	LOAD_ENTRY("selected item", getInventory()->saveSelectedItem(ser), 4);
	LOAD_ENTRY("positions", getEntities()->savePositions(ser), 4 * 1000);
	LOAD_ENTRY("compartments", getEntities()->saveCompartments(ser), 4 * 16 * 2);
	LOAD_ENTRY("progress", getProgress().saveLoadWithSerializer(ser), 4 * 128);
	LOAD_ENTRY("events", getState()->syncEvents(ser), 512);
	LOAD_ENTRY("inventory", getInventory()->saveLoadWithSerializer(ser), 7 * 32);
	LOAD_ENTRY("objects", getObjects()->saveLoadWithSerializer(ser), 5 * 128);
	LOAD_ENTRY("entities", getEntities()->saveLoadWithSerializer(ser), 1262 * 40);
	LOAD_ENTRY_ONLY("sound", getSoundQueue()->saveLoadWithSerializer(ser));
	LOAD_ENTRY_ONLY("savepoints", getSavePoints()->saveLoadWithSerializer(ser));

	// Update chapter
	getProgress().chapter = entry.chapter;

	// Skip padding
	uint32 offset = (uint32)_savegame->pos() - originalPosition;
	if (offset & 0xF) {
		_savegame->seek((~offset & 0xF) + 1, SEEK_SET);
	}
}

SaveLoad::SavegameEntryHeader *SaveLoad::getEntry(uint32 index) {
	if (index >= _gameHeaders.size())
		error("[SaveLoad::getEntry] Invalid index (was:%d, max:%d)", index, _gameHeaders.size() - 1);

	return _gameHeaders[index];
}

//////////////////////////////////////////////////////////////////////////
// Checks
//////////////////////////////////////////////////////////////////////////

// Check if a specific savegame exists
bool SaveLoad::isSavegamePresent(GameId id) {
	if (g_system->getSavefileManager()->listSavefiles(getFilename(id)).size() == 0)
		return false;

	return true;
}

// Check if the game has been started in the specific savegame
bool SaveLoad::isSavegameValid(GameId id) {
	if (!isSavegamePresent(id)) {
		debugC(2, kLastExpressDebugSavegame, "Savegame does not exist: %s", getFilename(id).c_str());
		return false;
	}

	SavegameMainHeader header;

	Common::InSaveFile *save = openForLoading(id);
	bool isHeaderValid = loadMainHeader(save, &header);
	delete save;

	return isHeaderValid;
}

bool SaveLoad::isGameFinished(uint32 menuIndex, uint32 savegameIndex) {
	SavegameEntryHeader *data = getEntry(menuIndex);

	if (savegameIndex != menuIndex)
		return false;

	if (data->type != kSavegameTypeEvent)
		return false;

	return (data->value == kEventAnnaKilled
	     || data->value == kEventKronosHostageAnnaNoFirebird
	     || data->value == kEventKahinaPunchBaggageCarEntrance
	     || data->value == kEventKahinaPunchBlue
	     || data->value == kEventKahinaPunchYellow
	     || data->value == kEventKahinaPunchSalon
	     || data->value == kEventKahinaPunchKitchen
	     || data->value == kEventKahinaPunchBaggageCar
	     || data->value == kEventKahinaPunchCar
	     || data->value == kEventKahinaPunchSuite4
	     || data->value == kEventKahinaPunchRestaurant
	     || data->value == kEventKahinaPunch
	     || data->value == kEventKronosGiveFirebird
	     || data->value == kEventAugustFindCorpse
	     || data->value == kEventMertensBloodJacket
	     || data->value == kEventMertensCorpseFloor
	     || data->value == kEventMertensCorpseBed
	     || data->value == kEventCoudertBloodJacket
	     || data->value == kEventGendarmesArrestation
	     || data->value == kEventAbbotDrinkGiveDetonator
	     || data->value == kEventMilosCorpseFloor
	     || data->value == kEventLocomotiveAnnaStopsTrain
	     || data->value == kEventTrainStopped
	     || data->value == kEventCathVesnaRestaurantKilled
	     || data->value == kEventCathVesnaTrainTopKilled
	     || data->value == kEventLocomotiveConductorsDiscovered
	     || data->value == kEventViennaAugustUnloadGuns
	     || data->value == kEventViennaKronosFirebird
	     || data->value == kEventVergesAnnaDead
	     || data->value == kEventTrainExplosionBridge
	     || data->value == kEventKronosBringNothing);
}


//////////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////////

// Get the file name from the savegame ID
Common::String SaveLoad::getFilename(GameId id) {
	if (id >= 6)
		error("[SaveLoad::getFilename] Attempting to use an invalid game id. Valid values: 0 - 5, was %d", id);

	return gameInfo[id].saveFile;
}

Common::InSaveFile *SaveLoad::openForLoading(GameId id) {
	Common::InSaveFile *load = g_system->getSavefileManager()->openForLoading(getFilename(id));

	if (!load)
		debugC(2, kLastExpressDebugSavegame, "Cannot open savegame for loading: %s", getFilename(id).c_str());

	return load;
}

Common::OutSaveFile *SaveLoad::openForSaving(GameId id) {
	Common::OutSaveFile *save = g_system->getSavefileManager()->openForSaving(getFilename(id));

	if (!save)
		debugC(2, kLastExpressDebugSavegame, "Cannot open savegame for writing: %s", getFilename(id).c_str());

	return save;
}

} // End of namespace LastExpress
