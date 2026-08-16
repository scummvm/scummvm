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
	int16 _carMode;
	int16 _carFrame;
	int16 _carMoveMode;
	int16 _dogDeathMode;
	int16 _carHotspotId;
	int16 _barkCount;
	int16 _polycementHotspotId;
	int16 _animationMode;
	int16 _nextTrigger;
	int16 _throwMode;

	bool _resetPositionsFl;
	bool _dogActiveFl;
	bool _dogBarkingFl;
	bool _dogFirstEncounter;
	bool _rexBeingEaten;
	bool _dogHitWindow;
	bool _checkFl;
	bool _dogSquashFl;
	bool _dogSafeFl;
	bool _buttonPressedonTimeFl;
	bool _dogUnderCar;
	bool _dogYelping;

	int32 _dogWindowTimer;
	int32 _dogRunTimer;
	int32 _dogTimer1;
	int32 _dogTimer2;
};

static Scratch local;


static void resetDogVariables() {
	g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 1);
	int idx = kernel_add_dynamic(words_obnoxious_dog, words_walkto, 0, g_sequence_ids[5], 0, 0, 0, 0);
	kernel_dynamic_walk(idx, 194, 142, FACING_EAST);
	kernel_seq_depth(g_sequence_ids[5], 4);
	local._dogBarkingFl = false;
	local._dogFirstEncounter = false;
}

static void restoreAnimations() {
	kernel_abort_animation(0);
	local._carMode = 0;
	player.commands_allowed = true;
	if (local._throwMode == 6)
		local._dogSquashFl = true;

	if (global[kCarStatus] == CAR_UP) {
		kernel_seq_delete(g_sequence_ids[6]);
		kernel_seq_delete(g_sequence_ids[7]);
		kernel_run_animation(kernel_name('A', -1), 0);
	} else {
		kernel_seq_delete(g_sequence_ids[8]);
		kernel_seq_delete(g_sequence_ids[6]);
		kernel_seq_delete(g_sequence_ids[7]);
		kernel_run_animation(kernel_name('A', -1), 0);
		kernel_reset_animation(0, 6);
	}
}

static void setCarAnimations() {
	kernel_abort_animation(0);
	if (global[kCarStatus] == CAR_UP) {
		g_sequence_ids[6] = kernel_seq_stamp(g_sprite_ids[6], false, 1);
		kernel_seq_loc(g_sequence_ids[6], 143, 98);
		kernel_seq_depth(g_sequence_ids[6], 6);
		g_sequence_ids[7] = kernel_seq_stamp(g_sprite_ids[7], false, 1);
		kernel_seq_loc(g_sequence_ids[7], 141, 67);
		kernel_seq_depth(g_sequence_ids[7], 15);
	} else {
		g_sequence_ids[6] = kernel_seq_stamp(g_sprite_ids[6], false, 1);
		kernel_seq_loc(g_sequence_ids[6], 143, 128);
		kernel_seq_depth(g_sequence_ids[6], 6);
		g_sequence_ids[7] = kernel_seq_stamp(g_sprite_ids[7], false, 1);
		kernel_seq_loc(g_sequence_ids[7], 141, 97);
		kernel_seq_depth(g_sequence_ids[7], 15);
		g_sequence_ids[8] = kernel_seq_stamp(g_sprite_ids[8], false, 1);
		kernel_seq_loc(g_sequence_ids[8], 144, 126);
		kernel_seq_depth(g_sequence_ids[8], 5);
	}
}

static void handleThrowingBone() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		setCarAnimations();
		kernel_seq_delete(g_sequence_ids[5]);
		local._animationMode = -1;
		player.walker_visible = false;
		local._carMode = local._throwMode;
		if (local._throwMode == 4)
			kernel_run_animation(kernel_name('X', 2), 1);
		else if (local._throwMode == 5)
			kernel_run_animation(kernel_name('X', 1), 1);
		else
			kernel_run_animation(kernel_name('X', 3), 1);
		break;

	case 1:
		local._nextTrigger = 1;
		kernel_timing_trigger(1, 2);
		break;

	case 2:
		if (local._nextTrigger != 2)
			kernel_timing_trigger(1, 2);
		else {
			if (player_has(OBJ_BONE))
				inter_move_object(OBJ_BONE, 1);
			else {
				inter_move_object(OBJ_BONES, 1);
				inter_give_to_player(OBJ_BONE);
			}
			kernel_timing_trigger(60, 3);
		}
		break;

	case 3:
		if (local._throwMode != 6) {
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x304), 0, 0, 0x1110, 120, 0, 34);
			kernel_timing_trigger(120, 4);
		} else
			restoreAnimations();
		break;

	case 4:
		restoreAnimations();
		break;

	default:
		break;
	}
}

