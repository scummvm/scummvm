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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section7.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _vaseHotspotId;
	int16 _vaseMode;
	int16 _animationMode;
	int16 _animationFrame;
	bool _emptyPedestral;
};

static Scratch local;

static void handleRexDeath() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->loadAnimation(formAnimName('a', -1), 2);
		break;

	case 2:
		if (local._animationMode == 1)
			_vm->_dialogs->show(70625);
		else if (_globals[kBottleStatus] < 2)
			_vm->_dialogs->show(70628);
		else
			_vm->_dialogs->show(70629);

		_game._objects.setRoom(OBJ_VASE, _scene->_currentSceneId);
		if (local._animationMode == 2)
			_game._objects.setRoom(OBJ_BOTTLE, 2);

		local._animationMode = 0;
		_scene->_reloadSceneFlag = true;
		break;

	default:
		break;
	}
}

static void handleTakeVase() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 4, 2, 0, 0);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 7, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
		_vm->_sound->command(9);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_scene->_dynamicHotspots.remove(local._vaseHotspotId);
		_game._objects.addToInventory(OBJ_VASE);
		if (local._vaseMode == 1) {
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(195, 99));
			int idx = _scene->_dynamicHotspots.add(NOUN_BOTTLE, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
			_game._objects.setRoom(OBJ_BOTTLE, _scene->_currentSceneId);
		}
		break;

	case 2:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
		_game._player._visible = true;
		_vm->_dialogs->showItem(OBJ_VASE, 70630);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void room_706_init() {
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_3");
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('b', -1));

	if (!_game._visitedScenes._sceneRevisited)
		local._emptyPedestral = false;

	if (_game._objects[OBJ_VASE]._roomNumber == _scene->_currentSceneId) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('v', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
		int idx = _scene->_dynamicHotspots.add(NOUN_VASE, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		local._vaseHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
	} else if (_game._objects.isInRoom(OBJ_BOTTLE)) {
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(195, 99));
		int idx = _scene->_dynamicHotspots.add(NOUN_BOTTLE, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
	}

	_game._player._visible = true;

	if (_scene->_priorSceneId == 707) {
		_game._player._playerPos = Common::Point(277, 103);
		_game._player._facing = FACING_SOUTHWEST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(167, 152);
		_game._player._facing = FACING_NORTH;
	}

	if (_globals[kTeleporterCommand]) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;

		switch (_globals[kTeleporterCommand]) {
		case 1:
			_scene->loadAnimation(formAnimName('E', 1), 75);
			break;

		case 2:
			_scene->loadAnimation(formAnimName('E', -1), 80);
			break;

		default:
			_game._player.walk(Common::Point(264, 116), FACING_SOUTHWEST);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		}
		_globals[kTeleporterCommand] = 0;
	}

	local._animationMode = 0;

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_BOTTLE);
		_globals[kBottleStatus] = 2;
	}

	section_7_music();
}

static void room_706_daemon() {
	if (_game._trigger == 75) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player.walk(Common::Point(264, 116), FACING_SOUTHWEST);
	}

	if (_game._trigger == 80) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}

	if (_scene->_animation[0] != nullptr) {
		if ((local._animationMode != 0) && (_scene->_animation[0]->getCurrentFrame() != local._animationFrame)) {
			local._animationFrame = _scene->_animation[0]->getCurrentFrame();

			if (local._animationFrame == 6) {
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_game._objects.setRoom(OBJ_VASE, 2);

				if (local._animationMode == 2) {
					_game._objects.setRoom(OBJ_BOTTLE, 1);

					_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
					_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(195, 99));
					int idx = _scene->_dynamicHotspots.add(NOUN_BOTTLE, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
					_scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
				}
			}
		}
	}
}

static void room_706_pre_parser() {
	if (_action.isAction(VERB_LOOK, NOUN_PORTRAIT))
		_game._player._needToWalk = true;
}

