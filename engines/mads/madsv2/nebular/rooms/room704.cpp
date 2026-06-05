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

Scene704::Scene704(RexNebularEngine *vm) : Scene7xx(vm) {
	_bottleHotspotId = -1;
	_boatCurrentFrame = -1;
	_animationMode = -1;
	_boatDirection = -1;

	_takeBottleFl = false;
}

void Scene704::synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);

	s.syncAsSint16LE(_bottleHotspotId);
	s.syncAsSint16LE(_boatCurrentFrame);
	s.syncAsSint16LE(_animationMode);
	s.syncAsSint16LE(_boatDirection);

	s.syncAsByte(_takeBottleFl);
}

void Scene704::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
	_scene->addActiveVocab(NOUN_BOTTLE);
	_scene->addActiveVocab(VERB_LOOK_AT);
}

void Scene704::handleBottleInterface() {
	switch (_globals[kBottleStatus]) {
	case 0:
		_dialog1.write(0x311, true);
		_dialog1.write(0x312, true);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 1:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, true);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 2:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, false);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 3:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, false);
		_dialog1.write(0x313, false);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	default:
		break;
	}
}

void Scene704::setBottleSequence() {
	_scene->_userInterface.setup(kInputBuildingSentences);
	_game._player._stepEnabled = false;
	if (_boatDirection == 2)
		_animationMode = 6;
	else
		_animationMode = 7;
}

void Scene704::handleFillBottle(int quote) {
	switch (quote) {
	case 0x311:
		_globals[kBottleStatus] = 1;
		setBottleSequence();
		break;

	case 0x312:
		_globals[kBottleStatus] = 2;
		setBottleSequence();
		break;

	case 0x313:
		_globals[kBottleStatus] = 3;
		setBottleSequence();
		break;

	case 0x314:
		_globals[kBottleStatus] = 4;
		setBottleSequence();
		break;

	case 0x315:
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	default:
		break;
	}
}

void Scene704::enter() {
	if (_game._objects[OBJ_BOTTLE]._roomNumber == _scene->_currentSceneId) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		if (_scene->_priorSceneId == 705) {
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(123, 125));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		} else {
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(190, 122));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		}
		int idx = _scene->_dynamicHotspots.add(NOUN_BOTTLE, VERB_LOOK_AT, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_bottleHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
	}

	_game._player._visible = false;
	_takeBottleFl = false;
	_boatCurrentFrame = -1;

	if (_scene->_priorSceneId == 705) {
		_game._player._stepEnabled = false;
		_animationMode = 2;
		_boatDirection = 2;
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(36);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._stepEnabled = false;
		_boatDirection = 1;
		_scene->loadAnimation(formAnimName('A', -1));
	} else if (_boatDirection == 1) {
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(8);
	} else if (_boatDirection == 2) {
		if (_game._objects[OBJ_BOTTLE]._roomNumber == _scene->_currentSceneId) {
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(123, 125));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		}
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(57);
	}

	if (_scene->_roomChanged)
		_globals[kMonsterAlive] = false;

	_game.loadQuoteSet(0x311, 0x312, 0x313, 0x314, 0x315, 0);
	_dialog1.setup(0x98, 0x311, 0x312, 0x313, 0x314, 0x315, 0);

	sceneEntrySound();
	_vm->_sound->command(28);
}

