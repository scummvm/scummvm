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
#include "hdb/ai.h"
#include "hdb/gfx.h"
#include "hdb/input.h"
#include "hdb/map.h"
#include "hdb/sound.h"
#include "hdb/menu.h"
#include "hdb/window.h"

namespace HDB {

void Input::init() {
	_stylusDown = false;
	_buttons = 0;

	_mouseX = g_hdb->_screenWidth / 2;
	_mouseY = g_hdb->_screenHeight / 2;
}

void Input::setButtons(uint16 b) {
	_buttons = b;
	if (!b)
		return;

	// Change Game State
	if ((_buttons & kButtonA) && (g_hdb->getGameState() != GAME_MENU)) {
		if (g_hdb->_ai->cinematicsActive() && g_hdb->_ai->cineAbortable()) {
			g_hdb->_ai->cineAbort();
			g_hdb->_sound->playSound(SND_POP);
			return;
		}

		if (g_hdb->getGameState() == GAME_TITLE)
			g_hdb->_menu->changeToMenu();
		g_hdb->_sound->playSound(SND_MENU_BACKOUT);
		g_hdb->changeGameState();
	}

	if (g_hdb->isPPC()) {
		if (_buttons & kButtonD) {
			if (g_hdb->_window->inventoryActive()) {
				g_hdb->_window->closeInv();
				g_hdb->_window->openDeliveries(false);
			} else if (g_hdb->_window->deliveriesActive()) {
				g_hdb->_window->closeDlvs();
			} else {
				g_hdb->_window->openInventory();
			}
			return;
		}
	}

	// Debug Mode Cycling
	if ((_buttons & kButtonExit) && g_hdb->getCheatingOn()) {
		int	debugFlag = g_hdb->getDebug();
		debugFlag++;
		if (debugFlag > 2)
			debugFlag = 0;
		g_hdb->setDebug(debugFlag);

		if (debugFlag == 2)
			g_hdb->_ai->clearWaypoints();

		if (!debugFlag && g_hdb->getGameState() == GAME_PLAY) {
			int	x, y;
			g_hdb->_ai->getPlayerXY(&x, &y);
			g_hdb->_map->centerMapXY(x + 16, y + 16);	// point to center of player
		}
	}

	if (g_hdb->getGameState() == GAME_PLAY) {
		// Is Player Dead? Click on TRY AGAIN
		if (g_hdb->_ai->playerDead()) {
			// TRY AGAIN is onscreen...
			if (_buttons & kButtonB) {
				if (g_hdb->loadGameState(kAutoSaveSlot).getCode() == Common::kNoError) {
					g_hdb->_window->clearTryAgain();
					g_hdb->setGameState(GAME_PLAY);
				}
			}
			return;
		}

		if (g_hdb->isPPC()) {
			// Deliveries screen?
			if (g_hdb->_window->deliveriesActive() && !g_hdb->_window->animatingDelivery()) {
				if (_buttons & kButtonLeft) {
					int	amount = g_hdb->_ai->getDeliveriesAmount();
					int	current = g_hdb->_window->getSelectedDelivery();

					if (!current)
						current = amount - 1;
					else
						current--;

					g_hdb->_window->setSelectedDelivery(current);
				} else if (_buttons & kButtonRight) {
					int amount = g_hdb->_ai->getDeliveriesAmount();
					int current = g_hdb->_window->getSelectedDelivery();

					current++;
					if (current == amount)
						current = 0;

					g_hdb->_window->setSelectedDelivery(current);
				} else if (_buttons & kButtonB)
					g_hdb->_window->closeDlvs();
				return;
			}

			//
			// Resources screen?  Move select cursor around
			//
			if (g_hdb->_window->inventoryActive()) {
				// select weapon?
				if (_buttons & kButtonB) {
					static AIType lastWeaponSelected = AI_NONE;

					if (!g_hdb->getActionMode()) {
						g_hdb->_window->closeInv();
						return;
					}

					AIType t = g_hdb->_ai->getInvItemType(g_hdb->_window->getInvSelect());
					Tile *gfx = g_hdb->_ai->getInvItemGfx(g_hdb->_window->getInvSelect());

					switch (t) {
					case ITEM_CLUB:
					case ITEM_ROBOSTUNNER:
					case ITEM_SLUGSLINGER:
						g_hdb->_ai->setPlayerWeapon(t, gfx);
						if (t == lastWeaponSelected) {
							g_hdb->_window->closeInv();
							return;
						}
						lastWeaponSelected = t;
						g_hdb->_sound->playSound(SND_MENU_ACCEPT);
						return;
					default:
						break;
					}
					g_hdb->_sound->playSound(SND_CELLHOLDER_USE_REJECT);
					return;
				}


				if (_buttons & kButtonLeft) {
					int	amount = g_hdb->_ai->getInvMax();
					int	current = g_hdb->_window->getInvSelect();

					if (!amount)
						return;

					if (current == 5)
						current = amount - 1;
					else if (!current && amount > 5)
						current = 4;
					else if (!current)
						current = amount - 1;
					else
						current--;

					g_hdb->_sound->playSound(SND_MENU_SLIDER);
					g_hdb->_window->setInvSelect(current);
				} else if (_buttons & kButtonRight) {
					int amount = g_hdb->_ai->getInvMax();
					int current = g_hdb->_window->getInvSelect();

					if (!amount)
						return;

					if (amount > 5) {
						if (current == amount - 1)
							current = 5;
						else if (current == 4)
							current = 0;
						else
							current++;
					} else if (current == amount - 1)
						current = 0;
					else
						current++;

					g_hdb->_sound->playSound(SND_MENU_SLIDER);
					g_hdb->_window->setInvSelect(current);
				} else if (_buttons & kButtonUp) {
					int	amount = g_hdb->_ai->getInvMax();
					int	current = g_hdb->_window->getInvSelect();

					if (amount < 6)
						return;

					if (current - 5 >= 0)
						current -= 5;

					g_hdb->_sound->playSound(SND_MENU_SLIDER);
					g_hdb->_window->setInvSelect(current);
				} else if (_buttons & kButtonDown) {
					int amount = g_hdb->_ai->getInvMax();
					int current = g_hdb->_window->getInvSelect();

					if (amount < 6)
						return;

					if (current + 5 < amount)
						current += 5;
					else if (current < 5)
						current = amount - 1;

					g_hdb->_sound->playSound(SND_MENU_SLIDER);
					g_hdb->_window->setInvSelect(current);
				}
				return;
			}
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
	static uint32 delay = 0, time;

	// Don't let the screen get clicked too fast
	time = g_system->getMillis();
	if (time - delay < 100)
		return;
	time = delay;

	GameState gs = g_hdb->getGameState();

	switch (gs) {
	case GAME_TITLE:
		g_hdb->_menu->changeToMenu();
		g_hdb->changeGameState();
		break;
	case GAME_MENU:
		g_hdb->_menu->processInput(x, y);
		break;
	case GAME_PLAY:
		{
		// Is Player Dead? Click on TRY AGAIN
		if (g_hdb->_ai->playerDead()) {
			if (y >= g_hdb->_window->_tryRestartY && y <= g_hdb->_window->_tryRestartY + 24) {
				if (g_hdb->loadGameState(kAutoSaveSlot).getCode() == Common::kNoError) {
					g_hdb->_window->clearTryAgain();
					g_hdb->setGameState(GAME_PLAY);
				}
			}
			return;
		}

		if (g_hdb->isPPC()) {
			// is Deliveries active?
			if (g_hdb->_window->deliveriesActive()) {
				if (!g_hdb->_window->checkDlvsClose(x, y))
					return;
				if (!g_hdb->_ai->cinematicsActive())
					return;
			}

			// is Inventory active?
			if (g_hdb->_window->inventoryActive()) {
				if (!g_hdb->_window->checkInvClose(x, y))
					return;
				if (!g_hdb->_ai->cinematicsActive())
					return;
			}
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
		if (g_hdb->_window->msgBarActive()) {
			if (g_hdb->_window->checkMsgClose(x, y))
				return;
		}

		// In a cinematic?
		if (g_hdb->_ai->playerLocked())
			return;

		// Check for map dragging in debug Mode and place player there
		if ((GAME_PLAY == g_hdb->getGameState()) && g_hdb->getDebug() == 2) {
			int mx, my;
			g_hdb->_map->getMapXY(&mx, &my);

			mx = ((mx + x) / kTileWidth) * kTileWidth;
			my = ((my + y) / kTileHeight) * kTileHeight;
			g_hdb->_ai->setPlayerXY(mx, my);

			g_hdb->startMoveMap(x, y);
			return;
		}

		// Clicked in the world
		int worldX, worldY;
		g_hdb->_map->getMapXY(&worldX, &worldY);
		worldX = ((worldX + x) / kTileWidth) * kTileWidth;
		worldY = ((worldY + y) / kTileHeight) * kTileHeight;

		if (!g_hdb->isPPC()) {
			// Don't allow a click into INV/DELIVERIES area to go into the world
			if (x >= (g_hdb->_screenWidth - 32 * 5))
				return;
		}

		// Double-Clicking on the player to open inventory?
		int nx, ny;
		g_hdb->_ai->getPlayerXY(&nx, &ny);
		if (g_hdb->isPPC()) {
			if (nx == worldX && ny == worldY) {
				static uint32 dblClickTimer = 0;

				if (dblClickTimer && ((int)(g_system->getMillis() - dblClickTimer) < (int)(kGameFPS * 1000 / 60))) {
					g_hdb->_window->openInventory();
					dblClickTimer = 0;
					g_hdb->_ai->togglePlayerRunning();
					if (g_hdb->_ai->playerRunning())
						g_hdb->_window->centerTextOut("Running Speed", g_hdb->_screenHeight - 32, kRunToggleDelay * kGameFPS);
					else
						g_hdb->_window->centerTextOut("Walking Speed", g_hdb->_screenHeight - 32, kRunToggleDelay * kGameFPS);
					g_hdb->_sound->playSound(SND_SWITCH_USE);
					return;
				} else
					dblClickTimer = g_system->getMillis();
			}
		}

		// Toggle Walk Speed if we clicked Player
		static uint32 lastRunning = g_system->getMillis();
		if (nx == worldX && ny == worldY) {
			if (lastRunning > g_system->getMillis())
				return;
			lastRunning = g_system->getMillis() + 1000 * kRunToggleDelay;
			g_hdb->_ai->togglePlayerRunning();
			if (g_hdb->_ai->playerRunning())
				g_hdb->_window->centerTextOut("Running Speed", g_hdb->_screenHeight - 32, kRunToggleDelay * kGameFPS);
			else
				g_hdb->_window->centerTextOut("Walking Speed", g_hdb->_screenHeight - 32, kRunToggleDelay * kGameFPS);
			g_hdb->_sound->playSound(SND_SWITCH_USE);
		}

		g_hdb->setTargetXY(worldX, worldY);
		break;
		}
	case GAME_LOADING:
		break;
	default:
		break;
	}
}

void Input::stylusMove(int x, int y) {
	// In a cinematic?
	if (g_hdb->_ai->playerLocked() || g_hdb->_ai->playerDead())
		return;

	switch (g_hdb->getGameState()) {
	case GAME_PLAY:
		if (g_hdb->getDebug() == 2)
			g_hdb->moveMap(x, y);
		break;
	case GAME_MENU:
		g_hdb->_menu->processInput(x, y);
		break;
	default:
		break;
	}
}

void Input::updateMouse(int newX, int newY) {
	_mouseX = CLIP(newX, 0, g_hdb->_screenWidth - 1);
	_mouseY = CLIP(newY, 0, g_hdb->_screenHeight - 1);

	// Turn Cursor back on?
	if (!g_hdb->_gfx->getPointer())
		g_hdb->_gfx->showPointer(true);

	// Check if LButton is being dragged
	if (_stylusDown)
		stylusMove(_mouseX, _mouseY);
}

void Input::updateMouseButtons(bool isDown) {
	_stylusDown = isDown;

	// Check if LButton has been pressed
	// Check if LButton has been lifted
	if (isDown) {
		if (g_hdb->isPPC()) {
			stylusDown(_mouseX, _mouseY);
			return;
		}
		if (_mouseX > (g_hdb->_screenWidth - 32 * 5) && _mouseY < 240) {
			g_hdb->_window->checkInvSelect(_mouseX, _mouseY);
		} else if (_mouseX > (g_hdb->_screenWidth - 32 * 5) && _mouseY >= 240) {
			g_hdb->_window->checkDlvSelect(_mouseX, _mouseY);
		} else {
			if (g_hdb->getPause() && g_hdb->getGameState() == GAME_PLAY) {
				g_hdb->_window->checkPause(_mouseX, _mouseY);
				return;
			}
			stylusDown(_mouseX, _mouseY);
		}
	}
}

void Input::updateActions(Common::Event event, bool keyDown, bool fromMouse) {
	static bool current = false, last = false;

	if (keyDown && event.customType == kHDBActionQuit) {
		g_hdb->quitGame();
		return;
	}

	uint16 buttons = getButtons();

	// PAUSE key pressed?
	last = current;
	if (keyDown && event.customType == kHDBActionPause && g_hdb->getGameState() == GAME_PLAY) {
		current = true;
		if (!last) {
			g_hdb->togglePause();
			g_hdb->_sound->playSound(SND_POP);
		}
	} else
		current = false;

	if (!g_hdb->getPause()) {
		if (event.customType == kHDBActionUp) {
			if (keyDown) {
				buttons |= kButtonUp;
				if (g_hdb->_gfx->getPointer() && !fromMouse)
					g_hdb->_gfx->showPointer(false);
			} else {
				buttons &= ~kButtonUp;
			}
		} else if (event.customType == kHDBActionDown) {
			if (keyDown) {
				buttons |= kButtonDown;
				if (g_hdb->_gfx->getPointer() && !fromMouse)
					g_hdb->_gfx->showPointer(false);
			} else {
				buttons &= ~kButtonDown;
			}
		} else if (event.customType == kHDBActionLeft) {
			if (keyDown) {
				buttons |= kButtonLeft;
				if (g_hdb->_gfx->getPointer() && !fromMouse)
					g_hdb->_gfx->showPointer(false);
			} else {
				buttons &= ~kButtonLeft;
			}
		} else if (event.customType == kHDBActionRight) {
			if (keyDown) {
				buttons |= kButtonRight;
				if (g_hdb->_gfx->getPointer() && !fromMouse)
					g_hdb->_gfx->showPointer(false);
			} else {
				buttons &= ~kButtonRight;
			}
		} else if (event.customType == kHDBActionUse) {
			if (keyDown) {
				buttons |= kButtonB;
				if (g_hdb->_gfx->getPointer() && !fromMouse)
					g_hdb->_gfx->showPointer(false);
			} else {
				buttons &= ~kButtonB;
			}
		} else if (event.customType == kHDBActionClearWaypoints) {
			if (keyDown) {
				g_hdb->_ai->clearWaypoints();
				g_hdb->_sound->playSound(SND_POP);
			}
		}
		// TODO: Inventory key
	}

	if (event.customType == kHDBActionMenu) {
		if (keyDown) {
			buttons |= kButtonA;
			g_hdb->_gfx->showPointer(true);
			g_hdb->_menu->setMenuKey(1);
		} else {
			buttons &= ~kButtonA;
			g_hdb->_menu->setMenuKey(0);
		}
	} else if (event.customType == kHDBActionDebug) {
		if (keyDown)
			buttons |= kButtonExit;
		else
			buttons &= ~kButtonExit;
	}

	setButtons(buttons);
}

} // End of Namespace
