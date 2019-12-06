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
#include "mads/nebular/nebular_scenes8.h"

namespace MADS {

namespace Nebular {

void Scene8xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	if ((_globals[kFromCockpit] && !_globals[kExitShip]) ||
			_scene->_nextSceneId == 804 || _scene->_nextSceneId == 805 ||
			_scene->_nextSceneId == 808 || _scene->_nextSceneId == 810) {
		_game._player._spritesPrefix = "";
	} else
		_game._player._spritesPrefix = _globals[kSexOfRex] == SEX_FEMALE ? "ROX" : "RXM";

	_vm->_palette->setEntry(16, 0x0A, 0x3F, 0x3F);
	_vm->_palette->setEntry(17, 0x0A, 0x2D, 0x2D);
}

void Scene8xx::setAAName() {
	_game._aaName = Resources::formatAAName(5);
}

void Scene8xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else {
		switch (_scene->_nextSceneId) {
		case 801:
		case 802:
		case 803:
		case 804:
		case 806:
		case 807:
		case 808:
			_vm->_sound->command(20);
			break;

		case 805:
			_vm->_sound->command(23);
			break;

		case 810:
			_vm->_sound->command(10);
			break;

		default:
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

Scene801::Scene801(MADSEngine *vm) : Scene8xx(vm) {
	_walkThroughDoor = false;
}

void Scene801::synchronize(Common::Serializer &s) {
	Scene8xx::synchronize(s);

	s.syncAsByte(_walkThroughDoor);
}

void Scene801::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene801::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('a', -1));

	if (_scene->_priorSceneId != 802) {
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
	}

	if ((_globals[kCameFromCut]) && (_globals[kCutX] != 0)) {
		_game._player._playerPos = Common::Point(_globals[kCutX], _globals[kCutY]);
		_game._player._facing = (Facing)_globals[kCutFacing];
		_globals[kCutX] = 0;
		_globals[kCameFromCut] = false;
		_globals[kReturnFromCut] = false;
		_globals[kBeamIsUp] = false;
		_globals[kForceBeamDown] = false;
		_globals[kDontRepeat] = false;
	} else if (_scene->_priorSceneId == 808) {
		_game._player._playerPos = Common::Point(148, 110);
		_game._player._facing = FACING_NORTH;
	} else if (_scene->_priorSceneId == 802) {
		_game._player._playerPos = Common::Point(307, 111);
		_game._player.walk(Common::Point(270, 118), FACING_WEST);
		_game._player._visible = true;
	} else if ((_scene->_priorSceneId != RETURNING_FROM_DIALOG) && !_globals[kTeleporterCommand]) {
		_game._player._playerPos = Common::Point(8, 117);
		_game._player.walk(Common::Point(41, 115), FACING_EAST);
		_game._player._visible = true;
	}

	_globals[kBetweenRooms] = false;

	if (_globals[kTeleporterCommand]) {
		_game._player._stepEnabled = false;
		switch (_globals[kTeleporterCommand]) {
		case 1:
			_game._player._playerPos = Common::Point(8, 117);
			_globals[kTeleporterUnderstood] = true;
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 13);
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			_vm->_sound->command(30);
			break;

		case 2:
			_game._player._playerPos = Common::Point(8, 117);
			_globals[kTeleporterUnderstood] = true;
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 13);
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			_vm->_sound->command(30);
			break;

		case 3:
		case 4:
			_game._player._playerPos = Common::Point(8, 117);
			_game._player.walk(Common::Point(41, 115), FACING_EAST);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_globals[kTeleporterCommand] = 0;
	}

	_walkThroughDoor = false;
	if (_scene->_priorSceneId == 802) {
		_game._player._stepEnabled = false;
		_walkThroughDoor = true;
	}

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 11, 0, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 9, 0, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

	sceneEntrySound();
}

void Scene801::step() {
	if (_game._trigger == 75) {
		if (_globals[kSexOfRex] == REX_FEMALE) {
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 140);
		} else {
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			_game._player._playerPos = Common::Point(8, 117);
			_game._player.walk(Common::Point(41, 115), FACING_EAST);
		}
	}

	if (_game._trigger == 140) {
		_vm->_sound->command(27);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 8);
		_scene->_sequences.addTimer(100, 141);
	}

	if (_game._trigger == 141) {
		_scene->_reloadSceneFlag = true;
		_scene->_nextSceneId = _scene->_priorSceneId;
		_globals[kTeleporterCommand] = 0;
	}

	if (_game._trigger == 80) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}

	if (_walkThroughDoor && (_game._player._playerPos == Common::Point(270, 118))) {
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_walkThroughDoor = false;
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 120);
	}

	if (_game._trigger == 120) {
		_vm->_sound->command(12);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 90) {
		_game._player.walk(Common::Point(307, 111), FACING_EAST);
		_scene->_sequences.addTimer(80, 130);
	}

	if (_game._trigger == 130) {
		_vm->_sound->command(12);
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 110);
	}

	if (_game._trigger == 110) {
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_scene->_nextSceneId = 802;
	}
}

