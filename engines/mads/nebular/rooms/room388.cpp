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

static void room_388_init() {
	_scene->_userInterface.setup(kInputLimitedSentences);

	if (_globals[kAfterHavoc])
		_scene->_hotspots.activate(words_sauropod, false);
	else {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('b', 0));
		_globals._sequenceIndexes[0] = _scene->_sequences.startCycle(_globals._spriteIndexes[0], false, 1);
	}

	_game._player._visible = false;
	_vm->_palette->setEntry(252, 63, 30, 20);
	_vm->_palette->setEntry(253, 45, 15, 12);
	_game.loadQuoteSet(0x154, 0x155, 0x156, 0x157, 0x158, 0);

	section_3_music();
}

static void room_388_parser() {
	if (player_said_2(return_to, air_shaft))
		_scene->_nextSceneId = 313;
	else if (player_said_2(talkto, sauropod)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(160, 136), 0x1110, 32, 1, 120, _game.getQuote(0x154));
			break;

		case 1:
			_scene->_kernelMessages.add(Common::Point(82, 38), 0xFDFC, 0, 0, 300, _game.getQuote(0x156));
			_scene->_kernelMessages.add(Common::Point(82, 52), 0xFDFC, 0, 0, 300, _game.getQuote(0x157));
			_scene->_kernelMessages.add(Common::Point(82, 66), 0xFDFC, 0, 2, 300, _game.getQuote(0x158));
			break;

		case 2:
			_game._player._stepEnabled = true;
			_scene->_kernelMessages.add(Common::Point(160, 136), 0x1110, 32, 0, 120, _game.getQuote(0x155));
			break;

		default:
			break;
		}
	} else if (player_said_2(look_through, grate)) {
		if (_globals[kAfterHavoc])
			_vm->_dialogs->show(38811);
		else
			_vm->_dialogs->show(38810);
	} else if (player_said_2(open, grate))
		_vm->_dialogs->show(38812);
	else
		return;

	_action._inProgress = false;
}

void room_388_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_388_preload() {
	room_init_code_pointer = room_388_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_388_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
