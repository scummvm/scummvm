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
#include "mads/nebular/mac_nebular.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section2.h"
#include "mads/nebular/sound/mac_sound.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _activeMsgFl;
	bool _ladderTopFl;
	bool _waitingMeteoFl;
	bool _toStationFl;
	bool _toTeleportFl;
	long _ladderHotspotId;
	long _lastRoute;
	long _stationCounter;
	long _meteoFrame;
	long _meteoClock1;
	long _meteoClock2;
	long _startTime;
	byte _meteorologistSpecial;
};

static Scratch local;


static void room_202_init() {
	player.walker_been_visible = true;
	g_sprite_ids[1] = kernel_load_series(kernel_name('b', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('b', 1), 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('b', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('l', -1), 0);
	if (global[kSexOfRex] != SEX_MALE) {
		g_sprite_ids[7] = kernel_load_series("*ROXBD_2", 0);
	} else {
		g_sprite_ids[7] = kernel_load_series("*RXMBD_2", 0);
	}
	g_sprite_ids[8] = kernel_load_series(kernel_name('a', 0), 0);
	g_sprite_ids[9] = kernel_load_series(kernel_name('a', 1), 0);
	g_sprite_ids[11] = kernel_load_series(kernel_name('a', 2), 0);

	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 0, 0, 0);
	g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 6, 0, 0, 0);
	kernel_seq_loc(g_sequence_ids[2], 149, 113);
	kernel_seq_depth(g_sequence_ids[2], 10);
	int idx = kernel_add_dynamic(words_skull, words_walkto, 0, g_sequence_ids[2], 0, 0, 0, 0);
	kernel_dynamic_walk(idx, 153, 97, FACING_SOUTH);

	if (!(global[kBone202Status] & 1)) {
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 6, 0, 0, 0);
		kernel_seq_loc(g_sequence_ids[1], 130, 108);
		kernel_seq_depth(g_sequence_ids[1], 10);
		idx = kernel_add_dynamic(words_bone, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 132, 97, FACING_SOUTH);
	}

	if (!(global[kBone202Status] & 2)) {
		g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 6, 0, 0, 0);
		kernel_seq_loc(g_sequence_ids[6], 166, 110);
		kernel_seq_depth(g_sequence_ids[6], 10);
		idx = kernel_add_dynamic(words_bone, words_walkto, 0, g_sequence_ids[6], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 165, 99, FACING_SOUTH);
	}

	if (global[kBone202Status])
		kernel_load_variant(global[kBone202Status]);

	if (previous_room == 201) {
		player.x = 190;
		player.y = 91;
		player.facing = FACING_SOUTH;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 178;
		player.y = 152;
		player.facing = FACING_NORTH;
	}

	if (global[kLadderBroken]) {
		g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 6, 0, 0, 0);
		kernel_seq_depth(g_sequence_ids[5], 6);
		kernel_flip_hotspot(words_ladder, false);
		idx = kernel_add_dynamic(words_broken_ladder, words_walkto, 0, g_sequence_ids[5], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 246, 124, FACING_NORTH);
	}

	kernel.quotes = quote_load(92, 93, 94, 95, 96, 98, 99, 100, 101, 102, 97, 0);
	local._activeMsgFl = false;

	if (previous_room == KERNEL_RESTORING_GAME) {
		if (local._waitingMeteoFl) {
			g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 1);
			player.walker_visible = false;
		}
	} else {
		local._waitingMeteoFl = false;
		local._ladderTopFl = false;
	}

	local._meteoClock1 = local._meteoClock2 = kernel.clock;

	if (kernel.teleported_in)
		inter_give_to_player(OBJ_BINOCULARS);

	if (global[kMeteorologistWatch] != METEOROLOGIST_NORMAL) {
		player.walker_visible = false;
		player.commands_allowed = false;
		local._ladderTopFl = (global[kMeteorologistWatch] == METEOROLOGIST_TOWER);

		if (local._ladderTopFl) {
			g_sequence_ids[10] = kernel_seq_stamp(g_sprite_ids[9], true, 8);
			kernel_seq_depth(g_sequence_ids[10], 1);

			kernel_seq_loc(g_sequence_ids[10], 247, 82);
			player.x = 246;
			player.y = 124;
			player.facing = FACING_NORTH;
			global[kTeleporterUnderstood] = true;
		} else {
			g_sequence_ids[10] = kernel_seq_stamp(g_sprite_ids[9], false, 6);
			kernel_seq_depth(g_sequence_ids[10], 1);

			kernel_seq_loc(g_sequence_ids[10], 172, 123);
			player.x = 171;
			player.y = 122;
			player.facing = FACING_NORTH;
		}

		kernel_run_animation(kernel_name('M', -1), 71);
		kernel_reset_animation(0, 200);
	} else {
		if (local._ladderTopFl) {
			player.walker_visible = false;
			kernel_seq_stamp(g_sequence_ids[9], true, 1);
			kernel_seq_depth(g_sequence_ids[9], 1);
			kernel_seq_loc(g_sequence_ids[9], 247, 82);
			player.x = 246;
			player.y = 124;
			player.facing = FACING_NORTH;
		}
	}

	local._meteorologistSpecial = false;

	setMacintoshMessageColors(63, 10, 0, 63, 10, 0);
	section_2_music();
}

