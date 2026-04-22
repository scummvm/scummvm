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
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section3.h"
#include "mads/madsv2/phantom/rooms/room305.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_305_init() {
	local->unmask = false;
	local->prevent = false;
	player.walker_visible = false;
	local->anim_0_running = false;
	local->anim_1_running = false;

	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	global_speech_load(speech_raoul_strangle);

	/* viewing_at_y = ((video_y - display_y) >> 1); */

	kernel.quotes = quote_load(quote_305a0, quote_305a1, 0);

	if (player.x == 100) {
		aa[0] = kernel_run_animation(kernel_name('r', 1), ROOM_305_DEAD);
		kernel_flip_hotspot(words_mask, false);
		local->anim_1_running = true;

	} else if (player.x == 200) {
		aa[0] = kernel_run_animation(kernel_name('u', 1), 0);
		local->anim_0_running = true;
		kernel_flip_hotspot(words_cane, false);
	}

	section_3_music();
}

void room_305_parser() {
	if (player_said_2(push, cane)) {
		new_room = 304;
		goto handled;
	}

	if (player_said_2(take, mask)) {
		local->unmask = true;
		player.commands_allowed = false;
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

static void handle_animation_unmask() {
	int unmask_reset_frame;
	/* int id; */

	if (kernel_anim[aa[0]].frame != local->unmask_frame) {
		local->unmask_frame = kernel_anim[aa[0]].frame;
		unmask_reset_frame = -1;

		switch (local->unmask_frame) {
		case 25:  /* end of struggle with phantom masked */
			if (!local->unmask) {
				unmask_reset_frame = 0;
			}
			break;

		case 60:  /* somewhere when we can see phantom's face */
			global_speech(10);

			kernel_message_add(quote_string(kernel.quotes, quote_305a0),
				176, 53, MESSAGE_COLOR, SIX_SECONDS, 0, 0);
			kernel_message_add(quote_string(kernel.quotes, quote_305a1),
				176, 68, MESSAGE_COLOR, SIX_SECONDS, 0, 0);
			break;

		case 95:  /* end of unmasking */
			new_room = 306;
			break;
		}

		if (unmask_reset_frame >= 0) {
			kernel_reset_animation(aa[0], unmask_reset_frame);
			local->unmask_frame = unmask_reset_frame;
		}
	}
}

void room_305_daemon() {
	if (local->anim_0_running) {
		handle_animation_unmask();
	}

	if (local->anim_1_running) {
		if (kernel_anim[aa[0]].frame == 53) {
			player.commands_allowed = false;
		}

		if (kernel_anim[aa[0]].frame == 54 && !local->prevent) {
			global_speech_go(speech_raoul_strangle);
			local->prevent = true;
		}
	}

	if (kernel.trigger == ROOM_305_DEAD) {
		global[player_score] -= 10;
		inter_turn_off_object();
		inter_screen_update();
		new_room = 303;
	}
}

void room_305_preload() {
	room_init_code_pointer = room_305_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = room_305_parser;
	room_daemon_code_pointer = room_305_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
