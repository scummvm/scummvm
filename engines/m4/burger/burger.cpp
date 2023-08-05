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

#include "common/debug.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"
#include "m4/graphics/krn_pal.h"

namespace M4 {
namespace Burger {

static const seriesPlayBreak PLAY_BREAKS1[] = {
	{  0, 17, nullptr,   1,   0,    -1, 2048,  0, 0, 0 },
	{ 18, 18, nullptr,   0,   0, 10008,    0,  0, 0, 0 },
	{ 19, 21, "999blow", 1, 125,    -1,    0, -1, 0, 0 },
	{ 22, 30, nullptr,   0,   0,    -1,    0,  0, 0, 0 },
	{  7,  0, nullptr,   0,   0,    -1,    0,  0, 0, 0 },
	{ -1, -1, nullptr,   0,   0,    -1,    0,  0, 0, 0 }
};

static const seriesPlayBreak PLAY_BREAKS2[] = {
	{  0, 11, nullptr,   1,   0, -1, 2048, 0, 0, 0 },
	{ 12, 13, nullptr,   0,   0, -1,    0, 5, 0, 0 },
	{ 14, 26, "302w002", 1, 255, -1,    0, 0, 0, 0 },
	{  7,  0, nullptr,   0,   0, -1,    0, 0, 0, 0 },
	{ -1, -1, nullptr,   0,   0, -1,    0, 0, 0, 0 }
};

static const seriesPlayBreak PLAY_BREAKS3[] = {
	{  0, 14, nullptr,   1,   0, -1, 2048, 0, 0, 0 },
	{ 15, 16, "600_008", 2, 255, -1,    0, 0, 0, 0 },
	{ 17, 21, "602w012", 1, 255, -1,    0, 0, 0, 0 },
	{  7,  0, nullptr,   0,   0, -1,    0, 0, 0, 0 },
	{ -1, -1, nullptr,   0,   0, -1,    0, 0, 0, 0 }
};

static const seriesPlayBreak PLAY_BREAKS4[] = {
	{ 0, 12, 0, 1, 0, -1, 2048, 0, 0, 0 },
	{ 12, 13, "500_004", 1, 255, -1, 0, 2,  0, 0 },
	{ 12, 0, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ -1, -1, 0, 0, 0, -1, 0, 0, 0, 0 }
};

static const seriesPlayBreak PLAY_BREAKS5[] = {
	{  0, 15, nullptr,   1,   0, -1, 2048, 0, 0, 0 },
	{ 16, 20, "999_001", 1, 200, -1,    0, 0, 0, 0 },
	{ 19, 19, nullptr,   0,   0, -1,    0, 0, 0, 0 },
	{ 15, 18, "999_001", 1, 200, -1,    0, 0, 0, 0 },
	{ 15, 18, "999_001", 1, 200, -1,    0, 0, 0, 0 },
	{ 14,  0, nullptr,   0,   0, -1,    0, 0, 0, 0 },
	{ -1, -1, nullptr,   0,   0, -1,    0, 0, 0, 0 }
};

static const seriesPlayBreak PLAY_BREAKS6[] = {
	{  0, 15, nullptr,   1,   0, -1, 2048, 0, 0, 0 },
	{ 16, 23, "999_001", 1, 200, -1,    0, 0, 0, 0 },
	{ 24, 30, "999_002", 1, 200, -1,    0, 0, 0, 0 },
	{ 31, 34, nullptr,   1, 200, -1,    0, 0, 0, 0 },
	{ 34, 34, nullptr,   0,   0, -1,    0, 0, 0, 0 },
	{ 14,  0, nullptr,   0,   0, -1,    0, 0, 0, 0 },
	{ -1, -1, nullptr,   0,   0, -1,    0, 0, 0, 0 }
};

BurgerEngine::BurgerEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		M4Engine(syst, gameDesc) {
	_sections.push_back(&_section1);
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section1);	// TODO: Replace me
	_sections.push_back(&_section9);
}

M4::Vars *BurgerEngine::createVars() {
	return new Burger::Vars();
}

void BurgerEngine::showEngineInfo() {
	debug("Orion Burger\n");
	debug("Game Version %s -- %s\n", "Giraffe", "September 27, 1996");
	debug("%s\n", "M4 Runtime Engine by NickP, MikeE, AndrasK, Tinman, XiH, PaulL, MattP");
	debug("M4 Library Version %s -- %s\n", "v1.400 OB", "January 21, 1996");
	debug("%s.\n\n", "Copyright (c) 1996 by Sanctuary Woods Multimedia Corporation");
	debug("Orion Burger tastes great!\n");
}

void BurgerEngine::syncFlags(Common::Serializer &s) {
	g_vars->_flags.sync(s);
}

void BurgerEngine::global_daemon() {
	switch (_G(kernel).trigger) {
	case gSERIES_STREAM_BREAK:
		handle_series_stream_break();
		break;

	case gRELEASE_TRIGGER_DIGI_CHECK:
		release_trigger_digi_check();
		break;

	case gWILBURS_SPEECH_START:
		_G(walker).wilbur_say();
		break;

	case gWILBURS_SPEECH_FINISHED:
		_G(walker).wilburs_speech_finished();
		break;

	case gABDUCT:
		switch (_G(roomVal1)) {
		case 10001:
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);
			_G(flags)[V299] = 0;
			_G(flags)[V298] = 0;
			_G(roomVal1) = 10002;
			break;
		case 10002:
			_G(flags)[V299] = 0;
			_G(flags)[V298] = 0;
			break;
		case 10003:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(roomVal1) = 10013;
			series_play_with_breaks(PLAY_BREAKS1, "999wbw", UNKNOWN_DEPTH_1, gABDUCT, 3, 7,
				UNKNOWN_SCALE_1, _G(player_info).x, _G(player_info).y);
			break;
		case 10004:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(roomVal1) = 10012;
			series_play_with_breaks(PLAY_BREAKS2, "999wbj", UNKNOWN_DEPTH_1, gABDUCT, 3, 6,
				UNKNOWN_SCALE_1, _G(player_info).x, _G(player_info).y);
			break;
		case 10005:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(roomVal1) = 10009;
			series_play_with_breaks(PLAY_BREAKS3, "600wek", UNKNOWN_DEPTH_1, gABDUCT, 3, 6,
				UNKNOWN_SCALE_1, _G(player_info).x, _G(player_info).y);
			break;
		case 10006:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(roomVal1) = 10001;
			series_play_with_breaks(PLAY_BREAKS4, "999wsdu", UNKNOWN_DEPTH_1, gABDUCT, 3, 7,
				UNKNOWN_SCALE_1, _G(player_info).x, _G(player_info).y);
			break;
		case 10007:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(roomVal1) = 10014;
			series_play_with_breaks(PLAY_BREAKS5, "999wtpe", UNKNOWN_DEPTH_1, gABDUCT, 3, 6,
				UNKNOWN_SCALE_1, _G(player_info).x, _G(player_info).y);
			break;
		case 10008:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(roomVal1) = 10001;
			series_play_with_breaks(PLAY_BREAKS6, "999wtpf", UNKNOWN_DEPTH_1, gABDUCT, 3, 6,
				UNKNOWN_SCALE_1, _G(player_info).x, _G(player_info).y);
			break;
		case 10009:
			ws_unhide_walker(_G(my_walker));
			_G(roomVal1) = 10018;
			_G(walker).wilbur_speech("602w012x", gABDUCT);
			break;
		case 10010:
			_G(roomVal1) = 10011;
			_G(walker).wilbur_speech("602w012y", gABDUCT);
			break;
		case 10011:
			player_set_commands_allowed(true);
			_G(roomVal1) = 10002;
			_G(walker).wilbur_speech("602w012z");
			break;
		case 10012:
			player_set_commands_allowed(true);
			ws_unhide_walker(_G(my_walker));
			_G(roomVal1) = 10002;
			_G(walker).wilbur_speech("300w055");
			break;
		case 10013:
			player_set_commands_allowed(true);
			ws_unhide_walker(_G(my_walker));
			_G(roomVal1) = 10002;
			break;
		case 10014:
			enable_player();
			_G(roomVal1) = 10002;
			_G(walker).wilbur_speech("999w208");
			break;
		case 10017:
			wilbur_abduct(10024);
			break;
		case 10018:
			wilbur_abduct(10027);
			break;
		default:
			break;
		}
		break;

	case gTELEPROTED1:
		disable_player_commands_and_fade_init(gTELEPROTED2);
		break;
	case gTELEPROTED2:
		_G(walker).wilbur_teleported();
		break;

	// TODO: Other cases

	default:
		break;
	}
}

void BurgerEngine::global_pre_parser() {
	if (player_said("wilbur")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said("SKY"))
		player_set_facing_hotspot();
}

void BurgerEngine::global_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
//	_G(walker).wilbur_said(

	// TODO: implement
}

} // namespace Burger
} // namespace M4
