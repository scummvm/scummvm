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
#include "mads/nebular/rooms/section7.h"
#include "mads/nebular/sound/mac_sound.h"
#include "mads/nebular/rooms/dialog.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _bottleHotspotId;
	int16 _boatCurrentFrame;
	int16 _animationMode;
	int16 _boatDirection;
	bool _takeBottleFl;
	Dialog _dialog1;
};

static Scratch local;


static void handleBottleInterface() {
	switch (global[kBottleStatus]) {
	case 0:
		local._dialog1.write(0x311, true);
		local._dialog1.write(0x312, true);
		local._dialog1.write(0x313, true);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	case 1:
		local._dialog1.write(0x311, false);
		local._dialog1.write(0x312, true);
		local._dialog1.write(0x313, true);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	case 2:
		local._dialog1.write(0x311, false);
		local._dialog1.write(0x312, false);
		local._dialog1.write(0x313, true);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	case 3:
		local._dialog1.write(0x311, false);
		local._dialog1.write(0x312, false);
		local._dialog1.write(0x313, false);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	default:
		break;
	}
}

static void setBottleSequence() {
	kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
	player.commands_allowed = false;
	if (local._boatDirection == 2)
		local._animationMode = 6;
	else
		local._animationMode = 7;
}

static void handleFillBottle(int quote) {
	switch (quote) {
	case 0x311:
		global[kBottleStatus] = 1;
		setBottleSequence();
		break;

	case 0x312:
		global[kBottleStatus] = 2;
		setBottleSequence();
		break;

	case 0x313:
		global[kBottleStatus] = 3;
		setBottleSequence();
		break;

	case 0x314:
		global[kBottleStatus] = 4;
		setBottleSequence();
		break;

	case 0x315:
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		break;

	default:
		break;
	}
}

static void room_704_init() {
	if (object[OBJ_BOTTLE].location == room_id) {
		g_sprite_ids[1] = kernel_load_series(kernel_name('b', 0), 0);
		g_sequence_ids[1] = kernel_seq_pingpong(g_sprite_ids[1], false, 6, 0, 0, 0);
		kernel_seq_depth(g_sequence_ids[1], 1);
		if (previous_room == 705) {
			kernel_seq_loc(g_sequence_ids[1], 123, 125);
			kernel_seq_depth(g_sequence_ids[1], 1);
		} else {
			kernel_seq_loc(g_sequence_ids[1], 190, 122);
			kernel_seq_depth(g_sequence_ids[1], 2);
		}
		int idx = kernel_add_dynamic(words_bottle, words_look_at, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -2, 0, FACING_NONE);
		local._bottleHotspotId = idx;
	}

	player.walker_visible = false;
	local._takeBottleFl = false;
	local._boatCurrentFrame = -1;

	if (previous_room == 705) {
		player.commands_allowed = false;
		local._animationMode = 2;
		local._boatDirection = 2;
		kernel_run_animation(kernel_name('A', -1), 0);
		kernel_reset_animation(0, 36);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.commands_allowed = false;
		local._boatDirection = 1;
		kernel_run_animation(kernel_name('A', -1), 0);
	} else if (local._boatDirection == 1) {
		kernel_run_animation(kernel_name('A', -1), 0);
		kernel_reset_animation(0, 8);
	} else if (local._boatDirection == 2) {
		if (object[OBJ_BOTTLE].location == room_id) {
			kernel_seq_loc(g_sequence_ids[1], 123, 125);
			kernel_seq_depth(g_sequence_ids[1], 1);
		}
		kernel_run_animation(kernel_name('A', -1), 0);
		kernel_reset_animation(0, 57);
	}

	if (kernel.teleported_in)
		global[kMonsterAlive] = false;

	kernel.quotes = quote_load(785, 786, 787, 788, 789, 0);
	local._dialog1.setup(0x98, 0x311, 0x312, 0x313, 0x314, 0x315, 0);

	section_7_music();
	if (!Sound::commandMacintoshSound(Sound::kMacSoundPlay, 7028,
			0, 0, 0, true))
		g_engine->_soundManager->command(28, 0);
}

