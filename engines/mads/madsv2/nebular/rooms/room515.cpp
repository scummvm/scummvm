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

void Scene515::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

static void room_515_init() {
	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->_sequences.addTimer(30, 70);

	sceneEntrySound();
}

void Scene515::step() {
	if (_game._trigger == 70)
		_scene->loadAnimation(formAnimName('A', -1), 71);
	else if (_game._trigger == 71)
		_scene->_nextSceneId = 508;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