static void setRandomKernelMessage() {
	int vocabId = g_engine->getRandomNumber(92, 96);
	kernel_message_purge();
	kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
	kernel_message_add(quote_string(kernel.quotes, vocabId), 0, 0, 0x1110, 120, 70, 34);
	local._activeMsgFl = true;
}

static int subStep1(int randVal) {
	if ((randVal <= 100) || local._toStationFl)
		return 42;

	if ((randVal <= 200) || local._toTeleportFl)
		return 96;

	if ((randVal <= 300) && (local._lastRoute != 1))
		return 77;

	return 76;
}

static int subStep2(int randVal) {
	if ((randVal <= 150) && (local._stationCounter < 5))
		return 51;

	if ((randVal <= 300) || local._toTeleportFl)
		return 74;

	if (randVal <= 400)
		return 64;

	return 44;
}

static int subStep3(int randVal) {
	if ((randVal <= 100) || local._toStationFl)
		return 27;

	if ((randVal <= 200) || local._toTeleportFl)
		return 159;

	if ((randVal <= 300) && (local._lastRoute != 2))
		return 119;

	return 110;
}

static int subStep4(int randVal) {
	if ((randVal <= 100) || local._toTeleportFl)
		return 176;

	if (randVal <= 200)
		return 19;

	return 166;
}

