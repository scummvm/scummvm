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

#include "m4/riddle/rooms/section5/room504.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_vmng.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room504::init() {
	_val1 = -1;

	_vines1 = nullptr;
	_vines2 = nullptr;
	_vines3 = nullptr;
	_vines4 = nullptr;
	_rope = nullptr;

	_ladder = nullptr;
	_toy = nullptr;
	_shovel = nullptr;
	_driftwood = nullptr;
	_pole = 0;

	_waterfall = series_plain_play("peruvian waterfall", -1, 0, 100, 0xf00, 9, -1, 0);
	digi_preload("504_S01");
	_volume = 1;
	kernel_timing_trigger(1, 501);
	digi_play("504_501", 3, _volume);
	kernel_timing_trigger(828, 754);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 506:
		player_set_commands_allowed(false);
		ws_demand_location(1384, 205, 9);
		ws_hide_walker();
		MoveScreenDelta(_G(game_buff_ptr), -1280, 0);
		_xyzzy2 = 1;

		_downSteps = series_load("504 down steps");
		player_update_info();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
			_G(player_info).depth, 0, triggerMachineByHashCallbackNegative, "Rp");
		sendWSMessage_10000(1, _ripley, _downSteps, 1, 27, 647,
			_downSteps, 27, 27, 0);
		break;

	default:
		player_set_commands_allowed(false);
		ws_demand_location(50, 226, 3);
		midi_play("MOCAMO", 200, 0, -1, 949);

		if (player_been_here(504)) {
			inv_give_to_player("ROPE");
			_G(flags)[V154] = 3;
			ws_walk(183, 207, nullptr, 762, 3);
		} else {
			_G(flags)[V152] = 6;
			_G(flags)[V153] = 6;
			_G(flags)[V154] = 2;
			_G(flags)[V171] = 3;
			ws_walk(183, 207, nullptr, 655, 3);
		}

		_xyzzy2 = 0;
		break;
	}

	setVines();
	setVinesRope();
	setMiscItems();

	if (!_G(flags)[V141])
		kernel_timing_trigger(1, 502);
}

void Room504::daemon() {
}

void Room504::pre_parser() {

}

void Room504::parser() {

}

void Room504::setVines() {
	freeVines();

	bool ropeFlag = _G(flags)[V152] == 2 || _G(flags)[V153] == 2 ||
		_G(flags)[V154] == 2;
	bool tiedFlag = _G(flags)[V152] == 0 || _G(flags)[V153] == 0 ||
		_G(flags)[V154] == 0 || _G(flags)[V171] == 0;
	bool rightVine = _G(flags)[V152] == 1 || _G(flags)[V153] == 1 ||
		_G(flags)[V154] == 1 || _G(flags)[V171] == 1;
	bool acrossFlag = _G(flags)[V152] == 5 || _G(flags)[V153] == 5 ||
		_G(flags)[V154] == 5 || _G(flags)[V171] == 5;
	bool hangingFlag = _G(flags)[V152] == 4 || _G(flags)[V153] == 4 ||
		_G(flags)[V154] == 4 || _G(flags)[V171] == 4;

	if (hangingFlag)
		_vines1 = series_place_sprite("504 R VINE HANGING ",
			0, 0, 0, 100, 0xe00);
	if (acrossFlag)
		_vines2 = series_place_sprite("VINE SPRITE ACROSS CHASM",
			0, 0, 0, 100, 0xe00);
	if (tiedFlag)
		_vines3 = series_place_sprite("VINE SPRITE AFTER FIRST TIED",
			0, 0, 0, 100, 0xe00);
	if (rightVine)
		_vines4 = series_place_sprite("504 R VINE SPRITE", 0, 0, 0, 100, 0xe00);
	if (ropeFlag)
		_rope = series_place_sprite("ROPE SPRITE", 0, 0, 0, 100, 0xe00);
}

void Room504::freeVines() {
	terminateMachineAndNull(_vines1);
	terminateMachineAndNull(_vines2);
	terminateMachineAndNull(_vines3);
	terminateMachineAndNull(_vines4);
	terminateMachineAndNull(_rope);
}

