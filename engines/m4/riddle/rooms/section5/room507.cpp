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

#include "m4/riddle/rooms/section5/room507.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *BLOCK_NAMES[8] = {
	nullptr, "507 BLOCK ONE", "507 BLOCK TWO", "507 BLOCK THREE",
	"507 BLOCK FOUR", "507 BLOCK FIVE", "507 BLOCK SIX",
	"507 BLOCK SEVEN"
};
static const char *BLOCK_DIGI[8] = {
	nullptr, "507_s03", "507_s03a", "507_s03b", "507_s03c",
	"507_s03d", "507_s0e", "507_s0f"
};
int BLOCK_CORRECT[8] = {
	-1, 12, 4, 12, 8, 0, 8, 4
};

Room507::Room507() : Room() {
	Common::fill(_blockSeries, _blockSeries + 8, 0);
	Common::fill(_block, _block + 8, (machine *)nullptr);
	Common::fill(_blockFlag, _blockFlag + 8, false);
}

void Room507::preload() {
	_G(player).walker_in_this_scene = false;
	LoadWSAssets("OTHER SCRIPT");
}

void Room507::init() {
	digi_preload("507_s02");
	digi_preload("507_s03");
	digi_preload("507_s03a");
	digi_preload("507_s03b");
	digi_preload("507_s03c");
	digi_preload("507_s03d");
	digi_preload("507_s03e");
	digi_preload("507_s03f");
	digi_preload("507_s04");

	for (int i = 1; i <= 7; ++i) {
		_blockFlag[i] = false;
		_blockSeries[i] = series_load(BLOCK_NAMES[1]);
		_block[1] = series_show(BLOCK_NAMES[1], 0x400, 16, -1, -1, _G(flags)[V172 + i]);
	}

	_doorSeries = series_load("507DOOR2");
	_door = series_show("507DOOR2", 0x400, 16);
	digi_play_loop("507_s02", 3, 170);

	if (_G(game).previous_room == 506)
		digi_unload("506_s01");

	if (!player_been_here(507)) {
		kernel_timing_trigger(1, 8);
		player_set_commands_allowed(false);
	}
}

void Room507::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		break;

	case 3:
		for (int i = 1; i <= 7; ++i) {
			if (_G(flags)[V172 + i] != BLOCK_CORRECT[i]) {
				player_set_commands_allowed(true);
				return;
			}
		}

		kernel_timing_trigger(10, 4);
		_G(flags)[V156] = 1;
		break;

	case 4:
		digi_stop(1);
		terminateMachineAndNull(_door);
		_door = series_stream("507 DOOR LOWERS", 8, 0x400, -1);
		series_stream_break_on_frame(_door, 19, 6);
		digi_play("507_S04", 2);
		break;

	case 6:
		disable_player_commands_and_fade_init(6);
		break;

	case 7:
		for (int i = 1; i <= 7; ++i)
			digi_unload(BLOCK_DIGI[i]);
		digi_stop(1);
		digi_play_loop("507_s02", 3, 170);
		adv_kill_digi_between_rooms(false);
		break;

	case 8:
		digi_play("507R01", 1, 255, 1);
		break;

	default:
		break;
	}
}

void Room507::pre_parser() {
	_G(player).resetWalk();
}

void Room507::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool useFlag = player_said("gear");

	if (lookFlag && player_said("DOOR")) {
		digi_play("507r03", 1);
	} else if (lookFlag && player_said_any("STONE GLYPH ", "STONE GLYPH  ",
			"STONE GLYPH   ", "STONE GLYPH    ", "STONE GLYPH     ",
			"STONE GLYPH      ", "STONE GLYPH       ")) {
		if (_G(flags)[V172])
			digi_play("508R04", 1, 255, -1, 508);
		else
			digi_play("507R04", 1);
	} else if (lookFlag && player_said("SPIDER")) {
		digi_play("507R05", 1);
	} else if (lookFlag && player_said(" ")) {
		digi_play("507R02", 1);
	} else if (useFlag && player_said("STONE GLYPH ")) {
		useStoneGlyph(1);
	} else if (useFlag && player_said("STONE GLYPH  ")) {
		useStoneGlyph(2);
	} else if (useFlag && player_said("STONE GLYPH   ")) {
		useStoneGlyph(3);
	} else if (useFlag && player_said("STONE GLYPH    ")) {
		useStoneGlyph(4);
	} else if (useFlag && player_said("STONE GLYPH     ")) {
		useStoneGlyph(5);
	} else if (useFlag && player_said("STONE GLYPH      ")) {
		useStoneGlyph(6);
	} else if (useFlag && player_said("STONE GLYPH       ")) {
		useStoneGlyph(7);
	} else if (player_said("SHOVEL", "DOOR") && inv_player_has("SHOVEL")) {
		digi_play("507R07", 1);
	} else if (player_said("WOODEN LADDER", "DOOR") && inv_player_has("WOODEN LADDER")) {
		digi_play("507R07", 1);
	} else if (player_said("exit")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			disable_player_commands_and_fade_init(2);
			break;
		case 2:
			adv_kill_digi_between_rooms(false);
			digi_preload("507_s02");
			digi_play_loop("507_s02", 3, 100);
			_G(game).setRoom(506);
			break;
		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room507::useStoneGlyph(int blockNum) {
	player_set_commands_allowed(false);

	if (!_blockFlag[blockNum]) {
		terminateMachineAndNull(_block[blockNum]);
		_block[blockNum] = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallbackNegative, BLOCK_NAMES[blockNum]);
		_blockFlag[blockNum] = true;
		_G(flags)[V172] = 1;
	}

	_G(kernel).trigger_mode = KT_DAEMON;
	sendWSMessage_190000(_block[blockNum], 15);
	sendWSMessage_10000(1, _block[blockNum], _blockSeries[blockNum], _G(flags)[V172 + blockNum] + 1, _G(flags)[V172 + blockNum] + 4, 3,
		_blockSeries[blockNum], _G(flags)[V172 + blockNum] + 5, _G(flags)[V172 + blockNum] + 5, 0);
	digi_play("507_s93", 2);

	_G(flags)[V172 + blockNum] += 4;
	if (_G(flags)[V172 + blockNum] > 16)
		_G(flags)[V172 + blockNum] = 0;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
