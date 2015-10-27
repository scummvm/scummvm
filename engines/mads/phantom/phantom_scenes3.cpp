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
#include "mads/conversations.h"
#include "mads/scene.h"
#include "mads/phantom/phantom_scenes.h"
#include "mads/phantom/phantom_scenes3.h"

namespace MADS {

namespace Phantom {

void Scene3xx::setAAName() {
	_game._aaName = Resources::formatAAName(_globals[kTempInterface]);
	_vm->_palette->setEntry(254, 43, 47, 51);
}

void Scene3xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		return;

	switch (_scene->_nextSceneId) {
	case 303:
	case 304:
	case 305:
	case 307:
	case 308:
		if (_globals[kKnockedOverHead])
			_vm->_sound->command(33);
		else
			_vm->_sound->command(16);
		break;

	case 310:
	case 320:
	case 330:
	case 340:
		_vm->_sound->command(36);
		break;

	default:
		if (_scene->_nextSceneId != 306)
			_vm->_sound->command(16);
		break;
	}
}

void Scene3xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	if ((_scene->_nextSceneId == 304) || (_scene->_nextSceneId == 305) || (_scene->_nextSceneId == 306) || (_scene->_nextSceneId == 310))
		_game._player._spritesPrefix = "";
	else {
		Common::String oldName = _game._player._spritesPrefix;
		if (!_game._player._forcePrefix)
			_game._player._spritesPrefix = "RAL";
		if (oldName != _game._player._spritesPrefix)
			_game._player._spritesChanged = true;
	}

	_game._player._scalingVelocity = true;
}

/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
