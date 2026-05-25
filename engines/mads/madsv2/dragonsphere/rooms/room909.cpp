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
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/dragonsphere.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section9.h"
#include "mads/madsv2/dragonsphere/rooms/room909.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {


struct Scratch {
	int16 sprite[15];       // ss[] — series handles
	int16 sequence[15];     // seq[] — sequence handles
	int16 animation[4];     // aa[] — animation handles
	int16 prev_anim_frame;  // last observed animation frame (for frame-change detection)
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

/* ========================= Sprite Series =================== */

#define fx_bg0  0
#define fx_bg1  1
#define fx_bg2  2

/* ========================= Triggers ======================== */

#define TRIGGER_SHOW_SCORE  61  // fired by kernel_timing_trigger after frame 105; daemon shows score screen


/* ======================== Anim frames ====================== */

#define FRAME_HEARTBEAT_SOUND    52   // 0x34: play speech line 1 (or sound fallback)
#define FRAME_MUSIC_SOUND        82   // 0x52: play music sound (60) if music enabled
#define FRAME_TIMING_TRIGGER    105   // 0x69: arm 2-second delay for TRIGGER_SHOW_SCORE
#define FRAME_LOOP_RESET        106   // 0x6A: reset animation to FRAME_TIMING_TRIGGER


static void room_909_init() {
	player.commands_allowed = 0;
	player.walker_visible = 0;
	viewing_at_y = 22;
	kernel_init_dialog();
	kernel_set_interface_mode(2);

	ss[fx_bg0] = kernel_load_series(kernel_name('x', 0), 0);
	ss[fx_bg1] = kernel_load_series(kernel_name('x', 1), 0);
	ss[fx_bg2] = kernel_load_series(kernel_name('x', 2), 0);

	seq[fx_bg0] = kernel_seq_forward(ss[fx_bg0], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_bg0], 12);
	kernel_seq_range(seq[fx_bg0], -1, -2);

	seq[fx_bg1] = kernel_seq_forward(ss[fx_bg1], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_bg1], 12);
	kernel_seq_range(seq[fx_bg1], -1, -2);

	seq[fx_bg2] = kernel_seq_forward(ss[fx_bg2], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_bg2], 12);
	kernel_seq_range(seq[fx_bg2], -1, -2);

	aa[0] = kernel_run_animation(kernel_name('w', 1), 0);

	section_9_music();
}

/**
 * Monitors the 'w' animation for frame transitions and dispatches
 * sound/timing effects at specific frames.  When a looping reset is
 * needed (var_2 >= 0) the animation is jumped back and the frame
 * tracker is updated so the next call does not re-fire.
 *
 * Called once per tick from room_909_daemon.
 */
static void room_909_anim() {
	int16 frame;
	int   cur_frame = kernel_anim[aa[0]].frame;

	if (cur_frame == scratch.prev_anim_frame)
		return;

	frame = -1;
	scratch.prev_anim_frame = (int16)cur_frame;

	// Frame dispatch (ascending subtraction chain, mirroring original assembly)
	if (cur_frame == FRAME_LOOP_RESET) {
		// Frame 106: reset animation back to frame 105 so it loops continuously
		frame = FRAME_TIMING_TRIGGER;

	} else if (cur_frame < FRAME_LOOP_RESET) {
		int remaining = cur_frame;

		remaining -= FRAME_HEARTBEAT_SOUND;    // 52
		if (remaining == 0) {
			// Frame 52: play speech line 1 if speech system is active and enabled;
			// otherwise play the sound-effect fallback.
			// (word_845AE = speech_system_active, word_842A8 = speech_on,
			//  byte_106AF+9 = overlay thunk → global_speech_go)
			if (speech_system_active && speech_on)
				global_speech_go(1);
			else
				sound_play(61);
			goto apply;
		}

		remaining -= 30;                       // 52+30 = 82
		if (remaining == 0) {
			// Frame 82: play music sting only when music is enabled
			if (g_engine->_musicFlag)
				sound_play(60);
			goto apply;
		}

		remaining -= 23;                       // 52+30+23 = 105
		if (remaining == 0) {
			// Frame 105: arm the 2-second delay before showing the score screen
			kernel_timing_trigger(120, TRIGGER_SHOW_SCORE);
			goto apply;
		}
		// All other frames: no action, var_2 stays -1
	}
	// cur_frame > FRAME_LOOP_RESET: no action

apply:
	if (frame >= 0) {
		kernel_reset_animation(aa[0], frame);
		scratch.prev_anim_frame = frame;
	}
}

static void room_909_daemon() {
	room_909_anim();

	if (kernel.trigger != TRIGGER_SHOW_SCORE)
		return;

	// Clamp score to maximum of 250
	int score = global[player_score];
	if (score > 250)
		score = 250;

	// Populate text_index[] before calling text_show(99) so the score-screen
	// text can substitute the values via [INDEX N] commands.
	// text_index[0] = current score 
	// text_index[1] = maximum score
	// text_index[2] = rank tier 1-9
	text_index[1] = 250;
	text_index[0] = score;

	// Determine rank tier (mirrors Phantom room250 score screen but adds tier 9)
	int tier;
	if      (score <= 25)  tier = 1;
	else if (score <= 50)  tier = 2;
	else if (score <= 75)  tier = 3;
	else if (score <= 100) tier = 4;
	else if (score <= 150) tier = 5;
	else if (score <= 200) tier = 6;
	else if (score <= 249) tier = 7;
	else if (score <= 250) tier = 8;  // exactly 250
	else                   tier = 9;  // unreachable in normal play

	text_index[2] = tier;

	text_show(99);
}

static void room_909_pre_parser() {
}

static void room_909_parser() {
}

void room_909_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(ss[fx_bg0]);
	s.syncAsSint16LE(ss[fx_bg1]);
	s.syncAsSint16LE(ss[fx_bg2]);
	s.syncAsSint16LE(seq[fx_bg0]);
	s.syncAsSint16LE(seq[fx_bg1]);
	s.syncAsSint16LE(seq[fx_bg2]);
	s.syncAsSint16LE(aa[0]);
	s.syncAsSint16LE(scratch.prev_anim_frame);
}

void room_909_preload() {
	room_init_code_pointer       = room_909_init;
	room_pre_parser_code_pointer = room_909_pre_parser;
	room_parser_code_pointer     = room_909_parser;
	room_daemon_code_pointer     = room_909_daemon;

	section_9_walker();
	section_9_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
