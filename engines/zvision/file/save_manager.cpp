/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"

#include "zvision/zvision.h"
#include "zvision/file/save_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"

#include "common/system.h"
#include "common/translation.h"

#include "graphics/surface.h"
#include "graphics/thumbnail.h"

#include "gui/message.h"
#include "gui/saveload.h"

namespace ZVision {

const uint32 SaveManager::SAVEGAME_ID = MKTAG('Z', 'E', 'N', 'G');

bool SaveManager::scummVMSaveLoadDialog(bool isSave) {
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	if (isSave) {
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

		slot = dialog->runModalWithCurrentTarget();
		desc = dialog->getResultString();

		if (desc.empty()) {
			// create our own description for the saved game, the user didn't enter it
			desc = dialog->createDefaultSaveDescription(slot);
		}

		if (desc.size() > 28)
			desc = Common::String(desc.c_str(), 28);
	} else {
		dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		slot = dialog->runModalWithCurrentTarget();
	}

	delete dialog;

	if (slot < 0)
		return false;

	if (isSave) {
		saveGame(slot, desc, false);
		return true;
	} else {
		Common::ErrorCode result = loadGame(slot).getCode();
		return (result == Common::kNoError);
	}
}

void SaveManager::saveGame(uint slot, const Common::String &saveName, bool useSaveBuffer) {
	if (!_tempSave && useSaveBuffer)
		return;

	Common::SaveFileManager *saveFileManager = g_system->getSavefileManager();
	Common::OutSaveFile *file = saveFileManager->openForSaving(_engine->getSaveStateName(slot));

	writeSaveGameHeader(file, saveName, useSaveBuffer);

	if (useSaveBuffer)
		file->write(_tempSave->getData(), _tempSave->size());
	else
		_engine->getScriptManager()->serialize(file);

	file->finalize();
	delete file;

	if (useSaveBuffer)
		flushSaveBuffer();

	_lastSaveTime = g_system->getMillis();
}

void SaveManager::writeSaveGameHeader(Common::OutSaveFile *file, const Common::String &saveName, bool useSaveBuffer) {
	file->writeUint32BE(SAVEGAME_ID);

	// Write version
	file->writeByte(SAVE_VERSION);

	// Write savegame name
	file->writeString(saveName);
	file->writeByte(0);

	// Save the game thumbnail
	if (useSaveBuffer)
		file->write(_tempThumbnail->getData(), _tempThumbnail->size());
	else
		Graphics::saveThumbnail(*file);

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	file->writeSint16LE(td.tm_year + 1900);
	file->writeSint16LE(td.tm_mon + 1);
	file->writeSint16LE(td.tm_mday);
	file->writeSint16LE(td.tm_hour);
	file->writeSint16LE(td.tm_min);

	file->writeUint32LE(g_engine->getTotalPlayTime() / 1000);
}

Common::Error SaveManager::loadGame(int slot) {
	Common::SeekableReadStream *saveFile = NULL;

	if (slot >= 0) {
		saveFile = getSlotFile(slot);
	} else {
		saveFile = _engine->getSearchManager()->openFile("r.svr");
		if (!saveFile) {
			Common::File *restoreFile = new Common::File();
			if (!restoreFile->open("r.svr")) {
				delete restoreFile;
				return Common::kPathDoesNotExist;
			}

			saveFile = restoreFile;
		}
	}

	if (!saveFile)
		return Common::kPathDoesNotExist;

	// Read the header
	SaveGameHeader header;
	if (!readSaveGameHeader(saveFile, header)) {
		return Common::kUnknownError;
	}

	ScriptManager *scriptManager = _engine->getScriptManager();
	// Update the state table values
	scriptManager->deserialize(saveFile);

	delete saveFile;

	if (_engine->getGameId() == GID_NEMESIS && scriptManager->getCurrentLocation() == "tv2f") {
		// WORKAROUND for script bug #6793: location tv2f (stairs) has two states:
		// one at the top of the stairs, and one at the bottom. When the player
		// goes to the bottom of the stairs, the screen changes, and hotspot
		// 4652 (exit opposite the stairs) is enabled. However, the variable that
		// controls the state (2408) is reset when the player goes down the stairs.
		// Furthermore, the room's initialization script disables the stair exit
		// control (4652). This leads to an impossible situation, where all the
		// exit controls are disabled, and the player can't more anywhere. Thus,
		// when loading a game in that room, we check for that impossible
		// situation, which only occurs after the player has moved down the stairs,
		// and fix it here by setting the correct background, and enabling the
		// stair exit hotspot.
		if ((scriptManager->getStateFlag(2411) & Puzzle::DISABLED) &&
			(scriptManager->getStateFlag(2408) & Puzzle::DISABLED) &&
			(scriptManager->getStateFlag(4652) & Puzzle::DISABLED)) {
			_engine->getRenderManager()->setBackgroundImage("tv2fb21c.tga");
			scriptManager->unsetStateFlag(4652, Puzzle::DISABLED);
		}
	}

	g_engine->setTotalPlayTime(header.playTime * 1000);

	return Common::kNoError;
}

bool SaveManager::readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader &header, bool skipThumbnail) {
	header.saveYear    = 0;
	header.saveMonth   = 0;
	header.saveDay     = 0;
	header.saveHour    = 0;
	header.saveMinutes = 0;
	header.playTime    = 0;
	header.saveName.clear();
	header.thumbnail   = nullptr;
	header.version     = 0;

	uint32 tag = in->readUint32BE();
	// Check if it's original savegame than fill header structure
	if (tag == MKTAG('Z', 'N', 'S', 'G')) {
		header.saveName = "Original Save";
		header.version = SAVE_ORIGINAL;
		in->seek(-4, SEEK_CUR);
		return true;
	}
	if (tag != SAVEGAME_ID) {
		warning("File is not a Z-Vision saved game. Aborting load");
		return false;
	}

	// Read in the version
	header.version = in->readByte();

	// Check that the save version isn't newer than this binary
	if (header.version > SAVE_VERSION) {
		uint tempVersion = header.version;
		GUI::MessageDialog dialog(
			Common::U32String::format(
				_("This saved game uses version %u, but this engine only "
				  "supports up to version %d. You will need an updated version "
				  "of the engine to use this saved game."), tempVersion, SAVE_VERSION
			),
		_("OK"));
		dialog.runModal();
	}

	// Read in the save name
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header.saveName += ch;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header.thumbnail, skipThumbnail)) {
		return false;
	}

	// Read in save date/time
	header.saveYear    = in->readSint16LE();
	header.saveMonth   = in->readSint16LE();
	header.saveDay     = in->readSint16LE();
	header.saveHour    = in->readSint16LE();
	header.saveMinutes = in->readSint16LE();

	if (header.version >= 2) {
		header.playTime  = in->readUint32LE();
	}

	return true;
}

