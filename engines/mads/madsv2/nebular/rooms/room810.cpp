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

Scene810::Scene810(RexNebularEngine *vm) : Scene8xx(vm) {
	_moveAllowed = false;
}

void room_810_synchronize(Common::Serializer &s) {
	Scene8xx::synchronize(s);

	s.syncAsByte(_moveAllowed);
}

void Scene810::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

static void room_810_init() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(Resources::formatName(810, 'a', -1, EXT_AA, ""));
	_moveAllowed = true;

	sceneEntrySound();
}

void Scene810::step() {
	if (_scene->_animation[0] && (_scene->_animation[0]->getCurrentFrame() == 200)
		&& _moveAllowed) {
		_scene->_sequences.addTimer(100, 70);
		_moveAllowed = false;
	}

	if (_game._trigger == 70)
		_scene->_nextSceneId = 804;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
