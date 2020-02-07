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

#include "common/textconsole.h"
#include "common/translation.h"

#include "engines/savestate.h"
#include "graphics/thumbnail.h"
#include "gui/message.h"
#include "gui/saveload.h"

#include "drascula/drascula.h"

namespace Drascula {

#define MAGIC_HEADER 0xD6A55A57		// (D)rascula (GA)me (S)cummVM (SA)ve (ST)ate
#define SAVEGAME_VERSION 1

void DrasculaEngine::checkForOldSaveGames() {
	Common::String indexFileName = Common::String::format("%s.epa", _targetName.c_str());
	Common::InSaveFile *indexFile = _saveFileMan->openForLoading(indexFileName);

	// Check for the existence of an old index file
	if (!indexFile)
		return;

	GUI::MessageDialog dialog0(
	    _("ScummVM found that you have old saved games for Drascula that should be converted.\n"
	      "The old saved game format is no longer supported, so you will not be able to load your games if you don't convert them.\n\n"
	      "Press OK to convert them now, otherwise you will be asked again the next time you start the game.\n"), _("OK"), _("Cancel"));

	int choice = dialog0.runModal();
	if (choice == GUI::kMessageCancel)
		return;

	// Convert every save slot we find in the index file to the new format
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern = Common::String::format("%s??", _targetName.c_str());

	// Get list of savefiles for target game
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);
	Common::Array<int> slots;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		// Ensure save slot is within valid range
		if (slotNum >= 1 && slotNum <= 10) {
			slots.push_back(slotNum);
		}
	}

	// Sort save slot ids
	Common::sort<int>(slots.begin(), slots.end());

	// Get savegame names from index
	Common::String saveDesc;

	int line = 1;
	for (uint i = 0; i < slots.size(); i++) {
		// Ignore lines corresponding to unused saveslots
		for (; line < slots[i]; line++)
			indexFile->readLine();

		// Copy the name in the line corresponding to the save slot
		saveDesc = indexFile->readLine();

		// Handle cases where the save directory and save index are detectably out of sync
		if (saveDesc == "*")
			saveDesc = "No name specified.";

		// Increment line number to keep it in sync with slot number
		line++;

		// Convert savegame
		convertSaveGame(slots[i], saveDesc);
	}

	delete indexFile;

	// Remove index file
	_saveFileMan->removeSavefile(indexFileName);
}

SaveStateDescriptor loadMetaData(Common::ReadStream *s, int slot, bool setPlayTime) {
	uint32 sig = s->readUint32BE();
	byte version = s->readByte();

	SaveStateDescriptor desc(-1, "");	// init to an invalid save slot

	if (sig != MAGIC_HEADER || version > SAVEGAME_VERSION)
		return desc;

	// Save is valid, set its slot number
	desc.setSaveSlot(slot);

	Common::String name;
	byte size = s->readByte();
	for (int i = 0; i < size; ++i)
		name += s->readByte();
	desc.setDescription(name);

	uint32 saveDate = s->readUint32LE();
	int day = (saveDate >> 24) & 0xFF;
	int month = (saveDate >> 16) & 0xFF;
	int year = saveDate & 0xFFFF;
	desc.setSaveDate(year, month, day);

	uint16 saveTime = s->readUint16LE();
	int hour = (saveTime >> 8) & 0xFF;
	int minutes = saveTime & 0xFF;
	desc.setSaveTime(hour, minutes);

	uint32 playTime = s->readUint32LE();
	desc.setPlayTime(playTime * 1000);
	if (setPlayTime)
		g_engine->setTotalPlayTime(playTime * 1000);

	return desc;
}

void saveMetaData(Common::WriteStream *s, const Common::String &desc) {
	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;

	s->writeUint32BE(MAGIC_HEADER);
	s->writeByte(SAVEGAME_VERSION);
	s->writeByte(desc.size());
	s->writeString(desc);
	s->writeUint32LE(saveDate);
	s->writeUint16LE(saveTime);
	s->writeUint32LE(playTime);
}

void DrasculaEngine::convertSaveGame(int slot, const Common::String &desc) {
	Common::String oldFileName = Common::String::format("%s%02d", _targetName.c_str(), slot);
	Common::String newFileName = Common::String::format("%s.%03d", _targetName.c_str(), slot);
	Common::InSaveFile *oldFile = _saveFileMan->openForLoading(oldFileName);
	if (!oldFile)
		error("Can't open %s", oldFileName.c_str());
	Common::OutSaveFile *newFile = _saveFileMan->openForSaving(newFileName);
	if (!newFile)
		error("Can't open %s", newFileName.c_str());

	// Read data from old file
	int32 dataSize = oldFile->size();
	byte *buffer = new byte[dataSize];
	oldFile->read(buffer, dataSize);

	// First, write the appropriate meta data in the new file
	saveMetaData(newFile, desc);
	Graphics::saveThumbnail(*newFile);	// basically, at this point this will capture a black screen

	// And then attach the actual save data
	newFile->write(buffer, dataSize);
	newFile->finalize();
	if (newFile->err())
		warning("Can't write file '%s'. (Disk full?)", newFileName.c_str());

	delete[] buffer;
	delete newFile;
	delete oldFile;

	// Remove old save file
	_saveFileMan->removeSavefile(oldFileName);
}

