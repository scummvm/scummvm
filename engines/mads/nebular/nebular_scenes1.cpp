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
#include "common/math.h"
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
		if (_globals[kSexOfRex] == SEX_FEMALE)
			_game._player._spritesPrefix = "ROX";
		else {
			_game._player._spritesPrefix = "RXM";
			_globals[kSexOfRex] = SEX_MALE;
		}
	} else if (_scene->_nextSceneId <= 110) {
		_game._player._spritesPrefix = "RXSW";
		_globals[kSexOfRex] = SEX_UNKNOWN;
	} else if (_scene->_nextSceneId == 112)
		_game._player._spritesPrefix = "";

	if (oldName != _game._player._spritesPrefix)
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

Scene101::Scene101(MADSEngine *vm) : Scene1xx(vm) {
	_sittingFl = false;
	_panelOpened = false;
	_messageNum = 0;
	_posY = 0;
	_shieldSpriteIdx = 0;
	_chairHotspotId = 0;
	_oldSpecial = 0;
}

void Scene101::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_sittingFl);
	s.syncAsByte(_panelOpened);
	s.syncAsSint16LE(_messageNum);
	s.syncAsSint16LE(_posY);
	s.syncAsSint16LE(_shieldSpriteIdx);
	s.syncAsSint16LE(_chairHotspotId);
	s.syncAsSint16LE(_oldSpecial);
}

void Scene101::setup() {
	_scene->_animationData->preLoad(formAnimName('A', -1), 3);
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene101::sayDang() {
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
	_game._player._stepEnabled = false;

	switch (_game._trigger) {
	case 0:
		_scene->_sequences.remove(_globals._sequenceIndexes[11]);
		_globals._sequenceIndexes[11] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[11], false, 3, 6, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 21);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
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
		_vm->_dialogs->show(10117);
		_game._player._stepEnabled = true;
		break;

	default:
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
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 7, 70);
	_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 10, 0, 0, 60);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 5, 0, 1, 0);
	_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 0, 2, 0);
	_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 0, 0, 0);
	_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, 0, 10, 4);
	_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 6, 0, 32, 47);

	_scene->_hotspots.activate(NOUN_SHIELD_MODULATOR, false);
	_panelOpened = false;

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING)
		_globals[kNeedToStandUp] = false;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(100, 152);

	if ((_scene->_priorSceneId == 112) || ((_scene->_priorSceneId == RETURNING_FROM_DIALOG) && _sittingFl )) {
		_game._player._visible = false;
		_sittingFl = true;
		_game._player._playerPos = Common::Point(161, 123);
		_game._player._facing = FACING_NORTHEAST;
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 17);
		_scene->_hotspots.activate(NOUN_CHAIR, false);
		_chairHotspotId = _scene->_dynamicHotspots.add(NOUN_CHAIR, VERB_SIT_IN, -1, Common::Rect(159, 84, 159 + 33, 84 + 36));
		if (_scene->_priorSceneId == 112)
			sayDang();
	} else {
		_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 4);
	}

	_game.loadQuoteSet(0x31, 0x32, 0x39, 0x36, 0x37, 0x38, 0);

	if (_globals[kNeedToStandUp]) {
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
		_globals[kNeedToStandUp] = false;
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		break;

	case 72:
	case 73:
		sayDang();
		break;

	default:
		break;
	}

	if (_scene->_animation[0] != nullptr) {
		if ((_scene->_animation[0]->getCurrentFrame() >= 6) && (_messageNum == 0)) {
			_messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, _posY), 0x1110, 0, 0, 240, _game.getQuote(49));
			_posY += 14;
		}

		if ((_scene->_animation[0]->getCurrentFrame() >= 7) && (_messageNum == 1)) {
			_messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, _posY), 0x1110, 0, 0, 240, _game.getQuote(54));
			_posY += 14;
		}

		if ((_scene->_animation[0]->getCurrentFrame() >= 10) && (_messageNum == 2)) {
			_messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, _posY), 0x1110, 0, 0, 240, _game.getQuote(55));
			_posY += 14;
		}

		if ((_scene->_animation[0]->getCurrentFrame() >= 17) && (_messageNum == 3)) {
			_messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, _posY), 0x1110, 0, 0, 240, _game.getQuote(56));
			_posY += 14;
		}

		if ((_scene->_animation[0]->getCurrentFrame() >= 20) && (_messageNum == 4)) {
			_messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, _posY), 0x1110, 0, 0, 240, _game.getQuote(50));
			_posY += 14;
		}
	}
}

void Scene101::preActions() {
	if (_action.isAction(VERB_LOOK, NOUN_VIEW_SCREEN))
		_game._player._needToWalk = true;

	if (_sittingFl) {
		if (_action.isAction(VERB_LOOK) || _action.isObject(NOUN_CHAIR) || _action.isAction(VERB_TALKTO) || _action.isAction(VERB_PEER_THROUGH) || _action.isAction(VERB_EXAMINE))
			_game._player._needToWalk = false;

		if (_game._player._needToWalk) {
			switch (_game._trigger) {
			case 0:
				_game._player._readyToWalk = false;
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[11]);
				_globals._sequenceIndexes[11] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 1, 17);
				_vm->_sound->command(16);
				break;

			case 1:
				_sittingFl = false;
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_game._player._readyToWalk = true;
				_scene->_hotspots.activate(71, true);
				_scene->_dynamicHotspots.remove(_chairHotspotId);
				_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 6, 0, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 4);
				break;

			default:
				break;
			}
		}
	}

	if (_panelOpened && !(_action.isObject(NOUN_SHIELD_ACCESS_PANEL) || _action.isObject(NOUN_SHIELD_MODULATOR))) {
		switch (_game._trigger) {
		case 0:
			if (_game._player._needToWalk) {
				_scene->_sequences.remove(_globals._sequenceIndexes[13]);
				_shieldSpriteIdx = _game._objects.isInRoom(OBJ_SHIELD_MODULATOR) ? 13 : 14;
				_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[_shieldSpriteIdx], false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_game._player._stepEnabled = false;
				_vm->_sound->command(20);
			}
			break;

		case 1:
			_game._player._stepEnabled = true;
			_panelOpened = false;
			_scene->_hotspots.activate(NOUN_SHIELD_MODULATOR, false);
			break;

		default:
			break;
		}
	}
}

