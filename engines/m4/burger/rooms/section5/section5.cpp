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

static const char *PARSER[] = {
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
	"Th-th-th-that's all folks...",
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
	"Th-th-th-that's all folks...",
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
	"Th-th-th-that's all folks...",
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
	"Th-th-th-that's all folks...",
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
	"Th-th-th-that's all folks...",
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
	"Th-th-th-that's all folks...",
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
	"Th-th-th-that's all folks...",
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
	"Th-th-th-that's all folks...",
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
	"Th-th-th-that's all folks...",
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
	"Th-th-th-that's all folks...",
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
	"Th-th-th-that's all folks...",
	nullptr
};

machine *Section5::_bork;
const char *Section5::_digiName;

Section5::Section5() : Rooms::Section() {
	_bork = nullptr;
	_digiName = nullptr;
	Common::fill(_gizmoRoomNums, _gizmoRoomNums + 15, false);

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
	// TODO: daemon method
	_G(kernel).continue_handling_trigger = true;
}

void Section5::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if ((player_said("GEAR", "RUBBER GLOVES") || player_said("RUBBER GLOVES", "WILBUR")) &&
			_G(flags)[V234] == 0 && _G(game).previous_room != 508) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 5001;
		ws_turn_to_face(4, gCHANGE_WILBUR_ANIMATION);

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
		gizmoAnim(pal);
		player_set_commands_allowed(false);

		if (_G(flags)[V238] == 0) {
			gizmoDigiPlay("510w001", 255, _gizmoRoomNums[0]);
			++_G(flags)[V238];
		} else if (_G(flags)[V238] == 1) {
			gizmoDigiPlay("510w002", 255, _gizmoRoomNums[1]);
			++_G(flags)[V238];
		}
	}
}

void Section5::gizmoAnim(RGB8 *pal) {
	// TODO
}

void Section5::gizmoDigiPlay(const char *name, int vol, bool &done) {
	if (!done) {
		done = true;
		digi_play(name, 2, vol);
		digi_read_another_chunk();
		player_set_commands_allowed(false);

		while (!g_engine->shouldQuit() && digi_play_state(2)) {
			digi_read_another_chunk();
			midi_loop();
			gui_system_event_handler();
		}

		player_set_commands_allowed(true);
	}
}

bool Section5::checkFlags() {
	return _G(flags)[V197] && _G(flags)[V200] == 5003 &&
		_G(flags)[V210] == 5002 && _G(flags)[V218] == 5003 &&
		_G(flags)[V223] == 2 && _G(flags)[V227] != 0;
}

void Section5::flagsTrigger() {
	player_set_commands_allowed(false);

	if (checkFlags()) {
		_G(wilbur_should) = 5003;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else {
		kernel_trigger_dispatch_now(5002);
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