static void room_202_daemon() {
	if (!local._activeMsgFl && (Common::Point(player.x, player.y) == Common::Point(77, 105)) && (player.facing == FACING_NORTH) && (g_engine->getRandomNumber(999) == 0)) {
		kernel_message_purge();
		local._activeMsgFl = false;
		if (g_engine->getRandomNumber(4) == 0)
			setRandomKernelMessage();
	}

	if (kernel.trigger == 70)
		local._activeMsgFl = false;

	if (kernel.trigger == 71) {
		g_engine->_soundManager->command(3, 0);
		g_engine->_soundManager->command(9, 0);

		local._meteoClock1 = kernel.clock + 15 * 60;

		if (global[kMeteorologistWatch] != METEOROLOGIST_NORMAL) {
			Common::Point msgPos;
			int msgFlag;
			if (!local._ladderTopFl) {
				msgPos = Common::Point(0, 0);
				msgFlag = 2;
			} else {
				msgPos = Common::Point(248, 15);
				msgFlag = 0;
			}
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 102), msgPos.x, msgPos.y, 0x1110, 120, 0, msgFlag | 32);
			kernel_message_teletype(msgIndex, 4, true);

			if (global[kMeteorologistWatch] == METEOROLOGIST_GROUND) {
				player2.words[0] = words_look;
				player2.words[1] = words_binoculars;
				player2.words[2] = words_strange_device;
				kernel.trigger_setup_mode = KERNEL_TRIGGER_PARSER;
				kernel_timing_trigger(2 * 60, 2);
				local._meteorologistSpecial = true;
			} else if (global[kMeteorologistWatch] == METEOROLOGIST_TOWER) {
				kernel_timing_trigger(2 * 60, 90);
			}
		}

		global[kMeteorologistWatch] = METEOROLOGIST_NORMAL;
	}

	switch (kernel.trigger) {
	case 90:
		g_engine->_soundManager->command(41, 0);
		kernel_seq_delete(g_sequence_ids[10]);
		g_sequence_ids[9] = kernel_seq_backward(g_sprite_ids[9], true, 6, 0, 0, 1);
		kernel_seq_loc(g_sequence_ids[9], 247, 82);
		kernel_seq_depth(g_sequence_ids[9], 1);
		kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 91);
		break;
	case 91:
		g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], true, 1);
		kernel_seq_loc(g_sequence_ids[9], 247, 82);
		kernel_seq_depth(g_sequence_ids[9], 1);
		kernel_timing_trigger(60, 92);
		break;
	case 92:
	{
		kernel_seq_delete(g_sequence_ids[9]);
		g_sequence_ids[11] = kernel_seq_forward(g_sprite_ids[11], false, 6, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[11], 1);
		kernel_seq_trigger(g_sequence_ids[11], KERNEL_TRIGGER_EXPIRE, 0, 93);
		kernel_message_purge();
		int msgIndex = kernel_message_add(quote_string(kernel.quotes, 98), 0, -65, 0x1110, 60, 0, 32);
		kernel_message_attach(msgIndex, g_sequence_ids[11]);
	}
	break;
	case 93:
	{
		global[kLadderBroken] = false;
		g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 6, 0, 0, 0);
		kernel_flip_hotspot(words_ladder, false);
		int idx = kernel_add_dynamic(words_broken_ladder, words_walkto, 0, g_sequence_ids[5], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 246, 124, FACING_NORTH);
		kernel_seq_timeout(g_sequence_ids[5], g_sequence_ids[11]);
		kernel_seq_timeout(g_sequence_ids[11], -1);
		player.commands_allowed = true;
		player.walker_visible = true;
		local._ladderTopFl = false;
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 99), 0, 0, 0x1110, 120, 0, 34);
	}
	break;
	default:
		break;
	}

	if ((kernel_anim[0].anim == nullptr) && (global[kMeteorologistStatus] != METEOROLOGIST_GONE) && (local._meteoClock2 <= kernel.clock) && (local._meteoClock1 <= kernel.clock)) {
		int randVal = g_engine->getRandomNumber(1, 500);
		int threshold = 1;
		if (local._ladderTopFl)
			threshold += 25;
		if (!global[kMeteorologistEverSeen])
			threshold += 25;
		if (threshold >= randVal) {
			g_engine->_soundManager->command(17, 0);
			kernel_run_animation(kernel_name('M', -1), 71);
			local._toStationFl = true;
			local._toTeleportFl = false;
			global[kMeteorologistEverSeen] = true;
			local._lastRoute = 0;
			local._stationCounter = 0;
			local._meteoClock2 = kernel.clock + 2;
		}
	}

	if (kernel_anim[0].anim == nullptr)
		return;

	if (local._waitingMeteoFl) {
		if (kernel_anim[0].frame >= 200) {
			if ((global[kMeteorologistWatch] == METEOROLOGIST_TOWER) || global[kLadderBroken]) {
				new_room = 213;
			} else {
				text_show(20201);
				kernel.force_restart = true;
			}
		}

		if ((kernel_anim[0].frame == 160) && (local._meteoFrame != kernel_anim[0].frame)) {
			Common::Point msgPos;
			int msgFlag;
			if (!local._ladderTopFl) {
				msgPos = Common::Point(0, 0);
				msgFlag = 2;
			} else {
				msgPos = Common::Point(248, 15);
				msgFlag = 0;
			}
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 101), msgPos.x, msgPos.y, 0x1110, 120, 0, msgFlag | 32);
			kernel_message_teletype(msgIndex, 4, true);
		}
	}

	if (local._meteoClock2 + 120 * 60 <= kernel.clock) {
		local._toTeleportFl = true;
	}

	if (kernel_anim[0].frame == local._meteoFrame) {
		return;
	}

	local._meteoFrame = kernel_anim[0].frame;
	int randVal = g_engine->getRandomNumber(1, 1000);
	int frameStep = -1;

	switch (kernel_anim[0].frame) {
	case 42:
	case 77:
	case 96:
		local._stationCounter = 0;
		frameStep = subStep1(randVal);
		break;
	case 51:
	case 74:
		local._toStationFl = false;
		frameStep = subStep2(randVal);
		break;
	case 27:
	case 119:
	case 159:
		frameStep = subStep3(randVal);
		break;
	case 176:
		frameStep = subStep4(randVal);
		break;
	case 59:
		local._lastRoute = 3;
		++local._stationCounter;
		if (randVal <= 800)
			frameStep = 55;
		break;
	case 89:
		local._lastRoute = 1;
		if (randVal <= 700)
			frameStep = 83;
		break;
	case 137:
		local._lastRoute = 2;
		if (randVal <= 700)
			frameStep = 126;
		break;
	default:
		break;
	}

	if (frameStep >= 0 && frameStep != kernel_anim[0].frame + 1) {
		kernel_reset_animation(0, frameStep);
		local._meteoFrame = frameStep;
	}
}

