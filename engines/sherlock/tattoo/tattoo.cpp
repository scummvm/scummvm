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

#include "common/config-manager.h"
#include "engines/util.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_resources.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/widget_base.h"
#include "sherlock/people.h"

namespace Sherlock {

namespace Tattoo {

TattooEngine::TattooEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		SherlockEngine(syst, gameDesc), _darts(this), _foolscapWidget(this) {
	_runningProlog = false;
	_fastMode = false;
	_allowFastMode = true;
	_transparentMenus = true;
	_textWindowsOn = true;
}

TattooEngine::~TattooEngine() {
}

void TattooEngine::showOpening() {
	// No implementation - opening is done using in-game scenes
}

void TattooEngine::initialize() {
	initGraphics(640, 480);

	// Initialize the base engine
	SherlockEngine::initialize();

	// Initialise the global flags
	_flags.resize(3200);
	_flags[1] = _flags[4] = _flags[76] = true;
	_runningProlog = true;

	// Add some more files to the cache
	_res->addToCache("walk.lib");

	// Set up list of people
	TattooFixedText &fixedText = *(TattooFixedText *)_fixedText;
	const char *peopleNamePtr = nullptr;

	for (int idx = 0; idx < TATTOO_MAX_PEOPLE; ++idx) {
		peopleNamePtr = fixedText.getText(PEOPLE_DATA[idx].fixedTextId);

		_people->_characters.push_back(PersonData(
			peopleNamePtr,
			PEOPLE_DATA[idx].portrait, nullptr, nullptr));
	}

	// Load the inventory
	loadInventory();

	// Starting scene
	_scene->_goToScene = STARTING_INTRO_SCENE;

	// Load an initial palette
	loadInitialPalette();
}

void TattooEngine::startScene() {
	TattooUserInterface &ui = *(TattooUserInterface *)_ui;

	switch (_scene->_goToScene) {
	case 7:
	case 8:
	case 18:
	case 53:
	case 68:
		// Load overlay mask(s) for the scene
		ui._mask = _res->load(Common::String::format("res%02d.msk", _scene->_goToScene));
		if (_scene->_goToScene == 8)
			ui._mask1 = _res->load("res08a.msk");
		else if (_scene->_goToScene == 18 || _scene->_goToScene == 68)
			ui._mask1 = _res->load("res08a.msk");
		break;

	case STARTING_INTRO_SCENE:
		// Disable input so that the intro can't be skipped until the game's logo has been shown
		ui._lockoutTimer = STARTUP_KEYS_DISABLED_DELAY;
		break;

	case OVERHEAD_MAP:
	case OVERHEAD_MAP2:
		// Show the map
		_scene->_currentScene = OVERHEAD_MAP;
		_scene->_goToScene = _map->show();

		_people->_savedPos = Common::Point(-1, -1);
		_people->_savedPos._facing = -1;
		break;

	case 101:
		// Darts Board minigame
		_darts.playDarts(GAME_CRICKET);
		break;

	case 102:
		// Darts Board minigame
		_darts.playDarts(GAME_301);
		break;

	case 103:
		// Darts Board minigame
		_darts.playDarts(GAME_501);
		break;

	default:
		break;
	}

	_events->setCursor(ARROW);
}

void TattooEngine::loadInitialPalette() {
	byte palette[768];
	Common::SeekableReadStream *stream = _res->load("room.pal");
	stream->read(palette, PALETTE_SIZE);
	_screen->translatePalette(palette);
	_screen->setPalette(palette);

	delete stream;
}

void TattooEngine::loadInventory() {
	Inventory &inv = *_inventory;

	Common::String inv1 = _fixedText->getText(kFixedText_Inv1);
	Common::String inv2 = _fixedText->getText(kFixedText_Inv2);
	Common::String inv3 = _fixedText->getText(kFixedText_Inv3);
	Common::String inv4 = _fixedText->getText(kFixedText_Inv4);
	Common::String inv5 = _fixedText->getText(kFixedText_Inv5);
	Common::String inv6 = _fixedText->getText(kFixedText_Inv6);
	Common::String inv7 = _fixedText->getText(kFixedText_Inv7);
	Common::String inv8 = _fixedText->getText(kFixedText_Inv8);
	Common::String invDesc1 = _fixedText->getText(kFixedText_InvDesc1);
	Common::String invDesc2 = _fixedText->getText(kFixedText_InvDesc2);
	Common::String invDesc3 = _fixedText->getText(kFixedText_InvDesc3);
	Common::String invDesc4 = _fixedText->getText(kFixedText_InvDesc4);
	Common::String invDesc5 = _fixedText->getText(kFixedText_InvDesc5);
	Common::String invDesc6 = _fixedText->getText(kFixedText_InvDesc6);
	Common::String invDesc7 = _fixedText->getText(kFixedText_InvDesc7);
	Common::String invDesc8 = _fixedText->getText(kFixedText_InvDesc8);
	Common::String solve = _fixedText->getText(kFixedText_Solve);

	// Initial inventory
	inv._holdings = 5;
	inv.push_back(InventoryItem(0, inv1, invDesc1, "_ITEM01A"));
	inv.push_back(InventoryItem(0, inv2, invDesc2, "_ITEM02A"));
	inv.push_back(InventoryItem(0, inv3, invDesc3, "_ITEM03A"));
	inv.push_back(InventoryItem(0, inv4, invDesc4, "_ITEM04A"));
	inv.push_back(InventoryItem(0, inv5, invDesc5, "_ITEM05A"));

	// Hidden items
	inv.push_back(InventoryItem(295, inv6, invDesc6, "_PAP212D", solve));
	inv.push_back(InventoryItem(294, inv7, invDesc7, "_PAP212I"));
	inv.push_back(InventoryItem(818, inv8, invDesc8, "_LANT02I"));
}

void TattooEngine::doFoolscapPuzzle() {
	_foolscapWidget.show();
}

void TattooEngine::loadConfig() {
	SherlockEngine::loadConfig();

	_transparentMenus = ConfMan.getBool("transparent_windows");
	_textWindowsOn = ConfMan.getBool("subtitles") || !_sound->_speechOn;
}

void TattooEngine::saveConfig() {
	SherlockEngine::saveConfig();

	ConfMan.setBool("transparent_windows", _transparentMenus);
	ConfMan.setBool("subtitles", _textWindowsOn);
	ConfMan.flushToDisk();
}

bool TattooEngine::canLoadGameStateCurrently() {
	TattooUserInterface &ui = *(TattooUserInterface *)_ui;
	return _canLoadSave && !ui._creditsWidget.active() && !_runningProlog;
}

bool TattooEngine::canSaveGameStateCurrently() {
	TattooUserInterface &ui = *(TattooUserInterface *)_ui;
	return _canLoadSave && !ui._creditsWidget.active() && !_runningProlog;
}

} // End of namespace Tattoo

} // End of namespace Sherlock
