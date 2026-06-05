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

Scene309::Scene309(RexNebularEngine *vm) : Scene3xx(vm) {
	for (int i = 0; i < 3; i++) {
		_characterSpriteIndexes[i] = -1;
		_messagesIndexes[i] = -1;
	}

	_lastFrame = -1;
	_forceField.init();
}

void Scene309::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	_forceField.synchronize(s);

	for (int i = 0; i < 3; ++i)
		s.syncAsSint32LE(_characterSpriteIndexes[i]);
	for (int i = 0; i < 3; ++i)
		s.syncAsSint32LE(_messagesIndexes[i]);
	s.syncAsSint32LE(_lastFrame);
}

void Scene309::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene309::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC003x0");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*SC003x1");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*SC003x2");

	initForceField(&_forceField, true);

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));

	_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, 3);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 3, 70);

	_vm->_palette->setEntry(252, 63, 37, 26);
	_vm->_palette->setEntry(253, 45, 24, 17);
	_vm->_palette->setEntry(16, 63, 63, 63);
	_vm->_palette->setEntry(17, 45, 45, 45);
	_vm->_palette->setEntry(250, 63, 20, 20);
	_vm->_palette->setEntry(251, 45, 10, 10);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(formAnimName('a', -1), 60);

	_characterSpriteIndexes[0] = _scene->_animation[0]->_spriteListIndexes[2];
	_characterSpriteIndexes[1] = _scene->_animation[0]->_spriteListIndexes[2];
	_characterSpriteIndexes[2] = _scene->_animation[0]->_spriteListIndexes[1];

	_messagesIndexes[0] = -1;
	_messagesIndexes[1] = -1;
	_messagesIndexes[2] = -1;

	sceneEntrySound();

	_game.loadQuoteSet(0xF7, 0xF8, 0xF9, 0x15C, 0x15D, 0x15E, 0);
}

void Scene309::step() {
	handleForceField(&_forceField, &_globals._spriteIndexes[0]);

	if (_game._trigger == 61) {
		_messagesIndexes[0] = -1;
		_messagesIndexes[1] = -1;
	}

	if (_game._trigger == 62)
		_messagesIndexes[2] = -1;

	if (_scene->_animation[0] != nullptr) {
		if (_lastFrame != _scene->_animation[0]->getCurrentFrame()) {
			_lastFrame = _scene->_animation[0]->getCurrentFrame();
			if (_lastFrame == 39) {
				_messagesIndexes[0] = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 32, 61, 210, _game.getQuote(348));
				_messagesIndexes[1] = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 32, 0, 210, _game.getQuote(349));
			}

			if (_lastFrame == 97)
				_messagesIndexes[2] = _scene->_kernelMessages.add(Common::Point(0, 0), 0xFBFA, 32, 62, 180, _game.getQuote(350));

			for (int charIdx = 0; charIdx < 3; charIdx++) {
				if (_messagesIndexes[charIdx] >= 0) {
					bool match = false;
					int j = -1;
					for (j = _scene->_animation[0]->_oldFrameEntry; j < _scene->_animation[0]->_header._frameEntriesCount; j++) {
						if (_scene->_animation[0]->_frameEntries[j]._spriteSlot._spritesIndex == _characterSpriteIndexes[charIdx]) {
							match = true;
							break;
						}
					}

					if (match) {
						SpriteSlotSubset *curSpriteSlot = &_scene->_animation[0]->_frameEntries[j]._spriteSlot;
						_scene->_kernelMessages._entries[_messagesIndexes[charIdx]]._position.x = curSpriteSlot->_position.x;
						_scene->_kernelMessages._entries[_messagesIndexes[charIdx]]._position.y = curSpriteSlot->_position.y - (50 + (14 * ((charIdx == 0) ? 2 : 1)));
					}
				}
			}
		}
	}

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
		{
			int idx = _scene->_dynamicHotspots.add(689, 690, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(142, 146), FACING_NORTHEAST);
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 4, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 3);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		}
		break;

		case 71:
		{
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 4, 7);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			int idx = _scene->_kernelMessages.add(Common::Point(85, 37), 0xFDFC, 0, 0, 120, _game.getQuote(248));
			_scene->_kernelMessages.setQuoted(idx, 2, true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		}
		break;

		case 72:
		{
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 8, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 8, 11);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
		}
		break;

		case 73:
		{
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 12, 20);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			int idx = _scene->_kernelMessages.add(Common::Point(170, 49), 0xFDFC, 0, 0, 120, _game.getQuote(249));
			_scene->_kernelMessages.setQuoted(idx, 2, true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
		}
		break;

		case 74:
		{
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 6, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 21, 23);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
		}
		break;

		case 75:
		{
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 12, 6, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 24, 25);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
		}
		break;

		case 76:
		{
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 26, 28);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 77);
		}
		break;

		case 77:
		{
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 90, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 29, 30);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			int idx = _scene->_kernelMessages.add(Common::Point(15, 46), 0xFDFC, 0, 0, 120, _game.getQuote(247));
			_scene->_kernelMessages.setQuoted(idx, 2, true);
			_scene->_sequences.addTimer(120, 78);
		}
		break;

		default:
			break;
		}
	}

	if (_game._trigger == 60)
		_scene->_nextSceneId = 308;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
