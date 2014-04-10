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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
		_scene->_sequences.remove(_globals._spriteIndexes[26]);
		_globals._spriteIndexes[26] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[11], false, 3, 6, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[26], 17, 21);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[26], SM_0, 0, 72);
		_vm->_sound->command(17);
		_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 3, 2, 0, 0);
		break;

	case 72:
		_globals._spriteIndexes[26] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[26], 17, 17);
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
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1), false);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2), false);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3), false);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4), false);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5), false);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 6), false);
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('x', 7), false);
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('m', -1), false);
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('b', 1), false);
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('b', 2), false);
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 0), false);
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('a', 1), false);
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('x', 8), false);
	_globals._spriteIndexes[14] = _scene->_sprites.addSprites(formAnimName('x', 0), false);

	_globals._spriteIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 0, 0, 25);
	_globals._spriteIndexes[17] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 0, 1, 0);
	_globals._spriteIndexes[18] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 2, 0);
	_scene->_sequences.addSubEntry(_globals._spriteIndexes[18], SM_FRAME_INDEX, 7, 70);
	_globals._spriteIndexes[19] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 10, 0, 0, 60);
	_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 5, 0, 1, 0);
	_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 0, 2, 0);
	_globals._spriteIndexes[22] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 0, 0, 0);
	_globals._spriteIndexes[24] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, 0, 10, 4);
	_globals._spriteIndexes[25] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 6, 0, 32, 47);

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
		_globals._spriteIndexes[26] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[26], 17, 17);
		_scene->_hotspots.activate(0x47, false);
		_chairHotspotId = _scene->_dynamicHotspots.add(0x47, 0x13F, -1, Common::Rect(159, 84, 33, 36));
		if (_scene->_priorSceneId == 112)
			sayDang();
	} else {
		_globals._spriteIndexes[27] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._spriteIndexes[27], 4);
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
				_scene->_sequences.remove(_globals._spriteIndexes[26]);
				_globals._spriteIndexes[26] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[26], SM_0, 0, 1);
				_scene->_sequences.setAnimRange(_globals._spriteIndexes[26], 1, 17);
				_vm->_sound->command(16);
				break;

			case 1:
				_sittingFl = false;
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_game._player._needToWalk = true;
				_scene->_hotspots.activate(71, true);
				_scene->_dynamicHotspots.remove(_chairHotspotId);
				_globals._spriteIndexes[27] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 6, 0, 0, 0);
				_scene->_sequences.setDepth(_globals._spriteIndexes[27], 4);
				break;
			}
		}
	}

	if (_panelOpened && !(_action.isAction(0x135) || _action.isAction(0x137))) {
		switch (_game._trigger) {
		case 0:
			if (_game._player._needToWalk) {
				_scene->_sequences.remove(_globals._spriteIndexes[28]);
				_shieldSpriteIdx = _game._objects.isInRoom(0x18) ? 13 : 14;
				_globals._spriteIndexes[28] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[_shieldSpriteIdx], false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[28], SM_0, 0, 1);
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
				_scene->_sequences.remove(_globals._spriteIndexes[27]);
				_globals._spriteIndexes[26] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 1);
				_scene->_sequences.setAnimRange(_globals._spriteIndexes[26], 1, 17);
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[26], SM_FRAME_INDEX, 10, 1);
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[26], SM_0, 0, 2);
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_action._inProgress = false;
				return;

			case 1:
				_vm->_sound->command(16);
				break;

			case 2:
				_globals._spriteIndexes[26] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._spriteIndexes[26], 17, 17);
				_game._player._stepEnabled = true;
				_sittingFl = true;
				_scene->_hotspots.activate(71, false);
				_chairHotspotId = _scene->_dynamicHotspots.add(71, 0x13F, -1, Common::Rect(159, 84, 33, 36));
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
			_globals._spriteIndexes[28] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[_shieldSpriteIdx], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._spriteIndexes[28], SM_0, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			break;

		case 1:
			_scene->_sequences.remove(_globals._spriteIndexes[28]);
			_globals._spriteIndexes[28] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[_shieldSpriteIdx], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._spriteIndexes[28], -2, -2);
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
		_scene->_sequences.remove(_globals._spriteIndexes[28]);
		_globals._spriteIndexes[28] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[14], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[28], -2, -2);
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
				_scene->_sequences.remove(_globals._spriteIndexes[26]);
				_globals._spriteIndexes[26] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._spriteIndexes[26], 17, 21);
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[26], SM_0, 0, 1);
				_vm->_sound->command(17);
				break;

			case 1:
				_globals._spriteIndexes[26] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[26], SM_0, 0, 2);
				_scene->_sequences.setAnimRange(_globals._spriteIndexes[26], 17, 21);
				break;

			case 2:
				_globals._spriteIndexes[26] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._spriteIndexes[26], 17, 17);
				_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[23], SM_0, 0, 3);
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

}

