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
#include "mads/nebular/rooms/section8.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_803_init() {
	_globals[kBetweenRooms] = false;
	player.walker_visible = false;
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('f', 1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites("*RXMBD_2");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(kernel_name('d', 1));

	kernel.quotes = quote_load(0x31B, 0x31C, 0x31D, 0x31E, 0x31F, 0x320, 0x321, 0x322, 0);

	if (_globals[kHoppyDead]) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(kernel_name('e', 1));
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
		int idx = _scene->_dynamicHotspots.add(words_guts, words_walkto, _globals._sequenceIndexes[7], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(66, 123), FACING_SOUTH);
	}

	if (!_globals[kBeamIsUp] && !_globals[kReturnFromCut] && (!_globals[kFromCockpit] || _globals[kExitShip])) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 2, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
	}

	if (!_globals[kFromCockpit]) {
		if (!_globals[kReturnFromCut]) {
			if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
				player.x = 15;
				player.y = 130;
				player.facing = FACING_EAST;
			}
			player.walker_visible = true;
		} else if (!_globals[kBeamIsUp]) {
			_globals._spriteIndexes[3] = _scene->_sprites.addSprites(kernel_name('a', 1));
			_globals._spriteIndexes[2] = _scene->_sprites.addSprites(kernel_name('a', 3));
			_globals._spriteIndexes[4] = _scene->_sprites.addSprites(kernel_name('a', 2));
			player.walker_visible = false;
			player.commands_allowed = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			g_engine->_soundManager->command(14, 0);
		}

		if (_globals[kBeamIsUp] && !_globals[kReturnFromCut]) {
			if (_globals[kForceBeamDown])
				player.walker_visible = false;
			else
				player.walker_visible = true;

			_globals._spriteIndexes[5] = _scene->_sprites.addSprites(kernel_name('b', 1));
			g_engine->_soundManager->command(15, 0);
			player.commands_allowed = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 100);
		}
	} else if (!_globals[kExitShip]) {
		if (!_globals[kBeamIsUp]) {
			_globals._spriteIndexes[3] = _scene->_sprites.addSprites(kernel_name('a', 1));
			player.walker_visible = false;
			player.commands_allowed = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 130);
			g_engine->_soundManager->command(14, 0);
		} else {
			_globals._spriteIndexes[8] = _scene->_sprites.addSprites(kernel_name('c', 1));
			player.walker_visible = false;
			player.commands_allowed = false;
			_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 1);
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 140);
		}
	} else {
		player.commands_allowed = false;
		player.x = 197;
		player.y = 96;
		player.facing = FACING_SOUTHWEST;
		player.walker_visible = true;
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(kernel_name('d', 1));
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 19);
		_scene->_sequences.addTimer(1, 150);
	}

	section_8_music();
}

static void room_803_daemon() {
	if (kernel.trigger == 120) {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 19);
		_scene->_nextSceneId = 804;
	}

	if (kernel.trigger == 100) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 2, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		if (!_globals[kHoppyDead]) {
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 7, 12);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 101);
		} else {
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
			int idx = _scene->_dynamicHotspots.add(words_guts, words_walkto, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(66, 123), FACING_SOUTH);
			g_engine->_soundManager->command(16, 0);
			_globals[kCameFromCut] = true;
			_globals[kBeamIsUp] = false;
			_globals[kReturnFromCut] = false;
			_globals[kDontRepeat] = false;
			_globals[kHoppyDead] = true;
			_globals[kHasWatchedAntigrav] = true;

			if (_globals[kForceBeamDown])
				_scene->_nextSceneId = _scene->_priorSceneId;
			else
				player.commands_allowed = true;
		}
	}

	if (kernel.trigger == 101) {
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
		int idx = _scene->_dynamicHotspots.add(words_guts, words_walkto, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(66, 123), FACING_SOUTH);
		g_engine->_soundManager->command(16, 0);
		_globals[kCameFromCut] = true;
		_globals[kBeamIsUp] = false;
		_globals[kReturnFromCut] = false;
		_globals[kDontRepeat] = false;
		_globals[kHoppyDead] = true;
		_globals[kHasWatchedAntigrav] = true;

		if (_globals[kForceBeamDown])
			_scene->_nextSceneId = _scene->_priorSceneId;
		else
			player.commands_allowed = true;
	}

	if (kernel.trigger == 80) {
		if (!_globals[kHoppyDead])
			_scene->_sequences.addTimer(350, 70);

		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
	}

	if (kernel.trigger == 70) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		g_engine->_soundManager->command(31, 0);
	}

	if (kernel.trigger == 71)
		_scene->_sequences.addTimer(200, 110);

	if (kernel.trigger == 90) {
		int syncIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 15, 0, 0, 0);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 4, 9);
		if (_globals[kHoppyDead])
			_scene->_sequences.addTimer(200, 110);
	}

	if (kernel.trigger == 110)
		_scene->_nextSceneId = 808;

	if (kernel.trigger == 130) {
		_globals[kBeamIsUp] = true;
		_scene->_nextSceneId = 804;
	}

	if (kernel.trigger == 140) {
		if (!_globals[kWindowFixed]) {
			_scene->_nextSceneId = 810;
			_globals[kInSpace] = true;
		} else {
			if (!_globals[kShieldModInstalled])
				win_status = 1;
			else if (!_globals[kTargetModInstalled])
				win_status = 2;
			else
				win_status = 3;

			game.going = false;
			return;
		}
	}

	if (kernel.trigger == 150) {
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		g_engine->_soundManager->command(18, 0);
		_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 19);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 151);
	}

	if (kernel.trigger == 151) {
		_globals[kBeamIsUp] = false;
		_globals[kFromCockpit] = false;
		_globals[kExitShip] = false;
		player.commands_allowed = true;
	}
}

