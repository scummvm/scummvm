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

#include "mads/madsv2/core/game.h"
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

static void room_387_init() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;

	section_3_music();
}

static void room_387_parser() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE))
		_vm->_dialogs->show(38710);
	else if (_action.isAction(VERB_OPEN, NOUN_GRATE))
		_vm->_dialogs->show(38711);
	else
		return;

	_action._inProgress = false;
}

void room_387_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_387_preload() {
	room_init_code_pointer = room_387_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_387_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