void Scene101::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(10125);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, NOUN_LIFE_SUPPORT_SECTION)) {
		_scene->_nextSceneId = 102;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_SIT_IN, NOUN_CHAIR) || (_action.isAction(VERB_LOOK, NOUN_VIEW_SCREEN) && !_sittingFl)) {
		if (!_sittingFl) {
			switch (_game._trigger) {
			case 0:
				_scene->_sequences.remove(_globals._sequenceIndexes[12]);
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 1, 17);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_SPRITE, 10, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
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
				_chairHotspotId = _scene->_dynamicHotspots.add(NOUN_CHAIR, VERB_SIT_IN, -1, Common::Rect(159, 84, 159 + 33, 84 + 36));
				if (!_action.isAction(VERB_LOOK, NOUN_VIEW_SCREEN)) {
					_action._inProgress = false;
					return;
				}
				_game._trigger = 0;
				break;

			default:
				break;
			}
		} else {
			_vm->_dialogs->show(10131);
			_action._inProgress = false;
			return;
		}
	}

	if ((_action.isAction(VERB_WALKTO, NOUN_SHIELD_ACCESS_PANEL) || _action.isAction(VERB_OPEN, NOUN_SHIELD_ACCESS_PANEL)) && !_panelOpened) {
		switch (_game._trigger) {
		case 0:
			_shieldSpriteIdx = _game._objects.isInRoom(OBJ_SHIELD_MODULATOR) ? 13 : 14;
			_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[_shieldSpriteIdx], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[13]);
			_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[_shieldSpriteIdx], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], -2, -2);
			_game._player._stepEnabled = true;
			_panelOpened = true;
			if (_game._objects.isInRoom(OBJ_SHIELD_MODULATOR))
				_scene->_hotspots.activate(NOUN_SHIELD_MODULATOR, true);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_TAKE, NOUN_SHIELD_MODULATOR) || _action.isAction(VERB_PULL, NOUN_SHIELD_MODULATOR)) && _game._objects.isInRoom(OBJ_SHIELD_MODULATOR)) {
		_game._objects.addToInventory(OBJ_SHIELD_MODULATOR);
		_scene->_sequences.remove(_globals._sequenceIndexes[13]);
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[14], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], -2, -2);
		_scene->_hotspots.activate(NOUN_SHIELD_MODULATOR, false);
		_vm->_dialogs->showItem(OBJ_SHIELD_MODULATOR, 10120);
		_vm->_sound->command(22);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_SHIELD_ACCESS_PANEL) || (_action.isAction(VERB_LOOK, NOUN_SHIELD_MODULATOR) && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR)) ) {
		if (_panelOpened) {
			if (_game._objects.isInRoom(OBJ_SHIELD_MODULATOR))
				_vm->_dialogs->show(10128);
			else
				_vm->_dialogs->show(10129);
		} else
			_vm->_dialogs->show(10127);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_SHIELD_ACCESS_PANEL) && _panelOpened) {
		_vm->_dialogs->show(10130);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_VIEW_SCREEN) && _sittingFl) {
		if (_globals[kWatchedViewScreen])
			sayDang();
		else {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[11]);
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 21);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_vm->_sound->command(17);
				break;

			case 1:
				_globals._sequenceIndexes[11] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 21);
				break;

			case 2:
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 17);
				_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_game._player._stepEnabled = true;
				_globals[kWatchedViewScreen] = true;
				_sittingFl = true;
				_scene->_nextSceneId = 112;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_CHAIR)) {
		_vm->_dialogs->show(10101);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_LOOK) || _action.isAction(VERB_PEER_THROUGH)) && (_action.isObject(NOUN_FRONT_WINDOW) || _action.isObject(NOUN_OUTSIDE))) {
		_vm->_dialogs->show(10102);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_HULL) || _action.isAction(VERB_LOOK, NOUN_OUTER_HULL) || _action.isAction(VERB_EXAMINE, NOUN_HULL) || _action.isAction(VERB_EXAMINE, NOUN_OUTER_HULL)) {
		_vm->_dialogs->show(10103);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_FUZZY_DICE)) {
		_vm->_dialogs->show(10104);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_MIRROR) || _action.isAction(VERB_LOOK_IN, NOUN_MIRROR)) {
		_vm->_dialogs->show(10105);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_CURTAINS)) {
		_vm->_dialogs->show(10106);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_PLASTIC_JESUS)) {
		_vm->_dialogs->show(10107);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_ESCAPE_HATCH) || (_action.isAction(VERB_OPEN, NOUN_ESCAPE_HATCH) && !_game._objects.isInInventory(OBJ_REBREATHER))) {
		_vm->_dialogs->show(10109);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_ESCAPE_HATCH)) {
		_vm->_dialogs->show(10110);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_TARGET_COMPUTER)) {
		_vm->_dialogs->show(10111);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_LIBRARY_COMPUTER)) {
		_vm->_dialogs->show(10126);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_DAMAGE_CONTROL_PANEL)) {
		_vm->_dialogs->show(10112);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_NAVIGATION_CONTROLS)) {
		_vm->_dialogs->show(10113);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_ENGINEERING_CONTROLS)) {
		_vm->_dialogs->show(10114);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_WEAPONS_DISPLAY)) {
		_vm->_dialogs->show(10115);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_SHIELD_STATUS_PANEL)) {
		_vm->_dialogs->show(10116);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_PLASTIC_JESUS)) {
		_vm->_dialogs->show(10118);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_FUZZY_DICE)) {
		_vm->_dialogs->show(10119);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_DAMAGE_CONTROL_PANEL)) {
		_vm->_dialogs->show(10121);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_CURTAINS)) {
		_vm->_dialogs->show(10122);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_CURTAINS)) {
		_vm->_dialogs->show(10123);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_LOOK) || _action.isAction(VERB_PLAY)) && _action.isObject(NOUN_VIDEO_GAME)) {
		_vm->_dialogs->show(10124);
		_action._inProgress = false;
		return;
	}
}

/*------------------------------------------------------------------------*/

Scene102::Scene102(MADSEngine *vm) : Scene1xx(vm) {
	_fridgeOpenedFl = false;
	_fridgeOpenedDescr = false;
	_fridgeFirstOpenFl = false;
	_chairDescrFl = false;
	_drawerDescrFl = false;
	_activeMsgFl = false;
	_fridgeCommentCount = 0;
}

void Scene102::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_fridgeOpenedFl);
	s.syncAsByte(_fridgeOpenedDescr);
	s.syncAsByte(_fridgeFirstOpenFl);
	s.syncAsByte(_chairDescrFl);
	s.syncAsByte(_drawerDescrFl);
	s.syncAsByte(_activeMsgFl);

	s.syncAsSint16LE(_fridgeCommentCount);
}

void Scene102::setup() {
	_scene->_animationData->preLoad(formAnimName('A', -1), 3);
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene102::addRandomMessage() {
	_scene->_kernelMessages.reset();
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
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

	_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 8, 0, 0, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 170, 0, 1, 6);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 11, 0, 2, 3);
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 0, 1, 0);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 3, 0, 0, 5);

	if (_game._objects.isInRoom(OBJ_BINOCULARS))
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 24, 0, 0, 24);
	else
		_scene->_hotspots.activate(NOUN_BINOCULARS, false);

	_scene->_hotspots.activate(NOUN_BURGER, false);

	if (_globals[kMedicineCabinetOpen]) {
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -2, -2);
	}

	if (_scene->_priorSceneId == 101) {
		_game._player._playerPos = Common::Point(229, 109);
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
	} else if (_scene->_priorSceneId == 103)
		_game._player._playerPos = Common::Point(47, 152);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
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
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
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
			_vm->_dialogs->show(10213);
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
	if (_action.isObject(NOUN_REFRIGERATOR) || _action.isObject(NOUN_POSTER))
		_game._player._needToWalk = _game._player._readyToWalk;

	if (_fridgeOpenedFl && !_action.isObject(NOUN_REFRIGERATOR)) {
		switch (_game._trigger) {
		case 0:
			if (_game._player._needToWalk) {
				_scene->_sequences.remove(_globals._sequenceIndexes[7]);
				_globals._sequenceIndexes[7] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[7], false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
				_game._player._stepEnabled = false;
				_vm->_sound->command(20);
			}
			break;

		case 1:
			if (_game._objects.isInRoom(OBJ_BURGER)) {
				_scene->_sequences.remove(_globals._sequenceIndexes[10]);
				_scene->_hotspots.activate(NOUN_BURGER, false);
			}
			_fridgeOpenedFl = false;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	}

	if (_game._player._needToWalk)
		_scene->_kernelMessages.reset();
}

