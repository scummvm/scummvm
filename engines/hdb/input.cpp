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
	_buttons = 0;

	_keyUp = Common::KEYCODE_UP;
	_keyDown = Common::KEYCODE_DOWN;
	_keyLeft = Common::KEYCODE_LEFT;
	_keyRight = Common::KEYCODE_RIGHT;
	_keyMenu = Common::KEYCODE_ESCAPE;
	_keyUse = Common::KEYCODE_RETURN;
	_keyInv = Common::KEYCODE_SPACE;
	_keyDebug = Common::KEYCODE_F1;
	_keyQuit = Common::KEYCODE_F10;

	_mouseX = kScreenWidth / 2;
	_mouseY = kScreenHeight / 2;

	_mouseLButton = _mouseMButton = _mouseRButton = 0;

	return true;
}

void Input::setButtons(uint16 b) {
	static int changeState = 0;
	static int drawDlg = 0;
	static int quit = 0;
	static int debugOn = 0;

	_buttons = b;
	if (!b)
		return;

	// Change Game State
	if ((_buttons & kButtonA) && !changeState && (g_hdb->getGameState() != GAME_MENU)) {
		if (g_hdb->_ai->cinematicsActive() && g_hdb->_ai->cineAbortable()) {
			g_hdb->_ai->cineAbort();
			g_hdb->_sound->playSound(SND_POP);
			return;
		}

		if (g_hdb->getGameState() == GAME_TITLE)
			warning("STUB: setButtons: changeToMenu() required");
		g_hdb->_sound->playSound(SND_MENU_BACKOUT);
		g_hdb->changeGameState();
	}

	// Debug Mode Cycling
	debug(9, "STUB: setButtons: Check and set Debug Mode");

	if (g_hdb->getGameState() == GAME_PLAY) {
		// Is Player Dead? Click on TRY AGAIN
		if (g_hdb->_ai->playerDead()) {
			warning("STUB: TRY AGAIN is onscreen");
			return;
		}

		// Choose from DialogChoice
		if (g_hdb->_window->dialogChoiceActive()) {
			if (_buttons & kButtonUp)
				g_hdb->_window->dialogChoiceMoveup();
			else if (_buttons & kButtonDown)
				g_hdb->_window->dialogChoiceMovedown();
			else if (_buttons & kButtonB)
				g_hdb->_window->closeDialogChoice();
			return;
		}

		// Try to move the player
		if (!g_hdb->_ai->playerDead())
			g_hdb->_ai->movePlayer(_buttons);
	}
}

uint16 Input::getButtons() {
	return _buttons;
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
		if (g_hdb->_window->dialogChoiceActive()) {
			if (!g_hdb->_window->checkDialogChoiceClose(x, y))
				return;
			if (!g_hdb->_ai->cinematicsActive())
				return;
		}

		// Is MessageBar active?
		if (g_hdb->_window->msgBarActive())
			if (g_hdb->_window->checkMsgClose(x, y))
				return;

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
			g_hdb->_sound->playSound(SND_SWITCH_USE);
		}

		g_hdb->setTargetXY(worldX, worldY);
		break;
	case GAME_LOADING:
		debug(9, "stylusDown: GAME_LOADING found");
		break;
	}
}

void Input::stylusUp(int x, int y) {
	_stylusDown = false;
}

void Input::stylusMove(int x, int y) {
	// In a cinematic?
	if (g_hdb->_ai->playerLocked() || g_hdb->_ai->playerDead())
		return;

	switch (g_hdb->getGameState()) {
	case GAME_PLAY:
		warning("STUB: stylusMove: Add GetDebug() check");
		break;
	case GAME_MENU:
		warning("STUB: stylusMove: Menu::processInput() required");
		break;
	default:
		break;
	}
}

void Input::updateMouse(int newX, int newY) {
	_mouseX = newX;
	_mouseY = newY;

	if (_mouseX < 0)
		_mouseX = 0;
	else if (_mouseX >= kScreenWidth)
		_mouseX = kScreenWidth - 1;

	if (_mouseY < 0)
		_mouseY = 0;
	else if (_mouseY >= kScreenHeight)
		_mouseY = kScreenHeight - 1;

	// Turn Cursor back on?
	if (!g_hdb->_gfx->getPointer()) {
		g_hdb->_gfx->showPointer(true);
	}

	// Check if LButton is being dragged
	if (_mouseLButton) {
		stylusMove(_mouseX, _mouseY);
	}
}

