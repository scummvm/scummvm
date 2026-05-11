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
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room118.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 guard_id;         /* hotspot id for guard */
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;

/* ========================= Sprites ========================= */

#define fx_guard             1     /* rm118e */

/* walk points */
#define START_X_ROOM_110     124
#define START_Y_ROOM_110     100
#define START_X_ROOM_106     84
#define START_Y_ROOM_106     152

#define WALK_TO_GUARD_X      124
#define WALK_TO_GUARD_Y      106

/* Animations */
/*   rm118a.aa  - guard walking */

/* random numbers */
#define RANDOM_LOW_NUMBER    0
#define RANDOM_HIGH_NUMBER   189


static void room_118_init() {
	int random;

	// Pick a random frame to start the guard animation
	random = imath_random(RANDOM_LOW_NUMBER, RANDOM_HIGH_NUMBER);
	kernel_anim[0].repeat = true;
	aa[0] = kernel_run_animation(kernel_name('a', -1), 0);
	kernel_reset_animation(aa[0], random);
	local->guard_id = kernel_add_dynamic(words_guard, words_walk_to, SYNTAX_MASC_NOT_PROPER,
		KERNEL_NONE, 0, 0, 0, 0);
	kernel_dynamic_walk(local->guard_id, WALK_TO_GUARD_X, WALK_TO_GUARD_Y, FACING_NORTH);
	kernel_dynamic_anim(local->guard_id, aa[0], 0);
	kernel_dynamic_anim(local->guard_id, aa[0], 1);
	kernel_dynamic_anim(local->guard_id, aa[0], 2);
	kernel_dynamic_anim(local->guard_id, aa[0], 3);

	if (previous_room == 110) {
		// Player comes from Way Station
		player.x = START_X_ROOM_110;
		player.y = START_Y_ROOM_110;
		player.facing = FACING_SOUTH;

	} else if (previous_room != KERNEL_RESTORING_GAME) {
		// Player comes from Throne room rm106
		player.x = START_X_ROOM_106;
		player.y = START_Y_ROOM_106;
		player.facing = FACING_NORTHEAST;
	}

	section_1_music();
}

static void room_118_daemon() {
	// No implementation
}

static void room_118_pre_parser() {
	// No implementation
}

static void room_118_parser() {
	if (player.look_around) {
		text_show(11801);
		goto handled;
	}

	if (player_parse(37, 157, 0)) {
		new_room = 110;
		goto handled;
	}

	if (player_parse(37, 164, 0)) {
		new_room = 106;
		goto handled;
	}

	if (player_parse(3, 0) || player_parse(30, 0)) {
		if (player_parse(171, 0)) { text_show(11802); goto handled; }
		if (player_parse(157, 0)) { text_show(11803); goto handled; }
		if (player_parse(161, 0)) { text_show(11804); goto handled; }
		if (player_parse(160, 0)) { text_show(11806); goto handled; }
		if (player_parse(159, 0)) { text_show(11807); goto handled; }
		if (player_parse(24, 0))  { text_show(11809); goto handled; }
		if (player_parse(164, 0)) { text_show(11810); goto handled; }
		if (player_parse(180, 0)) { text_show(11811); goto handled; }
		if (player_parse(272, 0)) { text_show(11812); goto handled; }
	}

	if (player_parse(8, 272, 0)) {
		text_show(11813);
		goto handled;
	}

	if (player_parse(6, 0)) {
		if (player_parse(160, 0) || player_parse(159, 0)) {
			text_show(11808);
			goto handled;
		}
	}

	if (player_parse(6, 0) || player_parse(10, 0) || player_parse(162, 0)) {
		if (player_parse(161, 0)) {
			text_show(11805);
			goto handled;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_118_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.guard_id);
}

void room_118_preload() {
	room_init_code_pointer = room_118_init;
	room_pre_parser_code_pointer = room_118_pre_parser;
	room_parser_code_pointer = room_118_parser;
	room_daemon_code_pointer = room_118_daemon;

	section_1_walker();
	section_1_interface();

	vocab_make_active(words_guard);
	vocab_make_active(words_walk_to);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
