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
 * Copyright 2020 Google
 *
 */
#include "hadesch/hadesch.h"
#include "hadesch/video.h"
#include "common/util.h"

namespace Hadesch {

enum {
	kBackgroundZ = 10000,
	kScrollZ = 9900,
	kScrollBarZ = 9000,
	kThumbnailZ = 5000,
	kArrowsZ = 2900,
	kThumbZ = 2800,
	kButtonZ = 2000,
	kTitleZ = 2000
};

static const struct {
	const char *image;
	const char *hotname;
} buttons[] = {
	{"return", "returntogame"},
	{"credits", "credits"},
	{"quit", "quitgame"},
	{"new", "new"},
	{"savegame", "savegame"},
	{"restoregame", "restoregame"},
	{"cancel", "cancel"},
	{"save", "save"},
	{"delete", "delete"},
	{"yes", "yes"},
	{"no", "no"},
	{"ok", "ok"}
};

static const char *saveDescs[] = {
	"",
	"%s in intro scene",
	"%s on mount olympus",
	"%s in wall of fame",
	"%s on Seriphos",
	"%s in Athena temple",
	"%s on Medusa Isle",
	"%s in Medusa fight",
	"%s on Argo",
	"%s in Troy",
	"%s in Catacombs",
	"%s in Priam's Castle",
	"%s in Trojan horse puzzle",
	"%s on Crete",
	"%s in Minos' Palace",
	"%s in Daedalus' Room",
	"%s in Minotaur puzzle",
	"%s on Volcano top",
	"%s near river Styx",
	"%s in Hades' throne room",
	"%s in ferryman puzzle",
	"%s in monster puzzle",
	"%s in Hades' Challenge",
	"%s in credits scene",
	""
};

class OptionsHandler : public Handler {
public:
	enum AlertType {
		kAlertSaveBeforeLoad,
		kAlertSaveBeforeExit,
		kAlertSaveBeforeNew,
		kDeleteFromLoad,
		kDeleteFromSave,
		kDeleteUser
	};
	enum SaveMenuVariant {
		kSaveFromMainMenu,
		kSaveBeforeLoad,
		kSaveBeforeExit,
		kSaveBeforeNew
	};
	OptionsHandler() {
		_savesLoaded = false;
		_showPos = 0;
		_selectedSave = -1;
		_isLast = false;
	}
	void handleClick(const Common::String &hotname) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		if (hotname == "returntogame") {
			g_vm->exitOptions();
			return;
		}
		if (hotname == "credits") {
			g_vm->enterOptionsCredits();
			return;
		}

		if (hotname == "savegame") {
			g_vm->resetOptionsRoom();
			saveMenu(kSaveFromMainMenu);
			return;
		}

		if (hotname == "restoregame") {
			g_vm->resetOptionsRoom();
			if (gameInProgress())
				alertMenu(kAlertSaveBeforeLoad);
			else
				loadMenuUser();
			return;
		}

		if (hotname == "cancel") {
			switch (_currentMenu) {
			case kLoadSlotMenu: 
				g_vm->resetOptionsRoom();
				loadMenuUser();
				break;
			case kSaveMenu:
			case kLoadUserMenu:
				if (gameInProgress()) {
					g_vm->resetOptionsRoom();
					gameMenu();
				} else {
					g_vm->exitOptions();
				}
				break;
				// No cancel in game menu
			case kGameMenu:
				break;
			}
			return;
		}

		if (hotname == "save" && _currentMenu == kSaveMenu) {
			performSave();
			return;
		}

		if (hotname.matchString("nameslot#")) {
			_selectedSave = _showPos + hotname.substr(8).asUint64();
			renderUserNames();
			return;
		}

		if (hotname.matchString("saveslot#")) {
			_selectedSave = _showPos + hotname.substr(8).asUint64();
			renderSaveSlots();
			return;
		}

		if (hotname.matchString("restoreslot#")) {
			_selectedSave = _showPos + hotname.substr(11).asUint64();
			renderLoadSlots();
			return;
		}

		if (hotname == "arrowup" && _currentMenu == kLoadSlotMenu) {
			if (_showPos < 6)
				_showPos = 0;
			else
				_showPos -= 6;
			renderLoadSlots();
			return;
		}

		if (hotname == "arrowdown" && _currentMenu == kLoadSlotMenu) {
			if (_showPos + 6 < (int) _userNames.size())
				_showPos += 6;
			renderLoadSlots();
			return;
		}

