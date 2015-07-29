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

#include "engines/stark/services/gameinterface.h"

#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/speech.h"

#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/item.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

#include "engines/stark/visual/image.h"

#include "engines/stark/cursor.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/userinterface.h"

namespace Stark {

GameInterface::GameInterface() {
}

GameInterface::~GameInterface() {
}

void GameInterface::skipCurrentSpeeches() {
	Current *current = StarkGlobal->getCurrent();

	if (!current) {
		return; // No current location, nothing to do
	}

	// Get all speeches
	Common::Array<Resources::Speech *> speeches;
	speeches.push_back(StarkGlobal->getLevel()->listChildrenRecursive<Resources::Speech>());
	speeches.push_back(current->getLevel()->listChildrenRecursive<Resources::Speech>());
	speeches.push_back(current->getLocation()->listChildrenRecursive<Resources::Speech>());

	// Stop them
	for (uint i = 0; i < speeches.size(); i++) {
		Resources::Speech *speech = speeches[i];
		if (speech->isPlaying()) {
			speech->stop();
		}
	}
}

void GameInterface::scrollLocation(int32 dX, int32 dY) {
	Current *current = StarkGlobal->getCurrent();

	if (!current) {
		return; // No current location, nothing to do
	}

	Resources::Location *location = current->getLocation();

	Common::Point scroll = location->getScrollPosition();
	scroll.x += dX;
	scroll.y += dY;
	location->setScrollPosition(scroll);
}

void GameInterface::walkTo(const Common::Point &mouse) {
	Resources::Floor *floor = StarkGlobal->getCurrent()->getFloor();
	Resources::MeshItem *april = StarkGlobal->getCurrent()->getInteractive();

	Math::Ray mouseRay = StarkScene->makeRayFromMouse(mouse);

	Math::Vector3d intersection;
	int32 floorFace = floor->findFaceHitByRay(mouseRay, intersection);
	if (april && floorFace >= 0) {
		// TODO: Complete, for now we just teleport to the target location
		april->setPosition3D(intersection);
		april->setFloorFaceIndex(floorFace);
	}
}

VisualImageXMG *GameInterface::getActionImage(uint32 itemIndex, bool active) {
	// Lookup the action's item in the inventory
	Resources::KnowledgeSet *inventory = StarkGlobal->getLevel()->findChildWithSubtype<Resources::KnowledgeSet>(Resources::KnowledgeSet::kInventory, true);

	// Get the visual for the action
	Resources::InventoryItem *action = inventory->findChildWithIndex<Resources::InventoryItem>(itemIndex);
	Visual *visual = action->getActionVisual(active);

	return visual->get<VisualImageXMG>();
}

VisualImageXMG *GameInterface::getCursorImage(uint32 itemIndex) {
	// Lookup the item's item in the inventory
	Resources::KnowledgeSet *inventory = StarkGlobal->getLevel()->findChildWithSubtype<Resources::KnowledgeSet>(Resources::KnowledgeSet::kInventory, true);

	// Get the visual for the item
	Resources::InventoryItem *item = inventory->findChildWithIndex<Resources::InventoryItem>(itemIndex);
	Visual *visual = item->getCursorVisual();

	return visual->get<VisualImageXMG>();
}

bool GameInterface::itemHasAction(Resources::ItemVisual *item, int32 action) {
	if (action != -1) {
		return item->canPerformAction(action, 0);
	} else {
		Resources::ActionArray actions = listActionsPossibleForObject(item);
		return !actions.empty();
	}
}

bool GameInterface::itemHasActionAt(Resources::ItemVisual *item, const Common::Point &position, int32 action) {
	int32 hotspotIndex = item->getHotspotIndexForPoint(position);
	if (action != -1) {
		return item->canPerformAction(action, hotspotIndex);
	} else {
		Resources::ActionArray actions = listActionsPossibleForObjectAt(item, position);
		return !actions.empty();
	}
}

int32 GameInterface::itemGetDefaultActionAt(Resources::ItemVisual *item, const Common::Point &position) {
	int32 hotspotIndex = item->getHotspotIndexForPoint(position);
	Resources::PATTable *table = item->findChildWithIndex<Resources::PATTable>(hotspotIndex);
	return table->getDefaultAction();
}

void GameInterface::itemDoAction(Resources::ItemVisual *item, uint32 action) {
	item->doAction(action, 0);
}

void GameInterface::itemDoActionAt(Resources::ItemVisual *item, uint32 action, const Common::Point &position) {
	int32 hotspotIndex = item->getHotspotIndexForPoint(position);
	item->doAction(action, hotspotIndex);
}

Common::String GameInterface::getItemTitleAt(Resources::ItemVisual *item, const Common::Point &pos) {
	int32 hotspotIndex = item->getHotspotIndexForPoint(pos);
	return item->getHotspotTitle(hotspotIndex);
}

Resources::ActionArray GameInterface::listActionsPossibleForObject(Resources::ItemVisual *item) {
	if (item == nullptr) {
		return Resources::ActionArray();
	}

	Resources::PATTable *table = item->findChildWithIndex<Resources::PATTable>(0);
	return table->listPossibleActions();
}

Resources::ActionArray GameInterface::listActionsPossibleForObjectAt(Resources::ItemVisual *item,
                                                                     const Common::Point &pos) {
	if (item == nullptr) {
		return Resources::ActionArray();
	}

	int index = item->getHotspotIndexForPoint(pos);
	if (index < 0) {
		warning("Position is outside of item '%s'", item->getName().c_str());
		return Resources::ActionArray();
	}

	Resources::PATTable *table = item->findChildWithIndex<Resources::PATTable>(index);
	return table->listPossibleActions();
}

Resources::ActionArray GameInterface::listStockActionsPossibleForObject(Resources::ItemVisual *item) {
	Resources::ActionArray actions = listActionsPossibleForObject(item);

	Resources::ActionArray stockActions;
	for (uint i = 0; i < actions.size(); i++) {
		if (actions[i] < 4) {
			stockActions.push_back(actions[i]);
		}
	}

	return stockActions;
}

Resources::ActionArray GameInterface::listStockActionsPossibleForObjectAt(Resources::ItemVisual *item,
                                                                          const Common::Point &pos) {
	Resources::ActionArray actions = listActionsPossibleForObjectAt(item, pos);

	Resources::ActionArray stockActions;
	for (uint i = 0; i < actions.size(); i++) {
		if (actions[i] < 4) {
			stockActions.push_back(actions[i]);
		}
	}

	return stockActions;
}


} // End of namespace Stark
