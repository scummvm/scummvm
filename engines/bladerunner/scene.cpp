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
#include "bladerunner/actor_dialogue_queue.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/chapters.h"
#include "bladerunner/game_info.h"
#include "bladerunner/items.h"
#include "bladerunner/overlays.h"
#include "bladerunner/regions.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/screen_effects.h"
#include "bladerunner/set.h"
#include "bladerunner/settings.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/script/police_maze.h"
#include "bladerunner/script/scene_script.h"
#include "bladerunner/ui/spinner.h"
#include "bladerunner/vqa_player.h"
#include "bladerunner/zbuffer.h"

#include "common/str.h"

namespace BladeRunner {

Scene::Scene(BladeRunnerEngine *vm)
	: _vm(vm),
	_setId(-1),
	_sceneId(-1),
	_vqaPlayer(nullptr),
	_defaultLoop(0),
	_defaultLoopSet(false),
	_specialLoopMode(kSceneLoopModeLoseControl),
	_specialLoop(0),
	_defaultLoopPreloadedSet(false),
	// _introFinished(false),
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
		_vm->_actorDialogueQueue->flush(1, false);
	}

	_vm->walkingReset();

	_setId = setId;
	_sceneId = sceneId;

	const Common::String sceneName = _vm->_gameInfo->getSceneName(_sceneId);

	if (isLoadingGame) {
		_vm->_overlays->resume(true);
	} else {
		_regions->clear();
		_exits->clear();
#if BLADERUNNER_ORIGINAL_BUGS
#else
		_vm->_screenEffects->toggleEntry(-1, false); // clear the skip list
#endif
		_vm->_screenEffects->_entries.clear();
		_vm->_overlays->removeAll();
		_defaultLoop = 0;
		_defaultLoopSet = false;
		_defaultLoopPreloadedSet = false;
		_specialLoopMode = kSceneLoopModeNone;
		_specialLoop = -1;
		_frame = -1;
	}

	Common::String vqaName;
	int currentResourceId = _vm->_chapters->currentResourceId();
	if (currentResourceId == 1) {
		vqaName = Common::String::format("%s.VQA", sceneName.c_str());
	} else {
		vqaName = Common::String::format("%s_%d.VQA", sceneName.c_str(), MIN(currentResourceId, 3));
	}

	if (_vqaPlayer != nullptr) {
		delete _vqaPlayer;
	}

	_vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceBack, vqaName);

	if (!_vm->_sceneScript->open(sceneName)) {
		return false;
	}

	if (!isLoadingGame) {
		_vm->_sceneScript->initializeScene();
	}

	Common::String setResourceName = Common::String::format("%s-MIN.SET", sceneName.c_str());
	if (!_set->open(setResourceName)) {
		return false;
	}

	_vm->_sliceRenderer->setView(_vm->_view);

	if (isLoadingGame) {
		resume(true);
		if (sceneId == kScenePS10    // police maze
		    || sceneId == kScenePS11 // police maze
		    || sceneId == kScenePS12 // police maze
		    || sceneId == kScenePS13 // police maze
#if BLADERUNNER_ORIGINAL_BUGS
#else
		    || sceneId == kSceneUG01 // Steam room
#endif // BLADERUNNER_ORIGINAL_BUGS
		) {
			_vm->_sceneScript->sceneLoaded();
		}
		return true;
	}

	if (!_vqaPlayer->open()) {
		return false;
	}

	if (_specialLoopMode == kSceneLoopModeNone) {
		startDefaultLoop();
	}

	// This frame advancement (frame skip) may be required here
	// It is in the original code and possible initializes some variables
	// (or perhaps z-buffering related stuff)
	// It may cause issues when in a scene we need to trigger some action
	// based on the first frame of the loop when entering the scene (using SceneFrameAdvanced())
	// (eg. it is contributing to the barrel flame glitch in pan from DR04 to DR01)
	// However, better to resolve those issues with a workaround (eg. using InitializeScene())
	advanceFrame();

	_vm->_playerActor->setAtXYZ(_actorStartPosition, _actorStartFacing);
	_vm->_playerActor->setSetId(setId);

	_vm->_sceneScript->sceneLoaded();

	_vm->_sceneObjects->clear();

	// Init click map
	int actorCount = _vm->_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i) {
		Actor *actor = _vm->_actors[i];
		if (actor->getSetId() == setId) {
			//debug("Actor added: %d", i);
#if !BLADERUNNER_ORIGINAL_BUGS
			// ensure that actors' "hotspot" areas from previous scene are cleared up
			actor->resetScreenRectangleAndBbox();
#endif
			_vm->_sceneObjects->addActor(
				i + kSceneObjectOffsetActors,
				actor->getBoundingBox(),
				actor->getScreenRectangle(),
				true,
				false,
				actor->isTarget(),
				actor->isRetired()
			);
		}
	}

	_set->addObjectsToScene(_vm->_sceneObjects);
	_vm->_items->addToSet(setId);
	_vm->_sceneObjects->updateObstacles();

	if (_specialLoopMode != kSceneLoopModeLoseControl) {
		_vm->_sceneScript->playerWalkedIn();
	}

	return true;
}

