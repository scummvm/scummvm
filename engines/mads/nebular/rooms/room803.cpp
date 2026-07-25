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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section8.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_803_init() {
	global[kBetweenRooms] = false;
	player.walker_visible = false;
	g_sprite_ids[1] = kernel_load_series(kernel_name('f', 1), 0);
	g_sprite_ids[9] = kernel_load_series("*RXMBD_2", 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('d', 1), 0);

	kernel.quotes = quote_load(0x31B, 0x31C, 0x31D, 0x31E, 0x31F, 0x320, 0x321, 0x322, 0);

	if (global[kHoppyDead]) {
		g_sprite_ids[7] = kernel_load_series(kernel_name('e', 1), 0);
		g_sequence_ids[7] = _scene->_sequences.startCycle(g_sprite_ids[7], false, 1);
		int idx = _scene->_dynamicHotspots.add(words_guts, words_walkto, g_sequence_ids[7], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(66, 123), FACING_SOUTH);
	}

	if (!global[kBeamIsUp] && !global[kReturnFromCut] && (!global[kFromCockpit] || global[kExitShip])) {
		g_sequence_ids[1] = _scene->_sequences.addSpriteCycle(g_sprite_ids[1], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[1], 2, 2);
		_scene->_sequences.setDepth(g_sequence_ids[1], 1);
	}

	if (!global[kFromCockpit]) {
		if (!global[kReturnFromCut]) {
			if (previous_room != RETURNING_FROM_DIALOG) {
				player.x = 15;
				player.y = 130;
				player.facing = FACING_EAST;
			}
			player.walker_visible = true;
		} else if (!global[kBeamIsUp]) {
			g_sprite_ids[3] = kernel_load_series(kernel_name('a', 1), 0);
			g_sprite_ids[2] = kernel_load_series(kernel_name('a', 3), 0);
			g_sprite_ids[4] = kernel_load_series(kernel_name('a', 2), 0);
			player.walker_visible = false;
			player.commands_allowed = false;
			g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 1);
			_scene->_sequences.setDepth(g_sequence_ids[3], 15);
			g_sequence_ids[3] = _scene->_sequences.addSpriteCycle(g_sprite_ids[3], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(g_sequence_ids[3], 1);
			_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			g_engine->_soundManager->command(14, 0);
		}

		if (global[kBeamIsUp] && !global[kReturnFromCut]) {
			if (global[kForceBeamDown])
				player.walker_visible = false;
			else
				player.walker_visible = true;

			g_sprite_ids[5] = kernel_load_series(kernel_name('b', 1), 0);
			g_engine->_soundManager->command(15, 0);
			player.commands_allowed = false;
			g_sequence_ids[5] = _scene->_sequences.addSpriteCycle(g_sprite_ids[5], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[5], 1, 6);
			_scene->_sequences.setDepth(g_sequence_ids[5], 1);
			_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_EXPIRE, 0, 100);
		}
	} else if (!global[kExitShip]) {
		if (!global[kBeamIsUp]) {
			g_sprite_ids[3] = kernel_load_series(kernel_name('a', 1), 0);
			player.walker_visible = false;
			player.commands_allowed = false;
			g_sequence_ids[3] = _scene->_sequences.addSpriteCycle(g_sprite_ids[3], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(g_sequence_ids[3], 1);
			_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 130);
			g_engine->_soundManager->command(14, 0);
		} else {
			g_sprite_ids[8] = kernel_load_series(kernel_name('c', 1), 0);
			player.walker_visible = false;
			player.commands_allowed = false;
			g_sequence_ids[8] = _scene->_sequences.startCycle(g_sprite_ids[8], false, 1);
			g_sequence_ids[8] = _scene->_sequences.addSpriteCycle(g_sprite_ids[8], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(g_sequence_ids[8], 1);
			_scene->_sequences.addSubEntry(g_sequence_ids[8], SEQUENCE_TRIGGER_EXPIRE, 0, 140);
		}
	} else {
		player.commands_allowed = false;
		player.x = 197;
		player.y = 96;
		player.facing = FACING_SOUTHWEST;
		player.walker_visible = true;
		g_sprite_ids[6] = kernel_load_series(kernel_name('d', 1), 0);
		g_sequence_ids[6] = _scene->_sequences.startCycle(g_sprite_ids[6], false, 19);
		_scene->_sequences.addTimer(1, 150);
	}

	section_8_music();
}

