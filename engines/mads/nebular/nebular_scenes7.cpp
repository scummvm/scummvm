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

// Scene 7xx verbs
enum {
	VERB_LOOK_AT = 0xD1,
	VERB_WALK_ALONG = 0x312,
	VERB_STEP_INTO = 0x2F9
};

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
	if (_action.isAction(VERB_WALKTO, NOUN_WEST_END_OF_PLATFORM)) {
		_game._player._walkOffScreenSceneId = 701;
	}
}

void Scene702::actions() {
	if (_action.isAction(VERB_WALK_ALONG, NOUN_PLATFORM)) {
		_action._inProgress = false;
	} else if (_action.isAction(VERB_STEP_INTO, NOUN_TELEPORTER)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 711;
		_action._inProgress = false;
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && _action._mainObjectSource == 4) {
		// Take bones
		if (!_game._objects.isInInventory(OBJ_BONES) || _game._trigger) {
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
					_game._objects.setRoom(OBJ_BONE, NOWHERE);
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

			_action._inProgress = false;
		}
	} else if (_action._lookFlag) {
		_vm->_dialogs->show(70210);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_PLATFORM)) {
		_vm->_dialogs->show(70211);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_CEMENT_BLOCK)) {
		_vm->_dialogs->show(70212);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_ROCK)) {
		_vm->_dialogs->show(70213);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_TAKE, NOUN_ROCK)) {
		_vm->_dialogs->show(70214);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_WEST_END_OF_PLATFORM)) {
		_vm->_dialogs->show(70215);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER)) {
		_vm->_dialogs->show(70216);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_BONES) && _action._mainObjectSource == 4) {
		_vm->_dialogs->show(70217);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && _action._mainObjectSource == 4) {
		if (_game._objects.isInInventory(OBJ_BONES))
			_vm->_dialogs->show(70219);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_SUBMERGED_CITY)) {
		_vm->_dialogs->show(70220);
		_action._inProgress = false;
	}
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
		_globals._sequenceIndexes[13] =  _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		int idx = _scene->_dynamicHotspots.add(0xB3, VERB_WALKTO, _globals._sequenceIndexes[13], Common::Rect(0, 0, 0, 0));
		_cardId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(234, 135), FACING_NORTH);
	}

	if (_game._globals[kLaserHoleIsThere]) {
		_globals._sequenceIndexes[14] =  _scene->_sequences.startCycle(_globals._spriteIndexes[14], false, 1);
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
	if (_action.isAction(VERB_WALK_ALONG, NOUN_PLATFORM)) {
		_action._inProgress = false;
	} else if (_action.isAction(VERB_STEP_INTO, NOUN_TELEPORTER)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 711;
		_action._inProgress = false;
	} else if (_action.isAction(VERB_TAKE, NOUN_ID_CARD)) {
		// Take ID card
		if (!_game._objects.isInInventory(OBJ_ID_CARD) || _game._trigger) {
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

			_action._inProgress = false;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && _action._mainObjectSource == 4) {
		// Take bones
		if (!_game._objects.isInInventory(OBJ_BONES) || _game._trigger) {
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
					_game._objects.setRoom(OBJ_BONE, NOWHERE);
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

			_action._inProgress = false;
		}
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_CITY)) {
		if (_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(75212);
		else
			_vm->_dialogs->show(75210);

		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_PLATFORM)) {
		_vm->_dialogs->show(75213);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_CEMENT_BLOCK)) {
		_vm->_dialogs->show(75214);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_ROCK)) {
		_vm->_dialogs->show(75215);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_TAKE, NOUN_ROCK)) {
		_vm->_dialogs->show(75216);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_WEST_END_OF_PLATFORM)) {
		_vm->_dialogs->show(75217);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER)) {
		_vm->_dialogs->show(75218);
		_action._inProgress = false;
	} else if ((_action.isAction(VERB_LOOK, NOUN_BONES) || _action.isAction(VERB_LOOK, NOUN_ID_CARD)) && _action._mainObjectSource == 4) {
		if (_game._objects[OBJ_ID_CARD]._roomNumber == 752)
			_vm->_dialogs->show(75219);
		else
			_vm->_dialogs->show(75220);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_TAKE, NOUN_BONES) && _action._mainObjectSource == 4) {
		if (_game._objects.isInInventory(OBJ_BONES))
			_vm->_dialogs->show(75222);
		_action._inProgress = false;
	}
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
