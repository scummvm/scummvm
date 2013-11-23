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

#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/gameui.h"
#include "buried/invdata.h"
#include "buried/inventory_window.h"
#include "buried/message.h"
#include "buried/navarrow.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"

namespace Buried {

class OldApartmentSuitCap : public SceneBase {
public:
	OldApartmentSuitCap(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation)
			: SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {}
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
};

int OldApartmentSuitCap::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	// Cloak before continuing
	((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipCloak);
	((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sendMessage(new LButtonUpMessage(Common::Point(18, 130), 0));

	// Play the second piece of the intro movie
	((SceneViewWindow *)viewWindow)->playSynchronousAnimationExtern(_vm->computeFileNameResourceID(_staticData.location.timeZone, _staticData.location.environment, 3));

	// Disable cloak before continuing
	((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled = 0;
	((GameUIWindow *)viewWindow->getParent())->_navArrowWindow->enableWindow(true);
	((GameUIWindow *)viewWindow->getParent())->_sceneViewWindow->enableWindow(true);
	((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->enableWindow(true);

	Common::String liveText;
	if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0))
		liveText = _vm->getString(IDS_OLD_APT_RECALL_MESSAGE);
	else
		liveText = "Auto-recall Engaged";

	((SceneViewWindow *)viewWindow)->displayLiveText(liveText, false);

	// Jump to the future apartment
	((SceneViewWindow *)viewWindow)->timeSuitJump(4);

	return SC_TRUE;
}

bool SceneViewWindow::checkCustomAICommentDependencies(const Location &commentLocation, const AIComment &commentData) {
	// TODO
	return false;
}

bool SceneViewWindow::startEnvironmentAmbient(int oldTimeZone, int oldEnvironment, int timeZone, int environment, bool fade) {
	// TODO

	switch (timeZone) {
	case 4:
		return startFutureApartmentAmbient(oldTimeZone, environment, environment, fade);
	case 10:
		return _vm->_sound->setAmbientSound();
	}

	return false;
}

SceneBase *SceneViewWindow::constructSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// If the class ID is zero, return the default base class
	if (sceneStaticData.classID == 0)
		return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);

	switch (sceneStaticData.location.timeZone) {
	case 0: // Miscellaneous scenes
	case 2: // Mayan
	case 3: // Agent 3's Lair
	case 5: // Da Vinci
	case 6: // AI Lab
	case 7: // Alien
		// TODO
		warning("Could not create scene object for time zone %d", sceneStaticData.location.timeZone);
		break;
	case 1: // Castle
		return constructCastleSceneObject(viewWindow, sceneStaticData, priorLocation);
	case 4:
		return constructFutureApartmentSceneObject(viewWindow, sceneStaticData, priorLocation);
	case 10: // Old Apartment
		return new OldApartmentSuitCap(_vm, viewWindow, sceneStaticData, priorLocation);
	}

	return 0;
}

bool SceneViewWindow::initializeTimeZoneAndEnvironment(Window *viewWindow, int timeZone, int environment) {
	// TODO
	switch (timeZone) {
	case 1:
		return initializeCastleTimeZoneAndEnvironment(viewWindow, environment);
	case 4:
		// Nothing to do
		return true;
	}

	return false;
}

} // End of namespace Buried
