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
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section2.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _ambushFl;
	bool _wakeFl;
	int32 _monkeyFrame;
	int32 _scrollY;
	int32 _monkeyTime;
};

static Scratch local;


static void room_211_init() {
	g_sprite_ids[1] = kernel_load_series("*SC002Z2", 0);
	local._wakeFl = false;

	if (previous_room == 210) {
		player.x = 25;
		player.y = 148;
	}
	else if (previous_room == 205) {
		player.x = 49;
		player.y = 133;
		player.facing = FACING_WEST;
		local._wakeFl = true;
		player.commands_allowed = false;
		player.walker_visible = false;
		kernel_run_animation(kernel_name('A', -1), 100);
		kernel_reset_animation(0, 169);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 310;
		player.y = 31;
		player.facing = FACING_SOUTHWEST;
	}

	if (g_engine->getRandomNumber(1, 8) == 1) {
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[1], false, 6, 0, 0, 0);
		kernel_seq_loc(g_sequence_ids[2], 202, 126);
		kernel_seq_depth(g_sequence_ids[2], 8);
		kernel_seq_motion(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, -200, 0);
		kernel_add_dynamic(words_slithering_snake, words_walkto, 0, g_sequence_ids[2], 1, 1, 41, 10);
	}

	if (kernel.teleported_in)
		inter_give_to_player(OBJ_BINOCULARS);

	pal_change_color(252, 63, 44, 30);
	pal_change_color(253, 63, 20, 22);
	kernel.quotes = quote_load(161, 162, 163, 164, 165, 166, 167, 168, 151, 152, 153, 154, 155, 156, 157, 158, 1, 0);

	if (global[kMonkeyStatus] == MONKEY_AMBUSH_READY)
		kernel_random_messages_init(2, 0, 54, 0, 30, 13, 2, 0xFDFC, 60, 151, 152, 153, 154, 0);

	local._monkeyTime = kernel.clock;
	local._scrollY = 30;

	local._ambushFl = false;
	local._monkeyFrame = 0;

	section_2_music();
}

