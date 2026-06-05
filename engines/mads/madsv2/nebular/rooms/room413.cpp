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

Scene413::Scene413(RexNebularEngine *vm) : Scene4xx(vm) {
	_rexDeath = -1;
	_canMove = -1;
}

void Scene413::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsSint32LE(_rexDeath);
	s.syncAsSint32LE(_canMove);
}

void Scene413::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene413::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 2));
	_rexDeath = false;

	if (_scene->_priorSceneId == 405) {
		_game._player._playerPos = Common::Point(142, 146);
		_game._player._facing = FACING_NORTH;
		_game._player._visible = true;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		if (_globals[kSexOfRex] == REX_MALE) {
			_scene->loadAnimation(Resources::formatName(413, 'd', 1, EXT_AA, ""), 78);
			_vm->_sound->command(30);
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_rexDeath = true;
		} else if (!_globals[kTeleporterCommand]) {
			_game._player._playerPos = Common::Point(136, 117);
			_game._player.walk(Common::Point(141, 130), FACING_SOUTH);
			_game._player._facing = FACING_SOUTH;
			_game._player._visible = true;
		}
	}

	if ((_globals[kTeleporterCommand]) && (!_rexDeath)) {
		switch (_globals[kTeleporterCommand]) {
		case 1:
			_vm->_sound->command(30);
			_game._player._visible = false;
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 19);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
			break;

		case 2:
			_game._player._visible = false;
			_vm->_sound->command(30);
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 20);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 77);
			break;

		case 3:
		case 4:
			_game._player._playerPos = Common::Point(136, 117);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(141, 130), FACING_SOUTH);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_globals[kTeleporterCommand] = 0;
	}

	_canMove = true;
	sceneEntrySound();
}

void Scene413::step() {
	if (_scene->_animation[0] && _scene->_animation[0]->getCurrentFrame() == 38)
		_scene->_animation[0]->setCurrentFrame(37);

	if (_scene->_animation[0] && _scene->_animation[0]->getCurrentFrame() == 21 && _canMove) {
		_vm->_sound->command(27);
		_canMove = false;
	}

	if (_game._trigger == 76) {
		_game._player._playerPos = Common::Point(136, 117);
		_game._player.walk(Common::Point(141, 130), FACING_SOUTH);
		_game._player._facing = FACING_SOUTH;
		_game._player.selectSeries();
		_game._player._visible = true;
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 77) {
		_globals[kTeleporterCommand] = TELEPORTER_BEAM_IN;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}

	if (_game._trigger == 78) {
		_scene->_reloadSceneFlag = true;
		_scene->_nextSceneId = _scene->_priorSceneId;
		_globals[kTeleporterCommand] = TELEPORTER_NONE;
	}
}

void Scene413::preActions() {
	if (_action.isAction(VERB_TAKE) || _action.isAction(VERB_PUT, NOUN_CONVEYOR_BELT))
		_game._player._needToWalk = false;

	if (_action.isAction(VERB_LOOK, NOUN_WOODEN_STATUE) || _action.isAction(VERB_LOOK, NOUN_DISPLAY)
		|| _action.isAction(VERB_LOOK, NOUN_PICTURE) || _action.isAction(VERB_LOOK, NOUN_PLANT)) {
		_game._player._needToWalk = true;
	}
}

void Scene413::actions() {
	if (_action.isAction(VERB_WALK_INSIDE, NOUN_TELEPORTER)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 409;
	} else if (_action.isAction(VERB_WALK_INTO, NOUN_CORRIDOR_TO_SOUTH))
		_scene->_nextSceneId = 405;
	else if (_action.isAction(VERB_LOOK, NOUN_WOODEN_STATUE))
		_vm->_dialogs->show(41310);
	else if (_action.isAction(VERB_TAKE, NOUN_WOODEN_STATUE))
		_vm->_dialogs->show(41311);
	else if (_action.isAction(VERB_LOOK, NOUN_CONVEYOR_BELT))
		_vm->_dialogs->show(41312);
	else if (_action.isAction(VERB_PUT, NOUN_CONVEYOR_BELT))
		_vm->_dialogs->show(41313);
	else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER))
		_vm->_dialogs->show(41314);
	else if (_action.isAction(VERB_LOOK, NOUN_DISPLAY))
		_vm->_dialogs->show(41315);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(41316);
	else if (_action.isAction(VERB_LOOK, NOUN_PICTURE))
		_vm->_dialogs->show(41317);
	else if (_action.isAction(VERB_LOOK, NOUN_PLANT))
		_vm->_dialogs->show(41318);
	else if (_action.isAction(VERB_TAKE, NOUN_PLANT))
		_vm->_dialogs->show(41319);
	else if (_action._lookFlag)
		_vm->_dialogs->show(41320);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
