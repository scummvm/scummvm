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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/riddle/rooms/section5/room501.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room501::init() {
	digi_preload("501_s01");
	_agentTalkLoop = series_load("AGENT TALK LOOP");
	_agentStridesForward = series_load("AGENT STRIDES FORWARD");

	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_flag = false;

	digi_play_loop("501_s01", 3, 30);
	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val2 = 0;
		_digiName = 0;
		_val4 = 0;
	}

	kernel_timing_trigger(1, 504);

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		if (!_flag)
			_paper = series_place_sprite("ONE FRAME PAPER", 0, 0, 0, 100, 0x780);
	} else {
		player_set_commands_allowed(false);

		if (_G(game).previous_room == 504) {
			if (inv_player_has("ROPE"))
				inv_move_object("ROPE", 504);
		} else {
			if (!player_been_here(501)) {
				_G(flags)[V147] = 0;
				_G(flags)[V141] = 0;
				_G(flags)[V143] = 0;
				_G(flags)[V145] = 0;
				_G(flags)[V146] = 0;
				_G(flags)[V148] = 0;
			}
		}

		++_G(flags)[V006];

		if (setItemsPlacedFlags()) {
			_digiName = getItemsPlacedDigi();
			++_val4;
		}

		ws_demand_location(588, 267, 9);
		ws_walk(287, 268, nullptr, 522, 9);
		_val5 = 0;
		_val6 = -1;
		_val7 = -1;
		_paper = nullptr;
	}

	_clock = series_place_sprite(
		(_G(flags)[V142] == 1) ? "501 PUFFIN CLOCK" : "PUFFIN CLOCK",
		0, 0, 0, 100, 0x780);
}