void Input::updateMouseButtons(int l, int m, int r) {
	_mouseLButton += l;
	_mouseMButton += m;
	_mouseRButton += r;

	// Check if LButton has been pressed
	// Check if LButton has been lifted
	if (_mouseLButton) {
		if (_mouseX > (kScreenWidth - 32 * 5) && _mouseY < 240) {
			g_hdb->_window->checkInvSelect(_mouseX, _mouseY);
		} else if (_mouseX > (kScreenWidth - 32 * 5) && _mouseY >= 240) {
			g_hdb->_window->checkDlvSelect(_mouseX, _mouseY);
		} else {
			if (g_hdb->getPause() && g_hdb->getGameState() == GAME_PLAY) {
				g_hdb->_window->checkPause(_mouseX, _mouseY);
				return;
			}
			stylusDown(_mouseX, _mouseY);
		}
	} else if (!_mouseLButton) {
		stylusUp(_mouseX, _mouseY);
	}

	// Check if MButton has been pressed
	if (_mouseMButton) {
		if (g_hdb->getPause() && g_hdb->getGameState() == GAME_PLAY)
			return;

		g_hdb->_ai->clearWaypoints();
		g_hdb->_sound->playSound(SND_POP);
	}

	// Check if RButton has been pressed
	if (_mouseRButton) {
		if (g_hdb->getPause() && g_hdb->getGameState() == GAME_PLAY)
			return;

		uint16 buttons = getButtons() | kButtonB;
		setButtons(buttons);
	}
}

void Input::updateKeys(Common::Event event, bool keyDown) {

	debug(9, "STUB: updateKeys: Check for Quit key");

	uint16 buttons = getButtons();

	// PAUSE key pressed?
	{
		static int	current = 0, last = 0;
		last = current;
		if (keyDown && event.kbd.keycode == Common::KEYCODE_p && g_hdb->getGameState() == GAME_PLAY) {
			current = 1;
			if (!last) {
				g_hdb->togglePause();
				g_hdb->_sound->playSound(SND_POP);
			}
		} else
			current = 0;
	}

	if (!g_hdb->getPause()) {
		if (event.kbd.keycode == _keyUp) {
			if (keyDown) {
				buttons |= kButtonUp;
				if (g_hdb->_gfx->getPointer())
					g_hdb->_gfx->showPointer(false);
			} else {
				buttons &= ~kButtonUp;
			}
		} else if (event.kbd.keycode == _keyDown) {
			if (keyDown) {
				buttons |= kButtonDown;
				if (g_hdb->_gfx->getPointer())
					g_hdb->_gfx->showPointer(false);
			} else {
				buttons &= ~kButtonDown;
			}
		} else if (event.kbd.keycode == _keyLeft) {
			if (keyDown) {
				buttons |= kButtonLeft;
				if (g_hdb->_gfx->getPointer())
					g_hdb->_gfx->showPointer(false);
			} else {
				buttons &= ~kButtonLeft;
			}
		} else if (event.kbd.keycode == _keyRight) {
			if (keyDown) {
				buttons |= kButtonRight;
				if (g_hdb->_gfx->getPointer())
					g_hdb->_gfx->showPointer(false);
			} else {
				buttons &= ~kButtonRight;
			}
		} else if (event.kbd.keycode == _keyUse) {
			if (keyDown) {
				buttons |= kButtonB;
				if (g_hdb->_gfx->getPointer())
					g_hdb->_gfx->showPointer(false);
			} else {
				buttons &= ~kButtonB;
			}
		}
	}

	if (event.kbd.keycode == _keyMenu) {
		if (keyDown) {
			buttons |= kButtonA;
			g_hdb->_gfx->showPointer(true);
		} else {
			buttons &= ~kButtonA;
		}
	} else if (event.kbd.keycode == _keyDebug) {
		if (keyDown) {
			buttons |= kButtonExit;
		} else {
			buttons &= ~kButtonExit;
		}
	}

	setButtons(buttons);
}

} // End of Namespace