void Scene801::preActions() {
	if (_action.isAction(VERB_LOOK, NOUN_CONTROL_PANEL)) {
		_game._player.walk(Common::Point(148, 110), FACING_NORTH);
		_game._player._needToWalk = true;
		_game._player._readyToWalk = true;
	}

	if (_action.isAction(VERB_WALK_INSIDE, NOUN_TELEPORTER) && _globals[kBeamIsUp]) {
		_globals[kCutX] = _game._player._playerPos.x;
		_globals[kCutY] = _game._player._playerPos.y;
		_globals[kCutFacing] = _game._player._facing;
		_globals[kForceBeamDown] = true;
		_globals[kDontRepeat] = true;
		_scene->_nextSceneId = 803;
	}
}

void Scene801::actions() {
	if (_action.isAction(VERB_LOOK, NOUN_CONTROL_PANEL))
		_scene->_nextSceneId = 808;
	else if (_action.isAction(VERB_WALK_INSIDE, NOUN_TELEPORTER)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 807;
	} else if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) && (_game._player._playerPos == Common::Point(270, 118))) {
		_game._player._stepEnabled = false;
		_game._player._facing = FACING_EAST;
		_game._player.selectSeries();
		_globals[kBetweenRooms] = true;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
		_vm->_sound->command(11);
	} else if (_action.isAction(VERB_LOOK, NOUN_CEILING))
		_vm->_dialogs->show(80110);
	else if (_action.isAction(VERB_LOOK, NOUN_MONITOR))
		_vm->_dialogs->show(80111);
	else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER))
		_vm->_dialogs->show(80112);
	else if (_action.isAction(VERB_LOOK, NOUN_EQUIPMENT) || _action._lookFlag)
		_vm->_dialogs->show(80113);
	else if (_action.isAction(VERB_LOOK, NOUN_SPEAKER))
		_vm->_dialogs->show(80114);
	else if (_action.isAction(VERB_LOOK, NOUN_EYE_CHART))
		_vm->_dialogs->show(80115);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(80116);
	else if (_action.isAction(VERB_LOOK, NOUN_DOOR))
		_vm->_dialogs->show(80117);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene802::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_SHIELD_MODULATOR);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_REMOTE);
}

void Scene802::enter() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRC_8");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXMBD_8");
	_globals[kBetweenRooms] = false;

	if ((_globals[kCameFromCut]) && (_globals[kCutX] != 0)) {
		_game._player._playerPos.x = _globals[kCutX];
		_game._player._playerPos.y = _globals[kCutY];
		_game._player._facing = (Facing)_globals[kCutFacing];
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
		int idx = _scene->_dynamicHotspots.add(NOUN_REMOTE, VERB_WALKTO,_globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(107, 99), FACING_NORTH);
	}

	if (!_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled]) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
		int idx = _scene->_dynamicHotspots.add(NOUN_SHIELD_MODULATOR, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(93, 97), FACING_NORTH);
	}
	sceneEntrySound();
}

void Scene802::step() {
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
		int idx = _scene->_dynamicHotspots.add(NOUN_REMOTE, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(107, 99), FACING_NORTH);

		_globals[kRemoteSequenceRan] = true;
		_globals[kRemoteOnGround] = true;
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 72)
		_vm->_sound->command(13);
}

void Scene802::preActions() {
	if (_action.isAction(VERB_WALK_TOWARDS, NOUN_BUILDING_TO_WEST))
		_game._player._walkOffScreenSceneId = 801;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_PATH_TO_EAST)) {
		_game._player._walkOffScreenSceneId = 803;
		_globals[kForceBeamDown] = false;
	}

	if (_action.isAction(VERB_TAKE, NOUN_SHIP))
		_game._player._needToWalk = false;
}