void Room504::setVinesRope() {
	disableVinesRope();

	if (_G(flags)[V154] == 2) {
		hotspot_set_active("ROPE ", true);
		hotspot_set_active("ROPE  ", true);
		addRope();
	}

	if (_G(flags)[V152] == 2) {
		hotspot_set_active("GREEN VINE ", true);
		hotspot_set_active("GREEN VINE  ", true);
		addGreenVine();
	}

	if (_G(flags)[V153] == 2) {
		hotspot_set_active("BROWN VINE ", true);
		hotspot_set_active("BROWN VINE  ", true);
		addBrownVine();
	}

	if (_G(flags)[154] == 1) {
		hotspot_set_active("ROPE ", true);
		hotspot_set_active("ROPE COIL ", true);
	}

	if (_G(flags)[V152] == 1) {
		hotspot_set_active("GREEN VINE ", true);
		hotspot_set_active("GREEN VINE COIL ", true);
	}

	if (_G(flags)[V153] == 1) {
		hotspot_set_active("BROWN VINE ", true);
		hotspot_set_active("BROWN VINE COIL ", true);
	}

	if (_G(flags)[V171] == 1) {
		hotspot_set_active("VINES ", true);
		hotspot_set_active("COIL OF VINES ", true);
	}

	if (_G(flags)[V154] == 0) {
		hotspot_set_active("ROPE  ", true);
		hotspot_set_active("ROPE COIL   ", true);
	}

	if (_G(flags)[V152] == 0) {
		hotspot_set_active("GREEN VINE  ", true);
		hotspot_set_active("GREEN VINE COIL   ", true);
	}

	if (_G(flags)[V153] == 0) {
		hotspot_set_active("BROWN VINE  ", true);
		hotspot_set_active("BROWN VINE COIL   ", true);
	}

	if (_G(flags)[V171] == 0) {
		hotspot_set_active("VINES  ", true);
		hotspot_set_active("COIL OF VINES   ", true);
	}

	if (_G(flags)[V154] == 4) {
		hotspot_set_active("ROPE ", true);
		hotspot_set_active("ROPE COIL  ", true);
	}

	if (_G(flags)[V152] == 4) {
		hotspot_set_active("GREEN VINE ", true);
		hotspot_set_active("GREEN VINE  ", true);
	}

	if (_G(flags)[V153] == 4) {
		hotspot_set_active("BROWN VINE ", true);
		hotspot_set_active("BROWN VINE COIL  ", true);
	}

	if (_G(flags)[V171] == 4) {
		hotspot_set_active("VINES ", true);
		hotspot_set_active("COIL OF VINES  ", true);
	}

	if (_G(flags)[V154] == 5) {
		hotspot_set_active("ROPE  ", true);
		hotspot_set_active("ROPE COIL    ", true);
	}

	if (_G(flags)[V152] == 5) {
		hotspot_set_active("GREEN VINE  ", true);
		hotspot_set_active("GREEN VINE COIL    ", true);
	}

	if (_G(flags)[V153] == 5) {
		hotspot_set_active("BROWN VINE  ", true);
		hotspot_set_active("BROWN VINE COIL    ", true);
	}

	if (_G(flags)[V171] == 5) {
		hotspot_set_active("VINES  ", true);
		hotspot_set_active("COIL OF VINES    ", true);
	}
}