static void room_202_pre_parser() {

	if (player.need_to_walk)
		kernel_message_purge();

	if (local._ladderTopFl && (player_said_2(climb_down, ladder) || player.need_to_walk)) {
		if (kernel.trigger == 0) {
			if (!Sound::commandMacintoshSound(
					Sound::kMacSoundPlayRepeatedVolume, 2029, 2, 200))
				g_engine->_soundManager->command(29, 0);
			player.ready_to_walk = false;
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[9]);
			g_sequence_ids[8] = kernel_seq_backward(g_sprite_ids[8], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[8], 1);
			kernel_seq_trigger(g_sequence_ids[8], KERNEL_TRIGGER_EXPIRE, 0, 1);
		} else if (kernel.trigger == 1) {
			kernel_seq_timeout(g_sequence_ids[8], -1);
			kernel_delete_dynamic(local._ladderHotspotId);
			player.walker_visible = true;
			player.ready_to_walk = true;
			player.commands_allowed = true;
			local._ladderTopFl = false;
		}
	}

	if (player_said_2(look, binoculars) && (player2.words[2] > words_none)) {
		if (!player.ready_to_walk || local._ladderTopFl)
			player.need_to_walk = false;
		else
			player.need_to_walk = true;

		if (!local._ladderTopFl)
			player_walk(171, 122, FACING_NORTH);
	}
}

