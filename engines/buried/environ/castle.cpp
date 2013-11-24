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

#include "buried/buried.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/inventory_window.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

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

	switch (sceneStaticData.classID) {
	case 4:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 114, 0, 324, 189, 1, 2, 5, 3, 1, 1, 2, 11, 395, 9);
	case 5:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 103, 0, 355, 189, 1, 3, 5, 1, 1, 1, 2, 11, 641, 8);
	case 7:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgWallExploded), 0, 0, 1, 0, 0);
	case 8:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgWallExploded), 0, 1, 0, 0, 0);
	case 9:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 81, 25, 360, 189, 1, 4, 2, 1, 1, 1, 2, 11, 413, 25);
	case 10:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 24, 5, 415, 189, 1, 5, 0, 2, 1, 1, 2, 11, 72, 22);
	case 16:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 131, 18, 322, 189, 1, 8, 10, 1, 1, 1, 2, 11, 307, 7);
	case 17:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 42, 0, 357, 189, 1, 8, 11, 3, 1, 1, 2, 11, 314, 7);
	case 18:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 26, 0, 432, 189, 1, 8, 6, 3, 1, 1, 2, 11, 288, 8);
	case 20:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHookPresent), 0, 0, 1, 0, 0);
	case 21:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHookPresent), 0, 1, 0, 0, 0);
	case 23:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgArrowPresent), 0, 0, 1, 0, 0);
	case 24:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgArrowPresent), 0, 1, 0, 0, 0);
	case 26:
		if (_vm->isDemo())
			return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHammerPresent), 0, 0, 1, 0, 0);
		break;
	case 27:
		if (_vm->isDemo())
			return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHammerPresent), 0, 1, 0, 0, 0);
		break;
	case 29:
		if (_vm->isDemo())
			return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHammerPresent), 0, 0, 0, 1, 0);
		break;
	case 30:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHookPresent), 0, 0, 0, 1, 0);
	case 31:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 1, kCursorFinger, 51, 25, 149, 149);
	case 32:
		return new OneShotEntryVideoWarning(_vm, viewWindow, sceneStaticData, priorLocation, 0, offsetof(GlobalFlags, cgSmithyGuard), IDS_HUMAN_PRESENCE_10METERS);
	case 33:
		return new OneShotEntryVideoWarning(_vm, viewWindow, sceneStaticData, priorLocation, _vm->isDemo() ? 2 : 6, offsetof(GlobalFlags, cgBaileyOneWayGuard), IDS_HUMAN_PRESENCE_10METERS);
	case 34:
		return new CycleEntryVideoWarning(_vm, viewWindow, sceneStaticData, priorLocation, _vm->isDemo() ? 5 : 7, _vm->isDemo() ? 6 : 8, offsetof(GlobalFlags, cgBaileyTwoWayGuards), IDS_HUMAN_PRESENCE_10METERS);
	case 47:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 2, kCursorFinger, 0, 75, 258, 123);
	default:
		warning("TODO: Castle scene object %d", sceneStaticData.classID);
		break;
	}

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
