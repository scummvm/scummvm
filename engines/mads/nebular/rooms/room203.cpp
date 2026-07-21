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
#include "mads/nebular/rooms/section2.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _rhotundaEat2Fl;
	bool _rhotundaEatFl;
};

static Scratch local;


static void room_203_init() {
	if (_scene->_priorSceneId == 202) {
		_game._player._playerPos = Common::Point(187, 99);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId == 209) {
		_game._player._playerPos = Common::Point(308, 117);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(155, 152);
		_game._player._facing = FACING_NORTH;
	}

	local._rhotundaEatFl = false;
	local._rhotundaEat2Fl = false;

	if ((_globals[kRhotundaStatus] == 0) && (!_scene->_roomChanged)) {
		local._rhotundaEatFl = true;
		_game._player.walk(Common::Point(158, 135), FACING_SOUTH);
		int idx = _scene->_dynamicHotspots.add(words_field_to_south, words_walk_towards, 0, Common::Rect(0, 0, 320, 156));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(155, 152), FACING_SOUTH);
		_scene->_dynamicHotspots.setCursor(idx, CURSOR_GO_DOWN);
	}

	if (!local._rhotundaEatFl) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('b', -1));
		if (_vm->getRandomNumber(1, 3) == 2) {
			_globals._spriteIndexes[15] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 9, 1, 0, 0);
			int idx = _scene->_dynamicHotspots.add(words_yellow_bird, words_look_at, _globals._spriteIndexes[15], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
			_vm->_sound->command(14);
		}
	}

	_game.loadQuoteSet(0x67, 0x68, 0x69, 0x6A, 0x5A, 0);

	if (local._rhotundaEatFl) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(_vm->getRandomNumber(103, 106)));
	}

	section_2_music();
}

static void room_203_daemon() {
	if (!local._rhotundaEatFl)
		return;

	if ((_game._trigger == 0) && local._rhotundaEat2Fl)
		return;

	if ((_game._player._playerPos != Common::Point(158, 136)) || (_game._player._facing != FACING_SOUTH))
		return;

	local._rhotundaEat2Fl = true;

	if (_game._trigger == 0) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_vm->_palette->lock();
		_scene->_kernelMessages.reset();
		_scene->resetScene();
		_vm->_events->setCursor2(CURSOR_WAIT);
		_scene->loadAnimation(Resources::formatName(203, 'a', -1, EXT_AA, ""), 81);
	} else if (_game._trigger == 81) {
		_scene->_nextSceneId = 208;
		_scene->_reloadSceneFlag = true;
	}
}

static void room_203_pre_parser() {
	if (local._rhotundaEatFl && !player_said_2(walk_towards, field_to_south)) {
		_game._player.walk(Common::Point(158, 136), FACING_SOUTH);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(walkto, open_area_to_east))
		_game._player._walkOffScreenSceneId = 209;
}

static void room_203_parser() {
	if (_action._savedFields._lookFlag) {
		_vm->_dialogs->show(20307);
	} else if (player_said_2(walk_towards, field_to_south)) {
		_scene->_nextSceneId = 208;
	} else if (player_said_2(walk_towards, field_to_north)) {
		_scene->_nextSceneId = 202;
	} else if (player_said_2(look, sky)) {
		_vm->_dialogs->show(20301);
	} else if (player_said_2(look, cliff_face)) {
		_vm->_dialogs->show(20302);
	} else if (player_said_2(look, palm_tree)) {
		_vm->_dialogs->show(20303);
	} else if (player_said_2(look, field_to_north)) {
		_vm->_dialogs->show(20304);
	} else if (player_said_2(look, grassy_field)) {
		_vm->_dialogs->show(20305);
	} else if (player_said_2(look, boulders)) {
		_vm->_dialogs->show(20305);
	} else
		return;

	_action._inProgress = false;
}

void room_203_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._rhotundaEat2Fl);
	s.syncAsByte(local._rhotundaEatFl);
}

void room_203_preload() {
	room_init_code_pointer = room_203_init;
	room_pre_parser_code_pointer = room_203_pre_parser;
	room_parser_code_pointer = room_203_parser;
	room_daemon_code_pointer = room_203_daemon;

	section_2_walker();
	section_2_interface();
	_scene->addActiveVocab(477);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