void Scene102::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(10234);
		_action._inProgress = false;
		return;
	}

	bool justOpenedFl = false;
	if (_action.isObject(NOUN_REFRIGERATOR) && !_fridgeOpenedFl) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
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
				_scene->_hotspots.activate(NOUN_BURGER, true);
			break;

		default:
			break;
		}
	}

	if (_action.isAction(VERB_LOOK, NOUN_REFRIGERATOR) || _action.isAction(VERB_OPEN, NOUN_REFRIGERATOR)) {
		if (_game._objects.isInRoom(OBJ_BURGER))
			_vm->_dialogs->show(10230);
		else
			_vm->_dialogs->show(10229);

		_fridgeFirstOpenFl = false;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, NOUN_REFRIGERATOR) && justOpenedFl) {
		_fridgeFirstOpenFl = false;
		int quoteId = _vm->getRandomNumber(59, 63);
		Common::String curQuote = _game.getQuote(quoteId);
		int width = _scene->_kernelMessages._talkFont->getWidth(curQuote, -1);
		_scene->_kernelMessages.reset();
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_kernelMessages.add(Common::Point(210, 60), 0x1110, 0, 73, 120, curQuote);
		_scene->_kernelMessages.add(Common::Point(214 + width, 60), 0x1110, 0, 73, 120, _game.getQuote(64));
		_activeMsgFl = true;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_REFRIGERATOR)) {
		_vm->_dialogs->show(10213);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_REFRIGERATOR)) {
		_vm->_dialogs->show(8);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR)) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			break;

		case 1:
			_scene->_nextSceneId = 101;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, NOUN_ENGINEERING_SECTION)) {
		_scene->_nextSceneId = 103;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, NOUN_POSTER) || _action.isAction(VERB_LOOK, NOUN_POSTER) || _action.isAction(VERB_WALKTO, NOUN_BINOCULARS)) {
		addRandomMessage();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_WEIGHT_MACHINE)) {
		_vm->_dialogs->show(10212);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_ENGINEERING_SECTION)) {
		_vm->_dialogs->show(10205);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_DOOR)) {
		_vm->_dialogs->show(10204);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_STARE_AT, NOUN_CEILING) || _action.isAction(VERB_LOOK, NOUN_CEILING)) {
		_vm->_dialogs->show(10203);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_STARE_AT, NOUN_OVERHEAD_LAMP) || _action.isAction(VERB_LOOK, NOUN_OVERHEAD_LAMP)) {
		_vm->_dialogs->show(10202);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_ROBO_KITCHEN)) {
		_vm->_dialogs->show(10215);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, NOUN_BURGER, NOUN_ROBO_KITCHEN) && _game._objects.isInInventory(OBJ_BURGER)) {
		_vm->_dialogs->show(10216);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, NOUN_REFRIGERATOR) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		_vm->_dialogs->show(10217);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, NOUN_DEAD_FISH, NOUN_ROBO_KITCHEN) || _action.isAction(VERB_PUT, NOUN_STUFFED_FISH, NOUN_ROBO_KITCHEN)) {
		_vm->_dialogs->show(10230);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_ROBO_KITCHEN)) {
		_vm->_dialogs->show(10218);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_CLOSET)) {
		_vm->_dialogs->show(10219);
		_action._inProgress = false;
		return;
	}

	if ((_action.isObject(NOUN_LADDER) || _action.isObject(NOUN_HATCHWAY)) && (_action.isAction(VERB_LOOK) || _action.isAction(VERB_CLIMB_UP) || _action.isAction(VERB_CLIMB_THROUGH))) {
		if (_game._objects.isInInventory(OBJ_REBREATHER)) {
			if (!_action.isAction(VERB_CLIMB_UP) && !_action.isAction(VERB_CLIMB_THROUGH)) {
				_vm->_dialogs->show(10231);
				_action._inProgress = false;
				return;
			}
		} else if (_action.isAction(VERB_LOOK) || (_game._difficulty != DIFFICULTY_HARD)) {
			_vm->_dialogs->show(10222);
			_action._inProgress = false;
			return;
		}
	}

	if ((_action.isObject(NOUN_LADDER) || _action.isObject(NOUN_HATCHWAY)) && (_action.isAction(VERB_CLIMB_UP) || _action.isAction(VERB_CLIMB_THROUGH)) ) {
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
				_vm->_dialogs->show(10237);
			_scene->_nextSceneId = 106;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_POWER_STATUS_PANEL)) {
		_vm->_dialogs->show(10226);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_WINDOW) || _action.isAction(VERB_LOOK_THROUGH, NOUN_WINDOW)) {
		_vm->_dialogs->show(10227);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_DOORWAY) || _action.isAction(VERB_WALKTO, NOUN_DOORWAY)) {
		_vm->_dialogs->show(10228);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_DRAWER) || ((_action.isAction(VERB_CLOSE, NOUN_DRAWER) || _action.isAction(VERB_PUSH, NOUN_DRAWER)) && !_drawerDescrFl)) {
		_vm->_dialogs->show(10220);
		_drawerDescrFl = true;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_DRAWER) || _action.isAction(VERB_PUSH, NOUN_DRAWER)) {
		_vm->_dialogs->show(10221);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_DRAWER)) {
		_vm->_dialogs->show(10236);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_CHAIR) || (_action.isAction(VERB_SIT_IN, NOUN_CHAIR) && !_chairDescrFl)) {
		_chairDescrFl = true;
		_vm->_dialogs->show(10210);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_SIT_IN, NOUN_CHAIR)) {
		_vm->_dialogs->show(10211);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_MEDICINE_CABINET)) {
		if (_globals[kMedicineCabinetOpen])
			_vm->_dialogs->show(10207);
		else
			_vm->_dialogs->show(10206);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_MEDICINE_CABINET) && _globals[kMedicineCabinetOpen]) {
		switch (_game._trigger) {
		case 0:
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			_globals._sequenceIndexes[8] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(21);
			break;

		case 1:
			_scene->_sequences.addTimer(48, 2);
			break;

		case 2:
			_game._player._stepEnabled = true;
			_globals[kMedicineCabinetOpen] = false;
			_vm->_dialogs->show(10209);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_MEDICINE_CABINET) && !_globals[kMedicineCabinetOpen]) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
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
				_vm->_dialogs->show(10208);
			} else {
				_vm->_dialogs->show(10207);
			}
			_globals[kMedicineCabinetVirgin] = false;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_BINOCULARS) && _game._objects.isInRoom(OBJ_BINOCULARS)) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[11] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[11]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			break;

		case 1:
			_game._objects.addToInventory(OBJ_BINOCULARS);
			_scene->_sequences.remove(_globals._sequenceIndexes[9]);
			_scene->_hotspots.activate(NOUN_BINOCULARS, false);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_vm->_sound->command(22);
			_vm->_dialogs->showItem(OBJ_BINOCULARS, 10201);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_BURGER) && _game._objects.isInRoom(OBJ_BURGER)) {
		if (_game._trigger == 0) {
			_vm->_dialogs->showItem(OBJ_BURGER, 10235);
			_scene->_sequences.remove(_globals._sequenceIndexes[10]);
			_game._objects.addToInventory(OBJ_BURGER);
			_scene->_hotspots.activate(NOUN_BURGER, false);
			_vm->_sound->command(22);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_POSTER)) {
		_vm->_dialogs->show(10224);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_WEIGHT_MACHINE)) {
		_vm->_dialogs->show(10225);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_FLOOR)) {
		_vm->_dialogs->show(10232);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS) && !_game._objects.isInInventory(OBJ_BINOCULARS)) {
		_vm->_dialogs->show(10233);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_BURGER) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		_vm->_dialogs->show(801);
		_action._inProgress = false;
	}
}

void Scene102::postActions() {
	if (_action.isAction(VERB_PUT, NOUN_ROBO_KITCHEN) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		_vm->_dialogs->show(10217);
		_action._inProgress = false;
	}
}

/*------------------------------------------------------------------------*/

Scene103::Scene103(MADSEngine *vm) : Scene1xx(vm) {
	_updateClock = 0;
}

void Scene103::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	byte dummy = 0;
	s.syncAsByte(dummy); // In order to avoid to break savegame compatibility
	s.syncAsUint32LE(_updateClock);
}

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
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 72);

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 1, 37);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 2, 73);

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

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(237, 74);

	if (_scene->_priorSceneId == 102) {
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
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
	_updateClock = _scene->_frameStartTime;
}

void Scene103::step() {
	switch (_vm->_game->_trigger) {
	case 70:
		_vm->_game->_player._stepEnabled = true;
		break;

	case 72: {
		Common::Point pt = _vm->_game->_player._playerPos;
		int dist = Common::hypotenuse(pt.x - 58, pt.y - 93);
		_vm->_sound->command(27, (dist * -128 / 378) + 127);
		}
		break;

	case 73: {
		Common::Point pt = _vm->_game->_player._playerPos;
		int dist = Common::hypotenuse(pt.x - 266, pt.y - 81);
		_vm->_sound->command(27, (dist * -127 / 378) + 127);
		}
		break;

	default:
		break;
	}

	if (_scene->_frameStartTime >= _updateClock) {
		Common::Point pt = _vm->_game->_player._playerPos;
		int dist = Common::hypotenuse(pt.x - 79, pt.y - 137);
		_vm->_sound->command(29, (dist * -127 / 378) + 127);

		pt = _vm->_game->_player._playerPos;
		dist = Common::hypotenuse(pt.x - 69, pt.y - 80);
		_vm->_sound->command(30, (dist * -127 / 378) + 127);

		pt = _vm->_game->_player._playerPos;
		dist = Common::hypotenuse(pt.x - 266, pt.y - 138);
		_vm->_sound->command(32, (dist * -127 / 378) + 127);

		_updateClock = _scene->_frameStartTime + _vm->_game->_player._ticksAmount;
	}
}

