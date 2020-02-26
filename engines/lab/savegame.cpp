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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/savefile.h"
#include "common/translation.h"

#include "gui/message.h"
#include "gui/saveload.h"

#include "graphics/thumbnail.h"
#include "engines/savestate.h"

#include "lab/lab.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/labsets.h"
#include "lab/music.h"
#include "lab/processroom.h"
#include "lab/speciallocks.h"

namespace Lab {

#define SAVEGAME_ID       MKTAG('L', 'O', 'T', 'S')
#define SAVEGAME_VERSION  1

void LabEngine::writeSaveGameHeader(Common::OutSaveFile *out, const Common::String &saveName) {
	out->writeUint32BE(SAVEGAME_ID);

	// Write version
	out->writeByte(SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(saveName);
	out->writeByte(0);

	// Save the game thumbnail
	Graphics::saveThumbnail(*out);

	// Creation date/time
	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);
	uint32 playTime = getTotalPlayTime() / 1000;

	out->writeUint32BE(saveDate);
	out->writeUint16BE(saveTime);
	out->writeUint32BE(playTime);
}

WARN_UNUSED_RESULT bool readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader &header, bool skipThumbnail) {
	uint32 id = in->readUint32BE();

	// Check if it's a valid ScummVM savegame
	if (id != SAVEGAME_ID)
		return false;

	// Read in the version
	header._version = in->readByte();

	// Check that the save version isn't newer than this binary
	if (header._version > SAVEGAME_VERSION)
		return false;

	// Read in the save name
	Common::String saveName;
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		saveName += ch;
	header._descr.setDescription(saveName);

	// Get the thumbnail
	Graphics::Surface *thumbnail = nullptr;
	if (!Graphics::loadThumbnail(*in, thumbnail, skipThumbnail)) {
		return false;
	}

	header._descr.setThumbnail(thumbnail);

	uint32 saveDate = in->readUint32BE();
	uint16 saveTime = in->readUint16BE();
	uint32 playTime = in->readUint32BE();

	int day = (saveDate >> 24) & 0xFF;
	int month = (saveDate >> 16) & 0xFF;
	int year = saveDate & 0xFFFF;
	header._descr.setSaveDate(year, month, day);

	int hour = (saveTime >> 8) & 0xFF;
	int minutes = saveTime & 0xFF;
	header._descr.setSaveTime(hour, minutes);

	header._descr.setPlayTime(playTime * 1000);
	if (g_engine)
		g_engine->setTotalPlayTime(playTime * 1000);

	return true;
}

bool LabEngine::saveGame(int slot, const Common::String desc) {
	Common::String fileName = getSaveStateName(slot);
	Common::SaveFileManager *saveFileManager = _system->getSavefileManager();
	Common::OutSaveFile *file = saveFileManager->openForSaving(fileName);

	if (!file)
		return false;

	// Load scene pic
	_graphics->readPict(getPictName(false));


	writeSaveGameHeader(file, desc);
	file->writeUint16LE(_roomNum);
	file->writeUint16LE(getDirection());
	file->writeUint16LE(getQuarters());

	// Conditions
	for (int i = 0; i < _conditions->_lastElement / (8 * 2); i++)
		file->writeUint16LE(_conditions->_array[i]);

	// Rooms found
	for (int i = 0; i < _roomsFound->_lastElement / (8 * 2); i++)
		file->writeUint16LE(_roomsFound->_array[i]);

	_specialLocks->save(file);

	// Breadcrumbs
	for (uint i = 0; i < MAX_CRUMBS; i++) {
		file->writeUint16LE(_breadCrumbs[i]._crumbRoomNum);
		file->writeUint16LE(_breadCrumbs[i]._crumbDirection);
	}

	file->flush();
	file->finalize();
	delete file;

	_mainDisplay = true;
	_alternate = false;
	_event->simulateEvent();
	_graphics->screenUpdate();

	return true;
}

bool LabEngine::loadGame(int slot) {
	Common::String fileName = getSaveStateName(slot);
	Common::SaveFileManager *saveFileManager = _system->getSavefileManager();
	Common::InSaveFile *file = saveFileManager->openForLoading(fileName);

	if (!file)
		return false;

	SaveGameHeader header;
	if (!readSaveGameHeader(file, header)) {
		delete file;
		return false;
	}

	_roomNum = file->readUint16LE();
	_music->checkRoomMusic(1, _roomNum);
	_direction = file->readUint16LE();
	setQuarters(file->readUint16LE());

	// Conditions
	for (int i = 0; i < _conditions->_lastElement / (8 * 2); i++)
		_conditions->_array[i] = file->readUint16LE();

	// Rooms found
	for (int i = 0; i < _roomsFound->_lastElement / (8 * 2); i++)
		_roomsFound->_array[i] = file->readUint16LE();

	_specialLocks->load(file);

	// Breadcrumbs
	for (int i = 0; i < MAX_CRUMBS; i++) {
		_breadCrumbs[i]._crumbRoomNum = file->readUint16LE();
		_breadCrumbs[i]._crumbDirection = file->readUint16LE();
	}

	_droppingCrumbs = (_breadCrumbs[0]._crumbRoomNum != 0);
	_followingCrumbs = false;

	for (int i = 0; i < MAX_CRUMBS; i++) {
		if (_breadCrumbs[i]._crumbRoomNum == 0)
			break;
		_numCrumbs = i;
	}

	delete file;

	_curFileName = " ";
	_closeDataPtr = nullptr;
	_followingCrumbs = false;
	_graphics->_longWinInFront = false;
	_event->initMouse();

	_mainDisplay = true;
	_alternate = false;
	_event->simulateEvent();
	_graphics->screenUpdate();

	return true;
}

bool LabEngine::saveRestoreGame() {
	bool isOK = false;

	// The original had one screen for saving/loading. We have two.
	// Ask the user which screen to use.
	GUI::MessageDialog saveOrLoad(_("Would you like to save or restore a game?"), _("Save"), _("Restore"));

	int choice = saveOrLoad.runModal();
	if (choice == GUI::kMessageOK) {
		// Save
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
		int slot = dialog->runModalWithCurrentTarget();
		if (slot >= 0) {
			Common::String desc = dialog->getResultString();

			if (desc.empty()) {
				// create our own description for the saved game, the user didn't enter it
				desc = dialog->createDefaultSaveDescription(slot);
			}

			isOK = saveGame(slot, desc);
		}
		delete dialog;
	} else {
		// Restore
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		int slot = dialog->runModalWithCurrentTarget();
		if (slot >= 0) {
			isOK = loadGame(slot);
		}
		delete dialog;
	}

	return isOK;
}

} // End of namespace Lab
