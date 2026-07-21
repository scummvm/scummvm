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
	bool _explosionFl;
};

static Scratch local;


static void room_105_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('m', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 4));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 0, 0, 0);

	if (_globals[kFishIn105]) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(48, 144));

		int idx = _scene->_dynamicHotspots.add(words_dead_fish, words_swim_to, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(56, 141), FACING_NORTHWEST);
	}

	if (_scene->_priorSceneId == 104)
		_game._player._playerPos = Common::Point(13, 97);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(116, 147);

	_game.loadQuoteSet(0x4A, 0x4B, 0x4C, 0x35, 0x34, 0);
	local._explosionFl = false;

	section_1_music();
}

static void room_105_daemon() {
	if ((_game._player._playerPos == Common::Point(170, 87)) && (_game._trigger || !local._explosionFl)) {
		local._explosionFl = true;
		switch (_game._trigger) {
		case 0:
			_scene->_kernelMessages.reset();
			_scene->resetScene();
			_game._player._stepEnabled = false;
			_game._player._visible = false;

			_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('m', 0));
			_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('m', 2));
			_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('m', 3));
			_vm->_sound->command(33);
			_scene->clearSequenceList();
			_vm->_palette->refreshSceneColors();

			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 1);

			if (_game._storyMode >= STORYMODE_NICE)
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 8, 3);
			break;

		case 1:
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 9, 1, 0, 0);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], _globals._sequenceIndexes[0]);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 5, 7);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
		{
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 9, 0, 0, 0);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
			_scene->_sequences.addTimer(90, 3);
		}
		break;

		case 3:
			_vm->_dialogs->show(10507);
			_scene->_reloadSceneFlag = true;
			_scene->_sequences.addTimer(90, 4);
			break;

		default:
			break;
		}
	}

	if (_game._player._moving && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(170, 87), FACING_NONE);
		_scene->_rails.resetNext();
	}

	if ((_game._player._special > 0) && _game._player._stepEnabled)
		_game._player._stepEnabled = false;
}

static void room_105_pre_parser() {
	if (player_said_2(swim_towards, western_cliff_face))
		_game._player._walkOffScreenSceneId = 104;

	if (player_said_2(swim_towards, open_area_to_south))
		_game._player._walkOffScreenSceneId = 107;

	if (player_said_1(mine) && (player_said_1(talkto) || player_said_1(look)))
		_game._player._needToWalk = false;
}

static void room_105_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10512);
	else if (player_said_2(take, dead_fish) && _globals[kFishIn105]) {
		if (_game._objects.isInInventory(OBJ_DEAD_FISH)) {
			int randVal = _vm->getRandomNumber(74, 76);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_game._objects.addToInventory(OBJ_DEAD_FISH);
			_globals[kFishIn105] = false;
			_vm->_dialogs->showItem(OBJ_DEAD_FISH, 802, 0);
		}
	} else if (player_said_2(look, western_cliff_face))
		_vm->_dialogs->show(10501);
	else if (player_said_2(look, cliff_face))
		_vm->_dialogs->show(10502);
	else if (player_said_2(look, ocean_floor))
		_vm->_dialogs->show(10503);
	else if (player_said_2(look, medical_waste))
		_vm->_dialogs->show(10504);
	else if (player_said_2(take, medical_waste))
		_vm->_dialogs->show(10505);
	else if (player_said_2(look, mine))
		_vm->_dialogs->show(10506);
	else if (player_said_2(look, dead_fish))
		_vm->_dialogs->show(10508);
	else if (player_said_2(look, surface))
		_vm->_dialogs->show(10509);
	else if (player_said_2(look, open_area_to_south))
		_vm->_dialogs->show(10510);
	else if (player_said_2(look, rocks))
		_vm->_dialogs->show(10511);
	else
		return;

	_action._inProgress = false;
}

void room_105_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._explosionFl);
}

void room_105_preload() {
	room_init_code_pointer = room_105_init;
	room_pre_parser_code_pointer = room_105_pre_parser;
	room_parser_code_pointer = room_105_parser;
	room_daemon_code_pointer = room_105_daemon;

	anim_himem_preload(formAnimName('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
