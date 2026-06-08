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
#include "mads/madsv2/nebular/rooms/room201.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

Scene202::Scene202(RexNebularEngine *vm) : Scene2xx(vm) {
	_activeMsgFl = false;
	_ladderTopFl = false;
	_waitingMeteoFl = false;
	_ladderHotspotId = -1;
	_meteoClock1 = 0;
	_meteoClock2 = 0;
	_toStationFl = false;
	_toTeleportFl = false;
	_lastRoute = 0;
	_stationCounter = 0;
	_meteoFrame = 0;
	_startTime = 0;
	_meteorologistSpecial = false;
}

void Scene202::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_activeMsgFl);
	s.syncAsByte(_ladderTopFl);
	s.syncAsByte(_waitingMeteoFl);
	s.syncAsByte(_toStationFl);
	s.syncAsByte(_toTeleportFl);

	s.syncAsSint32LE(_ladderHotspotId);
	s.syncAsSint32LE(_lastRoute);
	s.syncAsSint32LE(_stationCounter);
	s.syncAsSint32LE(_meteoFrame);

	s.syncAsUint32LE(_meteoClock1);
	s.syncAsUint32LE(_meteoClock2);
	s.syncAsUint32LE(_startTime);

	s.syncAsByte(_meteorologistSpecial);
}

void Scene202::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_LADDER);
	_scene->addActiveVocab(VERB_CLIMB_DOWN);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_BONE);
	_scene->addActiveVocab(NOUN_SKULL);
	_scene->addActiveVocab(NOUN_BROKEN_LADDER);
}

void Scene202::enter() {
	_game._player._beenVisible = true;
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('l', -1));
	if (_globals[kSexOfRex] != SEX_MALE) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*ROXBD_2");
	} else {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*RXMBD_2");
	}
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 2));

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 0);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(149, 113));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
	int idx = _scene->_dynamicHotspots.add(NOUN_SKULL, VERB_WALKTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(153, 97), FACING_SOUTH);

	if (!(_globals[kBone202Status] & 1)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(130, 108));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		idx = _scene->_dynamicHotspots.add(NOUN_BONE, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(132, 97), FACING_SOUTH);
	}

	if (!(_globals[kBone202Status] & 2)) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[6], Common::Point(166, 110));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 10);
		idx = _scene->_dynamicHotspots.add(NOUN_BONE, VERB_WALKTO, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(165, 99), FACING_SOUTH);
	}

	if (_globals[kBone202Status])
		_scene->changeVariant(_globals[kBone202Status]);

	if (_scene->_priorSceneId == 201) {
		_game._player._playerPos = Common::Point(190, 91);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(178, 152);
		_game._player._facing = FACING_NORTH;
	}

	if (_globals[kLadderBroken]) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 6);
		_scene->_hotspots.activate(NOUN_LADDER, false);
		idx = _scene->_dynamicHotspots.add(NOUN_BROKEN_LADDER, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(246, 124), FACING_NORTH);
	}

	_game.loadQuoteSet(0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x62, 0x63, 0x64, 0x65, 0x66, 0x61, 0);
	_activeMsgFl = false;

	if (_scene->_priorSceneId == RETURNING_FROM_DIALOG) {
		if (_waitingMeteoFl) {
			_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
			_game._player._visible = false;
		}
	} else {
		_waitingMeteoFl = false;
		_ladderTopFl = false;
	}

	_meteoClock1 = _meteoClock2 = _scene->_frameStartTime;

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BINOCULARS);

	if (_globals[kMeteorologistWatch] != METEOROLOGIST_NORMAL) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_ladderTopFl = (_globals[kMeteorologistWatch] == METEOROLOGIST_TOWER);

		if (_ladderTopFl) {
			_globals._sequenceIndexes[10] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, 8);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);

			_scene->_sequences.setPosition(_globals._sequenceIndexes[10], Common::Point(247, 82));
			_game._player._playerPos = Common::Point(246, 124);
			_game._player._facing = FACING_NORTH;
			_globals[kTeleporterUnderstood] = true;
		} else {
			_globals._sequenceIndexes[10] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);

			_scene->_sequences.setPosition(_globals._sequenceIndexes[10], Common::Point(172, 123));
			_game._player._playerPos = Common::Point(171, 122);
			_game._player._facing = FACING_NORTH;
		}

		_scene->loadAnimation(formAnimName('M', -1), 71);
		_scene->_animation[0]->setCurrentFrame(200);
	} else {
		if (_ladderTopFl) {
			_game._player._visible = false;
			_scene->_sequences.startCycle(_globals._sequenceIndexes[9], true, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[9], Common::Point(247, 82));
			_game._player._playerPos = Common::Point(246, 124);
			_game._player._facing = FACING_NORTH;
		}
	}

	_meteorologistSpecial = false;
}

