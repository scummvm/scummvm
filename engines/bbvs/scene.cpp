/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bbvs/bbvs.h"
#include "bbvs/gamemodule.h"
#include "bbvs/graphics.h"
#include "bbvs/sound.h"

#include "engines/advancedDetector.h"

namespace Bbvs {

static const int kAfterVideoSceneNum[] = {
	 0, 43, 23, 12,  4, 44,  2,
	16,  4,  4,  4, 44, 12, 44
};

static const int kAfterVideoSceneNumDemo[] = {
	 0, 32, 33, 23,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0
};

void BbvsEngine::loadScene(int sceneNum) {
	debug(0, "BbvsEngine::loadScene() sceneNum: %d", sceneNum);

	Common::String sprFilename = Common::String::format("vnm/vspr%04d.vnm", sceneNum);
	Common::String gamFilename = Common::String::format("vnm/game%04d.vnm", sceneNum);

	_screen->clear();

	_spriteModule->load(sprFilename.c_str());
	_gameModule->load(gamFilename.c_str());

	Palette palette = _spriteModule->getPalette();
	_screen->setPalette(palette);

	// Preload sounds
	for (uint i = 0; i < _gameModule->getPreloadSoundsCount(); ++i) {
		Common::String filename = Common::String::format("snd/snd%05d.aif", _gameModule->getPreloadSound(i));
		_sound->loadSound(filename);
	}

	if (sceneNum >= kMainMenu) {
		DrawList drawList;
		drawList.add(_gameModule->getBgSpriteIndex(0), 0, 0, 0);
		_screen->drawDrawList(drawList, _spriteModule);
		drawScreen();
	}

}

void BbvsEngine::initScene(bool sounds) {

	stopSpeech();
	stopSounds();
	_sound->unloadSounds();

	_gameState = kGSScene;
	_prevSceneNum = _currSceneNum;
	_sceneVisited[_currSceneNum] = 1;
	_mouseCursorSpriteIndex = 0;
	_verbPos.x = -1;
	_verbPos.y = -1;
	_activeItemType = kITEmpty;
	_activeItemIndex = 0;
	_cameraPos.x = 0;
	_cameraPos.y = 0;
	_newCameraPos.x = 0;
	_newCameraPos.y = 0;
	_inventoryButtonIndex = -1;
	_currTalkObjectIndex = -1;
	_currCameraNum = 0;
	_walkMousePos.x = -1;
	_walkMousePos.y = -1;
	_currAction = 0;
	_currActionCommandIndex = -1;
	_currActionCommandTimeStamp = 0;
	_dialogSlotCount = 0;
	_buttheadObject = 0;
	_beavisObject = 0;

	memset(_backgroundSoundsActive, 0, sizeof(_backgroundSoundsActive));

	memset(_sceneObjects, 0, sizeof(_sceneObjects));
	for (int i = 0; i < kSceneObjectsCount; ++i) {
		_sceneObjects[i].walkDestPt.x = -1;
		_sceneObjects[i].walkDestPt.y = -1;
	}

	memset(_dialogItemStatus, 0, sizeof(_dialogItemStatus));

	_sceneObjectActions.clear();

	loadScene(_newSceneNum);
	_currSceneNum = _newSceneNum;
	_newSceneNum = 0;

	for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i)
		_sceneObjects[i].sceneObjectDef = _gameModule->getSceneObjectDef(i);

	for (int i = 0; i < _gameModule->getSceneObjectInitsCount(); ++i) {
		SceneObjectInit *soInit = _gameModule->getSceneObjectInit(i);
		if (evalCondition(soInit->conditions)) {
			SceneObject *sceneObject = &_sceneObjects[soInit->sceneObjectIndex];
			sceneObject->anim = _gameModule->getAnimation(soInit->animIndex);
			sceneObject->animIndex = soInit->animIndex;
			sceneObject->frameIndex = sceneObject->anim->frameCount - 1;
			sceneObject->frameTicks = 1;
			sceneObject->x = soInit->x * 65536;
			sceneObject->y = soInit->y * 65536;
		}
	}

	if (_gameModule->getButtheadObjectIndex() >= 0) {
		_buttheadObject = &_sceneObjects[_gameModule->getButtheadObjectIndex()];
		// Search for the Beavis object
		for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i)
			if (!strcmp(_sceneObjects[i].sceneObjectDef->name, "Beavis")) {
				_beavisObject = &_sceneObjects[i];
				break;
			}
	}

	updateSceneObjectsTurnValue();

	updateWalkableRects();

	_currCameraNum = 0;
	if (_buttheadObject) {
		int minDistance = 0xFFFFFF;
		for (int cameraNum = 0; cameraNum < 4; ++cameraNum) {
			CameraInit *cameraInit = _gameModule->getCameraInit(cameraNum);
			int curDistance = ABS(cameraInit->cameraPos.x - (int)(_buttheadObject->x / 65536) + 160);
			if (curDistance < minDistance) {
				minDistance = curDistance;
				_currCameraNum = cameraNum;
			}
		}
	}

	_cameraPos = _gameModule->getCameraInit(_currCameraNum)->cameraPos;
	_newCameraPos = _cameraPos;

	_walkAreaActions.clear();
	for (int i = 0; i < _gameModule->getActionsCount(); ++i) {
		Action *action = _gameModule->getAction(i);
		for (int j = 0; j < 8; ++j)
			if (action->conditions.conditions[j].cond == kCondIsButtheadAtBgObject)
				_walkAreaActions.push_back(action);
	}

	_mouseCursorSpriteIndex = 0;

	_activeItemIndex = 0;
	_activeItemType = kITEmpty;

	for (int i = 0; i < _gameModule->getActionsCount(); ++i) {
		Action *action = _gameModule->getAction(i);
		if (evalCondition(action->conditions)) {
			_gameState = kGSWait;
			_currAction = action;
			for (uint j = 0; j < action->actionCommands.size(); ++j) {
				ActionCommand *actionCommand = &action->actionCommands[j];
				if (actionCommand->cmd == kActionCmdSetCameraPos) {
					_currCameraNum = actionCommand->param;
					_cameraPos = _gameModule->getCameraInit(_currCameraNum)->cameraPos;
					_newCameraPos = _cameraPos;
					break;
				}
			}
			break;
		}
	}

	if (sounds)
		updateBackgroundSounds();

}

bool BbvsEngine::changeScene() {

	writeContinueSavegame();

	if (_newSceneNum >= 27 && _newSceneNum <= 30) {
		// Run minigames
		stopSpeech();
		stopSounds();
		_sceneVisited[_currSceneNum] = 1;
		if (runMinigame(_newSceneNum - 27)) {
			SWAP(_currSceneNum, _newSceneNum);
		}
	} else if (_newSceneNum >= 31 && _newSceneNum <= 43) {
		// Play video
		stopSpeech();
		stopSounds();
		_sceneVisited[_currSceneNum] = 1;
		_playVideoNumber = _newSceneNum - 30;
		_currSceneNum = _newSceneNum;
		if (_gameDescription->flags & ADGF_DEMO)
			_newSceneNum = kAfterVideoSceneNumDemo[_playVideoNumber];
		else
			_newSceneNum = kAfterVideoSceneNum[_playVideoNumber];
	} else if (_newSceneNum >= 100 && _currSceneNum == kCredits) {
		// Play secret video
		stopSounds();
		_playVideoNumber = _newSceneNum;
		_currSceneNum = 49;
		_newSceneNum = kCredits;
	} else {
		// Normal scene
		initScene(true);
	}

	return true;

}

} // End of namespace Bbvs
