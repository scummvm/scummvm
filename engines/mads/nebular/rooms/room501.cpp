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
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section5.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _mainSequenceId;
	int32 _mainSpriteId;
	int32 _doorHotspotid;
	bool _rexPunched;
};

static Scratch local;

static void handleSlotActions() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		int numTicks, frameIndex;
		if (_globals[kSexOfRex] == REX_MALE) {
			local._mainSpriteId = _globals._spriteIndexes[4];
			numTicks = 8;
			frameIndex = 3;
		} else {
			local._mainSpriteId = _globals._spriteIndexes[5];
			numTicks = 10;
			frameIndex = 2;
		}

		local._mainSequenceId = _scene->_sequences.startPingPongCycle(local._mainSpriteId, false, numTicks, 1, 0, 0);
		_scene->_sequences.setAnimRange(local._mainSequenceId, 1, frameIndex);
		_scene->_sequences.setMsgLayout(local._mainSequenceId);
		_vm->_sound->command(10);
		_scene->_sequences.addSubEntry(local._mainSequenceId, SEQUENCE_TRIGGER_SPRITE, frameIndex, 1);
		_scene->_sequences.addSubEntry(local._mainSequenceId, SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 12, 6, 0, 0);
		break;

	case 2:
		_scene->_sequences.updateTimeout(-1, local._mainSequenceId);
		_game._player._visible = true;
		_scene->_sequences.addTimer(15, 3);
		break;

	case 3:
		_game._player.walk(Common::Point(282, 110), FACING_NORTH);
		_scene->_sequences.addTimer(60, 4);
		break;

	default:
		break;
	}
}

static void room_501_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));

	if (_globals[kSexOfRex] == REX_MALE) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXMRC_9");
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*RXCD_7");
	} else {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*ROXRC_9");
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*ROXCD_7");
	}

	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	int idx = _scene->_dynamicHotspots.add(words_door, words_walk_through, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
	local._doorHotspotid = _scene->_dynamicHotspots.setPosition(idx, Common::Point(282, 110), FACING_NORTH);
	_scene->_dynamicHotspots.setCursor(local._doorHotspotid, CURSOR_GO_UP);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
	local._rexPunched = true;

	if (_scene->_priorSceneId == 504) {
		_game._player._stepEnabled = false;
		_game._player._playerPos = Common::Point(74, 121);
		_game._player._facing = FACING_NORTHWEST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
		if (_globals[kSexOfRex] == REX_MALE)
			_scene->loadAnimation(formAnimName('G', 2), 70);
		else
			_scene->loadAnimation(formAnimName('R', 2), 70);
	} else if (_scene->_priorSceneId == 503) {
		_game._player._playerPos = Common::Point(317, 102);
		_game._player._facing = FACING_SOUTHWEST;
		_scene->_sequences.addTimer(15, 80);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(299, 131);

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_FAKE_ID);
		_game._objects.addToInventory(OBJ_SECURITY_CARD);
		_game._objects.addToInventory(OBJ_ID_CARD);
	}

	section_5_music();
	_game.loadQuoteSet(0x275, 0x276, 0x277, 0);

	if (!_game._visitedScenes._sceneRevisited)
		_scene->_sequences.addTimer(2, 90);
}

static void room_501_daemon() {
	if (_game._trigger == 90)
		_vm->_dialogs->show(50127);

	if (_game._trigger >= 80) {
		switch (_game._trigger) {
		case 80:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 12, 6, 0, 0);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_vm->_sound->command(11);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
			break;

		case 81:
			_scene->_dynamicHotspots.remove(local._doorHotspotid);
			_game._player.walk(Common::Point(276, 110), FACING_SOUTHWEST);
			_scene->_sequences.addTimer(120, 82);
			break;

		case 82:
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_vm->_sound->command(12);
			local._doorHotspotid = _scene->_dynamicHotspots.add(words_door, words_walk_through, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(_globals._sequenceIndexes[3], Common::Point(282, 110), FACING_NORTH);
			_scene->_dynamicHotspots.setCursor(local._doorHotspotid, CURSOR_GO_UP);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 83);
			break;

		case 83:
			_game._player._stepEnabled = true;
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			break;

		default:
			break;
		}
	}

	if (_game._trigger >= 70 && _game._trigger <= 73) {
		switch (_game._trigger) {
		case 70:
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
			_scene->_sequences.addTimer(15, 71);
			break;

		case 71:
			_game._player.walk(Common::Point(92, 130), FACING_SOUTH);
			_scene->_sequences.addTimer(30, 72);
			break;

		case 72:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
			break;

		case 73:
			_game._player._stepEnabled = true;
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			break;

		default:
			break;
		}
	}
}

static void room_501_pre_parser() {
	if (player_said_1(walk_down) && (player_said_1(street_to_east) || player_said_1(sidewalk_to_east)))
		_game._player._walkOffScreenSceneId = 551;
}

