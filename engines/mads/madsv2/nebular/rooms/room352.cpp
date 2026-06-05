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

#include "common/scummsys.h"
#include "math/utils.h"
#include "mads/madsv2/nebular/nebular.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

Scene352::Scene352(RexNebularEngine *vm) : Scene3xx(vm) {
	_vaultOpenFl = false;
	_mustPutArmDownFl = false;
	_leaveRoomFl = false;

	_tapePlayerHotspotIdx = -1;
	_hotspot1Idx = -1;
	_hotspot2Idx = -1;
	_lampHostpotIdx = -1;
	_commonSequenceIdx = -1;
	_commonSpriteIndex = -1;
}

void Scene352::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsByte(_vaultOpenFl);
	s.syncAsByte(_mustPutArmDownFl);
	s.syncAsByte(_leaveRoomFl);

	s.syncAsSint32LE(_tapePlayerHotspotIdx);
	s.syncAsSint32LE(_hotspot1Idx);
	s.syncAsSint32LE(_hotspot2Idx);
	s.syncAsSint32LE(_lampHostpotIdx);
	s.syncAsSint32LE(_commonSequenceIdx);
	s.syncAsSint32LE(_commonSpriteIndex);
}

void Scene352::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_YOUR_STUFF);
	_scene->addActiveVocab(NOUN_OTHER_STUFF);
	_scene->addActiveVocab(NOUN_LAMP);
}

void Scene352::putArmDown(bool corridorExit, bool doorwayExit) {
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
		int idx = _scene->_dynamicHotspots.add(NOUN_GUARDS_ARM2, VERB_WALKTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
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
			_mustPutArmDownFl = false;
			_game._player._stepEnabled = true;
		}
		break;

	case 4:
		_game._player.walk(Common::Point(116, 107), FACING_NORTH);
		_mustPutArmDownFl = false;
		_scene->_sequences.addTimer(180, 5);
		_leaveRoomFl = true;
		break;

	case 5:
		if (_leaveRoomFl)
			_scene->_nextSceneId = 351;

		break;

	case 6:
		_game._player.walk(Common::Point(171, 152), FACING_SOUTH);
		_game._player._stepEnabled = true;
		_mustPutArmDownFl = false;
		_scene->_sequences.addTimer(180, 7);
		_leaveRoomFl = true;
		break;

	case 7:
		if (_leaveRoomFl)
			_scene->_nextSceneId = 353;

		break;

	default:
		break;
	}
}

void Scene352::enter() {
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

	_leaveRoomFl = false;

	if (_game._objects.isInRoom(OBJ_TAPE_PLAYER)) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 5);
		int idx = _scene->_dynamicHotspots.add(NOUN_TAPE_PLAYER, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_tapePlayerHotspotIdx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(84, 145), FACING_WEST);
	}

	_vaultOpenFl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_mustPutArmDownFl = false;
		if (!_game._visitedScenes._sceneRevisited)
			_globals[kHaveYourStuff] = false;
	}

	if (_game._objects.isInRoom(OBJ_GUARDS_ARM)) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 0);
		int idx = _scene->_dynamicHotspots.add(NOUN_GUARDS_ARM2, VERB_WALKTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(230, 117), FACING_NORTHWEST);
	} else
		_mustPutArmDownFl = true;

	if (_scene->_priorSceneId == 353)
		_game._player._playerPos = Common::Point(171, 155);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(116, 107);

	sceneEntrySound();

	_game.loadQuoteSet(0xFF, 0x100, 0x101, 0x102, 0x103, 0);
}

