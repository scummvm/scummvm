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

Scene512::Scene512(RexNebularEngine *vm) : Scene5xx(vm) {
	_fishingRodHotspotId = -1;
	_keyHotspotId = -1;
}

void room_512_synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsSint16LE(_fishingRodHotspotId);
	s.syncAsSint16LE(_keyHotspotId);
}

void Scene512::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_FISHING_ROD);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_PADLOCK_KEY);
	_scene->addActiveVocab(NOUN_REGISTER_DRAWER);
}

static void room_512_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('r', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*RXMRC_8");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 3));

	if (_game._objects[OBJ_FISHING_ROD]._roomNumber == _scene->_currentSceneId) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		_fishingRodHotspotId = _scene->_dynamicHotspots.add(NOUN_FISHING_ROD, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_fishingRodHotspotId, Common::Point(199, 101), FACING_NORTHEAST);
	}

	if (!_game._visitedScenes._sceneRevisited)
		_globals[kRegisterOpen] = false;

	_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);
	if (_game._difficulty == DIFFICULTY_EASY) {
		if (_game._objects[OBJ_PADLOCK_KEY]._roomNumber == _scene->_currentSceneId) {
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 3);
			_keyHotspotId = _scene->_dynamicHotspots.add(NOUN_PADLOCK_KEY, VERB_WALKTO, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(_keyHotspotId, Common::Point(218, 152), FACING_NORTHEAST);
		}
		if (_globals[kRegisterOpen]) {
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
			_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);
		}
	} else if (_globals[kRegisterOpen]) {
		if (_game._objects[OBJ_PADLOCK_KEY]._roomNumber == _scene->_currentSceneId) {
			_scene->_hotspots.activate(NOUN_PADLOCK_KEY, true);
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 3);
		} else {
			_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		}
	} else
		_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(144, 152);
		_game._player._facing = FACING_NORTHEAST;
	}

	sceneEntrySound();
}

