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
	_starWarp = 0;

	warning("STUB: Menu::init()");

	return true;
}

void Menu::startTitle() {
	// REMOVE: Putting this here since Menu hasn't been implemented yet.
	// Defaults the game into Action Mode
	g_hdb->setActionMode(1);
	g_hdb->setGameState(GAME_PLAY);

	warning("STUB: Menu::startTitle()");

}

void Menu::fillSavegameSlots() {
	warning("STUB: PMenu::fillSavegameSlots()");
}

} // End of Namespace
