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

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _cardId;
};

static Scratch local;


static void room_752_init() {
	g_sprite_ids[14] = kernel_load_series(kernel_name('l', -1), 0);
	g_sprite_ids[12] = kernel_load_series("*RXMBD_8", 0);

	if (previous_room == 751) {
		player.x = 13;
		player.y = 145;
		player.facing = FACING_EAST;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 289;
		player.y = 138;
		player_walk(262, 148, FACING_WEST);
		player.facing = FACING_WEST;
		player.walker_visible = true;
	}

	if (object[OBJ_ID_CARD].location == 752) {
		g_sprite_ids[13] = kernel_load_series(kernel_name('i', -1), 0);
		g_sequence_ids[13] = kernel_seq_stamp(g_sprite_ids[13], false, 1);
		kernel_seq_depth(g_sequence_ids[13], 8);
		int idx = kernel_add_dynamic(words_id_card, words_walkto, 0, g_sequence_ids[13], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 234, 135, FACING_NORTH);
		local._cardId = idx;
	}

	if (global[kLaserHoleIsThere]) {
		g_sequence_ids[14] = kernel_seq_stamp(g_sprite_ids[14], false, 1);
		kernel_seq_depth(g_sequence_ids[14], 13);
		int idx = kernel_add_dynamic(words_laser_beam, words_look_at, 0, g_sequence_ids[14], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 215, 130, FACING_NORTHWEST);
	}

	if (global[kTeleporterCommand]) {
		switch (global[kTeleporterCommand]) {
		case TELEPORTER_BEAM_OUT:
		case TELEPORTER_WRONG:
		case TELEPORTER_STEP_OUT:
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		default:
			break;
		}

		global[kTeleporterCommand] = TELEPORTER_NONE;
	}

	int32 timer = (global[kTimebombTimer + 1] << 16) | global[kTimebombTimer];
	if (timer > 0) {
		global[kTimebombTimer] = 10800 - 600;
		global[kTimebombTimer + 1] = 0;
	}

	section_7_music();
}

static void room_752_daemon() {
	int32 timer = (global[kTimebombTimer + 1] << 16) | global[kTimebombTimer];

	if (timer >= 10800 && global[kTimebombStatus] == TIMEBOMB_ACTIVATED) {
		global[kTimebombStatus] = TIMEBOMB_DEAD;
		global[kTimebombTimer] = global[kTimebombTimer + 1] = 0;
		global[kCheckDaemonTimebomb] = false;
		new_room = 620;
	}
}

static void room_752_pre_parser() {
	if (player_said_2(walkto, west_end_of_platform)) {
		player.walk_off_edge_to_room = 751;
	}
}

static void room_752_parser() {
	if (player_said_2(walk_along, platform))
		;
	else if (player_said_2(step_into, teleporter)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		new_room = 711;
	} else if (player_said_2(take, id_card) && (!player_has(OBJ_ID_CARD) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[12] = kernel_seq_pingpong(g_sprite_ids[12], false, 5, 0, 0, 2);
			kernel_seq_player(g_sequence_ids[12], false);
			kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_SPRITE, 4, 1);
			kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			g_engine->_soundManager->command(15, 0);
			kernel_seq_delete(g_sequence_ids[13]);
			inter_give_to_player(OBJ_ID_CARD);
			kernel_delete_dynamic(local._cardId);
			object_examine(OBJ_ID_CARD, 830, 0);
			break;
		case 2:
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		default:
			break;
		}
	} else if (player_said_2(take, bones) && (player.main_object_source == STROKE_INTERFACE) &&
		(!player_has(OBJ_BONES) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[12] = kernel_seq_pingpong(g_sprite_ids[12], false, 5, 0, 0, 2);
			kernel_seq_player(g_sequence_ids[12], false);
			kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_SPRITE, 4, 1);
			kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			g_engine->_soundManager->command(15, 0);
			if (player_has(OBJ_BONE))
				inter_move_object(OBJ_BONE, NOWHERE);
			inter_give_to_player(OBJ_BONES);
			object_examine(OBJ_BONES, 75221, 0);
			break;
		case 2:
			kernel_seq_timeout(g_sequence_ids[12], -1);
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		default:
			break;
		}
	} else if (player.look_around || player_said_2(look, city)) {
		if (global[kLaserHoleIsThere])
			text_show(75212);
		else
			text_show(75210);
	} else if (player_said_2(look, platform))
		text_show(75213);
	else if (player_said_2(look, cement_block))
		text_show(75214);
	else if (player_said_2(look, rock))
		text_show(75215);
	else if (player_said_2(take, rock))
		text_show(75216);
	else if (player_said_2(look, west_end_of_platform))
		text_show(75217);
	else if (player_said_2(look, teleporter))
		text_show(75218);
	else if ((player_said_2(look, bones) || player_said_2(look, id_card)) && (player.main_object_source == STROKE_INTERFACE)) {
		if (object[OBJ_ID_CARD].location == 752)
			text_show(75219);
		else
			text_show(75220);
	} else if (player_said_2(take, bones) && (player.main_object_source == STROKE_INTERFACE)) {
		if (player_has(OBJ_BONES))
			text_show(75222);
	} else
		return;

	player.command_ready = false;
}

void room_752_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._cardId);
}

void room_752_preload() {
	room_init_code_pointer = room_752_init;
	room_daemon_code_pointer = room_752_daemon;
	room_pre_parser_code_pointer = room_752_pre_parser;
	room_parser_code_pointer = room_752_parser;

	section_7_walker();
	section_7_interface();
	vocab_make_active(words_id_card);
	vocab_make_active(words_walkto);
	vocab_make_active(words_look_at);
	vocab_make_active(words_laser_beam);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
