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
#include "mads/madsv2/nebular/rooms/room201.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

void Scene205::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_CHICKEN);
	_scene->addActiveVocab(NOUN_PIRANHA);
}

Scene205::Scene205(RexNebularEngine *vm) : Scene2xx(vm) {
	_lastFishTime = _scene->_frameStartTime;
	_chickenTime = _scene->_frameStartTime;

	_beingKicked = false;
	_kernelMessage = -1;
}

void Scene205::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_beingKicked);
	s.syncAsSint16LE(_kernelMessage);
}

void Scene205::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('f', -1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('p', -1));

	if (_globals[kSexOfRex] == SEX_MALE)
		_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 1));

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 10, 0, 0, 3);
	int idx = _scene->_dynamicHotspots.add(73, 13, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(162, 120), FACING_NORTHEAST);

	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 0);
	idx = _scene->_dynamicHotspots.add(73, 13, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(162, 120), FACING_NORTHEAST);

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 0, 0, 0);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 11);

	if (!_game._visitedScenes._sceneRevisited) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 7, 1, 0, 0);
		idx = _scene->_dynamicHotspots.add(269, 13, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(49, 86), FACING_NORTH);
	}

	if (_game._objects[12]._roomNumber == 205) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 11);
	} else {
		_scene->_hotspots.activate(450, false);
	}

	_beingKicked = false;
	_game.loadQuoteSet(0x6B, 0x70, 0x71, 0x72, 0x5A, 0x74, 0x75, 0x76, 0x77, 0x78, 0x73, 0x79, 0x7A, 0x7B, 0x7C,
		0x7D, 0x7E, 0x7F, 0x80, 0xAC, 0xAD, 0xAE, 0x6C, 0x6D, 0x6E, 0x6F, 0x2, 0);
	_dialog1.setup(0x2A, 0x5A, 0x78, 0x74, 0x75, 0x76, 0x77, 0);

	if (!_game._visitedScenes._sceneRevisited)
		_dialog1.set(0x5A, 0x74, 0x75, 0x77, 0);

	_vm->_palette->setEntry(250, 63, 50, 20);
	_vm->_palette->setEntry(251, 50, 40, 15);
	_vm->_palette->setEntry(252, 63, 63, 40);
	_vm->_palette->setEntry(253, 50, 50, 30);

	_chickenTime = _vm->_game->_scene._frameStartTime;

	if (_globals[kSexOfRex] == SEX_FEMALE)
		_scene->_kernelMessages.initRandomMessages(3,
			Common::Rect(195, 99, 264, 134), 13, 2, 0xFDFC, 60,
			108, 108, 109, 109, 110, 110, 111, 108, 0);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(99, 152);

	if (_globals[kSexOfRex] != SEX_MALE) {
		_scene->loadAnimation(formAnimName('a', -1));
		_scene->_animation[0]->_resetFlag = true;
	} else {
		_beingKicked = true;
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 6, 73);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 11, 74);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
	}
	sceneEntrySound();
}

void Scene205::step() {
	if (_globals[kSexOfRex] == SEX_FEMALE) {
		_scene->_kernelMessages.randomServer();

		if (_vm->_game->_scene._frameStartTime >= _chickenTime) {
			int chanceMinor = _scene->_kernelMessages.checkRandom() + 1;
			if (_scene->_kernelMessages.generateRandom(100, chanceMinor))
				_vm->_sound->command(28);

			_chickenTime = _vm->_game->_scene._frameStartTime + 2;
		}
	}

	if (_vm->_game->_scene._frameStartTime - _lastFishTime > 1300) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(
			_globals._spriteIndexes[6], false, 5, 1, 0, 0);
		int idx = _scene->_dynamicHotspots.add(269, 13, _globals._sequenceIndexes[6],
			Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(49, 86), FACING_NORTH);
		_lastFishTime = _vm->_game->_scene._frameStartTime;
	}

	if (_game._trigger == 73)
		_scene->_kernelMessages.add(Common::Point(160, 68), 0xFBFA, 32, 0, 60, _game.getQuote(112));

	if (_game._trigger == 74) {
		_vm->_sound->command(26);
		_scene->_kernelMessages.add(Common::Point(106, 90), 0x1110, 32, 0, 60, _game.getQuote(113));
	}

	if (_game._trigger == 71) {
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 2);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -2, -2);
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(160, 68), 0xFBFA, 32, 72, 180, _game.getQuote(114));
	}

	if (_game._trigger == 72)
		_scene->_nextSceneId = 211;
}

void Scene205::handleWomanSpeech(int quote) {
	_kernelMessage = _scene->_kernelMessages.add(Common::Point(186, 27), 0xFBFA, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(quote));
}

