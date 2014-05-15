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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

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
	//_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 1));	// FIXME: Broken sprite?
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
	_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);

	int boatStatus = (_scene->_priorSceneId == 703) ? BOAT_GONE : _globals[kBoatStatus];

	switch (boatStatus) {
	case BOAT_TIED_FLOATING:
		_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 20, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 10);
		break;
	case BOAT_ADRIFT:
		_globals._sequenceIndexes[6] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[6], false, 20, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 10);
		break;
	case BOAT_TIED:
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		break;
	case BOAT_GONE:
		_scene->_hotspots.activate(NOUN_BOAT, false);
		break;
	default:
		break;
	}

	// TODO: Enable once sprite 3 can be loaded properly
	/*
	if (_globals[kLineStatus] == LINE_DROPPED || _globals[kLineStatus] == LINE_TIED) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_fishingLineId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(234, 129), FACING_NORTHEAST);
	}
	*/

	if (_scene->_priorSceneId == 702) {
		_game._player._playerPos = Common::Point(309, 138);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 710) {
		_game._player._playerPos = Common::Point(154, 129);
		_game._player._facing = FACING_NORTH;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[5], Common::Point(155, 129));
		_scene->_sequences.addTimer(15, 60);
	} else if (_scene->_priorSceneId == 703) {
		_game._player._playerPos = Common::Point(231, 127);
		_game._player._facing = FACING_SOUTH;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(formAnimName('B', 1), 80);
		_vm->_sound->command(28);
	} else if (_scene->_priorSceneId != -2 && _scene->_priorSceneId != -620) {
		_game._player._playerPos = Common::Point(22, 131);
		_game._player._facing = FACING_EAST;
		// TODO: Enable once step() is implemented
		//_game._player._stepEnabled = false;
		//_scene->_sequences.addTimer(1 * 60, 70);
	}

	_game.loadQuoteSet(0x310, 0x30F, 0);
	sceneEntrySound();
}

void Scene701::step() {
	switch(_game._trigger) {
	case 60:
		// TODO
		break;
	case 61:
		// TODO
		break;
	case 70:
		// TODO
		break;
	case 71:
		_game._player.walk(Common::Point(61, 131), FACING_EAST);
		_scene->_sequences.addTimer(2 * 60, 72);
		break;
	case 72:
		// TODO
		break;
	case 73:
		// TODO
		break;
	case 80:
		_game._player._visible = true;
		// TODO
		_globals[kBoatStatus] = BOAT_TIED;
		_game._player._stepEnabled = true;
		break;
	}
}

void Scene701::preActions() {
	if (_action.isAction(VERB_WALKTO, NOUN_EAST_END_OF_PLATFORM)) {
		_game._player._walkOffScreenSceneId = 702;
	}

	if (_action.isAction(VERB_LOOK, NOUN_BUILDING)) {
		_game._player.walk(Common::Point(154, 129), FACING_NORTHEAST);
	}

	if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_BUILDING)) {
		_game._player.walk(Common::Point(154, 129), FACING_NORTH);
	}
}