static void room_512_parser() {
	if (_action.isAction(VERB_WALK, NOUN_OUTSIDE))
		_scene->_nextSceneId = 511;
	else if (_action.isAction(VERB_TAKE, NOUN_FISHING_ROD)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_FISHING_ROD)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(_fishingRodHotspotId);
				_game._objects.addToInventory(OBJ_FISHING_ROD);
				_vm->_dialogs->showItem(OBJ_FISHING_ROD, 51217);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (_action.isAction(VERB_OPEN, NOUN_CASH_REGISTER)) {
		if (!_globals[kRegisterOpen]) {
			switch (_game._trigger) {
			case 0:
				_vm->_dialogs->show(51236);
				_game._player._stepEnabled = false;
				_game._player._facing = FACING_NORTH;
				_scene->_sequences.addTimer(15, 1);
				break;

			case 1:
				_game._player._visible = false;
				_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 9, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 3);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[8]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[8]);
				_game._player._visible = true;
				_scene->_sequences.addTimer(30, 3);
				break;

			case 3:
				_game._player._facing = FACING_NORTHEAST;
				if (!_game._objects.isInRoom(OBJ_PADLOCK_KEY) || (_game._difficulty == DIFFICULTY_EASY)) {
					_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
				} else {
					_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 3);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
				}
				_vm->_sound->command(23);
				break;

			case 4:
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
				_scene->_sequences.addTimer(60, 6);
				break;

			case 5:
				_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 14, 0, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 3);
				_scene->_hotspots.activate(NOUN_PADLOCK_KEY, true);
				_scene->_sequences.addTimer(60, 6);
				break;

			case 6:
				_globals[kRegisterOpen] = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		} else
			_vm->_dialogs->show(51239);
	} else if (_action.isAction(VERB_CLOSE, NOUN_CASH_REGISTER) && _globals[kRegisterOpen]) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 10, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			if (!_game._objects.isInRoom(OBJ_PADLOCK_KEY) || _game._difficulty == DIFFICULTY_EASY) {
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
				_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			} else {
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 3);
				_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 2:
			_globals[kRegisterOpen] = false;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_PADLOCK_KEY)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_PADLOCK_KEY)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;

				int endVal;
				if (_game._player._playerPos == Common::Point(218, 152))
					endVal = 3;
				else
					endVal = 2;

				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 10, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, endVal);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, endVal, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_game._player._playerPos == Common::Point(218, 152)) {
					_scene->_sequences.remove(_globals._sequenceIndexes[6]);
					_scene->_dynamicHotspots.remove(_keyHotspotId);
				} else {
					_scene->_sequences.remove(_globals._sequenceIndexes[5]);
					_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
					_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);
				}
				_vm->_sound->command(9);
				_game._objects.addToInventory(OBJ_PADLOCK_KEY);
				_vm->_dialogs->showItem(OBJ_PADLOCK_KEY, 51226);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(51225);
	else if (_action.isAction(VERB_LOOK, NOUN_PADLOCK_KEY) && _game._objects.isInRoom(OBJ_PADLOCK_KEY))
		_vm->_dialogs->show(51215);
	else if (_action.isAction(VERB_LOOK, NOUN_FISHING_ROD) && (!_scene->_animation[0] ||
		_scene->_animation[0]->getCurrentFrame() == 4))
		_vm->_dialogs->show(51216);
	else if (_action.isAction(VERB_LOOK, NOUN_SHIPS_WHEEL))
		_vm->_dialogs->show(51218);
	else if (_action.isAction(VERB_TAKE, NOUN_SHIPS_WHEEL))
		_vm->_dialogs->show(51219);
	else if (_action.isAction(VERB_LOOK, NOUN_PORTHOLE) || _action.isAction(VERB_PEER_THROUGH, NOUN_PORTHOLE))
		_vm->_dialogs->show(51220);
	else if (_action.isAction(VERB_LOOK, NOUN_TABLE))
		_vm->_dialogs->show(51221);
	else if (_action.isAction(VERB_LOOK, NOUN_STARFISH))
		_vm->_dialogs->show(51222);
	else if (_action.isAction(VERB_TAKE, NOUN_STARFISH))
		_vm->_dialogs->show(51223);
	else if (_action.isAction(VERB_LOOK, NOUN_OUTSIDE))
		_vm->_dialogs->show(51224);
	else if (_action.isAction(VERB_LOOK, NOUN_POSTER))
		_vm->_dialogs->show(51227);
	else if (_action.isAction(VERB_TAKE, NOUN_POSTER))
		_vm->_dialogs->show(51228);
	else if (_action.isAction(VERB_LOOK, NOUN_TROPHY)) {
		if (_game._visitedScenes.exists(604))
			_vm->_dialogs->show(51229);
		else
			_vm->_dialogs->show(51230);
	} else if (_action.isAction(VERB_LOOK, NOUN_CHAIR))
		_vm->_dialogs->show(51231);
	else if (_action.isAction(VERB_LOOK, NOUN_ROPE))
		_vm->_dialogs->show(51232);
	else if (_action.isAction(VERB_TAKE, NOUN_ROPE))
		_vm->_dialogs->show(51233);
	else if (_action.isAction(VERB_LOOK, NOUN_LAMP))
		_vm->_dialogs->show(51234);
	else if (_action.isAction(VERB_WALK_BEHIND, NOUN_COUNTER)) {
		// WORKAROUND: Empty handling to prevent default "can't do that" dialogs showing
	} else if (_action.isAction(VERB_LOOK, NOUN_COUNTER))
		_vm->_dialogs->show(51235);
	else if (_action.isAction(VERB_LOOK, NOUN_ICE_CHESTS))
		_vm->_dialogs->show(51237);
	else if (_action.isAction(VERB_OPEN, NOUN_ICE_CHESTS))
		_vm->_dialogs->show(51238);
	else if (_action.isAction(VERB_LOOK, NOUN_CASH_REGISTER)) {
		if (!_globals[kRegisterOpen])
			_vm->_dialogs->show(51212);
		else if (_game._objects.isInRoom(OBJ_PADLOCK_KEY))
			_vm->_dialogs->show(51214);
		else
			_vm->_dialogs->show(51213);
	} else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
