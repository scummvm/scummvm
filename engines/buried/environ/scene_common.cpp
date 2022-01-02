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

#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/inventory_window.h"
#include "buried/message.h"
#include "buried/navarrow.h"
#include "buried/resources.h"
#include "buried/sound.h"
#include "buried/scene_view.h"
#include "buried/environ/scene_common.h"

#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"

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
		byte &flag, int upDepth, int leftDepth, int rightDepth, int downDepth, int forwardDepth) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	if (flag) {
		if (upDepth >= 0)
			_staticData.destUp.destinationScene.depth = upDepth;

		if (leftDepth >= 0)
			_staticData.destLeft.destinationScene.depth = leftDepth;

		if (rightDepth >= 0)
			_staticData.destRight.destinationScene.depth = rightDepth;

		if (downDepth >= 0)
			_staticData.destDown.destinationScene.depth = downDepth;

		if (forwardDepth >= 0)
			_staticData.destForward.destinationScene.depth = forwardDepth;
	}
}

GenericItemAcquire::GenericItemAcquire(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int itemID, int clearStillFrame, byte &itemFlag) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation), _itemFlag(itemFlag) {
	_itemPresent = true;
	_itemID = itemID;
	_acquireRegion = Common::Rect(left, top, right, bottom);
	_fullFrameIndex = sceneStaticData.navFrameIndex;
	_clearFrameIndex = clearStillFrame;

	if (_itemFlag != 0) {
		_itemPresent = false;
		_staticData.navFrameIndex = _clearFrameIndex;
	}
}

int GenericItemAcquire::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_acquireRegion.contains(pointLocation) && _itemPresent) {
		_itemPresent = false;
		_staticData.navFrameIndex = _clearFrameIndex;

		_itemFlag = 1;

		// Call inventory drag start function
		Common::Point ptInventoryWindow = viewWindow->convertPointToGlobal(pointLocation);
		ptInventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->convertPointToLocal(ptInventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(_itemID, ptInventoryWindow);

		// Update the biochips
		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();

		return SC_TRUE;
	}

	return SC_FALSE;
}

int GenericItemAcquire::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

	if (itemID == _itemID && !_itemPresent) {
		// Redraw the background
		_itemPresent = true;
		_staticData.navFrameIndex = _fullFrameIndex;

		_itemFlag = 0;

		viewWindow->invalidateWindow();

		// Update the biochips
		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();

		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int GenericItemAcquire::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_acquireRegion.contains(pointLocation) && _itemPresent)
		return kCursorOpenHand;

	return kCursorArrow;
}

PlaySoundExitingFromScene::PlaySoundExitingFromScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int soundFileNameID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_soundFileNameID = soundFileNameID;
}

int PlaySoundExitingFromScene::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_soundFileNameID >= 0 && _staticData.location.depth != newLocation.depth && _staticData.location.timeZone == newLocation.timeZone)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _soundFileNameID), 128, false, true);

	return SC_TRUE;
}

PlaySoundExitingFromSceneDeux::PlaySoundExitingFromSceneDeux(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int soundFileNameID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_soundFileNameID = soundFileNameID;
}

int PlaySoundExitingFromSceneDeux::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_soundFileNameID >= 0 && _staticData.location.node == newLocation.node && _staticData.location.timeZone == newLocation.timeZone)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _soundFileNameID), 128, false, true);

	return SC_TRUE;
}

PlaySoundEnteringScene::PlaySoundEnteringScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int soundFileNameID, byte &flag) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation), _flag(flag) {
	_soundFileNameID = soundFileNameID;
}

int PlaySoundEnteringScene::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_flag == 0) {
		_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _soundFileNameID));
		_flag = 1;
	}

	return SC_TRUE;
}

ClickChangeScene::ClickChangeScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int cursorID,
		int timeZone, int environment, int node, int facing, int orientation, int depth,
		int transitionType, int transitionData, int transitionStartFrame, int transitionLength) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(left, top, right, bottom);
	_cursorID = cursorID;

	_clickDestination.destinationScene.timeZone = timeZone;
	_clickDestination.destinationScene.environment = environment;
	_clickDestination.destinationScene.node = node;
	_clickDestination.destinationScene.facing = facing;
	_clickDestination.destinationScene.orientation = orientation;
	_clickDestination.destinationScene.depth = depth;
	_clickDestination.transitionType = transitionType;
	_clickDestination.transitionData = transitionData;
	_clickDestination.transitionStartFrame = transitionStartFrame;
	_clickDestination.transitionLength = transitionLength;
}

int ClickChangeScene::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		((SceneViewWindow *)viewWindow)->moveToDestination(_clickDestination);

	return SC_FALSE;
}