void Scene352::preActions() {
	_leaveRoomFl = false;

	if (_action.isAction(VERB_OPEN, NOUN_VAULT))
		_game._player.walk(Common::Point(266, 111), FACING_NORTHEAST);

	if (_vaultOpenFl && !_action.isObject(NOUN_VAULT) && !_action.isObject(NOUN_LAMP) && !_action.isObject(NOUN_OTHER_STUFF) && !_action.isObject(NOUN_YOUR_STUFF)) {
		if (_globals[kHaveYourStuff]) {
			_commonSpriteIndex = _globals._spriteIndexes[13];
			_commonSequenceIdx = _globals._sequenceIndexes[13];
		} else {
			_commonSpriteIndex = _globals._spriteIndexes[1];
			_commonSequenceIdx = _globals._sequenceIndexes[1];
		}

		switch (_game._trigger) {
		case 0:
			if (_game._player._needToWalk) {
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_commonSequenceIdx);
				_vm->_sound->command(20);
				_commonSequenceIdx = _scene->_sequences.addReverseSpriteCycle(_commonSpriteIndex, false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.setDepth(_commonSequenceIdx, 15);
			}
			break;

		case 1:
			if (!_globals[kHaveYourStuff])
				_scene->_dynamicHotspots.remove(_hotspot2Idx);

			_scene->_dynamicHotspots.remove(_hotspot1Idx);
			_scene->_dynamicHotspots.remove(_lampHostpotIdx);
			_vaultOpenFl = false;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	}

	if (_action.isAction(VERB_PUT, NOUN_GUARDS_ARM2, NOUN_SCANNER)) {
		if (_globals[kSexOfRex] == REX_MALE)
			_game._player.walk(Common::Point(269, 111), FACING_NORTHEAST);
		else
			_game._player.walk(Common::Point(271, 111), FACING_NORTHEAST);
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY) || _action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH) || _action.isAction(VERB_PUT, NOUN_GUARDS_ARM2, NOUN_FLOOR)) {
		if (_game._objects.isInInventory(OBJ_GUARDS_ARM))
			_game._player.walk(Common::Point(230, 117), FACING_NORTHWEST);
	}
}

