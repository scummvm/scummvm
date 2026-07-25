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
#include "mads/nebular/rooms/section5.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _penlightHotspotId;
};

static Scratch local;


static void room_507_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('p', -1), 0);
	g_sprite_ids[2] = kernel_load_series("*RXMRD_3", 0);

	if ((game.difficulty != DIFFICULTY_EASY) && (object[OBJ_PENLIGHT].location == room_id)) {
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 9, 0, 0, 0);
		local._penlightHotspotId = kernel_add_dynamic(words_penlight, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(local._penlightHotspotId, 233, 152, FACING_SOUTHEAST);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 121;
		player.y = 147;
		player.facing = FACING_NORTH;
	}

	section_5_music();
}

static void room_507_parser() {
	if (player_said_2(walk_through, entrance))
		new_room = 506;
	else if (player_said_2(take, penlight)) {
		if (kernel.trigger || !player_has(OBJ_PENLIGHT)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 6, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[2], 1, 5);
				kernel_seq_player(g_sequence_ids[2], false);
				kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 5, 1);
				kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				kernel_seq_delete(g_sequence_ids[1]);
				kernel_delete_dynamic(local._penlightHotspotId);
				g_engine->_soundManager->command(27, 0);
				inter_give_to_player(OBJ_PENLIGHT);
				object_examine(OBJ_PENLIGHT, 50730, 0);
				break;

			case 2:
				kernel_seq_timeout(g_sequence_ids[2], -1);
				player.walker_visible = true;
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		}
	} else if (player.look_around)
		text_show(50722);
	else if (player_said_2(look, swirling_light))
		text_show(50710);
	else if (player_said_2(take, swirling_light))
		text_show(50711);
	else if (player_said_2(look, old_software))
		text_show(50712);
	else if (player_said_2(take, old_software))
		text_show(50713);
	else if (player_said_2(look, advertisement))
		text_show(50714);
	else if (player_said_2(look, advertising_poster))
		text_show(50715);
	else if (player_said_2(look, sign)) {
		if (inter_point_x < 100)
			text_show(50726);
		else
			text_show(50716);
	} else if (player_said_2(look, hottest_software))
		text_show(50717);
	else if (player_said_2(look, software_shelf))
		text_show(50718);
	else if (player_said_2(look, sensor))
		text_show(50719);
	else if (player_said_2(look, cash_register))
		text_show(50720);
	else if (player_said_2(look, pad_of_paper))
		text_show(50721);
	else if (player_said_2(open, cash_register))
		text_show(50723);
	else if (player_said_2(look, bargain_vat))
		text_show(50724);
	else if (player_said_2(look, window))
		text_show(50725);
	else if (player_said_2(walk_behind, counter)) {
		// WORKAROUND: Empty handling to prevent default "can't do that" dialogs showing
	} else if (player_said_2(look, counter)) {
		if (object_is_here(OBJ_PENLIGHT))
			text_show(50728);
		else
			text_show(50727);
	} else if (player_said_2(look, penlight) && !player_has(OBJ_PENLIGHT)) {
		if (object_is_here(OBJ_PENLIGHT))
			text_show(50729);
	} else if (player_said_2(look, emergency_light))
		text_show(50731);
	else
		return;

	player.command_ready = false;
}

void room_507_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._penlightHotspotId);
}

void room_507_preload() {
	room_init_code_pointer = room_507_init;
	room_parser_code_pointer = room_507_parser;

	section_5_walker();
	section_5_interface();
	vocab_make_active(words_penlight);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