void Scene103::actions() {
	if (_action._savedFields._lookFlag)
		_vm->_dialogs->show(10322);
	else if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR)) {
		switch (_vm->_game->_trigger) {
		case 0:
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
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
	} else if (_action.isAction(VERB_TAKE, NOUN_TIMER_MODULE) && _game._objects.isInRoom(OBJ_TIMER_MODULE)) {
		switch (_vm->_game->_trigger) {
		case 0:
			_scene->changeVariant(1);
			_globals._sequenceIndexes[13] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[13], false, 3, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[13]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_SPRITE, 7, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
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
			_vm->_dialogs->showItem(OBJ_TIMER_MODULE, 805);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_REBREATHER, 0) && _game._objects.isInRoom(OBJ_REBREATHER)) {
		switch (_vm->_game->_trigger) {
		case 0:
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 3, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 6, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
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
			_vm->_dialogs->showItem(OBJ_REBREATHER, 804);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, 362))
		_vm->_dialogs->show(10301);
	else if (_action.isAction(VERB_TAKE, 362)) {
		// Take Turkey
		if (!_vm->_game->_trigger)
			_vm->_sound->command(31);

		if (_vm->_game->_trigger < 2) {
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, _vm->_game->_trigger < 1 ? 1 : 0);
			if (_vm->_game->_trigger) {
				// Lock the turkey into a permanent "exploded" frame
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], -2, -2);

				// Rex says "Gads.."
				Common::String msg = _game.getQuote(51);
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 18, 0, 60, msg);
				_scene->_sequences.addTimer(120, _vm->_game->_trigger + 1);
			} else {
				// Initial turky explosion
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
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
	} else if (_action.isAction(VERB_LOOK, 250))
		_vm->_dialogs->show(!_globals[kTurkeyExploded] ? 10323 : 10303);
	else if (_action.isAction(VERB_TALKTO, 27)) {
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
	} else if (_action.isAction(VERB_LOOK, 27))
		_vm->_dialogs->show(10304);
	else if (_action.isAction(VERB_LOOK, 36))
		_vm->_dialogs->show(10307);
	else if (_action.isAction(VERB_LOOK, 55))
		_vm->_dialogs->show(10308);
	else if (_action.isAction(VERB_TAKE, 315))
		_vm->_dialogs->show(10309);
	else if (_action.isAction(VERB_TAKE, 85))
		_vm->_dialogs->show(10310);
	else if (_action.isAction(VERB_LOOK, 144))
		_vm->_dialogs->show(10312);
	else if (_action.isAction(VERB_OPEN, 144))
		_vm->_dialogs->show(10313);
	else if (_action.isAction(VERB_CLOSE, 27))
		_vm->_dialogs->show(10314);
	else if (_action.isAction(VERB_LOOK, 310))
		_vm->_dialogs->show(10315);
	else if (_action.isAction(VERB_LOOK, 178))
		_vm->_dialogs->show(10316);
	else if (_action.isAction(VERB_LOOK, 283))
		_vm->_dialogs->show(10317);
	else if (_action.isAction(VERB_LOOK, 120))
		_vm->_dialogs->show(10318);
	else if (_action.isAction(VERB_LOOK, 289) && _game._objects.isInInventory(OBJ_REBREATHER))
		_vm->_dialogs->show(10319);
	else if (_action.isAction(VERB_LOOK, 371) && _game._objects.isInInventory(OBJ_TIMER_MODULE))
		_vm->_dialogs->show(10320);
	else if (_action.isAction(VERB_LOOK, 137))
		_vm->_dialogs->show(10321);
	else if (_action.isAction(VERB_LOOK, 409))
		_vm->_dialogs->show(_game._objects.isInInventory(OBJ_TIMER_MODULE) ? 10324 : 10325);
	else
		return;

	_action._inProgress = false;
}

void Scene103::postActions() {
	if (_action.isObject(NOUN_AUXILIARY_POWER) && !_action.isAction(VERB_WALKTO)) {
		_vm->_dialogs->show(10305);
		_action._inProgress = false;
	} else if (_action.isAction(VERB_PUT, NOUN_COAL, NOUN_FURNACE)) {
		Common::String msg = _game.getQuote(73);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, msg);
		_action._inProgress = false;
	}
}

/*------------------------------------------------------------------------*/

Scene104::Scene104(MADSEngine *vm) : Scene1xx(vm) {
	_kargShootingFl = false;
	_loseFl = false;
}

void Scene104::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_kargShootingFl);
	s.syncAsByte(_loseFl);
}

void Scene104::setup() {
	// Preloading has been skipped
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene104::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('h', -1));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 14, 0, 0, 1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);

	if (_scene->_priorSceneId == 105)
		_game._player._playerPos = Common::Point(302, 107);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
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
				_scene->freeAnimation();
				_scene->resetScene();
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 0));
				_vm->_palette->refreshSceneColors();
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], mirrorFl, 7, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], mirrorFl, 7, 0, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -2, -2);
				_scene->_sequences.addTimer(90, 2);
				break;

			case 2:
				_vm->_dialogs->show(10406);
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
			break;

		case FACING_SOUTH:
			switch (_game._trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->freeAnimation();
				_scene->resetScene();
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 1));
				_vm->_palette->refreshSceneColors();
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 14);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 15, 32);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 3, 0, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
				_scene->_sequences.addTimer(90, 3);
				break;

			case 3:
				_vm->_dialogs->show(10406);
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
			break;

		case FACING_NORTH:
			switch (_game._trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->freeAnimation();
				_scene->resetScene();
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 2));
				_vm->_palette->refreshSceneColors();
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 8, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				if (_game._storyMode >= STORYMODE_NICE)
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 15, 2);
				break;

			case 1:
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 8, 0, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -2, -2);
				_scene->_sequences.addTimer(90, 2);
				break;

			case 2:
				_vm->_dialogs->show(10406);
				_scene->_reloadSceneFlag = true;
				break;

			default:
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

	if (_kargShootingFl && (_scene->_animation[0]->getCurrentFrame() >= 19)) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(52));
		_kargShootingFl = false;
	}
}

void Scene104::preActions() {
	if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_EASTERN_CLIFF_FACE))
		_game._player._walkOffScreenSceneId = 105;

	if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_OPEN_AREA_TO_SOUTH))
		_game._player._walkOffScreenSceneId = 106;
}

void Scene104::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10405);
	else if (_action.isAction(VERB_LOOK, NOUN_CURIOUS_WEED_PATCH))
		_vm->_dialogs->show(10404);
	else if (_action.isAction(VERB_LOOK, NOUN_SURFACE))
		_vm->_dialogs->show(10403);
	else if (_action.isAction(VERB_LOOK, NOUN_CLIFF_FACE))
		_vm->_dialogs->show(10401);
	else if (_action.isAction(VERB_LOOK, NOUN_OCEAN_FLOOR))
		_vm->_dialogs->show(10402);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene105::Scene105(MADSEngine *vm) : Scene1xx(vm) {
	_explosionFl = false;
}

void Scene105::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_explosionFl);
}

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
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(48, 144));

		int idx = _scene->_dynamicHotspots.add(101, 348, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(56, 141), FACING_NORTHWEST);
	}

	if (_scene->_priorSceneId == 104)
		_game._player._playerPos = Common::Point(13, 97);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
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
			_vm->_palette->refreshSceneColors();

			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 1);

			if (_game._storyMode >= STORYMODE_NICE)
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 8, 3);
			break;

		case 1:
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 9, 1, 0, 0);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], _globals._sequenceIndexes[0]);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 5, 7);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
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
			_vm->_dialogs->show(10507);
			_scene->_reloadSceneFlag = true;
			_scene->_sequences.addTimer(90, 4);
			break;

		default:
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
	if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_WESTERN_CLIFF_FACE))
		_game._player._walkOffScreenSceneId = 104;

	if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_OPEN_AREA_TO_SOUTH))
		_game._player._walkOffScreenSceneId = 107;

	if (_action.isObject(NOUN_MINE) && (_action.isAction(VERB_TALKTO) || _action.isAction(VERB_LOOK)))
		_game._player._needToWalk = false;
}

