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

static void room_513_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_9");
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXMRC_9");

	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

	if ((_scene->_priorSceneId == 751) || (_scene->_priorSceneId == 701)) {
		_game._player._playerPos = Common::Point(296, 147);
		_game._player._facing = FACING_WEST;
		_game._player._stepEnabled = false;
		_scene->_sequences.addTimer(15, 80);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(63, 149);
		_game._player._facing = FACING_NORTHEAST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->loadAnimation(formAnimName('R', 1), 70);
	}

	section_5_music();

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_SECURITY_CARD);

	_game.loadQuoteSet(0x278, 0);
}

static void room_513_daemon() {
	switch (_game._trigger) {
	case 80:
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
		_vm->_sound->command(24);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
		break;

	case 81:
		_game._player.walk(Common::Point(265, 152), FACING_WEST);
		_scene->_sequences.addTimer(120, 82);
		break;

	case 82:
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
		_vm->_sound->command(25);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 83);
		break;

	case 83:
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 70:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(6, 71);
		break;

	case 71:
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void room_513_parser() {
	if (player_said_2(get_into, car)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
			_scene->_sequences.addTimer(6, 2);
		}
		break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_scene->_nextSceneId = 504;
		}
		break;

		default:
			break;
		}
	} else if (player_said_3(put, id_card, card_slot) || player_said_3(put, fake_id, card_slot)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[4]);
			_game._player._visible = true;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_vm->_sound->command(24);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x278));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			_game._player.walk(Common::Point(296, 147), FACING_WEST);
			_scene->_sequences.addTimer(120, 3);
			break;

		case 3:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_vm->_sound->command(25);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_game._player._stepEnabled = true;
			if (_globals[kCityFlooded])
				_scene->_nextSceneId = 701;
			else
				_scene->_nextSceneId = 751;

			break;

		default:
			break;
		}
	} else if ((_action._lookFlag) || player_said_2(look, street))
		_vm->_dialogs->show(51318);
	else if (player_said_2(look, elevator))
		_vm->_dialogs->show(51310);
	else if (player_said_2(look, elevator_door))
		_vm->_dialogs->show(51311);
	else if (player_said_2(look, card_slot))
		_vm->_dialogs->show(51312);
	else if (player_said_2(look, handicap_sign))
		_vm->_dialogs->show(51313);
	else if (player_said_2(look, bike_rack))
		_vm->_dialogs->show(51314);
	else if (player_said_2(look, building))
		_vm->_dialogs->show(51315);
	else if (player_said_2(look, sign))
		_vm->_dialogs->show(51316);
	else if (player_said_2(look, street_to_west) || player_said_2(walk_down, street_to_west))
		_vm->_dialogs->show(51317);
	else if (player_said_2(open, elevator_door) || player_said_2(open, elevator))
		_vm->_dialogs->show(51319);
	else if (player_said_2(look, car))
		_vm->_dialogs->show(51321);
	else if (player_said_2(look, brick_wall))
		_vm->_dialogs->show(51322);
	else if (player_said_3(put, security_card, card_slot))
		_vm->_dialogs->show(51320);
	else
		return;

	_action._inProgress = false;
}

void room_513_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_513_preload() {
	room_init_code_pointer = room_513_init;
	room_daemon_code_pointer = room_513_daemon;
	room_parser_code_pointer = room_513_parser;

	section_5_walker();
	section_5_interface();
	_scene->addActiveVocab(words_elevator_door);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
