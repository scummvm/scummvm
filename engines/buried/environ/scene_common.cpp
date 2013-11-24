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
	if (_clickable.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

TurnDepthPreChange::TurnDepthPreChange(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int flagOffset, int upDepth, int leftDepth, int rightDepth, int downDepth, int forwardDepth) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(flagOffset)) {
		if (upDepth >= 0)
			_staticData.destUp.destinationScene.depth = upDepth;

		if (leftDepth >= 0)
			_staticData.destUp.destinationScene.depth = leftDepth;

		if (rightDepth >= 0)
			_staticData.destUp.destinationScene.depth = rightDepth;

		if (downDepth >= 0)
			_staticData.destUp.destinationScene.depth = downDepth;

		if (forwardDepth >= 0)
			_staticData.destUp.destinationScene.depth = forwardDepth;
	}
}

PlayStingers::PlayStingers(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int stingerVolume, int lastStingerFlagOffset, int effectIDFlagOffset, int firstStingerFileID, int lastStingerFileID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_stingerVolume = stingerVolume;
	_lastStingerFlagOffset = lastStingerFlagOffset;
	_effectIDFlagOffset = effectIDFlagOffset;
	_firstStingerFileID = firstStingerFileID;
	_lastStingerFileID = lastStingerFileID;
}

int PlayStingers::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_effectIDFlagOffset >= 0) {
		// More evil.
		byte effectID = ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_effectIDFlagOffset);

		if (!_vm->_sound->isSoundEffectPlaying(effectID - 1)) {
			byte lastStinger = ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_lastStingerFlagOffset);
			lastStinger++;

			uint32 fileNameIndex = _vm->computeFileNameResourceID(_staticData.location.timeZone, _staticData.location.environment, _firstStingerFileID + lastStinger - 1);
			byte newStingerID = _vm->_sound->playSoundEffect(_vm->getFilePath(fileNameIndex), _stingerVolume, false, true) + 1;

			if (lastStinger > _lastStingerFileID - _firstStingerFileID)
				lastStinger = 0;

			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_effectIDFlagOffset, newStingerID);
			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_lastStingerFlagOffset, lastStinger);
		}
	}

	return SC_TRUE;
}

OneShotEntryVideoWarning::OneShotEntryVideoWarning(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animID, int flagOffset, int warningMessageID) : SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_animID = animID;
	_flagOffset = flagOffset;
	_warningMessageID = warningMessageID;
}

int OneShotEntryVideoWarning::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_flagOffset) == 0) {
		if (_warningMessageID >= 0)
			((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(_warningMessageID));

		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animID);
		((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagOffset, 1);
	}

	return SC_TRUE;
}

CycleEntryVideoWarning::CycleEntryVideoWarning(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animIDA, int animIDB, int flagOffset, int warningMessageID) : SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_animIDA = animIDA;
	_animIDB = animIDB;
	_flagOffset = flagOffset;
	_warningMessageID = warningMessageID;
}

int CycleEntryVideoWarning::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_warningMessageID >= 0)
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(_warningMessageID));

	if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_flagOffset) == 0) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animIDA);
		((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagOffset, 1);
	} else {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animIDB);
		((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagOffset, 0);
	}

	return SC_TRUE;
}

ClickPlayVideo::ClickPlayVideo(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animID, int cursorID, int left, int top, int right, int bottom)
		: SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	
	_cursorID = cursorID;
	_animID = animID;
	_clickRegion = Common::Rect(left, top, right, bottom);
}

int ClickPlayVideo::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animID);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickPlayVideo::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

} // End of namespace Buried
