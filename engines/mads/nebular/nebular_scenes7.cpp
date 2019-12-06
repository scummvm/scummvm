/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes7.h"

namespace MADS {

namespace Nebular {

void Scene7xx::setAAName() {
	_game._aaName = Resources::formatAAName(5);
}

void Scene7xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;

	if ((_scene->_nextSceneId == 703) || (_scene->_nextSceneId == 704) || (_scene->_nextSceneId == 705)
	 || (_scene->_nextSceneId == 707) || (_scene->_nextSceneId == 710) || (_scene->_nextSceneId == 711))
		_game._player._spritesPrefix = "";
	else if (_globals[kSexOfRex] == REX_MALE)
		_game._player._spritesPrefix = "RXM";
	else
		_game._player._spritesPrefix = "ROX";

	_game._player._scalingVelocity = true;

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene7xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 701:
	case 702:
	case 704:
	case 705:
	case 751:
		_vm->_sound->command(38);
		break;
	case 703:
		if (_globals[kMonsterAlive] == 0)
			_vm->_sound->command(24);
		else
			_vm->_sound->command(27);
		break;
	case 706:
	case 707:
	case 710:
	case 711:
		_vm->_sound->command(25);
		break;
	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

Scene701::Scene701(MADSEngine *vm) : Scene7xx(vm) {
	_fishingLineId = -1;
}

void Scene701::synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);

	s.syncAsSint16LE(_fishingLineId);
}

void Scene701::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_BOAT);
	_scene->addActiveVocab(VERB_CLIMB_INTO);
	_scene->addActiveVocab(NOUN_FISHING_LINE);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene701::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 5));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RM202A1");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', 8));

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_BINOCULARS);
		_game._objects.addToInventory(OBJ_TWINKIFRUIT);
		_game._objects.addToInventory(OBJ_BOMB);
		_game._objects.addToInventory(OBJ_CHICKEN);
		_game._objects.addToInventory(OBJ_BONES);

		_globals[kCityFlooded] = true;
		_globals[kLineStatus] = LINE_TIED;
		_globals[kBoatRaised] = false;
	}

	if (_globals[kBoatStatus] == BOAT_UNFLOODED) {
		if (_globals[kBoatRaised])
			_globals[kBoatStatus] = BOAT_GONE;
		else if (_globals[kLineStatus] == LINE_TIED)
			_globals[kBoatStatus] = BOAT_TIED_FLOATING;
		else if (_game._difficulty == DIFFICULTY_HARD)
			_globals[kBoatStatus] = BOAT_ADRIFT;
		else
			_globals[kBoatStatus] = BOAT_TIED;
	}

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);

	int boatStatus = (_scene->_priorSceneId == 703) ? BOAT_GONE : _globals[kBoatStatus];

	switch (boatStatus) {
	case BOAT_TIED_FLOATING:
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 20, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 10);
		break;
	case BOAT_ADRIFT:
		_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], false, 20, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 10);
		break;
	case BOAT_TIED: {
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		int idx = _scene->_dynamicHotspots.add(837, 759, _globals._sequenceIndexes[2], Common::Rect());
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(231, 127), FACING_NORTH);
		break;
	}
	case BOAT_GONE:
		_scene->_hotspots.activate(NOUN_BOAT, false);
		break;
	default:
		break;
	}

	if (_globals[kLineStatus] == LINE_DROPPED || _globals[kLineStatus] == LINE_TIED) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_fishingLineId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(234, 129), FACING_NORTHEAST);
	}

	if (_scene->_priorSceneId == 702) {
		_game._player._playerPos = Common::Point(309, 138);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 710) {
		_game._player._playerPos = Common::Point(154, 129);
		_game._player._facing = FACING_NORTH;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(155, 129));
		_scene->_sequences.addTimer(15, 60);
	} else if (_scene->_priorSceneId == 703) {
		_game._player._playerPos = Common::Point(231, 127);
		_game._player._facing = FACING_SOUTH;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(formAnimName('B', 1), 80);
		_vm->_sound->command(28);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG && _scene->_priorSceneId != 620) {
		_game._player._playerPos = Common::Point(22, 131);
		_game._player._facing = FACING_EAST;
		_game._player._stepEnabled = false;
		_scene->_sequences.addTimer(60, 70);
	}

	_game.loadQuoteSet(0x310, 0x30F, 0);
	sceneEntrySound();
}

void Scene701::step() {
	switch(_game._trigger) {
	case 60:
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[5], false, 6, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(155, 129));
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[5]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		break;

	case 70:
		_vm->_sound->command(16);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71:
		_game._player.walk(Common::Point(61, 131), FACING_EAST);
		_scene->_sequences.addTimer(120, 72);
		break;

	case 72:
		_vm->_sound->command(17);
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
		break;

	case 73:
		_game._player._stepEnabled = true;
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_scene->_kernelMessages.reset();
		break;

	case 80: {
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		int idx = _scene->_dynamicHotspots.add(NOUN_BOAT, VERB_CLIMB_INTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(234, 129), FACING_NORTH);
		_globals[kBoatStatus] = BOAT_TIED;
		_game._player._stepEnabled = true;
		}
		break;

	default:
		break;
	}
}

void Scene701::preActions() {
	if (_action.isAction(VERB_WALKTO, NOUN_EAST_END_OF_PLATFORM))
		_game._player._walkOffScreenSceneId = 702;

	if (_action.isAction(VERB_LOOK, NOUN_BUILDING))
		_game._player.walk(Common::Point(154, 129), FACING_NORTHEAST);

	if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_BUILDING))
		_game._player.walk(Common::Point(154, 129), FACING_NORTH);
}

