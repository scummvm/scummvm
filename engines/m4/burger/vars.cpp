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

#include "m4/burger/vars.h"
#include "m4/core/term.h"
#include "m4/adv_r/adv.h"
#include "m4/adv_r/adv_inv.h"

namespace M4 {
namespace Burger {

#define GLOBAL_VARS_COUNT 512

Vars::Vars() {
	resize(GLOBAL_VARS_COUNT);
}

void Vars::reset() {
	Common::fill(&(*this)[0], &(*this)[0] + GLOBAL_VARS_COUNT, 0);
}

int32 Vars::get_boonsville_time_and_display(bool showTime) {
	if (showTime) {
		int time = (*this)[BOONSVILLE_TIME];
		int seconds = time % 60;
		time /= 60;
		int minutes = time % 60;
		time /= 60;

		term_message("Boonsville time: %d:%d:%d", time, minutes, seconds);
	}

	return (*this)[BOONSVILLE_TIME];
}

void Vars::set_boonsville_time(int32 time) {
	(*this)[BOONSVILLE_TIME] = time;
	(*this)[BOONSVILLE_TIME2] = time - 1;
}

void Vars::reset1() {
	set_boonsville_time(0);
	(*this)[V000] = 1000;
	inv_give_to_player("money");

	(*this)[V001] = 20;
	(*this)[V005] = 0;
	(*this)[V010] = 0;
	(*this)[V011] = 0;
	(*this)[V012] = 0;
	(*this)[V013] = 0;
	(*this)[V014] = 0;
	(*this)[V015] = 0;
	(*this)[V017] = 0;
	(*this)[V018] = 0;
	(*this)[V019] = 0;
	(*this)[V023] = 0;
	(*this)[V027] = 0;
	(*this)[V028] = 0;
	(*this)[V031] = 0;
	(*this)[V032] = 0;
	(*this)[V033] = 2;
	(*this)[V034] = 0;

	inv_move_object("deed", 105);

	(*this)[V172] = 10023;
	(*this)[V037] = 1;
	(*this)[V039] = 0;
	(*this)[V043] = 0;
	(*this)[V046] = 0;
	(*this)[V047] = 0;
	(*this)[V048] = 0;
	(*this)[V049] = 0;

	inv_move_object("jawz o' life", 137);
	inv_move_object("keys", 138);

	(*this)[V112] = 0;
	(*this)[V058] = 0;
	(*this)[V059] = 0;
	(*this)[V245] = 10027;

	inv_move_object("pantyhose", 142);
	inv_move_object("block of ice", 142);
	inv_move_object("broken mouse trap", 143);
	inv_move_object("spring", NOWHERE);
	inv_move_object("amplifier", 145);

	(*this)[V079] = 1;
	inv_move_object("phone bill", 170);
	inv_move_object("whistle", 170);

	(*this)[V083] = 0;
	(*this)[V084] = 0;
	(*this)[V085] = 0;
	(*this)[V086] = 0;
	(*this)[V087] = 0;
	(*this)[V088] = 0;
	(*this)[V092] = 0;
	(*this)[V091] = 0;

	inv_move_object("carrot juice", 172);
	inv_move_object("laxative", 174);
	inv_move_object("broken puz dispenser", 176);
	inv_move_object("puz dispenser", NOWHERE);

	conv_reset_all();
}

} // namespace Burger
} // namespace M4
