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

#include "common/scummsys.h"

#include "zvision/core/save_manager.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"

#include "common/system.h"

#include "graphics/surface.h"
#include "graphics/thumbnail.h"

#include "gui/message.h"


namespace ZVision {

const uint32 SaveManager::SAVEGAME_ID = MKTAG('Z', 'E', 'N', 'G');

void SaveManager::saveGame(uint slot, const Common::String &saveName) {
	// The games only support 20 slots
	assert(slot <= 1 && slot <= 20);

	Common::SaveFileManager *saveFileManager = g_system->getSavefileManager();
	Common::OutSaveFile *file = saveFileManager->openForSaving(_engine->generateSaveFileName(slot));

	// Write out the savegame header
	file->writeUint32BE(SAVEGAME_ID);

	// Write version
	file->writeByte(SAVE_VERSION);

	// Write savegame name
	file->writeString(saveName);
	file->writeByte(0);

	// We can't call writeGameSaveData because the save menu is actually
	// a room, so writeGameSaveData would save us in the save menu.
	// However, an auto save is performed before each room change, so we
	// can copy the data from there. We can guarantee that an auto save file will
	// exist before this is called because the save menu can only be accessed
	// after the first room (the main menu) has loaded.
	Common::InSaveFile *autoSaveFile = saveFileManager->openForLoading(_engine->generateAutoSaveFileName());

	// Skip over the header info
	autoSaveFile->readSint32BE(); // SAVEGAME_ID
	autoSaveFile->readByte(); // Version
	autoSaveFile->seek(5, SEEK_CUR); // The string "auto" with terminating NULL

	// Read the rest to a buffer
	uint32 size = autoSaveFile->size() - autoSaveFile->pos();
	byte *buffer = new byte[size];
	autoSaveFile->read(buffer, size);

	// Then write the buffer to the new file
	file->write(buffer, size);

	// Cleanup
	delete[] buffer;
	file->finalize();
	delete file;
}

void SaveManager::autoSave() {
	Common::OutSaveFile *file = g_system->getSavefileManager()->openForSaving(_engine->generateAutoSaveFileName());

	// Write out the savegame header
	file->writeUint32BE(SAVEGAME_ID);

	// Version
	file->writeByte(SAVE_VERSION);

	file->writeString("auto");
	file->writeByte(0);

	writeSaveGameData(file);

	// Cleanup
	file->finalize();
	delete file;
}

void SaveManager::writeSaveGameData(Common::OutSaveFile *file) {
	// Create a thumbnail and save it
	Graphics::saveThumbnail(*file);
	
	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	file->writeSint16LE(td.tm_year + 1900);
	file->writeSint16LE(td.tm_mon + 1);
	file->writeSint16LE(td.tm_mday);
	file->writeSint16LE(td.tm_hour);
	file->writeSint16LE(td.tm_min);

	ScriptManager *scriptManager = _engine->getScriptManager();
	// Write out the current location
	Location currentLocation = scriptManager->getCurrentLocation();
	file->writeByte(currentLocation.world);
	file->writeByte(currentLocation.room);
	file->writeByte(currentLocation.node);
	file->writeByte(currentLocation.view);
	file->writeUint32LE(currentLocation.offset);

	// Write out the current state table values
	scriptManager->serializeStateTable(file);

	// Write out any controls needing to save state
	scriptManager->serializeControls(file);
}

Common::Error SaveManager::loadGame(uint slot) {
	// The games only support 20 slots
	assert(slot <= 1 && slot <= 20);

	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(_engine->generateSaveFileName(slot));
	if (saveFile == 0) {
		return Common::kPathDoesNotExist;
	}

	// Read the header
	SaveGameHeader header;
	if (!readSaveGameHeader(saveFile, header)) {
		return Common::kUnknownError;
	}

	char world = (char)saveFile->readByte();
	char room = (char)saveFile->readByte();
	char node = (char)saveFile->readByte();
	char view = (char)saveFile->readByte();
	uint32 offset = (char)saveFile->readUint32LE();

	ScriptManager *scriptManager = _engine->getScriptManager();
	// Update the state table values
	scriptManager->deserializeStateTable(saveFile);

	// Load the room
	scriptManager->changeLocation(world, room, node, view, offset);

	// Update the controls
	scriptManager->deserializeControls(saveFile);

	return Common::kNoError;
}

bool SaveManager::readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader &header) {
	if (in->readUint32BE() != SAVEGAME_ID) {
		warning("File is not a ZVision save file. Aborting load");
		return false;
	}
	
	// Read in the version
	header.version = in->readByte();

	// Check that the save version isn't newer than this binary
	if (header.version > SAVE_VERSION) {
		uint tempVersion = header.version;
		GUI::MessageDialog dialog(Common::String::format("This save file uses version %u, but this engine only supports up to version %d. You will need an updated version of the engine to use this save file.", tempVersion, SAVE_VERSION), "OK");
		dialog.runModal();
	}

	// Read in the save name
	header.saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header.saveName += ch;

	// Get the thumbnail
	header.thumbnail = Graphics::loadThumbnail(*in);
	if (!header.thumbnail)
		return false;

	// Read in save date/time
	header.saveYear = in->readSint16LE();
	header.saveMonth = in->readSint16LE();
	header.saveDay = in->readSint16LE();
	header.saveHour = in->readSint16LE();
	header.saveMinutes = in->readSint16LE();

	return true;
}

} // End of namespace ZVision