bool Scene::close(bool isLoadingGame) {
	bool result = true;
	if (getSetId() == -1) {
		return true;
	}

	_vm->_policeMaze->clear(!isLoadingGame);

	if (isLoadingGame) {
		_vm->_sceneScript->playerWalkedOut();
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

int Scene::advanceFrame(bool useTime) {
	int frame = _vqaPlayer->update(false, true, useTime);
	if (frame >= 0) {
		blit(_vm->_surfaceBack, _vm->_surfaceFront);
		_vqaPlayer->updateZBuffer(_vm->_zbuffer);
		_vqaPlayer->updateView(_vm->_view);
		_vqaPlayer->updateScreenEffects(_vm->_screenEffects);
		_vqaPlayer->updateLights(_vm->_lights);
	}

	if (_specialLoopMode == kSceneLoopModeLoseControl || _specialLoopMode == kSceneLoopModeOnce || _specialLoopMode == kSceneLoopModeSpinner) {
		if (!_defaultLoopSet) {
			_vqaPlayer->setLoop(_defaultLoop, -1, kLoopSetModeEnqueue, &Scene::loopEndedStatic, this);
			_defaultLoopSet = true;
			if (_specialLoopMode == kSceneLoopModeLoseControl) {
				_vm->playerLosesControl();
			}
		}
	} else if (_specialLoopMode == kSceneLoopModeChangeSet) {
		if (frame == -3) { // EOF
			_vm->_settings->setNewSetAndScene(_nextSetId, _nextSceneId);
			_vm->playerGainsControl();
		}
	} else if (_specialLoopMode == kSceneLoopModeNone) {
		if (!_defaultLoopPreloadedSet) {
			_vqaPlayer->setLoop(_defaultLoop + 1, -1, kLoopSetModeJustStart, &Scene::loopEndedStatic, this);
			_defaultLoopPreloadedSet = true;
		}
	}

	if (frame >= 0) {
		_frame = frame;
	}

	return frame;
}

void Scene::resume(bool isLoadingGame) {
	if (!_vqaPlayer) {
		return;
	}

	int targetFrame = _frame;

	if (isLoadingGame) {
		_vqaPlayer->open();
	} else {
		_vm->_zbuffer->disable();
	}

	if (_specialLoopMode == kSceneLoopModeNone) {
		startDefaultLoop();
	} else {
		if (_specialLoopMode == kSceneLoopModeChangeSet) {
			_vm->_settings->setNewSetAndScene(_setId, _sceneId);
		}
		if (_defaultLoopPreloadedSet) {
			_specialLoopMode = kSceneLoopModeNone;
			startDefaultLoop();
			advanceFrame(false);
			loopStartSpecial(_specialLoopMode, _specialLoop, false);
		} else {
			_defaultLoopPreloadedSet = true;
			loopStartSpecial(_specialLoopMode, _specialLoop, true);
			if (_specialLoopMode == kSceneLoopModeLoseControl || _specialLoopMode == kSceneLoopModeChangeSet) {
				_vm->playerGainsControl();

			}
		}
		if (_specialLoopMode == kSceneLoopModeChangeSet) {
			_vm->_settings->clearNewSetAndScene();
		}
	}

	int frame;
	do {
		frame = advanceFrame(false);
	} while (frame >= 0 && frame != targetFrame);

	if (!isLoadingGame) {
		_vm->_zbuffer->enable();
	}
}

void Scene::startDefaultLoop() {
	_vqaPlayer->setLoop(_defaultLoop, -1, kLoopSetModeImmediate, nullptr, nullptr);
	_defaultLoopSet = true;
	_defaultLoopPreloadedSet = false;
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
		_defaultLoopPreloadedSet = true;
		loopEnded(0, _specialLoop);
	}
}

int Scene::findObject(const Common::String &objectName) {
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
		_vm->_sceneObjects->setIsClickable(objectId + kSceneObjectOffsetObjects, isClickable);
	}
}

