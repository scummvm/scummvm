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

#include "common/config-manager.h"
#include "mads/forest/rooms/section9.h"
#include "mads/forest/global.h"
#include "mads/forest/midi.h"
#include "mads/core/game.h"
#include "mads/core/global.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/mouse.h"
#include "mads/core/object.h"
#include "mads/core/player.h"
#include "mads/engine.h"

namespace MADS {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];
	int16 sequence[10];
	int16 animation[7];
	AnimationInfo animation_info[2];
};

static Scratch local;

#define ss    local.sprite
#define seq   local.sequence
#define aa    local.animation
#define aainfo local.animation_info

/* Triggers */
#define TRIGGER1        101
#define TRIGGER_ACTION  150

static void room_904_setup_objects() {
	object[0].location  = 101;
	object[1].location  = 302;
	object[2].location  = 221;
	object[3].location  = 503;
	object[4].location  = 307;
	object[5].location  = 106;
	object[6].location  = 303;
	object[7].location  = 201;
	object[8].location  = 101;
	object[9].location  = 204;
	object[10].location = 301;
	object[11].location = 101;
	object[12].location = 201;
	object[13].location = 403;
	object[14].location = 307;
	object[15].location = 301;

	for (int i = 0; i < 16; ++i)
		object_set_quality(i, 0, 0);

	player.num_rooms_been_in = 0;
	global_init_code();
}

static void room_904_seen_intro() {
	ConfMan.setBool("seen_intro", true);
	ConfMan.flushToDisk();
}

static void room_904_init() {
	last_keypressed = -1;
	global_midi_play(3);
	aainfo[1]._active = 0;
	aainfo[1]._frame = 0;
	global[g009] = -1;
	viewing_at_y = 22;
	aainfo[1]._val3 = 0;
	global[play_background_sounds] = 0;

	ss[6] = kernel_load_series("*main0", 0);
	ss[8] = kernel_load_series("*main1", 0);
	ss[9] = kernel_load_series("*main2", 0);
	ss[7] = kernel_load_series("*main3", 0);
	seq[1] = kernel_load_series("*main4", 0);

	kernel_seq_depth(kernel_seq_stamp(ss[6], false, -1), 10);
	kernel_seq_depth(kernel_seq_stamp(ss[7], false, -1), 10);
	kernel_seq_depth(kernel_seq_stamp(ss[8], false, -1), 10);
	kernel_seq_depth(kernel_seq_stamp(ss[9], false, -1), 10);
	kernel_seq_depth(kernel_seq_stamp(seq[1], false, -1), 10);

	ss[0] = kernel_load_series("*maint0", 0);
	ss[2] = kernel_load_series("*maint1", 0);
	ss[3] = kernel_load_series("*maint2", 0);
	ss[1] = kernel_load_series("*maint3", 0);
	ss[5] = kernel_load_series("*maint4", 0);
	ss[4] = kernel_load_series("*maint5", 0);

	kernel_timing_trigger(10, TRIGGER1);
	player.walker_visible = false;
}

