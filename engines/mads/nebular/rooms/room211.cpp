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
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section2.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _ambushFl;
	bool _wakeFl;
	int32 _monkeyFrame;
	int32 _scrollY;
	int32 _monkeyTime;
};

static Scratch local;


static void room_211_init() {
	g_sprite_ids[1] = kernel_load_series("*SC002Z2", 0);
	local._wakeFl = false;

	if (previous_room == 210) {
		player.x = 25;
		player.y = 148;
	}
	else if (previous_room == 205) {
		player.x = 49;
		player.y = 133;
		player.facing = FACING_WEST;
		local._wakeFl = true;
		player.commands_allowed = false;
		player.walker_visible = false;
		kernel_run_animation(kernel_name('A', -1), 100);
		_scene->_animation[0]->setCurrentFrame(169);
	} else if (previous_room != RETURNING_FROM_DIALOG) {
		player.x = 310;
		player.y = 31;
		player.facing = FACING_SOUTHWEST;
	}

	if (g_engine->getRandomNumber(1, 8) == 1) {
		g_sequence_ids[2] = _scene->_sequences.addSpriteCycle(g_sprite_ids[1], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(g_sequence_ids[2], Common::Point(202, 126));
		_scene->_sequences.setDepth(g_sequence_ids[2], 8);
		_scene->_sequences.setMotion(g_sequence_ids[2], SEQUENCE_TRIGGER_SPRITE, -200, 0);
		_scene->_dynamicHotspots.add(words_slithering_snake, words_walkto, g_sequence_ids[2], Common::Rect(1, 1, 1 + 41, 1 + 10));
	}

	if (kernel.teleported_in)
		inter_give_to_player(OBJ_BINOCULARS);

	pal_change_color(252, 63, 44, 30);
	pal_change_color(253, 63, 20, 22);
	kernel.quotes = quote_load(0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 1, 0);

	if (global[kMonkeyStatus] == MONKEY_AMBUSH_READY)
		_scene->_kernelMessages.initRandomMessages(2,
			Common::Rect(0, 0, 54, 30), 13, 2, 0xFDFC, 60,
			151, 152, 153, 154, 0);

	local._monkeyTime = kernel.clock;
	local._scrollY = 30;

	local._ambushFl = false;
	local._monkeyFrame = 0;

	section_2_music();
}

static void room_211_daemon() {
	if (global[kMonkeyStatus] == MONKEY_AMBUSH_READY) {
		_scene->_kernelMessages.randomServer();

		if (!local._ambushFl && !local._wakeFl && (kernel.clock >= local._monkeyTime)) {
			int chanceMinor = _scene->_kernelMessages.checkRandom() * 4 + 1;
			if (_scene->_kernelMessages.generateRandom(80, chanceMinor))
				g_engine->_soundManager->command(18, 0);

			local._monkeyTime = kernel.clock + 2;
		}

		if ((Common::Point(player.x, player.y) == Common::Point(52, 132)) && (player.facing == FACING_WEST) && !player.walking &&
			(kernel.trigger || !local._ambushFl)) {
			switch (kernel.trigger) {
			case 0:
				if (player_has(OBJ_BINOCULARS)) {
					local._ambushFl = true;
					local._monkeyFrame = 0;
					player.commands_allowed = false;
					player.walker_visible = false;
					_scene->_kernelMessages.reset();
					kernel_run_animation(kernel_name('A', -1), 90);
					g_engine->_soundManager->command(19, 0);
					int count = (int)inven_num_objects;
					for (int idx = 0; idx < count; idx++) {
						if ((inven[idx] == OBJ_BINOCULARS) && (_scene->_userInterface._selectedInvIndex != idx))
							_scene->_userInterface.selectObject(idx);
					}
				}
				break;

			case 90:
				g_engine->_soundManager->command(10, 0);
				player.commands_allowed = true;
				player.walker_visible = true;
				player.x = 49;
				player.y = 133;
				local._ambushFl = false;
				global[kMonkeyStatus] = MONKEY_HAS_BINOCULARS;
				break;

			default:
				break;
			}
		}
	}

	if (local._ambushFl && (_scene->_animation[0]->getCurrentFrame() > local._monkeyFrame)) {
		local._monkeyFrame = _scene->_animation[0]->getCurrentFrame();
		switch (local._monkeyFrame) {
		case 2:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(12, 4), 0xFDFC, 0, 0, 60, quote_string(kernel.quotes, 157));
			_scene->_kernelMessages.setQuoted(msgIndex, 2, true);
		}
		break;

		case 12:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(35, 20), 0xFDFC, 0, 0, 60, quote_string(kernel.quotes, 155));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		}
		break;

		case 42:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(60, 45), 0xFDFC, 0, 0, 60, quote_string(kernel.quotes, 156));
			_scene->_kernelMessages.setQuoted(msgIndex, 6, true);
		}
		break;

		case 73:
			_scene->_kernelMessages.add(Common::Point(102, 95), 0xFDFC, 32, 0, 75, quote_string(kernel.quotes, 157));
			break;

		case 90:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(102, 95), 0xFDFC, 32, 0, 60, quote_string(kernel.quotes, 158));
			_scene->_kernelMessages.setQuoted(msgIndex, 6, true);
		}
		break;

		case 97:
			_scene->_userInterface.selectObject(-1);
			inter_take_from_player(OBJ_BINOCULARS, 1);
			break;

		case 177:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, quote_string(kernel.quotes, 161));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		case 181:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, quote_string(kernel.quotes, 162));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		case 188:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, quote_string(kernel.quotes, 163));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		case 200:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, quote_string(kernel.quotes, 164));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		default:
			break;
		}
	}

	if (local._wakeFl) {
		if (kernel.trigger == 100) {
			player.walker_visible = true;
			player.commands_allowed = true;
			local._wakeFl = false;
		}

		if (_scene->_animation[0]->getCurrentFrame() > local._monkeyFrame) {
			local._monkeyFrame = _scene->_animation[0]->getCurrentFrame();
			switch (_scene->_animation[0]->getCurrentFrame()) {
			case 177:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, quote_string(kernel.quotes, 165));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			case 181:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, quote_string(kernel.quotes, 166));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			case 188:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, quote_string(kernel.quotes, 167));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			case 200:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, quote_string(kernel.quotes, 168));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			default:
				break;
			}
		}
	}
}

