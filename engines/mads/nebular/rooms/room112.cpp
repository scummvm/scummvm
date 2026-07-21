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
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_112_init() {
	section_1_music();

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('X', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('X', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('X', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('X', 5));

	_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 10, 0, 17, 20);
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 0, 0, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 3, 0);

	_game._player._stepEnabled = false;
	_game._player._visible = false;

	_scene->_userInterface.emptyConversationList();
	_scene->_userInterface.setup(kInputConversation);

	_scene->loadAnimation(Resources::formatName(112, 'X', -1, EXT_AA, ""), 70);
}

static void room_112_daemon() {
	if ((_scene->_animation[0] != nullptr) && (_game._storyMode == STORYMODE_NICE)) {
		if (_scene->_animation[0]->getCurrentFrame() >= 54) {
			_scene->freeAnimation();
			_game._trigger = 70;
		}
	}

	if (_game._trigger == 70) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 3, 0, 11);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
	}

	if (_game._trigger == 71) {
		_scene->_nextSceneId = 101;
		_game._player._stepEnabled = true;
		_game._player._visible = true;
	}
}

void room_112_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_112_preload() {
	room_init_code_pointer = room_112_init;
	room_daemon_code_pointer = room_112_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
