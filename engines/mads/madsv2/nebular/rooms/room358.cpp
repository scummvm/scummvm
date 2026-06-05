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

void Scene358::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene358::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	if (_scene->_priorSceneId == 357)
		_game._player._playerPos = Common::Point(305, 142);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(12, 141);

	sceneEntrySound();
}

void Scene358::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_game._player._walkOffScreenSceneId = 357;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 359;
}

void Scene358::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35815);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(35810);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(35811);
	else if (_action.isAction(VERB_LOOK, NOUN_BED))
		_vm->_dialogs->show(35812);
	else if (_action.isAction(VERB_LOOK, NOUN_SINK))
		_vm->_dialogs->show(35813);
	else if (_action.isAction(VERB_LOOK, NOUN_TOILET))
		_vm->_dialogs->show(35814);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR) || _action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(35816);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(35817);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
