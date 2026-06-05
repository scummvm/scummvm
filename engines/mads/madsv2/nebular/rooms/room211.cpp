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

Scene211::Scene211(RexNebularEngine *vm) : Scene2xx(vm) {
	_ambushFl = false;
	_wakeFl = false;
	_monkeyFrame = 0;
	_scrollY = 0;
	_monkeyTime = 0;
}

void Scene211::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_ambushFl);
	s.syncAsByte(_wakeFl);

	s.syncAsSint32LE(_monkeyFrame);
	s.syncAsSint32LE(_scrollY);
	s.syncAsUint32LE(_monkeyTime);
}

void Scene211::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_SLITHERING_SNAKE);
}

void Scene211::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC002Z2");
	_wakeFl = false;

	if (_scene->_priorSceneId == 210)
		_game._player._playerPos = Common::Point(25, 148);
	else if (_scene->_priorSceneId == 205) {
		_game._player._playerPos = Common::Point(49, 133);
		_game._player._facing = FACING_WEST;
		_wakeFl = true;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->loadAnimation(formAnimName('A', -1), 100);
		_scene->_animation[0]->setCurrentFrame(169);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(310, 31);
		_game._player._facing = FACING_SOUTHWEST;
	}

	if (_vm->getRandomNumber(1, 8) == 1) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(202, 126));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
		_scene->_sequences.setMotion(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, -200, 0);
		_scene->_dynamicHotspots.add(324, 13, _globals._sequenceIndexes[2], Common::Rect(1, 1, 1 + 41, 1 + 10));
	}

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BINOCULARS);

	_vm->_palette->setEntry(252, 63, 44, 30);
	_vm->_palette->setEntry(253, 63, 20, 22);
	_game.loadQuoteSet(0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 1, 0);

	if (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY)
		_scene->_kernelMessages.initRandomMessages(2,
			Common::Rect(0, 0, 54, 30), 13, 2, 0xFDFC, 60,
			151, 152, 153, 154, 0);

	_monkeyTime = _vm->_game->_scene._frameStartTime;
	_scrollY = 30;

	_ambushFl = false;
	_monkeyFrame = 0;

	sceneEntrySound();
}

void Scene211::step() {
	if (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY) {
		_scene->_kernelMessages.randomServer();

		if (!_ambushFl && !_wakeFl && (_vm->_game->_scene._frameStartTime >= _monkeyTime)) {
			int chanceMinor = _scene->_kernelMessages.checkRandom() * 4 + 1;
			if (_scene->_kernelMessages.generateRandom(80, chanceMinor))
				_vm->_sound->command(18);

			_monkeyTime = _vm->_game->_scene._frameStartTime + 2;
		}

		if ((_game._player._playerPos == Common::Point(52, 132)) && (_game._player._facing == FACING_WEST) && !_game._player._moving &&
			(_game._trigger || !_ambushFl)) {
			switch (_game._trigger) {
			case 0:
				if (_game._objects.isInInventory(OBJ_BINOCULARS)) {
					_ambushFl = true;
					_monkeyFrame = 0;
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_scene->_kernelMessages.reset();
					_scene->loadAnimation(formAnimName('A', -1), 90);
					_vm->_sound->command(19);
					int count = (int)_game._objects._inventoryList.size();
					for (int idx = 0; idx < count; idx++) {
						if ((_game._objects._inventoryList[idx] == OBJ_BINOCULARS) && (_scene->_userInterface._selectedInvIndex != idx))
							_scene->_userInterface.selectObject(idx);
					}
				}
				break;

			case 90:
				_vm->_sound->command(10);
				_game._player._stepEnabled = true;
				_game._player._visible = true;
				_game._player._playerPos = Common::Point(49, 133);
				_ambushFl = false;
				_globals[kMonkeyStatus] = MONKEY_HAS_BINOCULARS;
				break;

			default:
				break;
			}
		}
	}

	if (_ambushFl && (_scene->_animation[0]->getCurrentFrame() > _monkeyFrame)) {
		_monkeyFrame = _scene->_animation[0]->getCurrentFrame();
		switch (_monkeyFrame) {
		case 2:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(12, 4), 0xFDFC, 0, 0, 60, _game.getQuote(157));
			_scene->_kernelMessages.setQuoted(msgIndex, 2, true);
		}
		break;

		case 12:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(35, 20), 0xFDFC, 0, 0, 60, _game.getQuote(155));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		}
		break;

		case 42:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(60, 45), 0xFDFC, 0, 0, 60, _game.getQuote(156));
			_scene->_kernelMessages.setQuoted(msgIndex, 6, true);
		}
		break;

		case 73:
			_scene->_kernelMessages.add(Common::Point(102, 95), 0xFDFC, 32, 0, 75, _game.getQuote(157));
			break;

		case 90:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(102, 95), 0xFDFC, 32, 0, 60, _game.getQuote(158));
			_scene->_kernelMessages.setQuoted(msgIndex, 6, true);
		}
		break;

		case 97:
			_scene->_userInterface.selectObject(-1);
			_game._objects.removeFromInventory(OBJ_BINOCULARS, 1);
			break;

		case 177:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, _scrollY), 0x1110, 0, 0, 180, _game.getQuote(161));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			_scrollY += 14;
		}
		break;

		case 181:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, _scrollY), 0x1110, 0, 0, 180, _game.getQuote(162));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			_scrollY += 14;
		}
		break;

		case 188:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, _scrollY), 0x1110, 0, 0, 180, _game.getQuote(163));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			_scrollY += 14;
		}
		break;

		case 200:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, _scrollY), 0x1110, 0, 0, 180, _game.getQuote(164));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			_scrollY += 14;
		}
		break;

		default:
			break;
		}
	}

	if (_wakeFl) {
		if (_game._trigger == 100) {
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_wakeFl = false;
		}

		if (_scene->_animation[0]->getCurrentFrame() > _monkeyFrame) {
			_monkeyFrame = _scene->_animation[0]->getCurrentFrame();
			switch (_scene->_animation[0]->getCurrentFrame()) {
			case 177:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, _scrollY), 0x1110, 0, 0, 180, _game.getQuote(165));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				_scrollY += 14;
			}
			break;

			case 181:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, _scrollY), 0x1110, 0, 0, 180, _game.getQuote(166));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				_scrollY += 14;
			}
			break;

			case 188:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, _scrollY), 0x1110, 0, 0, 180, _game.getQuote(167));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				_scrollY += 14;
			}
			break;

			case 200:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, _scrollY), 0x1110, 0, 0, 180, _game.getQuote(168));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				_scrollY += 14;
			}
			break;

			default:
				break;
			}
		}
	}
}

