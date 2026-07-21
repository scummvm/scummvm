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

static void room_366_init() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;
	section_3_music();
}

static void room_366_parser() {
	if (player_said_2(return_to, air_shaft))
		_scene->_nextSceneId = 302;
	else if (player_said_2(open, grate)) {
		if (_game._visitedScenes.exists(316))
			_vm->_dialogs->show(36612);
		else
			_vm->_dialogs->show(36613);
		_scene->_nextSceneId = 316;
	} else if (player_said_2(look_through, grate)) {
		if (_game._visitedScenes.exists(321))
			_vm->_dialogs->show(36611);
		else
			_vm->_dialogs->show(36610);
	} else {
		return;
	}

	_action._inProgress = false;
}

void room_366_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_366_preload() {
	room_init_code_pointer = room_366_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_366_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
