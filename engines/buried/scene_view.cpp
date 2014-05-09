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

#include "buried/avi_frames.h"
#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/frame_window.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/inventory_window.h"
#include "buried/livetext.h"
#include "buried/navarrow.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/video_window.h"
#include "buried/environ/scene_base.h"

#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"

namespace Buried {

SceneViewWindow::SceneViewWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_currentScene = 0;
	_preBuffer = 0;
	_walkMovie = 0;
	_useScenePaint = true;
	_timer = 0;
	_currentSprite.image = 0;
	_useSprite = true;
	_infoWindowDisplayed = false;
	_bioChipWindowDisplayed = false;
	_burnedLetterDisplayed = false;
	_asyncMovie = 0;
	_asyncMovieStartFrame = 0;
	_loopAsyncMovie = false;
	_paused = false;
	_useWaitCursor = false;
	_cycleEnabled = ((FrameWindow *)(_parent->getParent()))->isFrameCyclingDefault();
	_disableArthur = false;
	_demoSoundEffectHandle = -1;

	_preBuffer = new Graphics::Surface();
	_preBuffer->create(DIB_FRAME_WIDTH, DIB_FRAME_HEIGHT, g_system->getScreenFormat());

	_rect = Common::Rect(64, 128, 496, 317);

	_timer = setTimer(100);
	_demoSoundTimer = _vm->isDemo() ? setTimer(10) : 0;
	_curCursor = kCursorArrow;
	_stillFrames = new AVIFrames();
	_cycleFrames = new AVIFrames();

	memset(&_globalFlags, 0, sizeof(_globalFlags));
}

SceneViewWindow::~SceneViewWindow() {
	if (_currentScene) {
		_currentScene->preDestructor();
		delete _currentScene;
	}

	killTimer(_timer);

	if (_preBuffer) {
		_preBuffer->free();
		delete _preBuffer;
	}

	delete _stillFrames;
	delete _cycleFrames;
	delete _walkMovie;
	delete _asyncMovie;
}

bool SceneViewWindow::startNewGame(bool walkthrough) {
	Location newLocation;

	if (_vm->isDemo()) {
		newLocation.timeZone = 1;
		newLocation.environment = 4;
		newLocation.node = 0;
		newLocation.facing = 0;
		newLocation.orientation = 1;
		newLocation.depth = 0;
	} else {
		newLocation.timeZone = 4;
		newLocation.environment = 3;
		newLocation.node = 3;
		newLocation.facing = 0;
		newLocation.orientation = 1;
		newLocation.depth = 0;
	}

	jumpToScene(newLocation);

	if (_vm->isDemo()) {
		displayLiveText("To return to the main menu, click the 'Menu' button on the Interface Biochip Display to the right, then click Quit.");
		startDemoAmbientSound();

		// This is unlabeled in the original source, but it looks like a hidden feature
		// to access a bonus puzzle in the demo. (Complete with a typo, but who's counting?)
		if (((FrameWindow *)(_parent->getParent()))->_reviewerMode)
			((GameUIWindow *)_parent)->_inventoryWindow->addItem(kItemCopperMedallion);
	} else if (walkthrough) {
		// Set the mode flag
		_globalFlags.generalWalkthroughMode = 1;

		// Set specific state flags for walkthrough mode
		_globalFlags.cgSmithyStatus = 6;
		_globalFlags.cgTapestryFlag = 1;
		_globalFlags.myTPCodeWheelLeftIndex = 8;
		_globalFlags.myTPCodeWheelRightIndex = 12;
		_globalFlags.myTPCodeWheelStatus = 1;
		_globalFlags.myWGPlacedRope = 1;

		// Add the translate biochip
		((GameUIWindow *)_parent)->_inventoryWindow->addItem(kItemBioChipTranslate);
	}

	invalidateWindow(false);
	return true;
}

bool SceneViewWindow::startNewGameIntro(bool walkthrough) {
	Location newLocation;
	newLocation.timeZone = 10;
	newLocation.environment = 0;
	newLocation.node = 0;
	newLocation.facing = 0;
	newLocation.orientation = 0;
	newLocation.depth = 0;

	jumpToScene(newLocation);
	
	if (walkthrough) {
		// Set the mode flag
		_globalFlags.generalWalkthroughMode = 1;

		// Set specific state flags for walkthrough mode
		_globalFlags.cgSmithyStatus = 6;
		_globalFlags.cgTapestryFlag = 1;
		_globalFlags.myTPCodeWheelLeftIndex = 8;
		_globalFlags.myTPCodeWheelRightIndex = 12;
		_globalFlags.myTPCodeWheelStatus = 1;
		_globalFlags.myWGPlacedRope = 1;

		// Add the translate biochip
		((GameUIWindow *)_parent)->_inventoryWindow->addItem(kItemBioChipTranslate);
	}

	invalidateWindow(false);
	return true;
}

bool SceneViewWindow::startNewGame(const Location &startingLocation) {
	jumpToSceneRestore(startingLocation);

	if (_globalFlags.generalWalkthroughMode == 1) {
		if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0))
			displayLiveText(_vm->getString(IDS_PLAY_MODE_WALKTHROUGH_TEXT));
		else
			displayLiveText("You are playing in Walkthrough mode.");
	} else {
		if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0))
			displayLiveText(_vm->getString(IDS_PLAY_MODE_NORMAL_TEXT));
		else
			displayLiveText("You are playing in Adventure mode.");
	}

	return true;
}

bool SceneViewWindow::showDeathScene(int deathSceneIndex) {
	return ((FrameWindow *)(_parent->getParent()))->showDeathScene(deathSceneIndex, _globalFlags, ((GameUIWindow *)_parent)->_inventoryWindow->getItemArray());
}

bool SceneViewWindow::showCompletionScene() {
	return ((FrameWindow *)(_parent->getParent()))->showCompletionScene(_globalFlags);
}

bool SceneViewWindow::getSceneStaticData(const Location &location, LocationStaticData &sceneStaticData) {
	int curTimeZone = -1;
	int curEnvironment = -1;

	if (!_currentNavigationDatabase.empty()) {
		curTimeZone = _currentNavigationDatabase[0].location.timeZone;
		curEnvironment = _currentNavigationDatabase[0].location.environment;
	}

	if (location.timeZone != curTimeZone || location.environment != curEnvironment) {
		_currentNavigationDatabase.clear();

		int resID = _vm->computeNavDBResourceID(location.timeZone, location.environment);
		Common::SeekableReadStream *resource = _vm->getNavData(resID);
		resource->skip(2);

		while (resource->pos() < resource->size()) {
			LocationStaticData locationStaticData;

			locationStaticData.location.timeZone = resource->readSint16LE();
			locationStaticData.location.environment = resource->readSint16LE();
			locationStaticData.location.node = resource->readSint16LE();
			locationStaticData.location.facing = resource->readSint16LE();
			locationStaticData.location.orientation = resource->readSint16LE();
			locationStaticData.location.depth = resource->readSint16LE();

			locationStaticData.destUp.destinationScene.timeZone = resource->readSint16LE();
			locationStaticData.destUp.destinationScene.environment = resource->readSint16LE();
			locationStaticData.destUp.destinationScene.node = resource->readSint16LE();
			locationStaticData.destUp.destinationScene.facing = resource->readSint16LE();
			locationStaticData.destUp.destinationScene.orientation = resource->readSint16LE();
			locationStaticData.destUp.destinationScene.depth = resource->readSint16LE();
			locationStaticData.destUp.transitionType = resource->readSint16LE();
			locationStaticData.destUp.transitionData = resource->readSint16LE();
			locationStaticData.destUp.transitionStartFrame = resource->readSint32LE();
			locationStaticData.destUp.transitionLength = resource->readSint32LE();
			
			locationStaticData.destLeft.destinationScene.timeZone = resource->readSint16LE();
			locationStaticData.destLeft.destinationScene.environment = resource->readSint16LE();
			locationStaticData.destLeft.destinationScene.node = resource->readSint16LE();
			locationStaticData.destLeft.destinationScene.facing = resource->readSint16LE();
			locationStaticData.destLeft.destinationScene.orientation = resource->readSint16LE();
			locationStaticData.destLeft.destinationScene.depth = resource->readSint16LE();
			locationStaticData.destLeft.transitionType = resource->readSint16LE();
			locationStaticData.destLeft.transitionData = resource->readSint16LE();
			locationStaticData.destLeft.transitionStartFrame = resource->readSint32LE();
			locationStaticData.destLeft.transitionLength = resource->readSint32LE();
			
			locationStaticData.destRight.destinationScene.timeZone = resource->readSint16LE();
			locationStaticData.destRight.destinationScene.environment = resource->readSint16LE();
			locationStaticData.destRight.destinationScene.node = resource->readSint16LE();
			locationStaticData.destRight.destinationScene.facing = resource->readSint16LE();
			locationStaticData.destRight.destinationScene.orientation = resource->readSint16LE();
			locationStaticData.destRight.destinationScene.depth = resource->readSint16LE();
			locationStaticData.destRight.transitionType = resource->readSint16LE();
			locationStaticData.destRight.transitionData = resource->readSint16LE();
			locationStaticData.destRight.transitionStartFrame = resource->readSint32LE();
			locationStaticData.destRight.transitionLength = resource->readSint32LE();
			
			locationStaticData.destDown.destinationScene.timeZone = resource->readSint16LE();
			locationStaticData.destDown.destinationScene.environment = resource->readSint16LE();
			locationStaticData.destDown.destinationScene.node = resource->readSint16LE();
			locationStaticData.destDown.destinationScene.facing = resource->readSint16LE();
			locationStaticData.destDown.destinationScene.orientation = resource->readSint16LE();
			locationStaticData.destDown.destinationScene.depth = resource->readSint16LE();
			locationStaticData.destDown.transitionType = resource->readSint16LE();
			locationStaticData.destDown.transitionData = resource->readSint16LE();
			locationStaticData.destDown.transitionStartFrame = resource->readSint32LE();
			locationStaticData.destDown.transitionLength = resource->readSint32LE();
			
			locationStaticData.destForward.destinationScene.timeZone = resource->readSint16LE();
			locationStaticData.destForward.destinationScene.environment = resource->readSint16LE();
			locationStaticData.destForward.destinationScene.node = resource->readSint16LE();
			locationStaticData.destForward.destinationScene.facing = resource->readSint16LE();
			locationStaticData.destForward.destinationScene.orientation = resource->readSint16LE();
			locationStaticData.destForward.destinationScene.depth = resource->readSint16LE();
			locationStaticData.destForward.transitionType = resource->readSint16LE();
			locationStaticData.destForward.transitionData = resource->readSint16LE();
			locationStaticData.destForward.transitionStartFrame = resource->readSint32LE();
			locationStaticData.destForward.transitionLength = resource->readSint32LE();

			locationStaticData.classID = resource->readSint16LE();
			locationStaticData.navFrameIndex = resource->readSint32LE();
			locationStaticData.miscFrameIndex = resource->readSint32LE();
			locationStaticData.miscFrameCount = resource->readSint32LE();
			locationStaticData.cycleStartFrame = resource->readSint32LE();
			locationStaticData.cycleFrameCount = resource->readSint32LE();

			_currentNavigationDatabase.push_back(locationStaticData);
		}

		if (_currentNavigationDatabase.empty())
			return false;
	}

	for (uint32 i = 0; i < _currentNavigationDatabase.size(); i++) {
		if (location.timeZone == _currentNavigationDatabase[i].location.timeZone &&
				location.environment == _currentNavigationDatabase[i].location.environment &&
				location.node == _currentNavigationDatabase[i].location.node &&
				location.facing == _currentNavigationDatabase[i].location.facing &&
				location.orientation == _currentNavigationDatabase[i].location.orientation &&
				location.depth == _currentNavigationDatabase[i].location.depth) {
			sceneStaticData = _currentNavigationDatabase[i];
			return true;
		}
	}

	return false;
}

