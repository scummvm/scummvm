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
		int idx = _scene->_dynamicHotspots.add(131, 396, 0, Common::Rect(0, 0, 320, 156));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(155, 152), FACING_SOUTH);
		_scene->_dynamicHotspots.setCursor(idx, CURSOR_GO_DOWN);
	}

	if (!local._rhotundaEatFl) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('b', -1));
		if (_vm->getRandomNumber(1, 3) == 2) {
			_globals._spriteIndexes[15] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 9, 1, 0, 0);
			int idx = _scene->_dynamicHotspots.add(477, 209, _globals._spriteIndexes[15], Common::Rect(0, 0, 0, 0));
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
	if (local._rhotundaEatFl && !_action.isAction(VERB_WALK_TOWARDS, NOUN_FIELD_TO_SOUTH)) {
		_game._player.walk(Common::Point(158, 136), FACING_SOUTH);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, NOUN_OPEN_AREA_TO_EAST))
		_game._player._walkOffScreenSceneId = 209;
}

static void room_203_parser() {
	if (_action._savedFields._lookFlag) {
		_vm->_dialogs->show(20307);
	} else if (_action.isAction(VERB_WALK_TOWARDS, NOUN_FIELD_TO_SOUTH)) {
		_scene->_nextSceneId = 208;
	} else if (_action.isAction(VERB_WALK_TOWARDS, NOUN_FIELD_TO_NORTH)) {
		_scene->_nextSceneId = 202;
	} else if (_action.isAction(VERB_LOOK, NOUN_SKY)) {
		_vm->_dialogs->show(20301);
	} else if (_action.isAction(VERB_LOOK, NOUN_CLIFF_FACE)) {
		_vm->_dialogs->show(20302);
	} else if (_action.isAction(VERB_LOOK, NOUN_PALM_TREE)) {
		_vm->_dialogs->show(20303);
	} else if (_action.isAction(VERB_LOOK, NOUN_FIELD_TO_NORTH)) {
		_vm->_dialogs->show(20304);
	} else if (_action.isAction(VERB_LOOK, NOUN_GRASSY_FIELD)) {
		_vm->_dialogs->show(20305);
	} else if (_action.isAction(VERB_LOOK, NOUN_BOULDERS)) {
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
