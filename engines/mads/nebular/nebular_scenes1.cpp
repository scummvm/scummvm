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
#include "mads/nebular/nebular_scenes1.h"

namespace MADS {

namespace Nebular {

void Scene1xx::setAAName() {
	int idx = (_scene->_nextSceneId > 103 && _scene->_nextSceneId < 112) ? 1 : 0;
	_game._aaName = Resources::formatAAName(idx);
}

void Scene1xx::sceneEntrySound() {
	if (_vm->_musicFlag) {
		switch (_scene->_nextSceneId) {
		case 101:
			_vm->_sound->command(11);
			break;
		case 102:
			_vm->_sound->command(12);
			break;
		case 103:
			_vm->_sound->command(3);
			_vm->_sound->command(25);
			break;
		case 109:
			_vm->_sound->command(13);
			break;
		case 110:
			_vm->_sound->command(10);
			break;
		case 111:
			_vm->_sound->command(3);
			break;
		case 112:
			_vm->_sound->command(15);
			break;
		default:
			if (_scene->_priorSceneId < 104 || _scene->_priorSceneId > 108)
				_vm->_sound->command(10);
			break;
		}
	}
}

void Scene1xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;
	if (_scene->_nextSceneId <= 103 || _scene->_nextSceneId == 111) {
		if (_globals[kSexOfRex] == SEX_FEMALE) {
			_game._player._spritesPrefix = "ROX";
		} else {
			_game._player._spritesPrefix = "RXM";
			_globals[kSexOfRex] = SEX_MALE;
		}
	} else if (_scene->_nextSceneId <= 110) {
		_game._player._spritesPrefix = "RXSW";
		_globals[kSexOfRex] = SEX_UNKNOWN;
	} else if (_scene->_nextSceneId == 112) {
		_game._player._spritesPrefix = "";
	}

	if (oldName == _game._player._spritesPrefix)
		_game._player._spritesChanged = true;
	if (_scene->_nextSceneId == 105 || (_scene->_nextSceneId == 109 && _globals[kHoovicAlive])) {
		_game._player._spritesChanged = true;
		_game._player._loadsFirst = false;
	}

	_game._player._trigger = 0;
	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

/*------------------------------------------------------------------------*/

void Scene101::setup() {
	_scene->_animationData->preLoad(formAnimName('A', -1), 3);
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene101::sayDang() {
	_game._triggerSetupMode = KERNEL_TRIGGER_DAEMON;
	_game._player._stepEnabled = false;

	switch (_game._trigger) {
	case 0:
		_scene->_sequences.remove(_globals._sequenceIndexes[11]);
		_globals._sequenceIndexes[11] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[11], false, 3, 6, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 21);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SM_0, 0, 72);
		_vm->_sound->command(17);
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 3, 2, 0, 0);
		break;

	case 72:
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 17);
		_scene->_kernelMessages.add(Common::Point(143, 61), 0x1110, 0, 0, 60, _game.getQuote(57));
		_scene->_sequences.addTimer(120, 73);
		break; 

	case 73:
		_vm->_dialogs->show(0x2785);
		_game._player._stepEnabled = true;
		break;
	}
}

void Scene101::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 6));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('x', 7));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('m', -1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('b', 2));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('x', 8));
	_globals._spriteIndexes[14] = _scene->_sprites.addSprites(formAnimName('x', 0));

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 0, 0, 25);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 0, 1, 0);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 2, 0);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SM_FRAME_INDEX, 7, 70);
	_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 10, 0, 0, 60);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 5, 0, 1, 0);
	_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 0, 2, 0);
	_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 0, 0, 0);
	_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, 0, 10, 4);
	_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 6, 0, 32, 47);

	_scene->_hotspots.activate(0x137, false);
	_panelOpened = false;

	if (_scene->_priorSceneId != -1)
		_globals[kNeedToStandUp] = false;

	if (_scene->_priorSceneId != -2)
		_game._player._playerPos = Common::Point(100, 152);

	if ((_scene->_priorSceneId == 112) || ((_scene->_priorSceneId == -2) && _sittingFl )) {
		_game._player._visible = false;
		_sittingFl = true;
		_game._player._playerPos = Common::Point(161, 123);
		_game._player._facing = FACING_NORTHEAST;
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 17);
		_scene->_hotspots.activate(0x47, false);
		_chairHotspotId = _scene->_dynamicHotspots.add(0x47, 0x13F, -1, Common::Rect(159, 84, 33, 36));
		if (_scene->_priorSceneId == 112)
			sayDang();
	} else {
		_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 4);
	}

	_game.loadQuoteSet(0x31, 0x32, 0x39, 0x36, 0x37, 0x38, 0);

	if (_globals[10]) {
		_scene->loadAnimation(Resources::formatName(101, 'S', -1, EXT_AA, ""), 71);
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_game._player._playerPos = Common::Point(68, 140);
		_game._player._facing = FACING_WEST;

		_messageNum = 0;
		_posY = 30;
	}

	_oldSpecial = false;

	sceneEntrySound();
}

void Scene101::step() {
	if (_oldSpecial != _game._player._special) {
		_oldSpecial = _game._player._special;
		if (_oldSpecial)
			_vm->_sound->command(39);
		else
			_vm->_sound->command(11);
	}

	switch (_game._trigger) {
	case 70:
		_vm->_sound->command(9);
		break;

	case 71:
		_globals[10] = false;
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		break;

	case 72:
	case 73:
		sayDang();
		break;
	}

	if (_scene->_activeAnimation != nullptr) {
		if ((_scene->_activeAnimation->getCurrentFrame() >= 6) && (_messageNum == 0)) {
			_messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, _posY), 0x1110, 0, 0, 240, _game.getQuote(49));
			_posY += 14;
		}

		if ((_scene->_activeAnimation->getCurrentFrame() >= 7) && (_messageNum == 1)) {
			_messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, _posY), 0x1110, 0, 0, 240, _game.getQuote(54));
			_posY += 14;
		}

		if ((_scene->_activeAnimation->getCurrentFrame() >= 10) && (_messageNum == 2)) {
			_messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, _posY), 0x1110, 0, 0, 240, _game.getQuote(55));
			_posY += 14;
		}

		if ((_scene->_activeAnimation->getCurrentFrame() >= 17) && (_messageNum == 3)) {
			_messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, _posY), 0x1110, 0, 0, 240, _game.getQuote(56));
			_posY += 14;
		}

		if ((_scene->_activeAnimation->getCurrentFrame() >= 20) && (_messageNum == 4)) {
			_messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, _posY), 0x1110, 0, 0, 240, _game.getQuote(50));
			_posY += 14;
		}
	}
}