static void room_803_pre_parser() {
	if (player_said_2(walk_down, path_to_west))
		player.walk_off_edge_to_room = 802;

	if (player_said_2(take, ship))
		player.need_to_walk = false;
}

static void room_803_parser() {
	if (player_said_2(take, guts)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], true, 6, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 1, 4);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[9]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 160);
			break;

		case 160:
		{
			int syncIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 4);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], syncIdx);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[9]);
			_scene->_sequences.addTimer(60, 161);
		}
		break;

		case 161:
		{
			int quoteId = 0x31A + g_engine->getRandomNumber(1, 8);
			_scene->_kernelMessages.add(Common::Point(64, 67), 0x1110, 32, 0, 80, quote_string(kernel.quotes, quoteId));
			_scene->_sequences.addTimer(60, 162);
		}
		break;

		case 162:
			_scene->_sequences.remove(_globals._sequenceIndexes[9]);
			_globals._sequenceIndexes[9] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], true, 6, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 1, 4);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[9]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 163);
			break;

		case 163:
			player.clock = _scene->_frameStartTime + player.frame_delay;
			player.walker_visible = true;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(enter, ship)) {
		g_engine->_soundManager->command(17, 0);
		player.commands_allowed = false;
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 19);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 120);
		_globals[kBeamIsUp] = false;
	} else if (player_said_2(look, launch_pad))
		text_show(80310);
	else if (_action._lookFlag)
		text_show(80310);
	else if (player_said_2(look, pad_to_west))
		text_show(80311);
	else if (player_said_2(look, guts)) {
		if (config_file.naughtiness == STORYMODE_NICE)
			text_show(80312);
		else
			text_show(80313);
	} else if (player_said_2(look, bushes))
		text_show(80315);
	else if (player_said_2(look, ship))
		text_show(80317);
	else if (player_said_2(look, tower))
		text_show(80318);
	else if (player_said_2(look, tree) || player_said_2(look, trees))
		text_show(80319);
	else if (player_said_2(look, sky))
		text_show(80320);
	else if (player_said_2(take, ship))
		text_show(80321);
	else
		return;

	_action._inProgress = false;
}

void room_803_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_803_preload() {
	room_init_code_pointer = room_803_init;
	room_daemon_code_pointer = room_803_daemon;
	room_pre_parser_code_pointer = room_803_pre_parser;
	room_parser_code_pointer = room_803_parser;

	section_8_walker();
	section_8_interface();

	_scene->addActiveVocab(words_guts);
	_scene->addActiveVocab(words_walkto);

	if ((!_globals[kFromCockpit] && _globals[kReturnFromCut] && !_globals[kBeamIsUp])
		|| (_globals[kFromCockpit] && !_globals[kExitShip])) {
		*player.series_name = '\0';
		player.walker_must_reload = true;
	}
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
