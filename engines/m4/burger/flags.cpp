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

#include "m4/burger/flags.h"
#include "m4/core/term.h"
#include "m4/adv_r/adv.h"
#include "m4/adv_r/adv_inv.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {

struct BoonsvilleEvent {
	int32 _time;
	int32 _trigger;
	const char *_text;
};

static const BoonsvilleEvent EVENTS[] = {
	{ 600, -1, "After getting neuro stuff" },
	{ 2400, -1, "After getting survival stuff" },
	{ 6000, kBurlEntersTown, "Burl enters town" },
	{ 6600, -1, "After getting sensory stuff" },
	{ 9600, kBurlGetsFed, "Burl gets fed" },
	{ 19200, -1, "After getting language stuff" },
	{ 13200, kBurlStopsEating, "Burl stops eating" },
	{ 22800, -1, "After getting logic stuff" },
	{ 15000, kBurlLeavesTown, "Burl leaves town" },
	{ 18600, kBandEntersTown, "Band enters town" },
	{ 33600, kAbduction, "Abduction time" },
	{ 55200, kTestTimeout, "Nero timeout" },
	{ 166800, kTestTimeout, "Survival timeout" },
	{ 278400, kTestTimeout, "Sensory timeout" },
	{ 390000, kTestTimeout, "Language timeout" },
	{ 501600, kTestTimeout, "Logic timeout" },
	{ 505200, 10033, "Conclusion" },
	{ 0, 0, nullptr }
};

int32 Flags::_flags[FLAGS_COUNT];

Flags::Flags() {
	Common::fill(_flags, _flags + FLAGS_COUNT, 0);
}

void Flags::sync(Common::Serializer &s) {
	size_t count = size();
	for (uint i = 0; i < count; ++i)
		s.syncAsSint32LE(_flags[i]);
}

void Flags::reset() {
	Common::fill(_flags, _flags + FLAGS_COUNT, 0);
}

int32 Flags::get_boonsville_time_and_display(bool showTime) {
	if (showTime) {
		int time = (*this)[kBoonsvilleTime];
		int seconds = time % 60;
		time /= 60;
		int minutes = time % 60;
		time /= 60;

		term_message("Boonsville time: %d:%d:%d", time, minutes, seconds);
	}

	return (*this)[kBoonsvilleTime];
}

void Flags::set_boonsville_time(int32 time) {
	(*this)[kBoonsvilleTime] = time;
	(*this)[kBoonsvillePriorTime] = time - 1;
}

bool Flags::advance_boonsville_time_and_check_schedule(int32 time) {
	if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
		(*this)[kBoonsvillePriorTime] = (*this)[kBoonsvilleTime];
		(*this)[kBoonsvilleTime] += time;
		return dispatch_scheduled_boonsville_time_trigger(
			get_boonsville_time_and_display());
	} else {
		return false;
	}
}

bool Flags::dispatch_scheduled_boonsville_time_trigger(int32 time) {
	KernelTriggerType oldMode = _G(kernel).trigger_mode;
	_G(kernel).trigger_mode = KT_DAEMON;
	bool result = false;

	for (const BoonsvilleEvent *rec = EVENTS; rec->_time; ++rec) {
		if ((*this)[kBoonsvilleTime] >= rec->_time &&
				(*this)[kBoonsvillePriorTime] < rec->_time) {
			result = true;
			term_message("Time for: %s", rec->_text);
			schedule_boonsville_time();
			kernel_trigger_dispatch_now(rec->_trigger);
		}
	}

	_G(kernel).trigger_mode = oldMode;
	return result;
}

