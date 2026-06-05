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

Scene308::Scene308(RexNebularEngine *vm) : Scene3xx(vm) {
	_forceField.init();
}

void Scene308::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	_forceField.synchronize(s);
}


void Scene308::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene308::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC003x0");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*SC003x1");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*SC003x2");

	initForceField(&_forceField, true);

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));

	_vm->_palette->setEntry(252, 63, 30, 20);
	_vm->_palette->setEntry(253, 45, 15, 12);

	_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
	_scene->_sequences.addTimer(48, 70);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(formAnimName('a', -1), 60);

	sceneEntrySound();
	_game.loadQuoteSet(0xF4, 0xF5, 0xF6, 0);
}

void Scene308::step() {
	handleForceField(&_forceField, &_globals._spriteIndexes[0]);

	if (_game._trigger == 60)
		_scene->_nextSceneId = 307;

	if (_game._trigger < 70)
		return;

	switch (_game._trigger) {
	case 70:
	{
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 18, 9, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_kernelMessages.reset();
		int idx = _scene->_kernelMessages.add(Common::Point(171, 21), 0xFDFC, 0, 0, 120, _game.getQuote(244));
		_scene->_kernelMessages.setQuoted(idx, 2, true);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
	}
	break;

	case 71:
	{
		int seqIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], seqIdx);
		_scene->_sequences.addTimer(48, 72);
	}
	break;

	case 72:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 20, 5, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 3, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_kernelMessages.reset();
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
		break;

	case 73:
	{
		int seqIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], seqIdx);
		_scene->_sequences.addTimer(48, 74);
	}
	break;

	case 74:
	{
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 20, 8, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 6, 7);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_kernelMessages.reset();
		int idx = _scene->_kernelMessages.add(Common::Point(171, 21), 0xFDFC, 0, 0, 120, _game.getQuote(245));
		_scene->_kernelMessages.setQuoted(idx, 2, true);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
	}
	break;

	case 75:
	{
		int seqIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 23, 5, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 8, 10);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
	}
	break;

	case 76:
	{
		int seqIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 26, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_kernelMessages.reset();
		int idx = _scene->_kernelMessages.add(Common::Point(171, 21), 0xFDFC, 0, 0, 120, _game.getQuote(246));
		_scene->_kernelMessages.setQuoted(idx, 2, true);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], seqIdx);
	}
	break;

	default:
		break;
	}
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
