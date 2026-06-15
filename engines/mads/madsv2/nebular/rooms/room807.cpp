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
#include "mads/madsv2/nebular/rooms/section8.h"
#include "mads/madsv2/nebular/rooms/teleporter.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_807_init() {
	if (_globals[kSexOfRex] == REX_FEMALE)
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*ROXHAND");
	else
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*REXHAND");

	teleporter_init();

	// The original uses Scene8xx::section_8_music()
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else
		_vm->_sound->command(20);
}

static void room_807_daemon() {
	teleporter_daemon();
}

static void room_807_parser() {
	if (teleporter_parser()) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_VIEWPORT))
		_vm->_dialogs->show(80710);
	else if (_action.isAction(VERB_PEER_THROUGH, NOUN_VIEWPORT))
		_vm->_dialogs->show(80710);
	else if (_action.isAction(VERB_LOOK, NOUN_KEYPAD) && _action.isAction(VERB_INSPECT, NOUN_KEYPAD))
		_vm->_dialogs->show(80711);
	else if (_action.isAction(VERB_LOOK, NOUN_DISPLAY))
		_vm->_dialogs->show(80712);
	else if (_action.isAction(VERB_LOOK, NOUN_1_KEY) || _action.isAction(VERB_LOOK, NOUN_2_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_3_KEY) || _action.isAction(VERB_LOOK, NOUN_4_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_5_KEY) || _action.isAction(VERB_LOOK, NOUN_6_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_7_KEY) || _action.isAction(VERB_LOOK, NOUN_8_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_9_KEY) || _action.isAction(VERB_LOOK, NOUN_0_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_SMILE_KEY) || _action.isAction(VERB_LOOK, NOUN_FROWN_KEY))
		_vm->_dialogs->show(80713);
	else if (_action.isAction(VERB_LOOK, NOUN_DEVICE) && _action._lookFlag)
		_vm->_dialogs->show(80714);
	else
		return;

	_action._inProgress = false;
}

void room_807_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_807_preload() {
	room_init_code_pointer = room_807_init;
	room_daemon_code_pointer = room_807_daemon;
	room_parser_code_pointer = room_807_parser;

	section_8_walker();
	section_8_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
