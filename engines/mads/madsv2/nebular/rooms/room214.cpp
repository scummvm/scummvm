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
	int32 _devilTime;
	bool _devilRunningFl;
};

static Scratch local;


static void room_214_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('e', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('e', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('t', -1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXMRD_7");

	local._devilTime = _game._player._priorTimer;
	local._devilRunningFl = false;

	if (_game._objects.isInRoom(OBJ_POISON_DARTS)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(103, 86));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 11);
	} else {
		_scene->_hotspots.activate(words_poison_darts, false);
	}

	if (_game._objects.isInRoom(OBJ_BLOWGUN)) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(90, 87));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
	} else {
		_scene->_hotspots.activate(words_blowgun, false);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(191, 152);

	section_2_music();
}

static void room_214_daemon() {
	if ((_game._player._priorTimer - local._devilTime > 800) && !local._devilRunningFl) {
		local._devilRunningFl = true;
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 6, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 2);
		_scene->_dynamicHotspots.add(words_captive_creature, words_walkto, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
	}

	if (local._devilRunningFl) {
		switch (_game._trigger) {
		case 71:
		{
			int oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 5, 0, 0);
			_scene->_sequences.updateTimeout(oldIdx, _globals._sequenceIndexes[3]);
			_scene->_dynamicHotspots.add(words_captive_creature, words_walkto, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 5, 8);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		}
		break;

		case 72:
		{
			int oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
			_scene->_sequences.updateTimeout(oldIdx, _globals._sequenceIndexes[3]);
			_scene->_dynamicHotspots.add(words_captive_creature, words_walkto, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 9, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 2);
			local._devilTime = _game._player._priorTimer;
			local._devilRunningFl = false;
		}
		break;

		default:
			break;
		}
	}
}

static void room_214_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(21427);
	else if (player_said_2(walk_outside, hut))
		_scene->_nextSceneId = 207;
	else if (player_said_2(take, poison_darts) && (_game._trigger || _game._objects.isInRoom(OBJ_POISON_DARTS))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], true, 6, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], true, 6, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_game._objects.addToInventory(OBJ_POISON_DARTS);
			_scene->_hotspots.activate(words_poison_darts, false);
			break;

		case 2:
			_game._player._visible = true;
			_scene->_sequences.addTimer(48, 3);
			break;

		case 3:
			_game._player._stepEnabled = true;
			_vm->_dialogs->showItem(OBJ_POISON_DARTS, 0x53A5);
			break;

		default:
			break;
		}
	} else if (player_said_2(take, blowgun) && (_game._trigger || _game._objects.isInRoom(OBJ_BLOWGUN))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 6, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_game._objects.addToInventory(OBJ_BLOWGUN);
			_scene->_hotspots.activate(words_blowgun, false);
			break;

		case 2:
			_game._player._visible = true;
			_scene->_sequences.addTimer(48, 3);
			break;

		case 3:
			_game._player._stepEnabled = true;
			_vm->_dialogs->showItem(OBJ_BLOWGUN, 0x329);
			break;

		default:
			break;
		}
	} else if (player_said_2(look, window))
		_vm->_dialogs->show(21401);
	else if (player_said_2(look, experiment_cage))
		_vm->_dialogs->show(21402);
	else if (player_said_2(look, captive_creature))
		_vm->_dialogs->show(21403);
	else if (player_said_2(look, bear_rug))
		_vm->_dialogs->show(21404);
	else if (player_said_2(look, trophy))
		_vm->_dialogs->show(21405);
	else if (player_said_2(look, large_bowl)) {
		if (_game._storyMode == STORYMODE_NAUGHTY) {
			_vm->_dialogs->show(21406);
		} else {
			_vm->_dialogs->show(21407);
		}
	} else if (player_said_2(look, specimen_jars))
		_vm->_dialogs->show(21408);
	else if (player_said_2(take, large_bowl) || player_said_2(take, specimen_jars))
		_vm->_dialogs->show(21409);
	else if (player_said_2(look, shrunken_heads))
		_vm->_dialogs->show(21410);
	else if (player_said_2(take, shrunken_heads) || player_said_2(take, large_heads))
		_vm->_dialogs->show(21411);
	else if (player_said_2(look, large_heads))
		_vm->_dialogs->show(21428);
	else if (player_said_2(look, poison_darts) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(21412);
	else if (player_said_2(open, experiment_cage))
		_vm->_dialogs->show(21414);
	else if (player_said_2(talkto, captive_creature))
		_vm->_dialogs->show(21415);
	else if (player_said_3(give, twinkifruit, captive_creature))
		_vm->_dialogs->show(21416);
	else if (player_said_3(shoot, blowgun, captive_creature) || player_said_3(hose_down, blowgun, captive_creature))
		_vm->_dialogs->show(21417);
	else if (player_said_2(look, big_heads))
		_vm->_dialogs->show(21418);
	else if (player_said_2(take, big_heads))
		_vm->_dialogs->show(21419);
	else if (player_said_2(take, bear_rug))
		_vm->_dialogs->show(21420);
	else if (player_said_2(look, floor_of_hut))
		_vm->_dialogs->show(21421);
	else if (player_said_2(look, blowgun))
		_vm->_dialogs->show(21422);
	else if (player_said_2(look, table)) {
		if (_game._objects.isInRoom(OBJ_POISON_DARTS) && _game._objects.isInRoom(OBJ_BLOWGUN)) {
			_vm->_dialogs->show(21423);
		} else if (_game._objects.isInRoom(OBJ_POISON_DARTS) && !_game._objects.isInRoom(OBJ_BLOWGUN)) {
			_vm->_dialogs->show(21424);
		} else if (!_game._objects.isInRoom(OBJ_POISON_DARTS) && _game._objects.isInRoom(OBJ_BLOWGUN)) {
			_vm->_dialogs->show(21425);
		} else {
			_vm->_dialogs->show(21426);
		}
	} else
		return;

	_action._inProgress = false;
}

void room_214_synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(local._devilTime);
	s.syncAsByte(local._devilRunningFl);
}

void room_214_preload() {
	room_init_code_pointer = room_214_init;
	room_parser_code_pointer = room_214_parser;
	room_daemon_code_pointer = room_214_daemon;

	section_2_walker();
	section_2_interface();
	_scene->addActiveVocab(words_captive_creature);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