void Scene701::actions() {
	if (_action.isAction(VERB_WALK_ALONG, NOUN_PLATFORM))
		return;
	
	if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_BUILDING) && _game._objects[OBJ_VASE]._roomNumber == 706) {
		switch (_game._trigger) {
		case 0:
			// TODO
			break;
		case 1:
			// TODO
			break;
		case 2:
			_scene->_nextSceneId = 710;
			break;
		}
	} else if (_action.isAction(VERB_STEP_INTO, NOUN_ELEVATOR)) {
		switch (_game._trigger) {
		case 0:
			// TODO
			break;
		case 1:
			// TODO
			break;
		case 3:
			// TODO
			break;
		case 4:
			// TODO
			break;
		}
	} else if (false) {
		// TODO: boat + fishing line action
	} else if (_action.isAction(VERB_CLIMB_INTO, NOUN_BOAT) && _globals[kBoatStatus] == BOAT_TIED) {
		switch (_game._trigger) {
		case 0:
			// TODO
			break;
		case 1:
			_scene->_nextSceneId = 703;
			break;
		}
	} else if (_action._lookFlag) {
		if (_globals[kBoatStatus] != BOAT_GONE) {
			if (_globals[kBoatStatus] == BOAT_TIED)
				_vm->_dialogs->show(70128);
			else
				_vm->_dialogs->show(70110);
		} else {
			_vm->_dialogs->show(70111);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_SUBMERGED_CITY)) {
		_vm->_dialogs->show(70112);
	} else if (_action.isAction(VERB_LOOK, 0)) {
		_vm->_dialogs->show(70113);
	} else if (_action.isAction(VERB_LOOK, NOUN_PLATFORM)) {
		_vm->_dialogs->show(70114);
	} else if (_action.isAction(VERB_LOOK, NOUN_CEMENT_PYLON)) {
		_vm->_dialogs->show(70115);
	} else if (false) {
		// TODO: hook
	} else if (_action.isAction(VERB_LOOK, NOUN_ROCK)) {
		_vm->_dialogs->show(70118);
	} else if (_action.isAction(VERB_TAKE, NOUN_ROCK)) {
		_vm->_dialogs->show(70119);
	} else if (_action.isAction(VERB_LOOK, NOUN_EAST_END_OF_PLATFORM)) {
		_vm->_dialogs->show(70120);
	} else if (_action.isAction(VERB_LOOK, NOUN_BUILDING)) {
		_vm->_dialogs->show(70121);
	} else if (_action.isAction(VERB_LOOK, NOUN_BOAT)) {
		if (_globals[kBoatStatus] == BOAT_ADRIFT || _globals[kBoatStatus] == BOAT_TIED_FLOATING)
			_vm->_dialogs->show(70122);
		else
			_vm->_dialogs->show(70123);
	} else if (false) {
		// TODO: fishing rod + boat
	}
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
	} else if (_scene->_priorSceneId != -2 && _scene->_priorSceneId != 620) {
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
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && _action._mainObjectSource == 4 && (!_game._objects.isInInventory(OBJ_BONES) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
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
	else if (_action.isAction(VERB_LOOK, NOUN_BONES) && _action._mainObjectSource == 4)
		_vm->_dialogs->show(70217);
	else if (_action.isAction(VERB_TAKE, NOUN_BONES) && _action._mainObjectSource == 4) {
		if (_game._objects.isInInventory(OBJ_BONES))
			_vm->_dialogs->show(70219);
	} else if (_action.isAction(VERB_LOOK, NOUN_SUBMERGED_CITY))
		_vm->_dialogs->show(70220);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

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
		_scene->_hotspots.activate(0x468, false);

	if (_scene->_priorSceneId == 704) {
		_game._player._stepEnabled = false;
		_curSequence = 2;
		_boatDir = 2;
		_monsterMode = 0;
		_scene->loadAnimation(formAnimName('A', -1), 0);
		_scene->_activeAnimation->setCurrentFrame(34);
	} else if (_scene->_priorSceneId != -2) {
		_game._player._stepEnabled = false;
		_boatDir = 1;
		if (_globals[kMonsterAlive]) {
			_monsterMode = 1;
			_curSequence = 0;
			_scene->loadAnimation(formAnimName('B', -1), 0);
		} else {
			_curSequence = 0;
			_monsterMode = 0;
			_scene->loadAnimation(formAnimName('A', -1), 0);
		}
	} else if (_globals[kMonsterAlive]) {
		_curSequence = 0;
		_boatDir = 1;
		_monsterMode = 1;
		_scene->loadAnimation(formAnimName('B', -1), 0);
		_scene->_activeAnimation->setCurrentFrame(39);
	} else if (_boatDir == 1) {
		_curSequence = 0;
		_monsterMode = 0;
		_scene->loadAnimation(formAnimName('A', -1), 0);
		_scene->_activeAnimation->setCurrentFrame(9);
	} else if (_boatDir == 2) {
		_curSequence = 0;
		_monsterMode = 0;
		_scene->loadAnimation(formAnimName('A', -1), 0);
		_scene->_activeAnimation->setCurrentFrame(56);
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
		_scene->loadAnimation(formAnimName('D', -1), 0);
		_rexDeathFl = false;
		_monsterTime = 0;
	}


	if (_game._trigger == 70)
		_scene->_reloadSceneFlag = true;

	if ((_monsterMode == 3) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _boatFrame) {
			_boatFrame = _scene->_activeAnimation->getCurrentFrame();
			int nextBoatFrame = -1;

			if (_boatFrame == 62) {
				nextBoatFrame = 61;
				if (_restartTrigger70Fl) {
					_restartTrigger70Fl = false;
					_scene->_sequences.addTimer(15, 70);
				}
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextBoatFrame);
				_boatFrame = nextBoatFrame;
			}
		}
	}

	if (_game._trigger == 70)
		_scene->_reloadSceneFlag = true;

	if ((_monsterMode == 0) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _boatFrame) {
			_boatFrame = _scene->_activeAnimation->getCurrentFrame();
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

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextBoatFrame);
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


	if ((_monsterMode == 1) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _boatFrame) {
			_boatFrame = _scene->_activeAnimation->getCurrentFrame();
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

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextBoatFrame);
				_boatFrame = nextBoatFrame;
			}
		}
	}

	if ((_monsterMode == 2) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _boatFrame) {
			_boatFrame = _scene->_activeAnimation->getCurrentFrame();
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
				_scene->_activeAnimation->setCurrentFrame(39);
				_game._player._stepEnabled = true;
				break;

			case 91:
				if (!_useBomb) {
					_scene->freeAnimation();
					_monsterMode = 1;
					_scene->loadAnimation(formAnimName('B', -1));
					_scene->_activeAnimation->setCurrentFrame(39);
					_game._player._stepEnabled = true;
				} else
					_game._objects.setRoom(OBJ_CHICKEN_BOMB, 1);

				break;

			case 126:
				_scene->_hotspots.activate(0x468, false);
				_globals[kMonsterAlive] = false;
				_scene->freeAnimation();
				_monsterMode = 0;
				_scene->loadAnimation(formAnimName('A', -1));
				_scene->_activeAnimation->setCurrentFrame(9);
				_game._player._stepEnabled = true;
				if (_game._storyMode == STORYMODE_NAUGHTY)
					_vm->_dialogs->show(70321);
				else
					_vm->_dialogs->show(70322);

				break;

			default:
				break;
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextBoatFrame);
				_boatFrame = nextBoatFrame;
			}
		}
	}
}

