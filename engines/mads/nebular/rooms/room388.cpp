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
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_388_init() {
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	if (global[kAfterHavoc])
		kernel_flip_hotspot(words_sauropod, false);
	else {
		g_sprite_ids[0] = kernel_load_series(kernel_name('b', 0), 0);
		g_sequence_ids[0] = kernel_seq_stamp(g_sprite_ids[0], false, 1);
	}

	player.walker_visible = false;
	pal_change_color(252, 63, 30, 20);
	pal_change_color(253, 45, 15, 12);
	kernel.quotes = quote_load(340, 341, 342, 343, 344, 0);

	section_3_music();
}

static void room_388_parser() {
	if (player_said_2(return_to, air_shaft))
		new_room = 313;
	else if (player_said_2(talkto, sauropod)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x154), 160, 136, 0x1110, 120, 1, 32);
			break;

		case 1:
			kernel_message_add(quote_string(kernel.quotes, 0x156), 82, 38, 0xFDFC, 300, 0, 0);
			kernel_message_add(quote_string(kernel.quotes, 0x157), 82, 52, 0xFDFC, 300, 0, 0);
			kernel_message_add(quote_string(kernel.quotes, 0x158), 82, 66, 0xFDFC, 300, 2, 0);
			break;

		case 2:
			player.commands_allowed = true;
			kernel_message_add(quote_string(kernel.quotes, 0x155), 160, 136, 0x1110, 120, 0, 32);
			break;

		default:
			break;
		}
	} else if (player_said_2(look_through, grate)) {
		if (global[kAfterHavoc])
			text_show(38811);
		else
			text_show(38810);
	} else if (player_said_2(open, grate))
		text_show(38812);
	else
		return;

	player.command_ready = false;
}

void room_388_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_388_preload() {
	room_init_code_pointer = room_388_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_388_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
