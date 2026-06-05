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
#include "mads/nebular/nebular.h"
#include "mads/nebular/core/scene.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes8.h"

namespace MADS {

namespace Nebular {

void Scene8xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	if ((_globals[kFromCockpit] && !_globals[kExitShip]) ||
			_scene->_nextSceneId == 804 || _scene->_nextSceneId == 805 ||
			_scene->_nextSceneId == 808 || _scene->_nextSceneId == 810) {
		_game._player._spritesPrefix = "";
	} else
		_game._player._spritesPrefix = _globals[kSexOfRex] == SEX_FEMALE ? "ROX" : "RXM";

	_vm->_palette->setEntry(16, 0x0A, 0x3F, 0x3F);
	_vm->_palette->setEntry(17, 0x0A, 0x2D, 0x2D);
}

void Scene8xx::setAAName() {
	_game._aaName = Resources::formatAAName(5);
}

void Scene8xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else {
		switch (_scene->_nextSceneId) {
		case 801:
		case 802:
		case 803:
		case 804:
		case 806:
		case 807:
		case 808:
			_vm->_sound->command(20);
			break;

		case 805:
			_vm->_sound->command(23);
			break;

		case 810:
			_vm->_sound->command(10);
			break;

		default:
			break;
		}
	}
}

} // namespace Nebular
} // namespace MADS
