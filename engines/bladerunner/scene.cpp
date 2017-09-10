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

#include "bladerunner/scene.h"

#include "bladerunner/actor.h"
#include "bladerunner/adq.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/chapters.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/items.h"
#include "bladerunner/overlays.h"
#include "bladerunner/regions.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/screen_effects.h"
#include "bladerunner/set.h"
#include "bladerunner/settings.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/vqa_player.h"
#include "bladerunner/script/scene.h"
#include "bladerunner/spinner.h"

#include "common/str.h"

namespace BladeRunner {

Scene::Scene(BladeRunnerEngine *vm)
	: _vm(vm),
	_setId(-1),
	_sceneId(-1),
	_vqaPlayer(nullptr),
	_defaultLoop(0),
	_defaultLoopSet(false),
	_specialLoopMode(0),
	_specialLoop(0),
	_specialLoopAtEnd(false),
	_introFinished(false),
	_nextSetId(-1),
	_nextSceneId(-1),
	_frame(0),
	_actorStartFacing(0),
	_playerWalkedIn(false),
	_set(new Set(vm)),
	_regions(new Regions()),
	_exits(new Regions()) {
}

Scene::~Scene() {
	delete _set;
	delete _regions;
	delete _exits;
	delete _vqaPlayer;
}

bool Scene::open(int setId, int sceneId, bool isLoadingGame) {
	if (!isLoadingGame) {
		_vm->_adq->flush(1, false);
	}

	_setId = setId;
	_sceneId = sceneId;

	const Common::String setName = _vm->_gameInfo->getSceneName(_sceneId);

	if (isLoadingGame) {
		// TODO: Set up overlays
	} else {
		_regions->clear();
		_exits->clear();
		_vm->_screenEffects->_entries.clear();
		_vm->_overlays->removeAll();
		_defaultLoop = 0;
		_defaultLoopSet = false;
		_specialLoopAtEnd = false;
		_specialLoopMode = -1;
		_specialLoop = -1;
		_frame = -1;
	}

	Common::String vqaName;
	int currentResourceId = _vm->_chapters->currentResourceId();
	if (currentResourceId == 1) {
		vqaName = Common::String::format("%s.VQA", setName.c_str());
	} else {
		vqaName = Common::String::format("%s_%d.VQA", setName.c_str(), MIN(currentResourceId, 3));
	}

	if (_vqaPlayer != nullptr) {
		delete _vqaPlayer;
	}

	_vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceInterface);

	Common::String sceneName = _vm->_gameInfo->getSceneName(sceneId);
	if (!_vm->_sceneScript->Open(sceneName)) {
		return false;
	}

	if (!isLoadingGame) {
		_vm->_sceneScript->InitializeScene();
	}

	Common::String setResourceName = Common::String::format("%s-MIN.SET", sceneName.c_str());
	if (!_set->open(setResourceName)) {
		return false;
	}

	_vm->_sliceRenderer->setView(*_vm->_view);

	if (isLoadingGame) {
		// TODO: Advance VQA frame
		if (sceneId >= 73 && sceneId <= 76)
			_vm->_sceneScript->SceneLoaded();
		return true;
	}

	if (!_vqaPlayer->open(vqaName)) {
		return false;
	}

	if (_specialLoop == -1) {
		_vqaPlayer->setLoop(_defaultLoop, -1, kLoopSetModeImmediate, nullptr, nullptr);
		_defaultLoopSet = true;
		_specialLoopAtEnd = false;
	}
	_vm->_scene->advanceFrame();

	_vm->_playerActor->setAtXYZ(_actorStartPosition, _actorStartFacing);
	_vm->_playerActor->setSetId(setId);

	_vm->_sceneScript->SceneLoaded();

	_vm->_sceneObjects->clear();

	// Init click map
	int actorCount = _vm->_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i) {
		Actor *actor = _vm->_actors[i];
		if (actor->getSetId() == setId) {
			_vm->_sceneObjects->addActor(
				   i + SCENE_OBJECTS_ACTORS_OFFSET,
				   actor->getBoundingBox(),
				   actor->getScreenRectangle(),
				   1,
				   0,
				   actor->isTargetable(),
				   actor->isRetired());
		}
	}

	_set->addObjectsToScene(_vm->_sceneObjects);
	_vm->_items->addToSet(setId);
	_vm->_sceneObjects->updateObstacles();
	// TODO: add all items to scene
	// TODO: calculate walking obstacles??

	if (_specialLoopMode) {
		_vm->_sceneScript->PlayerWalkedIn();
	}

	return true;
}

bool Scene::close(bool isLoadingGame) {
	bool result = true;
	if (getSetId() == -1) {
		return true;
	}

	//_vm->_policeMaze->clear(!isLoadingGame);
	if (isLoadingGame) {
		_vm->_sceneScript->PlayerWalkedOut();
	}

	//	if (SceneScript_isLoaded() && !SceneScript_unload()) {
	//		result = false;
	//	}
	if (_vqaPlayer != nullptr) {
		//_vqaPlayer->stop();
		delete _vqaPlayer;
		_vqaPlayer = nullptr;
	}
	_sceneId = -1;
	_setId = -1;

	return result;
}

