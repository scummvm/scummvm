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

#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "gui/saveload.h"
#include "gui/message.h"

#include "parallaction/parallaction.h"
#include "parallaction/saveload.h"
#include "parallaction/sound.h"

/* Nippon Safes savefiles are called 'nippon.000' to 'nippon.099'.
 *
 * A special savefile named 'nippon.999' holds information on whether the user completed one or more parts of the game.
 */

#define NUM_SAVESLOTS		100
#define SPECIAL_SAVESLOT	999

namespace Parallaction {

Common::String SaveLoad::genSaveFileName(uint slot) {
	assert(slot < NUM_SAVESLOTS || slot == SPECIAL_SAVESLOT);

	char s[20];
	Common::sprintf_s(s, "%s.%.3u", _saveFilePrefix.c_str(), slot);

	return Common::String(s);
}

Common::InSaveFile *SaveLoad::getInSaveFile(uint slot) {
	Common::String name = genSaveFileName(slot);
	return _saveFileMan->openForLoading(name);
}

Common::OutSaveFile *SaveLoad::getOutSaveFile(uint slot) {
	Common::String name = genSaveFileName(slot);
	return _saveFileMan->openForSaving(name);
}

void SaveLoad_ns::doLoadGame(uint16 slot) {
	_vm->cleanupGame();

	Common::InSaveFile *f = getInSaveFile(slot);
	if (!f) return;

	Common::String s, character, location;

	// scrap the line with the savefile name
	f->readLine();

	character = f->readLine();
	location = f->readLine();

	s = f->readLine();
	_vm->_location._startPosition.x = atoi(s.c_str());

	s = f->readLine();
	_vm->_location._startPosition.y = atoi(s.c_str());

	s = f->readLine();
	_vm->_score = atoi(s.c_str());

	s = f->readLine();
	g_globalFlags = atoi(s.c_str());

	s = f->readLine();
	_vm->_numLocations = atoi(s.c_str());

	uint16 _si;
	for (_si = 0; _si < _vm->_numLocations; _si++) {
		s = f->readLine();
		Common::strlcpy(_vm->_locationNames[_si], s.c_str(), 32);

		s = f->readLine();
		_vm->_localFlags[_si] = atoi(s.c_str());
	}

	_vm->cleanInventory(false);
	ItemName name;
	uint32 value;

	for (_si = 0; _si < 30; _si++) {
		s = f->readLine();
		value = atoi(s.c_str());

		s = f->readLine();
		name = atoi(s.c_str());

		_vm->addInventoryItem(name, value);
	}

	delete f;

	// force reload of character to solve inventory
	// bugs, but it's a good maneuver anyway
	Common::strcpy_s(_vm->_characterName1, "null");

	char tmp[PATH_LEN];
	Common::sprintf_s(tmp, "%s.%s" , location.c_str(), character.c_str());
	_vm->scheduleLocationSwitch(tmp);
}

void SaveLoad_ns::doSaveGame(uint16 slot, const char* name) {
	Common::OutSaveFile *f = getOutSaveFile(slot);
	if (f == nullptr) {
		Common::U32String buf = Common::U32String::format(_("Can't save game in slot %i\n\n"), slot);
		GUI::MessageDialog dialog(buf);
		dialog.runModal();
		return;
	}

	char s[200];
	memset(s, 0, sizeof(s));

	if (!name || name[0] == '\0') {
		Common::sprintf_s(s, "default_%i", slot);
	} else {
		strncpy(s, name, 199);
	}

	f->writeString(s);
	f->writeString("\n");

	Common::sprintf_s(s, "%s\n", _vm->_char.getFullName());
	f->writeString(s);

	Common::sprintf_s(s, "%s\n", g_saveData1);
	f->writeString(s);
	Common::sprintf_s(s, "%d\n", _vm->_char._ani->getX());
	f->writeString(s);
	Common::sprintf_s(s, "%d\n", _vm->_char._ani->getY());
	f->writeString(s);
	Common::sprintf_s(s, "%d\n", _vm->_score);
	f->writeString(s);
	Common::sprintf_s(s, "%u\n", g_globalFlags);
	f->writeString(s);

	Common::sprintf_s(s, "%d\n", _vm->_numLocations);
	f->writeString(s);
	for (uint16 _si = 0; _si < _vm->_numLocations; _si++) {
		Common::sprintf_s(s, "%s\n%u\n", _vm->_locationNames[_si], _vm->_localFlags[_si]);
		f->writeString(s);
	}

	const InventoryItem *item;
	for (uint16 _si = 0; _si < 30; _si++) {
		item = _vm->getInventoryItem(_si);
		Common::sprintf_s(s, "%u\n%d\n", item->_id, item->_index);
		f->writeString(s);
	}

	delete f;
}

int SaveLoad::selectSaveFile(Common::String &selectedName, bool saveMode) {
	GUI::SaveLoadChooser slc(saveMode);

	selectedName.clear();

	int idx = slc.runModalWithCurrentTarget();
	if (idx >= 0) {
		selectedName = slc.getResultString();
	}

	return idx;
}

bool SaveLoad::loadGame() {
	Common::String null;
	int _di = selectSaveFile(null, false);
	if (_di == -1) {
		return false;
	}

	doLoadGame(_di);

	GUI::TimedMessageDialog dialog(_("Loading game..."), 1500);
	dialog.runModal();

	return true;
}

bool SaveLoad::saveGame() {
	Common::String saveName;
	int slot = selectSaveFile(saveName, true);
	if (slot == -1) {
		return false;
	}

	doSaveGame(slot, saveName.c_str());

	GUI::TimedMessageDialog dialog(_("Saving game..."), 1500);
	dialog.runModal();

	return true;
}

bool SaveLoad_ns::saveGame() {
	// NOTE: shouldn't this check be done before, so that the
	// user can't even select 'save'?
	if (!scumm_stricmp(_vm->_location._name, "caveau")) {
		return false;
	}

	return SaveLoad::saveGame();
}

void SaveLoad_ns::setPartComplete(const char *part) {
	Common::String s;
	bool alreadyPresent = false;

	Common::InSaveFile *inFile = getInSaveFile(SPECIAL_SAVESLOT);
	if (inFile) {
		s = inFile->readLine();
		delete inFile;

		if (s.contains(part)) {
			alreadyPresent = true;
		}
	}

	if (!alreadyPresent) {
		Common::OutSaveFile *outFile = getOutSaveFile(SPECIAL_SAVESLOT);
		outFile->writeString(s);
		outFile->writeString(part);
		outFile->finalize();
		delete outFile;
	}
}

void SaveLoad_ns::getGamePartProgress(bool *complete, int size) {
	assert(complete && size >= 3);

	Common::InSaveFile *inFile = getInSaveFile(SPECIAL_SAVESLOT);
	Common::String s = inFile->readLine();
	delete inFile;

	complete[0] = s.contains("dino");
	complete[1] = s.contains("donna");
	complete[2] = s.contains("dough");
}

static bool askRenameOldSavefiles() {
	GUI::MessageDialog dialog0(
		_("ScummVM found that you have old saved games for Nippon Safes that should be renamed.\n"
		"The old names are no longer supported, so you will not be able to load your games if you don't convert them.\n\n"
		"Press OK to convert them now, otherwise you will be asked next time.\n"), _("OK"), _("Cancel"));

	return (dialog0.runModal() == GUI::kMessageOK);
}

void SaveLoad_ns::renameOldSavefiles() {
	Common::StringArray oldFilenames = _saveFileMan->listSavefiles("game.*");
	uint numOldSaves = oldFilenames.size();

	bool rename = false;
	uint success = 0, id;
	Common::String oldName, newName;
	for (uint i = 0; i < oldFilenames.size(); ++i) {
		oldName = oldFilenames[i];
		int e = sscanf(oldName.c_str(), "game.%u", &id);
		if (e != 1) {
			// this wasn't a savefile, so adjust numOldSaves accordingly
			--numOldSaves;
			continue;
		}

		if (!rename) {
			rename = askRenameOldSavefiles();
		}
		if (!rename) {
			// return immediately if the user doesn't want to rename the files
			return;
		}

		newName = genSaveFileName(id);
		if (_saveFileMan->renameSavefile(oldName, newName)) {
			success++;
		} else {
			warning("Error %i (%s) occurred while renaming %s to %s", _saveFileMan->getError().getCode(),
				_saveFileMan->getErrorDesc().c_str(), oldName.c_str(), newName.c_str());
		}
	}

	if (numOldSaves == 0) {
		// there were no old savefiles: nothing to notify
		return;
	}

	Common::U32String msg;
	if (success == numOldSaves) {
		msg = _("ScummVM successfully converted all your saved games.");
	} else {
		msg = _("ScummVM printed some warnings in your console window and can't guarantee all your files have been converted.\n\n"
			"Please report to the team.");
	}

	GUI::MessageDialog dialog1(msg);
	dialog1.runModal();
}

void SaveLoad_br::doLoadGame(uint16 slot) {
	_vm->loadGameState(slot);
}

// The menus stay open if the game was not loaded
bool SaveLoad_br::loadGame() {
	Common::String null;
	int slot = selectSaveFile(null, false);
	if (slot == -1) {
		return false;
	}

	Common::Error result = _vm->loadGameState(slot);
	if (result.getCode() != Common::kNoError) {
		GUI::MessageDialog dialog(result.getDesc());
		dialog.runModal();
		return false;
	}

	return true;
}

void SaveLoad_br::doSaveGame(uint16 slot, const char* name) {
	Common::Error result = _vm->saveGameState(slot, name);
	if (result.getCode() != Common::kNoError) {
		GUI::MessageDialog dialog(result.getDesc());
		dialog.runModal();
	}
}

static const uint32 kSavegameVersion = 1;
static const int16 kNoFollowerPosition = -1000;

static void writeSaveString(Common::WriteStream *stream, const Common::String &str) {
	stream->writeString(str);
	stream->writeByte(0);
}

// Same data as savegame_ in the original
Common::Error Parallaction_br::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	// store the flags of the current location first
	ZoneList::iterator zit = _location._zones.begin();
	for ( ; zit != _location._zones.end(); ++zit) {
		restoreOrSaveZoneFlags(*zit, false);
	}
	AnimationList::iterator ait = _location._animations.begin();
	for ( ; ait != _location._animations.end(); ++ait) {
		restoreOrSaveZoneFlags(*ait, false);
	}

