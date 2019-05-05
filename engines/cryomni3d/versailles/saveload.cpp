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

#define DEBUG_SAVE

namespace CryOmni3D {
namespace Versailles {

#define SAVE_DESCRIPTION_LEN 20

Common::String CryOmni3DEngine_Versailles::getSaveFileName(bool visit, unsigned int saveNum) const {
	return Common::String::format("%s%s.%04u", _targetName.c_str(), visit ? "_visit" : "", saveNum);
}

bool CryOmni3DEngine_Versailles::canVisit() const {
	// Build a custom SearchSet
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	Common::SearchSet visitsSearchSet;
	visitsSearchSet.addSubDirectoryMatching(gameDataDir, "savegame/visite", 1);
	return visitsSearchSet.hasFile("game0001.sav");
}

void CryOmni3DEngine_Versailles::getSavesList(bool visit, Common::StringArray &saveNames) {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();

	char saveName[SAVE_DESCRIPTION_LEN + 1];
	saveName[SAVE_DESCRIPTION_LEN] = '\0';
	Common::String pattern = Common::String::format("%s%s.????", _targetName.c_str(),
	                         visit ? "_visit" : "");
	Common::StringArray filenames = saveMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	saveNames.clear();
	saveNames.reserve(100);

	int num = 1;
	int slotNum;

	if (visit) {
		// Add bootstrap visit
		const Common::FSNode gameDataDir(ConfMan.get("path"));
		Common::SearchSet visitsSearchSet;
		visitsSearchSet.addSubDirectoryMatching(gameDataDir, "savegame/visite", 1);
		if (visitsSearchSet.hasFile("game0001.sav")) {
			Common::File visitFile;
			if (!visitFile.open("game0001.sav", visitsSearchSet)) {
				error("Can't load visit file");
			}
			visitFile.read(saveName, SAVE_DESCRIPTION_LEN);
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
#ifdef DEBUG_SAVE
			Common::InSaveFile *in = _saveFileMan->openRawFile(*file);
#else
			Common::InSaveFile *in = _saveFileMan->openForLoading(*file);
#endif
			if (in) {
				if (in->read(saveName, SAVE_DESCRIPTION_LEN) == SAVE_DESCRIPTION_LEN) {
					saveNames.push_back(saveName);
				}
				delete in;
			}
		}
	}

	for (unsigned int i = saveNames.size(); i < 100; i++) {
		saveNames.push_back(_messages[55]);
	}
}

void CryOmni3DEngine_Versailles::saveGame(bool visit, unsigned int saveNum,
        const Common::String &saveName) const {
	if (visit && saveNum == 1) {
		error("Can't erase bootstrap visit");
	}

	Common::String saveFileName = getSaveFileName(visit, saveNum);

	Common::OutSaveFile *out;

	if (!(out = _saveFileMan->openForSaving(saveFileName,
#ifdef DEBUG_SAVE
	                                        false
#else
	                                        true
#endif
	                                       ))) {
		return;
	}

	// Write save name
	char saveNameC[SAVE_DESCRIPTION_LEN];
	memset(saveNameC, 0, sizeof(saveNameC));
	strncpy(saveNameC, saveName.c_str(), sizeof(saveNameC));
	out->write(saveNameC, sizeof(saveNameC));

	// dummy values
	out->writeUint32LE(0);
	out->writeUint32BE(0);
	out->writeUint32BE(0);

	// Dialog variables
	assert(_dialogsMan.size() < 200);
	for (unsigned int i = 0; i < _dialogsMan.size(); i++) {
		out->writeByte(_dialogsMan[i]);
	}
	for (unsigned int i = _dialogsMan.size(); i < 200; i++) {
		out->writeByte(0);
	}

	// Inventory
	assert(_inventory.size() == 50);
	for (Inventory::const_iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		unsigned int objId = -1;
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
	for (unsigned int i = 0; placeIt != _placeStates.end(); placeIt++, i++) {
		out->writeUint32BE(placeIt->state);
	}
	for (unsigned int i = _placeStates.size(); i < 100; i++) {
		out->writeUint32BE(0);
	}

	// Game variables
	assert(_gameVariables.size() < 100);
	for (Common::Array<unsigned int>::const_iterator it = _gameVariables.begin();
	        it != _gameVariables.end(); it++) {
		out->writeUint32BE(*it);
	}
	for (unsigned int i = _gameVariables.size(); i < 100; i++) {
		out->writeUint32BE(0);
	}

	out->finalize();

	delete out;
}

bool CryOmni3DEngine_Versailles::loadGame(bool visit, unsigned int saveNum) {
	Common::SeekableReadStream *in;

	if (visit && saveNum == 1) {
		// Load bootstrap visit
		const Common::FSNode gameDataDir(ConfMan.get("path"));
		Common::SearchSet visitsSearchSet;
		visitsSearchSet.addSubDirectoryMatching(gameDataDir, "savegame/visite", 1);
		Common::File *visitFile = new Common::File();
		if (!visitFile->open("game0001.sav", visitsSearchSet)) {
			delete visitFile;
			error("Can't load visit file");
		}
		in = visitFile;
	} else {
		Common::String saveFileName = getSaveFileName(visit, saveNum);

#ifdef DEBUG_SAVE
		in = _saveFileMan->openRawFile(saveFileName);
#else
		in = _saveFileMan->openForLoading(saveFileName);
#endif
	}

	if (!in || in->size() != 1260) {
		return false;
	}

	musicStop();

	// Load save name but don't use it
	char saveNameC[SAVE_DESCRIPTION_LEN];
	in->read(saveNameC, sizeof(saveNameC));

	// dummy values
	in->readUint32LE();
	in->readUint32BE();
	in->readUint32BE();

	// Dialog variables
	assert(_dialogsMan.size() < 200);
	for (unsigned int i = 0; i < _dialogsMan.size(); i++) {
		_dialogsMan[i] = in->readByte();
	}
	for (unsigned int i = _dialogsMan.size(); i < 200; i++) {
		in->readByte();
	}

	// Inventory
	assert(_inventory.size() == 50);
	for (Inventory::iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		unsigned int objId = in->readUint32BE();
		if (objId >= _objects.size()) {
			objId = -1;
		}
		if (objId != -1u) {
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
	for (unsigned int i = 0; i < 100; i++) {
		placesStates[i] = in->readUint32BE();
	}

	// Game variables
	assert(_gameVariables.size() < 100);
	for (Common::Array<unsigned int>::iterator it = _gameVariables.begin(); it != _gameVariables.end();
	        it++) {
		*it = in->readUint32BE();
	}
	for (unsigned int i = _gameVariables.size(); i < 100; i++) {
		in->readUint32BE();
	}

	delete in;

	if (_gameVariables[GameVariables::kCurrentTime] == 0) {
		_gameVariables[GameVariables::kCurrentTime] = 1;
	}

	// Everything has been loaded, setup new level
	// We will set places states and warp coordinates just after that to avoid them from being reset
	initNewLevel(_currentLevel);

	_omni3dMan.setAlpha(alpha);
	_omni3dMan.setBeta(beta);

	// _placeStates has just been resized in initNewLevel
	unsigned int i = 0;
	for (Common::Array<PlaceState>::iterator placeIt = _placeStates.begin();
	        placeIt != _placeStates.end() && i < ARRAYSIZE(placesStates); placeIt++, i++) {
		placeIt->state = placesStates[i];
	}

	// TODO: countdown

	return true;
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
