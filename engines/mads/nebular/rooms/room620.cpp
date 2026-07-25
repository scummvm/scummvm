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
namespace RexNebular {
namespace Rooms {

static void room_620_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('b', 0), 0);
	g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, -1);
	player.commands_allowed = false;
	player.walker_visible = false;
	_scene->_sequences.addTimer(30, 70);
	_scene->_userInterface.setup(kInputLimitedSentences);
	section_6_music();
}

static void room_620_daemon() {
	switch (kernel.trigger) {
	case 70:
		_scene->_sequences.remove(g_sequence_ids[1]);
		_scene->loadAnimation(kernel_name('E', -1), 71);
		break;

	case 71:
		if (_scene->_priorSceneId == 751) {
			global[kCityFlooded] = true;
			global[kTeleporterRoom + 5] = 0;
			_scene->_nextSceneId = 701;
		} else if (_scene->_priorSceneId == 752) {
			global[kCityFlooded] = true;
			global[kTeleporterRoom + 5] = 0;
			_scene->_nextSceneId = 702;
		} else if (_scene->_priorSceneId < 501 || _scene->_priorSceneId > 752) {
			global[kCityFlooded] = true;
			global[kTeleporterRoom + 5] = 0;
			_scene->_nextSceneId = _scene->_priorSceneId;
		} else if (_scene->_priorSceneId >= 501 && _scene->_priorSceneId <= 612) {
			global[kResurrectRoom] = global[kHoverCarLocation];
			inter_give_to_player(OBJ_TIMEBOMB);
			global[kTimebombStatus] = 0;
			global[kTimebombTimer] = 0;
			_scene->_nextSceneId = 605;
		}
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

void room_620_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_620_preload() {
	room_init_code_pointer = room_620_init;
	room_daemon_code_pointer = room_620_daemon;

	*player.series_name = '\0';
	section_6_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
