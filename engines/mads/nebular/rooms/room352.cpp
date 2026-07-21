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
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _vaultOpenFl;
	bool _mustPutArmDownFl;
	bool _leaveRoomFl;
	int32 _tapePlayerHotspotIdx;
	int32 _hotspot1Idx;
	int32 _hotspot2Idx;
	int32 _lampHostpotIdx;
	int32 _commonSequenceIdx;
	int32 _commonSpriteIndex;
};

static Scratch local;


static void putArmDown(bool corridorExit, bool doorwayExit) {
	switch (_game._trigger) {
	case 0:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0xFF));
		_scene->_sequences.addTimer(48, 1);
		break;

	case 1:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		if (_globals[kSexOfRex] == REX_FEMALE) {
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 5, 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		} else {
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 6, 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		}
		break;

	case 2:
	{
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 0);
		int idx = _scene->_dynamicHotspots.add(words_guards_arm2, words_walkto, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(230, 117), FACING_NORTHWEST);
		_scene->changeVariant(0);
	}
	break;

	case 3:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x100));
		_game._objects.setRoom(OBJ_GUARDS_ARM, _scene->_currentSceneId);
		_game._player._visible = true;
		if (corridorExit)
			_scene->_sequences.addTimer(48, 6);
		else if (doorwayExit)
			_scene->_sequences.addTimer(48, 4);
		else {
			local._mustPutArmDownFl = false;
			_game._player._stepEnabled = true;
		}
		break;

	case 4:
		_game._player.walk(Common::Point(116, 107), FACING_NORTH);
		local._mustPutArmDownFl = false;
		_scene->_sequences.addTimer(180, 5);
		local._leaveRoomFl = true;
		break;

	case 5:
		if (local._leaveRoomFl)
			_scene->_nextSceneId = 351;

		break;

	case 6:
		_game._player.walk(Common::Point(171, 152), FACING_SOUTH);
		_game._player._stepEnabled = true;
		local._mustPutArmDownFl = false;
		_scene->_sequences.addTimer(180, 7);
		local._leaveRoomFl = true;
		break;

	case 7:
		if (local._leaveRoomFl)
			_scene->_nextSceneId = 353;

		break;

	default:
		break;
	}
}

static void room_352_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RM302x0");
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites("*RM302x2");
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RM302x3");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('g', -1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('b', -1));


	if (_globals[kSexOfRex] == REX_FEMALE) {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*ROXRC_7");
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*ROXRC_6");
		_globals._spriteIndexes[15] = _scene->_sprites.addSprites("*ROXRC_9");
		_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 3));
		_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('a', 2));
	} else {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXRD_7");
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*RXRC_6");
		_globals._spriteIndexes[14] = _scene->_sprites.addSprites("*RXMRC_9");
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('a', 1));
		_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 0));
	}

	local._leaveRoomFl = false;

	if (_game._objects.isInRoom(OBJ_TAPE_PLAYER)) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 5);
		int idx = _scene->_dynamicHotspots.add(words_tape_player, words_walkto, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		local._tapePlayerHotspotIdx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(84, 145), FACING_WEST);
	}

	local._vaultOpenFl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		local._mustPutArmDownFl = false;
		if (!_game._visitedScenes._sceneRevisited)
			_globals[kHaveYourStuff] = false;
	}

	if (_game._objects.isInRoom(OBJ_GUARDS_ARM)) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 0);
		int idx = _scene->_dynamicHotspots.add(words_guards_arm2, words_walkto, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(230, 117), FACING_NORTHWEST);
	} else
		local._mustPutArmDownFl = true;

	if (_scene->_priorSceneId == 353)
		_game._player._playerPos = Common::Point(171, 155);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(116, 107);

	section_3_music();

	_game.loadQuoteSet(0xFF, 0x100, 0x101, 0x102, 0x103, 0);
}