void Scene101::preActions() {
	if (_action.isAction(VERB_LOOK, 0x180))
		_game._player._needToWalk = true;

	if (_sittingFl) {
		if (_action.isAction(VERB_LOOK) || _action.isAction(0x47) || _action.isAction(VERB_TALKTO) || _action.isAction(0x103) || _action.isAction(0x7D))
			_game._player._needToWalk = false;

		if (_game._player._needToWalk) {
			switch (_game._trigger) {
			case 0:
				_game._player._needToWalk = false;	
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[11]);
				_globals._sequenceIndexes[11] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SM_0, 0, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 1, 17);
				_vm->_sound->command(16);
				break;

			case 1:
				_sittingFl = false;
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_game._player._needToWalk = true;
				_scene->_hotspots.activate(71, true);
				_scene->_dynamicHotspots.remove(_chairHotspotId);
				_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 6, 0, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 4);
				break;
			}
		}
	}

	if (_panelOpened && !(_action.isAction(0x135) || _action.isAction(0x137))) {
		switch (_game._trigger) {
		case 0:
			if (_game._player._needToWalk) {
				_scene->_sequences.remove(_globals._sequenceIndexes[13]);
				_shieldSpriteIdx = _game._objects.isInRoom(0x18) ? 13 : 14;
				_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[_shieldSpriteIdx], false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SM_0, 0, 1);
				_game._player._stepEnabled = false;
				_vm->_sound->command(20);
			}
			break;

		case 1:
			_game._player._stepEnabled = true;
			_panelOpened = false;
			_scene->_hotspots.activate(0x137, false);
			break;
		}
	}
}

void Scene101::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(0x278D);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, 0xCC)) {
		_scene->_nextSceneId = 102;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(0x13F, 0x47) || (_action.isAction(VERB_LOOK, 0x180) && !_sittingFl)) {
		if (!_sittingFl) {
			switch (_game._trigger) {
			case 0:
				_scene->_sequences.remove(_globals._sequenceIndexes[12]);
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 1, 17);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SM_FRAME_INDEX, 10, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SM_0, 0, 2);
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_action._inProgress = false;
				return;

			case 1:
				_vm->_sound->command(16);
				break;

			case 2:
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 17);
				_game._player._stepEnabled = true;
				_sittingFl = true;
				_scene->_hotspots.activate(71, false);
				_chairHotspotId = _scene->_dynamicHotspots.add(71, 0x13F, -1, Common::Rect(159, 84, 159 + 33, 84 + 36));
				if (!_action.isAction(VERB_LOOK, 0x180)) {
					_action._inProgress = false;
					return;
				}
				_game._trigger = 0;
				break;
			}
		} else {
			_vm->_dialogs->show(0x2793);
			_action._inProgress = false;
			return;
		}
	}

	if (((_action.isAction(VERB_WALKTO, 0x135) || _action.isAction(VERB_OPEN, 0x135))) && !_panelOpened) {
		switch (_game._trigger) {
		case 0:
			_shieldSpriteIdx = _game._objects.isInRoom(0x18) ? 13 : 14;
			_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[_shieldSpriteIdx], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SM_0, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[13]);
			_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[_shieldSpriteIdx], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], -2, -2);
			_game._player._stepEnabled = true;
			_panelOpened = true;
			if (_game._objects.isInRoom(0x18))
				_scene->_hotspots.activate(0x137, true);
			break;
		}
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_TAKE, 0x137) || _action.isAction(VERB_PULL, 0x137)) && _game._objects.isInRoom(0x18)) {
		_game._objects.addToInventory(0x18);
		_scene->_sequences.remove(_globals._sequenceIndexes[13]);
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[14], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], -2, -2);
		_scene->_hotspots.activate(0x137, false);
		_vm->_dialogs->showPicture(0x18, 0x2788);
		_vm->_sound->command(22);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x135) || (_action.isAction(VERB_LOOK, 0x137) && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) ) {
		if (_panelOpened) {
			if (_game._objects.isInRoom(OBJ_SHIELD_MODULATOR))
				_vm->_dialogs->show(0x2790);
			else
				_vm->_dialogs->show(0x2791);
		} else {
			_vm->_dialogs->show(0x278F);
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, 0x135) && _panelOpened) {
		_vm->_dialogs->show(0x2792);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x180) && _sittingFl) {
		if (_globals[kWatchedViewScreen]) {
			sayDang();
		} else {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[11]);
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 21);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SM_0, 0, 1);
				_vm->_sound->command(17);
				break;

			case 1:
				_globals._sequenceIndexes[11] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SM_0, 0, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 21);
				break;

			case 2:
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 17);
				_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SM_0, 0, 3);
				break;

			case 3:
				_game._player._stepEnabled = true;
				_globals[kWatchedViewScreen] = true;
				_sittingFl = true;
				_scene->_nextSceneId = 112;
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x47)) {
		_vm->_dialogs->show(0x2775);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_LOOK) || _action.isAction(0x103)) && (_action.isAction(0x8E) || _action.isAction(0xF9))) {
		_vm->_dialogs->show(0x2776);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0xA8) || _action.isAction(VERB_LOOK, 0xF8) || _action.isAction(0x7D, 0xA8) || _action.isAction(0x7D, 0xF8)) {
		_vm->_dialogs->show(0x2777);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x91)) {
		_vm->_dialogs->show(0x2778);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0xE1) || _action.isAction(0xD2, 0xE1)) {
		_vm->_dialogs->show(0x2779);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x60)) {
		_vm->_dialogs->show(0x277A);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x111)) {
		_vm->_dialogs->show(0x277B);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x7B) || (_action.isAction(VERB_OPEN, 0x7B) && !_game._objects.isInInventory(OBJ_REBREATHER))) {
		_vm->_dialogs->show(0x277D);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, 0x7B)) {
		_vm->_dialogs->show(0x277E);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x166)) {
		_vm->_dialogs->show(0x277F);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0xCA)) {
		_vm->_dialogs->show(0x278E);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x63)) {
		_vm->_dialogs->show(0x2780);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0xEB)) {
		_vm->_dialogs->show(0x2781);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x78)) {
		_vm->_dialogs->show(0x2782);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x190)) {
		_vm->_dialogs->show(0x2783);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x138)) {
		_vm->_dialogs->show(0x2784);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, 0x111)) {
		_vm->_dialogs->show(0x2786);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, 0x91)) {
		_vm->_dialogs->show(0x2787);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, 0x63)) {
		_vm->_dialogs->show(0x2789);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, 0x60)) {
		_vm->_dialogs->show(0x278A);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, 0x60)) {
		_vm->_dialogs->show(0x278B);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_LOOK) || _action.isAction(0x112)) && _action.isAction(0x17E)) {
		_vm->_dialogs->show(0x278C);
		_action._inProgress = false;
		return;
	}
}

/*------------------------------------------------------------------------*/

void Scene102::setup() {
	_scene->_animationData->preLoad(formAnimName('A', -1), 3);
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene102::addRandomMessage() {
	_scene->_kernelMessages.reset();
	_game._triggerSetupMode = KERNEL_TRIGGER_DAEMON;
	int quoteId = _vm->getRandomNumber(65, 69);
	_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 73, 120, _game.getQuote(quoteId));
	_activeMsgFl = true;
}