int ClickChangeScene::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}


ClickPlayVideoSwitchAI::ClickPlayVideoSwitchAI(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animID, int cursorID, byte &flag, int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation), _flag(flag) {
	_cursorID = cursorID;
	_animID = animID;
	_clickRegion = Common::Rect(left, top, right, bottom);
}

int ClickPlayVideoSwitchAI::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		// Play the animation clip
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animID);
		_flag = 1;

		// Play any spontaneous AI comments
		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI)) {
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);
			((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickPlayVideoSwitchAI::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

ClickChangeSceneSetFlag::ClickChangeSceneSetFlag(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int cursorID,
		int timeZone, int environment, int node, int facing, int orientation, int depth,
		int transitionType, int transitionData, int transitionStartFrame, int transitionLength, byte &flag) :
		ClickChangeScene(vm, viewWindow, sceneStaticData, priorLocation, left, top, right, bottom, cursorID, timeZone, environment, node, facing, orientation, depth,
			transitionType, transitionData, transitionStartFrame, transitionLength) {
	flag = 1;
}

PlayStingers::PlayStingers(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int stingerVolume, byte &lastStingerFlag, byte &effectIDFlag, int firstStingerFileID, int lastStingerFileID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation),
		_lastStingerFlag(lastStingerFlag), _effectIDFlag(effectIDFlag) {
	_stingerVolume = stingerVolume;
	_firstStingerFileID = firstStingerFileID;
	_lastStingerFileID = lastStingerFileID;
}

int PlayStingers::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (!_vm->_sound->isSoundEffectPlaying(_effectIDFlag - 1)) {
		byte lastStinger = _lastStingerFlag;
		lastStinger++;

		uint32 fileNameIndex = _vm->computeFileNameResourceID(_staticData.location.timeZone, _staticData.location.environment, _firstStingerFileID + lastStinger - 1);
		byte newStingerID = _vm->_sound->playSoundEffect(_vm->getFilePath(fileNameIndex), _stingerVolume, false, true) + 1;

		if (lastStinger > _lastStingerFileID - _firstStingerFileID)
			lastStinger = 0;

		_effectIDFlag = newStingerID;
		_lastStingerFlag = lastStinger;
	}

	return SC_TRUE;
}

ClickPlaySound::ClickPlaySound(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		byte &flag, int soundID, int cursorID, int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation), _flag(flag) {
	_cursorID = cursorID;
	_soundID = soundID;
	_clickRegion = Common::Rect(left, top, right, bottom);
}

int ClickPlaySound::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _soundID), 127, false, true);

		_flag = 1;

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickPlaySound::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

ClickZoom::ClickZoom(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animInID, int stillInID, int animOutID, int stillOutID,
		int cursorID, int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_cursorID = cursorID;
	_animInID = animInID;
	_stillInID = stillInID;
	_animOutID = animOutID;
	_stillOutID = stillOutID;
	_zoomedIn = false;
	_clickRegion = Common::Rect(left, top, right, bottom);
	_savedNavData = _staticData;
}

int ClickZoom::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_zoomedIn) {
		_staticData.navFrameIndex = _stillOutID;
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animOutID);
		_zoomedIn = false;
		_staticData = _savedNavData;
		((GameUIWindow *)viewWindow->getParent())->_navArrowWindow->updateAllArrows(_staticData);
		return SC_TRUE;
	} else if (_clickRegion.contains(pointLocation)) {
		_staticData.navFrameIndex = _stillInID;
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animInID);
		_zoomedIn = true;
		_staticData.destUp.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destLeft.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destRight.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destDown.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		((GameUIWindow *)viewWindow->getParent())->_navArrowWindow->updateAllArrows(_staticData);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickZoom::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_zoomedIn)
		return kCursorPutDown;

	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

PlaySoundEnteringFromScene::PlaySoundEnteringFromScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int soundFileNameID, int timeZone, int environment, int node, int facing, int orientation, int depth) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_soundFileNameID = soundFileNameID;
	_soundLocation.timeZone = timeZone;
	_soundLocation.environment = environment;
	_soundLocation.node = node;
	_soundLocation.facing = facing;
	_soundLocation.orientation = orientation;
	_soundLocation.depth = depth;
}

int PlaySoundEnteringFromScene::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_soundLocation.timeZone == priorLocation.timeZone &&
			_soundLocation.environment == priorLocation.environment &&
			_soundLocation.node == priorLocation.node &&
			_soundLocation.facing == priorLocation.facing &&
			_soundLocation.orientation == priorLocation.orientation &&
			_soundLocation.depth == priorLocation.depth) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _soundFileNameID), 127, false, true);
	}

	return SC_TRUE;
}

