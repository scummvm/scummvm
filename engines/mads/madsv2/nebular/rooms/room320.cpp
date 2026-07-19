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
#include "mads/madsv2/nebular/rooms/section3.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _blinkFl;
	bool _flippedFl;
	int32 _buttonId;
	int32 _lastFrame;
	int32 _leftItemId;
	int32 _posX;
	int32 _rightItemId;
};

static Scratch local;


static void setRightView(int view) {
	if (local._rightItemId < 8) _scene->_sequences.remove(_globals._sequenceIndexes[10]);

	int spriteNum;
	switch (view) {
	case 0:
		spriteNum = 16;
		break;

	case 1:
		spriteNum = 14;
		break;

	case 2:
		spriteNum = 17;
		break;

	case 3:
		spriteNum = 15;
		break;

	default:
		spriteNum = view + 6;
		break;
	}

	if (view != 8) {
		_globals._sequenceIndexes[10] = _scene->_sequences.startCycle(_globals._spriteIndexes[spriteNum], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 0);
	}

	_globals[kRightView320] = local._rightItemId = view;
}

static void setLeftView(int view) {
	if (local._leftItemId < 10)
		_scene->_sequences.remove(_globals._sequenceIndexes[0]);

	if (view != 10) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[view], false, 6, 0, 0, 18);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 0);
		if (!local._blinkFl)
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 2, 2);
	}

	local._leftItemId = view;
}

static void handleButtons() {
	switch (_action._activeAction._objectNameId) {
	case 0x2DD:
		local._buttonId = 5;
		break;

	case 0x2DE:
		local._buttonId = 4;
		break;

	case 0x2E0:
		local._buttonId = 6;
		break;

	case 0x2E1:
		local._buttonId = 7;
		break;

	case 0x2E2:
		local._buttonId = 8;
		break;

	case 0x2E3:
		local._buttonId = 9;
		break;

	case 0x2E4:
		local._buttonId = 10;
		break;

	case 0x2E5:
		local._buttonId = 11;
		break;

	case 0x2E6:
		local._buttonId = 12;
		break;

	case 0x2E7:
		local._buttonId = 13;
		break;

	case 0x2E8:
		local._buttonId = 0;
		break;

	case 0x2E9:
		local._buttonId = 1;
		break;

	case 0x2EA:
		local._buttonId = 2;
		break;

	case 0x2EB:
		local._buttonId = 3;
		break;

	default:
		break;
	}

	if (local._buttonId <= 3) {
		local._posX = (8 * local._buttonId) - 2;
		local._flippedFl = true;
	} else if (local._buttonId <= 5) {
		local._posX = (13 * local._buttonId) - 14;
		local._flippedFl = true;
	} else {
		local._posX = (8 * local._buttonId) + 98;
		local._flippedFl = false;
	}
}

static void room_320_init() {
	local._blinkFl = true;
	local._rightItemId = 8;
	local._leftItemId = 10;
	local._lastFrame = 0;

	for (int i = 0; i < 10; i++)
		_globals._spriteIndexes[i] = _scene->_sprites.addSprites(formAnimName('M', i));

	for (int i = 0; i < 8; i++)
		_globals._spriteIndexes[10 + i] = _scene->_sprites.addSprites(formAnimName('N', i));

	_globals._spriteIndexes[18] = _scene->_sprites.addSprites("*REXHAND");
	_game._player._visible = false;

	setRightView(_globals[kRightView320]);
	setLeftView(0);

	_vm->_palette->setEntry(252, 63, 30, 20);
	_vm->_palette->setEntry(253, 45, 15, 10);

	section_3_music();
}

static void room_320_daemon() {
	if (_scene->_animation[0] != nullptr) {
		if (local._lastFrame != _scene->_animation[0]->getCurrentFrame()) {
			local._lastFrame = _scene->_animation[0]->getCurrentFrame();
			switch (local._lastFrame) {
			case 95:
				local._blinkFl = true;
				setLeftView(9);
				_vm->_sound->command(41);
				break;

			case 139:
				local._blinkFl = false;
				setLeftView(9);
				break;

			case 191:
				_scene->_kernelMessages.add(Common::Point(1, 1), 0xFDFC, 0, 0, 60, _game.getQuote(0xFE));
				break;

			case 417:
			case 457:
				_vm->_screen->_shakeCountdown = 40;
				_vm->_sound->command(59);
				break;

			case 430:
				local._blinkFl = true;
				setLeftView(4);
				break;

			default:
				break;
			}
		}
	}

	if (_game._trigger == 70) {
		_globals[kAfterHavoc] = true;
		_globals[kTeleporterRoom + 1] = 351;
		_scene->_nextSceneId = 361;
	}
}

