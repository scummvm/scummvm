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
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_391_init() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;
	section_3_music();
}

static void room_391_parser() {
	if (player_said_2(return_to, air_shaft))
		_scene->_nextSceneId = 313;
	else if (player_said_2(open, grate)) {
		if (_globals[kKickedIn391Grate])
			_vm->_dialogs->show(39113);
		else {
			_vm->_dialogs->show(39112);
			_globals[kKickedIn391Grate] = true;
		}

		if (_globals[kAfterHavoc])
			_scene->_nextSceneId = 361;
		else
			_scene->_nextSceneId = 311;
	} else if (player_said_2(look_through, grate)) {
		if (_globals[kAfterHavoc])
			_vm->_dialogs->show(39111);
		else
			_vm->_dialogs->show(39110);
	} else
		return;

	_action._inProgress = false;
}

void room_391_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_391_preload() {
	room_init_code_pointer = room_391_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_391_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