static void room_352_pre_parser() {
	local._leaveRoomFl = false;

	if (player_said_2(open, vault))
		_game._player.walk(Common::Point(266, 111), FACING_NORTHEAST);

	if (local._vaultOpenFl && !player_said_1(vault) && !player_said_1(lamp) && !player_said_1(other_stuff) && !player_said_1(your_stuff)) {
		if (_globals[kHaveYourStuff]) {
			local._commonSpriteIndex = _globals._spriteIndexes[13];
			local._commonSequenceIdx = _globals._sequenceIndexes[13];
		} else {
			local._commonSpriteIndex = _globals._spriteIndexes[1];
			local._commonSequenceIdx = _globals._sequenceIndexes[1];
		}

		switch (_game._trigger) {
		case 0:
			if (_game._player._needToWalk) {
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(local._commonSequenceIdx);
				_vm->_sound->command(20);
				local._commonSequenceIdx = _scene->_sequences.addReverseSpriteCycle(local._commonSpriteIndex, false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.setDepth(local._commonSequenceIdx, 15);
			}
			break;

		case 1:
			if (!_globals[kHaveYourStuff])
				_scene->_dynamicHotspots.remove(local._hotspot2Idx);

			_scene->_dynamicHotspots.remove(local._hotspot1Idx);
			_scene->_dynamicHotspots.remove(local._lampHostpotIdx);
			local._vaultOpenFl = false;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	}

	if (player_said_3(put, guards_arm2, scanner)) {
		if (_globals[kSexOfRex] == REX_MALE)
			_game._player.walk(Common::Point(269, 111), FACING_NORTHEAST);
		else
			_game._player.walk(Common::Point(271, 111), FACING_NORTHEAST);
	}

	if (player_said_2(walk_through, doorway) || player_said_2(walk_down, corridor_to_south) || player_said_3(put, guards_arm2, floor)) {
		if (_game._objects.isInInventory(OBJ_GUARDS_ARM))
			_game._player.walk(Common::Point(230, 117), FACING_NORTHWEST);
	}
}

static void room_352_parser() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(35225);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(open, vault)) {
		if (!local._vaultOpenFl) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE)
					local._commonSpriteIndex = _globals._spriteIndexes[9];
				else
					local._commonSpriteIndex = _globals._spriteIndexes[8];

				local._commonSequenceIdx = _scene->_sequences.addSpriteCycle(local._commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.updateTimeout(local._commonSequenceIdx, -1);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
			{
				_vm->_sound->command(21);
				_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 7, 2, 20, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], FACING_NORTH);
				int oldIdx = local._commonSequenceIdx;
				local._commonSequenceIdx = _scene->_sequences.startCycle(local._commonSpriteIndex, false, -2);
				_scene->_sequences.updateTimeout(local._commonSequenceIdx, oldIdx);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

			case 2:
				_vm->_sound->command(22);
				_scene->_sequences.remove(local._commonSequenceIdx);
				local._commonSequenceIdx = _scene->_sequences.startPingPongCycle(local._commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(local._commonSequenceIdx, 1, 3);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_scene->_sequences.updateTimeout(-1, local._commonSequenceIdx);
				_game._player._visible = true;
				_scene->_sequences.addTimer(60, 4);
				break;

			case 4:
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x101));
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_game._objects.isInInventory(OBJ_GUARDS_ARM)) {
		local._mustPutArmDownFl = true;
	}

	bool exit_corridor = false;
	bool exit_doorway = false;

	if (player_said_2(walk_down, corridor_to_south)) {
		exit_corridor = true;
	}

	if (player_said_2(walk_through, doorway)) {
		exit_doorway = true;
	}

	if (player_said_2(walk_down, corridor_to_south) || player_said_2(walk_through, doorway) || player_said_3(put, guards_arm2, floor)) {
		if (local._mustPutArmDownFl)
			putArmDown(exit_corridor, exit_doorway);
		else if (exit_corridor)
			_scene->_nextSceneId = 353;
		else
			_scene->_nextSceneId = 351;

		_action._inProgress = false;
		return;
	}

	if (player_said_2(take, guards_arm2)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_GUARDS_ARM)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE) {
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 6, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				_scene->_sequences.remove(_globals._sequenceIndexes[2]);
				_game._objects.addToInventory(OBJ_GUARDS_ARM);
				_scene->changeVariant(1);
				break;

			case 2:
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_vm->_dialogs->showItem(OBJ_GUARDS_ARM, 0x899C);
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (player_said_3(put, guards_arm2, scanner)) {
		if (!local._vaultOpenFl) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE)
					local._commonSpriteIndex = _globals._spriteIndexes[11];
				else
					local._commonSpriteIndex = _globals._spriteIndexes[10];

				local._commonSequenceIdx = _scene->_sequences.addSpriteCycle(local._commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.updateTimeout(local._commonSequenceIdx, -1);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
			{
				_vm->_sound->command(21);
				_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 7, 2, 20, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);
				int oldIdx = local._commonSequenceIdx;
				local._commonSequenceIdx = _scene->_sequences.startCycle(local._commonSpriteIndex, false, -2);
				_scene->_sequences.updateTimeout(local._commonSequenceIdx, oldIdx);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

			case 2:
				_vm->_sound->command(23);
				_scene->_sequences.remove(local._commonSequenceIdx);
				local._commonSequenceIdx = _scene->_sequences.addReverseSpriteCycle(local._commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(local._commonSequenceIdx, 1, 4);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;


			case 3:
				_scene->_sequences.updateTimeout(-1, local._commonSequenceIdx);
				_game._player._visible = true;
				if (_globals[kHaveYourStuff])
					local._commonSpriteIndex = _globals._spriteIndexes[13];
				else
					local._commonSpriteIndex = _globals._spriteIndexes[1];

				_vm->_sound->command(20);
				local._commonSequenceIdx = _scene->_sequences.addSpriteCycle(local._commonSpriteIndex, false, 6, 1, 0, 0);
				_scene->_sequences.setDepth(local._commonSequenceIdx, 15);
				_scene->_sequences.addSubEntry(local._commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 4);
				break;

			case 4:
				local._commonSequenceIdx = _scene->_sequences.addSpriteCycle(local._commonSpriteIndex, false, 6, 0, 0, 0);
				_scene->_sequences.setAnimRange(local._commonSequenceIdx, -2, -2);
				_scene->_sequences.setDepth(local._commonSequenceIdx, 15);
				_scene->_sequences.addTimer(60, 5);
				break;

			case 5:
			{
				local._vaultOpenFl = true;
				int idx;
				if (!_globals[kHaveYourStuff]) {
					idx = _scene->_dynamicHotspots.add(words_your_stuff, words_walkto, -1, Common::Rect(282, 87, 282 + 13, 87 + 7));
					local._hotspot2Idx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(280, 111), FACING_NORTHEAST);
					_globals._sequenceIndexes[1] = local._commonSequenceIdx;
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x102));
				} else {
					_globals._sequenceIndexes[13] = local._commonSequenceIdx;
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x103));
				}

				idx = _scene->_dynamicHotspots.add(words_other_stuff, words_walkto, -1, Common::Rect(282, 48, 282 + 36, 48 + 27));
				local._hotspot1Idx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(287, 115), FACING_NORTHEAST);
				idx = _scene->_dynamicHotspots.add(words_lamp, words_walkto, -1, Common::Rect(296, 76, 296 + 11, 76 + 17));
				local._lampHostpotIdx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(287, 115), FACING_NORTHEAST);
				_game._player._stepEnabled = true;
			}
			break;

			default:
				break;
			}
		}
	} else if (player_said_2(take, your_stuff)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			if (_globals[kSexOfRex] == REX_MALE) {
				_globals._sequenceIndexes[14] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[14], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[14], 1, 2);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[14]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[14], SEQUENCE_TRIGGER_SPRITE, 2, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[14], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			} else {
				_globals._sequenceIndexes[15] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[15], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[15], 1, 2);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[15]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[15], SEQUENCE_TRIGGER_SPRITE, 2, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[15], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 1:
			_scene->_dynamicHotspots.remove(local._hotspot2Idx);
			_globals[kHaveYourStuff] = true;

			for (uint16 i = 0; i < _game._objects.size(); i++) {
				if (_game._objects[i]._roomNumber == 50)
					_game._objects.addToInventory(i);
			}

			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 15);
			break;

		case 2:
			if (_globals[kSexOfRex] == REX_MALE)
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[14]);
			else
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[15]);

			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(take, tape_player) && !_game._objects.isInInventory(OBJ_TAPE_PLAYER)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			if (_globals[kSexOfRex] == REX_MALE) {
				_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], true, 6, 2, 0, 0);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_SPRITE, 6, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			} else {
				_globals._sequenceIndexes[7] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[7], true, 6, 2, 0, 0);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[7]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_SPRITE, 6, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			_scene->_dynamicHotspots.remove(local._tapePlayerHotspotIdx);
			break;

		case 2:
			_game._objects.addToInventory(OBJ_TAPE_PLAYER);
			if (_globals[kSexOfRex] == REX_MALE)
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[6]);
			else
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[7]);

			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_vm->_dialogs->showItem(OBJ_TAPE_PLAYER, 35227);
			break;

		default:
			break;
		}
	} else if (player_said_2(look, scanner))
		_vm->_dialogs->show(35210);
	else if (player_said_2(look, monitor)) {
		if (_game._storyMode == STORYMODE_NAUGHTY)
			_vm->_dialogs->show(35211);
		else
			_vm->_dialogs->show(35212);
	} else if (player_said_2(look, display))
		_vm->_dialogs->show(35213);
	else if (player_said_2(look, statue))
		_vm->_dialogs->show(35214);
	else if (player_said_2(look, tape_player) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(35215);
	else if (player_said_2(look, air_vent))
		_vm->_dialogs->show(35216);
	else if (player_said_2(look, guards_arm2) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(35217);
	else if (player_said_2(look, ironing_board))
		_vm->_dialogs->show(35218);
	else if (player_said_2(look, clock))
		_vm->_dialogs->show(35219);
	else if (player_said_2(look, gauge))
		_vm->_dialogs->show(35220);
	else if (player_said_2(look, vault)) {
		if (!local._vaultOpenFl)
			_vm->_dialogs->show(35221);
	} else if (player_said_2(look, your_stuff))
		_vm->_dialogs->show(35222);
	else if (player_said_2(look, other_stuff))
		_vm->_dialogs->show(35223);
	else if (player_said_2(look, corridor_to_south))
		_vm->_dialogs->show(35224);
	else if (player_said_2(take, other_stuff))
		_vm->_dialogs->show(35226);
	else if (player_said_2(look, desk))
		_vm->_dialogs->show(35229);
	else if (player_said_2(look, guard))
		_vm->_dialogs->show(35230);
	else if (player_said_2(look, doorway))
		_vm->_dialogs->show(35231);
	else if (player_said_2(look, table))
		_vm->_dialogs->show(35232);
	else if (player_said_2(look, projector))
		_vm->_dialogs->show(35233);
	else if (player_said_2(look, support))
		_vm->_dialogs->show(35234);
	else if (player_said_2(look, security_monitor))
		_vm->_dialogs->show(35235);
	else
		return;

	_action._inProgress = false;
}

void room_352_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._vaultOpenFl);
	s.syncAsByte(local._mustPutArmDownFl);
	s.syncAsByte(local._leaveRoomFl);

	s.syncAsSint32LE(local._tapePlayerHotspotIdx);
	s.syncAsSint32LE(local._hotspot1Idx);
	s.syncAsSint32LE(local._hotspot2Idx);
	s.syncAsSint32LE(local._lampHostpotIdx);
	s.syncAsSint32LE(local._commonSequenceIdx);
	s.syncAsSint32LE(local._commonSpriteIndex);
}

void room_352_preload() {
	room_init_code_pointer = room_352_init;
	room_pre_parser_code_pointer = room_352_pre_parser;
	room_parser_code_pointer = room_352_parser;

	section_3_walker();
	section_3_interface();
	_scene->addActiveVocab(words_walkto);
	_scene->addActiveVocab(words_your_stuff);
	_scene->addActiveVocab(words_other_stuff);
	_scene->addActiveVocab(words_lamp);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
