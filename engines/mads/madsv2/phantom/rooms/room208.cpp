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
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/rooms/section2.h"
#include "mads/madsv2/phantom/rooms/room208.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void animate_top_left() {
	int delay;
	int random;
	int random_2;
	int chance;

	if (kernel.trigger == ROOM_208_MOVE_TOP_LEFT) {
		kernel_seq_delete(seq[fx_top_left]);

		random_2 = imath_random(1, 2);
		if (local->top_left_frame == 3) {
			chance = 1;
		} else {
			chance = random_2;
		} /* this will keep the person looking at the stage more often */

		if (random_2 == chance) {
			random = imath_random(-1, 1);
			local->top_left_frame += random;
			if (local->top_left_frame == 0) {
				local->top_left_frame = 1;
			} else if (local->top_left_frame == 4) {
				local->top_left_frame = 3;
			}
		}

		seq[fx_top_left] = kernel_seq_stamp(ss[fx_top_left], false, local->top_left_frame);
		kernel_seq_depth(seq[fx_top_left], 1);
		delay = imath_random(QUARTER_SECOND, ONE_SECOND);
		kernel_timing_trigger(delay, ROOM_208_MOVE_TOP_LEFT);
	}
}

static void animate_top_right() {
	int delay;
	int random;
	int random_2;
	int chance;

	if (kernel.trigger == ROOM_208_MOVE_TOP_RIGHT) {
		kernel_seq_delete(seq[fx_top_right]);

		random_2 = imath_random(1, 2);
		if (local->top_right_frame == 4) {
			chance = 1;
		} else {
			chance = random_2;
		} /* this will keep the person looking at the stage more often */

		if (random_2 == chance) {
			random = imath_random(-1, 1);
			local->top_right_frame += random;
			if (local->top_right_frame == 0) {
				local->top_right_frame = 1;
			} else if (local->top_right_frame == 5) {
				local->top_right_frame = 4;
			}
		}

		seq[fx_top_right] = kernel_seq_stamp(ss[fx_top_right], false, local->top_right_frame);
		kernel_seq_depth(seq[fx_top_right], 1);
		delay = imath_random(QUARTER_SECOND, ONE_SECOND);
		kernel_timing_trigger(delay, ROOM_208_MOVE_TOP_RIGHT);
	}
}

static void animate_middle_left() {
	int delay;
	int random;
	int random_2;
	int chance;

	if (kernel.trigger == ROOM_208_MOVE_MIDDLE_LEFT) {
		kernel_seq_delete(seq[fx_middle_left]);
		delay = imath_random(ONE_SECOND, TWO_SECONDS);

		random_2 = imath_random(1, 2);
		if (local->middle_left_frame == 2) {
			chance = 1;
		} else {
			chance = random_2;
		} /* this will keep the person looking at the stage more often */

		if (random_2 == chance) {
			random = imath_random(-1, 1);
			local->middle_left_frame += random;
			if (local->middle_left_frame == 0) {
				local->middle_left_frame = 1;
			} else if (local->middle_left_frame == 5) {
				local->middle_left_frame = 4;
			}
		}

		if (local->middle_middle_frame == 3) { /* if man is hunched over, whispering */
			if (local->middle_left_frame < 4) {
				++local->middle_left_frame;
				delay = QUICKLY;
			}
		} /* if man is whispering, move left woman's head to the couple (she's annoyed) */

		seq[fx_middle_left] = kernel_seq_stamp(ss[fx_middle_left], false, local->middle_left_frame);
		kernel_seq_depth(seq[fx_middle_left], 1);
		kernel_timing_trigger(delay, ROOM_208_MOVE_MIDDLE_LEFT);
	}
}

static void animate_middle_middle() {
	int delay;

	if (kernel.trigger == ROOM_208_MOVE_MIDDLE_MIDDLE) {
		kernel_seq_delete(seq[fx_middle_middle]);

		if (local->middle_direction) { /* if going to whisper */
			++local->middle_middle_frame;
			delay = QUARTER_SECOND;
			if (local->middle_middle_frame == 4) {  /* whispering */
				delay = imath_random(FIVE_SECONDS, SEVEN_SECONDS);
				--local->middle_middle_frame;
				local->middle_direction = 0;
			}
		} else {  /* if returning to watch play */
			--local->middle_middle_frame;
			delay = QUARTER_SECOND;
			if (local->middle_middle_frame == 0) {  /* standing up */
				delay = imath_random(TEN_SECONDS, FIFTEEN_SECONDS);
				++local->middle_middle_frame;
				local->middle_direction = 1;
			}
		}

		seq[fx_middle_middle] = kernel_seq_stamp(ss[fx_middle_middle], false, local->middle_middle_frame);
		kernel_seq_depth(seq[fx_middle_middle], 14);
		kernel_timing_trigger(delay, ROOM_208_MOVE_MIDDLE_MIDDLE);
	}
}

