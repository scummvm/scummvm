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
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section6.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _monsterActive;
	bool _animationActiveFl;
	int16 _timebombHotspotId;
	int16 _bombMode;
	int16 _monsterFrame;
	int32 _monsterTimer;
};

static Scratch local;


static void room_604_init() {
	g_sprite_ids[2] = kernel_load_series(kernel_name('c', 0), 0);
	g_sprite_ids[4] = kernel_load_series("*RXCD_9", 0);
	g_sprite_ids[6] = kernel_load_series(kernel_full_name(620, 'b', 0, "", KERNEL_SS), 0);
	g_sprite_ids[5] = kernel_load_series("*RXMRC_9", 0);

	if (global[kTimebombStatus] == 1) {
		g_sequence_ids[6] = kernel_seq_stamp(g_sprite_ids[6], false, -1);
		local._timebombHotspotId = kernel_add_dynamic(words_timebomb, words_walkto, 0, g_sequence_ids[6], 0, 0, 0, 0);
		kernel_dynamic_walk(local._timebombHotspotId, 166, 118, FACING_NORTHEAST);
	}

	if (kernel.teleported_in)
		inter_give_to_player(OBJ_TIMEBOMB);

	pal_change_color(252, 63, 37, 26);
	pal_change_color(253, 45, 24, 17);
	local._animationActiveFl = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 72;
		player.y = 149;
		player.facing = FACING_NORTHEAST;
		player.walker_visible = false;
		player.commands_allowed = false;
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
		kernel_seq_depth(g_sequence_ids[2], 1);
		kernel_run_animation(kernel_name('R', 1), 70);
		local._animationActiveFl = true;
	} else {
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
		kernel_seq_depth(g_sequence_ids[2], 1);
	}

	local._monsterTimer = kernel.clock;
	local._monsterActive = false;

	section_6_music();
	kernel.quotes = quote_load(743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 0);
}

static void room_604_daemon() {
	switch (kernel.trigger) {
	case 70:
		player.walker_visible = true;
		player.clock = kernel_anim[0].next_clock - player.frame_delay;
		kernel_timing_trigger(30, 71);
		break;

	case 71:
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 6, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[2], 1);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
		kernel_seq_depth(g_sequence_ids[2], 1);
		player.commands_allowed = true;
		local._animationActiveFl = false;
		break;

	default:
		break;
	}

	if (local._monsterActive && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._monsterFrame) {
			local._monsterFrame = kernel_anim[0].frame;
			int nextMonsterFrame = -1;

			switch (local._monsterFrame) {
			case 50:
			case 137:
			case 174:
			{
				int randVal = g_engine->getRandomNumber(1, 1000);
				if ((randVal <= 450) && (player.special_code)) {
					if (player.special_code == 1)
						nextMonsterFrame = 50;
					else if (player.special_code == 2)
						nextMonsterFrame = 84;
					else
						nextMonsterFrame = 137;
				} else if (randVal <= 150)
					nextMonsterFrame = 50;
				else if (randVal <= 300)
					nextMonsterFrame = 84;
				else if (randVal <= 450)
					nextMonsterFrame = 137;
				else if (randVal < 750)
					nextMonsterFrame = 13;
				else
					nextMonsterFrame = 114;

			}
			break;

			case 84:
				nextMonsterFrame = 14;
				break;

			default:
				break;
			}

			if ((nextMonsterFrame >= 0) && (nextMonsterFrame != local._monsterFrame)) {
				kernel_reset_animation(0, nextMonsterFrame);
				local._monsterFrame = nextMonsterFrame;
			}
		}
	}

	if ((!local._monsterActive && !local._animationActiveFl) && (kernel.clock > (local._monsterTimer + 4))) {
		local._monsterTimer = kernel.clock;
		if ((g_engine->getRandomNumber(1, 1000) < 25) || !player.been_here_before) {
			local._monsterActive = true;
			kernel_abort_animation(0);
			kernel_run_animation(kernel_name('m', -1), 0);
		}
	}
}

