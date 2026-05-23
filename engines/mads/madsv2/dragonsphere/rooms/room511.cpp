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
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section5.h"
#include "mads/madsv2/dragonsphere/rooms/room511.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
	int16 death_frame;
	int16 speech_playing;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation


static void room_511_init() {
	viewing_at_y = ((video_y - display_y) >> 1);
	kernel_init_dialog();
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	player.commands_allowed = false;
	player.walker_visible   = false;
	local->speech_playing   = false;

	aa[0] = kernel_run_animation(kernel_name('p', 1), 0);

	section_5_music();
}

static void room_511_daemon() {
	int death_reset_frame;

	if (kernel_anim[aa[0]].frame != local->death_frame) {
		local->death_frame = kernel_anim[aa[0]].frame;
		death_reset_frame = -1;

		switch (local->death_frame) {

		case 19:
			if (speech_system_active && speech_on) {
				global_speech_go(5);
				local->speech_playing = 5;
			}
			break;

		case 33:
			if (local->speech_playing == 5) {
				if (speech_system_active && speech_on) {
					if (speech_status()) {
						death_reset_frame = 32;

					} else {
						global_speech_go(2);
						local->speech_playing = 2;
						death_reset_frame = 32;
					}
				}

			} else if (local->speech_playing == 2) {
				if (speech_system_active && speech_on) {
					if (speech_status()) {
						death_reset_frame = 32;

					} else {
						new_room = 505;
					}
				}

			} else {
				new_room = 505;
			}
			break;
		}

		if (death_reset_frame >= 0) {
			kernel_reset_animation(aa[0], death_reset_frame);
			local->death_frame = death_reset_frame;
		}
	}
}

static void room_511_pre_parser() {
}

static void room_511_parser() {
}

void room_511_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.death_frame);
	s.syncAsSint16LE(scratch.speech_playing);
}

void room_511_preload() {
	room_init_code_pointer       = room_511_init;
	room_pre_parser_code_pointer = room_511_pre_parser;
	room_parser_code_pointer     = room_511_parser;
	room_daemon_code_pointer     = room_511_daemon;

	section_5_walker();
	section_5_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
