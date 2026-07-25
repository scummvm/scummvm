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
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _fishingRodHotspotId;
	int16 _keyHotspotId;
};

static Scratch local;


static void room_512_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('r', -1), 0);
	g_sprite_ids[2] = kernel_load_series("*RXMRC_9", 0);
	g_sprite_ids[8] = kernel_load_series("*RXMRC_8", 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('x', 2), 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('x', 3), 0);

	if (object[OBJ_FISHING_ROD].location == room_id) {
		g_sequence_ids[1] = _scene->_sequences.addSpriteCycle(g_sprite_ids[1], false, 9, 0, 0, 0);
		local._fishingRodHotspotId = _scene->_dynamicHotspots.add(words_fishing_rod, words_walkto, g_sequence_ids[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._fishingRodHotspotId, Common::Point(199, 101), FACING_NORTHEAST);
	}

	if (!player.been_here_before)
		global[kRegisterOpen] = false;

	_scene->_hotspots.activate(words_padlock_key, false);
	if (game.difficulty == DIFFICULTY_EASY) {
		if (object[OBJ_PADLOCK_KEY].location == room_id) {
			g_sequence_ids[6] = _scene->_sequences.addSpriteCycle(g_sprite_ids[6], false, 10, 0, 0, 0);
			_scene->_sequences.setDepth(g_sequence_ids[6], 3);
			local._keyHotspotId = _scene->_dynamicHotspots.add(words_padlock_key, words_walkto, g_sequence_ids[6], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(local._keyHotspotId, Common::Point(218, 152), FACING_NORTHEAST);
		}
		if (global[kRegisterOpen]) {
			g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, -2);
			_scene->_sequences.setDepth(g_sequence_ids[3], 3);
			_scene->_hotspots.activate(words_padlock_key, false);
		}
	} else if (global[kRegisterOpen]) {
		if (object[OBJ_PADLOCK_KEY].location == room_id) {
			_scene->_hotspots.activate(words_padlock_key, true);
			g_sequence_ids[5] = _scene->_sequences.startCycle(g_sprite_ids[5], false, -2);
			_scene->_sequences.setDepth(g_sequence_ids[5], 3);
		} else {
			_scene->_hotspots.activate(words_padlock_key, false);
			g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, -2);
			_scene->_sequences.setDepth(g_sequence_ids[3], 3);
		}
	} else
		_scene->_hotspots.activate(words_padlock_key, false);

	if (previous_room != RETURNING_FROM_DIALOG) {
		player.x = 144;
		player.y = 152;
		player.facing = FACING_NORTHEAST;
	}

	section_5_music();
}

