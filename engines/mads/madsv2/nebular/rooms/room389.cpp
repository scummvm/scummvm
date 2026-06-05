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

Scene389::Scene389(RexNebularEngine *vm) : Scene300s(vm) {
	_monsterTime = 0;
	_circularQuoteId = -1;
}

void Scene389::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsUint32LE(_monsterTime);
	s.syncAsSint32LE(_circularQuoteId);
}

void Scene389::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene389::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_monsterTime = 0;
	_circularQuoteId = 0x159;

	if (_globals[kAfterHavoc])
		_scene->_hotspots.activate(NOUN_MONSTER, false);
	else {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('m', -1));
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 0, 0, 0);
		_scene->_kernelMessages.initRandomMessages(1,
			Common::Rect(88, 19, 177, 77), 13, 2, 0xFDFC, 60,
			247, 248, 249, 0);
	}

	_vm->_palette->setEntry(252, 63, 37, 26);
	_vm->_palette->setEntry(253, 45, 24, 17);
	_game._player._visible = false;
	_game.loadQuoteSet(0xF7, 0xF8, 0xF9, 0x159, 0x15A, 0x15B, 0);

	sceneEntrySound();
}

void Scene389::step() {
	_scene->_kernelMessages.randomServer();
	if (_scene->_frameStartTime >= _monsterTime) {
		int chanceMinor = _scene->_kernelMessages.checkRandom() * 4 + 1;
		_scene->_kernelMessages.generateRandom(20, chanceMinor);
		_monsterTime = _scene->_frameStartTime + 2;
	}
}

void Scene389::actions() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_TALKTO, NOUN_MONSTER)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_kernelMessages.add(Common::Point(160, 136), 0x1110, 32, 1, 120, _game.getQuote(_circularQuoteId));
			_circularQuoteId++;
			if (_circularQuoteId > 0x15B)
				_circularQuoteId = 0x159;

			break;

		case 1:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE)) {
		if (_globals[kAfterHavoc]) {
			if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
				_vm->_dialogs->show(38911);
			else
				_vm->_dialogs->show(38912);
		} else
			_vm->_dialogs->show(38910);
	} else if (_action.isAction(VERB_OPEN, NOUN_GRATE)) {
		if (_globals[kAfterHavoc])
			_vm->_dialogs->show(38914);
		else
			_vm->_dialogs->show(38913);
	} else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