void Scene802::actions() {
	if (_action.isAction(VERB_TAKE, NOUN_SHIELD_MODULATOR) && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) {
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
	} else if ((_action.isAction(VERB_TAKE, NOUN_REMOTE)) && (!_game._objects.isInInventory(OBJ_REMOTE))) {
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
		&& (_action.isAction(VERB_LOOK, NOUN_LAUNCH_PAD) || _action._lookFlag))
		_vm->_dialogs->show(80210);
	else if (!_globals[kRemoteOnGround]&& !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled]
		&& (_action.isAction(VERB_LOOK, NOUN_LAUNCH_PAD) || _action._lookFlag))
		_vm->_dialogs->show(80211);
	else if (_globals[kRemoteOnGround] && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled]
		&& (_action.isAction(VERB_LOOK, NOUN_LAUNCH_PAD) || _action._lookFlag))
		_vm->_dialogs->show(80213);
	else if (_globals[kRemoteOnGround] && (_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) || _globals[kShieldModInstalled])
		&& (_action.isAction(VERB_LOOK, NOUN_LAUNCH_PAD) || _action._lookFlag))
		_vm->_dialogs->show(80212);
	else if (!_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled] && _action.isAction(VERB_LOOK, NOUN_SHIELD_MODULATOR))
		_vm->_dialogs->show(80214);
	else if (_globals[kRemoteOnGround] && _action.isAction(VERB_LOOK, NOUN_REMOTE))
		_vm->_dialogs->show(80216);
	else if (_action.isAction(VERB_LOOK, NOUN_SHIP)) {
		if ((!_game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) && (!_globals[kShieldModInstalled]))
			_vm->_dialogs->show(80218);
		else
			_vm->_dialogs->show(80217);
	} else if (_action.isAction(VERB_LOOK, NOUN_BUSHES))
		_vm->_dialogs->show(80219);
	else if (_action.isAction(VERB_LOOK, NOUN_PATH_TO_EAST))
		_vm->_dialogs->show(80220);
	else if (_action.isAction(VERB_LOOK, NOUN_SKY))
		_vm->_dialogs->show(80221);
	else if (_action.isAction(VERB_TAKE, NOUN_SHIP))
		_vm->_dialogs->show(80222);
	else if (_action.isAction(VERB_LOOK, NOUN_TREE) || _action.isAction(VERB_LOOK, NOUN_TREES))
		_vm->_dialogs->show(80224);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING_TO_WEST))
		_vm->_dialogs->show(80225);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene803::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_GUTS);
	_scene->addActiveVocab(VERB_WALKTO);

	if ((!_globals[kFromCockpit] && _globals[kReturnFromCut] && !_globals[kBeamIsUp])
		|| (_globals[kFromCockpit] && !_globals[kExitShip])) {
		_game._player._spritesPrefix = "";
		_game._player._spritesChanged = true;
	}
}

void Scene803::enter() {
	_globals[kBetweenRooms] = false;
	_game._player._visible = false;
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites("*RXMBD_2");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('d', 1));

	_game.loadQuoteSet(0x31B, 0x31C, 0x31D, 0x31E, 0x31F, 0x320, 0x321, 0x322, 0);

	if (_globals[kHoppyDead]) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('e', 1));
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
		int idx = _scene->_dynamicHotspots.add(NOUN_GUTS, VERB_WALKTO, _globals._sequenceIndexes[7], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(66, 123), FACING_SOUTH);
	}

	if (!_globals[kBeamIsUp] && !_globals[kReturnFromCut] && (!_globals[kFromCockpit] || _globals[kExitShip])) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 2, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
	}

	if (!_globals[kFromCockpit]) {
		if (!_globals[kReturnFromCut]) {
			if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
				_game._player._playerPos = Common::Point(15, 130);
				_game._player._facing = FACING_EAST;
			}
			_game._player._visible = true;
		} else if (!_globals[kBeamIsUp]){
			_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 1));
			_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 3));
			_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 2));
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			_vm->_sound->command(14);
		}

		if (_globals[kBeamIsUp] && !_globals[kReturnFromCut]){
			if (_globals[kForceBeamDown])
				_game._player._visible = false;
			else
				_game._player._visible = true;

			_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('b', 1));
			_vm->_sound->command(15);
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 100);
		}
	} else if (!_globals[kExitShip]) {
		if (!_globals[kBeamIsUp]) {
			_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 1));
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 130);
			_vm->_sound->command(14);
		} else {
			_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('c', 1));
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 1);
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 140);
		}
	} else {
		_game._player._stepEnabled = false;
		_game._player._playerPos = Common::Point(197, 96);
		_game._player._facing = FACING_SOUTHWEST;
		_game._player._visible = true;
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('d', 1));
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 19);
		_scene->_sequences.addTimer(1, 150);
	}

	sceneEntrySound();
}

