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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section8.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _messWithThrottle;
	bool _movingThrottle;
	bool _throttleGone;
	bool _dontPullThrottleAgain;
	bool _pullThrottleReally;
	bool _alreadyOrgan;
	bool _alreadyPop;
	int16 _throttleCounter;
	int16 _resetFrame;
};

static Scratch local;


static void room_804_init() {
	local._messWithThrottle = false;
	local._throttleCounter = 0;
	local._movingThrottle = false;
	local._throttleGone = false;
	local._dontPullThrottleAgain = false;
	local._resetFrame = -1;
	local._pullThrottleReally = false;
	local._alreadyOrgan = false;
	local._alreadyPop = false;


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

	section_8_music();

	if (_globals[kInSpace] && !_globals[kWindowFixed]) {
		_scene->_userInterface.setup(kInputLimitedSentences);
		_vm->_sound->command(19);
	}
}

static void room_804_daemon() {
	if (!local._messWithThrottle) {

		if ((local._throttleGone) && (local._movingThrottle) && (_scene->_animation[0]->getCurrentFrame() == 39)) {
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle
			(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(133, 139));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
			local._throttleGone = false;
		}

		if ((local._movingThrottle) && (_scene->_animation[0]->getCurrentFrame() == 42)) {
			local._resetFrame = 0;
			local._movingThrottle = false;
		}

		if (_game._trigger == 70) {
			local._resetFrame = 42;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 65)
			_scene->_sequences.remove(_globals._sequenceIndexes[7]);

		switch (_game._storyMode) {
		case STORYMODE_NAUGHTY:
		default:
			if (_scene->_animation[0]->getCurrentFrame() == 81) {
				local._resetFrame = 80;
				_globals[kInSpace] = false;
				_globals[kBeamIsUp] = true;

				//assert(!_globals[kCopyProtectFailed]);
				_game._winStatus = 4;
				return;
			}
			break;

		case STORYMODE_NICE:
			if (_scene->_animation[0]->getCurrentFrame() == 68) {
				local._resetFrame = 66;
				_globals[kInSpace] = false;
				_globals[kBeamIsUp] = true;

				assert(!_globals[kCopyProtectFailed]);
				_game._winStatus = 4;
				return;
			}
		}

		if (_scene->_animation[0]->getCurrentFrame() == 34) {
			local._resetFrame = 36;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		}

		if (_scene->_animation[0]->getCurrentFrame() == 37) {
			local._resetFrame = 36;
			if (!local._dontPullThrottleAgain) {
				local._dontPullThrottleAgain = true;
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
			local._resetFrame = 0;
			_game._player._stepEnabled = true;
			_game._objects.setRoom(OBJ_POLYCEMENT, NOWHERE);
		}

		// FIXME: Original doesn't have resetFrame check. Check why this has been needed
		if (local._resetFrame == -1 && _scene->_animation[0]->getCurrentFrame() == 1) {
			int randomVal = _vm->getRandomNumber(29) + 1;
			switch (randomVal) {
			case 1:
				local._resetFrame = 25;
				break;
			case 2:
				local._resetFrame = 27;
				break;
			case 3:
				local._resetFrame = 29;
				break;
			default:
				local._resetFrame = 0;
				break;
			}
		}

		switch (_scene->_animation[0]->getCurrentFrame()) {
		case 26:
		case 28:
		case 31:
			local._resetFrame = 0;
			break;
		default:
			break;
		}
	} else {
		if ((_scene->_animation[0]->getCurrentFrame() == 36) && (!local._throttleGone)) {
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			local._throttleGone = true;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 39) {
			local._movingThrottle = false;
			switch (local._throttleCounter) {
			case 1:
				break;
			case 3:
				_scene->_sequences.addTimer(130, 120);
				break;
			default:
				break;
			}
		}

		if (!local._movingThrottle) {
			++local._throttleCounter;
			local._movingThrottle = true;
			if (local._throttleCounter < 4) {
				local._resetFrame = 34;
			} else {
				local._messWithThrottle = false;
				local._throttleCounter = 0;
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

	if (local._pullThrottleReally) {
		local._resetFrame = 32;
		local._pullThrottleReally = false;
	}

	if (local._resetFrame >= 0) {
		if (local._resetFrame != _scene->_animation[0]->getCurrentFrame()) {
			_scene->_animation[0]->setCurrentFrame(local._resetFrame);
			local._resetFrame = -1;
		}
	}

	if (_game._trigger == 90) {
		_scene->_nextSceneId = 803;
	}

	if ((_scene->_animation[0]->getCurrentFrame() == 72) && !local._alreadyPop) {
		_vm->_sound->command(21);
		local._alreadyPop = true;
	}

	if ((_scene->_animation[0]->getCurrentFrame() == 80) && !local._alreadyOrgan) {
		_vm->_sound->command(22);
		local._alreadyOrgan = true;
	}
}

static void room_804_parser() {
	if (player_said_2(look, service_panel) ||
		player_said_2(open, service_panel)) {
		_scene->_nextSceneId = 805;
	} else if ((player_said_2(activate, remote)) && _globals[kTopButtonPushed]) {
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
	} else if (player_said_2(pull, throttle)) {
		_game._player._stepEnabled = false;
		if (_globals[kBeamIsUp]) {
			if (!_game._objects.isInInventory(OBJ_VASE) && _globals[kWindowFixed]) {
				_vm->_dialogs->show(80423);
				_game._player._stepEnabled = true;
			} else {
				_action._inProgress = false;

				_vm->_dialogs->show(80424);
				local._pullThrottleReally = true;
				_scene->_kernelMessages.add(Common::Point(78, 75), 0x1110, 0, 0,
					120, _game.getQuote(791));
			}
		} else {
			local._messWithThrottle = true;
		}
	} else if (player_said_3(apply, polycement, crack) ||
		player_said_3(put, polycement, crack)) {
		if (!_globals[kWindowFixed]) {
			local._resetFrame = 2;
			_game._player._stepEnabled = false;
		}
	} else if (player_said_2(exit, ship)) {
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
	} else if ((player_said_2(look, window)) ||
		(player_said_2(look_out, window))) {
		if (_globals[kBeamIsUp]) {
			_vm->_dialogs->show(80412);
		} else {
			_vm->_dialogs->show(80411);
		}
	} else if (player_said_2(look, crack)) {
		if (_globals[kWindowFixed]) {
			_vm->_dialogs->show(80414);
		} else {
			_vm->_dialogs->show(80413);
		}
	} else if (player_said_2(look, controls)) {
		_vm->_dialogs->show(80415);
	} else if (player_said_2(look, status_panel)) {
		if (_globals[kBeamIsUp]) {
			_vm->_dialogs->show(80417);
		} else {
			_vm->_dialogs->show(80416);
		}
	} else if (player_said_2(look, tp)) {
		_vm->_dialogs->show(80418);
	} else if (player_said_2(take, tp)) {
		_vm->_dialogs->show(80419);
	} else if (player_said_2(look, instrumentation)) {
		_vm->_dialogs->show(80420);
	} else  if (player_said_2(look, seat)) {
		_vm->_dialogs->show(80421);
	} else
		return;

	_action._inProgress = false;
}

void room_804_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._messWithThrottle);
	s.syncAsByte(local._movingThrottle);
	s.syncAsByte(local._throttleGone);
	s.syncAsByte(local._dontPullThrottleAgain);
	s.syncAsByte(local._pullThrottleReally);
	s.syncAsByte(local._alreadyOrgan);
	s.syncAsByte(local._alreadyPop);

	s.syncAsSint16LE(local._resetFrame);
	s.syncAsUint32LE(local._throttleCounter);
}

void room_804_preload() {
	room_init_code_pointer = room_804_init;
	room_daemon_code_pointer = room_804_daemon;
	room_parser_code_pointer = room_804_parser;

	section_8_walker();
	section_8_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
