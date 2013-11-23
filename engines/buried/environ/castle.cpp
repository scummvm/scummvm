/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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
 */

#include "buried/gameui.h"
#include "buried/invdata.h"
#include "buried/inventory_window.h"
#include "buried/scene_view.h"
#include "buried/environ/scene_base.h"

namespace Buried {

bool SceneViewWindow::initializeCastleTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	// If we passed -1, initialize time zone, otherwise the environment
	if (environment == -1) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgWallExploded = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgHookPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemGrapplingHook) ? 1 : 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgArrowPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBloodyArrow) ? 1 : 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgHammerPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemHammer) ? 1 : 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyGuard = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgBaileyOneWayGuard = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgBaileyTwoWayGuards = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgTapestryFlag = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgStorageRoomVisit = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgBurnedLetterPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBurnedLetter) ? 1 : 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgGoldCoinsPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemGoldCoins) ? 1 : 0;

		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 1) {
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = 6;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgTapestryFlag = 1;
		}
	}

	// Environment-specific
	if (environment == 4) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreEnteredKeep = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMBVisited = 1;
	} else if (environment == 6) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgKCVisited = 1;
	} else if (environment == 10) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreEnteredTreasureRoom = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgTRVisited = 1;
	}

	return true;
}

SceneBase *SceneViewWindow::constructCastleSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