void Scene803::step() {
	if (_game._trigger == 120) {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 19);
		_scene->_nextSceneId = 804;
	}

	if (_game._trigger == 100) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 2, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		if (!_globals[kHoppyDead]) {
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 7, 12);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5],SEQUENCE_TRIGGER_EXPIRE, 0, 101);
		} else {
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
			int idx = _scene->_dynamicHotspots.add(NOUN_GUTS, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(66, 123), FACING_SOUTH);
			_vm->_sound->command(16);
			_globals[kCameFromCut] = true;
			_globals[kBeamIsUp] = false;
			_globals[kReturnFromCut] = false;
			_globals[kDontRepeat] = false;
			_globals[kHoppyDead] = true;
			_globals[kHasWatchedAntigrav] = true;

			if (_globals[kForceBeamDown])
				_scene->_nextSceneId = _scene->_priorSceneId;
			else
				_game._player._stepEnabled = true;
		}
	}

	if (_game._trigger == 101) {
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
		int idx = _scene->_dynamicHotspots.add(NOUN_GUTS, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(66, 123), FACING_SOUTH);
		_vm->_sound->command(16);
		_globals[kCameFromCut] = true;
		_globals[kBeamIsUp] = false;
		_globals[kReturnFromCut] = false;
		_globals[kDontRepeat] = false;
		_globals[kHoppyDead] = true;
		_globals[kHasWatchedAntigrav] = true;

		if (_globals[kForceBeamDown])
			_scene->_nextSceneId = _scene->_priorSceneId;
		else
			_game._player._stepEnabled = true;
	}

	if (_game._trigger == 80) {
		if (!_globals[kHoppyDead])
			_scene->_sequences.addTimer(350, 70);

		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
	}

	if (_game._trigger == 70) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		_vm->_sound->command(31);
	}

	if (_game._trigger == 71)
		_scene->_sequences.addTimer(200, 110);

	if (_game._trigger == 90) {
		int syncIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 15, 0, 0, 0);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 4, 9);
		if (_globals[kHoppyDead])
			_scene->_sequences.addTimer(200, 110);
	}

	if (_game._trigger == 110)
		_scene->_nextSceneId = 808;

	if (_game._trigger == 130) {
		_globals[kBeamIsUp] = true;
		_scene->_nextSceneId = 804;
	}

	if (_game._trigger == 140) {
		if (!_globals[kWindowFixed]) {
			_scene->_nextSceneId = 810;
			_globals[kInSpace] = true;
		} else {
			if (!_globals[kShieldModInstalled])
				_game._winStatus = 1;
			else if (!_globals[kTargetModInstalled])
				_game._winStatus = 2;
			else
				_game._winStatus = 3;

			return;
		}
	}

	if (_game._trigger == 150) {
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_vm->_sound->command(18);
		_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 19);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 151);
	}

	if (_game._trigger == 151) {
		_globals[kBeamIsUp] = false;
		_globals[kFromCockpit] = false;
		_globals[kExitShip] = false;
		_game._player._stepEnabled = true;
	}
}

void Scene803::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_PATH_TO_WEST))
		_game._player._walkOffScreenSceneId = 802;

	if (_action.isAction(VERB_TAKE, NOUN_SHIP))
		_game._player._needToWalk = false;
}

void Scene803::actions() {
	if (_action.isAction(VERB_TAKE, NOUN_GUTS)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], true, 6, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 1, 4);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[9]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 160);
			break;

		case 160: {
			int syncIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 4);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], syncIdx);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[9]);
			_scene->_sequences.addTimer(60, 161);
			}
			break;

		case 161: {
			int quoteId = 0x31A + _vm->getRandomNumber(1, 8);
			_scene->_kernelMessages.add(Common::Point(64, 67), 0x1110, 32, 0, 80, _game.getQuote(quoteId));
			_scene->_sequences.addTimer(60, 162);
			}
			break;

		case 162:
			_scene->_sequences.remove(_globals._sequenceIndexes[9]);
			_globals._sequenceIndexes[9] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], true, 6, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 1, 4);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[9]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 163);
			break;

		case 163:
			_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_ENTER, NOUN_SHIP)) {
		_vm->_sound->command(17);
		_game._player._stepEnabled = false;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 19);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 120);
		_globals[kBeamIsUp] = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_LAUNCH_PAD))
		_vm->_dialogs->show(80310);
	else if (_action._lookFlag)
		_vm->_dialogs->show(80310);
	else if (_action.isAction(VERB_LOOK, NOUN_PAD_TO_WEST))
		_vm->_dialogs->show(80311);
	else if (_action.isAction(VERB_LOOK, NOUN_GUTS)) {
		if (_game._storyMode == STORYMODE_NICE)
			_vm->_dialogs->show(80312);
		else
			_vm->_dialogs->show(80313);
	} else if (_action.isAction(VERB_LOOK, NOUN_BUSHES))
		_vm->_dialogs->show(80315);
	else if (_action.isAction(VERB_LOOK, NOUN_SHIP))
		_vm->_dialogs->show(80317);
	else if (_action.isAction(VERB_LOOK, NOUN_TOWER))
		_vm->_dialogs->show(80318);
	else if (_action.isAction(VERB_LOOK, NOUN_TREE) || _action.isAction(VERB_LOOK, NOUN_TREES))
		_vm->_dialogs->show(80319);
	else if (_action.isAction(VERB_LOOK, NOUN_SKY))
		_vm->_dialogs->show(80320);
	else if (_action.isAction(VERB_TAKE, NOUN_SHIP))
		_vm->_dialogs->show(80321);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene804::Scene804(MADSEngine *vm) : Scene8xx(vm) {
	_messWithThrottle = false;
	_movingThrottle = false;
	_throttleGone = false;
	_dontPullThrottleAgain = false;
	_pullThrottleReally = false;
	_alreadyOrgan = false;
	_alreadyPop = false;

	_throttleCounter = 0;
	_resetFrame = -1;
}