void Scene202::setRandomKernelMessage() {
	int vocabId = _vm->getRandomNumber(92, 96);
	_scene->_kernelMessages.reset();
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
	_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 70, 120, _game.getQuote(vocabId));
	_activeMsgFl = true;
}

void Scene202::step() {
	if (!_activeMsgFl && (_game._player._playerPos == Common::Point(77, 105)) && (_game._player._facing == FACING_NORTH) && (_vm->getRandomNumber(999) == 0)) {
		_scene->_kernelMessages.reset();
		_activeMsgFl = false;
		if (_vm->getRandomNumber(4) == 0)
			setRandomKernelMessage();
	}

	if (_game._trigger == 70)
		_activeMsgFl = false;

	if (_game._trigger == 71) {
		_vm->_sound->command(3);
		_vm->_sound->command(9);

		_meteoClock1 = _scene->_frameStartTime + 15 * 60;

		if (_globals[kMeteorologistWatch] != METEOROLOGIST_NORMAL) {
			Common::Point msgPos;
			int msgFlag;
			if (!_ladderTopFl) {
				msgPos = Common::Point(0, 0);
				msgFlag = 2;
			} else {
				msgPos = Common::Point(248, 15);
				msgFlag = 0;
			}
			int msgIndex = _scene->_kernelMessages.add(msgPos, 0x1110, msgFlag | 32, 0, 120, _game.getQuote(102));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);

			if (_globals[kMeteorologistWatch] == METEOROLOGIST_GROUND) {
				_action._activeAction._verbId = VERB_LOOK;
				_action._activeAction._objectNameId = NOUN_BINOCULARS;
				_action._activeAction._indirectObjectId = NOUN_STRANGE_DEVICE;
				_game._triggerSetupMode = SEQUENCE_TRIGGER_PARSER;
				_scene->_sequences.addTimer(2 * 60, 2);
				_meteorologistSpecial = true;
			} else if (_globals[kMeteorologistWatch] == METEOROLOGIST_TOWER) {
				_scene->_sequences.addTimer(2 * 60, 90);
			}
		}

		_globals[kMeteorologistWatch] = METEOROLOGIST_NORMAL;
	}

	switch (_game._trigger) {
	case 90:
		_vm->_sound->command(41);
		_scene->_sequences.remove(_globals._sequenceIndexes[10]);
		_globals._sequenceIndexes[9] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], true, 6, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[9], Common::Point(247, 82));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
		break;
	case 91:
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[9], Common::Point(247, 82));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 1);
		_scene->_sequences.addTimer(60, 92);
		break;
	case 92:
	{
		_scene->_sequences.remove(_globals._sequenceIndexes[9]);
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 93);
		_scene->_kernelMessages.reset();
		int msgIndex = _scene->_kernelMessages.add(Common::Point(0, -65), 0x1110, 32, 0, 60, _game.getQuote(98));
		_scene->_kernelMessages.setSeqIndex(msgIndex, _globals._sequenceIndexes[11]);
	}
	break;
	case 93:
	{
		_globals[kLadderBroken] = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 0, 0, 0);
		_scene->_hotspots.activate(NOUN_LADDER, false);
		int idx = _scene->_dynamicHotspots.add(NOUN_BROKEN_LADDER, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(246, 124), FACING_NORTH);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[11], _globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[11]);
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_ladderTopFl = false;
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(99));
	}
	break;
	default:
		break;
	}

	if (!_scene->_animation[0] && (_globals[kMeteorologistStatus] != METEOROLOGIST_GONE) && (_meteoClock2 <= _scene->_frameStartTime) && (_meteoClock1 <= _scene->_frameStartTime)) {
		int randVal = _vm->getRandomNumber(1, 500);
		int threshold = 1;
		if (_ladderTopFl)
			threshold += 25;
		if (!_globals[kMeteorologistEverSeen])
			threshold += 25;
		if (threshold >= randVal) {
			_vm->_sound->command(17);
			_scene->loadAnimation(formAnimName('M', -1), 71);
			_toStationFl = true;
			_toTeleportFl = false;
			_globals[kMeteorologistEverSeen] = true;
			_lastRoute = 0;
			_stationCounter = 0;
			_meteoClock2 = _scene->_frameStartTime + 2;
		}
	}

	if (!_scene->_animation[0])
		return;

	if (_waitingMeteoFl) {
		if (_scene->_animation[0]->getCurrentFrame() >= 200) {
			if ((_globals[kMeteorologistWatch] == METEOROLOGIST_TOWER) || _globals[kLadderBroken]) {
				_scene->_nextSceneId = 213;
			} else {
				_vm->_dialogs->show(20201);
				_scene->_reloadSceneFlag = true;
			}
		}

		if ((_scene->_animation[0]->getCurrentFrame() == 160) && (_meteoFrame != _scene->_animation[0]->getCurrentFrame())) {
			Common::Point msgPos;
			int msgFlag;
			if (!_ladderTopFl) {
				msgPos = Common::Point(0, 0);
				msgFlag = 2;
			} else {
				msgPos = Common::Point(248, 15);
				msgFlag = 0;
			}
			int msgIndex = _scene->_kernelMessages.add(msgPos, 0x1110, msgFlag | 32, 0, 120, _game.getQuote(101));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		}
	}

	if (_meteoClock2 + 120 * 60 <= _scene->_frameStartTime) {
		_toTeleportFl = true;
	}

	if (_scene->_animation[0]->getCurrentFrame() == _meteoFrame) {
		return;
	}

	_meteoFrame = _scene->_animation[0]->getCurrentFrame();
	int randVal = _vm->getRandomNumber(1, 1000);
	int frameStep = -1;

	switch (_scene->_animation[0]->getCurrentFrame()) {
	case 42:
	case 77:
	case 96:
		_stationCounter = 0;
		frameStep = subStep1(randVal);
		break;
	case 51:
	case 74:
		_toStationFl = false;
		frameStep = subStep2(randVal);
		break;
	case 27:
	case 119:
	case 159:
		frameStep = subStep3(randVal);
		break;
	case 176:
		frameStep = subStep4(randVal);
		break;
	case 59:
		_lastRoute = 3;
		++_stationCounter;
		if (randVal <= 800)
			frameStep = 55;
		break;
	case 89:
		_lastRoute = 1;
		if (randVal <= 700)
			frameStep = 83;
		break;
	case 137:
		_lastRoute = 2;
		if (randVal <= 700)
			frameStep = 126;
		break;
	default:
		break;
	}

	if (frameStep >= 0 && frameStep != _scene->_animation[0]->getCurrentFrame() + 1) {
		_scene->_animation[0]->setCurrentFrame(frameStep);
		_meteoFrame = frameStep;
	}
}

