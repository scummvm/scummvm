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
	} else if ((_scene->_priorSceneId != -2) && !_globals[kTeleporterCommand]) {
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
			_globals._sequenceIndexes[1] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 13);
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 8090);
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
	if (_action.isAction(VERB_LOOK, 0x59)) {
		_game._player.walk(Common::Point(148, 110), FACING_NORTH);
		_game._player._needToWalk = true;
		_game._player._readyToWalk = true;
	}

	if (_action.isAction(VERB_WALK_INSIDE, 0x16C) && _globals[kBeamIsUp]) {
		_globals[kCutX] = _game._player._playerPos.x;
		_globals[kCutY] = _game._player._playerPos.y;
		_globals[kCutFacing] = _game._player._facing;
		_globals[kForceBeamDown] = true;
		_globals[kDontRepeat] = true;
		_scene->_nextSceneId = 803;
	}
}

void Scene801::actions() {
	if (_action.isAction(VERB_LOOK, 0x59))
		_scene->_nextSceneId = 808;
	else if (_action.isAction(VERB_WALK_INSIDE, 0x16C)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 807;
	} else if (_action.isAction(VERB_WALK_THROUGH, 0x6E) && (_game._player._playerPos == Common::Point(270, 118))) {
		_game._player._stepEnabled = false;
		_game._player._facing = FACING_EAST;
		_game._player.selectSeries();
		_globals[kBetweenRooms] = true;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
		_vm->_sound->command(11);
	} else if (_action.isAction(VERB_LOOK, 0x46))
		_vm->_dialogs->show(80110);
	else if (_action.isAction(VERB_LOOK, 0xE2))
		_vm->_dialogs->show(80111);
	else if (_action.isAction(VERB_LOOK, 0x16C))
		_vm->_dialogs->show(80112);
	else if (_action.isAction(VERB_LOOK, 0x2C7) || _action._lookFlag)
		_vm->_dialogs->show(80113);
	else if (_action.isAction(VERB_LOOK, 0x390))
		_vm->_dialogs->show(80114);
	else if (_action.isAction(VERB_LOOK, 0x391))
		_vm->_dialogs->show(80115);
	else if (_action.isAction(VERB_LOOK, 0x18D))
		_vm->_dialogs->show(80116);
	else if (_action.isAction(VERB_LOOK, 0x6E))
		_vm->_dialogs->show(80117);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene802::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x137);
	_scene->addActiveVocab(0xD);
	_scene->addActiveVocab(0x123);
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

	} else if (_scene->_priorSceneId != -2) {
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
		int idx = _scene->_dynamicHotspots.add(0x123, 0xD,_globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(107, 99), FACING_NORTH);
	}

	if (!_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled]) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
		int idx = _scene->_dynamicHotspots.add(0x137, 0xD, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
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
		int idx = _scene->_dynamicHotspots.add(0x123, 0xD, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(107, 99), FACING_NORTH);

		_globals[kRemoteSequenceRan] = true;
		_globals[kRemoteOnGround] = true;
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 72)
		_vm->_sound->command(13);
}

void Scene802::preActions() {
	if (_action.isAction(VERB_WALK_TOWARDS, 0x393))
		_game._player._walkOffScreenSceneId = 801;

	if (_action.isAction(VERB_WALK_DOWN, 0x1C1)) {
		_game._player._walkOffScreenSceneId = 803;
		_globals[kForceBeamDown] = false;
	}

	if (_action.isAction(VERB_TAKE, 0x139))
		_game._player._needToWalk = false;
}