static void room_211_daemon() {
	if (global[kMonkeyStatus] == MONKEY_AMBUSH_READY) {
		kernel_random_message_server();

		if (!local._ambushFl && !local._wakeFl && (kernel.clock >= local._monkeyTime)) {
			int chanceMinor = kernel_check_random() * 4 + 1;
			if (kernel_generate_random_message(80, chanceMinor))
				g_engine->_soundManager->command(18, 0);

			local._monkeyTime = kernel.clock + 2;
		}

		if ((Common::Point(player.x, player.y) == Common::Point(52, 132)) && (player.facing == FACING_WEST) && !player.walking &&
			(kernel.trigger || !local._ambushFl)) {
			switch (kernel.trigger) {
			case 0:
				if (player_has(OBJ_BINOCULARS)) {
					local._ambushFl = true;
					local._monkeyFrame = 0;
					player.commands_allowed = false;
					player.walker_visible = false;
					kernel_message_purge();
					kernel_run_animation(kernel_name('A', -1), 90);
					g_engine->_soundManager->command(19, 0);
					int count = (int)inven_num_objects;
					for (int idx = 0; idx < count; idx++) {
						if ((inven[idx] == OBJ_BINOCULARS) && (active_inven != idx))
							inter_set_active_inven(idx);
					}
				}
				break;

			case 90:
				g_engine->_soundManager->command(10, 0);
				player.commands_allowed = true;
				player.walker_visible = true;
				player.x = 49;
				player.y = 133;
				local._ambushFl = false;
				global[kMonkeyStatus] = MONKEY_HAS_BINOCULARS;
				break;

			default:
				break;
			}
		}
	}

	if (local._ambushFl && (kernel_anim[0].frame > local._monkeyFrame)) {
		local._monkeyFrame = kernel_anim[0].frame;
		switch (local._monkeyFrame) {
		case 2:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 157), 12, 4, 0xFDFC, 60, 0, 0);
			kernel_message_teletype(msgIndex, 2, true);
		}
		break;

		case 12:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 155), 35, 20, 0xFDFC, 60, 0, 0);
			kernel_message_teletype(msgIndex, 4, true);
		}
		break;

		case 42:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 156), 60, 45, 0xFDFC, 60, 0, 0);
			kernel_message_teletype(msgIndex, 6, true);
		}
		break;

		case 73:
			kernel_message_add(quote_string(kernel.quotes, 157), 102, 95, 0xFDFC, 75, 0, 32);
			break;

		case 90:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 158), 102, 95, 0xFDFC, 60, 0, 32);
			kernel_message_teletype(msgIndex, 6, true);
		}
		break;

		case 97:
			inter_set_active_inven(-1);
			inter_take_from_player(OBJ_BINOCULARS, 1);
			break;

		case 177:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 161), 63, local._scrollY, 0x1110, 180, 0, 0);
			kernel_message_teletype(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		case 181:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 162), 63, local._scrollY, 0x1110, 180, 0, 0);
			kernel_message_teletype(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		case 188:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 163), 63, local._scrollY, 0x1110, 180, 0, 0);
			kernel_message_teletype(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		case 200:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 164), 63, local._scrollY, 0x1110, 180, 0, 0);
			kernel_message_teletype(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		default:
			break;
		}
	}

	if (local._wakeFl) {
		if (kernel.trigger == 100) {
			player.walker_visible = true;
			player.commands_allowed = true;
			local._wakeFl = false;
		}

		if (kernel_anim[0].frame > local._monkeyFrame) {
			local._monkeyFrame = kernel_anim[0].frame;
			switch (kernel_anim[0].frame) {
			case 177:
			{
				int msgIndex = kernel_message_add(quote_string(kernel.quotes, 165), 63, local._scrollY, 0x1110, 180, 0, 0);
				kernel_message_teletype(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			case 181:
			{
				int msgIndex = kernel_message_add(quote_string(kernel.quotes, 166), 63, local._scrollY, 0x1110, 180, 0, 0);
				kernel_message_teletype(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			case 188:
			{
				int msgIndex = kernel_message_add(quote_string(kernel.quotes, 167), 63, local._scrollY, 0x1110, 180, 0, 0);
				kernel_message_teletype(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			case 200:
			{
				int msgIndex = kernel_message_add(quote_string(kernel.quotes, 168), 63, local._scrollY, 0x1110, 180, 0, 0);
				kernel_message_teletype(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			default:
				break;
			}
		}
	}
}

static void room_211_pre_parser() {
	if (player_said_2(walk_down, jungle_path) && player_has(OBJ_BINOCULARS) && (global[kMonkeyStatus] == MONKEY_AMBUSH_READY)
		&& (inter_point_x <= 52) && (inter_point_y >= 132))
		player_walk(52, 132, FACING_WEST);

	if (player_said_2(walk_down, path_to_west)) {
		if (player_has(OBJ_BINOCULARS) && (global[kMonkeyStatus] == MONKEY_AMBUSH_READY))
			player_walk(52, 132, FACING_WEST);
		else
			player.walk_off_edge_to_room = 210;
	}

	if (player_said_2(walk_down, path_to_northeast))
		player.walk_off_edge_to_room = 207;
}

static void room_211_parser() {
	if (player.look_around && (global[kMonkeyStatus] == MONKEY_AMBUSH_READY))
		text_show(21111);
	else if (player_said_3(look, binoculars, palm_tree))
		text_show(21116);
	else if (player_said_2(look, bushy_fern))
		text_show(21101);
	else if (player_said_2(look, jungle_path))
		text_show(21102);
	else if (player_said_2(look, palm_tree)) {
		if (global[kMonkeyStatus] == MONKEY_AMBUSH_READY) {
			if (config_file.naughtiness == NAUGHTY)
				text_show(21103);
			else
				text_show(21104);
		} else {
			text_show(21105);
		}
	} else if (player_said_2(look, thick_undergrowth)) {
		if (config_file.naughtiness == NAUGHTY)
			text_show(21106);
		else
			text_show(21107);
	} else if (player_said_2(look, jungle))
		text_show(21108);
	else if (player_said_2(look, path_to_northeast))
		text_show(21109);
	else if (player_said_2(look, path_to_west))
		text_show(21110);
	else if (player_said_2(look, slithering_snake))
		text_show(21113);
	else if (player_said_2(take, slithering_snake))
		text_show(21114);
	else if (player_said_2(look, rocks))
		text_show(21115);
	else
		return;

	player.command_ready = false;
}

void room_211_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._ambushFl);
	s.syncAsByte(local._wakeFl);

	s.syncAsSint32LE(local._monkeyFrame);
	s.syncAsSint32LE(local._scrollY);
	s.syncAsUint32LE(local._monkeyTime);
}

void room_211_preload() {
	room_init_code_pointer = room_211_init;
	room_pre_parser_code_pointer = room_211_pre_parser;
	room_parser_code_pointer = room_211_parser;
	room_daemon_code_pointer = room_211_daemon;

	section_2_walker();
	section_2_interface();
	vocab_make_active(words_slithering_snake);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