void Scene105::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10512);
	else if (_action.isAction(VERB_TAKE, NOUN_DEAD_FISH) && _globals[kFishIn105]) {
		if (_game._objects.isInInventory(OBJ_DEAD_FISH)) {
			int randVal = _vm->getRandomNumber(74, 76);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_game._objects.addToInventory(OBJ_DEAD_FISH);
			_globals[kFishIn105] = false;
			_vm->_dialogs->showItem(OBJ_DEAD_FISH, 802, 0);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_WESTERN_CLIFF_FACE))
		_vm->_dialogs->show(10501);
	else if (_action.isAction(VERB_LOOK, NOUN_CLIFF_FACE))
		_vm->_dialogs->show(10502);
	else if (_action.isAction(VERB_LOOK, NOUN_OCEAN_FLOOR))
		_vm->_dialogs->show(10503);
	else if (_action.isAction(VERB_LOOK, NOUN_MEDICAL_WASTE))
		_vm->_dialogs->show(10504);
	else if (_action.isAction(VERB_TAKE, NOUN_MEDICAL_WASTE))
		_vm->_dialogs->show(10505);
	else if (_action.isAction(VERB_LOOK, NOUN_MINE))
		_vm->_dialogs->show(10506);
	else if (_action.isAction(VERB_LOOK, NOUN_DEAD_FISH))
		_vm->_dialogs->show(10508);
	else if (_action.isAction(VERB_LOOK, NOUN_SURFACE))
		_vm->_dialogs->show(10509);
	else if (_action.isAction(VERB_LOOK, NOUN_OPEN_AREA_TO_SOUTH))
		_vm->_dialogs->show(10510);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCKS))
		_vm->_dialogs->show(10511);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene106::Scene106(MADSEngine *vm) : Scene1xx(vm) {
	_backToShipFl = false;
	_shadowFl = false;
	_firstEmergingFl = false;
	_positionY = 0;
}

void Scene106::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_backToShipFl);
	s.syncAsByte(_shadowFl);
	s.syncAsByte(_firstEmergingFl);
	s.syncAsSint32LE(_positionY);
}

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
	_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 21, 0, 0, 0);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('I', -1));
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 32, 47);

	if (_scene->_priorSceneId == 102) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 1, 4, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_game._player._facing = FACING_EAST;
		_game._player._playerPos = Common::Point(106, 69);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
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
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 28, 71);
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

	if (_firstEmergingFl && (_scene->_animation[0]->getCurrentFrame() >= 19)) {
		_firstEmergingFl = false;
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(52));
	}

	if ((_game._trigger >= 80) && (_game._trigger <= 87)) {
		int tmpVal = _game._trigger - 80;
		int msgId = -1;
		switch (tmpVal) {
		case 0:
			_positionY = 26;
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
			int nextTrigger = _game._trigger + 1;
			_scene->_kernelMessages.add(Common::Point(15, _positionY), 0x1110, 0, 0, 360, _game.getQuote(msgId));
			_scene->_sequences.addTimer(150, nextTrigger);
			_positionY += 14;
		}
	}

	if (_backToShipFl) {
		if (!_shadowFl) {
			if (_game._player._playerPos.x < 204) {
				_shadowFl = true;
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 4, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 44, 73);
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
	if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_SEA_CLIFF) || _action.isAction(VERB_SWIM_TOWARDS, NOUN_SEAWEED_BANK)) {
		_game._player._stepEnabled = false;
		_scene->_sprites[_game._player._spritesStart + 1]->_charInfo->_velocity = 24;
		_game._player._walkOffScreenSceneId = 104;
	}

	if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_OPEN_AREA_TO_EAST))
		_game._player._walkOffScreenSceneId = 107;
}

void Scene106::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10614);
	else if (_action.isAction(VERB_SWIM_TO, NOUN_MAIN_AIRLOCK)) {
		_game._player._stepEnabled = false;
		_game._player._prepareWalkPos = Common::Point(95, 72);
		_game._player._prepareWalkFacing = FACING_WEST;
		_game._player._needToWalk = true;
		_game._player._readyToWalk = true;
		_game._player._frameNumber = 9;
		_backToShipFl = true;
	} else if (_action.isAction(VERB_LOOK, NOUN_ANEMONE) || _action.isAction(VERB_LOOK_AT, NOUN_ANEMONE))
		_vm->_dialogs->show(10601);
	else if (_action.isAction(VERB_TAKE, NOUN_ANEMONE))
		_vm->_dialogs->show(10602);
	else if (_action.isAction(VERB_LOOK, NOUN_SEAWEED) || _action.isAction(VERB_LOOK, NOUN_SEAWEED_BANK))
		_vm->_dialogs->show(10603);
	else if (_action.isAction(VERB_TAKE, NOUN_SEAWEED) || _action.isAction(VERB_TAKE, NOUN_SEAWEED_BANK))
		_vm->_dialogs->show(10604);
	else if (_action.isAction(VERB_LOOK, NOUN_OPEN_AREA_TO_EAST))
		_vm->_dialogs->show(10605);
	else if (_action.isAction(VERB_LOOK, NOUN_PILE_OF_ROCKS) || _action.isAction(VERB_LOOK_AT, NOUN_PILE_OF_ROCKS))
		_vm->_dialogs->show(10606);
	else if (_action.isObject(NOUN_PILE_OF_ROCKS) && (_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL) || _action.isAction(VERB_TAKE)))
		_vm->_dialogs->show(10607);
	else if (_action.isAction(VERB_LOOK, NOUN_SHIP) || _action.isAction(VERB_LOOK_AT, NOUN_SHIP))
		_vm->_dialogs->show(10608);
	else if (_action.isAction(VERB_LOOK, NOUN_MAIN_AIRLOCK))
		_vm->_dialogs->show(10609);
	else if (_action.isAction(VERB_OPEN, NOUN_MAIN_AIRLOCK))
		_vm->_dialogs->show(10610);
	else if (_action.isAction(VERB_CLOSE, NOUN_MAIN_AIRLOCK))
		_vm->_dialogs->show(10611);
	else if (_action.isAction(VERB_LOOK, NOUN_SEA_CLIFF))
		_vm->_dialogs->show(10612);
	else if (_action.isAction(VERB_LOOK, NOUN_OCEAN_FLOOR))
		_vm->_dialogs->show(10613);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene107::Scene107(MADSEngine *vm) : Scene1xx(vm) {
	_shootingFl = false;
}

void Scene107::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_shootingFl);
}

void Scene107::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_MANTA_RAY);
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
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(68, 151));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		int idx = _scene->_dynamicHotspots.add(101, 348, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(78, 135), FACING_SOUTHWEST);
	}

	if (_scene->_priorSceneId == 105)
		_game._player._playerPos = Common::Point(132, 47);
	else if (_scene->_priorSceneId == 106)
		_game._player._playerPos = Common::Point(20, 91);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(223, 151);

	if (((_scene->_priorSceneId == 105) || (_scene->_priorSceneId == 106)) && (_vm->getRandomNumber(1, 3) == 1)) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(Resources::formatName(105, 'R', 1, EXT_SS, ""));
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], true, 4, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[0], Common::Point(270, 150));
		_scene->_sequences.setMotion(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, -200, 0);
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
	if (_shootingFl && (_scene->_animation[0]->getCurrentFrame() >= 19)) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(52));
		_shootingFl = false;
	}
}

void Scene107::preActions() {
	if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_OPEN_AREA_TO_WEST))
		_game._player._walkOffScreenSceneId = 106;

	if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_OPEN_AREA_TO_SOUTH))
		_game._player._walkOffScreenSceneId = 108;
}

void Scene107::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10708);
	else if (_action.isAction(VERB_TAKE, NOUN_DEAD_FISH) && _globals[kFishIn107]) {
		if (_game._objects.isInInventory(OBJ_DEAD_FISH)) {
			int randVal = _vm->getRandomNumber(74, 76);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_game._objects.addToInventory(OBJ_DEAD_FISH);
			_globals[kFishIn107] = false;
			_vm->_dialogs->showItem(OBJ_DEAD_FISH, 802);
		}
	} else if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_NORTHERN_SEA_CLIFF))
		_scene->_nextSceneId = 105;
	else if (_action.isAction(VERB_LOOK, NOUN_NORTHERN_SEA_CLIFF))
		_vm->_dialogs->show(10701);
	else if (_action.isAction(VERB_LOOK, NOUN_DEAD_FISH) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(10702);
	else if (_action.isAction(VERB_LOOK, NOUN_BUSH_LIKE_FORMATION))
		_vm->_dialogs->show(10703);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCK))
		_vm->_dialogs->show(10704);
	else if (_action.isAction(VERB_LOOK, NOUN_SEAWEED))
		_vm->_dialogs->show(10705);
	else if (_action.isAction(VERB_LOOK, NOUN_OPEN_AREA_TO_SOUTH))
		_vm->_dialogs->show(10706);
	else if (_action.isAction(VERB_LOOK, NOUN_CLIFF_FACE))
		_vm->_dialogs->show(10707);
	else if (_action.isAction(VERB_LOOK, NOUN_MANTA_RAY))
		_vm->_dialogs->show(10709);
	else if (_action.isAction(VERB_TAKE, NOUN_MANTA_RAY))
		_vm->_dialogs->show(10710);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene108::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene108::enter() {
	if (_globals[kHoovicSated] == 2)
		_globals[kHoovicSated] = 0;

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('X', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('X', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('X', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('X', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(Resources::formatName(105, 'f', 4, EXT_SS, ""));

	_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 13, 0, 0, 7);
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 16, 0, 0, 9);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 17, 0, 0, 3);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 14, 0, 0, 13);

	for (int i = 0; i <= 3; i++)
		_scene->_sequences.setDepth(_globals._sequenceIndexes[i], 0);

	if (_globals[kFishIn108]) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(41, 109));
		int idx = _scene->_dynamicHotspots.add(101, 348, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(41, 109), FACING_NORTHWEST);
	}

	if (_scene->_priorSceneId == 107)
		_game._player._playerPos = Common::Point(138, 58);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(305, 98);

	_game.loadQuoteSet(0x4A, 0x4B, 0x4C, 0x35, 0x34, 0);
	sceneEntrySound();
}