void Room501::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 501:
		_val9 = 1;
		_xyzzy1 = 0;
		_xyzzy2 = -1;
		_xyzzy3 = 0;

		_ripTalkLoop = series_load("RIP TALK LOOP");
		player_update_info();
		_shadow = series_show("SAFARI SHADOW 3", 0xf00, 128, -1, -1, 0,
			_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x700, 0,
			triggerMachineByHashCallbackNegative, "Rip Delta Machine State");

		switch (_val3) {
		case 3:
			ws_hide_walker();
			sendWSMessage_10000(1, _ripley, _ripTalkLoop, 2, 11, 502,
				_ripTalkLoop, 11, 11, 0);
			_val9 = 3;
			break;

		case 4:
			sendWSMessage_10000(1, _ripley, _ripTalkLoop, 11, 11, 502,
				_ripTalkLoop, 11, 11, 0);
			_val9 = 3;
			break;

		case 5:
			sendWSMessage_10000(1, _ripley, _ripTalkLoop, 11, 11, 502,
				_ripTalkLoop, 11, 11, 0);
			_val3 = 3;
			_val9 = 3;
			break;

		default:
			break;
		}
		break;

	case 502:
		if (_val9 == 3 && _val3 == 3 && _val6 != -1) {
			kernel_trigger_dispatchx(_val6);
			_val6 = -1;
		}
		if (_val9 == 7 && _val3 == 7 && _xyzzy2 != -1) {
			kernel_trigger_dispatchx(_xyzzy2);
			_xyzzy2 = -1;
		}
		if (_val9 == 13 && _val3 == 13 && _xyzzy4 != -1) {
			kernel_trigger_dispatchx(_xyzzy4);
			_xyzzy4 = -1;
		}
		if (_val9 == 3 && _val3 == 4 && _val7 != -1) {
			kernel_trigger_dispatchx(_val7);
			_val7 = -1;
		}

		if (_xyzzy1) {
			terminateMachineAndNull(_ripley);

			if (!_xyzzy3)
				ws_unhide_walker();

			terminateMachineAndNull(_shadow);
			_val5 = 0;

			if (_val9 == 3)
				series_unload(_ripTalkLoop);

		} else {
			kernel_timing_trigger(1, 503);
		}
		break;

	case 503:
		switch (_val9) {
		case 3:
			switch (_val3) {
			case 2:
				_val3 = 3;
				_val6 = kernel_trigger_create(508);
				kernel_timing_trigger(1, 502);
				break;
			case 3:
				sendWSMessage_10000(1, _ripley, _ripTalkLoop, 11, 11, 502,
					_ripTalkLoop, 11, 11, 0);
				break;
			case 4:
				frame = imath_ranged_rand(52, 76);
				sendWSMessage_10000(1, _ripley, _ripTalkLoop, frame, frame, 502,
					_ripTalkLoop, frame, frame, 0);
				break;
			case 6:
				_xyzzy5 = 1;
				digi_play((_val4 == 1) ? "501x02" : "501x03", 1);
				sendWSMessage_10000(1, _ripley, _ripSeries1, 1, 85, 502,
					_ripSeries1, 85, 85, 0);
				_val3 = 7;
				_val9 = 7;
				break;
			case 9:
				_xyzzy5 = 1;
				sendWSMessage_10000(1, _ripley, _ripSeries2, 90, 1, 502,
					_ripSeries2, 1, 1, 0);
				_val3 = 10;
				break;
			case 10:
				digi_play("COM084", 1, 255, -1, 997);
				kernel_timing_trigger(1, 505);
				sendWSMessage_10000(1, _ripley, _ripSeries2, 1, 1, 502,
					_ripSeries2, 1, 1, 0);
				_val3 = 3;
				break;
			case 11:
				_xyzzy5 = 1;
				sendWSMessage_10000(1, _ripley, _ripSeries2, 1, 90, 502,
					_ripSeries2, 90, 90, 0);
				_val3 = 12;
				break;
			case 12:
				kernel_timing_trigger(1, 505);
				sendWSMessage_10000(1, _ripley, _ripSeries2, 1, 1, 502,
					_ripSeries2, 1, 1, 0);
				_val3 = 3;
				break;
			case 13:
				sendWSMessage_10000(1, _ripley, _ripTalkLoop, 11, 2, 502,
					_ripTalkLoop, 2, 2, 0);
				_val9 = 13;
				break;
			case 14:
				_xyzzy5 = 1;
				sendWSMessage_10000(1, _ripley, _ripMoneyExchange, 1, 85, 502,
					_ripMoneyExchange, 85, 85, 0);
				_val3 = 15;
				break;
			case 15:
				kernel_timing_trigger(1, 505);
				sendWSMessage_10000(1, _ripley, _ripMoneyExchange, 85, 85, 502,
					_ripMoneyExchange, 85, 85, 0);
				_val3 = 3;
				_val6 = kernel_trigger_create(551);

				if (!inv_player_has("PERUVIAN INTI"))
					inv_give_to_player("PERUVIAN INTI");
				break;
			case 16:
				_ripSignsPaper = series_load("RIP SIGNS PAPER");
				_xyzzy5 = 1;
				digi_play(conv_sound_to_play(), 1);
				sendWSMessage_10000(1, _ripley, _ripSignsPaper, 1, 91, 502,
					_ripSignsPaper, 91, 91, 0);
				_val3 = 17;
				break;
			case 17:
				sendWSMessage_10000(1, _ripley, _ripSignsPaper, 85, 92, -1,
					_ripSignsPaper, 90, 92, 4);
				digi_play("950_S35", 1, 255, 502);
				_val3 = 18;
				break;
			case 18:
				if (!_paper) {
					_paper = series_place_sprite("one frame paper", 0, 0, 0, 100, 0x780);
					_flag = 1;
				}

				kernel_timing_trigger(1, 505);
				sendWSMessage_10000(1, _ripley, _ripSignsPaper, 92, 111, 502,
					_ripSignsPaper, 111, 111, 0);
				_val3 = 3;
				break;
			case 19:
				_ripMoneyExchange = series_load("MONEY XCHANGE");
				_xyzzy5 = 1;
				sendWSMessage_10000(1, _ripley, _ripMoneyExchange, 61, 85, 502,
					_ripMoneyExchange, 85, 85, 0);
				_val3 = 20;
				break;
			case 20:
				if (!inv_player_has("US DOLLARS"))
					inv_give_to_player("US DOLLARS");

				kernel_timing_trigger(1, 505);
				sendWSMessage_10000(1, _ripley, _ripSignsPaper, 85, 85, 502,
					_ripSignsPaper, 85, 85, 0);
				_val3 = 3;
				break;
			default:
				break;
			}
			break;

		case 7:
			switch (_val3) {
			case 3:
				kernel_timing_trigger(1, 504);
				sendWSMessage_10000(1, _ripley, _ripSeries1, 86, 94, 502,
					_ripTalkLoop, 11, 11, 0);
				_val3 = 3;
				_val9 = 3;
				break;
			case 7:
				sendWSMessage_10000(1, _ripley, _ripSeries1, 85, 85, 502,
					_ripSeries1, 85, 85, 0);
				break;
			case 8:
				sendWSMessage_10000(1, _ripley, _ripSeries1, 85, 86, 502,
					_ripSeries1, 86, 85, 0);
				_val3 = 7;
				break;
			default:
				break;
			}
			break;

		case 13:
			switch (_val3) {
			case 3:
				sendWSMessage_10000(1, _ripley, _ripTalkLoop, 2, 11, 502,
					_ripTalkLoop, 11, 11, 0);
				_val9 = 3;
				break;
			case 13:
				sendWSMessage_10000(1, _ripley, _ripTalkLoop, 11, 2, 502,
					_ripTalkLoop, 2, 2, 0);
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 504:
		_xyzzy8 = 1;
		_val1 = 1;
		_xyzzy6 = -1;
		_xyzzy7 = -1;
		_xyzzy5 = -1;
		_agent = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x700, 0,
			triggerMachineByHashCallbackNegative, "Agent at Desk");
		sendWSMessage_10000(1, _agent, _agentTalkLoop, 1, 1, 506,
			_agentTalkLoop, 1, 1, 0);
		_xyzzy8 = 1;
		break;

	case 505:
		_xyzzy8 = 5;
		_val1 = 5;
		_xyzzy6 = -1;
		_xyzzy7 = -1;
		_xyzzy5 = -1;
		_agent = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x700, 0,
			triggerMachineByHashCallbackNegative, "Agent at Desk");
		sendWSMessage_10000(1, _agent, _agentStridesForward, 15, 15, 506,
			_agentStridesForward, 15, 15, 0);
		break;

	case 506:
		if (_val1 == 1 && _xyzzy8 == 1 && _xyzzy6 != -1) {
			kernel_trigger_dispatchx(_xyzzy6);
			_xyzzy6 = -1;
		}
		if (_val1 == 5 && _xyzzy8 == 5 && _xyzzy7 != -1) {
			kernel_trigger_dispatchx(_xyzzy7);
			_xyzzy7 = -1;
		}
		if (_xyzzy5 == 1) {
			terminateMachineAndNull(_agent);
		} else {
			kernel_timing_trigger(1, 507);
		}
		break;

	case 507:
		switch (_xyzzy8) {
		case 1:
			switch (_val1) {
			case 1:
				sendWSMessage_10000(1, _agent, _agentTalkLoop, 1, 1, 506,
					_agentTalkLoop, 1, 1, 0);
				break;
			case 2:
				frame = imath_ranged_rand(1, 50);
				sendWSMessage_10000(1, _agent, _agentTalkLoop, frame, frame, 506,
					_agentTalkLoop, frame, frame, 0);
				break;
			case 3:
				player_set_commands_allowed(false);
				_ripMoneyExchange = series_load("MONEY XCHANGE");
				ws_hide_walker();
				sendWSMessage_10000(1, _agent, _ripMoneyExchange, 1, 86, 506,
					_ripMoneyExchange, 1, 1, 0);
				_val1 = 4;
				break;
			case 4:
				ws_unhide_walker();
				sendWSMessage_10000(1, _agent, _agentTalkLoop, 1, 1, 506,
					_agentTalkLoop, 1, 1, 0);
				series_unload(_ripMoneyExchange);
				_val1 = 1;
				player_set_commands_allowed(true);
				break;
			case 5:
				sendWSMessage_10000(1, _agent, _agentStridesForward, 1, 15, 506,
					_agentStridesForward, 15, 15, 0);
				_xyzzy8 = 5;
				break;
			default:
				break;
			}
			break;

		case 5:
			switch (_val1) {
			case 1:
				sendWSMessage_10000(1, _agent, _agentStridesForward, 15, 1, 506,
					_agentTalkLoop, 1, 1, 0);
				_xyzzy8 = 1;
				break;

			case 5:
				sendWSMessage_10000(1, _agent, _agentStridesForward, 15, 15, 506,
					_agentStridesForward, 15, 15, 0);
				break;

			case 6:
				sendWSMessage_10000(1, _agent, _agentSeries1, 1, 46, 502,
					_agentSeries1, 46, 46, 0);
				digi_play("501R36", 1);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	// TODO
	default:
		break;
	}
}