void Scene701::actions() {
	if (_action.isAction(VERB_WALK_ALONG, NOUN_PLATFORM)) {
	} else if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_BUILDING) && _game._objects[OBJ_VASE]._roomNumber == 706) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(155, 129));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int temp = _globals._sequenceIndexes[5];
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(155, 129));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], temp);
			_scene->_sequences.addTimer(15, 2);
			}
			break;

		case 2:
			_scene->_nextSceneId = 710;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_STEP_INTO, NOUN_ELEVATOR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_vm->_sound->command(16);
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x310, 34, 0, 120, _game.getQuote(0x30D));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_game._player.walk(Common::Point(22, 131), FACING_EAST);
			_scene->_sequences.addTimer(120, 3);
			break;

		case 3:
			_vm->_sound->command(17);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_globals[kResurrectRoom] = 701;
			_scene->_nextSceneId = 605;
			break;

		default:
			break;
		}
	} else if ((_action.isAction(VERB_PULL, NOUN_BOAT) || _action.isAction(VERB_TAKE, NOUN_BOAT) ||
			   _action.isAction(VERB_PULL, NOUN_FISHING_LINE) || _action.isAction(VERB_TAKE, NOUN_FISHING_LINE)) &&
			   !_game._objects.isInInventory(OBJ_FISHING_LINE)) {
		if (_globals[kBoatStatus] == BOAT_TIED_FLOATING) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
				_scene->_dynamicHotspots.remove(_fishingLineId);
				_scene->_hotspots.activate(NOUN_BOAT, false);
				_game._player._visible = false;
				_scene->loadAnimation(formAnimName('E', -1), 1);
				break;

			case 1: {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
				_scene->_sequences.setDepth (_globals._sequenceIndexes[2], 9);
				int idx = _scene->_dynamicHotspots.add(NOUN_BOAT, VERB_CLIMB_INTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(231, 127), FACING_NORTH);
				_scene->_sequences.addTimer(15, 2);
				}
				break;

			case 2:
				_globals[kBoatStatus] = BOAT_TIED;
				_globals[kLineStatus] = LINE_NOW_UNTIED;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		} else if (_globals[kBoatStatus] == BOAT_TIED) {
			_vm->_dialogs->show(70125);
		} else if (_globals[kLineStatus] == LINE_DROPPED) {
			_globals[kLineStatus] = LINE_NOW_UNTIED;
			_game._objects.addToInventory(OBJ_FISHING_LINE);
			_vm->_sound->command(15);
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_vm->_dialogs->showItem(OBJ_FISHING_LINE, 70126);
		} else {
			_vm->_dialogs->show(70127);
		}
	} else if (_action.isAction(VERB_CLIMB_INTO, NOUN_BOAT) && _globals[kBoatStatus] == BOAT_TIED) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_game._player._visible = false;
			_scene->loadAnimation(formAnimName('B', 0), 1);
			break;

		case 1:
			_scene->_nextSceneId = 703;
			break;

		default:
			break;
		}
	} else if (_action._lookFlag) {
		if (_globals[kBoatStatus] != BOAT_GONE) {
			if (_globals[kBoatStatus] == BOAT_TIED)
				_vm->_dialogs->show(70128);
			else
				_vm->_dialogs->show(70110);
		} else
			_vm->_dialogs->show(70111);
	} else if (_action.isAction(VERB_LOOK, NOUN_SUBMERGED_CITY))
		_vm->_dialogs->show(70112);
	else if (_action.isAction(VERB_LOOK, NOUN_ELEVATOR))
		_vm->_dialogs->show(70113);
	else if (_action.isAction(VERB_LOOK, NOUN_PLATFORM))
		_vm->_dialogs->show(70114);
	else if (_action.isAction(VERB_LOOK, NOUN_CEMENT_PYLON))
		_vm->_dialogs->show(70115);
	else if (_action.isAction(VERB_LOOK, NOUN_HOOK)) {
		if (_globals[kLineStatus] == LINE_NOT_DROPPED || _globals[kLineStatus] == LINE_NOW_UNTIED)
			_vm->_dialogs->show(70116);
		else
			_vm->_dialogs->show(70117);
	} else if (_action.isAction(VERB_LOOK, NOUN_ROCK))
		_vm->_dialogs->show(70118);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCK))
		_vm->_dialogs->show(70119);
	else if (_action.isAction(VERB_LOOK, NOUN_EAST_END_OF_PLATFORM))
		_vm->_dialogs->show(70120);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING))
		_vm->_dialogs->show(70121);
	else if (_action.isAction(VERB_LOOK, NOUN_BOAT)) {
		if (_globals[kBoatStatus] == BOAT_ADRIFT || _globals[kBoatStatus] == BOAT_TIED_FLOATING)
			_vm->_dialogs->show(70122);
		else
			_vm->_dialogs->show(70123);
	} else if (_action.isAction(VERB_CAST, NOUN_FISHING_ROD, NOUN_BOAT) && _game._objects.isInInventory(OBJ_FISHING_LINE))
		_vm->_dialogs->show(70124);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene702::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene702::enter() {
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RXMBD_8");

	if (_scene->_priorSceneId == 701) {
		_game._player._playerPos = Common::Point(13, 145);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG && _scene->_priorSceneId != 620) {
		_game._player._playerPos = Common::Point(289, 138);
		_game._player.walk(Common::Point(262, 148), FACING_WEST);
		_game._player._facing = FACING_WEST;
		_game._player._visible = true;
	}

	if (_game._globals[kTeleporterCommand]) {
		switch(_game._globals[kTeleporterCommand]) {
		case TELEPORTER_BEAM_OUT:
		case TELEPORTER_WRONG:
		case TELEPORTER_STEP_OUT:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}

		_game._globals[kTeleporterCommand] = TELEPORTER_NONE;
	}

	sceneEntrySound();
}

void Scene702::preActions() {
	if (_action.isAction(VERB_WALKTO, NOUN_WEST_END_OF_PLATFORM))
		_game._player._walkOffScreenSceneId = 701;
}

void Scene702::actions() {
	if (_action.isAction(VERB_WALK_ALONG, NOUN_PLATFORM))
		; // Only set the action as finished
	else if (_action.isAction(VERB_STEP_INTO, NOUN_TELEPORTER)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 711;
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && (_action._mainObjectSource == CAT_HOTSPOT) && (!_game._objects.isInInventory(OBJ_BONES) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			_vm->_sound->command(0xF);
			if (_game._objects.isInInventory(OBJ_BONE))
				_game._objects.setRoom(OBJ_BONE, 1);
			_game._objects.addToInventory(OBJ_BONES);
			_vm->_dialogs->show(OBJ_BONES, 70218);
			break;
		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[12]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(70210);
	else if (_action.isAction(VERB_LOOK, NOUN_PLATFORM))
		_vm->_dialogs->show(70211);
	else if (_action.isAction(VERB_LOOK, NOUN_CEMENT_BLOCK))
		_vm->_dialogs->show(70212);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCK))
		_vm->_dialogs->show(70213);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCK))
		_vm->_dialogs->show(70214);
	else if (_action.isAction(VERB_LOOK, NOUN_WEST_END_OF_PLATFORM))
		_vm->_dialogs->show(70215);
	else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER))
		_vm->_dialogs->show(70216);
	else if (_action.isAction(VERB_LOOK, NOUN_BONES) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(70217);
	else if (_action.isAction(VERB_TAKE, NOUN_BONES) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		if (_game._objects.isInInventory(OBJ_BONES))
			_vm->_dialogs->show(70219);
	} else if (_action.isAction(VERB_LOOK, NOUN_SUBMERGED_CITY))
		_vm->_dialogs->show(70220);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene703::Scene703(MADSEngine *vm) : Scene7xx(vm) {
	_monsterMode = -1;
	_boatFrame = -1;
	_curSequence = -1;
	_boatDir = -1;

	_useBomb = false;
	_startMonsterTimer = false;
	_rexDeathFl = false;
	_restartTrigger70Fl = false;

	_lastFrameTime = 0;
	_monsterTime = 0;
}

void Scene703::synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);

	s.syncAsSint16LE(_monsterMode);
	s.syncAsSint16LE(_boatFrame);
	s.syncAsSint16LE(_curSequence);
	s.syncAsSint16LE(_boatDir);

	s.syncAsByte(_useBomb);
	s.syncAsByte(_startMonsterTimer);
	s.syncAsByte(_rexDeathFl);
	s.syncAsByte(_restartTrigger70Fl);

	s.syncAsUint32LE(_lastFrameTime);
	s.syncAsUint32LE(_monsterTime);
}

void Scene703::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene703::handleBottleInterface() {
	switch (_globals[kBottleStatus]) {
	case 0:
		_dialog1.write(0x311, true);
		_dialog1.write(0x312, true);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 1:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, true);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 2:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, false);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 3:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, false);
		_dialog1.write(0x313, false);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	default:
		break;
	}
}

void Scene703::setBottleSequence() {
	_scene->_userInterface.setup(kInputBuildingSentences);
	_game._player._stepEnabled = false;
	if (_boatDir == 2)
		_curSequence = 6;
	else
		_curSequence = 7;
}

void Scene703::handleFillBottle(int quote) {
	switch (quote) {
	case 0x311:
		_globals[kBottleStatus] = 1;
		setBottleSequence();
		break;

	case 0x312:
		_globals[kBottleStatus] = 2;
		setBottleSequence();
		break;

	case 0x313:
		_globals[kBottleStatus] = 3;
		setBottleSequence();
		break;

	case 0x314:
		_globals[kBottleStatus] = 4;
		setBottleSequence();
		break;

	case 0x315:
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	default:
		break;
	}
}