static void room_501_parser() {
	if (player_said_2(get_into, car)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
			_scene->_sequences.addTimer(15, 2);
		}
		break;

		case 2:
			_game._player._visible = false;
			if (_globals[kSexOfRex] == REX_MALE)
				local._mainSpriteId = _globals._spriteIndexes[6];
			else
				local._mainSpriteId = _globals._spriteIndexes[7];

			local._mainSequenceId = _scene->_sequences.addSpriteCycle(local._mainSpriteId, false, 8, 1, 0, 0);
			_scene->_sequences.setMsgLayout(local._mainSequenceId);
			_scene->_sequences.addSubEntry(local._mainSequenceId, SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = local._mainSequenceId;
			local._mainSequenceId = _scene->_sequences.startCycle(local._mainSpriteId, false, -2);
			_scene->_sequences.setMsgLayout(local._mainSequenceId);
			_scene->_sequences.updateTimeout(local._mainSequenceId, syncIdx);
			_scene->_sequences.addTimer(30, 4);
		}
		break;

		case 4:
			_scene->_nextSceneId = 504;
			break;

		default:
			break;
		}
	} else if (player_said_3(put, security_card, card_slot))
		_vm->_dialogs->show(50113);
	else if (player_said_3(put, fake_id, card_slot)) {
		switch (_game._trigger) {
		case 0:
		case 1:
		case 2:
		case 3:
			handleSlotActions();
			break;

		case 4:
			if (_globals[kSexOfRex] == REX_MALE) {
				_game._player._visible = false;
				_vm->_sound->command(13);
				_scene->loadAnimation(formAnimName('G', 1), 5);
			} else {
				local._rexPunched = false;
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 6, 120, _game.getQuote(0x277));
			}
			break;

		case 5:
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
			_scene->_sequences.addTimer(30, 6);
			break;

		case 6:
			if (_globals[kSexOfRex] == REX_MALE) {
				if (local._rexPunched) {
					_scene->_kernelMessages.reset();
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x275));
					local._rexPunched = false;
				} else {
					_scene->_kernelMessages.reset();
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x276));
				}
			}
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (player_said_3(put, id_card, card_slot)) {
		switch (_game._trigger) {
		case 0:
		case 1:
		case 2:
		case 3:
			handleSlotActions();
			break;

		case 4:
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_scene->_dynamicHotspots.remove(local._doorHotspotid);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_vm->_sound->command(11);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6:
			_game._player.walk(Common::Point(317, 102), FACING_NORTHEAST);
			_scene->_sequences.addTimer(120, 7);
			break;

		case 7:
		{
			_vm->_sound->command(12);
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_vm->_sound->command(12);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
		}
		break;

		case 8:
		{
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_scene->_nextSceneId = 503;
		}
		break;

		default:
			break;
		}
	} else if (_action._lookFlag || player_said_2(look, street))
		_vm->_dialogs->show(50121);
	else if (player_said_2(look, door))
		_vm->_dialogs->show(50110);
	else if (player_said_2(look, card_slot))
		_vm->_dialogs->show(50112);
	else if (player_said_2(look, sign))
		_vm->_dialogs->show(50114);
	else if (player_said_2(take, sign))
		_vm->_dialogs->show(50115);
	else if (player_said_2(look, sidewalk_to_east))
		_vm->_dialogs->show(50118);
	else if (player_said_2(look, sidewalk_to_west) || player_said_2(look, street_to_west)
		|| player_said_2(walk_down, sidewalk_to_west) || player_said_2(walk_down, street_to_west))
		_vm->_dialogs->show(50119);
	else if (player_said_2(look, building))
		_vm->_dialogs->show(50120);
	else if (player_said_2(open, door))
		_vm->_dialogs->show(50122);
	else if (player_said_2(look, fire_hydrant))
		_vm->_dialogs->show(50123);
	else if (player_said_2(open, fire_hydrant))
		_vm->_dialogs->show(50124);
	else if (player_said_2(look, equipment_overhead))
		_vm->_dialogs->show(50125);
	else if (player_said_2(look, pipes) || player_said_2(look, pipe))
		_vm->_dialogs->show(50126);
	else if (player_said_2(look, car)) {
		if (!_game._visitedScenes.exists(504))
			_vm->_dialogs->show(50116);
		else
			_vm->_dialogs->show(50117);
	} else
		return;

	_action._inProgress = false;
}

void room_501_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._mainSequenceId);
	s.syncAsSint16LE(local._mainSpriteId);
	s.syncAsSint16LE(local._doorHotspotid);
	s.syncAsByte(local._rexPunched);
}

void room_501_preload() {
	room_init_code_pointer = room_501_init;
	room_daemon_code_pointer = room_501_daemon;
	room_pre_parser_code_pointer = room_501_pre_parser;
	room_parser_code_pointer = room_501_parser;

	section_5_walker();
	section_5_interface();
	_scene->addActiveVocab(words_door);
	_scene->addActiveVocab(words_walk_through);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