		if (hotname == "arrowup" && _currentMenu == kLoadUserMenu) {
			if (_showPos < 6)
				_showPos = 0;
			else
				_showPos -= 6;
			renderUserNames();
			return;
		}

		if (hotname == "arrowdown" && _currentMenu == kLoadUserMenu) {
			if (_showPos + 6 < (int) _userNames.size())
				_showPos += 6;
			renderUserNames();
			return;
		}

		if (hotname == "arrowup" && _currentMenu == kSaveMenu) {
			if (_showPos < 3)
				_showPos = 0;
			else
				_showPos -= 3;
			renderSaveSlots();
			return;
		}

		if (hotname == "arrowdown" && _currentMenu == kSaveMenu) {
			if (_showPos + 3 < (int) _filteredSaves.size())
				_showPos += 3;
			renderSaveSlots();
			return;
		}

		if (hotname == "no") {
			switch (_alertType) {
			case kAlertSaveBeforeLoad:
			case kDeleteUser:
				g_vm->resetOptionsRoom();
				loadMenuUser();
				break;
			case kAlertSaveBeforeExit:
				g_vm->quit();
				break;
			case kAlertSaveBeforeNew:
				g_vm->newGame();
				g_vm->exitOptions();
				break;
			case kDeleteFromLoad:
				g_vm->resetOptionsRoom();
				loadMenuSlot();
				break;
			case kDeleteFromSave:
				g_vm->resetOptionsRoom();
				saveMenu(_saveVariant);
				break;
			}
			return;
		}

		if (hotname == "yes") {
			switch (_alertType) {
			case kAlertSaveBeforeLoad:
				g_vm->resetOptionsRoom();
				saveMenu(kSaveBeforeLoad);
				break;
			case kAlertSaveBeforeExit:
				g_vm->resetOptionsRoom();
				saveMenu(kSaveBeforeExit);
				break;
			case kAlertSaveBeforeNew:
				g_vm->resetOptionsRoom();
				saveMenu(kSaveBeforeNew);
				break;
			case kDeleteFromLoad:
				g_vm->deleteSave(_filteredSaves[_selectedSave]._slot);
				_savesLoaded = false; // Invalidate cache
				g_vm->resetOptionsRoom();
				loadMenuSlot();
				break;
			case kDeleteFromSave:
				g_vm->deleteSave(_filteredSaves[_selectedSave]._slot);
				_savesLoaded = false; // Invalidate cache
				g_vm->resetOptionsRoom();
				saveMenu(_saveVariant);
				break;
			case kDeleteUser: {
				Common::U32String username = _userNames[_selectedSave];
				for (unsigned i = 0; i < _saves.size(); i++)
					if (_saves[i]._heroName == username)
						g_vm->deleteSave(_saves[i]._slot);
				_savesLoaded = false; // Invalidate cache
			}
			}
			return;
		}

		if (hotname == "ok" && _currentMenu == kLoadUserMenu) {
			g_vm->resetOptionsRoom();
			_chosenName = _userNames[_selectedSave];
			loadMenuSlot();
			return;
		}

		if (hotname == "restore") {
			int slot = _filteredSaves[_selectedSave]._slot;
			g_vm->loadGameState(slot);
			g_vm->exitOptions();
			return;
		}
			

		if (hotname == "quitgame") {
			g_vm->resetOptionsRoom();
			if (gameInProgress())
				alertMenu(kAlertSaveBeforeExit);
			else
				g_vm->quit();
			return;
		}

		if (hotname == "new") {
			g_vm->resetOptionsRoom();
			if (gameInProgress())
				alertMenu(kAlertSaveBeforeNew);
			else
				g_vm->newGame();
			return;
		}

		if (hotname == "delete" && _currentMenu == kLoadUserMenu) {
			g_vm->resetOptionsRoom();
			alertMenu(kDeleteUser);
			return;
		}

		if (hotname == "delete" && _currentMenu == kLoadSlotMenu) {
			g_vm->resetOptionsRoom();
			alertMenu(kDeleteFromLoad);
			return;
		}

		if (hotname == "delete" && _currentMenu == kSaveMenu) {
			g_vm->resetOptionsRoom();
			alertMenu(kDeleteFromSave);
			return;
		}
	}

	void handleMouseOver(const Common::String &hotname) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for (unsigned i = 0; i < ARRAYSIZE(buttons); i++)
			if (hotname == buttons[i].hotname) {
				room->selectFrame(buttons[i].image, kButtonZ, 1);
				return;
			}
		if (hotname == "arrowup" && _showPos > 0) {
			room->selectFrame("arrows", kArrowsZ, 1);
			return;
		}

