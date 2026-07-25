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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_351_init() {
	_globals[kAfterHavoc] = -1;
	_globals[kTeleporterRoom + 1] = 351;

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('c', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*ROXRC_7");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXRD_7");

	if (object_is_here(OBJ_CREDIT_CHIP)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
	} else
		_scene->_hotspots.activate(words_credit_chip, false);

	if (_scene->_priorSceneId == 352) {
		player.x = 148;
		player.y = 152;
	}
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 207;
		player.y = 81;
		player.facing = FACING_NORTH;
	}

	if (_globals[kTeleporterCommand]) {
		player.walker_visible = false;
		player.commands_allowed = false;

		char sepChar = 'a';
		if (_globals[kSexOfRex] != REX_MALE)
			sepChar = 'b';

		int suffixNum = -1;
		int trigger = 0;

		switch (_globals[kTeleporterCommand]) {
		case 1:
			suffixNum = 0;
			trigger = 60;
			_globals[kTeleporterCommand] = true;
			break;

		case 2:
			suffixNum = 1;
			trigger = 61;
			break;

		case 3:
		case 4:
			player.walker_visible = true;
			player.commands_allowed = true;
			player.turn_to_facing = FACING_SOUTH;
			suffixNum = -1;
			break;

		default:
			break;
		}

		_globals[kTeleporterCommand] = 0;

		if (suffixNum >= 0)
			_scene->loadAnimation(kernel_name(sepChar, suffixNum), trigger);
	}

	section_3_music();
}

static void room_351_daemon() {
	if (kernel.trigger == 60) {
		player.commands_allowed = true;
		player.walker_visible = true;
		player.clock = _scene->_frameStartTime - player.frame_delay;
		player.turn_to_facing = FACING_SOUTH;
	}

	if (kernel.trigger == 61) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}
}

static void room_351_parser() {
	if (_action._lookFlag)
		text_show(35121);
	else if (player_said_2(step_into, teleporter))
		_scene->_nextSceneId = 322;
	else if (player_said_2(walk_down, corridor_to_south))
		_scene->_nextSceneId = 352;
	else if (player_said_2(take, credit_chip)) {
		if (kernel.trigger || !player_has(OBJ_CREDIT_CHIP)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE) {
					_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 6, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				_scene->_hotspots.activate(words_credit_chip, false);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				inter_give_to_player(OBJ_CREDIT_CHIP);
				break;

			case 2:
				player.walker_visible = true;
				player.commands_allowed = true;
				object_examine(OBJ_CREDIT_CHIP, 0x32F, 0);
				break;

			default:
				break;
			}
		}
	} else if (player_said_2(look, view_screen))
		text_show(35110);
	else if (player_said_2(look, rip_in_floor))
		text_show(35111);
	else if (player_said_2(look, fire_hydrant))
		text_show(35112);
	else if (player_said_2(look, guard)) {
		if (object[0xF].location == 351)
			text_show(35114);
		else
			text_show(35113);
	} else if (player_said_2(look, equipment))
		text_show(35115);
	else if (player_said_2(look, desk))
		text_show(35116);
	else if (player_said_2(look, machine))
		text_show(35117);
	else if (player_said_2(look, teleporter))
		text_show(35118);
	else if (player_said_2(look, control_panel))
		text_show(35119);
	else if (player_said_2(look, corridor_to_south))
		text_show(35120);
	else if (player_said_2(look, pole))
		text_show(35122);
	else
		return;

	_action._inProgress = false;
}

void room_351_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_351_preload() {
	room_init_code_pointer = room_351_init;
	room_parser_code_pointer = room_351_parser;
	room_daemon_code_pointer = room_351_daemon;

	if (_scene->_currentSceneId == 391)
		_globals[kSexOfRex] = REX_MALE;

	section_3_walker();
	section_3_interface();
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
