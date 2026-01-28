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

#include "m4/m4.h"
#include "m4/riddle/rooms/section8/room860.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room860::preload() {
	LoadWSAssets("other script", _G(master_palette));

	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room860::init() {
	interface_hide();
	player_set_commands_allowed(false);
	_G(camera_reacts_to_player) = false;
	MoveScreenDelta(_G(game_buff_ptr), -320, 0);
	g_engine->adv_camera_pan_step(3);
	digi_preload("950_s45", 950);
	digi_play_loop("950_s45", 3, 240, -1, 950);
	digi_preload("baronstg", 860);
	digi_preload("809_s04", 809);
	digi_preload("860r01", 860);
	digi_preload("860r02", 860);
	digi_preload("860m01", 860);
	digi_preload("860m02", 860);
	digi_preload("860b01", 860);

	_860RipCrossSeries = series_load("860 RIP CROSS", -1, nullptr);
	_860RipWalkSeries = series_load("860 RIP WALK", -1, nullptr);
	_860RipTalkSeries = series_load("860 RIP TALK", -1, nullptr);
	_860MeiTalkSeries = series_load("860 MEI TALK", -1, nullptr);
	_860DragonHeadsSpewingSeries = series_load("860 DRAGON HEADS SPEWING", -1, nullptr);
	series_play("860 DRAGON HEADS SPEWING", 1024, 0, -1, 12, -1, 100, 0, 0, 0, -1);
	_860RipMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "860 rip");
	sendWSMessage_10000(1, _860RipMach, _860RipCrossSeries, 1, 20, 80, _860RipCrossSeries, 20, 20, 0);
	_860McMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "860 mc");
	sendWSMessage_10000(1, _860McMach, _860MeiTalkSeries, 1, 1, -1, _860MeiTalkSeries, 1, 1, 0);
}

void Room860::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_stop(1);
		series_load("860SHLD", -1, nullptr);
		series_place_sprite("860SHLD", 0, 0, 0, 100, 512);
		sendWSMessage_10000(1, _860RipMach, _860RipWalkSeries, 1, 100, 2,
			_860RipTalkSeries, 1, 1, 0);
		sendWSMessage_190000(_860RipMach, 3);
		g_engine->set_camera_delta_pan(-290, 0);
		series_unload(_860RipCrossSeries);
		break;

	case 2:
		series_unload(_860RipWalkSeries);
		sendWSMessage_10000(1, _860McMach, _860MeiTalkSeries, 1, 1, -1,
			_860MeiTalkSeries, 1, 11, 4);
		digi_play("860m01", 2, 255, 3, 860);
		break;

	case 3:
		sendWSMessage_10000(1, _860McMach, _860MeiTalkSeries, 11, 11, -1,
			_860MeiTalkSeries, 11, 11, 0);
		sendWSMessage_10000(1, _860RipMach, _860RipTalkSeries, 1, 2, -1,
			_860RipTalkSeries, 1, 2, 4);
		digi_play("860r01", 2, 255, 4, 860);
		break;

	case 4:
		sendWSMessage_10000(1, _860RipMach, _860RipTalkSeries, 1, 6, -1, _860RipTalkSeries, 6, 6, 0);
		sendWSMessage_10000(1, _860McMach, _860MeiTalkSeries, 12, 26, -1, _860MeiTalkSeries, 26, 26, 0);
		kernel_timing_trigger(25, 5, nullptr);
		break;

	case 5:
		digi_play("860m02", 2, 255, -1, 860);
		kernel_timing_trigger(60, 6, nullptr);
		break;

	case 6:
		sendWSMessage_10000(1, _860McMach, _860MeiTalkSeries, 26, 26, -1, _860MeiTalkSeries, 26, 26, 0);
		digi_play("860r02", 2, 255, 7, -1);
		sendWSMessage_10000(1, _860RipMach, _860RipTalkSeries, 1, 2, -1, _860RipTalkSeries, 1, 2, 4);
		break;

	case 7:
		sendWSMessage_10000(1, _860McMach, _860MeiTalkSeries, 27, 32, 91, _860MeiTalkSeries, 32, 32, 0);
		sendWSMessage_10000(1, _860RipMach, _860RipTalkSeries, 6, 6, -1, _860RipTalkSeries, 6, 6, 0);
		break;

	case 80:
		sendWSMessage_10000(1, _860RipMach, _860RipCrossSeries, 21, 164, 1, _860RipWalkSeries, 1, 1, 0);
		digi_play_loop("809_s04", 1, 200, -1, 809);
		break;

	case 91:
		g_engine->set_camera_delta_pan(-30, 0);
		digi_play("baronstg", 1, 255, -1, 860);
		kernel_timing_trigger(210, 1000, nullptr);
		break;

	case 92:
		digi_stop(2);
		disable_player_commands_and_fade_init(93);
		break;

	case 93:
		adv_kill_digi_between_rooms(false);
		digi_stop(1);

		digi_unload("809_s04");
		digi_unload("860r01");
		digi_unload("860r02");
		digi_unload("860m01");
		digi_unload("860m02");
		digi_unload("860b01");
		digi_unload("baronstg");

		digi_play_loop("950_s45", 3, 80, -1, 950);

		_G(game).setRoom(811);
		break;

	case 1000:
		digi_play("860b01", 2, 255, 92, 860);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
