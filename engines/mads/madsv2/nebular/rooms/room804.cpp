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

Scene804::Scene804(RexNebularEngine *vm) : Scene8xx(vm) {
	_messWithThrottle = false;
	_movingThrottle = false;
	_throttleGone = false;
	_dontPullThrottleAgain = false;
	_pullThrottleReally = false;
	_alreadyOrgan = false;
	_alreadyPop = false;

	_throttleCounter = 0;
	_resetFrame = -1;
}

void Scene804::synchronize(Common::Serializer &s) {
	Scene8xx::synchronize(s);

	s.syncAsByte(_messWithThrottle);
	s.syncAsByte(_movingThrottle);
	s.syncAsByte(_throttleGone);
	s.syncAsByte(_dontPullThrottleAgain);
	s.syncAsByte(_pullThrottleReally);
	s.syncAsByte(_alreadyOrgan);
	s.syncAsByte(_alreadyPop);

	s.syncAsSint16LE(_resetFrame);
	s.syncAsUint32LE(_throttleCounter);
}

void Scene804::setup() {
	Scene8xx::setPlayerSpritesPrefix();
	Scene8xx::setAAName();
}

void Scene804::enter() {
	_messWithThrottle = false;
	_throttleCounter = 0;
	_movingThrottle = false;
	_throttleGone = false;
	_dontPullThrottleAgain = false;
	_resetFrame = -1;
	_pullThrottleReally = false;
	_alreadyOrgan = false;
	_alreadyPop = false;


	if (_globals[kCopyProtectFailed]) {
		// Copy protection failed
		_globals[kInSpace] = true;
		_globals[kWindowFixed] = 0;
	}

	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('f', 1));

	_game.loadQuoteSet(791, 0);

	if (_globals[kInSpace]) {
		if (_globals[kWindowFixed]) {
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], 0, 1);
			_scene->_sequences.addTimer(60, 100);
		} else {
			_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
			_globals._sequenceIndexes[7] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[7], false, 4, 0, 0, 0);
			_scene->_sequences.addTimer(160, 70);
			_game._player._stepEnabled = false;
		}
	} else {
		if (_globals[kBeamIsUp]) {
			_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 7);
		}

		if (_globals[kWindowFixed])
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);

		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(133, 139));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
	}

	_scene->loadAnimation(Resources::formatName(804, 'r', 1, EXT_AA, ""));

	Scene8xx::sceneEntrySound();

	if (_globals[kInSpace] && !_globals[kWindowFixed]) {
		_scene->_userInterface.setup(kInputLimitedSentences);
		_vm->_sound->command(19);
	}
}

void Scene804::step() {
	if (!_messWithThrottle) {

		if ((_throttleGone) && (_movingThrottle) && (_scene->_animation[0]->getCurrentFrame() == 39)) {
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle
			(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(133, 139));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
			_throttleGone = false;
		}

		if ((_movingThrottle) && (_scene->_animation[0]->getCurrentFrame() == 42)) {
			_resetFrame = 0;
			_movingThrottle = false;
		}

		if (_game._trigger == 70) {
			_resetFrame = 42;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 65)
			_scene->_sequences.remove(_globals._sequenceIndexes[7]);

		switch (_game._storyMode) {
		case STORYMODE_NAUGHTY:
		default:
			if (_scene->_animation[0]->getCurrentFrame() == 81) {
				_resetFrame = 80;
				_globals[kInSpace] = false;
				_globals[kBeamIsUp] = true;

				//assert(!_globals[kCopyProtectFailed]);
				_game._winStatus = 4;
				return;
			}
			break;

		case STORYMODE_NICE:
			if (_scene->_animation[0]->getCurrentFrame() == 68) {
				_resetFrame = 66;
				_globals[kInSpace] = false;
				_globals[kBeamIsUp] = true;

				assert(!_globals[kCopyProtectFailed]);
				_game._winStatus = 4;
				return;
			}
		}

		if (_scene->_animation[0]->getCurrentFrame() == 34) {
			_resetFrame = 36;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		}

		if (_scene->_animation[0]->getCurrentFrame() == 37) {
			_resetFrame = 36;
			if (!_dontPullThrottleAgain) {
				_dontPullThrottleAgain = true;
				_scene->_sequences.addTimer(60, 80);
			}
		}

		if (_game._trigger == 80) {
			_scene->_nextSceneId = 803;
		}

		if ((_scene->_animation[0]->getCurrentFrame() == 7) && (!_globals[kWindowFixed])) {
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_scene->_sequences.addTimer(20, 110);
			_globals[kWindowFixed] = true;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 10) {
			_resetFrame = 0;
			_game._player._stepEnabled = true;
			_game._objects.setRoom(OBJ_POLYCEMENT, NOWHERE);
		}

		// FIXME: Original doesn't have resetFrame check. Check why this has been needed
		if (_resetFrame == -1 && _scene->_animation[0]->getCurrentFrame() == 1) {
			int randomVal = _vm->getRandomNumber(29) + 1;
			switch (randomVal) {
			case 1:
				_resetFrame = 25;
				break;
			case 2:
				_resetFrame = 27;
				break;
			case 3:
				_resetFrame = 29;
				break;
			default:
				_resetFrame = 0;
				break;
			}
		}

		switch (_scene->_animation[0]->getCurrentFrame()) {
		case 26:
		case 28:
		case 31:
			_resetFrame = 0;
			break;
		default:
			break;
		}
	} else {
		if ((_scene->_animation[0]->getCurrentFrame() == 36) && (!_throttleGone)) {
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_throttleGone = true;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 39) {
			_movingThrottle = false;
			switch (_throttleCounter) {
			case 1:
				break;
			case 3:
				_scene->_sequences.addTimer(130, 120);
				break;
			default:
				break;
			}
		}

		if (!_movingThrottle) {
			++_throttleCounter;
			_movingThrottle = true;
			if (_throttleCounter < 4) {
				_resetFrame = 34;
			} else {
				_messWithThrottle = false;
				_throttleCounter = 0;
				_game._player._stepEnabled = true;
			}
		}
	}

	if (_game._trigger == 120) {
		_vm->_dialogs->show(80422);
	}

	if (_game._trigger == 110) {
		_vm->_dialogs->show(80426);
	}

	if (_pullThrottleReally) {
		_resetFrame = 32;
		_pullThrottleReally = false;
	}

	if (_resetFrame >= 0) {
		if (_resetFrame != _scene->_animation[0]->getCurrentFrame()) {
			_scene->_animation[0]->setCurrentFrame(_resetFrame);
			_resetFrame = -1;
		}
	}

	if (_game._trigger == 90) {
		_scene->_nextSceneId = 803;
	}

	if ((_scene->_animation[0]->getCurrentFrame() == 72) && !_alreadyPop) {
		_vm->_sound->command(21);
		_alreadyPop = true;
	}

	if ((_scene->_animation[0]->getCurrentFrame() == 80) && !_alreadyOrgan) {
		_vm->_sound->command(22);
		_alreadyOrgan = true;
	}
}