void Scene108::preActions() {
	if (_action.isAction(VERB_SWIM_UNDER, NOUN_OVERHANG_TO_EAST))
		_game._player._walkOffScreenSceneId = 109;
}

void Scene108::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10812);
	else if (_action.isAction(VERB_TAKE, NOUN_DEAD_FISH) && _globals[kFishIn108]) {
		if (_game._objects.isInInventory(OBJ_DEAD_FISH)) {
			int randVal = _vm->getRandomNumber(74, 76);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_game._objects.addToInventory(OBJ_DEAD_FISH);
			_globals[kFishIn108] = false;
			_vm->_dialogs->showItem(OBJ_DEAD_FISH, 10808);
		}
	} else if (_action.isAction(VERB_SWIM_TOWARDS, NOUN_OPEN_AREA_TO_NORTH))
		_scene->_nextSceneId = 107;
	else if (_action.isAction(VERB_LOOK, NOUN_CLIFF_FACE))
		_vm->_dialogs->show(10801);
	else if (_action.isAction(VERB_LOOK, NOUN_OCEAN_FLOOR))
		_vm->_dialogs->show(10802);
	else if (_action.isAction(VERB_LOOK, NOUN_ODD_ROCK_FORMATION))
		_vm->_dialogs->show(10803);
	else if (_action.isAction(VERB_TAKE, NOUN_ODD_ROCK_FORMATION))
		_vm->_dialogs->show(10804);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCKS))
		_vm->_dialogs->show(10805);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCKS))
		_vm->_dialogs->show(10806);
	else if (_action.isAction(VERB_LOOK, NOUN_DEAD_FISH))
		_vm->_dialogs->show(10807);
	else if (_action.isAction(VERB_LOOK, NOUN_OVERHANG_TO_EAST))
		_vm->_dialogs->show(10809);
	else if (_action.isAction(VERB_LOOK, NOUN_OPEN_AREA_TO_NORTH))
		_vm->_dialogs->show(10810);
	else if (_action.isAction(VERB_LOOK, NOUN_SURFACE))
		_vm->_dialogs->show(10811);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene109::Scene109(MADSEngine *vm) : Scene1xx(vm) {
	_rexThrowingObject = false;
	_hoovicDifficultFl = false;
	_beforeEatingRex = false;
	_eatingRex = false;
	_hungryFl = false;
	_eatingFirstFish = false;

	_throwingObjectId = -1;
	_hoovicTrigger = 0;
}

void Scene109::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_rexThrowingObject);
	s.syncAsByte(_hoovicDifficultFl);
	s.syncAsByte(_beforeEatingRex);
	s.syncAsByte(_eatingRex);
	s.syncAsByte(_hungryFl);
	s.syncAsByte(_eatingFirstFish);
	s.syncAsSint32LE(_throwingObjectId);
	s.syncAsSint32LE(_hoovicTrigger);
}

void Scene109::setup() {
	_scene->addActiveVocab(NOUN_DEAD_PURPLE_MONSTER);
	_scene->addActiveVocab(NOUN_MONSTER_SLUDGE);

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene109::enter() {
	_globals[kFishIn105] = true;

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*RXSWRC_6");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('O', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('O', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('O', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('H', 4));

	_rexThrowingObject = false;
	_throwingObjectId = 0;
	_beforeEatingRex = false;
	_eatingRex = false;
	_hungryFl = false;

	if (_scene->_priorSceneId == 110) {
		_game._player._playerPos = Common::Point(248, 38);
		_globals[kHoovicSated] = 2;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(20, 68);
		_game._player._facing = FACING_EAST;
	}

	if (!_globals[kHoovicAlive]) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], -2, -2);

		int idx = _scene->_dynamicHotspots.add(102, 348, -1, Common::Rect(256, 57, 267, 87));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
		idx = _scene->_dynamicHotspots.add(102, 348, -1, Common::Rect(242, 79, 265, 90));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
		idx = _scene->_dynamicHotspots.add(229, 348, -1, Common::Rect(231, 88, 253, 94));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
	}

	if (!_globals[kHoovicAlive] || _globals[kHoovicSated])
		_scene->changeVariant(1);

	if (_game._objects.isInRoom(OBJ_BURGER)) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
		int idx = _scene->_dynamicHotspots.add(53, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
	} else if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BURGER);

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_DEAD_FISH);
		_game._objects.addToInventory(OBJ_STUFFED_FISH);
	}

	_vm->_palette->setEntry(252, 50, 50, 63);
	_vm->_palette->setEntry(253, 30, 30, 50);

	_game.loadQuoteSet(0x53, 0x52, 0x54, 0x55, 0x56, 0x57, 0x58, 0);
	_eatingFirstFish = (!_game._visitedScenes._sceneRevisited) && (_scene->_priorSceneId < 110);

	if (_eatingFirstFish) {
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(Resources::formatName(105, 'F', 1, EXT_SS, ""));
		_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('H', 1));

		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], true, 4, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 5);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[10], Common::Point(126, 39));
		_scene->_sequences.setMotion(_globals._sequenceIndexes[10], 0, 200, 0);
		_scene->_sequences.setScale(_globals._sequenceIndexes[10], 80);
		_game._player._stepEnabled = false;
	}

	sceneEntrySound();
}

void Scene109::step() {
	if (_beforeEatingRex) {
		if (!_eatingRex) {
			if (_game._player._playerPos.x > 205) {
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 6, 70);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 71);

				_eatingRex = true;
				_vm->_sound->command(34);
			}
		} else {
			switch (_game._trigger) {
			case 70:
				_game._player._visible = false;
				break;

			case 71:
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
		}
	}

	if (_hungryFl && (_game._player._playerPos == Common::Point(160, 32)) && (_game._player._facing == FACING_EAST)) {
		_game._player.walk(Common::Point(226, 24), FACING_EAST);
		_game._player._stepEnabled = false;
		_hungryFl = false;
		_beforeEatingRex = true;
		_scene->_sprites.remove(_globals._spriteIndexes[6]);
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('H', 0));
		_vm->_palette->refreshSceneColors();
	}

	if (_game._player._moving && (_scene->_rails.getNext() > 0) && _globals[kHoovicAlive] && !_globals[kHoovicSated] && !_hungryFl && !_beforeEatingRex) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(160, 32), FACING_EAST);
		_scene->_rails.resetNext();
		_hungryFl = true;
	}

	if (_eatingFirstFish && (_scene->_sequences[_globals._sequenceIndexes[10]]._position.x >= 178)) {
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 4, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_SPRITE, 29, 72);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 29, 73);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[10], _globals._sequenceIndexes[9]);
		_eatingFirstFish = false;
		_game._player._stepEnabled = true;
		_vm->_sound->command(34);
	}

	if (_game._trigger == 72)
		_scene->_sequences.remove(_globals._sequenceIndexes[10]);

	if (_game._trigger == 73) {
		_scene->_sequences.remove(_globals._sequenceIndexes[9]);
		_scene->_sprites.remove(_globals._spriteIndexes[9]);
		_scene->_sprites.remove(_globals._spriteIndexes[10]);

		_scene->_spriteSlots.clear();
		_scene->_spriteSlots.fullRefresh();

		int randVal = _vm->getRandomNumber(85, 88);
		int idx = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(randVal));
		_scene->_kernelMessages.setQuoted(idx, 4, true);
		_scene->_kernelMessages._entries[idx]._frameTimer = _scene->_frameStartTime + 4;
	}
}

