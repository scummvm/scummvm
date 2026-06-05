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

void Scene705::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene705::synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);
}

void Scene705::handleBottleInterface() {
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

void Scene705::setBottleSequence() {
	_scene->_userInterface.setup(kInputBuildingSentences);
	_game._player._stepEnabled = false;
	_scene->_sequences.remove(_globals._sequenceIndexes[3]);
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
	_scene->loadAnimation(formAnimName('F', -1), 90);
}

void Scene705::handleFillBottle(int quote) {
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

void Scene705::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 0));

	_game._player._visible = false;

	if (_scene->_priorSceneId == 706) {
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._stepEnabled = false;
		_scene->_sequences.addTimer(1, 80);
		_vm->_sound->command(28);
	} else
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BOTTLE);

	_game.loadQuoteSet(0x311, 0x312, 0x313, 0x314, 0x315, 0);
	_dialog1.setup(0x98, 0x311, 0x312, 0x313, 0x314, 0x315, 0);
	sceneEntrySound();
}

void Scene705::step() {
	switch (_game._trigger) {
	case 70:
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71:
	{
		int syncIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
		_game._player._stepEnabled = true;
	}
	break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 80:
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
		break;

	case 81:
	{
		_vm->_sound->command(19);
		int syncIdx = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
		_game._player._stepEnabled = true;
	}
	break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 90:
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.addTimer(30, 91);
		break;

	case 91:
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
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene705::actions() {
	if (_game._screenObjects._inputMode == kInputConversation)
		handleFillBottle(_action._activeAction._verbId);
	else if (_action.isAction(VERB_STEER_TOWARDS, NOUN_OPEN_WATER_TO_SOUTH)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_vm->_sound->command(18);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
			_scene->_nextSceneId = 704;
			_game._player._stepEnabled = true;
		}
		break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_CLIMB_THROUGH, NOUN_WINDOW)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 16);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 16);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_scene->_nextSceneId = 706;
			_game._player._stepEnabled = true;
		}
		break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_FILL, NOUN_BOTTLE, NOUN_WATER) || _action.isAction(VERB_PUT, NOUN_BOTTLE, NOUN_WATER)) {
		if (_globals[kBottleStatus] != 4) {
			handleBottleInterface();
			_dialog1.start();
		} else
			_vm->_dialogs->show(70323);
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_WATER))
		_vm->_dialogs->show(70511);
	else if (_action.isAction(VERB_LOOK, NOUN_VOLCANO_RIM))
		_vm->_dialogs->show(70512);
	else if (_action.isAction(VERB_LOOK, NOUN_OPEN_WATER_TO_SOUTH))
		_vm->_dialogs->show(70513);
	else if (_action.isAction(VERB_LOOK, NOUN_SKY))
		_vm->_dialogs->show(70514);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING))
		_vm->_dialogs->show(70515);
	else if (_action.isAction(VERB_LOOK, NOUN_WINDOW))
		_vm->_dialogs->show(70516);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