void Scene804::actions() {
	if (_action.isAction(VERB_LOOK, NOUN_SERVICE_PANEL) ||
		_action.isAction(VERB_OPEN, NOUN_SERVICE_PANEL)) {
		_scene->_nextSceneId = 805;
	} else if ((_action.isAction(VERB_ACTIVATE, NOUN_REMOTE)) && _globals[kTopButtonPushed]) {
		if (!_globals[kInSpace]) {
			// Top button pressed on panel in hanger control
			if (!_globals[kBeamIsUp]) {
				_globals[kFromCockpit] = true;
				_globals[kUpBecauseOfRemote] = true;
				_scene->_nextSceneId = 803;
			} else {
				// Player turning off remote
				_globals[kBeamIsUp] = false;
				_globals[kUpBecauseOfRemote] = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[8]);
				_vm->_sound->command(15);
			}
		}
	} else if (_action.isAction(VERB_PULL, NOUN_THROTTLE)) {
		_game._player._stepEnabled = false;
		if (_globals[kBeamIsUp]) {
			if (!_game._objects.isInInventory(OBJ_VASE) && _globals[kWindowFixed]) {
				_vm->_dialogs->show(80423);
				_game._player._stepEnabled = true;
			} else {
				_action._inProgress = false;

				_vm->_dialogs->show(80424);
				_pullThrottleReally = true;
				_scene->_kernelMessages.add(Common::Point(78, 75), 0x1110, 0, 0,
					120, _game.getQuote(791));
			}
		} else {
			_messWithThrottle = true;
		}
	} else if (_action.isAction(VERB_APPLY, NOUN_POLYCEMENT, NOUN_CRACK) ||
		_action.isAction(VERB_PUT, NOUN_POLYCEMENT, NOUN_CRACK)) {
		if (!_globals[kWindowFixed]) {
			_resetFrame = 2;
			_game._player._stepEnabled = false;
		}
	} else if (_action.isAction(VERB_EXIT, NOUN_SHIP)) {
		_globals[kExitShip] = true;
		_globals[kFromCockpit] = true;
		if (_globals[kBeamIsUp]) {
			_vm->_dialogs->show(80425);
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			_vm->_sound->command(15);
			_globals[kBeamIsUp] = false;
		}
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(2, 90);
	} else  if (_action._lookFlag) {
		_vm->_dialogs->show(80410);
	} else if ((_action.isAction(VERB_LOOK, NOUN_WINDOW)) ||
		(_action.isAction(VERB_LOOK_OUT, NOUN_WINDOW))) {
		if (_globals[kBeamIsUp]) {
			_vm->_dialogs->show(80412);
		} else {
			_vm->_dialogs->show(80411);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_CRACK)) {
		if (_globals[kWindowFixed]) {
			_vm->_dialogs->show(80414);
		} else {
			_vm->_dialogs->show(80413);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_CONTROLS)) {
		_vm->_dialogs->show(80415);
	} else if (_action.isAction(VERB_LOOK, NOUN_STATUS_PANEL)) {
		if (_globals[kBeamIsUp]) {
			_vm->_dialogs->show(80417);
		} else {
			_vm->_dialogs->show(80416);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_TP)) {
		_vm->_dialogs->show(80418);
	} else if (_action.isAction(VERB_TAKE, NOUN_TP)) {
		_vm->_dialogs->show(80419);
	} else if (_action.isAction(VERB_LOOK, NOUN_INSTRUMENTATION)) {
		_vm->_dialogs->show(80420);
	} else  if (_action.isAction(VERB_LOOK, NOUN_SEAT)) {
		_vm->_dialogs->show(80421);
	} else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
