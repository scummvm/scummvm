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
#include "mads/madsv2/nebular/rooms/room102.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

Room105::Room105(RexNebularEngine *vm) : Room1xx(vm) {
	_explosionFl = false;
}

void Room105::synchronize(Common::Serializer &s) {
	Room1xx::synchronize(s);

	s.syncAsByte(_explosionFl);
}

void Room105::setup() {
	// Preloading has been skipped
	setPlayerSpritesPrefix();
	setAAName();
}

void Room105::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('m', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 4));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 0, 0, 0);

	if (_globals[kFishIn105]) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(48, 144));

		int idx = _scene->_dynamicHotspots.add(101, 348, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(56, 141), FACING_NORTHWEST);
	}

	if (_scene->_priorSceneId == 104)
		_game._player._playerPos = Common::Point(13, 97);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(116, 147);

	_game.loadQuoteSet(0x4A, 0x4B, 0x4C, 0x35, 0x34, 0);
	_explosionFl = false;

	sceneEntrySound();
}

void Room105::step() {
	if ((_game._player._playerPos == Common::Point(170, 87)) && (_game._trigger || !_explosionFl)) {
		_explosionFl = true;
		switch (_game._trigger) {
		case 0:
			_scene->_kernelMessages.reset();
			_scene->resetScene();
			_game._player._stepEnabled = false;
			_game._player._visible = false;

			_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('m', 0));
			_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('m', 2));
			_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('m', 3));
			_vm->_sound->command(33);
			_scene->clearSequenceList();
			_vm->_palette->refreshSceneColors();

			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 1);

			if (_game._storyMode >= STORYMODE_NICE)
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 8, 3);
			break;

		case 1:
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 9, 1, 0, 0);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], _globals._sequenceIndexes[0]);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 5, 7);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
		{
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 9, 0, 0, 0);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
			_scene->_sequences.addTimer(90, 3);
		}
		break;

		case 3:
			_vm->_dialogs->show(10507);
			_scene->_reloadSceneFlag = true;
			_scene->_sequences.addTimer(90, 4);
			break;

		default:
			break;
		}
	}

	if (_game._player._moving && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(170, 87), FACING_NONE);
		_scene->_rails.resetNext();
	}

	if ((_game._player._special > 0) && _game._player._stepEnabled)
		_game._player._stepEnabled = false;
}

void Room105::preActions() {
	if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_WESTERN_CLIFF_FACE))
		_game._player._walkOffScreenSceneId = 104;

	if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_OPEN_AREA_TO_SOUTH))
		_game._player._walkOffScreenSceneId = 107;

	if (_action.isObject(NOUN_MINE) && (_action.isAction(VERB_TALKTO) || _action.isAction(VERB_LOOK)))
		_game._player._needToWalk = false;
}

void Room105::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10512);
	else if (_action.isAction(VERB_TAKE, NOUN_DEAD_FISH) && _globals[kFishIn105]) {
		if (_game._objects.isInInventory(OBJ_DEAD_FISH)) {
			int randVal = _vm->getRandomNumber(74, 76);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_game._objects.addToInventory(OBJ_DEAD_FISH);
			_globals[kFishIn105] = false;
			_vm->_dialogs->showItem(OBJ_DEAD_FISH, 802, 0);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_WESTERN_CLIFF_FACE))
		_vm->_dialogs->show(10501);
	else if (_action.isAction(VERB_LOOK, NOUN_CLIFF_FACE))
		_vm->_dialogs->show(10502);
	else if (_action.isAction(VERB_LOOK, NOUN_OCEAN_FLOOR))
		_vm->_dialogs->show(10503);
	else if (_action.isAction(VERB_LOOK, NOUN_MEDICAL_WASTE))
		_vm->_dialogs->show(10504);
	else if (_action.isAction(VERB_TAKE, NOUN_MEDICAL_WASTE))
		_vm->_dialogs->show(10505);
	else if (_action.isAction(VERB_LOOK, NOUN_MINE))
		_vm->_dialogs->show(10506);
	else if (_action.isAction(VERB_LOOK, NOUN_DEAD_FISH))
		_vm->_dialogs->show(10508);
	else if (_action.isAction(VERB_LOOK, NOUN_SURFACE))
		_vm->_dialogs->show(10509);
	else if (_action.isAction(VERB_LOOK, NOUN_OPEN_AREA_TO_SOUTH))
		_vm->_dialogs->show(10510);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCKS))
		_vm->_dialogs->show(10511);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
