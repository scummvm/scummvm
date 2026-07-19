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

#include "math/utils.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section5.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _labDoorFl;
	bool _firstDoorFl;
	bool _actionFl;
	byte _heroFacing;
	int16 _doorDepth;
	int16 _doorSpriteIdx;
	int16 _doorSequenceIdx;
	int16 _doorWord;
	int16 _doorPos_x;
	int16 _doorPos_y;
};

static Scratch local;


static void handleDoorSequences() {
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;

	if (local._firstDoorFl) {
		if (_action.isAction(words_walk_into, words_software_store) || ((_scene->_priorSceneId == 507) && !local._actionFl)) {
			local._doorDepth = 13;
			local._doorSpriteIdx = _globals._spriteIndexes[2];
			local._doorSequenceIdx = _globals._sequenceIndexes[2];
			local._labDoorFl = false;
		} else {
			local._doorDepth = 10;
			local._doorSpriteIdx = _globals._spriteIndexes[1];
			local._doorSequenceIdx = _globals._sequenceIndexes[1];
			local._labDoorFl = true;
		}
		local._firstDoorFl = false;
	}

	switch (_game._trigger) {
	case 0:
	case 80:
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(local._doorSequenceIdx);
		local._doorSequenceIdx = _scene->_sequences.addSpriteCycle(local._doorSpriteIdx, false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(local._doorSequenceIdx, local._doorDepth);
		_scene->changeVariant(1);
		_scene->_sequences.addSubEntry(local._doorSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 81);
		break;


	case 81:
		local._doorSequenceIdx = _scene->_sequences.startCycle(local._doorSpriteIdx, false, -2);
		_scene->_sequences.setDepth(local._doorSequenceIdx, local._doorDepth);
		_game._player._walkAnywhere = true;
		_game._player.walk(Common::Point(local._doorPos_x, local._doorPos_y), local._heroFacing);
		_scene->_sequences.addTimer(120, 82);
		break;

	case 82:
		_scene->_sequences.remove(local._doorSequenceIdx);
		local._doorSequenceIdx = _scene->_sequences.addReverseSpriteCycle(local._doorSpriteIdx, false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(local._doorSequenceIdx, local._doorDepth);
		if (local._actionFl)
			_scene->_sequences.addSubEntry(local._doorSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 84);
		else
			_scene->_sequences.addSubEntry(local._doorSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 83);

		break;

	case 83:
	{
		local._doorSequenceIdx = _scene->_sequences.startCycle(local._doorSpriteIdx, false, 1);
		int idx = _scene->_dynamicHotspots.add(local._doorWord, words_walk_into, local._doorSequenceIdx, Common::Rect(0, 0, 0, 0));
		int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(local._doorPos_x, local._doorPos_y), FACING_NORTHWEST);
		_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_LEFT);
		_scene->_sequences.setDepth(local._doorSequenceIdx, local._doorDepth);
		local._firstDoorFl = true;
		if (local._labDoorFl) {
			_globals._spriteIndexes[1] = local._doorSpriteIdx;
			_globals._sequenceIndexes[1] = local._doorSequenceIdx;
		} else {
			_globals._spriteIndexes[2] = local._doorSpriteIdx;
			_globals._sequenceIndexes[2] = local._doorSequenceIdx;
		}
		_game._player._stepEnabled = true;

	}
	break;

	case 84:
		local._actionFl = false;
		_game._player._stepEnabled = true;
		if (local._labDoorFl)
			_scene->_nextSceneId = 508;
		else
			_scene->_nextSceneId = 507;

		break;

	default:
		break;
	}
}

static void room_506_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('q', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('q', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCD_3");

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	int idx = _scene->_dynamicHotspots.add(words_laboratory, words_walk_into, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
	int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(65, 125), FACING_NORTHWEST);
	_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_LEFT);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
	idx = _scene->_dynamicHotspots.add(words_software_store, words_walk_into, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
	hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(112, 102), FACING_NORTHWEST);
	_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_LEFT);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);

	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
	local._firstDoorFl = true;
	local._actionFl = false;

	if (_scene->_priorSceneId == 508) {
		_game._player._playerPos = Common::Point(16, 111);
		_game._player._facing = FACING_SOUTHEAST;
		_scene->_sequences.addTimer(15, 80);
		_game._player._stepEnabled = false;
	} else if (_scene->_priorSceneId == 507) {
		_game._player._playerPos = Common::Point(80, 102);
		_game._player._facing = FACING_SOUTHEAST;
		_scene->_sequences.addTimer(60, 80);
		_game._player._stepEnabled = false;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(138, 116);
		_game._player._facing = FACING_NORTHEAST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
		_scene->loadAnimation(formAnimName('R', 1), 70);
	}
	section_5_music();
}

