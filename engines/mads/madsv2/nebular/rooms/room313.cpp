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

#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section3.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_313_init() {
	_scene->_userInterface.setup(kInputLimitedSentences);

	if ((_scene->_priorSceneId == 366) || (_scene->_priorSceneId == 316)) {
		_game._player._playerPos = Common::Point(30, 80);
		_game._player._facing = FACING_NORTH;
	} else if ((_scene->_priorSceneId == 311) || (_scene->_priorSceneId == 361) || (_scene->_priorSceneId == 391)) {
		_game._player._playerPos = Common::Point(90, 70);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 390) {
		_game._player._playerPos = Common::Point(126, 70);
		_game._player._facing = FACING_EAST;
	} else if ((_scene->_priorSceneId == 389) || (_scene->_priorSceneId == 399)) {
		_game._player._playerPos = Common::Point(163, 70);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 388) {
		_game._player._playerPos = Common::Point(199, 70);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(234, 70);
		_game._player._facing = FACING_WEST;
	}

	if (_globals[kAfterHavoc]) {
		for (uint16 i = 0; i < cycle_list.num_cycles; i++) {
			int palIdx = cycle_list.table[i].first_list_color;
			int size = cycle_list.table[i].num_colors * 3;
			memset(&cycling_palette[palIdx], 0, size);
			memset(&master_palette[palIdx], 0, size);
		}
	}

	section_3_music();
}

static void room_313_parser() {
	if (_action.isAction(VERB_CRAWL_TO, NOUN_FOURTH_CELL))
		_scene->_nextSceneId = 387;
	else if (_action.isAction(VERB_CRAWL_TO, NOUN_THIRD_CELL))
		_scene->_nextSceneId = 388;
	else if (_action.isAction(VERB_CRAWL_TO, NOUN_SECOND_CELL)) {
		if (_globals[kAfterHavoc])
			_scene->_nextSceneId = 399;
		else
			_scene->_nextSceneId = 389;
	} else if (_action.isAction(VERB_CRAWL_TO, NOUN_FIRST_CELL))
		_scene->_nextSceneId = 390;
	else if (_action.isAction(VERB_CRAWL_TO, NOUN_SECURITY_STATION)) {
		if (_globals[kSexOfRex] == REX_FEMALE) {
			_globals[kSexOfRex] = REX_MALE;
			_vm->_dialogs->show(31301);
		}
		_scene->_nextSceneId = 391;
	} else if (_action.isAction(VERB_CRAWL_TO, NOUN_EQUIPMENT_ROOM)) {
		if (_globals[kSexOfRex] == REX_FEMALE) {
			_globals[kSexOfRex] = REX_MALE;
			_vm->_dialogs->show(31301);
		}
		_scene->_nextSceneId = 366;
	} else if (!_action.isAction(VERB_CRAWL_DOWN, NOUN_AIR_SHAFT))
		return;

	_action._inProgress = false;
}

void room_313_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_313_preload() {
	room_init_code_pointer = room_313_init;
	room_pre_parser_code_pointer = room_313_parser;
	section_3_walker();
	Common::strcpy_s(player.series_name, "RM313A");
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