SetFlagOnEntry::SetFlagOnEntry(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		byte &flag, byte flagNewValue) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	flag = flagNewValue;
}

InteractiveNewsNetwork::InteractiveNewsNetwork(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int enterTransition, int timeZone, int environment, int node, int facing, int orientation, int depth,
		int transitionType, int transitionData, int transitionStartFrame, int transitionLength) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	// Close the cycle/still movies
	((SceneViewWindow *)viewWindow)->changeStillFrameMovie("");
	((SceneViewWindow *)viewWindow)->changeCycleFrameMovie("");

	_currentMovieFrame = 0;
	_returnDestination.destinationScene.timeZone = timeZone;
	_returnDestination.destinationScene.environment = environment;
	_returnDestination.destinationScene.node = node;
	_returnDestination.destinationScene.facing = facing;
	_returnDestination.destinationScene.orientation = orientation;
	_returnDestination.destinationScene.depth = depth;
	_returnDestination.transitionType = transitionType;
	_returnDestination.transitionData = transitionData;
	_returnDestination.transitionStartFrame = transitionStartFrame;
	_returnDestination.transitionLength = transitionLength;
	_playingMovie = false;
	_loopingMovie = false;
	_playingAudio = false;
	_enterTransition = enterTransition;
	_audioChannel = -1;

	loadFrameDatabase();
	loadMovieDatabase();

	if (!_stillFrames.open(_vm->getFilePath(IDS_INN_STILL_FRAME_FILENAME)))
		error("Failed to open INN still frames");
}

InteractiveNewsNetwork::~InteractiveNewsNetwork() {
	// Restart sound
	_vm->_sound->restart();
}

int InteractiveNewsNetwork::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	// Play any entry animation
	if (_enterTransition >= 0)
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_enterTransition);

	// Stop the ambient sound
	_vm->_sound->setAmbientSound();

	// Play the intro movie
	_playingMovie = ((SceneViewWindow *)viewWindow)->playSynchronousAnimationExtern(IDS_INN_MEDIA_FILENAME_BASE);
	_currentMovieFrame = 1;

	// Start the INN ambient
	_vm->_sound->setAmbientSound(_vm->getFilePath(IDS_INN_AMBIENT_FILENAME));
	return SC_TRUE;
}

