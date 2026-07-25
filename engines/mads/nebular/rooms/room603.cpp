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
#include "mads/nebular/rooms/section6.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _compactCaseHotspotId;
	int16 _noteHotspotId;
};

static Scratch local;


static void room_603_init() {
	if (object[OBJ_COMPACT_CASE].location == room_id) {
		g_sprite_ids[4] = kernel_load_series("*RXMRD_3", 0);
		g_sprite_ids[1] = kernel_load_series(kernel_name('c', -1), 0);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
		kernel_seq_depth(g_sequence_ids[1], 1);
		local._compactCaseHotspotId = kernel_add_dynamic(words_compact_case, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(local._compactCaseHotspotId, 250, 152, FACING_SOUTHEAST);
	}

	if ((game.difficulty != DIFFICULTY_HARD) && (object[OBJ_NOTE].location == room_id)) {
		g_sprite_ids[3] = kernel_load_series("*RXMRC_9", 0);
		g_sprite_ids[2] = kernel_load_series(kernel_name('p', -1), 0);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
		kernel_seq_depth(g_sequence_ids[2], 14);
		local._noteHotspotId = kernel_add_dynamic(words_note, words_walkto, 0, g_sequence_ids[2], 0, 0, 0, 0);
		kernel_dynamic_walk(local._noteHotspotId, 242, 118, FACING_NORTHEAST);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 113;
		player.y = 134;
	}

	section_6_music();
}

static void room_603_parser() {
	if (player_said_2(walk_towards, livingroom))
		new_room = 602;
	else if (player_said_2(take, compact_case)) {
		if (kernel.trigger || !player_has(OBJ_COMPACT_CASE)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], false, 8, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[4], 1, 5);
				kernel_seq_player(g_sequence_ids[4], false);
				kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_SPRITE, 5, 1);
				kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				g_engine->_soundManager->command(9, 0);
				kernel_seq_delete(g_sequence_ids[1]);
				kernel_delete_dynamic(local._compactCaseHotspotId);
				inter_give_to_player(OBJ_COMPACT_CASE);
				object_examine(OBJ_COMPACT_CASE, 60330, 0);
				break;

			case 2:
				kernel_seq_timeout(g_sequence_ids[4], -1);
				player.walker_visible = true;
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		}
	} else if (player_said_2(take, note)) {
		if (kernel.trigger || !player_has(OBJ_NOTE)) {
			if (kernel.trigger == 0) {
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
				kernel_seq_player(g_sequence_ids[3], false);
				kernel_timing_trigger(15, 1);
			} else if (kernel.trigger == 1) {
				g_engine->_soundManager->command(9, 0);
				kernel_seq_delete(g_sequence_ids[2]);
				kernel_delete_dynamic(local._noteHotspotId);
				inter_give_to_player(OBJ_NOTE);
				kernel_seq_delete(g_sequence_ids[3]);
				player.clock = kernel.clock - player.frame_delay;
				player.walker_visible = true;
				player.commands_allowed = true;
			}
		} else
			text_show(60323);
	} else if (player.look_around)
		text_show(60310);
	else if (player_said_2(look, bed))
		text_show(60311);
	else if (player_said_2(look, wig_stand))
		text_show(60312);
	else if (player_said_2(take, wig_stand))
		text_show(60313);
	else if (player_said_2(look, review))
		text_show(60314);
	else if (player_said_2(look, souvenir_tickets))
		text_show(60315);
	else if (player_said_2(look, photograph))
		text_show(60316);
	else if (player_said_2(look, lamp))
		text_show(60317);
	else if (player_said_2(look, directors_slate) || player_said_2(look, crop) || player_said_2(look, megaphone))
		text_show(60318);
	else if (player_said_2(look, snapshot))
		text_show(60319);
	else if (player_said_2(take, snapshot))
		text_show(60320);
	else if (player_said_2(look, perfume))
		text_show(60321);
	else if (player_said_2(take, perfume))
		text_show(60322);
	else if (player_said_2(look, note)) {
		if (object[OBJ_NOTE].location == room_id)
			text_show(60324);
	} else if (player_said_2(look, corner_table)) {
		if (object[OBJ_NOTE].location == room_id)
			text_show(60326);
		else
			text_show(60325);
	} else if (player_said_2(look, vanity)) {
		if (object[OBJ_COMPACT_CASE].location == room_id)
			text_show(60327);
		else
			text_show(60328);
	} else if (player_said_2(look, compact_case) && (player.main_object_source == STROKE_INTERFACE))
		text_show(60329);
	// For the next two checks, the second part of the check wasn't surrounded par parenthesis, which was obviously wrong
	else if (player_said_1(look) && (player_said_1(bra) || player_said_1(boa) || player_said_1(slip)))
		text_show(60331);
	else if (player_said_1(take) && (player_said_1(bra) || player_said_1(boa) || player_said_1(slip)))
		text_show(60332);
	else
		return;

	player.command_ready = false;
}

void room_603_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._compactCaseHotspotId);
	s.syncAsSint16LE(local._noteHotspotId);
}

void room_603_preload() {
	room_init_code_pointer = room_603_init;
	room_parser_code_pointer = room_603_parser;

	section_6_walker();
	section_6_interface();
	vocab_make_active(words_walkto);
	vocab_make_active(words_compact_case);
	vocab_make_active(words_note);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
