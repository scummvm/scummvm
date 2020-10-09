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

#ifndef STARK_SERVICES_GAME_INTERFACE_H
#define STARK_SERVICES_GAME_INTERFACE_H

#include "engines/stark/resources/pattable.h"

#include "common/scummsys.h"
#include "common/rect.h"

namespace Stark {

class VisualImageXMG;

namespace Resources {
class ItemVisual;
}

/**
 * Facade object for the user interface to interact with the game world
 */
class GameInterface {
public:
	GameInterface();
	~GameInterface();

	/**
	 * Skip currently playing speeches
	 *
	 * @return true if at least one speech was skipped
	 */
	bool skipCurrentSpeeches();

	/** Make April try to go to the location under the cursor */
	void walkTo(const Common::Point &mouse);

	VisualImageXMG *getActionImage(uint32 itemIndex, bool active);
	VisualImageXMG *getCursorImage(uint32 itemIndex);

	/** Can the item be used for an action. -1 for all actions */
	bool itemHasAction(Resources::ItemVisual *item, int32 action);
	bool itemHasActionAt(Resources::ItemVisual *item, const Common::Point &position, int32 action);

	/** Get the item's default action */
	int32 itemGetDefaultActionAt(Resources::ItemVisual *item, const Common::Point &position);

	/** Do an action on the item */
	void itemDoAction(Resources::ItemVisual *item, uint32 action);
	void itemDoActionAt(Resources::ItemVisual *item, uint32 action, const Common::Point &position);

	/** Get the item's name */
	Common::String getItemTitle(Resources::ItemVisual *object);
	Common::String getItemTitleAt(Resources::ItemVisual *object, const Common::Point &pos);

	/** List the actions available for an item in the current game state */
	Resources::ActionArray listActionsPossibleForObject(Resources::ItemVisual *item);
	Resources::ActionArray listActionsPossibleForObjectAt(Resources::ItemVisual *item, const Common::Point &pos);

	/** List the stock actions available for an item in the current game state (hand, mouth, eye) */
	Resources::ActionArray listStockActionsPossibleForObject(Resources::ItemVisual *item);
	Resources::ActionArray listStockActionsPossibleForObjectAt(Resources::ItemVisual *item, const Common::Point &pos);

	/** Check if April is doing a walk movement */
	bool isAprilWalking() const;

	/** Make April run if she is walking */
	void setAprilRunning();

	/** List all the exit positions */
	Common::Array<Common::Point> listExitPositions();
};

} // End of namespace Stark

#endif // STARK_SERVICES_GAME_INTERFACE_H
