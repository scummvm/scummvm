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

Scene320::Scene320(RexNebularEngine *vm) : Scene300s(vm) {
	_blinkFl = false;
	_flippedFl = false;

	_buttonId = -1;
	_lastFrame = -1;
	_leftItemId = -1;
	_posX = -1;
	_rightItemId = -1;
}

void Scene320::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsByte(_blinkFl);
	s.syncAsByte(_flippedFl);

	s.syncAsSint32LE(_buttonId);
	s.syncAsSint32LE(_lastFrame);
	s.syncAsSint32LE(_leftItemId);
	s.syncAsSint32LE(_posX);
	s.syncAsSint32LE(_rightItemId);
}

void Scene320::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene320::setRightView(int view) {
	if (_rightItemId < 8) _scene->_sequences.remove(_globals._sequenceIndexes[10]);

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

	_globals[kRightView320] = _rightItemId = view;
}

void Scene320::setLeftView(int view) {
	if (_leftItemId < 10)
		_scene->_sequences.remove(_globals._sequenceIndexes[0]);

	if (view != 10) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[view], false, 6, 0, 0, 18);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 0);
		if (!_blinkFl)
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 2, 2);
	}

	_leftItemId = view;
}

void Scene320::handleButtons() {
	switch (_action._activeAction._objectNameId) {
	case 0x2DD:
		_buttonId = 5;
		break;

	case 0x2DE:
		_buttonId = 4;
		break;

	case 0x2E0:
		_buttonId = 6;
		break;

	case 0x2E1:
		_buttonId = 7;
		break;

	case 0x2E2:
		_buttonId = 8;
		break;

	case 0x2E3:
		_buttonId = 9;
		break;

	case 0x2E4:
		_buttonId = 10;
		break;

	case 0x2E5:
		_buttonId = 11;
		break;

	case 0x2E6:
		_buttonId = 12;
		break;

	case 0x2E7:
		_buttonId = 13;
		break;

	case 0x2E8:
		_buttonId = 0;
		break;

	case 0x2E9:
		_buttonId = 1;
		break;

	case 0x2EA:
		_buttonId = 2;
		break;

	case 0x2EB:
		_buttonId = 3;
		break;

	default:
		break;
	}

	if (_buttonId <= 3) {
		_posX = (8 * _buttonId) - 2;
		_flippedFl = true;
	} else if (_buttonId <= 5) {
		_posX = (13 * _buttonId) - 14;
		_flippedFl = true;
	} else {
		_posX = (8 * _buttonId) + 98;
		_flippedFl = false;
	}
}

void Scene320::enter() {
	_blinkFl = true;
	_rightItemId = 8;
	_leftItemId = 10;
	_lastFrame = 0;

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

	sceneEntrySound();
}