void Scene102::enter() {
	sceneEntrySound();

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5));

	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('e', -1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('n', -1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('g', -1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites("*RXMRC_8");
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('x', 0));

	_globals._spriteIndexes[15+1] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[1], false, 8, 0, 0, 0);
	_globals._spriteIndexes[15+2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 170, 0, 1, 6);
	_globals._spriteIndexes[15+3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 11, 0, 2, 3);
	_globals._spriteIndexes[15+4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 0, 1, 0);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 3, 0, 0, 5);

	if (!_game._objects.isInRoom(OBJ_BINOCULARS))
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 24, 0, 0, 24);
	else
		_scene->_hotspots.activate(0x27, false);

	_scene->_hotspots.activate(0x35, false);

	if (_globals[kMedicineCabinetOpen]) {
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -2, -2);
	}

	if (_scene->_priorSceneId == 101) {
		_game._player._playerPos = Common::Point(229, 109);
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 2, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SM_0, 0, 70);
	} else if (_scene->_priorSceneId == 103) {
		_game._player._playerPos = Common::Point(47, 152);
	} else if (_scene->_priorSceneId != -2) {
		_game._player._facing = FACING_NORTHWEST;
		_game._player._playerPos = Common::Point(32, 129);
	}

	if (_scene->_priorSceneId != 106) {
		if (_globals[kWaterInAPuddle]) {
			_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], -2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 5);
		}
	} else {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SM_0, 0, 72);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 5);
		_vm->_sound->command(24);
		_vm->_sound->command(28);
	}

	_fridgeOpenedFl = false;
	_fridgeOpenedDescr = false;
	_fridgeCommentCount = 0;
	_fridgeFirstOpenFl = true;
	_chairDescrFl = false;
	_activeMsgFl = false;

	_game.loadQuoteSet(0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x45, 0x43, 0);

	if (_scene->_priorSceneId == 101)
		_vm->_sound->command(20);
}

void Scene102::step() {
	if (_game._trigger == 70)
		_game._player._stepEnabled = true;

	if (_game._trigger == 72) {
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], -2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 5);
		_scene->_sequences.addTimer(48, 90);
	}

	if (_game._trigger >= 90) {
		if (_game._trigger >= 94) {
			_scene->loadAnimation(formAnimName('B', -1), 71);
			_game._player._stepEnabled = false;
			_game._player._visible = false;

			_globals[kWaterInAPuddle] = true;
			_vm->_sound->command(24);
		} else {
			_vm->_sound->command(23);
			_scene->_sequences.addTimer(48, _game._trigger + 1);
		}
	}

	if (_game._trigger == 71) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
	}

	if (_fridgeOpenedFl && !_fridgeOpenedDescr) {
		_fridgeCommentCount++;
		if (_fridgeCommentCount > 16384) {
			_fridgeOpenedDescr = true;
			_vm->_dialogs->show(0x27E5);
		}
	}

	if (!_activeMsgFl && (_game._player._playerPos == Common::Point(177, 114)) && (_game._player._facing == FACING_NORTH)
	&& (_vm->getRandomNumber(1, 5000) == 1)) {
		_scene->_kernelMessages.reset();
		_activeMsgFl = false;
		addRandomMessage();
	}

	if (_game._trigger == 73)
		_activeMsgFl = false;
}

void Scene102::preActions() {
	if (_action.isAction(0x122) || _action.isAction(0x117))
		_game._player._needToWalk = _game._player._readyToWalk;

	if (_fridgeOpenedFl && !_action.isAction(0x122)) {
		switch (_game._trigger) {
		case 0:
			if (_game._player._needToWalk) {
				_scene->_sequences.remove(_globals._sequenceIndexes[7]);
				_globals._sequenceIndexes[7] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[7], false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SM_0, 0, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
				_game._player._stepEnabled = false;
				_vm->_sound->command(20);
			}
			break;

		case 1:
			if (_game._objects.isInRoom(OBJ_BURGER)) {
				_scene->_sequences.remove(_globals._sequenceIndexes[10]);
				_scene->_hotspots.activate(0x35, false);
			}
			_fridgeOpenedFl = false;
			_game._player._stepEnabled = true;
			break;
		}
	}

	if (_game._player._needToWalk)
		_scene->_kernelMessages.reset();
}