void Scene109::preActions() {
	if (_action.isAction(VERB_SWIM_UNDER, NOUN_OVERHANG_TO_WEST))
		_game._player._walkOffScreenSceneId = 108;

	if ((_action.isAction(VERB_THROW) || _action.isAction(VERB_GIVE) || _action.isAction(VERB_PUT))
			&& (_action.isTarget(NOUN_SMALL_HOLE) || _action.isTarget(NOUN_TUNNEL))
			&& (_action.isObject(NOUN_DEAD_FISH) || _action.isObject(NOUN_STUFFED_FISH) || _action.isObject(NOUN_BURGER))) {
		int idx = _game._objects.getIdFromDesc(_action._activeAction._objectNameId);
		if ((idx >= 0) && _game._objects.isInInventory(idx)) {
			_game._player._prepareWalkPos = Common::Point(106, 38);
			_game._player._prepareWalkFacing = FACING_EAST;
			_game._player._needToWalk = true;
			_game._player._readyToWalk = true;
		}
	}

	if ((_action.isAction(VERB_SWIM_INTO, NOUN_TUNNEL) || _action.isAction(VERB_SWIM_TO, NOUN_SMALL_HOLE))
	&& (!_globals[kHoovicAlive] || _globals[kHoovicSated]) && (_action.isObject(NOUN_TUNNEL)))
		_game._player._walkOffScreenSceneId = 110;

	_hungryFl = false;
}

void Scene109::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(10912);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_THROW) || _action.isAction(VERB_GIVE)) && (_action.isTarget(NOUN_SMALL_HOLE) || _action.isTarget(NOUN_TUNNEL))) {
		if (_action.isObject(NOUN_DEAD_FISH) || _action.isObject(NOUN_STUFFED_FISH) || _action.isObject(NOUN_BURGER)) {
			_throwingObjectId = _game._objects.getIdFromDesc(_action._activeAction._objectNameId);
			if (_throwingObjectId >= 0) {
				if ((_game._objects.isInInventory(_throwingObjectId) && _globals[kHoovicAlive]) || _rexThrowingObject) {
					switch (_game._trigger) {
					case 0:
						_rexThrowingObject = true;
						_hoovicDifficultFl = false;
						_game._objects.setRoom(_throwingObjectId, NOWHERE);
						_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 4, 1, 0, 0);
						_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[0]);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
						_game._player._visible = false;
						_game._player._stepEnabled = false;

						switch (_throwingObjectId) {
						case OBJ_DEAD_FISH:
						case OBJ_STUFFED_FISH:
							_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('H', 1));
							break;

						case OBJ_BURGER:
							_hoovicDifficultFl = (_game._difficulty == DIFFICULTY_HARD);
							_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('H', (_hoovicDifficultFl ? 3 : 1)));
							break;

						default:
							break;
						}

						_vm->_palette->refreshSceneColors();
						break;

					case 1:
						_game._player._visible = true;
						_hoovicTrigger = 4;
						switch (_throwingObjectId) {
						case OBJ_BURGER:
							_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, (_hoovicDifficultFl ? 4 : 6), 1, 0, 0);
							_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 2, 2);
							if (_hoovicDifficultFl) {
								_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 30);
								_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
							} else {
								_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
								_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
								_hoovicTrigger = 3;
							}
							break;
						case OBJ_DEAD_FISH:
							_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
							_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 2, 2);
							break;
						case OBJ_STUFFED_FISH:
							_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
							_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 2);
							_hoovicTrigger = 3;
							break;
						default:
							break;
						}
						break;

					case 2:
						if (_hoovicDifficultFl)
							_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 4, 2, 0, 0);
						else
							_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 4, 1, 0, 0);

						_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 4);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, _hoovicTrigger);
						_vm->_sound->command(34);
						break;

					case 3:
						_scene->loadAnimation(Resources::formatName(109, 'H', 2, EXT_AA, ""), 4);
						_vm->_sound->command(35);
						_globals[kHoovicAlive] = false;
						break;

					case 4:
						if (!_globals[kHoovicAlive]) {
							_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
							_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 4);
							_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], -2, -2);
							int idx = _scene->_dynamicHotspots.add(102, 348, -1, Common::Rect(256, 57, 256 + 12, 57 + 31));
							_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
							idx = _scene->_dynamicHotspots.add(102, 348, -1, Common::Rect(242, 79, 242 + 24, 79 + 12));
							_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
							idx = _scene->_dynamicHotspots.add(229, 348, -1, Common::Rect(231, 88, 231 + 23, 88 + 7));
							_scene->_dynamicHotspots.setPosition(idx, Common::Point(241, 91), FACING_NORTHEAST);
							_scene->changeVariant(1);
						} else {
							if (_throwingObjectId == OBJ_DEAD_FISH) {
								++_globals[kHoovicFishEaten];
								int threshold;
								switch (_game._difficulty) {
								case DIFFICULTY_HARD:
									threshold = 1;
									break;
								case DIFFICULTY_MEDIUM:
									threshold = 3;
									break;
								default:
									threshold = 50;
									break;
								}

								if (_globals[kHoovicFishEaten] >= threshold) {
									int randVal = _vm->getRandomNumber(83, 84);
									_scene->_kernelMessages.add(Common::Point(230, 24), 0xFDFC, 0, 0, 120, _game.getQuote(randVal));
									_globals[kHoovicFishEaten] = 0;
									_globals[kHoovicSated] = 1;
									_scene->changeVariant(1);
								}
							}
						}
						_scene->freeAnimation();
						_scene->_sequences.remove(_globals._sequenceIndexes[8]);
						_scene->_sprites.remove(_globals._spriteIndexes[8]);
						_scene->_spriteSlots.clear();
						_scene->_spriteSlots.fullRefresh();
						_scene->_sequences.scan();
						if (_game._player._visible) {
							_game._player._forceRefresh = true;
							_game._player.update();
						}

						_game._player._stepEnabled = true;
						_rexThrowingObject = false;
						break;

					case 5: {
						_game._objects.setRoom(OBJ_BURGER, _scene->_currentSceneId);
						_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 30, 30);
						int idx = _scene->_dynamicHotspots.add(53, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
						_scene->_sequences.addTimer(65, 6);
						}
						break;

					case 6: {
						_scene->_sequences.remove(_globals._sequenceIndexes[3]);
						_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 31, 46);
						int idx = _scene->_dynamicHotspots.add(53, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 7);
						}
						break;

					case 7: {
						_scene->_sequences.remove(_globals._sequenceIndexes[3]);
						_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
						int idx = _scene->_dynamicHotspots.add(53, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(-3, 0), FACING_NORTHEAST);
						_vm->_dialogs->show(10915);
						}
						break;

					case 8:
						_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 5, 16);
						break;

					default:
						break;
					}
					_action._inProgress = false;
					return;
				} else if (_game._objects.isInInventory(_throwingObjectId)) {
					// Nothing.
				}
			}
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_BURGER) && _game._objects.isInRoom(OBJ_BURGER)) {
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_game._objects.addToInventory(OBJ_BURGER);
	} else if (_action.isAction(VERB_LOOK, NOUN_OCEAN_FLOOR))
		_vm->_dialogs->show(10901);
	else if (_action.isAction(VERB_LOOK, NOUN_CORAL))
		_vm->_dialogs->show(10902);
	else if ((_action.isAction(VERB_TAKE) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_CORAL))
		_vm->_dialogs->show(10903);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCKS))
		_vm->_dialogs->show(10904);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCKS))
		_vm->_dialogs->show(10905);
	else if (_action.isAction(VERB_LOOK, NOUN_CAVE_WALL))
		_vm->_dialogs->show(10906);
	else if (_action.isAction(VERB_LOOK, NOUN_TUNNEL)) {
		if (_globals[kHoovicAlive])
			_vm->_dialogs->show(10907);
		else
			_vm->_dialogs->show(10913);
	} else if (_action.isAction(VERB_LOOK, NOUN_SMALL_HOLE))
		_vm->_dialogs->show(10908);
	else if (_action.isAction(VERB_LOOK, NOUN_OVERHANG_TO_WEST))
		_vm->_dialogs->show(10911);
	else if (_action.isAction(VERB_PUT, NOUN_SMALL_HOLE))
		_vm->_dialogs->show(10910);
	else if (_action.isAction(VERB_LOOK, NOUN_DEAD_PURPLE_MONSTER))
		_vm->_dialogs->show(10914);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene110::Scene110(MADSEngine *vm) : Scene1xx(vm) {
	_crabsFl = false;
}

