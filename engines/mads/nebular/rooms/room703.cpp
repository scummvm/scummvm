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
#include "mads/nebular/rooms/section7.h"
#include "mads/nebular/sound/mac_sound.h"
#include "mads/nebular/rooms/dialog.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _monsterMode;
	int16 _boatFrame;
	int16 _curSequence;
	int16 _boatDir;

	bool _useBomb;
	bool _startMonsterTimer;
	bool _rexDeathFl;
	bool _restartTrigger70Fl;

	int32 _lastFrameTime;
	int32 _monsterTime;

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
	if (local._boatDir == 2)
		local._curSequence = 6;
	else
		local._curSequence = 7;
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

static void room_703_init() {
	player.walker_visible = false;

	if (!player.been_here_before) {
		if (previous_room == 704)
			global[kMonsterAlive] = false;
		else
			global[kMonsterAlive] = true;
	}

	local._startMonsterTimer = true;
	local._rexDeathFl = true;
	local._monsterTime = 0;
	local._restartTrigger70Fl = true;
	local._useBomb = false;
	local._boatFrame = -1;

	if (!global[kMonsterAlive])
		kernel_flip_hotspot(words_sea_monster, false);

	if (previous_room == 704) {
		player.commands_allowed = false;
		local._curSequence = 2;
		local._boatDir = 2;
		local._monsterMode = 0;
		kernel_run_animation(kernel_name('A', -1), 0);
		kernel_reset_animation(0, 34);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.commands_allowed = false;
		local._boatDir = 1;
		if (global[kMonsterAlive]) {
			local._monsterMode = 1;
			local._curSequence = 0;
			kernel_run_animation(kernel_name('B', -1), 0);
		} else {
			local._curSequence = 0;
			local._monsterMode = 0;
			kernel_run_animation(kernel_name('A', -1), 0);
		}
	} else if (global[kMonsterAlive]) {
		local._curSequence = 0;
		local._boatDir = 1;
		local._monsterMode = 1;
		kernel_run_animation(kernel_name('B', -1), 0);
		kernel_reset_animation(0, 39);
	} else if (local._boatDir == 1) {
		local._curSequence = 0;
		local._monsterMode = 0;
		kernel_run_animation(kernel_name('A', -1), 0);
		kernel_reset_animation(0, 9);
	} else if (local._boatDir == 2) {
		local._curSequence = 0;
		local._monsterMode = 0;
		kernel_run_animation(kernel_name('A', -1), 0);
		kernel_reset_animation(0, 56);
	}

	if (kernel.teleported_in) {
		inter_give_to_player(OBJ_TWINKIFRUIT);
		inter_give_to_player(OBJ_BOMB);
		inter_give_to_player(OBJ_CHICKEN);
		inter_give_to_player(OBJ_BONES);
	}

	kernel.quotes = quote_load(785, 786, 787, 788, 789, 0);
	local._dialog1.setup(0x98, 0x311, 0x312, 0x313, 0x314, 0x315, 0);
	section_7_music();
	if (!Sound::commandMacintoshSound(Sound::kMacSoundPlay, 7028,
			0, 0, 0, true))
		g_engine->_soundManager->command(28, 0);
}