void Scene102::actions() {
	bool justOpenedFl = false;

	if (_action._lookFlag) {
		_vm->_dialogs->show(0x27FA);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(0x122) && !_fridgeOpenedFl) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SM_0, 0, 1);
			if (_game._objects.isInRoom(OBJ_BURGER)) {
				_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 7, 0, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);
			}
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			_action._inProgress = false;
			return;

		case 1:
			_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], -2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
			int delay;
			if (_action.isAction(VERB_WALKTO) && !_fridgeFirstOpenFl)
				delay = 0;
			else
				delay = 48;
			_scene->_sequences.addTimer(delay, 2);
			_action._inProgress = false;
			return;

		case 2:
			_fridgeOpenedFl = true;
			_fridgeOpenedDescr = false;
			_fridgeCommentCount = 0;
			_game._player._stepEnabled = true;
			justOpenedFl = true;
			if (_game._objects.isInRoom(OBJ_BURGER))
				_scene->_hotspots.activate(0x35, true);
			break;
		}
	}

	if (_action.isAction(VERB_LOOK, 0x122) || _action.isAction(VERB_OPEN, 0x122)) {
		if (_game._objects.isInRoom(OBJ_BURGER))
			_vm->_dialogs->show(0x27F6);
		else
			_vm->_dialogs->show(0x27F5);

		_fridgeFirstOpenFl = false;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, 0x122) && justOpenedFl) {
		_fridgeFirstOpenFl = false;
		int quoteId = _vm->getRandomNumber(59, 63);
		Common::String curQuote = _game.getQuote(quoteId);
		int width = _vm->_font->getWidth(curQuote, -1);
		_scene->_kernelMessages.reset();
		_game._triggerSetupMode = KERNEL_TRIGGER_DAEMON;
		_scene->_kernelMessages.add(Common::Point(210, 60), 0x1110, 0, 73, 120, curQuote);
		_scene->_kernelMessages.add(Common::Point(214 + width, 60), 0x1110, 0, 73, 120, _game.getQuote(64));
		_activeMsgFl = true;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, 0x122)) {
		_vm->_dialogs->show(0x27E5);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, 0x122)) {
		_vm->_dialogs->show(0x8);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(0x18B, 0x6E)) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SM_0, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			break;

		case 1:
			_scene->_nextSceneId = 101;
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, 0x79)) {
		_scene->_nextSceneId = 103;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, 0x117) || _action.isAction(VERB_LOOK, 0x117) || _action.isAction(VERB_WALKTO, 0x27)) {
		addRandomMessage();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x194)) {
		_vm->_dialogs->show(0x27E4);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x79)) {
		_vm->_dialogs->show(0x27DD);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x6E)) {
		_vm->_dialogs->show(0x27DC);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(0x155, 0x46) || _action.isAction(VERB_LOOK, 0x46)) {
		_vm->_dialogs->show(0x27DB);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(0x155, 0xFD) || _action.isAction(VERB_LOOK, 0xFD)) {
		_vm->_dialogs->show(0x27DA);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x127)) {
		_vm->_dialogs->show(0x27E7);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, 0x35, 0x127) && _game._objects.isInInventory(OBJ_BURGER)) {
		_vm->_dialogs->show(0x27E8);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, 0x122) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		_vm->_dialogs->show(0x27E9);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, 0x127) && (_action.isAction(0x65) || _action.isAction(0x157))) {
		_vm->_dialogs->show(0x27F6);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, 0x127)) {
		_vm->_dialogs->show(0x27EA);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x52)) {
		_vm->_dialogs->show(0x27EB);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(0xC7) || _action.isAction(0xA3)) && (_action.isAction(VERB_LOOK) || _action.isAction(0x50) || _action.isAction(0x4F))) {
		if (_game._objects.isInInventory(OBJ_REBREATHER)) {
			if (!_action.isAction(0x50) && !_action.isAction(0x4F)) {
				_vm->_dialogs->show(0x27F7);
				_action._inProgress = false;
				return;
			}
		} else if (_action.isAction(VERB_LOOK) || (_game._difficulty == DIFFICULTY_IMPOSSIBLE)) {
			_vm->_dialogs->show(0x27EE);
			_action._inProgress = false;
			return;
		}
	}

	if ( (_action.isAction(0xC7) || _action.isAction(0xA3)) && (_action.isAction(0x50) || _action.isAction(0x4F)) ) {
		switch (_game._trigger) {
		case 0:
			_scene->loadAnimation(formAnimName('A', -1), 1);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			break;

		case 1:
			_vm->_sound->command(24);
			_scene->_sequences.addTimer(48, 2);
			break;

		case 2:
		case 3:
		case 4:
			_vm->_sound->command(23);
			_scene->_sequences.addTimer(48, _game._trigger + 1);
			break;

		case 5:
			_vm->_sound->command(24);
			_scene->_sequences.addTimer(48, _game._trigger + 1);
			break;

		case 6:
			if (_game._objects.isInInventory(OBJ_REBREATHER) && !_game._visitedScenes.exists(106))
				_vm->_dialogs->show(0x27FD);
			_scene->_nextSceneId = 106;
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x118)) {
		_vm->_dialogs->show(0x27F2);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x197) || _action.isAction(0xD3, 0x197)) {
		_vm->_dialogs->show(0x27F3);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x70) || _action.isAction(VERB_WALKTO, 0x70)) {
		_vm->_dialogs->show(0x27F4);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x71) || ((_action.isAction(VERB_CLOSE, 0x71) || _action.isAction(VERB_PUSH, 0x71)) && !_drawerDescrFl)) {
		_vm->_dialogs->show(0x27EC);
		_drawerDescrFl = true;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, 0x71) || _action.isAction(VERB_PUSH, 0x71)) {
		_vm->_dialogs->show(0x27ED);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, 0x71)) {
		_vm->_dialogs->show(0x27FC);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x47) || (_action.isAction(0x13F, 0x47) && !_chairDescrFl)) {
		_chairDescrFl = true;
		_vm->_dialogs->show(0x27E2);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(0x13F, 0x47)) {
		_vm->_dialogs->show(0x27E3);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0xDE)) {
		if (_globals[kMedicineCabinetOpen])
			_vm->_dialogs->show(0x27DF);
		else
			_vm->_dialogs->show(0x27DE);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, 0xDE) && _globals[kMedicineCabinetOpen]) {
		switch (_game._trigger) {
		case 0:
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			_globals._sequenceIndexes[8] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SM_0, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(21);
			break;

		case 1:
			_scene->_sequences.addTimer(48, 2);
			break;

		case 2:
			_game._player._stepEnabled = true;
			_globals[kMedicineCabinetOpen] = false;
			_vm->_dialogs->show(0x27E1);
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, 0xDE) && !_globals[kMedicineCabinetOpen]) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SM_0, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(21);
			break;

		case 1:
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -2, -2);
			_scene->_sequences.addTimer(48, 2);
			break;

		case 2:
			_game._player._stepEnabled = true;
			_globals[kMedicineCabinetOpen] = true;
			if (_globals[kMedicineCabinetVirgin]) {
				_vm->_dialogs->show(0x27E0);
			} else {
				_vm->_dialogs->show(0x27DF);
			}
			_globals[kMedicineCabinetVirgin] = false;
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, 0x27) && _game._objects.isInRoom(OBJ_BINOCULARS)) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[11] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[11]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SM_0, 0, 1);
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			break;

		case 1:
			_game._objects.addToInventory(OBJ_BINOCULARS);
			_scene->_sequences.remove(_globals._spriteIndexes[15+9]);
			_scene->_hotspots.activate(0x27, false);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_vm->_sound->command(22);
			_vm->_dialogs->showPicture(OBJ_BINOCULARS, 0x27D9);
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, 0x35) && _game._objects.isInRoom(OBJ_BURGER)) {
		if (_game._trigger == 0) {
			_vm->_dialogs->showPicture(OBJ_BURGER, 0x27FB);
			_scene->_sequences.remove(_globals._sequenceIndexes[10]);
			_game._objects.addToInventory(OBJ_BURGER);
			_scene->_hotspots.activate(0x35, false);
			_vm->_sound->command(22);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, 0x117)) {
		_vm->_dialogs->show(0x27F0);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isAction(0x194)) {
		_vm->_dialogs->show(0x27F1);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x89)) {
		_vm->_dialogs->show(0x27F8);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x27) && !_game._objects.isInInventory(OBJ_BINOCULARS)) {
		_vm->_dialogs->show(0x27F9);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x35) && (_action._mainObjectSource == 4)) {
		_vm->_dialogs->show(0x321);
		_action._inProgress = false;
	}
}

void Scene102::postActions() {
	if (_action.isAction(VERB_PUT, 0x127) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		_vm->_dialogs->show(0x27E9);
		_action._inProgress = false;
	}
}

/*------------------------------------------------------------------------*/

void Scene103::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene103::enter() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('h', -1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('m', -1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('t', -1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('r', -1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('c', -1));

	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RXMBD_2");
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites("*RXMRD_3");
	_globals._spriteIndexes[15] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 7, 0, 1, 0);
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 2, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 0);

	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 25);	
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SM_FRAME_INDEX, 2, 72);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 1, 37);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SM_FRAME_INDEX, 2, 73);

	_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8);
	_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6);
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6);

	if (_game._objects.isInRoom(OBJ_TIMER_MODULE))
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6);
	else
		_vm->_game->_scene._hotspots.activate(371, false);

	if (_game._objects.isInRoom(OBJ_REBREATHER))
		_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 6);
	else
		_vm->_game->_scene._hotspots.activate(289, false);
	
	if (_globals[kTurkeyExploded]) {
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], -2, -2);
		_scene->_hotspots.activate(362, false);
	}

	if (_scene->_priorSceneId != -2)
		_game._player._playerPos = Common::Point(237, 74);

	if (_scene->_priorSceneId == 102) {
		_game._player._stepEnabled = false;

		_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SM_0, 0, 70);
	}

	sceneEntrySound();
	_vm->_game->loadQuoteSet(70, 51, 71, 7, 73, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		int msgIndex = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(70));
		_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
	}

	if (_scene->_priorSceneId == 102)
		_vm->_sound->command(20);

	_vm->_palette->setEntry(252, 63, 63, 10);
	_vm->_palette->setEntry(253, 45, 45, 10);
	_globals._v0 = 0;
	_globals._frameTime = _scene->_frameStartTime;
}