int Scene202::subStep1(int randVal) {
	if ((randVal <= 100) || _toStationFl)
		return 42;

	if ((randVal <= 200) || _toTeleportFl)
		return 96;

	if ((randVal <= 300) && (_lastRoute != 1))
		return 77;

	return 76;
}

int Scene202::subStep2(int randVal) {
	if ((randVal <= 150) && (_stationCounter < 5))
		return 51;

	if ((randVal <= 300) || _toTeleportFl)
		return 74;

	if (randVal <= 400)
		return 64;

	return 44;
}

int Scene202::subStep3(int randVal) {
	if ((randVal <= 100) || _toStationFl)
		return 27;

	if ((randVal <= 200) || _toTeleportFl)
		return 159;

	if ((randVal <= 300) && (_lastRoute != 2))
		return 119;

	return 110;
}

int Scene202::subStep4(int randVal) {
	if ((randVal <= 100) || _toTeleportFl)
		return 176;

	if (randVal <= 200)
		return 19;

	return 166;
}

void Scene202::preActions() {
	Player &player = _vm->_game->_player;

	if (player._needToWalk)
		_scene->_kernelMessages.reset();

	if (_ladderTopFl && (_action.isAction(VERB_CLIMB_DOWN, NOUN_LADDER) || player._needToWalk)) {
		if (_game._trigger == 0) {
			_vm->_sound->command(29);
			player._readyToWalk = false;
			player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[9]);
			_globals._sequenceIndexes[8] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
		} else if (_game._trigger == 1) {
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[8]);
			_scene->_dynamicHotspots.remove(_ladderHotspotId);
			player._visible = true;
			player._readyToWalk = true;
			player._stepEnabled = true;
			_ladderTopFl = false;
		}
	}

	if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS) && (_action._activeAction._indirectObjectId > 0)) {
		if (!player._readyToWalk || _ladderTopFl)
			player._needToWalk = false;
		else
			player._needToWalk = true;

		if (!_ladderTopFl)
			player.walk(Common::Point(171, 122), FACING_NORTH);
	}
}

