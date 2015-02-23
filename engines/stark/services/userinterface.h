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

#include "engines/stark/gfx/renderentry.h"

#include "common/scummsys.h"
#include "common/rect.h"

namespace Stark {

class ActionMenu;
class VisualImageXMG;
class Cursor;

namespace Resources {
class Item;
class Object;
}

namespace Gfx {
class Driver;
class Texture;
class RenderEntry;
}

/**
 * Facade object for interacting with the game world
 */
class UserInterface {
public:
	UserInterface(Gfx::Driver *driver, const Cursor *cursor);
	~UserInterface();

	void init();

	/** Skip currently playing speeches */
	void skipCurrentSpeeches();

	/** Scroll the current location by an offset */
	void scrollLocation(int32 dX, int32 dY);

	/** Draw the mouse pointer, and any additional currently active UI */
	void render();
	
	/** Update the current state of the user interface */
	void update();

	Gfx::RenderEntry *getEntryAtPosition(Common::Point, Gfx::RenderEntryArray entries);
	
	Resources::Object *getObjectForRenderEntryAtPosition(Common::Point pos, Gfx::RenderEntry *entry);

	Common::String getMouseHintForObject(Resources::Object *object);

	enum ActionFlags {
		kActionNonePossible = 0,
		kActionUsePossible = 1,
		kActionLookPossible = 2,
		kActionTalkPossible = 4,
		kActionExitPossible = 8
	};

	int getActionsPossibleForObject(Resources::Object *object);
	/** Attempt to run the relevant action on the object, returns true if action menu is needed, false if no action is possible */
	bool performActionOnObject(Resources::Object *object, Resources::Object *activeObject);
	void activateActionMenuOn(Common::Point pos, Resources::Object *activeObject);
	bool isActionMenuOpen() const { return _actionMenuActive; }
	void deactivateActionMenu() { _actionMenuActive = false; }
private:

	int _indexForCurrentObject;
	Resources::Object *_object;
	bool _interfaceVisible;
	bool _actionMenuActive;
	Common::Point _actionMenuPos;
	const Cursor *_cursor;
	VisualImageXMG *_exitButton;
	ActionMenu *_actionMenu;
	Gfx::Driver *_gfx;
};

} // End of namespace Stark

#endif // STARK_SERVICES_USER_INTERFACE_H
