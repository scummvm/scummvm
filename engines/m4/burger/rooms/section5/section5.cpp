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

#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/gui/gui_gizmo.h"
#include "m4/burger/vars.h"
#include "m4/burger/burger.h"
#include "m4/gui/gui_sys.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Section5::PARSER[] = {
	"GIZMO",
	"LOOK AT",
	"500w006",
	"TAKE",
	nullptr,
	"BORK",
	"500w007",
	"BORK ",
	"500w007",
	"BORK  ",
	"500w007",
	"MICROWAVE",
	"500w008",
	"OVEN",
	"500w008",
	"OUTLET",
	"500w009",
	"TOASTER",
	"500w010",
	"WASHING MACHINE",
	"500w011",
	"TELEVISION",
	"500w012",
	"VIDEO GAME",
	"500w012",
	"TOILET",
	"500w011",
	"TUB",
	"500w011",
	"CHRISTMAS LIGHTS  ",
	"500w012",
	"CHRISTMAS LIGHTS   ",
	"500w012",
	"TELEPHONE",
	"500w012",
	"AMPLIFIER",
	nullptr,
	nullptr,
	"500w013",
	PARSER_ITEM_END,
	 "ROLLING PIN",
	"LOOK AT",
	"500w014",
	"TAKE",
	nullptr,
	"BORK",
	"500w015",
	"BORK ",
	"500w015",
	"BORK  ",
	"500w015",
	"RUBBER DUCKY",
	"500w032",
	"RUBBER DUCKY ",
	"500w032",
	"RUBBER DUCKY  ",
	"500w032",
	"FIREPLACE",
	"500w016",
	"PRUNES",
	"500w018",
	"WILBUR",
	"500w019",
	nullptr,
	"500w020",
	PARSER_ITEM_END,
	"DIRTY SOCK",
	"LOOK AT",
	"500w021",
	"TAKE",
	nullptr,
	"BORK",
	"500w022",
	"BORK ",
	"500w022",
	"BORK  ",
	"500w022",
	"FIREPLACE",
	"500w023",
	"MICROWAVE",
	"500w023",
	"FRIDGE",
	"500w024",
	"OVEN",
	"500w024",
	"SINK",
	"500w025",
	"WASHING MACHINE",
	"500w025",
	 "LAUNDRY HAMPER",
	"500w023",
	"WARDROBE",
	"500w023",
	"TOILET",
	"500w023",
	"TUB",
	"500w025",
	"SINK",
	"500w025",
	 "SOAPY WATER",
	"500w052",
	 "SOAPY WATER ",
	"500w052",
	"HOLE",
	"500w026",
	"WILBUR",
	"500w027",
	nullptr,
	"500w028",
	PARSER_ITEM_END,
	"RUBBER DUCKY",
	"LOOK AT",
	"500w029",
	"TAKE",
	nullptr,
	"GEAR",
	nullptr,
	"ROLLING PIN ",
	"500w032",
	"MICROWAVE",
	"500w032",
	"FRIDGE",
	"500w033",
	"OVEN",
	"500w032",
	"OUTLET",
	"500w035",
	"WRINGER",
	"500w032",
	"WASHING MACHINE",
	"500w037",
	"FIRE",
	"500w030",
	nullptr,
	"500w043",
	PARSER_ITEM_END,
	"SOAPY WATER",
	"LOOK AT",
	"500w044",
	"TAKE",
	nullptr,
	"BORK",
	"500w045",
	"BORK ",
	"500w045",
	"BORK  ",
	"500w045",
	"STAIRS",
	"500w046",
	"KINDLING ",
	"500w048",
	"FIREPLACE",
	"500w048",
	"DIRTY SOCK ",
	"500w052",
	"WASHING MACHINE",
	"500w053",
	"TELEVISION",
	"500w055",
	"VIDEO GAME",
	"500w056",
	"FIRE",
	"500w055",
	"TOILET",
	"500w057",
	"TUB",
	"500w058",
	"WILBUR",
	"500w059",
	nullptr,
	"500w060",
	PARSER_ITEM_END,
	"KINDLING",
	"LOOK AT",
	"500w061",
	"TAKE",
	nullptr,
	"KINDLING ",
	"500w064",
	"BREAKER BOX",
	"500w066",
	"TOILET",
	"500w068",
	"TUB",
	"500w068",
	"CHIMNEY",
	"500w070",
	 "CHIMNEY POTS",
	"500w070",
	"WILBUR",
	"500w071",
	nullptr,
	"500w072",
	PARSER_ITEM_END,
	"RUBBER GLOVES",
	"LOOK AT",
	"500w073",
	"TAKE",
	nullptr,
	"BORK",
	"500w074",
	"BORK ",
	"500w074",
	"BORK  ",
	"500w074",
	"WRINGER",
	"500w075",
	"WASHING MACHINE",
	"500w076",
	nullptr,
	"500w078"
	PARSER_ITEM_END,
	"CHRISTMAS LIGHTS",
	"LOOK AT",
	"500w080",
	"TAKE",
	nullptr,
	"GEAR",
	"500w019",
	"WILBUR'S ROOM",
	"500w081",
	"BATHROOM",
	"500w082",
	"CHIMNEY",
	"500w083",
	"CHIMNEY POTS",
	"500w083",
	"WILBUR",
	"500w090",
	nullptr,
	"500w091",
	PARSER_ITEM_END,
	"CHRISTMAS LIGHTS ",
	"LOOK AT",
	"500w079",
	"TAKE",
	nullptr,
	"WILBUR'S ROOM",
	"500w081",
	"BATHROOM",
	"500w082",
	"CHIMNEY",
	"500w083",
	"CHIMNEY POTS",
	"500w083",
	"WILBUR",
	"500w090",
	nullptr,
	"500w091",
	PARSER_ITEM_END,
	"BOTTLE",
	"TAKE",
	nullptr,
	"SINK",
	"503w047",
	"WASHING MACHINE",
	"504w033",
	"TOILET",
	"507w037",
	nullptr,
	nullptr,
	PARSER_ITEM_END,
	"AMPLIFIER",
	"TAKE",
	nullptr,
	"PIANO",
	"502w056",
	"OUTLET",
	"503w048",
	"VIDEO GAME",
	"506w039",
	"PHONE CORD",
	"509w028",
	"GIZMO",
	"999w028",
	nullptr,
	nullptr,
	PARSER_ITEM_END,
	nullptr
};