static void room_703_daemon() {
	if (local._startMonsterTimer) {
		long diff = kernel.clock - local._lastFrameTime;
		if ((diff >= 0) && (diff <= 12))
			local._monsterTime += diff;
		else
			local._monsterTime++;

		local._lastFrameTime = kernel.clock;
	}

	if ((local._monsterTime >= 2400) && !local._rexDeathFl && !local._useBomb) {
		local._startMonsterTimer = false;
		local._rexDeathFl = true;
		player.commands_allowed = false;
		kernel_abort_animation(0);
		local._monsterMode = 3;
		kernel_run_animation(kernel_name('D', -1), 0);
		local._rexDeathFl = false;
		local._monsterTime = 0;
	}


	if (kernel.trigger == 70)
		kernel.force_restart = true;

	if ((local._monsterMode == 3) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._boatFrame) {
			local._boatFrame = kernel_anim[0].frame;
			int nextBoatFrame = -1;

			if (local._boatFrame == 62) {
				nextBoatFrame = 61;
				if (local._restartTrigger70Fl) {
					local._restartTrigger70Fl = false;
					kernel_timing_trigger(15, 70);
				}
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, nextBoatFrame);
				local._boatFrame = nextBoatFrame;
			}
		}
	}

	if (kernel.trigger == 70)
		kernel.force_restart = true;

	if ((local._monsterMode == 0) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._boatFrame) {
			local._boatFrame = kernel_anim[0].frame;
			int nextBoatFrame = -1;

			switch (local._boatFrame) {
			case 11:
				if (local._curSequence == 7) {
					local._curSequence = 0;
					nextBoatFrame = 100;
				} else if (local._curSequence == 5)
					nextBoatFrame = 82;
				else if (local._curSequence == 1)
					nextBoatFrame = 11;
				else {
					nextBoatFrame = 9;
					if (!player.commands_allowed)
						player.commands_allowed = true;
				}
				break;

			case 34:
				if (local._curSequence != 2)
					new_room = 704;
				break;

			case 57:
				if (local._curSequence == 6) {
					local._curSequence = 0;
					nextBoatFrame = 91;
				} else if (local._curSequence == 4)
					nextBoatFrame = 73;
				else if (local._curSequence == 3)
					nextBoatFrame = 57;
				else {
					nextBoatFrame = 56;
					if (!player.commands_allowed)
						player.commands_allowed = true;
				}
				break;

			case 73:
				new_room = 701;
				break;

			case 82:
				nextBoatFrame = 11;
				break;

			case 91:
				nextBoatFrame = 57;
				break;

			case 100:
				nextBoatFrame = 56;
				if (!player.commands_allowed) {
					kernel_timing_trigger(30, 80);
					player.commands_allowed = true;
				}
				break;

			case 110:
				nextBoatFrame = 9;
				if (!player.commands_allowed) {
					kernel_timing_trigger(30, 80);
					player.commands_allowed = true;
				}
				break;

			default:
				break;
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, nextBoatFrame);
				local._boatFrame = nextBoatFrame;
			}
		}
	}

	if (kernel.trigger == 80) {
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


	if ((local._monsterMode == 1) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._boatFrame) {
			local._boatFrame = kernel_anim[0].frame;
			int nextBoatFrame = -1;

			switch (local._boatFrame) {
			case 39:
				player.commands_allowed = true;
				local._startMonsterTimer = true;
				local._rexDeathFl = false;
				break;

			case 40:
			case 49:
			case 54:
			case 67:
			case 78:
			case 87:
			case 96:
			case 105:
			case 114:
			case 123:
				if (local._curSequence == 8)
					nextBoatFrame = 129;

				break;

			case 129:
				nextBoatFrame = 39;
				break;

			case 151:
				new_room = 701;
				break;

			default:
				break;
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, nextBoatFrame);
				local._boatFrame = nextBoatFrame;
			}
		}
	}

	if ((local._monsterMode == 2) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._boatFrame) {
			local._boatFrame = kernel_anim[0].frame;
			int nextBoatFrame = -1;

			switch (local._boatFrame) {
			case 14:
				if (!local._useBomb) {
					if (game.difficulty == DIFFICULTY_HARD)
						inter_move_object(OBJ_CHICKEN, 1);
					else
						text_show(70319);
				}
				nextBoatFrame = 80;
				break;

			case 33:
				if (player_has(OBJ_BONES)) {
					inter_move_object(OBJ_BONES, 1);
					inter_give_to_player(OBJ_BONE);
				} else
					inter_move_object(OBJ_BONE, 1);

				nextBoatFrame = 80;
				break;

			case 53:
				inter_move_object(OBJ_TWINKIFRUIT, 1);
				nextBoatFrame = 80;
				local._curSequence = 9;
				break;

			case 80:
				if (game.difficulty == DIFFICULTY_HARD) {
					inter_move_object(OBJ_BOMB, 1);
					text_show(70318);
				} else
					text_show(70317);

				kernel_abort_animation(0);
				local._monsterMode = 1;
				kernel_run_animation(kernel_name('B', -1), 0);
				kernel_reset_animation(0, 39);
				player.commands_allowed = true;
				break;

			case 91:
				if (!local._useBomb) {
					kernel_abort_animation(0);
					local._monsterMode = 1;
					kernel_run_animation(kernel_name('B', -1), 0);
					kernel_reset_animation(0, 39);
					player.commands_allowed = true;
				} else
					inter_move_object(OBJ_CHICKEN_BOMB, 1);

				break;

			case 126:
				kernel_flip_hotspot(words_sea_monster, false);
				global[kMonsterAlive] = false;
				kernel_abort_animation(0);
				local._monsterMode = 0;
				kernel_run_animation(kernel_name('A', -1), 0);
				kernel_reset_animation(0, 9);
				player.commands_allowed = true;
				if (config_file.naughtiness == NAUGHTY)
					text_show(70321);
				else
					text_show(70322);

				break;

			default:
				break;
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, nextBoatFrame);
				local._boatFrame = nextBoatFrame;
			}
		}
	}
}

