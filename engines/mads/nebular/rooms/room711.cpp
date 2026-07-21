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
#include "mads/nebular/rooms/section7.h"
#include "mads/nebular/rooms/teleporter.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_711_init() {
	if (_globals[kSexOfRex] == REX_FEMALE)
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*ROXHAND");
	else
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*REXHAND");

	teleporter_init();

	// The original was using Scene7xx_section_7_music()
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else
		_vm->_sound->command(25);
}

static void room_711_daemon() {
	teleporter_daemon();
}

static void room_711_parser() {
	if (teleporter_parser())
		_action._inProgress = false;
}

void room_711_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_711_preload() {
	room_init_code_pointer = room_711_init;
	room_daemon_code_pointer = room_711_daemon;
	room_parser_code_pointer = room_711_parser;

	section_7_walker();
	section_7_interface();
	*player.series_name = '\0';
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
