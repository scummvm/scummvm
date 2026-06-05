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

Scene752::Scene752(RexNebularEngine *vm) : Scene7xx(vm) {
	_cardId = -1;
}

void Scene752::synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);

	s.syncAsSint16LE(_cardId);
}

void Scene752::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_ID_CARD);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(VERB_LOOK_AT);
	_scene->addActiveVocab(NOUN_LASER_BEAM);
}

void Scene752::enter() {
	_globals._spriteIndexes[14] = _scene->_sprites.addSprites(formAnimName('l', -1));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RXMBD_8");

	if (_scene->_priorSceneId == 751) {
		_game._player._playerPos = Common::Point(13, 145);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(289, 138);
		_game._player.walk(Common::Point(262, 148), FACING_WEST);
		_game._player._facing = FACING_WEST;
		_game._player._visible = true;
	}

	if (_game._objects[OBJ_ID_CARD]._roomNumber == 752) {
		_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('i', -1));
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		int idx = _scene->_dynamicHotspots.add(NOUN_ID_CARD, VERB_WALKTO, _globals._sequenceIndexes[13], Common::Rect(0, 0, 0, 0));
		_cardId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(234, 135), FACING_NORTH);
	}

	if (_game._globals[kLaserHoleIsThere]) {
		_globals._sequenceIndexes[14] = _scene->_sequences.startCycle(_globals._spriteIndexes[14], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 13);
		int idx = _scene->_dynamicHotspots.add(NOUN_LASER_BEAM, VERB_LOOK_AT, _globals._sequenceIndexes[14], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(215, 130), FACING_NORTHWEST);
	}

	if (_game._globals[kTeleporterCommand]) {
		switch (_game._globals[kTeleporterCommand]) {
		case TELEPORTER_BEAM_OUT:
		case TELEPORTER_WRONG:
		case TELEPORTER_STEP_OUT:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}

		_game._globals[kTeleporterCommand] = TELEPORTER_NONE;
	}

	if (_globals._timebombTimer > 0)
		_globals._timebombTimer = 10800 - 600;

	sceneEntrySound();
}

void Scene752::step() {
	if (_globals._timebombTimer >= 10800 && _game._globals[kTimebombStatus] == TIMEBOMB_ACTIVATED) {
		_globals[kTimebombStatus] = TIMEBOMB_DEAD;
		_globals._timebombTimer = 0;
		_globals[kCheckDaemonTimebomb] = false;
		_scene->_nextSceneId = 620;
	}
}

void Scene752::preActions() {
	if (_action.isAction(VERB_WALKTO, NOUN_WEST_END_OF_PLATFORM)) {
		_game._player._walkOffScreenSceneId = 751;
	}
}

void Scene752::actions() {
	if (_action.isAction(VERB_WALK_ALONG, NOUN_PLATFORM))
		;
	else if (_action.isAction(VERB_STEP_INTO, NOUN_TELEPORTER)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 711;
	} else if (_action.isAction(VERB_TAKE, NOUN_ID_CARD) && (!_game._objects.isInInventory(OBJ_ID_CARD) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			_vm->_sound->command(15);
			_scene->_sequences.remove(_globals._sequenceIndexes[13]);
			_game._objects.addToInventory(OBJ_ID_CARD);
			_scene->_dynamicHotspots.remove(_cardId);
			_vm->_dialogs->showItem(OBJ_ID_CARD, 830);
			break;
		case 2:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && (_action._savedFields._mainObjectSource == CAT_HOTSPOT) &&
		(!_game._objects.isInInventory(OBJ_BONES) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			_vm->_sound->command(15);
			if (_game._objects.isInInventory(OBJ_BONE))
				_game._objects.setRoom(OBJ_BONE, NOWHERE);
			_game._objects.addToInventory(OBJ_BONES);
			_vm->_dialogs->showItem(OBJ_BONES, 75221);
			break;
		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[12]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_CITY)) {
		if (_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(75212);
		else
			_vm->_dialogs->show(75210);
	} else if (_action.isAction(VERB_LOOK, NOUN_PLATFORM))
		_vm->_dialogs->show(75213);
	else if (_action.isAction(VERB_LOOK, NOUN_CEMENT_BLOCK))
		_vm->_dialogs->show(75214);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCK))
		_vm->_dialogs->show(75215);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCK))
		_vm->_dialogs->show(75216);
	else if (_action.isAction(VERB_LOOK, NOUN_WEST_END_OF_PLATFORM))
		_vm->_dialogs->show(75217);
	else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER))
		_vm->_dialogs->show(75218);
	else if ((_action.isAction(VERB_LOOK, NOUN_BONES) || _action.isAction(VERB_LOOK, NOUN_ID_CARD)) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		if (_game._objects[OBJ_ID_CARD]._roomNumber == 752)
			_vm->_dialogs->show(75219);
		else
			_vm->_dialogs->show(75220);
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && (_action._savedFields._mainObjectSource == CAT_HOTSPOT)) {
		if (_game._objects.isInInventory(OBJ_BONES))
			_vm->_dialogs->show(75222);
	} else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
