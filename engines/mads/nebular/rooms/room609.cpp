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
	int16 _videoDoorMode;
};

static Scratch local;


static void room_609_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('c', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(kernel_name('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(kernel_name('h', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCD_9");
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXMRC_9");

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);

	if (!player.been_here_before)
		_globals[kBeenInVideoStore] = false;

	if (_scene->_priorSceneId == 611) {
		player.x = 264;
		player.y = 69;
		player.facing = FACING_SOUTHWEST;
	} else if (_scene->_priorSceneId == 610) {
		player.x = 23;
		player.y = 90;
		player.facing = FACING_EAST;
		_scene->_sequences.addTimer(60, 60);
		player.commands_allowed = false;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 86;
		player.y = 136;
		player.facing = FACING_NORTHEAST;
		player.walker_visible = false;
		player.commands_allowed = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
		_scene->loadAnimation(kernel_name('R', 1), 70);
	}

	if (_scene->_roomChanged) {
		inter_give_to_player(OBJ_DOOR_KEY);
		if (game.difficulty != DIFFICULTY_EASY)
			inter_give_to_player(OBJ_PENLIGHT);
	}

	section_6_music();
	kernel.quotes = quote_load(0x305, 0x306, 0x307, 0x308, 0x309, 0);
}

static void room_609_daemon() {
	switch (kernel.trigger) {
	case 60:
		player.commands_allowed = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		_scene->_hotspots.activate(words_video_store_door, false);
		player_walk(101, 100, FACING_EAST);
		_scene->_sequences.addTimer(180, 62);
		break;

	case 62:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_hotspots.activate(words_video_store_door, true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 63);
		break;

	case 63:
		if (!_globals[kHasTalkedToHermit] && (game.difficulty != DIFFICULTY_HARD)) {
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 26, 2, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(287, 73));
			_scene->_sequences.setScale(_globals._sequenceIndexes[3], 47);
		}
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		player.commands_allowed = true;
		break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case 70:
		player.walker_visible = true;
		player.clock = _scene->_animation[0]->getNextFrameTimer() - player.frame_delay;
		_scene->_sequences.addTimer(6, 71);
		break;

	case 71:
		if (!_globals[kHasTalkedToHermit]) {
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 26, 2, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(287, 73));
			_scene->_sequences.setScale(_globals._sequenceIndexes[3], 47);
		}
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void enterStore() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		if (local._videoDoorMode == 2)
			_scene->_sequences.addTimer(1, 4);
		else {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x305));
			_scene->_sequences.addTimer(120, 1);
		}
		break;

	case 1:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x306));
		_scene->_sequences.addTimer(60, 2);
		break;

	case 2:
		player.walker_visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], true, 11, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 2);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		break;

	case 3:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[5]);
		player.walker_visible = true;
		inter_move_object(OBJ_DOOR_KEY, 1);
		_scene->_sequences.addTimer(15, 4);
		break;

	case 4:
		player.walker_visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], true, 1);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.addTimer(15, 5);
		break;

	case 5:
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		player.clock = _scene->_frameStartTime - player.frame_delay;
		player.walker_visible = true;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 6);
		break;

	case 6:
		_scene->_hotspots.activate(words_video_store_door, false);
		if (local._videoDoorMode == 1) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 180, quote_string(kernel.quotes, 0x307));
		}
		player_walk(23, 90, FACING_WEST);
		_scene->_sequences.addTimer(180, 7);
		break;

	case 7:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
		break;

	case 8:
		_scene->_hotspots.activate(words_video_store_door, true);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_globals[kBeenInVideoStore] = true;
		player.commands_allowed = true;
		_scene->_nextSceneId = 610;
		break;

	default:
		break;
	}
}

static void room_609_pre_parser() {
	if (player_said_3(unlock, door_key, video_store_door))
		player_walk(78, 99, FACING_NORTHWEST);
}

static void room_609_parser() {
	if (player_said_2(walk_towards, alley))
		_scene->_nextSceneId = 611;
	else if (player_said_2(walk_through, video_store_door)) {
		if (!_globals[kBeenInVideoStore]) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x308));
				_scene->_sequences.addTimer(120, 1);
				break;

			case 1:
				player.walker_visible = false;
				_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], true, 1);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.addTimer(30, 2);
				break;

			case 2:
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				player.clock = _scene->_frameStartTime - player.frame_delay;
				player.walker_visible = true;
				_scene->_sequences.addTimer(60, 3);
				break;

			case 3:
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 0x309));
				_scene->_sequences.addTimer(120, 4);
				break;

			case 4:
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		} else {
			local._videoDoorMode = 2;
			enterStore();
		}
	} else if (player_said_3(unlock, door_key, video_store_door)) {
		local._videoDoorMode = 1;
		enterStore();
	} else if (player_said_2(get_inside, car)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
			_scene->_sequences.addTimer(6, 2);
		}
		break;

		case 2:
			player.walker_visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_nextSceneId = 504;
		}
		break;

		default:
			break;
		}
	} else if (_action._lookFlag)
		text_show(60910);
	else if (player_said_2(look, street))
		text_show(60911);
	else if (player_said_2(look, spot_a_pot))
		text_show(60912);
	else if (player_said_2(look, video_store))
		text_show(60913);
	else if (player_said_2(look, billboard))
		text_show(60914);
	else if (player_said_2(look, statue))
		text_show(60915);
	else if (player_said_2(look, car))
		text_show(60916);
	else if (player_said_2(look, newsstand))
		text_show(60917);
	else if (player_said_2(look, video_store_door)) {
		if (!_globals[kBeenInVideoStore])
			text_show(60918);
		else
			text_show(60919);
	} else if (player_said_2(walk_down, street))
		text_show(60730);
	else
		return;

	_action._inProgress = false;
}

void room_609_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._videoDoorMode);
}

void room_609_preload() {
	room_init_code_pointer = room_609_init;
	room_daemon_code_pointer = room_609_daemon;
	room_pre_parser_code_pointer = room_609_pre_parser;
	room_parser_code_pointer = room_609_parser;

	section_6_walker();
	section_6_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