void Scene703::enter() {
	_game._player._visible = false;

	if (!_game._visitedScenes._sceneRevisited) {
		if (_scene->_priorSceneId == 704)
			_globals[kMonsterAlive] = false;
		else
			_globals[kMonsterAlive] = true;
	}

	_startMonsterTimer = true;
	_rexDeathFl = true;
	_monsterTime = 0;
	_restartTrigger70Fl = true;
	_useBomb = false;
	_boatFrame = -1;

	if (!_globals[kMonsterAlive])
		_scene->_hotspots.activate(NOUN_SEA_MONSTER, false);

	if (_scene->_priorSceneId == 704) {
		_game._player._stepEnabled = false;
		_curSequence = 2;
		_boatDir = 2;
		_monsterMode = 0;
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(34);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._stepEnabled = false;
		_boatDir = 1;
		if (_globals[kMonsterAlive]) {
			_monsterMode = 1;
			_curSequence = 0;
			_scene->loadAnimation(formAnimName('B', -1));
		} else {
			_curSequence = 0;
			_monsterMode = 0;
			_scene->loadAnimation(formAnimName('A', -1));
		}
	} else if (_globals[kMonsterAlive]) {
		_curSequence = 0;
		_boatDir = 1;
		_monsterMode = 1;
		_scene->loadAnimation(formAnimName('B', -1));
		_scene->_animation[0]->setCurrentFrame(39);
	} else if (_boatDir == 1) {
		_curSequence = 0;
		_monsterMode = 0;
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(9);
	} else if (_boatDir == 2) {
		_curSequence = 0;
		_monsterMode = 0;
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(56);
	}

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_TWINKIFRUIT);
		_game._objects.addToInventory(OBJ_BOMB);
		_game._objects.addToInventory(OBJ_CHICKEN);
		_game._objects.addToInventory(OBJ_BONES);
	}

	_game.loadQuoteSet(0x311, 0x312, 0x313, 0x314, 0x315, 0);
	_dialog1.setup(0x98, 0x311, 0x312, 0x313, 0x314, 0x315, 0);
	sceneEntrySound();
	_vm->_sound->command(28);
}

void Scene703::step() {
	if (_startMonsterTimer) {
		long diff = _scene->_frameStartTime - _lastFrameTime;
		if ((diff >= 0) && (diff <= 12))
			_monsterTime += diff;
		else
			_monsterTime++;

		_lastFrameTime = _scene->_frameStartTime;
	}

	if ((_monsterTime >= 2400) && !_rexDeathFl && !_useBomb) {
		_startMonsterTimer = false;
		_rexDeathFl = true;
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 3;
		_scene->loadAnimation(formAnimName('D', -1));
		_rexDeathFl = false;
		_monsterTime = 0;
	}


	if (_game._trigger == 70)
		_scene->_reloadSceneFlag = true;

	if ((_monsterMode == 3) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _boatFrame) {
			_boatFrame = _scene->_animation[0]->getCurrentFrame();
			int nextBoatFrame = -1;

			if (_boatFrame == 62) {
				nextBoatFrame = 61;
				if (_restartTrigger70Fl) {
					_restartTrigger70Fl = false;
					_scene->_sequences.addTimer(15, 70);
				}
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextBoatFrame);
				_boatFrame = nextBoatFrame;
			}
		}
	}

	if (_game._trigger == 70)
		_scene->_reloadSceneFlag = true;

	if ((_monsterMode == 0) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _boatFrame) {
			_boatFrame = _scene->_animation[0]->getCurrentFrame();
			int nextBoatFrame = -1;

			switch (_boatFrame) {
			case 11:
				if (_curSequence == 7) {
					_curSequence = 0;
					nextBoatFrame = 100;
				} else if (_curSequence == 5)
					nextBoatFrame = 82;
				else if (_curSequence == 1)
					nextBoatFrame = 11;
				else {
					nextBoatFrame = 9;
					if (!_game._player._stepEnabled)
						_game._player._stepEnabled = true;
				}
				break;

			case 34:
				if (_curSequence != 2)
					_scene->_nextSceneId = 704;
				break;

			case 57:
				if (_curSequence == 6) {
					_curSequence = 0;
					nextBoatFrame = 91;
				} else if (_curSequence == 4)
					nextBoatFrame = 73;
				else if (_curSequence == 3)
					nextBoatFrame = 57;
				else {
					nextBoatFrame = 56;
					if (!_game._player._stepEnabled)
						_game._player._stepEnabled = true;
				}
				break;

			case 73:
				_scene->_nextSceneId = 701;
				break;

			case 82:
				nextBoatFrame = 11;
				break;

			case 91:
				nextBoatFrame = 57;
				break;

			case 100:
				nextBoatFrame = 56;
				if (!_game._player._stepEnabled) {
					_scene->_sequences.addTimer(30, 80);
					_game._player._stepEnabled = true;
				}
				break;

			case 110:
				nextBoatFrame = 9;
				if (!_game._player._stepEnabled) {
					_scene->_sequences.addTimer(30, 80);
					_game._player._stepEnabled = true;
				}
				break;

			default:
				break;
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextBoatFrame);
				_boatFrame = nextBoatFrame;
			}
		}
	}

	if (_game._trigger == 80) {
		switch (_globals[kBottleStatus]) {
		case 0:
			_vm->_dialogs->show(432);
			break;

		case 1:
			_vm->_dialogs->show(70324);
			break;

		case 2:
			_vm->_dialogs->show(70325);
			break;

		case 3:
			_vm->_dialogs->show(70326);
			break;

		case 4:
			_vm->_dialogs->show(70327);
			break;

		default:
			break;
		}
	}


	if ((_monsterMode == 1) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _boatFrame) {
			_boatFrame = _scene->_animation[0]->getCurrentFrame();
			int nextBoatFrame = -1;

			switch (_boatFrame) {
			case 39:
				_game._player._stepEnabled = true;
				_startMonsterTimer = true;
				_rexDeathFl = false;
				break;

			case 40:
			case 49:
			case 54:
			case 67:
			case 78:
			case 87:
			case 96:
			case 105:
			case 114:
			case 123:
				if (_curSequence == 8)
					nextBoatFrame = 129;

				break;

			case 129:
				nextBoatFrame = 39;
				break;

			case 151:
				_scene->_nextSceneId = 701;
				break;

			default:
				break;
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextBoatFrame);
				_boatFrame = nextBoatFrame;
			}
		}
	}

	if ((_monsterMode == 2) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _boatFrame) {
			_boatFrame = _scene->_animation[0]->getCurrentFrame();
			int nextBoatFrame = -1;

			switch (_boatFrame) {
			case 14:
				if (!_useBomb) {
					if (_game._difficulty == DIFFICULTY_HARD)
						_game._objects.setRoom(OBJ_CHICKEN, 1);
					else
						_vm->_dialogs->show(70319);
				}
				nextBoatFrame = 80;
				break;

			case 33:
				if (_game._objects.isInInventory(OBJ_BONES)) {
					_game._objects.setRoom(OBJ_BONES, 1);
					_game._objects.addToInventory(OBJ_BONE);
				} else
					_game._objects.setRoom(OBJ_BONE, 1);

				nextBoatFrame = 80;
				break;

			case 53:
				_game._objects.setRoom(OBJ_TWINKIFRUIT, 1);
				nextBoatFrame = 80;
				_curSequence = 9;
				break;

			case 80:
				if (_game._difficulty == DIFFICULTY_HARD) {
					_game._objects.setRoom(OBJ_BOMB, 1);
					_vm->_dialogs->show(70318);
				} else
					_vm->_dialogs->show(70317);

				_scene->freeAnimation();
				_monsterMode = 1;
				_scene->loadAnimation(formAnimName('B', -1));
				_scene->_animation[0]->setCurrentFrame(39);
				_game._player._stepEnabled = true;
				break;

			case 91:
				if (!_useBomb) {
					_scene->freeAnimation();
					_monsterMode = 1;
					_scene->loadAnimation(formAnimName('B', -1));
					_scene->_animation[0]->setCurrentFrame(39);
					_game._player._stepEnabled = true;
				} else
					_game._objects.setRoom(OBJ_CHICKEN_BOMB, 1);

				break;

			case 126:
				_scene->_hotspots.activate(NOUN_SEA_MONSTER, false);
				_globals[kMonsterAlive] = false;
				_scene->freeAnimation();
				_monsterMode = 0;
				_scene->loadAnimation(formAnimName('A', -1));
				_scene->_animation[0]->setCurrentFrame(9);
				_game._player._stepEnabled = true;
				if (_game._storyMode == STORYMODE_NAUGHTY)
					_vm->_dialogs->show(70321);
				else
					_vm->_dialogs->show(70322);

				break;

			default:
				break;
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextBoatFrame);
				_boatFrame = nextBoatFrame;
			}
		}
	}
}

