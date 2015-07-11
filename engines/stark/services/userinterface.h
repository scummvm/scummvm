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

#include "engines/stark/resources/pattable.h"

#include "common/scummsys.h"
#include "common/rect.h"

namespace Stark {

class ActionMenu;
class Cursor;
class VisualImageXMG;

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
	UserInterface();
	~UserInterface();

	/** Skip currently playing speeches */
	void skipCurrentSpeeches();

	/** Scroll the current location by an offset */
	void scrollLocation(int32 dX, int32 dY);

	/** Make April try to go to the location under the cursor */
	void walkTo(const Common::Point &mouse);

	VisualImageXMG *getActionImage(uint32 itemIndex, bool active);
	VisualImageXMG *getCursorImage(uint32 itemIndex);

	bool itemHasAction(Resources::ItemVisual *item, uint32 action);
	bool itemDoActionAt(Resources::ItemVisual *item, uint32 action, const Common::Point &position);

	Common::String getItemTitle(Resources::ItemVisual *object, bool local, const Common::Point &pos);

	Resources::ActionArray getActionsPossibleForObject(Resources::ItemVisual *item);
	Resources::ActionArray getActionsPossibleForObject(Resources::ItemVisual *item, const Common::Point &pos);

	Resources::ActionArray getStockActionsPossibleForObject(Resources::ItemVisual *item);
	Resources::ActionArray getStockActionsPossibleForObject(Resources::ItemVisual *item, const Common::Point &pos);

	bool isInventoryObject(Resources::ItemVisual *item);
};

} // End of namespace Stark

#endif // STARK_SERVICES_USER_INTERFACE_H