int InteractiveNewsNetwork::preExitRoom(Window *viewWindow, const Location &newLocation) {
	// Stop a playing movie
	if (_playingMovie) {
		((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();
		_playingMovie = false;
		_loopingMovie = false;
		_vm->_sound->restart();
	}

	// Stop audio
	if (_playingAudio && _audioChannel != -1) {
		_vm->_sound->stopSoundEffect(_audioChannel);
		_audioChannel = -1;
		_playingAudio = false;
	}

	// Stop the INN ambient
	_vm->_sound->setAmbientSound();

	// Start the environment ambient
	((SceneViewWindow *)viewWindow)->startEnvironmentAmbient(-1, -1, _staticData.location.timeZone, _staticData.location.environment);

	return SC_TRUE;
}

int InteractiveNewsNetwork::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	int oldMovieFrame = _currentMovieFrame;

	if (_currentMovieFrame > (int)_frameDatabase.size())
		return SC_FALSE;

	const INNFrame &currentData = _frameDatabase[_currentMovieFrame];

	for (int i = 0; i < 8; i++) {
		const INNHotspotData &hotspotData = currentData.hotspots[i];

		if (hotspotData.stillFrameOffset >= 0 || hotspotData.stillFrameOffset == -2) {
			Common::Rect currentRegion(hotspotData.left, hotspotData.top, hotspotData.right, hotspotData.bottom);

			if (currentRegion.contains(pointLocation)) {
				if (hotspotData.stillFrameOffset == -2) {
					// Return from a hyperlink
					if (!_hyperLinkHistory.empty()) {
						_currentMovieFrame = _hyperLinkHistory.back();
						_hyperLinkHistory.pop_back();
					}
				} else {
					// Check if we clicked on a hyperlink
					int newMovieFrame = hotspotData.stillFrameOffset - 1;

					if (i < 5 && newMovieFrame > 58 && newMovieFrame < 157)
						_hyperLinkHistory.push_back(_currentMovieFrame);

					_currentMovieFrame = newMovieFrame;
				}

				// Check for the exit frame
				if (_currentMovieFrame == 157) {
					((SceneViewWindow *)viewWindow)->moveToDestination(_returnDestination);
					return SC_TRUE;
				}

				// If we are in Agent 3's lair, and we clicked on the symbiotry talks, change the destination
				if (_staticData.location.timeZone == 3 && _currentMovieFrame == 8 && oldMovieFrame != 7) {
					_currentMovieFrame = 7;
					((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreResearchINNUpdate = 1;
				}

				// If we are playing a video, make sure it's stipped
				if (_playingMovie) {
					((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();
					_playingMovie = false;
					_loopingMovie = false;
				}

				if (_playingAudio && _audioChannel != -1) {
					_vm->_sound->stopSoundEffect(_audioChannel);
					_audioChannel = -1;
					_playingAudio = false;
				}

				// Repaint
				viewWindow->invalidateWindow();

				// Start playing any video clip
				const INNFrame &newFrame = _frameDatabase[_currentMovieFrame];

				// No full screen video -> restart the sound
				if (newFrame.pageType != MEDIA_TYPE_VIDEO_FULL)
					_vm->_sound->restart();

				if (newFrame.pageType > 0) {
					for (uint j = 0; j < _movieDatabase.size(); j++) {
						const INNMediaElement &mediaCurrentData = _movieDatabase[j];

						if (mediaCurrentData.frameIndex == _currentMovieFrame) {
							switch (mediaCurrentData.mediaType) {
							case MEDIA_TYPE_VIDEO_FULL:
								// Check for a commercial, play the sponsor clip
								if (_currentMovieFrame >= 2 && _currentMovieFrame <= 4)
									_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(IDS_INN_MEDIA_FILENAME_BASE + 39));


								_vm->_sound->stop();
								_playingMovie = ((SceneViewWindow *)viewWindow)->startAsynchronousAnimationExtern(IDS_INN_MEDIA_FILENAME_BASE + mediaCurrentData.fileIDOffset, -1, -1, -1, false);
								_loopingMovie = false;
								break;
							case MEDIA_TYPE_VIDEO_SMALL_A:
								_playingMovie = ((SceneViewWindow *)viewWindow)->startPlacedAsynchronousAnimationExtern(275, 16, 120, 120, IDS_INN_MEDIA_FILENAME_BASE + mediaCurrentData.fileIDOffset, -1, -1, -1, true);
								_loopingMovie = true;
								break;
							case MEDIA_TYPE_VIDEO_SMALL_B:
								_playingMovie = ((SceneViewWindow *)viewWindow)->startPlacedAsynchronousAnimationExtern(255, 16, 159, 120, IDS_INN_MEDIA_FILENAME_BASE + mediaCurrentData.fileIDOffset, -1, -1, -1, true);
								_loopingMovie = true;
								break;
							case MEDIA_TYPE_AUDIO:
								_playingAudio = true;
								_audioChannel = _vm->_sound->playSoundEffect(_vm->getFilePath(IDS_INN_MEDIA_FILENAME_BASE + mediaCurrentData.fileIDOffset));
								break;
							}
						}
					}
				}

				// Check for scoring frames
				switch (_currentMovieFrame) {
				case 20:
					((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreResearchINNHighBidder = 1;
					break;
				case 25:
					((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreResearchINNAppeal = 1;
					break;
				case 109:
					((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreResearchINNJumpsuit = 1;
					break;
				case 159: // Read global_flags.h to see why I hate this
					((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreResearchINNLouvreReport = 1;
					break;
				}

				return SC_TRUE;
			}
		}
	}

	return SC_FALSE;
}

int InteractiveNewsNetwork::paint(Window *viewWindow, Graphics::Surface *preBuffer) {
	const Graphics::Surface *stillFrame = _stillFrames.getFrame(_currentMovieFrame);

	if (stillFrame)
		_vm->_gfx->crossBlit(preBuffer, 0, 0, 432, 189, stillFrame, 0, 0);

	return SC_REPAINT;
}

int InteractiveNewsNetwork::movieCallback(Window *viewWindow, VideoWindow *movie, int animationID, int status) {
	// Restart sound if the movie has ended
	if (animationID == -1 && status == MOVIE_STOPPED) {
		_vm->_sound->restart();
		return SC_FALSE;
	}

	return SC_TRUE;
}

int InteractiveNewsNetwork::timerCallback(Window *viewWindow) {
	// Check to see if audio has stopped
	if (_playingAudio && _audioChannel != -1 && !_vm->_sound->isSoundEffectPlaying(_audioChannel)) {
		_audioChannel = -1;
		_playingAudio = false;
	}

	return SC_TRUE;
}

void InteractiveNewsNetwork::loadFrameDatabase() {
	Common::SeekableReadStream *frameData = _vm->getINNData(IDBD_INN_BINARY_DATA);

	if (!frameData)
		error("Failed to find INN frame database");

	uint16 count = frameData->readUint16LE();
	_frameDatabase.resize(count);

	for (uint16 i = 0; i < count; i++) {
		 INNFrame &frame = _frameDatabase[i];
		 frame.topicID = frameData->readSint16LE();
		 frame.pageType = frameData->readSint16LE();
		 frame.stillFrameOffset = frameData->readSint32LE();

		for (int j = 0; j < 8; j++) {
			frame.hotspots[j].left = frameData->readSint16LE();
			frame.hotspots[j].top = frameData->readSint16LE();
			frame.hotspots[j].right = frameData->readSint16LE();
			frame.hotspots[j].bottom = frameData->readSint16LE();
			frame.hotspots[j].stillFrameOffset = frameData->readSint32LE();
		}
	}

	delete frameData;
}

void InteractiveNewsNetwork::loadMovieDatabase() {
	Common::SeekableReadStream *movieData = _vm->getINNData(IDBD_INN_MEDIA_BINARY_DATA);

	if (!movieData)
		error("Failed to find INN movie database");

	uint16 count = movieData->readUint16LE();
	_movieDatabase.resize(count);

	for (uint16 i = 0; i < count; i++) {
		INNMediaElement &element = _movieDatabase[i];
		element.frameIndex = movieData->readSint32LE();
		element.mediaType = movieData->readSint16LE();
		element.fileIDOffset = movieData->readSint16LE();
	}

	delete movieData;
}

DisplayMessageWithEvidenceWhenEnteringNode::DisplayMessageWithEvidenceWhenEnteringNode(BuriedEngine *vm, Window *viewWindow,
			const LocationStaticData &sceneStaticData, const Location &priorLocation, int evidenceID, int messageBoxTextID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_evidenceID = evidenceID;
	_messageBoxTextID = messageBoxTextID;
}

int DisplayMessageWithEvidenceWhenEnteringNode::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if ((_staticData.location.timeZone != priorLocation.timeZone ||
			_staticData.location.environment != priorLocation.environment ||
			_staticData.location.node != priorLocation.node ||
			_staticData.location.facing != priorLocation.facing ||
			_staticData.location.orientation != priorLocation.orientation ||
			_staticData.location.depth != priorLocation.depth) &&
			!((SceneViewWindow *)viewWindow)->isNumberInGlobalFlagTable(_evidenceID)) {
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(_messageBoxTextID));
	}

	return SC_TRUE;
}

ClickPlayLoopingVideoClip::ClickPlayLoopingVideoClip(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int cursorID, int animID, int left, int top, int right, int bottom, byte &flag, int newFlagValue) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation), _flag(flag) {
	_cursorID = cursorID;
	_animID = animID;
	_clickRegion = Common::Rect(left, top, right, bottom);
	_flagValue = newFlagValue;
	_playing = false;
}

int ClickPlayLoopingVideoClip::preExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_playing) {
		((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();
		_vm->_sound->restart();
		_playing = false;
		_flag = _flagValue;
	}

	return SC_TRUE;
}

int ClickPlayLoopingVideoClip::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		if (_playing) {
			// Stop the clip
			((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();
			_playing = false;
			_vm->_sound->restart();

			// Change the flag
			_flag = _flagValue;

			// Check for spontaneous AI comments
			if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
				((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

			((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
			return SC_TRUE;
		} else {
			// Start playing asynchronously
			_vm->_sound->stop();
			_playing = ((SceneViewWindow *)viewWindow)->startAsynchronousAnimation(_animID, true);
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int ClickPlayLoopingVideoClip::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

OneShotEntryVideoWarning::OneShotEntryVideoWarning(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animID, byte &flag, int warningMessageID) :
	SceneBase(vm, viewWindow, sceneStaticData, priorLocation), _flag(flag) {
	_animID = animID;
	_warningMessageID = warningMessageID;
}

int OneShotEntryVideoWarning::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_flag == 0) {
		if (_warningMessageID >= 0)
			((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(_warningMessageID));

		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animID);
		_flag = 1;
	}

	return SC_TRUE;
}

DisableForwardMovement::DisableForwardMovement(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int flagValue) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	if (globalFlags.generalWalkthroughMode == flagValue)
		_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
}

CycleEntryVideoWarning::CycleEntryVideoWarning(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animIDA, int animIDB, int warningMessageID) : SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_animIDA = animIDA;
	_animIDB = animIDB;
	_warningMessageID = warningMessageID;
}

int CycleEntryVideoWarning::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (_warningMessageID >= 0)
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(_warningMessageID));

	if (globalFlags.cgBaileyTwoWayGuards == 0) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animIDA);
		globalFlags.cgBaileyTwoWayGuards = 1;
	} else {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animIDB);
		globalFlags.cgBaileyTwoWayGuards = 0;
	}

	return SC_TRUE;
}

ClickPlayVideoSwitch::ClickPlayVideoSwitch(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animID, int cursorID, byte &flag, int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation), _flag(flag) {
	_cursorID = cursorID;
	_animID = animID;
	_clickRegion = Common::Rect(left, top, right, bottom);
}

int ClickPlayVideoSwitch::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animID);

		_flag = 1;

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();

		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickPlayVideoSwitch::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
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

VideoDeath::VideoDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation, int deathID, int messageTextID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation), _deathID(deathID), _messageTextID(messageTextID) {
}

int VideoDeath::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_messageTextID >= -1)
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(_messageTextID));

	return SC_TRUE;
}

int VideoDeath::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (newLocation.timeZone == _staticData.location.timeZone &&
			newLocation.environment == _staticData.location.environment &&
			newLocation.node == _staticData.location.node &&
			newLocation.facing == _staticData.location.facing &&
			newLocation.orientation == _staticData.location.orientation &&
			newLocation.depth == _staticData.location.depth) {
		// Notify the player of his gruesome death
		((SceneViewWindow *)viewWindow)->showDeathScene(_deathID);
		return SC_DEATH;
	}

	return SC_TRUE;
}

ClickChangeDepth::ClickChangeDepth(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int newDepth, int cursorID, int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_newDepth = newDepth;
	_cursorID = cursorID;
	_clickableRegion = Common::Rect(left, top, right, bottom);
}

int ClickChangeDepth::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableRegion.contains(pointLocation)) {
		DestinationScene clickDestination;
		clickDestination.destinationScene = _staticData.location;
		clickDestination.destinationScene.depth = _newDepth;
		clickDestination.transitionType = TRANSITION_FADE;
		clickDestination.transitionData = -1;
		clickDestination.transitionStartFrame = -1;
		clickDestination.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(clickDestination);
	}

	return SC_TRUE;
}

