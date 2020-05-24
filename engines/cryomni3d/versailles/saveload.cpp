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
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"

#include "cryomni3d/versailles/engine.h"

namespace CryOmni3D {
namespace Versailles {

Common::Error CryOmni3DEngine_Versailles::loadGameState(int slot) {
	_loadedSave = slot + 1;
	_abortCommand = kAbortLoadGame;
	return Common::kNoError;
}

Common::Error CryOmni3DEngine_Versailles::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	saveGame(_isVisiting, slot + 1, desc);
	return Common::kNoError;
}

Common::String CryOmni3DEngine_Versailles::getSaveFileName(bool visit, uint saveNum) const {
	return Common::String::format("%s%s.%04u", _targetName.c_str(), visit ? "_visit" : "", saveNum);
}

Common::String CryOmni3DEngine_Versailles::getSaveStateName(int slot) const {
	return Common::String::format("%s.%04u", _targetName.c_str(), slot);
}

bool CryOmni3DEngine_Versailles::canVisit() const {
	return Common::File::exists("game0001.sav");
}

void CryOmni3DEngine_Versailles::getSavesList(bool visit, Common::StringArray &saveNames,
        int &nextSaveNum) {
	nextSaveNum = 1;
	bool supportsAutoName = (_messages.size() >= 148);

	char saveName[kSaveDescriptionLen + 1];
	// Terminate saveName here forever (we don't overrun kSaveDescriptionLen)
	saveName[kSaveDescriptionLen] = '\0';
	Common::String pattern = Common::String::format("%s%s.????", _targetName.c_str(),
	                         visit ? "_visit" : "");
	Common::StringArray filenames = _saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	saveNames.clear();
	saveNames.reserve(100);

	int num = 1;
	int slotNum;

	if (visit) {
		// Add bootstrap visit
		if (Common::File::exists("game0001.sav")) {
			Common::File visitFile;
			if (!visitFile.open("game0001.sav")) {
				error("Can't load visit file");
			}
			visitFile.read(saveName, kSaveDescriptionLen);
			saveNames.push_back(saveName);
		} else {
			warning("visiting mode but no bootstrap");
			// No bootstrap visit, too bad
			saveNames.push_back(_messages[55]); //Fill with free slot
		}
		num++;
	}

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end();
	        ++file) {
		// Obtain the last 4 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 4);

		if (slotNum >= 1 && slotNum <= 99) {
			while (num < slotNum) {
				saveNames.push_back(_messages[55]); //Fill with free slot
				num++;
			}

			num++;
			Common::InSaveFile *in = _saveFileMan->openForLoading(*file);
			if (in) {
				if (in->read(saveName, kSaveDescriptionLen) != kSaveDescriptionLen) {
					warning("getSavesList(): Corrupted save %s", saveName);
					delete in;

					continue;
				}

				Common::String saveNameStr = saveName;
				if (supportsAutoName && saveNameStr.hasPrefix("AUTO")) {
					int saveNum = atoi(saveName + 4);
					if (saveNum >= 1 && saveNum <= 9999) {
						in->seek(436); // Go to current level
						uint32 level = in->readUint32BE();

						if (level < 8) {
							saveNameStr = Common::String::format(_messages[146].c_str(), level);
						} else {
							saveNameStr = _messages[147];
						}
						saveNameStr += Common::String::format(" - %d", saveNum);
						if (saveNum >= nextSaveNum) {
							if (saveNum >= 9999) {
								nextSaveNum = 9999;
							} else {
								nextSaveNum = saveNum + 1;
							}
						}
					}
				}

				saveNames.push_back(saveNameStr);
				delete in;
			}
		}
	}

	for (uint i = saveNames.size(); i < 100; i++) {
		saveNames.push_back(_messages[55]);
	}
}