void Scene202::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(20219);
		return;
	}

	if (_action.isAction(VERB_CLIMB_DOWN, NOUN_LADDER)) {
		_action._inProgress = false;
		return;
	} else if (_action.isAction(VERB_WALK_TOWARDS, NOUN_FIELD_TO_SOUTH)) {
		_scene->_nextSceneId = 203;
	} else if (_action.isAction(VERB_WALK_TOWARDS, NOUN_FIELD_TO_NORTH)) {
		if (_globals[kMeteorologistStatus] != METEOROLOGIST_GONE) {
			if (_scene->_animation[0])
				_globals[kMeteorologistStatus] = METEOROLOGIST_PRESENT;
			else
				_globals[kMeteorologistStatus] = METEOROLOGIST_ABSENT;
		}
		_scene->_nextSceneId = 201;
	} else if (_action.isAction(VERB_TAKE, NOUN_BONE) && (_action._savedFields._mainObjectSource == 4)) {
		switch (_game._trigger) {
		case 0:
			if (_game._objects.isInInventory(OBJ_BONES)) {
				_vm->_dialogs->show(20221);
			} else {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[7] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[7], false, 3, 2, 0, 0);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[7]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_SPRITE, 6, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;
		case 1:
			if (_game._player._playerPos == Common::Point(132, 97)) {
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_globals[kBone202Status] |= BONE_202_LEFT_GONE;
			} else {
				_scene->_sequences.remove(_globals._sequenceIndexes[6]);
				_globals[kBone202Status] |= BONE_202_RIGHT_GONE;
			}
			break;
		case 2:
			if (_game._objects.isInInventory(OBJ_BONE)) {
				_game._objects.removeFromInventory(OBJ_BONE, NOWHERE);
				_game._objects.addToInventory(OBJ_BONES);
				_vm->_dialogs->showItem(OBJ_BONES, 20218);
			} else {
				_game._objects.addToInventory(OBJ_BONE);
				_vm->_dialogs->showItem(OBJ_BONE, 20218);
			}
			_scene->changeVariant(_globals[kBone202Status]);
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			break;
		default:
			break;
		}

		_action._inProgress = false;
	} else if (_action.isAction(VERB_CLIMB_UP, NOUN_LADDER) && !_globals[kLadderBroken]) {
		switch (_game._trigger) {
		case 0:
			_vm->_sound->command(29);
			_meteoClock1 = _scene->_frameStartTime;
			_game._player._visible = false;
			_game._player._stepEnabled = false;

			_ladderHotspotId = _scene->_dynamicHotspots.add(NOUN_LADDER, 78, -1, Common::Rect(241, 68, 241 + 12, 68 + 54));
			_scene->_dynamicHotspots.setPosition(_ladderHotspotId, Common::Point(246, 124), FACING_NORTH);
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;
		case 1:
		{
			_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[9], Common::Point(247, 82));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[8], _globals._sequenceIndexes[9]);
			_ladderTopFl = true;
			_game._player._stepEnabled = true;
			int msgIndex = _scene->_kernelMessages.add(Common::Point(248, 15), 0x1110, 32, 0, 60, _game.getQuote(97));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		}
		break;
		default:
			_action._inProgress = false;
			return;
		}
	} else if ((_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_FIELD_TO_NORTH) || (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_STRANGE_DEVICE))) && (_globals[kSexOfRex] == SEX_MALE)) {
		if (!_ladderTopFl) {
			switch (_game._trigger) {
			case 0:
				_toTeleportFl = true;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 1, 6);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[10], Common::Point(172, 123));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[10]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;
			case 1:
				_globals._sequenceIndexes[10] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 6);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[10], Common::Point(172, 123));
				if (_scene->_animation[0]) {
					_waitingMeteoFl = true;
					_globals[kMeteorologistWatch] = METEOROLOGIST_GROUND;
				} else {
					_scene->_sequences.addTimer(120, 2);
				}
				break;
			case 2:
				if (!_scene->_animation[0] && !_meteorologistSpecial) {
					_vm->_dialogs->show(20222);
				}
				_scene->_sequences.remove(_globals._sequenceIndexes[10]);
				_globals._sequenceIndexes[10] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], false, 6, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 1, 6);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[10], Common::Point(172, 123));
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;
			case 3:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[10]);
				_game._player._stepEnabled = true;
				_game._player._visible = true;
				break;
			default:
				_action._inProgress = false;
				return;
			}
		} else {
			switch (_game._trigger) {
			case 0:
				_toTeleportFl = true;
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[9]);
				_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], true, 6, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[9], Common::Point(247, 82));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;
			case 1:
				_globals._sequenceIndexes[10] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, -2);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[10], Common::Point(247, 82));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
				if (_scene->_animation[0]) {
					if (_scene->_animation[0]->getCurrentFrame() > 200) {
						_scene->_sequences.addTimer(120, 2);
					} else {
						_waitingMeteoFl = true;
						_globals[kMeteorologistWatch] = METEOROLOGIST_GONE;
						if ((_scene->_animation[0]->getCurrentFrame() >= 44) && (_scene->_animation[0]->getCurrentFrame() <= 75)) {
							_scene->_kernelMessages.reset();
							int msgIndex = _scene->_kernelMessages.add(Common::Point(248, 15), 0x1110, 32, 0, 60, _game.getQuote(100));
							_scene->_kernelMessages.setQuoted(msgIndex, 4, false);
						} else {
							_action._inProgress = false;
							return;
						}
					}
				} else {
					_scene->_sequences.addTimer(120, 2);
				}
				break;
			case 2:
				if (!_scene->_animation[0])
					_vm->_dialogs->show(20222);
				_meteorologistSpecial = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[10]);
				_globals._sequenceIndexes[9] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], false, 6, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[9], Common::Point(247, 82));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;
			case 3:
				_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, 1);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[9], Common::Point(247, 82));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 1);
				_game._player._stepEnabled = true;
				break;
			default:
				_action._inProgress = false;
				return;
			}
		}
	} else if (_action.isAction(VERB_WALK_INSIDE, NOUN_HUT)) {
		setRandomKernelMessage();
	} else if (_action.isAction(VERB_LOOK, NOUN_ROCKS)) {
		_vm->_dialogs->show(20202);
	} else if (_action.isAction(VERB_LOOK, NOUN_FIRE_PIT)) {
		_vm->_dialogs->show(20203);
	} else if (_action.isAction(VERB_LOOK, NOUN_GRASS)) {
		_vm->_dialogs->show(20204);
	} else if (_action.isAction(VERB_LOOK, NOUN_FIELD_TO_NORTH)) {
		if ((_globals[kMeteorologistStatus] == METEOROLOGIST_ABSENT) || (_globals[kMeteorologistStatus] == METEOROLOGIST_GONE))
			_vm->_dialogs->show(20205);
		else if (_globals[kMeteorologistStatus] == METEOROLOGIST_PRESENT)
			_vm->_dialogs->show(20220);
	} else if (_action.isAction(VERB_LOOK, NOUN_WATCH_TOWER)) {
		_vm->_dialogs->show(20206);
	} else if (_action.isAction(VERB_LOOK, NOUN_TALL_GRASS)) {
		_vm->_dialogs->show(20207);
	} else if (_action.isAction(VERB_LOOK, NOUN_TREES)) {
		_vm->_dialogs->show(20208);
	} else if (_action.isAction(VERB_LOOK, NOUN_TREE)) {
		_vm->_dialogs->show(20209);
	} else if (_action.isAction(VERB_LOOK, NOUN_SKY)) {
		_vm->_dialogs->show(20210);
	} else if (_action.isAction(VERB_LOOK, NOUN_HUT)) {
		if ((_game._player._playerPos == Common::Point(77, 105)) && (_game._player._facing == FACING_NORTH))
			_vm->_dialogs->show(20212);
		else
			_vm->_dialogs->show(20211);
	} else if (_action.isAction(VERB_LOOK, NOUN_STRANGE_DEVICE)) {
		_vm->_dialogs->show(20213);
	} else if (_action.isAction(VERB_LOOK, NOUN_OCEAN_IN_DISTANCE)) {
		_vm->_dialogs->show(20214);
	} else if (_action.isAction(VERB_LOOK, NOUN_SKULL)) {
		_vm->_dialogs->show(20215);
	} else if (_action.isAction(VERB_TAKE, NOUN_SKULL)) {
		_vm->_dialogs->show(20216);
	} else if (_action.isAction(VERB_LOOK, NOUN_BONES) && _action._commandSource == 4) {
		_vm->_dialogs->show(20217);
	} else {
		return;
	}

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