void Scene802::actions() {
	if (_action.isAction(VERB_TAKE, 0x137) && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], true, 7, 2, 0, 0);
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
	} else if ((_action.isAction(VERB_TAKE, 0x123)) && (!_game._objects.isInInventory(OBJ_REMOTE))) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[5], true, 7, 2, 0, 0);
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
		&& (_action.isAction(VERB_LOOK, 0x392) || _action._lookFlag))
		_vm->_dialogs->show(80210);
	else if (!_globals[kRemoteOnGround]&& !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled]
		&& (_action.isAction(VERB_LOOK, 0x392) || _action._lookFlag))
		_vm->_dialogs->show(80211);
	else if (_globals[kRemoteOnGround] && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled]
		&& (_action.isAction(VERB_LOOK, 0x392) || _action._lookFlag))
		_vm->_dialogs->show(80213);
	else if (_globals[kRemoteOnGround] && (_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) || _globals[kShieldModInstalled])
		&& (_action.isAction(VERB_LOOK, 0x392) || _action._lookFlag))
		_vm->_dialogs->show(80212);
	else if (!_game._objects.isInInventory(OBJ_SHIELD_MODULATOR) && !_globals[kShieldModInstalled] && _action.isAction(VERB_LOOK, 0x137))
		_vm->_dialogs->show(80214);
	else if (_globals[kRemoteOnGround] && _action.isAction(VERB_LOOK, 0x123))
		_vm->_dialogs->show(80216);
	else if (_action.isAction(VERB_LOOK, 0x139)) {
		if ((!_game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) && (!_globals[kShieldModInstalled]))
			_vm->_dialogs->show(80218);
		else
			_vm->_dialogs->show(80217);
	} else if (_action.isAction(VERB_LOOK, 0x39))
		_vm->_dialogs->show(80219);
	else if (_action.isAction(VERB_LOOK, 0x1C1))
		_vm->_dialogs->show(80220);
	else if (_action.isAction(VERB_LOOK, 0x142))
		_vm->_dialogs->show(80221);
	else if (_action.isAction(VERB_TAKE, 0x139))
		_vm->_dialogs->show(80222);
	else if (_action.isAction(VERB_LOOK, 0x174) || _action.isAction(VERB_LOOK, 0x175))
		_vm->_dialogs->show(80224);
	else if (_action.isAction(VERB_LOOK, 0x393))
		_vm->_dialogs->show(80225);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene803::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x472);
	_scene->addActiveVocab(0xD);

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
		int idx = _scene->_dynamicHotspots.add(0x472, 0xD, _globals._sequenceIndexes[7], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(66, 123), FACING_SOUTH);
	}

	if (!_globals[kBeamIsUp] && !_globals[kReturnFromCut] && (!_globals[kFromCockpit] || _globals[kExitShip])) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 2, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
	}

	if (!_globals[kFromCockpit]) {
		if (!_globals[kReturnFromCut]) {
			if (_scene->_priorSceneId != -2) {
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
			int idx = _scene->_dynamicHotspots.add(0x472, 0xD, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
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
		int idx = _scene->_dynamicHotspots.add(0x472, 0xD, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
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
		_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 15, 0, 0, 0);
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

			_vm->quitGame();
		}
	}

	if (_game._trigger == 150) {
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_vm->_sound->command(18);
		_globals._sequenceIndexes[6] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[6], false, 8, 1, 0, 0);
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
	if (_action.isAction(VERB_WALK_DOWN, 0x1AE))
		_game._player._walkOffScreenSceneId = 802;

	if (_action.isAction(VERB_TAKE, 0x139))
		_game._player._needToWalk = false;
}

void Scene803::actions() {
	if (_action.isAction(VERB_TAKE, 0x472)) {
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
			_globals._sequenceIndexes[9] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[9], true, 6, 1, 0, 0);
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
	} else if (_action.isAction(VERB_ENTER, 0x139)) {
		_vm->_sound->command(17);
		_game._player._stepEnabled = false;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 19);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 120);
		_globals[kBeamIsUp] = false;
	} else if (_action.isAction(VERB_LOOK, 0x392))
		_vm->_dialogs->show(80310);
	else if (_action._lookFlag)
		_vm->_dialogs->show(80310);
	else if (_action.isAction(VERB_LOOK, 0x395))
		_vm->_dialogs->show(80311);
	else if (_action.isAction(VERB_LOOK, 0x472)) {
		if (_game._storyMode == STORYMODE_NICE)
			_vm->_dialogs->show(80312);
		else
			_vm->_dialogs->show(80313);
	} else if (_action.isAction(VERB_LOOK, 0x39))
		_vm->_dialogs->show(80315);
	else if (_action.isAction(VERB_LOOK, 0x139))
		_vm->_dialogs->show(80317);
	else if (_action.isAction(VERB_LOOK, 0x396))
		_vm->_dialogs->show(80318);
	else if (_action.isAction(VERB_LOOK, 0x174) || _action.isAction(VERB_LOOK, 0x175))
		_vm->_dialogs->show(80319);
	else if (_action.isAction(VERB_LOOK, 0x142))
		_vm->_dialogs->show(80320);
	else if (_action.isAction(VERB_TAKE, 0x139))
		_vm->_dialogs->show(80321);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/


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
			_globals._sequenceIndexes[7] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[7], false, 4, 0, 0, 0);
			_scene->_sequences.addTimer(160, 70);
			_game._player._stepEnabled = false;
		}
	} else {
		if (_globals[kBeamIsUp] == 0)
			_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 1);

		if (_globals[kWindowFixed] == 0)
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);

		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[1], Common::Point(133, 139));
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

		if ((_throttleGone) && (_movingThrottle) && (_scene->_activeAnimation->getCurrentFrame() == 39)) {
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle
				(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[1], Common::Point(133, 139));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);
			_throttleGone = false;
		}

		if ((_movingThrottle) && (_scene->_activeAnimation->getCurrentFrame() == 42)) {
			_resetFrame = 0;
			_movingThrottle = false;
		}

		if (_game._trigger == 70) {
			_resetFrame = 42;
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 65)
			_scene->_sequences.remove(_globals._sequenceIndexes[7]);

		switch (_game._storyMode) {
		case STORYMODE_NAUGHTY:
			if (_scene->_activeAnimation->getCurrentFrame() == 81) {
				_resetFrame = 80;
				_globals[kInSpace] = false;
				_globals[kBeamIsUp] = true;

				assert(!_globals[kCopyProtectFailed]);
				_game._winStatus = 4;
				_vm->quitGame();
			}
			break;

		case STORYMODE_NICE:
			if (_scene->_activeAnimation->getCurrentFrame() == 68) {
				_resetFrame = 66;
				_globals[kInSpace] = false;
				_globals[kBeamIsUp] = true;

				assert(!_globals[kCopyProtectFailed]);
				_game._winStatus = 4;
				_vm->quitGame();
			}
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 34) {
			_resetFrame = 36;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 37) {
			_resetFrame = 36;
			if (!_dontPullThrottleAgain) {
				_dontPullThrottleAgain = true;
				_scene->_sequences.addTimer(60, 80);
			}
		}

		if (_game._trigger == 80) {
			_scene->_nextSceneId = 803;
		}

		if ((_scene->_activeAnimation->getCurrentFrame() == 7) && (!_globals[kWindowFixed])) {
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_scene->_sequences.addTimer(20, 110);
			_globals[kWindowFixed] = true;
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 10) {
			_resetFrame = 0;
			_game._player._stepEnabled = true;
			_game._objects.setRoom(OBJ_POLYCEMENT, NOWHERE);
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 1) {
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

		switch (_scene->_activeAnimation->getCurrentFrame()) {
		case 26:
		case 28:
		case 31:
			_resetFrame = 0;
			break;
		}
	} else {
		if ((_scene->_activeAnimation->getCurrentFrame() == 36) && (!_throttleGone)) {
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_throttleGone = true;
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 39) {
			_movingThrottle = false;
			switch (_throttleCounter) {
			case 1:
				break;
			case 3:
				_scene->_sequences.addTimer(130, 120);
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
		if (_resetFrame != _scene->_activeAnimation->getCurrentFrame()) {
			_scene->_activeAnimation->setCurrentFrame(_resetFrame);
			_resetFrame = -1;
		}
	}

	if (_game._trigger == 90) {
		_scene->_nextSceneId = 803;
	}

	if ((_scene->_activeAnimation->getCurrentFrame() == 72) && !_alreadyPop) {
		_vm->_sound->command(21);
		_alreadyPop = true;
	}

	if ((_scene->_activeAnimation->getCurrentFrame() == 80) && !_alreadyOrgan) {
		_vm->_sound->command(22);
		_alreadyOrgan = true;
	}
}

/*------------------------------------------------------------------------*/

void Scene805::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x476);
	_scene->addActiveVocab(0x167);
	_scene->addActiveVocab(0x137);
}

