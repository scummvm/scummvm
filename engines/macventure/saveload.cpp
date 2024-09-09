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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "macventure/macventure.h"

#include "common/error.h"
#include "common/savefile.h"
#include "engines/savestate.h"
#include "gui/saveload.h"
#include "graphics/thumbnail.h"

namespace MacVenture {

#define MACVENTURE_SAVE_HEADER MKTAG('M', 'V', 'S', 'S') // (M)ac(V)enture (S)cummVM (S)ave (0x4d565353, uint32)
#define MACVENTURE_SAVE_VERSION 1 //1 BYTE
#define MACVENTURE_DESC_LENGTH 4 //4 BYTE for the metadata length

SaveStateDescriptor loadMetaData(Common::SeekableReadStream *s, int slot, bool skipThumbnail) {
	// Metadata is stored at the end of the file
	// |THUMBNAIL						|
	// |								|
	// |DESCSIZE| DESCRIPTION			|
	// |HEADER			|VERSION|DESCLEN|
	s->seek(-(5 + MACVENTURE_DESC_LENGTH), SEEK_END);
	uint32 sig = s->readUint32BE();
	byte version = s->readByte();

	SaveStateDescriptor desc;	// init to an invalid save slot

	if (sig != MACVENTURE_SAVE_HEADER || version > MACVENTURE_SAVE_VERSION)
		return desc;

	// Save is valid, set its slot number
	desc.setSaveSlot(slot);

	// Depends on MACVENTURE_DESC_LENGTH
	uint32 metaSize = s->readUint32BE();
	s->seek(-((int32)(5 + MACVENTURE_DESC_LENGTH + metaSize)), SEEK_END);

	// Load the thumbnail
	Graphics::Surface *thumbnail;
	if (!Graphics::loadThumbnail(*s, thumbnail, skipThumbnail)) {
		return desc;
	}
	desc.setThumbnail(thumbnail);

	// Load the description
	Common::String name;
	uint32 descSize = s->readUint32BE();
	for (uint32 i = 0; i < descSize; ++i) {
		name += s->readByte();
	}
	desc.setDescription(name);

	// Load date
	uint32 saveDate = s->readUint32LE();
	int day = (saveDate >> 24) & 0xFF;
	int month = (saveDate >> 16) & 0xFF;
	int year = saveDate & 0xFFFF;
	desc.setSaveDate(year, month, day);

	uint16 saveTime = s->readUint16LE();
	int hour = (saveTime >> 8) & 0xFF;
	int minutes = saveTime & 0xFF;
	desc.setSaveTime(hour, minutes);

	// Load playtime
	uint32 playTime = s->readUint32LE();
	desc.setPlayTime(playTime * 1000);

	return desc;
}

uint saveCurrentDate(Common::OutSaveFile *file) {
	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);

	file->writeUint32LE(saveDate);
	file->writeUint16LE(saveTime);

	// Return the number of bytes occupied
	return 6;
}

uint savePlayTime(Common::OutSaveFile *file) {
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;
	file->writeUint32LE(playTime);
	// Return the number of bytes occupied
	return 4;
}

void writeMetaData(Common::OutSaveFile *file, Common::String desc) {

	// Write thumbnail
	uint thumbSize = file->pos();
	Graphics::saveThumbnail(*file);
	thumbSize = file->pos() - thumbSize;

	// Write description
	file->writeUint32BE(desc.size());
	file->writeString(desc);

	uint dateSize = saveCurrentDate(file);
	uint playTimeSize = savePlayTime(file);

	file->writeUint32BE(MACVENTURE_SAVE_HEADER);
	file->writeByte(MACVENTURE_SAVE_VERSION);
	file->writeUint32BE(4 + desc.size() + dateSize + playTimeSize + thumbSize);
}

Common::Error MacVentureEngine::loadGameState(int slot) {
	Common::String saveFileName = getSaveStateName(slot);
	Common::InSaveFile *file;
	if (!(file = Common::MacResManager::openFileOrDataFork(Common::Path(saveFileName)))) {
		error("ENGINE: Missing savegame file %s", saveFileName.c_str());
	}
	_world->loadGameFrom(file);
	reset();
	return Common::kNoError;
}

Common::Error MacVentureEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::String saveFileName = getSaveStateName(slot);
	Common::SaveFileManager *manager = getSaveFileManager();
	// HACK Get a real name!
	Common::OutSaveFile *file = manager->openForSaving(saveFileName);
	_world->saveGameInto(file);
	writeMetaData(file, desc);

	file->finalize();
	if (file->err()) {
		warning("Could not save '%s' correctly.", saveFileName.c_str());
	}
	delete file;
	return Common::kNoError;
}

bool MacVentureEngine::scummVMSaveLoadDialog(bool isSave) {
	if (!isSave) {
		// do loading
		GUI::SaveLoadChooser dialog = GUI::SaveLoadChooser(Common::String("Load game:"), Common::String("Load"), false);
		int slot = dialog.runModalWithCurrentTarget();

		if (slot < 0)
			return true;

		return loadGameState(slot).getCode() == Common::kNoError;
	}

	// do saving
	GUI::SaveLoadChooser dialog = GUI::SaveLoadChooser(Common::String("Save game:"), Common::String("Save"), true);
	int slot = dialog.runModalWithCurrentTarget();
	Common::String desc = dialog.getResultString();

	if (desc.empty()) {
		// create our own description for the saved game, the user didn't enter it
		desc = dialog.createDefaultSaveDescription(slot);
	}

	/*
	if (desc.size() > (1 << MACVENTURE_DESC_LENGTH * 8) - 1)
		desc = Common::String(desc.c_str(), (1 << MACVENTURE_DESC_LENGTH * 8) - 1);
	*/
	if (slot < 0)
		return true;

	return saveGameState(slot, desc).getCode() == Common::kNoError;
}

bool MacVentureEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return true;
}

bool MacVentureEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return true;
}

} // End of namespace MacVenture
