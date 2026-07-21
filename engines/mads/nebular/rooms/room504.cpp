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

#include "math/utils.h"
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section5.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _carAnimationMode;
	int32 _carFrame;
};

static Scratch local;


static void room_504_init() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 2));

	for (int i = 0; i < 4; i++)
		_globals._spriteIndexes[5 + i] = _scene->_sprites.addSprites(formAnimName('m', i));

	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 0));
	else {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 1));
		_scene->changeVariant(1);
	}

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 0);
	local._carFrame = -1;

	if ((_scene->_priorSceneId == 505) && (_globals[kHoverCarDestination] != _globals[kHoverCarLocation])) {
		local._carAnimationMode = 1;
		_scene->loadAnimation(formAnimName('A', -1));
		_vm->_sound->command(14);
		_scene->_sequences.addTimer(1, 70);
		_game._player._stepEnabled = false;
	} else {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 3));
		local._carAnimationMode = 1;
		_scene->loadAnimation(formAnimName('A', -1));
		if ((_scene->_priorSceneId != RETURNING_FROM_DIALOG) && (_scene->_priorSceneId != 505))
			_globals[kHoverCarLocation] = _scene->_priorSceneId;

		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
	}

	if (_globals[kTimebombTimer] > 10500)
		_globals[kTimebombTimer] = 10500;

	section_5_music();
}

static void room_504_daemon() {
	if ((local._carAnimationMode == 1) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._carFrame) {
			local._carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame;

			if (local._carFrame == 1)
				nextFrame = 0;
			else
				nextFrame = -1;

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				local._carFrame = nextFrame;
			}
		}
	}


	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
			if (_globals[kHoverCarDestination] != -1) {
				_game._player._stepEnabled = false;
				_scene->freeAnimation();
				local._carAnimationMode = 2;
				if (((_globals[kHoverCarLocation] >= 500 && _globals[kHoverCarLocation] <= 599) &&
					(_globals[kHoverCarDestination] >= 500 && _globals[kHoverCarDestination] <= 599)) ||
					((_globals[kHoverCarLocation] >= 600 && _globals[kHoverCarLocation] <= 699) &&
						(_globals[kHoverCarDestination] >= 600 && _globals[kHoverCarDestination] <= 699))) {
					_scene->loadAnimation(formAnimName('A', -1), 71);
				} else if (_globals[kHoverCarLocation] > _globals[kHoverCarDestination])
					_scene->loadAnimation(formAnimName('C', -1), 71);
				else
					_scene->loadAnimation(formAnimName('B', -1), 71);
			}
			break;

		case 71:
			_vm->_sound->command(15);
			_scene->_nextSceneId = _globals[kHoverCarDestination];
			break;

		default:
			break;
		}
	}

	if ((_globals[kTimebombTimer] >= 10800) && (_globals[kTimebombStatus] == TIMEBOMB_ACTIVATED) && (_game._difficulty != 3)) {
		_globals[kTimebombStatus] = TIMEBOMB_DEAD;
		_globals[kTimebombTimer] = 0;
		_globals[kCheckDaemonTimebomb] = false;
		_scene->_nextSceneId = 620;
	}
}

static void room_504_pre_parser() {
	_game._player._needToWalk = false;
}

static void room_504_parser() {
	if (player_said_2(exit_from, car)) {
		_vm->_sound->command(15);
		_scene->_nextSceneId = _globals[kHoverCarLocation];
	} else if (player_said_2(activate, car_controls)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_vm->_sound->command(39);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_scene->_sequences.remove(_globals._sequenceIndexes[7]);
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 18, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 6);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
		}
		break;

		case 2:
			_scene->_sequences.addTimer(10, 3);
			break;

		case 3:
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			if (_globals[kSexOfRex] == REX_MALE) {
				_vm->_sound->command(34);
				_scene->_sequences.addTimer(60, 4);
				_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
			} else {
				_vm->_sound->command(40);
				_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 18, 0, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 14);
				_scene->_sequences.addTimer(120, 5);
			}
			break;

		case 4:
			_game._player._stepEnabled = true;
			_globals[kHoverCarDestination] = _globals[kHoverCarLocation];
			_scene->_nextSceneId = 505;
			break;

		case 5:
			_game._player._stepEnabled = true;
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
			_vm->_dialogs->show(50421);
			break;

		default:
			break;
		}
	} else if (_action._lookFlag || player_said_2(look, interior_of_car))
		_vm->_dialogs->show(50412);
	else if (player_said_2(look, glove_compartment))
		_vm->_dialogs->show(50410);
	else if (player_said_2(look, car_controls) || player_said_2(look, dashboard))
		_vm->_dialogs->show(50411);
	else if (player_said_2(look, scent_packet))
		_vm->_dialogs->show(50413);
	else if (player_said_2(look, soda_cans))
		_vm->_dialogs->show(50414);
	else if (player_said_2(look, kitty))
		_vm->_dialogs->show(50415);
	else if (player_said_2(look, windshield) || player_said_2(look_through, windshield))
		_vm->_dialogs->show(50416);
	else if (player_said_2(look, rearview_mirror))
		_vm->_dialogs->show(50417);
	else if (player_said_2(take, rearview_mirror))
		_vm->_dialogs->show(50418);
	else if (player_said_2(look, moldy_sock))
		_vm->_dialogs->show(50419);
	else if (player_said_2(take, moldy_sock))
		_vm->_dialogs->show(50420);
	else
		return;

	_action._inProgress = false;
}

void room_504_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._carAnimationMode);
	s.syncAsSint16LE(local._carFrame);
}

void room_504_preload() {
	room_init_code_pointer = room_504_init;
	room_daemon_code_pointer = room_504_daemon;
	room_pre_parser_code_pointer = room_504_pre_parser;
	room_parser_code_pointer = room_504_parser;

	*player.series_name = '\0';
	section_5_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