static void room_320_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(32011);
	else if ((_action.isAction(words_press) || _action.isAction(words_push)) &&
		(_action.isObject(words_left_1_key) || _action.isObject(words_left_2_key) || _action.isObject(words_left_3_key) || _action.isObject(words_left_4_key) ||
			_action.isObject(words_green_button) || _action.isObject(words_red_button) || _action.isObject(words_right_1_key) || _action.isObject(words_right_2_key) ||
			_action.isObject(words_right_3_key) || _action.isObject(words_right_4_key) || _action.isObject(words_right_5_key) || _action.isObject(words_right_6_key) ||
			_action.isObject(words_right_7_key) || _action.isObject(words_right_8_key)
			)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			handleButtons();
			_globals._sequenceIndexes[18] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[18], local._flippedFl, 4, 2, 0, 0);
			_scene->_sequences.setScale(_globals._sequenceIndexes[18], 60);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[18], Common::Point(local._posX, 170));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[18], 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[18], SEQUENCE_TRIGGER_LOOP, 0, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[18], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			if (local._buttonId >= 6) {
				_vm->_sound->command(60);
				setRightView(local._buttonId - 6);
			}
			if (local._buttonId == 4) {
				_vm->_sound->command(38);
				if (local._leftItemId == 3)
					setLeftView(0);
				else
					setLeftView(3);
			}
			if (local._buttonId == 5) {
				_vm->_sound->command(38);
				if (local._leftItemId == 1)
					setLeftView(2);
				else
					setLeftView(1);
			}
			if (local._buttonId <= 3) {
				_vm->_sound->command(60);
				setLeftView(local._buttonId + 5);
			}
			break;

		case 2:
			_game._player._stepEnabled = true;
			if (local._buttonId == 5) {
				if (local._leftItemId == 2) {
					_game._player._stepEnabled = false;
					setRightView(8);
					setLeftView(10);
					_scene->_kernelMessages.reset();
					_scene->resetScene();
					_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('m', 2));
					_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('m', 4));
					_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('m', 9));
					local._blinkFl = false;
					setLeftView(2);
					_game.loadQuoteSet(0xFE, 0);
					_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
					_scene->loadAnimation(formAnimName('a', -1), 70);
					_vm->_sound->command(17);
				}
			}
			break;

		default:
			break;
		}
	} else if (_action.isAction(words_leave, words_security_station))
		_scene->_nextSceneId = 311;
	else if (_action.isAction(words_look, words_right_monitor))
		_vm->_dialogs->show(32001);
	else if (_action.isAction(words_look, words_left_monitor))
		_vm->_dialogs->show(32002);
	else if (_action.isAction(words_look, words_desk))
		_vm->_dialogs->show(32003);
	else if (_action.isAction(words_look, words_security_station))
		_vm->_dialogs->show(32004);
	else if (_action.isAction(words_look, words_mug))
		_vm->_dialogs->show(32005);
	else if (_action.isAction(words_look, words_doughnut))
		_vm->_dialogs->show(32006);
	else if (_action.isAction(words_look, words_magazine))
		_vm->_dialogs->show(32007);
	else if (_action.isAction(words_look, words_paper_football))
		_vm->_dialogs->show(32008);
	else if (_action.isAction(words_look, words_newspaper))
		_vm->_dialogs->show(32009);
	else if (_action.isAction(words_look, words_clipboard))
		_vm->_dialogs->show(32010);
	else if (_action.isAction(words_take, words_mug))
		_vm->_dialogs->show(32012);
	else if (_action.isAction(words_take, words_clipboard))
		_vm->_dialogs->show(32013);
	else if (_action.isAction(words_take, words_doughnut) || _action.isAction(words_eat, words_doughnut))
		_vm->_dialogs->show(32014);
	else if (_action.isAction(words_take, words_paper_football))
		_vm->_dialogs->show(32015);
	else if (_action.isAction(words_take, words_magazine))
		_vm->_dialogs->show(32016);
	else if (_action.isAction(words_take, words_newspaper))
		_vm->_dialogs->show(32017);
	else
		return;

	_action._inProgress = false;
}

void room_320_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._blinkFl);
	s.syncAsByte(local._flippedFl);

	s.syncAsSint32LE(local._buttonId);
	s.syncAsSint32LE(local._lastFrame);
	s.syncAsSint32LE(local._leftItemId);
	s.syncAsSint32LE(local._posX);
	s.syncAsSint32LE(local._rightItemId);
}

void room_320_preload() {
	room_init_code_pointer = room_320_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_320_parser;
	room_daemon_code_pointer = room_320_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