void Flags::schedule_boonsville_time() {
	int theTime = get_boonsville_time_and_display();
	int hours = theTime / 216000;
	int minutes = (theTime % 216000) / 3600;
	int seconds = (theTime % 3600) / 60;
	bool flag = false;

	term_message("************  Schedule  ************");

	for (const auto &te : EVENTS) {
		int teHours = te._time / 216000;
		int teMinutes = (te._time % 216000) / 3600;
		int teSeconds = (te._time % 3600) / 60;

		if (te._time <= theTime) {
			term_message("done    %1d:%2d:%2d  %s", teHours, teMinutes, teSeconds, te._text);
		} else {
			if (!flag) {
				term_message("------> %1d:%2d:%2d  Current time", hours, minutes, seconds);
				flag = true;
			}

			term_message("pending %1d:%2d:%2d  %s", teHours, teMinutes, teSeconds, te._text);
		}
	}

	term_message("************************************");
}

void Flags::reset1() {
	set_boonsville_time(0);
	(*this)[V000] = 1000;

	if (_G(executing) == WHOLE_GAME)
		inv_give_to_player("money");
	else
		inv_move_object("money", NOWHERE);

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
	(*this)[kHarryComingToRoof] = 0;
	(*this)[WAKE_UP_STOLIE_COUNT] = 0;
	(*this)[V028] = 0;
	(*this)[V031] = 0;
	(*this)[V032] = 0;
	(*this)[V033] = 2;
	(*this)[V034] = 0;

	inv_move_object("deed", 105);

	(*this)[V172] = 10023;
	(*this)[V037] = 1;
	(*this)[V039] = 0;
	(*this)[kRoadOpened] = 0;
	(*this)[V046] = 0;
	(*this)[kPoliceState] = 0;
	(*this)[kPoliceCheckCtr] = 0;
	(*this)[V049] = 0;

	inv_move_object("jawz o' life", 137);
	inv_move_object("keys", 138);

	(*this)[kPerkinsLostIsland] = 0;
	(*this)[kTourBusAtDiner] = 0;
	(*this)[kIceBoxOpened] = 0;
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

void Flags::reset2() {
	if ((*this)[V245] != 10027)
		inv_give_to_player("BLOCK OF ICE");
	if ((*this)[V277] == 6003)
		inv_give_to_player("PANTYHOSE");

	(*this)[kHampsterState] = 6000;
	(*this)[V244] = 6000;
	(*this)[V245] = 10027;
	(*this)[V246] = 0;
	(*this)[V255] = 0;
	(*this)[kGerbilCageDoor] = 0;
	(*this)[V257] = 0;
	(*this)[V258] = 0;
	(*this)[V263] = 0;
	(*this)[V265] = 0;
	(*this)[V266] = 0;
	(*this)[V269] = 1;
	(*this)[V270] = 6000;
	(*this)[V273] = 0;
	(*this)[V274] = 0;
	(*this)[V277] = 6000;
	(*this)[V278] = 0;

	if (_G(executing) != WHOLE_GAME) {
		inv_move_object("RAY GUN", NOWHERE);
		inv_give_to_player("BLOCK OF ICE");
		inv_give_to_player("PANTYHOSE");
	}
}

void Flags::reset3() {
	if (inv_player_has("BOTTLE")) {
		inv_give_to_player("CARROT JUICE");
		inv_move_object("BOTTLE", NOWHERE);
	}

	(*this)[V107] = 0;
	(*this)[kTrufflesInMine] = 0;
	inv_move_object("BURGER MORSEL", NOWHERE);

	(*this)[V117] = 0;
	(*this)[kDrunkCarrotJuice] = 0;
	(*this)[V118] = 3001;
	(*this)[V119] = 0;
	(*this)[V120] = 0;
	(*this)[V121] = 3001;
	(*this)[V122] = 0;
	(*this)[kDrunkCarrotJuice] = 0;

	inv_move_object("JUG", 303);
	inv_move_object("DISTILLED CARROT JUICE", NOWHERE);

	(*this)[V130] = 0;
	inv_move_object("MATCHES", 304);

	(*this)[V134] = 0;
	(*this)[V140] = 0;
	(*this)[kTrufflesRanAway] = 0;
	(*this)[V145] = 0;
}

void Flags::reset4() {
	(*this)[V186] = 0;
	(*this)[V234] = 0;
	(*this)[V196] = 0;
	(*this)[V197] = 0;
	(*this)[kFireplaceHasFire] = 0;
	(*this)[V199] = 0;
	(*this)[kStairsBorkState] = 5000;

	inv_move_object("KINDLING", 502);
	inv_move_object("BURNING KINDLING", 502);
	inv_move_object("GIZMO", 502);

	(*this)[kBORK_STATE] = 0;
	(*this)[V204] = 5000;
	(*this)[V205] = 0;
	(*this)[V207] = 0;

	inv_move_object("ROLLING PIN", 503);
	inv_move_object("RUBBER GLOVES", 503);

	(*this)[V210] = 5000;
	(*this)[V211] = 5000;
	(*this)[V212] = 5001;
	(*this)[V213] = 2;
	(*this)[V214] = 0;

	inv_move_object("DIRTY SOCK", 504);

	(*this)[V218] = 5000;
	(*this)[kTVOnFire] = 0;
	(*this)[V223] = 0;
	(*this)[V224] = 0;

	inv_move_object("RUBBER DUCKY", 507);

	if (inv_player_has("SOAPY WATER")) {
		inv_move_object("SOAPY WATER", 507);
		inv_give_to_player("BOTTLE");
	}

	(*this)[V227] = 0;
	(*this)[V228] = 0;
	(*this)[V229] = 1;

	inv_move_object("CHRISTMAS LIGHTS", 508);
	inv_move_object("CHRISTMAS LIGHTS ", 508);

	(*this)[V237] = 0;
}

void Flags::reset5() {
	if ((*this)[V159])
		inv_give_to_player("DEED");

	(*this)[V157] = 0;
	(*this)[V158] = 0;
	(*this)[V159] = 0;
	(*this)[V162] = 0;

	inv_move_object("FISH", 402);
	inv_move_object("QUARTER", 404);

	(*this)[V175] = 0;
	(*this)[V171] = 4000;
	(*this)[V172] = 10023;
	(*this)[V173] = 0;
	(*this)[V174] = 4000;
	(*this)[V176] = 0;

	inv_move_object("COLLAR", 406);
	inv_move_object("HOOK", 406);

	(*this)[V166] = 0;
	(*this)[V168] = 0;
	(*this)[V167] = 0;

	inv_move_object("RECORDS", 405);

	(*this)[V180] = 0;
	(*this)[V181] = 0;
}

void Flags::conv_reset_all() {
	_GC(convSave).clear();
}

const char *Flags::getConvName() {
	const char *conv = nullptr;
	if (_G(flags)[kFirstTestPassed] && !_G(flags)[V242])
		conv = "conv51";

	if (_G(flags)[V242] == 5 && _G(flags)[kSecondTestPassed] == 0 && !player_been_here(612)) {
		if (_G(flags)[V248])
			conv = "conv52";
		else
			--_G(flags)[V242];
	}

	if (_G(flags)[kSecondTestPassed] && !_G(flags)[V100])
		conv = "conv53";

	if (_G(flags)[V100] == 5 && !_G(flags)[kThirdTestPassed] && !player_been_here(310) &&
		!player_been_here(311) && !player_been_here(312) && !player_been_here(313) &&
		!player_been_here(314) && !player_been_here(315) && !player_been_here(316) &&
		!player_been_here(317) && !player_been_here(318) && !player_been_here(319))
		conv = "conv54";

	if (_G(flags)[kThirdTestPassed] && !_G(flags)[V185])
		conv = "conv55";

	if (_G(flags)[V185] == 5 && !_G(flags)[kFourthTestPassed])
		conv = "conv56";

	if (_G(flags)[kFourthTestPassed] && !_G(flags)[V153])
		conv = "conv57";

	if (_G(flags)[V153] == 5)
		conv = "conv58";

	return conv;
}

} // namespace Burger
} // namespace M4
