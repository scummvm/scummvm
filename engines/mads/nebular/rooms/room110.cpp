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
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _crabsFl;
};

static Scratch local;


static void room_110_init() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(kernel_name('X', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('X', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(kernel_name('X', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(kernel_name('X', 3));

	local._crabsFl = false;

	if (_scene->_priorSceneId == 109) {
		player.x = 59;
		player.y = 71;
		player.facing = FACING_EAST;

		_globals._sequenceIndexes[0] = _scene->_sequences.startCycle(_globals._spriteIndexes[0], false, 1);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);

		local._crabsFl = true;

		int idx = _scene->_dynamicHotspots.add(words_crab, words_swim_to, _globals._sequenceIndexes[0], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(words_crab, words_swim_to, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(words_crab, words_swim_to, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(words_crab, words_swim_to, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 194;
		player.y = 23;
		player.facing = FACING_SOUTH;
		player.walker_visible = false;
		player.commands_allowed = false;
		_scene->loadAnimation(kernel_full_name(110, 'T', 1, "", EXT_AA), 70);
	}

	section_1_music();
	kernel.quotes = quote_load(89, 0);

	if (!player.been_here_before && (_scene->_priorSceneId == 109))
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 89));
}

static void room_110_daemon() {
	if (kernel.trigger == 70) {
		player.walker_visible = true;
		player.commands_allowed = true;
	}
}

static void room_110_pre_parser() {
	if (player_said_2(swim_through, cave_entrance))
		player.walk_off_edge_to_room = 109;

	if (local._crabsFl) {
		local._crabsFl = false;

		_scene->_sequences.remove(_globals._sequenceIndexes[0]);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);

		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 16, 1, 0, 0);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 16, 1, 0, 0);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 16, 1, 0, 0);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 16, 1, 0, 0);

		int idx = _scene->_dynamicHotspots.add(words_crab, words_swim_to, _globals._sequenceIndexes[0], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(words_crab, words_swim_to, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(words_crab, words_swim_to, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(words_crab, words_swim_to, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
	}
}

static void room_110_parser() {
	if (player_said_2(swim_through, tunnel)) {
		switch (kernel.trigger) {
		case 0:
			_scene->loadAnimation(kernel_full_name(110, 'T', 0, "", EXT_AA), 1);
			_scene->_animation[0]->setNextFrameTimer(player.frame_delay + player.clock);
			player.commands_allowed = false;
			player.walker_visible = false;
			break;
		case 1:
			player.walker_visible = true;
			player.commands_allowed = true;
			_scene->_nextSceneId = 111;
			break;
		default:
			break;
		}
	} else if ((_action._lookFlag) || player_said_2(look, cave))
		text_show(11001);
	else if (player_said_2(look, cave_ceiling) || player_said_2(look_at, cave_ceiling))
		text_show(11002);
	else if (player_said_2(look, rocks))
		text_show(11003);
	else if (player_said_2(take, rocks))
		text_show(11004);
	else if (player_said_2(look, tunnel))
		text_show(11005);
	else if (player_said_2(look, cave_entrance))
		text_show(11006);
	else if (player_said_2(look, fungoids))
		text_show(11007);
	else if (player_said_2(take, fungoids))
		text_show(11008);
	else
		return;

	_action._inProgress = false;
}

void room_110_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._crabsFl);
}

void room_110_preload() {
	room_init_code_pointer = room_110_init;
	room_pre_parser_code_pointer = room_110_pre_parser;
	room_parser_code_pointer = room_110_parser;
	room_daemon_code_pointer = room_110_daemon;

	section_1_walker();
	section_1_interface();
	_scene->addActiveVocab(words_crab);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
