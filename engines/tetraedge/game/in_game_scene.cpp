/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "common/path.h"
#include "common/textconsole.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/billboard.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/in_game_scene.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/object3d.h"

namespace Tetraedge {

InGameScene::InGameScene() : _character(nullptr) {
}

void InGameScene::draw() {
	error("TODO: implement InGameScene::draw");
}

bool InGameScene::changeBackground(const Common::String &name) {
	if (Common::File::exists(name)) {
		TeSpriteLayout *spriteLayout = _bgGui.spriteLayout("root");
		assert(spriteLayout);
		spriteLayout->load(name);
		return true;
	}
	return false;
}


/*static*/ float InGameScene::angularDistance(float a1, float a2) {
	float result;

	result = a2 - a1;
	if (result >= -3.141593 && result > 3.141593) {
		result = result + -6.283185;
	} else {
		result = result + 6.283185;
	}
	return result;
}

TeLayout *InGameScene::background() {
	return _bgGui.layout("background");
}

void InGameScene::close() {
	reset();
	error("TODO: Implement InGameScene::close");
}

void InGameScene::reset() {
	if (_character)
		_character->setFreeMoveZone(Common::SharedPtr<TeFreeMoveZone>());
	freeSceneObjects();
	_bgGui.unload();
	unloadSpriteLayouts();
	_gui2.unload();
	_gui3.unload();
}

void InGameScene::deleteAllCallback() {
	warning("TODO: implement InGameScene::deleteAllCallback");
}


void InGameScene::freeSceneObjects() {
	if (_character) {
		warning("TODO: InGameScene::freeSceneObjects: Set field on character here");
		_character->deleteAllCallback();
	}
	if (_characters.size() == 1) {
		_characters[0]->deleteAllCallback();
	}

	Game *game = g_engine->getGame();
	game->unloadCharacters();

	_characters.clear();

	for (Object3D *obj : _object3Ds) {
		obj->deleteLater();
	}
	_object3Ds.clear();

	for (Billboard *bb : _billboards) {
		bb->deleteLater();
	}
	_billboards.clear();

	for (TeSpriteLayout *sprite : _sprites) {
		sprite->deleteLater();
	}
	_sprites.clear();

	deleteAllCallback();
	_markers.clear();

	for (InGameScene::AnchorZone *zone : _anchorZones) {
		delete zone;
	}
	_anchorZones.clear();
}

void InGameScene::unloadSpriteLayouts() {
	for (auto *animobj : _animObjects) {
		delete animobj;
	}
	_animObjects.clear();
}

Character *InGameScene::character(const Common::String &name) {
	error("TODO: Implement InGameScene::character");
}

bool InGameScene::loadCharacter(const Common::String &name) {
	error("TODO: Implement InGameScene::loadCharacter");
}

bool InGameScene::loadPlayerCharacter(const Common::String &name) {
	if (_character == nullptr) {
		_character = new Character();
		if (!_character->loadModel(name, true)) {
			_playerCharacterModel.release();
			return false;
		}

		_playerCharacterModel = _character->_model;

		if (!findKate()) {
			Common::Array<TeIntrusivePtr<TeModel>> &ms = models();
			ms.push_back(_character->_model);
			ms.push_back(_character->_shadowModel[0]);
			ms.push_back(_character->_shadowModel[1]);
		}
	}

	_character->_model->setVisible(true);
	return true;
}

void InGameScene::unloadPlayerCharacter(const Common::String &name) {
	error("TODO: Implement InGameScene::unloadPlayerCharacter %s", name.c_str());
}

void InGameScene::unloadCharacter(const Common::String &name) {
	warning("TODO: Implement InGameScene::unloadCharacter %s", name.c_str());
}

bool InGameScene::findKate() {
	for (auto &m : models()) {
		if (m->name() == "Kate")
			return true;
	}
	return false;
}

Common::Path InGameScene::getBlockersFileName() {
	Game *game = g_engine->getGame();
	Common::Path retval("scenes");
	retval.joinInPlace(game->currentZone());
	retval.joinInPlace(game->currentScene());
	retval.joinInPlace("blockers.bin");
	return retval;
}

void InGameScene::loadBlockers() {
	_blockers.clear();
	_rectBlockers.clear();
	const Common::Path blockersPath = getBlockersFileName();
	if (Common::File::exists(blockersPath)) {
		error("TODO: Implement InGameScene::loadBlockers");
	}
}

void InGameScene::loadBackground(const Common::Path &path) {
	_bgGui.load(path);
	TeLayout *bg = _bgGui.layout("background");
	TeLayout *root = _bgGui.layout("root");
	bg->setRatioMode(TeILayout::RATIO_MODE_NONE);
	root->setRatioMode(TeILayout::RATIO_MODE_NONE);
	TeCamera *wincam = g_engine->getApplication()->mainWindowCamera();
	bg->disableAutoZ();
	bg->setZPosition(wincam->_orthNearVal);

	for (auto layoutEntry : _bgGui.spriteLayouts()) {
		AnimObject *animobj = new AnimObject();
		animobj->_name = layoutEntry._key;
		animobj->_layout = layoutEntry._value;
		animobj->_layout->_tiledSurfacePtr->_frameAnim.onFinished().add<AnimObject>(animobj, &AnimObject::onFinished);
		if (animobj->_name != "root")
			animobj->_layout->setVisible(false);
		_animObjects.push_back(animobj);
	}
}

void InGameScene::loadInteractions(const Common::Path &path) {
	error("TODO: Implement InGameScene::loadInteractions");
}

void InGameScene::setStep(const Common::String &scene, const Common::String &step1, const Common::String &step2) {
	SoundStep ss;
	ss._stepSound1 = step1;
	ss._stepSound2 = step2;
	_soundSteps[scene] = ss;
}

void InGameScene::initScroll() {
	_someScrollVector = TeVector2f32(0.5f, 0.0f);
}

bool InGameScene::AnimObject::onFinished() {
	error("TODO: Implement InGameScene::AnimObject::onFinished");
}

} // end namespace Tetraedge
