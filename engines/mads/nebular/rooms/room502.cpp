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
#include "mads/nebular/rooms/teleporter.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_502_init() {
	if (global[kSexOfRex] == REX_MALE)
		g_sprite_ids[4] = kernel_load_series("*REXHAND", 0);
	else
		g_sprite_ids[4] = kernel_load_series("*ROXHAND", 0);

	teleporter_init();

	// The original uses scene5xx_section_5_music
	if (!config_file.music_flag)
		g_engine->_soundManager->command(2, 0);
	else if (previous_room == 503)
		g_engine->_soundManager->command(38, 0);
	else
		g_engine->_soundManager->command(29, 0);
}

static void room_502_daemon() {
	teleporter_daemon();
}

static void room_502_parser() {
	if (teleporter_parser()) {
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, viewport) || player_said_2(peer_through, viewport))
		text_show(50210);
	else if (player_said_2(look, keypad))
		text_show(50211);
	else if (player_said_2(look, display))
		text_show(50212);
	else if (player_said_2(look, 0_key) || player_said_2(look, 1_key)
		|| player_said_2(look, 2_key) || player_said_2(look, 3_key)
		|| player_said_2(look, 4_key) || player_said_2(look, 5_key)
		|| player_said_2(look, 6_key) || player_said_2(look, 7_key)
		|| player_said_2(look, 8_key) || player_said_2(look, 9_key)
		|| player_said_2(look, smile_key) || player_said_2(look, enter_key)
		|| player_said_2(look, frown_key))
		text_show(50213);
	else if (player_said_2(look, device) || player.look_around)
		text_show(50214);
	else
		return;

	player.command_ready = false;
}

void room_502_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_502_preload() {
	room_init_code_pointer = room_502_init;
	room_daemon_code_pointer = room_502_daemon;
	room_parser_code_pointer = room_502_parser;

	*player.series_name = '\0';
	section_5_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
