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

#include "mads/madsv2/core/digi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/midi.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/rooms/section2.h"
#include "mads/madsv2/forest/rooms/room204.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];                /* Sprite series handles  0x00 */
	int16 sequence[10];              /* Sequence handles       0x14 */
	int16 animation[10];             /* Animation handles      0x28 */
	AnimationInfo animation_info[10];/* Animation info         0x3C */
	int16 _8c;
	int16 _8e;
	int16 _90;
	int16 _92;                       /*                        0x92 */
	int16 _pad94[3];                 /*                        0x94 */
	int16 _9a;                       /*                        0x9A */
	int16 _9c;                       /*                        0x9C */
	int16 _9e;
	int16 _a0;
	int16 _a2;
	int16 _a4;
	int16 _a6;                       /*                        0xA6 */
	int16 _a8;                       /*                        0xA8 */
};

static Scratch scratch;

#define local  (&scratch)
#define ss     local->sprite
#define seq    local->sequence
#define aa     local->animation
#define aainfo scratch.animation_info

static void room_204_init1();

static void room_204_init() {
	global[player_score] = -1;
	midi_stop();
	scratch._a8 = 0;
	scratch._a6 = -1;

	if (object_is_here(9)) {
		ss[0] = kernel_load_series(kernel_name('p', 1), 0);
		seq[0] = kernel_seq_stamp(ss[0], false, KERNEL_FIRST);
		kernel_seq_depth(seq[0], 7);
		kernel_seq_loc(seq[0], 293, 87);
		kernel_seq_scale(seq[0], 54);
	} else {
		kernel_flip_hotspot(158, -1);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = 0;
			player.commands_allowed = 0;
		}

		for (int count = 0; count < 10; count++) {
			aainfo[count]._active = 0;
			aainfo[count]._frame = 1;
			aainfo[count]._val3 = 0;
			aainfo[count]._val4 = 0;
		}

		if (previous_room != 199 && flags[18] != 3)
			flags[18]++;
	}

	room_204_init1();
	if (flags[18] == -3)
		flags[18] = 1;
}

static void room_204_init1() {
	global[g131] = 0;
	global[g141] = 0;
	scratch._9c = kernel_run_animation_disp('r', 2, 0);
	kernel_position_anim(scratch._9c, 171, 116, 73, 8);
	scratch._9a = kernel_run_animation_disp('e', 3, 0);
	kernel_position_anim(scratch._9a, 148, 120, 76, 7);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 194;
		player.y = 121;
		player.facing = 1;
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	} else if (previous_room == 203) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 74;
		return;
	} else if (previous_room == 205) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._92 = 78;
		return;
	} else if (previous_room == 220) {
		aa[0] = kernel_run_animation(kernel_name('y', 3), 100);
		aainfo[0]._active = -1;
		scratch._92 = 66;
		return;
	} else if (previous_room == 199) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
	} else {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
	}

	player.commands_allowed = -1;
	player.walker_visible = -1;
}

static void room_204_daemon() {
}

static void room_204_pre_parser() {
	if (player_parse(13, 23, 0))
		player.walk_off_edge_to_room = 203;

	if (player_parse(13, 25, 0))
		player.walk_off_edge_to_room = flags[13] ? 205 : 220;
}

static void room_204_parser() {
	if (global[walker_converse_state]) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		goto handled;
	}

	if (global[player_selected_object] >= 0) {
		goto handled;
	}

	if (player_parse(114, 103, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		local->_8c = 5;
		local->_a6 = 1;
		goto handled;
	}

	if (player_parse(114, 82, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		local->_8c = 5;
		local->_a6 = 2;
		goto handled;
	}

	if (player_parse(126, 158, 0)) {
		global[g154] = 2;
		player.commands_allowed = false;
		kernel_run_animation_talk('b', 9, 0);
		kernel_position_anim(local->_9e, player.x, player.y, player.scale, player.depth);
		goto handled;
	}

	if (player_parse(78, 119, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		local->_8c = 3;
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_204_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._8c);
	s.syncAsSint16LE(scratch._8e);
	s.syncAsSint16LE(scratch._90);
	s.syncAsSint16LE(scratch._92);
	for (int16 &v : scratch._pad94)  s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch._9a);
	s.syncAsSint16LE(scratch._9c);
	s.syncAsSint16LE(scratch._9e);
	s.syncAsSint16LE(scratch._a0);
	s.syncAsSint16LE(scratch._a2);
	s.syncAsSint16LE(scratch._a4);
	s.syncAsSint16LE(scratch._a6);
	s.syncAsSint16LE(scratch._a8);
}

void room_204_preload() {
	room_init_code_pointer = room_204_init;
	room_pre_parser_code_pointer = room_204_pre_parser;
	room_parser_code_pointer = room_204_parser;
	room_daemon_code_pointer = room_204_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
