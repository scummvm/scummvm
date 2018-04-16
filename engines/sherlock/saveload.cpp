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

#include "sherlock/saveload.h"
#include "sherlock/surface.h"
#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel_saveload.h"
#include "sherlock/tattoo/widget_files.h"
#include "common/system.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"

namespace Sherlock {

const char *const EMPTY_SAVEGAME_SLOT = "-EMPTY-";
static const char *const SAVEGAME_STR = "SHLK";
#define SAVEGAME_STR_SIZE 4

/*----------------------------------------------------------------*/

SaveManager *SaveManager::init(SherlockEngine *vm, const Common::String &target) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelSaveManager(vm, target);
	else
		return new Tattoo::WidgetFiles(vm, target);
}

SaveManager::SaveManager(SherlockEngine *vm, const Common::String &target) :
		_vm(vm), _target(target) {
	_saveThumb = nullptr;
	_justLoaded = false;
	_savegameIndex = 0;
}

SaveManager::~SaveManager() {
	if (_saveThumb) {
		_saveThumb->free();
		delete _saveThumb;
	}
}

void SaveManager::createSavegameList() {
	Screen &screen = *_vm->_screen;

	_savegames.clear();
	for (int idx = 0; idx < MAX_SAVEGAME_SLOTS; ++idx)
		_savegames.push_back(EMPTY_SAVEGAME_SLOT);

	SaveStateList saveList = getSavegameList(_target);
	for (uint idx = 0; idx < saveList.size(); ++idx) {
		int slot = saveList[idx].getSaveSlot();
		if (slot >= 0 && slot < MAX_SAVEGAME_SLOTS)
			_savegames[slot] = saveList[idx].getDescription();
	}

	// Ensure the names will fit on the screen
	for (uint idx = 0; idx < _savegames.size(); ++idx) {
		int width = screen.stringWidth(_savegames[idx]) + 24;
		if (width > 308) {
			// It won't fit in, so remove characters until it does
			do {
				width -= screen.charWidth(_savegames[idx].lastChar());
				_savegames[idx].deleteLastChar();
			} while (width > 300);
		}
	}
}

SaveStateList SaveManager::getSavegameList(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target.c_str());
	SherlockSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVEGAME_SLOTS) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(slot, header._saveName));

				delete in;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

WARN_UNUSED_RESULT bool SaveManager::readSavegameHeader(Common::InSaveFile *in, SherlockSavegameHeader &header, bool skipThumbnail) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version < MINIMUM_SAVEGAME_VERSION || header._version > CURRENT_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0') header._saveName += ch;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header._thumbnail, skipThumbnail)) {
		return false;
	}

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

	out->writeByte(CURRENT_SAVEGAME_VERSION);

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

void SaveManager::createThumbnail() {
	if (_saveThumb) {
		_saveThumb->free();
		delete _saveThumb;
	}

	_saveThumb = new Graphics::Surface();

	if (!IS_3DO) {
		uint8 thumbPalette[PALETTE_SIZE];
		_vm->_screen->getPalette(thumbPalette);
		::createThumbnail(_saveThumb, (const byte *)_vm->_screen->getPixels(), SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT, thumbPalette);
	} else {
		::createThumbnailFromScreen(_saveThumb);
	}
}

void SaveManager::loadGame(int slot) {
	Events &events = *_vm->_events;
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
		generateSaveName(slot));
	if (!saveFile)
		return;

	// Load the savaegame header
	SherlockSavegameHeader header;
	if (!readSavegameHeader(saveFile, header))
		error("Invalid savegame");

	// Synchronize the savegame data
	Serializer s(saveFile, nullptr);
	s.setVersion(header._version);
	synchronize(s);

	delete saveFile;
	events.clearEvents();
}

void SaveManager::saveGame(int slot, const Common::String &name) {
	Events &events = *_vm->_events;
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(
		generateSaveName(slot));

	SherlockSavegameHeader header;
	header._saveName = name;
	writeSavegameHeader(out, header);

	// Synchronize the savegame data
	Serializer s(nullptr, out);
	s.setVersion(CURRENT_SAVEGAME_VERSION);
	synchronize(s);

	out->finalize();
	delete out;
	events.clearEvents();
}

Common::String SaveManager::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _target.c_str(), slot);
}

void SaveManager::synchronize(Serializer &s) {
	Inventory &inv = *_vm->_inventory;
	Journal &journal = *_vm->_journal;
	Map &map = *_vm->_map;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;

	int oldFont = screen.fontNumber();

	inv.synchronize(s);
	journal.synchronize(s);
	people.synchronize(s);
	map.synchronize(s);
	scene.synchronize(s);
	screen.synchronize(s);
	talk.synchronize(s);
	_vm->synchronize(s);

	if (screen.fontNumber() != oldFont)
		journal.resetPosition();

	_justLoaded = s.isLoading();
}

bool SaveManager::isSlotEmpty(int slot) const {
	return _savegames[slot].equalsIgnoreCase(EMPTY_SAVEGAME_SLOT);
}

} // End of namespace Sherlock
