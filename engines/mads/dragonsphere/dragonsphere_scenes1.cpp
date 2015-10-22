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
#include "mads/dragonsphere/dragonsphere_scenes.h"
#include "mads/dragonsphere/dragonsphere_scenes1.h"

namespace MADS {

namespace Dragonsphere {

void Scene1xx::setAAName() {
	int interface;

	switch (_scene->_nextSceneId) {
	case 108:
	case 109:
		interface = 3;
		break;
	case 110:
		interface = 5;
		break;
	case 113:
	case 114:
	case 115:
	case 117:
	case 119:
		interface = 1;
		break;
	case 116:
		interface = 2;
		break;
	case 120:
		interface = 8;
		break;
	default:
		interface = 0;
		break;
	}

	_game._aaName = Resources::formatAAName(interface);
	_vm->_palette->setEntry(254, 56, 47, 32);

}

void Scene1xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		return;

	switch (_scene->_nextSceneId) {
	case 104:
		if (_globals[kPlayerPersona] == 1)
			_vm->_sound->command(44);
		else
			_vm->_sound->command(16);
		break;

	case 106:
		if (_globals[kEndOfGame])
			_vm->_sound->command(47);
		else
			_vm->_sound->command(16);
		break;

	case 108:
		if (_game._visitedScenes.exists(109))
			_vm->_sound->command(32);
		else
			_vm->_sound->command(33);
		break;

	case 109:
		_vm->_sound->command(32);
		break;

	case 110:
		_vm->_sound->command(37);
		break;

	case 111:
		_vm->_sound->command(34);
		break;

	case 112:
		_vm->_sound->command(38);
		break;

	case 113:
		_vm->_sound->command(5);
		if (_globals[kPlayerIsSeal])
			_vm->_sound->command(35);
		else
			_vm->_sound->command(36);
		break;

	case 114:
		_vm->_sound->command(36);
		break;

	case 115:
		_vm->_sound->command(39);
		break;

	case 116:
		_vm->_sound->command(40);
		break;

	case 117:
		_vm->_sound->command(35);
		break;

	case 119:
		_vm->_sound->command(41);
		break;

	case 120:
		_vm->_sound->command(46);
		break;

	default:
		_vm->_sound->command(16);
		break;
	}
}

void Scene1xx::setPlayerSpritesPrefix() {
	int darkSceneFl = false;
	int noPlayerFl = false;

	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;

	_globals[kPerformDisplacements] = true;

	switch (_scene->_nextSceneId) {
	case 106:
		if (_scene->_currentSceneId == 120)
			noPlayerFl = true;
		break;

	case 108:
	case 109:
	case 114:
	case 115:
		darkSceneFl = true;
		break;

	case 111:
	case 112:
	case 117:
	case 120:
	case 119:
		noPlayerFl = true;
		break;

	case 113:
		if (!_globals[kPlayerPersona])
			noPlayerFl = true;
		darkSceneFl = true;
		break;
	}

	if (noPlayerFl || _globals[kNoLoadWalker]) {
		_game._player._spritesPrefix = "";
	} else if (!_game._player._forcePrefix) {
		if (!_globals[kPlayerPersona] || _scene->_nextSceneId == 108 || _scene->_nextSceneId == 109) {
			if (_scene->_nextSceneId == 113 || _scene->_nextSceneId == 114 || _scene->_nextSceneId == 115 || _scene->_nextSceneId == 116)
				_game._player._spritesPrefix = "PD";
			else
				_game._player._spritesPrefix = "KG";
		} else
			_game._player._spritesPrefix = "PD";

		if (darkSceneFl)
			_game._player._spritesPrefix += "D";
	}

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_game._player._scalingVelocity = true;
}

/*------------------------------------------------------------------------*/

} // End of namespace Dragonsphere
} // End of namespace MADS
