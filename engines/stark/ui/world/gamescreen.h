/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_UI_WORLD_GAME_SCREEN_H
#define STARK_UI_WORLD_GAME_SCREEN_H

#include "engines/stark/ui/screen.h"

namespace Stark {

class ActionMenu;
class Cursor;
class DialogPanel;
class GameWindow;
class InventoryWindow;
class TopMenu;
class Window;

/**
 * Game world screen
 *
 * Container for all the game world windows
 */
class GameScreen : public Screen {
public:
	GameScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~GameScreen();

	// Screen API
	void open() override;
	void close() override;
	void handleGameLoop() override;
	void render() override;
	void onScreenChanged() override;
	void handleMouseMove() override;
	void handleClick() override;
	void handleRightClick() override;
	void handleDoubleClick() override;

	/** Get individual windows */
	InventoryWindow *getInventoryWindow() const;
	GameWindow *getGameWindow() const;
	DialogPanel *getDialogPanel() const;

	/** Clear any location dependant state */
	void reset();

	/** A new item has been added to the player's inventory */
	void notifyInventoryItemEnabled(uint16 itemIndex);

	/** A new entry has been added to the player's diary */
	void notifyDiaryEntryEnabled();


private:
	Gfx::Driver *_gfx;
	Cursor *_cursor;

	// Game Screen windows
	ActionMenu *_actionMenu;
	DialogPanel *_dialogPanel;
	InventoryWindow *_inventoryWindow;
	TopMenu *_topMenu;
	GameWindow *_gameWindow;

	// Game screen windows array
	Common::Array<Window *> _gameScreenWindows;

	typedef void (Window::*WindowHandler)();
	void dispatchEvent(WindowHandler handler);
	void pauseGame(bool pause);
};

} // End of namespace Stark

#endif // STARK_UI_WORLD_GAME_SCREEN_H
