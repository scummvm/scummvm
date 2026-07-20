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
#include "mads/madsv2/nebular/rooms/section4.h"
#include "mads/madsv2/nebular/rooms/teleporter.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_409_init() {
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*ROXHAND");
	teleporter_init();
	section_4_music();
}

static void room_409_daemon() {
	teleporter_daemon();
}

static void room_409_parser() {
	if (teleporter_parser()) {
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, viewport))
		_vm->_dialogs->show(40910);
	else if (player_said_2(peer_through, viewport))
		_vm->_dialogs->show(40910);
	else if (player_said_2(look, keypad))
		_vm->_dialogs->show(40911);
	else if (player_said_2(inspect, keypad))
		_vm->_dialogs->show(40911);
	else if (player_said_2(look, display))
		_vm->_dialogs->show(40912);
	else if (player_said_2(look, 1_key) || player_said_2(look, 2_key)
		|| player_said_2(look, 3_key) || player_said_2(look, 4_key)
		|| player_said_2(look, 5_key) || player_said_2(look, 6_key)
		|| player_said_2(look, 7_key) || player_said_2(look, 8_key)
		|| player_said_2(look, 9_key) || player_said_2(look, 0_key)
		|| player_said_2(look, smile_key) || player_said_2(look, frown_key))
		_vm->_dialogs->show(40913);
	else if (player_said_2(look, device))
		_vm->_dialogs->show(40914);
	else if (_action._lookFlag)
		_vm->_dialogs->show(40914);
	else
		return;

	_action._inProgress = false;
}

void room_409_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_409_preload() {
	room_init_code_pointer = room_409_init;
	room_parser_code_pointer = room_409_parser;
	room_daemon_code_pointer = room_409_daemon;

	*player.series_name = '\0';
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
