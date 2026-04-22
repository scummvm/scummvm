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
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/rooms/section2.h"
#include "mads/madsv2/phantom/rooms/room250.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_250_init(void) {
	viewing_at_y = ((video_y - display_y) >> 1);

	player.commands_allowed = false;
	player.walker_visible = false;

	kernel.quotes = quote_load(quote_score_1, quote_score_2,
		quote_score_3, quote_score_4,
		quote_score_rank_1,
		quote_score_rank_2,
		quote_score_rank_3,
		quote_score_rank_4,
		quote_score_rank_5,
		quote_score_rank_6,
		quote_score_rank_7,
		quote_score_rank_8, 0);

	aa[0] = kernel_run_animation("*RM150Q1", 1);
}


void room_250_daemon(void) {
	int score;
	int id;
	int y;
	static char message[40];

	if (kernel.trigger == 1) {
		kernel_timing_trigger(12, 2);
	}

	if (kernel.trigger == 2) {
		y = 68;

		kernel_message_add(quote_string(kernel.quotes, quote_score_1),
			video_x >> 1, y, MESSAGE_COLOR,
			FIFTEEN_SECONDS, 0,
			KERNEL_MESSAGE_CENTER);
		y += 16;

		if (global[player_score] > 250) global[player_score] = 250;

		mads_itoa(global[player_score], message, 10);
		Common::strcat_s(message, " ");
		Common::strcat_s(message, quote_string(kernel.quotes, quote_score_2));
		Common::strcat_s(message, " 250 ");
		Common::strcat_s(message, quote_string(kernel.quotes, quote_score_3));

		kernel_message_add(message,
			video_x >> 1, y, MESSAGE_COLOR,
			FIFTEEN_SECONDS, 3,
			KERNEL_MESSAGE_CENTER);
		y += 16;

		kernel_message_add(quote_string(kernel.quotes, quote_score_4),
			video_x >> 1, y, MESSAGE_COLOR,
			FIFTEEN_SECONDS, 0,
			KERNEL_MESSAGE_CENTER);
		y += 16;

		score = global[player_score];

		if (score <= 25) {
			id = quote_score_rank_1;    /* Stage sweeper */
		} else if (score <= 50) {
			id = quote_score_rank_2;    /* Dresser */
		} else if (score <= 75) {
			id = quote_score_rank_3;    /* Usher */
		} else if (score <= 100) {
			id = quote_score_rank_4;    /* Stagehand */
		} else if (score <= 150) {
			id = quote_score_rank_5;    /* Chorus Member */
		} else if (score <= 200) {
			id = quote_score_rank_6;    /* Supporting Player */
		} else if (score <= 249) {
			id = quote_score_rank_7;    /* Star Player */
		} else {
			id = quote_score_rank_8;    /* Director */
		}

		kernel_message_add(quote_string(kernel.quotes, id),
			video_x >> 1, y, MESSAGE_COLOR,
			FIFTEEN_SECONDS, 0,
			KERNEL_MESSAGE_CENTER);

		kernel_timing_trigger(FIFTEEN_SECONDS + HALF_SECOND, 3);
	}

	if (kernel.trigger == 3) {
		game.going = false;
		win_status = 1;
	}
}

void room_250_preload(void) {
	room_init_code_pointer = room_250_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_daemon_code_pointer = room_250_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
