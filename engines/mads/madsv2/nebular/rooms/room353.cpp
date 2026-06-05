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

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

void Scene353::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene353::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(303, 'B', 0, EXT_SS, ""));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 0, 5, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

	if (_scene->_priorSceneId == 352)
		_game._player._playerPos = Common::Point(144, 95);
	else
		_game._player._playerPos = Common::Point(139, 155);

	sceneEntrySound();
}

void Scene353::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35315);
	else if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY))
		_scene->_nextSceneId = 352;
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH))
		_scene->_nextSceneId = 354;
	else if (_action.isAction(VERB_LOOK, NOUN_ROCK_CHUNK))
		_vm->_dialogs->show(35310);
	else if (_action.isAction(VERB_LOOK, NOUN_PIPES) || _action.isAction(VERB_LOOK, NOUN_PIPE))
		_vm->_dialogs->show(35311);
	else if (_action.isAction(VERB_LOOK, NOUN_BROKEN_BEAM))
		_vm->_dialogs->show(35312);
	else if (_action.isAction(VERB_LOOK, NOUN_DOORWAY))
		_vm->_dialogs->show(35313);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(35314);
	else if (_action.isAction(VERB_LOOK, NOUN_FLOOR))
		_vm->_dialogs->show(35316);
	else if (_action.isAction(VERB_LOOK, NOUN_CEILING))
		_vm->_dialogs->show(35317);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(35318);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