static void room_803_daemon() {
	if (kernel.trigger == 120) {
		g_sequence_ids[6] = _scene->_sequences.startCycle(g_sprite_ids[6], false, 19);
		new_room = 804;
	}

	if (kernel.trigger == 100) {
		g_sequence_ids[1] = _scene->_sequences.addSpriteCycle(g_sprite_ids[1], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[1], 2, 2);
		_scene->_sequences.setDepth(g_sequence_ids[1], 1);
		if (!global[kHoppyDead]) {
			g_sequence_ids[5] = _scene->_sequences.addSpriteCycle(g_sprite_ids[5], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[5], 7, 12);
			_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_EXPIRE, 0, 101);
		} else {
			g_sequence_ids[5] = _scene->_sequences.startCycle(g_sprite_ids[5], false, -2);
			int idx = _scene->_dynamicHotspots.add(words_guts, words_walkto, g_sequence_ids[5], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(66, 123), FACING_SOUTH);
			g_engine->_soundManager->command(16, 0);
			global[kCameFromCut] = true;
			global[kBeamIsUp] = false;
			global[kReturnFromCut] = false;
			global[kDontRepeat] = false;
			global[kHoppyDead] = true;
			global[kHasWatchedAntigrav] = true;

			if (global[kForceBeamDown])
				new_room = previous_room;
			else
				player.commands_allowed = true;
		}
	}

	if (kernel.trigger == 101) {
		g_sequence_ids[5] = _scene->_sequences.startCycle(g_sprite_ids[5], false, -2);
		int idx = _scene->_dynamicHotspots.add(words_guts, words_walkto, g_sequence_ids[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(66, 123), FACING_SOUTH);
		g_engine->_soundManager->command(16, 0);
		global[kCameFromCut] = true;
		global[kBeamIsUp] = false;
		global[kReturnFromCut] = false;
		global[kDontRepeat] = false;
		global[kHoppyDead] = true;
		global[kHasWatchedAntigrav] = true;

		if (global[kForceBeamDown])
			new_room = previous_room;
		else
			player.commands_allowed = true;
	}

	if (kernel.trigger == 80) {
		if (!global[kHoppyDead])
			_scene->_sequences.addTimer(350, 70);

		g_sequence_ids[4] = _scene->_sequences.addSpriteCycle(g_sprite_ids[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[4], 1, 3);
		_scene->_sequences.addSubEntry(g_sequence_ids[4], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
	}

	if (kernel.trigger == 70) {
		g_sequence_ids[2] = _scene->_sequences.addSpriteCycle(g_sprite_ids[2], false, 8, 1, 0, 0);
		_scene->_sequences.addSubEntry(g_sequence_ids[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		g_engine->_soundManager->command(31, 0);
	}

	if (kernel.trigger == 71)
		_scene->_sequences.addTimer(200, 110);

	if (kernel.trigger == 90) {
		int syncIdx = g_sequence_ids[4];
		g_sequence_ids[4] = _scene->_sequences.startPingPongCycle(g_sprite_ids[4], false, 15, 0, 0, 0);
		_scene->_sequences.updateTimeout(g_sequence_ids[4], syncIdx);
		_scene->_sequences.setAnimRange(g_sequence_ids[4], 4, 9);
		if (global[kHoppyDead])
			_scene->_sequences.addTimer(200, 110);
	}

	if (kernel.trigger == 110)
		new_room = 808;

	if (kernel.trigger == 130) {
		global[kBeamIsUp] = true;
		new_room = 804;
	}

	if (kernel.trigger == 140) {
		if (!global[kWindowFixed]) {
			new_room = 810;
			global[kInSpace] = true;
		} else {
			if (!global[kShieldModInstalled])
				win_status = 1;
			else if (!global[kTargetModInstalled])
				win_status = 2;
			else
				win_status = 3;

			game.going = false;
			return;
		}
	}

	if (kernel.trigger == 150) {
		_scene->_sequences.remove(g_sequence_ids[6]);
		g_engine->_soundManager->command(18, 0);
		g_sequence_ids[6] = _scene->_sequences.addReverseSpriteCycle(g_sprite_ids[6], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[6], 1, 19);
		_scene->_sequences.setDepth(g_sequence_ids[6], 4);
		_scene->_sequences.addSubEntry(g_sequence_ids[6], SEQUENCE_TRIGGER_EXPIRE, 0, 151);
	}

	if (kernel.trigger == 151) {
		global[kBeamIsUp] = false;
		global[kFromCockpit] = false;
		global[kExitShip] = false;
		player.commands_allowed = true;
	}
}

static void room_803_pre_parser() {
	if (player_said_2(walk_down, path_to_west))
		player.walk_off_edge_to_room = 802;

	if (player_said_2(take, ship))
		player.need_to_walk = false;
}

static void room_803_parser() {
	if (player_said_2(take, guts)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[9] = _scene->_sequences.addSpriteCycle(g_sprite_ids[9], true, 6, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[9], 1, 4);
			_scene->_sequences.setMsgLayout(g_sequence_ids[9]);
			_scene->_sequences.addSubEntry(g_sequence_ids[9], SEQUENCE_TRIGGER_EXPIRE, 0, 160);
			break;

		case 160:
		{
			int syncIdx = g_sequence_ids[9];
			g_sequence_ids[9] = _scene->_sequences.startCycle(g_sprite_ids[9], false, 4);
			_scene->_sequences.updateTimeout(g_sequence_ids[9], syncIdx);
			_scene->_sequences.setMsgLayout(g_sequence_ids[9]);
			_scene->_sequences.addTimer(60, 161);
		}
		break;

		case 161:
		{
			int quoteId = 0x31A + g_engine->getRandomNumber(1, 8);
			_scene->_kernelMessages.add(Common::Point(64, 67), 0x1110, 32, 0, 80, quote_string(kernel.quotes, quoteId));
			_scene->_sequences.addTimer(60, 162);
		}
		break;

		case 162:
			_scene->_sequences.remove(g_sequence_ids[9]);
			g_sequence_ids[9] = _scene->_sequences.addReverseSpriteCycle(g_sprite_ids[9], true, 6, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[9], 1, 4);
			_scene->_sequences.setMsgLayout(g_sequence_ids[9]);
			_scene->_sequences.addSubEntry(g_sequence_ids[9], SEQUENCE_TRIGGER_EXPIRE, 0, 163);
			break;

		case 163:
			player.clock = kernel.clock + player.frame_delay;
			player.walker_visible = true;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(enter, ship)) {
		g_engine->_soundManager->command(17, 0);
		player.commands_allowed = false;
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		g_sequence_ids[6] = _scene->_sequences.addSpriteCycle(g_sprite_ids[6], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[6], 1, 19);
		_scene->_sequences.setDepth(g_sequence_ids[6], 4);
		_scene->_sequences.addSubEntry(g_sequence_ids[6], SEQUENCE_TRIGGER_EXPIRE, 0, 120);
		global[kBeamIsUp] = false;
	} else if (player_said_2(look, launch_pad))
		text_show(80310);
	else if (player.look_around)
		text_show(80310);
	else if (player_said_2(look, pad_to_west))
		text_show(80311);
	else if (player_said_2(look, guts)) {
		if (config_file.naughtiness == STORYMODE_NICE)
			text_show(80312);
		else
			text_show(80313);
	} else if (player_said_2(look, bushes))
		text_show(80315);
	else if (player_said_2(look, ship))
		text_show(80317);
	else if (player_said_2(look, tower))
		text_show(80318);
	else if (player_said_2(look, tree) || player_said_2(look, trees))
		text_show(80319);
	else if (player_said_2(look, sky))
		text_show(80320);
	else if (player_said_2(take, ship))
		text_show(80321);
	else
		return;

	player.command_ready = false;
}

void room_803_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_803_preload() {
	room_init_code_pointer = room_803_init;
	room_daemon_code_pointer = room_803_daemon;
	room_pre_parser_code_pointer = room_803_pre_parser;
	room_parser_code_pointer = room_803_parser;

	section_8_walker();
	section_8_interface();

	vocab_make_active(words_guts);
	vocab_make_active(words_walkto);

	if ((!global[kFromCockpit] && global[kReturnFromCut] && !global[kBeamIsUp])
		|| (global[kFromCockpit] && !global[kExitShip])) {
		*player.series_name = '\0';
		player.walker_must_reload = true;
	}
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