void Scene703::actions() {
	if (_game._screenObjects._inputMode == kInputConversation)
		handleFillBottle(_action._activeAction._verbId);
	else if (_action.isAction(VERB_STEER_TOWARDS, NOUN_DOCK_TO_SOUTH)) {
		_game._player._stepEnabled = false;
		if (_globals[kMonsterAlive])
			_curSequence = 8;
		else if (_boatDir == 1)
			_curSequence = 5;
		else
			_curSequence = 3;
	} else if (_action.isAction(VERB_STEER_TOWARDS, NOUN_BUILDING_TO_NORTH)) {
		_game._player._stepEnabled = false;
		if (_globals[kMonsterAlive]) {
			_startMonsterTimer = false;
			_rexDeathFl = true;
			_monsterTime = 0;
			_scene->freeAnimation();
			_monsterMode = 3;
			_scene->loadAnimation(formAnimName('D', -1));
		} else if (_boatDir == 2)
			_curSequence = 4;
		else
			_curSequence = 1;
	} else if (_action.isAction(VERB_THROW, NOUN_BONE, NOUN_SEA_MONSTER) || _action.isAction(VERB_THROW, NOUN_BONES, NOUN_SEA_MONSTER)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
		_scene->_animation[0]->setCurrentFrame(19);
	} else if (_action.isAction(VERB_THROW, NOUN_CHICKEN, NOUN_SEA_MONSTER)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
	} else if (_action.isAction(VERB_THROW, NOUN_TWINKIFRUIT, NOUN_SEA_MONSTER)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
		_scene->_animation[0]->setCurrentFrame(39);
	} else if (_action.isAction(VERB_THROW, NOUN_BOMB, NOUN_SEA_MONSTER)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
		_scene->_animation[0]->setCurrentFrame(59);
	} else if (_action.isAction(VERB_THROW, NOUN_CHICKEN_BOMB, NOUN_SEA_MONSTER)) {
		_useBomb = true;
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
	} else if (_action.isAction(VERB_PUT, NOUN_BOTTLE, NOUN_WATER) || _action.isAction(VERB_FILL, NOUN_BOTTLE, NOUN_WATER)) {
		if (_globals[kBottleStatus] != 4) {
			handleBottleInterface();
			_dialog1.start();
		} else
			_vm->_dialogs->show(70323);
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_SEA_MONSTER)) {
		if (_globals[kMonsterAlive])
			_vm->_dialogs->show(70310);
	} else if (_action.isAction(VERB_LOOK, NOUN_WATER)) {
		if (!_globals[kMonsterAlive])
			_vm->_dialogs->show(70311);
		else
			_vm->_dialogs->show(70312);
	} else if (_action.isAction(VERB_LOOK, NOUN_BUILDING_TO_NORTH)) {
		if (_globals[kMonsterAlive])
			_vm->_dialogs->show(70313);
		else if (_game._visitedScenes.exists(710))
			_vm->_dialogs->show(70314);
		else
			_vm->_dialogs->show(70315);
	} else if (_action.isAction(VERB_LOOK, NOUN_VOLCANO_RIM))
		_vm->_dialogs->show(70316);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene704::Scene704(MADSEngine *vm) : Scene7xx(vm) {
	_bottleHotspotId = -1;
	_boatCurrentFrame = -1;
	_animationMode = -1;
	_boatDirection = -1;

	_takeBottleFl = false;
}

void Scene704::synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);

	s.syncAsSint16LE(_bottleHotspotId);
	s.syncAsSint16LE(_boatCurrentFrame);
	s.syncAsSint16LE(_animationMode);
	s.syncAsSint16LE(_boatDirection);

	s.syncAsByte(_takeBottleFl);
}

void Scene704::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
	_scene->addActiveVocab(NOUN_BOTTLE);
	_scene->addActiveVocab(VERB_LOOK_AT);
}

void Scene704::handleBottleInterface() {
	switch (_globals[kBottleStatus]) {
	case 0:
		_dialog1.write(0x311, true);
		_dialog1.write(0x312, true);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 1:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, true);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 2:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, false);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 3:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, false);
		_dialog1.write(0x313, false);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	default:
		break;
	}
}

void Scene704::setBottleSequence() {
	_scene->_userInterface.setup(kInputBuildingSentences);
	_game._player._stepEnabled = false;
	if (_boatDirection == 2)
		_animationMode = 6;
	else
		_animationMode = 7;
}

void Scene704::handleFillBottle(int quote) {
	switch (quote) {
	case 0x311:
		_globals[kBottleStatus] = 1;
		setBottleSequence();
		break;

	case 0x312:
		_globals[kBottleStatus] = 2;
		setBottleSequence();
		break;

	case 0x313:
		_globals[kBottleStatus] = 3;
		setBottleSequence();
		break;

	case 0x314:
		_globals[kBottleStatus] = 4;
		setBottleSequence();
		break;

	case 0x315:
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	default:
		break;
	}
}

void Scene704::enter() {
	if (_game._objects[OBJ_BOTTLE]._roomNumber == _scene->_currentSceneId) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		if (_scene->_priorSceneId == 705) {
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(123, 125));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		} else {
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(190, 122));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		}
		int idx = _scene->_dynamicHotspots.add(NOUN_BOTTLE, VERB_LOOK_AT, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_bottleHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
	}

	_game._player._visible = false;
	_takeBottleFl = false;
	_boatCurrentFrame = -1;

	if (_scene->_priorSceneId == 705) {
		_game._player._stepEnabled = false;
		_animationMode = 2;
		_boatDirection = 2;
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(36);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._stepEnabled = false;
		_boatDirection = 1;
		_scene->loadAnimation(formAnimName('A', -1));
	} else if (_boatDirection == 1) {
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(8);
	} else if (_boatDirection == 2) {
		if (_game._objects[OBJ_BOTTLE]._roomNumber == _scene->_currentSceneId) {
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(123, 125));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		}
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(57);
	}

	if (_scene->_roomChanged)
		_globals[kMonsterAlive] = false;

	_game.loadQuoteSet(0x311, 0x312, 0x313, 0x314, 0x315, 0);
	_dialog1.setup(0x98, 0x311, 0x312, 0x313, 0x314, 0x315, 0);

	sceneEntrySound();
	_vm->_sound->command(28);
}