void Room504::setMiscItems() {
	hotspot_set_active("WHEELED TOY", false);

	hotspot_set_active("SHOVEL", false);
	deleteHotspot("SHOVEL ");

	hotspot_set_active("WOODEN LADDER", false);
	hotspot_set_active("WOODEN LADDER ", false);
	hotspot_set_active("DRIFTWOOD STUMP", false);
	hotspot_set_active("POLE", false);
	hotspot_set_active("HOLE", false);
	hotspot_set_active("HOLE ", false);
	hotspot_set_active("PERSON IN HOLE", false);
	hotspot_set_active("MENENDEZ", false);

	terminateMachineAndNull(_ladder);
	terminateMachineAndNull(_toy);
	terminateMachineAndNull(_shovel);
	terminateMachineAndNull(_driftwood);
	terminateMachineAndNull(_pole);

	if (_G(flags)[V155] == 0) {
		_ladder = series_place_sprite("MENENDEZ LADDER", 0, 640, 0, 100, 0xf00);
		hotspot_set_active("WOODEN LADDER", true);
	}

	if (_G(flags)[V155] == 1 && inv_object_is_here("WOODEN LADDER")) {
		_ladder = series_place_sprite("504 DROPPED LADDER", 0, 0, 0, 100, 0xe00);
		hotspot_set_active("WOODEN LADDER ", true);
	}

	if (inv_object_is_here("WHEELED TOY")) {
		_toy = series_place_sprite("504TOY", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("WHEELED TOY", true);
	}

	if (_G(flags)[V044] == 0 && inv_object_is_here("SHOVEL") &&
			_G(flags)[V141] == 1) {
		_shovel = series_place_sprite("504shov", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("SHOVEL", true);
	}

	if (_G(flags)[V044] == 1 && inv_object_is_here("SHOVEL") &&
			_G(flags)[V141] == 1) {
		_shovel = series_place_sprite("504 DROPPED SHOVEL", 0, 0, 0, 100, 0xff0);
		addShovel();
	}

	if (inv_object_is_here("DRIFTWOOD STUMP")) {
		_shovel = series_place_sprite("504 DROPPED DRIFTWOOD", 0, 0, 0, 100, 0x750);
		hotspot_set_active("DRIFTWOOD STUMP", true);
	}

	if (inv_object_is_here("POLE")) {
		_shovel = series_place_sprite("504 DROPPED POLE", 0, 0, 0, 100, 0x750);
		hotspot_set_active("POLE", true);
	}

	if (_G(flags)[V141] != 0)
		hotspot_set_active("HOLE ", true);
	else if (_G(flags)[V040] != 1)
		hotspot_set_active("PERSON IN HOLE", true);
	else
		hotspot_set_active("MENENDEZ", true);
}

void Room504::disableVinesRope() {
	static const char *HOTSPOTS[] = {
		"ROPE ", "GREEN VINE ", "BROWN VINE ", "VINES ",
		"ROPE  ", "GREEN VINE  ", "BROWN VINE  "
		"VINES  ", "ROPE COIL ", "GREEN VINE COIL ",
		"BROWN VINE COIL ", "COIL OF VINES ",
		"ROPE COIL  ", "GREEN VINE COIL  ",
		"BROWN VINE COIL  ", "COIL OF VINES  ",
		"ROPE COIL   ", "GREEN VINE COIL   ",
		"BROWN VINE COIL   ", "COIL OF VINES   ",
		"ROPE COIL    ", "GREEN VINE COIL    ",
		"BROWN VINE COIL    ", "COIL OF VINES    ",
		nullptr
	};
	for (const char **hs = HOTSPOTS; *hs; ++hs)
		hotspot_set_active(*hs, false);

	deleteHotspot("ROPE   ");
	deleteHotspot("GREEN VINE   ");
	deleteHotspot("BROWN VINE   ");
}

void Room504::deleteHotspot(const char *hotspotName) {
	for (auto *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next) {
		if (!strcmp(hs->vocab, hotspotName)) {
			_G(currentSceneDef).hotspots = hotspot_delete_record(
				_G(currentSceneDef).hotspots, hs);
			break;
		}
	}
}

void Room504::addRope() {
	if (_G(flags)[V152] != 2 && _G(flags)[V153] != 2)
		addHotspot(145, 73, 460, 118, "LOOK AT", "ROPE   ");
	if (_G(flags)[V152] == 2 && _G(flags)[V153] != 2)
		addHotspot(145, 73, 460, 95, "LOOK AT", "ROPE   ");
	if (_G(flags)[V152] == 2 && _G(flags)[V153] == 2)
		addHotspot(145, 73, 460, 88, "LOOK AT", "ROPE   ");
}

void Room504::addGreenVine() {
	if (_G(flags)[V154] != 2 && _G(flags)[V153] != 2)
		addHotspot(145, 73, 460, 118, "LOOK AT", "GREEN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V153] != 2)
		addHotspot(145, 96, 460, 118, "LOOK AT", "GREEN VINE   ");
	if (_G(flags)[V154] != 2 && _G(flags)[V153] == 2)
		addHotspot(145, 73, 460, 95, "LOOK AT", "GREEN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V153] == 2)
		addHotspot(145, 89, 460, 104, "LOOK AT", "GREEN VINE   ");
}

void Room504::addBrownVine() {
	if (_G(flags)[V154] != 2 && _G(flags)[V152] != 2)
		addHotspot(145, 73, 460, 118, "LOOK AT", "BROWN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V152] != 2)
		addHotspot(145, 96, 460, 118, "LOOK AT", "BROWN VINE   ");
	if (_G(flags)[V154] != 2 && _G(flags)[V152] == 2)
		addHotspot(145, 96, 460, 118, "LOOK AT", "BROWN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V152] == 2)
		addHotspot(145, 104, 460, 118, "LOOK AT", "BROWN VINE   ");
}

void Room504::addHotspot(int x1, int y1, int x2, int y2,
		const char *verb, const char *vocab) {
	HotSpotRec *hs = hotspot_new(x1, y1, x2, y2);
	hotspot_newVerb(hs, verb);
	hotspot_newVocab(hs, vocab);
	hs->cursor_number = 6;
	hotspot_add(_G(currentSceneDef).hotspots, hs, true);
}

void Room504::addShovel() {
	HotSpotRec *hs = hotspot_new(529, 109, 546, 143);
	hotspot_newVerb(hs, "LOOK AT");
	hotspot_newVocab(hs, "SHOVEL ");
	hs->cursor_number = 6;
	hs->feet_x = 543;
	hs->feet_y = 142;
	hs->facing = 11;

	_G(currentSceneDef).hotspots = hotspot_add(_G(currentSceneDef).hotspots, hs, true);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
