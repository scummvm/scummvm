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

#include "engines/util.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_resources.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/widget_base.h"
#include "sherlock/people.h"

namespace Sherlock {

namespace Tattoo {

TattooEngine::TattooEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		SherlockEngine(syst, gameDesc) {
	_creditsActive = false;
	_runningProlog = false;
	_fastMode = false;
	_allowFastMode = true;
	_transparentMenus = true;
}

TattooEngine::~TattooEngine() {
}

void TattooEngine::showOpening() {
	// TODO
}

void TattooEngine::initialize() {
	initGraphics(640, 480, true);

	// Initialize the base engine
	SherlockEngine::initialize();

	// Initialise the global flags
	_flags.resize(3200);
	_flags[1] = _flags[4] = _flags[76] = true;
	_runningProlog = true;

	// Add some more files to the cache
	_res->addToCache("walk.lib");
	
	// Set up list of people
	for (int idx = 0; idx < TATTOO_MAX_PEOPLE; ++idx) {
		_people->_characters.push_back(PersonData(
			getLanguage() == Common::FR_FRA ? FRENCH_NAMES[idx] : ENGLISH_NAMES[idx],
			PORTRAITS[idx], nullptr, nullptr));
	}

	// Load the inventory
	loadInventory();

	// Starting scene
	_scene->_goToScene = STARTING_INTRO_SCENE;

	// Load an initial palette
	loadInitialPalette();
}

void TattooEngine::startScene() {
	if (_scene->_goToScene == OVERHEAD_MAP || _scene->_goToScene == OVERHEAD_MAP2) {
		// Show the map
		_scene->_currentScene = OVERHEAD_MAP;
		_scene->_goToScene = _map->show();

		_people->_savedPos = Common::Point(-1, -1);
		_people->_savedPos._facing = -1;
	}

	// TODO
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
	inv.push_back(InventoryItem(0, inv6, invDesc6, "_PAP212D", solve));
	inv.push_back(InventoryItem(0, inv7, invDesc7, "_PAP212I"));
	inv.push_back(InventoryItem(0, inv8, invDesc8, "_LANT02I"));
}

void TattooEngine::drawCredits() {
	// TODO
}

void TattooEngine::blitCredits() {
	// TODO
}

void TattooEngine::eraseCredits() {
	// TODO
}

void TattooEngine::doHangManPuzzle() {
	// TODO
}

} // End of namespace Tattoo

} // End of namespace Sherlock