void Scene704::step() {
	if (_scene->_animation[0] != nullptr) {
		if (_scene->_animation[0]->getCurrentFrame() != _boatCurrentFrame) {
			_boatCurrentFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			switch (_boatCurrentFrame) {
			case 10:
				switch (_animationMode) {
				case 1:
					nextFrame = 10;
					break;
				case 5:
					nextFrame = 74;
					break;
				case 7:
					_animationMode = 0;
					nextFrame = 92;
					break;
				default:
					if (!_game._player._stepEnabled)
						_game._player._stepEnabled = true;

					nextFrame = 8;
					break;
				}
				break;

			case 36:
				if (_animationMode != 2)
					_scene->_nextSceneId = 705;
				break;

			case 59:
				switch (_animationMode) {
				case 3:
					nextFrame = 59;
					break;

				case 4:
					nextFrame = 65;
					break;

				case 6:
					_animationMode = 0;
					nextFrame = 83;
					break;

				default:
					if (!_game._player._stepEnabled) {
						_game._player._stepEnabled = true;
					}
					nextFrame = 57;
					break;
				}
				break;

			case 65:
				_scene->_nextSceneId = 703;
				break;

			case 74:
				nextFrame = 10;
				break;

			case 83:
				nextFrame = 59;
				break;

			case 90:
				if (_takeBottleFl) {
					_scene->_sequences.remove(_globals._sequenceIndexes[1]);
					_scene->_dynamicHotspots.remove(_bottleHotspotId);
					_game._objects.addToInventory(OBJ_BOTTLE);
					_vm->_sound->command(15);
					_vm->_dialogs->showItem(OBJ_BOTTLE, 70415);
				}
				break;

			case 92:
				nextFrame = 57;
				if (!_game._player._stepEnabled && !_takeBottleFl) {
					_scene->_sequences.addTimer(30, 70);
					_game._player._stepEnabled = true;
				}
				break;

			case 98:
				if (_takeBottleFl) {
					_scene->_sequences.remove(_globals._sequenceIndexes[1]);
					_scene->_dynamicHotspots.remove(_bottleHotspotId);
					_game._objects.addToInventory(OBJ_BOTTLE);
					_vm->_sound->command(15);
					_vm->_dialogs->showItem(OBJ_BOTTLE, 70415);
				}
				break;

			case 101:
				nextFrame = 8;
				if (!_game._player._stepEnabled && !_takeBottleFl) {
					_scene->_sequences.addTimer(30, 70);
					_game._player._stepEnabled = true;
				}
				break;

			default:
				break;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_boatCurrentFrame = nextFrame;
			}
		}
	}

	if (_game._trigger == 70) {
		switch (_globals[kBottleStatus]) {
		case 0:
			_vm->_dialogs->show(432);
			break;

		case 1:
			_vm->_dialogs->show(70324);
			break;

		case 2:
			_vm->_dialogs->show(70325);
			break;

		case 3:
			_vm->_dialogs->show(70326);
			break;

		case 4:
			_vm->_dialogs->show(70327);
			break;

		default:
			break;
		}
	}
}

void Scene704::actions() {
	if (_game._screenObjects._inputMode == kInputConversation)
		handleFillBottle(_action._activeAction._verbId);
	else if (_action.isAction(VERB_STEER_TOWARDS, NOUN_OPEN_WATER_TO_SOUTH)) {
		_game._player._stepEnabled = false;
		if (_boatDirection == 1)
			_animationMode = 5;
		else
			_animationMode = 3;
	} else if (_action.isAction(VERB_STEER_TOWARDS, NOUN_BUILDING_TO_NORTH)) {
		_game._player._stepEnabled = false;
		if (_boatDirection == 2)
			_animationMode = 4;
		else
			_animationMode = 1;
	} else if (_action.isAction(VERB_TAKE, NOUN_BOTTLE)) {
		if (!_game._objects.isInInventory(OBJ_BOTTLE)) {
			_game._player._stepEnabled = false;
			_takeBottleFl = true;
			if (_boatDirection == 2) {
				_animationMode = 6;
			} else {
				_animationMode = 7;
			}
		}
	} else if (_action.isAction(VERB_PUT, NOUN_BOTTLE, NOUN_WATER) || _action.isAction(VERB_FILL, NOUN_BOTTLE, NOUN_WATER)) {
		if (_game._objects.isInInventory(OBJ_BOTTLE)) {
			if (_globals[kBottleStatus] != 4) {
				_takeBottleFl = false;
				handleBottleInterface();
				_dialog1.start();
			} else
				_vm->_dialogs->show(70323);
		}
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_WATER))
		_vm->_dialogs->show(70410);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING_TO_NORTH)) {
		if (_game._visitedScenes.exists(710))
			_vm->_dialogs->show(70411);
		else
			_vm->_dialogs->show(70412);
	} else if (_action.isAction(VERB_LOOK, NOUN_VOLCANO_RIM))
		_vm->_dialogs->show(70413);
	else if (_action.isAction(VERB_LOOK, NOUN_BOTTLE) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(70414);
	else if (_action.isAction(VERB_LOOK, NOUN_OPEN_WATER_TO_SOUTH))
		_vm->_dialogs->show(70416);
	else if (_action.isAction(VERB_LOOK, NOUN_SKY))
		_vm->_dialogs->show(70417);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene705::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene705::synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);
}

void Scene705::handleBottleInterface() {
	switch (_globals[kBottleStatus]) {
	case 0:
		_dialog1.write(0x311, true);
		_dialog1.write(0x312, true);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 1:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, true);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 2:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, false);
		_dialog1.write(0x313, true);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	case 3:
		_dialog1.write(0x311, false);
		_dialog1.write(0x312, false);
		_dialog1.write(0x313, false);
		_dialog1.write(0x314, true);
		_dialog1.write(0x315, true);
		break;

	default:
		break;
	}
}

void Scene705::setBottleSequence() {
	_scene->_userInterface.setup(kInputBuildingSentences);
	_game._player._stepEnabled = false;
	_scene->_sequences.remove(_globals._sequenceIndexes[3]);
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
	_scene->loadAnimation(formAnimName('F', -1), 90);
}

void Scene705::handleFillBottle(int quote) {
	switch (quote) {
	case 0x311:
		_globals[kBottleStatus] = 1;
		setBottleSequence();
		break;

	case 0x312:
		_globals[kBottleStatus] = 2;
		setBottleSequence();
		break;

	case 0x313:
		_globals[kBottleStatus] = 3;
		setBottleSequence();
		break;

	case 0x314:
		_globals[kBottleStatus] = 4;
		setBottleSequence();
		break;

	case 0x315:
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	default:
		break;
	}
}

void Scene705::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 0));

	_game._player._visible = false;

	if (_scene->_priorSceneId == 706) {
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._stepEnabled = false;
		_scene->_sequences.addTimer(1, 80);
		_vm->_sound->command(28);
	} else
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BOTTLE);

	_game.loadQuoteSet(0x311, 0x312, 0x313, 0x314, 0x315, 0);
	_dialog1.setup(0x98, 0x311, 0x312, 0x313, 0x314, 0x315, 0);
	sceneEntrySound();
}

void Scene705::step() {
	switch (_game._trigger) {
	case 70:
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71: {
		int syncIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
		_game._player._stepEnabled = true;
		}
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 80:
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
		break;

	case 81: {
		_vm->_sound->command(19);
		int syncIdx = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
		_game._player._stepEnabled = true;
		}
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 90:
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.addTimer(30, 91);
		break;

	case 91:
		switch (_globals[kBottleStatus]) {
		case 0:
			_vm->_dialogs->show(432);
			break;

		case 1:
			_vm->_dialogs->show(70324);
			break;

		case 2:
			_vm->_dialogs->show(70325);
			break;

		case 3:
			_vm->_dialogs->show(70326);
			break;

		case 4:
			_vm->_dialogs->show(70327);
			break;

		default:
			break;
		}
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene705::actions() {
	if (_game._screenObjects._inputMode == kInputConversation)
		handleFillBottle(_action._activeAction._verbId);
	else if (_action.isAction(VERB_STEER_TOWARDS, NOUN_OPEN_WATER_TO_SOUTH)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_vm->_sound->command(18);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
			_scene->_nextSceneId = 704;
			_game._player._stepEnabled = true;
			}
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_CLIMB_THROUGH, NOUN_WINDOW)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 16);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 16);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_scene->_nextSceneId = 706;
			_game._player._stepEnabled = true;
			}
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_FILL, NOUN_BOTTLE, NOUN_WATER) || _action.isAction(VERB_PUT, NOUN_BOTTLE, NOUN_WATER)) {
		if (_globals[kBottleStatus] != 4) {
			handleBottleInterface();
			_dialog1.start();
		} else
			_vm->_dialogs->show(70323);
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_WATER))
		_vm->_dialogs->show(70511);
	else if (_action.isAction(VERB_LOOK, NOUN_VOLCANO_RIM))
		_vm->_dialogs->show(70512);
	else if (_action.isAction(VERB_LOOK, NOUN_OPEN_WATER_TO_SOUTH))
		_vm->_dialogs->show(70513);
	else if (_action.isAction(VERB_LOOK, NOUN_SKY))
		_vm->_dialogs->show(70514);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING))
		_vm->_dialogs->show(70515);
	else if (_action.isAction(VERB_LOOK, NOUN_WINDOW))
		_vm->_dialogs->show(70516);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene706::Scene706(MADSEngine *vm) : Scene7xx(vm) {
	_vaseHotspotId = -1;
	_vaseMode = -1;
	_animationMode = -1;
	_animationFrame = -1;

	_emptyPedestral = false;
}

