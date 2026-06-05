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

Scene310::Scene310(RexNebularEngine *vm) : Scene3xx(vm) {
	_forceField.init();
}

void Scene310::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	_forceField.synchronize(s);
}

void Scene310::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene310::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC003x0");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*SC003x1");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*SC003x2");

	initForceField(&_forceField, true);

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(formAnimName('a', -1), 70);

	sceneEntrySound();
}

void Scene310::step() {
	handleForceField(&_forceField, &_globals._spriteIndexes[0]);

	if (_game._trigger == 70)
		_scene->_nextSceneId = 309;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
