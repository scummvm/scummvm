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
#include "mads/madsv2/nebular/rooms/teleporter.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_213_init() {
	if (_globals[kMeteorologistWatch] != METEOROLOGIST_NORMAL)
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*METHAND");
	else if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*REXHAND");
	else
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*ROXHAND");

	teleporter_init();

	// The original is calling Scene2xx::section_2_music()
	if (_vm->_musicFlag) {
		if (_globals[kMeteorologistWatch] == METEOROLOGIST_NORMAL)
			_vm->_sound->command(1);
		else
			_vm->_sound->command(9);
	} else {
		_vm->_sound->command(2);
	}
}

static void room_213_daemon() {
	teleporter_daemon();
}

static void room_213_parser() {
	if (teleporter_parser()) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(words_look, words_control_panel))
		_vm->_dialogs->show(21301);
	else if (_action.isAction(words_look, words_keypad) || _action.isAction(words_inspect, words_keypad))
		_vm->_dialogs->show(21302);
	else if (_action.isAction(words_look, words_display))
		_vm->_dialogs->show(21303);
	else if (_action.isAction(words_look, words_viewport) || _action.isAction(words_peer_through, words_viewport))
		_vm->_dialogs->show(21304);
	else if (_action.isAction(words_look, words_device))
		_vm->_dialogs->show(21305);
	else if (_action.isAction(words_look, words_0_key) || _action.isAction(words_look, words_1_key)
		|| _action.isAction(words_look, words_2_key) || _action.isAction(words_look, words_3_key)
		|| _action.isAction(words_look, words_4_key) || _action.isAction(words_look, words_5_key)
		|| _action.isAction(words_look, words_6_key) || _action.isAction(words_look, words_7_key)
		|| _action.isAction(words_look, words_8_key) || _action.isAction(words_look, words_9_key)
		|| _action.isAction(words_look, words_smile_key) || _action.isAction(words_look, words_frown_key)
		|| _action.isAction(words_look, words_enter_key))
		_vm->_dialogs->show(21306);
	else
		return;

	_action._inProgress = false;
}

void room_213_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_213_preload() {
	room_init_code_pointer = room_213_init;
	room_parser_code_pointer = room_213_parser;
	room_daemon_code_pointer = room_213_daemon;

	Common::strcpy_s(player.series_name, "");
	section_2_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