void Scene320::step() {
	if (_scene->_animation[0] != nullptr) {
		if (_lastFrame != _scene->_animation[0]->getCurrentFrame()) {
			_lastFrame = _scene->_animation[0]->getCurrentFrame();
			switch (_lastFrame) {
			case 95:
				_blinkFl = true;
				setLeftView(9);
				_vm->_sound->command(41);
				break;

			case 139:
				_blinkFl = false;
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
				_blinkFl = true;
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

void Scene320::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(32011);
	else if ((_action.isAction(VERB_PRESS) || _action.isAction(VERB_PUSH)) &&
		(_action.isObject(NOUN_LEFT_1_KEY) || _action.isObject(NOUN_LEFT_2_KEY) || _action.isObject(NOUN_LEFT_3_KEY) || _action.isObject(NOUN_LEFT_4_KEY) ||
			_action.isObject(NOUN_GREEN_BUTTON) || _action.isObject(NOUN_RED_BUTTON) || _action.isObject(NOUN_RIGHT_1_KEY) || _action.isObject(NOUN_RIGHT_2_KEY) ||
			_action.isObject(NOUN_RIGHT_3_KEY) || _action.isObject(NOUN_RIGHT_4_KEY) || _action.isObject(NOUN_RIGHT_5_KEY) || _action.isObject(NOUN_RIGHT_6_KEY) ||
			_action.isObject(NOUN_RIGHT_7_KEY) || _action.isObject(NOUN_RIGHT_8_KEY)
			)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			handleButtons();
			_globals._sequenceIndexes[18] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[18], _flippedFl, 4, 2, 0, 0);
			_scene->_sequences.setScale(_globals._sequenceIndexes[18], 60);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[18], Common::Point(_posX, 170));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[18], 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[18], SEQUENCE_TRIGGER_LOOP, 0, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[18], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			if (_buttonId >= 6) {
				_vm->_sound->command(60);
				setRightView(_buttonId - 6);
			}
			if (_buttonId == 4) {
				_vm->_sound->command(38);
				if (_leftItemId == 3)
					setLeftView(0);
				else
					setLeftView(3);
			}
			if (_buttonId == 5) {
				_vm->_sound->command(38);
				if (_leftItemId == 1)
					setLeftView(2);
				else
					setLeftView(1);
			}
			if (_buttonId <= 3) {
				_vm->_sound->command(60);
				setLeftView(_buttonId + 5);
			}
			break;

		case 2:
			_game._player._stepEnabled = true;
			if (_buttonId == 5) {
				if (_leftItemId == 2) {
					_game._player._stepEnabled = false;
					setRightView(8);
					setLeftView(10);
					_scene->_kernelMessages.reset();
					_scene->resetScene();
					_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('m', 2));
					_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('m', 4));
					_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('m', 9));
					_blinkFl = false;
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
	} else if (_action.isAction(VERB_LEAVE, NOUN_SECURITY_STATION))
		_scene->_nextSceneId = 311;
	else if (_action.isAction(VERB_LOOK, NOUN_RIGHT_MONITOR))
		_vm->_dialogs->show(32001);
	else if (_action.isAction(VERB_LOOK, NOUN_LEFT_MONITOR))
		_vm->_dialogs->show(32002);
	else if (_action.isAction(VERB_LOOK, NOUN_DESK))
		_vm->_dialogs->show(32003);
	else if (_action.isAction(VERB_LOOK, NOUN_SECURITY_STATION))
		_vm->_dialogs->show(32004);
	else if (_action.isAction(VERB_LOOK, NOUN_MUG))
		_vm->_dialogs->show(32005);
	else if (_action.isAction(VERB_LOOK, NOUN_DOUGHNUT))
		_vm->_dialogs->show(32006);
	else if (_action.isAction(VERB_LOOK, NOUN_MAGAZINE))
		_vm->_dialogs->show(32007);
	else if (_action.isAction(VERB_LOOK, NOUN_PAPER_FOOTBALL))
		_vm->_dialogs->show(32008);
	else if (_action.isAction(VERB_LOOK, NOUN_NEWSPAPER))
		_vm->_dialogs->show(32009);
	else if (_action.isAction(VERB_LOOK, NOUN_CLIPBOARD))
		_vm->_dialogs->show(32010);
	else if (_action.isAction(VERB_TAKE, NOUN_MUG))
		_vm->_dialogs->show(32012);
	else if (_action.isAction(VERB_TAKE, NOUN_CLIPBOARD))
		_vm->_dialogs->show(32013);
	else if (_action.isAction(VERB_TAKE, NOUN_DOUGHNUT) || _action.isAction(VERB_EAT, NOUN_DOUGHNUT))
		_vm->_dialogs->show(32014);
	else if (_action.isAction(VERB_TAKE, NOUN_PAPER_FOOTBALL))
		_vm->_dialogs->show(32015);
	else if (_action.isAction(VERB_TAKE, NOUN_MAGAZINE))
		_vm->_dialogs->show(32016);
	else if (_action.isAction(VERB_TAKE, NOUN_NEWSPAPER))
		_vm->_dialogs->show(32017);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
