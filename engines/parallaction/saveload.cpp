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
	GUI::GraphicsWidget	*_gfxWidget;
	GUI::StaticTextWidget	*_date;
	GUI::StaticTextWidget	*_time;
	GUI::StaticTextWidget	*_playtime;
	GUI::ContainerWidget	*_container;

	uint8 _fillR, _fillG, _fillB;

public:
	SaveLoadChooser(const String &title, const String &buttonLabel);
	~SaveLoadChooser();

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	const String &getResultString() const;
	void setList(const StringList& list);
	int runModal();

	virtual void reflowLayout();
};

Common::String SaveLoad_ns::genOldSaveFileName(uint slot) {
	assert(slot < NUM_SAVESLOTS || slot == SPECIAL_SAVESLOT);

	char s[20];
	sprintf(s, "game.%i", slot);

	return Common::String(s);
}


Common::String SaveLoad::genSaveFileName(uint slot) {
	assert(slot < NUM_SAVESLOTS || slot == SPECIAL_SAVESLOT);

	char s[20];
	sprintf(s, "%s.%.3d", _saveFilePrefix.c_str(), slot);

	return Common::String(s);
}

Common::InSaveFile *SaveLoad::getInSaveFile(uint slot) {
	Common::String name = genSaveFileName(slot);
	return _saveFileMan->openForLoading(name.c_str());
}

Common::OutSaveFile *SaveLoad::getOutSaveFile(uint slot) {
	Common::String name = genSaveFileName(slot);
	return _saveFileMan->openForSaving(name.c_str());
}


