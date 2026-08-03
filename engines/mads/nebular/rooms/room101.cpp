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
#include "mads/nebular/rooms/section1.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _sittingFl;
	int16 _panelOpened;
	int16 _messageNum;
	int16 _posY;
	int16 _shieldSpriteIdx;
	int16 _chairHotspotId;
	int16 _oldSpecial;
};

static Scratch local;


static void room_101_say_dang() {
	kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
	player.commands_allowed = false;

	switch (kernel.trigger) {
	case 0:
		kernel_seq_delete(g_sequence_ids[11]);
		g_sequence_ids[11] = kernel_seq_pingpong(g_sprite_ids[11], false, 3, 0, 0, 6);
		kernel_seq_range(g_sequence_ids[11], 17, 21);
		kernel_seq_trigger(g_sequence_ids[11], KERNEL_TRIGGER_EXPIRE, 0, 72);
		g_engine->_soundManager->command(17, 0);
		g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 3, 0, 0, 2);
		break;

	case 72:
		g_sequence_ids[11] = kernel_seq_forward(g_sprite_ids[11], false, 6, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[11], 17, 17);
		kernel_message_add(quote_string(kernel.quotes, 57), 143, 61, 0x1110, 60, 0, 0);
		kernel_timing_trigger(120, 73);
		break;

	case 73:
		text_show(10117);
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void room_101_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 3), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('x', 4), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('x', 5), 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('x', 6), 0);
	g_sprite_ids[7] = kernel_load_series(kernel_name('x', 7), 0);
	g_sprite_ids[8] = kernel_load_series(kernel_name('m', -1), 0);
	g_sprite_ids[9] = kernel_load_series(kernel_name('b', 1), 0);
	g_sprite_ids[10] = kernel_load_series(kernel_name('b', 2), 0);
	g_sprite_ids[11] = kernel_load_series(kernel_name('a', 0), 0);
	g_sprite_ids[12] = kernel_load_series(kernel_name('a', 1), 0);
	g_sprite_ids[13] = kernel_load_series(kernel_name('x', 8), 0);
	g_sprite_ids[14] = kernel_load_series(kernel_name('x', 0), 0);

	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 5, 25, 0, 0);
	g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 4, 0, 1, 0);
	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 0, 2, 0);
	kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_SPRITE, 7, 70);
	g_sequence_ids[4] = kernel_seq_backward(g_sprite_ids[4], false, 10, 60, 0, 0);
	g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 5, 0, 1, 0);
	g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 10, 0, 2, 0);
	g_sequence_ids[7] = kernel_seq_forward(g_sprite_ids[7], false, 6, 0, 0, 0);
	g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 6, 4, 10, 0);
	g_sequence_ids[10] = kernel_seq_forward(g_sprite_ids[10], false, 6, 47, 32, 0);

	kernel_flip_hotspot(words_shield_modulator, false);
	local._panelOpened = false;

	if (previous_room != KERNEL_STARTING_GAME)
		global[kNeedToStandUp] = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 100;
		player.y = 152;
	}

	if ((previous_room == 112) || ((previous_room == KERNEL_RESTORING_GAME) && local._sittingFl)) {
		player.walker_visible = false;
		local._sittingFl = true;
		player.x = 161;
		player.y = 123;
		player.facing = FACING_NORTHEAST;
		g_sequence_ids[11] = kernel_seq_forward(g_sprite_ids[11], false, 3, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[11], 17, 17);
		kernel_flip_hotspot(words_chair, false);
		local._chairHotspotId = kernel_add_dynamic(words_chair, words_sit_in, 0, -1, 159, 84, 33, 36);
		if (previous_room == 112)
			room_101_say_dang();
	} else {
		g_sequence_ids[12] = kernel_seq_forward(g_sprite_ids[12], false, 6, 0, 0, 0);
		kernel_seq_depth(g_sequence_ids[12], 4);
	}

	kernel.quotes = quote_load(49, 50, 57, 54, 55, 56, 0);

	if (global[kNeedToStandUp]) {
		kernel_run_animation(kernel_full_name(101, 'S', -1, "", KERNEL_AA), 71);
		player.walker_visible = false;
		player.commands_allowed = false;
		player.x = 68;
		player.y = 140;
		player.facing = FACING_WEST;

		local._messageNum = 0;
		local._posY = 30;
	}

	local._oldSpecial = false;

	section_1_music();
}