void Scene352::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(35225);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_VAULT)) {
		if (!_vaultOpenFl) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE)
					_commonSpriteIndex = _globals._spriteIndexes[9];
				else
					_commonSpriteIndex = _globals._spriteIndexes[8];

				_commonSequenceIdx = _scene->_sequences.addSpriteCycle(_commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.updateTimeout(_commonSequenceIdx, -1);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
			{
				_vm->_sound->command(21);
				_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 7, 2, 20, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], FACING_NORTH);
				int oldIdx = _commonSequenceIdx;
				_commonSequenceIdx = _scene->_sequences.startCycle(_commonSpriteIndex, false, -2);
				_scene->_sequences.updateTimeout(_commonSequenceIdx, oldIdx);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

			case 2:
				_vm->_sound->command(22);
				_scene->_sequences.remove(_commonSequenceIdx);
				_commonSequenceIdx = _scene->_sequences.startPingPongCycle(_commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_commonSequenceIdx, 1, 3);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_scene->_sequences.updateTimeout(-1, _commonSequenceIdx);
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
		_mustPutArmDownFl = true;
	}

	bool exit_corridor = false;
	bool exit_doorway = false;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH)) {
		exit_corridor = true;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY)) {
		exit_doorway = true;
	}

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH) || _action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY) || _action.isAction(VERB_PUT, NOUN_GUARDS_ARM2, NOUN_FLOOR)) {
		if (_mustPutArmDownFl)
			putArmDown(exit_corridor, exit_doorway);
		else if (exit_corridor)
			_scene->_nextSceneId = 353;
		else
			_scene->_nextSceneId = 351;

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_GUARDS_ARM2)) {
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

	if (_action.isAction(VERB_PUT, NOUN_GUARDS_ARM2, NOUN_SCANNER)) {
		if (!_vaultOpenFl) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE)
					_commonSpriteIndex = _globals._spriteIndexes[11];
				else
					_commonSpriteIndex = _globals._spriteIndexes[10];

				_commonSequenceIdx = _scene->_sequences.addSpriteCycle(_commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.updateTimeout(_commonSequenceIdx, -1);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
			{
				_vm->_sound->command(21);
				_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 7, 2, 20, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);
				int oldIdx = _commonSequenceIdx;
				_commonSequenceIdx = _scene->_sequences.startCycle(_commonSpriteIndex, false, -2);
				_scene->_sequences.updateTimeout(_commonSequenceIdx, oldIdx);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

			case 2:
				_vm->_sound->command(23);
				_scene->_sequences.remove(_commonSequenceIdx);
				_commonSequenceIdx = _scene->_sequences.addReverseSpriteCycle(_commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_commonSequenceIdx, 1, 4);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;


			case 3:
				_scene->_sequences.updateTimeout(-1, _commonSequenceIdx);
				_game._player._visible = true;
				if (_globals[kHaveYourStuff])
					_commonSpriteIndex = _globals._spriteIndexes[13];
				else
					_commonSpriteIndex = _globals._spriteIndexes[1];

				_vm->_sound->command(20);
				_commonSequenceIdx = _scene->_sequences.addSpriteCycle(_commonSpriteIndex, false, 6, 1, 0, 0);
				_scene->_sequences.setDepth(_commonSequenceIdx, 15);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 4);
				break;

			case 4:
				_commonSequenceIdx = _scene->_sequences.addSpriteCycle(_commonSpriteIndex, false, 6, 0, 0, 0);
				_scene->_sequences.setAnimRange(_commonSequenceIdx, -2, -2);
				_scene->_sequences.setDepth(_commonSequenceIdx, 15);
				_scene->_sequences.addTimer(60, 5);
				break;

			case 5:
			{
				_vaultOpenFl = true;
				int idx;
				if (!_globals[kHaveYourStuff]) {
					idx = _scene->_dynamicHotspots.add(NOUN_YOUR_STUFF, VERB_WALKTO, -1, Common::Rect(282, 87, 282 + 13, 87 + 7));
					_hotspot2Idx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(280, 111), FACING_NORTHEAST);
					_globals._sequenceIndexes[1] = _commonSequenceIdx;
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x102));
				} else {
					_globals._sequenceIndexes[13] = _commonSequenceIdx;
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x103));
				}

				idx = _scene->_dynamicHotspots.add(NOUN_OTHER_STUFF, VERB_WALKTO, -1, Common::Rect(282, 48, 282 + 36, 48 + 27));
				_hotspot1Idx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(287, 115), FACING_NORTHEAST);
				idx = _scene->_dynamicHotspots.add(NOUN_LAMP, VERB_WALKTO, -1, Common::Rect(296, 76, 296 + 11, 76 + 17));
				_lampHostpotIdx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(287, 115), FACING_NORTHEAST);
				_game._player._stepEnabled = true;
			}
			break;

			default:
				break;
			}
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_YOUR_STUFF)) {
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
			_scene->_dynamicHotspots.remove(_hotspot2Idx);
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
	} else if (_action.isAction(VERB_TAKE, NOUN_TAPE_PLAYER) && !_game._objects.isInInventory(OBJ_TAPE_PLAYER)) {
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
			_scene->_dynamicHotspots.remove(_tapePlayerHotspotIdx);
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
	} else if (_action.isAction(VERB_LOOK, NOUN_SCANNER))
		_vm->_dialogs->show(35210);
	else if (_action.isAction(VERB_LOOK, NOUN_MONITOR)) {
		if (_game._storyMode == STORYMODE_NAUGHTY)
			_vm->_dialogs->show(35211);
		else
			_vm->_dialogs->show(35212);
	} else if (_action.isAction(VERB_LOOK, NOUN_DISPLAY))
		_vm->_dialogs->show(35213);
	else if (_action.isAction(VERB_LOOK, NOUN_STATUE))
		_vm->_dialogs->show(35214);
	else if (_action.isAction(VERB_LOOK, NOUN_TAPE_PLAYER) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(35215);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(35216);
	else if (_action.isAction(VERB_LOOK, NOUN_GUARDS_ARM2) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(35217);
	else if (_action.isAction(VERB_LOOK, NOUN_IRONING_BOARD))
		_vm->_dialogs->show(35218);
	else if (_action.isAction(VERB_LOOK, NOUN_CLOCK))
		_vm->_dialogs->show(35219);
	else if (_action.isAction(VERB_LOOK, NOUN_GAUGE))
		_vm->_dialogs->show(35220);
	else if (_action.isAction(VERB_LOOK, NOUN_VAULT)) {
		if (!_vaultOpenFl)
			_vm->_dialogs->show(35221);
	} else if (_action.isAction(VERB_LOOK, NOUN_YOUR_STUFF))
		_vm->_dialogs->show(35222);
	else if (_action.isAction(VERB_LOOK, NOUN_OTHER_STUFF))
		_vm->_dialogs->show(35223);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(35224);
	else if (_action.isAction(VERB_TAKE, NOUN_OTHER_STUFF))
		_vm->_dialogs->show(35226);
	else if (_action.isAction(VERB_LOOK, NOUN_DESK))
		_vm->_dialogs->show(35229);
	else if (_action.isAction(VERB_LOOK, NOUN_GUARD))
		_vm->_dialogs->show(35230);
	else if (_action.isAction(VERB_LOOK, NOUN_DOORWAY))
		_vm->_dialogs->show(35231);
	else if (_action.isAction(VERB_LOOK, NOUN_TABLE))
		_vm->_dialogs->show(35232);
	else if (_action.isAction(VERB_LOOK, NOUN_PROJECTOR))
		_vm->_dialogs->show(35233);
	else if (_action.isAction(VERB_LOOK, NOUN_SUPPORT))
		_vm->_dialogs->show(35234);
	else if (_action.isAction(VERB_LOOK, NOUN_SECURITY_MONITOR))
		_vm->_dialogs->show(35235);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
