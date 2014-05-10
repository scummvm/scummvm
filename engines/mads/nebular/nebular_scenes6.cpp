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
#include "mads/nebular/nebular_scenes6.h"

namespace MADS {

namespace Nebular {

void Scene6xx::setAAName() {
	_game._aaName = Resources::formatAAName(5);
}

void Scene6xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;

	if ((_scene->_nextSceneId == 605) || (_scene->_nextSceneId == 620))
		_game._player._spritesPrefix = "";
	else if (_globals[kSexOfRex] == REX_MALE)
		_game._player._spritesPrefix = "RXM";
	else
		_game._player._spritesPrefix = "ROX";

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_game._player._scalingVelocity = true;
	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene6xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 601:
	case 602:
	case 603:
	case 604:
	case 605:
	case 607:
	case 608:
	case 609:
	case 610:
	case 612:
	case 620:
		_vm->_sound->command(29);
		break;
	case 611:
		_vm->_sound->command(24);
		break;
	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

void Scene601::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x343);
	_scene->addActiveVocab(0xD1);
}

void Scene601::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_4");

	if (_globals[kLaserHoleIsThere]) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_dynamicHotspots.add(0x343, 0xD1, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
	}

	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);  
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);

	if (_scene->_priorSceneId == 504) {
		_game._player._playerPos = Common::Point(73, 148);
		_game._player._facing = FACING_WEST; 
		_game._player._visible   = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]); 
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
		_scene->loadAnimation(formAnimName('R', 1), 70); 
	} else if (_scene->_priorSceneId != -2) {
		_game._player._playerPos = Common::Point(229, 129);
		_game._player._facing = FACING_SOUTHWEST;
	}

	sceneEntrySound();
}

void Scene601::step() {
	switch (_game._trigger) {
	case 70:
		_game._player._visible   = true;
		_game._player._priorTimer = _scene->_activeAnimation->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(30, 71);
		break;

	case 71:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break; 

	case 72:
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);  
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
		_game._player._stepEnabled = true;
		break; 

	default:
		break;
	}
}

void Scene601::actions() {
	if (_action.isAction(0x18B, 0x378))
		_scene->_nextSceneId = 602;
	else if (_action.isAction(0x325, 0x324)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);  
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);  
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
			_scene->_sequences.addTimer(6, 2);
			}
			break;

		case 2:
			_game._player._visible   = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3: {
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx); 
			_scene->_nextSceneId = 504;
			}
			break;

		default:
			break;
		}
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, 0x31E)) {
		if (!_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(60110);
		else
			_vm->_dialogs->show(60111);
	} else if (_action.isAction(VERB_LOOK, 0x324))
		_vm->_dialogs->show(60112);
	else if (_action.isAction(VERB_LOOK, 0x32C))
		_vm->_dialogs->show(60113);
	else if (_action.isAction(VERB_LOOK, 0x323))
		_vm->_dialogs->show(60114);
	else if (_action.isAction(0x1AD, 0x31E))
		_vm->_dialogs->show(60115);
	else if (_action.isAction(VERB_LOOK, 0x48E))
		_vm->_dialogs->show(60116);
	else if (_action.isAction(VERB_LOOK, 0x378))
		_vm->_dialogs->show(60117);
	else if (_action.isAction(VERB_LOOK, 0x18D))
		_vm->_dialogs->show(60118);
	else if (_action.isAction(VERB_LOOK, 0x38F))
		_vm->_dialogs->show(60119);
	else if (_action.isAction(VERB_LOOK, 0x3C4))
		_vm->_dialogs->show(60120);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
