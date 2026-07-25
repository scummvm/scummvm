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

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _detonatorHotspotId;
};

static Scratch local;


static void room_503_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('c', -1), 0);

	if (global[kSexOfRex] == REX_MALE)
		g_sprite_ids[2] = kernel_load_series("*RXMBD_2", 0);
	else
		g_sprite_ids[3] = kernel_load_series("*ROXBD_2", 0);

	if (object[OBJ_DETONATORS].location == room_id) {
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 9, 0, 0, 0);
		local._detonatorHotspotId = kernel_add_dynamic(words_detonators, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(local._detonatorHotspotId, 254, 135, FACING_SOUTH);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 191;
		player.y = 152;
		player.facing = FACING_NORTHWEST;
	}

	section_5_music();
}

static void room_503_parser() {
	if (player_said_2(walk, outside))
		new_room = 501;
	else if (player_said_2(take, detonators)) {
		if (kernel.trigger || !player_has(OBJ_DETONATORS)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				if (global[kSexOfRex] == REX_MALE) {
					g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 8, 0, 0, 1);
					kernel_seq_range(g_sequence_ids[2], 1, 3);
					kernel_seq_player(g_sequence_ids[2], false);
					kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 3, 1);
					kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
				} else {
					g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], true, 8, 0, 0, 1);
					kernel_seq_range(g_sequence_ids[3], 1, 4);
					kernel_seq_player(g_sequence_ids[3], false);
					kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_SPRITE, 4, 1);
					kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				g_engine->_soundManager->command(9, 0);
				kernel_seq_delete(g_sequence_ids[1]);
				kernel_delete_dynamic(local._detonatorHotspotId);
				inter_give_to_player(OBJ_DETONATORS);
				object_examine(OBJ_DETONATORS, 50326, 0);
				break;

			case 2:
				if (global[kSexOfRex] == REX_MALE)
					kernel_seq_timeout(g_sequence_ids[2], -1);
				else
					kernel_seq_timeout(g_sequence_ids[3], -1);

				player.walker_visible = true;
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		}
	} else if (player.look_around)
		text_show(50328);
	else if (player_said_2(look, monitoring_equipment))
		text_show(50310);
	else if (player_said_2(look, photon_rifles))
		text_show(50311);
	else if (player_said_2(take, photon_rifles) || player_said_2(take, nuclear_slingshot))
		text_show(50312);
	else if (player_said_2(look, display_case))
		text_show(50313);
	else if (player_said_2(look, nuclear_slingshot))
		text_show(50314);
	else if (player_said_2(look, water_cooler))
		text_show(50315);
	else if (player_said_2(look, storage_box))
		text_show(50316);
	else if (player_said_2(open, storage_box))
		text_show(50317);
	else if (player_said_2(look, warning_label))
		text_show(50318);
	else if (player_said_2(look, desk))
		text_show(50319);
	else if (player_said_2(look, monitor))
		text_show(50320);
	else if (player_said_2(look, file_cabinets))
		text_show(50322);
	else if (player_said_2(look, box)) {
		if (object_is_here(OBJ_DETONATORS))
			text_show(50323);
		else
			text_show(50324);
	} else if (player_said_2(look, detonators) && (player.main_object_source == 4))
		text_show(50325);
	else if (player_said_2(look, windows))
		text_show(50327);
	else if (player_said_2(open, display_case))
		text_show(50329);
	else if (player_said_2(throw, display_case) && player_has(object_named(player2.words[1])))
		text_show(50330);
	else
		return;

	player.command_ready = false;
}

void room_503_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._detonatorHotspotId);
}

void room_503_preload() {
	room_init_code_pointer = room_503_init;
	room_parser_code_pointer = room_503_parser;

	section_5_walker();
	section_5_interface();
	vocab_make_active(words_detonators);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
