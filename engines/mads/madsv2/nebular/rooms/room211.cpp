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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section2.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _ambushFl;
	bool _wakeFl;
	int32 _monkeyFrame;
	int32 _scrollY;
	int32 _monkeyTime;
};

static Scratch local;


static void room_211_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC002Z2");
	local._wakeFl = false;

	if (_scene->_priorSceneId == 210)
		_game._player._playerPos = Common::Point(25, 148);
	else if (_scene->_priorSceneId == 205) {
		_game._player._playerPos = Common::Point(49, 133);
		_game._player._facing = FACING_WEST;
		local._wakeFl = true;
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
		_scene->_dynamicHotspots.add(words_slithering_snake, words_walkto, _globals._sequenceIndexes[2], Common::Rect(1, 1, 1 + 41, 1 + 10));
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

	local._monkeyTime = _vm->_game->_scene._frameStartTime;
	local._scrollY = 30;

	local._ambushFl = false;
	local._monkeyFrame = 0;

	section_2_music();
}

static void room_211_daemon() {
	if (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY) {
		_scene->_kernelMessages.randomServer();

		if (!local._ambushFl && !local._wakeFl && (_vm->_game->_scene._frameStartTime >= local._monkeyTime)) {
			int chanceMinor = _scene->_kernelMessages.checkRandom() * 4 + 1;
			if (_scene->_kernelMessages.generateRandom(80, chanceMinor))
				_vm->_sound->command(18);

			local._monkeyTime = _vm->_game->_scene._frameStartTime + 2;
		}

		if ((_game._player._playerPos == Common::Point(52, 132)) && (_game._player._facing == FACING_WEST) && !_game._player._moving &&
			(_game._trigger || !local._ambushFl)) {
			switch (_game._trigger) {
			case 0:
				if (_game._objects.isInInventory(OBJ_BINOCULARS)) {
					local._ambushFl = true;
					local._monkeyFrame = 0;
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
				local._ambushFl = false;
				_globals[kMonkeyStatus] = MONKEY_HAS_BINOCULARS;
				break;

			default:
				break;
			}
		}
	}

	if (local._ambushFl && (_scene->_animation[0]->getCurrentFrame() > local._monkeyFrame)) {
		local._monkeyFrame = _scene->_animation[0]->getCurrentFrame();
		switch (local._monkeyFrame) {
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
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, _game.getQuote(161));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		case 181:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, _game.getQuote(162));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		case 188:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, _game.getQuote(163));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		case 200:
		{
			int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, _game.getQuote(164));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			local._scrollY += 14;
		}
		break;

		default:
			break;
		}
	}

	if (local._wakeFl) {
		if (_game._trigger == 100) {
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			local._wakeFl = false;
		}

		if (_scene->_animation[0]->getCurrentFrame() > local._monkeyFrame) {
			local._monkeyFrame = _scene->_animation[0]->getCurrentFrame();
			switch (_scene->_animation[0]->getCurrentFrame()) {
			case 177:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, _game.getQuote(165));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			case 181:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, _game.getQuote(166));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			case 188:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, _game.getQuote(167));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			case 200:
			{
				int msgIndex = _scene->_kernelMessages.add(Common::Point(63, local._scrollY), 0x1110, 0, 0, 180, _game.getQuote(168));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
				local._scrollY += 14;
			}
			break;

			default:
				break;
			}
		}
	}
}

static void room_211_pre_parser() {
	if (player_said_2(walk_down, jungle_path) && _game._objects.isInInventory(OBJ_BINOCULARS) && (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY)
		&& (_scene->_customDest.x <= 52) && (_scene->_customDest.y >= 132))
		_game._player.walk(Common::Point(52, 132), FACING_WEST);

	if (player_said_2(walk_down, path_to_west)) {
		if (_game._objects.isInInventory(OBJ_BINOCULARS) && (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY))
			_game._player.walk(Common::Point(52, 132), FACING_WEST);
		else
			_game._player._walkOffScreenSceneId = 210;
	}

	if (player_said_2(walk_down, path_to_northeast))
		_game._player._walkOffScreenSceneId = 207;
}

static void room_211_parser() {
	if (_action._lookFlag && (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY))
		_vm->_dialogs->show(21111);
	else if (player_said_3(look, binoculars, palm_tree))
		_vm->_dialogs->show(21116);
	else if (player_said_2(look, bushy_fern))
		_vm->_dialogs->show(21101);
	else if (player_said_2(look, jungle_path))
		_vm->_dialogs->show(21102);
	else if (player_said_2(look, palm_tree)) {
		if (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY) {
			if (_game._storyMode == STORYMODE_NAUGHTY)
				_vm->_dialogs->show(21103);
			else
				_vm->_dialogs->show(21104);
		} else {
			_vm->_dialogs->show(21105);
		}
	} else if (player_said_2(look, thick_undergrowth)) {
		if (_game._storyMode == STORYMODE_NAUGHTY)
			_vm->_dialogs->show(21106);
		else
			_vm->_dialogs->show(21107);
	} else if (player_said_2(look, jungle))
		_vm->_dialogs->show(21108);
	else if (player_said_2(look, path_to_northeast))
		_vm->_dialogs->show(21109);
	else if (player_said_2(look, path_to_west))
		_vm->_dialogs->show(21110);
	else if (player_said_2(look, slithering_snake))
		_vm->_dialogs->show(21113);
	else if (player_said_2(take, slithering_snake))
		_vm->_dialogs->show(21114);
	else if (player_said_2(look, rocks))
		_vm->_dialogs->show(21115);
	else
		return;

	_action._inProgress = false;
}

void room_211_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._ambushFl);
	s.syncAsByte(local._wakeFl);

	s.syncAsSint32LE(local._monkeyFrame);
	s.syncAsSint32LE(local._scrollY);
	s.syncAsUint32LE(local._monkeyTime);
}

void room_211_preload() {
	room_init_code_pointer = room_211_init;
	room_pre_parser_code_pointer = room_211_pre_parser;
	room_parser_code_pointer = room_211_parser;
	room_daemon_code_pointer = room_211_daemon;

	section_2_walker();
	section_2_interface();
	_scene->addActiveVocab(words_slithering_snake);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