void Scene804::synchronize(Common::Serializer &s) {
	Scene8xx::synchronize(s);

	s.syncAsByte(_messWithThrottle);
	s.syncAsByte(_movingThrottle);
	s.syncAsByte(_throttleGone);
	s.syncAsByte(_dontPullThrottleAgain);
	s.syncAsByte(_pullThrottleReally);
	s.syncAsByte(_alreadyOrgan);
	s.syncAsByte(_alreadyPop);

	s.syncAsSint16LE(_resetFrame);
	s.syncAsUint32LE(_throttleCounter);
}

void Scene804::setup() {
	Scene8xx::setPlayerSpritesPrefix();
	Scene8xx::setAAName();
}

void Scene804::enter() {
	_messWithThrottle = false;
	_throttleCounter = 0;
	_movingThrottle = false;
	_throttleGone = false;
	_dontPullThrottleAgain = false;
	_resetFrame = -1;
	_pullThrottleReally = false;
	_alreadyOrgan = false;
	_alreadyPop = false;


	if (_globals[kCopyProtectFailed]) {
		// Copy protection failed
		_globals[kInSpace] = true;
		_globals[kWindowFixed] = 0;
	}

	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('f', 1));

	_game.loadQuoteSet(791, 0);

	if (_globals[kInSpace]) {
		if (_globals[kWindowFixed]) {
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], 0, 1);
			_scene->_sequences.addTimer(60, 100);
		} else {
			_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
			_globals._sequenceIndexes[7] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[7], false, 4, 0, 0, 0);
			_scene->_sequences.addTimer(160, 70);
			_game._player._stepEnabled = false;
		}
	} else {
		if (_globals[kBeamIsUp]) {
			_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 7);
		}

		if (_globals[kWindowFixed])
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);

		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(133, 139));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
	}

	_scene->loadAnimation(Resources::formatName(804, 'r', 1, EXT_AA, ""));

	Scene8xx::sceneEntrySound();

	if (_globals[kInSpace] && !_globals[kWindowFixed]) {
		_scene->_userInterface.setup(kInputLimitedSentences);
		_vm->_sound->command(19);
	}
}

void Scene804::step() {
	if (!_messWithThrottle) {

		if ((_throttleGone) && (_movingThrottle) && (_scene->_animation[0]->getCurrentFrame() == 39)) {
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle
				(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(133, 139));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
			_throttleGone = false;
		}

		if ((_movingThrottle) && (_scene->_animation[0]->getCurrentFrame() == 42)) {
			_resetFrame = 0;
			_movingThrottle = false;
		}

		if (_game._trigger == 70) {
			_resetFrame = 42;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 65)
			_scene->_sequences.remove(_globals._sequenceIndexes[7]);

		switch (_game._storyMode) {
		case STORYMODE_NAUGHTY:
		default:
			if (_scene->_animation[0]->getCurrentFrame() == 81) {
				_resetFrame = 80;
				_globals[kInSpace] = false;
				_globals[kBeamIsUp] = true;

				//assert(!_globals[kCopyProtectFailed]);
				_game._winStatus = 4;
				return;
			}
			break;

		case STORYMODE_NICE:
			if (_scene->_animation[0]->getCurrentFrame() == 68) {
				_resetFrame = 66;
				_globals[kInSpace] = false;
				_globals[kBeamIsUp] = true;

				assert(!_globals[kCopyProtectFailed]);
				_game._winStatus = 4;
				return;
			}
		}

		if (_scene->_animation[0]->getCurrentFrame() == 34) {
			_resetFrame = 36;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		}

		if (_scene->_animation[0]->getCurrentFrame() == 37) {
			_resetFrame = 36;
			if (!_dontPullThrottleAgain) {
				_dontPullThrottleAgain = true;
				_scene->_sequences.addTimer(60, 80);
			}
		}

		if (_game._trigger == 80) {
			_scene->_nextSceneId = 803;
		}

		if ((_scene->_animation[0]->getCurrentFrame() == 7) && (!_globals[kWindowFixed])) {
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_scene->_sequences.addTimer(20, 110);
			_globals[kWindowFixed] = true;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 10) {
			_resetFrame = 0;
			_game._player._stepEnabled = true;
			_game._objects.setRoom(OBJ_POLYCEMENT, NOWHERE);
		}

		// FIXME: Original doesn't have resetFrame check. Check why this has been needed
		if (_resetFrame == -1 &&  _scene->_animation[0]->getCurrentFrame() == 1) {
			int randomVal = _vm->getRandomNumber(29) + 1;
			switch (randomVal) {
			case 1:
				_resetFrame = 25;
				break;
			case 2:
				_resetFrame = 27;
				break;
			case 3:
				_resetFrame = 29;
				break;
			default:
				_resetFrame = 0;
				break;
			}
		}

		switch (_scene->_animation[0]->getCurrentFrame()) {
		case 26:
		case 28:
		case 31:
			_resetFrame = 0;
			break;
		default:
			break;
		}
	} else {
		if ((_scene->_animation[0]->getCurrentFrame() == 36) && (!_throttleGone)) {
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_throttleGone = true;
		}

		if (_scene->_animation[0]->getCurrentFrame() == 39) {
			_movingThrottle = false;
			switch (_throttleCounter) {
			case 1:
				break;
			case 3:
				_scene->_sequences.addTimer(130, 120);
				break;
			default:
				break;
			}
		}

		if (!_movingThrottle) {
			++_throttleCounter;
			_movingThrottle = true;
			if (_throttleCounter < 4) {
				_resetFrame = 34;
			} else {
				_messWithThrottle = false;
				_throttleCounter = 0;
				_game._player._stepEnabled = true;
			}
		}
	}

	if (_game._trigger == 120) {
		_vm->_dialogs->show(80422);
	}

	if (_game._trigger == 110) {
		_vm->_dialogs->show(80426);
	}

	if (_pullThrottleReally) {
		_resetFrame = 32;
		_pullThrottleReally = false;
	}

	if (_resetFrame >= 0) {
		if (_resetFrame != _scene->_animation[0]->getCurrentFrame()) {
			_scene->_animation[0]->setCurrentFrame(_resetFrame);
			_resetFrame = -1;
		}
	}

	if (_game._trigger == 90) {
		_scene->_nextSceneId = 803;
	}

	if ((_scene->_animation[0]->getCurrentFrame() == 72) && !_alreadyPop) {
		_vm->_sound->command(21);
		_alreadyPop = true;
	}

	if ((_scene->_animation[0]->getCurrentFrame() == 80) && !_alreadyOrgan) {
		_vm->_sound->command(22);
		_alreadyOrgan = true;
	}
}