static void room_904_daemon() {
	int16 selection = 0;

	if (kernel.trigger == TRIGGER1)
		mouse_cursor_sprite(cursor, 1);

	if (mouse_hidden)
		mouse_show();

	// Hit-test the 6 menu button regions
	if (mouse_x >= 42 && mouse_x <= 71 && mouse_y >= 30 && mouse_y <= 52)
		selection = 1;
	if (mouse_x >= 41 && mouse_x <= 71 && mouse_y >= 146 && mouse_y <= 170)
		selection = 2;
	if (mouse_x >= 248 && mouse_x <= 278 && mouse_y >= 30 && mouse_y <= 52)
		selection = 3;
	if (mouse_x >= 125 && mouse_x <= 191 && mouse_y >= 146 && mouse_y <= 171)
		selection = 4;
	if (mouse_x >= 248 && mouse_x <= 277 && mouse_y >= 146 && mouse_y <= 170)
		selection = 5;
	if (mouse_x >= 72 && mouse_x <= 247 && mouse_y >= 55 && mouse_y <= 144)
		selection = 6;

	// Hover: stamp highlight when first entering a button area
	if (aainfo[1]._active == 0) {
		switch (selection) {
		case 1:
			seq[5] = kernel_seq_stamp(ss[0], false, 1);
			kernel_seq_depth(seq[5], 1);
			aainfo[1]._active = 1;
			break;
		case 2:
			seq[7] = kernel_seq_stamp(ss[2], false, 1);
			kernel_seq_depth(seq[7], 1);
			aainfo[1]._active = 2;
			break;
		case 3:
			seq[8] = kernel_seq_stamp(ss[3], false, 1);
			kernel_seq_depth(seq[8], 1);
			aainfo[1]._active = 3;
			break;
		case 4:
			seq[6] = kernel_seq_stamp(ss[1], false, 1);
			kernel_seq_depth(seq[6], 1);
			aainfo[1]._active = 4;
			break;
		case 5:
			aa[0] = kernel_seq_stamp(ss[5], false, 1);
			kernel_seq_depth(aa[0], 1);
			aainfo[1]._active = 5;
			break;
		case 6:
			seq[9] = kernel_seq_stamp(ss[4], false, 1);
			kernel_seq_depth(seq[9], 1);
			aainfo[1]._active = 6;
			break;
		}
	}

	// Hover: delete old highlight when leaving a button area or switching buttons
	if (selection == 0 || aainfo[1]._active != selection) {
		switch (aainfo[1]._active) {
		case 1: kernel_seq_delete(seq[5]); break;
		case 2: kernel_seq_delete(seq[7]); break;
		case 3: kernel_seq_delete(seq[8]); break;
		case 4: kernel_seq_delete(seq[6]); break;
		case 5: kernel_seq_delete(aa[0]);  break;
		case 6: kernel_seq_delete(seq[9]); break;
		}
		aainfo[1]._active = 0;
	}

	// Click: stamp pressed state when button first clicked
	if (mouse_stroke_going &&
	    aainfo[1]._val3 == 0 &&
	    aainfo[1]._frame == 0) {
		switch (selection) {
		case 1:
			aa[1] = kernel_seq_stamp(ss[6], false, 2);
			kernel_seq_depth(aa[1], 1);
			aainfo[1]._frame = 1;
			break;
		case 2:
			aa[3] = kernel_seq_stamp(ss[8], false, 2);
			kernel_seq_depth(aa[3], 1);
			aainfo[1]._frame = 2;
			break;
		case 3:
			aa[4] = kernel_seq_stamp(ss[9], false, 2);
			kernel_seq_depth(aa[4], 1);
			aainfo[1]._frame = 3;
			break;
		case 4:
			aa[2] = kernel_seq_stamp(ss[7], false, 2);
			kernel_seq_depth(aa[2], 1);
			aainfo[1]._frame = 4;
			break;
		case 5:
			aa[6] = kernel_seq_stamp(seq[1], false, 2);
			kernel_seq_depth(aa[6], 1);
			aainfo[1]._frame = 5;
			break;
		case 6:
			aainfo[1]._frame = 6;
			break;
		}
	}

	// Cancel: delete pressed stamp when mouse moves off a button while held
	if (aainfo[1]._frame != 0 &&
	    aainfo[1]._val3 == 0 &&
	    selection == 0) {
		switch (aainfo[1]._frame) {
		case 1: kernel_seq_delete(aa[1]); break;
		case 2: kernel_seq_delete(aa[3]); break;
		case 3: kernel_seq_delete(aa[4]); break;
		case 4: kernel_seq_delete(aa[2]); break;
		case 5: kernel_seq_delete(aa[6]); break;
		default: break;
		}
		aainfo[1]._frame = 0;
	}

	// Release: delete pressed stamp on mouse release and schedule the action
	if (!mouse_stroke_going && aainfo[1]._val3 == 0) {
		switch (aainfo[1]._frame) {
		case 1: kernel_seq_delete(aa[1]); break;
		case 2: kernel_seq_delete(aa[3]); break;
		case 3: kernel_seq_delete(aa[4]); break;
		case 4: kernel_seq_delete(aa[2]); break;
		case 5: kernel_seq_delete(aa[6]); break;
		default: break;
		}
		if (aainfo[1]._frame != 0) {
			kernel_timing_trigger(1, TRIGGER_ACTION);
			aainfo[1]._val3 = -1;
		}
	}

	// Action: execute the chosen menu option when the delay trigger fires
	if (kernel.trigger == TRIGGER_ACTION) {
		switch (aainfo[1]._frame) {
		case 1:
			// Credits
			midi_stop();
			global[play_background_sounds] = 0;
			global[g016] = -1;
			global[g102] = -1;
			new_room = 510;
			break;

		case 2: {
			// Resume Game
			auto saves = g_engine->listSaves();
			if (!saves.empty()) {
				int savegameSlot = saves.back().getSaveSlot();

				midi_stop();
				if (g_engine->loadGameState(savegameSlot).getCode() != Common::kNoError)
					new_room = 101;
			}
			break;
		}

		case 3:
			// Restore game
			midi_stop();
			kernel.activate_menu = GAME_RESTORE_MENU;
			break;

		case 4:
			// Play Intro
			room_904_setup_objects();
			global[play_background_sounds] = 0;
			midi_stop();
			flags[0] = -4;
			global[g016] = -1;
			new_room = 101;
			room_904_seen_intro();
			break;

		case 5:
			// Exit
			game.going = 0;
			break;

		case 6:
			// Start Game
			new_room = 101;
			midi_stop();
			room_904_setup_objects();
			global[g016] = 0;
			flags[0] = flags[1] = flags[2] = flags[3] = 0;
			break;

		default:
			break;
		}

		aainfo[1]._val3 = 0;
		aainfo[1]._frame = 0;
	}

	// Escape key: trigger quit
	if (last_keypressed == 27) {
		kernel_timing_trigger(1, TRIGGER_ACTION);
		aainfo[1]._val3 = -1;
		aainfo[1]._frame = 5;
		last_keypressed = -1;
	}

	// Enter key: trigger play/continue
	if (last_keypressed == 13) {
		kernel_timing_trigger(1, TRIGGER_ACTION);
		aainfo[1]._val3 = -1;
		aainfo[1]._frame = 6;
		last_keypressed = -1;
	}
}

void room_904_synchronize(Common::Serializer &s) {
	for (int16 &v : ss)     s.syncAsSint16LE(v);
	for (int16 &v : seq)    s.syncAsSint16LE(v);
	for (int16 &v : aa)     s.syncAsSint16LE(v);
	for (auto &ai : aainfo) ai.synchronize(s);
}

void room_904_preload() {
	room_init_code_pointer = room_904_init;
	room_daemon_code_pointer = room_904_daemon;

	global[g016] = true;
	kernel.activate_menu = GAME_NO_MENU;
	section_9_walker();
	section_9_interface();
}

} // namespace Rooms
} // namespace Forest
} // namespace MADS