int ClickChangeDepth::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

OpenFirstItemAcquire::OpenFirstItemAcquire(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int openLeft, int openTop, int openRight, int openBottom, int getLeft, int getTop, int getRight,
		int getBottom, int animOpenWith, int animOpenWithout, int itemID, int fullStillFrame, int clearStillFrame):
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_open = false;
	_itemPresent = ((SceneViewWindow *)viewWindow)->getGlobalFlags().cgGoldCoinsPresent == 0;
	_openClickRegion = Common::Rect(openLeft, openTop, openRight, openBottom);
	_acquireRegion = Common::Rect(getLeft, getTop, getRight, getBottom);
	_fullFrameIndex = fullStillFrame;
	_clearFrameIndex = clearStillFrame;
	_itemID = itemID;
	_animOpenWith = animOpenWith;
	_animOpenWithout = animOpenWithout;
}

int OpenFirstItemAcquire::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_acquireRegion.contains(pointLocation) && _itemPresent && _open) {
		_itemPresent = false;
		_staticData.navFrameIndex = _clearFrameIndex;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgGoldCoinsPresent = 1;

		// Call inventory drag start function
		Common::Point ptInventoryWindow = viewWindow->convertPointToGlobal(pointLocation);
		ptInventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->convertPointToLocal(ptInventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(_itemID, ptInventoryWindow);

		return SC_TRUE;
	}

	return SC_FALSE;
}