static void room_704_daemon() {
	if (kernel_anim[0].anim != nullptr) {
		if (kernel_anim[0].frame != local._boatCurrentFrame) {
			local._boatCurrentFrame = kernel_anim[0].frame;
			int nextFrame = -1;

			switch (local._boatCurrentFrame) {
			case 10:
				switch (local._animationMode) {
				case 1:
					nextFrame = 10;
					break;
				case 5:
					nextFrame = 74;
					break;
				case 7:
					local._animationMode = 0;
					nextFrame = 92;
					break;
				default:
					if (!player.commands_allowed)
						player.commands_allowed = true;

					nextFrame = 8;
					break;
				}
				break;

			case 36:
				if (local._animationMode != 2)
					new_room = 705;
				break;

			case 59:
				switch (local._animationMode) {
				case 3:
					nextFrame = 59;
					break;

				case 4:
					nextFrame = 65;
					break;

				case 6:
					local._animationMode = 0;
					nextFrame = 83;
					break;

				default:
					if (!player.commands_allowed) {
						player.commands_allowed = true;
					}
					nextFrame = 57;
					break;
				}
				break;

			case 65:
				new_room = 703;
				break;

			case 74:
				nextFrame = 10;
				break;

			case 83:
				nextFrame = 59;
				break;

			case 90:
				if (local._takeBottleFl) {
					kernel_seq_delete(g_sequence_ids[1]);
					kernel_delete_dynamic(local._bottleHotspotId);
					inter_give_to_player(OBJ_BOTTLE);
					g_engine->_soundManager->command(15, 0);
					object_examine(OBJ_BOTTLE, 70415, 0);
				}
				break;

			case 92:
				nextFrame = 57;
				if (!player.commands_allowed && !local._takeBottleFl) {
					kernel_timing_trigger(30, 70);
					player.commands_allowed = true;
				}
				break;

			case 98:
				if (local._takeBottleFl) {
					kernel_seq_delete(g_sequence_ids[1]);
					kernel_delete_dynamic(local._bottleHotspotId);
					inter_give_to_player(OBJ_BOTTLE);
					g_engine->_soundManager->command(15, 0);
					object_examine(OBJ_BOTTLE, 70415, 0);
				}
				break;

			case 101:
				nextFrame = 8;
				if (!player.commands_allowed && !local._takeBottleFl) {
					kernel_timing_trigger(30, 70);
					player.commands_allowed = true;
				}
				break;

			default:
				break;
			}

			if ((nextFrame >= 0) && (nextFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, nextFrame);
				local._boatCurrentFrame = nextFrame;
			}
		}
	}

	if (kernel.trigger == 70) {
		switch (global[kBottleStatus]) {
		case 0:
			text_show(432);
			break;

		case 1:
			text_show(70324);
			break;

		case 2:
			text_show(70325);
			break;

		case 3:
			text_show(70326);
			break;

		case 4:
			text_show(70327);
			break;

		default:
			break;
		}
	}
}

static void room_704_parser() {
	if (inter_input_mode == INTER_CONVERSATION)
		handleFillBottle(player2.words[0]);
	else if (player_said_2(steer_towards, open_water_to_south)) {
		player.commands_allowed = false;
		if (local._boatDirection == 1)
			local._animationMode = 5;
		else
			local._animationMode = 3;
	} else if (player_said_2(steer_towards, building_to_north)) {
		player.commands_allowed = false;
		if (local._boatDirection == 2)
			local._animationMode = 4;
		else
			local._animationMode = 1;
	} else if (player_said_2(take, bottle)) {
		if (!player_has(OBJ_BOTTLE)) {
			player.commands_allowed = false;
			local._takeBottleFl = true;
			if (local._boatDirection == 2) {
				local._animationMode = 6;
			} else {
				local._animationMode = 7;
			}
		}
	} else if (player_said_3(put, bottle, water) || player_said_3(fill, bottle, water)) {
		if (player_has(OBJ_BOTTLE)) {
			if (global[kBottleStatus] != 4) {
				local._takeBottleFl = false;
				handleBottleInterface();
				local._dialog1.start();
			} else
				text_show(70323);
		}
	} else if (player.look_around || player_said_2(look, water))
		text_show(70410);
	else if (player_said_2(look, building_to_north)) {
		if (player_has_been_in_room(710))
			text_show(70411);
		else
			text_show(70412);
	} else if (player_said_2(look, volcano_rim))
		text_show(70413);
	else if (player_said_2(look, bottle) && (player.main_object_source == STROKE_INTERFACE))
		text_show(70414);
	else if (player_said_2(look, open_water_to_south))
		text_show(70416);
	else if (player_said_2(look, sky))
		text_show(70417);
	else
		return;

	player.command_ready = false;
}

void room_704_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._bottleHotspotId);
	s.syncAsSint16LE(local._boatCurrentFrame);
	s.syncAsSint16LE(local._animationMode);
	s.syncAsSint16LE(local._boatDirection);

	s.syncAsByte(local._takeBottleFl);
}

void room_704_preload() {
	room_init_code_pointer = room_704_init;
	room_daemon_code_pointer = room_704_daemon;
	room_parser_code_pointer = room_704_parser;

	*player.series_name = '\0';
	section_7_interface();
	vocab_make_active(words_bottle);
	vocab_make_active(words_look_at);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
