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

#include "sherlock/saveload.h"
#include "sherlock/graphics.h"
#include "sherlock/sherlock.h"
#include "common/system.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"

namespace Sherlock {

SaveManager::SaveManager(SherlockEngine *vm, const Common::String &target) : 
		_vm(vm), _target(target) {
	_saveThumb = nullptr;
}

SaveManager::~SaveManager() {
	if (_saveThumb) {
		_saveThumb->free();
		delete _saveThumb;
	}
}

/**
 * Shows the in-game dialog interface for loading and saving games
 */
void SaveManager::show() {
	createSavegameList();

	// TODO
}

/**
 * Build up a savegame list, with empty slots given an explicit Empty message
 */
void SaveManager::createSavegameList() {
	_savegames.clear();
	for (int idx = 0; idx < NUM_SAVEGAME_SLOTS; ++idx)
		_savegames.push_back("-EMPTY");

	SaveStateList saveList = getSavegameList(_target);
	for (uint idx = 0; idx < saveList.size(); ++idx)
		_savegames[saveList[idx].getSaveSlot()] = saveList[idx].getDescription();
}

/**
 * Load a list of savegames
 */
SaveStateList SaveManager::getSavegameList(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0??", target.c_str());
	SherlockSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort to get the files in numerical order

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < NUM_SAVEGAME_SLOTS) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				readSavegameHeader(in, header);
				saveList.push_back(SaveStateDescriptor(slot, header._saveName));

				header._thumbnail->free();
				delete header._thumbnail;
				delete in;
			}
		}
	}

	return saveList;
}

const char *const SAVEGAME_STR = "SHLK";
#define SAVEGAME_STR_SIZE 4

bool SaveManager::readSavegameHeader(Common::InSaveFile *in, SherlockSavegameHeader &header) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];
	header._thumbnail = nullptr;

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version > SHERLOCK_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0') header._saveName += ch;

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

void SaveManager::writeSavegameHeader(Common::OutSaveFile *out, SherlockSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(SHERLOCK_SAVEGAME_VERSION);

	// Write savegame name
	out->write(header._saveName.c_str(), header._saveName.size());
	out->writeByte('\0');

	// Handle the thumbnail. If there's already one set by the game, create one
	if (!_saveThumb)
		createThumbnail();
	Graphics::saveThumbnail(*out, *_saveThumb);

	_saveThumb->free();
	delete _saveThumb;
	_saveThumb = nullptr;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
	out->writeUint32LE(_vm->_events->getFrameCounter());
}

/**
 * Creates a thumbnail for the current on-screen contents
 */
void SaveManager::createThumbnail() {
	if (_saveThumb) {
		_saveThumb->free();
		delete _saveThumb;
	}

	uint8 thumbPalette[PALETTE_SIZE];
	_vm->_screen->getPalette(thumbPalette);
	_saveThumb = new Graphics::Surface();
	::createThumbnail(_saveThumb, (const byte *)_vm->_screen->getPixels(), SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT, thumbPalette);
}

} // End of namespace Sherlock