void CryOmni3DEngine_Versailles::saveGame(bool visit, uint saveNum,
        const Common::String &saveName) {
	if (visit && saveNum == 1) {
		error("Can't erase bootstrap visit");
	}

	Common::String saveFileName = getSaveFileName(visit, saveNum);

	Common::OutSaveFile *out;

	if (!(out = _saveFileMan->openForSaving(saveFileName))) {
		return;
	}

	// Sync countdown to game variable before saving it to file
	syncCountdown();

	// Write save name
	char saveNameC[kSaveDescriptionLen];
	memset(saveNameC, 0, sizeof(saveNameC));
	// Silence -Wstringop-truncation using parentheses, we don't have to have a null-terminated string here
	(strncpy(saveNameC, saveName.c_str(), sizeof(saveNameC)));
	out->write(saveNameC, sizeof(saveNameC));

	// dummy values
	out->writeUint32LE(0);
	out->writeUint32BE(0);
	out->writeUint32BE(0);

	// Dialog variables
	assert(_dialogsMan.size() < 200);
	for (uint i = 0; i < _dialogsMan.size(); i++) {
		out->writeByte(_dialogsMan[i]);
	}
	for (uint i = _dialogsMan.size(); i < 200; i++) {
		out->writeByte(0);
	}

	// Inventory
	assert(_inventory.size() == 50);
	for (Inventory::const_iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		uint objId = uint(-1);
		if (*it != nullptr) {
			// Inventory contains pointers to objects stored in _objects
			objId = *it - _objects.begin();
		}
		out->writeUint32BE(objId);
	}
	// Offset of inventory in toolbar
	out->writeUint32BE(_toolbar.inventoryOffset());

	// Level, place, warp position
	out->writeUint32BE(_currentLevel);
	out->writeUint32BE(_currentPlaceId);
	out->writeDoubleBE(_omni3dMan.getAlpha());
	out->writeDoubleBE(_omni3dMan.getBeta());

	// Places states
	assert(_placeStates.size() <= 100);
	Common::Array<PlaceState>::const_iterator placeIt = _placeStates.begin();
	for (uint i = 0; placeIt != _placeStates.end(); placeIt++, i++) {
		out->writeUint32BE(placeIt->state);
	}
	for (uint i = _placeStates.size(); i < 100; i++) {
		out->writeUint32BE(0);
	}

	// Game variables
	assert(_gameVariables.size() < 100);
	for (Common::Array<uint>::const_iterator it = _gameVariables.begin();
	        it != _gameVariables.end(); it++) {
		out->writeUint32BE(*it);
	}
	for (uint i = _gameVariables.size(); i < 100; i++) {
		out->writeUint32BE(0);
	}

	out->finalize();

	delete out;
}

bool CryOmni3DEngine_Versailles::loadGame(bool visit, uint saveNum) {
	Common::SeekableReadStream *in;

	if (visit && saveNum == 1) {
		// Load bootstrap visit
		Common::File *visitFile = new Common::File();
		if (!visitFile->open("game0001.sav")) {
			delete visitFile;
			error("Can't load visit file");
		}
		in = visitFile;
	} else {
		Common::String saveFileName = getSaveFileName(visit, saveNum);
		in = _saveFileMan->openForLoading(saveFileName);
	}

	if (!in || in->size() != 1260) {
		return false;
	}

	musicStop();

	// Load save name but don't use it
	char saveNameC[kSaveDescriptionLen];
	in->read(saveNameC, sizeof(saveNameC));

	// dummy values
	(void) in->readUint32LE();
	(void) in->readUint32BE();
	(void) in->readUint32BE();

	// Dialog variables
	assert(_dialogsMan.size() < 200);
	for (uint i = 0; i < _dialogsMan.size(); i++) {
		_dialogsMan[i] = in->readByte();
	}
	for (uint i = _dialogsMan.size(); i < 200; i++) {
		// Read the remaining bytes but don't use them
		(void) in->readByte();
	}

	// Inventory
	assert(_inventory.size() == 50);
	for (Inventory::iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		uint objId = in->readUint32BE();
		if (objId >= _objects.size()) {
			objId = uint(-1);
		}
		if (objId != uint(-1)) {
			*it = _objects.begin() + objId;
		} else {
			*it = nullptr;
		}
	}
	// Offset of inventory in toolbar
	_toolbar.setInventoryOffset(in->readUint32BE());

	// Level, place, warp position
	_currentLevel = in->readUint32BE();
	// Use nextPlace to force place move
	_nextPlaceId = in->readUint32BE();

	// Store alpha and beta for later use
	double alpha = in->readDoubleBE();
	double beta = in->readDoubleBE();

	// Places states
	// Store them and use them once we called initNewLevel, we can't call it before because it needs _gameVariables (and especially kCurrentTime) to be correctly set
	uint32 placesStates[100];
	for (uint i = 0; i < 100; i++) {
		placesStates[i] = in->readUint32BE();
	}

	// Game variables
	assert(_gameVariables.size() < 100);
	for (Common::Array<uint>::iterator it = _gameVariables.begin(); it != _gameVariables.end();
	        it++) {
		*it = in->readUint32BE();
	}
	for (uint i = _gameVariables.size(); i < 100; i++) {
		// Read the remaining variables but don't use them
		(void) in->readUint32BE();
	}

	delete in;

	if (_gameVariables[GameVariables::kCurrentTime] == 0) {
		_gameVariables[GameVariables::kCurrentTime] = 1;
	}
	initCountdown();

	// Everything has been loaded, setup new level
	// We will set places states and warp coordinates just after that to avoid them from being reset
	initNewLevel(_currentLevel);

	_omni3dMan.setAlpha(alpha);
	_omni3dMan.setBeta(beta);

	// _placeStates has just been resized in initNewLevel
	uint i = 0;
	for (Common::Array<PlaceState>::iterator placeIt = _placeStates.begin();
	        placeIt != _placeStates.end() && i < ARRAYSIZE(placesStates); placeIt++, i++) {
		placeIt->state = placesStates[i];
	}

	return true;
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
