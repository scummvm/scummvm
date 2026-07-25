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
#include "mads/nebular/rooms/section7.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
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
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		player.walker_visible = false;
		_scene->loadAnimation(kernel_name('a', -1), 2);
		break;

	case 2:
		if (local._animationMode == 1)
			text_show(70625);
		else if (_globals[kBottleStatus] < 2)
			text_show(70628);
		else
			text_show(70629);

		inter_move_object(OBJ_VASE, _scene->_currentSceneId);
		if (local._animationMode == 2)
			inter_move_object(OBJ_BOTTLE, 2);

		local._animationMode = 0;
		_scene->_reloadSceneFlag = true;
		break;

	default:
		break;
	}
}

static void handleTakeVase() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		player.walker_visible = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 4, 2, 0, 0);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 7, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
		g_engine->_soundManager->command(9, 0);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_scene->_dynamicHotspots.remove(local._vaseHotspotId);
		inter_give_to_player(OBJ_VASE);
		if (local._vaseMode == 1) {
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(195, 99));
			int idx = _scene->_dynamicHotspots.add(words_bottle, words_walkto, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
			inter_move_object(OBJ_BOTTLE, _scene->_currentSceneId);
		}
		break;

	case 2:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
		player.walker_visible = true;
		object_examine(OBJ_VASE, 70630, 0);
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void room_706_init() {
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_3");
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(kernel_name('b', -1));

	if (!player.been_here_before)
		local._emptyPedestral = false;

	if (object[OBJ_VASE].location == _scene->_currentSceneId) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('v', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
		int idx = _scene->_dynamicHotspots.add(words_vase, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		local._vaseHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
	} else if (object_is_here(OBJ_BOTTLE)) {
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(195, 99));
		int idx = _scene->_dynamicHotspots.add(words_bottle, words_walkto, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
	}

	player.walker_visible = true;

	if (_scene->_priorSceneId == 707) {
		player.x = 277;
		player.y = 103;
		player.facing = FACING_SOUTHWEST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 167;
		player.y = 152;
		player.facing = FACING_NORTH;
	}

	if (_globals[kTeleporterCommand]) {
		player.walker_visible = false;
		player.commands_allowed = false;

		switch (_globals[kTeleporterCommand]) {
		case 1:
			_scene->loadAnimation(kernel_name('E', 1), 75);
			break;

		case 2:
			_scene->loadAnimation(kernel_name('E', -1), 80);
			break;

		default:
			player_walk(264, 116, FACING_SOUTHWEST);
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		}
		_globals[kTeleporterCommand] = 0;
	}

	local._animationMode = 0;

	if (_scene->_roomChanged) {
		inter_give_to_player(OBJ_BOTTLE);
		_globals[kBottleStatus] = 2;
	}

	section_7_music();
}

static void room_706_daemon() {
	if (kernel.trigger == 75) {
		player.commands_allowed = true;
		player.walker_visible = true;
		player.clock = _scene->_frameStartTime - player.frame_delay;
		player_walk(264, 116, FACING_SOUTHWEST);
	}

	if (kernel.trigger == 80) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}

	if (_scene->_animation[0] != nullptr) {
		if ((local._animationMode != 0) && (_scene->_animation[0]->getCurrentFrame() != local._animationFrame)) {
			local._animationFrame = _scene->_animation[0]->getCurrentFrame();

			if (local._animationFrame == 6) {
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				inter_move_object(OBJ_VASE, 2);

				if (local._animationMode == 2) {
					inter_move_object(OBJ_BOTTLE, 1);

					_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
					_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(195, 99));
					int idx = _scene->_dynamicHotspots.add(words_bottle, words_walkto, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
					_scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
				}
			}
		}
	}
}

static void room_706_pre_parser() {
	if (player_said_2(look, portrait))
		player.need_to_walk = true;
}

static void room_706_parser() {
	if (player_said_2(walk_inside, teleporter)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		_scene->_nextSceneId = 707;
		player.command_ready = false;
		return;
	}

	if (player_said_2(exit, room)) {
		_scene->_nextSceneId = 705;
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, vase)) {
		if (game.difficulty != DIFFICULTY_EASY) {
			local._animationMode = 1;
			handleRexDeath();
		} else if (kernel.trigger || !player_has(OBJ_VASE)) {
			handleTakeVase();
			local._emptyPedestral = true;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_3(put, bottle, pedestal)) {
		if ((_globals[kBottleStatus] == 2 && game.difficulty == DIFFICULTY_HARD) ||
			(_globals[kBottleStatus] != 0 && game.difficulty != DIFFICULTY_HARD)) {
			if (!player_has(OBJ_VASE) || kernel.trigger) {
				local._vaseMode = 1;
				handleTakeVase();
				player.command_ready = false;
				return;
			}
		} else if (object_is_here(OBJ_VASE) || kernel.trigger) {
			local._animationMode = 2;
			handleRexDeath();
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(put, pedestal) && player_has(object_named(player2.words[1]))) {
		int objectId = object_named(player2.words[1]);
		if (object_has_quality(objectId, 10))
			text_show(70626);
		else
			text_show(70627);
	} else if (player_said_2(take, bottle) && player_has(OBJ_VASE))
		text_show(70631);
	else if (player.look_around) {
		if (object[OBJ_VASE].location == _scene->_currentSceneId)
			text_show(70610);
		else
			text_show(70611);
	} else if (player_said_2(look, floor))
		text_show(70612);
	else if (player_said_2(look, pillar))
		text_show(70613);
	else if (player_said_2(look, old_tea_cup))
		text_show(70614);
	else if (player_said_2(take, old_tea_cup))
		text_show(70615);
	else if (player_said_2(look, old_vase))
		text_show(70616);
	else if (player_said_2(look, portrait))
		text_show(70617);
	else if (player_said_2(look, name_plate))
		text_show(70618);
	else if (player_said_2(look, wall))
		text_show(70619);
	else if (player_said_2(look, pedestal)) {
		if (object[OBJ_VASE].location == _scene->_currentSceneId)
			text_show(70620);
		else if (object[OBJ_BOTTLE].location == _scene->_currentSceneId)
			text_show(70622);
		else
			text_show(70621);
	} else if (player_said_2(look, teleporter))
		text_show(70623);
	else if (player_said_2(look, vase) && (object[OBJ_VASE].location == _scene->_currentSceneId))
		text_show(70624);
	else if (player_said_2(look, bottle) && (player.main_object_source == CAT_HOTSPOT))
		text_show(70632);
	else
		return;

	player.command_ready = false;
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
	_scene->addActiveVocab(words_bottle);
	_scene->addActiveVocab(words_vase);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