static void room_608_init() {
	g_sprite_ids[2] = kernel_load_series("*RXMRD_7", 0);
	g_sprite_ids[3] = kernel_load_series("*RXMRC_9", 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('c', 2), 0);
	g_sprite_ids[7] = kernel_load_series(kernel_name('c', 0), 0);
	g_sprite_ids[8] = kernel_load_series(kernel_name('c', 1), 0);

	if (object_is_here(OBJ_POLYCEMENT)) {
		g_sprite_ids[1] = kernel_load_series(kernel_name('g', -1), 0);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
		int idx = kernel_add_dynamic(words_polycement, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 249, 129, FACING_NORTHEAST);
		local._polycementHotspotId = idx;
		kernel_seq_depth(g_sequence_ids[1], 6);
	}

	if (object_is_here(OBJ_REARVIEW_MIRROR)) {
		g_sprite_ids[12] = kernel_load_series(kernel_name('m', -1), 0);
		g_sequence_ids[12] = kernel_seq_stamp(g_sprite_ids[12], false, 1);
		int idx = kernel_add_dynamic(words_rearview_mirror, words_walkto, 0, g_sequence_ids[12], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 71, 113, FACING_NORTHEAST);
		kernel_seq_depth(g_sequence_ids[12], 15);
	}

	if (game.difficulty == DIFFICULTY_HARD) {
		g_sprite_ids[9] = kernel_load_series(kernel_name('g', 0), 0);
		g_sprite_ids[4] = kernel_load_series(kernel_name('g', 1), 0);
		g_sprite_ids[5] = kernel_load_series(kernel_name('g', 2), 0);
		g_sprite_ids[10] = kernel_load_series(kernel_name('h', 2), 0);
		g_sprite_ids[11] = kernel_load_series(kernel_name('h', 3), 0);
		local._rexBeingEaten = false;

		if (!player.been_here_before) {
			global[kDogStatus] = DOG_GONE;
			local._dogActiveFl = true;
		} else
			local._dogActiveFl = (global[kDogStatus] != DOG_DEAD);
	} else {
		global[kDogStatus] = DOG_DEAD;
		local._dogActiveFl = false;
	}

	local._dogSquashFl = false;
	local._buttonPressedonTimeFl = false;
	local._dogWindowTimer = 0;
	local._dogRunTimer = 0;
	local._dogHitWindow = false;
	local._checkFl = false;
	local._dogUnderCar = false;
	local._dogYelping = false;


	if (!player.been_here_before)
		global[kCarStatus] = CAR_UP;

	local._animationMode = 0;
	local._carMoveMode = 0;
	local._carFrame = -1;

	if (global[kCarStatus] == CAR_UP) {
		local._carMode = 0;
		local._dogDeathMode = 0;
		local._resetPositionsFl = false;
		int idx = kernel_add_dynamic(words_car, words_walkto, 0, -1, 99, 69, 82, 25);
		kernel_dynamic_walk(idx, 96, 132, FACING_NORTHEAST);
		local._carHotspotId = idx;
		kernel_run_animation(kernel_name('A', -1), 0);
	} else if (global[kCarStatus] == CAR_DOWN) {
		local._carMode = 0;
		local._dogDeathMode = 0;
		local._resetPositionsFl = false;
		int idx = kernel_add_dynamic(words_car, words_walkto, 0, -1, 100, 100, 82, 25);
		kernel_dynamic_walk(idx, 96, 132, FACING_NORTHEAST);
		local._carHotspotId = idx;
		kernel_run_animation(kernel_name('A', -1), 0);
		kernel_reset_animation(0, 6);
	} else if (global[kCarStatus] == CAR_SQUASHES_DOG) {
		local._carMode = 2;
		local._dogDeathMode = 0;
		local._resetPositionsFl = false;
		int idx = kernel_add_dynamic(words_car, words_walkto, 0, -1, 99, 69, 82, 25);
		kernel_dynamic_walk(idx, 96, 132, FACING_NORTHEAST);
		local._carHotspotId = idx;
		kernel_run_animation(kernel_name('C', -1), 0);
	} else if (global[kCarStatus] == CAR_SQUASHES_DOG_AGAIN) {
		local._carMode = 1;
		local._dogDeathMode = 2;
		local._resetPositionsFl = true;
		int idx = kernel_add_dynamic(words_car, words_walkto, 0, -1, 99, 69, 82, 25);
		kernel_dynamic_walk(idx, 96, 132, FACING_NORTHEAST);
		local._carHotspotId = idx;
		kernel_run_animation(kernel_name('B', -1), 0);
	} else {
		local._carMode = 3;
		local._dogDeathMode = 2;
		local._resetPositionsFl = true;
		int idx = kernel_add_dynamic(words_car, words_walkto, 0, -1, 100, 100, 82, 25);
		kernel_dynamic_walk(idx, 96, 132, FACING_NORTHEAST);
		local._carHotspotId = idx;
		kernel_run_animation(kernel_name('D', -1), 0);
	}

	pal_change_color(252, 63, 44, 30);
	pal_change_color(253, 63, 20, 22);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 46;
		player.y = 132;
		player.facing = FACING_EAST;
		if (game.difficulty == DIFFICULTY_HARD) {
			if (!player.been_here_before)
				local._dogFirstEncounter = true;
			else if (local._dogActiveFl)
				resetDogVariables();
		}
	} else if ((game.difficulty == DIFFICULTY_HARD) && !local._dogFirstEncounter && local._dogActiveFl) {
		if (!local._dogUnderCar)
			resetDogVariables();
		else {
			g_sequence_ids[10] = kernel_seq_pingpong(g_sprite_ids[10], false, 9, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[10], 10, 11);
			kernel_seq_depth(g_sequence_ids[10], 6);
		}
	}

	section_6_music();

	if (kernel.teleported_in)
		inter_give_to_player(OBJ_BONES);

	kernel.quotes = quote_load(763, 764, 766, 765, 767, 768, 769, 770, 771, 772, 0);
}