void Scene205::actions() {
	if (_game._screenObjects._inputMode == kInputConversation) {
		if (_kernelMessage >= 0)
			_scene->_kernelMessages.remove(_kernelMessage);
		_kernelMessage = -1;

		if (_game._trigger == 0) {
			_game._player._stepEnabled = false;
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 18, 1, 120, _game.getQuote(_action._activeAction._verbId));
		} else {
			if ((_game._trigger > 1) || (_action._activeAction._verbId != 0x76))
				_game._player._stepEnabled = true;

			switch (_action._activeAction._verbId) {
			case 0x5A:
				handleWomanSpeech(0x7A);
				_dialog1.write(0x78, true);
				_dialog1.write(0x5A, false);
				break;

			case 0x74:
				handleWomanSpeech(0x7C);
				_dialog1.write(0x74, false);
				_dialog1.write(0x76, true);
				break;

			case 0x75:
			case 0x78:
				handleWomanSpeech(0x7B);
				_dialog1.write(_action._activeAction._verbId, false);
				_vm->_dialogs->show(20501);
				break;

			case 0x76:
				if (_game._trigger == 1) {
					handleWomanSpeech(0x7D);
					_scene->_sequences.addTimer(120, 2);
				} else if (_game._trigger == 2) {
					handleWomanSpeech(0x7E);
					_dialog1.write(0x76, false);
					_globals[kChickenPermitted] = true;
				}
				break;

			case 0x77:
				_scene->_kernelMessages.add(Common::Point(186, 27), 0xFBFA, 0, 0, 120, _game.getQuote(0x7F));
				_scene->_userInterface.setup(kInputBuildingSentences);
				break;

			default:
				break;
			}

			if (_action._activeAction._verbId != 0x77)
				_dialog1.start();
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(20502);
	else if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_OPPOSITE_BANK))
		_vm->_dialogs->show(20518);
	else if (_action.isAction(VERB_TALKTO, NOUN_NATIVE_WOMAN)) {
		if (_game._trigger == 0) {
			_game._player._stepEnabled = false;
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 18, 1, 120, _game.getQuote(0x73));
		} else if (_game._trigger == 1) {
			_game._player._stepEnabled = true;
			handleWomanSpeech(0x79);
			_dialog1.write(0x5A, true);
			_dialog1.write(0x75, true);
			_dialog1.start();
		}
	} else if (_action.isAction(VERB_GIVE, NOUN_NATIVE_WOMAN) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		if (_game._trigger == 0) {
			_game._player._stepEnabled = false;
			int rndVal = _vm->getRandomNumber(0xAC, 0xAE);
			_scene->_kernelMessages.add(Common::Point(186, 27), 0xFBFA, 32, 1, 120, _game.getQuote(rndVal));
		} else if (_game._trigger == 1)
			_game._player._stepEnabled = true;
	} else if (_action.isAction(VERB_WALKTO, NOUN_OPPOSITE_BANK)) {
		if (_game._trigger == 0) {
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_vm->_palette->lock();
			_scene->_kernelMessages.reset();
			_game._player.removePlayerSprites();
			_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('a', 0));
			_vm->_palette->refreshSceneColors();
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], -1);
			_vm->_sound->command(27);
		} else if (_game._trigger == 1) {
			if (_scene->_animation[0] != nullptr)
				_scene->_animation[0]->resetSpriteSetsCount();

			_vm->_dialogs->show(20516);
			_scene->_reloadSceneFlag = true;
		}
	} else {
		if (_action.isAction(VERB_WALK_DOWN, NOUN_PATH_TO_SOUTH))
			_scene->_nextSceneId = 210;

		if (_action.isAction(VERB_WALKTO, NOUN_FIRE_PIT) || _action.isAction(VERB_WALKTO, NOUN_CHICKEN_ON_SPIT)) {
			if (_game._objects.isInRoom(OBJ_CHICKEN)) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x6B));
			}
		} else if (_action.isAction(VERB_TAKE, NOUN_CHICKEN_ON_SPIT) && _globals[kChickenPermitted] && _game._objects.isInRoom(OBJ_CHICKEN)) {
			_game._objects.addToInventory(OBJ_CHICKEN);
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_scene->_hotspots.activate(NOUN_CHICKEN_ON_SPIT, false);
			_vm->_dialogs->showItem(OBJ_CHICKEN, 812);
		} else if (_action.isAction(VERB_TAKE, NOUN_CHICKEN_ON_SPIT) && (!_globals[kChickenPermitted]))
			_scene->_kernelMessages.add(Common::Point(186, 27), 0xFBFA, 32, 0, 120, _game.getQuote(0x80));
		else if (_action.isAction(VERB_LOOK, NOUN_NATIVE_WOMAN))
			_vm->_dialogs->show(20503);
		else if (_action.isAction(VERB_LOOK, NOUN_HUT))
			_vm->_dialogs->show(20504);
		else if (_action.isAction(VERB_LOOK, NOUN_CHICKEN) && (_action._mainObjectSource == CAT_HOTSPOT))
			_vm->_dialogs->show(20505);
		else if (_action.isAction(VERB_TAKE, NOUN_CHICKEN) && (_action._mainObjectSource == CAT_HOTSPOT))
			_vm->_dialogs->show(20506);
		else if (_action.isAction(VERB_LOOK, NOUN_CHICKEN_ON_SPIT))
			_vm->_dialogs->show(20507);
		else if (_action.isAction(VERB_LOOK, NOUN_FIRE_PIT))
			_vm->_dialogs->show(20508);
		else if (_action.isAction(VERB_TAKE, NOUN_FIRE_PIT))
			_vm->_dialogs->show(20509);
		else if (_action.isAction(VERB_LOOK, NOUN_STREAM))
			_vm->_dialogs->show(20510);
		else if (_action.isAction(VERB_LOOK, NOUN_OPPOSITE_BANK))
			_vm->_dialogs->show(20511);
		else if (_game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))
			&& (_action.isAction(VERB_GIVE, NOUN_STREAM) || _action.isAction(VERB_THROW, NOUN_STREAM)
				|| _action.isAction(VERB_GIVE, NOUN_PIRANHA) || _action.isAction(VERB_THROW, NOUN_PIRANHA)))
			_vm->_dialogs->show(20512);
		else if (_action.isAction(VERB_LOOK, NOUN_PIRANHA))
			_vm->_dialogs->show(20513);
		else if (_action.isAction(VERB_LOOK, NOUN_TWINKIFRUIT_BUSH))
			_vm->_dialogs->show(20514);
		else if (_action.isAction(VERB_TAKE, NOUN_TWINKIFRUIT_BUSH))
			_vm->_dialogs->show(20515);
		else if (_action.isAction(VERB_TAKE, NOUN_NATIVE_WOMAN))
			_vm->_dialogs->show(20517);
		else
			return;
	}

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