void Scene804::actions() {
	if (_action.isAction(VERB_LOOK, NOUN_SERVICE_PANEL) ||
		_action.isAction(VERB_OPEN, NOUN_SERVICE_PANEL)) {
		_scene->_nextSceneId = 805;
	} else if ((_action.isAction(VERB_ACTIVATE, NOUN_REMOTE)) && _globals[kTopButtonPushed]) {
		if (!_globals[kInSpace]) {
			// Top button pressed on panel in hanger control
			if (!_globals[kBeamIsUp]) {
				_globals[kFromCockpit] = true;
				_globals[kUpBecauseOfRemote] = true;
				_scene->_nextSceneId = 803;
			} else {
				// Player turning off remote
				_globals[kBeamIsUp] = false;
				_globals[kUpBecauseOfRemote] = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[8]);
				_vm->_sound->command(15);
			}
		}
	} else if (_action.isAction(VERB_PULL, NOUN_THROTTLE)) {
		_game._player._stepEnabled = false;
		if (_globals[kBeamIsUp]) {
			if (!_game._objects.isInInventory(OBJ_VASE) && _globals[kWindowFixed]) {
				_vm->_dialogs->show(80423);
				_game._player._stepEnabled = true;
			} else {
				_action._inProgress = false;

				_vm->_dialogs->show(80424);
				_pullThrottleReally = true;
				_scene->_kernelMessages.add(Common::Point(78, 75), 0x1110, 0, 0,
					120, _game.getQuote(791));
			}
		} else {
			_messWithThrottle = true;
		}
	} else if (_action.isAction(VERB_APPLY, NOUN_POLYCEMENT, NOUN_CRACK) ||
		_action.isAction(VERB_PUT, NOUN_POLYCEMENT, NOUN_CRACK)) {
		if (!_globals[kWindowFixed]) {
			_resetFrame = 2;
			_game._player._stepEnabled = false;
		}
	} else if (_action.isAction(VERB_EXIT, NOUN_SHIP)) {
		_globals[kExitShip] = true;
		_globals[kFromCockpit] = true;
		if (_globals[kBeamIsUp]) {
			_vm->_dialogs->show(80425);
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			_vm->_sound->command(15);
			_globals[kBeamIsUp] = false;
		}
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(2, 90);
	} else  if (_action._lookFlag) {
		_vm->_dialogs->show(80410);
	} else if ((_action.isAction(VERB_LOOK, NOUN_WINDOW)) ||
			(_action.isAction(VERB_LOOK_OUT, NOUN_WINDOW))) {
		if (_globals[kBeamIsUp]) {
			_vm->_dialogs->show(80412);
		} else {
			_vm->_dialogs->show(80411);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_CRACK)) {
		if (_globals[kWindowFixed]) {
			_vm->_dialogs->show(80414);
		} else {
			_vm->_dialogs->show(80413);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_CONTROLS)) {
		_vm->_dialogs->show(80415);
	} else if (_action.isAction(VERB_LOOK, NOUN_STATUS_PANEL)) {
		if (_globals[kBeamIsUp]) {
			_vm->_dialogs->show(80417);
		} else {
			_vm->_dialogs->show(80416);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_TP)) {
		_vm->_dialogs->show(80418);
	} else if (_action.isAction(VERB_TAKE, NOUN_TP)) {
		_vm->_dialogs->show(80419);
	} else if (_action.isAction(VERB_LOOK, NOUN_INSTRUMENTATION)) {
		_vm->_dialogs->show(80420);
	} else  if (_action.isAction(VERB_LOOK, NOUN_SEAT)) {
		_vm->_dialogs->show(80421);
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene805::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(VERB_REMOVE);
	_scene->addActiveVocab(NOUN_TARGET_MODULE);
	_scene->addActiveVocab(NOUN_SHIELD_MODULATOR);
}

