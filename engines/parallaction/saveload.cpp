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
 * $URL$
 * $Id$
 *
 */

#include "common/savefile.h"

#include "gui/dialog.h"
#include "gui/widget.h"
#include "gui/ListWidget.h"
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



class SaveLoadChooser : public GUI::Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
protected:
	GUI::ListWidget		*_list;
	GUI::ButtonWidget	*_chooseButton;
	GUI::ContainerWidget	*_container;

public:
	SaveLoadChooser(const String &title, const String &buttonLabel);
	~SaveLoadChooser();

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	const String &getResultString() const;
	void setList(const StringList& list);
	int runModal();

	virtual void reflowLayout();
};

Common::String SaveLoad::genSaveFileName(uint slot) {
	assert(slot < NUM_SAVESLOTS || slot == SPECIAL_SAVESLOT);

	char s[20];
	sprintf(s, "%s.%.3d", _saveFilePrefix.c_str(), slot);

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
	_globalFlags = atoi(s.c_str());

	s = f->readLine();
	_vm->_numLocations = atoi(s.c_str());

	uint16 _si;
	for (_si = 0; _si < _vm->_numLocations; _si++) {
		s = f->readLine();
		strcpy(_vm->_locationNames[_si], s.c_str());

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
	strcpy(_vm->_characterName1, "null");

	char tmp[PATH_LEN];
	sprintf(tmp, "%s.%s" , location.c_str(), character.c_str());
	_vm->scheduleLocationSwitch(tmp);

	return;
}


void SaveLoad_ns::doSaveGame(uint16 slot, const char* name) {

	Common::OutSaveFile *f = getOutSaveFile(slot);
	if (f == 0) {
		char buf[32];
		sprintf(buf, "Can't save game in slot %i\n\n", slot);
		GUI::MessageDialog dialog(buf);
		dialog.runModal();
		return;
	}

	char s[200];
	memset(s, 0, sizeof(s));

	if (!name || name[0] == '\0') {
		sprintf(s, "default_%i", slot);
	} else {
		strncpy(s, name, 199);
	}

	f->writeString(s);
	f->writeString("\n");

	sprintf(s, "%s\n", _vm->_char.getFullName());
	f->writeString(s);

	sprintf(s, "%s\n", _saveData1);
	f->writeString(s);
	sprintf(s, "%d\n", _vm->_char._ani->getX());
	f->writeString(s);
	sprintf(s, "%d\n", _vm->_char._ani->getY());
	f->writeString(s);
	sprintf(s, "%d\n", _vm->_score);
	f->writeString(s);
	sprintf(s, "%u\n", _globalFlags);
	f->writeString(s);

	sprintf(s, "%d\n", _vm->_numLocations);
	f->writeString(s);
	for (uint16 _si = 0; _si < _vm->_numLocations; _si++) {
		sprintf(s, "%s\n%u\n", _vm->_locationNames[_si], _vm->_localFlags[_si]);
		f->writeString(s);
	}

	const InventoryItem *item;
	for (uint16 _si = 0; _si < 30; _si++) {
		item = _vm->getInventoryItem(_si);
		sprintf(s, "%u\n%d\n", item->_id, item->_index);
		f->writeString(s);
	}

	delete f;

	return;
}

enum {
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kPlayCmd = 'PLAY',
	kOptionsCmd = 'OPTN',
	kHelpCmd = 'HELP',
	kAboutCmd = 'ABOU',
	kQuitCmd = 'QUIT',
	kChooseCmd = 'CHOS'
};



SaveLoadChooser::SaveLoadChooser(const String &title, const String &buttonLabel)
	: Dialog("ScummSaveLoad"), _list(0), _chooseButton(0) {

//	_drawingHints |= GUI::THEME_HINT_SPECIAL_COLOR;
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;

	new GUI::StaticTextWidget(this, "ScummSaveLoad.Title", title);

	// Add choice list
	_list = new GUI::ListWidget(this, "ScummSaveLoad.List");
	_list->setEditable(true);
	_list->setNumberingMode(GUI::kListNumberingOne);

	// Buttons
	new GUI::ButtonWidget(this, "ScummSaveLoad.Cancel", "Cancel", GUI::kCloseCmd, 0);
	_chooseButton = new GUI::ButtonWidget(this, "ScummSaveLoad.Choose", buttonLabel, kChooseCmd, 0);
	_chooseButton->setEnabled(false);
}

SaveLoadChooser::~SaveLoadChooser() {
}

const Common::String &SaveLoadChooser::getResultString() const {
	return _list->getSelectedString();
}

void SaveLoadChooser::setList(const StringList& list) {
	_list->setList(list);
}

int SaveLoadChooser::runModal() {
	return GUI::Dialog::runModal();
}

void SaveLoadChooser::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	int selItem = _list->getSelected();
	switch (cmd) {
	case GUI::kListItemActivatedCmd:
	case GUI::kListItemDoubleClickedCmd:
		if (selItem >= 0) {
			if (!getResultString().empty()) {
				_list->endEditMode();
				setResult(selItem);
				close();
			}
		}
		break;
	case kChooseCmd:
		_list->endEditMode();
		setResult(selItem);
		close();
		break;
	case GUI::kListSelectionChangedCmd: {
		_list->startEditMode();
		// Disable button if nothing is selected, or (in load mode) if an empty
		// list item is selected. We allow choosing an empty item in save mode
		// because we then just assign a default name.
		_chooseButton->setEnabled(selItem >= 0 && (!getResultString().empty()));
		_chooseButton->draw();
	} break;
	case GUI::kCloseCmd:
		setResult(-1);
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadChooser::reflowLayout() {
	Dialog::reflowLayout();
}


int SaveLoad::buildSaveFileList(Common::StringList& l) {
	Common::String pattern = _saveFilePrefix + ".???";
	Common::StringList filenames = _saveFileMan->listSavefiles(pattern);

	Common::String s;

	int count = 0;

	for (int i = 0; i < NUM_SAVESLOTS; i++) {
		s.clear();

		Common::InSaveFile *f = getInSaveFile(i);
		if (f) {
			s = f->readLine();
			count++;
		}

		delete f;
		l.push_back(s);
	}

	return count;
}


int SaveLoad::selectSaveFile(Common::String &selectedName, const Common::String &caption, const Common::String &button) {
	Common::StringList list;
	buildSaveFileList(list);

	SaveLoadChooser slc(caption, button);
	slc.setList(list);

	selectedName.clear();

	int slot = slc.runModal();
	if (slot >= 0) {
		selectedName = slc.getResultString();
	}

	return slot;
}



bool SaveLoad::loadGame() {
	Common::String null;
	int _di = selectSaveFile(null, "Load file", "Load");
	if (_di == -1) {
		return false;
	}

	doLoadGame(_di);

	GUI::TimedMessageDialog dialog("Loading game...", 1500);
	dialog.runModal();

	_vm->_input->setArrowCursor();

	return true;
}


bool SaveLoad::saveGame() {
	Common::String saveName;
	int slot = selectSaveFile(saveName, "Save file", "Save");
	if (slot == -1) {
		return false;
	}

	doSaveGame(slot, saveName.c_str());

	GUI::TimedMessageDialog dialog("Saving game...", 1500);
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

	return;
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

void SaveLoad_ns::renameOldSavefiles() {
	Common::StringList oldFilenames = _saveFileMan->listSavefiles("game.*");

	if (oldFilenames.size() == 0) {
		// there are no old savefiles: nothing to do
		return;
	}

	GUI::MessageDialog dialog0(
		"ScummVM found that you have old savefiles for Nippon Safes that should be renamed.\n"
		"The old names are no longer supported, so you will not be able to load your games if you don't convert them.\n\n"
		"Press OK to convert them now, otherwise you will be asked you next time.\n", "OK", "Cancel");

	int choice = dialog0.runModal();
	if (choice == 0) {
		// user pressed cancel
		return;
	}

	uint success = 0, id;
	Common::String oldName, newName;
	for (uint i = 0; i < oldFilenames.size(); ++i) {
		oldName = oldFilenames[i];
		int e = sscanf(oldName.c_str(), "game.%u", &id);
		assert(e == 1);
		newName = genSaveFileName(id);

		if (_saveFileMan->renameSavefile(oldName, newName)) {
			success++;
		} else {
			warning("Error %i (%s) occurred while renaming %s to %s", _saveFileMan->getError(),
				_saveFileMan->getErrorDesc().c_str(), oldName.c_str(), newName.c_str());
		}
	}

	char msg[200];
	if (success == oldFilenames.size()) {
		sprintf(msg, "ScummVM successfully converted all your savefiles.");
	} else {
		sprintf(msg,
			"ScummVM printed some warnings in your console window and can't guarantee all your files have been converted.\n\n"
			"Please report to the team.");
	}

	GUI::MessageDialog dialog1(msg);
	dialog1.runModal();

	return;
}


void SaveLoad_br::doLoadGame(uint16 slot) {
	// TODO: implement loadgame
	return;
}

void SaveLoad_br::doSaveGame(uint16 slot, const char* name) {
	// TODO: implement savegame
	return;
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
