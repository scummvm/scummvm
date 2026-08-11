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
#include "mads/core/mouse.h"
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
	bool _rhotundaEat2Fl;
	bool _rhotundaEatFl;
};

static Scratch local;


static void room_203_init() {
	if (previous_room == 202) {
		player.x = 187;
		player.y = 99;
		player.facing = FACING_SOUTH;
	} else if (previous_room == 209) {
		player.x = 308;
		player.y = 117;
		player.facing = FACING_WEST;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 155;
		player.y = 152;
		player.facing = FACING_NORTH;
	}

	local._rhotundaEatFl = false;
	local._rhotundaEat2Fl = false;

	if ((global[kRhotundaStatus] == 0) && (!kernel.teleported_in)) {
		local._rhotundaEatFl = true;
		player_walk(158, 135, FACING_SOUTH);
		int idx = kernel_add_dynamic(words_field_to_south, words_walk_towards, 0, 0, 0, 0, 320, 156);
		kernel_dynamic_walk(idx, 155, 152, FACING_SOUTH);
		kernel_dynamic_cursor(idx, CURSOR_DOWN);
	}

	if (!local._rhotundaEatFl) {
		g_sprite_ids[0] = kernel_load_series(kernel_name('b', -1), 0);
		if (g_engine->getRandomNumber(1, 3) == 2) {
			g_sprite_ids[15] = kernel_seq_forward(g_sprite_ids[0], false, 9, 0, 0, 1);
			int idx = kernel_add_dynamic(words_yellow_bird, words_look_at, 0, g_sprite_ids[15], 0, 0, 0, 0);
			kernel_dynamic_walk(idx, -2, 0, FACING_NONE);
			g_engine->_soundManager->command(14, 0);
		}
	}

	kernel.quotes = quote_load(103, 104, 105, 106, 90, 0);

	if (local._rhotundaEatFl) {
		kernel_message_add(quote_string(kernel.quotes, g_engine->getRandomNumber(103, 106)), 0, 0, 0x1110, 120, 0, 34);
	}

	section_2_music();
}

static void room_203_daemon() {
	if (!local._rhotundaEatFl)
		return;

	if ((kernel.trigger == 0) && local._rhotundaEat2Fl)
		return;

	if ((Common::Point(player.x, player.y) != Common::Point(158, 136)) || (player.facing != FACING_SOUTH))
		return;

	local._rhotundaEat2Fl = true;

	if (kernel.trigger == 0) {
		player.walker_visible = false;
		player.commands_allowed = false;
		pal_lock();
		kernel_message_purge();
		kernel_dump_all();
		cursor_id = CURSOR_WAIT;
		mouse_cursor_sprite(cursor, CURSOR_WAIT);
		kernel_run_animation(kernel_full_name(203, 'a', -1, "", KERNEL_AA), 81);
	} else if (kernel.trigger == 81) {
		new_room = 208;
		kernel.force_restart = true;
	}
}

static void room_203_pre_parser() {
	if (local._rhotundaEatFl && !player_said_2(walk_towards, field_to_south)) {
		player_walk(158, 136, FACING_SOUTH);
		player.command_ready = false;
		return;
	}

	if (player_said_2(walkto, open_area_to_east))
		player.walk_off_edge_to_room = 209;
}

static void room_203_parser() {
	if (player.look_around) {
		text_show(20307);
	} else if (player_said_2(walk_towards, field_to_south)) {
		new_room = 208;
	} else if (player_said_2(walk_towards, field_to_north)) {
		new_room = 202;
	} else if (player_said_2(look, sky)) {
		text_show(20301);
	} else if (player_said_2(look, cliff_face)) {
		text_show(20302);
	} else if (player_said_2(look, palm_tree)) {
		text_show(20303);
	} else if (player_said_2(look, field_to_north)) {
		text_show(20304);
	} else if (player_said_2(look, grassy_field)) {
		text_show(20305);
	} else if (player_said_2(look, boulders)) {
		text_show(20305);
	} else
		return;

	player.command_ready = false;
}

void room_203_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._rhotundaEat2Fl);
	s.syncAsByte(local._rhotundaEatFl);
}

void room_203_preload() {
	room_init_code_pointer = room_203_init;
	room_pre_parser_code_pointer = room_203_pre_parser;
	room_parser_code_pointer = room_203_parser;
	room_daemon_code_pointer = room_203_daemon;

	section_2_walker();
	section_2_interface();
	vocab_make_active(477);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