void SaveLoad_ns::doLoadGame(uint16 slot) {

	_vm->cleanupGame();

	Common::InSaveFile *f = getInSaveFile(slot);
	if (!f) return;

	char s[200];
	char n[16];
	char l[16];

	f->readLine_OLD(s, 199);

	f->readLine_OLD(n, 15);

	f->readLine_OLD(l, 15);

	f->readLine_OLD(s, 15);
	_vm->_location._startPosition.x = atoi(s);

	f->readLine_OLD(s, 15);
	_vm->_location._startPosition.y = atoi(s);

	f->readLine_OLD(s, 15);
	_score = atoi(s);

	f->readLine_OLD(s, 15);
	_globalFlags = atoi(s);

	f->readLine_OLD(s, 15);

	// TODO (LIST): unify (and parametrize) calls to freeZones.
	// We aren't calling freeAnimations because it is not needed, since
	// kChangeLocation will trigger a complete deletion. Anyway, we still
	// need to invoke freeZones here with _quit set, because the
	// call in changeLocation preserve certain zones.
	_vm->_quit = true;
	_vm->freeZones();
	_vm->_quit = false;

	_vm->_numLocations = atoi(s);

	uint16 _si;
	for (_si = 0; _si < _vm->_numLocations; _si++) {
		f->readLine_OLD(s, 20);
		s[strlen(s)] = '\0';

		strcpy(_vm->_locationNames[_si], s);

		f->readLine_OLD(s, 15);
		_vm->_localFlags[_si] = atoi(s);
	}

	_vm->cleanInventory(false);
	ItemName name;
	uint32 value;

	for (_si = 0; _si < 30; _si++) {
		f->readLine_OLD(s, 15);
		value = atoi(s);

		f->readLine_OLD(s, 15);
		name = atoi(s);

		_vm->addInventoryItem(name, value);
	}

	delete f;

	// force reload of character to solve inventory
	// bugs, but it's a good maneuver anyway
	strcpy(_vm->_characterName1, "null");

	char tmp[PATH_LEN];
	sprintf(tmp, "%s.%s" , l, n);
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
	sprintf(s, "%d\n", _score);
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
	: Dialog("ScummSaveLoad"), _list(0), _chooseButton(0), _gfxWidget(0) {

//	_drawingHints |= GUI::THEME_HINT_SPECIAL_COLOR;
	_backgroundType = GUI::Theme::kDialogBackgroundSpecial;

	new GUI::StaticTextWidget(this, "ScummSaveLoad.Title", title);

	// Add choice list
	_list = new GUI::ListWidget(this, "ScummSaveLoad.List");
	_list->setEditable(true);
	_list->setNumberingMode(GUI::kListNumberingOne);

	_gfxWidget = new GUI::GraphicsWidget(this, 0, 0, 10, 10);

	_date = new GUI::StaticTextWidget(this, 0, 0, 10, 10, "No date saved", GUI::kTextAlignCenter);
	_time = new GUI::StaticTextWidget(this, 0, 0, 10, 10, "No time saved", GUI::kTextAlignCenter);
	_playtime = new GUI::StaticTextWidget(this, 0, 0, 10, 10, "No playtime saved", GUI::kTextAlignCenter);

	// Buttons
	new GUI::ButtonWidget(this, "ScummSaveLoad.Cancel", "Cancel", GUI::kCloseCmd, 0);
	_chooseButton = new GUI::ButtonWidget(this, "ScummSaveLoad.Choose", buttonLabel, kChooseCmd, 0);
	_chooseButton->setEnabled(false);

	_container = new GUI::ContainerWidget(this, 0, 0, 10, 10);
//	_container->setHints(GUI::THEME_HINT_USE_SHADOW);
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
	if (_gfxWidget)
		_gfxWidget->setGfx(0);
	int ret = GUI::Dialog::runModal();
	return ret;
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
	_container->setFlags(GUI::WIDGET_INVISIBLE);
	_gfxWidget->setFlags(GUI::WIDGET_INVISIBLE);
	_date->setFlags(GUI::WIDGET_INVISIBLE);
	_time->setFlags(GUI::WIDGET_INVISIBLE);
	_playtime->setFlags(GUI::WIDGET_INVISIBLE);

	Dialog::reflowLayout();
}

int SaveLoad_ns::buildSaveFileList(Common::StringList& l) {

	char buf[200];

	int count = 0;

	for (int i = 0; i < NUM_SAVESLOTS; i++) {
		buf[0] = '\0';

		Common::InSaveFile *f = getInSaveFile(i);
		if (f) {
			f->readLine_OLD(buf, 199);
			delete f;

			count++;
		}

		l.push_back(buf);
	}

	return count;
}


int SaveLoad_ns::selectSaveFile(uint16 arg_0, const char* caption, const char* button) {

	SaveLoadChooser* slc = new SaveLoadChooser(caption, button);

	Common::StringList l;

	/*int count = */ buildSaveFileList(l);
	slc->setList(l);

	int idx = slc->runModal();
	if (idx >= 0) {
		_saveFileName = slc->getResultString();
	}

	delete slc;

	return idx;
}



bool SaveLoad_ns::loadGame() {

	int _di = selectSaveFile( 0, "Load file", "Load" );
	if (_di == -1) {
		return false;
	}

	doLoadGame(_di);

	GUI::TimedMessageDialog dialog("Loading game...", 1500);
	dialog.runModal();

	_vm->_input->setArrowCursor();

	return true;
}


bool SaveLoad_ns::saveGame() {

	if (!scumm_stricmp(_vm->_location._name, "caveau")) {
		return false;
	}

	int slot = selectSaveFile( 1, "Save file", "Save" );
	if (slot == -1) {
		return false;
	}

	doSaveGame(slot, _saveFileName.c_str());

	GUI::TimedMessageDialog dialog("Saving game...", 1500);
	dialog.runModal();

	return true;
}


void SaveLoad_ns::setPartComplete(const char *part) {
	char buf[30];
	bool alreadyPresent = false;

	memset(buf, 0, sizeof(buf));

	Common::InSaveFile *inFile = getInSaveFile(SPECIAL_SAVESLOT);
	if (inFile) {
		inFile->readLine_OLD(buf, 29);
		delete inFile;

		if (strstr(buf, part)) {
			alreadyPresent = true;
		}
	}

	if (!alreadyPresent) {
		Common::OutSaveFile *outFile = getOutSaveFile(SPECIAL_SAVESLOT);
		outFile->writeString(buf);
		outFile->writeString(part);
		outFile->finalize();
		delete outFile;
	}

	return;
}

void SaveLoad_ns::getGamePartProgress(bool *complete, int size) {
	assert(complete && size >= 3);

	char buf[30];
	Common::InSaveFile *inFile = getInSaveFile(SPECIAL_SAVESLOT);
	inFile->readLine_OLD(buf, 29);
	delete inFile;

	complete[0] = strstr(buf, "dino");
	complete[1] = strstr(buf, "donna");
	complete[2] = strstr(buf, "dough");
}

void SaveLoad_ns::renameOldSavefiles() {

	bool exists[NUM_SAVESLOTS];
	uint num = 0;
	uint i;

	for (i = 0; i < NUM_SAVESLOTS; i++) {
		exists[i] = false;
		Common::String name = genOldSaveFileName(i);
		Common::InSaveFile *f = _saveFileMan->openForLoading(name.c_str());
		if (f) {
			exists[i] = true;
			num++;
		}
		delete f;
	}

	if (num == 0) {
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

	uint success = 0;
	for (i = 0; i < NUM_SAVESLOTS; i++) {
		if (exists[i]) {
			Common::String oldName = genOldSaveFileName(i);
			Common::String newName = genSaveFileName(i);
			if (_saveFileMan->renameSavefile(oldName.c_str(), newName.c_str())) {
				success++;
			} else {
				warning("Error %i (%s) occurred while renaming %s to %s", _saveFileMan->getError(),
					_saveFileMan->getErrorDesc().c_str(), oldName.c_str(), newName.c_str());
			}
		}
	}

	char msg[200];
	if (success == num) {
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


bool SaveLoad_br::loadGame() {
	// TODO: implement loadgame
	return false;
}

bool SaveLoad_br::saveGame() {
	// TODO: implement savegame
	return false;
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
