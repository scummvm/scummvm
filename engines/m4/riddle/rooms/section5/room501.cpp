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

Room501::Room501() : Room() {
	Common::fill(_items, _items + 12, 0);
	Common::fill(_queuedDigi, _queuedDigi + 4, (const char *)nullptr);
}

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
				sendWSMessage_10000(1, _ripley, _ripParcelExchange, 90, 1, 502,
					_ripParcelExchange, 1, 1, 0);
				_val3 = 10;
				break;
			case 10:
				digi_play("COM084", 1, 255, -1, 997);
				kernel_timing_trigger(1, 505);
				sendWSMessage_10000(1, _ripley, _ripParcelExchange, 1, 1, 502,
					_ripParcelExchange, 1, 1, 0);
				_val3 = 3;
				break;
			case 11:
				_xyzzy5 = 1;
				sendWSMessage_10000(1, _ripley, _ripParcelExchange, 1, 90, 502,
					_ripParcelExchange, 90, 90, 0);
				_val3 = 12;
				break;
			case 12:
				kernel_timing_trigger(1, 505);
				sendWSMessage_10000(1, _ripley, _ripParcelExchange, 1, 1, 502,
					_ripParcelExchange, 1, 1, 0);
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

	case 508:
		_val8 = 0;
		_G(kernel).trigger_mode = KT_PARSE;
		conv_load("conv501a", 10, 10, 747);

		conv_export_value_curr((_G(flags)[V088] >= 3) ? 1 : 0, 0);
		_hasItems = updateItems();
		conv_export_pointer_curr(&_G(flags)[V182], 1);

		if (inv_player_has("DRIFTWOOD PUFFIN")) {
			_xyzzy9 = 2;
		} else if (_G(flags)[V183] == 1) {
			_xyzzy9 = 1;
		} else {
			_xyzzy9 = 0;
		}

		_hasLetter = inv_player_has("MENENDEZ'S LETTER");
		conv_export_pointer_curr(&_hasLetter, 3);
		conv_export_pointer_curr(&_hasItems, 4);
		conv_export_pointer_curr(&_G(flags)[V145], 5);
		conv_export_value_curr(_G(flags)[146] > 0 ? 1 : 0, 6);
		conv_export_pointer_curr(&_G(flags)[V143], 7);
		conv_export_pointer_curr(&_G(flags)[V142], 8);
		conv_export_pointer_curr(&_G(flags)[V147], 9);
		conv_export_value_curr(_G(flags)[145] == 1 ||
			_G(flags)[V146] > 0 || _G(flags)[V143] == 1 ? 1 : 0, 10);

		_hasStickAndShellMap = inv_player_has("STICK AND SHELL MAP");
		_hasWheeledToy = inv_player_has("WHEELED TOY");
		_hasRebusAmulet = inv_player_has("REBUS AMULET");
		_hasShrunkenHead = inv_player_has("SHRUNKEN HEAD");
		_hasSilverButterfly = inv_player_has("SILVER BUTTERFLY");
		_hasPostageStamp = inv_player_has("POSTAGE STAMP");
		_hasGermanBanknote = inv_player_has("GERMAN BANKNOTE");
		_hasWhaleBoneHorn = inv_player_has("WHALE BONE HORN");
		_hasChisel = inv_player_has("CHISEL");
		_hasIncenseBurner = inv_player_has("INCENSE BURNER");
		_hasRomanovEmerald = inv_player_has("ROMANOV EMERALD");

		conv_export_pointer_curr(&_hasCrystalSkull, 11);
		conv_export_pointer_curr(&_hasStickAndShellMap, 12);
		conv_export_pointer_curr(&_hasWheeledToy, 13);
		conv_export_pointer_curr(&_hasRebusAmulet, 14);
		conv_export_pointer_curr(&_hasShrunkenHead, 15);
		conv_export_pointer_curr(&_hasSilverButterfly, 16);
		conv_export_pointer_curr(&_hasPostageStamp, 17);
		conv_export_pointer_curr(&_hasGermanBanknote, 18);
		conv_export_pointer_curr(&_hasWhaleBoneHorn, 19);
		conv_export_pointer_curr(&_hasChisel, 20);
		conv_export_pointer_curr(&_hasIncenseBurner, 21);
		conv_export_pointer_curr(&_hasRomanovEmerald, 22);
		conv_export_pointer_curr(&_G(flags)[V035], 30);
		conv_play();
		break;

	case 509:
		_val3 = 3;
		_val6 = kernel_trigger_create(510);
		break;

	case 510:
		if (_val8 == 1) {
			kernel_timing_trigger(1, 512);
		} else {
			_val3 = 13;
			_xyzzy4 = kernel_trigger_create(511);
		}
		break;

	case 511:
	case 524:
		_xyzzy1 = 1;
		player_set_commands_allowed(true);
		break;

	case 512:
		player_set_commands_allowed(false);
		pal_fade_init(21, 255, 0, 30, 513);
		break;

	case 513:
		switch (_convEntry) {
		case 0:
			_G(flags)[V129] = 1;
			break;
		case 1:
			_G(flags)[V129] = 2;
			break;
		case 2:
			_G(flags)[V129] = 0;
			break;
		case 3:
			_G(flags)[V129] = 3;
			break;
		case 4:
			_G(flags)[V129] = 5;
			break;
		default:
			break;
		}

		if (_G(flags)[V161] == 1)
			_G(flags)[V371] = 1;

		_G(game).setRoom((_convEntry == 5) ? 504 : 495);

		if (_G(flags)[V035] == 1)
			_G(flags)[V147] = 1;
		break;

	case 514:
		_val1 = 1;
		_val3 = 3;
		_val6 = kernel_trigger_create(515);
		kernel_timing_trigger(2, 501);
		break;

	case 515:
		_val3 = 4;
		digi_play("501r01", 1, 255, 516);
		break;

	case 516:
		_val3 = 3;
		_val1 = 2;
		digi_play("501x01", 1, 255, 517);
		break;

	case 517:
		_val4 = checkFlags(true);
		_val1 = 1;

		if (_val4 > 0) {
			_ripSeries1 = series_load("TELEGRAM XCHANGE");
			kernel_timing_trigger(1, 518);
		} else {
			kernel_timing_trigger(1, 523);
		}
		break;

	case 518:
		_val3 = 6;
		_xyzzy2 = kernel_trigger_create(526);
		break;

	case 522:
		kernel_timing_trigger(30, 514);
		break;

	case 523:
		_val3 = 13;
		_xyzzy4 = kernel_trigger_create(524);
		break;

	case 526:
		Common::fill(_queuedDigi, _queuedDigi + 4, (const char *)nullptr);
		inv_give_to_player("MESSAGE LOG");

		if (_val4 <= 0) {
			kernel_timing_trigger(1, 523);
		} else {
			if (_digiName) {
				_queuedDigi[0] = _digiName;
				_digiName = nullptr;
			} else if (_G(flags)[V364] == 1) {
				_queuedDigi[0] = "201R26";
				_G(flags)[V364] = 0;
			} else if (_G(flags)[V365] == 1) {
				_queuedDigi[0] = "201R61";
				_G(flags)[V365] = 0;
			} else if (_G(flags)[V373] == 1) {
				_queuedDigi[0] = "401R36";
				_G(flags)[V373] = 0;
			} else if (_G(flags)[V366] == 1) {
				_queuedDigi[0] = "401R31";
				_G(flags)[V366] = 0;
			} else if (_G(flags)[V370] == 1) {
				_queuedDigi[0] = "501R02B";
				_queuedDigi[1] = nullptr;
				_queuedDigi[2] = nullptr;
				_G(flags)[V370] = 0;
			} else if (_G(flags)[V372] == 1) {
				_queuedDigi[0] = "701R39";
				_queuedDigi[1] = "701R39A";
				_G(flags)[V372] = 0;
			} else if (_G(flags)[V367] == 1) {
				_queuedDigi[0] = "401R37";
				_G(flags)[V367] = 0;
				_val2 = 1;
			} else if (_G(flags)[V368] == 1) {
				_queuedDigi[0] = "401R38";
				_G(flags)[V368] = 0;
				_val2 = 1;
			} else if (_G(flags)[V369] == 1) {
				_queuedDigi[0] = "401R39";
				_G(flags)[V369] = 0;
				_val2 = 1;
			}

			kernel_timing_trigger(1, 527);
		}
		break;

	case 527:
		_val3 = 7;
		_xyzzy2 = kernel_trigger_create(528);
		break;

	case 528:
		if (_queuedDigi[0])
			digi_play(_queuedDigi[0], 1, 255, 529);
		else
			kernel_timing_trigger(1, 529);
		break;

	case 529:
		if (_queuedDigi[1])
			digi_play(_queuedDigi[1], 1, 255, 530);
		else
			kernel_timing_trigger(1, 530);
		break;

	case 530:
		if (_queuedDigi[2])
			digi_play(_queuedDigi[2], 1, 255, 531);
		else
			kernel_timing_trigger(1, 531);
		break;

	case 531:
		if (_queuedDigi[3])
			digi_play(_queuedDigi[3], 1, 255, 532);
		else
			kernel_timing_trigger(1, 532);
		break;

	case 532:
		if (_val2 == 1) {
			_val2 = 0;
			kernel_timing_trigger(1, 534);
		} else {
			if (--_val4 > 0) {
				_val3 = 8;
				_xyzzy2 = kernel_trigger_create(526);
			} else {
				kernel_timing_trigger(1, 533);
			}
		}
		break;

	case 533:
		_val3 = 3;
		_val6 = kernel_trigger_create(523);
		break;

	case 534:
		_val3 = 3;
		_val6 = kernel_trigger_create(535);
		break;

	case 535:
		_ripParcelExchange = series_load("PARCEL XCHANGE");
		_val1 = 5;
		_xyzzy7 = kernel_trigger_create(536);
		break;

	case 536:
		_val3 = 9;
		_xyzzy7 = kernel_trigger_create(537);
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

bool Room501::updateItems() {
	Common::fill(_items, _items + 12, 0);
	_itemsCount = 0;

	static const char *ITEMS[12] = {
		"CRYSTAL SKULL", "STICK AND SHELL MAP", "WHEELED TOY",
		"REBUS AMULET", "SHRUNKEN HEAD", "SILVER BUTTERFLY",
		"POSTAGE STAMP", "GERMAN BANKNOTE", "WHALE BONE HORN",
		"CHISEL", "INCENSE BURNER", "ROMANOV EMERALD"
	};

	for (int i = 0; i < 12; ++i) {
		if (inv_player_has(ITEMS[i]))
			_items[_itemsCount++] = i + 1;
	}

	return _itemsCount > 0;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
