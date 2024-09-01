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

#include "m4/riddle/rooms/section4/room409.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room409::init() {
	player_set_commands_allowed(false);
	digi_preload("409r01a");
	digi_preload("409r01b");
	digi_preload("409r01c");
	digi_preload("950_s27");
	ws_demand_location(239, 295, 3);

	_seriesShadow = series_load("SAFARI SHADOW 3");
	_seriesDoorOpen = series_load("409 DOOR OPEN");
	_seriesRope = series_load("409ROPE");
	_seriesWolfOpensDoor = series_load("409 WOLF OPENS DOOR");
	_seriesReadTelegram = series_load("RIP TREK READS TELEGRAM POS5");
	_seriesRp02 = series_load("409RP02");
	_seriesHeadTurn = series_load("RIP TREK HEAD TURN POS5");
	_seriesTalker = series_load("RIP TALKER POS 5");
	_seriesReachHand = series_load("RIP TREK MED REACH HAND POS1");

	_turtleRope = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
		triggerMachineByHashCallbackNegative, "TURTLE & rope");
	sendWSMessage_10000(1, _turtleRope, _seriesRope, 1, 20, 100, _seriesRope, 20, 20, 0);

	_volume = 255;
	_flag = true;

	digi_play("950_s27", 2, 255, -1, 950);
	digi_play("409_s04", 3);

	_wolfWalker = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x300, 0,
		triggerMachineByHashCallbackNegative, "WOLFIE");
	sendWSMessage_10000(1, _wolfWalker, _seriesWolfOpensDoor, 1, 20, 350,
		_seriesWolfOpensDoor, 20, 20, 0);
}

