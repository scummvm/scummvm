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
#include "mads/core/mcga.h"
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _blinkFl;
	bool _flippedFl;
	int32 _buttonId;
	int32 _lastFrame;
	int32 _leftItemId;
	int32 _posX;
	int32 _rightItemId;
};

static Scratch local;


static void setRightView(int view) {
	if (local._rightItemId < 8) kernel_seq_delete(g_sequence_ids[10]);

	int spriteNum;
	switch (view) {
	case 0:
		spriteNum = 16;
		break;

	case 1:
		spriteNum = 14;
		break;

	case 2:
		spriteNum = 17;
		break;

	case 3:
		spriteNum = 15;
		break;

	default:
		spriteNum = view + 6;
		break;
	}

	if (view != 8) {
		g_sequence_ids[10] = kernel_seq_stamp(g_sprite_ids[spriteNum], false, 1);
		kernel_seq_depth(g_sequence_ids[10], 0);
	}

	global[kRightView320] = local._rightItemId = view;
}

static void setLeftView(int view) {
	if (local._leftItemId < 10)
		kernel_seq_delete(g_sequence_ids[0]);

	if (view != 10) {
		g_sequence_ids[0] = kernel_seq_backward(g_sprite_ids[view], false, 6, 18, 0, 0);
		kernel_seq_depth(g_sequence_ids[0], 0);
		if (!local._blinkFl)
			kernel_seq_range(g_sequence_ids[0], 2, 2);
	}

	local._leftItemId = view;
}

static void handleButtons() {
	switch (player2.words[1]) {
	case words_red_button:
		local._buttonId = 5;
		break;

	case words_green_button:
		local._buttonId = 4;
		break;

	case words_right_1_key:
		local._buttonId = 6;
		break;

	case words_right_2_key:
		local._buttonId = 7;
		break;

	case words_right_3_key:
		local._buttonId = 8;
		break;

	case words_right_4_key:
		local._buttonId = 9;
		break;

	case words_right_5_key:
		local._buttonId = 10;
		break;

	case words_right_6_key:
		local._buttonId = 11;
		break;

	case words_right_7_key:
		local._buttonId = 12;
		break;

	case words_right_8_key:
		local._buttonId = 13;
		break;

	case words_left_1_key:
		local._buttonId = 0;
		break;

	case words_left_2_key:
		local._buttonId = 1;
		break;

	case words_left_3_key:
		local._buttonId = 2;
		break;

	case words_left_4_key:
		local._buttonId = 3;
		break;

	default:
		break;
	}

	if (local._buttonId <= 3) {
		local._posX = (8 * local._buttonId) - 2;
		local._flippedFl = true;
	} else if (local._buttonId <= 5) {
		local._posX = (13 * local._buttonId) - 14;
		local._flippedFl = true;
	} else {
		local._posX = (8 * local._buttonId) + 98;
		local._flippedFl = false;
	}
}

static void room_320_init() {
	local._blinkFl = true;
	local._rightItemId = 8;
	local._leftItemId = 10;
	local._lastFrame = 0;

	for (int i = 0; i < 10; i++)
		g_sprite_ids[i] = kernel_load_series(kernel_name('M', i), 0);

	for (int i = 0; i < 8; i++)
		g_sprite_ids[10 + i] = kernel_load_series(kernel_name('N', i), 0);

	g_sprite_ids[18] = kernel_load_series("*REXHAND", 0);
	player.walker_visible = false;

	setRightView(global[kRightView320]);
	setLeftView(0);

	pal_change_color(252, 63, 30, 20);
	pal_change_color(253, 45, 15, 10);

	section_3_music();
}

static void room_320_daemon() {
	if (kernel_anim[0].anim != nullptr) {
		if (local._lastFrame != kernel_anim[0].frame) {
			local._lastFrame = kernel_anim[0].frame;
			switch (local._lastFrame) {
			case 95:
				local._blinkFl = true;
				setLeftView(9);
				g_engine->_soundManager->command(41, 0);
				break;

			case 139:
				local._blinkFl = false;
				setLeftView(9);
				break;

			case 191:
				kernel_message_add(quote_string(kernel.quotes, 0xFE), 1, 1, 0xFDFC, 60, 0, 0);
				break;

			case 417:
			case 457:
				mcga_shakes = 40;
				g_engine->_soundManager->command(59, 0);
				break;

			case 430:
				local._blinkFl = true;
				setLeftView(4);
				break;

			default:
				break;
			}
		}
	}

	if (kernel.trigger == 70) {
		global[kAfterHavoc] = true;
		global[kTeleporterRoom + 1] = 351;
		new_room = 361;
	}
}

