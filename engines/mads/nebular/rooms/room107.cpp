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
	bool _shootingFl;
};

static Scratch local;


static void room_107_init() {
	for (int i = 0; i < 3; i++)
		_globals._spriteIndexes[i + 1] = _scene->_sprites.addSprites(formAnimName('G', i));

	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(Resources::formatName(105, 'f', 4, EXT_SS, ""));

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 14, 0, 0, 7);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 17, 0, 0, 13);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 19, 0, 0, 9);

	for (int i = 1; i < 4; i++)
		_scene->_sequences.setDepth(_globals._sequenceIndexes[i], 0);

	if (_globals[kFishIn107]) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(68, 151));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		int idx = _scene->_dynamicHotspots.add(words_dead_fish, words_swim_to, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(78, 135), FACING_SOUTHWEST);
	}

	if (_scene->_priorSceneId == 105)
		_game._player._playerPos = Common::Point(132, 47);
	else if (_scene->_priorSceneId == 106)
		_game._player._playerPos = Common::Point(20, 91);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(223, 151);

	if (((_scene->_priorSceneId == 105) || (_scene->_priorSceneId == 106)) && (_vm->getRandomNumber(1, 3) == 1)) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(Resources::formatName(105, 'R', 1, EXT_SS, ""));
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], true, 4, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[0], Common::Point(270, 150));
		_scene->_sequences.setMotion(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, -200, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
		_scene->_dynamicHotspots.add(words_manta_ray, words_swim_to, _globals._sequenceIndexes[0], Common::Rect(0, 0, 0, 0));
	}

	_game.loadQuoteSet(0x4A, 0x4B, 0x4C, 0x35, 0x34, 0);
	local._shootingFl = false;

	if (_vm->getRandomNumber(1, 3) == 1) {
		_scene->loadAnimation(Resources::formatName(107, 'B', -1, EXT_AA, ""), 0);
		local._shootingFl = true;
	}

	section_1_music();
}

static void room_107_daemon() {
	if (local._shootingFl && (_scene->_animation[0]->getCurrentFrame() >= 19)) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(52));
		local._shootingFl = false;
	}
}

static void room_107_pre_parser() {
	if (player_said_2(swim_towards, open_area_to_west))
		_game._player._walkOffScreenSceneId = 106;

	if (player_said_2(swim_towards, open_area_to_south))
		_game._player._walkOffScreenSceneId = 108;
}

static void room_107_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10708);
	else if (player_said_2(take, dead_fish) && _globals[kFishIn107]) {
		if (_game._objects.isInInventory(OBJ_DEAD_FISH)) {
			int randVal = _vm->getRandomNumber(74, 76);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_game._objects.addToInventory(OBJ_DEAD_FISH);
			_globals[kFishIn107] = false;
			_vm->_dialogs->showItem(OBJ_DEAD_FISH, 802);
		}
	} else if (player_said_2(swim_towards, northern_sea_cliff))
		_scene->_nextSceneId = 105;
	else if (player_said_2(look, northern_sea_cliff))
		_vm->_dialogs->show(10701);
	else if (player_said_2(look, dead_fish) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(10702);
	else if (player_said_2(look, bush_like_formation))
		_vm->_dialogs->show(10703);
	else if (player_said_2(look, rock))
		_vm->_dialogs->show(10704);
	else if (player_said_2(look, seaweed))
		_vm->_dialogs->show(10705);
	else if (player_said_2(look, open_area_to_south))
		_vm->_dialogs->show(10706);
	else if (player_said_2(look, cliff_face))
		_vm->_dialogs->show(10707);
	else if (player_said_2(look, manta_ray))
		_vm->_dialogs->show(10709);
	else if (player_said_2(take, manta_ray))
		_vm->_dialogs->show(10710);
	else
		return;

	_action._inProgress = false;
}

void room_107_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._shootingFl);
}

void room_107_preload() {
	room_init_code_pointer = room_107_init;
	room_pre_parser_code_pointer = room_107_pre_parser;
	room_parser_code_pointer = room_107_parser;
	room_daemon_code_pointer = room_107_daemon;

	section_1_walker();
	section_1_interface();
	vocab_make_active(words_manta_ray);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
