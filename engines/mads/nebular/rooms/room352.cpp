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
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _vaultOpenFl;
	bool _mustPutArmDownFl;
	bool _leaveRoomFl;
	int32 _tapePlayerHotspotIdx;
	int32 _hotspot1Idx;
	int32 _hotspot2Idx;
	int32 _lampHostpotIdx;
	int32 _commonSequenceIdx;
	int32 _commonSpriteIndex;
};

static Scratch local;


static void putArmDown(bool corridorExit, bool doorwayExit) {
	switch (kernel.trigger) {
	case 0:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0xFF));
		_scene->_sequences.addTimer(48, 1);
		break;

	case 1:
		player.commands_allowed = false;
		player.walker_visible = false;
		if (global[kSexOfRex] == REX_FEMALE) {
			g_sequence_ids[3] = _scene->_sequences.startPingPongCycle(g_sprite_ids[3], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
			_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_SPRITE, 5, 2);
			_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		} else {
			g_sequence_ids[4] = _scene->_sequences.startPingPongCycle(g_sprite_ids[4], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(g_sequence_ids[4]);
			_scene->_sequences.addSubEntry(g_sequence_ids[4], SEQUENCE_TRIGGER_SPRITE, 6, 2);
			_scene->_sequences.addSubEntry(g_sequence_ids[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		}
		break;

	case 2:
	{
		g_sequence_ids[2] = _scene->_sequences.addSpriteCycle(g_sprite_ids[2], false, 6, 0, 0, 0);
		int idx = _scene->_dynamicHotspots.add(words_guards_arm2, words_walkto, g_sequence_ids[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(230, 117), FACING_NORTHWEST);
		kernel_load_variant(0);
	}
	break;

	case 3:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x100));
		inter_move_object(OBJ_GUARDS_ARM, room_id);
		player.walker_visible = true;
		if (corridorExit)
			_scene->_sequences.addTimer(48, 6);
		else if (doorwayExit)
			_scene->_sequences.addTimer(48, 4);
		else {
			local._mustPutArmDownFl = false;
			player.commands_allowed = true;
		}
		break;

	case 4:
		player_walk(116, 107, FACING_NORTH);
		local._mustPutArmDownFl = false;
		_scene->_sequences.addTimer(180, 5);
		local._leaveRoomFl = true;
		break;

	case 5:
		if (local._leaveRoomFl)
			new_room = 351;

		break;

	case 6:
		player_walk(171, 152, FACING_SOUTH);
		player.commands_allowed = true;
		local._mustPutArmDownFl = false;
		_scene->_sequences.addTimer(180, 7);
		local._leaveRoomFl = true;
		break;

	case 7:
		if (local._leaveRoomFl)
			new_room = 353;

		break;

	default:
		break;
	}
}

