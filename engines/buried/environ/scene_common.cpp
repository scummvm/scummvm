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
#include "buried/sound.h"
#include "buried/scene_view.h"
#include "buried/environ/scene_common.h"

namespace Buried {

BasicDoor::BasicDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int timeZone, int environment, int node, int facing,
		int orientation, int depth, int transitionType, int transitionData,
		int transitionStartFrame, int transitionLength, int openingSoundID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clicked = false;

	_clickable = Common::Rect(left, top, right, bottom);

	_destData.destinationScene.timeZone = timeZone;
	_destData.destinationScene.environment = environment;
	_destData.destinationScene.node = node;
	_destData.destinationScene.facing = facing;
	_destData.destinationScene.orientation = orientation;
	_destData.destinationScene.depth = depth;

	_destData.transitionType = transitionType;
	_destData.transitionData = transitionData;
	_destData.transitionStartFrame = transitionStartFrame;
	_destData.transitionLength = transitionLength;

	_openingSoundID = openingSoundID;

	if (viewWindow) {
		((SceneViewWindow *)viewWindow)->changeStillFrameMovie(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, SF_STILLS));

		if (_staticData.cycleStartFrame >= 0)
			((SceneViewWindow *)viewWindow)->changeCycleFrameMovie(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, SF_CYCLES));
	}
}

int BasicDoor::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickable.contains(pointLocation))
		_clicked = true;

	return SC_TRUE;
}

int BasicDoor::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clicked) {
		_clicked = false;

		if (_openingSoundID >= 0)
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _openingSoundID));

		if (_clickable.contains(pointLocation))
			((SceneViewWindow *)viewWindow)->moveToDestination(_destData);		
	}

	return SC_TRUE;
}

int BasicDoor::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	//debug("(%d, %d) in (%d, %d, %d, %d)", pointLocation.x, pointLocation.y, _clickable.left, _clickable.top, _clickable.right, _clickable.bottom);

	if (_clickable.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

} // End of namespace Buried