static void room_101_daemon() {
	if (local._oldSpecial != player.special_code) {
		local._oldSpecial = player.special_code;
		if (local._oldSpecial)
			g_engine->_soundManager->command(39, 0);
		else
			g_engine->_soundManager->command(11, 0);
	}

	switch (kernel.trigger) {
	case 70:
		g_engine->_soundManager->command(9, 0);
		break;

	case 71:
		global[kNeedToStandUp] = false;
		player.walker_visible = true;
		player.commands_allowed = true;
		player.clock = kernel.clock - player.frame_delay;
		break;

	case 72:
	case 73:
		room_101_say_dang();
		break;

	default:
		break;
	}

	if (kernel_anim[0].anim != nullptr) {
		if ((kernel_anim[0].frame >= 6) && (local._messageNum == 0)) {
			local._messageNum++;
			kernel_message_add(quote_string(kernel.quotes, 49), 63, local._posY, 0x1110, 240, 0, 0);
			local._posY += 14;
		}

		if ((kernel_anim[0].frame >= 7) && (local._messageNum == 1)) {
			local._messageNum++;
			kernel_message_add(quote_string(kernel.quotes, 54), 63, local._posY, 0x1110, 240, 0, 0);
			local._posY += 14;
		}

		if ((kernel_anim[0].frame >= 10) && (local._messageNum == 2)) {
			local._messageNum++;
			kernel_message_add(quote_string(kernel.quotes, 55), 63, local._posY, 0x1110, 240, 0, 0);
			local._posY += 14;
		}

		if ((kernel_anim[0].frame >= 17) && (local._messageNum == 3)) {
			local._messageNum++;
			kernel_message_add(quote_string(kernel.quotes, 56), 63, local._posY, 0x1110, 240, 0, 0);
			local._posY += 14;
		}

		if ((kernel_anim[0].frame >= 20) && (local._messageNum == 4)) {
			local._messageNum++;
			kernel_message_add(quote_string(kernel.quotes, 50), 63, local._posY, 0x1110, 240, 0, 0);
			local._posY += 14;
		}
	}
}

static void room_101_pre_parser() {
	if (player_said_2(look, view_screen))
		player.need_to_walk = true;

	if (local._sittingFl) {
		if (player_said_1(look) || player_said_1(chair) || player_said_1(talkto) || player_said_1(peer_through) || player_said_1(examine))
			player.need_to_walk = false;

		if (player.need_to_walk) {
			switch (kernel.trigger) {
			case 0:
				player.ready_to_walk = false;
				player.commands_allowed = false;
				kernel_seq_delete(g_sequence_ids[11]);
				g_sequence_ids[11] = kernel_seq_backward(g_sprite_ids[11], false, 3, 0, 0, 1);
				kernel_seq_trigger(g_sequence_ids[11], KERNEL_TRIGGER_EXPIRE, 0, 1);
				kernel_seq_range(g_sequence_ids[11], 1, 17);
				g_engine->_soundManager->command(16, 0);
				break;

			case 1:
				local._sittingFl = false;
				player.walker_visible = true;
				player.commands_allowed = true;
				player.ready_to_walk = true;
				kernel_flip_hotspot(71, true);
				kernel_delete_dynamic(local._chairHotspotId);
				g_sequence_ids[12] = kernel_seq_forward(g_sprite_ids[12], false, 6, 0, 0, 0);
				kernel_seq_depth(g_sequence_ids[12], 4);
				break;

			default:
				break;
			}
		}
	}

	if (local._panelOpened && !(player_said_1(shield_access_panel) || player_said_1(shield_modulator))) {
		switch (kernel.trigger) {
		case 0:
			if (player.need_to_walk) {
				kernel_seq_delete(g_sequence_ids[13]);
				local._shieldSpriteIdx = object_is_here(OBJ_SHIELD_MODULATOR) ? 13 : 14;
				g_sequence_ids[13] = kernel_seq_backward(g_sprite_ids[local._shieldSpriteIdx], false, 6, 0, 0, 1);
				kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 1);
				player.commands_allowed = false;
				g_engine->_soundManager->command(20, 0);
			}
			break;

		case 1:
			player.commands_allowed = true;
			local._panelOpened = false;
			kernel_flip_hotspot(words_shield_modulator, false);
			break;

		default:
			break;
		}
	}
}

