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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/dragonsphere.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room102.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

struct Scratch {
	int16 x02;
	int16 x04;
	int16 x08;
	int16 x0a;
	int16 x0c;
	int16 x0e;
	int16 x10;
	int16 x20;
	int16 x22;
	int16 x26;
	int16 x28;
	int16 x2a;
	int16 x2c;
	int16 x2e;
	int16 x3c;
	int16 x3e;
	int16 x44;
	int16 x46;
	int16 x48;
	int16 x4a;
	int16 x4c;
};

static Scratch scratch;

void room_102_init() {
	// Load all required series
	scratch.x02 = kernel_load_series(kernel_name('p', 0), 0);
	scratch.x04 = kernel_load_series(kernel_name('p', 1), 0);
	scratch.x08 = kernel_load_series(kernel_name('y', 0), 0);
	scratch.x0a = kernel_load_series(kernel_name('y', 1), 0);
	scratch.x0c = kernel_load_series(kernel_name('x', -1), 0);
	scratch.x0e = kernel_load_series("pd", 0);
	scratch.x10 = kernel_load_series(kernel_name('y', 2), 0);

	// Start looping background animations
	scratch.x26 = kernel_seq_forward(scratch.x08, false, 7, 0, 0, 0);
	scratch.x28 = kernel_seq_forward(scratch.x0a, false, 7, 0, 0, 0);
	scratch.x2e = kernel_seq_forward(scratch.x10, false, 7, 0, 0, 0);

	// Stamp NPC sprites and attach them as dynamic walkers
	scratch.x20 = kernel_seq_stamp(-1, scratch.x02, 0);
	kernel_seq_depth(scratch.x20, 12);
	scratch.x44 = kernel_add_dynamic(264, 13, 1, scratch.x20, 0, 0, 0, 0);
	kernel_dynamic_walk(scratch.x44, 47, 123, 7);

	scratch.x22 = kernel_seq_stamp(-1, scratch.x04, 0);
	kernel_seq_depth(scratch.x22, 12);
	scratch.x46 = kernel_add_dynamic(264, 13, 1, scratch.x22, 0, 0, 0, 0);
	kernel_dynamic_walk(scratch.x46, 47, 123, 7);

	// Position the player depending on which room they came from
	if (previous_room == 103) {
		// Returning from room 103: place player at a fixed position
		player.x = 170;
		player.y = 152;
		player.facing = 7;
	}

	if (previous_room == 103 || previous_room == -2) {
		// previous_room == 103 or 0xFFFE: stamp idle pose at frame -1
		scratch.x2a = kernel_seq_stamp(-1, scratch.x0c, 0);
		kernel_seq_stamp(-1, scratch.x2a, 5);
	} else {
		// Normal entry: stamp the walk-entry pose at frame -2, then walk
		// the player in from off-screen (x=-10) to (130, y) facing 6.
		scratch.x2a = kernel_seq_stamp(-2, scratch.x0c, 0);
		kernel_seq_depth(scratch.x2a, 5);
		player_first_walk(-10, 130, 6, 35, 144, 6, 0);
		player_walk_trigger(70);
		goto music;
	}

music:
	section_1_music();
}

void room_102_pre_parser() {
	// No implementation
}