void Scene103::step() {
	Common::Point pt;
	int dist;

	switch (_vm->_game->_trigger) {
	case 70:
		_vm->_game->_player._stepEnabled = true;
		break;

	case 72:
		pt = _vm->_game->_player._playerPos;
		dist = _vm->hypotenuse(pt.x - 58, pt.y - 93);
		_vm->_sound->command(27, (dist * -128 / 378) + 127);
		break;

	case 73:
		pt = _vm->_game->_player._playerPos;
		dist = _vm->hypotenuse(pt.x - 266, pt.y - 81);
		_vm->_sound->command(27, (dist * -127 / 378) + 127);
		break;

	default:
		break;
	}
	
	if (_globals._frameTime <= _scene->_frameStartTime) {
		pt = _vm->_game->_player._playerPos;
		dist = _vm->hypotenuse(pt.x - 79, pt.y - 137);
		_vm->_sound->command(29, (dist * -127 / 378) + 127);

		pt = _vm->_game->_player._playerPos;
		dist = _vm->hypotenuse(pt.x - 69, pt.y - 80);
		_vm->_sound->command(30, (dist * -127 / 378) + 127);

		pt = _vm->_game->_player._playerPos;
		dist = _vm->hypotenuse(pt.x - 266, pt.y - 138);
		_vm->_sound->command(32, (dist * -127 / 378) + 127);

		_globals._frameTime = _scene->_frameStartTime + _vm->_game->_player._ticksAmount;
	}
}

void Scene103::actions() {
	if (_action._savedFields._lookFlag) {
		_vm->_dialogs->show(10322);
	} else if (_action.isAction(NOUN_WALK_THROUGH, NOUN_DOOR)) {
		switch (_vm->_game->_trigger) {
		case 0:
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SM_0, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			break;

		case 1:
			_vm->_sound->command(1);
			_scene->_nextSceneId = 102;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, 371) && _game._objects.isInRoom(OBJ_TIMER_MODULE)) {
		switch (_vm->_game->_trigger) {
		case 0:
			_scene->changeVariant(1);
			_globals._sequenceIndexes[13] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[13], false, 3, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[13]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SM_FRAME_INDEX, 7, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SM_0, 0, 2);
			_vm->_game->_player._visible = false;
			_vm->_game->_player._stepEnabled = false;
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[11]);
			break;

		case 2:
			_vm->_sound->command(22);
			_game._objects.addToInventory(OBJ_TIMER_MODULE);
			_scene->changeVariant(0);
			_scene->drawElements(kTransitionNone, false);
			_scene->_hotspots.activate(371, false);
			_vm->_game->_player._visible = true;
			_vm->_game->_player._stepEnabled = true;
			_vm->_dialogs->showPicture(OBJ_REBREATHER, 805);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, 289, 0) && _game._objects.isInRoom(OBJ_REBREATHER)) {
		switch (_vm->_game->_trigger) {
		case 0:
			_scene->changeVariant(1);
			_globals._sequenceIndexes[12] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[12], false, 3, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SM_FRAME_INDEX, 6, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SM_0, 0, 2);
			_vm->_game->_player._visible = false;
			_vm->_game->_player._stepEnabled = false;
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[10]);
			break;

		case 2:
			_vm->_sound->command(22);
			_game._objects.addToInventory(OBJ_REBREATHER);
			_scene->_hotspots.activate(289, false);
			_vm->_game->_player._visible = true;
			_vm->_game->_player._stepEnabled = true;
			_vm->_dialogs->showPicture(OBJ_REBREATHER, 804);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, 362)) {
		_vm->_dialogs->show(10301);
	} else if (_action.isAction(VERB_TAKE, 362)) {
		// Take Turkey
		if (!_vm->_game->_trigger)
			_vm->_sound->command(31);

		if (_vm->_game->_trigger < 2) {
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, _vm->_game->_trigger < 1 ? 1 : 0);
			if (_vm->_game->_trigger) {
				// Lock the turkey into a permanent "exploded" frame
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], -1, -1);

				// Rex says "Gads.."
				Common::String msg = _game.getQuote(51);
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 18, 0, 60, msg);
				_scene->_sequences.addTimer(120, _vm->_game->_trigger + 1);
			} else {
				// Initial turky explosion
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SM_0, 0, 1);
			}
		}

		// Re-enable player if sequence is ended, and set global flag
		_game._player._stepEnabled = _game._trigger == 2;
		_globals[kTurkeyExploded] = -1;

		if (_game._trigger == 2) {
			// Show exposition dialog at end of sequence
			_vm->_dialogs->show(10302);
			_scene->_hotspots.activate(362, false);
		}
	} else if (_action.isAction(VERB_LOOK, 250)) {
		_vm->_dialogs->show(!_globals[kTurkeyExploded] ? 10323 : 10303);
	} else if (_action.isAction(VERB_TALKTO, 27)) {
		switch (_vm->_game->_trigger) {
		case 0: {
			_game._player._stepEnabled = false;
			Common::String msg = _game.getQuote(71);
			_scene->_kernelMessages.add(Common::Point(), 0x1110, 18, 1, 120, msg);
			break;
		}

		case 1: {
			Common::String msg = _game.getQuote(72);
			_scene->_kernelMessages.add(Common::Point(310, 132), 0xFDFC, 16, 2, 120, msg);
			_globals._v0 = 0;
			break;
		}
			
		case 2:
			_scene->_kernelMessages.reset();
			_scene->_sequences.addTimer(1, 3);
			break;

		case 3:
			_game._player._stepEnabled = true;
			_vm->_dialogs->show(10306);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, 27)) {
		_vm->_dialogs->show(10304);
	} else if (_action.isAction(VERB_LOOK, 36)) {
		_vm->_dialogs->show(10307);
	} else if (_action.isAction(VERB_LOOK, 55)) {
		_vm->_dialogs->show(10308);
	} else if (_action.isAction(VERB_TAKE, 315)) {
		_vm->_dialogs->show(10309);
	} else if (_action.isAction(VERB_TAKE, 85)) {
		_vm->_dialogs->show(10310);
	} else if (_action.isAction(VERB_LOOK, 144)) {
		_vm->_dialogs->show(10312);
	} else if (_action.isAction(VERB_OPEN, 144)) {
		_vm->_dialogs->show(10313);
	} else if (_action.isAction(VERB_CLOSE, 27)) {
		_vm->_dialogs->show(10314);
	} else if (_action.isAction(VERB_LOOK, 310)) {
		_vm->_dialogs->show(10315);
	} else if (_action.isAction(VERB_LOOK, 178)) {
		_vm->_dialogs->show(10316);
	} else if (_action.isAction(VERB_LOOK, 283)) {
		_vm->_dialogs->show(10317);
	} else if (_action.isAction(VERB_LOOK, 120)) {
		_vm->_dialogs->show(10318);
	} else if (_action.isAction(VERB_LOOK, 289) &&
			_game._objects.isInInventory(OBJ_REBREATHER)) {
		_vm->_dialogs->show(10319);
	} else if (_action.isAction(VERB_LOOK, 371) &&
			_game._objects.isInInventory(OBJ_TIMER_MODULE)) {
		_vm->_dialogs->show(10320);
	} else if (_action.isAction(VERB_LOOK, 137)) {
		_vm->_dialogs->show(10321);
	} else if (_action.isAction(VERB_LOOK, 409)) {
		_vm->_dialogs->show(_game._objects.isInInventory(OBJ_TIMER_MODULE) ? 10324 : 10325);
	} else {
		return;
	}

	_action._inProgress = false;
}

