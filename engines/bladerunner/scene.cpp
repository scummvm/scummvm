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
#include "bladerunner/script/script.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/scene_objects.h"

#include "common/str.h"
#include "common/stream.h"

namespace BladeRunner {

bool Scene::open(int setId, int sceneId, bool isLoadingGame) {
	if (!isLoadingGame) {
		// flush ADQ
	}
	// reset mouse button status
	_setId = setId;
	_sceneId = sceneId;

	const Common::String setName = _vm->_gameInfo->getSetName(_sceneId);

	if (isLoadingGame) {
		// TODO: Set up overlays
	} else {
		// TODO: Clear regions
		// reset aesc
		// TODO: Destroy all overlays
		_defaultLoop = 0;
		_frame = -1;
		//_loopStartSpecial = -1;
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

	_vm->_sliceRenderer->setView(_vm->_view);

	if (isLoadingGame) {
		// resume()
		if (sceneId >= 73 && sceneId <= 76) {
			_vm->_script->SceneLoaded();
			return true;
		}
	}

	// TODO: set VQADecoder parameters

	// TODO: Set actor position from scene info
	//advance frame 0
	_vm->_playerActor->set_at_xyz(_actorStartPosition, _actorStartFacing);
	// TODO: Set actor set
	_vm->_script->SceneLoaded();

	_vm->_sceneObjects->reset();
	
	// Init click map
	int actorCount = _vm->_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i) {
		Actor *actor = _vm->_actors[i];
		if (actor->getSet() == setId) {
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
	
	_set->addAllObjectsToScene(_vm->_sceneObjects);
	//add all items to scene
	//calculate walking obstacles??

	 if (_playerWalkedIn) {
		//_vm->_script->PlayerWalkedIn();
	 }
	
	return true;
}

int Scene::advanceFrame(Graphics::Surface &surface, uint16 *&zBuffer) {
	int frame = _vqaPlayer.update();
	if (frame >= 0) {
		surface.copyFrom(*_vqaPlayer.getSurface());
		memcpy(zBuffer, _vqaPlayer.getZBuffer(), 640*480*2);
	}
	return frame;
}

void Scene::setActorStart(Vector3 position, int facing) {
	_actorStartPosition = position;
	_actorStartFacing = facing;
}

} // End of namespace BladeRunner