static void room_506_daemon() {
	if (_game._trigger >= 80) {
		if (local._firstDoorFl) {
			local._heroFacing = FACING_SOUTHEAST;
			if (_scene->_priorSceneId == 507) {
				local._doorPos_x = 112;
				local._doorPos_y = 102;
				local._doorWord = 0x336;
			} else {
				local._doorPos_x = 65;
				local._doorPos_y = 125;
				local._doorWord = 0x37D;
			}
		}

		handleDoorSequences();
	}

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			_scene->_sequences.addTimer(6, 71);
			break;

		case 71:
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
			break;

		case 72:
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	}
}

static void room_506_parser() {
	if (_action.isAction(words_walk_into, words_laboratory)) {
		if (local._firstDoorFl) {
			local._heroFacing = FACING_NORTHWEST;
			local._doorPos_x = 16;
			local._doorPos_y = 111;
		}
		local._actionFl = true;
		handleDoorSequences();
	} else if (_action.isAction(words_walk_into, words_software_store)) {
		if (local._firstDoorFl) {
			local._heroFacing = FACING_NORTHWEST;
			local._doorPos_x = 80;
			local._doorPos_y = 102;
		}
		local._actionFl = true;
		handleDoorSequences();
	} else if (_action.isAction(words_get_into, words_car)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_scene->_sequences.addTimer(6, 2);
		}
		break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_nextSceneId = 504;
		}
		break;

		default:
			break;
		}
	} else if (_action._lookFlag || _action.isAction(words_look, words_street))
		_vm->_dialogs->show(50618);
	else if (_action.isAction(words_look, words_restaurant))
		_vm->_dialogs->show(50610);
	else if (_action.isAction(words_look, words_motel))
		_vm->_dialogs->show(50611);
	else if (_action.isAction(words_look, words_cycle_shop))
		_vm->_dialogs->show(50612);
	else if (_action.isAction(words_look, words_air_bike))
		_vm->_dialogs->show(50613);
	else if (_action.isAction(words_take, words_air_bike))
		_vm->_dialogs->show(50614);
	else if (_action.isAction(words_look, words_software_store))
		_vm->_dialogs->show(50615);
	else if (_action.isAction(words_look, words_laboratory))
		_vm->_dialogs->show(50616);
	else if (_action.isAction(words_look, words_street_to_west) || _action.isAction(words_walk_down, words_street_to_west))
		_vm->_dialogs->show(50617);
	else if (_action.isAction(words_look, words_software_store_sign))
		_vm->_dialogs->show(50619);
	else if (_action.isAction(words_look, words_car))
		_vm->_dialogs->show(50620);
	else if (_action.isAction(words_look, words_sky))
		_vm->_dialogs->show(50621);
	else
		return;

	_action._inProgress = false;
}

void room_506_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._doorPos_x);
	s.syncAsSint16LE(local._doorPos_y);

	s.syncAsByte(local._heroFacing);

	s.syncAsSint16LE(local._doorDepth);
	s.syncAsSint16LE(local._doorSpriteIdx);
	s.syncAsSint16LE(local._doorSequenceIdx);
	s.syncAsSint16LE(local._doorWord);

	s.syncAsByte(local._labDoorFl);
	s.syncAsByte(local._firstDoorFl);
	s.syncAsByte(local._actionFl);
}

void room_506_preload() {
	room_init_code_pointer = room_506_init;
	room_daemon_code_pointer = room_506_daemon;
	room_parser_code_pointer = room_506_parser;

	section_5_walker();
	section_5_interface();
	_scene->addActiveVocab(words_walk_into);
	_scene->addActiveVocab(words_software_store);
	_scene->addActiveVocab(words_laboratory);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