void room_102_parser() {
	// -----------------------------------------------------------------------
	// Special case: look-around command -> show a dedicated text page and exit
	// -----------------------------------------------------------------------
	if (player.look_around) {
		text_show(10201);
		goto done;
	}

	// -----------------------------------------------------------------------
	// Verb-group 1: verb 0xF5 with objects 0x25, 6, or 0xA
	// -----------------------------------------------------------------------
	if (player_parse(0xF5, 0x25, 0)
		|| player_parse(0xF5, 6, 0)
		|| player_parse(0xF5, 0x0A, 0)) {

		switch (kernel.trigger) {
		case 0:
		{
			// Trigger 0 (initial activation): disable UI, start ping-pong anim
			// for the "pd" series, register two trigger callbacks (2 and 3),
			// then fall through to done.
			player.commands_allowed = 0;
			player.walker_visible = 0;

			scratch.x2c = kernel_seq_pingpong(scratch.x0e, -1, 8, 0, 0, 2);
			kernel_seq_player(scratch.x2c, -1);

			kernel_seq_trigger(scratch.x2c, 2, 1, 1);
			kernel_seq_trigger(scratch.x2c, 0, 0, 3);
			goto done;
		}

		case 1:
		{
			// Trigger 1: delete the walk-entry pose, play a sound, then
			// start a forward anim on the 'x' series at speed 9, depth 5,
			// and arm trigger 2 on it (falls through to the shared synch).
			kernel_seq_delete(scratch.x2a);
			sound_play(24);

			scratch.x2a = kernel_seq_forward(scratch.x0c, false, 9, 1, 0, 0);
			kernel_seq_depth(scratch.x2a, 5);

			// Arm trigger 2 then fall into the shared kernel_synch below.
			kernel_seq_trigger(scratch.x2a, 2, 0, 2);
			goto done;
		}

		case 2:
		{
			// Trigger 2: save old walk-entry handle, stamp a new pose at
			// frame 5, then synch the running animation (scratch.x3c) with
			// the new pose as slave - shares the kernel_synch call path.
			scratch.x4c = scratch.x2a;

			scratch.x2a = kernel_seq_stamp(5, scratch.x0c, 0);
			kernel_seq_depth(scratch.x2a, 5);

			// kernel_synch(ax=1, dx=scratch.x2a, bx=1, push scratch.x4c, push 3)
			// i.e. synch type 1: new pose (x2a) as master, x3c as slave (trigger 3)
			kernel_synch(1, 1, scratch.x2a, scratch.x4c);
			goto done;
		}

		case 3:
		{
			// Trigger 3: re-show the player walker and walk off to room 0x65.
			player.walker_visible = -1;

			kernel_synch(1, 2, 0, scratch.x2c);
			player_walk(0, 0x82, 4);
			player.walk_off_edge_to_room = 101;
			goto done;
		}

		default:
			goto done;
		}
	}

	// -----------------------------------------------------------------------
	// Verb-group 2: verb 0xC5 with objects 0x25, 6, or 0xA -> go to room 0x67
	// -----------------------------------------------------------------------
	if (player_parse(0xC5, 0x25, 0)
		|| player_parse(0xC5, 6, 0)
		|| player_parse(0xC5, 0x0A, 0)) {
		new_room = 0x67;
		goto done;
	}

	// -----------------------------------------------------------------------
	// Verb-group 3: verb 0x108 with objects 4 or 6
	// -----------------------------------------------------------------------
	if (player_parse(0x108, 4, 0) || player_parse(0x108, 6, 0)) {

		switch (kernel.trigger) {
		case 0:
		{
			// Trigger 0: disable UI, set scratch.x4a=1, run the 'B' animation,
			// then synch it (shared path with trigger 0 of the 'A' cutscene below).
			player.commands_allowed = 0;
			player.walker_visible = 0;
			scratch.x4a = 1;

			scratch.x3c = kernel_run_animation(kernel_name('B', -1), 1);

			kernel_synch(3, scratch.x3c, 2, 0);
			goto done;
		}

		case 1:
		{
			// Trigger 1: stamp NPC sprite A (x02) at frame -1, set depth 12,
			// add it as a dynamic at sprite-id 0x108, walk it to (47,123)
			// facing 7, re-show walker, synch x3c for trigger 2, then arm a
			// timing trigger (6 ticks -> trigger 2).
			scratch.x20 = kernel_seq_stamp(-1, scratch.x02, 0);
			kernel_seq_depth(scratch.x20, 12);
			scratch.x44 = kernel_add_dynamic(0x108, 13, 1, scratch.x20, 0, 0, 0, 0);
			kernel_dynamic_walk(scratch.x44, 47, 123, 7);

			player.walker_visible = -1;

			kernel_synch(3, 2, 0, scratch.x3c);
			kernel_timing_trigger(6, 2);
			goto done;
		}

		case 2:
		{
			// Trigger 2: walk player to (0x33, 0x79) facing 7, arm trigger 3.
			player_walk(0x33, 0x79, 7);
			player_walk_trigger(3);
			goto done;
		}

		case 3:
		{
			// Trigger 3: hide walker, set scratch.x4a=2, run the 'A' animation
			// with flags=4, synch it - shares kernel_synch path.
			player.walker_visible = 0;
			scratch.x4a = 2;

			scratch.x3c = kernel_run_animation(kernel_name('A', -1), 4);
			kernel_synch(3, scratch.x3c, 2, 0);
			goto done;
		}

		case 4:
		{
			// Trigger 4: stamp NPC sprite B (x04) at frame -1, set depth 12,
			// add it as a dynamic, walk it, re-show walker and re-enable
			// commands, synch x3c - shared kernel_synch path.
			scratch.x22 = kernel_seq_stamp(-1, scratch.x04, 0);
			kernel_seq_depth(scratch.x22, 12);
			scratch.x46 = kernel_add_dynamic(0x108, 13, 1, scratch.x22, 0, 0, 0, 0);
			kernel_dynamic_walk(scratch.x46, 47, 123, 7);

			player.walker_visible = -1;
			player.commands_allowed = -1;

			kernel_synch(3, 2, 0, scratch.x3c);
			goto done;
		}

		default:
			goto done;
		}
	}

	// -----------------------------------------------------------------------
	// Look-at / examine chain: verb 3 or 0x1E (no-flag forms)
	// -----------------------------------------------------------------------
	if (player_parse(3, 0)
		|| player_parse(0x1E, 0)) {

		// Each object check: if matched, show the corresponding text page.
		if (player_parse(0x22, 0)) {
			text_show(0x27DA); goto done;
		}
		if (player_parse(0x15, 0)) {
			text_show(0x27DB); goto done;
		}
		if (player_parse(0xFC, 0)) {
			text_show(0x27DC); goto done;
		}
		if (player_parse(0x12, 0)) {
			text_show(0x27DE); goto done;
		}
		if (player_parse(0xFD, 0)) {
			text_show(0x27E0); goto done;
		}
		if (player_parse(0x108, 0)) {
			text_show(0x27E1); goto done;
		}
		if (player_parse(0xF5, 0)) {
			text_show(0x27E7); goto done;
		}
		if (player_parse(0xFB, 0)) {
			text_show(0x27E8); goto done;
		}
		if (player_parse(0x18, 0)) {
			text_show(0x27E9); goto done;
		}
		if (player_parse(0xCC, 0)) {
			text_show(0x27EB); goto done;
		}
		if (player_parse(0x23, 0)) {
			text_show(0x27EC); goto done;
		}
		if (player_parse(0x19, 0)) {
			text_show(0x27EE); goto done;
		}
		if (player_parse(0xC5, 0)) {
			text_show(0x27EF); goto done;
		}
		if (player_parse(0x17, 0)) {
			text_show(0x27F0); goto done;
		}
		if (player_parse(0x1A, 0)) {
			text_show(0x27F2); goto done;
		}
		if (player_parse(0x149, 0)) {
			text_show(0x27F3); goto done;
		}
		if (player_parse(0x10, 0)) {
			text_show(0x27F4); goto done;
		}
		if (player_parse(0x14, 0)) {
			text_show(0x27F5); goto done;
		}
		if (player_parse(0x29, 0)) {
			text_show(0x27F6); goto done;
		}
		if (player_parse(0xC4, 0)) {
			text_show(0x27F7); goto done;
		}

		// Verb 6 variants (use/take with direction flag)
		if (player_parse(0x17, 6, 0)) {
			text_show(0x27F0); goto done;
		}
		if (player_parse(0xFB, 4, 0)) {
			text_show(0x27F1); goto done;
		}
		if (player_parse(0x18, 6, 0)) {
			text_show(0x27EA); goto done;
		}

		// Generic use/touch objects
		if (player_parse(5, 0) || player_parse(0x0A, 0)) {
			if (player_parse(0x23, 0)) {
				text_show(0x27ED); goto done;
			}
		}

		if (player_parse(0xFC, 0x0B, 0)) {
			text_show(0x27DD); goto done;
		}
		if (player_parse(0x12, 0x0A, 0)) {
			text_show(0x27DF); goto done;
		}

		// No match in examine chain - fall through to done without clearing
		// command_ready (the retf at locret_39BE0 skips the clear).
		return;
	}

done:
	player.command_ready = 0;
}

