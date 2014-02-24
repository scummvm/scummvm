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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/scene.h"
#include "mads/mads.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

Scene::Scene(MADSEngine *vm): _vm(vm) {
	_sectionNum = 1;
	_sectionNumPrior = -1;
	_priorSectionNum = 0;
	_priorSceneId = 0;
	_nextSceneId = 0;
	_currentSceneId = 0;
	_vocabBuffer = nullptr;
	_sceneLogic = nullptr;
}

Scene::~Scene() {
	delete[] _vocabBuffer;
	delete _sceneLogic;
}

void Scene::clearSprites(bool flag) {
	for (int i = 0; i < TEXT_DISPLAY_COUNT; ++i)
		_textDisplay[i]._active = false;

	if (flag)
		_spriteList.clear();

	_spriteSlots.clear();
	_spriteSlots.push_back(SpriteSlot(ST_FULL_SCREEN_REFRESH, -1));
}

/**
 * Releases any sprites used by the player
 */
void Scene::releasePlayerSprites() {
	Player &player = _vm->_game->player();

	if (player._spritesLoaded && player._numSprites > 0) {
		int spriteEnd = player._spriteListStart + player._numSprites - 1;
		do {
			deleteSpriteEntry(spriteEnd);
		} while (--spriteEnd >= player._spriteListStart);
	}
}

void Scene::deleteSpriteEntry(int listIndex) {
	delete _spriteList[listIndex];
	_spriteList.remove_at(listIndex);
}

void Scene::clearDynamicHotspots() {
	_dynamicHotspots.clear();
	_dynamicHotspotsChanged = false;
}

void Scene::clearVocab() {
	freeVocab();
	_activeVocabs.clear();
}

void Scene::freeVocab() {
	delete[] _vocabBuffer;
	_vocabBuffer = nullptr;
}

void Scene::addActiveVocab(int vocabId) {
	if (activeVocabIndexOf(vocabId) == -1) {
		assert(_activeVocabs.size() < 200);
		_activeVocabs.push_back(vocabId);
	}
}

int Scene::activeVocabIndexOf(int vocabId) {
	for (uint i = 0; i < _activeVocabs.size(); ++i) {
		if (_activeVocabs[i] == vocabId)
			return i;
	}

	return -1;
}

void Scene::loadScene() {
	delete _sceneLogic;

	switch (_vm->getGameID()) {
	case GType_RexNebular:
		_sceneLogic = Nebular::SceneFactory::createScene(this);
		break;
	default:
		error("Unknown game");
	}
}

void Scene::free() {
	warning("TODO: Scene::free");
}

/*------------------------------------------------------------------------*/

SpriteSlot::SpriteSlot() {
	_spriteType = ST_NONE;
	_seqIndex = 0;
	_spriteListIndex = 0;
	_frameNumber = 0;
	_depth = 0;
	_scale = 0;
}

SpriteSlot::SpriteSlot(SpriteType type, int seqIndex) {
	_spriteType = type;
	_seqIndex = seqIndex;
	_spriteListIndex = 0;
	_frameNumber = 0;
	_depth = 0;
	_scale = 0;
}

/*------------------------------------------------------------------------*/

TextDisplay::TextDisplay() {
	_active = false;
	_spacing = 0;
	_expire = 0;
	_col1 = _col2 = 0;
}

/*------------------------------------------------------------------------*/

DynamicHotspot::DynamicHotspot() {
	_seqIndex = 0;
	_facing = 0;
	_descId = 0;
	_field14 = 0;
	_articleNumber = 0;
	_cursor = 0;
}

} // End of namespace MADS
