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

#include "lastexpress/game/savegame.h"

#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/menu.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

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
	{"blue.egg"},
	{"red.egg"},
	{"green.egg"},
	{"purple.egg"},
	{"teal.egg"},
	{"gold.egg"}
};

//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////

SaveLoad::SaveLoad(LastExpressEngine *engine) : _engine(engine), _savegame(NULL), _gameTicksLastSavegame(0) {
}

SaveLoad::~SaveLoad() {
	clear();

	SAFE_DELETE(_savegame);

	//Zero passed pointers
	_engine = NULL;
}

void SaveLoad::initStream() {
	SAFE_DELETE(_savegame);

	_savegame = new SavegameStream();
}

void SaveLoad::flushStream(GameId id) {
	Common::OutSaveFile *save = openForSaving(id);
	if (!save)
		error("SaveLoad::initSave: Cannot init savegame (%s)!", getFilename(id).c_str());

	save->write(_savegame->getData(), _savegame->size());

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

	// Open savegame and check size
	Common::InSaveFile *save = openForLoading(id);
	if (save->size() < 32)
		error("SaveLoad::init - Savegame seems to be corrupted (not enough data: %i bytes)", save->size());

	// Load all savegame data
	uint8* buf = new uint8[4096];
	while (!save->eos() && !save->err()) {
		uint32 count = save->read(buf, sizeof(buf));
		if (count) {
			uint32 w = _savegame->write(buf, count);
			assert (w == count);
		}
	}
	if (save->err())
		error("SaveLoad::init - Error reading savegame");
	delete[] buf;
	_savegame->seek(0);

	SAFE_DELETE(save);

	// Load the main header
	Common::Serializer ser(_savegame, NULL);
	SavegameMainHeader mainHeader;
	mainHeader.saveLoadWithSerializer(ser);
	if (!mainHeader.isValid())
		error("SaveLoad::init - Savegame seems to be corrupted (invalid header)");

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
		while (!_savegame->eos() && !_savegame->err()) {

			// Update sound queue while we go through the savegame
			getSound()->updateQueue();

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

void SaveLoad::clear() {
	for (uint i = 0; i < _gameHeaders.size(); i++)
		SAFE_DELETE(_gameHeaders[i]);

	_gameHeaders.clear();
}

//////////////////////////////////////////////////////////////////////////
// Save & Load
//////////////////////////////////////////////////////////////////////////

// Load game
bool SaveLoad::loadGame(GameId id) {

	if (!SaveLoad::isSavegamePresent(id))
		return false;

	//Common::InSaveFile *save = SaveLoad::openForLoading(id);
	// Validate header

	error("SaveLoad::loadgame: not implemented!");

	return false;
}

bool SaveLoad::loadGame2(GameId id) {
	error("SaveLoad::loadgame2: not implemented!");
}

// Save game
void SaveLoad::saveGame(SavegameType type, EntityIndex entity, uint32 value) {
	if (getState()->scene <= kSceneIntro)
		return;

	// Validate main header
	SavegameMainHeader header;
	if (!loadMainHeader(_savegame, &header)) {
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::saveGame - Cannot load main header: %s", getFilename(getMenu()->getGameId()).c_str());
		return;
	}

	// Validate the current entry if it exists
	if (header.count > 0) {
		_savegame->seek(header.offsetEntry);

		// Load entry header
		SavegameEntryHeader entry;
		Common::Serializer ser(_savegame, NULL);
		entry.saveLoadWithSerializer(ser);

		if (!entry.isValid()) {
			warning("SaveLoad::saveGame: Invalid entry. This savegame might be corrupted!");
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
		header.offsetEntry = _savegame->pos();

	// Write the savegame entry
	writeEntry(type, entity, value);

	if (!header.keepIndex)
		++header.count;

	if (type == kSavegameTypeEvent2 || type == kSavegameTypeAuto) {
		header.keepIndex = 1;
	} else {
		header.keepIndex = 0;
		header.offset = _savegame->pos();

		// Save ticks
		_gameTicksLastSavegame = getState()->timeTicks;
	}

	// Validate the main header
	if (!header.isValid())
		error("SaveLoad::saveGame: main game header is invalid!");

	// Write the main header
	_savegame->seek(0);
	Common::Serializer ser(NULL, _savegame);
	header.saveLoadWithSerializer(ser);

	flushStream(getMenu()->getGameId());
}

void SaveLoad::saveVolumeBrightness() {
	warning("SaveLoad::saveVolumeBrightness: not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Static Members
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
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::isSavegameValid - Savegame does not exist: %s", getFilename(id).c_str());
		return false;
	}

	SavegameMainHeader header;

	Common::InSaveFile *save = openForLoading(id);
	return loadMainHeader(save, &header);
}

//////////////////////////////////////////////////////////////////////////
// Headers
//////////////////////////////////////////////////////////////////////////
bool SaveLoad::loadMainHeader(Common::InSaveFile *stream, SavegameMainHeader *header) {
	if (!stream)
		return false;

	// Check there is enough data (32 bytes)
	if (stream->size() < 32) {
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::loadMainHeader - Savegame seems to be corrupted (not enough data: %i bytes)!", stream->size());
		return false;
	}

	// Rewind stream
	stream->seek(0);

	Common::Serializer ser(stream, NULL);
	header->saveLoadWithSerializer(ser);

	// Validate the header
	if (!header->isValid()) {
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::loadMainHeader - Cannot validate main header!");
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Entries
//////////////////////////////////////////////////////////////////////////
void SaveLoad::writeEntry(SavegameType type, EntityIndex entity, uint32 value) {
#define WRITE_ENTRY(name, func, expected) { \
	uint32 _prevPosition = _savegame->pos(); \
	func; \
	uint32 _count = _savegame->pos() - _prevPosition; \
	debugC(kLastExpressDebugSavegame, "Savegame: Writing " #name ": %d bytes", _count); \
	if (expected != 0 && _count != expected)\
		error("SaveLoad::writeEntry: Number of bytes written (%d) differ from expected count (%d)", _count, expected); \
}

	SavegameEntryHeader header;

	header.type = type;
	header.time = getState()->time;
	header.chapter = getProgress().chapter;
	header.value = value;

	// Save position
	uint32 originalPosition = _savegame->pos();

	// Write header
	Common::Serializer ser(NULL, _savegame);
	header.saveLoadWithSerializer(ser);

	computeOffset();

	// Write game data
	WRITE_ENTRY("entity index", ser.syncAsUint32LE(entity), 4);
	WRITE_ENTRY("state", getState()->saveLoadWithSerializer(ser), 4 + 4 + 4 + 4 + 1 + 4 + 4);
	WRITE_ENTRY("selected item", getInventory()->saveSelectedItem(ser), 4);
	WRITE_ENTRY("positions", getEntities()->savePositions(ser), 4 * 1000);
	WRITE_ENTRY("compartments", getEntities()->saveCompartments(ser), 4 * 16 * 2);
	WRITE_ENTRY("progress", getProgress().saveLoadWithSerializer(ser), 4 * 128);
	WRITE_ENTRY("events", getState()->saveEvents(ser), 512);
	WRITE_ENTRY("inventory", getInventory()->saveLoadWithSerializer(ser), 7 * 32);
	WRITE_ENTRY("objects", getObjects()->saveLoadWithSerializer(ser), 5 * 128);
	WRITE_ENTRY("entities", getEntities()->saveLoadWithSerializer(ser), 1262 * 40);
	WRITE_ENTRY("sound", getSound()->saveLoadWithSerializer(ser), 0);
	WRITE_ENTRY("savepoints", getSavePoints()->saveLoadWithSerializer(ser), 0);

	header.offset = computeOffset(originalPosition);

	// Add padding if necessary
	while (header.offset & 0xF) {
		_savegame->writeByte(0);
		header.offset++;
	}

	// Save end position
	uint32 endPosition = _savegame->pos();

	// Validate entry header
	if (!header.isValid())
		error("SaveLoad::writeEntry: entry header is invalid");

	// Save the header with the updated info
	_savegame->seek(originalPosition);
	header.saveLoadWithSerializer(ser);

	// Move back to the end of the entry
	_savegame->seek(endPosition);
}

void SaveLoad::readEntry(SavegameType type, EntityIndex entity, uint32 value) {
	warning("SaveLoad::readEntry: not implemented!");
}

SaveLoad::SavegameEntryHeader *SaveLoad::getEntry(uint32 index) {
	if (index >= _gameHeaders.size())
		error("SaveLoad::getEntry: invalid index (was:%d, max:%d)", index, _gameHeaders.size() - 1);

	return _gameHeaders[index];
}

uint32 SaveLoad::computeOffset(uint32 originalPosition) {
	warning("SaveLoad::computePadding: not implemented!");

	return (_savegame->pos() - originalPosition - 32);
}

//////////////////////////////////////////////////////////////////////////
// Checks
//////////////////////////////////////////////////////////////////////////
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
		error("SaveLoad::getName - attempting to use an invalid game id. Valid values: 0 - 5, was %d", id);

	return gameInfo[id].saveFile;
}

Common::InSaveFile *SaveLoad::openForLoading(GameId id) {
	Common::InSaveFile *load = g_system->getSavefileManager()->openForLoading(getFilename(id));

	if (!load)
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::openForLoading - Cannot open savegame for loading: %s", getFilename(id).c_str());

	return load;
}

Common::OutSaveFile *SaveLoad::openForSaving(GameId id) {
	Common::OutSaveFile *save = g_system->getSavefileManager()->openForSaving(getFilename(id));

	if (!save)
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::openForSaving - Cannot open savegame for writing: %s", getFilename(id).c_str());

	return save;
}

} // End of namespace LastExpress
