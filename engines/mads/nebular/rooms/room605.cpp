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
#include "mads/nebular/rooms/section6.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_605_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('r', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('l', -1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('p', -1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('n', -1));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('f', -1));

	_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 15, 0, 0, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 17, 0, 0, 0);
	_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 14, 0, 0, 0);
	_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 13, 0, 0, 0);
	_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 17, 0, 0, 0);
	_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], false, 18, 0, 0, 0);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->_sequences.addTimer(600, 70);
	_scene->_userInterface.setup(kInputLimitedSentences);
	section_6_music();
	_vm->_sound->command(22);
}

static void room_605_daemon() {
	if (_game._trigger == 70) {
		_vm->_sound->command(23);
		if (_globals[kResurrectRoom] >= 700)
			_vm->_dialogs->show(60598);
		else
			_vm->_dialogs->show(60599);

		_scene->_nextSceneId = _globals[kResurrectRoom];
	}
}

void room_605_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_605_preload() {
	room_init_code_pointer = room_605_init;
	room_daemon_code_pointer = room_605_daemon;

	*player.series_name = '\0';
	section_6_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
