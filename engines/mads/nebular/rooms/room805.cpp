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

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_805_init() {
	player.walker_visible = false;
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	g_sprite_ids[1] = kernel_load_series(kernel_name('a', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('a', 2), 0);

	if (global[kShieldModInstalled]) {
		kernel_flip_hotspot(OBJ_SHIELD_MODULATOR, false);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 25);
		int idx = kernel_add_dynamic(words_shield_modulator, words_remove, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 0, 0, 0);
	}

	if (global[kTargetModInstalled]) {
		kernel_flip_hotspot(OBJ_TARGET_MODULE, false);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 12);
		int idx = kernel_add_dynamic(words_target_module, words_remove, 0, g_sequence_ids[2], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 0, 0, 0);
	}

	section_8_music();
}

static void room_805_daemon() {

	if (kernel.trigger == 70) {
		kernel_flip_hotspot(OBJ_SHIELD_MODULATOR, false);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 25);
		int idx = kernel_add_dynamic(words_shield_modulator, words_remove, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 0, 0, 0);
		global[kShieldModInstalled] = true;
		inter_move_object(OBJ_SHIELD_MODULATOR, NOWHERE);
		active_inven = -1;
		player.commands_allowed = true;
		g_engine->_soundManager->command(24, 0);
	}

	if (kernel.trigger == 80) {
		kernel_flip_hotspot(OBJ_TARGET_MODULE, false);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 12);
		int idx = kernel_add_dynamic(words_target_module, words_remove, 0, g_sequence_ids[2], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 0, 0, 0);
		global[kTargetModInstalled] = true;
		inter_move_object(OBJ_TARGET_MODULE, NOWHERE);
		active_inven = -1;
		player.commands_allowed = true;
		g_engine->_soundManager->command(24, 0);
	}

	if (kernel.trigger == 71) {
		kernel_flip_hotspot(OBJ_SHIELD_MODULATOR, true);
		global[kShieldModInstalled] = false;
		inter_give_to_player(OBJ_SHIELD_MODULATOR);
		player.commands_allowed = true;
	}

	if (kernel.trigger == 81) {
		kernel_flip_hotspot(OBJ_TARGET_MODULE, true);
		global[kTargetModInstalled] = false;
		inter_give_to_player(OBJ_TARGET_MODULE);
		player.commands_allowed = true;
	}
}

static void room_805_pre_parser() {
	player.need_to_walk = false;
}

static void room_805_parser() {
	if (player_said_2(exit, service_panel))
		new_room = 804;
	else if (player_said_2(install, shield_modulator) && player_has(OBJ_SHIELD_MODULATOR)) {
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 7, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[1], -1, -2);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 70);
		player.commands_allowed = false;
	} else if (player_said_2(install, target_module) && player_has(OBJ_TARGET_MODULE)) {
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 7, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[2], -1, -2);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 80);
		player.commands_allowed = false;
	} else if (player_said_2(remove, shield_modulator) && global[kShieldModInstalled]) {
		kernel_seq_delete(g_sequence_ids[1]);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 7, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[1], -1, -2);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 71);
		player.commands_allowed = false;
	} else if (player_said_2(remove, target_module) && global[kTargetModInstalled]) {
		kernel_seq_delete(g_sequence_ids[2]);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 7, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[2], -1, -2);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 81);
		player.commands_allowed = false;
	} else if (player_said_2(install, shield_modulator) && !player_has(OBJ_SHIELD_MODULATOR))
		text_show(80511);
	else if (player_said_2(install, target_module) && !player_has(OBJ_TARGET_MODULE))
		text_show(80510);
	else if (player_said_2(remove, life_support_module))
		text_show(80512);
	else
		return;

	player.command_ready = false;
}

void room_805_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_805_preload() {
	room_init_code_pointer = room_805_init;
	room_daemon_code_pointer = room_805_daemon;
	room_pre_parser_code_pointer = room_805_pre_parser;
	room_parser_code_pointer = room_805_parser;

	section_8_walker();
	section_8_interface();
	vocab_make_active(words_remove);
	vocab_make_active(words_target_module);
	vocab_make_active(words_shield_modulator);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
