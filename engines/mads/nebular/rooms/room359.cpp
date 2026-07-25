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

#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _cardHotspotId;
};

static Scratch local;


static void room_359_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('b', -1), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_full_name(307, 'X', 0, "", KERNEL_SS), 0);

	if (global[kSexOfRex] == REX_MALE)
		g_sprite_ids[2] = kernel_load_series("*RXMBD_2", 0);
	else
		g_sprite_ids[4] = kernel_load_series("*ROXBD_2", 0);

	g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
	kernel_seq_loc(g_sequence_ids[3], 127, 78);
	kernel_seq_depth(g_sequence_ids[3], 15);

	if (object_is_here(OBJ_SECURITY_CARD)) {
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 9, 0, 0, 0);
		local._cardHotspotId = kernel_add_dynamic(words_security_card, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(local._cardHotspotId, 107, 107, FACING_SOUTH);
	}

	if (previous_room == 358) {
		player.x = 301;
		player.y = 141;
	}
	else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 15;
		player.y = 148;
	}

	section_3_music();
}

static void room_359_pre_parser() {
	if (player_said_2(walk_down, corridor_to_east))
		player.walk_off_edge_to_room = 358;

	if (player_said_2(walk_down, corridor_to_west))
		player.walk_off_edge_to_room = 360;
}

static void room_359_parser() {
	if (player.look_around) {
		if ((game.difficulty != DIFFICULTY_HARD) && (object[OBJ_SECURITY_CARD].location == 359))
			text_show(35914);
		else
			text_show(35915);
	} else if (player_said_2(take, security_card)) {
		if (kernel.trigger || !player_has(OBJ_SECURITY_CARD)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				text_show(35920);
				if (global[kSexOfRex] == REX_MALE) {
					g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 4, 0, 0, 2);
					kernel_seq_player(g_sequence_ids[2], false);
					kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 6, 1);
					kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
				} else {
					g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], true, 7, 0, 0, 2);
					kernel_seq_player(g_sequence_ids[4], false);
					kernel_seq_loc(g_sequence_ids[4], 106, 110);
					kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_SPRITE, 6, 1);
					kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				kernel_seq_delete(g_sequence_ids[1]);
				kernel_delete_dynamic(local._cardHotspotId);
				g_engine->_soundManager->command(57, 0);
				inter_give_to_player(OBJ_SECURITY_CARD);
				object_examine(OBJ_SECURITY_CARD, 0x330, 0);
				kernel_load_variant(1);
				break;

			case 2:
				if (global[kSexOfRex] == REX_MALE)
					kernel_seq_timeout(g_sequence_ids[2], -1);
				else
					kernel_seq_timeout(g_sequence_ids[4], -1);

				player.walker_visible = true;
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		}
	} else if (player_said_2(look, bloody_cell_wall))
		text_show(35910);
	else if (player_said_2(look, bed))
		text_show(35911);
	else if (player_said_2(look, sink))
		text_show(35912);
	else if (player_said_2(look, toilet))
		text_show(35913);
	else if (player_said_2(look, corridor_to_east))
		text_show(35916);
	else if (player_said_2(look, corridor_to_west))
		text_show(35917);
	else if (player_said_2(look, limb))
		text_show(35918);
	else if (player_said_2(take, limb))
		text_show(35919);
	else if (player_said_2(look, security_card) && (player.main_object_source == STROKE_INTERFACE))
		text_show(35921);
	else if (player_said_2(look, blood_stain)) {
		if ((game.difficulty != DIFFICULTY_HARD) && (object[OBJ_SECURITY_CARD].location == 359))
			text_show(35922);
		else
			text_show(35923);
	} else if (player_said_2(look, wall_board))
		text_show(35924);
	else if (player_said_2(take, wall_board))
		text_show(35925);
	else if (player_said_2(look, rip_in_floor))
		text_show(35926);
	else if (player_said_2(look, corridor))
		text_show(35927);
	else if (player_said_2(look, floor)) {
		if ((game.difficulty != DIFFICULTY_HARD) && (object[OBJ_SECURITY_CARD].location == 359))
			text_show(35928);
		else
			text_show(35929);
	} else if (player_said_2(open, air_vent) || player_said_2(look, air_vent))
		text_show(36016);
	else
		return;

	player.command_ready = false;
}

void room_359_synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(local._cardHotspotId);
}

void room_359_preload() {
	room_init_code_pointer = room_359_init;
	room_pre_parser_code_pointer = room_359_pre_parser;
	room_parser_code_pointer = room_359_parser;

	section_3_walker();
	section_3_interface();
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