static void room_608_daemon() {
	if (local._dogFirstEncounter) {
		long diff = kernel.clock - local._dogTimer1;
		if ((diff >= 0) && (diff <= 1))
			local._dogWindowTimer += diff;
		else
			local._dogWindowTimer++;

		local._dogTimer1 = kernel.clock;
	}

	if (local._dogActiveFl && (local._dogWindowTimer >= 2) && !local._dogHitWindow) {
		local._dogHitWindow = true;
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 11, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[4], 1);
		g_engine->_soundManager->command(14, 0);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 70);
		local._dogWindowTimer = 0;
	}

	if (kernel.trigger == 70)
		resetDogVariables();

	if ((game.difficulty == DIFFICULTY_HARD) && !local._animationMode && local._dogActiveFl && !local._dogFirstEncounter && !local._dogUnderCar) {
		if (!local._dogBarkingFl) {
			if (g_engine->getRandomNumber(1, 50) == 10) {
				local._dogBarkingFl = true;
				kernel_seq_delete(g_sequence_ids[5]);
				g_sequence_ids[5] = kernel_seq_pingpong(g_sprite_ids[5], false, 5, 0, 0, 8);
				kernel_seq_depth(g_sequence_ids[5], 4);
				int idx = kernel_add_dynamic(words_obnoxious_dog, words_walkto, 0, g_sequence_ids[5], 0, 0, 0, 0);
				kernel_dynamic_walk(idx, 194, 142, FACING_EAST);
				local._barkCount = 0;
				kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_SPRITE, 2, 100);
				kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 60);
			}
		} else if (kernel.trigger == 60) {
			int syncIdx = g_sequence_ids[5];
			g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 1);
			kernel_seq_depth(g_sequence_ids[5], 6);
			kernel_seq_timeout(syncIdx, g_sequence_ids[5]);
			kernel_message_purge();
			local._dogBarkingFl = false;
		}
	}

	if ((kernel.trigger == 100) && local._dogBarkingFl) {
		g_engine->_soundManager->command(12, 0);
		local._barkCount++;

		if ((local._barkCount >= 1) && (local._barkCount <= 4)) {
			Common::Point _barkPos(0, 0);
			switch (local._barkCount) {
			case 1:
				_barkPos = Common::Point(197, 66);
				break;

			case 2:
				_barkPos = Common::Point(230, 76);
				break;

			case 3:
				_barkPos = Common::Point(197, 86);
				break;

			case 4:
				_barkPos = Common::Point(230, 97);
				break;

			default:
				break;
			}
			kernel_message_add(quote_string(kernel.quotes, 0x2FB), _barkPos.x, _barkPos.y, 0xFDFC, 120, 0, 0);
		}
	}

	if (local._dogSquashFl && !local._dogFirstEncounter && local._dogUnderCar && local._dogActiveFl) {
		long diff = kernel.clock - local._dogTimer2;
		if ((diff >= 0) && (diff <= 4))
			local._dogRunTimer += diff;
		else
			local._dogRunTimer++;

		local._dogTimer2 = kernel.clock;
	}

	if (local._dogRunTimer >= 480 && !local._checkFl && !local._buttonPressedonTimeFl && !local._dogFirstEncounter && local._dogUnderCar && local._dogActiveFl) {
		local._dogSquashFl = false;
		local._dogSafeFl = true;
		local._checkFl = false;
		local._dogRunTimer = 0;
	} else {
		local._dogSafeFl = false;
		if (player.walking && (game.difficulty == DIFFICULTY_HARD) && local._dogActiveFl && (player.next_special_code > 0) && local._dogUnderCar)
			local._dogSafeFl = true;
	}

	if (local._dogActiveFl && local._dogSafeFl && !local._buttonPressedonTimeFl) {
		local._dogDeathMode = 0;
		global[kCarStatus] = CAR_UP;
		local._carMode = 0;
		kernel_seq_delete(g_sequence_ids[10]);
		local._dogUnderCar = false;
		local._dogYelping = false;
		kernel_message_purge();
		g_sequence_ids[11] = kernel_seq_forward(g_sprite_ids[11], false, 6, 0, 0, 1);
		kernel_seq_trigger(g_sequence_ids[11], KERNEL_TRIGGER_EXPIRE, 0, 92);
		kernel_message_add(quote_string(kernel.quotes, 0x2FF), 0, 0, 0x1110, 120, 0, 34);
	}

	if (kernel.trigger == 92) {
		resetDogVariables();
		local._animationMode = 0;
	}

	if ((local._carMode == 4) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._carFrame) {
			local._carFrame = kernel_anim[0].frame;

			if (local._carFrame == 10) {
				player.walker_visible = true;
				player.clock = kernel_anim[0].next_clock - player.frame_delay;
			} else if (local._carFrame == 56) {
				resetDogVariables();
				local._animationMode = 0;
				local._nextTrigger = 2;
			}
		}
	}

	if ((local._carMode == 5) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._carFrame) {
			local._carFrame = kernel_anim[0].frame;
			if (local._carFrame == 10) {
				player.walker_visible = true;
				player.clock = kernel_anim[0].next_clock - player.frame_delay;
			} else if (local._carFrame == 52) {
				resetDogVariables();
				local._animationMode = 0;
				local._nextTrigger = 2;
			}
		}
	}

	if ((local._carMode == 6) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._carFrame) {
			local._carFrame = kernel_anim[0].frame;

			if (local._carFrame == 11) {
				player.walker_visible = true;
				player.clock = kernel_anim[0].next_clock - player.frame_delay;
			} else if (local._carFrame == 41) {
				g_sequence_ids[10] = kernel_seq_pingpong(g_sprite_ids[10], false, 9, 0, 0, 0);
				kernel_seq_range(g_sequence_ids[10], 10, 11);
				kernel_seq_depth(g_sequence_ids[10], 6);
				local._dogUnderCar = true;
				local._nextTrigger = 2;
			}
		}
	}

	if (local._dogUnderCar) {
		if (!local._dogYelping) {
			if (g_engine->getRandomNumber(1, 50) == 10) {
				local._dogYelping = true;
				local._barkCount = 0;
				kernel_timing_trigger(12, 110);
				kernel_timing_trigger(22, 111);
				kernel_timing_trigger(120, 112);
			}
			kernel_message_purge();
		}
	} else
		local._dogYelping = false;

	if (kernel.trigger == 110) {
		g_engine->_soundManager->command(12, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x303), 150, 97, 0xFDFC, 60, 0, 0);
	}

	if (kernel.trigger == 111) {
		g_engine->_soundManager->command(12, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x303), 183, 93, 0xFDFC, 60, 0, 0);
	}

	if (kernel.trigger == 112)
		local._dogYelping = false;

	if ((local._carMode == 0) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._carFrame) {
			local._carFrame = kernel_anim[0].frame;
			int nextFrame = -1;

			if ((global[kCarStatus] == CAR_UP) || (global[kCarStatus] == CAR_DOWN)) {
				switch (local._carMoveMode) {
				case 0:
					if (global[kCarStatus] == CAR_UP)
						nextFrame = 0;
					else
						nextFrame = 6;
					break;

				case 1:
					if (kernel_anim[0].frame >= 12) {
						nextFrame = 0;
						local._carMoveMode = 0;
						global[kCarStatus] = CAR_UP;
					}
					break;

				case 2:
					if (kernel_anim[0].frame >= 6) {
						nextFrame = 6;
						local._carMoveMode = 0;
						global[kCarStatus] = CAR_DOWN;
					}
					break;

				default:
					break;
				}
			}

			if ((nextFrame >= 0) && (nextFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, nextFrame);
				local._carFrame = nextFrame;
			}
		}
	}

	if ((local._carMode == 2) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._carFrame) {
			local._carFrame = kernel_anim[0].frame;
			int nextFrame = -1;

			if (local._carMoveMode == 0)
				nextFrame = 28;
			else if (kernel_anim[0].frame >= 28) {
				nextFrame = 28;
				local._carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, nextFrame);
				local._carFrame = nextFrame;
			}
		}
	}

	if ((local._carMode == 3) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._carFrame) {
			local._carFrame = kernel_anim[0].frame;
			int nextFrame = -1;

			if (local._resetPositionsFl) {
				nextFrame = 0;
				local._carMoveMode = 0;
			} else if (local._carMoveMode == 0)
				nextFrame = 6;
			else if (kernel_anim[0].frame >= 6) {
				nextFrame = 6;
				local._carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, nextFrame);
				local._carFrame = nextFrame;
			}
		}
	}


	if ((local._carMode == 1) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame != local._carFrame) {
			local._carFrame = kernel_anim[0].frame;
			int nextFrame = -1;

			if (local._resetPositionsFl) {
				nextFrame = 0;
				local._carMoveMode = 0;
			} else if (local._carMoveMode == 0)
				nextFrame = 6;
			else if (kernel_anim[0].frame >= 6) {
				nextFrame = 6;
				local._carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, nextFrame);
				local._carFrame = nextFrame;
			}
		}
	}

	if (player.walking && (game.difficulty == DIFFICULTY_HARD) && local._dogActiveFl && (player.next_special_code > 0)) {
		player_cancel_command();
		player_start_walking(194, 142, FACING_EAST);
		player.next_special_code = 0;
		if (local._dogUnderCar)
			local._dogSafeFl = true;
	}

	if (player.special_code > 0 && (game.difficulty == DIFFICULTY_HARD) && local._dogActiveFl && player.commands_allowed)
		player.commands_allowed = false;

	if ((game.difficulty == DIFFICULTY_HARD) && local._dogActiveFl && (Common::Point(player.x, player.y) == Common::Point(194, 142))
		&& (kernel.trigger || !local._rexBeingEaten)) {
		local._rexBeingEaten = true;
		switch (kernel.trigger) {
		case 0:
			kernel_seq_delete(g_sequence_ids[5]);
			local._animationMode = 1;
			g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 10, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[9], 1, 2);
			kernel_seq_depth(g_sequence_ids[9], 5);
			kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 80);
			kernel_timing_trigger(10, 85);
			break;

		case 80:
			player.walker_visible = false;
			g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 10, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[9], 3, 5);
			kernel_seq_depth(g_sequence_ids[9], 5);
			kernel_seq_timeout(-1, g_sequence_ids[9]);
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x2FC), 0, 0, 0x1110, 120, 0, 34);
			kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 81);
			break;

		case 81:
		{
			int syncIdx = g_sequence_ids[9];
			g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 5, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[9], 6, 38);
			kernel_seq_depth(g_sequence_ids[9], 5);
			kernel_seq_timeout(syncIdx, g_sequence_ids[9]);
			kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 82);
		}
		break;

		case 82:
		{
			int syncIdx = g_sequence_ids[9];
			g_sequence_ids[9] = kernel_seq_pingpong(g_sprite_ids[9], false, 15, 0, 0, 5);
			kernel_seq_range(g_sequence_ids[9], 39, 40);
			kernel_seq_depth(g_sequence_ids[9], 5);
			kernel_seq_timeout(syncIdx, g_sequence_ids[9]);
			kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 83);
		}
		break;

		case 83:
		{
			local._animationMode = 2;
			int syncIdx = g_sequence_ids[9];
			g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, -2);
			kernel_seq_depth(g_sequence_ids[9], 5);
			kernel_seq_timeout(syncIdx, g_sequence_ids[9]);
			kernel_timing_trigger(60, 84);
		}
		break;

		case 84:
			local._rexBeingEaten = false;
			local._animationMode = 0;
			kernel.force_restart = true;
			player.commands_allowed = true;
			break;

		case 85:
			if (local._rexBeingEaten && (local._animationMode == 1)) {
				g_engine->_soundManager->command(12, 0);
				kernel_timing_trigger(10, 85);
			}
			break;

		default:
			break;
		}
	}
}

