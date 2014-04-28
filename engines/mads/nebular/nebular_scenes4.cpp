/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes4.h"

namespace MADS {

namespace Nebular {

void Scene4xx::setAAName() {
	_game._aaName = Resources::formatAAName(4);
}

void Scene4xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;

	if ((_scene->_nextSceneId == 403) || (_scene->_nextSceneId == 409))
		_game._player._spritesPrefix = "";
	else if (_globals[kSexOfRex] == REX_FEMALE)
		_game._player._spritesPrefix = "ROX";
	else
		_game._player._spritesPrefix = "RXM";

	_game._player._scalingVelocity = true;

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene4xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 401:
		_vm->_sound->startQueuedCommands();
		if (_scene->_priorSceneId == 402)
			_vm->_sound->command(12, 64);
		else
			_vm->_sound->command(12, 1);
		break;

	case 402:
		_vm->_sound->startQueuedCommands();
		_vm->_sound->command(12, 127);
		break;

	case 405:
	case 407:
	case 409:
	case 410:
	case 413:
		_vm->_sound->command(10);
		break;

	case 408:
		_vm->_sound->command(52);
		break;

	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
