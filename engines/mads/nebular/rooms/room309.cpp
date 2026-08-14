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
#include "mads/nebular/mac_nebular.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/forcefield.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _characterSpriteIndexes[3];
	int16 _messagesIndexes[3];
	int32 _lastFrame;
	Forcefield _forcefield;
};

static Scratch local;


static void room_309_init() {
	g_sprite_ids[1] = kernel_load_series("*SC003x0", 0);
	g_sprite_ids[0] = kernel_load_series("*SC003x1", 0);
	g_sprite_ids[2] = kernel_load_series("*SC003x2", 0);

	init_forcefield(&local._forcefield, true);

	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_full_name(307, 'X', 0, "", KERNEL_SS), 0);

	g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 1);
	kernel_seq_loc(g_sequence_ids[4], 127, 78);
	kernel_seq_depth(g_sequence_ids[4], 15);

	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 7, 0, 0, 1);
	kernel_seq_range(g_sequence_ids[3], -1, 3);
	kernel_seq_depth(g_sequence_ids[3], 11);
	kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_SPRITE, 3, 70);

	pal_change_color(252, 63, 37, 26);
	pal_change_color(253, 45, 24, 17);
	if (!setMacintoshMessageColors(63, 63, 0, 63, 63, 0)) {
		pal_change_color(16, 63, 63, 63);
		pal_change_color(17, 45, 45, 45);
	}
	pal_change_color(250, 63, 20, 20);
	pal_change_color(251, 45, 10, 10);

	player.walker_visible = false;
	player.commands_allowed = false;
	kernel_run_animation(kernel_name('a', -1), 60);

	local._characterSpriteIndexes[0] = kernel_anim[0].anim->series_id[2];
	local._characterSpriteIndexes[1] = kernel_anim[0].anim->series_id[2];
	local._characterSpriteIndexes[2] = kernel_anim[0].anim->series_id[1];

	local._messagesIndexes[0] = -1;
	local._messagesIndexes[1] = -1;
	local._messagesIndexes[2] = -1;

	section_3_music();

	kernel.quotes = quote_load(247, 248, 249, 348, 349, 350, 0);
}

static void room_309_daemon() {
	handle_forcefield(&local._forcefield, &g_sprite_ids[0]);

	if (kernel.trigger == 61) {
		local._messagesIndexes[0] = -1;
		local._messagesIndexes[1] = -1;
	}

	if (kernel.trigger == 62)
		local._messagesIndexes[2] = -1;

	if (kernel_anim[0].anim != nullptr) {
		if (local._lastFrame != kernel_anim[0].frame) {
			local._lastFrame = kernel_anim[0].frame;
			if (local._lastFrame == 39) {
				local._messagesIndexes[0] = kernel_message_add(quote_string(kernel.quotes, 348), 0, 0, 0x1110, 210, 61, 32);
				local._messagesIndexes[1] = kernel_message_add(quote_string(kernel.quotes, 349), 0, 0, 0x1110, 210, 0, 32);
			}

			if (local._lastFrame == 97)
				local._messagesIndexes[2] = kernel_message_add(quote_string(kernel.quotes, 350), 0, 0, 0xFBFA, 180, 62, 32);

			for (int charIdx = 0; charIdx < 3; charIdx++) {
				if (local._messagesIndexes[charIdx] >= 0) {
					bool match = false;
					int j = -1;
					for (j = kernel_anim[0].image; j < (kernel_anim[0].anim ? kernel_anim[0].anim->num_images : 0); j++) {
						if (kernel_anim[0].anim->image[j].series_id == local._characterSpriteIndexes[charIdx]) {
							match = true;
							break;
						}
					}

					if (match) {
						const Image &img = kernel_anim[0].anim->image[j];
						KernelMessage &kmsg = kernel_message[local._messagesIndexes[charIdx]];
						kmsg.x = img.x;
						kmsg.y = img.y - (50 + (14 * ((charIdx == 0) ? 2 : 1)));
					}
				}
			}
		}
	}

	if (kernel.trigger >= 70) {
		switch (kernel.trigger) {
		case 70:
		{
			int idx = kernel_add_dynamic(words_ghastly_beast, words_gawk_at, 0, g_sequence_ids[3], 0, 0, 0, 0);
			kernel_dynamic_walk(idx, 142, 146, FACING_NORTHEAST);
			g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 7, 0, 0, 4);
			kernel_seq_range(g_sequence_ids[3], 2, 3);
			kernel_seq_depth(g_sequence_ids[3], 11);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 71);
		}
		break;

		case 71:
		{
			int _oldIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 7, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[3], 4, 7);
			kernel_seq_depth(g_sequence_ids[3], 11);
			kernel_seq_timeout(_oldIdx, g_sequence_ids[3]);
			int idx = kernel_message_add(quote_string(kernel.quotes, 248), 85, 37, 0xFDFC, 120, 0, 0);
			kernel_message_teletype(idx, 2, true);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 72);
		}
		break;

		case 72:
		{
			int _oldIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 7, 0, 0, 8);
			kernel_seq_range(g_sequence_ids[3], 8, 11);
			kernel_seq_depth(g_sequence_ids[3], 11);
			kernel_seq_timeout(_oldIdx, g_sequence_ids[3]);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 73);
		}
		break;

		case 73:
		{
			int _oldIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 7, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[3], 12, 20);
			kernel_seq_depth(g_sequence_ids[3], 11);
			kernel_seq_timeout(_oldIdx, g_sequence_ids[3]);
			int idx = kernel_message_add(quote_string(kernel.quotes, 249), 170, 49, 0xFDFC, 120, 0, 0);
			kernel_message_teletype(idx, 2, true);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 74);
		}
		break;

		case 74:
		{
			int _oldIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 7, 0, 0, 6);
			kernel_seq_range(g_sequence_ids[3], 21, 23);
			kernel_seq_depth(g_sequence_ids[3], 11);
			kernel_seq_timeout(_oldIdx, g_sequence_ids[3]);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 75);
		}
		break;

		case 75:
		{
			int _oldIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 12, 0, 0, 6);
			kernel_seq_range(g_sequence_ids[3], 24, 25);
			kernel_seq_timeout(_oldIdx, g_sequence_ids[3]);
			kernel_seq_depth(g_sequence_ids[3], 11);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 76);
		}
		break;

		case 76:
		{
			int _oldIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 12, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[3], 26, 28);
			kernel_seq_timeout(_oldIdx, g_sequence_ids[3]);
			kernel_seq_depth(g_sequence_ids[3], 11);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 77);
		}
		break;

		case 77:
		{
			g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 90, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[3], 29, 30);
			kernel_seq_depth(g_sequence_ids[3], 11);
			int idx = kernel_message_add(quote_string(kernel.quotes, 247), 15, 46, 0xFDFC, 120, 0, 0);
			kernel_message_teletype(idx, 2, true);
			kernel_timing_trigger(120, 78);
		}
		break;

		default:
			break;
		}
	}

	if (kernel.trigger == 60)
		new_room = 308;
}

void room_309_synchronize(Common::Serializer &s) {
	local._forcefield.synchronize(s);

	for (int i = 0; i < 3; ++i)
		s.syncAsSint32LE(local._characterSpriteIndexes[i]);
	for (int i = 0; i < 3; ++i)
		s.syncAsSint32LE(local._messagesIndexes[i]);
	s.syncAsSint32LE(local._lastFrame);
}

void room_309_preload() {
	local._lastFrame = -1;
	local._forcefield.init();

	room_init_code_pointer = room_309_init;
	room_daemon_code_pointer = room_309_daemon;
	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
