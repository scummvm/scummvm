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

namespace HDB {

bool Input::init() {
	_stylusDown = false;

	warning("STUB: Input::init: Set the default key values");

	_mouseX = kScreenWidth / 2;
	_mouseY = kScreenHeight / 2;
	_lastMouseX = _mouseX;
	_lastMouseY = _mouseY;

	return true;
}

void Input::setButtons(uint16 b) {
	warning("STUB: Input: setButtons required");
}

uint16 Input::getButtons() {
	warning("STUB: Input: getButtons required");
	return 0;
}

void Input::stylusDown(int x, int y) {
	int worldX, worldY;
	GameState gs;
	static uint32 delay = 0, time;

	// Don't let the screen get clicked too fast
	time = g_system->getMillis();
	if (time - delay < 100)
		return;
	time = delay;

	_stylusDown = true;
	_stylusDownX = x;
	_stylusDownY = y;
	gs = g_hdb->getGameState();

	switch (gs) {
	case GAME_TITLE:
		warning("STUB: Menu: changeToMenu required");
		g_hdb->changeGameState();
		break;
	case GAME_MENU:
		warning("STUB: Menu: processInput required");
		break;
	case GAME_PLAY:
		// Is Player Dead? Click on TRY AGAIN
		if (g_hdb->_ai->playerDead()) {
			warning("STUB: TRY AGAIN is onscreen");
			return;
		}

		// Is Dialog Active?
		if (g_hdb->_window->dialogActive()) {
			g_hdb->_window->closeDialog();
			if (!g_hdb->_ai->cinematicsActive())
				return;
		}

		// Is a Choice Dialog Active?
		warning("STUB: stylusDown: Check Choice Dialog Active");

		// Is MessageBar active?
		warning("STUB: stylusDown: Check Message Bar Active");

		// In a cinematic?
		if (g_hdb->_ai->playerLocked())
			return;

		// Check for map dragging in debug Mode and place player there
		warning("STUB: stylusDown: Check for Map dragging in Debug Mode");

		// Clicked in the world
		g_hdb->_map->getMapXY(&worldX, &worldY);
		worldX = ((worldX + x) / kTileWidth) * kTileWidth;
		worldY = ((worldY + y) / kTileHeight) * kTileHeight;

		// Don't allow a click into INV/DELIVERIES area to go into the world
		if (x >= (kScreenWidth - 32 * 5))
			return;

		// Toggle Walk Speed if we clicked Player
		int nx, ny;
		static uint32 lastRunning = g_system->getMillis();
		g_hdb->_ai->getPlayerXY(&nx, &ny);
		if (nx == worldX && ny == worldY) {
			if (lastRunning > g_system->getMillis())
				return;
			lastRunning = g_system->getMillis() + 1000 * kRunToggleDelay;
			g_hdb->_ai->togglePlayerRunning();
			if (g_hdb->_ai->playerRunning())
				g_hdb->_window->centerTextOut("Running Speed", kScreenHeight - 32, kRunToggleDelay * kGameFPS);
			else
				g_hdb->_window->centerTextOut("Walking Speed", kScreenHeight - 32, kRunToggleDelay * kGameFPS);
			warning("STUB: Play SND_SWITCH_USE");
		}

		g_hdb->setTargetXY(worldX, worldY);
		break;
	case GAME_LOADING:
		debug(9, "stylusDown: GAME_LOADING found");
		break;
	}
}

void stylusUp(int x, int y) {
	warning("STUB: Input: stylusUp required");
}

void stylusMove(int x, int y) {
	warning("STUB: Input: stylusMove required");
}

} // End of Namespace
