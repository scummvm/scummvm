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
#include "buried/graphics.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

#include "common/system.h"

namespace Buried {

class NormalTransporter : public SceneBase {
public:
	NormalTransporter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _clickRegion;
};

NormalTransporter::NormalTransporter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(118, 93, 153, 125);
}

int NormalTransporter::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asAmbassadorEncounter == 1) {
			_vm->_sound->setAmbientSound();
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(1);
			_staticData.navFrameIndex = -1;
			((SceneViewWindow *)viewWindow)->showCompletionScene();
		} else {
			DestinationScene destData;
			destData.destinationScene = Location(3, 2, 4, 0, 1, 0);
			destData.transitionType = TRANSITION_VIDEO;
			destData.transitionData = 1;
			destData.transitionStartFrame = -1;
			destData.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int NormalTransporter::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

bool SceneViewWindow::initializeAlienTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	if (environment == -1) {
		GlobalFlags &flags = ((SceneViewWindow *)viewWindow)->getGlobalFlags();

		flags.asInitialGuardsPass = 0;
		flags.asRBPodAStatus = 0;
		flags.asRBPodBStatus = 0;
		flags.asRBPodCStatus = 0;
		flags.asRBPodDStatus = 0;
		flags.asRBPodEStatus = 0;
		flags.asRBPodFStatus = 0;
	} else if (environment == 1) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreTransportToKrynn = 1;
	}

	return true;
}

bool SceneViewWindow::startAlienAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	_vm->_sound->setAmbientSound(_vm->getFilePath(7, environment, SF_AMBIENT), oldTimeZone == 7 && fade, 64);
	return true;
}


SceneBase *SceneViewWindow::constructAlienSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	switch (sceneStaticData.classID) {
	case 12:
		return new NormalTransporter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 50:
		return new PlayStingers(_vm, viewWindow, sceneStaticData, priorLocation, 127, offsetof(GlobalFlags, asRBLastStingerID), offsetof(GlobalFlags, asRBStingerID), 10, 14);
	}

	warning("TODO: Alien scene object %d", sceneStaticData.classID);
	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