void Scene805::enter() {
	_game._player._visible = false;
	_scene->_userInterface.setup(kInputLimitedSentences);

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 2));

	if (_globals[kShieldModInstalled]) {
		_scene->_hotspots.activate(OBJ_SHIELD_MODULATOR, false);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 25);
		int idx = _scene->_dynamicHotspots.add(NOUN_SHIELD_MODULATOR, VERB_REMOVE, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_DUMMY);
	}

	if (_globals[kTargetModInstalled]) {
		_scene->_hotspots.activate(OBJ_TARGET_MODULE, false);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 12);
		int idx = _scene->_dynamicHotspots.add(NOUN_TARGET_MODULE, VERB_REMOVE, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_DUMMY);
	}

	sceneEntrySound();
}

void Scene805::step() {
	UserInterface &userInterface = _vm->_game->_scene._userInterface;

	if (_game._trigger == 70) {
		_scene->_hotspots.activate(OBJ_SHIELD_MODULATOR, false);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 25);
		int idx = _scene->_dynamicHotspots.add(NOUN_SHIELD_MODULATOR, VERB_REMOVE, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_DUMMY);
		_globals[kShieldModInstalled] = true;
		_game._objects.setRoom(OBJ_SHIELD_MODULATOR, NOWHERE);
		userInterface._selectedInvIndex = -1;
		_game._player._stepEnabled = true;
		_vm->_sound->command(24);
	}

	if (_game._trigger == 80) {
		_scene->_hotspots.activate(OBJ_TARGET_MODULE, false);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 12);
		int idx = _scene->_dynamicHotspots.add(NOUN_TARGET_MODULE, VERB_REMOVE, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_DUMMY);
		_globals[kTargetModInstalled] = true;
		_game._objects.setRoom(OBJ_TARGET_MODULE, NOWHERE);
		userInterface._selectedInvIndex = -1;
		_game._player._stepEnabled = true;
		_vm->_sound->command(24);
	}

	if (_game._trigger == 71) {
		_scene->_hotspots.activate(OBJ_SHIELD_MODULATOR, true);
		_globals[kShieldModInstalled] = false;
		_game._objects.addToInventory(OBJ_SHIELD_MODULATOR);
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 81) {
		_scene->_hotspots.activate(OBJ_TARGET_MODULE, true);
		_globals[kTargetModInstalled] = false;
		_game._objects.addToInventory(OBJ_TARGET_MODULE);
		_game._player._stepEnabled = true;
	}
}

void Scene805::preActions() {
	_game._player._needToWalk = false;
}

void Scene805::actions() {
	if (_action.isAction(VERB_EXIT, NOUN_SERVICE_PANEL))
		_scene->_nextSceneId = 804;
	else if (_action.isAction(VERB_INSTALL, NOUN_SHIELD_MODULATOR) && _game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_INSTALL, NOUN_TARGET_MODULE) && _game._objects.isInInventory(OBJ_TARGET_MODULE)) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_REMOVE, NOUN_SHIELD_MODULATOR) && _globals[kShieldModInstalled]) {
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_REMOVE, NOUN_TARGET_MODULE) && _globals[kTargetModInstalled]) {
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_INSTALL, NOUN_SHIELD_MODULATOR) && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR))
		_vm->_dialogs->show(80511);
	else if (_action.isAction(VERB_INSTALL, NOUN_TARGET_MODULE) && !_game._objects.isInInventory(OBJ_TARGET_MODULE))
		_vm->_dialogs->show(80510);
	else if (_action.isAction(VERB_REMOVE, NOUN_LIFE_SUPPORT_MODULE))
		_vm->_dialogs->show(80512);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene807::setup() {
	_game._player._spritesPrefix = "";
	// The original was calling Scene8xx::setAAName()
	_game._aaName = Resources::formatAAName(5);
}

void Scene807::enter() {
	if (_globals[kSexOfRex] == REX_FEMALE)
		_handSpriteId = _scene->_sprites.addSprites("*ROXHAND");
	else
		_handSpriteId = _scene->_sprites.addSprites("*REXHAND");

	teleporterEnter();

	// The original uses Scene8xx::SceneEntrySound()
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else
		_vm->_sound->command(20);
}

void Scene807::step() {
	teleporterStep();
}

