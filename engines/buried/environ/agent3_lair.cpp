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
#include "buried/inventory_window.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

namespace Buried {

bool SceneViewWindow::initializeAgent3LairTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	if (environment == -1)
		((SceneViewWindow *)viewWindow)->getGlobalFlags().alNMWrongAlienPrefixCode = 0;

	return true;
}

bool SceneViewWindow::startAgent3LairAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	_vm->_sound->setAmbientSound(_vm->getFilePath(3, environment, SF_AMBIENT), fade, 64);
	return true;
}

SceneBase *SceneViewWindow::constructAgent3LairSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	switch (sceneStaticData.classID) {
	case 1:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 177, 96, 231, 184, kItemGeneratorCore, 15, offsetof(GlobalFlags, alRDTakenLiveCore));
	case 20:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 36, 15, 396, 189, kCursorFinger, 3, 2, 0, 1, 1, 1, TRANSITION_VIDEO, 0, -1, -1);
	case 25:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 150, 24, 280, 124, kCursorFinger, 3, 2, 4, 0, 1, 1, TRANSITION_VIDEO, 6, -1, -1);
	}

	warning("TODO: Agent 3 lair scene object %d", sceneStaticData.classID);

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
