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
	int16 _frame;
	int16 _nextButtonId;
	int16 _homeSelectedId;
	int16 _selectedId;
	int16 _activeCars;
	int16 _carLocations[9];
};

static Scratch local;


static void room_505_init() {
	for (int i = 0; i < 9; i++)
		_globals._spriteIndexes[i] = _scene->_sprites.addSprites(formAnimName('a', i + 1));

	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('g', 1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('g', 0));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('t', -1));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('e', -1));

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_globals._sequenceIndexes[12] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[12], false, 6, 1, 0, 0);

	_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 6, 1, 120, 0);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
	_scene->_sequences.addTimer(30, 62);

	local._carLocations[0] = 501;
	local._carLocations[1] = 506;
	local._carLocations[2] = 511;
	local._carLocations[3] = 513;
	local._carLocations[4] = 601;
	local._carLocations[5] = 604;
	local._carLocations[6] = 607;
	local._carLocations[7] = 609;
	local._carLocations[8] = 612;

	local._activeCars = false;

	for (int i = 0; i < 9; i++) {
		if (_globals[kHoverCarLocation] == local._carLocations[i]) {
			local._homeSelectedId = i;
			if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
				local._selectedId = i;
		}
	}

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	local._frame = -1;
	_scene->loadAnimation(formAnimName('a', -1));
	_scene->_animation[0]->setCurrentFrame(86);

	section_5_music();
	_vm->_sound->command(16);
}

static void room_505_daemon() {
	if (local._frame != _scene->_animation[0]->getCurrentFrame()) {
		local._frame = _scene->_animation[0]->getCurrentFrame();
		int resetFrame = -1;

		switch (local._frame) {
		case 4:
		case 24:
		case 33:
		case 53:
		case 62:
		case 82:
			if (local._nextButtonId == 0x38A)
				resetFrame = 4;
			else if (local._nextButtonId == 0x38B)
				resetFrame = 33;
			else if (local._nextButtonId == 0x2DE)
				resetFrame = 62;

			break;

		case 15:
		case 44:
		case 73:
		{
			int this_button;
			int old_select;
			_vm->_sound->command(17);
			old_select = local._selectedId;
			if (local._frame == 15) {
				this_button = 0x38A;
				local._selectedId = (local._selectedId + 1) % 9;
			} else if (local._frame == 44) {
				this_button = 0x38B;
				local._selectedId--;
				if (local._selectedId < 0)
					local._selectedId = 8;
			} else {
				this_button = 0x2DE;
				if ((_globals[kTimebombStatus] == TIMEBOMB_ACTIVATED) && (local._carLocations[local._selectedId] == 501))
					_vm->_dialogs->show(431);
				else if (local._selectedId != local._homeSelectedId) {
					local._nextButtonId = 0;
					local._activeCars = true;
					_game._player._stepEnabled = false;
					_scene->_sequences.remove(_globals._sequenceIndexes[1]);
					_scene->_sequences.remove(_globals._sequenceIndexes[0]);
					_scene->_sequences.remove(_globals._sequenceIndexes[13]);
					_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[13], false, 6, 1, 0, 0);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 63);
					_vm->_sound->command(18);
				}
			}

			if (local._nextButtonId == this_button)
				local._nextButtonId = 0;

			if (old_select != local._selectedId) {
				_scene->_sequences.remove(_globals._sequenceIndexes[11]);
				_globals._sequenceIndexes[11] = _scene->_sequences.startCycle(_globals._spriteIndexes[11], false, local._selectedId + 1);
				if (old_select != local._homeSelectedId)
					_scene->_sequences.remove(_globals._sequenceIndexes[0]);

				if (local._selectedId != local._homeSelectedId) {
					_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0 + local._selectedId], false, 24, 0, 0, 0);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 1);
				}
			}
			break;
		}

		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			if (local._nextButtonId == 0x38A)
				resetFrame = 29 - local._frame;

			break;

		case 26:
		case 55:
		case 84:
			if (local._nextButtonId != 0)
				resetFrame = 3;

			break;

		case 27:
		case 56:
		case 85:
			if (local._nextButtonId != 0)
				resetFrame = 2;

			break;

		case 29:
		case 58:
		case 87:
			if (local._activeCars)
				_globals[kHoverCarDestination] = local._carLocations[local._selectedId];

			if (local._nextButtonId == 0x38A)
				resetFrame = 0;
			else if (local._nextButtonId == 0x38B)
				resetFrame = 29;
			else if (local._nextButtonId == 0x2DE)
				resetFrame = 58;
			else
				resetFrame = 86;
			break;

		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
			if (local._nextButtonId == 0x38B)
				resetFrame = 87 - local._frame;

			break;

		case 76:
		case 77:
		case 78:
		case 79:
		case 80:
		case 81:
			if (local._nextButtonId == 0x2DE)
				resetFrame = 145 - local._frame;

			break;

		default:
			break;
		}

		if ((resetFrame >= 0) && (resetFrame != _scene->_animation[0]->getCurrentFrame())) {
			_scene->_animation[0]->setCurrentFrame(resetFrame);
			local._frame = resetFrame;
		}
	}

	switch (_game._trigger) {
	case 60:
	{
		_game._player._stepEnabled = true;
		int syncIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], syncIdx);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[local._homeSelectedId], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_globals._sequenceIndexes[11] = _scene->_sequences.startCycle(_globals._spriteIndexes[11], false, local._selectedId + 1);

		if (local._selectedId != local._homeSelectedId) {
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0 + local._selectedId], false, 24, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 1);
		}
		break;
	}

	case 61:
		_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 8, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[10], _globals._sequenceIndexes[9]);
		break;

	case 62:
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 8, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 63:
		_globals[kHoverCarDestination] = local._carLocations[local._selectedId];
		_scene->_nextSceneId = 504;
		break;

	default:
		break;
	}
}

static void room_505_parser() {
	if (player_said_1(press))
		local._nextButtonId = _action._activeAction._objectNameId;
	else if (player_said_2(return_to, inside_of_car))
		_scene->_nextSceneId = 504;
	else if (player_said_2(look, view_screen))
		_vm->_dialogs->show(50510);
	else if (player_said_2(look, control_panel))
		_vm->_dialogs->show(50511);
	else
		return;

	_action._inProgress = false;
}

void room_505_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._frame);
	s.syncAsSint16LE(local._nextButtonId);
	s.syncAsSint16LE(local._homeSelectedId);
	s.syncAsSint16LE(local._selectedId);
	s.syncAsSint16LE(local._activeCars);

	for (int i = 0; i < 9; i++)
		s.syncAsSint16LE(local._carLocations[i]);
}


void room_505_preload() {
	room_init_code_pointer = room_505_init;
	room_daemon_code_pointer = room_505_daemon;
	room_parser_code_pointer = room_505_parser;

	*player.series_name = '\0';
	section_5_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