static void animate_middle_right() {
	int delay;
	int random;
	int random_2;
	int chance;

	if (kernel.trigger == ROOM_208_MOVE_MIDDLE_RIGHT) {
		kernel_seq_delete(seq[fx_middle_right]);

		random_2 = imath_random(1, 2);
		if (local->middle_right_frame == 1) {
			chance = 1;
		} else {
			chance = random_2;
		} /* this will keep the person looking at the stage more often */

		if (random_2 == chance) {
			random = imath_random(-1, 1);
			local->middle_right_frame += random;
			if (local->middle_right_frame == 0) {
				local->middle_right_frame = 1;
			} else if (local->middle_right_frame == 4) {
				local->middle_right_frame = 3;
			}

			if (local->middle_middle_frame == 3) {
				local->middle_right_frame = 3;
			} /* if man is whispering to woman, then freeze woman at frame 3 (listening to man) */
		}

		seq[fx_middle_right] = kernel_seq_stamp(ss[fx_middle_right], false, local->middle_right_frame);
		kernel_seq_depth(seq[fx_middle_right], 1);
		delay = imath_random(QUARTER_SECOND, ONE_SECOND);
		kernel_timing_trigger(delay, ROOM_208_MOVE_MIDDLE_RIGHT);
	}
}

static void animate_bottom_left() {
	int delay;
	int random;
	int random_2;
	int chance;

	if (kernel.trigger == ROOM_208_MOVE_BOTTOM_LEFT) {
		kernel_seq_delete(seq[fx_bottom_left]);

		random_2 = imath_random(1, 2);
		if (local->bottom_left_frame == 4) {
			chance = 1;
		} else {
			chance = random_2;
		} /* this will keep the person looking at the stage more often */

		if (random_2 == chance) {
			random = imath_random(-1, 1);
			local->bottom_left_frame += random;
			if (local->bottom_left_frame == 0) {
				local->bottom_left_frame = 1;
			} else if (local->bottom_left_frame == 5) {
				local->bottom_left_frame = 4;
			}
		}

		seq[fx_bottom_left] = kernel_seq_stamp(ss[fx_bottom_left], false, local->bottom_left_frame);
		kernel_seq_depth(seq[fx_bottom_left], 1);
		delay = imath_random(QUARTER_SECOND, ONE_SECOND);
		kernel_timing_trigger(delay, ROOM_208_MOVE_BOTTOM_LEFT);
	}
}

static void animate_bottom_middle() {
	int delay;
	int random;
	int random_2;
	int chance;

	if (kernel.trigger == ROOM_208_MOVE_BOTTOM_MIDDLE) {
		kernel_seq_delete(seq[fx_bottom_middle]);

		random_2 = imath_random(1, 2);
		if (local->bottom_middle_frame == 4) {
			chance = 1;
		} else {
			chance = random_2;
		} /* this will keep the person looking at the stage more often */

		if (random_2 == chance) {
			random = imath_random(-1, 1);
			local->bottom_middle_frame += random;
			if (local->bottom_middle_frame == 0) {
				local->bottom_middle_frame = 1;
			} else if (local->bottom_middle_frame == 5) {
				local->bottom_middle_frame = 4;
			}
		}

		seq[fx_bottom_middle] = kernel_seq_stamp(ss[fx_bottom_middle], false, local->bottom_middle_frame);
		kernel_seq_depth(seq[fx_bottom_middle], 14);
		delay = imath_random(QUARTER_SECOND, ONE_SECOND);
		kernel_timing_trigger(delay, ROOM_208_MOVE_BOTTOM_MIDDLE);
	}
}

static void animate_bottom_right() {
	int delay;
	int random;
	int random_2;
	int chance;

	if (kernel.trigger == ROOM_208_MOVE_BOTTOM_RIGHT) {
		kernel_seq_delete(seq[fx_bottom_right]);

		random_2 = imath_random(1, 2);
		if ((local->bottom_right_frame == 3) || (local->bottom_right_frame == 1)) {
			chance = 1;
		} else {
			chance = random_2;
		} /* this will keep the person looking at the stage more often */

		if (random_2 == chance) {
			random = imath_random(-1, 1);
			local->bottom_right_frame += random;
			if (local->bottom_right_frame == 0) {
				local->bottom_right_frame = 1;
			} else if (local->bottom_right_frame == 4) {
				local->bottom_right_frame = 3;
			}
		}

		seq[fx_bottom_right] = kernel_seq_stamp(ss[fx_bottom_right], false, local->bottom_right_frame);
		kernel_seq_depth(seq[fx_bottom_right], 1);
		delay = imath_random(QUARTER_SECOND, ONE_SECOND);
		kernel_timing_trigger(delay, ROOM_208_MOVE_BOTTOM_RIGHT);
	}
}

