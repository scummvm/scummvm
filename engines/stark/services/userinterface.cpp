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

#include "engines/stark/services/userinterface.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/speech.h"

#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/object.h"
#include "engines/stark/resources/pattable.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/item.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

#include "engines/stark/visual/image.h"

#include "engines/stark/cursor.h"
#include "engines/stark/scene.h"
#include "engines/stark/ui.h"

namespace Stark {

UserInterface::UserInterface() {
}

UserInterface::~UserInterface() {
}

void UserInterface::skipCurrentSpeeches() {
	Global *global = StarkServices::instance().global;
	Current *current = global->getCurrent();

	if (!current) {
		return; // No current location, nothing to do
	}

	// Get all speeches
	Common::Array<Resources::Speech *> speeches;
	speeches.push_back(global->getLevel()->listChildrenRecursive<Resources::Speech>());
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

void UserInterface::scrollLocation(int32 dX, int32 dY) {
	Global *global = StarkServices::instance().global;
	Current *current = global->getCurrent();

	if (!current) {
		return; // No current location, nothing to do
	}

	Resources::Location *location = current->getLocation();

	Common::Point scroll = location->getScrollPosition();
	scroll.x += dX;
	scroll.y += dY;
	location->setScrollPosition(scroll);
}

void UserInterface::walkTo(const Common::Point &mouse) {
	Global *global = StarkServices::instance().global;
	Scene *scene = StarkServices::instance().scene;

	Resources::Floor *floor = global->getCurrent()->getFloor();
	Resources::ItemSub10 *april = global->getCurrent()->getInteractive();

	Math::Vector3d origin, direction, intersection;
	scene->makeRayFromMouse(mouse, origin, direction);
	int32 floorFace = floor->findFaceHitByRay(origin, direction, intersection);
	if (april && floorFace >= 0) {
		// TODO: Complete, for now we just teleport to the target location
		april->setPosition3D(intersection);
		april->setFloorFaceIndex(floorFace);
	}
}

VisualImageXMG *UserInterface::getActionImage(uint32 itemIndex, bool active) {
	// Lookup the action's item in the inventory
	Global *global = StarkServices::instance().global;
	Resources::KnowledgeSet *inventory = global->getLevel()->findChildWithSubtype<Resources::KnowledgeSet>(Resources::KnowledgeSet::kInventory, true);

	// Get the visual for the action
	Resources::ItemSub2 *action = inventory->findChildWithIndex<Resources::ItemSub2>(itemIndex);
	Visual *visual = action->getActionVisual(active);

	return visual->get<VisualImageXMG>();
}

VisualImageXMG *UserInterface::getCursorImage(uint32 itemIndex) {
	// Lookup the item's item in the inventory
	Global *global = StarkServices::instance().global;
	Resources::KnowledgeSet *inventory = global->getLevel()->findChildWithSubtype<Resources::KnowledgeSet>(Resources::KnowledgeSet::kInventory, true);

	// Get the visual for the item
	Resources::ItemSub2 *item = inventory->findChildWithIndex<Resources::ItemSub2>(itemIndex);
	Visual *visual = item->getCursorVisual();

	return visual->get<VisualImageXMG>();
}

bool UserInterface::itemHasAction(Resources::ItemVisual *item, uint32 action) {
	if (action != -1) {
		return item->canPerformAction(action, 0);
	} else {
		warning("itemHasAction called with action = -1");
		return false;
	}
}

bool UserInterface::itemDoActionAt(Resources::ItemVisual *item, uint32 action, const Common::Point &position) {
	int32 hotspotIndex = item->getHotspotIndexForPoint(position);
	return item->doAction(action, hotspotIndex);
}

Common::String UserInterface::getItemTitle(Resources::ItemVisual *item, bool local, const Common::Point &pos) {
	int32 hotspotIndex = 0;
	if (local) {
		hotspotIndex = item->getHotspotIndexForPoint(pos);
	}

	return item->getHotspotTitle(hotspotIndex);
}

Resources::ActionArray UserInterface::getActionsPossibleForObject(Resources::ItemVisual *item) {
	if (item == nullptr) {
		return Resources::ActionArray();
	}

	Resources::PATTable *table = item->findChildWithIndex<Resources::PATTable>(0);
	return table->listPossibleActions();
}

Resources::ActionArray UserInterface::getActionsPossibleForObject(Resources::ItemVisual *item, const Common::Point &pos) {
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

Resources::ActionArray UserInterface::getStockActionsPossibleForObject(Resources::ItemVisual *item) {
	Resources::ActionArray actions = getActionsPossibleForObject(item);

	Resources::ActionArray stockActions;
	for (uint i = 0; i < actions.size(); i++) {
		if (actions[i] < 4) {
			stockActions.push_back(actions[i]);
		}
	}

	return stockActions;
}

Resources::ActionArray UserInterface::getStockActionsPossibleForObject(Resources::ItemVisual *item, const Common::Point &pos) {
	Resources::ActionArray actions = getActionsPossibleForObject(item, pos);

	Resources::ActionArray stockActions;
	for (uint i = 0; i < actions.size(); i++) {
		if (actions[i] < 4) {
			stockActions.push_back(actions[i]);
		}
	}

	return stockActions;
}

bool UserInterface::isInventoryObject(Resources::ItemVisual *item) {
	if (item->getIndex() < 4 || item->getSubType() != Resources::Item::kItemSub2) {
		// Do not explicitly add use on action-menu buttons.
		item = nullptr;
	}
	return item != nullptr;
}

} // End of namespace Stark
