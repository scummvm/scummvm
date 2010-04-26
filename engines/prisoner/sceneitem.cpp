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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"

namespace Prisoner {

/* Scene items */

int16 PrisonerEngine::addSceneItem(int16 inventoryItemIndex, int16 pathNodeIndex) {

	int16 sceneItemIndex = _sceneItems.getFreeSlot();
	SceneItem *sceneItem = &_sceneItems[sceneItemIndex];

	InventoryItem *inventoryItem = &_inventoryItems[inventoryItemIndex];

	Common::String pakName;
	int16 pakSlot;
	_res->getSlotInfo(inventoryItem->resourceCacheSlot, pakName, pakSlot);

	sceneItem->actorIndex = addActor(pakName, pakSlot, inventoryItem->id + 2, pathNodeIndex, 0, 0);
	sceneItem->inventoryItemIndex = inventoryItemIndex;

	return sceneItemIndex;
}

void PrisonerEngine::removeSceneItem(int16 inventoryItemIndex) {
	for (int16 sceneItemIndex = 0; sceneItemIndex < _sceneItems.count(); sceneItemIndex++) {
		SceneItem *sceneItem = &_sceneItems[sceneItemIndex];
		if (sceneItem->inventoryItemIndex == inventoryItemIndex) {
			clearActor(sceneItem->actorIndex);
			sceneItem->clear();
			break;
		}
	}
}

void PrisonerEngine::clearSceneItemActors() {
	for (int16 sceneItemIndex = 0; sceneItemIndex < _sceneItems.count(); sceneItemIndex++) {
		if (_sceneItems[sceneItemIndex].actorIndex != -1)
			clearActor(_sceneItems[sceneItemIndex].actorIndex);
	}
}

} // End of namespace Prisoner
