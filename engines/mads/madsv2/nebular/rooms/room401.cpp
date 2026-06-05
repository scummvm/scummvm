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

Scene401::Scene401(RexNebularEngine *vm) : Scene4xx(vm), _destPos(0, 0) {
	_northFl = false;
	_timer = 0;
}

void Scene401::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_northFl);
	s.syncAsSint16LE(_destPos.x);
	s.syncAsSint16LE(_destPos.y);
	s.syncAsUint32LE(_timer);
}

void Scene401::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene401::enter() {
	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_northFl = false;

	_timer = 0;

	if (_scene->_priorSceneId == 402) {
		_game._player._playerPos = Common::Point(203, 115);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 354) {
		_game._player._playerPos = Common::Point(149, 90);
		_game._player._facing = FACING_SOUTH;
		_northFl = true;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(142, 131);
		_game._player._facing = FACING_NORTH;
	}

	_game.loadQuoteSet(0x1D4, 0);
	sceneEntrySound();
}

void Scene401::step() {
	if (_game._trigger == 70) {
		_scene->_nextSceneId = 354;
		_scene->_reloadSceneFlag = true;
	}

	if (_game._trigger == 80) {
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_northFl = false;
		_game._player.walk(Common::Point(149, 110), FACING_SOUTH);
	}

	if (_scene->_frameStartTime >= _timer) {
		int dist = 64 - ((Math::hypotenuse(_game._player._playerPos.x - 219, _game._player._playerPos.y - 115) * 64) / 120);

		if (dist > 64)
			dist = 64;
		else if (dist < 1)
			dist = 1;

		_vm->_sound->command(12, dist);
		_timer = _scene->_frameStartTime + _game._player._ticksAmount;
	}

}

void Scene401::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_NORTH)) {
		_game._player.walk(Common::Point(149, 89), FACING_NORTH);
		_northFl = false;
	}

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH) && !_northFl)
		_game._player._walkOffScreenSceneId = 405;

	if (_action.isAction(VERB_TAKE))
		_game._player._needToWalk = false;

	if (_game._player._needToWalk && _northFl) {
		if (_globals[kSexOfRex] == REX_MALE)
			_destPos = Common::Point(148, 94);
		else
			_destPos = Common::Point(149, 99);

		_game._player.walk(_destPos, FACING_SOUTH);
	}
}

void Scene401::actions() {
	if ((_game._player._playerPos == _destPos) && _northFl) {
		if (_globals[kSexOfRex] == REX_MALE) {
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_vm->_sound->command(21);
			_scene->loadAnimation(formAnimName('s', 1), 70);
			_globals[kHasBeenScanned] = true;
			_vm->_sound->command(22);
			int idx = _scene->_kernelMessages.add(Common::Point(153, 46), 0x1110, 32, 0, 60, _game.getQuote(0x1D4));
			_scene->_kernelMessages.setQuoted(idx, 4, true);
		}

		if (_globals[kSexOfRex] == REX_FEMALE) {
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_vm->_sound->command(21);
			_scene->loadAnimation(formAnimName('s', 2), 80);
			_vm->_sound->command(23);
			_globals[kHasBeenScanned] = true;
		}
	}

	if (_action.isAction(VERB_WALK_INTO, NOUN_BAR)) {
		if (!_northFl)
			_scene->_nextSceneId = 402;
	} else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_NORTH))
		_scene->_nextSceneId = 354;
	else if (_action.isAction(VERB_LOOK, NOUN_SCANNER)) {
		if (_globals[kHasBeenScanned])
			_vm->_dialogs->show(40111);
		else
			_vm->_dialogs->show(40110);
	} else if (_action.isAction(VERB_LOOK, NOUN_BAR))
		_vm->_dialogs->show(40112);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGN))
		_vm->_dialogs->show(40113);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(40114);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_NORTH))
		_vm->_dialogs->show(40115);
	else if (_action._lookFlag)
		_vm->_dialogs->show(40116);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
