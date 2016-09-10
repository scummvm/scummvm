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

#include "bladerunner/bladerunner.h"

#include "bladerunner/actor.h"
#include "bladerunner/chapters.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/script/script.h"
#include "bladerunner/slice_renderer.h"

#include "common/str.h"
#include "common/stream.h"

namespace BladeRunner {

bool Scene::open(int setId, int sceneId, bool isLoadingGame) {
	if (!isLoadingGame) {
		// flush ADQ
	}

	_setId = setId;
	_sceneId = sceneId;

	const Common::String setName = _vm->_gameInfo->getSetName(_sceneId);

	if (isLoadingGame) {
		// TODO: Set up overlays
	} else {
		_regions->clear();
		_exits->clear();
		// TODO: Reset aesc
		// TODO: Clear regions
		// TODO: Destroy all overlays
		_defaultLoop         =  0;
		_defaultLoopSet      =  0;
		_field_20_loop_stuff =  0;
		_specialLoopMode     = -1;
		_specialLoop         = -1;
		_frame               = -1;
	}

	Common::String vqaName;
	int currentResourceId = _vm->_chapters->currentResourceId();
	if (currentResourceId == 1) {
		vqaName = Common::String::format("%s.VQA", setName.c_str());
	} else {
		vqaName = Common::String::format("%s_%d.VQA", setName.c_str(), MIN(currentResourceId, 3));
	}

	if (!_vqaPlayer.open(vqaName))
		return false;

	Common::String sceneName = _vm->_gameInfo->getSetName(sceneId);
	if (!_vm->_script->open(sceneName))
		return false;

	if (!isLoadingGame)
		_vm->_script->InitializeScene();

	Common::String setResourceName = Common::String::format("%s-MIN.SET", sceneName.c_str());
	if (!_set->open(setResourceName))
		return false;

	_vm->_sliceRenderer->setView(*_vm->_view);

	if (isLoadingGame) {
		// TODO: Advance VQA frame
		if (sceneId >= 73 && sceneId <= 76)
			_vm->_script->InitializeScene();
		return true;
	}

	// TODO: set VQADecoder parameters

	// TODO: Set actor position from scene info
	_vm->_playerActor->setAtXYZ(_actorStartPosition, _actorStartFacing);

	// TODO: Set actor set

	_vm->_script->SceneLoaded();

	_vm->_sceneObjects->clear();

	// Init click map
	int actorCount = _vm->_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i) {
		Actor *actor = _vm->_actors[i];
		if (actor->getSetId() == setId) {
			_vm->_sceneObjects->addActor(
				i,
				actor->getBoundingBox(),
				actor->getScreenRectangle(),
				1,
				0,
				actor->isTargetable(),
				actor->isRetired());
		}
	}

	_set->addObjectsToScene(_vm->_sceneObjects);
	// TODO: add all items to scene
	// TODO: calculate walking obstacles??

	// if (_playerWalkedIn) { // TODO: Not _playerWalkedIn
	// 	_vm->_script->PlayerWalkedIn();
	// }

	return true;
}

int Scene::advanceFrame(Graphics::Surface &surface, uint16 *&zBuffer) {
	int frame = _vqaPlayer.update();
	if (frame >= 0) {
		surface.copyFrom(*_vqaPlayer.getSurface());
		memcpy(zBuffer, _vqaPlayer.getZBuffer(), 640*480*2);
		_view = _vqaPlayer.getView();
	}

	if (frame < 0) {
		return frame;
	}
	_frame = frame;

	if (_specialLoopMode == 0 && frame == _vqaPlayer.getLoopEndFrame(_specialLoop)) {
		_playerWalkedIn = true;
		_specialLoopMode = -1;
	}
	if (_specialLoopMode == 0 && !_defaultLoopSet) {
		_vqaPlayer.setLoop(_defaultLoop + 1);
		_defaultLoopSet = true;
	}

	return frame;
}

void Scene::setActorStart(Vector3 position, int facing) {
	_actorStartPosition = position;
	_actorStartFacing = facing;
}

void Scene::loopSetDefault(int a) {
	// warning("\t\t\tScene::loopSetDefault(%d)", a);
	_defaultLoop = a;
}

void Scene::loopStartSpecial(int a, int b, int c) {
	// warning("\t\t\tScene::loopStartSpecial(%d, %d, %d)", a, b, c);
	_specialLoopMode = a;
	_specialLoop = b;

	if (_specialLoop == 1) {
		// a1->on_loop_end_switch_to_set_id = sub_42BE08_options_get_set_enter_arg_1(&unk_48E910_options);
		// a1->on_loop_end_switch_to_scene_id = sub_42BE00_options_get_set_enter_arg_2(&unk_48E910_options);
	}

	if (c) {
		// _field_20_loop_stuff = 1;
		// v6 = a1->_field_28_loop_special_loop_number;
		// sub_453434_scene_method_loop(a1);
	}
}

int Scene::findObject(const char *objectName) {
	return _set->findObject(objectName);
}

bool Scene::objectSetHotMouse(int objectId) {
	return _set->objectSetHotMouse(objectId);
}

bool Scene::objectGetBoundingBox(int objectId, BoundingBox* boundingBox) {
	return _set->objectGetBoundingBox(objectId, boundingBox);
}

void Scene::objectSetIsClickable(int objectId, bool isClickable, bool sceneLoaded) {
	_set->objectSetIsClickable(objectId, isClickable);
	if (sceneLoaded) {
		_vm->_sceneObjects->setIsClickable(objectId + 198, isClickable);
	}
}

void Scene::objectSetIsObstacle(int objectId, bool isObstacle, bool sceneLoaded, bool updateWalkpath) {
	_set->objectSetIsObstacle(objectId, isObstacle);
	if (sceneLoaded) {
		_vm->_sceneObjects->setIsObstacle(objectId + 198, isObstacle);
		if(updateWalkpath) {
			_vm->_sceneObjects->updateWalkpath();
		}
	}
}

void Scene::objectSetIsObstacleAll(bool isObstacle, bool sceneLoaded) {
	int i;
	for (i = 0; i < (int)_set->getObjectCount(); i++) {
		_set->objectSetIsObstacle(i, isObstacle);
		if (sceneLoaded) {
			_vm->_sceneObjects->setIsObstacle(i + 198, isObstacle);
		}
	}
}

void Scene::objectSetIsTarget(int objectId, bool isTarget, bool sceneLoaded) {
	_set->objectSetIsTarget(objectId, isTarget);
	if (sceneLoaded) {
		_vm->_sceneObjects->setIsTarget(objectId + 198, isTarget);
	}
}

const char *Scene::objectGetName(int objectId) {
	return _set->objectGetName(objectId);
}

} // End of namespace BladeRunner
