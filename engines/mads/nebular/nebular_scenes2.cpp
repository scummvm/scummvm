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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
#include "mads/nebular/nebular_scenes2.h"

namespace MADS {

namespace Nebular {

void Scene2xx::setAAName() {
	int idx = (_scene->_nextSceneId == 216) ? 4 : 2;
	_game._aaName = Resources::formatAAName(idx);
}

void Scene2xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;

	switch(_scene->_nextSceneId) {
	case 213:
	case 216:
		_game._player._spritesPrefix = "";
		break;
	default:
		if (_globals[0] == SEX_MALE) {
			_game._player._spritesPrefix = "ROX";
		} else {
			_game._player._spritesPrefix = "RXM";
		}
		break;
	}

	if (_scene->_nextSceneId > 212)
		_game._player._unk4 = 0;
	else
		_game._player._unk4 = -1;

	if (oldName == _game._player._spritesPrefix)
		_game._player._spritesChanged = true;
	
	if ((_scene->_nextSceneId == 203 || _scene->_nextSceneId == 204) && _globals[34])
		_game._v3 = 0;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene201::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	Scene &scene = _vm->_game->_scene;
	scene.addActiveVocab(NOUN_15F);
	scene.addActiveVocab(NOUN_487);
	scene.addActiveVocab(NOUN_D);
}

void Scene201::enter() {
}

void Scene201::step() {
}

void Scene201::actions() {
}

} // End of namespace Nebular
} // End of namespace MADS