	stream->writeUint32LE(kSavegameVersion);
	stream->writeSint32LE(_part);
	writeSaveString(stream, _locationNames[_currentLocationIndex]);
	writeSaveString(stream, _char.getName());

	Common::Point foot;
	_char._ani->getFoot(foot);
	stream->writeSint16LE(foot.x);
	stream->writeSint16LE(foot.y);
	stream->writeSint16LE(_char._ani->getF());

	int16 followerFrame = 0;
	foot.x = foot.y = kNoFollowerPosition;
	if (_follower) {
		_follower->getFoot(foot);
		followerFrame = _follower->getF();
	}
	writeSaveString(stream, _followerName);
	stream->writeSint16LE(foot.x);
	stream->writeSint16LE(foot.y);
	stream->writeSint16LE(followerFrame);

	stream->writeUint32LE(g_globalFlags);

	stream->writeUint16LE(_numLocations);
	for (uint i = 0; i < _numLocations; i++) {
		writeSaveString(stream, _locationNames[i]);
		stream->writeUint32LE(_localFlags[i]);
	}
	for (uint i = 0; i < _numLocations; i++) {
		for (uint j = 0; j < NUM_ZONES; j++)
			stream->writeUint32LE(_zoneFlags[i][j]);
	}

	for (uint i = 0; i < ARRAYSIZE(_counters); i++)
		stream->writeSint32LE(_counters[i]);