static void handleBombActions() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[5] = kernel_seq_pingpong(g_sprite_ids[5], false, 9, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[5], 1, 3);
		kernel_seq_player(g_sequence_ids[5], false);
		if (local._bombMode == 1)
			kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_SPRITE, 3, 1);
		else
			kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_SPRITE, 3, 2);

		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 3);
		break;

	case 1:
		g_sequence_ids[6] = kernel_seq_stamp(g_sprite_ids[6], false, -1);
		local._timebombHotspotId = kernel_add_dynamic(words_timebomb, words_walkto, 0, g_sequence_ids[6], 0, 0, 0, 0);
		kernel_dynamic_walk(local._timebombHotspotId, 166, 118, FACING_NORTHEAST);
		inter_move_object(OBJ_TIMEBOMB, room_id);
		break;

	case 2:
		kernel_seq_delete(g_sequence_ids[6]);
		kernel_delete_dynamic(local._timebombHotspotId);
		inter_give_to_player(OBJ_TIMEBOMB);
		break;

	case 3:
		kernel_seq_timeout(g_sequence_ids[5], -1);
		player.walker_visible = true;
		player.commands_allowed = true;
		if (local._bombMode == 1) {
			text_show(60421);
			global[kTimebombStatus] = TIMEBOMB_ACTIVATED;
			global[kTimebombTimer] = 0;
		} else {
			text_show(60423);
			global[kTimebombStatus] = TIMEBOMB_DEACTIVATED;
			global[kTimebombTimer] = 0;
		}
		break;

	default:
		break;
	}
}

static void room_604_parser() {
	if (player_said_2(get_inside, car)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[2];
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_timeout(syncIdx, g_sequence_ids[2]);
			kernel_timing_trigger(6, 2);
		}
		break;

		case 2:
			player.walker_visible = false;
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 10, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = g_sequence_ids[4];
			g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, -2);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_timeout(syncIdx, g_sequence_ids[4]);
			new_room = 504;
		}
		break;

		default:
			break;
		}
	} else if ((player_said_2(put, ledge) || player_said_2(put, viewport) || player_said_2(throw, viewport))
		&& (player_said_1(bomb) || player_said_1(bombs)))
		text_show(60420);
	else if (player_said_3(put, timebomb, ledge) || player_said_3(put, timebomb, viewport)) {
		local._bombMode = 1;
		if ((game.difficulty == DIFFICULTY_HARD) || global[kWarnedFloodCity])
			handleBombActions();
		else if (
			(player_has(OBJ_POLYCEMENT) && (player_has(OBJ_CHICKEN) || player_has(OBJ_CHICKEN_BOMB)))
			&& (global[kLineStatus] == LINE_TIED || (game.difficulty == DIFFICULTY_EASY && !global[kBoatRaised]))
			)
			// The original can get in an impossible state at this point, if the player has
			// combined the chicken with the bomb before placing the timer bomb on the ledge.
			// Therefore, we also allow the player to place the bomb if the chicken bomb is
			// in the inventory.
			handleBombActions();
		else if (game.difficulty == DIFFICULTY_EASY)
			text_show(60424);
		else {
			text_show(60425);
			global[kWarnedFloodCity] = true;
		}
	} else if (player_said_2(take, timebomb)) {
		if (kernel.trigger || !player_has(OBJ_TIMEBOMB)) {
			local._bombMode = 2;
			handleBombActions();
		}
	} else if (player.look_around)
		text_show(60411);
	else if (player_said_2(look, viewport)) {
		if (local._monsterActive) {
			text_show(60413);
		} else {
			text_show(60412);
		}
	} else if (player_said_2(look, wall))
		text_show(60414);
	else if (player_said_2(look, vent))
		text_show(60415);
	else if (player_said_2(look, indicator))
		text_show(60416);
	else if (player_said_2(look, sculpture))
		text_show(60417);
	else if (player_said_2(look, car))
		text_show(60418);
	else if (player_said_2(look, fountain))
		text_show(60419);
	else
		return;

	player.command_ready = false;
}

void room_604_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._timebombHotspotId);
	s.syncAsSint16LE(local._bombMode);
	s.syncAsSint16LE(local._monsterFrame);
	s.syncAsUint32LE(local._monsterTimer);
	s.syncAsByte(local._monsterActive);
	s.syncAsByte(local._animationActiveFl);
}


void room_604_preload() {
	room_init_code_pointer = room_604_init;
	room_daemon_code_pointer = room_604_daemon;
	room_parser_code_pointer = room_604_parser;

	section_6_walker();
	section_6_interface();
	vocab_make_active(words_sea_monster);
	vocab_make_active(words_walkto);
	vocab_make_active(words_timebomb);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