void Scene102::enter() {

}

void Scene102::step() {

}

void Scene102::preActions() {

}

void Scene102::actions() {

}

void Scene102::postActions() {

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
	_globals._spriteIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 2, 0);
	_scene->_sequences.setDepth(_globals._spriteIndexes[16], 0);

	_globals._spriteIndexes[17] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 25);	
	_scene->_sequences.addSubEntry(_globals._spriteIndexes[17], SM_FRAME_INDEX, 2, 72);
	_globals._spriteIndexes[18] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 1, 37);
	_scene->_sequences.addSubEntry(_globals._spriteIndexes[18], SM_FRAME_INDEX, 2, 73);

	_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8);
	_globals._spriteIndexes[22] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6);
	_globals._spriteIndexes[19] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6);
	_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6);

	if (_game._objects.isInRoom(OBJ_TIMER_MODULE))
		_globals._spriteIndexes[26] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6);
	else
		_vm->_game->_scene._hotspots.activate(371, false);

	if (_game._objects.isInRoom(OBJ_REBREATHER))
		_globals._spriteIndexes[25] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 6);
	else
		_vm->_game->_scene._hotspots.activate(289, false);
	
	if (_globals[kTurkeyExploded]) {
		_globals._spriteIndexes[24] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[24], -2, -2);
		_scene->_hotspots.activate(362, false);
	}

	if (_scene->_priorSceneId != -2)
		_game._player._playerPos = Common::Point(237, 74);

	if (_scene->_priorSceneId == 102) {
		_game._player._stepEnabled = false;

		_globals._spriteIndexes[21] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_0, 0, 70);
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
		int dist = _vm->hypotenuse(pt.x - 79, pt.y - 137);
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

void Scene103::preActions() {
}

void Scene103::actions() {
	if (_action._savedFields._lookFlag) {
		_vm->_dialogs->show(10322);
	} else if (_action.isAction(395, 110)) {
		switch (_vm->_game->_trigger) {
		case 0:
			_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1);
			_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_0, 0, 1);
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
			_globals._spriteIndexes[28] = _scene->_sequences.startReverseCycle(
				_globals._spriteIndexes[13], false, 3, 2);
			_scene->_sequences.setMsgLayout(_globals._spriteIndexes[28]);

			_scene->_sequences.addSubEntry(_globals._spriteIndexes[28], SM_FRAME_INDEX, 7, 1);
			_scene->_sequences.addSubEntry(_globals._spriteIndexes[28], SM_0, 0, 2);
			_vm->_game->_player._visible = false;
			_vm->_game->_player._stepEnabled = false;
			break;

		case 1:
			_scene->_sequences.remove(_globals._spriteIndexes[26]);
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
			_globals._spriteIndexes[27] = _scene->_sequences.startReverseCycle(
				_globals._spriteIndexes[12], false, 3, 2);
			_scene->_sequences.setMsgLayout(_globals._spriteIndexes[27]);

			_scene->_sequences.addSubEntry(_globals._spriteIndexes[27], SM_FRAME_INDEX, 6, 1);
			_scene->_sequences.addSubEntry(_globals._spriteIndexes[27], SM_0, 0, 2);
			_vm->_game->_player._visible = false;
			_vm->_game->_player._stepEnabled = false;
			break;

		case 1:
			_scene->_sequences.remove(_globals._spriteIndexes[25]);
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
			_globals._spriteIndexes[24] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, _vm->_game->_trigger < 1 ? 1 : 0);
			if (_vm->_game->_trigger) {
				// Lock the turkey into a permanent "exploded" frame
				_scene->_sequences.setAnimRange(_globals._spriteIndexes[24], -1, -1);

				// Rex says "Gads.."
				Common::String msg = _game.getQuote(51);
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 18, 0, 60, msg);
				_scene->_sequences.addTimer(120, _vm->_game->_trigger + 1);
			} else {
				// Initial turky explosion
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[24], SM_0, 0, 1);
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

} // End of namespace Nebular

} // End of namespace MADS
