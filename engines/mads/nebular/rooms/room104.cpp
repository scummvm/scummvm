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
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _kargShootingFl;
	bool _loseFl;
};

static Scratch local;

static void room_104_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('h', -1));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 14, 0, 0, 1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);

	if (_scene->_priorSceneId == 105)
		_game._player._playerPos = Common::Point(302, 107);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(160, 134);

	local._loseFl = false;
	_game.loadQuoteSet(0x35, 0x34, 0);
	local._kargShootingFl = false;

	if (_vm->getRandomNumber(1, 3) == 1) {
		_scene->loadAnimation(Resources::formatName(104, 'B', -1, EXT_AA, ""), 0);
		local._kargShootingFl = true;
	}

	section_1_music();
}

static void room_104_daemon() {
	if ((_game._player._playerPos == Common::Point(189, 70)) && (_game._trigger || !local._loseFl)) {
		if (_game._player._facing == FACING_SOUTHWEST || _game._player._facing == FACING_SOUTHEAST)
			_game._player._facing = FACING_SOUTH;

		if (_game._player._facing == FACING_NORTHWEST || _game._player._facing == FACING_NORTHEAST)
			_game._player._facing = FACING_NORTH;

		bool mirrorFl = false;
		if (_game._player._facing == FACING_WEST) {
			_game._player._facing = FACING_EAST;
			mirrorFl = true;
		}

		local._loseFl = true;

		switch (_game._player._facing) {
		case FACING_EAST:
			switch (_game._trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->freeAnimation();
				_scene->resetScene();
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 0));
				_vm->_palette->refreshSceneColors();
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], mirrorFl, 7, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], mirrorFl, 7, 0, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -2, -2);
				_scene->_sequences.addTimer(90, 2);
				break;

			case 2:
				_vm->_dialogs->show(10406);
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
			break;

		case FACING_SOUTH:
			switch (_game._trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->freeAnimation();
				_scene->resetScene();
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 1));
				_vm->_palette->refreshSceneColors();
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 14);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 15, 32);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 3, 0, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
				_scene->_sequences.addTimer(90, 3);
				break;

			case 3:
				_vm->_dialogs->show(10406);
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
			break;

		case FACING_NORTH:
			switch (_game._trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->freeAnimation();
				_scene->resetScene();
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 2));
				_vm->_palette->refreshSceneColors();
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 8, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				if (_game._storyMode >= STORYMODE_NICE)
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 15, 2);
				break;

			case 1:
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 8, 0, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -2, -2);
				_scene->_sequences.addTimer(90, 2);
				break;

			case 2:
				_vm->_dialogs->show(10406);
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
			break;
		default:
			break;
		}

		if (!_game._trigger)
			_vm->_sound->command(34);
	}

	if (_game._player._moving && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(189, 70), FACING_NONE);
		_scene->_rails.resetNext();
	}

	if ((_game._player._special > 0) && _game._player._stepEnabled)
		_game._player._stepEnabled = false;

	if (local._kargShootingFl && (_scene->_animation[0]->getCurrentFrame() >= 19)) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(52));
		local._kargShootingFl = false;
	}
}

static void room_104_pre_parser() {
	if (player_said_2(swim_towards, eastern_cliff_face))
		_game._player._walkOffScreenSceneId = 105;

	if (player_said_2(swim_towards, open_area_to_south))
		_game._player._walkOffScreenSceneId = 106;
}

static void room_104_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10405);
	else if (player_said_2(look, curious_weed_patch))
		_vm->_dialogs->show(10404);
	else if (player_said_2(look, surface))
		_vm->_dialogs->show(10403);
	else if (player_said_2(look, cliff_face))
		_vm->_dialogs->show(10401);
	else if (player_said_2(look, ocean_floor))
		_vm->_dialogs->show(10402);
	else
		return;

	_action._inProgress = false;
}

void room_104_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._kargShootingFl);
	s.syncAsByte(local._loseFl);
}

void room_104_preload() {
	room_init_code_pointer = room_104_init;
	room_pre_parser_code_pointer = room_104_pre_parser;
	room_parser_code_pointer = room_104_parser;
	room_daemon_code_pointer = room_104_daemon;

	anim_himem_preload(formAnimName('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
