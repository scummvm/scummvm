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

#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section6.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _handsetHotspotId;
	int16 _checkVal;
	bool _cellCharging;
	int32 _cellChargingTimer;
	int32 _lastFrameTimer;
};

static Scratch local;


static void room_610_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('p', -1), 0);
	g_sprite_ids[2] = kernel_load_series("*RXMRC_9", 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('x', 1), 0);

	g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 60, 0, 0, 0);
	kernel_seq_depth(g_sequence_ids[4], 13);
	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 30, 0, 0, 0);
	kernel_seq_depth(g_sequence_ids[3], 9);

	if (!player.been_here_before)
		local._cellCharging = false;

	if (object[OBJ_PHONE_HANDSET].location == room_id) {
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 9, 0, 0, 0);
		local._handsetHotspotId = kernel_add_dynamic(words_phone_handset, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(local._handsetHotspotId, 132, 121, FACING_NORTHWEST);
		if ((global[kHandsetCellStatus] == 2) && (game.difficulty == DIFFICULTY_HARD) && !global[kDurafailRecharged])
			global[kHandsetCellStatus] = 1;
	}

	if (kernel.teleported_in && game.difficulty != DIFFICULTY_EASY)
		inter_give_to_player(OBJ_PENLIGHT);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 175;
		player.y = 152;
		player.facing = FACING_NORTHWEST;
	}

	section_6_music();
}

static void room_610_daemon() {
	if (local._cellCharging) {
		long diff = kernel.clock - local._lastFrameTimer;
		if ((diff >= 0) && (diff <= 60))
			local._cellChargingTimer += diff;
		else
			local._cellChargingTimer++;

		local._lastFrameTimer = kernel.clock;
	}

	// CHECKME: local._checkVal is always false, could be removed
	if ((local._cellChargingTimer >= 60) && !local._checkVal) {
		local._checkVal = true;
		global[kHandsetCellStatus] = 1;
		local._cellCharging = false;
		local._checkVal = false;
		local._cellChargingTimer = 0;
	}
}

static void room_610_parser() {
	if (player_said_2(exit_from, video_store))
		new_room = 609;
	else if (player_said_2(take, phone_handset)) {
		if (kernel.trigger || !player_has(OBJ_PHONE_HANDSET)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], true, 8, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[2], 1, 2);
				kernel_seq_player(g_sequence_ids[2], false);
				kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 2, 1);
				kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				g_engine->_soundManager->command(9, 0);
				kernel_seq_delete(g_sequence_ids[1]);
				kernel_delete_dynamic(local._handsetHotspotId);
				inter_give_to_player(OBJ_PHONE_HANDSET);
				object_examine(OBJ_PHONE_HANDSET, 61017, 0);
				break;

			case 2:
				kernel_seq_timeout(g_sequence_ids[2], -1);
				player.walker_visible = true;
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		}
	} else if (player_said_3(put, phone_handset, phone_cradle)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], true, 8, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[2], 1, 2);
			kernel_seq_player(g_sequence_ids[2], false);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 9, 0, 0, 0);
			local._handsetHotspotId = kernel_add_dynamic(words_phone_handset, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
			kernel_dynamic_walk(local._handsetHotspotId, 132, 121, FACING_NORTHWEST);
			inter_move_object(OBJ_PHONE_HANDSET, room_id);
			break;

		case 2:
			kernel_seq_timeout(g_sequence_ids[2], -1);
			player.walker_visible = true;
			player.commands_allowed = true;
			if ((global[kHandsetCellStatus] == 2) && (game.difficulty == DIFFICULTY_HARD) && !global[kDurafailRecharged])
				local._cellCharging = true;

			text_show(61032);
			break;

		default:
			break;
		}
	} else if (player_said_2(look, pippy_billboard))
		text_show(61010);
	else if (player_said_2(look, civilization_ad))
		text_show(61011);
	else if (player_said_2(look, marx_bros_poster))
		text_show(61012);
	else if (player_said_2(look, video_monitor))
		text_show(61013);
	else if (player_said_2(look, video_store))
		text_show(61014);
	else if (player.look_around)
		text_show(61015);
	else if (player_said_2(look, logo))
		text_show(61018);
	else if (player_said_2(look, cement)) {
		if (player_has_been_in_room(601))
			text_show(61020);
		else
			text_show(61019);
	} else if (player_said_2(look, counter))
		text_show(61021);
	else if (player_said_2(look, phone_antenna))
		text_show(61022);
	else if (player_said_2(look, smelly_sneaker))
		text_show(61023);
	else if (player_said_2(take, smelly_sneaker))
		text_show(61024);
	else if (player_said_2(look, spotlight))
		text_show(61025);
	else if (player_said_2(look, phone_handset) && (player.main_object_source == STROKE_INTERFACE))
		text_show(61026);
	else if (player_said_2(look, phone_cradle))
		text_show(61027);
	else if (player_said_2(look, return_slot))
		text_show(61028);
	else if (player_said_2(put, return_slot)
		&& player_has(object_named(player2.words[1])))
		text_show(61029);
	else if (player_said_1(classic_videos) || player_said_1(more_classic_videos) || player_said_1(drama_videos)
		|| player_said_1(new_release_videos) || player_said_1(porno_videos) || player_said_1(educational_videos)
		|| player_said_1(instructional_videos) || player_said_1(workout_videos) || player_said_1(foreign_videos)
		|| player_said_1(adventure_videos) || player_said_1(comedy_videos)) {
		if (player_said_1(look))
			text_show(61030);
		else if (player_said_1(take))
			text_show(61031);
		else
			return;
	} else
		return;

	player.command_ready = false;
}

void room_610_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._handsetHotspotId);
	s.syncAsSint16LE(local._checkVal);

	s.syncAsByte(local._cellCharging);

	s.syncAsSint32LE(local._cellChargingTimer);
	s.syncAsUint32LE(local._lastFrameTimer);
}

void room_610_preload() {
	room_init_code_pointer = room_610_init;
	room_daemon_code_pointer = room_610_daemon;
	room_parser_code_pointer = room_610_parser;

	section_6_walker();
	section_6_interface();
	vocab_make_active(words_phone_handset);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