void Scene706::synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);

	s.syncAsSint16LE(_vaseHotspotId);
	s.syncAsSint16LE(_vaseMode);
	s.syncAsSint16LE(_animationMode);
	s.syncAsSint16LE(_animationFrame);

	s.syncAsByte(_emptyPedestral);
}

void Scene706::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_BOTTLE);
	_scene->addActiveVocab(NOUN_VASE);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene706::handleRexDeath() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->loadAnimation(formAnimName('a', -1), 2);
		break;

	case 2:
		if (_animationMode == 1)
			_vm->_dialogs->show(70625);
		else if (_globals[kBottleStatus] < 2)
			_vm->_dialogs->show(70628);
		else
			_vm->_dialogs->show(70629);

		_game._objects.setRoom(OBJ_VASE, _scene->_currentSceneId);
		if (_animationMode == 2)
			_game._objects.setRoom(OBJ_BOTTLE, 2);

		_animationMode = 0;
		_scene->_reloadSceneFlag = true;
		break;

	default:
		break;
	}
}

void Scene706::handleTakeVase() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 4, 2, 0, 0);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 7, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
		_vm->_sound->command(9);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_scene->_dynamicHotspots.remove(_vaseHotspotId);
		_game._objects.addToInventory(OBJ_VASE);
		if (_vaseMode == 1) {
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(195, 99));
			int idx = _scene->_dynamicHotspots.add(NOUN_BOTTLE, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
			_game._objects.setRoom(OBJ_BOTTLE, _scene->_currentSceneId);
		}
		break;

	case 2:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
		_game._player._visible = true;
		_vm->_dialogs->showItem(OBJ_VASE, 70630);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene706::enter() {
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_3");
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('b', -1));

	if (!_game._visitedScenes._sceneRevisited)
		_emptyPedestral = false;

	if (_game._objects[OBJ_VASE]._roomNumber == _scene->_currentSceneId) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('v', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
		int idx = _scene->_dynamicHotspots.add(NOUN_VASE, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_vaseHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
	} else if (_game._objects.isInRoom(OBJ_BOTTLE)) {
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(195, 99));
		int idx = _scene->_dynamicHotspots.add(NOUN_BOTTLE, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
	}

	_game._player._visible = true;

	if (_scene->_priorSceneId == 707) {
		_game._player._playerPos = Common::Point(277, 103);
		_game._player._facing = FACING_SOUTHWEST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(167, 152);
		_game._player._facing = FACING_NORTH;
	}

	if (_globals[kTeleporterCommand]) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;

		switch (_globals[kTeleporterCommand]) {
		case 1:
			_scene->loadAnimation(formAnimName('E', 1), 75);
			break;

		case 2:
			_scene->loadAnimation(formAnimName('E', -1), 80);
			break;

		default:
			_game._player.walk(Common::Point(264, 116), FACING_SOUTHWEST);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		}
		_globals[kTeleporterCommand] = 0;
	}

	_animationMode = 0;

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_BOTTLE);
		_globals[kBottleStatus] = 2;
	}

	sceneEntrySound();
}

void Scene706::step() {
	if (_game._trigger == 75) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player.walk(Common::Point(264, 116), FACING_SOUTHWEST);
	}

	if (_game._trigger == 80) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}

	if (_scene->_animation[0] != nullptr) {
		if ((_animationMode != 0) && (_scene->_animation[0]->getCurrentFrame() != _animationFrame)) {
			_animationFrame = _scene->_animation[0]->getCurrentFrame();

			if (_animationFrame == 6) {
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_game._objects.setRoom(OBJ_VASE, 2);

				if (_animationMode == 2) {
					_game._objects.setRoom(OBJ_BOTTLE, 1);

					_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
					_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(195, 99));
					int idx = _scene->_dynamicHotspots.add(NOUN_BOTTLE, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
					_scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 124), FACING_SOUTHEAST);
				}
			}
		}
	}
}

void Scene706::preActions() {
	if (_action.isAction(VERB_LOOK, NOUN_PORTRAIT))
		_game._player._needToWalk = true;
}

void Scene706::actions() {
	if (_action.isAction(VERB_WALK_INSIDE, NOUN_TELEPORTER)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 707;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT, NOUN_ROOM)) {
		_scene->_nextSceneId = 705;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_VASE)) {
		if (_game._difficulty != DIFFICULTY_EASY) {
			_animationMode = 1;
			handleRexDeath();
		} else if (_game._trigger || !_game._objects.isInInventory(OBJ_VASE)) {
			handleTakeVase();
			_emptyPedestral = true;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, NOUN_BOTTLE, NOUN_PEDESTAL)) {
		if ((_globals[kBottleStatus] == 2 && _game._difficulty == DIFFICULTY_HARD) ||
			 (_globals[kBottleStatus] != 0 && _game._difficulty != DIFFICULTY_HARD)) {
			if (!_game._objects.isInInventory(OBJ_VASE) || _game._trigger) {
				_vaseMode = 1;
				handleTakeVase();
				_action._inProgress = false;
				return;
			}
		} else if (_game._objects.isInRoom(OBJ_VASE) || _game._trigger) {
			_animationMode = 2;
			handleRexDeath();
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_PUT, NOUN_PEDESTAL) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		int objectId = _game._objects.getIdFromDesc(_action._activeAction._objectNameId);
		if (_game._objects[objectId].hasQuality(10))
			_vm->_dialogs->show(70626);
		else
			_vm->_dialogs->show(70627);
	} else if (_action.isAction(VERB_TAKE, NOUN_BOTTLE) && _game._objects.isInInventory(OBJ_VASE))
		_vm->_dialogs->show(70631);
	else if (_action._lookFlag) {
		if (_game._objects[OBJ_VASE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(70610);
		else
			_vm->_dialogs->show(70611);
	} else if (_action.isAction(VERB_LOOK, NOUN_FLOOR))
		_vm->_dialogs->show(70612);
	else if (_action.isAction(VERB_LOOK, NOUN_PILLAR))
		_vm->_dialogs->show(70613);
	else if (_action.isAction(VERB_LOOK, NOUN_OLD_TEA_CUP))
		_vm->_dialogs->show(70614);
	else if (_action.isAction(VERB_TAKE, NOUN_OLD_TEA_CUP))
		_vm->_dialogs->show(70615);
	else if (_action.isAction(VERB_LOOK, NOUN_OLD_VASE))
		_vm->_dialogs->show(70616);
	else if (_action.isAction(VERB_LOOK, NOUN_PORTRAIT))
		_vm->_dialogs->show(70617);
	else if (_action.isAction(VERB_LOOK, NOUN_NAME_PLATE))
		_vm->_dialogs->show(70618);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(70619);
	else if (_action.isAction(VERB_LOOK, NOUN_PEDESTAL)) {
		if (_game._objects[OBJ_VASE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(70620);
		else if (_game._objects[OBJ_BOTTLE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(70622);
		else
			_vm->_dialogs->show(70621);
	} else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER))
		_vm->_dialogs->show(70623);
	else if (_action.isAction(VERB_LOOK, NOUN_VASE) && (_game._objects[OBJ_VASE]._roomNumber == _scene->_currentSceneId))
		_vm->_dialogs->show(70624);
	else if (_action.isAction(VERB_LOOK, NOUN_BOTTLE) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(70632);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene707::setup() {
	_game._player._spritesPrefix = "";
	// The original calls Scene7xx::setAAName()
	_game._aaName = Resources::formatAAName(5);
}

void Scene707::enter() {
	_handSpriteId = _scene->_sprites.addSprites("*REXHAND");
	teleporterEnter();

	// The original uses Scene7xx_sceneEntrySound
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else
		_vm->_sound->command(25);
}

void Scene707::step() {
	teleporterStep();
}

void Scene707::actions() {
	if (teleporterActions()) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_VIEWPORT) || _action.isAction(VERB_PEER_THROUGH, NOUN_VIEWPORT))
		_vm->_dialogs->show(70710);
	else if (_action.isAction(VERB_LOOK, NOUN_KEYPAD))
		_vm->_dialogs->show(70711);
	else if (_action.isAction(VERB_LOOK, NOUN_DISPLAY))
		_vm->_dialogs->show(70712);
	else if (_action.isAction(VERB_LOOK, NOUN_0_KEY) || _action.isAction(VERB_LOOK, NOUN_1_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_2_KEY) || _action.isAction(VERB_LOOK, NOUN_3_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_4_KEY) || _action.isAction(VERB_LOOK, NOUN_5_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_6_KEY) || _action.isAction(VERB_LOOK, NOUN_7_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_8_KEY) || _action.isAction(VERB_LOOK, NOUN_9_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_SMILE_KEY) || _action.isAction(VERB_LOOK, NOUN_ENTER_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_FROWN_KEY))
		_vm->_dialogs->show(70713);
	else if (_action.isAction(VERB_LOOK, NOUN_DEVICE) || _action._lookFlag)
		_vm->_dialogs->show(70714);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene710::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene710::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);

	if (_game._objects[OBJ_VASE]._roomNumber == 706) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
	}

	_game._player._visible = false;
	_scene->_sequences.addTimer(600, 70);

	sceneEntrySound();
}

void Scene710::step() {
	if (_game._trigger == 70) {
		if (_game._globals[kCityFlooded])
			_scene->_nextSceneId = 701;
		else
			_scene->_nextSceneId = 751;
	}
}

void Scene710::actions() {
	if (_action.isAction(VERB_PUT_DOWN, NOUN_BINOCULARS)) {
		_game._player._stepEnabled = false;

		if (_game._globals[kCityFlooded])
			_scene->_nextSceneId = 701;
		else
			_scene->_nextSceneId = 751;

		_action._inProgress = false;
	}
}

/*------------------------------------------------------------------------*/

void Scene711::setup() {
	// The original was calling Scene7xx::setPlayerSpreitesPrefix()
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;
	_game._player._spritesPrefix = "";
	_game._player._scalingVelocity = true;

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);

	// The original was calling Scene7xx::setAAName()
	_game._aaName = Resources::formatAAName(5);

	_game._player._spritesPrefix = "";
}

