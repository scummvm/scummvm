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

void Menu::startTitle() {
	// REMOVE: Putting this here since Menu hasn't been implemented yet.
	// Defaults the game into Action Mode
	g_hdb->setActionMode(1);
	g_hdb->setGameState(GAME_PLAY);

	warning("STUB: Menu::startTitle()");

}

void Menu::startMenu() {
	debug(9, "STUB: Start Menu");
}

void Menu::freeMenu() {
	debug(9, "STUB: Free Menu");
}

void Menu::fillSavegameSlots() {
	warning("STUB: PMenu::fillSavegameSlots()");
}

} // End of Namespace