int OpenFirstItemAcquire::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_openClickRegion.contains(pointLocation) && !_open) {
		_open = true;

		if (_itemPresent) {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animOpenWith);
			_staticData.navFrameIndex = _fullFrameIndex;
		} else {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animOpenWithout);
			_staticData.navFrameIndex = _clearFrameIndex;
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

int OpenFirstItemAcquire::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (_itemID == itemID && !_itemPresent && _open && pointLocation.x != -1 && pointLocation.y != -1) {
		_itemPresent = true;
		_staticData.navFrameIndex = _fullFrameIndex;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgGoldCoinsPresent = 0;

		viewWindow->invalidateWindow(false);
		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int OpenFirstItemAcquire::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_openClickRegion.contains(pointLocation) && !_open)
		return kCursorFinger;

	if (_acquireRegion.contains(pointLocation) && _itemPresent && _open)
		return kCursorOpenHand;

	return kCursorArrow;
}

BrowseBook::BrowseBook(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int bookResID, int textStartResID, int startingPageID, int timeZone, int environment,
		int node, int facing, int orientation, int depth, int transitionType, int transitionData,
		int transitionStartFrame, int transitionLength) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_putDownDestination.destinationScene.timeZone = timeZone;
	_putDownDestination.destinationScene.environment = environment;
	_putDownDestination.destinationScene.node = node;
	_putDownDestination.destinationScene.facing = facing;
	_putDownDestination.destinationScene.orientation = orientation;
	_putDownDestination.destinationScene.depth = depth;
	_putDownDestination.transitionType = transitionType;
	_putDownDestination.transitionData = transitionData;
	_putDownDestination.transitionStartFrame = transitionStartFrame;
	_putDownDestination.transitionLength = transitionLength;

	Common::SeekableReadStream *pageData = _vm->getBookData(bookResID);
	if (!pageData)
		error("Failed to find book resource %d", bookResID);

	uint16 pageCount = pageData->readUint16LE();

	for (uint16 i = 0; i < pageCount; i++) {
		BookPage page;
		page.pageID = pageData->readSint16LE();
		page.pageFrameIndex = pageData->readSint32LE();
		page.numLines = pageData->readSint16LE();
		page.up.typeOfTrans = pageData->readSint16LE();
		page.up.destPage = pageData->readSint16LE();
		page.left.typeOfTrans = pageData->readSint16LE();
		page.left.destPage = pageData->readSint16LE();
		page.right.typeOfTrans = pageData->readSint16LE();
		page.right.destPage = pageData->readSint16LE();
		page.down.typeOfTrans = pageData->readSint16LE();
		page.down.destPage = pageData->readSint16LE();
		_bookDatabase.push_back(page);
	}

	delete pageData;

	_curPage = _bookDatabase[startingPageID].pageID;
	_staticData.navFrameIndex = _bookDatabase[startingPageID].pageFrameIndex;
	_curLineIndex = -1;
	_translatedTextResourceID = textStartResID;

	_top = Common::Rect(150, 0, 282, 70);
	_bottom = Common::Rect(150, 119, 282, 189);
	_left = Common::Rect(0, 0, 150, 189);
	_right = Common::Rect(282, 0, 432, 189);
	_putDown = Common::Rect(150, 70, 282, 119);

	// Mark that we read the journals in the King's Study
	if (_staticData.location.timeZone == 1 && _staticData.location.environment == 8)
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgKSReadJournal = 1;
}