const seriesPlayBreak Section5::PLAY1[] = {
	{ 0, 25, 0, 1, 0, -1, 2048, 0, 0, 0 },
	{ 26, 42, "500_030", 1, 255, -1, 0, 0, 0, 0 },
	{ 43, -1, "500_030", 1, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

machine *Section5::_bork;
const char *Section5::_digiName;
const char *Section5::_bgDigiName;

Section5::Section5() : Rooms::Section() {
	_bork = nullptr;
	_digiName = nullptr;

	add(501, &_room501);
	add(502, &_room502);
	add(503, &_room503);
	add(504, &_room504);
	add(505, &_room505);
	add(506, &_room506);
	add(507, &_room507);
	add(508, &_room508);
	add(509, &_room509);
	add(510, &_room510);
	add(511, &_room511);
	add(512, &_room512);
	add(513, &_room513);
}

void Section5::init() {
	if (_G(executing) != WHOLE_GAME) {
		inv_give_to_player("AMPLIFIER");
		inv_give_to_player("BOTTLE");
	}
}

void Section5::daemon() {
	switch (_G(kernel).trigger) {
	case 5001:
		flagsTrigger();
		break;

	case 5002:
		break;

	case 5003: {
		term_message("SET AMBIENCE trigger hit");
		Section5Room *room = dynamic_cast<Section5Room *>(g_engine->_activeRoom);
		assert(room);
		room->init();
		break;
	}

	case 5004:
		term_message("Bork ambient noises trigger hit");

		if (!digi_play_state(1) && _bgDigiName) {
			digi_unload(_bgDigiName);
			_bgDigiName = nullptr;
		}

		if (!digi_play_state(1) && !digi_play_state(2)) {
			// FIXME: The original does a random of 1 to 9, but the switch has 1 to 13.
			// Check if extra cases are valid, and if so, re-enable them
			switch (imath_ranged_rand(1, 9)) {
			case 1:
				_bgDigiName = "500_015";
				break;
			case 2:
				_bgDigiName = "500_016";
				break;
			case 3:
				_bgDigiName = "500_017";
				break;
			case 4:
				_bgDigiName = "500_018";
				break;
			case 5:
				_bgDigiName = "500_019";
				break;
			case 6:
				_bgDigiName = "500_020";
				break;
			case 7:
				_bgDigiName = "500_021";
				break;
			case 8:
				_bgDigiName = "500_022";
				break;
			case 9:
				_bgDigiName = "500_023";
				break;
			case 10:
				_bgDigiName = "500_024";
				break;
			case 11:
				_bgDigiName = "500_025";
				break;
			case 12:
				_bgDigiName = "500_026";
				break;
			case 13:
				_bgDigiName = "500_027";
				break;
			default:
				break;
			}

			digi_preload(_bgDigiName);
			digi_play(_bgDigiName, 1, 125, -1);
		}

		kernel_timing_trigger(imath_ranged_rand(240, 360), 5004);
		break;

	case 5006:
		_G(game).new_room = 502;
		break;

	case 5007:
		_G(game).new_room = 503;
		break;

	case 5008:
		_G(game).new_room = 504;
		break;

	case 5009:
		_G(game).new_room = 505;
		break;

	case 5010:
		_G(game).new_room = 506;
		break;

	case 5011:
		_G(game).new_room = 507;
		break;

	case 5012:
		_G(game).new_room = 508;
		break;

	case 5013:
		_G(game).new_room = 509;
		break;

	case 5014:
		player_update_info();
		_G(flags)[V187] = _G(player_info).x;
		_G(flags)[V188] = _G(player_info).y;
		_G(flags)[V189] = _G(player_info).facing;
		_G(game).new_room = 510;
		break;

	case 5015:
		_G(game).new_room = 511;
		break;

	case 5016:
		_G(game).new_room = 512;
		break;

	case 5017:
		_G(game).new_room = 513;
		break;

	case 5018:
		disable_player_commands_and_fade_init(5017);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 5001:
			_G(flags)[V234] = 1;
			ws_hide_walker();
			_G(wilbur_should) = 5002;
			player_update_info();
			series_play_with_breaks(PLAY1, "503wi07", _G(player_info).depth,
				kCHANGE_WILBUR_ANIMATION, 3, 6, _G(player_info).scale,
				_G(player_info).x, _G(player_info).y);
			break;

		case 5002:
			inv_move_object("RUBBER GLOVES", NOWHERE);
			ws_unhide_walker();
			player_set_commands_allowed(true);
			wilbur_speech("500w077");
			break;

		case 5003:
			player_set_commands_allowed(false);
			ws_unhide_walker();
			wilbur_speech("500w092", 5018);
			break;

		case 10015:
			_G(game).new_room = 512;
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Section5::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if ((player_said("GEAR", "RUBBER GLOVES") || player_said("RUBBER GLOVES", "WILBUR")) &&
			_G(flags)[V234] == 0 && _G(game).previous_room != 508) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 5001;
		ws_turn_to_face(4, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GIZMO") && player_said_any("WILBUR", "GEAR", "TAKE")) {
		takeGizmo(_G(master_palette));

	} else if (_G(walker).wilbur_parser(PARSER)) {
		// Already handled
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Section5::takeGizmo(RGB8 *pal) {
	if (player_commands_allowed() && INTERFACE_VISIBLE &&
			!_G(pal_fade_in_progress) && !_GIZMO(initialized)) {
		GUI::gizmo_anim(pal);
		player_set_commands_allowed(false);

		if (_G(flags)[V238] == 0) {
			GUI::gizmo_digi_play("510w001", 255, _GIZMO(roomFlags)[0]);
			++_G(flags)[V238];
		} else if (_G(flags)[V238] == 1) {
			GUI::gizmo_digi_play("510w002", 255, _GIZMO(roomFlags)[1]);
			++_G(flags)[V238];
		}

		player_set_commands_allowed(true);
	}
}

bool Section5::checkFlags() {
	return _G(flags)[V197] && _G(flags)[kStairsBorkState] == 5003 &&
		_G(flags)[V210] == 5002 && _G(flags)[V218] == 5003 &&
		_G(flags)[V223] == 2 && _G(flags)[V227] != 0;
}

void Section5::flagsTrigger() {
	player_set_commands_allowed(false);

	if (checkFlags()) {
		_G(wilbur_should) = 5003;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else {
		kernel_trigger_dispatch_now(5002);
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
