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

#ifndef STARK_SERVICES_USER_INTERFACE_H
#define STARK_SERVICES_USER_INTERFACE_H

#include "common/scummsys.h"
#include "common/str-array.h"

namespace Stark {

namespace Gfx {
class Driver;
}

class ActionMenu;
class DialogPanel;
class InventoryWindow;
class TopMenu;
class Cursor;
class FMVPlayer;
class GameWindow;
class Window;

/**
 * Facade object for interacting with the user interface from the rest of the engine
 */
class UserInterface {
public:
	UserInterface(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~UserInterface();
	
	void init();
	void update();
	void render();
	void handleClick();
	void handleRightClick();
	void notifyShouldExit() { _exitGame = true; }
	void notifyShouldOpenInventory();
	void notifyFMVRequest(const Common::String &name);
	bool isPlayingFMV() const;
	void stopPlayingFMV();
	bool shouldExit() { return _exitGame; }

	/** Can the player interact with the game world? */
	bool isInteractive() const;

	/** Allow or forbid interaction with the game world */
	void setInteractive(bool interactive);

private:
	ActionMenu *_actionMenu;
	FMVPlayer *_fmvPlayer;
	DialogPanel *_dialogPanel;
	InventoryWindow *_inventoryWindow;
	TopMenu *_topMenu;
	GameWindow *_gameWindow;

	Common::Array<Window *> _windows;

	Gfx::Driver *_gfx;
	Cursor *_cursor;
	bool _exitGame;

	bool _interactive;
};

} // End of namespace Stark

#endif // STARK_SERVICES_USER_INTERFACE_H
