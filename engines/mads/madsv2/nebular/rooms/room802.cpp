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
#include "mads/madsv2/nebular/rooms/section8.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_802_init() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRC_8");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXMBD_8");
	_globals[kBetweenRooms] = false;

	if ((_globals[kCameFromCut]) && (_globals[kCutX] != 0)) {
		_game._player._playerPos.x = _globals[kCutX];
		_game._player._playerPos.y = _globals[kCutY];
		_game._player._facing = _globals[kCutFacing];
		_globals[kCutX] = 0;
		_globals[kCameFromCut] = false;
		_globals[kReturnFromCut] = false;
		_globals[kBeamIsUp] = false;
		_globals[kForceBeamDown] = false;
		_globals[kDontRepeat] = false;
		_globals[kAntigravClock] = _scene->_frameStartTime;
	} else if (_scene->_priorSceneId == 801) {
		_game._player._playerPos = Common::Point(15, 129);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 803) {
		_game._player._playerPos = Common::Point(303, 119);
		_game._player._facing = FACING_WEST;

	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(15, 129);
		_game._player._facing = FACING_EAST;
	}

	_game._player._visible = true;



	if (_globals[kHasWatchedAntigrav] && !_globals[kRemoteSequenceRan]) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(200, 70);
	}

	if ((_globals[kRemoteOnGround]) && (!_game._objects.isInInventory(OBJ_REMOTE))) {
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 8);
		int idx = _scene->_dynamicHotspots.add(words_remote, words_walkto, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(107, 99), FACING_NORTH);
	}

	if (!_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled]) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
		int idx = _scene->_dynamicHotspots.add(words_shield_modulator, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(93, 97), FACING_NORTH);
	}
	section_8_music();
}

static void room_802_daemon() {
	if (_game._trigger == 70) {
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 19);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 4, 72);
	}

	if (_game._trigger == 71) {
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 8);
		int idx = _scene->_dynamicHotspots.add(words_remote, words_walkto, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(107, 99), FACING_NORTH);

		_globals[kRemoteSequenceRan] = true;
		_globals[kRemoteOnGround] = true;
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 72)
		_vm->_sound->command(13);
}

static void room_802_pre_parser() {
	if (_action.isAction(words_walk_towards, words_building_to_west))
		_game._player._walkOffScreenSceneId = 801;

	if (_action.isAction(words_walk_down, words_path_to_east)) {
		_game._player._walkOffScreenSceneId = 803;
		_globals[kForceBeamDown] = false;
	}

	if (_action.isAction(words_take, words_ship))
		_game._player._needToWalk = false;
}

static void room_802_parser() {
	if (_action.isAction(words_take, words_shield_modulator) && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 7, 2, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_vm->_sound->command(9);
			break;

		case 2:
			_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			_game._player._stepEnabled = true;
			_game._objects.addToInventory(OBJ_SHIELD_MODULATOR);
			_vm->_dialogs->showItem(OBJ_SHIELD_MODULATOR, 80215);
			break;

		default:
			break;
		}
	} else if ((_action.isAction(words_take, words_remote)) && (!_game._objects.isInInventory(OBJ_REMOTE))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], true, 7, 2, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 4);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_vm->_sound->command(9);
			_globals[kRemoteOnGround] = false;
			break;

		case 2:
			_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			_game._player._stepEnabled = true;
			_game._objects.addToInventory(OBJ_REMOTE);
			_vm->_dialogs->showItem(OBJ_REMOTE, 80223);
			break;

		default:
			break;
		}
	} else if (!_globals[kRemoteOnGround] && (_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) || _globals[kShieldModInstalled])
		&& (_action.isAction(words_look, words_launch_pad) || _action._lookFlag))
		_vm->_dialogs->show(80210);
	else if (!_globals[kRemoteOnGround] && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled]
		&& (_action.isAction(words_look, words_launch_pad) || _action._lookFlag))
		_vm->_dialogs->show(80211);
	else if (_globals[kRemoteOnGround] && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled]
		&& (_action.isAction(words_look, words_launch_pad) || _action._lookFlag))
		_vm->_dialogs->show(80213);
	else if (_globals[kRemoteOnGround] && (_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) || _globals[kShieldModInstalled])
		&& (_action.isAction(words_look, words_launch_pad) || _action._lookFlag))
		_vm->_dialogs->show(80212);
	else if (!_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled] && _action.isAction(words_look, words_shield_modulator))
		_vm->_dialogs->show(80214);
	else if (_globals[kRemoteOnGround] && _action.isAction(words_look, words_remote))
		_vm->_dialogs->show(80216);
	else if (_action.isAction(words_look, words_ship)) {
		if ((!_game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) && (!_globals[kShieldModInstalled]))
			_vm->_dialogs->show(80218);
		else
			_vm->_dialogs->show(80217);
	} else if (_action.isAction(words_look, words_bushes))
		_vm->_dialogs->show(80219);
	else if (_action.isAction(words_look, words_path_to_east))
		_vm->_dialogs->show(80220);
	else if (_action.isAction(words_look, words_sky))
		_vm->_dialogs->show(80221);
	else if (_action.isAction(words_take, words_ship))
		_vm->_dialogs->show(80222);
	else if (_action.isAction(words_look, words_tree) || _action.isAction(words_look, words_trees))
		_vm->_dialogs->show(80224);
	else if (_action.isAction(words_look, words_building_to_west))
		_vm->_dialogs->show(80225);
	else
		return;

	_action._inProgress = false;
}

void room_802_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_802_preload() {
	room_init_code_pointer = room_802_init;
	room_daemon_code_pointer = room_802_daemon;
	room_pre_parser_code_pointer = room_802_pre_parser;
	room_parser_code_pointer = room_802_parser;

	section_8_walker();
	section_8_interface();
	_scene->addActiveVocab(words_shield_modulator);
	_scene->addActiveVocab(words_walkto);
	_scene->addActiveVocab(words_remote);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