Common::Error DrasculaEngine::loadGameState(int slot) {
	// The boolean returned by loadGame() indicates if loading is in the same
	// chapter or in a different one. Thus it does not indicate an error.
	loadGame(slot);
	return Common::kNoError;
}

bool DrasculaEngine::canLoadGameStateCurrently() {
	return _canSaveLoad;
}

Common::Error DrasculaEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	saveGame(slot, desc);
	return Common::kNoError;
}

bool DrasculaEngine::canSaveGameStateCurrently() {
	return _canSaveLoad;
}

/**
 * Loads the first 10 save names, to be used in Drascula's save/load screen
 */
void DrasculaEngine::loadSaveNames() {
	Common::String saveFileName;
	Common::InSaveFile *in;

	for (int n = 0; n < NUM_SAVES; n++) {
		saveFileName = Common::String::format("%s.%03d", _targetName.c_str(), n + 1);
		if ((in = _saveFileMan->openForLoading(saveFileName))) {
			SaveStateDescriptor desc = loadMetaData(in, n + 1, false);
			_saveNames[n] = desc.getDescription();
			delete in;
		}
	}
}

void DrasculaEngine::saveGame(int slot, const Common::String &desc) {
	Common::OutSaveFile *out;
	int l;

	Common::String saveFileName = getSaveStateName(slot);
	if (!(out = _saveFileMan->openForSaving(saveFileName))) {
		error("Unable to open the file");
	}

	saveMetaData(out, desc);
	Graphics::saveThumbnail(*out);

	// Actual save data follows
	out->writeSint32LE(currentChapter);
	out->write(currentData, 20);
	out->writeSint32LE(curX);
	out->writeSint32LE(curY);
	out->writeSint32LE(trackProtagonist);

	for (l = 1; l < ARRAYSIZE(inventoryObjects); l++) {
		out->writeSint32LE(inventoryObjects[l]);
	}

	for (l = 0; l < NUM_FLAGS; l++) {
		out->writeSint32LE(flags[l]);
	}

	out->writeSint32LE(takeObject);
	out->writeSint32LE(pickedObject);

	out->finalize();
	if (out->err())
		warning("Can't write file '%s'. (Disk full?)", saveFileName.c_str());

	delete out;
}

bool DrasculaEngine::loadGame(int slot) {
	int l, savedChapter, roomNum = 0;
	Common::InSaveFile *in;

	previousMusic = roomMusic;
	_menuScreen = false;
	if (currentChapter != 1)
		clearRoom();

	Common::String saveFileName = getSaveStateName(slot);
	if (!(in = _saveFileMan->openForLoading(saveFileName))) {
		error("missing savegame file %s", saveFileName.c_str());
	}

	// If we currently are in room 102 while being attached below the pendulum
	// the character is invisible and some surface are temporarily used for other
	// things. Reset those before loading the savegame otherwise we may have some
	// issues such as the protagonist being invisible after reloading a savegame.
	if (_roomNumber == 102 && flags[1] == 2) {
		_characterVisible = true;
		loadPic(96, frontSurface);
		loadPic(97, frontSurface);
		loadPic(97, extraSurface);
		loadPic(99, backSurface);
	}


	loadMetaData(in, slot, true);
	Graphics::skipThumbnail(*in);

	savedChapter = in->readSint32LE();
	if (savedChapter != currentChapter) {
		_currentSaveSlot = slot;
		currentChapter = savedChapter - 1;
		_loadedDifferentChapter = true;
		delete in;
		return false;
	}

	in->read(currentData, 20);
	curX = in->readSint32LE();
	curY = in->readSint32LE();
	trackProtagonist = in->readSint32LE();

	for (l = 1; l < ARRAYSIZE(inventoryObjects); l++) {
		inventoryObjects[l] = in->readSint32LE();
	}

	for (l = 0; l < NUM_FLAGS; l++) {
		flags[l] = in->readSint32LE();
	}

	takeObject = in->readSint32LE();
	pickedObject = in->readSint32LE();
	_loadedDifferentChapter = false;
	if (!sscanf(currentData, "%d.ald", &roomNum))
		error("Bad save format");

	// When loading room 102 while being attached below the pendulum Some variables
	// are not correctly set and can cause random crashes when calling enterRoom below.
	// The crash occurs in moveCharacters() when accessing factor_red[curY + curHeight].
	if (roomNum == 102 && flags[1] == 2) {
		curX = 103;
		curY = 108;
		curWidth = curHeight = 0;
	}

	enterRoom(roomNum);
	selectVerb(kVerbNone);

	// When loading room 102 while being attached below the pendulum we
	// need to call activatePendulum() to properly initialized the scene.
	if (_roomNumber == 102 && flags[1] == 2)
		activatePendulum();

	return true;
}