void Scene805::enter() {
	_game._player._visible = false;
	_scene->_userInterface.setup(kInputLimitedSentences);

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 2));

	if (_globals[kShieldModInstalled]) {
		_scene->_hotspots.activate(OBJ_SHIELD_MODULATOR, false);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 25);
		int idx = _scene->_dynamicHotspots.add(0x137, 0x476, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_DUMMY);
	}

	if (_globals[kTargetModInstalled]) {
		_scene->_hotspots.activate(OBJ_TARGET_MODULE, false);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 12);
		int idx = _scene->_dynamicHotspots.add(0x167, 0x476, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_DUMMY);
	}

	sceneEntrySound();
}

void Scene805::step() {
	if (_game._trigger == 70) {
		_scene->_hotspots.activate(OBJ_SHIELD_MODULATOR, false);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 25);
		int idx = _scene->_dynamicHotspots.add(0x137, 0x476, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_DUMMY);
		_globals[kShieldModInstalled] = true;
		_game._objects.setRoom(OBJ_SHIELD_MODULATOR, NOWHERE);
		_game._player._stepEnabled = true;
		_vm->_sound->command(24);
	}

	if (_game._trigger == 80) {
		_scene->_hotspots.activate(OBJ_TARGET_MODULE, false);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 12);
		int idx = _scene->_dynamicHotspots.add(0x167, 0x476, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_DUMMY);
		_globals[kTargetModInstalled] = true;
		_game._objects.setRoom(OBJ_TARGET_MODULE, NOWHERE);
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
	if (_action.isAction(VERB_EXIT, 0x398))
		_scene->_nextSceneId = 804;
	else if (_action.isAction(VERB_INSTALL, 0x137) && _game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_INSTALL, 0x167) && _game._objects.isInInventory(OBJ_TARGET_MODULE)) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_REMOVE, 0x137) && _globals[kShieldModInstalled]) {
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[1] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_REMOVE, 0x167) && _globals[kTargetModInstalled]) {
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
		_game._player._stepEnabled = false;
	} else if (_action.isAction(VERB_INSTALL, 0x137) && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR))
		_vm->_dialogs->show(80511);
	else if (_action.isAction(VERB_INSTALL, 0x167) && !_game._objects.isInInventory(OBJ_TARGET_MODULE))
		_vm->_dialogs->show(80510);
	else if (_action.isAction(VERB_REMOVE, 0x475))
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

	if (_action.isAction(VERB_LOOK, 0x181))
		_vm->_dialogs->show(80710);
	else if (_action.isAction(VERB_PEER_THROUGH, 0x181))
		_vm->_dialogs->show(80710);
	else if (_action.isAction(VERB_LOOK, 0xC4) && _action.isAction(VERB_INSPECT, 0xC4))
		_vm->_dialogs->show(80711);
	else if (_action.isAction(VERB_LOOK, 0x1CC))
		_vm->_dialogs->show(80712);
	else if (_action.isAction(VERB_LOOK, 0x1D1) || _action.isAction(VERB_LOOK, 0x1D2)
	 || _action.isAction(VERB_LOOK, 0x1D3) || _action.isAction(VERB_LOOK, 0x1D4)
	 || _action.isAction(VERB_LOOK, 0x1D5) || _action.isAction(VERB_LOOK, 0x1D6)
	 || _action.isAction(VERB_LOOK, 0x1D7) || _action.isAction(VERB_LOOK, 0x1D8)
	 || _action.isAction(VERB_LOOK, 0x1D9) || _action.isAction(VERB_LOOK, 0x1D0)
	 || _action.isAction(VERB_LOOK, 0x1DB) || _action.isAction(VERB_LOOK, 0x1DA))
		_vm->_dialogs->show(80713);
	else if (_action.isAction(VERB_LOOK, 0x1CF) && _action._lookFlag)
		_vm->_dialogs->show(80714);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

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
	if (_action.isAction(VERB_PRESS, 0x3BC)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(248, 211));
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
			_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(248, 211));
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
	} else if (_action.isAction(VERB_PRESS, 0x3BE)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(248, 186));
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
			_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(248, 186));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
			break;

		case 91:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRESS, 0x3BD)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(248, 163));
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
			_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 4, 1, 0, 0);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(248, 163));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
			break;

		case 81:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRESS, 0x3BF)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 4, 2, 0, 0);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(168, 211));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
			break;

		case 70:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRESS, 0x3BB)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 4, 2, 0, 0);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(172, 163));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			break;

		case 80:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRESS, 0x3BA)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 4, 2, 0, 0);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(172, 186));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
			break;

		case 90:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_EXIT, 0x1E0)) {
		_scene->_nextSceneId = 801;
		_globals[kBetweenRooms] = true;
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

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
	if ((_scene->_activeAnimation->getCurrentFrame() == 200) && _moveAllowed) {
		_scene->_sequences.addTimer(100, 70);
		_moveAllowed = false;
	}

	if (_game._trigger == 70)
		_scene->_nextSceneId = 804;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
