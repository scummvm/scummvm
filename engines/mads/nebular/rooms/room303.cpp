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

static void room_303_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 1));

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 10, 0, 50, 120);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 10, 0, 0, 0);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(formAnimName('a', -1), 60);

	section_3_music();
}

static void room_303_daemon() {
	if (_game._trigger == 60)
		_scene->_nextSceneId = 304;
}

void room_303_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_303_preload() {
	room_init_code_pointer = room_303_init;
	room_daemon_code_pointer = room_303_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