static void room_352_init() {
	g_sprite_ids[1] = kernel_load_series("*RM302x0", 0);
	g_sprite_ids[13] = kernel_load_series("*RM302x2", 0);
	g_sprite_ids[12] = kernel_load_series("*RM302x3", 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('g', -1), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('b', -1), 0);


	if (global[kSexOfRex] == REX_FEMALE) {
		g_sprite_ids[3] = kernel_load_series("*ROXRC_7", 0);
		g_sprite_ids[7] = kernel_load_series("*ROXRC_6", 0);
		g_sprite_ids[15] = kernel_load_series("*ROXRC_9", 0);
		g_sprite_ids[11] = kernel_load_series(kernel_name('a', 3), 0);
		g_sprite_ids[9] = kernel_load_series(kernel_name('a', 2), 0);
	} else {
		g_sprite_ids[4] = kernel_load_series("*RXRD_7", 0);
		g_sprite_ids[6] = kernel_load_series("*RXRC_6", 0);
		g_sprite_ids[14] = kernel_load_series("*RXMRC_9", 0);
		g_sprite_ids[10] = kernel_load_series(kernel_name('a', 1), 0);
		g_sprite_ids[8] = kernel_load_series(kernel_name('a', 0), 0);
	}

	local._leaveRoomFl = false;

	if (object_is_here(OBJ_TAPE_PLAYER)) {
		g_sequence_ids[5] = _scene->_sequences.addSpriteCycle(g_sprite_ids[5], false, 12, 0, 0, 0);
		_scene->_sequences.setDepth(g_sequence_ids[5], 5);
		int idx = _scene->_dynamicHotspots.add(words_tape_player, words_walkto, g_sequence_ids[5], Common::Rect(0, 0, 0, 0));
		local._tapePlayerHotspotIdx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(84, 145), FACING_WEST);
	}

	local._vaultOpenFl = false;

	if (previous_room != RETURNING_FROM_DIALOG) {
		local._mustPutArmDownFl = false;
		if (!player.been_here_before)
			global[kHaveYourStuff] = false;
	}

	if (object_is_here(OBJ_GUARDS_ARM)) {
		g_sequence_ids[2] = _scene->_sequences.addSpriteCycle(g_sprite_ids[2], false, 6, 0, 0, 0);
		int idx = _scene->_dynamicHotspots.add(words_guards_arm2, words_walkto, g_sequence_ids[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(230, 117), FACING_NORTHWEST);
	} else
		local._mustPutArmDownFl = true;

	if (previous_room == 353) {
		player.x = 171;
		player.y = 155;
	}
	else if (previous_room != RETURNING_FROM_DIALOG) {
		player.x = 116;
		player.y = 107;
	}

	section_3_music();

	kernel.quotes = quote_load(0xFF, 0x100, 0x101, 0x102, 0x103, 0);
}