Common::String DrasculaEngine::enterName(Common::String &selectedName) {
	Common::KeyCode key;
	Common::String inputLine = selectedName;

	flushKeyBuffer();
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	while (!shouldQuit()) {
		copyBackground(115, 14, 115, 14, 176, 9, bgSurface, screenSurface);
		print_abc((inputLine + "-").c_str(), 117, 15);
		updateScreen();

		key = getScan();

		if (key != 0) {
			if (key >= 0 && key <= 0xFF && isAlpha(key)) {
				inputLine += tolower(key);
			} else if ((key >= Common::KEYCODE_0 && key <= Common::KEYCODE_9) || key == Common::KEYCODE_SPACE) {
				inputLine += key;
			} else if (key == Common::KEYCODE_ESCAPE) {
				inputLine.clear();
				break;
			} else if (key == Common::KEYCODE_RETURN) {
				break;
			} else if (key == Common::KEYCODE_BACKSPACE) {
				inputLine.deleteLastChar();
			}
		}
	}

	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	return inputLine;
}

bool DrasculaEngine::scummVMSaveLoadDialog(bool isSave) {
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	if (isSave) {
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

		slot = dialog->runModalWithCurrentTarget();
		desc = dialog->getResultString();

		if (desc.empty()) {
			// create our own description for the saved game, the user didnt enter it
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
		return true;

	if (isSave) {
		saveGame(slot, desc);
		return true;
	} else {
		return loadGame(slot);
	}
}

bool DrasculaEngine::saveLoadScreen() {
	int n, selectedSlot = 0;
	Common::String selectedName;

	clearRoom();
	loadPic("savescr.alg", bgSurface, HALF_PAL);
	color_abc(kColorLightGreen);
	setCursor(kCursorCrosshair);
	loadSaveNames();

	while (!shouldQuit()) {
		copyBackground();
		for (n = 0; n < NUM_SAVES; n++) {
			print_abc(_saveNames[n].c_str(), 116, 27 + 9 * n);
		}
		print_abc(selectedName.c_str(), 117, 15);

		updateScreen();
		updateEvents();

		if (_leftMouseButton == 1) {
			// Check if the user has clicked on a save slot
			for (n = 0; n < NUM_SAVES; n++) {
				if (_mouseX > 115 && _mouseY > 27 + (9 * n) && _mouseX < 115 + 175 && _mouseY < 27 + 10 + (9 * n)) {
					selectedSlot = n;
					selectedName = _saveNames[selectedSlot];
					if (selectedName.empty()) {
						selectedName = enterName(selectedName);
						if (!selectedName.empty())
							_saveNames[selectedSlot] = selectedName;	// update save name
					}
					break;
				}
			}

			// Check if the user has clicked in the text area above the save slots
			if (_mouseX > 117 && _mouseY > 15 && _mouseX < 295 && _mouseY < 24 && !selectedName.empty()) {
				selectedName = enterName(selectedName);
				if (!selectedName.empty())
					_saveNames[selectedSlot] = selectedName;	// update save name
			}

			// Check if the user has clicked a button
			if (_mouseX > 208 && _mouseY > 123 && _mouseX < 282 && _mouseY < 149) {
				// "Save" button
				if (selectedName.empty()) {
					print_abc("Please select a slot", 117, 15);
					updateScreen();
					delay(200);
				} else {
					selectVerb(kVerbNone);
					clearRoom();
					loadPic(_roomNumber, bgSurface, HALF_PAL);
					updateRoom();
					updateScreen();

					saveGame(selectedSlot + 1, _saveNames[selectedSlot]);
					return true;
				}
			} else if (_mouseX > 125 && _mouseY > 123 && _mouseX < 199 && _mouseY < 149) {
				// "Load" button
				if (selectedName.empty()) {
					print_abc("Please select a slot", 117, 15);
					updateScreen();
					delay(200);
				} else {
					return loadGame(selectedSlot + 1);
				}
			} else if (_mouseX > 168 && _mouseY > 154 && _mouseX < 242 && _mouseY < 180) {
				// "Play" button
				break;
			}
		}	// if (_leftMouseButton == 1)

		_leftMouseButton = 0;
		delay(10);
	}

	selectVerb(kVerbNone);
	clearRoom();
	loadPic(_roomNumber, bgSurface, HALF_PAL);
	return true;
}

} // End of namespace Drascula
