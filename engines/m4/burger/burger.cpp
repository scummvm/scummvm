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
#include "m4/burger/console.h"
#include "m4/burger/vars.h"
#include "m4/burger/core/conv.h"
#include "m4/burger/gui/gui_gizmo.h"
#include "m4/burger/gui/game_menu.h"
#include "m4/graphics/krn_pal.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"

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

static const char *SAID1[][4] = {
	{ "WILBUR", "999w010", "999w011", "999w011" },
	{ "BROKEN MOUSE TRAP", "405W016", nullptr, "400W001" },
	{ "CARROT JUICE", "300w043", nullptr, nullptr },
	{ "BOTTLE", "300w071", nullptr, nullptr },
	{ "WHISTLE", "300w058", nullptr, nullptr },
	{ "PANTYHOSE", "999w005", nullptr, nullptr },
	{ "AMPLIFIER", "999w026", nullptr, nullptr },
	{ "DEED", "999w101", nullptr, nullptr },
	{ "JAWZ O' LIFE", "999w103", nullptr, nullptr },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const char *GLOBAL_PARSER_LIST[] = {
	"DEED", "LOOK AT", "999w101", "WILBUR", "400w001",
		nullptr, "999w102", PARSER_ITEM_END,
	"JAWZ O' LIFE", "LOOK AT", "999w103", "WILBUR", "400w001",
		nullptr, "999w104", PARSER_ITEM_END,
	"CARROT JUICE", "JUG", "300w046", "JUG ", "300w046",
		"JUG", "300w046", "AUNT POLLY", "999w225",
		nullptr, "300w057", PARSER_ITEM_END,
	"SPRING", "BROKEN MOUSE TRAP", "405w018",
		nullptr, nullptr, PARSER_ITEM_END,
	"BOTTLE", "WATER", "999w224", "WILBUR", "300w077",
		nullptr, "300w078", PARSER_ITEM_END,
	"WHISTLE", nullptr, "300w070", PARSER_ITEM_END,
	"BLOCK OF ICE", "FLOOR", "999w003", "FLOOR ", "999w003",
		"MICROWAVE", "999w222", "WILBUR", "999w003",
		nullptr, "999w004", PARSER_ITEM_END,
	"PANTYHOSE", "ENGINE", "999w223",
		nullptr, "999w008", PARSER_ITEM_END,
	"AMPLIFIER", "WILBUR", "999w027",
		nullptr, "999w029", PARSER_ITEM_END,
	"MONEY", "STOLIE", "999w205",
		nullptr, "999w206", PARSER_ITEM_END,
	"BROKEN PUZ DISPENSER", "LOOK AT", "999w207",
		nullptr, "999w209", PARSER_ITEM_END,
	"PUZ DISPENSER", "LOOK AT", "999w207",
		nullptr, "999w209", PARSER_ITEM_END,
	"SPRING", "LOOK AT", "999w210",
		nullptr, "999w211", PARSER_ITEM_END,
	"PHONE BILL", "LOOK AT", "999w212", "AUNT POLLY", "999w213",
		nullptr, "999w206", PARSER_ITEM_END,
	"LAXATIVE", "LOOK AT", "999w214", "GEAR", "999w215",
		"WILBUR", "999w215", "PEGLEG", "999w216",
		"PEGLEG ", "999w216", "PEGLEG  ", "999w216",
		"TRUFFLES", "999w217", "GERBILS", "999w218",
		"BORK", "999w219", "BORK ", "999w219",
		"BORK  ", "999w219", "BORK   ", "999w219",
		"BORK    ", "999w219",
		nullptr, nullptr, PARSER_ITEM_END,
	"KEYS", "LOOK AT", "999w220", "WILBUR", "999w206",
		nullptr, "999w221", PARSER_ITEM_END,
	"WILBUR", nullptr, "999w012", PARSER_ITEM_END,
	nullptr
};

BurgerEngine::BurgerEngine(OSystem *syst, const M4GameDescription *gameDesc) :
		M4Engine(syst, gameDesc) {
	_sections.push_back(&_section1);
	_sections.push_back(&_section2);
	_sections.push_back(&_section3);
	_sections.push_back(&_section4);
	_sections.push_back(&_section5);
	_sections.push_back(&_section6);
	_sections.push_back(&_section7);
	_sections.push_back(&_section8);
	_sections.push_back(&_section9);
}

BurgerEngine::~BurgerEngine() {
}

M4::Vars *BurgerEngine::createVars() {
	return new Burger::Vars();
}

void BurgerEngine::setupConsole() {
	setDebugger(new Burger::Console());
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
	case kRESUME_CONVERSATION:
		conv_resume_curr();
		break;

	case kSERIES_STREAM_BREAK:
		handle_series_stream_break();
		break;

	case kSERIES_PLAY_BREAK_0:
		handle_series_play_break(0);
		break;

	case kSERIES_PLAY_BREAK_1:
		handle_series_play_break(1);
		break;

	case kSERIES_PLAY_BREAK_2:
		handle_series_play_break(2);
		break;

	case kRELEASE_TRIGGER_DIGI_CHECK:
		release_trigger_digi_check();
		break;

	case kAdvanceTime:
		_G(flags).advance_boonsville_time_and_check_schedule(300);
		kernel_timing_trigger(300, kAdvanceTime);
		break;

	case kNPC_SPEECH_FINISHED:
		if (_G(npcSpeech1))
			terminateMachineAndNull(_G(npcSpeech1));
		if (_G(npcSpeech2))
			terminateMachineAndNull(_G(npcSpeech2));

		kernel_trigger_dispatch_now(_G(npcTrigger));
		break;

	case kWILBURS_SPEECH_FINISHED:
		_G(walker).wilburs_speech_finished();
		break;

	case kWILBURS_SPEECH_START:
		_G(walker).wilbur_say();
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 10001:
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);
			_G(flags)[V299] = 0;
			_G(flags)[kDisableFootsteps] = 0;
			_G(wilbur_should) = 10002;
			break;
		case 10002:
			_G(flags)[V299] = 0;
			_G(flags)[kDisableFootsteps] = 0;
			break;
		case 10003:
			player_set_commands_allowed(false);
			player_update_info();
			ws_hide_walker();
			_G(wilbur_should) = 10013;
			series_play_with_breaks(PLAY_BREAKS1, "999wbw", _G(player_info).depth, kCHANGE_WILBUR_ANIMATION, 3, 7,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		case 10004:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(wilbur_should) = 10012;
			series_play_with_breaks(PLAY_BREAKS2, "999wcj", _G(player_info).depth, kCHANGE_WILBUR_ANIMATION, 3, 6,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		case 10005:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(wilbur_should) = 10009;
			series_play_with_breaks(PLAY_BREAKS3, "600wek", _G(player_info).depth, kCHANGE_WILBUR_ANIMATION, 3, 6,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		case 10006:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY_BREAKS4, "999wsdu", _G(player_info).depth, kCHANGE_WILBUR_ANIMATION, 3, 7,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		case 10007:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(wilbur_should) = 10014;
			series_play_with_breaks(PLAY_BREAKS5, "999wtpe", _G(player_info).depth, kCHANGE_WILBUR_ANIMATION, 3, 6,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		case 10008:
			player_set_commands_allowed(false);
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY_BREAKS6, "999wtpf", _G(player_info).depth, kCHANGE_WILBUR_ANIMATION, 3, 6,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		case 10009:
			ws_unhide_walker(_G(my_walker));
			_G(wilbur_should) = 10010;
			_G(walker).wilbur_speech("602w012x", kCHANGE_WILBUR_ANIMATION);
			break;
		case 10010:
			_G(wilbur_should) = 10011;
			_G(walker).wilbur_speech("602w012y", kCHANGE_WILBUR_ANIMATION);
			break;
		case 10011:
			player_set_commands_allowed(true);
			_G(wilbur_should) = 10002;
			_G(walker).wilbur_speech("602w012z");
			break;
		case 10012:
			player_set_commands_allowed(true);
			ws_unhide_walker(_G(my_walker));
			_G(wilbur_should) = 10002;
			_G(walker).wilbur_speech("300w055");
			break;
		case 10013:
			player_set_commands_allowed(true);
			ws_unhide_walker(_G(my_walker));
			_G(wilbur_should) = 10002;
			break;
		case 10014:
			enable_player();
			_G(wilbur_should) = 10002;
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

	case kAdvanceHour:
		if (!player_commands_allowed() && !_G(flags)[V299] && _G(my_walker) &&
				_G(player).walker_in_this_scene && _G(player).walker_visible)
			sendWSMessage(0, 0, _G(my_walker), 0, 0, 1);

		kernel_timing_trigger(imath_ranged_rand(900, 1800), kAdvanceHour);
		break;

	case kSET_FACING:
		player_set_facing_at(_G(player_facing_x), _G(player_facing_y),
			_G(player_trigger));
		break;

	case kPOOF:
		ws_demand_location(_G(my_walker), _G(player_dest_x), _G(player_dest_y));
		_G(walker).wilbur_poof();
		kernel_trigger_dispatch_now(kSET_COMMANDS_ALLOWED);
		break;

	case kSET_COMMANDS_ALLOWED:
		player_set_commands_allowed(true);
		break;

	case kUNPOOF:
		_G(walker).wilbur_unpoof();
		break;

	case kAbduction:
		if (_G(game).room_id < 200) {
			_G(wilbur_should) = 10017;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			error_show(FL, 'Burg', "Time to abduct Wilbur in space?");
		}
		break;

	case kTestTimeout:
		if (_G(game).room_id < 700) {
			_G(wilbur_should) = 10018;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
		break;

	case kTELEPORTED1:
		disable_player_commands_and_fade_init(kTELEPORTED2);
		break;

	case kTELEPORTED2:
		testDone();
		break;

	case 10027:
		wilburTeleported();
		break;

	case 10033:
		player_set_commands_allowed(false);
		_G(game).setRoom(701);
		break;

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

	if (_G(walker).wilbur_said(SAID1))
		goto done;

	if (player_said_any("WILBUR", "GEAR")) {
		if (player_said("CARROT JUICE")) {
			if (_G(flags)[V123] && !imath_rand_bool(3)) {
				_G(walker).wilbur_speech("300w056");
				goto done;

			} else if (_G(my_walker) && _G(player).walker_in_this_scene && _G(player).walker_visible) {
				_G(flags)[V123] = 1;
				_G(wilbur_should) = 10004;
				ws_turn_to_face(_G(my_walker), 3, kCHANGE_WILBUR_ANIMATION);
				goto done;
			}
		}
	}

	if (player_said("WHISTLE") && player_said_any("GEAR", "WILBUR") &&
			_G(my_walker) && _G(player).walker_in_this_scene && _G(player).walker_visible) {
		_G(wilbur_should) = 10003;
		ws_turn_to_face(_G(my_walker), 7, kCHANGE_WILBUR_ANIMATION);
		goto done;
	}

	if (player_said("kibble") && player_said_any("gear", "wilbur") &&
			_G(my_walker) && _G(player).walker_in_this_scene && _G(player).walker_visible) {
		_G(wilbur_should) = 10005;
		ws_turn_to_face(_G(my_walker), 9, kCHANGE_WILBUR_ANIMATION);
		goto done;
	}

	if (player_said("rubber ducky") && player_said_any("gear", "wilbur") &&
			_G(my_walker) && _G(player).walker_in_this_scene && _G(player).walker_visible) {
		_G(wilbur_should) = 10006;
		ws_turn_to_face(_G(my_walker), 9, kCHANGE_WILBUR_ANIMATION);
		goto done;
	}

	if (player_said("broken puz dispenser") && player_said_any("gear", "wilbur") &&
			_G(my_walker) && _G(player).walker_in_this_scene && _G(player).walker_visible) {
		_G(wilbur_should) = 10007;
		ws_turn_to_face(_G(my_walker), 5, 10016);
		goto done;
	}

	if (player_said("puz dispenser") && player_said_any("gear", "wilbur") &&
			_G(my_walker) && _G(player).walker_in_this_scene && _G(player).walker_visible) {
		_G(wilbur_should) = 10008;
		ws_turn_to_face(_G(my_walker), 5, kCHANGE_WILBUR_ANIMATION);
		goto done;
	}

	if (player_said("spring", "broken puz dispenser")) {
		inv_move_object("BROKEN PUZ DISPENSER", NOWHERE);
		inv_move_object("SPRING", NOWHERE);
		inv_give_to_player("PUZ DISPENSER");

	} else if (player_said("LOOK AT", "BLOCK OF ICE")) {
		_G(walker).wilbur_speech(_G(flags)[V250] ? "999w002" : "999w001");

	} else if (player_said("PANTYHOSE", "WILBUR")) {
		if (_G(flags)[V297]) {
			_G(walker).wilbur_speech("999w007");
		} else {
			_G(walker).wilbur_speech("999w006");
			_G(flags)[V297] = 1;
		}

	} else if (player_said("TAKE")) {
		_G(walker).wilbur_speech(saidofInterest() ? "999w021" : "999w016");

	} else if (player_said("LOOK AT", "MONEY")) {
		switch (_G(flags)[V001]) {
		case 11:
			_G(walker).wilbur_speech("999w204");
			break;
		case 12:
			_G(walker).wilbur_speech("999w203");
			break;
		case 19:
			_G(walker).wilbur_speech("999w202");
			break;
		case 20:
			_G(walker).wilbur_speech("999w201");
			break;
		default:
			break;
		}

	} else if (player_said("GEAR")) {
		_G(walker).wilbur_speech_random("999w017", "999w018", "999w019", "999w020");

	} else if (!_G(walker).wilbur_parser(GLOBAL_PARSER_LIST)) {
		if (player_said("LOOK AT")) {
			_G(walker).wilbur_speech_random("999w013", "999w014", "999w015");
		} else if (player_said("WALK") || player_said("WALK TO") ||
				player_said("WALK ACROSS") || player_said("WALK ON")) {
			term_message("Just a walk, no response needed.");
		} else {
			_G(walker).wilbur_speech_random("999w017", "999w018", "999w019", "999w020");
		}
	}

done:
	_G(player).command_ready = false;
}

bool BurgerEngine::saidofInterest() const {
	return player_said_any("DISTILLED CARROT JUICE", "broken puz dispenser", "puz dispenser") ||
		player_said_any("broken mouse trap", "mouse trap", "kindling", "burning kindling") ||
		player_said_any("CHRISTMAS LIGHTS", "CHRISTMAS LIGHTS ", "bottle", "carrot juice") ||
		player_said_any("soapy water", "iron filings", "waxed hair", "fish") ||
		player_said_any("hook", "keys", "records", "DOG DOLLAR") ||
		player_said_any("AMPLIFIER", "rubber gloves", "DIRTY SOCK", "JAWZ O' LIFE") ||
		player_said_any("deed", "burger morsel", "whistle", "QUARTER") ||
		player_said_any("matches", "phone cord", "kibble", "pantyhose") ||
		player_said_any("fan belt", "spring", "mirror", "PHONE BILL") ||
		player_said_any("ray gun", "BLOCK OF ICE", "rolling pin", "rubber duck") ||
		player_said_any("LAXATIVE", "money", "crow bar", "Wilbur");
}

void BurgerEngine::testDone() {
	player_set_commands_allowed(false);
	if (_G(flags).getConvName())
		_G(game).setRoom(207);
	else
		handleTestDone();
}

void BurgerEngine::handleTestDone() {
	if (_G(flags)[kFirstTestPassed]) {
		testDone1();

		if (_G(flags)[kSecondTestPassed]) {
			testDone2();

			if (_G(flags)[kThirdTestPassed]) {
				testDone3();

				if (_G(flags)[kFourthTestPassed]) {
					testDone4();

					if (_G(flags)[kFifthTestPassed]) {
						testDone5();

						_G(game).setRoom(701);
						_G(flags).set_boonsville_time(505200);
					} else {
						_G(game).setRoom(_G(flags)[V153] ? 402 : 401);
						_G(flags).set_boonsville_time(393600);
					}
				} else {
					_G(game).setRoom(_G(flags)[V185] ? 502 : 501);
					_G(flags).set_boonsville_time(282000);
				}
			} else {
				_G(game).setRoom(_G(flags)[V100] ? 302 : 301);
				_G(flags).set_boonsville_time(170400);
			}
		} else {
			_G(game).setRoom(_G(flags)[V242] ? 604 : 601);
			_G(flags).set_boonsville_time(58800);
		}
	} else {
		_G(game).setRoom(801);
		_G(flags).set_boonsville_time(37200);
	}
}

void BurgerEngine::testDone1() {
	_G(flags)[kFirstTestPassed] = 1;
	if (!_G(flags)[kNEURO_TEST_COUNTER])
		_G(flags)[kNEURO_TEST_COUNTER]++;
}

void BurgerEngine::testDone2() {
	if (!_G(flags)[V242])
		_G(flags)[V242]++;

	_G(flags)[kSecondTestPassed] = 1;
	_G(flags)[kHampsterState] = 6007;
	_G(flags)[V244] = 6004;
	_G(flags)[V245] = 10031;
	_G(flags)[V246] = 0;
	_G(flags)[V247] = 1;
	_G(flags)[V248] = 1;
	_G(flags)[V249] = 1;
	_G(flags)[V250] = 1;
	_G(flags)[V251] = 1;
	_G(flags)[V255] = 1;
	_G(flags)[kGerbilCageDoor] = 2;
	_G(flags)[V257] = -140;
	_G(flags)[V258] = 0;
	_G(flags)[V259] = 1;
	_G(flags)[V260] = 1;
	_G(flags)[V261] = 1;
	_G(flags)[V263] = 0;
	_G(flags)[V265] = 1;
	_G(flags)[V266] = 0;
	_G(flags)[V269] = 1;
	_G(flags)[V270] = 6001;
	_G(flags)[V273] = 1;
	_G(flags)[V274] = 1;
	_G(flags)[V277] = 6003;
	_G(flags)[V278] = 1;

	inv_move_object("ray gun", NOWHERE);
	inv_move_object("kibble", NOWHERE);
	inv_move_object("block of ice", NOWHERE);
	inv_move_object("pantyhose", NOWHERE);
}

void BurgerEngine::testDone3() {
	if (!_G(flags)[V100])
		_G(flags)[V100]++;

	inv_move_object("BURGER MORSEL", NOWHERE);
	inv_move_object("MATCHES", NOWHERE);
	inv_move_object("JUG", NOWHERE);
	inv_move_object("DISTILLED CARROT JUICE", NOWHERE);
	inv_move_object("CARROT JUICE", NOWHERE);
	inv_give_to_player("BOTTLE");

	_G(flags)[kThirdTestPassed] = 1;
	_G(flags)[V101] = 1;
	_G(flags)[V107] = 1;
	_G(flags)[V108] = 1;
	_G(flags)[V109] = 1;
	_G(flags)[V110] = 1;
	_G(flags)[kTrufflesInMine] = 1;
	_G(flags)[kPerkinsLostIsland] = 1;
	_G(flags)[V115] = 5;
	_G(flags)[V116] = 1;
	_G(flags)[V117] = 1;
	_G(flags)[V118] = 3002;
	_G(flags)[V119] = 0;
	_G(flags)[V120] = 1;
	_G(flags)[V121] = 3003;
	_G(flags)[V122] = 1;
	_G(flags)[V123] = 1;
	_G(flags)[kDrunkCarrotJuice] = 1;
	_G(flags)[V129] = 1;
	_G(flags)[V130] = 1;
	_G(flags)[V134] = 1;
	_G(flags)[V140] = 1;
	_G(flags)[kTrufflesRanAway] = 1;
	_G(flags)[V145] = 1;
}

void BurgerEngine::testDone4() {
	if (!_G(flags)[V185])
		_G(flags)[V185]++;

	inv_move_object("GIZMO", NOWHERE);
	inv_move_object("CHRISTMAS LIGHTS", NOWHERE);
	inv_move_object("CHRISTMAS LIGHTS ", NOWHERE);
	inv_move_object("RUBBER DUCKY", NOWHERE);
	inv_move_object("SOAPY WATER", NOWHERE);
	inv_move_object("DIRTY SOCK", NOWHERE);
	inv_move_object("ROLLING PIN", NOWHERE);
	inv_move_object("RUBBER GLOVES", NOWHERE);
	inv_move_object("KINDLING", NOWHERE);
	inv_move_object("BURNING KINDLING", NOWHERE);
	inv_move_object("GIZMO", NOWHERE);
	inv_move_object("GIZMO", NOWHERE);
	inv_give_to_player("BOTTLE");

	_G(flags)[kFourthTestPassed] = 1;
	_G(flags)[V186] = 0;
	_G(flags)[V195] = 1;
	_G(flags)[V196] = 0;
	_G(flags)[V197] = 1;
	_G(flags)[kFireplaceHasFire] = 1;
	_G(flags)[V199] = 1;
	_G(flags)[kStairsBorkState] = 5003;
	_G(flags)[kBORK_STATE] = 16;
	_G(flags)[V204] = 5003;
	_G(flags)[V205] = 1;
	_G(flags)[V206] = 0;
	_G(flags)[V207] = 1;
	_G(flags)[V210] = 5002;
	_G(flags)[V211] = 5000;
	_G(flags)[V212] = 5000;
	_G(flags)[V213] = 2;
	_G(flags)[V214] = 0;
	_G(flags)[V218] = 5003;
	_G(flags)[kTVOnFire] = 1;
	_G(flags)[V223] = 2;
	_G(flags)[V224] = 0;
	_G(flags)[V227] = 1;
	_G(flags)[V228] = 1;
	_G(flags)[V229] = 0;
	_G(flags)[V234] = 1;
}

void BurgerEngine::testDone5() {
	if (!_G(flags)[V153])
		_G(flags)[V153]++;

	_G(flags)[kFifthTestPassed] = 1;
	_G(flags)[V157] = 1;
	_G(flags)[V158] = 1;
	_G(flags)[V159] = 1;
	_G(flags)[V162] = 1;
	_G(flags)[V165] = 1;
	_G(flags)[V166] = 1;
	_G(flags)[V167] = 1;
	_G(flags)[V175] = 1;
	_G(flags)[V171] = 4003;
	_G(flags)[V172] = 10026;
	_G(flags)[V173] = 0;
	_G(flags)[V174] = 4004;
	_G(flags)[V176] = 1;
	_G(flags)[V181] = 1;

	inv_move_object("QUARTER", NOWHERE);
	inv_move_object("FISH", NOWHERE);
	inv_move_object("RECORDS", NOWHERE);
	inv_move_object("DOG COLLAR", NOWHERE);
}

#define MAX_INT 0x7FFFFFFF

void BurgerEngine::wilburTeleported() {
	KernelTriggerType oldMode = _G(kernel).trigger_mode;
	int oldSection = _G(game).section_id;

	switch (oldSection) {
	case 3:
		if (_G(flags)[V100] < MAX_INT)
			_G(flags)[V100]++;
		break;

	case 4:
		if (_G(flags)[V153] < MAX_INT)
			_G(flags)[V153]++;
		break;

	case 5:
		if (_G(flags)[V185] < MAX_INT)
			_G(flags)[V185]++;
		break;

	case 6:
		if (_G(flags)[V242] < MAX_INT)
			_G(flags)[V242]++;
		break;

	case 8:
		if (_G(flags)[kNEURO_TEST_COUNTER] < MAX_INT)
			_G(flags)[kNEURO_TEST_COUNTER]++;
		break;

	default:
		break;
	}

	if (_G(executing) != WHOLE_GAME) {
		_G(flags).reset2();
		_G(game).setRoom(604);
	} else {
		if (_G(flags)[kFifthTestPassed]) {
			testDone5();
			kernel_trigger_dispatch_now(10033);
		} else if (_G(flags)[kFourthTestPassed] && !_G(flags)[V153]) {
			testDone4();
			_G(game).setRoom(207);
		} else if (_G(flags)[kThirdTestPassed] && !_G(flags)[V185]) {
			testDone3();
			_G(game).setRoom(207);
		} else if (_G(flags)[kSecondTestPassed] && !_G(flags)[V100]) {
			testDone2();
			_G(game).setRoom(207);
		} else if (_G(flags)[kFirstTestPassed] && !_G(flags)[V242]) {
			testDone1();
			_G(game).setRoom(204);
		} else {
			static_cast<Inventory *>(_G(inventory))->reset();
			_G(flags).reset5();
			_G(flags).reset4();
			_G(flags).reset3();
			_G(flags).reset2();
			_G(flags).reset1();
			_G(game).setRoom(101);
		}
	}

	_G(kernel).trigger_mode = oldMode;
}

void BurgerEngine::showSaveScreen() {
	if (_useOriginalSaveLoad) {
		GUI::CreateF2SaveMenu(_G(master_palette));
	} else {
		M4Engine::showSaveScreen();
	}
}

void BurgerEngine::showLoadScreen(LoadDialogSource source) {
	if (_useOriginalSaveLoad) {
		switch (source) {
		case kLoadFromMainMenu:
			GUI::CreateLoadMenuFromMain(_G(master_palette));
			break;
		case kLoadFromGameDialog:
			GUI::CreateLoadMenu(_G(master_palette));
			break;
		case kLoadFromHotkey:
			GUI::CreateF3LoadMenu(_G(master_palette));
			break;
		}
	} else {
		M4Engine::showLoadScreen(source);
	}
}

bool BurgerEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	if (g_vars && _G(game).room_id == 903)
		// Allow loading games from the main menu
		return true;
	else
		return M4Engine::canLoadGameStateCurrently(msg);
}

} // namespace Burger
} // namespace M4