int BrowseBook::gdiPaint(Window *viewWindow) {
	if (_curLineIndex >= 0 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		int lineCount = _bookDatabase[_curPage].numLines;
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(1, (187 / lineCount) * _curLineIndex, 430, (187 / lineCount) * (_curLineIndex + 1) - 1);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int BrowseBook::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	const BookPage &pageData = _bookDatabase[_curPage];

	if (_top.contains(pointLocation) && pageData.up.destPage >= 0) {
		// Change the still
		_curPage = pageData.up.destPage;
		_staticData.navFrameIndex = _bookDatabase[_curPage].pageFrameIndex;

		// Perform the transition
		Graphics::Surface *newBackground = ((SceneViewWindow *)viewWindow)->getStillFrameCopy(_staticData.navFrameIndex);
		((SceneViewWindow *)viewWindow)->pushNewTransition(newBackground, 0, _vm->_gfx->computeVPushOffset(_vm->getTransitionSpeed()), 0);
		newBackground->free();
		delete newBackground;
		_curLineIndex = -1;
		viewWindow->invalidateWindow(false);
		pageChanged(viewWindow);
		return SC_TRUE;
	} else if (_bottom.contains(pointLocation) && pageData.down.destPage >= 0) {
		// Change the still
		_curPage = pageData.down.destPage;
		_staticData.navFrameIndex = _bookDatabase[_curPage].pageFrameIndex;

		// Perform the transition
		Graphics::Surface *newBackground = ((SceneViewWindow *)viewWindow)->getStillFrameCopy(_staticData.navFrameIndex);
		((SceneViewWindow *)viewWindow)->pushNewTransition(newBackground, 3, _vm->_gfx->computeVPushOffset(_vm->getTransitionSpeed()), 0);
		newBackground->free();
		delete newBackground;
		_curLineIndex = -1;
		viewWindow->invalidateWindow(false);
		pageChanged(viewWindow);
		return SC_TRUE;
	} else if (_left.contains(pointLocation) && pageData.left.destPage >= 0) {
		// Change the still
		_curPage = pageData.left.destPage;
		_staticData.navFrameIndex = _bookDatabase[_curPage].pageFrameIndex;

		// Perform the transition
		Graphics::Surface *newBackground = ((SceneViewWindow *)viewWindow)->getStillFrameCopy(_staticData.navFrameIndex);
		((SceneViewWindow *)viewWindow)->pushNewTransition(newBackground, 1, _vm->_gfx->computeHPushOffset(_vm->getTransitionSpeed()), 0);
		newBackground->free();
		delete newBackground;
		_curLineIndex = -1;
		viewWindow->invalidateWindow(false);
		pageChanged(viewWindow);
		return SC_TRUE;
	} else if (_right.contains(pointLocation) && pageData.right.destPage >= 0) {
		// Change the still
		_curPage = pageData.right.destPage;
		_staticData.navFrameIndex = _bookDatabase[_curPage].pageFrameIndex;

		// Perform the transition
		Graphics::Surface *newBackground = ((SceneViewWindow *)viewWindow)->getStillFrameCopy(_staticData.navFrameIndex);
		((SceneViewWindow *)viewWindow)->pushNewTransition(newBackground, 1, _vm->_gfx->computeHPushOffset(_vm->getTransitionSpeed()), 0);
		newBackground->free();
		delete newBackground;
		_curLineIndex = -1;
		viewWindow->invalidateWindow(false);
		pageChanged(viewWindow);
		return SC_TRUE;
	} else if (_putDown.contains(pointLocation) && _putDownDestination.destinationScene.timeZone >= 0) {
		// Move to the new destination
		((SceneViewWindow *)viewWindow)->moveToDestination(_putDownDestination);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int BrowseBook::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	const BookPage &pageData = _bookDatabase[_curPage];

	if (_top.contains(pointLocation) && pageData.up.destPage >= 0)
		return kCursorMoveUp;
	else if (_bottom.contains(pointLocation) && pageData.down.destPage >= 0)
		return kCursorMoveDown;
	else if (_left.contains(pointLocation) && pageData.left.destPage >= 0)
		return kCursorPrevPage;
	else if (_right.contains(pointLocation) && pageData.right.destPage >= 0)
		return kCursorNextPage;
	else if (_putDown.contains(pointLocation) && _putDownDestination.destinationScene.timeZone >= 0)
		return kCursorPutDown;

	return kCursorArrow;
}

int BrowseBook::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	if (_translatedTextResourceID >= 0) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
			int lineCount = _bookDatabase[_curPage].numLines;

			int textLineNumber = 0;
			for (int i = 0; i < _curPage; i++)
				textLineNumber += _bookDatabase[i].numLines;

			// Determine the line index of the cursor
			int lineIndex = (pointLocation.y - 2) / (187 / lineCount);
			if (lineIndex > lineCount - 1)
				lineIndex = lineCount - 1;

			if (_curLineIndex != lineIndex) {
				_curLineIndex = lineIndex;
				viewWindow->invalidateWindow(false);

				Common::String translatedText = _vm->getString(_translatedTextResourceID + textLineNumber + _curLineIndex);
				((SceneViewWindow *)viewWindow)->displayTranslationText(translatedText);
				textTranslated(viewWindow);
			}

			return SC_TRUE;
		}

		if (_curLineIndex != -1) {
			_curLineIndex = -1;
			viewWindow->invalidateWindow(false);
		}
	}

	return SC_FALSE;
}

