/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/environ/scene_base.h"

namespace Buried {

SceneBase::SceneBase(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) : _vm(vm), _staticData(sceneStaticData) {
	_frameCycleCount = _staticData.cycleStartFrame;

	((SceneViewWindow *)viewWindow)->changeStillFrameMovie(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, SF_STILLS));

	if (_staticData.cycleStartFrame >= 0)
		((SceneViewWindow *)viewWindow)->changeCycleFrameMovie(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, SF_CYCLES));
}

int SceneBase::paint(Window *viewWindow, Graphics::Surface *preBuffer) {
	const Graphics::Surface *newFrame = nullptr;

	if (_frameCycleCount >= 0) {
		newFrame = ((SceneViewWindow *)viewWindow)->getCycleFrame(_frameCycleCount);

		if (!newFrame)
			newFrame = ((SceneViewWindow *)viewWindow)->getStillFrame(_staticData.navFrameIndex);
	} else if (_staticData.navFrameIndex >= 0) {
		newFrame = ((SceneViewWindow *)viewWindow)->getStillFrame(_staticData.navFrameIndex);
	}

	if (newFrame)
		_vm->_gfx->crossBlit(preBuffer, 0, 0, 432, 189, newFrame, 0, 0);

	return SC_REPAINT;
}

int SceneBase::timerCallback(Window *viewWindow) {
	// Check if we're cycling
	if (_frameCycleCount < 0)
		return SC_FALSE;

	// Increment and wrap cycle counter
	_frameCycleCount++;
	if (_frameCycleCount > (_staticData.cycleStartFrame + _staticData.cycleFrameCount - 1))
		_frameCycleCount = _staticData.cycleStartFrame;

	// Update the view window
	viewWindow->invalidateWindow(false);
	return SC_TRUE;
}

int SceneBase::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	return (int)kCursorArrow;
}

} // End of namespace Buried
