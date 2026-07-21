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
#include "mads/nebular/rooms/section6.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _handsetHotspotId;
	int16 _checkVal;
	bool _cellCharging;
	int32 _cellChargingTimer;
	int32 _lastFrameTimer;
};

static Scratch local;


static void room_610_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('p', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 1));

	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 60, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 13);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 30, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);

	if (!_game._visitedScenes._sceneRevisited)
		local._cellCharging = false;

	if (_game._objects[OBJ_PHONE_HANDSET]._roomNumber == _scene->_currentSceneId) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		local._handsetHotspotId = _scene->_dynamicHotspots.add(words_phone_handset, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._handsetHotspotId, Common::Point(132, 121), FACING_NORTHWEST);
		if ((_globals[kHandsetCellStatus] == 2) && (_game._difficulty == DIFFICULTY_HARD) && !_globals[kDurafailRecharged])
			_globals[kHandsetCellStatus] = 1;
	}

	if (_scene->_roomChanged && _game._difficulty != DIFFICULTY_EASY)
		_game._objects.addToInventory(OBJ_PENLIGHT);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(175, 152);
		_game._player._facing = FACING_NORTHWEST;
	}

	section_6_music();
}

static void room_610_daemon() {
	if (local._cellCharging) {
		long diff = _scene->_frameStartTime - local._lastFrameTimer;
		if ((diff >= 0) && (diff <= 60))
			local._cellChargingTimer += diff;
		else
			local._cellChargingTimer++;

		local._lastFrameTimer = _scene->_frameStartTime;
	}

	// CHECKME: local._checkVal is always false, could be removed
	if ((local._cellChargingTimer >= 60) && !local._checkVal) {
		local._checkVal = true;
		_globals[kHandsetCellStatus] = 1;
		local._cellCharging = false;
		local._checkVal = false;
		local._cellChargingTimer = 0;
	}
}

static void room_610_parser() {
	if (player_said_2(exit_from, video_store))
		_scene->_nextSceneId = 609;
	else if (player_said_2(take, phone_handset)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_PHONE_HANDSET)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(local._handsetHotspotId);
				_game._objects.addToInventory(OBJ_PHONE_HANDSET);
				_vm->_dialogs->showItem(OBJ_PHONE_HANDSET, 61017);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (player_said_3(put, phone_handset, phone_cradle)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
			local._handsetHotspotId = _scene->_dynamicHotspots.add(words_phone_handset, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(local._handsetHotspotId, Common::Point(132, 121), FACING_NORTHWEST);
			_game._objects.setRoom(OBJ_PHONE_HANDSET, _scene->_currentSceneId);
			break;

		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			if ((_globals[kHandsetCellStatus] == 2) && (_game._difficulty == DIFFICULTY_HARD) && !_globals[kDurafailRecharged])
				local._cellCharging = true;

			_vm->_dialogs->show(61032);
			break;

		default:
			break;
		}
	} else if (player_said_2(look, pippy_billboard))
		_vm->_dialogs->show(61010);
	else if (player_said_2(look, civilization_ad))
		_vm->_dialogs->show(61011);
	else if (player_said_2(look, marx_bros_poster))
		_vm->_dialogs->show(61012);
	else if (player_said_2(look, video_monitor))
		_vm->_dialogs->show(61013);
	else if (player_said_2(look, video_store))
		_vm->_dialogs->show(61014);
	else if (_action._lookFlag)
		_vm->_dialogs->show(61015);
	else if (player_said_2(look, logo))
		_vm->_dialogs->show(61018);
	else if (player_said_2(look, cement)) {
		if (_game._visitedScenes.exists(601))
			_vm->_dialogs->show(61020);
		else
			_vm->_dialogs->show(61019);
	} else if (player_said_2(look, counter))
		_vm->_dialogs->show(61021);
	else if (player_said_2(look, phone_antenna))
		_vm->_dialogs->show(61022);
	else if (player_said_2(look, smelly_sneaker))
		_vm->_dialogs->show(61023);
	else if (player_said_2(take, smelly_sneaker))
		_vm->_dialogs->show(61024);
	else if (player_said_2(look, spotlight))
		_vm->_dialogs->show(61025);
	else if (player_said_2(look, phone_handset) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(61026);
	else if (player_said_2(look, phone_cradle))
		_vm->_dialogs->show(61027);
	else if (player_said_2(look, return_slot))
		_vm->_dialogs->show(61028);
	else if (player_said_2(put, return_slot)
		&& _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId)))
		_vm->_dialogs->show(61029);
	else if (player_said_1(classic_videos) || player_said_1(more_classic_videos) || player_said_1(drama_videos)
		|| player_said_1(new_release_videos) || player_said_1(porno_videos) || player_said_1(educational_videos)
		|| player_said_1(instructional_videos) || player_said_1(workout_videos) || player_said_1(foreign_videos)
		|| player_said_1(adventure_videos) || player_said_1(comedy_videos)) {
		if (player_said_1(look))
			_vm->_dialogs->show(61030);
		else if (player_said_1(take))
			_vm->_dialogs->show(61031);
		else
			return;
	} else
		return;

	_action._inProgress = false;
}

void room_610_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._handsetHotspotId);
	s.syncAsSint16LE(local._checkVal);

	s.syncAsByte(local._cellCharging);

	s.syncAsSint32LE(local._cellChargingTimer);
	s.syncAsUint32LE(local._lastFrameTimer);
}

void room_610_preload() {
	room_init_code_pointer = room_610_init;
	room_daemon_code_pointer = room_610_daemon;
	room_parser_code_pointer = room_610_parser;

	section_6_walker();
	section_6_interface();
	_scene->addActiveVocab(words_phone_handset);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