void Scene211::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_JUNGLE_PATH) && _game._objects.isInInventory(OBJ_BINOCULARS) && (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY)
		&& (_scene->_customDest.x <= 52) && (_scene->_customDest.y >= 132))
		_game._player.walk(Common::Point(52, 132), FACING_WEST);

	if (_action.isAction(VERB_WALK_DOWN, NOUN_PATH_TO_WEST)) {
		if (_game._objects.isInInventory(OBJ_BINOCULARS) && (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY))
			_game._player.walk(Common::Point(52, 132), FACING_WEST);
		else
			_game._player._walkOffScreenSceneId = 210;
	}

	if (_action.isAction(VERB_WALK_DOWN, NOUN_PATH_TO_NORTHEAST))
		_game._player._walkOffScreenSceneId = 207;
}

void Scene211::actions() {
	if (_action._lookFlag && (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY))
		_vm->_dialogs->show(21111);
	else if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_PALM_TREE))
		_vm->_dialogs->show(21116);
	else if (_action.isAction(VERB_LOOK, NOUN_BUSHY_FERN))
		_vm->_dialogs->show(21101);
	else if (_action.isAction(VERB_LOOK, NOUN_JUNGLE_PATH))
		_vm->_dialogs->show(21102);
	else if (_action.isAction(VERB_LOOK, NOUN_PALM_TREE)) {
		if (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY) {
			if (_game._storyMode == STORYMODE_NAUGHTY)
				_vm->_dialogs->show(21103);
			else
				_vm->_dialogs->show(21104);
		} else {
			_vm->_dialogs->show(21105);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_THICK_UNDERGROWTH)) {
		if (_game._storyMode == STORYMODE_NAUGHTY)
			_vm->_dialogs->show(21106);
		else
			_vm->_dialogs->show(21107);
	} else if (_action.isAction(VERB_LOOK, NOUN_JUNGLE))
		_vm->_dialogs->show(21108);
	else if (_action.isAction(VERB_LOOK, NOUN_PATH_TO_NORTHEAST))
		_vm->_dialogs->show(21109);
	else if (_action.isAction(VERB_LOOK, NOUN_PATH_TO_WEST))
		_vm->_dialogs->show(21110);
	else if (_action.isAction(VERB_LOOK, NOUN_SLITHERING_SNAKE))
		_vm->_dialogs->show(21113);
	else if (_action.isAction(VERB_TAKE, NOUN_SLITHERING_SNAKE))
		_vm->_dialogs->show(21114);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCKS))
		_vm->_dialogs->show(21115);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