void Scene807::actions() {
	if (teleporterActions()) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_VIEWPORT))
		_vm->_dialogs->show(80710);
	else if (_action.isAction(VERB_PEER_THROUGH, NOUN_VIEWPORT))
		_vm->_dialogs->show(80710);
	else if (_action.isAction(VERB_LOOK, NOUN_KEYPAD) && _action.isAction(VERB_INSPECT, NOUN_KEYPAD))
		_vm->_dialogs->show(80711);
	else if (_action.isAction(VERB_LOOK, NOUN_DISPLAY))
		_vm->_dialogs->show(80712);
	else if (_action.isAction(VERB_LOOK, NOUN_1_KEY) || _action.isAction(VERB_LOOK, NOUN_2_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_3_KEY) || _action.isAction(VERB_LOOK, NOUN_4_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_5_KEY) || _action.isAction(VERB_LOOK, NOUN_6_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_7_KEY) || _action.isAction(VERB_LOOK, NOUN_8_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_9_KEY) || _action.isAction(VERB_LOOK, NOUN_0_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_SMILE_KEY) || _action.isAction(VERB_LOOK, NOUN_FROWN_KEY))
		_vm->_dialogs->show(80713);
	else if (_action.isAction(VERB_LOOK, NOUN_DEVICE) && _action._lookFlag)
		_vm->_dialogs->show(80714);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene808::Scene808(MADSEngine *vm) : Scene8xx(vm) {
	_goingTo803 = false;
}

void Scene808::synchronize(Common::Serializer &s) {
	Scene8xx::synchronize(s);

	s.syncAsByte(_goingTo803);
}

void Scene808::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene808::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);

	_globals._spriteIndexes[4] = _scene->_sprites.addSprites ("*REXHAND");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 2));

	if (_globals[kTopButtonPushed])
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	else
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);

	_goingTo803 = false;

	if (_globals[kCameFromCut] && _globals[kCutX] != 0) {
		_globals[kCutX] = 0;
		_globals[kCameFromCut] = false;
		_globals[kReturnFromCut] = false;
		_globals[kBeamIsUp] = false;
		_globals[kForceBeamDown] = false;
		_globals[kDontRepeat] = false;
	} else if ((_scene->_priorSceneId == 803) && _globals[kReturnFromCut]){
		_globals[kDontRepeat] = false;
		_globals[kBeamIsUp] = true;
		_globals[kAntigravClock] = _scene->_frameStartTime;
		_globals[kAntigravTiming] = _scene->_frameStartTime;
		_globals[kForceBeamDown] = false;
		_globals[kReturnFromCut] = false;
	}

	_globals[kBetweenRooms] = false;

	if (_globals[kBeamIsUp]) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
	}

	sceneEntrySound();
}

void Scene808::actions() {
	if (_action.isAction(VERB_PRESS, NOUN_START_BUTTON_2)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 211));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
			break;

		case 70:
			if (!_globals[kBeamIsUp] && !_globals[kTopButtonPushed]) {
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
				_goingTo803 = true;
				_vm->_sound->command(20);
				_vm->_sound->command(25);
			}
			_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 211));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		case 71:
			_game._player._stepEnabled = true;
			if (_goingTo803 && !_globals[kTopButtonPushed]) {
				_goingTo803 = false;
				_globals[kReturnFromCut] = true;
				_scene->_nextSceneId = 803;
			}
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRESS, NOUN_TIMER_BUTTON_2)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 186));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
			break;
		case 90:
			if (_globals[kTopButtonPushed]) {
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
				_vm->_sound->command(20);
			}
			_globals[kTopButtonPushed] = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 186));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
			break;

		case 91:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRESS, NOUN_REMOTE_BUTTON_2)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 163));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			break;

		case 80:
			if (!_globals[kTopButtonPushed]) {
				_scene->_sequences.remove(_globals._sequenceIndexes[2]);
				_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
				_vm->_sound->command(20);
			 }
			_globals[kTopButtonPushed] = true;
			_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(248, 163));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
			break;

		case 81:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRESS, NOUN_START_BUTTON_1)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 4, 2, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(168, 211));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
			break;

		case 70:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRESS, NOUN_REMOTE_BUTTON_1)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 4, 2, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(172, 163));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			break;

		case 80:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRESS, NOUN_TIMER_BUTTON_1)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 4, 2, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(172, 186));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
			break;

		case 90:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_EXIT, NOUN_PANEL)) {
		_scene->_nextSceneId = 801;
		_globals[kBetweenRooms] = true;
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene810::Scene810(MADSEngine *vm) : Scene8xx(vm) {
	_moveAllowed = false;
}

void Scene810::synchronize(Common::Serializer &s) {
	Scene8xx::synchronize(s);

	s.syncAsByte(_moveAllowed);
}

void Scene810::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene810::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(Resources::formatName(810, 'a', -1, EXT_AA, ""));
	_moveAllowed = true;

	sceneEntrySound();
}

void Scene810::step() {
	if (_scene->_animation[0] && (_scene->_animation[0]->getCurrentFrame() == 200)
			&& _moveAllowed) {
		_scene->_sequences.addTimer(100, 70);
		_moveAllowed = false;
	}

	if (_game._trigger == 70)
		_scene->_nextSceneId = 804;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
