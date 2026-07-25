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
#include "mads/nebular/rooms/section6.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _lastSpriteIdx;
	int16 _lastSequenceIdx;
	int16 _cycleIndex;
	int16 _safeMode;
};

static Scratch local;


static void room_602_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('h', -1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('l', 0), 0);
	g_sprite_ids[5] = kernel_load_series("*RXMRC_9", 0);

	if (!player.been_here_before)
		global[kSafeStatus] = 0;

	if (global[kLaserHoleIsThere]) {
		g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 1);
		_scene->_sequences.setDepth(g_sequence_ids[1], 9);
		g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 1);
		_scene->_sequences.setDepth(g_sequence_ids[4], 9);
		int idx = _scene->_dynamicHotspots.add(words_laser_beam, words_walkto, g_sequence_ids[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(80, 134), FACING_NORTHEAST);
		kernel_load_variant(1);
	} else
		_scene->_hotspots.activate(words_hole, false);

	if (global[kSafeStatus] == 0) {
		local._lastSpriteIdx = g_sprite_ids[2];
		local._cycleIndex = -1;
	} else if (global[kSafeStatus] == 1) {
		local._lastSpriteIdx = g_sprite_ids[2];
		local._cycleIndex = -2;
	} else if (global[kSafeStatus] == 3) {
		local._lastSpriteIdx = g_sprite_ids[3];
		local._cycleIndex = -2;
	} else {
		local._lastSpriteIdx = g_sprite_ids[3];
		local._cycleIndex = -1;
	}

	local._lastSequenceIdx = _scene->_sequences.startCycle(local._lastSpriteIdx, false, local._cycleIndex);
	_scene->_sequences.setDepth(local._lastSequenceIdx, 14);
	int idx = _scene->_dynamicHotspots.add(words_safe, words_walkto, local._lastSequenceIdx, Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(185, 113), FACING_NORTHWEST);

	if (object_is_here(OBJ_DOOR_KEY)) {
		g_sprite_ids[6] = kernel_load_series(kernel_name('k', -1), 0);
		g_sequence_ids[6] = _scene->_sequences.startCycle(g_sprite_ids[6], false, -1);
		_scene->_sequences.setDepth(g_sequence_ids[6], 15);
		if (global[kSafeStatus] == 0 || global[kSafeStatus] == 2)
			_scene->_hotspots.activate(words_door_key, false);
	} else
		_scene->_hotspots.activate(words_door_key, false);

	if (previous_room == 603) {
		player.x = 228;
		player.y = 126;
		player.facing = FACING_WEST;
	} else if (previous_room != RETURNING_FROM_DIALOG) {
		player.x = 50;
		player.y = 127;
		player.facing = FACING_EAST;
	}

	section_6_music();
	kernel.quotes = quote_load(0x2F1, 0x2F2, 0x2F3, 0);

	if (kernel.teleported_in) {
		inter_give_to_player(OBJ_NOTE);
		inter_give_to_player(OBJ_REARVIEW_MIRROR);
		inter_give_to_player(OBJ_COMPACT_CASE);
	}
}

