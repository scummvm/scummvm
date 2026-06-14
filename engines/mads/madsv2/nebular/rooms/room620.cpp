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

#include "common/scummsys.h"
#include "math/utils.h"
#include "mads/madsv2/nebular/nebular.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

void Scene620::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

static void room_620_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
	_game._player._stepEnabled = false;
	_game._player._visible = false;
	_scene->_sequences.addTimer(30, 70);
	_scene->_userInterface.setup(kInputLimitedSentences);
	sceneEntrySound();
}

void Scene620::step() {
	switch (_game._trigger) {
	case 70:
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_scene->loadAnimation(formAnimName('E', -1), 71);
		break;

	case 71:
		if (_scene->_priorSceneId == 751) {
			_globals[kCityFlooded] = true;
			_globals[kTeleporterRoom + 5] = 0;
			_scene->_nextSceneId = 701;
		} else if (_scene->_priorSceneId == 752) {
			_globals[kCityFlooded] = true;
			_globals[kTeleporterRoom + 5] = 0;
			_scene->_nextSceneId = 702;
		} else if (_scene->_priorSceneId < 501 || _scene->_priorSceneId > 752) {
			_globals[kCityFlooded] = true;
			_globals[kTeleporterRoom + 5] = 0;
			_scene->_nextSceneId = _scene->_priorSceneId;
		} else if (_scene->_priorSceneId >= 501 && _scene->_priorSceneId <= 612) {
			_globals[kResurrectRoom] = _globals[kHoverCarLocation];
			_game._objects.addToInventory(OBJ_TIMEBOMB);
			_globals[kTimebombStatus] = 0;
			_globals[kTimebombTimer] = 0;
			_scene->_nextSceneId = 605;
		}
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