void room_208_daemon() {
	animate_top_left();
	animate_top_right();
	animate_middle_left();
	animate_middle_middle();
	animate_middle_right();
	animate_bottom_left();
	animate_bottom_middle();
	animate_bottom_right();

	if (!local->prevent) {
		if (kernel_anim[aa[0]].frame == 49) {
			if (global_prefer_roland) {
				sound_play(N_WomanScream002);
			} else {
				global_speech_go(speech_woman_scream);
			}
			local->prevent = true;
		}
	}

	if (!local->prevent_2) {
		if (kernel_anim[aa[0]].frame == 68) {
			if (global_prefer_roland) {
				sound_play(N_WomanScream002);
			} else {
				global_speech_go(speech_woman_scream);
			}
			local->prevent_2 = true;
		}
	}

	if (kernel.trigger == ROOM_208_DONE_PHANTOM) {
		new_room = 150;
	}
}

void room_208_init() {
	global_speech_load(speech_woman_scream);

	local->prevent   = false;
	local->prevent_2 = false;

	/* Load Sprite Series */

	ss[fx_top_left]      = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_top_right]     = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_middle_left]   = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_middle_middle] = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_middle_right]  = kernel_load_series(kernel_name('x', 4), false);
	ss[fx_bottom_left]   = kernel_load_series(kernel_name('x', 5), false);
	ss[fx_bottom_middle] = kernel_load_series(kernel_name('x', 6), false);
	ss[fx_bottom_right]  = kernel_load_series(kernel_name('x', 7), false);

	kernel_init_dialog();
	kernel_set_interface_mode(INTER_CONVERSATION);

	global_speech_load(speech_woman_scream); /* called twice in original source */

	player.commands_allowed = false;
	player.walker_visible   = false;


	/* Deal with top left person */

	local->top_left_frame = 2;
	seq[fx_top_left] = kernel_seq_stamp(ss[fx_top_left], false, local->top_left_frame);
	kernel_seq_depth(seq[fx_top_left], 1);
	kernel_timing_trigger(TWO_SECONDS, ROOM_208_MOVE_TOP_LEFT);


	/* Deal with top right person */

	local->top_right_frame = 2;
	seq[fx_top_right] = kernel_seq_stamp(ss[fx_top_right], false, local->top_right_frame);
	kernel_seq_depth(seq[fx_top_right], 1);
	kernel_timing_trigger(HALF_SECOND, ROOM_208_MOVE_TOP_RIGHT);


	/* Deal with middle left person */

	local->middle_left_frame = 2;
	seq[fx_middle_left] = kernel_seq_stamp(ss[fx_middle_left], false, local->middle_left_frame);
	kernel_seq_depth(seq[fx_middle_left], 1);
	kernel_timing_trigger(HALF_SECOND, ROOM_208_MOVE_MIDDLE_LEFT);


	/* Deal with middle middle person */

	local->middle_middle_frame = 1;
	local->middle_direction    = 1;
	seq[fx_middle_middle] = kernel_seq_stamp(ss[fx_middle_middle], false, local->middle_middle_frame);
	kernel_seq_depth(seq[fx_middle_middle], 14);
	kernel_timing_trigger(FIVE_SECONDS, ROOM_208_MOVE_MIDDLE_MIDDLE);


	/* Deal with middle right person */

	local->middle_right_frame = 3;
	seq[fx_middle_right] = kernel_seq_stamp(ss[fx_middle_right], false, local->middle_right_frame);
	kernel_seq_depth(seq[fx_middle_right], 1);
	kernel_timing_trigger(ONE_SECOND, ROOM_208_MOVE_MIDDLE_RIGHT);


	/* Deal with bottom left person */

	local->bottom_left_frame = 4;
	seq[fx_bottom_left] = kernel_seq_stamp(ss[fx_bottom_left], false, local->bottom_left_frame);
	kernel_seq_depth(seq[fx_bottom_left], 1);
	kernel_timing_trigger(ONE_SECOND, ROOM_208_MOVE_BOTTOM_LEFT);


	/* Deal with bottom middle person */

	local->bottom_middle_frame = 4;
	seq[fx_bottom_middle] = kernel_seq_stamp(ss[fx_bottom_middle], false, local->bottom_middle_frame);
	kernel_seq_depth(seq[fx_bottom_middle], 14);
	kernel_timing_trigger(HALF_SECOND, ROOM_208_MOVE_BOTTOM_MIDDLE);


	/* Deal with bottom right person */

	local->bottom_right_frame = 3;
	seq[fx_bottom_right] = kernel_seq_stamp(ss[fx_bottom_right], false, local->bottom_right_frame);
	kernel_seq_depth(seq[fx_bottom_right], 1);
	kernel_timing_trigger(QUARTER_SECOND, ROOM_208_MOVE_BOTTOM_RIGHT);


	aa[0] = kernel_run_animation(kernel_name('p', 1), ROOM_208_DONE_PHANTOM);
	/* run anim of Phantom swinging down and taking Christine */

	section_2_music();
}

void room_208_preload() {
	room_init_code_pointer = room_208_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_daemon_code_pointer = room_208_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