void Scene103::postActions() {
	if (_action.isAction(27) && !_action.isAction(VERB_WALKTO)) {
		_vm->_dialogs->show(0x2841);
		_action._inProgress = false;
	} else {
		if (_action.isAction(VERB_PUT, 85, 144)) {
			Common::String msg = _game.getQuote(73);
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110,
				34, 0, 120, msg);
			_action._inProgress = false;
		}
	}
}

/*------------------------------------------------------------------------*/

void Scene104::setup() {
	// Preloading has been skipped
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene104::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('h', -1));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 14, 0, 0, 1);

	if (_scene->_priorSceneId == 105)
		_game._player._playerPos = Common::Point(302, 107);
	else if (_scene->_priorSceneId != -2)
		_game._player._playerPos = Common::Point(160, 134);

	_loseFl = false;
	_game.loadQuoteSet(0x35, 0x34, 0);
	_kargShootingFl = false;

	if (_vm->getRandomNumber(1, 3) == 1) {
		_scene->loadAnimation(Resources::formatName(104, 'B', -1, EXT_AA, ""), 0);
		_kargShootingFl = true;
	}

	sceneEntrySound();
}

void Scene104::step() {
	if ((_game._player._playerPos == Common::Point(189, 70)) && (_game._trigger || !_loseFl)) {
		if (_game._player._facing == FACING_SOUTHWEST || _game._player._facing == FACING_SOUTHEAST)
			_game._player._facing = FACING_SOUTH;

		if (_game._player._facing == FACING_NORTHWEST || _game._player._facing == FACING_NORTHEAST)
			_game._player._facing = FACING_NORTH;

		bool mirrorFl = false;
		if (_game._player._facing == FACING_WEST) {
			_game._player._facing = FACING_EAST;
			mirrorFl = true;
		}

		_loseFl = true;

		switch (_game._player._facing) {
		case FACING_EAST:
			switch (_game._trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->_activeAnimation->free();
				_scene->resetScene();
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 0));
				_vm->_palette->refreshHighColors();
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], mirrorFl, 7, 1, 0, 0);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[2], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SM_0, 0, 1);
				break;

			case 1:
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], mirrorFl, 7, 0, 0, 0);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[2], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -2, -2);
				_scene->_sequences.addTimer(90, 2);
				break;

			case 2:
				_vm->_dialogs->show(0x28A6);
				_scene->_reloadSceneFlag = true;
				break;
			}
			break;

		case FACING_SOUTH:
			switch (_game._trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->_activeAnimation->free();
				_scene->resetScene();
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 1));
				_vm->_palette->refreshHighColors();
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 14);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SM_0, 0, 1);
				break;

			case 1:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1, 0, 0);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 15, 32);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SM_0, 0, 2);
				break;

			case 2:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 3, 0, 0, 0);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
				_scene->_sequences.addTimer(90, 3);
				break;

			case 3:
				_vm->_dialogs->show(0x28A6);
				_scene->_reloadSceneFlag = true;
				break;
			}
			break;

		case FACING_NORTH:
			switch (_game._trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->_activeAnimation->free();
				_scene->resetScene();
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 2));
				_vm->_palette->refreshHighColors();
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 8, 1, 0, 0);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SM_0, 0, 1);
				if (_game._storyMode >= STORYMODE_NICE)
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SM_FRAME_INDEX, 15, 2);
				break;

			case 1:
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 8, 0, 0, 0);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -2, -2);
				_scene->_sequences.addTimer(90, 2);
				break;

			case 2:
				_vm->_dialogs->show(0x28A6);
				_scene->_reloadSceneFlag = true;
				break;
			}
			break;
		default:
			break;
		}

		if (!_game._trigger)
			_vm->_sound->command(34);
	}

	if (_game._player._moving && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(189, 70), FACING_NONE);
		_scene->_rails.resetNext();
	}

	if ((_game._player._special > 0) && _game._player._stepEnabled)
		_game._player._stepEnabled = false;

	if (_kargShootingFl && (_scene->_activeAnimation->getCurrentFrame() >= 19)) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(52));
		_kargShootingFl = false;
	}
}

void Scene104::preActions() {
	if (_action.isAction(0x15D, 0x74))
		_game._player._walkOffScreenSceneId = 105;

	if (_action.isAction(0x15D, 0x75))
		_game._player._walkOffScreenSceneId = 106;
}

void Scene104::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(0x28A5);
	else if (_action.isAction(VERB_LOOK, 0x5E))
		_vm->_dialogs->show(0x28A4);
	else if (_action.isAction(VERB_LOOK, 0x158))
		_vm->_dialogs->show(0x28A3);
	else if (_action.isAction(VERB_LOOK, 0x4D))
		_vm->_dialogs->show(0x28A1);
	else if (_action.isAction(VERB_LOOK, 0xF0))
		_vm->_dialogs->show(0x28A2);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene105::setup() {
	// Preloading has been skipped
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene105::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('m', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 4));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 0, 0, 0);

	if (_globals[kFishIn105]) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 0, 0);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(48, 144));

		int idx = _scene->_dynamicHotspots.add(101, 348, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(56, 141), FACING_NORTHWEST);
	}

	if (_scene->_priorSceneId == 104)
		_game._player._playerPos = Common::Point(13, 97);
	else if (_scene->_priorSceneId != -2)
		_game._player._playerPos = Common::Point(116, 147);

	_game.loadQuoteSet(0x4A, 0x4B, 0x4C, 0x35, 0x34, 0);
	_explosionFl = false;

	sceneEntrySound();
}

