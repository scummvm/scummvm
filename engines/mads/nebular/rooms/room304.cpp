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
	int32 _explosionSpriteId;
};

static Scratch local;

static void room_304_init() {
	if (previous_room == 303) {
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_run_animation(kernel_name('a', -1), 60);
	} else {
		if (global[kSexOfRex] == REX_MALE)
			g_sprite_ids[1] = kernel_load_series(kernel_name('a', 0), 0);
		else
			g_sprite_ids[4] = kernel_load_series(kernel_name('a', 2), 0);

		g_sprite_ids[2] = kernel_load_series(kernel_name('a', 1), 0);
		g_sprite_ids[3] = kernel_load_series(kernel_name('b', 0), 0);

		g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 150, 0, 3, 0);
		kernel_seq_depth(g_sequence_ids[3], 2);
		pal_change_color(252, 45, 63, 45);
		pal_change_color(253, 20, 45, 20);

		if (global[kSexOfRex] == REX_MALE) {
			player.x = 111;
			player.y = 117;
		}
		else {
			player.x = 113;
			player.y = 116;
		}

		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 11, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[2], -1, -1);
		kernel_timing_trigger(48, 70);
	}

	section_3_music();
	kernel.quotes = quote_load(235, 236, 0);
}

static void room_304_daemon() {
	if (kernel.trigger == 60)
		new_room = 311;

	if (kernel.trigger >= 70) {
		switch (kernel.trigger) {
		case 70:
		{
			player.walker_visible = false;
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 8, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[2], 2, 4);
			kernel_seq_depth(g_sequence_ids[2], 1);
			if (global[kSexOfRex] == REX_MALE)
				local._explosionSpriteId = g_sprite_ids[1];
			else
				local._explosionSpriteId = g_sprite_ids[4];

			int sprIdx = kernel_seq_forward(local._explosionSpriteId, false, 8, 0, 0, 1);
			kernel_seq_range(sprIdx, -1, 4);
			kernel_seq_depth(sprIdx, 1);
			kernel_seq_trigger(sprIdx, KERNEL_TRIGGER_EXPIRE, 0, 71);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 74);
		}
		break;

		case 71:
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0xEB), 0, 0, 0x1110, 60, 0, 34);
			kernel_timing_trigger(1, 72);
			break;

		case 72:
		{
			g_engine->_soundManager->command(43, 0);
			int sprIdx = kernel_seq_forward(local._explosionSpriteId, false, 8, 0, 0, 1);
			kernel_seq_range(sprIdx, 5, -2);
			kernel_seq_depth(sprIdx, 1);
			kernel_seq_trigger(sprIdx, KERNEL_TRIGGER_EXPIRE, 0, 73);
			if (config_file.naughtiness == NICE)
				kernel_seq_trigger(sprIdx, KERNEL_TRIGGER_SPRITE, 8, 78);
		}
		break;

		case 73:
		{
			int sprIdx = kernel_seq_forward(local._explosionSpriteId, false, 8, 0, 0, 0);
			kernel_seq_range(sprIdx, -2, -2);
			kernel_seq_depth(sprIdx, 1);
		}
		break;

		case 74:
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 8, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[2], 5, -2);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 75);
			break;

		case 75:
			g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 8, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[2], 2, -2);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 76);
			break;

		case 76:
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 8, 0, 0, 0);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_range(g_sequence_ids[2], 2, 2);
			kernel_timing_trigger(48, 77);
			break;

		case 77:
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0xEC), 211, 45, 0xFDFC, 180, 0, 32);
			kernel_timing_trigger(120, 78);
			break;

		case 78:
			new_room = 316;
			break;

		default:
			break;
		}
	}
}

void room_304_synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(local._explosionSpriteId);
}

void room_304_preload() {
	room_init_code_pointer = room_304_init;
	room_daemon_code_pointer = room_304_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
