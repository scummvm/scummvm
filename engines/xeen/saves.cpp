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
#include "common/algorithm.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "xeen/saves.h"
#include "xeen/files.h"
#include "xeen/xeen.h"

namespace Xeen {

SavesManager::SavesManager(const Common::String &targetName): _targetName(targetName),
		_wonWorld(false), _wonDarkSide(false) {
	File::_xeenSave = nullptr;
	File::_darkSave = nullptr;

	if (g_vm->getGameID() != GType_Clouds) {
		File::_darkSave = new SaveArchive(g_vm->_party);
		File::_darkSave->reset(File::_darkCc);
	}
	if (g_vm->getGameID() != GType_DarkSide) {
		File::_xeenSave = new SaveArchive(g_vm->_party);
		File::_xeenSave->reset(File::_xeenCc);
	}

	File::_currentSave = g_vm->getGameID() == GType_DarkSide ?
		File::_darkSave : File::_xeenSave;
	assert(File::_currentSave);

	// Set any final initial values
	Party &party = *g_vm->_party;
	party.resetBlacksmithWares();
	party._year = g_vm->getGameID() == GType_WorldOfXeen ? 610 : 850;
	party._totalTime = 0;
}

SavesManager::~SavesManager() {
	delete File::_xeenSave;
	delete File::_darkSave;
}

const char *const SAVEGAME_STR = "XEEN";
#define SAVEGAME_STR_SIZE 6

bool SavesManager::readSavegameHeader(Common::InSaveFile *in, XeenSavegameHeader &header) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];
	header._thumbnail = nullptr;

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version > XEEN_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header._saveName += ch;

	// Get the thumbnail
	header._thumbnail = Graphics::loadThumbnail(*in);
	if (!header._thumbnail)
		return false;

	// Read in save date/time
	header._year = in->readSint16LE();
	header._month = in->readSint16LE();
	header._day = in->readSint16LE();
	header._hour = in->readSint16LE();
	header._minute = in->readSint16LE();
	header._totalFrames = in->readUint32LE();

	return true;
}

void SavesManager::writeSavegameHeader(Common::OutSaveFile *out, XeenSavegameHeader &header) {
	EventsManager &events = *g_vm->_events;
	Screen &screen = *g_vm->_screen;

	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(XEEN_SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(header._saveName);
	out->writeByte('\0');

	// Write a thumbnail of the screen
	uint8 thumbPalette[768];
	screen.getPalette(thumbPalette);
	Graphics::Surface saveThumb;
	::createThumbnail(&saveThumb, (const byte *)screen.getPixels(),
		screen.w, screen.h, thumbPalette);
	Graphics::saveThumbnail(*out, saveThumb);
	saveThumb.free();

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
	out->writeUint32LE(events.playTime());
}

Common::Error SavesManager::saveGameState(int slot, const Common::String &desc) {
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(
		generateSaveName(slot));
	if (!out)
		return Common::kCreatingFileFailed;

	// Push map and party data to the save archives
	Map &map = *g_vm->_map;
	map.saveMaze();


	XeenSavegameHeader header;
	header._saveName = desc;
	writeSavegameHeader(out, header);

	// Loop through saving the sides' save archives
	SaveArchive *archives[2] = { File::_xeenSave, File::_darkSave };
	for (int idx = 0; idx < 2; ++idx) {
		if (archives[idx]) {
			archives[idx]->save(*out);
		} else {
			// Side isn't present
			out->writeUint32LE(0);
		}
	}

	// Write out miscellaneous
	FileManager &files = *g_vm->_files;
	files.save(*out);

	out->finalize();
	delete out;

	return Common::kNoError;
}

Common::Error SavesManager::loadGameState(int slot) {
	EventsManager &events = *g_vm->_events;
	FileManager &files = *g_vm->_files;
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;

	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
		generateSaveName(slot));
	if (!saveFile)
		return Common::kReadingFailed;

	// Load the savaegame header
	XeenSavegameHeader header;
	if (!readSavegameHeader(saveFile, header))
		error("Invalid savegame");

	if (header._thumbnail) {
		header._thumbnail->free();
		delete header._thumbnail;
	}

	// Set the total play time
	events.setPlayTime(header._totalFrames);

	// Loop through loading the sides' save archives
	SaveArchive *archives[2] = { File::_xeenSave, File::_darkSave };
	for (int idx = 0; idx < 2; ++idx) {
		uint fileSize = saveFile->readUint32LE();

		if (archives[idx]) {
			Common::SeekableSubReadStream arcStream(saveFile, saveFile->pos(),
				saveFile->pos() + fileSize);
			archives[idx]->load(arcStream);
		} else {
			assert(!fileSize);
		}
	}

	// Read in miscellaneous
	files.load(*saveFile);

	// Load the new map
	map.load(party._mazeId);

	return Common::kNoError;
}

Common::String SavesManager::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

} // End of namespace Xeen
