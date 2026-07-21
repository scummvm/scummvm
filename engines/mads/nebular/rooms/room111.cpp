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
	bool _stampedFl;
	bool _launch1Fl;
	bool _launched2Fl;
	bool _rexDivingFl;
};

static Scratch local;


static void room_111_init() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('X', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('X', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('X', 2));

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('B', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('B', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('B', 2));

	_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 8, 0, 0, 0);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 9, 73);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 13, 73);

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 0, 0, 0);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 71, 71);

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 0, 0, 0);
	_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
	_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);

	int idx = _scene->_dynamicHotspots.add(words_bats, words_look_at, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
	idx = _scene->_dynamicHotspots.add(words_bats, words_look_at, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
	idx = _scene->_dynamicHotspots.add(words_bats, words_look_at, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);

	local._launch1Fl = false;
	local._launched2Fl = false;
	local._stampedFl = false;

	if ((_scene->_priorSceneId < 201) && (_scene->_priorSceneId != RETURNING_FROM_DIALOG)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->loadAnimation(Resources::formatName(111, 'A', 0, EXT_AA, ""), 70);
		_game._player._playerPos = Common::Point(234, 116);
		_game._player._facing = FACING_EAST;

		local._launch1Fl = true;
		local._launched2Fl = true;

		_vm->_sound->command(36);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(300, 130);
		_game._player._facing = FACING_WEST;
	}

	local._rexDivingFl = false;

	section_1_music();
}

static void room_111_daemon() {
	if (_game._trigger == 70) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		local._launch1Fl = false;
		local._launched2Fl = false;
	}

	if ((_game._trigger == 71) && !local._stampedFl) {
		local._stampedFl = true;
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 18, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
	}

	if (_game._trigger == 72) {
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 20);
	}

	if (!local._launch1Fl && (_vm->getRandomNumber(1, 5000) == 1)) {
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 5, 1, 0, 0);
		local._launch1Fl = true;
		int idx = _scene->_dynamicHotspots.add(words_bats, words_look_at, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
	}

	if (!local._launched2Fl && (_vm->getRandomNumber(1, 30000) == 1)) {
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 5, 1, 0, 0);
		int idx = _scene->_dynamicHotspots.add(words_bats, words_look_at, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
		local._launched2Fl = true;
	}

	if (_game._trigger == 73)
		_vm->_sound->command(37);

	if (local._rexDivingFl && (_scene->_animation[0]->getCurrentFrame() >= 9)) {
		_vm->_sound->command(36);
		local._rexDivingFl = false;
	}
}

static void room_111_pre_parser() {
	if (player_said_2(walk_through, cave_entrance))
		_game._player._walkOffScreenSceneId = 212;
}

static void room_111_parser() {
	if (player_said_2(dive_into, pool) && _game._objects.isInInventory(OBJ_REBREATHER)) {
		switch (_game._trigger) {
		case 0:
			_scene->loadAnimation(Resources::formatName(111, 'A', 1, EXT_AA, ""), 1);
			local._rexDivingFl = true;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			break;

		case 1:
			_scene->_nextSceneId = 110;
			break;

		default:
			break;
		}
	} else if (player_said_2(look, cave_floor))
		_vm->_dialogs->show(11101);
	else if (player_said_2(look, pool))
		_vm->_dialogs->show(11102);
	else if (player_said_2(look, cave_entrance))
		_vm->_dialogs->show(11103);
	else if (player_said_2(look, stalagmites))
		_vm->_dialogs->show(11104);
	else if (player_said_2(look, large_stalagmite))
		_vm->_dialogs->show(11105);
	else if ((player_said_1(pull) || player_said_1(take)) && (player_said_1(stalagmites) || player_said_1(large_stalagmite)))
		_vm->_dialogs->show(11106);
	else
		return;

	_action._inProgress = false;
}

void room_111_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._stampedFl);
	s.syncAsByte(local._launch1Fl);
	s.syncAsByte(local._launched2Fl);
	s.syncAsByte(local._rexDivingFl);
}

void room_111_preload() {
	room_init_code_pointer = room_111_init;
	room_pre_parser_code_pointer = room_111_pre_parser;
	room_parser_code_pointer = room_111_parser;
	room_daemon_code_pointer = room_111_daemon;

	_scene->addActiveVocab(words_bats);
	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