static void room_706_parser() {
	if (_action.isAction(VERB_WALK_INSIDE, NOUN_TELEPORTER)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 707;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT, NOUN_ROOM)) {
		_scene->_nextSceneId = 705;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_VASE)) {
		if (_game._difficulty != DIFFICULTY_EASY) {
			local._animationMode = 1;
			handleRexDeath();
		} else if (_game._trigger || !_game._objects.isInInventory(OBJ_VASE)) {
			handleTakeVase();
			local._emptyPedestral = true;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, NOUN_BOTTLE, NOUN_PEDESTAL)) {
		if ((_globals[kBottleStatus] == 2 && _game._difficulty == DIFFICULTY_HARD) ||
			(_globals[kBottleStatus] != 0 && _game._difficulty != DIFFICULTY_HARD)) {
			if (!_game._objects.isInInventory(OBJ_VASE) || _game._trigger) {
				local._vaseMode = 1;
				handleTakeVase();
				_action._inProgress = false;
				return;
			}
		} else if (_game._objects.isInRoom(OBJ_VASE) || _game._trigger) {
			local._animationMode = 2;
			handleRexDeath();
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_PUT, NOUN_PEDESTAL) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		int objectId = _game._objects.getIdFromDesc(_action._activeAction._objectNameId);
		if (_game._objects[objectId].hasQuality(10))
			_vm->_dialogs->show(70626);
		else
			_vm->_dialogs->show(70627);
	} else if (_action.isAction(VERB_TAKE, NOUN_BOTTLE) && _game._objects.isInInventory(OBJ_VASE))
		_vm->_dialogs->show(70631);
	else if (_action._lookFlag) {
		if (_game._objects[OBJ_VASE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(70610);
		else
			_vm->_dialogs->show(70611);
	} else if (_action.isAction(VERB_LOOK, NOUN_FLOOR))
		_vm->_dialogs->show(70612);
	else if (_action.isAction(VERB_LOOK, NOUN_PILLAR))
		_vm->_dialogs->show(70613);
	else if (_action.isAction(VERB_LOOK, NOUN_OLD_TEA_CUP))
		_vm->_dialogs->show(70614);
	else if (_action.isAction(VERB_TAKE, NOUN_OLD_TEA_CUP))
		_vm->_dialogs->show(70615);
	else if (_action.isAction(VERB_LOOK, NOUN_OLD_VASE))
		_vm->_dialogs->show(70616);
	else if (_action.isAction(VERB_LOOK, NOUN_PORTRAIT))
		_vm->_dialogs->show(70617);
	else if (_action.isAction(VERB_LOOK, NOUN_NAME_PLATE))
		_vm->_dialogs->show(70618);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(70619);
	else if (_action.isAction(VERB_LOOK, NOUN_PEDESTAL)) {
		if (_game._objects[OBJ_VASE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(70620);
		else if (_game._objects[OBJ_BOTTLE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(70622);
		else
			_vm->_dialogs->show(70621);
	} else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER))
		_vm->_dialogs->show(70623);
	else if (_action.isAction(VERB_LOOK, NOUN_VASE) && (_game._objects[OBJ_VASE]._roomNumber == _scene->_currentSceneId))
		_vm->_dialogs->show(70624);
	else if (_action.isAction(VERB_LOOK, NOUN_BOTTLE) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(70632);
	else
		return;

	_action._inProgress = false;
}

void room_706_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._vaseHotspotId);
	s.syncAsSint16LE(local._vaseMode);
	s.syncAsSint16LE(local._animationMode);
	s.syncAsSint16LE(local._animationFrame);

	s.syncAsByte(local._emptyPedestral);
}

void room_706_preload() {
	room_init_code_pointer = room_706_init;
	room_daemon_code_pointer = room_706_daemon;
	room_pre_parser_code_pointer = room_706_pre_parser;
	room_parser_code_pointer = room_706_parser;

	section_7_walker();
	section_7_interface();
	_scene->addActiveVocab(NOUN_BOTTLE);
	_scene->addActiveVocab(NOUN_VASE);
	_scene->addActiveVocab(VERB_WALKTO);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