		if (hotname == "arrowdown" && !_isLast) {
			room->selectFrame("arrows", kArrowsZ, 2);
			return;
		}
	}

	void handleMouseOut(const Common::String &hotname) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for (unsigned i = 0; i < ARRAYSIZE(buttons); i++)
			if (hotname == buttons[i].hotname) {
				room->selectFrame(buttons[i].image, kButtonZ, 0);
				return;
			}

		if (hotname == "arrowup" || hotname == "arrowdown") {
			room->selectFrame("arrows", kArrowsZ, 0);
			return;
		}
	}

	void handleKeypress(uint32 ucode) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (_currentMenu == kSaveMenu) {
			if (ucode == '\n' || ucode == '\r') {
				performSave();
				return;
			}

			if (ucode == '\b' && _typedSlotName.size() > 0) {
				_typedSlotName.deleteLastChar();
				room->playSFX("keyclick");
				renderSaveName();
				return;
			}

			if (ucode == '\0' || ucode < ' ')
				return;

			if (_typedSlotName.size() < 11) {
				_typedSlotName += ucode;
				room->playSFX("keyclick");
				renderSaveName();
			}
		}
	}

	void handleEvent(int eventId) override {
	}

	~OptionsHandler() override {}

	void prepareRoom() override {
		Persistent *persistent = g_vm->getPersistent();
		if (persistent->_currentRoomId == kOlympusRoom)
			loadMenuUser();
		else
			gameMenu();
	}