static void room_101_parser() {
	if (player.look_around) {
		text_show(10125);
		player.command_ready = false;
		return;
	}

	if (player_said_2(walkto, life_support_section)) {
		new_room = 102;
		player.command_ready = false;
		return;
	}

	if (player_said_2(sit_in, chair) || (player_said_2(look, view_screen) && !local._sittingFl)) {
		if (!local._sittingFl) {
			switch (kernel.trigger) {
			case 0:
				kernel_seq_delete(g_sequence_ids[12]);
				g_sequence_ids[11] = kernel_seq_forward(g_sprite_ids[11], false, 3, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[11], 1, 17);
				kernel_seq_trigger(g_sequence_ids[11], KERNEL_TRIGGER_SPRITE, 10, 1);
				kernel_seq_trigger(g_sequence_ids[11], KERNEL_TRIGGER_EXPIRE, 0, 2);
				player.commands_allowed = false;
				player.walker_visible = false;
				player.command_ready = false;
				return;

			case 1:
				g_engine->_soundManager->command(16, 0);
				break;

			case 2:
				g_sequence_ids[11] = kernel_seq_forward(g_sprite_ids[11], false, 3, 0, 0, 0);
				kernel_seq_range(g_sequence_ids[11], 17, 17);
				player.commands_allowed = true;
				local._sittingFl = true;
				kernel_flip_hotspot(71, false);
				local._chairHotspotId = kernel_add_dynamic(words_chair, words_sit_in, 0, -1, 159, 84, 33, 36);
				if (!player_said_2(look, view_screen)) {
					player.command_ready = false;
					return;
				}
				kernel.trigger = 0;
				break;

			default:
				break;
			}
		} else {
			text_show(10131);
			player.command_ready = false;
			return;
		}
	}

	if ((player_said_2(walkto, shield_access_panel) || player_said_2(open, shield_access_panel)) && !local._panelOpened) {
		switch (kernel.trigger) {
		case 0:
			local._shieldSpriteIdx = object_is_here(OBJ_SHIELD_MODULATOR) ? 13 : 14;
			g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[local._shieldSpriteIdx], false, 6, 0, 0, 1);
			kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 1);
			player.commands_allowed = false;
			g_engine->_soundManager->command(20, 0);
			break;

		case 1:
			kernel_seq_delete(g_sequence_ids[13]);
			g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[local._shieldSpriteIdx], false, 6, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[13], -2, -2);
			player.commands_allowed = true;
			local._panelOpened = true;
			if (object_is_here(OBJ_SHIELD_MODULATOR))
				kernel_flip_hotspot(words_shield_modulator, true);
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if ((player_said_2(take, shield_modulator) || player_said_2(pull, shield_modulator)) && object_is_here(OBJ_SHIELD_MODULATOR)) {
		inter_give_to_player(OBJ_SHIELD_MODULATOR);
		kernel_seq_delete(g_sequence_ids[13]);
		g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[14], false, 6, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[13], -2, -2);
		kernel_flip_hotspot(words_shield_modulator, false);
		object_examine(OBJ_SHIELD_MODULATOR, 10120, 0);
		g_engine->_soundManager->command(22, 0);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, shield_access_panel) || (player_said_2(look, shield_modulator) && !player_has(OBJ_SHIELD_MODULATOR))) {
		if (local._panelOpened) {
			if (object_is_here(OBJ_SHIELD_MODULATOR))
				text_show(10128);
			else
				text_show(10129);
		} else
			text_show(10127);

		player.command_ready = false;
		return;
	}

	if (player_said_2(open, shield_access_panel) && local._panelOpened) {
		text_show(10130);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, view_screen) && local._sittingFl) {
		if (global[kWatchedViewScreen])
			room_101_say_dang();
		else {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				kernel_seq_delete(g_sequence_ids[11]);
				g_sequence_ids[11] = kernel_seq_forward(g_sprite_ids[11], false, 3, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[11], 17, 21);
				kernel_seq_trigger(g_sequence_ids[11], KERNEL_TRIGGER_EXPIRE, 0, 1);
				g_engine->_soundManager->command(17, 0);
				break;

			case 1:
				g_sequence_ids[11] = kernel_seq_backward(g_sprite_ids[11], false, 3, 0, 0, 1);
				kernel_seq_trigger(g_sequence_ids[11], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_range(g_sequence_ids[11], 17, 21);
				break;

			case 2:
				g_sequence_ids[11] = kernel_seq_forward(g_sprite_ids[11], false, 3, 0, 0, 0);
				kernel_seq_range(g_sequence_ids[11], 17, 17);
				g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 3, 0, 0, 1);
				kernel_seq_trigger(g_sequence_ids[8], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				player.commands_allowed = true;
				global[kWatchedViewScreen] = true;
				local._sittingFl = true;
				new_room = 112;
				break;

			default:
				break;
			}
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, chair)) {
		text_show(10101);
		player.command_ready = false;
		return;
	}

	if ((player_said_1(look) || player_said_1(peer_through)) && (player_said_1(front_window) || player_said_1(outside))) {
		text_show(10102);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, hull) || player_said_2(look, outer_hull) || player_said_2(examine, hull) || player_said_2(examine, outer_hull)) {
		text_show(10103);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, fuzzy_dice)) {
		text_show(10104);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, mirror) || player_said_2(look_in, mirror)) {
		text_show(10105);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, curtains)) {
		text_show(10106);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, plastic_jesus)) {
		text_show(10107);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, escape_hatch) || (player_said_2(open, escape_hatch) && !player_has(OBJ_REBREATHER))) {
		text_show(10109);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, escape_hatch)) {
		text_show(10110);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, target_computer)) {
		text_show(10111);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, library_computer)) {
		text_show(10126);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, damage_control_panel)) {
		text_show(10112);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, navigation_controls)) {
		text_show(10113);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, engineering_controls)) {
		text_show(10114);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, weapons_display)) {
		text_show(10115);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, shield_status_panel)) {
		text_show(10116);
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, plastic_jesus)) {
		text_show(10118);
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, fuzzy_dice)) {
		text_show(10119);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, damage_control_panel)) {
		text_show(10121);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, curtains)) {
		text_show(10122);
		player.command_ready = false;
		return;
	}

	if (player_said_2(close, curtains)) {
		text_show(10123);
		player.command_ready = false;
		return;
	}

	if ((player_said_1(look) || player_said_1(play)) && player_said_1(video_game)) {
		text_show(10124);
		player.command_ready = false;
		return;
	}
}

void room_101_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._sittingFl);
	s.syncAsByte(local._panelOpened);
	s.syncAsSint16LE(local._messageNum);
	s.syncAsSint16LE(local._posY);
	s.syncAsSint16LE(local._shieldSpriteIdx);
	s.syncAsSint16LE(local._chairHotspotId);
	s.syncAsSint16LE(local._oldSpecial);
}

void room_101_preload() {
	room_init_code_pointer = room_101_init;
	room_pre_parser_code_pointer = room_101_pre_parser;
	room_parser_code_pointer = room_101_parser;
	room_daemon_code_pointer = room_101_daemon;

	anim_himem_preload(kernel_name('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
