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
#include "mads/nebular/rooms/section2.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _vultureFl;
	bool _spiderFl;
	bool _eyeFl;
	int16 _spiderHotspotId;
	int16 _vultureHotspotId;
	long _spiderTime;
	long _vultureTime;
};

static Scratch local;


static void room_207_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('h', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(kernel_name('h', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(kernel_name('c', -1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(kernel_name('e', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(kernel_name('e', 1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(kernel_name('g', 1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(kernel_name('g', 0));
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 7, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 7);

	int var2;
	if (!player.been_here_before) {
		var2 = 1;
	} else {
		var2 = g_engine->getRandomNumber(4) + 1;
	}

	if (var2 > 2)
		local._vultureFl = false;
	else
		local._vultureFl = true;

	local._spiderFl = (var2 & 1);

	if (local._vultureFl) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 30, 0, 0, 400);
		local._vultureHotspotId = _scene->_dynamicHotspots.add(words_vulture, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._vultureHotspotId, Common::Point(254, 94), FACING_WEST);
	}

	if (local._spiderFl) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -1);
		local._spiderHotspotId = _scene->_dynamicHotspots.add(words_spider, words_walkto, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._spiderHotspotId, Common::Point(59, 132), FACING_SOUTH);
	}

	local._eyeFl = false;
	if (_scene->_priorSceneId == 211) {
		player.x = 13;
		player.y = 105;
		player.facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 214) {
		player.x = 164;
		player.y = 117;
		player.facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 305;
		player.y = 131;
	}

	section_2_music();

	_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 1, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 22);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
}

static void moveVulture() {
	_scene->_sequences.remove(_globals._sequenceIndexes[1]);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
	g_engine->_soundManager->command(43, 0);
	local._vultureFl = false;
	local._vultureTime = player.clock;
	_scene->_dynamicHotspots.remove(local._vultureHotspotId);
}

static void moveSpider() {
	_scene->_sequences.remove(_globals._sequenceIndexes[4]);
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 5, 1, 0, 0);
	local._spiderFl = false;
	local._spiderTime = player.clock;
	_scene->_dynamicHotspots.remove(local._spiderHotspotId);
}

static void room_207_daemon() {

	if (local._vultureFl) {
		if (((int32)player.clock - local._vultureTime) > 1700)
			moveVulture();
	}

	if (local._spiderFl) {
		if (((int32)player.clock - local._spiderTime) > 800)
			moveSpider();
	}

	if (kernel.trigger == 70) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 23, 34);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
	}

	if (kernel.trigger == 71)
		local._eyeFl = false;

	if (local._eyeFl)
		return;

	if ((player.x >= 124) && (player.x <= 201)) {
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 10, 1, 0, 0);
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 6);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		local._eyeFl = true;
	}
}

static void room_207_pre_parser() {
	if (player_said_2(walk_down, path_to_west))
		player.walk_off_edge_to_room = 211;

	if (player_said_2(walk_towards, open_field_to_east))
		player.walk_off_edge_to_room = 208;

	if (player_said_1(walkto) || player_said_1(look)) {
		if (player_said_1(vulture)) {
			local._vultureTime = -9999;
		} else if (player_said_1(spider)) {
			local._spiderTime = -9999;
		}
	}
}

static void room_207_parser() {
	if (_action._savedFields._lookFlag)
		text_show(20711);
	else if (player_said_2(walk_through, doorway))
		_scene->_nextSceneId = 214;
	else {
		if ((player.x > 150) && (player.x < 189) &&
			(player.y > 111) && (player.y < 130)) {
			if ((player.x <= 162) || (player.x >= 181) ||
				(player.y <= 115) || (player.y >= 126)) {
				_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 10, 2, 0, 0);
				_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 2, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 6);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 6);
			}
		} else if (local._eyeFl) {
			_scene->_sequences.remove(_globals._sequenceIndexes[7]);
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			local._eyeFl = false;
		}

		if (player_said_2(look, dense_forest))
			text_show(20701);
		else if (player_said_2(look, hedge))
			text_show(20702);
		else if (player_said_2(look, skull_and_crossbones))
			text_show(20703);
		else if (player_said_2(look, cauldron))
			text_show(20704);
		else if (player_said_2(look, witchdoctor_hut))
			text_show(20705);
		else if (player_said_2(look, path_to_west))
			text_show(20706);
		else if (player_said_2(look, mountains))
			text_show(20707);
		else if (player_said_2(look, aloe_plant))
			text_show(20708);
		else if (player_said_2(look, lawn))
			text_show(20709);
		else if (player_said_2(look, vulture))
			text_show(20710);
		else if (player_said_2(take, skull_and_crossbones))
			text_show(20712);
		else if (player_said_2(take, aloe_plant))
			text_show(20713);
		else if (player_said_2(look, spider))
			text_show(20714);
		else if (player_said_2(take, spider))
			text_show(20715);
		else
			return;
	}

	_action._inProgress = false;
}

void room_207_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._vultureFl);
	s.syncAsByte(local._spiderFl);
	s.syncAsByte(local._eyeFl);

	s.syncAsSint32LE(local._spiderHotspotId);
	s.syncAsSint32LE(local._vultureHotspotId);
}

void room_207_preload() {
	local._spiderTime = player.clock;
	local._vultureTime = player.clock;

	room_init_code_pointer = room_207_init;
	room_pre_parser_code_pointer = room_207_pre_parser;
	room_parser_code_pointer = room_207_parser;
	room_daemon_code_pointer = room_207_daemon;

	section_2_walker();
	section_2_interface();
	_scene->addActiveVocab(words_vulture);
	_scene->addActiveVocab(words_walkto);
	_scene->addActiveVocab(words_spider);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