void Room409::daemon() {
	switch (_G(kernel).trigger) {
	case 100:
		if (_flag) {
			sendWSMessage_10000(1, _turtleRope, _seriesRope, 1, 20, 100,
				_seriesRope, 20, 20, 0);
			digi_play("950_s27", 3, _volume, -1, 950);
		}
		break;

	case 350:
		kernel_timing_trigger(75, 351);
		break;

	case 351:
		sendWSMessage_10000(1, _wolfWalker, _seriesWolfOpensDoor, 21, 38, 352,
			_seriesWolfOpensDoor, 38, 38, 0);
		digi_play("409_s01", 1);
		break;

	case 352:
		terminateMachineAndNull(_wolfWalker);
		_doorOpen = series_place_sprite("409 DOOR OPEN", 0, 0, 0, 100, 0x300);
		kernel_timing_trigger(120, 353);
		digi_play("409w11a", 1);
		break;

	case 353:
		ws_walk(323, 295, nullptr, 354, 3);
		break;

	case 354:
		kernel_timing_trigger(180, 355);
		break;

	case 355:
		ws_walk(333, 319, nullptr, 356, 5);
		break;

	case 356:
		kernel_timing_trigger(60, 357);
		break;

	case 357:
		ws_hide_walker();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallbackNegative, "rip looks at turtle");
		sendWSMessage_10000(1, _ripley, _seriesRp02, 1, 16, 358,
			_seriesRp02, 16, 16, 0);
		break;

	case 358:
		kernel_timing_trigger(60, 359);
		break;

	case 359:
		sendWSMessage_10000(1, _ripley, _seriesRp02, 16, 1, 360, _seriesRp02, 1, 1, 0);
		break;

	case 360:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();

		player_update_info();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x100, 0,
			triggerMachineByHashCallbackNegative, "rip looks around");
		ws_hide_walker();
		sendWSMessage_10000(1, _ripley, _seriesHeadTurn, 1, 6, -1,
			_seriesHeadTurn, 6, 6, 0);
		kernel_timing_trigger(150, 361);
		break;

	case 361:
		sendWSMessage_10000(1, _ripley, _seriesHeadTurn, 6, 1, -1, _seriesHeadTurn, 1, 1, 0);
		kernel_timing_trigger(180, 362);
		kernel_timing_trigger(90, 363);
		break;

	case 362:
		_volume = 40;
		digi_play("409r01c", 1, 255, 366);
		break;

	case 363:
		sendWSMessage_10000(1, _ripley, _seriesHeadTurn, 8, 12, -1, _seriesHeadTurn, 12, 12, 0);
		kernel_timing_trigger(150, 364);
		break;

	case 364:
		sendWSMessage_10000(1, _ripley, _seriesHeadTurn, 12, 8, -1, _seriesHeadTurn, 8, 8, 0);
		kernel_timing_trigger(150, 365);
		break;

	case 366:
		sendWSMessage_10000(1, _ripley, _seriesTalker, 1, 4, -1, _seriesTalker, 1, 4, 1);
		digi_play("409r01a", 1, 255, 367);
		midi_play("ATONING", 255, 1, -1, 949);
		break;

	case 367:
		digi_play("409r01b", 1, 255, 368);
		break;

	case 368:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		ws_walk(379, 330, nullptr, 380, 1);
		break;

	case 380:
		setGlobals1(1, _seriesReachHand, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(381);
		break;

	case 381:
		digi_play("950_s07", 2);
		sendWSMessage_120000(382);
		inv_give_to_player("TOMB MESSAGE");
		break;

	case 382:
		sendWSMessage_150000(384);
		break;

	case 384:
		ws_walk(377, 332, nullptr, 385, 5);
		break;

	case 385:
		player_update_info();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x100, 0,
			triggerMachineByHashCallbackNegative, "rip looks around");
		_shadow = series_place_sprite("SAFARI SHADOW 5", 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);
		sendWSMessage_10000(1, _ripley, _seriesReadTelegram, 1, 21, 386,
			_seriesReadTelegram, 22, 31, 0);
		break;

	case 386:
		sendWSMessage_1a0000(_ripley, 12);
		digi_play("409r02d", 1, 255, 387);
		break;

	case 387:
		digi_play("409r02a", 1, 255, 388);
		break;

	case 388:
		digi_play("409r02b", 1, 255, 389);
		break;

	case 389:
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, -1);
		sendWSMessage_10000(1, _ripley, _seriesReadTelegram, 21, 1, 391,
			_seriesReadTelegram, 1, 1, 0);
		sendWSMessage_190000(_ripley, 5);
		break;

	case 391:
		sendWSMessage_10000(1, _ripley, _seriesHeadTurn, 8, 12, -1,
			_seriesHeadTurn, 12, 12, 0);
		digi_play("409r02c", 1, 255, 401);
		break;

	case 401:
		sendWSMessage_10000(1, _ripley, _seriesHeadTurn, 12, 8, 402,
			_seriesHeadTurn, 1, 1, 0);
		break;

	case 402:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_shadow);
		ws_unhide_walker();
		ws_walk(379, 330, nullptr, 403, 1);
		break;

	case 403:
		setGlobals1(1, _seriesReachHand, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(405);
		break;

	case 405:
		sendWSMessage_120000(-1);
		digi_play("409_s03", 2);
		inv_give_to_player("KEY");
		kernel_examine_inventory_object("PING KEY", _G(master_palette),
			5, 1, 270, 150, -1, 0, -1);
		digi_play("409r03a", 1, 255, 406);
		break;

	case 406:
		kernel_unexamine_inventory_object(_G(master_palette), 5, 1);
		sendWSMessage_150000(-1);

		if (!_G(flags)[V286] || !_G(flags)[V287] || !_G(flags)[V288])
			digi_play("409r04a", 1, 255, 407);
		else
			digi_play("409r05a", 1, 255, 407);
		break;

	case 407:
		ws_walk(333, 319, nullptr, 408, 5);
		_volume = 255;
		break;

	case 408:
		ws_hide_walker();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallbackNegative, "rip looks at turtle");
		sendWSMessage_10000(1, _ripley, _seriesRp02, 1, 16, 409, _seriesRp02, 16, 16, 0);
		break;

	case 409:
		digi_play("409r06", 1, 255, 410);
		break;

	case 410:
		disable_player_commands_and_fade_init(420);
		break;

	case 420:
		_G(game).setRoom(401);
		break;

	default:
		break;	
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
