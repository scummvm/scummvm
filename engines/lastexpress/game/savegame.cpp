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

#include "lastexpress/game/logic.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/state.h"

#include "lastexpress/debug.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/helpers.h"

#include "common/file.h"
#include "common/system.h"

namespace LastExpress {

// Savegame signatures
#define SAVEGAME_SIGNATURE 0x12001200
#define SAVEGAME_HEADER    0xE660E660

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

SaveLoad::SaveLoad(LastExpressEngine *engine) : _engine(engine) {
	_gameTicksLastSavegame = 0;
}

SaveLoad::~SaveLoad() {
	//Zero passed pointers
	_engine = NULL;

	clearEntries();
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

// Save game
void SaveLoad::saveGame(SavegameType type, EntityIndex entity, uint32 value) {

	// Save ticks
	_gameTicksLastSavegame = getState()->timeTicks;

	warning("SaveLoad::savegame: not implemented!");
}

void SaveLoad::saveVolumeBrightness() {
	warning("SaveLoad::saveVolumeBrightness: not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Static Members
//////////////////////////////////////////////////////////////////////////

// Check if a specific savegame exists
bool SaveLoad::isSavegamePresent(GameId id) {
	if (g_system->getSavefileManager()->listSavefiles(getSavegameName(id)).size() == 0)
		return false;

	return true;
}

// Check if the game has been started in the specific savegame
bool SaveLoad::isSavegameValid(GameId id) {
	if (!isSavegamePresent(id)) {
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::isSavegameValid - Savegame does not exist: %s", getSavegameName(id).c_str());
		return false;
	}

	SavegameMainHeader header;
	if (!loadMainHeader(id, &header))
		return false;

	return validateMainHeader(header);
}


//////////////////////////////////////////////////////////////////////////
// Headers
//////////////////////////////////////////////////////////////////////////
bool SaveLoad::loadMainHeader(GameId id, SavegameMainHeader *header) {
	// Read first 32 bytes of savegame
	Common::InSaveFile *save = openForLoading(id);
	if (!save) {
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::loadMainHeader - Cannot open savegame for reading: %s", getSavegameName(id).c_str());
		return false;
	}

	// Check there is enough data
	if (save->size() < 32) {
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::loadMainHeader - Savegame seems to be corrupted (not enough data: %i bytes): %s", save->size(), getSavegameName(id).c_str());
		delete save;
		return false;
	}

	header->signature = save->readUint32LE();
	header->index = save->readUint32LE();
	header->time = save->readUint32LE();
	header->field_C = save->readUint32LE();
	header->field_10 = save->readUint32LE();
	header->brightness = save->readSint32LE();
	header->volume = save->readSint32LE();
	header->field_1C = save->readUint32LE();

	delete save;

	// Valide the header
	if (!validateMainHeader(*header)) {
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::loadMainHeader - Cannot validate main header for savegame %s.", getSavegameName(id).c_str());
		return false;
	}

	return true;
}

void SaveLoad::loadEntryHeader(Common::InSaveFile *save, SavegameEntryHeader *header) {
	header->signature = save->readUint32LE();
	header->type = (HeaderType)save->readUint32LE();
	header->time = save->readUint32LE();
	header->field_C = save->readUint32LE();
	header->chapter = (ChapterIndex)save->readUint32LE();
	header->event = (EventIndex)save->readUint32LE();
	header->field_18 = save->readUint32LE();
	header->field_1C = save->readUint32LE();
}

bool SaveLoad::validateMainHeader(const SavegameMainHeader &header) {
	if (header.signature != SAVEGAME_SIGNATURE)
		return false;

	/* Check not needed as it can never be < 0
	if (header.chapter < 0)
		return false;*/

	if (header.time < 32)
		return false;

	if (header.field_C < 32)
		return false;

	if (header.field_10 != 1 && header.field_10)
		return false;

	if (header.brightness < 0 || header.brightness > 6)
		return false;

	if (header.volume < 0 || header.volume > 7)
		return false;

	if (header.field_1C != 9)
		return false;

	return true;
}

bool SaveLoad::validateEntryHeader(const SavegameEntryHeader &header) {
	if (header.signature != SAVEGAME_HEADER)
		return false;

	if (header.type < kHeaderType1 || header.type > kHeaderType5)
		return false;

	if (header.time < kTimeStartGame || header.time > kTimeCityConstantinople)
		return false;

	if (header.field_C <= 0 || header.field_C >= 15)
		return false;

	/* No check for < 0, as it cannot happen normaly */
	if (header.chapter == 0)
		return false;

	return true;
}

SaveLoad::SavegameEntryHeader *SaveLoad::getEntry(uint32 index) {
	if (index >= _gameHeaders.size())
		error("SaveLoad::getEntry: invalid index (was:%d, max:%d)", index, _gameHeaders.size() - 1);

	return _gameHeaders[index];
}

void SaveLoad::clearEntries() {
	for (uint i = 0; i < _gameHeaders.size(); i++)
		SAFE_DELETE(_gameHeaders[i]);

	_gameHeaders.clear();
}

//////////////////////////////////////////////////////////////////////////
// Init
//////////////////////////////////////////////////////////////////////////
void SaveLoad::writeMainHeader(GameId id) {
	Common::OutSaveFile *save = openForSaving(id);
	if (!save) {
		debugC(2, kLastExpressDebugSavegame, "SaveLoad::initSavegame - Cannot open savegame for writing: %s", getSavegameName(id).c_str());
		return;
	}

	// Write default values to savegame
	save->writeUint32LE(SAVEGAME_SIGNATURE);
	save->writeUint32LE(0);
	save->writeUint32LE(32);
	save->writeUint32LE(32);
	save->writeUint32LE(0);
	save->writeUint32LE(3);
	save->writeUint32LE(7);
	save->writeUint32LE(9);

	delete save;
}

void SaveLoad::initSavegame(GameId id, bool resetHeaders) {
	//Common::OutSaveFile *save = openForSaving(id);
	//if (!save) {
	//	debugC(2, kLastExpressDebugSavegame, "SaveLoad::initSavegame - Cannot open savegame for writing: %s", getSavegameName(id).c_str());
	//	return;
	//}

	if (resetHeaders) {
		clearEntries();

		SavegameEntryHeader *header = new SavegameEntryHeader();
		header->time = kTimeCityParis;
		header->chapter = kChapter1;

		_gameHeaders.push_back(header);
	}

	// Open the savegame and read all game headers

	warning("SaveLoad::initSavegame: not implemented!");

	//delete save;
}

//////////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////////

// Get the file name from the savegame ID
Common::String SaveLoad::getSavegameName(GameId id) {
	if (id >= 6)
		error("SaveLoad::getSavegameName - attempting to use an invalid game id. Valid values: 0 - 5, was %d", id);

	return gameInfo[id].saveFile;
}

Common::InSaveFile *SaveLoad::openForLoading(GameId id) {
	return g_system->getSavefileManager()->openForLoading(getSavegameName(id));
}

Common::OutSaveFile *SaveLoad::openForSaving(GameId id) {
	return g_system->getSavefileManager()->openForSaving(getSavegameName(id));
}

} // End of namespace LastExpress
