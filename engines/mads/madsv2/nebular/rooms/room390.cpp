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

void Scene390::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene390::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;

	sceneEntrySound();
}

void Scene390::actions() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE))
		_vm->_dialogs->show(39010);
	else if (_action.isAction(VERB_OPEN, NOUN_GRATE))
		_vm->_dialogs->show(39011);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