void room_102_daemon() {
	int16 var_2;

	// -----------------------------------------------------------------------
	// Animation monitor - mode 1 ('B' cutscene): frames 6, 10, 26
	// -----------------------------------------------------------------------
	if (scratch.x4a == 1) {
		// Skip if animation handle is inactive (both anim words zero)
		if (kernel_anim[scratch.x3c].anim != 0) {

			// Skip if frame hasn't changed since last tick
			if (kernel_anim[scratch.x3c].frame != scratch.x48) {
				var_2 = -1;
				scratch.x48 = kernel_anim[scratch.x3c].frame;

				if (scratch.x48 == 26) {
					// Frame 26: show three sequential text pages (subtitles /
					// dialogue lines for the 'B' cutscene), then check reset.
					text_show(10210);
					text_show(10211);
					text_show(10212);
				} else if (scratch.x48 < 26) {
					int remaining = scratch.x48;

					remaining -= 6;        // frame 6
					if (remaining == 0) {
						// Frame 6: delete the NPC-A stamp, then synch
						// (type 1, slave = x3c, trigger 3) so the cutscene
						// waits for the stamp removal to complete.
						kernel_seq_delete(scratch.x20);
						kernel_synch(3, 1, scratch.x20, scratch.x3c);
					} else {
						remaining -= 4;    // frame 10
						if (remaining == 0) {
							// Frame 10: play a sound effect.
							sound_play(65);
						}
						// All other frames < 26: no action, var_2 stays -1
					}
				}
				// frames > 26: no action

				// If var_2 was set to a reset target (>= 0), loop the
				// animation back to that frame unless it's already there.
				if (var_2 >= 0) {
					if (kernel_anim[scratch.x3c].frame != var_2) {
						kernel_reset_animation(scratch.x3c, var_2);
						scratch.x48 = var_2;
					}
				}
			}
		}
	}

	// -----------------------------------------------------------------------
	// Animation monitor - mode 2 ('A' cutscene): frames 6, 26
	// -----------------------------------------------------------------------
	if (scratch.x4a == 2) {
		// Skip if animation handle is inactive
		if (kernel_anim[scratch.x3c].anim != 0) {

			// Skip if frame hasn't changed since last tick
			if (kernel_anim[scratch.x3c].frame != scratch.x48) {
				var_2 = -1;
				scratch.x48 = kernel_anim[scratch.x3c].frame;

				int remaining = scratch.x48;

				remaining -= 6;            // frame 6
				if (remaining == 0) {
					// Frame 6: delete the NPC-B stamp, synch against x3c
					// (type 1, slave = x3c, trigger 3) - mirrors mode 1.
					kernel_seq_delete(scratch.x22);
					kernel_synch(3, 1, scratch.x22, scratch.x3c);
				} else {
					remaining -= 20;       // frame 26
					if (remaining == 0) {
						// Frame 26: show two text pages for the 'A' cutscene.
						text_show(10213);
						text_show(10214);
					}
					// All other frames: no action, var_2 stays -1
				}

				// Reset guard - same pattern as mode 1.
				if (var_2 >= 0) {
					if (kernel_anim[scratch.x3c].frame != var_2) {
						kernel_reset_animation(scratch.x3c, var_2);
						scratch.x48 = var_2;
					}
				}
			}
		}
	}

	// -----------------------------------------------------------------------
	// Walk-entry trigger dispatch (fired by player_walk_trigger in init)
	// -----------------------------------------------------------------------
	if (kernel.trigger < 70)
		return;

	switch (kernel.trigger - 70) {
	case 0:
		// Trigger 70: walk-entry complete - delete the entry-pose stamp,
		// play the arrival sound, then start a backward anim on the 'x'
		// series (speed 9, range 1..4) and arm trigger 71 on it.
		kernel_seq_delete(scratch.x2a);
		sound_play(25);

		scratch.x2a = kernel_seq_backward(scratch.x0c, false, 9, 1, 0, 0);
		kernel_seq_depth(scratch.x2a, 5);
		kernel_seq_range(scratch.x2a, 1, 4);
		kernel_seq_trigger(scratch.x2a, 0, 0, 71);
		return;

	case 1:
		// Trigger 71: backward anim finished - save old pose handle, stamp
		// the idle pose at frame -1, set depth 0xFFFE (behind everything),
		// synch type 1 (new pose as master, old pose as slave), then
		// re-enable player commands.
		scratch.x4c = scratch.x2a;

		scratch.x2a = kernel_seq_stamp(-1, scratch.x0c, 0);
		kernel_seq_depth(scratch.x2a, 0xFFFE);
		kernel_synch(1, 1, scratch.x2a, scratch.x4c);

		player.commands_allowed = -1;
		return;

	default:
		return;
	}
}

void room_102_synchronize(Common::Serializer &s) {
	s.syncMultipleLE(
		scratch.x02, scratch.x04, scratch.x08, scratch.x0a,
		scratch.x0c, scratch.x0e, scratch.x10, scratch.x20,
		scratch.x22, scratch.x26, scratch.x28, scratch.x2a,
		scratch.x2c, scratch.x2e, scratch.x3c, scratch.x3e,
		scratch.x44, scratch.x46, scratch.x48, scratch.x4a,
		scratch.x4c
	);
}

void room_102_preload() {
	room_init_code_pointer = room_102_init;
	room_pre_parser_code_pointer = room_102_pre_parser;
	room_parser_code_pointer = room_102_parser;
	room_daemon_code_pointer = room_102_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
