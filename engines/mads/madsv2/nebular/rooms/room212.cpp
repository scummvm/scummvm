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
#include "mads/madsv2/nebular/rooms/section2.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_212_init() {
	if (_scene->_priorSceneId == 208) {
		_game._player._playerPos = Common::Point(195, 85);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(67, 117);
		_game._player._facing = FACING_NORTHEAST;
	}

	section_2_music();
}

static void room_212_daemon() {
	// No implementation
}

static void room_212_pre_parser() {
	if (_action.isAction(words_walk_through, words_cave_entrance))
		_game._player._walkOffScreenSceneId = 111;
}

static void room_212_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(21209);
	else if (_action.isAction(words_walk_towards) && (_action.isObject(words_field_to_north) || _action.isObject(words_mountains)))
		_scene->_nextSceneId = 208;
	else if (_action.isAction(words_walk_towards, words_cave))
		_scene->_nextSceneId = 111;
	else if (_action.isAction(words_look, words_grass))
		_vm->_dialogs->show(21201);
	else if (_action.isAction(words_look, words_rocks))
		_vm->_dialogs->show(21202);
	else if (_action.isAction(words_look, words_cave_entrance))
		_vm->_dialogs->show(21203);
	else if (_action.isAction(words_look, words_sky))
		_vm->_dialogs->show(21204);
	else if (_action.isAction(words_look, words_field_to_north))
		_vm->_dialogs->show(21205);
	else if (_action.isAction(words_look, words_trees))
		_vm->_dialogs->show(21206);
	else if (_action.isAction(words_look, words_plants))
		_vm->_dialogs->show(21207);
	else if (_action.isAction(words_look, words_mountains))
		_vm->_dialogs->show(21208);
	else
		return;

	_action._inProgress = false;
}

void room_212_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_212_preload() {
	room_init_code_pointer = room_212_init;
	room_pre_parser_code_pointer = room_212_pre_parser;
	room_parser_code_pointer = room_212_parser;
	room_daemon_code_pointer = room_212_daemon;

	section_2_walker();
	section_2_interface();
	_scene->addActiveVocab(words_bouncing_reptile);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
