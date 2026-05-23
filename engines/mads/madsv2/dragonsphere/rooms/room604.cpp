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

#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/rooms/room604.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
	int16 prevent;
};

#define local ((Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_box                  0
#define fx_stone                1
#define fx_take                 2
#define fx_fire                 3

#define ROOM_604_DOOR_CLOSES    60

#define PLAYER_X_FROM_605       315
#define PLAYER_Y_FROM_605       111
#define WALK_TO_X_FROM_605      282
#define WALK_TO_Y_FROM_605      112

#define PLAYER_X_FROM_603       -15
#define PLAYER_Y_FROM_603       107
#define WALK_TO_X_FROM_603      15
#define WALK_TO_Y_FROM_603      107


static void room_604_init() {
	local->prevent = false;

	ss[fx_fire]  = kernel_load_series(kernel_name('y', 0), false);
	seq[fx_fire] = kernel_seq_forward(ss[fx_fire], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire], 12);
	kernel_seq_range(seq[fx_fire], KERNEL_FIRST, KERNEL_LAST);

	if (object_is_here(magic_music_box) || object_is_here(power_vacuum_stone)) {
		ss[fx_take] = kernel_load_series("*KGDRM_9", false);
	}

	if (object_is_here(magic_music_box)) {
		ss[fx_box]  = kernel_load_series(kernel_name('p', 0), false);
		seq[fx_box] = kernel_seq_stamp(ss[fx_box], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_box], 4);
	} else {
		kernel_flip_hotspot(words_music_box, false);
	}

	if (object_is_here(power_vacuum_stone)) {
		ss[fx_stone]  = kernel_load_series(kernel_name('p', 1), false);
		seq[fx_stone] = kernel_seq_stamp(ss[fx_stone], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_stone], 14);
	} else {
		kernel_flip_hotspot(words_vortex_stone, false);
	}

	if (previous_room == 603) {
		player_first_walk(PLAYER_X_FROM_603, PLAYER_Y_FROM_603, FACING_EAST,
		                  WALK_TO_X_FROM_603, WALK_TO_Y_FROM_603, FACING_EAST, true);

	} else if (previous_room == 605 || previous_room != KERNEL_RESTORING_GAME) {
		player_first_walk(PLAYER_X_FROM_605, PLAYER_Y_FROM_605, FACING_WEST,
		                  WALK_TO_X_FROM_605, WALK_TO_Y_FROM_605, FACING_WEST, true);
	}

	section_6_music();
}

static void room_604_daemon() {
}

static void room_604_pre_parser() {
	if (player_said_2(walk_through, door_to_eye_chamber) || player_said_2(open, door_to_eye_chamber) ||
	    player_said_2(pull, door_to_eye_chamber)) {
		player.walk_off_edge_to_room = 603;
	}

	if (player_said_1(doorway_to_east) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}
}

static void room_604_parser() {
	if (player_said_2(walk_through, doorway_to_east)) {
		new_room = 605;
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, music_box)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(magic_music_box)) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 6, 1);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_depth(seq[fx_take], 2);
				kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take], true);
				kernel_seq_loc(seq[fx_take], player.x - 3, player.y + 6);
				player.command_ready = false;
				return;
			}
			break;

		case 1:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_box]);
				kernel_flip_hotspot(words_music_box, false);
				global[player_score] += 5;
				inter_give_to_player(magic_music_box);
				sound_play(N_TakeObjectSnd);
				sound_play(N_MusicBoxOn);
				object_examine(magic_music_box, 60420, 0);
				sound_play(N_MusicBoxOff);
			}
			local->prevent = true;
			player.command_ready = false;
			return;

		case 2:
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->prevent          = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(take, vortex_stone)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(power_vacuum_stone)) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 6, 1);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_depth(seq[fx_take], 2);
				kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take], true);
				kernel_seq_loc(seq[fx_take], player.x - 3, player.y + 5);
				player.command_ready = false;
				return;
			}
			break;

		case 1:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_stone]);
				kernel_flip_hotspot(words_vortex_stone, false);
				++global[player_score];
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(power_vacuum_stone);
				object_examine(power_vacuum_stone, 60405, 0);
			}
			local->prevent = true;
			player.command_ready = false;
			return;

		case 2:
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->prevent          = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
			player.command_ready = false;
			return;
		}
	}

	if (player.look_around) {
		text_show(60401);
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(magic_tomes)) {
			text_show(60402);
			player.command_ready = false;
			return;
		}

		if (player_said_1(dresser)) {
			if (object_is_here(power_vacuum_stone)) {
				text_show(60403);
			} else {
				text_show(60429);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(stone_sphere)) {
			text_show(60415);
			player.command_ready = false;
			return;
		}

		if (player_said_1(doorway_to_east)) {
			text_show(60406);
			player.command_ready = false;
			return;
		}

		if (player_said_1(magic_paraphernalia)) {
			text_show(60407);
			player.command_ready = false;
			return;
		}

		if (player_said_1(scroll)) {
			text_show(60408);
			player.command_ready = false;
			return;
		}

		if (player_said_1(shadow_of_window)) {
			text_show(60411);
			player.command_ready = false;
			return;
		}

		if (player_said_1(hourglass)) {
			text_show(60412);
			player.command_ready = false;
			return;
		}

		if (player_said_1(witch_paraphernalia)) {
			text_show(60409);
			player.command_ready = false;
			return;
		}

		if (player_said_1(door_to_eye_chamber)) {
			text_show(60413);
			player.command_ready = false;
			return;
		}

		if (player_said_1(book_of_magic)) {
			text_show(60414);
			player.command_ready = false;
			return;
		}

		if (player_said_1(shelves)) {
			text_show(60416);
			player.command_ready = false;
			return;
		}

		if (player_said_1(pedestal)) {
			if (object_is_here(magic_music_box)) {
				text_show(60417);
			} else {
				text_show(60418);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(closet)) {
			text_show(60421);
			player.command_ready = false;
			return;
		}

		if (player_said_1(music_box) && object_is_here(magic_music_box)) {
			text_show(60419);
			player.command_ready = false;
			return;
		}

		if (player_said_1(vortex_stone) && object_is_here(power_vacuum_stone)) {
			text_show(60404);
			player.command_ready = false;
			return;
		}

		if (player_said_1(wall)) {
			text_show(60422);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_1(take)) {
		if (!player_said_1(music_box) && !player_said_1(vortex_stone)) {
			text_show(60423);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(open, book_of_magic)) {
		text_show(60424);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, dresser)) {
		text_show(60424);
		player.command_ready = false;
		return;
	}

	if (player_said_2(push, pedestal)) {
		text_show(60425);
		player.command_ready = false;
		return;
	}

	if (player_said_2(put, pedestal)) {
		if (player_said_1(red_powerstone) ||
		    player_said_1(yellow_powerstone) ||
		    player_said_1(blue_powerstone)) {
			text_show(60427);
		} else {
			text_show(60426);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(put, torch)) {
		text_show(60428);
		player.command_ready = false;
		return;
	}
}

void room_604_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 4; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint16LE(local->prevent);
}

void room_604_preload() {
	room_init_code_pointer = room_604_init;
	room_pre_parser_code_pointer = room_604_pre_parser;
	room_parser_code_pointer = room_604_parser;
	room_daemon_code_pointer = room_604_daemon;

	section_6_walker();
	section_6_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
