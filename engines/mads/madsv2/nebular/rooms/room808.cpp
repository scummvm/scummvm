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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section8.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _goingTo803;
};

static Scratch local;


static void room_808_init() {
	_scene->_userInterface.setup(kInputLimitedSentences);

	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*REXHAND");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 2));

	if (_globals[kTopButtonPushed])
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	else
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);

	local._goingTo803 = false;

	if (_globals[kCameFromCut] && _globals[kCutX] != 0) {
		_globals[kCutX] = 0;
		_globals[kCameFromCut] = false;
		_globals[kReturnFromCut] = false;
		_globals[kBeamIsUp] = false;
		_globals[kForceBeamDown] = false;
		_globals[kDontRepeat] = false;
	} else if ((_scene->_priorSceneId == 803) && _globals[kReturnFromCut]) {
		_globals[kDontRepeat] = false;
		_globals[kBeamIsUp] = true;
		_globals[kAntigravClock] = _scene->_frameStartTime;
		_globals[kAntigravTiming] = _scene->_frameStartTime;
		_globals[kForceBeamDown] = false;
		_globals[kReturnFromCut] = false;
	}

	_globals[kBetweenRooms] = false;

	if (_globals[kBeamIsUp]) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
	}

	section_8_music();
}

static void room_808_parser() {
	if (_action.isAction(words_press, words_start_button_2)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 211));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
			break;

		case 70:
			if (!_globals[kBeamIsUp] && !_globals[kTopButtonPushed]) {
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
				local._goingTo803 = true;
				_vm->_sound->command(20);
				_vm->_sound->command(25);
			}
			_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 211));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		case 71:
			_game._player._stepEnabled = true;
			if (local._goingTo803 && !_globals[kTopButtonPushed]) {
				local._goingTo803 = false;
				_globals[kReturnFromCut] = true;
				_scene->_nextSceneId = 803;
			}
			break;

		default:
			break;
		}
	} else if (_action.isAction(words_press, words_timer_button_2)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 186));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
			break;
		case 90:
			if (_globals[kTopButtonPushed]) {
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
				_vm->_sound->command(20);
			}
			_globals[kTopButtonPushed] = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 186));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
			break;

		case 91:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(words_press, words_remote_button_2)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 163));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			break;

		case 80:
			if (!_globals[kTopButtonPushed]) {
				_scene->_sequences.remove(_globals._sequenceIndexes[2]);
				_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
				_vm->_sound->command(20);
			}
			_globals[kTopButtonPushed] = true;
			_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 163));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
			break;

		case 81:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(words_press, words_start_button_1)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 4, 2, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(168, 211));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
			break;

		case 70:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(words_press, words_remote_button_1)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 4, 2, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(172, 163));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			break;

		case 80:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(words_press, words_timer_button_1)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 4, 2, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(172, 186));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
			break;

		case 90:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(words_exit, words_panel)) {
		_scene->_nextSceneId = 801;
		_globals[kBetweenRooms] = true;
	} else
		return;

	_action._inProgress = false;
}

void room_808_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._goingTo803);
}

void room_808_preload() {
	room_init_code_pointer = room_808_init;

	section_8_walker();
	section_8_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
