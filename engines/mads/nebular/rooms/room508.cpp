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

#include "math/utils.h"
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section5.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _chosenObject;
};

static Scratch local;


static void room_508_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('a', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(kernel_name('m', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(kernel_name('h', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(kernel_name('l', 2));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(kernel_name('t', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(kernel_name('l', 3));

	if (!player.been_here_before) {
		_globals[kLaserOn] = false;
		local._chosenObject = 0;
	}

	if (!_globals[kLaserOn]) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
		int idx = _scene->_dynamicHotspots.add(words_spinach_patch_doll, words_walkto, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(57, 116), FACING_NORTHEAST);
		_scene->_hotspots.activate(words_hole, false);
		_scene->_hotspots.activate(words_laser_beam, false);
	} else {
		_scene->changeVariant(1);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 11);
		int idx = _scene->_dynamicHotspots.add(words_laser_beam, words_walkto, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(57, 116), FACING_NORTHEAST);
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 6, 8);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 6);
		if (_globals[kLaserHoleIsThere]) {
			_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, -2);
			_scene->_hotspots.activate(words_hole, true);
			_scene->_hotspots.activate(words_laser_beam, true);
		}
		g_engine->_soundManager->command(21, 0);
	}
	g_engine->_soundManager->command(20, 0);

	if (_scene->_priorSceneId == 515) {
		player.x = 57;
		player.y = 116;
		player.facing = FACING_NORTHEAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 289;
		player.y = 139;
		player.facing = FACING_WEST;
	}

	section_5_music();
	kernel.quotes = quote_load(0x273, 0);

	if (_scene->_roomChanged) {
		inter_give_to_player(OBJ_COMPACT_CASE);
		inter_give_to_player(OBJ_REARVIEW_MIRROR);
	}
}

static void room_508_pre_parser() {
	if (player_said_2(walk, outside))
		player.walk_off_edge_to_room = 506;
}

static void handlePedestral() {
	if (!_globals[kLaserOn])
		text_show(50835);

	if (_globals[kLaserHoleIsThere])
		text_show(50836);

	if (_globals[kLaserOn] && !_globals[kLaserHoleIsThere]) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], false, 9, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 4);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			if (local._chosenObject == 2)
				inter_take_from_player(OBJ_COMPACT_CASE, 1);
			else
				inter_take_from_player(OBJ_REARVIEW_MIRROR, 1);

			_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, -2);
			_scene->_hotspots.activate(words_hole, true);
			_scene->_hotspots.activate(words_laser_beam, true);
			break;

		case 3:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[6]);
			player.walker_visible = true;
			_scene->_sequences.addTimer(120, 4);
			break;

		case 4:
			text_show(50834);
			_globals[kLaserHoleIsThere] = true;
			_scene->_nextSceneId = 515;
			break;

		default:
			break;
		}
	}
}

static void room_508_parser() {
	if (player_said_2(pull, lever)) {
		if (!_globals[kLaserOn]) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 2, 120, quote_string(kernel.quotes, 0x273));
				break;

			case 2:
				player.walker_visible = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
				_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 10, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 7);
				_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], -1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				g_engine->_soundManager->command(19, 0);
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 6);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[1]);
				player.walker_visible = true;
				_scene->_sequences.addTimer(15, 5);
				break;

			case 4:
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 6, 8);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 6);
				break;

			case 5:
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_scene->loadAnimation(kernel_name('B', 1), 6);
				break;

			case 6:
			{
				g_engine->_soundManager->command(22, 0);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 11);
				int idx = _scene->_dynamicHotspots.add(words_laser_beam, words_walkto, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(57, 116), FACING_NORTHEAST);
				_scene->_kernelMessages.reset();
				_scene->changeVariant(1);
				_scene->_sequences.addTimer(30, 7);
			}
			break;

			case 7:
				_globals[kLaserOn] = true;
				text_show(50833);
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		} else {
			text_show(50837);
		}
	} else if (player_said_3(reflect, rearview_mirror, laser_beam) || player_said_3(put, rearview_mirror, pedestal) || player_said_3(put, rearview_mirror, laser_beam)) {
		local._chosenObject = 1;
		handlePedestral();
	} else if (player_said_3(put, compact_case, pedestal) || player_said_3(put, compact_case, laser_beam) || player_said_3(reflect, compact_case, laser_beam)) {
		local._chosenObject = 2;
		handlePedestral();
	} else if (_action._lookFlag)
		text_show(50822);
	else if (player_said_2(look, target_area))
		text_show(50810);
	else if (player_said_2(look, spinach_patch_doll))
		text_show(50811);
	else if (player_said_2(take, spinach_patch_doll))
		text_show(50812);
	else if (player_said_2(look, sand_bags))
		text_show(50816);
	else if (player_said_2(take, sand_bags))
		text_show(50817);
	else if (player_said_2(look, control_station))
		text_show(50818);
	else if (player_said_2(look, monitor)) {
		if (_globals[kLaserOn])
			text_show(50820);
		else
			text_show(50819);
	} else if (player_said_2(look, laser_cannon)) {
		if (_globals[kLaserOn])
			text_show(50822);
		else
			text_show(50821);
	} else if (player_said_2(take, laser_cannon))
		text_show(50823);
	else if (player_said_2(look, lever)) {
		if (_globals[kLaserOn])
			text_show(50825);
		else
			text_show(50824);
	} else if (player_said_2(push, lever))
		text_show(50826);
	else if (player_said_2(look, laser_beam)) {
		if (_globals[kLaserHoleIsThere])
			text_show(50828);
		else
			text_show(50827);
	} else if (player_said_2(take, laser_beam))
		text_show(50829);
	else if (player_said_2(look, ceiling)) {
		if (_globals[kLaserHoleIsThere])
			text_show(50831);
		else
			text_show(50830);
	} else if (player_said_2(look, wall))
		text_show(50832);
	else if (player_said_2(look, pedestal)) {
		if (!_globals[kLaserOn])
			text_show(50813);
		else if (!_globals[kLaserHoleIsThere])
			text_show(50814);
		else
			text_show(50815);
	} else
		return;

	_action._inProgress = false;
}

void room_508_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._chosenObject);
}

void room_508_preload() {
	room_init_code_pointer = room_508_init;
	room_pre_parser_code_pointer = room_508_pre_parser;
	room_parser_code_pointer = room_508_parser;

	section_5_walker();
	section_5_interface();
	_scene->addActiveVocab(words_spinach_patch_doll);
	_scene->addActiveVocab(words_walkto);
	_scene->addActiveVocab(words_laser_beam);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