void Scene711::enter() {
	if (_globals[kSexOfRex] == REX_FEMALE)
		_handSpriteId = _scene->_sprites.addSprites("*ROXHAND");
	else
		_handSpriteId = _scene->_sprites.addSprites("*REXHAND");

	teleporterEnter();

	// The original was using Scene7xx_SceneEntrySound()
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else
		_vm->_sound->command(25);
}

void Scene711::step() {
	teleporterStep();
}

void Scene711::actions() {
	if (teleporterActions())
		_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene751::Scene751(MADSEngine *vm) : Scene7xx(vm) {
	_rexHandingLine = false;
}

void Scene751::synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);

	s.syncAsByte(_rexHandingLine);
}

void Scene751::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_FISHING_LINE);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene751::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RM701X0");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RM202A1");

	if (!_game._visitedScenes._sceneRevisited)
		_rexHandingLine = false;

	if (_globals[kLineStatus] == 2 || _globals[kLineStatus] == 3) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
		int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(268, 140), FACING_NORTHWEST);
	}

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);

	if (_scene->_priorSceneId == 752) {
		_game._player._playerPos = Common::Point(309, 138);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 710) {
		_game._player._playerPos = Common::Point(154, 129);
		_game._player._facing = FACING_NORTH;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(155, 129));
		_scene->_sequences.addTimer(15, 70);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(22, 131);
		_game._player._facing = FACING_EAST;
		_game._player._stepEnabled = false;
		_scene->_sequences.addTimer(60, 60);
	} else if (_rexHandingLine) {
		_game._player._visible = false;
		_game._player._playerPos = Common::Point(268, 140);
		_game._player._facing = FACING_NORTHWEST;
		_game._player._visible = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 7);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
	} else if (_globals[kLineStatus] == 2) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
		int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(268, 140), FACING_NORTHWEST);
	}

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_FISHING_LINE);
		_game._objects.addToInventory(OBJ_BINOCULARS);
	}

	sceneEntrySound();
	_game.loadQuoteSet(0x30A, 0x30B, 0x30C, 0x30D, 0x30E, 0);

	if (_globals[kTimebombTimer] > 0)
		_globals[kTimebombTimer] = 10200;
}

void Scene751::step() {
	switch (_game._trigger) {
	case 70:
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 6, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(155, 129));
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[4]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	if ((_globals[kTimebombTimer] >= 10800) && (_globals[kTimebombStatus] == 1)) {
		_globals[kTimebombStatus] = 3;
		_globals[kTimebombTimer] = 0;
		_globals[kCheckDaemonTimebomb] = false;
		_scene->_nextSceneId = 620;
	}

	switch (_game._trigger) {
	case 60:
		_vm->_sound->command(16);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		_game._player.walk(Common::Point(61, 131), FACING_EAST);
		_scene->_sequences.addTimer(120, 62);
		break;

	case 62:
		_vm->_sound->command(17);
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 63);
		break;

	case 63:
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_game._player._stepEnabled = true;
		_scene->_kernelMessages.reset();
		break;

	default:
		break;
	}
}

void Scene751::preActions() {
	if (_action.isAction(VERB_LOOK, NOUN_TALL_BUILDING))
		_game._player.walk(Common::Point(154, 129), FACING_NORTHEAST);

	if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_TALL_BUILDING))
		_game._player.walk(Common::Point(154, 129), FACING_NORTH);

	if (_action.isAction(VERB_WALKTO, NOUN_EAST_END_OF_PLATFORM))
		_game._player._walkOffScreenSceneId = 752;

	if (!_rexHandingLine)
		return;

	if (_action.isAction(VERB_LOOK) || _action.isObject(NOUN_FISHING_LINE) || _action.isAction(VERB_TALKTO))
		_game._player._needToWalk = false;

	if ((!_action.isAction(VERB_PUT, NOUN_FISHING_LINE, NOUN_HOOK) || !_action.isAction(VERB_TIE, NOUN_FISHING_LINE, NOUN_HOOK) || !_action.isAction(VERB_ATTACH, NOUN_FISHING_LINE, NOUN_HOOK))
	&& (_game._player._needToWalk)) {
		switch (_game._trigger) {
		case 0:
			_game._player._readyToWalk = false;
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 11, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, 7);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			_rexHandingLine = false;
			_game._player._stepEnabled = true;
			_game._player._readyToWalk = true;
			break;

		default:
			break;
		}
	}
}

