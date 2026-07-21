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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _explosionSpriteId;
};

static Scratch local;

static void room_304_init() {
	if (_scene->_priorSceneId == 303) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(formAnimName('a', -1), 60);
	} else {
		if (_globals[kSexOfRex] == REX_MALE)
			_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 0));
		else
			_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 2));

		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 1));
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 0));

		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 150, 0, 3, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 2);
		_vm->_palette->setEntry(252, 45, 63, 45);
		_vm->_palette->setEntry(253, 20, 45, 20);

		if (_globals[kSexOfRex] == REX_MALE)
			_game._player._playerPos = Common::Point(111, 117);
		else
			_game._player._playerPos = Common::Point(113, 116);

		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 11, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -1);
		_scene->_sequences.addTimer(48, 70);
	}

	section_3_music();
	_game.loadQuoteSet(0xEB, 0xEC, 0);
}

static void room_304_daemon() {
	if (_game._trigger == 60)
		_scene->_nextSceneId = 311;

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
		{
			_game._player._visible = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, 4);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			if (_globals[kSexOfRex] == REX_MALE)
				local._explosionSpriteId = _globals._spriteIndexes[1];
			else
				local._explosionSpriteId = _globals._spriteIndexes[4];

			int sprIdx = _scene->_sequences.addSpriteCycle(local._explosionSpriteId, false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(sprIdx, -1, 4);
			_scene->_sequences.setDepth(sprIdx, 1);
			_scene->_sequences.addSubEntry(sprIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
		}
		break;

		case 71:
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, _game.getQuote(0xEB));
			_scene->_sequences.addTimer(1, 72);
			break;

		case 72:
		{
			_vm->_sound->command(43);
			int sprIdx = _scene->_sequences.addSpriteCycle(local._explosionSpriteId, false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(sprIdx, 5, -2);
			_scene->_sequences.setDepth(sprIdx, 1);
			_scene->_sequences.addSubEntry(sprIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 73);
			if (_game._storyMode == STORYMODE_NICE)
				_scene->_sequences.addSubEntry(sprIdx, SEQUENCE_TRIGGER_SPRITE, 8, 78);
		}
		break;

		case 73:
		{
			int sprIdx = _scene->_sequences.addSpriteCycle(local._explosionSpriteId, false, 8, 0, 0, 0);
			_scene->_sequences.setAnimRange(sprIdx, -2, -2);
			_scene->_sequences.setDepth(sprIdx, 1);
		}
		break;

		case 74:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 5, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			break;

		case 75:
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
			break;

		case 76:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, 2);
			_scene->_sequences.addTimer(48, 77);
			break;

		case 77:
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(211, 45), 0xFDFC, 32, 0, 180, _game.getQuote(0xEC));
			_scene->_sequences.addTimer(120, 78);
			break;

		case 78:
			_scene->_nextSceneId = 316;
			break;

		default:
			break;
		}
	}
}

void room_304_synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(local._explosionSpriteId);
}

void room_304_preload() {
	room_init_code_pointer = room_304_init;
	room_daemon_code_pointer = room_304_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
