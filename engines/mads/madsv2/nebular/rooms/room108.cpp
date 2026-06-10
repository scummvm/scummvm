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

static void room_108_init() {
	if (_globals[kHoovicSated] == 2)
		_globals[kHoovicSated] = 0;

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('X', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('X', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('X', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('X', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(Resources::formatName(105, 'f', 4, EXT_SS, ""));

	_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 13, 0, 0, 7);
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 16, 0, 0, 9);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 17, 0, 0, 3);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 14, 0, 0, 13);

	for (int i = 0; i <= 3; i++)
		_scene->_sequences.setDepth(_globals._sequenceIndexes[i], 0);

	if (_globals[kFishIn108]) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(41, 109));
		int idx = _scene->_dynamicHotspots.add(101, 348, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(41, 109), FACING_NORTHWEST);
	}

	if (_scene->_priorSceneId == 107)
		_game._player._playerPos = Common::Point(138, 58);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(305, 98);

	_game.loadQuoteSet(0x4A, 0x4B, 0x4C, 0x35, 0x34, 0);
	section_1_music();
}

static void room_108_daemon() {
	// No implementation
}

static void room_108_pre_parser() {
	if (_action.isAction(VERB_SWIM_UNDER, NOUN_OVERHANG_TO_EAST))
		_game._player._walkOffScreenSceneId = 109;
}

static void room_108_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10812);
	else if (_action.isAction(VERB_TAKE, NOUN_DEAD_FISH) && _globals[kFishIn108]) {
		if (_game._objects.isInInventory(OBJ_DEAD_FISH)) {
			int randVal = _vm->getRandomNumber(74, 76);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_game._objects.addToInventory(OBJ_DEAD_FISH);
			_globals[kFishIn108] = false;
			_vm->_dialogs->showItem(OBJ_DEAD_FISH, 10808);
		}
	} else if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_OPEN_AREA_TO_NORTH))
		_scene->_nextSceneId = 107;
	else if (_action.isAction(VERB_LOOK, NOUN_CLIFF_FACE))
		_vm->_dialogs->show(10801);
	else if (_action.isAction(VERB_LOOK, NOUN_OCEAN_FLOOR))
		_vm->_dialogs->show(10802);
	else if (_action.isAction(VERB_LOOK, NOUN_ODD_ROCK_FORMATION))
		_vm->_dialogs->show(10803);
	else if (_action.isAction(VERB_TAKE, NOUN_ODD_ROCK_FORMATION))
		_vm->_dialogs->show(10804);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCKS))
		_vm->_dialogs->show(10805);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCKS))
		_vm->_dialogs->show(10806);
	else if (_action.isAction(VERB_LOOK, NOUN_DEAD_FISH))
		_vm->_dialogs->show(10807);
	else if (_action.isAction(VERB_LOOK, NOUN_OVERHANG_TO_EAST))
		_vm->_dialogs->show(10809);
	else if (_action.isAction(VERB_LOOK, NOUN_OPEN_AREA_TO_NORTH))
		_vm->_dialogs->show(10810);
	else if (_action.isAction(VERB_LOOK, NOUN_SURFACE))
		_vm->_dialogs->show(10811);
	else
		return;

	_action._inProgress = false;
}

void room_108_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_108_preload() {
	room_init_code_pointer = room_108_init;
	room_pre_parser_code_pointer = room_108_pre_parser;
	room_parser_code_pointer = room_108_parser;
	room_daemon_code_pointer = room_108_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
