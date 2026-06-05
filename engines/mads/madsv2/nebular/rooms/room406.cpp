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

Scene406::Scene406(RexNebularEngine *vm) : Scene4xx(vm) {
	_hitStorageDoor = false;
}

void Scene406::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_hitStorageDoor);
}

void Scene406::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene406::enter() {
	_game._player._visible = true;
	if (_scene->_priorSceneId == 405) {
		_game._player._playerPos = Common::Point(15, 129);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 407) {
		_game._player._playerPos = Common::Point(270, 127);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 410) {
		_game._player._playerPos = Common::Point(30, 108);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId == 411) {
		_game._player._playerPos = Common::Point(153, 108);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(15, 129);
		_game._player._facing = FACING_EAST;
	}

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*ROXCL_8");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 1));

	if (_scene->_roomChanged) {
		_globals[kStorageDoorOpen] = false;
		_game._objects.addToInventory(OBJ_SECURITY_CARD);
	}

	if (!_globals[kStorageDoorOpen])
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);

	if (_scene->_priorSceneId != 411)
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	else {
		_game._player._stepEnabled = false;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 3, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
		_vm->_sound->command(19);
	}

	_game.loadQuoteSet(0x24F, 0);
	_hitStorageDoor = false;
	sceneEntrySound();
}

void Scene406::step() {
	if (_game._trigger == 90) {
		_game._player._stepEnabled = true;
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	}

	if (_game._trigger == 80)
		_scene->_nextSceneId = 411;

	if (_game._trigger == 100) {
		_vm->_dialogs->show(40622);
		_hitStorageDoor = true;
	}

	if (_game._trigger == 110) {
		_scene->_sequences.addTimer(20, 111);
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
	}

	if (_game._trigger == 111) {
		_game._player._stepEnabled = true;
		_vm->_dialogs->show(40613);
	}

	if (_game._trigger == 70) {
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		_vm->_sound->command(19);
	}

	if (_game._trigger == 71) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_globals[kStorageDoorOpen] = false;
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 75) {
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
		_globals[kStorageDoorOpen] = true;
		_game._player._stepEnabled = true;
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
		_vm->_sound->command(19);
	}
}

void Scene406::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 405;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_game._player._walkOffScreenSceneId = 407;

	if (_action.isAction(VERB_TAKE))
		_game._player._needToWalk = false;

	if (_action.isAction(VERB_LOOK, NOUN_SIGN) || _action.isAction(VERB_LOOK, NOUN_TRASH))
		_game._player._needToWalk = true;
}

void Scene406::actions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) && (_game._player._targetPos.x > 100)) {
		_game._player._stepEnabled = false;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 3, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
		_vm->_sound->command(19);
	} else if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) && _globals[kStorageDoorOpen] && (_game._player._targetPos.x < 100))
		_scene->_nextSceneId = 410;
	else if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) && !_globals[kStorageDoorOpen] && (_game._player._targetPos.x < 100)) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, _game.getQuote(0x24F));
		if (!_hitStorageDoor) {
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addTimer(80, 100);
		}
	} else if (_action.isAction(VERB_PUT, NOUN_SECURITY_CARD, NOUN_CARD_SLOT) && !_globals[kStorageDoorOpen]) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
		Common::Point msgPos = Common::Point(_game._player._playerPos.x, _game._player._playerPos.y + 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], msgPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[2], 87);
	} else if (_action.isAction(VERB_PUT, NOUN_SECURITY_CARD, NOUN_CARD_SLOT) && _globals[kStorageDoorOpen]) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		Common::Point msgPos = Common::Point(_game._player._playerPos.x, _game._player._playerPos.y + 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], msgPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[2], 87);
	} else if (_action.isAction(VERB_PUT, NOUN_CARD_SLOT)) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 110);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], _game._player._playerPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[2], 87);
	} else if (_action.isAction(VERB_LOOK, NOUN_TRASH))
		_vm->_dialogs->show(40610);
	else if (_action.isAction(VERB_TAKE, NOUN_TRASH))
		_vm->_dialogs->show(40611);
	else if (_action.isAction(VERB_LOOK, NOUN_CARD_SLOT))
		_vm->_dialogs->show(40612);
	else if (_action.isAction(VERB_LOOK, NOUN_FIRE_EXTINGUISHER))
		_vm->_dialogs->show(40614);
	else if (_action.isAction(VERB_TAKE, NOUN_FIRE_EXTINGUISHER))
		_vm->_dialogs->show(40615);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(40616);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(40617);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR) || _action._lookFlag)
		_vm->_dialogs->show(40618);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(40619);
	else if (_action.isAction(VERB_LOOK, NOUN_DOOR)) {
		if (_globals[kStorageDoorOpen])
			_vm->_dialogs->show(40621);
		else
			_vm->_dialogs->show(40620);
	} else if (_action.isAction(VERB_LOOK, NOUN_MONITOR))
		_vm->_dialogs->show(40623);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGNPOST))
		_vm->_dialogs->show(40624);
	else if (_action.isAction(VERB_TAKE, NOUN_SIGNPOST))
		_vm->_dialogs->show(40625);
	else if (_action.isAction(VERB_LOOK, NOUN_BOULDER))
		_vm->_dialogs->show(40626);
	else if (_action.isAction(VERB_TAKE, NOUN_BOULDER))
		_vm->_dialogs->show(40627);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGN))
		_vm->_dialogs->show(40628);
	else if (_action.isAction(VERB_TAKE, NOUN_SIGN))
		_vm->_dialogs->show(40629);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