static void handleSafeActions() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[5] = _scene->_sequences.startPingPongCycle(g_sprite_ids[5], true, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[5], 1, 3);
		_scene->_sequences.setMsgLayout(g_sequence_ids[5]);
		_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_SPRITE, 3, 1);
		_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		break;

	case 1:
		if (local._safeMode == 1 || local._safeMode == 3) {
			if (global[kSafeStatus] == 0 && local._safeMode == 1) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x2F1));
				_scene->_sequences.addTimer(120, 4);
			} else {
				_scene->_sequences.remove(local._lastSequenceIdx);
				if (local._safeMode == 3)
					local._lastSpriteIdx = g_sprite_ids[2];
				else
					local._lastSpriteIdx = g_sprite_ids[3];

				local._lastSequenceIdx = _scene->_sequences.addSpriteCycle(local._lastSpriteIdx, false, 12, 1, 0, 0);
				_scene->_sequences.setDepth(local._lastSequenceIdx, 14);
				if (object[OBJ_DOOR_KEY].location == room_id)
					_scene->_hotspots.activate(words_door_key, true);

				_scene->_sequences.addSubEntry(local._lastSequenceIdx,
					SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
		} else {
			_scene->_sequences.remove(local._lastSequenceIdx);
			if (global[kSafeStatus] == 1)
				local._lastSpriteIdx = g_sprite_ids[2];
			else
				local._lastSpriteIdx = g_sprite_ids[3];

			local._lastSequenceIdx = _scene->_sequences.startPingPongCycle(local._lastSpriteIdx, false, 12, 1, 0, 0);
			_scene->_sequences.setDepth(local._lastSequenceIdx, 14);
			if (object[OBJ_DOOR_KEY].location == room_id)
				_scene->_hotspots.activate(words_door_key, false);

			_scene->_sequences.addSubEntry(local._lastSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		}
		break;

	case 2:
	{
		int synxIdx = local._lastSequenceIdx;
		local._lastSequenceIdx = _scene->_sequences.startCycle(local._lastSpriteIdx, false, local._cycleIndex);
		_scene->_sequences.setDepth(local._lastSequenceIdx, 14);
		_scene->_sequences.updateTimeout(local._lastSequenceIdx, synxIdx);
		int idx = _scene->_dynamicHotspots.add(words_safe, words_walkto, local._lastSequenceIdx, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(185, 113), FACING_NORTHWEST);
		if (local._safeMode == 3) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x2F3));
			_scene->_sequences.addTimer(120, 4);
		} else
			_scene->_sequences.addTimer(60, 4);
		break;
	}

	case 3:
		_scene->_sequences.updateTimeout(-1, g_sequence_ids[5]);
		player.walker_visible = true;
		break;

	case 4:
		if (local._safeMode == 1) {
			if (global[kSafeStatus] == 2)
				global[kSafeStatus] = 3;
		} else if (local._safeMode == 2) {
			if (global[kSafeStatus] == 3)
				global[kSafeStatus] = 2;
			else
				global[kSafeStatus] = 0;
		} else
			global[kSafeStatus] = 1;

		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void room_602_parser() {
	if (player_said_2(walk_through, hallway))
		new_room = 601;
	else if (player_said_2(walk_through, doorway))
		new_room = 603;
	else if (player_said_2(open, safe) && ((global[kSafeStatus] == 0) || (global[kSafeStatus] == 2))) {
		local._safeMode = 1;
		local._cycleIndex = -2;
		handleSafeActions();
	} else if (player_said_2(close, safe) && ((global[kSafeStatus] == 1) || (global[kSafeStatus] == 3))) {
		local._safeMode = 2;
		local._cycleIndex = -1;
		handleSafeActions();
	} else if (player_said_3(unlock, combination, safe)) {
		if ((global[kSafeStatus] == 0) && (game.difficulty != DIFFICULTY_HARD)) {
			local._safeMode = 3;
			local._cycleIndex = -2;
			handleSafeActions();
		}
	} else if ((player_said_3(put, rearview_mirror, laser_beam) || player_said_3(put, compact_case, laser_beam)
		|| player_said_3(reflect, compact_case, laser_beam) || player_said_3(reflect, rearview_mirror, laser_beam)) && (global[kSafeStatus] == 0)) {
		switch (kernel.trigger) {
		case 0:
			text_show(60230);
			player.commands_allowed = false;
			player.walker_visible = false;
			_scene->_sequences.remove(g_sequence_ids[4]);
			_scene->_sequences.remove(local._lastSequenceIdx);
			kernel_run_animation(kernel_name('L', 1), 1);
			break;

		case 1:
		{
			player.walker_visible = true;
			player.clock = _scene->_animation[0]->getNextFrameTimer() - player.frame_delay;
			local._lastSpriteIdx = g_sprite_ids[3];
			local._lastSequenceIdx = _scene->_sequences.startCycle(local._lastSpriteIdx, false, -1);
			_scene->_sequences.setDepth(local._lastSequenceIdx, 14);
			int idx = _scene->_dynamicHotspots.add(words_safe, words_walkto, local._lastSequenceIdx, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(185, 113), FACING_NORTHWEST);
			g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 1);
			_scene->_sequences.setDepth(g_sequence_ids[4], 9);
			idx = _scene->_dynamicHotspots.add(words_laser_beam, words_walkto, g_sequence_ids[4], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(80, 134), FACING_NORTHEAST);
			_scene->_sequences.addTimer(60, 2);
		}
		break;

		case 2:
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x2F2));
			global[kSafeStatus] = 2;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(take, door_key) && (kernel.trigger || object_is_here(OBJ_DOOR_KEY))) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[5] = _scene->_sequences.startPingPongCycle(g_sprite_ids[5], true, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[5], 1, 3);
			_scene->_sequences.setMsgLayout(g_sequence_ids[5]);
			_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_SPRITE, 3, 1);
			_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(g_sequence_ids[6]);
			_scene->_hotspots.activate(words_door_key, false);
			g_engine->_soundManager->command(9, 0);
			inter_give_to_player(OBJ_DOOR_KEY);
			break;

		case 2:
			_scene->_sequences.updateTimeout(-1, g_sequence_ids[5]);
			player.walker_visible = true;
			player.commands_allowed = true;
			object_examine(OBJ_DOOR_KEY, 835, 0);
			break;

		default:
			break;
		}
	} else if (player.look_around)
		text_show(60210);
	else if (player_said_2(look, floor))
		text_show(60211);
	else if (player_said_2(look, hallway))
		text_show(60212);
	else if (player_said_2(look, table))
		text_show(60213);
	else if (player_said_2(look, chair) || player_said_2(look, lounge_chair))
		text_show(60214);
	else if (player_said_2(look, neon_lights))
		text_show(60215);
	else if (player_said_2(look, fireplace))
		text_show(60216);
	else if (player_said_2(look, picture))
		text_show(60217);
	else if (player_said_2(look, lamp))
		text_show(60218);
	else if (player_said_2(look, masks))
		text_show(60219);
	else if (player_said_2(look, glass_block_wall))
		text_show(60220);
	else if (player_said_2(look, doorway))
		text_show(60221);
	else if (player_said_2(look, safe)) {
		if (global[kSafeStatus] == 0)
			text_show(60222);
		else if (global[kSafeStatus] == 1) {
			if (!object_is_here(OBJ_DOOR_KEY))
				text_show(60223);
			else
				text_show(60224);
		} else if (global[kSafeStatus] == 2)
			text_show(60234);
		else if (object_is_here(OBJ_DOOR_KEY))
			text_show(60235);
		else
			text_show(60236);
	} else if (player_said_3(unlock, door_key, safe) || player_said_3(unlock, padlock_key, safe))
		text_show(60225);
	else if (player_said_2(pull, safe))
		text_show(60226);
	else if (player_said_2(put, fireplace) && player_has(object_named(player2.words[1])))
		text_show(60227);
	else if (player_said_2(look, hole))
		text_show(60228);
	else if (player_said_2(look, laser_beam))
		text_show(60229);
	else if (player_said_2(look, flower_box))
		text_show(60231);
	else if (player_said_3(throw, bomb, safe) || player_said_3(throw, bombs, safe))
		text_show(60232);
	else if (player_said_2(put, timebomb))
		text_show(60233);
	else
		return;

	player.command_ready = false;
}

void room_602_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._lastSpriteIdx);
	s.syncAsSint16LE(local._lastSequenceIdx);
	s.syncAsSint16LE(local._cycleIndex);
	s.syncAsSint16LE(local._safeMode);
}

void room_602_preload() {
	room_init_code_pointer = room_602_init;
	room_parser_code_pointer = room_602_parser;

	section_6_walker();
	section_6_interface();
	vocab_make_active(words_walkto);
	vocab_make_active(words_safe);
	vocab_make_active(words_laser_beam);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