void Room501::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool talkFlag = player_said_any("talk", "talk to");
	bool useFlag = player_said("gear");

	if (player_said("conv501a")) {
		conv501a();
	} else if (_G(kernel).trigger == 747) {
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 509);
	} else if (lookFlag && player_said(" ")) {
		digi_play("COM001", 1, 255, -1, 997);
	} else if (lookFlag && player_said("CLOCK") && _G(flags)[V143] == 1) {
		digi_play("501R05", 1);
	} else if (lookFlag && player_said("CLOCK") && _G(flags)[V143] == 0) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			digi_play("501R05", 1, 255, 2);
			break;
		case 2:
			_G(flags)[V143] = 1;
			digi_play("501R05A", 1);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("POSTCARD RACK")) {
		digi_play("501R06", 1);
	} else if (lookFlag && player_said_any("PUFFIN", "PUFFINS", "PUFFIN POSTER") &&
			_G(flags)[V145] == 1) {
		digi_play("501R07", 1);
	} else if (lookFlag && player_said_any("PUFFIN", "PUFFINS", "PUFFIN POSTER") &&
			_G(flags)[V145] == 0) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			digi_play("501R07", 1, 255, 2);
			break;
		case 2:
			_G(flags)[V145] = 1;
			digi_play("501R07A", 1);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("STELE")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);

			if (!_G(flags)[V146]) {
				digi_play("501R08", 1, 255, 2);
				_G(flags)[V146]++;
			} else {
				if (_G(flags)[V146] < 2)
					++_G(flags)[V146];

				digi_play("501R08", 1);
			}
			break;

		case 2:
			_G(kernel).trigger_mode = KT_DAEMON;
			digi_play("501R08A", 1, 255, _G(flags)[V148] ? 612 : 544);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("POSTER")) {
		// No implementation
	} else if (takeFlag && player_said("POSTCARD RACK")) {
		digi_play("COM004", 1, 255, -1, 997);
	} else if (takeFlag && player_said("MAGAZINES")) {
		digi_play("COM005", 1, 255, -1, 997);
	} else if (takeFlag && player_said("STELE")) {
		digi_play("501R19", 1);
	} else if (takeFlag && player_said("PUFFIN")) {
		digi_play("501R20", 1);
	} else if (takeFlag && player_said("CLOCK")) {
		digi_play("501R21", 1);
	} else if (takeFlag) {
		Common::String digiName = Common::String::format("COM%.3d",
			imath_ranged_rand(6, 11));
		digi_play(digiName.c_str(), 1, 255, -1, 997);
	} else if (player_said("DRIFTWOOD PUFFIN", "AGENT") &&
		_G(flags)[V041] == 1 && _G(flags)[V144] == 1) {
		player_set_commands_allowed(false);
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 555);
	} else if (player_said("DRIFTWOOD PUFFIN", "AGENT") &&
		(_G(flags)[V041] == 0 || _G(flags)[V144] == 0)) {
		player_set_commands_allowed(false);
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 594);
	} else if (useFlag && player_said("SOFA")) {
		digi_play("COM025", 1, 255, -1, 997);
	} else if (useFlag && player_said("PHONE")) {
		digi_play("COM026", 1, 255, -1, 997);
	} else if (useFlag && !inv_player_has(_G(player).noun)) {
		digi_play("COM027", 1, 255, -1, 997);
	} else if (player_said("WALK THROUGH") && !player_been_here(504)) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			ws_walk(595, 267, nullptr, 2, 3);
			break;
		case 2:
			player_set_commands_allowed(false);
			digi_play("501R40", 1);
			ws_walk(287, 268, nullptr, 3, 9);
			break;
		case 3:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("WALK THROUGH") && player_been_here(504)) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(595, 267, nullptr, 2, 3);
			break;
		case 2:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(3);
			break;
		case 3:
			_G(game).setRoom(504);
			if (_G(flags)[V035] == 1)
				_G(flags)[V147] = 1;
			break;
		default:
			break;
		}
	} else if (player_said("JOURNAL", "STELE")) {
		if (_G(flags)[V148] == 1) {
			digi_play("501R34", 1);
		} else {
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 544);
		}
	} else if (lookFlag && player_said("JOURNAL") && !player_said("STELE")) {
		digi_play("501R35", 1);
	} else if (talkFlag && player_said("AGENT")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			_val1 = 1;
			_val3 = 3;
			_val6 = kernel_trigger_create(2);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(2, 501);
			break;
		case 2:
			_val3 = 2;
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room501::conv501a() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (node == 15) {
		if (entry == 0)
			_G(flags)[V041] = 1;
		if (entry == 5)
			_G(flags)[V144] = 1;
	}

	if (_G(kernel).trigger == 1) {
		if (who <= 0) {
			_val1 = 1;

			if (node == 1 && (entry == 2 || entry == 3)) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 549);
				return;
			} else if (node == 5 && entry == 0) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 610);
				return;
			}
		} else if (who == 1) {
			if (node == 1 && entry == 1) {
				int32 x1, y1, x2, y2;
				conv_set_box_xy(490, -4);
				set_dlg_rect();
				conv_get_dlg_coords(&x1, &y1, &x2, &y2);
				conv_set_dlg_coords(x1, y1 - 10, x2, y2 - 10);
				set_dlg_rect();
			} else {
				conv_set_box_xy(10, 10);
			}

			_val3 = 3;
			if (node == 2)
				_convEntry = entry;
			if (node == 8 && entry == 0)
				_val8 = 1;
		}
	} else {
		if (who <= 0) {
			if (node == 15 && entry == 5)
				midi_play("SADBOY1", 255, 0, -1, 949);

			if (node == 7 && entry == 0) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 585);
				return;
			}

			_val1 = 2;

		} else if (who == 1) {
			if (node == 14 && entry != 12) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 539);
				conv_set_box_xy(10, 10);
				return;
			}

			if (node == 14 && entry == 12) {
				conv_set_box_xy(10, 10);
				conv_resume();
				return;
			}
		}

		if (sound) {
			digi_play(sound, 1, 255, 1);
			return;
		}
	}

	conv_resume();
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