	Inventory *inventories[] = { _dinoInventory, _dougInventory, _donnaInventory };
	for (uint i = 0; i < ARRAYSIZE(inventories); i++) {
		stream->writeSint16LE(inventories[i]->getNumItems());
		for (int j = 0; j < inventories[i]->getNumItems(); j++) {
			const InventoryItem *item = inventories[i]->getItem(j);
			stream->writeUint32LE(item->_id);
			stream->writeUint16LE(item->_index);
		}
	}

	return stream->err() ? Common::kWritingFailed : Common::kNoError;
}

// As restoregame_ in the original: reload the part, then the saved state
Common::Error Parallaction_br::loadGameStream(Common::SeekableReadStream *stream) {
	if (stream->readUint32LE() != kSavegameVersion)
		return Common::Error(Common::kReadingFailed, "unknown savegame version");

	int part = stream->readSint32LE();
	Common::String location = stream->readString();
	Common::String character = stream->readString();
	if (stream->err() || stream->eos())
		return Common::kReadingFailed;

	_soundMan->execute(SC_SETMUSICFILE, "");
	loadPart(part);

	_location._startPosition.x = stream->readSint16LE();
	_location._startPosition.y = stream->readSint16LE();
	_location._startFrame = stream->readSint16LE();

	_followerName = stream->readString();
	_location._followerStartPosition.x = stream->readSint16LE();
	_location._followerStartPosition.y = stream->readSint16LE();
	_location._followerStartFrame = stream->readSint16LE();

	g_globalFlags = stream->readUint32LE();

	_numLocations = MIN<uint16>(stream->readUint16LE(), NUM_LOCATIONS);
	for (uint i = 0; i < _numLocations; i++) {
		Common::strlcpy(_locationNames[i], stream->readString().c_str(), sizeof(_locationNames[i]));
		_localFlags[i] = stream->readUint32LE();
	}
	for (uint i = 0; i < _numLocations; i++) {
		for (uint j = 0; j < NUM_ZONES; j++)
			_zoneFlags[i][j] = stream->readUint32LE();
	}

	for (uint i = 0; i < ARRAYSIZE(_counters); i++)
		_counters[i] = stream->readSint32LE();

	Inventory *inventories[] = { _dinoInventory, _dougInventory, _donnaInventory };
	for (uint i = 0; i < ARRAYSIZE(inventories); i++) {
		inventories[i]->clear(false);
		int16 numItems = stream->readSint16LE();
		for (int j = 0; j < numItems; j++) {
			uint32 value = stream->readUint32LE();
			ItemName name = stream->readUint16LE();
			inventories[i]->addItem(name, value);
		}
	}

	changeCharacter(character.c_str());

	// the zones of common.slf stay loaded
	ZoneList::iterator zit = _location._zones.begin();
	for ( ; zit != _location._zones.end(); ++zit) {
		restoreOrSaveZoneFlags(*zit, true);
	}
	AnimationList::iterator ait = _location._animations.begin();
	for ( ; ait != _location._animations.end(); ++ait) {
		restoreOrSaveZoneFlags(*ait, true);
	}

	scheduleLocationSwitch(location.c_str());
	return Common::kNoError;
}

void SaveLoad_br::getGamePartProgress(bool *complete, int size) {
	assert(complete && size >= 3);

	// TODO: implement progress loading

	complete[0] = true;
	complete[1] = true;
	complete[2] = true;
}

void SaveLoad_br::setPartComplete(const char *part) {
	// TODO: implement progress saving
}

} // namespace Parallaction
