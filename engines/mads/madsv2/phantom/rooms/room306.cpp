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
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section3.h"
#include "mads/madsv2/phantom/rooms/room306.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_306_init() {
	/* ===================== Load Sprite Series ================== */

	global_speech_load(speech_raoul_catwalk);

	local->prevent = false;

	viewing_at_y = ((video_y - display_y) >> 1);

	player.walker_visible = false;
	player.commands_allowed = false;

	aa[0] = kernel_run_animation(kernel_name('a', 1), ROOM_306_END);

	section_3_music();
}

void room_306_daemon() {
	if (kernel.trigger == ROOM_306_END) {
		if (global_prefer_roland) {
			kernel_timing_trigger(TWO_SECONDS, ROOM_306_END + 1);
		} else {
			kernel_timing_trigger(FIVE_SECONDS, ROOM_306_END + 1);
		}
	}

	if (!local->prevent) {
		if (kernel_anim[aa[0]].frame == 3) {
			global_speech_go(speech_raoul_catwalk);
			local->prevent = true;
		}
	}

	if (kernel.trigger == ROOM_306_END + 1) {
		sound_play(N_AllFade);
		sound_play(N_Crash003);
		kernel_timing_trigger(TWO_SECONDS, ROOM_306_END + 2);
	}

	if (kernel.trigger == ROOM_306_END + 2) {
		new_room = 150;
	}
}

void room_306_preload() {
	room_init_code_pointer = room_306_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_daemon_code_pointer = room_306_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
