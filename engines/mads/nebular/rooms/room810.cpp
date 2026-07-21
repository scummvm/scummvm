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
#include "mads/nebular/rooms/section8.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _moveAllowed;
};

static Scratch local;


static void room_810_init() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(Resources::formatName(810, 'a', -1, EXT_AA, ""));
	local._moveAllowed = true;

	section_8_music();
}

static void room_810_daemon() {
	if (_scene->_animation[0] && (_scene->_animation[0]->getCurrentFrame() == 200)
		&& local._moveAllowed) {
		_scene->_sequences.addTimer(100, 70);
		local._moveAllowed = false;
	}

	if (_game._trigger == 70)
		_scene->_nextSceneId = 804;
}

void room_810_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._moveAllowed);
}

void room_810_preload() {
	room_init_code_pointer = room_810_init;
	room_daemon_code_pointer = room_810_daemon;

	section_8_walker();
	section_8_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