static void room_320_parser() {
	if (player.look_around)
		text_show(32011);
	else if ((player_said_1(press) || player_said_1(push)) &&
		(player_said_1(left_1_key) || player_said_1(left_2_key) || player_said_1(left_3_key) || player_said_1(left_4_key) ||
			player_said_1(green_button) || player_said_1(red_button) || player_said_1(right_1_key) || player_said_1(right_2_key) ||
			player_said_1(right_3_key) || player_said_1(right_4_key) || player_said_1(right_5_key) || player_said_1(right_6_key) ||
			player_said_1(right_7_key) || player_said_1(right_8_key)
			)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			handleButtons();
			g_sequence_ids[18] = kernel_seq_pingpong(g_sprite_ids[18], local._flippedFl, 4, 0, 0, 2);
			kernel_seq_scale(g_sequence_ids[18], 60);
			kernel_seq_loc(g_sequence_ids[18], local._posX, 170);
			kernel_seq_depth(g_sequence_ids[18], 0);
			kernel_seq_trigger(g_sequence_ids[18], KERNEL_TRIGGER_LOOP, 0, 1);
			kernel_seq_trigger(g_sequence_ids[18], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			if (local._buttonId >= 6) {
				g_engine->_soundManager->command(60, 0);
				setRightView(local._buttonId - 6);
			}
			if (local._buttonId == 4) {
				g_engine->_soundManager->command(38, 0);
				if (local._leftItemId == 3)
					setLeftView(0);
				else
					setLeftView(3);
			}
			if (local._buttonId == 5) {
				g_engine->_soundManager->command(38, 0);
				if (local._leftItemId == 1)
					setLeftView(2);
				else
					setLeftView(1);
			}
			if (local._buttonId <= 3) {
				g_engine->_soundManager->command(60, 0);
				setLeftView(local._buttonId + 5);
			}
			break;

		case 2:
			player.commands_allowed = true;
			if (local._buttonId == 5) {
				if (local._leftItemId == 2) {
					player.commands_allowed = false;
					setRightView(8);
					setLeftView(10);
					kernel_message_purge();
					kernel_dump_all();
					g_sprite_ids[2] = kernel_load_series(kernel_name('m', 2), 0);
					g_sprite_ids[4] = kernel_load_series(kernel_name('m', 4), 0);
					g_sprite_ids[9] = kernel_load_series(kernel_name('m', 9), 0);
					local._blinkFl = false;
					setLeftView(2);
					kernel.quotes = quote_load(254, 0);
					kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
					kernel_run_animation(kernel_name('a', -1), 70);
					g_engine->_soundManager->command(17, 0);
				}
			}
			break;

		default:
			break;
		}
	} else if (player_said_2(leave, security_station))
		new_room = 311;
	else if (player_said_2(look, right_monitor))
		text_show(32001);
	else if (player_said_2(look, left_monitor))
		text_show(32002);
	else if (player_said_2(look, desk))
		text_show(32003);
	else if (player_said_2(look, security_station))
		text_show(32004);
	else if (player_said_2(look, mug))
		text_show(32005);
	else if (player_said_2(look, doughnut))
		text_show(32006);
	else if (player_said_2(look, magazine))
		text_show(32007);
	else if (player_said_2(look, paper_football))
		text_show(32008);
	else if (player_said_2(look, newspaper))
		text_show(32009);
	else if (player_said_2(look, clipboard))
		text_show(32010);
	else if (player_said_2(take, mug))
		text_show(32012);
	else if (player_said_2(take, clipboard))
		text_show(32013);
	else if (player_said_2(take, doughnut) || player_said_2(eat, doughnut))
		text_show(32014);
	else if (player_said_2(take, paper_football))
		text_show(32015);
	else if (player_said_2(take, magazine))
		text_show(32016);
	else if (player_said_2(take, newspaper))
		text_show(32017);
	else
		return;

	player.command_ready = false;
}

void room_320_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._blinkFl);
	s.syncAsByte(local._flippedFl);

	s.syncAsSint32LE(local._buttonId);
	s.syncAsSint32LE(local._lastFrame);
	s.syncAsSint32LE(local._leftItemId);
	s.syncAsSint32LE(local._posX);
	s.syncAsSint32LE(local._rightItemId);
}

void room_320_preload() {
	room_init_code_pointer = room_320_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_320_parser;
	room_daemon_code_pointer = room_320_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