int Scene::advanceFrame() {
	int frame = _vqaPlayer->update();
	if (frame >= 0) {
		blit(_vm->_surfaceInterface, _vm->_surfaceGame);
		_vqaPlayer->updateZBuffer(_vm->_zbuffer);
		_vqaPlayer->updateView(_vm->_view);
		_vqaPlayer->updateScreenEffects(_vm->_screenEffects);
		_vqaPlayer->updateLights(_vm->_lights);
	}
	if (_specialLoopMode && _specialLoopMode != kSceneLoopMode2 && _specialLoopMode != kSceneLoopModeSpinner) {
		if (_specialLoopMode == kSceneLoopModeChangeSet) {
			if (frame == -3) { // TODO: when will this happen? bad data in/eof of vqa
				_vm->_settings->setNewSetAndScene(_nextSetId, _nextSceneId);
				_vm->playerGainsControl();
			}
		} else if (!_specialLoopAtEnd) {
			_vqaPlayer->setLoop(_defaultLoop + 1, -1, kLoopSetModeJustStart, &Scene::loopEndedStatic, this);
			_specialLoopAtEnd = true;
		}
	} else if (!_defaultLoopSet) {
		_vqaPlayer->setLoop(_defaultLoop, -1, kLoopSetModeEnqueue, &Scene::loopEndedStatic, this);
		_defaultLoopSet = true;
		if (_specialLoopMode == kSceneLoopModeLoseControl) {
			_vm->playerLosesControl();
		}
	}

	if (frame >= 0) {
		_frame = frame;
	}

	return frame;
}

void Scene::setActorStart(Vector3 position, int facing) {
	_actorStartPosition = position;
	_actorStartFacing = facing;
}

void Scene::loopSetDefault(int loopId) {
	_defaultLoop = loopId;
}

void Scene::loopStartSpecial(int specialLoopMode, int loopId, bool immediately) {
	_specialLoopMode = specialLoopMode;
	_specialLoop = loopId;

	int repeats = -1;
	if (_specialLoopMode == kSceneLoopModeChangeSet) {
		repeats = 0;
	}

	int loopMode = kLoopSetModeEnqueue;
	if (immediately) {
		loopMode = kLoopSetModeImmediate;
	}

	_vqaPlayer->setLoop(_specialLoop, repeats, loopMode, &Scene::loopEndedStatic, this);
	if (_specialLoopMode == kSceneLoopModeChangeSet) {
		_nextSetId = _vm->_settings->getNewSet();
		_nextSceneId = _vm->_settings->getNewScene();
	}
	if (immediately) {
		_specialLoopAtEnd = true;
		loopEnded(0, _specialLoop);
	}
}

int Scene::findObject(const char *objectName) {
	return _set->findObject(objectName);
}

bool Scene::objectSetHotMouse(int objectId) {
	return _set->objectSetHotMouse(objectId);
}

bool Scene::objectGetBoundingBox(int objectId, BoundingBox *boundingBox) {
	return _set->objectGetBoundingBox(objectId, boundingBox);
}

void Scene::objectSetIsClickable(int objectId, bool isClickable, bool sceneLoaded) {
	_set->objectSetIsClickable(objectId, isClickable);
	if (sceneLoaded) {
		_vm->_sceneObjects->setIsClickable(objectId + SCENE_OBJECTS_OBJECTS_OFFSET, isClickable);
	}
}

void Scene::objectSetIsObstacle(int objectId, bool isObstacle, bool sceneLoaded, bool updateWalkpath) {
	_set->objectSetIsObstacle(objectId, isObstacle);
	if (sceneLoaded) {
		_vm->_sceneObjects->setIsObstacle(objectId + SCENE_OBJECTS_OBJECTS_OFFSET, isObstacle);
		if (updateWalkpath) {
			_vm->_sceneObjects->updateObstacles();
		}
	}
}

void Scene::objectSetIsObstacleAll(bool isObstacle, bool sceneLoaded) {
	int i;
	for (i = 0; i < (int)_set->getObjectCount(); i++) {
		_set->objectSetIsObstacle(i, isObstacle);
		if (sceneLoaded) {
			_vm->_sceneObjects->setIsObstacle(i + SCENE_OBJECTS_OBJECTS_OFFSET, isObstacle);
		}
	}
}

void Scene::objectSetIsTarget(int objectId, bool isTarget, bool sceneLoaded) {
	_set->objectSetIsTarget(objectId, isTarget);
	if (sceneLoaded) {
		_vm->_sceneObjects->setIsTarget(objectId + SCENE_OBJECTS_OBJECTS_OFFSET, isTarget);
	}
}

const char *Scene::objectGetName(int objectId) {
	return _set->objectGetName(objectId);
}

void Scene::loopEnded(int frame, int loopId) {
	if (_specialLoopMode && _specialLoopMode != kSceneLoopMode2 && _specialLoopMode != kSceneLoopModeSpinner) {
		if (_specialLoopMode == kSceneLoopModeChangeSet) {
			_defaultLoopSet = true;
			_specialLoopAtEnd = false;
			_vm->playerLosesControl();
		}
	} else if (_specialLoopAtEnd) {
		_vqaPlayer->setLoop(_defaultLoop, -1, kLoopSetModeEnqueue, &Scene::loopEndedStatic, this);
		_defaultLoopSet = true;
		_specialLoopAtEnd = false;
		if (_specialLoopMode == kSceneLoopModeLoseControl) {
			_vm->playerLosesControl();
		}
	} else {
		if (_specialLoopMode == kSceneLoopModeLoseControl) {
			_vm->playerGainsControl();
			_playerWalkedIn = true;
		}
		if (_specialLoopMode == kSceneLoopModeSpinner) {
			_vm->_spinner->open();
		}
		_specialLoopMode = -1;
		_specialLoop = -1;
		_vqaPlayer->setLoop(_defaultLoop + 1, -1, kLoopSetModeJustStart, nullptr, nullptr);
		_specialLoopAtEnd = true;
	}
}

void Scene::loopEndedStatic(void *data, int frame, int loopId) {
	((Scene*)data)->loopEnded(frame, loopId);
}
} // End of namespace BladeRunner