void Scene703::actions() {
	if (_game._screenObjects._inputMode == 1)
		handleFillBottle(_action._activeAction._verbId);
	else if (_action.isAction(0x3B1, 0x3B0)) {
		_game._player._stepEnabled = false;
		if (_globals[kMonsterAlive])
			_curSequence = 8;
		else if (_boatDir == 1)
			_curSequence = 5;
		else
			_curSequence = 3;
	} else if (_action.isAction(0x3B1, 0x3B2)) {
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
	} else if (_action.isAction(VERB_THROW, 0x2C, 0x468) || _action.isAction(VERB_THROW, 0x2D, 0x468)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
		_scene->_activeAnimation->setCurrentFrame(19);
	} else if (_action.isAction(VERB_THROW, 0x49, 0x468)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
	} else if (_action.isAction(VERB_THROW, 0x17A, 0x468)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
		_scene->_activeAnimation->setCurrentFrame(39);
	} else if (_action.isAction(VERB_THROW, 0x2A, 0x468)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
		_scene->_activeAnimation->setCurrentFrame(59);
	} else if (_action.isAction(VERB_THROW, 0x4A, 0x468)) {
		_useBomb = true;
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		_monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
	} else if (_action.isAction(VERB_PUT, 0x2E, 0x18F) || _action.isAction(0x85, 0x2E, 0x18F)) {
		if (_globals[kBottleStatus] != 4) {
			handleBottleInterface();
			_dialog1.start();
		} else
			_vm->_dialogs->show(70323);
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, 0x468)) {
		if (_globals[kMonsterAlive])
			_vm->_dialogs->show(70310);
	} else if (_action.isAction(VERB_LOOK, 0x18F)) {
		if (!_globals[kMonsterAlive])
			_vm->_dialogs->show(70311);
		else
			_vm->_dialogs->show(70312);
	} else if (_action.isAction(VERB_LOOK, 0x3B2)) {
		if (_globals[kMonsterAlive])
			_vm->_dialogs->show(70313);
		else if (_game._visitedScenes.exists(710))
			_vm->_dialogs->show(70314);
		else
			_vm->_dialogs->show(70315);
	} else if (_action.isAction(VERB_LOOK, 0x3B3))
		_vm->_dialogs->show(70316);
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

	if (_action.isAction(VERB_LOOK, 0x181) || _action.isAction(0x103, 0x181))
		_vm->_dialogs->show(70710);
	else if (_action.isAction(VERB_LOOK, 0xC4))
		_vm->_dialogs->show(70711);
	else if (_action.isAction(VERB_LOOK, 0x1CC))
		_vm->_dialogs->show(70712);
	else if (_action.isAction(VERB_LOOK, 0x1D0) || _action.isAction(VERB_LOOK, 0x1D1)
	 || _action.isAction(VERB_LOOK, 0x1D2) || _action.isAction(VERB_LOOK, 0x1D3)
	 || _action.isAction(VERB_LOOK, 0x1D4) || _action.isAction(VERB_LOOK, 0x1D5)
	 || _action.isAction(VERB_LOOK, 0x1D6) || _action.isAction(VERB_LOOK, 0x1D7)
	 || _action.isAction(VERB_LOOK, 0x1D8) || _action.isAction(VERB_LOOK, 0x1D9)
	 || _action.isAction(VERB_LOOK, 0x1DB) || _action.isAction(VERB_LOOK, 0x7A)
	 || _action.isAction(VERB_LOOK, 0x1DA))
		_vm->_dialogs->show(70713);
	else if (_action.isAction(VERB_LOOK, 0x1CF) || _action._lookFlag)
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
		_globals._sequenceIndexes[1] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
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
	if (_action.isAction(0x46F, 0x27)) {
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
	} else if (_scene->_priorSceneId != -2) {
		_game._player._playerPos = Common::Point(289, 138);
		_game._player.walk(Common::Point(262, 148), FACING_WEST);
		_game._player._facing = FACING_WEST;
		_game._player._visible = true;
	}

	if (_game._objects[OBJ_ID_CARD]._roomNumber == 752) {
		_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('i', -1));
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		int idx = _scene->_dynamicHotspots.add(0xB3, VERB_WALKTO, _globals._sequenceIndexes[13], Common::Rect(0, 0, 0, 0));
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
			_globals._sequenceIndexes[12] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			_vm->_sound->command(0xF);
			_scene->_sequences.remove(_globals._sequenceIndexes[13]);
			_game._objects.addToInventory(OBJ_ID_CARD);
			_scene->_dynamicHotspots.remove(_cardId);
			_vm->_dialogs->show(OBJ_ID_CARD, 830);
			break;
		case 2:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && _action._mainObjectSource == 4 && (!_game._objects.isInInventory(OBJ_BONES) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			_vm->_sound->command(0xF);
			if (_game._objects.isInInventory(OBJ_BONE))
				_game._objects.setRoom(OBJ_BONE, 1);
			_game._objects.addToInventory(OBJ_BONES);
			_vm->_dialogs->show(OBJ_BONES, 75221);
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
	else if ((_action.isAction(VERB_LOOK, NOUN_BONES) || _action.isAction(VERB_LOOK, NOUN_ID_CARD)) && _action._mainObjectSource == 4) {
		if (_game._objects[OBJ_ID_CARD]._roomNumber == 752)
			_vm->_dialogs->show(75219);
		else
			_vm->_dialogs->show(75220);
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && _action._mainObjectSource == 4) {
		if (_game._objects.isInInventory(OBJ_BONES))
			_vm->_dialogs->show(75222);
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