static void room_608_pre_parser() {
	kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;

	if ((player_said_3(throw, bone, rear_of_garage) || player_said_3(throw, bones, rear_of_garage)
		|| player_said_3(throw, bone, front_of_garage) || player_said_3(throw, bones, front_of_garage)
		|| player_said_3(throw, bones, obnoxious_dog) || player_said_3(throw, bone, obnoxious_dog)) && local._dogActiveFl) {
		player.commands_allowed = false;
		player_walk(56, 146, FACING_EAST);
	}

	if ((player_said_3(throw, bones, area_behind_car) || player_said_3(throw, bone, area_behind_car)
		|| player_said_3(throw, bones, danger_zone) || player_said_3(throw, bone, danger_zone)) && local._dogActiveFl) {
		player.commands_allowed = false;
		player_walk(75, 136, FACING_EAST);
	}

	if (player_said_2(push, down_button) && local._dogUnderCar) {
		local._buttonPressedonTimeFl = true;
		local._dogDeathMode = 1;
	} else
		local._buttonPressedonTimeFl = false;
}

static void room_608_parser() {
	if (player_said_2(walk_through, doorway))
		new_room = 607;
	else if (player_said_2(push, down_button)) {
		player.commands_allowed = true;
		switch (kernel.trigger) {
		case 0:
			if ((global[kCarStatus] == CAR_UP) || (global[kCarStatus] == CAR_SQUASHES_DOG) || (global[kCarStatus] == CAR_SQUASHES_DOG_AGAIN)) {
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], true, 6, 0, 0, 2);
				kernel_seq_range(g_sequence_ids[3], 1, 2);
				kernel_seq_player(g_sequence_ids[3], false);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 1);
			} else
				text_show(60839);
			break;

		case 1:
			kernel_seq_timeout(g_sequence_ids[3], -1);
			player.walker_visible = true;
			if (local._dogDeathMode == 0)
				local._carMode = 0;
			else if (local._dogDeathMode == 1) {
				kernel_message_purge();
				kernel_message_add(quote_string(kernel.quotes, 0x300), 0, 0, 0x1110, 120, 0, 34);
				global[kCarStatus] = CAR_SQUASHES_DOG;
				local._carMode = 2;
				global[kDogStatus] = DOG_DEAD;
				local._dogActiveFl = false;
				local._dogUnderCar = false;
				kernel_seq_delete(g_sequence_ids[10]);
				kernel_abort_animation(0);
				kernel_run_animation(kernel_name('C', -1), 0);
			} else {
				local._resetPositionsFl = false;
				local._carMode = 1;
				kernel_abort_animation(0);
				kernel_run_animation(kernel_name('B', -1), 0);
			}

			local._carMoveMode = 2;
			kernel_timing_trigger(1, 2);
			break;

		case 2:
			if (local._carMoveMode != 0)
				kernel_timing_trigger(1, 2);
			else {
				kernel_delete_dynamic(local._carHotspotId);
				int idx = kernel_add_dynamic(words_car, words_walkto, 0, -1, 100, 100, 82, 25);
				kernel_dynamic_walk(idx, 96, 132, FACING_NORTHEAST);
				local._carHotspotId = idx;
				if (global[kCarStatus] == CAR_SQUASHES_DOG)
					kernel_timing_trigger(120, 3);
				else {
					if (local._dogDeathMode == 0)
						global[kCarStatus] = CAR_DOWN;
					else {
						global[kCarStatus] = CAR_DOWN_ON_SQUASHED_DOG;
						local._carMode = 3;
						local._dogDeathMode = 2;
					}
					player.commands_allowed = true;
				}
			}
			break;

		case 3:
			kernel_message_add(quote_string(kernel.quotes, 0x302), 0, 0, 0x1110, 120, 0, 34);
			global[kCarStatus] = CAR_DOWN_ON_SQUASHED_DOG;
			local._carMode = 3;
			local._dogDeathMode = 2;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(push, up_button)) {
		switch (kernel.trigger) {
		case 0:
			if ((global[kCarStatus] == CAR_DOWN) || (global[kCarStatus] == CAR_DOWN_ON_SQUASHED_DOG)) {
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], true, 6, 0, 0, 2);
				kernel_seq_range(g_sequence_ids[3], 1, 3);
				kernel_seq_player(g_sequence_ids[3], false);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 1);
			} else
				text_show(60840);
			break;

		case 1:
			kernel_seq_timeout(g_sequence_ids[3], -1);
			player.walker_visible = true;
			if (local._dogDeathMode == 0)
				local._carMode = 0;
			else {
				local._carMode = 3;
				local._resetPositionsFl = false;
				kernel_abort_animation(0);
				kernel_run_animation(kernel_name('D', -1), 0);
			}
			local._carMoveMode = 1;
			kernel_timing_trigger(1, 2);
			break;

		case 2:
		{
			if (local._carMoveMode != 0)
				kernel_timing_trigger(1, 2);
			else if (local._dogDeathMode == 0)
				global[kCarStatus] = CAR_UP;
			else if (local._dogDeathMode == 2) {
				global[kCarStatus] = CAR_SQUASHES_DOG_AGAIN;
				local._carMode = 3;
			}
			kernel_delete_dynamic(local._carHotspotId);
			int idx = kernel_add_dynamic(words_car, words_walkto, 0, -1, 99, 69, 82, 25);
			kernel_dynamic_walk(idx, 96, 132, FACING_NORTHEAST);
			local._carHotspotId = idx;
			player.commands_allowed = true;
		}
		break;

		default:
			break;
		}
	} else if (player_said_3(throw, bone, rear_of_garage) || player_said_3(throw, bones, rear_of_garage)
		|| player_said_3(throw, bones, obnoxious_dog) || player_said_3(throw, bone, obnoxious_dog)) {
		player.commands_allowed = true;
		if (local._dogActiveFl) {
			if (kernel.trigger == 0) {
				kernel_message_purge();
				kernel_message_add(quote_string(kernel.quotes, 0x2FE), 0, 0, 0x1110, 120, 0, 34);
			}
			local._throwMode = 4;
			handleThrowingBone();
		} else
			text_show(60841);
	} else if (player_said_3(throw, bone, front_of_garage) || player_said_3(throw, bones, front_of_garage)) {
		player.commands_allowed = true;
		if (local._dogActiveFl) {
			if (kernel.trigger == 0) {
				kernel_message_purge();
				kernel_message_add(quote_string(kernel.quotes, 0x2FD), 0, 0, 0x1110, 120, 0, 34);
			}
			local._throwMode = 5;
			handleThrowingBone();
		} else
			text_show(60841);
	} else if (player_said_3(throw, bones, area_behind_car) || player_said_3(throw, bone, area_behind_car)
		|| player_said_3(throw, bones, danger_zone) || player_said_3(throw, bone, danger_zone)) {
		player.commands_allowed = true;
		if ((global[kCarStatus] == CAR_UP) && local._dogActiveFl) {
			if (kernel.trigger == 0) {
				kernel_message_purge();
				kernel_message_add(quote_string(kernel.quotes, 0x301), 0, 0, 0x1110, 120, 0, 34);
			}
			local._throwMode = 6;
			handleThrowingBone();
		} else
			text_show(60842);
	} else if (player_said_2(take, polycement) && (kernel.trigger || !player_has(OBJ_POLYCEMENT))) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], true, 6, 0, 0, 2);
			kernel_seq_player(g_sequence_ids[2], false);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 4, 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			g_engine->_soundManager->command(9, 0);
			kernel_seq_delete(g_sequence_ids[1]);
			kernel_delete_dynamic(local._polycementHotspotId);
			break;

		case 2:
			inter_give_to_player(OBJ_POLYCEMENT);
			kernel_seq_timeout(g_sequence_ids[2], -1);
			player.walker_visible = true;
			player.commands_allowed = true;
			object_examine(OBJ_POLYCEMENT, 60833, 0);
			break;

		default:
			break;
		}
	} else if (player_said_2(take, rearview_mirror) && (kernel.trigger || !player_has(OBJ_REARVIEW_MIRROR))) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], true, 6, 0, 0, 2);
			kernel_seq_range(g_sequence_ids[2], 1, 2);
			kernel_seq_player(g_sequence_ids[2], false);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			g_engine->_soundManager->command(9, 0);
			kernel_seq_delete(g_sequence_ids[12]);
			inter_give_to_player(OBJ_REARVIEW_MIRROR);
			object_examine(OBJ_REARVIEW_MIRROR, 60827, 0);
			break;

		case 2:
			kernel_seq_timeout(g_sequence_ids[2], -1);
			player.walker_visible = true;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player.look_around) {
		if (game.difficulty != DIFFICULTY_HARD)
			text_show(60810);
		else if (global[kDogStatus] == DOG_DEAD)
			text_show(60812);
		else
			text_show(60811);
	} else if (player_said_1(look) && (player_said_1(muffler) || player_said_1(car_seat) || player_said_1(hubcap)
		|| player_said_1(coils) || player_said_1(quarter_panel)))
		text_show(60813);
	else if (player_said_1(take) && (player_said_1(muffler) || player_said_1(car_seat) || player_said_1(hubcap)
		|| player_said_1(coils) || player_said_1(quarter_panel)))
		text_show(60814);
	else if (player_said_2(look, garage_floor) || player_said_2(look, front_of_garage) || player_said_2(look, rear_of_garage)) {
		if (local._dogActiveFl)
			text_show(60815);
		else
			text_show(60816);
	} else if (player_said_2(look, spare_ribs))
		text_show(60817);
	else if (player_said_2(take, spare_ribs)) {
		if (game.difficulty == DIFFICULTY_HARD)
			text_show(60818);
		else
			text_show(60819);
	} else if (player_said_2(look, up_button))
		text_show(60820);
	else if (player_said_2(look, down_button))
		text_show(60821);
	else if (player_said_2(look, trash_can))
		text_show(60822);
	else if (player_said_2(look, calendar))
		text_show(60823);
	else if (player_said_2(look, storage_box)) {
		if (object[OBJ_REARVIEW_MIRROR].location == room_id)
			text_show(60825);
		else
			text_show(60824);
	} else if (player_said_2(open, storage_box))
		text_show(60826);
	else if (player_said_2(look, rearview_mirror) && (player.main_object_source == STROKE_INTERFACE))
		text_show(60828);
	else if (player_said_2(look, tool_box)) {
		if (object[OBJ_POLYCEMENT].location == room_id)
			text_show(60829);
		else
			text_show(60830);
	} else if (player_said_2(open, tool_box))
		text_show(60831);
	else if ((player_said_2(look, polycement)) && (object_is_here(OBJ_POLYCEMENT)))
		text_show(60832);
	else if (player_said_2(look, grease_can) || player_said_2(look, oil_can))
		text_show(60834);
	else if (player_said_2(look, car_lift))
		text_show(60835);
	else if (player_said_2(look, chair) || player_said_2(look, hat))
		text_show(60836);
	else if (player_said_2(look, danger_zone))
		text_show(60838);
	else
		return;

	player.command_ready = false;
}