static void room_211_pre_parser() {
	if (player_said_2(walk_down, jungle_path) && player_has(OBJ_BINOCULARS) && (global[kMonkeyStatus] == MONKEY_AMBUSH_READY)
		&& (_scene->_customDest.x <= 52) && (_scene->_customDest.y >= 132))
		player_walk(52, 132, FACING_WEST);

	if (player_said_2(walk_down, path_to_west)) {
		if (player_has(OBJ_BINOCULARS) && (global[kMonkeyStatus] == MONKEY_AMBUSH_READY))
			player_walk(52, 132, FACING_WEST);
		else
			player.walk_off_edge_to_room = 210;
	}

	if (player_said_2(walk_down, path_to_northeast))
		player.walk_off_edge_to_room = 207;
}

static void room_211_parser() {
	if (player.look_around && (global[kMonkeyStatus] == MONKEY_AMBUSH_READY))
		text_show(21111);
	else if (player_said_3(look, binoculars, palm_tree))
		text_show(21116);
	else if (player_said_2(look, bushy_fern))
		text_show(21101);
	else if (player_said_2(look, jungle_path))
		text_show(21102);
	else if (player_said_2(look, palm_tree)) {
		if (global[kMonkeyStatus] == MONKEY_AMBUSH_READY) {
			if (config_file.naughtiness == STORYMODE_NAUGHTY)
				text_show(21103);
			else
				text_show(21104);
		} else {
			text_show(21105);
		}
	} else if (player_said_2(look, thick_undergrowth)) {
		if (config_file.naughtiness == STORYMODE_NAUGHTY)
			text_show(21106);
		else
			text_show(21107);
	} else if (player_said_2(look, jungle))
		text_show(21108);
	else if (player_said_2(look, path_to_northeast))
		text_show(21109);
	else if (player_said_2(look, path_to_west))
		text_show(21110);
	else if (player_said_2(look, slithering_snake))
		text_show(21113);
	else if (player_said_2(take, slithering_snake))
		text_show(21114);
	else if (player_said_2(look, rocks))
		text_show(21115);
	else
		return;

	player.command_ready = false;
}

void room_211_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._ambushFl);
	s.syncAsByte(local._wakeFl);

	s.syncAsSint32LE(local._monkeyFrame);
	s.syncAsSint32LE(local._scrollY);
	s.syncAsUint32LE(local._monkeyTime);
}

void room_211_preload() {
	room_init_code_pointer = room_211_init;
	room_pre_parser_code_pointer = room_211_pre_parser;
	room_parser_code_pointer = room_211_parser;
	room_daemon_code_pointer = room_211_daemon;

	section_2_walker();
	section_2_interface();
	vocab_make_active(words_slithering_snake);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