void Scene105::step() {
	if ((_game._player._playerPos == Common::Point(170, 87)) && (_game._trigger || !_explosionFl)) {
		_explosionFl = true;
		switch (_game._trigger) {
		case 0:
			_scene->_kernelMessages.reset();
			_scene->resetScene();
			_game._player._stepEnabled = false;
			_game._player._visible = false;

			_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('m', 0));
			_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('m', 2));
			_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('m', 3));
			_vm->_sound->command(33);
			_scene->clearSequenceList();
			_vm->_palette->refreshHighColors();

			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SM_0, 0, 1);

			if (_game._storyMode >= STORYMODE_NICE)
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SM_FRAME_INDEX, 8, 3);
			break;

		case 1:
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 9, 1, 0, 0);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], _globals._sequenceIndexes[0]);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 5, 7);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SM_0, 0, 2);
			break;

		case 2: {
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 9, 0, 0, 0);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
			_scene->_sequences.addTimer(90, 3);
			}
			break;

		case 3:
			_vm->_dialogs->show(0x290B);
			_scene->_reloadSceneFlag = true;
			_scene->_sequences.addTimer(90, 4);
			break;
		}
	}

	if (_game._player._moving && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(170, 87), FACING_NONE);
		_scene->_rails.resetNext();
	}

	if ((_game._player._special > 0) && _game._player._stepEnabled)
		_game._player._stepEnabled = false;
}

void Scene105::preActions() {
	if (_action.isAction(0x15D, 0x195))
		_game._player._walkOffScreenSceneId = 104;

	if (_action.isAction(0x15D, 0xF5))
		_game._player._walkOffScreenSceneId = 107;

	if (_action.isAction(0xE0) && (_action.isAction(VERB_TALKTO) || _action.isAction(VERB_LOOK)))
		_game._player._needToWalk = false;
}

void Scene105::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(0x2910);
	else if (_action.isAction(VERB_TAKE, 0x65) && _globals[kFishIn105]) {
		if (_game._objects.isInInventory(OBJ_DEAD_FISH)) {
			int randVal = _vm->getRandomNumber(74, 76);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		} else {
			_scene->_sequences.remove (_globals._sequenceIndexes[4]);
			_game._objects.addToInventory(OBJ_DEAD_FISH);
			_globals[kFishIn105] = false;
			_vm->_dialogs->showPicture(OBJ_DEAD_FISH, 0x322, 0);
		}
	} else if (_action.isAction(VERB_LOOK, 0x195))
		_vm->_dialogs->show(0x2905); 
	else if (_action.isAction(VERB_LOOK, 0x4D))
		_vm->_dialogs->show(0x2906); 
	else if (_action.isAction(VERB_LOOK, 0xF0))
		_vm->_dialogs->show(0x2907); 
	else if (_action.isAction(VERB_LOOK, 0xDD))
		_vm->_dialogs->show(0x2908); 
	else if (_action.isAction(VERB_TAKE, 0xDD))
		_vm->_dialogs->show(0x2909); 
	else if (_action.isAction(VERB_LOOK, 0xE0))
		_vm->_dialogs->show(0x290A); 
	else if (_action.isAction(VERB_LOOK, 0x65))
		_vm->_dialogs->show(0x290C); 
	else if (_action.isAction(VERB_LOOK, 0x158))
		_vm->_dialogs->show(0x290D); 
	else if (_action.isAction(VERB_LOOK, 0xF5))
		_vm->_dialogs->show(0x290E); 
	else if (_action.isAction(VERB_LOOK, 0x129))
		_vm->_dialogs->show(0x290F); 
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene106::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if ((_scene->_priorSceneId == 102) && !_game._objects.isInInventory(OBJ_REBREATHER) && !_scene->_roomChanged)
		_game._player._spritesPrefix = "";

	_vm->_dialogs->_defaultPosition.y = 100;
}

void Scene106::enter() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('H', -1));

	if (_game._objects.isInInventory(OBJ_REBREATHER) || (_scene->_priorSceneId != 102) || _scene->_roomChanged) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('A', 0));
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('A', 1));
	}

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('G', -1));
	_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 21, 0, 0, 0);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('I', -1));
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 32, 47);

	if (_scene->_priorSceneId == 102) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 1, 4, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SM_0, 0, 70);
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_game._player._facing = FACING_EAST;
		_game._player._playerPos = Common::Point(106, 69);
	} else if (_scene->_priorSceneId != -2) {
		if (_scene->_priorSceneId == 107) {
			_game._player._playerPos = Common::Point(319, 84);
			_game._player._facing = _game._player._prepareWalkFacing = FACING_WEST;
		} else {
			_game._player._playerPos = Common::Point(319, 44);
			_game._player._facing = _game._player._prepareWalkFacing = FACING_SOUTHWEST;
			_scene->_sprites[_game._player._spritesStart + 3]->_charInfo->_velocity = 24;
		}

		_game._player._prepareWalkPos = Common::Point(246, 69);
		_game._player._needToWalk = true;
		_game._player._readyToWalk = true;
	}

	if (_scene->_priorSceneId != 102) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	}

	_backToShipFl = false;
	_shadowFl = false;
	_firstEmergingFl = false;

	_game.loadQuoteSet(0x31, 0x32, 0x34, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0);
	sceneEntrySound();
}

void Scene106::step() {
	if (_game._trigger == 70) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);

		if (!_game._objects.isInInventory(OBJ_REBREATHER) && !_scene->_roomChanged) {
			_scene->loadAnimation(Resources::formatName(106, 'A', -1, EXT_AA, ""), 75);
		} else {
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SM_FRAME_INDEX, 28, 71);
		}
	}

	if (_game._trigger == 71) {
		_game._player._prepareWalkPos = Common::Point(246, 69);
		_game._player._prepareWalkFacing = FACING_EAST;
		_game._player._needToWalk = true;
		_game._player._readyToWalk = true;
		_game._player._visible = true;

		if (_game._visitedScenes._sceneRevisited) {
			_game._player._stepEnabled = true;
		} else {
			_game._player._prepareWalkFacing = FACING_SOUTHWEST;
			_firstEmergingFl = true;
			_scene->loadAnimation(Resources::formatName(106, 'B', -1, EXT_AA, ""), 80);
		}
	}

	if (_firstEmergingFl && (_scene->_activeAnimation->getCurrentFrame() >= 19)) {
		_firstEmergingFl = false;
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(52));
	}

	if ((_game._trigger >= 80) && (_game._trigger <= 87)) {
		int tmpVal = _game._trigger - 80;
		int msgId = -1;
		switch (tmpVal) {
		case 0:
			_msgPosY = 26;
			msgId = 49;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			msgId = 76 + tmpVal;
			break;

		case 6:
			msgId = 50;
			break;

		default:
			msgId = -1;
			_game._player._stepEnabled = true;
			break;
		}

		if (msgId >= 0) {
			int nextAbortVal = _game._trigger + 1;
			_scene->_kernelMessages.add(Common::Point(15, _msgPosY), 0x1110, 0, 0, 360, _game.getQuote(msgId));
			_scene->_sequences.addTimer(150, nextAbortVal);
			_msgPosY += 14;
		}
	}

	if (_backToShipFl) {
		if (!_shadowFl) {
			if (_game._player._playerPos.x < 204) {
				_shadowFl = true;
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 4, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SM_0, 0, 72);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SM_FRAME_INDEX, 44, 73);
			}
		} else if (_game._trigger == 73)
				_game._player._visible = false;
		else if (_game._trigger == 72)
				_scene->_sequences.addTimer(24, 74);
		else if (_game._trigger == 74)
				_scene->_nextSceneId = 102;
	}

	if (_game._trigger == 75) {
		_game._visitedScenes.pop_back();
		_scene->_nextSceneId = 102;
	}
}