void Scene::objectSetIsObstacle(int objectId, bool isObstacle, bool sceneLoaded, bool updateWalkpath) {
	_set->objectSetIsObstacle(objectId, isObstacle);
	if (sceneLoaded) {
		_vm->_sceneObjects->setIsObstacle(objectId + kSceneObjectOffsetObjects, isObstacle);
		if (updateWalkpath) {
			_vm->_sceneObjects->updateObstacles();
		}
	}
}

void Scene::objectSetIsObstacleAll(bool isObstacle, bool sceneLoaded) {
	int i;
	for (i = 0; i < (int)_set->getObjectCount(); ++i) {
		_set->objectSetIsObstacle(i, isObstacle);
		if (sceneLoaded) {
			_vm->_sceneObjects->setIsObstacle(i + kSceneObjectOffsetObjects, isObstacle);
		}
	}
}

void Scene::objectSetIsTarget(int objectId, bool isTarget, bool sceneLoaded) {
	_set->objectSetIsTarget(objectId, isTarget);
	if (sceneLoaded) {
		_vm->_sceneObjects->setIsTarget(objectId + kSceneObjectOffsetObjects, isTarget);
	}
}

const Common::String &Scene::objectGetName(int objectId) {
	return _set->objectGetName(objectId);
}

void Scene::loopEnded(int frame, int loopId) {
	if (_specialLoopMode == kSceneLoopModeLoseControl || _specialLoopMode == kSceneLoopModeOnce || _specialLoopMode == kSceneLoopModeSpinner) {
		if (_defaultLoopPreloadedSet) {
			_vqaPlayer->setLoop(_defaultLoop, -1, kLoopSetModeEnqueue, &Scene::loopEndedStatic, this);
			_defaultLoopSet = true;
			_defaultLoopPreloadedSet = false;
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
			_specialLoopMode = kSceneLoopModeNone;
			_specialLoop = -1;
			_vqaPlayer->setLoop(_defaultLoop + 1, -1, kLoopSetModeJustStart, nullptr, nullptr);
			_defaultLoopPreloadedSet = true;
		}
	} else if (_specialLoopMode == kSceneLoopModeChangeSet) {
		_defaultLoopSet = true;
		_defaultLoopPreloadedSet = false;
		_vm->playerLosesControl();
	}
}

void Scene::loopEndedStatic(void *data, int frame, int loopId) {
	((Scene *)data)->loopEnded(frame, loopId);
}

void Scene::save(SaveFileWriteStream &f) {
	f.writeInt(_setId);
	f.writeInt(_sceneId);
	f.writeInt(_defaultLoop);
	f.writeBool(_defaultLoopSet);
	f.writeBool(_defaultLoopPreloadedSet);
	f.writeInt(_specialLoopMode);
	f.writeInt(_specialLoop);
	f.writeInt(_nextSetId);
	f.writeInt(_nextSceneId);
	f.writeInt(_frame);
	f.writeVector3(_actorStartPosition);
	f.writeInt(_actorStartFacing);
	f.writeBool(_playerWalkedIn);
}

void Scene::load(SaveFileReadStream &f) {
	_setId = f.readInt();
	_sceneId = f.readInt();
	_defaultLoop = f.readInt();
	_defaultLoopSet = f.readBool();
	_defaultLoopPreloadedSet = f.readBool();
	_specialLoopMode = f.readInt();
	_specialLoop = f.readInt();
	_nextSetId = f.readInt();
	_nextSceneId = f.readInt();
	_frame = f.readInt();
	_actorStartPosition = f.readVector3();
	_actorStartFacing = f.readInt();
	_playerWalkedIn = f.readBool();
}

} // End of namespace BladeRunner
