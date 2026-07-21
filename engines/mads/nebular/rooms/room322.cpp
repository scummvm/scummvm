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
#include "mads/nebular/rooms/teleporter.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_322_init() {
	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*REXHAND");
	else
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*ROXHAND");

	teleporter_init();

	// The original is using scene3xx_section_3_music()
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else
		_vm->_sound->command(10);
}

static void room_322_daemon() {
	teleporter_daemon();
}

static void room_322_parser() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(32214);
		_action._inProgress = false;
		return;
	}

	if (teleporter_parser()) {
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, viewport) || player_said_2(peer_through, viewport))
		_vm->_dialogs->show(32210);
	else if (player_said_2(look, keypad))
		_vm->_dialogs->show(32211);
	else if (player_said_2(look, display))
		_vm->_dialogs->show(32212);
	else if (player_said_2(look, 0_key) || player_said_2(look, 1_key)
		|| player_said_2(look, 2_key) || player_said_2(look, 3_key)
		|| player_said_2(look, 4_key) || player_said_2(look, 5_key)
		|| player_said_2(look, 6_key) || player_said_2(look, 7_key)
		|| player_said_2(look, 8_key) || player_said_2(look, 9_key)
		|| player_said_2(look, smile_key) || player_said_2(look, enter_key)
		|| player_said_2(look, frown_key))
		_vm->_dialogs->show(32213);
	else if (player_said_2(look, device))
		_vm->_dialogs->show(32214);
	else
		return;

	_action._inProgress = false;
}

void room_322_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_322_preload() {
	room_init_code_pointer = room_322_init;
	room_parser_code_pointer = room_322_parser;
	room_daemon_code_pointer = room_322_daemon;

	*player.series_name = '\0';
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
