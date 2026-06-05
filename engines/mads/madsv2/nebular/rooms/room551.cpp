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

void Scene551::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene551::enter() {
	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 0));
	else
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 1));

	if (_scene->_priorSceneId == 501)
		_game._player._playerPos = Common::Point(18, 130);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(124, 119);
		_game._player._facing = FACING_NORTH;
	}

	if (_globals[kTeleporterCommand]) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;

		char sepChar;
		if (_globals[kSexOfRex] == REX_MALE)
			sepChar = 'e';
		else
			sepChar = 'u';

		int suffixNum;
		int trigger;

		switch (_globals[kTeleporterCommand]) {
		case 1:
			suffixNum = 3;
			trigger = 75;
			_globals[kTeleporterUnderstood] = true;
			break;

		case 2:
			suffixNum = 1;
			trigger = 80;
			break;

		case 4:
			suffixNum = 2;
			trigger = 90;
			break;

		default:
			trigger = 0;
			suffixNum = 0;
		}

		_globals[kTeleporterCommand] = 0;

		if (suffixNum > 0)
			_scene->loadAnimation(formAnimName(sepChar, suffixNum), trigger);
		else {
			_game._player._visible = true;
			_game._player._stepEnabled = true;
		}
	}

	sceneEntrySound();
}

void Scene551::step() {
	switch (_game._trigger) {
	case 75:
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		break;

	case 80:
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
		break;

	case 90:
		if (_globals[kSexOfRex] == REX_MALE) {
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
		} else {
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		}
		_vm->_sound->command(28);
		_scene->_sequences.addTimer(60, 91);
		break;

	case 91:
		_scene->_reloadSceneFlag = true;
		break;

	default:
		break;
	}
}

void Scene551::preActions() {
	if (_action.isAction(VERB_WALK_DOWN) && (_action.isObject(NOUN_STREET_TO_WEST) || _action.isObject(NOUN_SIDEWALK_TO_WEST)))
		_game._player._walkOffScreenSceneId = 501;
}

void Scene551::actions() {
	if (_action.isAction(VERB_STEP_INTO, NOUN_TELEPORTER))
		_scene->_nextSceneId = 502;
	else if ((_action._lookFlag))
		_vm->_dialogs->show(55117);
	else if (_action.isAction(VERB_LOOK, NOUN_SKELETON))
		_vm->_dialogs->show(55110);
	else if (_action.isAction(VERB_LOOK, NOUN_ELEVATOR_SHAFT))
		_vm->_dialogs->show(55111);
	else if (_action.isAction(VERB_WALKTO, NOUN_ELEVATOR_SHAFT))
		_vm->_dialogs->show(55112);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING))
		_vm->_dialogs->show(55113);
	else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER))
		_vm->_dialogs->show(55114);
	else if (_action.isAction(VERB_LOOK, NOUN_SIDEWALK_TO_WEST)) {
		if (_game._visitedScenes.exists(505))
			_vm->_dialogs->show(55116);
		else
			_vm->_dialogs->show(55115);
	} else if (_action.isAction(VERB_LOOK, NOUN_SIDEWALK))
		_vm->_dialogs->show(55118);
	else if (_action.isAction(VERB_LOOK, NOUN_EQUIPMENT_OVERHEAD))
		_vm->_dialogs->show(55119);
	else if (_action.isAction(VERB_LOOK, NOUN_RAILING))
		_vm->_dialogs->show(55120);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
