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
	if ((_globals[kFromCockpit] && !_globals[kExitShip]) ||
			_scene->_nextSceneId == 804 || _scene->_nextSceneId == 805 ||
			_scene->_nextSceneId == 808 || _scene->_nextSceneId == 810) {
		_game._player._spritesPrefix = "";
	} else {
		_game._player._spritesPrefix = _globals[kSexOfRex] == SEX_FEMALE ? "ROX" : "RXM";
	}

	_vm->_palette->setEntry(16, 0x0A, 0x3F, 0x3F);
	_vm->_palette->setEntry(17, 0x0A, 0x2D, 0x2D);
}

void Scene8xx::setup2() {
	_game._aaName = Resources::formatAAName(5);
}

void Scene8xx::enter1() {
	if (_vm->_musicFlag) {
		switch (_scene->_nextSceneId) {
		case 801:
		case 802:
		case 803:
		case 804:
		case 806:
		case 807:
		case 808:
			_vm->_sound->command(20);
			break;
		case 805:
			_vm->_sound->command(23);
			break;
		case 810:
			_vm->_sound->command(10);
			break;
		default:
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

void Scene804::setup() {
	Scene8xx::setup1();
	Scene8xx::setup2();
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
			_globals._sequenceIndexes[7] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[7], false, 4, 0, 0, 0);
			_scene->_sequences.addTimer(160, 70);
			_game._player._stepEnabled = false;
		}
	} else {
		if (_globals[kBeamIsUp] == 0)
			_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 1);

		if (_globals[kWindowFixed] == 0)
			_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);

		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[1], Common::Point(133, 139));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
	}

	_scene->loadAnimation(Resources::formatName(804, 'r', 1, EXT_AA, ""));

	Scene8xx::enter1();

	if (_globals[kInSpace] && !_globals[kWindowFixed]) {
		_scene->_userInterface.setup(kInputLimitedSentences);
		_vm->_sound->command(19);
	}
}

void Scene804::step() {
	if (!_messWithThrottle) {

		if ((_throttleGone) && (_movingThrottle) && (_scene->_activeAnimation->getCurrentFrame() == 39)) {
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle
				(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[1], Common::Point(133, 139));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
			_throttleGone = false;
		}

		if ((_movingThrottle) && (_scene->_activeAnimation->getCurrentFrame() == 42)) {
			_resetFrame = 0;
			_movingThrottle = false;
		}

		if (_game._trigger == 70) {
			_resetFrame = 42;
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 65) {
			_scene->_sequences.remove(_globals._sequenceIndexes[7]);
		}

		switch (_game._storyMode) {
		case STORYMODE_NAUGHTY:
			if (_scene->_activeAnimation->getCurrentFrame() == 81) {
				_resetFrame = 80; 
				_globals[kInSpace] = false;
				_globals[kBeamIsUp] = true;

				assert(!_globals[kCopyProtectFailed]);
				_game._winStatus = 4;
				_vm->quitGame();
			}
			break;

		case STORYMODE_NICE:			 
			if (_scene->_activeAnimation->getCurrentFrame() == 68) {
				_resetFrame = 66;
				_globals[kInSpace] = false;
				_globals[kBeamIsUp] = true;

				assert(!_globals[kCopyProtectFailed]);
				_game._winStatus = 4;
				_vm->quitGame();
			}
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 34) {
			_resetFrame = 36;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 37) {
			_resetFrame = 36;
			if (!_dontPullThrottleAgain) {
				_dontPullThrottleAgain = true;
				_scene->_sequences.addTimer(60, 80);
			}
		}

		if (_game._trigger == 80) {
			_scene->_nextSceneId = 803;
		}

		if ((_scene->_activeAnimation->getCurrentFrame() == 7) && (!_globals[kWindowFixed])) {
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_scene->_sequences.addTimer(20, 110);
			_globals[kWindowFixed] = true;
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 10) {
			_resetFrame = 0;
			_game._player._stepEnabled = true;
			_game._objects.setRoom(OBJ_POLYCEMENT, NOWHERE);
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 1) {
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

		switch (_scene->_activeAnimation->getCurrentFrame()) {
		case 26:
		case 28:
		case 31:
			_resetFrame = 0; 
			break;
		}
	} else {   
		if ((_scene->_activeAnimation->getCurrentFrame() == 36) && (!_throttleGone)) {
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_throttleGone = true;
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 39) {
			_movingThrottle = false;
			switch (_throttleCounter) {
			case 1:
				break;
			case 3:
				_scene->_sequences.addTimer(130, 120);
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
		if (_resetFrame != _scene->_activeAnimation->getCurrentFrame()) {
			_scene->_activeAnimation->setCurrentFrame(_resetFrame);
			_resetFrame = -1;
		}
	}

	if (_game._trigger == 90) {
		_scene->_nextSceneId = 803;
	}

	if ((_scene->_activeAnimation->getCurrentFrame() == 72) && !_alreadyPop)  {
		_vm->_sound->command(21);
		_alreadyPop = true;
	}

	if ((_scene->_activeAnimation->getCurrentFrame() == 80) && !_alreadyOrgan) {
		_vm->_sound->command(22);
		_alreadyOrgan = true;
	}
}

} // End of namespace Nebular

} // End of namespace MADS
