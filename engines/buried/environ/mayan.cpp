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
#include "buried/invdata.h"
#include "buried/inventory_window.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"

namespace Buried {

bool SceneViewWindow::initializeMayanTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	if (environment == -1) {
		GlobalFlags &flags = ((SceneViewWindow *)viewWindow)->getGlobalFlags();

		flags.myTPCodeWheelStatus = flags.generalWalkthroughMode;
		flags.myTPCodeWheelLeftIndex = flags.generalWalkthroughMode == 1 ? 8 : 0;
		flags.myTPCodeWheelRightIndex = flags.generalWalkthroughMode == 1 ? 12 : 0;
		flags.myMCDeathGodOfferings = 0;
		flags.myWGPlacedRope = flags.generalWalkthroughMode;
		flags.myWTCurrentBridgeStatus = 0;
		flags.myAGHeadAStatus = flags.generalWalkthroughMode == 1 ? 2 : 0;
		flags.myAGHeadBStatus = 0;
		flags.myAGHeadCStatus = 0;
		flags.myAGHeadDStatus = flags.generalWalkthroughMode == 1 ? 2 : 0;
		flags.myAGHeadAStatusSkullID = flags.generalWalkthroughMode == 1 ? kItemCavernSkull : 0;
		flags.myAGHeadBStatusSkullID = 0;
		flags.myAGHeadCStatusSkullID = 0;
		flags.myAGHeadDStatusSkullID = flags.generalWalkthroughMode == 1 ? kItemSpearSkull : 0;
		flags.myAGTimerHeadID = 0;
		flags.myAGTimerStartTime = 0;
		flags.myDGOfferedHeart = 0;
		flags.myAGHeadAOpenedTime = 0;
		flags.myAGHeadBOpenedTime = 0;
		flags.myAGHeadCOpenedTime = 0;
		flags.myAGHeadDOpenedTime = 0;

		flags.myPickedUpCeramicBowl = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemCeramicBowl) ? 1 : 0;
		flags.myMCPickedUpSkull = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemCavernSkull) ? 1 : 0;
		flags.myWGRetrievedJadeBlock = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemJadeBlock) ? 1 : 0;
		flags.myWTRetrievedLimestoneBlock = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemLimestoneBlock) ? 1 : 0;
		flags.myAGRetrievedEntrySkull = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemEntrySkull) ? 1 : 0;
		flags.myAGRetrievedSpearSkull = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemSpearSkull) ? 1 : 0;
		flags.myAGRetrievedCopperMedal = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemCopperMedallion) ? 1 : 0;
		flags.myAGRetrievedObsidianBlock = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemObsidianBlock) ? 1 : 0;
		flags.takenEnvironCart = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemEnvironCart) ? 1 : 0;

		if (flags.generalWalkthroughMode == 1) {
			flags.myMCPickedUpSkull = 1;
			flags.myAGRetrievedSpearSkull = 1;
			flags.myAGRetrievedCopperMedal = 1;
		}
	} else if (environment == 2) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreEnteredMainCavern = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedMainCavern = 1;
	} else if (environment == 3) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedWealthGod = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedSpecRooms = 1;
	} else if (environment == 4) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedWaterGod = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedSpecRooms = 1;
	} else if (environment == 5) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedArrowGod = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedSpecRooms = 1;
	} else if (environment == 6) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedDeathGod = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedSpecRooms = 1;
	}

	return true;
}

bool SceneViewWindow::startMayanAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	// TODO: Fix sound fading
	fade = false;
	bool checkFade = false;

	if (environment == 3) {
		if (oldEnvironment == 2)
			return _vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), checkFade, 64);

		return _vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), checkFade, 16); 
	} else if (environment == 4) {
		if (oldTimeZone == -2)
			_vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), fade, 64);
		else
			_vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), fade, 16);

		return _vm->_sound->setSecondaryAmbientSound(_vm->getFilePath(2, environment, 13), checkFade, 0);
	} else if (environment == 5) {
		_vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), fade, 64);
		return _vm->_sound->setSecondaryAmbientSound(_vm->getFilePath(2, environment, 12), checkFade, 128);
	}

	_vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), false /* fade */, 64);
	return true;
}

SceneBase *SceneViewWindow::constructMayanSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	warning("TODO: Mayan scene object %d", sceneStaticData.classID);
	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
