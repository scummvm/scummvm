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

namespace Buried {

class ClickZoomToyShelf : public SceneBase {
public:
	ClickZoomToyShelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);

	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _toyZooms[4];
};

ClickZoomToyShelf::ClickZoomToyShelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) : SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_toyZooms[0] = Common::Rect(102, 8, 170, 108);
	_toyZooms[1] = Common::Rect(188, 24, 228, 108);
	_toyZooms[2] = Common::Rect(282, 24, 330, 108);
	_toyZooms[3] = Common::Rect(330, 28, 366, 108);
}

int ClickZoomToyShelf::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 4; i++) {
		if (_toyZooms[i].contains(pointLocation)) {
			DestinationScene newScene;
			newScene.destinationScene = _staticData.location;
			newScene.destinationScene.depth = i + 1;
			newScene.transitionType = TRANSITION_VIDEO;
			newScene.transitionStartFrame = -1;
			newScene.transitionLength = -1;

			static const int transitionData[4] = { 13, 16, 19, 22 };
			newScene.transitionData = transitionData[i];

			((SceneViewWindow *)viewWindow)->moveToDestination(newScene);

			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int ClickZoomToyShelf::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 4; i++)
		if (_toyZooms[i].contains(pointLocation))
			return kCursorMagnifyingGlass;

	return kCursorArrow;
}

bool SceneViewWindow::startFutureApartmentAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	_vm->_sound->setAmbientSound(_vm->getFilePath(4, environment, SF_AMBIENT));
	return true;
}

SceneBase *SceneViewWindow::constructFutureApartmentSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	switch (sceneStaticData.classID) {
	case 30:
		return new PlayStingers(_vm, viewWindow, sceneStaticData, priorLocation, 128, offsetof(GlobalFlags, faStingerID), offsetof(GlobalFlags, faStingerChannelID), 10, 14);
	case 37:
		return new ClickZoomToyShelf(_vm, viewWindow, sceneStaticData, priorLocation);
	}

	warning("TODO: Future apartment scene object %d", sceneStaticData.classID);

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