bool SceneViewWindow::jumpToScene(const Location &newLocation) {
	Location oldLocation;
	oldLocation.timeZone = -1;
	oldLocation.environment = -1;
	oldLocation.node = -1;
	oldLocation.facing = -1;
	oldLocation.orientation = -1;
	oldLocation.depth = -1;

	Location passedLocation;
	passedLocation.timeZone = -1;
	passedLocation.environment = -1;
	passedLocation.node = -1;
	passedLocation.facing = -1;
	passedLocation.orientation = -1;
	passedLocation.depth = -1;

	// Destroy any window displayed
	if (_infoWindowDisplayed)
		((GameUIWindow *)_parent)->_inventoryWindow->destroyInfoWindow();
	if (_bioChipWindowDisplayed)
		((GameUIWindow *)_parent)->_bioChipRightWindow->destroyBioChipViewWindow();
	if (_burnedLetterDisplayed)
		((GameUIWindow *)_parent)->_inventoryWindow->destroyBurnedLetterWindow();

	LocationStaticData  newSceneStaticData;
	if (!getSceneStaticData(newLocation, newSceneStaticData))
		return false;

	if (_currentScene)
		oldLocation = _currentScene->_staticData.location;

	// Clear the live text window
	if (newLocation.timeZone != oldLocation.timeZone || newLocation.environment != oldLocation.environment)
		((GameUIWindow *)_parent)->_liveTextWindow->updateLiveText();

	// Call the pre-transition function
	if (_currentScene)
		_currentScene->preExitRoom(this, passedLocation);

	if (newLocation.timeZone != oldLocation.timeZone && newLocation.timeZone != -2)
		initializeTimeZoneAndEnvironment(this, newLocation.timeZone, -1);
	if (newLocation.environment != oldLocation.environment && newLocation.environment != -2)
		initializeTimeZoneAndEnvironment(this, newLocation.timeZone, newLocation.environment);

	SceneBase *newScene = constructSceneObject(this, newSceneStaticData, passedLocation);

	if (_currentScene && _currentScene->postExitRoom(this, passedLocation) == SC_DEATH)
		return false;

	if (!newScene)
		error("Failed to create scene");

	if (_currentScene) {
		_currentScene->preDestructor();
		delete _currentScene;
		_currentScene = 0;
	}

	if (newLocation.timeZone != oldLocation.timeZone || newLocation.environment != oldLocation.environment || oldLocation.timeZone < 0)
		startEnvironmentAmbient(passedLocation.timeZone, passedLocation.environment, newLocation.timeZone, newLocation.environment);

	_currentScene = newScene;

	if (_cycleEnabled && newSceneStaticData.cycleStartFrame == -1)
		flushCycleFrameCache();

	invalidateWindow(false);

	if (_currentScene->preEnterRoom(this, passedLocation) == SC_END_PROCESSING)
		return true;

	if (_globalFlags.bcCloakingEnabled != 1)
		((GameUIWindow *)_parent)->_navArrowWindow->updateAllArrows(newScene->_staticData);

	if (newLocation.timeZone != oldLocation.timeZone)
		((GameUIWindow *)_parent)->changeCurrentDate(newLocation.timeZone);

	invalidateWindow(false);

	_currentScene->postEnterRoom(this, passedLocation);

	_parent->invalidateWindow(false);

	if (((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipAI))
		playAIComment(newSceneStaticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

	((GameUIWindow *)_parent)->_bioChipRightWindow->sceneChanged();

	return true;
}

bool SceneViewWindow::jumpToSceneRestore(const Location &newLocation) {
	Location oldLocation(-2, -2, -2, -2, -2, -2);
	Location passedLocation(-2, -2, -2, -2, -2, -2);

	if (_infoWindowDisplayed)
		((GameUIWindow *)getParent())->_inventoryWindow->destroyInfoWindow();
	if (_bioChipWindowDisplayed)
		((GameUIWindow *)getParent())->_bioChipRightWindow->destroyBioChipViewWindow();
	if (_burnedLetterDisplayed)
		((GameUIWindow *)getParent())->_inventoryWindow->destroyBurnedLetterWindow();

	// Get the static scene data for this new location
	LocationStaticData newSceneStaticData;
	if (!getSceneStaticData(newLocation, newSceneStaticData))
		return false;
	if (_currentScene)
		oldLocation = _currentScene->_staticData.location;

	// Clear the live text window
	if (newLocation.timeZone != oldLocation.timeZone || newLocation.environment != oldLocation.environment)
		((GameUIWindow *)getParent())->_liveTextWindow->updateLiveText("");

	// If we have a scene, call the pre-transition function
	if (_currentScene)
		_currentScene->preExitRoom(this, passedLocation);

	// The original resets the environment upon loading, which is clearly not correct.
	// We won't.

	//if (newLocation.timeZone != oldLocation.timeZone && newLocation.timeZone != -2)
	//	initializeTimeZoneAndEnvironment(this, newLocation.timeZone, -1);
	//if (newLocation.timeZone != oldLocation.timeZone && newLocation.environment != -2)
	//	initializeTimeZoneAndEnvironment(this, newLocation.timeZone, newLocation.environment);

	// Create the new scene object
	SceneBase *newScene = constructSceneObject(this, newSceneStaticData, passedLocation);

	// Call the post-transition function
	if (_currentScene && _currentScene->postExitRoom(this, passedLocation) == SC_DEATH)
		return false;

	if (_currentScene) {
		_currentScene->preDestructor();
		delete _currentScene;
		_currentScene = 0;
	}

	// Change the ambient music
	if (newLocation.timeZone != oldLocation.timeZone || newLocation.environment != oldLocation.environment || oldLocation.timeZone < 0)
		startEnvironmentAmbient(passedLocation.timeZone, passedLocation.environment, newLocation.timeZone, newLocation.environment);

	_currentScene = newScene;

	if (_cycleEnabled && newSceneStaticData.cycleStartFrame == -1)
		flushCycleFrameCache();

	if (_currentScene->preEnterRoom(this, passedLocation) == SC_END_PROCESSING)
		return true;

	if (_globalFlags.bcCloakingEnabled != 1)
		((GameUIWindow *)getParent())->_navArrowWindow->updateAllArrows(newScene->_staticData);

	if (newLocation.timeZone != oldLocation.timeZone)
		((GameUIWindow *)getParent())->changeCurrentDate(newLocation.timeZone);

	invalidateWindow(false);

	_currentScene->postEnterRoom(this, passedLocation);
	getParent()->invalidateWindow(false);

	// Check AI database for a spontaneous comment
	if (((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
		playAIComment(newSceneStaticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

	// Notify biochip right window of change
	((GameUIWindow *)getParent())->_bioChipRightWindow->sceneChanged();

	return true;
}

bool SceneViewWindow::moveInDirection(int direction) {
	if (!_currentScene)
		return false;

	((GameUIWindow *)_parent)->_navArrowWindow->updateArrow(direction, NavArrowWindow::BUTTON_SELECTED);

	DestinationScene destinationData;

	switch (direction) {
	case 0: // Up
		destinationData = _currentScene->_staticData.destUp;
		break;
	case 1: // Left
		destinationData = _currentScene->_staticData.destLeft;
		break;
	case 2: // Right
		destinationData = _currentScene->_staticData.destRight;
		break;
	case 3: // Down
		destinationData = _currentScene->_staticData.destDown;
		break;
	case 4: // Forward
		destinationData = _currentScene->_staticData.destForward;
		break;
	}

	return moveToDestination(destinationData);
}

bool SceneViewWindow::moveToDestination(const DestinationScene &destinationData) {
	// Close any information window
	if (_infoWindowDisplayed)
		((GameUIWindow *)_parent)->_inventoryWindow->destroyInfoWindow();
	if (_bioChipWindowDisplayed)
		((GameUIWindow *)_parent)->_bioChipRightWindow->destroyBioChipViewWindow();
	if (_burnedLetterDisplayed)
		((GameUIWindow *)_parent)->_inventoryWindow->destroyBurnedLetterWindow();

	// Check to see if this is a valid move
	if (destinationData.destinationScene.timeZone == -1) {
		((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(true);
		return true;
	}

	assert(_currentScene); // clone2727: sanity check -- the original code was broken

	LocationStaticData newSceneStaticData;
	if (!getSceneStaticData(destinationData.destinationScene, newSceneStaticData)) {
		((GameUIWindow *)_parent)->_navArrowWindow->updateAllArrows(_currentScene->_staticData);
		((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(true);
		return true;
	}

	Location oldLocation = _currentScene->_staticData.location;

	// Disable locate and clear the live text window
	if (newSceneStaticData.location.timeZone != oldLocation.timeZone ||
			newSceneStaticData.location.environment != oldLocation.environment ||
			newSceneStaticData.location.node != oldLocation.node) {
		((GameUIWindow *)_parent)->_bioChipRightWindow->disableEvidenceCapture();
		((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(false);
	}

	// Disable the arrow window
	((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(false);

	// Get thr esults from the pre-exit room function
	int retVal = _currentScene->preExitRoom(this, destinationData.destinationScene);

	// If we died, return here
	if (retVal == SC_DEATH)
		return false;

	// If we did not return success, the move is disallowed
	if (retVal != SC_TRUE) {
		((GameUIWindow *)_parent)->_navArrowWindow->updateAllArrows(_currentScene->_staticData);
		((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(true);
		return true;
	}

	// Initialize the time zone and environment
	if (newSceneStaticData.location.timeZone != oldLocation.timeZone && newSceneStaticData.location.timeZone != -2)
		initializeTimeZoneAndEnvironment(this, newSceneStaticData.location.timeZone, -1);
	if (newSceneStaticData.location.environment != oldLocation.environment && newSceneStaticData.location.environment != -2)
		initializeTimeZoneAndEnvironment(this, newSceneStaticData.location.timeZone, newSceneStaticData.location.environment);

	// If we are movinto a different node or time zone, reset the evidence flag
	if (newSceneStaticData.location.timeZone != oldLocation.timeZone ||
			newSceneStaticData.location.environment != oldLocation.environment ||
			newSceneStaticData.location.node != oldLocation.node) {
		if (_globalFlags.bcLocateEnabled == 1) {
			_globalFlags.bcLocateEnabled = 0;
			((GameUIWindow *)_parent)->_bioChipRightWindow->invalidateWindow(false);
		}
	}

	// Create the new scene object
	SceneBase *newScene = constructSceneObject(this, newSceneStaticData, oldLocation);

	if (!newScene)
		error("Failed to create new scene");

	// Switch on the type of transition
	if (destinationData.transitionType == TRANSITION_VIDEO) {
		// Play transition
		playTransition(destinationData, newScene->_staticData.navFrameIndex);

		// Change the ambient sound
		if (newSceneStaticData.location.timeZone != oldLocation.timeZone || newSceneStaticData.location.environment != oldLocation.environment || oldLocation.timeZone < 0)
			startEnvironmentAmbient(oldLocation.timeZone, oldLocation.environment, newSceneStaticData.location.timeZone, newSceneStaticData.location.environment, false);
	} else {
		// Change the ambient sound
		if (newSceneStaticData.location.timeZone != oldLocation.timeZone || newSceneStaticData.location.environment != oldLocation.environment || oldLocation.timeZone < 0)
			startEnvironmentAmbient(oldLocation.timeZone, oldLocation.environment, newSceneStaticData.location.timeZone, newSceneStaticData.location.environment);

		// Play transition
		playTransition(destinationData, newScene->_staticData.navFrameIndex);
	}

	// Call the post-exit function
	retVal = _currentScene->postExitRoom(this, destinationData.destinationScene);

	if (retVal == SC_DEATH)
		return false;

	if (retVal != SC_TRUE) {
		newScene->preDestructor();
		delete newScene;
		((GameUIWindow *)_parent)->_navArrowWindow->updateAllArrows(_currentScene->_staticData);
		((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(true);
		return true;
	}

	// Delete the current scene
	_currentScene->preDestructor();
	delete _currentScene;
	_currentScene = newScene;

	// If this scene has no cycle frames, flush the cycle frame cache
	if (_cycleEnabled && newSceneStaticData.cycleStartFrame == -1)
		flushCycleFrameCache();

	// Call the pre-enter function, exiting this function if SC_END_PROCESSING is returned
	if (_currentScene->preEnterRoom(this, oldLocation) == SC_END_PROCESSING)
		return true;

	// Send new navigation data to navigation buttons
	if (_globalFlags.bcCloakingEnabled != 1)
		((GameUIWindow *)_parent)->_navArrowWindow->updateAllArrows(newScene->_staticData);

	// If this is a different time zone, then change the date
	if (newSceneStaticData.location.timeZone != oldLocation.timeZone)
		((GameUIWindow *)_parent)->changeCurrentDate(newSceneStaticData.location.timeZone);

	// Call for a repaint
	invalidateWindow(false);

	// Call the post-enter function
	_currentScene->postEnterRoom(this, oldLocation);

	// Invalidate this too, for some reason
	_parent->invalidateWindow(false);

	// Check the AI database for this environment to see if there is a spontaneous comment for this scene
	if (((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipAI) && !_disableArthur)
		playAIComment(newSceneStaticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

	// Notify the right BioChip window of change
	((GameUIWindow *)_parent)->_bioChipRightWindow->sceneChanged();

	// Re-enable navigation arrows
	((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(true);

	// Hardcoded demo ambient
	if (_vm->isDemo() && newSceneStaticData.location.environment != oldLocation.environment)
		startDemoAmbientSound();

	return true;
}

bool SceneViewWindow::timeSuitJump(int destination) {
	// Determine if this is a valid jump
	if (destination < 0 || destination > 4)
		return false;

	// Clear any live text
	((GameUIWindow *)_parent)->_liveTextWindow->updateLiveText();

	// Disable movement controls
	((GameUIWindow *)_parent)->_navArrowWindow->updateAllArrows(0, 0, 0, 0, 0);

	Location newLocation, oldLocation;

	switch (destination) {
	case 0:
		newLocation = Location(2, 1, 6, 1, 1, 0);
		oldLocation = Location(-2, -2, -2, -2, -2, -2);
		break;
	case 1:
		newLocation = Location(1, 1, 3, 3, 1, 0);
		oldLocation = Location(-2, -2, -2, -2, -2, -2);
		break;
	case 2:
		newLocation = Location(5, 1, 2, 4, 1, 0);
		oldLocation = Location(-2, -2, -2, -2, -2, -2);
		break;
	case 3:
		if (_globalFlags.generalWalkthroughMode == 0)
			newLocation = Location(6, 10, 0, 0, 0, 0);
		else
			newLocation = Location(6, 1, 1, 1, 2, 0);
		oldLocation = Location(-2, -2, -2, -2, -2, -2);
		break;
	case 4:
		newLocation = Location(4, 3, 3, 0, 1, 0);
		oldLocation = Location(-2, -2, -2, -2, -2, -2);
		break;
	}

	// Save the old location to use for the scene we are leaving
	Location specOldLocation = oldLocation;

	// Call the pre-transition function
	if (_currentScene)
		_currentScene->preExitRoom(this, specOldLocation);

	// Make sure the right BioChip is displayed
	((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipJump);

	// Play the movie
	VideoWindow *jumpMovie = new VideoWindow(_vm, ((GameUIWindow *)_parent)->_bioChipRightWindow);
	if (!jumpMovie->openVideo(_vm->getFilePath(IDS_BC_JUMP_MOVIE_FILENAME)))
		error("Failed to play small jump movie");

	// Reposition
	jumpMovie->setWindowPos(0, 0, 28, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder | kWindowPosHideWindow);

	// Notify the BioChip of the change
	((GameUIWindow *)_parent)->_bioChipRightWindow->jumpInitiated(false);

	// Show and disable the movie window
	jumpMovie->enableWindow(false);
	jumpMovie->showWindow(kWindowShow);

	// Start fading down the current ambient and start the jump audio file
	_vm->_sound->setAmbientSound();
	_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_BC_JUMP_AUDIO_FILENAME));

	// Play the movie
	jumpMovie->playToFrame(24);

	while (!_vm->shouldQuit() && jumpMovie->getMode() != VideoWindow::kModeStopped && _vm->_sound->isInterfaceSoundPlaying()) {
		_vm->yield();
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit())
		return true;

	// Make sure the interface sound has stopped
	_vm->_sound->stopInterfaceSound();

	delete jumpMovie;
	_vm->_sound->timerCallback();
	jumpMovie = new VideoWindow(_vm, this);

	Common::String fileName;
	switch (destination) {
	case 0:
		fileName = _vm->getFilePath(IDS_MAYAN_JUMP_MOVIE_FILENAME);
		break;
	case 1:
		fileName = _vm->getFilePath(IDS_CASTLE_JUMP_MOVIE_FILENAME);
		break;
	case 2:
		fileName = _vm->getFilePath(IDS_DAVINCI_JUMP_MOVIE_FILENAME);
		break;
	case 3:
		fileName = _vm->getFilePath(IDS_AILAB_JUMP_MOVIE_FILENAME);
		break;
	case 4:
		fileName = _vm->getFilePath(IDS_FUTAPT_JUMP_MOVIE_FILENAME);
		break;
	}

	if (!jumpMovie->openVideo(fileName))
		error("Failed to play movie '%s'", fileName.c_str());

	jumpMovie->setWindowPos(0, 0, 0, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder | kWindowPosHideWindow);

	// Show and disable the window
	jumpMovie->enableWindow(false);
	jumpMovie->showWindow(kWindowShow);

	_vm->_sound->stop();

	// Play the movie
	jumpMovie->playVideo();

	while (!_vm->shouldQuit() && jumpMovie->getMode() != VideoWindow::kModeStopped)
		_vm->yield();

	if (_vm->shouldQuit())
		return true;

	_vm->_sound->restart();
	delete jumpMovie;

	// Initialize the time zone and environment
	initializeTimeZoneAndEnvironment(this, newLocation.timeZone, -1);
	initializeTimeZoneAndEnvironment(this, newLocation.timeZone, newLocation.environment);

	// Get the static scene data for this new location
	LocationStaticData newSceneStaticData;
	if (!getSceneStaticData(newLocation, newSceneStaticData))
		return false;

	// And the old location
	if (_currentScene)
		oldLocation = _currentScene->_staticData.location;

	// Create the new scene object
	SceneBase *newScene = constructSceneObject(this, newSceneStaticData, oldLocation);

	if (_currentScene) {
		// Post-transition function
		if (_currentScene->postExitRoom(this, specOldLocation) == SC_DEATH)
			return false;

		// Delete the old scene
		_currentScene->preDestructor();
		delete _currentScene;
	}

	if (!newScene)
		error("Failed to create scene object for time zone %d", destination);

	// Set the new scene
	_currentScene = newScene;

	// If this scene has no cycle frames, flush the cycle frame cache
	if (_cycleEnabled && newSceneStaticData.cycleStartFrame == -1)
		flushCycleFrameCache();

	// Update navigation buttons, if not cloaked
	if (_globalFlags.bcCloakingEnabled != 1)
		((GameUIWindow *)_parent)->_navArrowWindow->updateAllArrows(_currentScene->_staticData);

	// Change the date if the time zone changed
	if (newLocation.timeZone != oldLocation.timeZone)
		((GameUIWindow *)_parent)->changeCurrentDate(newLocation.timeZone);

	// Call for a repaint
	invalidateWindow(false);
	_vm->_sound->timerCallback();

	// Time to show and play the right-hand small movie to the mid point, with proper sound
	jumpMovie = new VideoWindow(_vm, ((GameUIWindow *)_parent)->_bioChipRightWindow);

	if (!jumpMovie->openVideo(_vm->getFilePath(IDS_BC_JUMP_MOVIE_FILENAME)))
		error("Failed to play small jump movie");

	jumpMovie->setWindowPos(0, 0, 28, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder | kWindowPosHideWindow);

	// Notify the BioChip of the change
	((GameUIWindow *)_parent)->_bioChipRightWindow->jumpEnded(false);

	jumpMovie->enableWindow(false);
	jumpMovie->showWindow(kWindowShow);

	// Start the ambient fading back up, and play the jump sound
	startEnvironmentAmbient(oldLocation.timeZone, oldLocation.environment, newLocation.timeZone, newLocation.environment);
	_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_BC_JUMP_AUDIO_FILENAME));

	// Play the movie
	jumpMovie->seekToFrame(24);
	jumpMovie->playToFrame(48);

	while (!_vm->shouldQuit() && jumpMovie->getMode() != VideoWindow::kModeStopped && _vm->_sound->isInterfaceSoundPlaying()) {
		_vm->yield();
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit())
		return true;

	// Forceably stop the interface sound
	_vm->_sound->stopInterfaceSound();

	// Destroy the movie
	delete jumpMovie;

	// Repaint the BioChip view window
	((GameUIWindow *)_parent)->_bioChipRightWindow->invalidateWindow(false);

	// Repaint the window
	invalidateWindow(false);
	
	// Call the post-enter function
	_currentScene->postEnterRoom(this, oldLocation);
	_parent->invalidateWindow(false);

	if (((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipAI))
		playAIComment(newSceneStaticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

	// Notify the right BioChip window of the change
	((GameUIWindow *)_parent)->_bioChipRightWindow->sceneChanged();

	return true;
}

bool SceneViewWindow::playTransition(const DestinationScene &destinationData, int navFrame) {
	// Call the appropriate function for the transition type
	switch (destinationData.transitionType) {
	case TRANSITION_PUSH:
		if (_vm->isControlDown()) {
			if (navFrame >= 0) {
				LocationStaticData destinationStaticData;
				if (!getSceneStaticData(destinationData.destinationScene, destinationStaticData))
					return false;

				changeStillFrameMovie(_vm->getFilePath(destinationStaticData.location.timeZone, destinationStaticData.location.environment, SF_STILLS));

				Graphics::Surface *newBackground = getStillFrameCopy(navFrame);
				_vm->_gfx->crossBlit(_preBuffer, 0, 0, 432, 189, newBackground, 0, 0);
				newBackground->free();
				delete newBackground;
			}
			return true;
		} else {
			LocationStaticData destinationStaticData;
			if (!getSceneStaticData(destinationData.destinationScene, destinationStaticData))
				return false;

			Graphics::Surface *newBackground = getStillFrameCopy(navFrame);
			Graphics::Surface *curBackground = _preBuffer;

			bool retVal = false;
			if (destinationData.transitionData == 0 || destinationData.transitionData == 3)
				retVal = pushTransition(curBackground, newBackground, destinationData.transitionData, _vm->_gfx->computeVPushOffset(_vm->getTransitionSpeed()), 0);
			else
				retVal = pushTransition(curBackground, newBackground, destinationData.transitionData, _vm->_gfx->computeHPushOffset(_vm->getTransitionSpeed()), 0);

			newBackground->free();
			delete newBackground;
			return retVal;
		}
		break;
	case TRANSITION_WALK:
		if (_vm->isControlDown()) {
			if (navFrame >= 0) {
				LocationStaticData destinationStaticData;
				if (!getSceneStaticData(destinationData.destinationScene, destinationStaticData))
					return false;

				changeStillFrameMovie(_vm->getFilePath(destinationStaticData.location.timeZone, destinationStaticData.location.environment, SF_STILLS));

				Graphics::Surface *newBackground = getStillFrameCopy(navFrame);
				_vm->_gfx->crossBlit(_preBuffer, 0, 0, 432, 189, newBackground, 0, 0);
				newBackground->free();
				delete newBackground;
			}
			return true;
		} else {
			// The demo has a hardcoded door open sound
			// This, and the code below the walkTransition call, are glue around the
			// demo's sound implementation. The demo is based on an alpha which uses
			// waveOut to play sounds, as opposed to the final which uses WAIL.
			if (_vm->isDemo() && destinationData.destinationScene.depth == 1) {
				// Stop the current ambient sound
				// onTimer() will restart it
				_vm->_sound->setAmbientSound();

				if (_currentScene->_staticData.location.environment == 4)
					_demoSoundEffectHandle = _vm->_sound->playSoundEffect("CASTLE/CGMBDO.WAV");
				else
					_demoSoundEffectHandle = _vm->_sound->playSoundEffect("CASTLE/CGBSDO.WAV");
			}
		
			bool retVal = walkTransition(_currentScene->_staticData.location, destinationData, navFrame);

			// And also a door close sound
			if (_vm->isDemo() && destinationData.destinationScene.environment != _currentScene->_staticData.location.environment) {
				// Stop the current ambient sound
				// onTimer() will restart it
				_vm->_sound->setAmbientSound();

				if (_currentScene->_staticData.location.environment == 4)
					_demoSoundEffectHandle = _vm->_sound->playSoundEffect("CASTLE/CGBSDC.WAV");
				else
					_demoSoundEffectHandle = _vm->_sound->playSoundEffect("CASTLE/CGMBDC.WAV");
			}

			return retVal;
		}
		break;
	case TRANSITION_VIDEO:
		if (_vm->isControlDown() && false) { // TODO: debug mode check (maybe?)
			if (navFrame >= 0) {
				LocationStaticData destinationStaticData;
				if (!getSceneStaticData(destinationData.destinationScene, destinationStaticData))
					return false;

				changeStillFrameMovie(_vm->getFilePath(destinationStaticData.location.timeZone, destinationStaticData.location.environment, SF_STILLS));

				Graphics::Surface *newBackground = getStillFrameCopy(navFrame);
				_vm->_gfx->crossBlit(_preBuffer, 0, 0, 432, 189, newBackground, 0, 0);
				newBackground->free();
				delete newBackground;
			}
			return true;
		} else {
			return videoTransition(_currentScene->_staticData.location, destinationData, navFrame);
		}
		break;
	}

	return false;
}

bool SceneViewWindow::videoTransition(const Location &location, DestinationScene destinationData, int navFrame) {
	Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);

	_paused = true;
	bool audioStream = true;

	// If the start frame is less than 0, open up the animation database and retrieve all of the info
	if (destinationData.transitionStartFrame < 0) {
		Common::Array<AnimEvent> animEvents = getAnimationDatabase(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment);

		bool found = false;
		uint i = 0;
		for (; i < animEvents.size(); i++) {
			if (animEvents[i].animationID == destinationData.transitionData) {
				found = true;
				break;
			}
		}

		if (!found) {
			_paused = false;
			return false;
		}

		destinationData.transitionData = animEvents[i].fileNameID;
		destinationData.transitionStartFrame = animEvents[i].startFrame;
		destinationData.transitionLength = animEvents[i].frameCount;

		if (animEvents[i].audioStreamCount < 1)
			audioStream = false;
	}

	LocationStaticData destinationStaticData;
	if (!getSceneStaticData(destinationData.destinationScene, destinationStaticData)) {
		_paused = false;
		return false;
	}
	
	changeStillFrameMovie(_vm->getFilePath(destinationStaticData.location.timeZone, destinationStaticData.location.environment, SF_STILLS));

	Graphics::Surface *newBackground = 0;
	if (destinationStaticData.navFrameIndex >= 0)
		newBackground = getStillFrameCopy(navFrame);

	// Open the movie
	VideoWindow *animationMovie = new VideoWindow(_vm, this);

	Common::String fileName = _vm->getFilePath(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment, destinationData.transitionData);
	if (!animationMovie->openVideo(fileName))
		error("Failed to open video transition movie '%s'", fileName.c_str());

	if (audioStream)
		_vm->_sound->stop();

	animationMovie->seekToFrame(destinationData.transitionStartFrame);
	animationMovie->showWindow(kWindowShow);
	animationMovie->playToFrame(destinationData.transitionStartFrame + destinationData.transitionLength - 1);

	while (!_vm->shouldQuit() && animationMovie->getMode() != VideoWindow::kModeStopped) {
		_vm->yield();
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit())
		return true;

	delete animationMovie;

	if (audioStream)
		_vm->_sound->restart();

	if (newBackground) {
		_vm->_gfx->crossBlit(_preBuffer, 0, 0, 432, 189, newBackground, 0, 0);
		newBackground->free();
		delete newBackground;
	}

	_vm->_gfx->setCursor(oldCursor);
	_paused = false;

	return true;
}

bool SceneViewWindow::walkTransition(const Location &location, const DestinationScene &destinationData, int navFrame) {
	_paused = true;
	Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);
	Graphics::Surface *newBackground = 0;

	if (navFrame >= 0) {
		changeStillFrameMovie(_vm->getFilePath(destinationData.destinationScene.timeZone, destinationData.destinationScene.environment, SF_STILLS));
		newBackground = getStillFrameCopy(navFrame);
	}

	Common::String walkFileName = _vm->getFilePath(location.timeZone, location.environment, SF_NAVIGATION);
	if (_walkMovieFileName != walkFileName) {
		delete _walkMovie;
		_walkMovie = new VideoWindow(_vm, this);
		_walkMovie->setWindowPos(kWindowPosTop, 0, 0, 0, 0, kWindowPosNoActivate | kWindowPosNoZOrder | kWindowPosNoSize);

		if (!_walkMovie->openVideo(walkFileName))
			error("Failed to open walk movie '%s'", walkFileName.c_str());

		_walkMovieFileName = walkFileName;
	}

	_vm->_sound->timerCallback(); // necessary?

	_walkMovie->seekToFrame(destinationData.transitionStartFrame);

	if (navFrame < 0) {
		// FIXME: Is this possible?
		_paused = false;
		return true;
	}

	_walkMovie->showWindow(kWindowShow);
	_walkMovie->invalidateWindow(false);

	// Start the footsteps
	_vm->_sound->startFootsteps(destinationData.transitionData);

	_walkMovie->playToFrame(destinationData.transitionStartFrame + destinationData.transitionLength - 1);
	while (!_vm->shouldQuit() && _walkMovie->getMode() != VideoWindow::kModeStopped) {
		_vm->yield();
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit())
		return true;

	_vm->_sound->stopFootsteps();

	_vm->_gfx->crossBlit(_preBuffer, 0, 0, 432, 189, newBackground, 0, 0);
	newBackground->free();
	delete newBackground;

	_walkMovie->showWindow(kWindowHide);
	_vm->_gfx->setCursor(oldCursor);
	_paused = false;

	return true;
}

bool SceneViewWindow::pushTransition(Graphics::Surface *curBackground, Graphics::Surface *newBackground, int direction, int stripSize, int totalTime) {
	// Check the validity of the parameters
	if (!curBackground || !newBackground || direction < 0 || direction > 4 || stripSize <= 0 || totalTime < 0)
		return false;

	// Change the cursor to an hourglass
	Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);
	_useScenePaint = false;

	switch (direction) {
	case 0: // Push down
		for (int i = 0; i < DIB_FRAME_HEIGHT; i += stripSize) {
			curBackground->move(0, stripSize, curBackground->h);

			for (int j = 0; j < stripSize; j++)
				memcpy(curBackground->getBasePtr(0, j), newBackground->getBasePtr(0, curBackground->h - (i + stripSize) + j), newBackground->w * newBackground->format.bytesPerPixel);

			invalidateWindow(false);
			_vm->yield();
		}
		break;
	case 1: // Push right
		for (int i = 0; i < DIB_FRAME_WIDTH; i += stripSize) {
			curBackground->move(stripSize, 0, curBackground->h);

			for (int j = 0; j < curBackground->h; j++)
				memcpy(curBackground->getBasePtr(0, j), newBackground->getBasePtr(newBackground->w - (i + stripSize), j), stripSize * newBackground->format.bytesPerPixel);

			invalidateWindow(false);
			_vm->yield();
		}
		break;
	case 2: // Push left
		for (int i = 0; i < DIB_FRAME_WIDTH; i += stripSize) {
			curBackground->move(-stripSize, 0, curBackground->h);

			for (int j = 0; j < curBackground->h; j++)
				memcpy(curBackground->getBasePtr(curBackground->w - stripSize, j), newBackground->getBasePtr(i, j), stripSize * newBackground->format.bytesPerPixel);

			invalidateWindow(false);
			_vm->yield();
		}
		break;
	case 3: // Push up
		for (int i = 0; i < DIB_FRAME_HEIGHT; i += stripSize) {
			curBackground->move(0, -stripSize, curBackground->h);

			for (int j = 0; j < stripSize; j++)
				memcpy(curBackground->getBasePtr(0, curBackground->h - stripSize + j), newBackground->getBasePtr(0, i + j), newBackground->w * newBackground->format.bytesPerPixel);

			invalidateWindow(false);
			_vm->yield();
		}
		break;
	}

	_vm->_gfx->setCursor(oldCursor);
	_useScenePaint = true;
	return true;
}

bool SceneViewWindow::pushNewTransition(Graphics::Surface *newBackground, int direction, int stripSize, int totalTime) {
	// Check the validity of the parameters
	if (!newBackground || direction < 0 || direction > 4 || stripSize <= 0 || totalTime < 0)
		return false;

	// Call the push transition function
	if (direction == 0 || direction == 3)
		return pushTransition(_preBuffer, newBackground, direction, _vm->_gfx->computeVPushOffset(_vm->getTransitionSpeed()), totalTime);

	return pushTransition(_preBuffer, newBackground, direction, _vm->_gfx->computeHPushOffset(_vm->getTransitionSpeed()), totalTime);
}

bool SceneViewWindow::slideInTransition(Graphics::Surface *newBackground, int direction, int stripSize, int totalTime) {
	// Check the validity of the parameters
	if (!newBackground || direction < 0 || direction > 4 || stripSize <= 0 || totalTime < 0)
		return false;

	// Change the cursor to an hourglass
	Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);

	switch (direction) {
	case 0: // Push down
		for (int i = stripSize; i <= DIB_FRAME_HEIGHT; i += stripSize) {
			for (int j = 0; j < i; j++)
				memcpy(_preBuffer->getBasePtr(0, j), newBackground->getBasePtr(0, DIB_FRAME_HEIGHT - j), newBackground->w * newBackground->format.bytesPerPixel);

			invalidateWindow(false);
			_vm->yield();
		}
		break;
	case 1: // Push right
		for (int i = stripSize; i <= DIB_FRAME_WIDTH; i += stripSize) {
			for (int j = 0; j < DIB_FRAME_HEIGHT; j++)
				memcpy(_preBuffer->getBasePtr(0, j), newBackground->getBasePtr(DIB_FRAME_WIDTH - i, j), i * newBackground->format.bytesPerPixel);

			invalidateWindow(false);
			_vm->yield();
		}
		break;
	case 2: // Push left
		for (int i = stripSize; i <= DIB_FRAME_WIDTH; i += stripSize) {
			for (int j = 0; j < DIB_FRAME_HEIGHT; j++)
				memcpy(_preBuffer->getBasePtr(0, DIB_FRAME_WIDTH - i), newBackground->getBasePtr(0, j), i * newBackground->format.bytesPerPixel);

			invalidateWindow(false);
			_vm->yield();
		}
		break;
	case 3: // Push up
		for (int i = stripSize; i <= DIB_FRAME_HEIGHT; i += stripSize) {
			for (int j = 0; j < i; j++)
				memcpy(_preBuffer->getBasePtr(0, DIB_FRAME_HEIGHT - j), newBackground->getBasePtr(0, j), newBackground->w * newBackground->format.bytesPerPixel);

			invalidateWindow(false);
			_vm->yield();
		}
		break;
	}

	_vm->_gfx->setCursor(oldCursor);
	return true;
}

bool SceneViewWindow::slideOutTransition(Graphics::Surface *newBackground, int direction, int stripSize, int totalTime) {
	// Check the validity of the parameters
	if (!newBackground || direction < 0 || direction > 4 || stripSize <= 0 || totalTime < 0)
		return false;

	// Change the cursor to an hourglass
	Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);

	Graphics::Surface curBackground;
	curBackground.copyFrom(*_preBuffer);
	_useScenePaint = false;

	switch (direction) {
	case 0: // Push down
		for (int i = stripSize; i <= DIB_FRAME_HEIGHT; i += stripSize) {
			_vm->_gfx->crossBlit(_preBuffer, 0, 0, 432, 189, newBackground, 0, 0);
			_vm->_gfx->crossBlit(_preBuffer, 0, i, 432, 189 - i, &curBackground, 0, 0);
			invalidateWindow(false);
			_vm->yield();
		}
		break;
	case 1: // Push right
		for (int i = DIB_FRAME_WIDTH; i >= 0; i -= stripSize) {
			if (i < DIB_FRAME_WIDTH)
				_vm->_gfx->crossBlit(_preBuffer, i, 0, DIB_FRAME_WIDTH - i, 189, newBackground, i, 0);
			_vm->_gfx->crossBlit(_preBuffer, 0, 0, i, 189, &curBackground, DIB_FRAME_WIDTH - i, 0);
			invalidateWindow(false);
			_vm->yield();
		}
		break;
	case 2: // Push left
		for (int i = stripSize; i <= DIB_FRAME_WIDTH; i += stripSize) {
			_vm->_gfx->crossBlit(_preBuffer, 0, 0, i, 189, newBackground, 0, 0);
			_vm->_gfx->crossBlit(_preBuffer, i, 0, 432 - i, 189, &curBackground, 0, 0);
			invalidateWindow(false);
			_vm->yield();
		}
		break;
	case 3: // Push up
		for (int i = DIB_FRAME_HEIGHT; i >= 0; i -= stripSize) {
			_vm->_gfx->crossBlit(_preBuffer, 0, 0, 432, 189, newBackground, 0, 0);
			_vm->_gfx->crossBlit(_preBuffer, 0, 189 - i, 432, i, &curBackground, 0, 0);
			invalidateWindow(false);
			_vm->yield();
		}
		break;
	}

	curBackground.free();
	_useScenePaint = true;
	_vm->_gfx->setCursor(oldCursor);
	return true;
}

bool SceneViewWindow::changeStillFrameMovie(const Common::String &fileName) {
	return _stillFrames->open(fileName);
}

bool SceneViewWindow::changeCycleFrameMovie(const Common::String &fileName) {
	// Only continue if cycling is enabled
	if (!_cycleEnabled)
		return false;

	if (((FrameWindow *)(_parent->getParent()))->isFrameCachingAllowed())
		return _cycleFrames->open(fileName, 5);

	return _cycleFrames->open(fileName);
}

Graphics::Surface *SceneViewWindow::getStillFrameCopy(int frameIndex) {
	return _stillFrames->getFrameCopy(frameIndex);
}

const Graphics::Surface *SceneViewWindow::getStillFrame(int frameIndex) {
	return _stillFrames->getFrame(frameIndex);
}

Graphics::Surface *SceneViewWindow::getCycleFrameCopy(int frameIndex) {
	if (!_cycleEnabled)
		return 0;

	return _cycleFrames->getFrameCopy(frameIndex);
}

const Graphics::Surface *SceneViewWindow::getCycleFrame(int frameIndex) {
	if (!_cycleEnabled)
		return 0;

	return _cycleFrames->getFrame(frameIndex);
}

bool SceneViewWindow::enableCycleFrameCache(bool enable) {
	if (!_cycleEnabled)
		return false;

	_cycleFrames->enableFrameCache(enable);
	return true;
}

bool SceneViewWindow::flushCycleFrameCache() {
	if (!_cycleEnabled)
		return false;

	return _cycleFrames->flushFrameCache();
}

bool SceneViewWindow::enableCycling(bool enable) {
	_cycleEnabled = enable;

	if (!enable) {
		flushCycleFrameCache();
		_cycleFrames->close();
	}
	
	return true;
}

bool SceneViewWindow::closeCycleFrameMovie() {
	_cycleFrames->close();
	return true;
}

int SceneViewWindow::getGlobalFlag(int offset) {
	// TODO: Verify the offset
	const byte *data = (const byte *)&_globalFlags;
	return READ_UINT16(data + offset);
}

byte SceneViewWindow::getGlobalFlagByte(int offset) {
	// TODO: Verify the offset

	if (offset < 0)
		return 0;

	const byte *data = (const byte *)&_globalFlags;
	return data[offset];
}

bool SceneViewWindow::setGlobalFlag(int offset, int value) {
	// TODO: Verify the offset

	byte *data = (byte *)&_globalFlags;
	WRITE_UINT16(data + offset, value);
	return true;
}

bool SceneViewWindow::setGlobalFlagByte(int offset, byte value) {
	// TODO: Verify the offset

	byte *data = (byte *)&_globalFlags;
	data[offset] = value;
	return true;
}

uint32 SceneViewWindow::getGlobalFlagDWord(int offset) {
	// TODO: Verify the offset
	const byte *data = (const byte *)&_globalFlags;
	return READ_UINT32(data + offset);
}

bool SceneViewWindow::setGlobalFlagDWord(int offset, uint32 value) {
	// TODO: Verify the offset

	byte *data = (byte *)&_globalFlags;
	WRITE_UINT32(data + offset, value);
	return true;
}

bool SceneViewWindow::addNumberToGlobalFlagTable(int tableOffset, int curItemCountOffset, int maxItems, byte numberToAdd) {
	// TODO: Rewrite this
	byte *data = (byte *)&_globalFlags;
	byte *itemCountPtr = data + curItemCountOffset;
	int itemCount = *itemCountPtr;

	if (itemCount >= maxItems)
		return false;

	byte *tableEntries = data + tableOffset;
	for (int i = 0; i < itemCount; i++)
		if (tableEntries[i] == numberToAdd)
			return false;

	tableEntries[itemCount] = numberToAdd;
	*itemCountPtr = itemCount + 1;
	return true;
}

byte SceneViewWindow::getNumberFromGlobalFlagTable(int tableOffset, int tableIndex) {
	const byte *data = (const byte *)&_globalFlags;
	return data[tableOffset + tableIndex];
}

bool SceneViewWindow::isNumberInGlobalFlagTable(int tableOffset, int curItemCountOffset, byte numberToCheck) {
	const byte *data = (const byte *)&_globalFlags;
	int itemCount = *(data + curItemCountOffset);

	const byte *tableEntries = data + tableOffset;

	for (int i = 0; i < itemCount; i++)
		if (tableEntries[i] == numberToCheck)
			return true;

	return false;
}

bool SceneViewWindow::getCurrentSceneLocation(Location &location) {
	if (!_currentScene)
		return false;

	location = _currentScene->_staticData.location;
	return true;
}

bool SceneViewWindow::playSynchronousAnimation(int animationID) {
	_useWaitCursor = true;

	Common::Array<AnimEvent> animDatabase = getAnimationDatabase(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment);

	bool found = false;
	uint i = 0;
	for (; i < animDatabase.size(); i++) {
		if (animDatabase[i].animationID == animationID) {
			found = true;
			break;
		}
	}

	if (!found)
		return false;

	VideoWindow *animationMovie = new VideoWindow(_vm, this);
	Common::String fileName = _vm->getFilePath(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment, animDatabase[i].fileNameID);
	if (!animationMovie->openVideo(fileName))
		error("Failed to open video '%s'", fileName.c_str());

	// Switch to the second audio stream if translation is enabled
	if (_globalFlags.bcTranslateEnabled == 1 && animDatabase[i].audioStreamCount > 1)
		animationMovie->setAudioTrack(2);

	if (_currentScene && _currentScene->movieCallback(this, animationMovie, animationID, MOVIE_START) == SC_FALSE) {
		// FIXME: Nah, why bother to free the movie
		// (Probably, this is never hit)
		return false;
	}

	animationMovie->seekToFrame(animDatabase[i].startFrame);
	animationMovie->enableWindow(false);
	animationMovie->showWindow(kWindowShow);
	_parent->invalidateWindow(false);

	// Empty the input queue
	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);

	// Stop background sound if the video has sound
	if (animDatabase[i].audioStreamCount > 0)
		_vm->_sound->stop();

	animationMovie->playToFrame(animDatabase[i].startFrame + animDatabase[i].frameCount - 1);

	while (!_vm->shouldQuit() && animationMovie->getMode() != VideoWindow::kModeStopped) {
		_vm->yield();
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit()) {
		delete animationMovie;
		return true;
	}

	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);

	// Restart background sound if the video had sound
	if (animDatabase[i].audioStreamCount > 0)
		_vm->_sound->restart();

	if (_currentScene && _currentScene->movieCallback(this, animationMovie, animationID, MOVIE_STOPPED) == SC_FALSE)
		return false;

	delete animationMovie;
	_useWaitCursor = false;
	return true;
}

bool SceneViewWindow::playSynchronousAnimationExtern(int animationID) {
	_useWaitCursor = true;

	VideoWindow *animationMovie = new VideoWindow(_vm, this);
	Common::String fileName = _vm->getFilePath(animationID);
	if (!animationMovie->openVideo(fileName))
		error("Failed to open video '%s'", fileName.c_str());

	if (_currentScene && _currentScene->movieCallback(this, animationMovie, animationID, MOVIE_START) == SC_FALSE) {
		// FIXME: Nah, why bother to free the movie
		// (Probably, this is never hit)
		return false;
	}

	animationMovie->enableWindow(false);
	animationMovie->showWindow(kWindowShow);
	_parent->invalidateWindow(false);

	// Empty the input queue
	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);

	_vm->_sound->stop();
	animationMovie->playVideo();

	while (!_vm->shouldQuit() && animationMovie->getMode() != VideoWindow::kModeStopped) {
		_vm->yield();
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit()) {
		delete animationMovie;
		return true;
	}

	_vm->_sound->restart();
	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);

	if (_currentScene && _currentScene->movieCallback(this, animationMovie, animationID, MOVIE_STOPPED) == SC_FALSE)
		return false;

	delete animationMovie;
	_useWaitCursor = false;
	return true;
}

bool SceneViewWindow::playPlacedSynchronousAnimation(int animationID, int left, int top) {
	_useWaitCursor = true;

	Common::Array<AnimEvent> animDatabase = getAnimationDatabase(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment);

	bool found = false;
	uint i = 0;
	for (; i < animDatabase.size(); i++) {
		if (animDatabase[i].animationID == animationID) {
			found = true;
			break;
		}
	}

	if (!found)
		return false;

	VideoWindow *animationMovie = new VideoWindow(_vm, this);
	Common::String fileName = _vm->getFilePath(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment, animDatabase[i].fileNameID);
	if (!animationMovie->openVideo(fileName))
		error("Failed to open video '%s'", fileName.c_str());

	animationMovie->setWindowPos(kWindowPosTopMost, left, top, 0, 0, kWindowPosNoSize | kWindowPosNoActivate | kWindowPosNoZOrder);

	// Switch to the second audio stream if translation is enabled
	if (_globalFlags.bcTranslateEnabled == 1 && animDatabase[i].audioStreamCount > 1)
		animationMovie->setAudioTrack(2);

	if (_currentScene && _currentScene->movieCallback(this, animationMovie, animationID, MOVIE_START) == SC_FALSE) {
		// FIXME: Nah, why bother to free the movie
		// (Probably, this is never hit)
		return false;
	}

	animationMovie->seekToFrame(animDatabase[i].startFrame);
	animationMovie->enableWindow(false);
	animationMovie->showWindow(kWindowShow);
	_parent->invalidateWindow(false);

	// Empty the input queue
	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);

	// Stop background sound if the video has sound
	if (animDatabase[i].audioStreamCount > 0)
		_vm->_sound->stop();

	animationMovie->playToFrame(animDatabase[i].startFrame + animDatabase[i].frameCount - 1);

	while (!_vm->shouldQuit() && animationMovie->getMode() != VideoWindow::kModeStopped) {
		_vm->yield();
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit()) {
		delete animationMovie;
		return true;
	}

	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);

	// Restart background sound if the video had sound
	if (animDatabase[i].audioStreamCount > 0)
		_vm->_sound->restart();

	if (_currentScene && _currentScene->movieCallback(this, animationMovie, animationID, MOVIE_STOPPED) == SC_FALSE)
		return false;

	delete animationMovie;
	_useWaitCursor = false;
	return true;
}

bool SceneViewWindow::playClippedSynchronousAnimation(int animationID, int left, int top, int right, int bottom) {
	_useWaitCursor = true;

	Common::Array<AnimEvent> animDatabase = getAnimationDatabase(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment);

	bool found = false;
	uint i = 0;
	for (; i < animDatabase.size(); i++) {
		if (animDatabase[i].animationID == animationID) {
			found = true;
			break;
		}
	}

	if (!found)
		return false;

	VideoWindow *animationMovie = new VideoWindow(_vm, this);
	Common::String fileName = _vm->getFilePath(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment, animDatabase[i].fileNameID);
	if (!animationMovie->openVideo(fileName))
		error("Failed to open video '%s'", fileName.c_str());

	animationMovie->setWindowPos(kWindowPosTopMost, left, top, right - left, bottom - top, kWindowPosNoActivate | kWindowPosNoZOrder);

	animationMovie->setSourceRect(Common::Rect(left, top, right, bottom));
	animationMovie->setDestRect(Common::Rect(0, 0, right - left, bottom - top));

	// Switch to the second audio stream if translation is enabled
	if (_globalFlags.bcTranslateEnabled == 1 && animDatabase[i].audioStreamCount > 1)
		animationMovie->setAudioTrack(2);

	if (_currentScene && _currentScene->movieCallback(this, animationMovie, animationID, MOVIE_START) == SC_FALSE) {
		// FIXME: Nah, why bother to free the movie
		// (Probably, this is never hit)
		return false;
	}

	animationMovie->seekToFrame(animDatabase[i].startFrame);
	animationMovie->enableWindow(false);
	animationMovie->showWindow(kWindowShow);
	_parent->invalidateWindow(false);

	// Empty the input queue
	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);

	// Stop background sound if the video has sound
	if (animDatabase[i].audioStreamCount > 0)
		_vm->_sound->stop();

	animationMovie->playToFrame(animDatabase[i].startFrame + animDatabase[i].frameCount - 1);

	while (!_vm->shouldQuit() && animationMovie->getMode() != VideoWindow::kModeStopped) {
		_vm->yield();
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit()) {
		delete animationMovie;
		return true;
	}

	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);

	// Restart background sound if the video had sound
	if (animDatabase[i].audioStreamCount > 0)
		_vm->_sound->restart();

	if (_currentScene && _currentScene->movieCallback(this, animationMovie, animationID, MOVIE_STOPPED) == SC_FALSE)
		return false;

	delete animationMovie;
	_useWaitCursor = false;
	return true;
}

bool SceneViewWindow::startAsynchronousAnimation(int animationID, bool loopAnimation) {
	return startPlacedAsynchronousAnimation(0, 0, 432, 189, animationID, loopAnimation);
}

bool SceneViewWindow::startAsynchronousAnimation(int fileNameID, int startPosition, int playStartPosition, int frameCount, bool loopAnimation) {
	return startPlacedAsynchronousAnimation(0, 0, 432, 189, fileNameID, startPosition, playStartPosition, frameCount, loopAnimation);
}

bool SceneViewWindow::startAsynchronousAnimationExtern(int fileNameID, int startPosition, int playStartPosition, int frameCount, bool loopAnimation) {
	return startPlacedAsynchronousAnimationExtern(0, 0, 432, 189, fileNameID, startPosition, playStartPosition, frameCount, loopAnimation);
}

bool SceneViewWindow::stopAsynchronousAnimation() {
	if (!_currentScene)
		return false;

	if (!_asyncMovie)
		return false;

	_asyncMovie->stopVideo();

	_currentScene->movieCallback(this, _asyncMovie, 0, MOVIE_STOPPED);

	delete _asyncMovie;
	_asyncMovie = 0;
	_asyncMovieFileName.clear();
	_asyncMovieStartFrame = 0;
	_asyncMovieFrameCount = 0;
	_loopAsyncMovie = false;

	return true;
}

bool SceneViewWindow::isAsynchronousAnimationStillPlaying() {
	if (!_asyncMovie)
		return false;

	return _asyncMovie->getMode() != VideoWindow::kModeStopped;
}

int SceneViewWindow::getAsynchronousAnimationCurrentPosition() {
	if (!_asyncMovie)
		return -1;

	return _asyncMovie->getCurFrame();
}

bool SceneViewWindow::asynchronousAnimationTimerCallback() {
	if (!_asyncMovie)
		return false;

	if (_asyncMovie->getMode() == VideoWindow::kModeStopped) {
		if (_loopAsyncMovie) {
			_asyncMovie->seekToFrame(_asyncMovieStartFrame);
			_asyncMovie->playToFrame(_asyncMovieStartFrame + _asyncMovieFrameCount - 1);

			if (_currentScene && _currentScene->movieCallback(this, _asyncMovie, -1, MOVIE_LOOPING_RESTART) == SC_FALSE)
				return false;
		} else {
			if (_currentScene) {
				if (_currentScene->movieCallback(this, _asyncMovie, -1, MOVIE_STOPPED) == SC_TRUE) {
					stopAsynchronousAnimation();
					return true;
				}

				return false;
			} else {
				stopAsynchronousAnimation();
			}
		}
	}

	return true;
}

bool SceneViewWindow::startPlacedAsynchronousAnimation(int left, int top, int width, int height, int animationID, bool loopAnimation) {
	if (!_currentScene)
		return false;

	if (_walkMovie) {
		delete _walkMovie;
		_walkMovie = 0;
		_walkMovieFileName.clear();
	}

	Common::Array<AnimEvent> animDatabase = getAnimationDatabase(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment);

	if (animDatabase.empty())
		return false;

	const AnimEvent *animData = 0;

	for (uint i = 0; i < animDatabase.size() && !animData; i++)
		if (animDatabase[i].animationID == animationID)
			animData = &animDatabase[i];

	if (!animData)
		return false;

	Common::String fileName = _vm->getFilePath(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment, animData->fileNameID);

	if (fileName != _asyncMovieFileName) {
		_asyncMovieFileName.clear();

		if (_asyncMovie) {
			_asyncMovie->stopVideo();
			_asyncMovie->closeVideo();
		} else {
			_asyncMovie = new VideoWindow(_vm, this);
		}

		if (!_asyncMovie->openVideo(fileName))
			return false;

		_asyncMovieFileName = fileName;
	}

	_asyncMovie->setWindowPos(0, left, top, width, height, kWindowPosNoZOrder);
	_asyncMovie->enableWindow(false);

	_asyncMovieStartFrame = animData->startFrame;
	_asyncMovieFrameCount = animData->frameCount;
	_loopAsyncMovie = loopAnimation;

	if (_currentScene->movieCallback(this, _asyncMovie, animationID, MOVIE_START) == SC_FALSE)
		return false;

	_asyncMovie->seekToFrame(animData->startFrame);
	_asyncMovie->showWindow(kWindowShow);
	_asyncMovie->playToFrame(animData->startFrame + animData->frameCount - 1);

	return true;
}

bool SceneViewWindow::startPlacedAsynchronousAnimation(int left, int top, int width, int height, int fileNameID, int startPosition, int playStartPosition, int frameCount, bool loopAnimation) {
	if (!_currentScene)
		return false;

	if (_walkMovie) {
		delete _walkMovie;
		_walkMovie = 0;
		_walkMovieFileName.clear();
	}

	Common::String fileName = _vm->getFilePath(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment, fileNameID);

	if (fileName != _asyncMovieFileName) {
		_asyncMovieFileName.clear();

		if (_asyncMovie) {
			_asyncMovie->stopVideo();
			_asyncMovie->closeVideo();
		} else {
			_asyncMovie = new VideoWindow(_vm, this);
		}

		if (!_asyncMovie->openVideo(fileName))
			return false;

		_asyncMovieFileName = fileName;
	}

	_asyncMovie->setWindowPos(0, left, top, width, height, kWindowPosNoZOrder);
	_asyncMovie->enableWindow(false);

	_asyncMovieStartFrame = (startPosition < 0) ? 0 : startPosition;
	_asyncMovieFrameCount = (frameCount < 0) ? _asyncMovie->getFrameCount() : frameCount;
	_loopAsyncMovie = loopAnimation;

	if (_currentScene->movieCallback(this, _asyncMovie, 0, MOVIE_START) == SC_FALSE)
		return false;

	_asyncMovie->seekToFrame((playStartPosition < 0) ? 0 : playStartPosition);
	_asyncMovie->showWindow(kWindowShow);
	_asyncMovie->playToFrame(_asyncMovieStartFrame + _asyncMovieFrameCount - 1);

	return true;
}

bool SceneViewWindow::startPlacedAsynchronousAnimationExtern(int left, int top, int width, int height, int fileNameID, int startPosition, int playStartPosition, int frameCount, bool loopAnimation) {
	if (!_currentScene)
		return false;

	if (_walkMovie) {
		delete _walkMovie;
		_walkMovie = 0;
		_walkMovieFileName.clear();
	}

	Common::String fileName = _vm->getFilePath(fileNameID);

	if (fileName != _asyncMovieFileName) {
		_asyncMovieFileName.clear();

		if (_asyncMovie) {
			_asyncMovie->stopVideo();
			_asyncMovie->closeVideo();
		} else {
			_asyncMovie = new VideoWindow(_vm, this);
		}

		if (!_asyncMovie->openVideo(fileName))
			return false;

		_asyncMovieFileName = fileName;
	}

	_asyncMovie->setWindowPos(0, left, top, width, height, kWindowPosNoZOrder);
	_asyncMovie->enableWindow(false);

	_asyncMovieStartFrame = (startPosition < 0) ? 0 : startPosition;
	_asyncMovieFrameCount = (frameCount < 0) ? _asyncMovie->getFrameCount() : frameCount;
	_loopAsyncMovie = loopAnimation;

	if (_currentScene->movieCallback(this, _asyncMovie, 0, MOVIE_START) == SC_FALSE)
		return false;

	_asyncMovie->seekToFrame((playStartPosition < 0) ? 0 : playStartPosition);
	_asyncMovie->showWindow(kWindowShow);
	_asyncMovie->playToFrame(_asyncMovieStartFrame + _asyncMovieFrameCount - 1);

	return true;
}

bool SceneViewWindow::retrieveAICommentEntry(const Location &commentLocation, int commentType, const Common::Array<AIComment> &commentDatabase, int &lastFoundIndex, AIComment &currentCommentData) {
	if (commentDatabase.empty() || (uint32)lastFoundIndex >= commentDatabase.size())
		return false;

	const AIComment *commentData = &commentDatabase[lastFoundIndex];

	bool entryFound = false;

	if (_globalFlags.generalWalkthroughMode == 1 && commentType == AI_COMMENT_TYPE_SPONTANEOUS) {
		// Look for any spontaneous comments
		for (; lastFoundIndex < (int)commentDatabase.size() && !entryFound; lastFoundIndex++) {
			if ((commentData->commentFlags & AI_COMMENT_TYPE_SPONTANEOUS || (commentData->commentFlags & AI_COMMENT_TYPE_HELP && commentData->dependencyValueA == 0))
					&& (commentLocation.timeZone == commentData->location.timeZone || commentData->location.timeZone == -1)
					&& (commentLocation.environment == commentData->location.environment || commentData->location.environment == -1)
					&& (commentLocation.node == commentData->location.node || commentData->location.node == -1)
					&& (commentLocation.facing == commentData->location.facing || commentData->location.facing == -1)
					&& (commentLocation.orientation == commentData->location.orientation || commentData->location.orientation == -1)
					&& (commentLocation.depth == commentData->location.depth || commentData->location.depth == -1)) {
				entryFound = true;
			} else {
				commentData++;
			}
		}
	} else {
		for (; lastFoundIndex < (int)commentDatabase.size() && !entryFound; lastFoundIndex++) {
			if ((commentData->commentFlags & commentType)
					&& (commentLocation.timeZone == commentData->location.timeZone || commentData->location.timeZone == -1)
					&& (commentLocation.environment == commentData->location.environment || commentData->location.environment == -1)
					&& (commentLocation.node == commentData->location.node || commentData->location.node == -1)
					&& (commentLocation.facing == commentData->location.facing || commentData->location.facing == -1)
					&& (commentLocation.orientation == commentData->location.orientation || commentData->location.orientation == -1)
					&& (commentLocation.depth == commentData->location.depth || commentData->location.depth == -1)) {
				entryFound = true;
			} else {
				commentData++;
			}
		}
	}

	if (entryFound)
		currentCommentData = *commentData;

	currentCommentData.location = commentLocation;

	return entryFound;
}

bool SceneViewWindow::checkAICommentDependencies(const Location &commentLocation, const AIComment &commentData) {
	// Ignore comments designed for solely adventure mode in walkthrough mode
	if (_globalFlags.generalWalkthroughMode == 1 && commentData.commentFlags & AI_COMMENT_DISABLE_IN_WALKTHROUGH)
		return false;

	byte flagValueA = 0;
	if (commentData.commentFlags & AI_DEPENDENCY_FLAG_NON_BASE_DERIVED_A)
		flagValueA = getGlobalFlagByte(commentData.dependencyFlagOffsetA);
	else
		flagValueA = _globalFlags.aiData[commentData.dependencyFlagOffsetA];

	bool dependencyA;
	if (commentData.commentFlags & AI_DEPENDENCY_CHECK_FOR_MINIMUM_A)
		dependencyA = flagValueA >= commentData.dependencyValueA;
	else
		dependencyA = flagValueA <= commentData.dependencyValueA;

	if (!dependencyA)
		return false;

	if (commentData.commentFlags & AI_COMMENT_FLAG_SPECIAL_LOGIC)
		return checkCustomAICommentDependencies(commentLocation, commentData);

	byte flagValueB = 0;
	if (commentData.commentFlags & AI_DEPENDENCY_FLAG_NON_BASE_DERIVED_B)
		flagValueB = getGlobalFlagByte(commentData.dependencyFlagOffsetB);
	else
		flagValueB = _globalFlags.aiData[commentData.dependencyFlagOffsetB];

	bool dependencyB;
	if (commentData.commentFlags & AI_DEPENDENCY_CHECK_FOR_MINIMUM_B)
		dependencyB = flagValueB >= commentData.dependencyValueB;
	else
		dependencyB = flagValueB <= commentData.dependencyValueB;

	return dependencyB;
}

bool SceneViewWindow::playAICommentFromData(const AIComment &commentData) {
	if (_vm->_sound->isAsynchronousAICommentPlaying())
		return false;

	Common::String commentFileName = "BITDATA/";

	switch (commentData.location.timeZone) {
	case 1: // Castle
		commentFileName += "CASTLE/";

		switch (commentData.location.environment) {
		case 1:
			commentFileName += "CGTT";
			break;
		case 2:
			commentFileName += "CGTS";
			break;
		case 3:
			commentFileName += "CGMW";
			break;
		case 4:
			commentFileName += "CGMB";
			break;
		case 5:
			commentFileName += "CGBS";
			break;
		case 6:
			commentFileName += "CGKC";
			break;
		case 7:
			commentFileName += "CGST";
			break;
		case 8:
			commentFileName += "CGKS";
			break;
		case 9:
			commentFileName += "CGSR";
			break;
		case 10:
			commentFileName += "CGTR";
			break;
		default:
			return false;
		}
		break;
	case 2: // Mayan
		commentFileName += "MAYAN/";

		switch (commentData.location.environment) {
		case 1:
			commentFileName += "MYTP";
			break;
		case 2:
			commentFileName += "MYMC";
			break;
		case 3:
			commentFileName += "MYWG";
			break;
		case 4:
			commentFileName += "MYWT";
			break;
		case 5:
			commentFileName += "MYAG";
			break;
		case 6:
			commentFileName += "MYDG";
			break;
		default:
			return false;
		}
		break;
	case 4: // Future Apartment
		commentFileName += "FUTAPT/";

		switch (commentData.location.environment) {
		case 1:
			commentFileName += "FAKI";
			break;
		case 2:
			commentFileName += "FAER";
			break;
		case 3:
			commentFileName += "FAMN";
			break;
		default:
			return false;
		}
		break;
	case 5: // Da Vinci
		commentFileName += "DAVINCI/";

		switch (commentData.location.environment) {
		case 1:
			commentFileName += "DSPT";
			break;
		case 2:
			commentFileName += "DSCT";
			break;
		case 3:
			commentFileName += "DSGD";
			break;
		case 4:
			commentFileName += "DSWS";
			break;
		case 5:
			commentFileName += "DSCY";
			break;
		default:
			return false;
		}
		break;
	case 6: // Space Station
		commentFileName += "AILAB/";

		switch (commentData.location.environment) {
		case 1:
			commentFileName += "AIHW";
			break;
		case 2:
			commentFileName += "AICR";
			break;
		case 3:
			commentFileName += "AIDB";
			break;
		case 4:
			commentFileName += "AISC";
			break;
		case 5:
			commentFileName += "AINX";
			break;
		case 6:
			commentFileName += "AIIC";
			break;
		case 7:
			commentFileName += "AISW";
			break;
		case 8:
			commentFileName += "AIMR";
			break;
		case 9:
			// There is no 9.
			return false;
		case 10:
			commentFileName += "AIHW";
			break;
		default:
			return false;
		}
		break;
	default:
		return false;
	}

	commentFileName += "_";

	if (commentData.commentFlags & AI_COMMENT_TYPE_INFORMATION)
		commentFileName += "I";
	if (commentData.commentFlags & AI_COMMENT_TYPE_HELP)
		commentFileName += "H";
	if (commentData.commentFlags & AI_COMMENT_TYPE_SPONTANEOUS)
		commentFileName += "C";
	if (commentData.commentFlags & AI_COMMENT_TYPE_OTHER)
		commentFileName += "O";

	commentFileName += Common::String::format("%02d.BTA", commentData.commentID);

	Cursor currentCursor = _vm->_gfx->setCursor(kCursorWait);
	bool playedSuccessfully = _vm->_sound->playAsynchronousAIComment(commentFileName);
	_vm->_gfx->setCursor(currentCursor);

	if (playedSuccessfully) {
		_lastAICommentFileName = commentFileName;

		// This is pure evil. Ugh.
		// The [g|s]etGlobalFlagByte nonsense, anyway.

		byte flagValue = 0;
		if (commentData.commentFlags & AI_STATUS_FLAG_NON_BASE_DERIVED)
			flagValue = getGlobalFlagByte(commentData.statusFlagOffset);
		else
			flagValue = _globalFlags.aiData[commentData.statusFlagOffset];

		flagValue++;

		if (commentData.commentFlags & AI_STATUS_FLAG_NON_BASE_DERIVED)
			setGlobalFlagByte(commentData.statusFlagOffset, flagValue);
		else
			_globalFlags.aiData[commentData.statusFlagOffset] = flagValue;

		return true;
	}

	return false;
}

bool SceneViewWindow::playAIComment(int commentType) {
	if (!_currentScene)
		return false;

	if (_vm->_sound->isAsynchronousAICommentPlaying())
		return false;

	return playAIComment(_currentScene->_staticData.location, commentType);
}

bool SceneViewWindow::playAIComment(const Location &commentLocation, int commentType) {
	// Make sure no other comments are playing
	if (_vm->_sound->isAsynchronousAICommentPlaying())
		return false;

	Common::Array<AIComment> commentDatabase = getAICommentDatabase(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment);

	if (commentDatabase.empty())
		return false;

	AIComment currentCommentData;
	int lastFoundEntry = 0;
	bool playedSuccessfully = false;

	while (retrieveAICommentEntry(commentLocation, commentType, commentDatabase, lastFoundEntry, currentCommentData) && !playedSuccessfully)
		if (checkAICommentDependencies(commentLocation, currentCommentData))
			playedSuccessfully = playAICommentFromData(currentCommentData);

	if (playedSuccessfully) {
		if (commentType == AI_COMMENT_TYPE_HELP && _globalFlags.generalWalkthroughMode == 0)
			_globalFlags.scoreHintsTotal++;

		return true;
	}

	return false;
}

bool SceneViewWindow::checkForAIComment(int commentType) {
	if (!_currentScene)
		return false;

	return checkForAIComment(_currentScene->_staticData.location, commentType);
}

bool SceneViewWindow::checkForAIComment(const Location &commentLocation, int commentType) {
	Common::Array<AIComment> commentDatabase = getAICommentDatabase(_currentScene->_staticData.location.timeZone, _currentScene->_staticData.location.environment);

	if (commentDatabase.empty())
		return false;

	AIComment currentCommentData;
	int lastFoundEntry = 0;

	while (retrieveAICommentEntry(commentLocation, commentType, commentDatabase, lastFoundEntry, currentCommentData))
		if (checkAICommentDependencies(commentLocation, currentCommentData))
			return true;

	return false;
}

bool SceneViewWindow::infoWindowDisplayed(bool flag) {
	if (flag && !_walkMovie) {
		delete _walkMovie;
		_walkMovie = 0;
		_walkMovieFileName.clear();
		changeCycleFrameMovie();
	}

	if (_asyncMovie) {
		if (flag)
			_asyncMovie->showWindow(kWindowHide);
		else
			_asyncMovie->showWindow(kWindowShow);
	}

	if (_bioChipWindowDisplayed && flag)
		((GameUIWindow *)_parent)->_bioChipRightWindow->destroyBioChipViewWindow();

	_infoWindowDisplayed = flag;
	return true;
}

bool SceneViewWindow::bioChipWindowDisplayed(bool flag) {
	if (flag && !_walkMovie) {
		delete _walkMovie;
		_walkMovie = 0;
		_walkMovieFileName.clear();
		changeCycleFrameMovie();
	}

	if (_asyncMovie) {
		if (flag)
			_asyncMovie->showWindow(kWindowHide);
		else
			_asyncMovie->showWindow(kWindowShow);
	}

	if (_infoWindowDisplayed && flag)
		((GameUIWindow *)_parent)->_inventoryWindow->destroyInfoWindow();

	_bioChipWindowDisplayed = flag;
	return true;
}

bool SceneViewWindow::burnedLetterWindowDisplayed(bool flag) {
	if (flag && !_walkMovie) {
		delete _walkMovie;
		_walkMovie = 0;
		_walkMovieFileName.clear();
		changeCycleFrameMovie();
	}

	if (_asyncMovie) {
		if (flag)
			_asyncMovie->showWindow(kWindowHide);
		else
			_asyncMovie->showWindow(kWindowShow);
	}

	if (_burnedLetterDisplayed)
		((GameUIWindow *)_parent)->_inventoryWindow->destroyBurnedLetterWindow();

	_burnedLetterDisplayed = flag;
	return true;
}

bool SceneViewWindow::isAuxWindowDisplayed() {
	return _burnedLetterDisplayed || _infoWindowDisplayed || _bioChipWindowDisplayed;
}

void SceneViewWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (_currentScene && _globalFlags.bcLocateEnabled == 0)
		_currentScene->mouseDown(this, point);
}

void SceneViewWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (_currentScene) {
		if (_globalFlags.bcLocateEnabled == 0)
			_currentScene->mouseUp(this, point);
		else
			_currentScene->locateAttempted(this, point);
	}
}

void SceneViewWindow::onMouseMove(const Common::Point &point, uint flags) {
	_curMousePos = point;
	if (_currentScene)
		_currentScene->mouseMove(this, point);
}

void SceneViewWindow::onKeyUp(const Common::KeyState &key, uint flags) {
	switch (key.keycode) {
	case Common::KEYCODE_a:
		if ((key.flags & Common::KBD_CTRL) && ((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipAI)) {
			((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipAI);
			return;
		}
		break;
	case Common::KEYCODE_b:
		if ((key.flags & Common::KBD_CTRL) && ((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipBlank)) {
			((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipBlank);
			return;
		}
		break;
	case Common::KEYCODE_c:
		if ((key.flags & Common::KBD_CTRL) && ((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipCloak)) {
			((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipCloak);
			return;
		}
		break;
	case Common::KEYCODE_e:
		if ((key.flags & Common::KBD_CTRL) && ((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipEvidence)) {
			((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipEvidence);
			return;
		}
		break;
	case Common::KEYCODE_f:
		if ((key.flags & Common::KBD_CTRL) && ((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipFiles)) {
			((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipFiles);
			return;
		}
		break;
	case Common::KEYCODE_i:
		if ((key.flags & Common::KBD_CTRL) && ((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipInterface)) {
			((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipInterface);
			return;
		}
		break;
	case Common::KEYCODE_j:
		if ((key.flags & Common::KBD_CTRL) && ((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipJump)) {
			((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipJump);
			return;
		}
		break;
	case Common::KEYCODE_t:
		if ((key.flags & Common::KBD_CTRL) && ((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipTranslate)) {
			((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipTranslate);
			return;
		}
		break;
	case Common::KEYCODE_p:
		if (key.flags & Common::KBD_CTRL) {
			// TODO: Pause game
			return;
		}
		break;
	case Common::KEYCODE_q:
		if (key.flags & Common::KBD_CTRL) {
			// Return to main menu
			if (_vm->runQuitDialog())
				((FrameWindow *)_vm->_mainWindow)->showMainMenu();
			return;
		}
		break;
	case Common::KEYCODE_SPACE:
		if (((GameUIWindow *)_parent)->_inventoryWindow->isItemInInventory(kItemBioChipAI) && _globalFlags.bcCloakingEnabled != 1) {
			if (!_lastAICommentFileName.empty() && !_vm->_sound->isAsynchronousAICommentPlaying()) {
				Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);
				_vm->_sound->playAsynchronousAIComment(_lastAICommentFileName);
				_vm->_gfx->setCursor(oldCursor);
			}
			return;
		}
		break;
	default:
		break;
	}

	if (_currentScene)
		_currentScene->onCharacter(this, key);
}

void SceneViewWindow::onPaint() {
	// Original didn't draw if the async movie was playing, but that doesn't seem right.
	if (_currentScene && !_infoWindowDisplayed && !_bioChipWindowDisplayed) {
		if (_currentScene->_staticData.navFrameIndex >= -1) {
			if (_useScenePaint)
				_currentScene->paint(this, _preBuffer);
		} else {
			return;
		}

		// If we have a sprite, update the prebuffer with it now
		if (_currentSprite.image && _useSprite)
			_vm->_gfx->opaqueTransparentBlit(_preBuffer, _currentSprite.xPos, _currentSprite.yPos, _currentSprite.width, _currentSprite.height, _currentSprite.image, 0, 0, 0, _currentSprite.redTrans, _currentSprite.greenTrans, _currentSprite.blueTrans);

		// Update the screen
		_vm->_gfx->blit(_preBuffer, _rect.left, _rect.top);

		if (_useScenePaint)
			_currentScene->gdiPaint(this);
	}
}

void SceneViewWindow::onTimer(uint timer) {
	// Check first to see if this is the demo's sound timer
	if (timer == _demoSoundTimer) {
		// If no sound is playing, restart the ambient
		if (!_vm->_sound->isAmbientSoundPlaying() && !_vm->_sound->isSoundEffectPlaying(_demoSoundEffectHandle)) {
			// Reset the sound effect handle
			_demoSoundEffectHandle = -1;
			startDemoAmbientSound();
		}

		return;
	}

	SoundManager *sound = _vm->_sound; // Take a copy in case we die while in the timer
	sound->timerCallback();

	if (_paused)
		return;

	if (_asyncMovie)
		asynchronousAnimationTimerCallback();

	if (_currentScene && !_infoWindowDisplayed && !_bioChipWindowDisplayed && !_burnedLetterDisplayed)
		_currentScene->timerCallback(this);

	sound->timerCallback();
}

bool SceneViewWindow::onSetCursor(uint message) {
	if (_useWaitCursor) {
		_vm->_gfx->setCursor(kCursorWait);
		return true;
	}

	// Check the scene cursor callback function to see if we need to change the cursor
	int newCursor = (int)kCursorArrow;
	if (_currentScene)
		newCursor = _currentScene->specifyCursor(this, _curMousePos);

	// If the locate button is enabled, follow different logic
	if (_globalFlags.bcLocateEnabled == 1) {
		if (_curCursor >= 0 || (newCursor < 0 && newCursor != _curCursor)) {
			// If the new cursor is less than zero, use it, otherwise use the default locate cursor
			if (newCursor < 0) {
				if (newCursor == -2)
					_curCursor = (int)kCursorLocateB;
				else
					_curCursor = (int)kCursorLocateA;
			} else {
				_curCursor = (int)kCursorLocateA;
			}
		}
	} else {
		_curCursor = newCursor;
	}

	_vm->_gfx->setCursor((Cursor)_curCursor);
	return true;
}

void SceneViewWindow::onEnable(bool enable) {
	// If we're enabling, clear out the message queue of mouse messages
	_vm->removeMouseMessages(this);
}

bool SceneViewWindow::resetNavigationArrows() {
	if (!_currentScene)
		return false;

	if (_globalFlags.bcCloakingEnabled != 1)
		((GameUIWindow *)_parent)->_navArrowWindow->updateAllArrows(_currentScene->_staticData);

	return true;
}

int SceneViewWindow::draggingItem(int itemID, const Common::Point &location, int itemFlags) {
	if (!_currentScene)
		return 0;

	return _currentScene->draggingItem(this, itemID, location, itemFlags);
}

int SceneViewWindow::droppedItem(int itemID, const Common::Point &location, int itemFlags) {
	if (!_currentScene)
		return 0;

	return _currentScene->droppedItem(this, itemID, location, itemFlags);
}

bool SceneViewWindow::updatePrebufferWithSprite(Sprite &spriteData) {
	if (_currentSprite.image != spriteData.image && _currentSprite.image != 0) {
		_currentSprite.image->free();
		delete _currentSprite.image;
	}

	_currentSprite = spriteData;
	invalidateWindow(false);
	return true;
}

bool SceneViewWindow::changeSpriteStatus(bool status) {
	bool prevStatus = _useSprite;
	_useSprite = status;
	return prevStatus;
}

bool SceneViewWindow::resetCursor() {
	_vm->_gfx->setCursor((Cursor)_curCursor);
	return true;
}

bool SceneViewWindow::displayLiveText(const Common::String &text, bool notifyUser) {
	if (((GameUIWindow *)_parent)->_liveTextWindow)
		return ((GameUIWindow *)_parent)->_liveTextWindow->updateLiveText(text, notifyUser);

	return false;
}

bool SceneViewWindow::displayTranslationText(const Common::String &text) {
	if (((GameUIWindow *)_parent)->_liveTextWindow)
		return ((GameUIWindow *)_parent)->_liveTextWindow->updateTranslationText(text);
	
	return false;
}

Common::Array<AnimEvent> SceneViewWindow::getAnimationDatabase(int timeZone, int environment) {
	Common::SeekableReadStream *stream = _vm->getAnimData(_vm->computeAnimDBResourceID(timeZone, environment));
	stream->readUint16LE();

	Common::Array<AnimEvent> animEvents;
	while (stream->pos() < stream->size()) {
		AnimEvent animEvent;
		animEvent.animationID = stream->readSint16LE();
		animEvent.fileNameID = stream->readSint16LE();
		animEvent.audioStreamCount = stream->readSint16LE();
		animEvent.startFrame = stream->readSint32LE();
		animEvent.frameCount = stream->readSint32LE();
		animEvents.push_back(animEvent);
	}

	delete stream;
	return animEvents;
}

Common::Array<AIComment> SceneViewWindow::getAICommentDatabase(int timeZone, int environment) {
	Common::SeekableReadStream *stream = _vm->getAIData(_vm->computeAIDBResourceID(timeZone, environment));
	Common::Array<AIComment> comments;

	if (!stream)
		return comments;

	uint16 count = stream->readUint16LE();
	
	for (uint16 i = 0; i < count; i++) {
		AIComment comment;
		comment.location.timeZone = stream->readSint16LE();
		comment.location.environment = stream->readSint16LE();
		comment.location.node = stream->readSint16LE();
		comment.location.facing = stream->readSint16LE();
		comment.location.orientation = stream->readSint16LE();
		comment.location.depth = stream->readSint16LE();
		comment.commentID = stream->readUint16LE();
		comment.commentFlags = stream->readUint16LE();
		comment.dependencyFlagOffsetA = stream->readUint16LE();
		comment.dependencyValueA = stream->readUint16LE();
		comment.dependencyFlagOffsetB = stream->readUint16LE();
		comment.dependencyValueB = stream->readUint16LE();
		comment.statusFlagOffset = stream->readUint16LE();
		comments.push_back(comment);
	}

	delete stream;
	return comments;
}

void SceneViewWindow::startDemoAmbientSound() {
	assert(_currentScene);

	if (_currentScene->_staticData.location.environment == 5)
		_vm->_sound->setAmbientSound("CASTLE/CGBSSNG.WAV", false, 127);
	else
		_vm->_sound->setAmbientSound("CASTLE/CGMBSNG.WAV", false, 127);
}

} // End of namespace Buried
