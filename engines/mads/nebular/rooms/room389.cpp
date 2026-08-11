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
#include "mads/nebular/rooms/section3.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _monsterTime;
	int32 _circularQuoteId;

};

static Scratch local;


static void room_389_init() {
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
	local._monsterTime = 0;
	local._circularQuoteId = 0x159;

	if (global[kAfterHavoc])
		kernel_flip_hotspot(words_monster, false);
	else {
		g_sprite_ids[0] = kernel_load_series(kernel_name('m', -1), 0);
		g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], false, 6, 0, 0, 0);
		kernel_random_messages_init(1, 88, 177, 19, 77, 13, 2, 0xFDFC, 60, 247, 248, 249, 0);
	}

	pal_change_color(252, 63, 37, 26);
	pal_change_color(253, 45, 24, 17);
	player.walker_visible = false;
	kernel.quotes = quote_load(247, 248, 249, 345, 346, 347, 0);

	section_3_music();
}

static void room_389_daemon() {
	kernel_random_message_server();
	if (kernel.clock >= local._monsterTime) {
		int chanceMinor = kernel_check_random() * 4 + 1;
		kernel_generate_random_message(20, chanceMinor);
		local._monsterTime = kernel.clock + 2;
	}
}

static void room_389_parser() {
	if (player_said_2(return_to, air_shaft))
		new_room = 313;
	else if (player_said_2(talkto, monster)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_message_add(quote_string(kernel.quotes, local._circularQuoteId), 160, 136, 0x1110, 120, 1, 32);
			local._circularQuoteId++;
			if (local._circularQuoteId > 0x15B)
				local._circularQuoteId = 0x159;

			break;

		case 1:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(look_through, grate)) {
		if (global[kAfterHavoc]) {
			if ((game.difficulty != DIFFICULTY_HARD) && (object[OBJ_SECURITY_CARD].location == 359))
				text_show(38911);
			else
				text_show(38912);
		} else
			text_show(38910);
	} else if (player_said_2(open, grate)) {
		if (global[kAfterHavoc])
			text_show(38914);
		else
			text_show(38913);
	} else
		return;

	player.command_ready = false;
}

void room_389_synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(local._monsterTime);
	s.syncAsSint32LE(local._circularQuoteId);
}

void room_389_preload() {
	room_init_code_pointer = room_389_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_389_parser;
	room_daemon_code_pointer = room_389_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
