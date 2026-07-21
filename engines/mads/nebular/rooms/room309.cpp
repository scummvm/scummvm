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
#include "mads/nebular/rooms/forcefield.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _characterSpriteIndexes[3];
	int16 _messagesIndexes[3];
	int32 _lastFrame;
	Forcefield _forcefield;
};

static Scratch local;


static void room_309_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC003x0");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*SC003x1");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*SC003x2");

	init_forcefield(&local._forcefield, true);

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

	local._characterSpriteIndexes[0] = _scene->_animation[0]->_spriteListIndexes[2];
	local._characterSpriteIndexes[1] = _scene->_animation[0]->_spriteListIndexes[2];
	local._characterSpriteIndexes[2] = _scene->_animation[0]->_spriteListIndexes[1];

	local._messagesIndexes[0] = -1;
	local._messagesIndexes[1] = -1;
	local._messagesIndexes[2] = -1;

	section_3_music();

	_game.loadQuoteSet(0xF7, 0xF8, 0xF9, 0x15C, 0x15D, 0x15E, 0);
}

static void room_309_daemon() {
	handle_forcefield(&local._forcefield, &_globals._spriteIndexes[0]);

	if (_game._trigger == 61) {
		local._messagesIndexes[0] = -1;
		local._messagesIndexes[1] = -1;
	}

	if (_game._trigger == 62)
		local._messagesIndexes[2] = -1;

	if (_scene->_animation[0] != nullptr) {
		if (local._lastFrame != _scene->_animation[0]->getCurrentFrame()) {
			local._lastFrame = _scene->_animation[0]->getCurrentFrame();
			if (local._lastFrame == 39) {
				local._messagesIndexes[0] = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 32, 61, 210, _game.getQuote(348));
				local._messagesIndexes[1] = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 32, 0, 210, _game.getQuote(349));
			}

			if (local._lastFrame == 97)
				local._messagesIndexes[2] = _scene->_kernelMessages.add(Common::Point(0, 0), 0xFBFA, 32, 62, 180, _game.getQuote(350));

			for (int charIdx = 0; charIdx < 3; charIdx++) {
				if (local._messagesIndexes[charIdx] >= 0) {
					bool match = false;
					int j = -1;
					for (j = _scene->_animation[0]->_oldFrameEntry; j < _scene->_animation[0]->_frameEntriesCount; j++) {
						if (_scene->_animation[0]->_frameEntries[j].series_id == local._characterSpriteIndexes[charIdx]) {
							match = true;
							break;
						}
					}

					if (match) {
						const Image &img = _scene->_animation[0]->_frameEntries[j];
						KernelMessage &kmsg = kernel_message[local._messagesIndexes[charIdx]];
						kmsg.x = img.x;
						kmsg.y = img.y - (50 + (14 * ((charIdx == 0) ? 2 : 1)));
					}
				}
			}
		}
	}

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
		{
			int idx = _scene->_dynamicHotspots.add(words_ghastly_beast, words_gawk_at, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
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

void room_309_synchronize(Common::Serializer &s) {
	local._forcefield.synchronize(s);

	for (int i = 0; i < 3; ++i)
		s.syncAsSint32LE(local._characterSpriteIndexes[i]);
	for (int i = 0; i < 3; ++i)
		s.syncAsSint32LE(local._messagesIndexes[i]);
	s.syncAsSint32LE(local._lastFrame);
}

void room_309_preload() {
	local._lastFrame = -1;
	local._forcefield.init();

	room_init_code_pointer = room_309_init;
	room_daemon_code_pointer = room_309_daemon;
	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
