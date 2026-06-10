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
#include "mads/madsv2/nebular/rooms/section1.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _crabsFl;
};

static Scratch local;


static void room_110_init() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('X', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('X', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('X', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('X', 3));

	local._crabsFl = false;

	if (_scene->_priorSceneId == 109) {
		_game._player._playerPos = Common::Point(59, 71);

		_globals._sequenceIndexes[0] = _scene->_sequences.startCycle(_globals._spriteIndexes[0], false, 1);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);

		local._crabsFl = true;

		int idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[0], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(194, 23);
		_game._player._facing = FACING_SOUTH;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(Resources::formatName(110, 'T', 1, EXT_AA, ""), 70);
	}

	section_1_music();
	_game.loadQuoteSet(0x59, 0);

	if (!_game._visitedScenes._sceneRevisited && (_scene->_priorSceneId == 109))
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(109));
}

static void room_110_daemon() {
	if (_game._trigger == 70) {
		_game._player._visible = true;
		_game._player._stepEnabled = true;
	}
}

static void room_110_pre_parser() {
	if (_action.isAction(VERB_SWIM_THROUGH, NOUN_CAVE_ENTRANCE))
		_game._player._walkOffScreenSceneId = 109;

	if (local._crabsFl) {
		local._crabsFl = false;

		_scene->_sequences.remove(_globals._sequenceIndexes[0]);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);

		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 16, 1, 0, 0);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 16, 1, 0, 0);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 16, 1, 0, 0);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 16, 1, 0, 0);

		int idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[0], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
	}
}

static void room_110_parser() {
	if (_action.isAction(VERB_SWIM_THROUGH, NOUN_TUNNEL)) {
		switch (_game._trigger) {
		case 0:
			_scene->loadAnimation(Resources::formatName(110, 'T', 0, EXT_AA, ""), 1);
			_scene->_animation[0]->setNextFrameTimer(_game._player._ticksAmount + _game._player._priorTimer);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			break;
		case 1:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_scene->_nextSceneId = 111;
			break;
		default:
			break;
		}
	} else if ((_action._lookFlag) || _action.isAction(VERB_LOOK, NOUN_CAVE))
		_vm->_dialogs->show(11001);
	else if (_action.isAction(VERB_LOOK, NOUN_CAVE_CEILING) || _action.isAction(VERB_LOOK_AT, NOUN_CAVE_CEILING))
		_vm->_dialogs->show(11002);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCKS))
		_vm->_dialogs->show(11003);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCKS))
		_vm->_dialogs->show(11004);
	else if (_action.isAction(VERB_LOOK, NOUN_TUNNEL))
		_vm->_dialogs->show(11005);
	else if (_action.isAction(VERB_LOOK, NOUN_CAVE_ENTRANCE))
		_vm->_dialogs->show(11006);
	else if (_action.isAction(VERB_LOOK, NOUN_FUNGOIDS))
		_vm->_dialogs->show(11007);
	else if (_action.isAction(VERB_TAKE, NOUN_FUNGOIDS))
		_vm->_dialogs->show(11008);
	else
		return;

	_action._inProgress = false;
}

void room_110_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._crabsFl);
}

void room_110_preload() {
	room_init_code_pointer = room_110_init;
	room_pre_parser_code_pointer = room_110_pre_parser;
	room_parser_code_pointer = room_110_parser;
	room_daemon_code_pointer = room_110_daemon;

	section_1_walker();
	section_1_interface();
	_scene->addActiveVocab(NOUN_CRAB);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