void Scene751::actions() {
	if (_action.isAction(VERB_WALK_ALONG, NOUN_PLATFORM))
		; // Nothing
	else if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_TALL_BUILDING)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(155, 129));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(155, 129));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_sequences.addTimer(15, 2);
			}
			break;

		case 2:
			_scene->_nextSceneId = 710;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_STEP_INTO, NOUN_ELEVATOR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_vm->_sound->command(16);
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x30D));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_game._player.walk(Common::Point(22, 131), FACING_EAST);
			_scene->_sequences.addTimer(120, 3);
			break;

		case 3:
			_vm->_sound->command(17);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_scene->_sequences.addTimer(60, 5);
			break;

		case 5:
			_game._player._stepEnabled = true;
			_scene->_nextSceneId = 513;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PUT, NOUN_FISHING_LINE, NOUN_HOOK) || _action.isAction(VERB_TIE, NOUN_FISHING_LINE, NOUN_HOOK) || _action.isAction(VERB_ATTACH, NOUN_FISHING_LINE, NOUN_HOOK)) {
		if (_globals[kLineStatus] == 1) {
			switch (_game._trigger) {
			case 0:
				_game._player._visible = false;
				_game._player._stepEnabled = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, 6);
				_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], -1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1: {
				int syncIdx = _globals._sequenceIndexes[2];
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 7);
				_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
				_scene->_sequences.addTimer(30, 2);
				}
				break;

			case 2:
				_rexHandingLine = true;
				_scene->_sequences.remove(_globals._sequenceIndexes[2]);
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 8, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3: {
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				_game._player._visible = true;
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
				int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(268, 140), FACING_NORTHWEST);
				_scene->_kernelMessages.reset();
				_game._objects.setRoom(OBJ_FISHING_LINE, _scene->_currentSceneId);
				_rexHandingLine = false;
				_globals[kLineStatus] = 2;
				_game._player._stepEnabled = true;
				_vm->_dialogs->show(75120);
				}
				break;

			default:
				break;
			}
		}
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_CITY))
		_vm->_dialogs->show(75110);
	else if (_action.isAction(VERB_LOOK, NOUN_ELEVATOR))
		_vm->_dialogs->show(75112);
	else if (_action.isAction(VERB_LOOK, NOUN_PLATFORM))
		_vm->_dialogs->show(75113);
	else if (_action.isAction(VERB_LOOK, NOUN_CEMENT_PYLON))
		_vm->_dialogs->show(75114);
	else if ((_action.isAction(VERB_LOOK, NOUN_HOOK) || _action.isAction(VERB_LOOK, NOUN_FISHING_LINE))
		&& (_globals[kLineStatus] == 2 || _globals[kLineStatus] == 3))
		_vm->_dialogs->show(75116);
	else if (_action.isAction(VERB_LOOK, NOUN_HOOK))
		_vm->_dialogs->show(75115);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCK))
		_vm->_dialogs->show(75117);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCK))
		_vm->_dialogs->show(75118);
	else if (_action.isAction(VERB_LOOK, NOUN_EAST_END_OF_PLATFORM))
		_vm->_dialogs->show(75119);
	else if (_action.isAction(VERB_TAKE, NOUN_FISHING_LINE) && (_globals[kLineStatus] == 3 || _globals[kLineStatus] == 2))
		_vm->_dialogs->show(75121);
	else if (_action.isAction(VERB_LOOK, NOUN_TALL_BUILDING))
		_vm->_dialogs->show(75122);
	else if (_action.isAction(VERB_TIE, NOUN_FISHING_LINE, NOUN_CEMENT_PYLON) || _action.isAction(VERB_ATTACH, NOUN_FISHING_LINE, NOUN_CEMENT_PYLON))
		_vm->_dialogs->show(75123);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene752::Scene752(MADSEngine *vm) : Scene7xx(vm) {
	_cardId = -1;
}

void Scene752::synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);

	s.syncAsSint16LE(_cardId);
}

void Scene752::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_ID_CARD);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(VERB_LOOK_AT);
	_scene->addActiveVocab(NOUN_LASER_BEAM);
}

void Scene752::enter() {
	_globals._spriteIndexes[14] = _scene->_sprites.addSprites(formAnimName('l', -1));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RXMBD_8");

	if (_scene->_priorSceneId == 751) {
		_game._player._playerPos = Common::Point(13, 145);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(289, 138);
		_game._player.walk(Common::Point(262, 148), FACING_WEST);
		_game._player._facing = FACING_WEST;
		_game._player._visible = true;
	}

	if (_game._objects[OBJ_ID_CARD]._roomNumber == 752) {
		_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('i', -1));
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		int idx = _scene->_dynamicHotspots.add(NOUN_ID_CARD, VERB_WALKTO, _globals._sequenceIndexes[13], Common::Rect(0, 0, 0, 0));
		_cardId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(234, 135), FACING_NORTH);
	}

	if (_game._globals[kLaserHoleIsThere]) {
		_globals._sequenceIndexes[14] = _scene->_sequences.startCycle(_globals._spriteIndexes[14], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 13);
		int idx = _scene->_dynamicHotspots.add(NOUN_LASER_BEAM, VERB_LOOK_AT, _globals._sequenceIndexes[14], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(215, 130), FACING_NORTHWEST);
	}

	if (_game._globals[kTeleporterCommand]) {
		switch(_game._globals[kTeleporterCommand]) {
		case TELEPORTER_BEAM_OUT:
		case TELEPORTER_WRONG:
		case TELEPORTER_STEP_OUT:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}

		_game._globals[kTeleporterCommand] = TELEPORTER_NONE;
	}

	if (_globals._timebombTimer > 0)
		_globals._timebombTimer = 10800 - 600;

	sceneEntrySound();
}

void Scene752::step() {
	if (_globals._timebombTimer >= 10800 && _game._globals[kTimebombStatus] == TIMEBOMB_ACTIVATED) {
		_globals[kTimebombStatus] = TIMEBOMB_DEAD;
		_globals._timebombTimer = 0;
		_globals[kCheckDaemonTimebomb] = false;
		_scene->_nextSceneId = 620;
	}
}

void Scene752::preActions() {
	if (_action.isAction(VERB_WALKTO, NOUN_WEST_END_OF_PLATFORM)) {
		_game._player._walkOffScreenSceneId = 751;
	}
}

void Scene752::actions() {
	if (_action.isAction(VERB_WALK_ALONG, NOUN_PLATFORM))
		;
	else if (_action.isAction(VERB_STEP_INTO, NOUN_TELEPORTER)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 711;
	} else if (_action.isAction(VERB_TAKE, NOUN_ID_CARD) && (!_game._objects.isInInventory(OBJ_ID_CARD) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			_vm->_sound->command(15);
			_scene->_sequences.remove(_globals._sequenceIndexes[13]);
			_game._objects.addToInventory(OBJ_ID_CARD);
			_scene->_dynamicHotspots.remove(_cardId);
			_vm->_dialogs->showItem(OBJ_ID_CARD, 830);
			break;
		case 2:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && (_action._savedFields._mainObjectSource == CAT_HOTSPOT) &&
			(!_game._objects.isInInventory(OBJ_BONES) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			_vm->_sound->command(15);
			if (_game._objects.isInInventory(OBJ_BONE))
				_game._objects.setRoom(OBJ_BONE, NOWHERE);
			_game._objects.addToInventory(OBJ_BONES);
			_vm->_dialogs->showItem(OBJ_BONES, 75221);
			break;
		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[12]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_CITY)) {
		if (_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(75212);
		else
			_vm->_dialogs->show(75210);
	} else if (_action.isAction(VERB_LOOK, NOUN_PLATFORM))
		_vm->_dialogs->show(75213);
	else if (_action.isAction(VERB_LOOK, NOUN_CEMENT_BLOCK))
		_vm->_dialogs->show(75214);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCK))
		_vm->_dialogs->show(75215);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCK))
		_vm->_dialogs->show(75216);
	else if (_action.isAction(VERB_LOOK, NOUN_WEST_END_OF_PLATFORM))
		_vm->_dialogs->show(75217);
	else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER))
		_vm->_dialogs->show(75218);
	else if ((_action.isAction(VERB_LOOK, NOUN_BONES) || _action.isAction(VERB_LOOK, NOUN_ID_CARD)) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		if (_game._objects[OBJ_ID_CARD]._roomNumber == 752)
			_vm->_dialogs->show(75219);
		else
			_vm->_dialogs->show(75220);
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && (_action._savedFields._mainObjectSource == CAT_HOTSPOT)) {
		if (_game._objects.isInInventory(OBJ_BONES))
			_vm->_dialogs->show(75222);
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
