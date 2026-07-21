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
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _oldFrame;
};

static Scratch local;


static void room_302_init() {
	_game._player._stepEnabled = false;
	_game._player._visible = false;

	_scene->loadAnimation(formAnimName('a', -1), 71);
	section_3_music();
}

static void room_302_daemon() {
	if (_game._trigger == 71)
		_scene->_nextSceneId = 303;

	if ((_scene->_animation[0] != nullptr) && (_scene->_animation[0]->getCurrentFrame() != local._oldFrame)) {
		local._oldFrame = _scene->_animation[0]->getCurrentFrame();
		if (local._oldFrame == 147) {
			_game._objects.setRoom(OBJ_POISON_DARTS, 1);
			_game._objects.setRoom(OBJ_BLOWGUN, 1);
			_game._objects.setRoom(OBJ_REBREATHER, 1);
			_game._objects.setRoom(OBJ_STUFFED_FISH, 1);
			_game._objects.setRoom(OBJ_DEAD_FISH, 1);
			_game._objects.setRoom(OBJ_BURGER, 1);

			int count = (int)_game._objects.size();
			for (int idx = 0; idx < count; idx++) {
				if (_game._objects.isInInventory(idx))
					_game._objects.setRoom(idx, 50);
			}
		}
	}
}

void room_302_synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(local._oldFrame);
}

void room_302_preload() {
	room_init_code_pointer = room_302_init;
	room_daemon_code_pointer = room_302_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
