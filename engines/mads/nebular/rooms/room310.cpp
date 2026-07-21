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
#include "mads/nebular/rooms/forcefield.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	Forcefield _forcefield;
};

static Scratch local;


static void room_310_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC003x0");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*SC003x1");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*SC003x2");

	init_forcefield(&local._forcefield, true);

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(formAnimName('a', -1), 70);

	section_3_music();
}

static void room_310_daemon() {
	handle_forcefield(&local._forcefield, &_globals._spriteIndexes[0]);

	if (_game._trigger == 70)
		_scene->_nextSceneId = 309;
}

void room_310_synchronize(Common::Serializer &s) {
	local._forcefield.synchronize(s);
}

void room_310_preload() {
	local._forcefield.init();

	room_init_code_pointer = room_310_init;
	room_daemon_code_pointer = room_310_daemon;
	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
