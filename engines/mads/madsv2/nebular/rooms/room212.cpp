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

#include "common/scummsys.h"
#include "math/utils.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/rooms/room201.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

void Scene212::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_BOUNCING_REPTILE);
}

void Scene212::enter() {
	// CHECKME: Some useless variables have been remove here

	if (_scene->_priorSceneId == 208) {
		_game._player._playerPos = Common::Point(195, 85);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(67, 117);
		_game._player._facing = FACING_NORTHEAST;
	}

	sceneEntrySound();
}

void Scene212::step() {
	// CHECKME: Could we move the dino?
}

void Scene212::preActions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_CAVE_ENTRANCE))
		_game._player._walkOffScreenSceneId = 111;
}

void Scene212::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(21209);
	else if (_action.isAction(VERB_WALK_TOWARDS) && (_action.isObject(NOUN_FIELD_TO_NORTH) || _action.isObject(NOUN_MOUNTAINS)))
		_scene->_nextSceneId = 208;
	else if (_action.isAction(VERB_WALK_TOWARDS, NOUN_CAVE))
		_scene->_nextSceneId = 111;
	else if (_action.isAction(VERB_LOOK, NOUN_GRASS))
		_vm->_dialogs->show(21201);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCKS))
		_vm->_dialogs->show(21202);
	else if (_action.isAction(VERB_LOOK, NOUN_CAVE_ENTRANCE))
		_vm->_dialogs->show(21203);
	else if (_action.isAction(VERB_LOOK, NOUN_SKY))
		_vm->_dialogs->show(21204);
	else if (_action.isAction(VERB_LOOK, NOUN_FIELD_TO_NORTH))
		_vm->_dialogs->show(21205);
	else if (_action.isAction(VERB_LOOK, NOUN_TREES))
		_vm->_dialogs->show(21206);
	else if (_action.isAction(VERB_LOOK, NOUN_PLANTS))
		_vm->_dialogs->show(21207);
	else if (_action.isAction(VERB_LOOK, NOUN_MOUNTAINS))
		_vm->_dialogs->show(21208);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
