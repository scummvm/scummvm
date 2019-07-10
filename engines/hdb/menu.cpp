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

#include "hdb/hdb.h"
#include "hdb/menu.h"

namespace HDB {

bool Menu::init() {
//	_starWarp = 0;
	_titleActive = false;
	_menuActive = false;
	_optionsActive = false;
	_gamefilesActive = false;
	_newgameActive = false;
	_warpActive = false;
	_optionsScrolling = false;
	_optionsScrollX = kMenuX;
	_rocketX = kMRocketX;
	_sayHDB = false;

	_nebulaY = 0;		// Used as a flag
	_fStars[0].y = 0;	// Used as a flag

	_keyAssignUp = Common::KEYCODE_UP;
	_keyAssignDown = Common::KEYCODE_DOWN;
	_keyAssignLeft = Common::KEYCODE_LEFT;
	_keyAssignRight = Common::KEYCODE_RIGHT;
	_keyAssignUse = Common::KEYCODE_RETURN;

	_gCheckEmpty = g_hdb->_gfx->loadPic(G_CHECK_EMPTY);
	_gCheckOff = g_hdb->_gfx->loadPic(G_CHECK_OFF);
	_gCheckOn = g_hdb->_gfx->loadPic(G_CHECK_ON);
	_gCheckLeft = g_hdb->_gfx->loadPic(G_CHECK_LEFT);
	_gCheckRight = g_hdb->_gfx->loadPic(G_CHECK_RIGHT);

	_contArrowUp = g_hdb->_gfx->loadPic(CTRL_ARROWUP);
	_contArrowDown = g_hdb->_gfx->loadPic(CTRL_ARROWDN);
	_contArrowLeft = g_hdb->_gfx->loadPic(CTRL_ARROWLEFT);
	_contArrowRight = g_hdb->_gfx->loadPic(CTRL_ARROWRIGHT);
	_contAssign = g_hdb->_gfx->loadPic(CTRL_ASSIGN);

	_waitingForKey = false;
	_warpPlaque = g_hdb->_gfx->loadPic(WARP_PLAQUE);
	_hdbLogoScreen = g_hdb->_gfx->loadPic(TITLESCREEN);

	return true;
}

void Menu::readConfig() {
	warning("STUB: readConfig: Music Config not implemented");

	if (ConfMan.hasKey(CONFIG_MSTONE7)) {
		g_hdb->setStarsMonkeystone7(ConfMan.getInt(CONFIG_MSTONE7));
	} else {
		ConfMan.setInt(CONFIG_MSTONE7, STARS_MONKEYSTONE_7_FAKE);
	}

	if (ConfMan.hasKey(CONFIG_MSTONE14)) {
		g_hdb->setStarsMonkeystone14(ConfMan.getInt(CONFIG_MSTONE14));
	} else {
		ConfMan.setInt(CONFIG_MSTONE14, STARS_MONKEYSTONE_14_FAKE);
	}

	if (ConfMan.hasKey(CONFIG_MSTONE21)) {
		g_hdb->setStarsMonkeystone21(ConfMan.getInt(CONFIG_MSTONE21));
	} else {
		ConfMan.setInt(CONFIG_MSTONE21, STARS_MONKEYSTONE_21_FAKE);
	}

	if (ConfMan.hasKey(CONFIG_KEY_UP)) {
		_keyAssignUp = (Common::KeyCode)ConfMan.getInt(CONFIG_KEY_UP);
		g_hdb->_input->assignKey(0, _keyAssignUp);
	} else {
		ConfMan.setInt(CONFIG_KEY_UP, _keyAssignUp);
	}

	if (ConfMan.hasKey(CONFIG_KEY_DOWN)) {
		_keyAssignDown = (Common::KeyCode)ConfMan.getInt(CONFIG_KEY_DOWN);
		g_hdb->_input->assignKey(1, _keyAssignDown);
	} else {
		ConfMan.setInt(CONFIG_KEY_DOWN, _keyAssignDown);
	}

	if (ConfMan.hasKey(CONFIG_KEY_LEFT)) {
		_keyAssignLeft = (Common::KeyCode)ConfMan.getInt(CONFIG_KEY_LEFT);
		g_hdb->_input->assignKey(2, _keyAssignLeft);
	} else {
		ConfMan.setInt(CONFIG_KEY_LEFT, _keyAssignLeft);
	}

	if (ConfMan.hasKey(CONFIG_KEY_RIGHT)) {
		_keyAssignRight = (Common::KeyCode)ConfMan.getInt(CONFIG_KEY_RIGHT);
		g_hdb->_input->assignKey(3, _keyAssignRight);
	} else {
		ConfMan.setInt(CONFIG_KEY_RIGHT, _keyAssignRight);
	}

	if (ConfMan.hasKey(CONFIG_KEY_USE)) {
		_keyAssignUse = (Common::KeyCode)ConfMan.getInt(CONFIG_KEY_USE);
		g_hdb->_input->assignKey(4, _keyAssignUse);
	} else {
		ConfMan.setInt(CONFIG_KEY_USE, _keyAssignUse);
	}

	if (ConfMan.hasKey(CONFIG_CHEAT)) {
		g_hdb->setCheatingOn();
	}

	ConfMan.flushToDisk();
}

void Menu::writeConfig() {
	warning("STUB: writeConfig: Music Config not implemented");

	int value;

	value = g_hdb->getStarsMonkeystone7();
	ConfMan.setInt(CONFIG_MSTONE7, value);
	value = g_hdb->getStarsMonkeystone14();
	ConfMan.setInt(CONFIG_MSTONE14, value);
	value = g_hdb->getStarsMonkeystone21();
	ConfMan.setInt(CONFIG_MSTONE21, value);

	ConfMan.setInt(CONFIG_KEY_UP, _keyAssignUp);
	ConfMan.setInt(CONFIG_KEY_DOWN, _keyAssignDown);
	ConfMan.setInt(CONFIG_KEY_LEFT, _keyAssignLeft);
	ConfMan.setInt(CONFIG_KEY_RIGHT, _keyAssignRight);
	ConfMan.setInt(CONFIG_KEY_USE, _keyAssignUse);

	if (g_hdb->getCheatingOn())
		ConfMan.set(CONFIG_CHEAT, "1");
}

void Menu::startMenu() {
	debug(9, "STUB: Start Menu");
}

void Menu::changeToMenu() {
	warning("STUB: Change To Menu");
}

void Menu::drawMenu() {
	warning("STUB: Draw Menu");
}

void Menu::freeMenu() {
	debug(9, "STUB: Free Menu");
}

void Menu::startTitle() {
	// REMOVE: Putting this here since Menu hasn't been implemented yet.
	// Defaults the game into Action Mode
	g_hdb->setActionMode(1);
	g_hdb->setGameState(GAME_PLAY);

	warning("STUB: Menu::startTitle()");

}

void Menu::drawTitle() {
	warning("STUB: Draw Title");
}

void Menu::fillSavegameSlots() {
	warning("STUB: PMenu::fillSavegameSlots()");
}

} // End of Namespace