void Scene106::preActions() {
	if (_action.isAction(0x15D, 0x12E) || _action.isAction(0x15D, 0x130)) {
		_game._player._stepEnabled = false;
		_scene->_sprites[_game._player._spritesStart + 1]->_charInfo->_velocity = 24;
		_game._player._walkOffScreenSceneId = 104;
	}

	if (_action.isAction(0x15D, 0xF3))
		_game._player._walkOffScreenSceneId = 107;
}

void Scene106::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(0x2976);
	else if (_action.isAction(0x15C, 0xD9)) {
		_game._player._stepEnabled = false;
		_game._player._prepareWalkPos = Common::Point(95, 72);
		_game._player._prepareWalkFacing = FACING_WEST;
		_game._player._needToWalk = true;
		_game._player._readyToWalk = true;
		_game._player._frameNumber = 9;
		_backToShipFl = true;
	} else if (_action.isAction(VERB_LOOK, 0x13) || _action.isAction(0xD1, 0x13))
		_vm->_dialogs->show(0x2969);
	else if (_action.isAction(VERB_TAKE, 0x13))
		_vm->_dialogs->show(0x296A);
	else if (_action.isAction(VERB_LOOK, 0x12F) || _action.isAction(VERB_LOOK, 0x130))
		_vm->_dialogs->show(0x296B);
	else if (_action.isAction(VERB_TAKE, 0x12F) || _action.isAction(VERB_TAKE, 0x130))
		_vm->_dialogs->show(0x296C);
	else if (_action.isAction(VERB_LOOK, 0xF3))
		_vm->_dialogs->show(0x296D);
	else if (_action.isAction(VERB_LOOK, 0x109) || _action.isAction(0xD1, 0x109))
		_vm->_dialogs->show(0x296E);
	else if (_action.isAction(0x109) && (_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL) || _action.isAction(VERB_TAKE)))
		_vm->_dialogs->show(0x296F);
	else if (_action.isAction(VERB_LOOK, 0x139) || _action.isAction(0xD1, 0x139))
		_vm->_dialogs->show(0x2970);
	else if (_action.isAction(VERB_LOOK, 0xD9))
		_vm->_dialogs->show(0x2971);
	else if (_action.isAction(VERB_OPEN, 0xD9))
		_vm->_dialogs->show(0x2972);
	else if (_action.isAction(VERB_CLOSE, 0xD9))
		_vm->_dialogs->show(0x2973);
	else if (_action.isAction(VERB_LOOK, 0x12E))
		_vm->_dialogs->show(0x2974);
	else if (_action.isAction(VERB_LOOK, 0xF0))
		_vm->_dialogs->show(0x2975);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene107::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(0xDA);
}

void Scene107::enter() {
	for (int i = 0; i < 3; i++)
		_globals._spriteIndexes[i + 1] = _scene->_sprites.addSprites(formAnimName('G', i));

	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(Resources::formatName(105, 'f', 4, EXT_SS, ""));

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 14, 0, 0, 7);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 17, 0, 0, 13);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 19, 0, 0, 9);

	for (int i = 1; i < 4; i++)
		_scene->_sequences.setDepth(_globals._sequenceIndexes[i], 0);

	if (_globals[kFishIn107]) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 0, 0);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(68, 151));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		int idx = _scene->_dynamicHotspots.add(101, 348, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(78, 135), FACING_SOUTHWEST);
	}

	if (_scene->_priorSceneId == 105)
		_game._player._playerPos = Common::Point(132, 47);
	else if (_scene->_priorSceneId == 106)
		_game._player._playerPos = Common::Point(20, 91);
	else if (_scene->_priorSceneId != -2)
		_game._player._playerPos = Common::Point(223, 151);

	if (((_scene->_priorSceneId == 105) || (_scene->_priorSceneId == 106)) && (_vm->getRandomNumber(1, 3) == 1)) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(Resources::formatName(105, 'R', 1, EXT_SS, ""));
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], true, 4, 0, 0, 0);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[0], Common::Point(270, 150));
		_scene->_sequences.sub70C52(_globals._sequenceIndexes[0], SM_FRAME_INDEX, -200, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
		_scene->_dynamicHotspots.add(218, 348, _globals._sequenceIndexes[0], Common::Rect(0, 0, 0, 0));
	}

	_game.loadQuoteSet(0x4A, 0x4B, 0x4C, 0x35, 0x34, 0);
	_shootingFl = false;

	if (_vm->getRandomNumber(1, 3) == 1) {
		_scene->loadAnimation(Resources::formatName(107, 'B', -1, EXT_AA, ""), 0);
		_shootingFl = true;
	}

	sceneEntrySound();
}

void Scene107::step() {
	if (_shootingFl && (_scene->_activeAnimation->getCurrentFrame() >= 19)) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(52));
		_shootingFl = false;
	}
}

void Scene107::preActions() {
	if (_action.isAction(0x15D, 0xF6))
		_game._player._walkOffScreenSceneId = 106;

	if (_action.isAction(0x15D, 0xF5))
		_game._player._walkOffScreenSceneId = 108;
}

void Scene107::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(0x29D4);
	else if (_action.isAction(VERB_TAKE, 0x65) && _globals[kFishIn107]) {
		if (_game._objects.isInInventory(OBJ_DEAD_FISH)) {
			int randVal = _vm->getRandomNumber(74, 76);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_game._objects.addToInventory(OBJ_DEAD_FISH);
			_globals[kFishIn107] = false;
			_vm->_dialogs->showPicture(OBJ_DEAD_FISH, 0x322);
		}
	} else if (_action.isAction(0x15D, 0xEE))
		_scene->_nextSceneId = 105;
	else if (_action.isAction(VERB_LOOK, 0xEE))
		_vm->_dialogs->show(0x29CD);
	else if (_action.isAction(VERB_LOOK, 0x65) && (_action._mainObjectSource == 4))
		_vm->_dialogs->show(0x29CE);
	else if (_action.isAction(VERB_LOOK, 0x38))
		_vm->_dialogs->show(0x29CF);
	else if (_action.isAction(VERB_LOOK, 0x128))
		_vm->_dialogs->show(0x29D0);
	else if (_action.isAction(VERB_LOOK, 0x12F))
		_vm->_dialogs->show(0x29D1);
	else if (_action.isAction(VERB_LOOK, 0xF5))
		_vm->_dialogs->show(0x29D2);
	else if (_action.isAction(VERB_LOOK, 0x4D))
		_vm->_dialogs->show(0x29D3);
	else if (_action.isAction(VERB_LOOK, 0xDA))
		_vm->_dialogs->show(0x29D5);
	else if (_action.isAction(VERB_TAKE, 0xDA))
		_vm->_dialogs->show(0x29D6);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