static void room_352_pre_parser() {
	local._leaveRoomFl = false;

	if (player_said_2(open, vault))
		player_walk(266, 111, FACING_NORTHEAST);

	if (local._vaultOpenFl && !player_said_1(vault) && !player_said_1(lamp) && !player_said_1(other_stuff) && !player_said_1(your_stuff)) {
		if (global[kHaveYourStuff]) {
			local._commonSpriteIndex = g_sprite_ids[13];
			local._commonSequenceIdx = g_sequence_ids[13];
		} else {
			local._commonSpriteIndex = g_sprite_ids[1];
			local._commonSequenceIdx = g_sequence_ids[1];
		}

		switch (kernel.trigger) {
		case 0:
			if (player.need_to_walk) {
				player.commands_allowed = false;
				_scene->_sequences.remove(local._commonSequenceIdx);
				g_engine->_soundManager->command(20, 0);
				local._commonSequenceIdx = _scene->_sequences.addReverseSpriteCycle(local._commonSpriteIndex, false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.setDepth(local._commonSequenceIdx, 15);
			}
			break;

		case 1:
			if (!global[kHaveYourStuff])
				_scene->_dynamicHotspots.remove(local._hotspot2Idx);

			_scene->_dynamicHotspots.remove(local._hotspot1Idx);
			_scene->_dynamicHotspots.remove(local._lampHostpotIdx);
			local._vaultOpenFl = false;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	}

	if (player_said_3(put, guards_arm2, scanner)) {
		if (global[kSexOfRex] == REX_MALE)
			player_walk(269, 111, FACING_NORTHEAST);
		else
			player_walk(271, 111, FACING_NORTHEAST);
	}

	if (player_said_2(walk_through, doorway) || player_said_2(walk_down, corridor_to_south) || player_said_3(put, guards_arm2, floor)) {
		if (player_has(OBJ_GUARDS_ARM))
			player_walk(230, 117, FACING_NORTHWEST);
	}
}

static void room_352_parser() {
	if (player.look_around) {
		text_show(35225);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, vault)) {
		if (!local._vaultOpenFl) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				if (global[kSexOfRex] == REX_FEMALE)
					local._commonSpriteIndex = g_sprite_ids[9];
				else
					local._commonSpriteIndex = g_sprite_ids[8];

				local._commonSequenceIdx = _scene->_sequences.addSpriteCycle(local._commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.updateTimeout(local._commonSequenceIdx, -1);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
			{
				g_engine->_soundManager->command(21, 0);
				g_sequence_ids[12] = _scene->_sequences.startPingPongCycle(g_sprite_ids[12], false, 7, 2, 20, 0);
				_scene->_sequences.setDepth(g_sequence_ids[12], FACING_NORTH);
				int oldIdx = local._commonSequenceIdx;
				local._commonSequenceIdx = _scene->_sequences.startCycle(local._commonSpriteIndex, false, -2);
				_scene->_sequences.updateTimeout(local._commonSequenceIdx, oldIdx);
				_scene->_sequences.addSubEntry(g_sequence_ids[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

			case 2:
				g_engine->_soundManager->command(22, 0);
				_scene->_sequences.remove(local._commonSequenceIdx);
				local._commonSequenceIdx = _scene->_sequences.startPingPongCycle(local._commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(local._commonSequenceIdx, 1, 3);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_scene->_sequences.updateTimeout(-1, local._commonSequenceIdx);
				player.walker_visible = true;
				_scene->_sequences.addTimer(60, 4);
				break;

			case 4:
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x101));
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		}
		player.command_ready = false;
		return;
	}

	if (player_has(OBJ_GUARDS_ARM)) {
		local._mustPutArmDownFl = true;
	}

	bool exit_corridor = false;
	bool exit_doorway = false;

	if (player_said_2(walk_down, corridor_to_south)) {
		exit_corridor = true;
	}

	if (player_said_2(walk_through, doorway)) {
		exit_doorway = true;
	}

	if (player_said_2(walk_down, corridor_to_south) || player_said_2(walk_through, doorway) || player_said_3(put, guards_arm2, floor)) {
		if (local._mustPutArmDownFl)
			putArmDown(exit_corridor, exit_doorway);
		else if (exit_corridor)
			new_room = 353;
		else
			new_room = 351;

		player.command_ready = false;
		return;
	}

	if (player_said_2(take, guards_arm2)) {
		if (kernel.trigger || !player_has(OBJ_GUARDS_ARM)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				if (global[kSexOfRex] == REX_FEMALE) {
					g_sequence_ids[3] = _scene->_sequences.startPingPongCycle(g_sprite_ids[3], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
					_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					g_sequence_ids[4] = _scene->_sequences.startPingPongCycle(g_sprite_ids[4], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(g_sequence_ids[4]);
					_scene->_sequences.addSubEntry(g_sequence_ids[4], SEQUENCE_TRIGGER_SPRITE, 6, 1);
					_scene->_sequences.addSubEntry(g_sequence_ids[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				_scene->_sequences.remove(g_sequence_ids[2]);
				inter_give_to_player(OBJ_GUARDS_ARM);
				kernel_load_variant(1);
				break;

			case 2:
				player.walker_visible = true;
				player.commands_allowed = true;
				object_examine(OBJ_GUARDS_ARM, 0x899C, 0);
				break;

			default:
				break;
			}
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(put, guards_arm2, scanner)) {
		if (!local._vaultOpenFl) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				if (global[kSexOfRex] == REX_FEMALE)
					local._commonSpriteIndex = g_sprite_ids[11];
				else
					local._commonSpriteIndex = g_sprite_ids[10];

				local._commonSequenceIdx = _scene->_sequences.addSpriteCycle(local._commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.updateTimeout(local._commonSequenceIdx, -1);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
			{
				g_engine->_soundManager->command(21, 0);
				g_sequence_ids[12] = _scene->_sequences.startPingPongCycle(g_sprite_ids[12], false, 7, 2, 20, 0);
				_scene->_sequences.setDepth(g_sequence_ids[12], 8);
				int oldIdx = local._commonSequenceIdx;
				local._commonSequenceIdx = _scene->_sequences.startCycle(local._commonSpriteIndex, false, -2);
				_scene->_sequences.updateTimeout(local._commonSequenceIdx, oldIdx);
				_scene->_sequences.addSubEntry(g_sequence_ids[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

			case 2:
				g_engine->_soundManager->command(23, 0);
				_scene->_sequences.remove(local._commonSequenceIdx);
				local._commonSequenceIdx = _scene->_sequences.addReverseSpriteCycle(local._commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(local._commonSequenceIdx, 1, 4);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;


			case 3:
				_scene->_sequences.updateTimeout(-1, local._commonSequenceIdx);
				player.walker_visible = true;
				if (global[kHaveYourStuff])
					local._commonSpriteIndex = g_sprite_ids[13];
				else
					local._commonSpriteIndex = g_sprite_ids[1];

				g_engine->_soundManager->command(20, 0);
				local._commonSequenceIdx = _scene->_sequences.addSpriteCycle(local._commonSpriteIndex, false, 6, 1, 0, 0);
				_scene->_sequences.setDepth(local._commonSequenceIdx, 15);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 4);
				break;

			case 4:
				local._commonSequenceIdx = _scene->_sequences.addSpriteCycle(local._commonSpriteIndex, false, 6, 0, 0, 0);
				_scene->_sequences.setAnimRange(local._commonSequenceIdx, -2, -2);
				_scene->_sequences.setDepth(local._commonSequenceIdx, 15);
				_scene->_sequences.addTimer(60, 5);
				break;

			case 5:
			{
				local._vaultOpenFl = true;
				int idx;
				if (!global[kHaveYourStuff]) {
					idx = _scene->_dynamicHotspots.add(words_your_stuff, words_walkto, -1, Common::Rect(282, 87, 282 + 13, 87 + 7));
					local._hotspot2Idx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(280, 111), FACING_NORTHEAST);
					g_sequence_ids[1] = local._commonSequenceIdx;
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x102));
				} else {
					g_sequence_ids[13] = local._commonSequenceIdx;
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x103));
				}

				idx = _scene->_dynamicHotspots.add(words_other_stuff, words_walkto, -1, Common::Rect(282, 48, 282 + 36, 48 + 27));
				local._hotspot1Idx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(287, 115), FACING_NORTHEAST);
				idx = _scene->_dynamicHotspots.add(words_lamp, words_walkto, -1, Common::Rect(296, 76, 296 + 11, 76 + 17));
				local._lampHostpotIdx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(287, 115), FACING_NORTHEAST);
				player.commands_allowed = true;
			}
			break;

			default:
				break;
			}
		}
	} else if (player_said_2(take, your_stuff)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			if (global[kSexOfRex] == REX_MALE) {
				g_sequence_ids[14] = _scene->_sequences.startPingPongCycle(g_sprite_ids[14], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(g_sequence_ids[14], 1, 2);
				_scene->_sequences.setMsgLayout(g_sequence_ids[14]);
				_scene->_sequences.addSubEntry(g_sequence_ids[14], SEQUENCE_TRIGGER_SPRITE, 2, 1);
				_scene->_sequences.addSubEntry(g_sequence_ids[14], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			} else {
				g_sequence_ids[15] = _scene->_sequences.startPingPongCycle(g_sprite_ids[15], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(g_sequence_ids[15], 1, 2);
				_scene->_sequences.setMsgLayout(g_sequence_ids[15]);
				_scene->_sequences.addSubEntry(g_sequence_ids[15], SEQUENCE_TRIGGER_SPRITE, 2, 1);
				_scene->_sequences.addSubEntry(g_sequence_ids[15], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 1:
			_scene->_dynamicHotspots.remove(local._hotspot2Idx);
			global[kHaveYourStuff] = true;

			for (uint16 i = 0; i < num_objects; i++) {
				if (object[i].location == 50)
					inter_give_to_player(i);
			}

			_scene->_sequences.remove(g_sequence_ids[1]);
			g_sequence_ids[13] = _scene->_sequences.startCycle(g_sprite_ids[13], false, -2);
			_scene->_sequences.setDepth(g_sequence_ids[13], 15);
			break;

		case 2:
			if (global[kSexOfRex] == REX_MALE)
				_scene->_sequences.updateTimeout(-1, g_sequence_ids[14]);
			else
				_scene->_sequences.updateTimeout(-1, g_sequence_ids[15]);

			player.walker_visible = true;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(take, tape_player) && !player_has(OBJ_TAPE_PLAYER)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			if (global[kSexOfRex] == REX_MALE) {
				g_sequence_ids[6] = _scene->_sequences.startPingPongCycle(g_sprite_ids[6], true, 6, 2, 0, 0);
				_scene->_sequences.setMsgLayout(g_sequence_ids[6]);
				_scene->_sequences.addSubEntry(g_sequence_ids[6], SEQUENCE_TRIGGER_SPRITE, 6, 1);
				_scene->_sequences.addSubEntry(g_sequence_ids[6], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			} else {
				g_sequence_ids[7] = _scene->_sequences.startPingPongCycle(g_sprite_ids[7], true, 6, 2, 0, 0);
				_scene->_sequences.setMsgLayout(g_sequence_ids[7]);
				_scene->_sequences.addSubEntry(g_sequence_ids[7], SEQUENCE_TRIGGER_SPRITE, 6, 1);
				_scene->_sequences.addSubEntry(g_sequence_ids[7], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 1:
			_scene->_sequences.remove(g_sequence_ids[5]);
			_scene->_dynamicHotspots.remove(local._tapePlayerHotspotIdx);
			break;

		case 2:
			inter_give_to_player(OBJ_TAPE_PLAYER);
			if (global[kSexOfRex] == REX_MALE)
				_scene->_sequences.updateTimeout(-1, g_sequence_ids[6]);
			else
				_scene->_sequences.updateTimeout(-1, g_sequence_ids[7]);

			player.walker_visible = true;
			player.commands_allowed = true;
			object_examine(OBJ_TAPE_PLAYER, 35227, 0);
			break;

		default:
			break;
		}
	} else if (player_said_2(look, scanner))
		text_show(35210);
	else if (player_said_2(look, monitor)) {
		if (config_file.naughtiness == STORYMODE_NAUGHTY)
			text_show(35211);
		else
			text_show(35212);
	} else if (player_said_2(look, display))
		text_show(35213);
	else if (player_said_2(look, statue))
		text_show(35214);
	else if (player_said_2(look, tape_player) && (player.main_object_source == 4))
		text_show(35215);
	else if (player_said_2(look, air_vent))
		text_show(35216);
	else if (player_said_2(look, guards_arm2) && (player.main_object_source == 4))
		text_show(35217);
	else if (player_said_2(look, ironing_board))
		text_show(35218);
	else if (player_said_2(look, clock))
		text_show(35219);
	else if (player_said_2(look, gauge))
		text_show(35220);
	else if (player_said_2(look, vault)) {
		if (!local._vaultOpenFl)
			text_show(35221);
	} else if (player_said_2(look, your_stuff))
		text_show(35222);
	else if (player_said_2(look, other_stuff))
		text_show(35223);
	else if (player_said_2(look, corridor_to_south))
		text_show(35224);
	else if (player_said_2(take, other_stuff))
		text_show(35226);
	else if (player_said_2(look, desk))
		text_show(35229);
	else if (player_said_2(look, guard))
		text_show(35230);
	else if (player_said_2(look, doorway))
		text_show(35231);
	else if (player_said_2(look, table))
		text_show(35232);
	else if (player_said_2(look, projector))
		text_show(35233);
	else if (player_said_2(look, support))
		text_show(35234);
	else if (player_said_2(look, security_monitor))
		text_show(35235);
	else
		return;

	player.command_ready = false;
}

void room_352_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._vaultOpenFl);
	s.syncAsByte(local._mustPutArmDownFl);
	s.syncAsByte(local._leaveRoomFl);

	s.syncAsSint32LE(local._tapePlayerHotspotIdx);
	s.syncAsSint32LE(local._hotspot1Idx);
	s.syncAsSint32LE(local._hotspot2Idx);
	s.syncAsSint32LE(local._lampHostpotIdx);
	s.syncAsSint32LE(local._commonSequenceIdx);
	s.syncAsSint32LE(local._commonSpriteIndex);
}

void room_352_preload() {
	room_init_code_pointer = room_352_init;
	room_pre_parser_code_pointer = room_352_pre_parser;
	room_parser_code_pointer = room_352_parser;

	section_3_walker();
	section_3_interface();
	vocab_make_active(words_walkto);
	vocab_make_active(words_your_stuff);
	vocab_make_active(words_other_stuff);
	vocab_make_active(words_lamp);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