static void room_512_parser() {
	if (player_said_2(walk, outside))
		new_room = 511;
	else if (player_said_2(take, fishing_rod)) {
		if (kernel.trigger || !player_has(OBJ_FISHING_ROD)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sequence_ids[2] = _scene->_sequences.startPingPongCycle(g_sprite_ids[2], false, 8, 1, 0, 0);
				_scene->_sequences.setMsgLayout(g_sequence_ids[2]);
				_scene->_sequences.addSubEntry(g_sequence_ids[2], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(g_sequence_ids[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				g_engine->_soundManager->command(9, 0);
				_scene->_sequences.remove(g_sequence_ids[1]);
				_scene->_dynamicHotspots.remove(local._fishingRodHotspotId);
				inter_give_to_player(OBJ_FISHING_ROD);
				object_examine(OBJ_FISHING_ROD, 51217, 0);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, g_sequence_ids[2]);
				player.walker_visible = true;
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		}
	} else if (player_said_2(open, cash_register)) {
		if (!global[kRegisterOpen]) {
			switch (kernel.trigger) {
			case 0:
				text_show(51236);
				player.commands_allowed = false;
				player.facing = FACING_NORTH;
				_scene->_sequences.addTimer(15, 1);
				break;

			case 1:
				player.walker_visible = false;
				g_sequence_ids[8] = _scene->_sequences.startPingPongCycle(g_sprite_ids[8], false, 9, 1, 0, 0);
				_scene->_sequences.setAnimRange(g_sequence_ids[8], 1, 3);
				_scene->_sequences.setMsgLayout(g_sequence_ids[8]);
				_scene->_sequences.addSubEntry(g_sequence_ids[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, g_sequence_ids[8]);
				player.walker_visible = true;
				_scene->_sequences.addTimer(30, 3);
				break;

			case 3:
				player.facing = FACING_NORTHEAST;
				if (!object_is_here(OBJ_PADLOCK_KEY) || (game.difficulty == DIFFICULTY_EASY)) {
					g_sequence_ids[3] = _scene->_sequences.addSpriteCycle(g_sprite_ids[3], false, 12, 1, 0, 0);
					_scene->_sequences.setDepth(g_sequence_ids[3], 3);
					_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
				} else {
					g_sequence_ids[4] = _scene->_sequences.addSpriteCycle(g_sprite_ids[4], false, 12, 1, 0, 0);
					_scene->_sequences.setDepth(g_sequence_ids[4], 3);
					_scene->_sequences.addSubEntry(g_sequence_ids[4], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
				}
				g_engine->_soundManager->command(23, 0);
				break;

			case 4:
				g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, -2);
				_scene->_sequences.setDepth(g_sequence_ids[3], 3);
				_scene->_sequences.addTimer(60, 6);
				break;

			case 5:
				g_sequence_ids[5] = _scene->_sequences.startPingPongCycle(g_sprite_ids[5], false, 14, 0, 0, 0);
				_scene->_sequences.setDepth(g_sequence_ids[5], 3);
				_scene->_hotspots.activate(words_padlock_key, true);
				_scene->_sequences.addTimer(60, 6);
				break;

			case 6:
				global[kRegisterOpen] = true;
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		} else
			text_show(51239);
	} else if (player_said_2(close, cash_register) && global[kRegisterOpen]) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[2] = _scene->_sequences.startPingPongCycle(g_sprite_ids[2], false, 10, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[2], 1, 2);
			_scene->_sequences.setMsgLayout(g_sequence_ids[2]);
			_scene->_sequences.addSubEntry(g_sequence_ids[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, g_sequence_ids[2]);
			player.walker_visible = true;
			if (!object_is_here(OBJ_PADLOCK_KEY) || game.difficulty == DIFFICULTY_EASY) {
				_scene->_sequences.remove(g_sequence_ids[3]);
				g_sequence_ids[3] = _scene->_sequences.startPingPongCycle(g_sprite_ids[3], false, 12, 1, 0, 0);
				_scene->_sequences.setDepth(g_sequence_ids[3], 3);
				_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			} else {
				_scene->_sequences.remove(g_sequence_ids[5]);
				g_sequence_ids[4] = _scene->_sequences.startPingPongCycle(g_sprite_ids[4], false, 12, 1, 0, 0);
				_scene->_sequences.setDepth(g_sequence_ids[4], 3);
				_scene->_hotspots.activate(words_padlock_key, false);
				_scene->_sequences.addSubEntry(g_sequence_ids[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 2:
			global[kRegisterOpen] = false;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(take, padlock_key)) {
		if (kernel.trigger || !player_has(OBJ_PADLOCK_KEY)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;

				int endVal;
				if (Common::Point(player.x, player.y) == Common::Point(218, 152))
					endVal = 3;
				else
					endVal = 2;

				g_sequence_ids[2] = _scene->_sequences.startPingPongCycle(g_sprite_ids[2], false, 10, 1, 0, 0);
				_scene->_sequences.setAnimRange(g_sequence_ids[2], 1, endVal);
				_scene->_sequences.setMsgLayout(g_sequence_ids[2]);
				_scene->_sequences.addSubEntry(g_sequence_ids[2], SEQUENCE_TRIGGER_SPRITE, endVal, 1);
				_scene->_sequences.addSubEntry(g_sequence_ids[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (Common::Point(player.x, player.y) == Common::Point(218, 152)) {
					_scene->_sequences.remove(g_sequence_ids[6]);
					_scene->_dynamicHotspots.remove(local._keyHotspotId);
				} else {
					_scene->_sequences.remove(g_sequence_ids[5]);
					g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, -2);
					_scene->_sequences.setDepth(g_sequence_ids[3], 3);
					_scene->_hotspots.activate(words_padlock_key, false);
				}
				g_engine->_soundManager->command(9, 0);
				inter_give_to_player(OBJ_PADLOCK_KEY);
				object_examine(OBJ_PADLOCK_KEY, 51226, 0);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, g_sequence_ids[2]);
				player.walker_visible = true;
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		}
	} else if (player.look_around)
		text_show(51225);
	else if (player_said_2(look, padlock_key) && object_is_here(OBJ_PADLOCK_KEY))
		text_show(51215);
	else if (player_said_2(look, fishing_rod) && (!_scene->_animation[0] ||
		_scene->_animation[0]->getCurrentFrame() == 4))
		text_show(51216);
	else if (player_said_2(look, ships_wheel))
		text_show(51218);
	else if (player_said_2(take, ships_wheel))
		text_show(51219);
	else if (player_said_2(look, porthole) || player_said_2(peer_through, porthole))
		text_show(51220);
	else if (player_said_2(look, table))
		text_show(51221);
	else if (player_said_2(look, starfish))
		text_show(51222);
	else if (player_said_2(take, starfish))
		text_show(51223);
	else if (player_said_2(look, outside))
		text_show(51224);
	else if (player_said_2(look, poster))
		text_show(51227);
	else if (player_said_2(take, poster))
		text_show(51228);
	else if (player_said_2(look, trophy)) {
		if (player_has_been_in_room(604))
			text_show(51229);
		else
			text_show(51230);
	} else if (player_said_2(look, chair))
		text_show(51231);
	else if (player_said_2(look, rope))
		text_show(51232);
	else if (player_said_2(take, rope))
		text_show(51233);
	else if (player_said_2(look, lamp))
		text_show(51234);
	else if (player_said_2(walk_behind, counter)) {
		// WORKAROUND: Empty handling to prevent default "can't do that" dialogs showing
	} else if (player_said_2(look, counter))
		text_show(51235);
	else if (player_said_2(look, ice_chests))
		text_show(51237);
	else if (player_said_2(open, ice_chests))
		text_show(51238);
	else if (player_said_2(look, cash_register)) {
		if (!global[kRegisterOpen])
			text_show(51212);
		else if (object_is_here(OBJ_PADLOCK_KEY))
			text_show(51214);
		else
			text_show(51213);
	} else
		return;

	player.command_ready = false;
}

void room_512_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._fishingRodHotspotId);
	s.syncAsSint16LE(local._keyHotspotId);
}

void room_512_preload() {
	room_init_code_pointer = room_512_init;
	room_parser_code_pointer = room_512_parser;

	section_5_walker();
	section_5_interface();
	vocab_make_active(words_fishing_rod);
	vocab_make_active(words_walkto);
	vocab_make_active(words_padlock_key);
	vocab_make_active(words_register_drawer);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