void Scene110::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_crabsFl);
}

void Scene110::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_CRAB);
}

void Scene110::enter() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('X', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('X', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('X', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('X', 3));

	_crabsFl = false;

	if (_scene->_priorSceneId == 109) {
		_game._player._playerPos = Common::Point(59, 71);

		_globals._sequenceIndexes[0] = _scene->_sequences.startCycle(_globals._spriteIndexes[0], false, 1);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);

		_crabsFl = true;

		int idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[0], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(194, 23);
		_game._player._facing = FACING_SOUTH;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(Resources::formatName(110, 'T', 1,EXT_AA, ""), 70);
	}

	sceneEntrySound();
	_game.loadQuoteSet(0x59, 0);

	if (!_game._visitedScenes._sceneRevisited && (_scene->_priorSceneId == 109))
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(109));
}

void Scene110::step() {
	if (_game._trigger == 70) {
		_game._player._visible = true;
		_game._player._stepEnabled = true;
	}
}

void Scene110::preActions() {
	if (_action.isAction(VERB_SWIM_THROUGH, NOUN_CAVE_ENTRANCE))
		_game._player._walkOffScreenSceneId = 109;

	if (_crabsFl) {
		_crabsFl = false;

		_scene->_sequences.remove(_globals._sequenceIndexes[0]);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);

		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 16, 1, 0, 0);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 16, 1, 0, 0);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 16, 1, 0, 0);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 16, 1, 0, 0);

		int idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[0], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
		idx = _scene->_dynamicHotspots.add(91, 348, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-1, 0), FACING_NONE);
	}
}

void Scene110::actions() {
	if (_action.isAction(VERB_SWIM_THROUGH, NOUN_TUNNEL)) {
		switch (_game._trigger) {
		case 0:
			_scene->loadAnimation(Resources::formatName(110, 'T', 0, EXT_AA, ""), 1);
			_scene->_animation[0]->setNextFrameTimer(_game._player._ticksAmount + _game._player._priorTimer);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			break;
		case 1:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_scene->_nextSceneId = 111;
			break;
		default:
			break;
		}
	} else if ((_action._lookFlag) || _action.isAction(VERB_LOOK, NOUN_CAVE))
		_vm->_dialogs->show(11001);
	else if (_action.isAction(VERB_LOOK, NOUN_CAVE_CEILING) || _action.isAction(VERB_LOOK_AT, NOUN_CAVE_CEILING))
		_vm->_dialogs->show(11002);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCKS))
		_vm->_dialogs->show(11003);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCKS))
		_vm->_dialogs->show(11004);
	else if (_action.isAction(VERB_LOOK, NOUN_TUNNEL))
		_vm->_dialogs->show(11005);
	else if (_action.isAction(VERB_LOOK, NOUN_CAVE_ENTRANCE))
		_vm->_dialogs->show(11006);
	else if (_action.isAction(VERB_LOOK, NOUN_FUNGOIDS))
		_vm->_dialogs->show(11007);
	else if (_action.isAction(VERB_TAKE, NOUN_FUNGOIDS))
		_vm->_dialogs->show(11008);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene111::Scene111(MADSEngine *vm) : Scene1xx(vm) {
	_stampedFl = false;
	_launch1Fl = false;
	_launched2Fl = false;
	_rexDivingFl = false;
}

void Scene111::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_stampedFl);
	s.syncAsByte(_launch1Fl);
	s.syncAsByte(_launched2Fl);
	s.syncAsByte(_rexDivingFl);
}

void Scene111::setup() {
	_scene->addActiveVocab(NOUN_BATS);

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene111::enter() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('X', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('X', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('X', 2));

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('B', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('B', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('B', 2));

	_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 8, 0, 0, 0);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 9, 73);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 13, 73);

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 0, 0, 0);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 71, 71);

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 0, 0, 0);
	_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
	_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);

	int idx = _scene->_dynamicHotspots.add(NOUN_BATS, VERB_LOOK_AT, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
	idx = _scene->_dynamicHotspots.add(NOUN_BATS, VERB_LOOK_AT, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
	idx = _scene->_dynamicHotspots.add(NOUN_BATS, VERB_LOOK_AT, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);

	_launch1Fl = false;
	_launched2Fl = false;
	_stampedFl = false;

	if ((_scene->_priorSceneId < 201) && (_scene->_priorSceneId != RETURNING_FROM_DIALOG)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->loadAnimation(Resources::formatName(111, 'A', 0, EXT_AA, ""), 70);
		_game._player._playerPos = Common::Point(234, 116);
		_game._player._facing = FACING_EAST;

		_launch1Fl = true;
		_launched2Fl = true;

		_vm->_sound->command(36);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(300, 130);
		_game._player._facing = FACING_WEST;
	}

	_rexDivingFl = false;

	sceneEntrySound();
}

void Scene111::step() {
	if (_game._trigger == 70) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_launch1Fl = false;
		_launched2Fl = false;
	}

	if ((_game._trigger == 71) && !_stampedFl) {
		_stampedFl = true;
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 18, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
	}

	if (_game._trigger == 72) {
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 20);
	}

	if (!_launch1Fl && (_vm->getRandomNumber(1, 5000) == 1)) {
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 5, 1, 0, 0);
		_launch1Fl = true;
		int idx = _scene->_dynamicHotspots.add(NOUN_BATS, VERB_LOOK_AT, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
	}

	if (!_launched2Fl && (_vm->getRandomNumber(1, 30000) == 1)) {
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 5, 1, 0, 0);
		int idx = _scene->_dynamicHotspots.add(NOUN_BATS, VERB_LOOK_AT, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
		_launched2Fl = true;
	}

	if (_game._trigger == 73)
		_vm->_sound->command(37);

	if (_rexDivingFl && (_scene->_animation[0]->getCurrentFrame() >= 9)) {
		_vm->_sound->command(36);
		_rexDivingFl = false;
	}
}

void Scene111::preActions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_CAVE_ENTRANCE))
		_game._player._walkOffScreenSceneId = 212;
}

void Scene111::actions() {
	if (_action.isAction(VERB_DIVE_INTO, NOUN_POOL) && _game._objects.isInInventory(OBJ_REBREATHER)) {
		switch (_game._trigger) {
		case 0:
			_scene->loadAnimation(Resources::formatName(111, 'A', 1, EXT_AA, ""), 1);
			_rexDivingFl = true;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			break;

		case 1:
			_scene->_nextSceneId = 110;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_CAVE_FLOOR))
		_vm->_dialogs->show(11101);
	else if (_action.isAction(VERB_LOOK, NOUN_POOL))
		_vm->_dialogs->show(11102);
	else if (_action.isAction(VERB_LOOK, NOUN_CAVE_ENTRANCE))
		_vm->_dialogs->show(11103);
	else if (_action.isAction(VERB_LOOK, NOUN_STALAGMITES))
		_vm->_dialogs->show(11104);
	else if (_action.isAction(VERB_LOOK, NOUN_LARGE_STALAGMITE))
		_vm->_dialogs->show(11105);
	else if ((_action.isAction(VERB_PULL) || _action.isAction(VERB_TAKE)) && (_action.isObject(NOUN_STALAGMITES) || _action.isObject(NOUN_LARGE_STALAGMITE)))
		_vm->_dialogs->show(11106);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene112::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene112::enter() {
	sceneEntrySound();

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('X', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('X', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('X', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('X', 5));

	_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 10, 0, 17, 20);
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 0, 0, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 3, 0);

	_game._player._stepEnabled = false;
	_game._player._visible = false;

	_scene->_userInterface.emptyConversationList();
	_scene->_userInterface.setup(kInputConversation);

	_scene->loadAnimation(Resources::formatName(112, 'X', -1, EXT_AA, ""), 70);
}

void Scene112::step() {
	if ((_scene->_animation[0] != nullptr) && (_game._storyMode == STORYMODE_NICE)) {
		if (_scene->_animation[0]->getCurrentFrame() >= 54) {
			_scene->freeAnimation();
			_game._trigger = 70;
		}
	}

	if (_game._trigger == 70) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 3, 0, 11);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
	}

	if (_game._trigger == 71) {
		_scene->_nextSceneId = 101;
		_game._player._stepEnabled = true;
		_game._player._visible = true;
	}
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
