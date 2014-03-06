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
#include "mads/nebular/nebular_scenes8.h"

namespace MADS {

namespace Nebular {

void Scene8xx::setup1() {
	_vm->_sound->command(5);
	if ((_vm->_game->_globalFlags[178] && !_vm->_game->_globalFlags[179]) ||
			_scene->_nextSceneId == 804 || _scene->_nextSceneId == 805 ||
			_scene->_nextSceneId == 808 || _scene->_nextSceneId == 810) {
		_vm->_game->_player._spritesPrefix = "";
	} else {
		_vm->_game->_player._spritesPrefix = _vm->_game->_globalFlags[0] 
			== SEX_FEMALE ? "ROX" : "RXM";
	}

	_vm->_palette->setEntry(16, 0x0A, 0x3F, 0x3F);
	_vm->_palette->setEntry(17, 0x0A, 0x2D, 0x2D);
}

void Scene8xx::setup2() {
	_vm->_game->_aaName = Resources::formatAAName(5);
}

/*------------------------------------------------------------------------*/

void Scene804::setup() {
}

void Scene804::enter() {
}

void Scene804::step() {
}

void Scene804::preActions() {
}

void Scene804::actions() {
}

void Scene804::postActions() {
}

} // End of namespace Nebular

} // End of namespace MADS