void Scene704::step() {
	if (_scene->_animation[0] != nullptr) {
		if (_scene->_animation[0]->getCurrentFrame() != _boatCurrentFrame) {
			_boatCurrentFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			switch (_boatCurrentFrame) {
			case 10:
				switch (_animationMode) {
				case 1:
					nextFrame = 10;
					break;
				case 5:
					nextFrame = 74;
					break;
				case 7:
					_animationMode = 0;
					nextFrame = 92;
					break;
				default:
					if (!_game._player._stepEnabled)
						_game._player._stepEnabled = true;

					nextFrame = 8;
					break;
				}
				break;

			case 36:
				if (_animationMode != 2)
					_scene->_nextSceneId = 705;
				break;

			case 59:
				switch (_animationMode) {
				case 3:
					nextFrame = 59;
					break;

				case 4:
					nextFrame = 65;
					break;

				case 6:
					_animationMode = 0;
					nextFrame = 83;
					break;

				default:
					if (!_game._player._stepEnabled) {
						_game._player._stepEnabled = true;
					}
					nextFrame = 57;
					break;
				}
				break;

			case 65:
				_scene->_nextSceneId = 703;
				break;

			case 74:
				nextFrame = 10;
				break;

			case 83:
				nextFrame = 59;
				break;

			case 90:
				if (_takeBottleFl) {
					_scene->_sequences.remove(_globals._sequenceIndexes[1]);
					_scene->_dynamicHotspots.remove(_bottleHotspotId);
					_game._objects.addToInventory(OBJ_BOTTLE);
					_vm->_sound->command(15);
					_vm->_dialogs->showItem(OBJ_BOTTLE, 70415);
				}
				break;

			case 92:
				nextFrame = 57;
				if (!_game._player._stepEnabled && !_takeBottleFl) {
					_scene->_sequences.addTimer(30, 70);
					_game._player._stepEnabled = true;
				}
				break;

			case 98:
				if (_takeBottleFl) {
					_scene->_sequences.remove(_globals._sequenceIndexes[1]);
					_scene->_dynamicHotspots.remove(_bottleHotspotId);
					_game._objects.addToInventory(OBJ_BOTTLE);
					_vm->_sound->command(15);
					_vm->_dialogs->showItem(OBJ_BOTTLE, 70415);
				}
				break;

			case 101:
				nextFrame = 8;
				if (!_game._player._stepEnabled && !_takeBottleFl) {
					_scene->_sequences.addTimer(30, 70);
					_game._player._stepEnabled = true;
				}
				break;

			default:
				break;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_boatCurrentFrame = nextFrame;
			}
		}
	}

	if (_game._trigger == 70) {
		switch (_globals[kBottleStatus]) {
		case 0:
			_vm->_dialogs->show(432);
			break;

		case 1:
			_vm->_dialogs->show(70324);
			break;

		case 2:
			_vm->_dialogs->show(70325);
			break;

		case 3:
			_vm->_dialogs->show(70326);
			break;

		case 4:
			_vm->_dialogs->show(70327);
			break;

		default:
			break;
		}
	}
}

void Scene704::actions() {
	if (_game._screenObjects._inputMode == kInputConversation)
		handleFillBottle(_action._activeAction._verbId);
	else if (_action.isAction(VERB_STEER_TOWARDS, NOUN_OPEN_WATER_TO_SOUTH)) {
		_game._player._stepEnabled = false;
		if (_boatDirection == 1)
			_animationMode = 5;
		else
			_animationMode = 3;
	} else if (_action.isAction(VERB_STEER_TOWARDS, NOUN_BUILDING_TO_NORTH)) {
		_game._player._stepEnabled = false;
		if (_boatDirection == 2)
			_animationMode = 4;
		else
			_animationMode = 1;
	} else if (_action.isAction(VERB_TAKE, NOUN_BOTTLE)) {
		if (!_game._objects.isInInventory(OBJ_BOTTLE)) {
			_game._player._stepEnabled = false;
			_takeBottleFl = true;
			if (_boatDirection == 2) {
				_animationMode = 6;
			} else {
				_animationMode = 7;
			}
		}
	} else if (_action.isAction(VERB_PUT, NOUN_BOTTLE, NOUN_WATER) || _action.isAction(VERB_FILL, NOUN_BOTTLE, NOUN_WATER)) {
		if (_game._objects.isInInventory(OBJ_BOTTLE)) {
			if (_globals[kBottleStatus] != 4) {
				_takeBottleFl = false;
				handleBottleInterface();
				_dialog1.start();
			} else
				_vm->_dialogs->show(70323);
		}
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_WATER))
		_vm->_dialogs->show(70410);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING_TO_NORTH)) {
		if (_game._visitedScenes.exists(710))
			_vm->_dialogs->show(70411);
		else
			_vm->_dialogs->show(70412);
	} else if (_action.isAction(VERB_LOOK, NOUN_VOLCANO_RIM))
		_vm->_dialogs->show(70413);
	else if (_action.isAction(VERB_LOOK, NOUN_BOTTLE) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(70414);
	else if (_action.isAction(VERB_LOOK, NOUN_OPEN_WATER_TO_SOUTH))
		_vm->_dialogs->show(70416);
	else if (_action.isAction(VERB_LOOK, NOUN_SKY))
		_vm->_dialogs->show(70417);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