static void room_703_parser() {
	if (inter_input_mode == INTER_CONVERSATION)
		handleFillBottle(player2.words[0]);
	else if (player_said_2(steer_towards, dock_to_south)) {
		player.commands_allowed = false;
		if (global[kMonsterAlive])
			local._curSequence = 8;
		else if (local._boatDir == 1)
			local._curSequence = 5;
		else
			local._curSequence = 3;
	} else if (player_said_2(steer_towards, building_to_north)) {
		player.commands_allowed = false;
		if (global[kMonsterAlive]) {
			local._startMonsterTimer = false;
			local._rexDeathFl = true;
			local._monsterTime = 0;
			kernel_abort_animation(0);
			local._monsterMode = 3;
			kernel_run_animation(kernel_name('D', -1), 0);
		} else if (local._boatDir == 2)
			local._curSequence = 4;
		else
			local._curSequence = 1;
	} else if (player_said_3(throw, bone, sea_monster) || player_said_3(throw, bones, sea_monster)) {
		player.commands_allowed = false;
		kernel_abort_animation(0);
		local._monsterMode = 2;
		kernel_run_animation(kernel_name('C', -1), 0);
		kernel_reset_animation(0, 19);
	} else if (player_said_3(throw, chicken, sea_monster)) {
		player.commands_allowed = false;
		kernel_abort_animation(0);
		local._monsterMode = 2;
		kernel_run_animation(kernel_name('C', -1), 0);
	} else if (player_said_3(throw, twinkifruit, sea_monster)) {
		player.commands_allowed = false;
		kernel_abort_animation(0);
		local._monsterMode = 2;
		kernel_run_animation(kernel_name('C', -1), 0);
		kernel_reset_animation(0, 39);
	} else if (player_said_3(throw, bomb, sea_monster)) {
		player.commands_allowed = false;
		kernel_abort_animation(0);
		local._monsterMode = 2;
		kernel_run_animation(kernel_name('C', -1), 0);
		kernel_reset_animation(0, 59);
	} else if (player_said_3(throw, chicken_bomb, sea_monster)) {
		local._useBomb = true;
		player.commands_allowed = false;
		kernel_abort_animation(0);
		local._monsterMode = 2;
		kernel_run_animation(kernel_name('C', -1), 0);
	} else if (player_said_3(put, bottle, water) || player_said_3(fill, bottle, water)) {
		if (global[kBottleStatus] != 4) {
			handleBottleInterface();
			local._dialog1.start();
		} else
			text_show(70323);
	} else if (player.look_around || player_said_2(look, sea_monster)) {
		if (global[kMonsterAlive])
			text_show(70310);
	} else if (player_said_2(look, water)) {
		if (!global[kMonsterAlive])
			text_show(70311);
		else
			text_show(70312);
	} else if (player_said_2(look, building_to_north)) {
		if (global[kMonsterAlive])
			text_show(70313);
		else if (player_has_been_in_room(710))
			text_show(70314);
		else
			text_show(70315);
	} else if (player_said_2(look, volcano_rim))
		text_show(70316);
	else
		return;

	player.command_ready = false;
}

void room_703_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._monsterMode);
	s.syncAsSint16LE(local._boatFrame);
	s.syncAsSint16LE(local._curSequence);
	s.syncAsSint16LE(local._boatDir);

	s.syncAsByte(local._useBomb);
	s.syncAsByte(local._startMonsterTimer);
	s.syncAsByte(local._rexDeathFl);
	s.syncAsByte(local._restartTrigger70Fl);

	s.syncAsUint32LE(local._lastFrameTime);
	s.syncAsUint32LE(local._monsterTime);
}

void room_703_preload() {
	room_init_code_pointer = room_703_init;
	room_daemon_code_pointer = room_703_daemon;
	room_parser_code_pointer = room_703_parser;

	*player.series_name = '\0';
	section_7_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