private:
	void performSave() {		
		int slot = g_vm->firstAvailableSlot();
		Persistent *persistent = g_vm->getPersistent();
		Common::String heroNameUTF8 = persistent->_heroName.encode(Common::kUtf8);
		// UTF-8
		Common::String descPos = Common::String::format(
			saveDescs[persistent->_currentRoomId],
			heroNameUTF8.c_str());
		// UTF-8
		Common::String desc = _typedSlotName.empty() ? descPos
		    : _typedSlotName + " (" + descPos + ")";

		persistent->_slotDescription = _typedSlotName;
		Common::Error res = g_vm->saveGameState(slot, desc);
		debug("%d, %s->[%d, %s]", slot, desc.c_str(), res.getCode(), res.getDesc().c_str());
		_savesLoaded = false; // Invalidate cache
		switch (_saveVariant) {
		case kSaveFromMainMenu:
			g_vm->exitOptions();
			break;
		case kSaveBeforeLoad:
			g_vm->resetOptionsRoom();
			loadMenuUser();
			break;
		case kSaveBeforeExit:
			g_vm->quit();
			break;
		case kSaveBeforeNew:
			g_vm->newGame();
			g_vm->exitOptions();
			break;
		}
	}
	void renderSaveName() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		// One character more to handle possible backspace clicks.
		room->hideString("smallascii", _typedSlotName.size() + 1);
		room->renderString("smallascii", _typedSlotName, Common::Point(150, 266), 4000);
	}

	void gameMenu() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_currentMenu = kGameMenu;
		room->loadHotZones("OPGame.HOT");
		room->addStaticLayer("black", kBackgroundZ);
		room->disableHeroBelt();
		room->selectFrame("gamemenu", kScrollBarZ, 0);
		room->selectFrame("return", kButtonZ, 0);
		room->selectFrame("credits", kButtonZ, 0);
		room->selectFrame("quit", kButtonZ, 0);
		room->selectFrame("new", kButtonZ, 0);
		room->selectFrame("savegame", kButtonZ, 0);
		if (g_vm->hasAnySaves())
			room->selectFrame("restoregame", kButtonZ, 0);
		else
			room->disableHotzone("restoregame");
	}

	bool gameInProgress() {
		Persistent *persistent = g_vm->getPersistent();
		return persistent->_currentRoomId != kOlympusRoom && persistent->_currentRoomId != kIntroRoom;
	}

  	void loadMenuUser() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Common::HashMap<Common::U32String, bool> userset;

		loadSaves();
		_currentMenu = kLoadUserMenu;
		_userNames.clear();
		for (unsigned i = 0; i < _saves.size(); i++)
			if (!userset[_saves[i]._heroName]) {
				userset[_saves[i]._heroName] = true;
				_userNames.push_back(_saves[i]._heroName);
			}
		Common::sort(_userNames.begin(), _userNames.end());

		room->loadHotZones("OPRest1.HOT");
		room->addStaticLayer("black", kBackgroundZ);
		room->disableHeroBelt();
		room->selectFrame("scroll", kScrollZ, 0);
		room->selectFrame("restorescroll", kScrollBarZ, 0);

		room->selectFrame("cancel", kButtonZ, 0);
		room->selectFrame("delete", kButtonZ, 0);
		room->selectFrame("ok", kButtonZ, 0);
		room->selectFrame("choosename", kTitleZ, 0);
		if (_userNames.size() > 6)
			room->selectFrame("arrows", kArrowsZ, 0);
		else {
			room->disableHotzone("arrowup");
			room->disableHotzone("arrowdown");
		}
		_showPos = 0;
		_selectedSave = -1;
		renderUserNames();
	}

	void renderUserNames() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		bool selectedIsShown = false;
		for (int i = 0; i < 6 && _showPos + i < (int) _userNames.size(); i++) {
			Common::U32String name = _userNames[_showPos + i];
			if (name == "")
				name = "No name";
			room->renderString("largeascii", name,
					   Common::Point(150, 134 + 36 * i), 4000, 0,
					   Common::String::format("username%d", i));
			if (_showPos + i == _selectedSave) {
				selectedIsShown = true;
				room->selectFrame("thumb", kThumbZ, 0, Common::Point(109, 134 + 36 * i));
			}
		}

		for (unsigned i = 0; i < 6; i++) {
			room->setHotzoneEnabled(Common::String::format("nameslot%d", i), _showPos + i < _userNames.size());
		}

		_isLast = _showPos + 3 >= (int) _userNames.size();

		room->setHotzoneEnabled("delete", selectedIsShown);
		room->setHotzoneEnabled("ok", selectedIsShown);
		room->setHotzoneEnabled("arrowdown", !_isLast);
		room->setHotzoneEnabled("arrowup", _showPos > 0);
	}

	void loadMenuSlot() {
		loadSaves();

		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		loadFilteredSaves(_chosenName);

		_currentMenu = kLoadSlotMenu;
		room->loadHotZones("OPRest2.HOT");
		room->addStaticLayer("black", kBackgroundZ);
		room->disableHeroBelt();
		room->selectFrame("scroll", kScrollZ, 0);
		room->selectFrame("restore2scroll", kScrollBarZ, 0);
		room->renderStringCentered("largeascii", _chosenName, Common::Point(320, 77), 4000);
		if (_filteredSaves.size() > 6)
			room->selectFrame("arrows", kArrowsZ, 0);
		else {
			room->disableHotzone("arrowup");
			room->disableHotzone("arrowdown");
		}
			
		room->selectFrame("cancel", kButtonZ, 0);
		room->selectFrame("restore", kButtonZ, 0);
		room->selectFrame("delete", kButtonZ, 0);
		room->disableHotzone("delete");
		_selectedSave = -1;
		_showPos = 0;

		renderLoadSlots();
	}

	void alertMenu(AlertType alertType) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->loadHotZones("OPAlert.HOT");
		room->addStaticLayer("black", kBackgroundZ);
		room->disableHeroBelt();
		// Original uses other Z values but it generates the same
		// resulting image and we can keep button code consistent
		room->selectFrame("alert", 4000, 0);
		switch (alertType) {
		case kAlertSaveBeforeLoad:
		case kAlertSaveBeforeExit:
		case kAlertSaveBeforeNew:
			room->selectFrame("exit", 3800, 0);
			break;
		case kDeleteFromLoad:
		case kDeleteFromSave:
			room->selectFrame("deletegame", 3800, 0);
			break;
		case kDeleteUser:
			room->selectFrame("deletename", 3800, 0);
			break;
		}
		room->selectFrame("yes", kButtonZ, 0);
		room->selectFrame("no", kButtonZ, 0);
		_alertType = alertType;
	}

	void saveMenu(SaveMenuVariant saveVariant) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();

		loadFilteredSaves(persistent->_heroName);

		_currentMenu = kSaveMenu;
		_saveVariant = saveVariant;
		room->loadHotZones("OPSave.HOT");
		room->addStaticLayer("black", kBackgroundZ);
		room->disableHeroBelt();
		room->selectFrame("scroll", kScrollZ, 0);
		room->selectFrame("savescroll", kScrollBarZ, 0);
		room->renderStringCentered("largeascii", persistent->_heroName, Common::Point(320, 77), 4000);
		if (_filteredSaves.size() > 3)
			room->selectFrame("arrows", kArrowsZ, 0);
		else {
			room->disableHotzone("arrowup");
			room->disableHotzone("arrowdown");
		}
			
		room->selectFrame("cancel", kButtonZ, 0);
		room->selectFrame("save", kButtonZ, 0);
		room->selectFrame("delete", kButtonZ, 0);
		room->disableHotzone("delete");
		_selectedSave = -1;
		_showPos = 0;

		_typedSlotName = "";

		room->selectFrame("saveas", kTitleZ, 0);
		room->selectFrame(LayerId("thumbnails", 0, "save"), 5000,
				  persistent->_currentRoomId - 1, Common::Point(184, 204));
		renderSaveSlots();
	}

	void renderSaveSlots() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		bool selectedIsShown = false;
		for (int i = 0; i < 3; i++) {
			Common::Point base = Common::Point(310, 128+76*i);
			bool isValid = _showPos + i < (int)_filteredSaves.size();
			room->hideString("smallascii", 30, Common::String::format("saveslots%d", i));
			room->setHotzoneEnabled(Common::String::format("saveslot%d", i), isValid);
			if (isValid) {
				room->selectFrame(LayerId("thumbnails", i, "right"), 5000,
						  _filteredSaves[_showPos + i]._room - 1, base + Common::Point(31, 0));
				room->renderString("smallascii", _filteredSaves[_showPos + i]._slotName,
						   base + Common::Point(31, 62), 5000,
						   0, Common::String::format("saveslots%d", i));
				if (_showPos + i == _selectedSave) {
					// TODO: original uses 300 - thumbWidth, I ust hardcode thumbWidth
					room->selectFrame("thumb", kThumbZ, 0, base - Common::Point(31+10, 0));
					selectedIsShown = true;
				}
			} else {
				room->stopAnim(LayerId("thumbnails", i, "right"));
			}
		}

		_isLast = _showPos + 3 >= (int) _filteredSaves.size();

		room->setHotzoneEnabled("delete", selectedIsShown);
		room->setHotzoneEnabled("arrowdown", !_isLast);
		room->setHotzoneEnabled("arrowup", _showPos > 0);
	}

	void renderLoadSlots() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		bool selectedIsShown = false;
		for (int i = 0; i < 6; i++) {
			bool isValid = _showPos + i < (int) _filteredSaves.size();
			room->hideString("smallascii", 30, Common::String::format("loadslots%d", i));
			room->setHotzoneEnabled(Common::String::format("restoreslot%d", i), isValid);
			if (isValid) {
				Common::Point base = Common::Point(153 + 157 * (i % 2), 128 + 76 * (i / 2));
				room->selectFrame(LayerId("thumbnails", i, "right"), 5000,
						  _filteredSaves[_showPos + i]._room - 1, base + Common::Point(31, 0));
				room->renderString("smallascii", _filteredSaves[_showPos + i]._slotName,
						   base + Common::Point(31, 62), 5000,
						   0, Common::String::format("loadslots%d", i));
				if (_showPos + i == _selectedSave) {
					// TODO: original uses 300 - thumbWidth, I ust hardcode thumbWidth
					room->selectFrame("thumb", kThumbZ, 0, base);
					selectedIsShown = true;
				}
			} else {
				room->stopAnim(LayerId("thumbnails", i, "right"));
			}
		}

		_isLast = _showPos + 6 >= (int) _filteredSaves.size();

		room->setHotzoneEnabled("arrowdown", !_isLast);
		room->setHotzoneEnabled("arrowup", _showPos > 0);
		room->setHotzoneEnabled("restore", selectedIsShown);
		room->setHotzoneEnabled("delete", selectedIsShown);
	}

	void loadSaves() {
		if (_savesLoaded)
			return;
		_saves = g_vm->getHadeschSavesList();
	}

	void loadFilteredSaves(const Common::U32String &heroname) {
		loadSaves();
		_filteredSaves.clear();
		for (unsigned i = 0; i < _saves.size(); i++)
			if (_saves[i]._heroName == heroname)
				_filteredSaves.push_back(_saves[i]);
	}

	enum {
		kGameMenu,
		kSaveMenu,
		kLoadUserMenu,
		kLoadSlotMenu
	} _currentMenu;
	SaveMenuVariant _saveVariant;
	AlertType _alertType;

	Common::Array<HadeschSaveDescriptor> _saves;
	Common::Array<HadeschSaveDescriptor> _filteredSaves;
	Common::Array<Common::U32String> _userNames;
	Common::U32String _chosenName;
	Common::U32String _typedSlotName;
	int _showPos;
	int _selectedSave;
	bool _savesLoaded;
	bool _isLast;
};

Common::SharedPtr<Hadesch::Handler> makeOptionsHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new OptionsHandler());
}

}
