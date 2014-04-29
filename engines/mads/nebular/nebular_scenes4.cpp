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
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes4.h"

namespace MADS {

namespace Nebular {

void Scene4xx::setAAName() {
	_game._aaName = Resources::formatAAName(4);
}

void Scene4xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;

	if ((_scene->_nextSceneId == 403) || (_scene->_nextSceneId == 409))
		_game._player._spritesPrefix = "";
	else if (_globals[kSexOfRex] == REX_FEMALE)
		_game._player._spritesPrefix = "ROX";
	else
		_game._player._spritesPrefix = "RXM";

	_game._player._scalingVelocity = true;

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene4xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 401:
		_vm->_sound->startQueuedCommands();
		if (_scene->_priorSceneId == 402)
			_vm->_sound->command(12, 64);
		else
			_vm->_sound->command(12, 1);
		break;

	case 402:
		_vm->_sound->startQueuedCommands();
		_vm->_sound->command(12, 127);
		break;

	case 405:
	case 407:
	case 409:
	case 410:
	case 413:
		_vm->_sound->command(10);
		break;

	case 408:
		_vm->_sound->command(52);
		break;

	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

void Scene401::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene401::enter() {
	if (_scene->_priorSceneId != -2)
		_northFl = false;

	_timer = 0;

	if (_scene->_priorSceneId == 402) {
		_game._player._playerPos = Common::Point(203, 115);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 354) {
		_game._player._playerPos = Common::Point(149, 90);
		_game._player._facing = FACING_SOUTH;
		_northFl = true;
	} else if (_scene->_priorSceneId != -2) {
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
		_northFl  = false;
		_game._player.walk(Common::Point(149, 110), FACING_SOUTH);
	}

	if (_scene->_frameStartTime >= _timer) {
		int dist = 64 - ((_vm->hypotenuse(_game._player._playerPos.x - 219, _game._player._playerPos.y - 115) * 64) / 120);
		
		if (dist > 64)
			dist = 64;
		else if (dist < 1)
			dist = 1;

		_vm->_sound->command(12, dist);
		_timer = _scene->_frameStartTime + _game._player._ticksAmount;
	}

}

void Scene401::preActions() {
	if (_action.isAction(0x1AD, 0x2B4)) {
		_game._player.walk(Common::Point(149, 89), FACING_NORTH); 
		_northFl = false;
	}           

	if (_action.isAction(0x1AD, 0x2B3) && !_northFl)
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

	if (_action.isAction(0x242, 0x241)) {
		if (!_northFl)
			_scene->_nextSceneId = 402;
	} else if (_action.isAction(0x1AD, 0x2B4))
		_scene->_nextSceneId = 354;
	else if (_action.isAction(VERB_LOOK, 0x1F3)) {
		if (_globals[kHasBeenScanned])
			_vm->_dialogs->show(40111);
		else
			_vm->_dialogs->show(40110);
	} else if (_action.isAction(VERB_LOOK, 0x241))
		_vm->_dialogs->show(40112);
	else if (_action.isAction(VERB_LOOK, 0x244))
		_vm->_dialogs->show(40113);
	else if (_action.isAction(VERB_LOOK, 0x2B3))
		_vm->_dialogs->show(40114);
	else if (_action.isAction(VERB_LOOK, 0x2B4))
		_vm->_dialogs->show(40115);
	else if (_action._lookFlag)
		_vm->_dialogs->show(40116);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