int BrowseBook::pageChanged(Window *viewWindow) {
	if (_translatedTextResourceID == IDBD_DIARY2_TRANS_TEXT_BASE && _curPage >= 7 && _curPage <= 10)
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgKSSmithyEntryRead = 1;

	return SC_TRUE;
}

int BrowseBook::textTranslated(Window *viewWindow) {
	if (_translatedTextResourceID == IDBD_DIARY2_TRANS_TEXT_BASE && _curPage >= 7 && _curPage <= 10)
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgKSSmithyEntryTranslated = 1;

	return SC_TRUE;
}

ClickPlaySoundSynchronous::ClickPlaySoundSynchronous(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int soundID, int cursorID, int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_cursorID = cursorID;
	_soundID = soundID;
	_clickRegion = Common::Rect(left, top, right, bottom);
}

int ClickPlaySoundSynchronous::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (_clickRegion.contains(pointLocation)) {
		_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _soundID), 127);

		globalFlags.cgTSTriedDoorA = 1;

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickPlaySoundSynchronous::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

TrialRecallScene::TrialRecallScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	// Disable all movement
	_staticData.destUp.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	_staticData.destLeft.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	_staticData.destRight.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	_staticData.destDown.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
}

int TrialRecallScene::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	// Display the message
	static const char *const message =
		"This timezone is not available in this Trial Version.  "
		"Call (800) 943-3664 to purchase the complete version of Buried in Time.\n"
		"Initiating Auto-Recall to Future Apartment...";
	((SceneViewWindow *)viewWindow)->displayLiveText(message, false);

	// Wait about 10 seconds
	Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);
	uint32 start = g_system->getMillis();
	while (g_system->getMillis() - start < 10000)
		_vm->yield(nullptr, -1);
	_vm->_gfx->setCursor(oldCursor);

	// Force a recall
	((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipJump);
	((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sendMessage(new LButtonUpMessage(Common::Point(50, 150), 0));

	return SC_TRUE;
}

} // End of namespace Buried