static void room_202_parser() {
	if (player.look_around) {
		text_show(20219);
		return;
	}

	if (player_said_2(climb_down, ladder)) {
		player.command_ready = false;
		return;
	} else if (player_said_2(walk_towards, field_to_south)) {
		new_room = 203;
	} else if (player_said_2(walk_towards, field_to_north)) {
		if (global[kMeteorologistStatus] != METEOROLOGIST_GONE) {
			if ((kernel_anim[0].anim != nullptr))
				global[kMeteorologistStatus] = METEOROLOGIST_PRESENT;
			else
				global[kMeteorologistStatus] = METEOROLOGIST_ABSENT;
		}
		new_room = 201;
	} else if (player_said_2(take, bone) && (player.main_object_source == 4)) {
		switch (kernel.trigger) {
		case 0:
			if (player_has(OBJ_BONES)) {
				text_show(20221);
			} else {
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sequence_ids[7] = kernel_seq_pingpong(g_sprite_ids[7], false, 3, 0, 0, 2);
				kernel_seq_player(g_sequence_ids[7], false);
				kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_SPRITE, 6, 1);
				kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_EXPIRE, 0, 2);
			}
			break;
		case 1:
			if (Common::Point(player.x, player.y) == Common::Point(132, 97)) {
				kernel_seq_delete(g_sequence_ids[1]);
				global[kBone202Status] |= BONE_202_LEFT_GONE;
			} else {
				kernel_seq_delete(g_sequence_ids[6]);
				global[kBone202Status] |= BONE_202_RIGHT_GONE;
			}
			break;
		case 2:
			if (player_has(OBJ_BONE)) {
				inter_take_from_player(OBJ_BONE, NOWHERE);
				inter_give_to_player(OBJ_BONES);
				object_examine(OBJ_BONES, 20218, 0);
			} else {
				inter_give_to_player(OBJ_BONE);
				object_examine(OBJ_BONE, 20218, 0);
			}
			kernel_load_variant(global[kBone202Status]);
			player.commands_allowed = true;
			player.walker_visible = true;
			break;
		default:
			break;
		}

		player.command_ready = false;
	} else if (player_said_2(climb_up, ladder) && !global[kLadderBroken]) {
		switch (kernel.trigger) {
		case 0:
			if (!Sound::commandMacintoshSound(Sound::kMacSoundWait,
					2029, 60, 2))
				g_engine->_soundManager->command(29, 0);
			local._meteoClock1 = kernel.clock;
			player.walker_visible = false;
			player.commands_allowed = false;

			local._ladderHotspotId = kernel_add_dynamic(words_ladder, words_climb_down, 0, -1, 241, 68, 12, 54);
			kernel_dynamic_walk(local._ladderHotspotId, 246, 124, FACING_NORTH);
			g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[8], 1);
			kernel_seq_trigger(g_sequence_ids[8], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;
		case 1:
		{
			g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], true, 1);
			kernel_seq_loc(g_sequence_ids[9], 247, 82);
			kernel_seq_depth(g_sequence_ids[9], 1);
			kernel_seq_timeout(g_sequence_ids[9], g_sequence_ids[8]);
			local._ladderTopFl = true;
			player.commands_allowed = true;
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 97), 248, 15, 0x1110, 60, 0, 32);
			kernel_message_teletype(msgIndex, 4, true);
		}
		break;
		default:
			player.command_ready = false;
			return;
		}
	} else if ((player_said_3(look, binoculars, field_to_north) || (player_said_3(look, binoculars, strange_device))) && (global[kSexOfRex] == SEX_MALE)) {
		if (!local._ladderTopFl) {
			switch (kernel.trigger) {
			case 0:
				local._toTeleportFl = true;
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sequence_ids[10] = kernel_seq_forward(g_sprite_ids[9], false, 6, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[10], 1, 6);
				kernel_seq_loc(g_sequence_ids[10], 172, 123);
				kernel_seq_depth(g_sequence_ids[10], 1);
				kernel_seq_timeout(g_sequence_ids[10], -1);
				kernel_seq_trigger(g_sequence_ids[10], KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;
			case 1:
				g_sequence_ids[10] = kernel_seq_stamp(g_sprite_ids[9], false, 6);
				kernel_seq_depth(g_sequence_ids[10], 1);
				kernel_seq_loc(g_sequence_ids[10], 172, 123);
				if ((kernel_anim[0].anim != nullptr)) {
					local._waitingMeteoFl = true;
					global[kMeteorologistWatch] = METEOROLOGIST_GROUND;
				} else {
					kernel_timing_trigger(120, 2);
				}
				break;
			case 2:
				if ((kernel_anim[0].anim == nullptr) && !local._meteorologistSpecial) {
					text_show(20222);
				}
				kernel_seq_delete(g_sequence_ids[10]);
				g_sequence_ids[10] = kernel_seq_backward(g_sprite_ids[9], false, 6, 0, 0, 1);
				kernel_seq_depth(g_sequence_ids[10], 1);
				kernel_seq_range(g_sequence_ids[10], 1, 6);
				kernel_seq_loc(g_sequence_ids[10], 172, 123);
				kernel_seq_trigger(g_sequence_ids[10], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;
			case 3:
				kernel_seq_timeout(g_sequence_ids[10], -1);
				player.commands_allowed = true;
				player.walker_visible = true;
				break;
			default:
				player.command_ready = false;
				return;
			}
		} else {
			switch (kernel.trigger) {
			case 0:
				local._toTeleportFl = true;
				player.commands_allowed = false;
				kernel_seq_delete(g_sequence_ids[9]);
				g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], true, 6, 0, 0, 1);
				kernel_seq_loc(g_sequence_ids[9], 247, 82);
				kernel_seq_depth(g_sequence_ids[9], 1);
				kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;
			case 1:
				g_sequence_ids[10] = kernel_seq_stamp(g_sprite_ids[9], true, -2);
				kernel_seq_loc(g_sequence_ids[10], 247, 82);
				kernel_seq_depth(g_sequence_ids[10], 1);
				if ((kernel_anim[0].anim != nullptr)) {
					if (kernel_anim[0].frame > 200) {
						kernel_timing_trigger(120, 2);
					} else {
						local._waitingMeteoFl = true;
						global[kMeteorologistWatch] = METEOROLOGIST_GONE;
						if ((kernel_anim[0].frame >= 44) && (kernel_anim[0].frame <= 75)) {
							kernel_message_purge();
							int msgIndex = kernel_message_add(quote_string(kernel.quotes, 100), 248, 15, 0x1110, 60, 0, 32);
							kernel_message_teletype(msgIndex, 4, false);
						} else {
							player.command_ready = false;
							return;
						}
					}
				} else {
					kernel_timing_trigger(120, 2);
				}
				break;
			case 2:
				if (kernel_anim[0].anim == nullptr)
					text_show(20222);
				local._meteorologistSpecial = false;
				kernel_seq_delete(g_sequence_ids[10]);
				g_sequence_ids[9] = kernel_seq_backward(g_sprite_ids[9], false, 6, 0, 0, 1);
				kernel_seq_loc(g_sequence_ids[9], 247, 82);
				kernel_seq_depth(g_sequence_ids[9], 1);
				kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;
			case 3:
				g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], true, 1);
				kernel_seq_loc(g_sequence_ids[9], 247, 82);
				kernel_seq_depth(g_sequence_ids[9], 1);
				player.commands_allowed = true;
				break;
			default:
				player.command_ready = false;
				return;
			}
		}
	} else if (player_said_2(walk_inside, hut)) {
		setRandomKernelMessage();
	} else if (player_said_2(look, rocks)) {
		text_show(20202);
	} else if (player_said_2(look, fire_pit)) {
		text_show(20203);
	} else if (player_said_2(look, grass)) {
		text_show(20204);
	} else if (player_said_2(look, field_to_north)) {
		if ((global[kMeteorologistStatus] == METEOROLOGIST_ABSENT) || (global[kMeteorologistStatus] == METEOROLOGIST_GONE))
			text_show(20205);
		else if (global[kMeteorologistStatus] == METEOROLOGIST_PRESENT)
			text_show(20220);
	} else if (player_said_2(look, watch_tower)) {
		text_show(20206);
	} else if (player_said_2(look, tall_grass)) {
		text_show(20207);
	} else if (player_said_2(look, trees)) {
		text_show(20208);
	} else if (player_said_2(look, tree)) {
		text_show(20209);
	} else if (player_said_2(look, sky)) {
		text_show(20210);
	} else if (player_said_2(look, hut)) {
		if ((Common::Point(player.x, player.y) == Common::Point(77, 105)) && (player.facing == FACING_NORTH))
			text_show(20212);
		else
			text_show(20211);
	} else if (player_said_2(look, strange_device)) {
		text_show(20213);
	} else if (player_said_2(look, ocean_in_distance)) {
		text_show(20214);
	} else if (player_said_2(look, skull)) {
		text_show(20215);
	} else if (player_said_2(take, skull)) {
		text_show(20216);
	} else if (player_said_2(look, bones) && player.command_source == 4) {
		text_show(20217);
	} else {
		return;
	}

	player.command_ready = false;
}

void room_202_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._activeMsgFl);
	s.syncAsByte(local._ladderTopFl);
	s.syncAsByte(local._waitingMeteoFl);
	s.syncAsByte(local._toStationFl);
	s.syncAsByte(local._toTeleportFl);

	s.syncAsSint32LE(local._ladderHotspotId);
	s.syncAsSint32LE(local._lastRoute);
	s.syncAsSint32LE(local._stationCounter);
	s.syncAsSint32LE(local._meteoFrame);

	s.syncAsUint32LE(local._meteoClock1);
	s.syncAsUint32LE(local._meteoClock2);
	s.syncAsUint32LE(local._startTime);

	s.syncAsByte(local._meteorologistSpecial);
}

void room_202_preload() {
	room_init_code_pointer = room_202_init;
	room_pre_parser_code_pointer = room_202_pre_parser;
	room_parser_code_pointer = room_202_parser;
	room_daemon_code_pointer = room_202_daemon;

	section_2_walker();
	section_2_interface();

	vocab_make_active(words_ladder);
	vocab_make_active(words_climb_down);
	vocab_make_active(words_walkto);
	vocab_make_active(words_bone);
	vocab_make_active(words_skull);
	vocab_make_active(words_broken_ladder);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
