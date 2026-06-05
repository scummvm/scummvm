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

void Scene399::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene399::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;
	sceneEntrySound();
}

void Scene399::actions() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE)) {
		if (_globals[kAfterHavoc]) {
			if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
				_vm->_dialogs->show(38911);
			else
				_vm->_dialogs->show(38912);
		} else
			_vm->_dialogs->show(38910);
	} else if (_action.isAction(VERB_OPEN, NOUN_GRATE)) {
		if (_globals[kAfterHavoc])
			_vm->_dialogs->show(38914);
		else
			_vm->_dialogs->show(38913);
	} else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