Common::SeekableReadStream *SaveManager::getSlotFile(uint slot) {
	Common::SeekableReadStream *saveFile = g_system->getSavefileManager()->openForLoading(_engine->getSaveStateName(slot));
	if (saveFile == NULL) {
		// Try to load standard save file
		Common::Path filename;
		if (_engine->getGameId() == GID_GRANDINQUISITOR)
			filename = Common::Path(Common::String::format("inqsav%u.sav", slot));
		else if (_engine->getGameId() == GID_NEMESIS)
			filename = Common::Path(Common::String::format("nemsav%u.sav", slot));

		saveFile = _engine->getSearchManager()->openFile(filename);
		if (saveFile == NULL) {
			Common::File *tmpFile = new Common::File;
			if (!tmpFile->open(filename)) {
				delete tmpFile;
			} else {
				saveFile = tmpFile;
			}
		}

	}

	return saveFile;
}

void SaveManager::prepareSaveBuffer() {
	delete _tempThumbnail;
	_tempThumbnail = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	Graphics::saveThumbnail(*_tempThumbnail);

	delete _tempSave;
	_tempSave = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	_engine->getScriptManager()->serialize(_tempSave);
}

void SaveManager::flushSaveBuffer() {
	delete _tempThumbnail;
	_tempThumbnail = NULL;

	delete _tempSave;
	_tempSave = NULL;
}

} // End of namespace ZVision