void room_608_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._carMode);
	s.syncAsSint16LE(local._carFrame);
	s.syncAsSint16LE(local._carMoveMode);
	s.syncAsSint16LE(local._dogDeathMode);
	s.syncAsSint16LE(local._carHotspotId);
	s.syncAsSint16LE(local._barkCount);
	s.syncAsSint16LE(local._polycementHotspotId);
	s.syncAsSint16LE(local._animationMode);
	s.syncAsSint16LE(local._nextTrigger);
	s.syncAsSint16LE(local._throwMode);

	s.syncAsByte(local._resetPositionsFl);
	s.syncAsByte(local._dogActiveFl);
	s.syncAsByte(local._dogBarkingFl);
	s.syncAsByte(local._dogFirstEncounter);
	s.syncAsByte(local._rexBeingEaten);
	s.syncAsByte(local._dogHitWindow);
	s.syncAsByte(local._checkFl);
	s.syncAsByte(local._dogSquashFl);
	s.syncAsByte(local._dogSafeFl);
	s.syncAsByte(local._buttonPressedonTimeFl);
	s.syncAsByte(local._dogUnderCar);
	s.syncAsByte(local._dogYelping);

	s.syncAsSint32LE(local._dogWindowTimer);
	s.syncAsSint32LE(local._dogRunTimer);

	s.syncAsUint32LE(local._dogTimer1);
	s.syncAsUint32LE(local._dogTimer2);
}

void room_608_preload() {
	room_init_code_pointer = room_608_init;
	room_daemon_code_pointer = room_608_daemon;
	room_pre_parser_code_pointer = room_608_pre_parser;
	room_parser_code_pointer = room_608_parser;

	section_6_walker();
	section_6_interface();
	vocab_make_active(words_walkto);
	vocab_make_active(words_polycement);
	vocab_make_active(words_car);
	vocab_make_active(words_obnoxious_dog);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
