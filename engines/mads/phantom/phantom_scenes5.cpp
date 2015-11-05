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
#include "mads/conversations.h"
#include "mads/scene.h"
#include "mads/phantom/phantom_scenes.h"
#include "mads/phantom/phantom_scenes5.h"

namespace MADS {

namespace Phantom {

void Scene5xx::setAAName() {
	_game._aaName = Resources::formatAAName(1);
	_vm->_palette->setEntry(254, 43, 47, 51);
}

void Scene5xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		return;

	if ((_globals[kCoffinStatus] == 2) && !_game._visitedScenes.exists(506) && (_globals[kFightStatus] == 0) && (_scene->_currentSceneId == 504))
		_vm->_sound->command(33);
	else if (_scene->_currentSceneId == 505)
		_vm->_sound->command((_vm->_gameConv->_restoreRunning == 20) ? 39 : 16);
	else
		_vm->_sound->command(16);
}

void Scene5xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;
	if (!_game._player._forcePrefix)
		_game._player._spritesPrefix = "RAL";
	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_game._player._scalingVelocity = true;
}

/*------------------------------------------------------------------------*/

Scene501::Scene501(MADSEngine *vm) : Scene5xx(vm) {
	_anim0ActvFl = false;
	_skipFl = false;
}

void Scene501::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_skipFl);
}

void Scene501::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_CHRISTINE);
	_scene->addActiveVocab(VERB_LOOK_AT);
	_scene->addActiveVocab(VERB_WALK_TO);
}

void Scene501::enter() {
	_scene->_hotspots.activate(NOUN_CHRISTINE, false);
	_scene->_hotspots.activate(NOUN_BOAT, false);

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_anim0ActvFl = false;
		_skipFl = false;
	}

	_vm->_gameConv->get(26);

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0), false);
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1), false);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2), false);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('a', 1), false);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('a', 0), false);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_6", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_9", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_8", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RDRR_6", false);

	_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		if (_globals[kChristineIsInBoat]) {
			_anim0ActvFl = true;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 1), 100);
			_scene->setAnimFrame(_globals._animationIndexes[0], 124);
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true, Common::Point(113, 93));
			_scene->_hotspots.activate(NOUN_BOAT, true);
		}

		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
	}

	if (_scene->_priorSceneId == 506) {
		_game._player._playerPos = Common::Point(305, 112);
		_game._player._facing = FACING_WEST;
		_game._player._stepEnabled = false;

		if (_globals[kChristineIsInBoat]) {
			_anim0ActvFl = true;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 1), 100);
			_scene->setAnimFrame(_globals._animationIndexes[0], 124);
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true, Common::Point(113, 93));
			_scene->_hotspots.activate(NOUN_BOAT, true);
			_game._player.walk(Common::Point(260, 112), FACING_SOUTHWEST);
			_game._player.setWalkTrigger(80);
			_game._player.setWalkTrigger(55);
		} else {
			_anim0ActvFl = true;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 1), 100);
			_globals[kChristineIsInBoat] = true;
			_scene->_hotspots.activate(NOUN_BOAT, true);
			_game._player.walk(Common::Point(260, 112), FACING_SOUTHWEST);
			_game._player.setWalkTrigger(80);
		}

		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);

		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
	} else if ((_scene->_priorSceneId == 401) || (_scene->_priorSceneId == 408) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player.firstWalk(Common::Point(-20, 109), FACING_EAST, Common::Point(24, 109), FACING_EAST, true);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);

		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
	}

	sceneEntrySound();
}

void Scene501::step() {
	switch (_game._trigger) {
	case 55:
		_scene->deleteSequence(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[2], 0, 0, 56);
		break;

	case 56:
		_vm->_sound->command(25);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		if (!_globals[kChrisWillTakeSeat])
			_game._player._stepEnabled = true;

		_globals[kChrisWillTakeSeat] = false;
		break;

	default:
		break;
	}


	switch (_game._trigger) {
	case 60:
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 0, 0, 61);
		break;

	case 61:
		_vm->_sound->command(25);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
		_game._player._stepEnabled = true;
		break;

	case 80: {
		_game._player.walk(Common::Point(255, 118), FACING_NORTHWEST);
		_scene->setAnimFrame(_globals._animationIndexes[0], 2);
		int idx = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->setDynamicAnim(idx, _globals._animationIndexes[0], 0);
		_scene->setDynamicAnim(idx, _globals._animationIndexes[0], 1);
		_scene->setDynamicAnim(idx, _globals._animationIndexes[0], 2);
		_scene->setDynamicAnim(idx, _globals._animationIndexes[0], 3);
		}
		break;

	case 90:
		_globals[kPlayerScore] += 5;
		_scene->_nextSceneId = 310;
		break;

	case 100:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 9, 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[5], 0, 0, 90);
		break;

	default:
		break;
	}

	if (_anim0ActvFl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 103)
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true, Common::Point(125, 94));

		if ((_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 28) && !_skipFl) {
			_skipFl = true;
			_scene->_sequences.setTimingTrigger(1, 55);
		}

		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 124) {
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, false, Common::Point(125, 94));
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true , Common::Point(113, 93));
		}

		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 125)
			_scene->setAnimFrame(_globals._animationIndexes[0], 124);

		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 1)
			_scene->setAnimFrame(_globals._animationIndexes[0], 0);

		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 84)
			_game._player._stepEnabled = true;
	}
}

void Scene501::actions() {
	if (_vm->_gameConv->_running == 26) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST) && (_globals[kChristineIsInBoat])) {
		_vm->_gameConv->run(26);
		_vm->_gameConv->exportValue(3);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_CHRISTINE)) {
		_vm->_gameConv->run(26);
		_vm->_gameConv->exportValue(1);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_INTO, NOUN_BOAT)) {
		if (_game._objects.isInInventory(OBJ_OAR))
			_anim0ActvFl = false;
		else
			_vm->_dialogs->show(50123);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_UNLOCK, NOUN_DOOR) || _action.isAction(VERB_LOCK, NOUN_DOOR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 1);
			break;

		case 1: {
			int idx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 4);
			_game.syncTimers(1, _globals._sequenceIndexes[3], 1, idx);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
			_scene->_sequences.setTimingTrigger(15, 2);
			_vm->_sound->command(74);
			}
			break;

		case 2:
			_scene->deleteSequence(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 3);
			break;

		case 3:
			_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[3]);
			_game._player._visible = true;
			_vm->_dialogs->show(50122);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR)) {
		if (_scene->_customDest.x < 287) {
			if (!_globals[kChristineIsInBoat]) {
				switch (_game._trigger) {
				case 0:
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 1);
					break;

				case 1: {
					int idx = _globals._sequenceIndexes[3];
					_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 4);
					_game.syncTimers(1, _globals._sequenceIndexes[3], 1, idx);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.setTimingTrigger(15, 2);
					_vm->_sound->command(74);
					}
					break;

				case 2:
					_scene->deleteSequence(_globals._sequenceIndexes[3]);
					_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 3);
					break;

				case 3:
					_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[3]);
					_game._player._visible = true;
					_vm->_dialogs->show(50120);
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
			} else {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 2, 4, 65);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 67);
					break;

				case 65:
					_scene->deleteSequence(_globals._sequenceIndexes[2]);
					_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[2], 0, 0, 66);
					_vm->_sound->command(24);
					break;

				case 66:
					_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -2);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
					_game._player.walk(Common::Point(305, 112), FACING_EAST);
					_game._player.setWalkTrigger(68);
					break;

				case 67:
					_game._player._visible = true;
					break;

				case 68:
					_vm->_gameConv->abortConv();
					_scene->_nextSceneId = 506;
					break;
				}
			}
		} else {
			if (!_globals[kChristineIsInBoat]) {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 2, 4, 65);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 67);
					break;

				case 65: {
					int idx = _globals._sequenceIndexes[1];
					_scene->deleteSequence(_globals._sequenceIndexes[1]);
					_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
					_game.syncTimers(1, _globals._sequenceIndexes[1], 1, idx);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 0, 0, 66);
					_vm->_sound->command(24);
						 }
						 break;

				case 66:
					_game._player.walk(Common::Point(319, 116), FACING_NORTHWEST);
					_game._player.setWalkTrigger(68);
					break;

				case 67:
					_game._player._visible = true;
					break;

				case 68:
					_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 0, 0, 69);
					_vm->_sound->command(25);
					break;

				case 69:
					_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 5);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
					_scene->_nextSceneId = 502;
					break;
				}
			} else {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 1);
					break;

				case 1: {
					int idx = _globals._sequenceIndexes[3];
					_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 4);
					_game.syncTimers(1, _globals._sequenceIndexes[3], 1, idx);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.setTimingTrigger(15, 2);
					_vm->_sound->command(73);
						}
						break;

				case 2:
					_scene->deleteSequence(_globals._sequenceIndexes[3]);
					_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 3);
					break;

				case 3:
					_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[3]);
					_game._player._visible = true;
					_vm->_dialogs->show(50120);
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(50110);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(50111);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(50112);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(50113);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LAKE)) {
			_vm->_dialogs->show(50114);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOAT)) {
			_vm->_dialogs->show(50126);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TORCH)) {
			_vm->_dialogs->show(50117);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY_TO_WEST)) {
			_vm->_dialogs->show(50118);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			if (_scene->_customDest.x < 287) {
				if (_game._visitedScenes.exists(506))
					_vm->_dialogs->show(50127);
				else
					_vm->_dialogs->show(50119);
			} else {
				if (_game._visitedScenes.exists(506))
					_vm->_dialogs->show(50128);
				else
					_vm->_dialogs->show(50119);
			}
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COLUMN)) {
			_vm->_dialogs->show(50121);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHRISTINE)) {
			_vm->_dialogs->show(50124);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_TORCH)) {
		_vm->_dialogs->show(50125);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_CHRISTINE)) {
		_vm->_dialogs->show(50129);
		_action._inProgress = false;
	}
}

void Scene501::preActions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST)) {
		if (_globals[kLanternStatus] == 0) {
			_game._player._needToWalk = false;
			_vm->_dialogs->show(30918);
			_game._player.cancelCommand();
		} else if (!_globals[kChristineIsInBoat])
			_game.enterCatacombs(0);
	}

	if ((_action.isObject(NOUN_DOOR)) && (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK) || _action.isAction(VERB_OPEN))) {
		if (_scene->_customDest.x < 287)
			_game._player.walk(Common::Point(266, 112), FACING_EAST);
		else
			_game._player.walk(Common::Point(287, 118), FACING_EAST);
	}

	if ((_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR)) && _game._visitedScenes.exists(506) && _scene->_customDest.x < 287) {
		switch (_game._trigger) {
		case 0:
			_game._player._readyToWalk = false;
			_game._player._needToWalk = false;
			_game._player._stepEnabled = false;
			_vm->_gameConv->run(26);
			_vm->_gameConv->exportValue(2);
			_scene->_sequences.setTimingTrigger(6, 1);
			break;

		case 1:
			if (_vm->_gameConv->_running >= 0)
				_scene->_sequences.setTimingTrigger(6, 1);
			else {
				_game._player._stepEnabled = true;
				_action._inProgress = true;
				_game._player._needToWalk = true;
				_game._player._readyToWalk = true;
			}
			break;

		default:
			break;
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST) && _globals[kLanternStatus] && _globals[kRightDoorIsOpen504])
		_game._player.walk(Common::Point(24, 110), FACING_WEST);
}

/*------------------------------------------------------------------------*/

Scene502::Scene502(MADSEngine *vm) : Scene5xx(vm) {
	_fire1ActiveFl = false;
	_fire2ActiveFl = false;
	_fire3ActiveFl = false;
	_fire4ActiveFl = false;
	_panelTurningFl = false;
	_trapDoorHotspotEnabled = false;
	_acceleratedFireActivationFl = false;

	for (int i = 0; i < 16; i++) {
		_puzzlePictures[i] = -1;
		_puzzleSprites[i] = -1;
		_puzzleSequences[i] = -1;
	}

	_panelPushedNum = -1;
	_messageLevel = -1;
	_cycleStage = -1;

	_nextPos = Common::Point(-1, -1);

	_lastFrameTime = 0;
	_timer = 0;
	_deathTimer = 0;

	_cyclePointer = nullptr;
}

Scene502::~Scene502() {
	if (_cyclePointer)
		delete(_cyclePointer);
}

void Scene502::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsByte(_fire1ActiveFl);
	s.syncAsByte(_fire2ActiveFl);
	s.syncAsByte(_fire3ActiveFl);
	s.syncAsByte(_fire4ActiveFl);
	s.syncAsByte(_panelTurningFl);
	s.syncAsByte(_trapDoorHotspotEnabled);
	s.syncAsByte(_acceleratedFireActivationFl);

	for (int i = 0; i < 16; i++) {
		s.syncAsSint16LE(_puzzlePictures[i]);
		s.syncAsSint16LE(_puzzleSprites[i]);
		s.syncAsSint16LE(_puzzleSequences[i]);
	}

	s.syncAsSint16LE(_panelPushedNum);
	s.syncAsSint16LE(_messageLevel);
	s.syncAsSint16LE(_cycleStage);

	s.syncAsSint16LE(_nextPos.x);
	s.syncAsSint16LE(_nextPos.y);

	s.syncAsUint32LE(_lastFrameTime);
	s.syncAsUint32LE(_timer);
	s.syncAsUint32LE(_deathTimer);

	warning("more syncing required");
}

void Scene502::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene502::enter() {
	loadCyclingInfo();
	_scene->loadSpeech(7);

	_panelPushedNum = -1;
	_panelTurningFl = false;
	_fire1ActiveFl = false;
	_fire2ActiveFl = false;
	_fire3ActiveFl = false;
	_fire4ActiveFl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_lastFrameTime = _scene->_frameStartTime;
		_cycleStage = 0;
		_timer = 0;
		_deathTimer = 0;
		_messageLevel = 1;
		_acceleratedFireActivationFl = true;
		_trapDoorHotspotEnabled = false;
	}

	_scene->_hotspots.activate(NOUN_ROPE, false);
	_scene->_hotspots.activateAtPos(NOUN_TRAP_DOOR, false, Common::Point(225, 28));

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 2), false);
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 3), false);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 4), false);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 5), false);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 2), false);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 0), false);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 1), false);
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 1), false);
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 3), false);
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('x', 6), false);
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('a', 0), false);
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('j', 0), false);
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('k', 0), false);
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('l', 0), false);
	_globals._spriteIndexes[14] = _scene->_sprites.addSprites(formAnimName('m', 0), false);
	_globals._spriteIndexes[16] = _scene->_sprites.addSprites(formAnimName('h', 0), false);

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
	} else
		_scene->drawToBackground(_globals._spriteIndexes[5], -2, Common::Point(-32000, -32000), 0, 100);

	if ((_scene->_priorSceneId == 501) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		if (!_game._visitedScenes._sceneRevisited) {
			if (_game._objects.isInInventory(OBJ_ROPE))
				_globals[kCableHookWasSeparate] = true;
			else
				_globals[kCableHookWasSeparate] = false;
		} else if (_globals[kCableHookWasSeparate]) {
			_game._objects.addToInventory(OBJ_ROPE);
			_game._objects.addToInventory(OBJ_CABLE_HOOK);
			_game._objects.setRoom(OBJ_ROPE_WITH_HOOK, NOWHERE);
		} else {
			_game._objects.setRoom(OBJ_ROPE, NOWHERE);
			_game._objects.setRoom(OBJ_CABLE_HOOK, NOWHERE);
			_game._objects.addToInventory(OBJ_ROPE_WITH_HOOK);
		}

		_game._player._playerPos = Common::Point(43, 154);
		_game._player._facing = FACING_EAST;
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(87, 153), FACING_EAST);
		_game._player.setWalkTrigger(77);
	}

	room_502_initialize_panels();

	if (_trapDoorHotspotEnabled) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 1);
		_scene->_hotspots.activate(NOUN_TRAP_DOOR, false);
		_scene->_hotspots.activateAtPos(NOUN_TRAP_DOOR, true, Common::Point(225, 28));
		if (!_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK) && !_game._objects.isInInventory(OBJ_CABLE_HOOK)) {
			_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 12);
			_scene->_hotspots.activate(NOUN_ROPE, true);
		}
	} else {
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
	}

	sceneEntrySound();
}

void Scene502::step() {
	if (_acceleratedFireActivationFl) {
		int32 diff = _scene->_frameStartTime - _lastFrameTime;
		if ((diff >= 0) && (diff <= 4)) {
			_timer += diff;
			_deathTimer += diff;
		} else {
			_timer += 1;
			_deathTimer += 1;
		}
		_lastFrameTime = _scene->_frameStartTime;

		if (_timer >= 300) {
			_timer = 0;
			if (_cycleStage < 8)
				++_cycleStage;
		}
	}

	if ((_deathTimer >= 7200)  && !_panelTurningFl) {
		_vm->_dialogs->show(50215);
		_game._player.walk(Common::Point(160, 148), FACING_NORTH);
		_game._player.setWalkTrigger(71);
		_game._player._stepEnabled = false;
		_panelTurningFl = true;
		_deathTimer = 0;
	}

	if ((_deathTimer > 900) && (_messageLevel == 1) && !_panelTurningFl) {
		_messageLevel = 2;
		_vm->_dialogs->show(50212);
	}

	if ((_deathTimer > 3600) && (_messageLevel == 2) && !_panelTurningFl) {
		_messageLevel = 3;
		_vm->_dialogs->show(50213);
	}

	if ((_deathTimer > 5400) && (_messageLevel == 3) && !_panelTurningFl) {
		_messageLevel = 4;
		_vm->_dialogs->show(50214);
	}

	switch (_game._trigger) {
	case 71:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -2);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[4], 0, 0, 72);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[4], 2, 44, 73);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[4], 2, 51, 74);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[4], 2, 32, 75);
		break;

	case 72:
		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		_scene->_userInterface.noInventoryAnim();
		// CHECKME: Not sure about the next function call
		_scene->_userInterface.refresh();
		_scene->_sequences.setTimingTrigger(120, 76);
		break;

	case 73:
		_vm->_sound->command(1);
		_vm->_sound->command(67);
		break;

	case 74:
		_vm->_sound->command(27);
		break;

	case 75:
		_scene->playSpeech(7);
		break;

	case 76:
		_scene->_reloadSceneFlag = true;
		break;

	case 77:
		_scene->deleteSequence(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 7, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[5], 0, 0, 78);
		break;

	case 78:
		_vm->_dialogs->show(50211);
		_scene->drawToBackground(_globals._spriteIndexes[5], -2, Common::Point(-32000, -32000), 0, 100);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	if (!_trapDoorHotspotEnabled)
		animateFireBursts();

	setPaletteCycle();
}

void Scene502::actions() {
	if (_game._trigger >= 110) {
		handlePanelAnimation();
		_action._inProgress = false;
		return;
	}

	switch (_game._trigger) {
	case 80:
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 6);
		_scene->_hotspots.activateAtPos(NOUN_ROPE, true, Common::Point(225, 28));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 1);
		_scene->_hotspots.activate(NOUN_TRAP_DOOR, false);
		_scene->_hotspots.activateAtPos(NOUN_TRAP_DOOR, true, Common::Point(225, 28));
		if (!_panelTurningFl)
			_vm->_dialogs->show(50216);

		_action._inProgress = false;
		return;

	case 90:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 7, 2);
		_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[10], true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 14, 18);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 0, 0, 91);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 2, 18, 110);
		_action._inProgress = false;
		return;

	case 91:
		_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[10]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_scene->_sequences.setTimingTrigger(5, 102);
		_action._inProgress = false;
		return;

	case 95:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 7, 2);
		_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[10], true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 8, 13);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 0, 0, 96);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 2, 13, 110);
		_action._inProgress = false;
		return;

	case 96:
		_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[10]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_scene->_sequences.setTimingTrigger(5, 102);
		_action._inProgress = false;
		return;

	case 100:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 9, 2);
		_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[10], true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 5, 7);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 0, 0, 101);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 2, 7, 110);
		_action._inProgress = false;
		return;

	case 101:
		_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[10]);
		_game._player._visible = true;
		_scene->_sequences.setTimingTrigger(5, 102);
		_action._inProgress = false;
		return;

	case 102:
		_panelTurningFl = false;
		_game._player._stepEnabled = true;
		_action._inProgress = false;
		return;

	case 105:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 8, 2);
		_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[10], true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 1, 4);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 0, 0, 106);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 2, 4, 110);
		_action._inProgress = false;
		return;

	case 106:
		_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[10]);
		_game._player._visible = true;
		_scene->_sequences.setTimingTrigger(5, 102);
		_action._inProgress = false;
		return;

	default:
		break;
	}

	if (_action.isAction(VERB_PUSH, NOUN_PANEL)) {
		if (_panelTurningFl) {
			_action._inProgress = false;
			return;
		}

		Common::Point walkToPos;
		getPanelInfo(&walkToPos, &_panelPushedNum, _scene->_customDest, &_nextPos);
		_panelTurningFl = true;

		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_scene->_sequences.setTimingTrigger(1, 90);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_scene->_sequences.setTimingTrigger(1, 95);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_scene->_sequences.setTimingTrigger(1, 100);
			break;

		default:
			_scene->_sequences.setTimingTrigger(1, 105);
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_THROW, NOUN_ROPE_WITH_HOOK, NOUN_TRAP_DOOR) || _action.isAction(VERB_GRAPPLE, NOUN_TRAP_DOOR)) {
		if (_trapDoorHotspotEnabled) {
			switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_panelTurningFl = true;
			_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 13);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], -1, -2);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[7], 0, 0, 82);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[7], 2, 10, 83);
			_game._objects.setRoom(OBJ_ROPE_WITH_HOOK, NOWHERE);
			break;

		case 82:
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			_panelTurningFl = false;
			_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[7]);
			_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 13);
			_scene->_hotspots.activate(NOUN_ROPE, true);
			break;

		case 83:
			_vm->_sound->command(69);
			break;

		default:
			break;
			}
		} else
			_vm->_dialogs->show(50229);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR)
	 && (_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK) || _game._objects.isInInventory(OBJ_CABLE_HOOK))) {
		_vm->_dialogs->show(50228);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_THROW, NOUN_ROPE, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(50226);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_THROW, NOUN_CABLE_HOOK, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(50227);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB, NOUN_ROPE) || _action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR)) {
		switch (_game._trigger) {
		case 0:
			_globals[kPlayerScore] += 5;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_panelTurningFl = true;
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 10);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -1, -2);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[8], 0, 0, 82);
			break;

		case 82:
			_scene->_nextSceneId = 504;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(50210);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(50217);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TRAP_DOOR)) {
			_vm->_dialogs->show(_trapDoorHotspotEnabled ? 50220 : 50225);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(50219);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			_vm->_dialogs->show(50221);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PANELS)) {
			_vm->_dialogs->show(50222);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PANEL)) {
			_vm->_dialogs->show(50223);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(50224);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ROPE) && !_game._objects.isInInventory(OBJ_ROPE) && !_game._objects.isInInventory(OBJ_CABLE_HOOK) && !_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK)) {
			_vm->_dialogs->show(50233);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_OPEN, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(_trapDoorHotspotEnabled ? 50230 : 50228);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(_trapDoorHotspotEnabled ? 50228 : 50231);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_ROPE) && !_game._objects.isInInventory(OBJ_ROPE) && !_game._objects.isInInventory(OBJ_CABLE_HOOK) && !_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK)) {
		_vm->_dialogs->show(50234);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LASSO, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(50232);
		_action._inProgress = false;
		return;
	}
}

void Scene502::preActions() {
	int panel;

	if (_action.isAction(VERB_PUSH, NOUN_PANEL)) {
		Common::Point walkToPos;
		Common::Point tmpPos;
		getPanelInfo(&walkToPos, &panel, _scene->_customDest, &tmpPos);
		_game._player.walk(walkToPos, FACING_NORTH);
	}

	if (_trapDoorHotspotEnabled && (_action.isAction(VERB_CLIMB, NOUN_ROPE) || _action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR)))
		_game._player.walk(Common::Point(211, 149), FACING_NORTH);

	if (_trapDoorHotspotEnabled && (_action.isAction(VERB_THROW, NOUN_ROPE_WITH_HOOK, NOUN_TRAP_DOOR) || _action.isAction(VERB_GRAPPLE, NOUN_TRAP_DOOR)))
		_game._player.walk(Common::Point(200, 149), FACING_NORTH);
}

void Scene502::room_502_initialize_panels() {
	for (int i = 0, curPuzzleSprite = 2, count = 1; i < 16; i++) {
		if (_scene->_priorSceneId != RETURNING_FROM_LOADING)
			_puzzlePictures[i] = _vm->getRandomNumber(1, 4);

		curPuzzleSprite += (_puzzlePictures[i] * 3) - 3;
		_puzzleSprites[i] = curPuzzleSprite;

		int sprIdx;

		switch (i) {
		case 0:
		case 1:
		case 2:
		case 3:
			sprIdx = _globals._spriteIndexes[11];
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			sprIdx = _globals._spriteIndexes[12];
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			sprIdx = _globals._spriteIndexes[13];
			break;

		default:
			sprIdx = _globals._spriteIndexes[14];
			break;
		}

		_globals._sequenceIndexes[15] = _scene->_sequences.addStampCycle(sprIdx, false, curPuzzleSprite);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[15], 14);
		_puzzleSequences[i] = _globals._sequenceIndexes[15];

		++count;
		if (count >= 5)
			count = 1;

		switch (count) {
		case 1:
			curPuzzleSprite = 2;
			break;

		case 2:
			curPuzzleSprite = 14;
			break;

		case 3:
			curPuzzleSprite = 26;
			break;

		case 4:
			curPuzzleSprite = 38;
			break;
		}
	}
}

void Scene502::loadCyclingInfo() {
	warning("TODO: loadCyclingInfo");
}

void Scene502::animateFireBursts() {
	int rndTrigger;

	if (_acceleratedFireActivationFl)
		rndTrigger = _vm->getRandomNumber(1, 50);
	else
		rndTrigger = _vm->getRandomNumber(1, 400);

	if (rndTrigger == 1) {
		rndTrigger = _vm->getRandomNumber(1, 4);

		switch (rndTrigger) {
		case 1:
			if (!_fire1ActiveFl) {
				_scene->_sequences.setTimingTrigger(_vm->getRandomNumber(300, 600), 60);
				_fire1ActiveFl = true;
			}
			break;

		case 2:
			if (!_fire2ActiveFl) {
				_scene->_sequences.setTimingTrigger(_vm->getRandomNumber(300, 600), 63);
				_fire2ActiveFl = true;
			}
			break;

		case 3:
			if (!_fire3ActiveFl) {
				_scene->_sequences.setTimingTrigger(_vm->getRandomNumber(300, 600), 66);
				_fire3ActiveFl = true;
			}
			break;

		case 4:
			if (!_fire4ActiveFl) {
				_scene->_sequences.setTimingTrigger(_vm->getRandomNumber(300, 600), 69);
				_fire4ActiveFl = true;
			}
			break;
		}
	}

	switch (_game._trigger) {
	case 60:
		if ((_game._player._playerPos.x < 198) || (_game._player._playerPos.y > 150)) {
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 5, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 10);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[0], 0, 0, 61);
		}
		break;

	case 61:
		_fire1ActiveFl = false;
		break;

	case 63:
		if ((_game._player._playerPos.x > 127) || (_game._player._playerPos.y < 150)) {
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 10);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 0, 0, 64);
		}
		break;

	case 64:
		_fire2ActiveFl = false;
		break;

	case 66:
		if (_game._player._playerPos.x < 198) {
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 5, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 10);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[2], 0, 0, 67);
		}
		break;

	case 67:
		_fire3ActiveFl = false;
		break;

	case 69:
		if ((_game._player._playerPos.x > 110) || (_game._player._playerPos.y > 150)) {
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 10);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 70);
		}
		break;

	case 70:
		_fire4ActiveFl = false;
		break;

	default:
		break;
	}
}

void Scene502::setPaletteCycle() {
	warning("TODO: setPaletteCycle");
}

void Scene502::getPanelInfo(Common::Point *walkToPos, int *panel, Common::Point mousePos, Common::Point *interimPos) {
	walkToPos->y = 148;

	if ((mousePos.x < 120) || (mousePos.y < 75) || (mousePos.y > 137))
		return;

	if (mousePos.x <= 139) {
		interimPos->x = 129;
		if (mousePos.y <= 90) {
			*panel = 0;
			interimPos->y = 90;
			walkToPos->x = 107;
		} else if (mousePos.y <= 106) {
			*panel = 4;
			interimPos->y = 106;
			walkToPos->x = 107;
		} else if (mousePos.y <= 122) {
			*panel = 8;
			interimPos->y = 122;
			walkToPos->x = 107;
		} else {
			*panel = 12;
			interimPos->y = 138;
			walkToPos->x = 107;
		}
	} else if (mousePos.x <= 159) {
		interimPos->x = 149;
		if (mousePos.y <= 90) {
			*panel = 1;
			interimPos->y = 90;
			walkToPos->x = 127;
		} else if (mousePos.y <= 106) {
			*panel = 5;
			interimPos->y = 106;
			walkToPos->x = 127;
		} else if (mousePos.y <= 122) {
			*panel = 9;
			interimPos->y = 122;
			walkToPos->x = 127;
		} else {
			*panel = 13;
			interimPos->y = 138;
			walkToPos->x = 127;
		}
	} else if (mousePos.x <= 179) {
		interimPos->x = 169;
		if (mousePos.y <= 90) {
			*panel = 2;
			interimPos->y = 90;
			walkToPos->x = 147;
		} else if (mousePos.y <= 106) {
			*panel = 6;
			interimPos->y = 106;
			walkToPos->x = 147;
		} else if (mousePos.y <= 122) {
			*panel = 10;
			interimPos->y = 122;
			walkToPos->x = 147;
		} else {
			*panel = 14;
			interimPos->y = 138;
			walkToPos->x = 147;
		}
	} else if (mousePos.x <= 199) {
		interimPos->x = 189;
		if (mousePos.y <= 90) {
			*panel = 3;
			interimPos->y = 90;
			walkToPos->x = 167;
		} else if (mousePos.y <= 106) {
			*panel = 7;
			interimPos->y = 106;
			walkToPos->x = 167;
		} else if (mousePos.y <= 122) {
			*panel = 11;
			interimPos->y = 122;
			walkToPos->x = 167;
		} else {
			*panel = 15;
			interimPos->y = 138;
			walkToPos->x = 167;
		}
	}
}

void Scene502::handlePanelAnimation() {
	int puzzleSolvedFl = true;

	switch (_game._trigger) {
	case 110:
		_vm->_sound->command(65);
		_scene->deleteSequence(_puzzleSequences[_panelPushedNum]);
		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, _puzzleSprites[_panelPushedNum] - 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);
			_scene->_sequences.setTimingTrigger(5, 111);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, _puzzleSprites[_panelPushedNum] - 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);
			_scene->_sequences.setTimingTrigger(5, 111);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_globals._sequenceIndexes[13] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[13], false, _puzzleSprites[_panelPushedNum] - 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 14);
			_scene->_sequences.setTimingTrigger(5, 111);
			break;

		default:
			_globals._sequenceIndexes[14] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[14], false, _puzzleSprites[_panelPushedNum] - 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 14);
			_scene->_sequences.setTimingTrigger(5, 111);
			break;
		}
		break;

	case 111:
		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_scene->deleteSequence(_globals._sequenceIndexes[11]);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_scene->deleteSequence(_globals._sequenceIndexes[12]);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_scene->deleteSequence(_globals._sequenceIndexes[13]);
			break;

		default:
			_scene->deleteSequence(_globals._sequenceIndexes[14]);
			break;
		}

		_globals._sequenceIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[16], false, 5, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[16], 14);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[16], _nextPos);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[16], -1, -2);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[16], 0, 0, 112);
		break;

	case 112: {
		int idx = _globals._sequenceIndexes[16];
		int newSprId = _puzzleSprites[_panelPushedNum] + 4;

		switch (_panelPushedNum) {
		case 0:
		case 4:
		case 8:
		case 12:
			if (newSprId > 12)
				newSprId = 3;
			break;

		case 1:
		case 5:
		case 9:
		case 13:
			if (newSprId > 24)
				newSprId = 15;
			break;

		case 2:
		case 6:
		case 10:
		case 14:
			if (newSprId > 36)
				newSprId = 27;
			break;

		default:
			if (newSprId > 48)
				newSprId = 39;
			break;
		}

		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);
			_game.syncTimers(1, _globals._sequenceIndexes[11], 1, idx);
			_scene->_sequences.setTimingTrigger(5, 113);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);
			_game.syncTimers(1, _globals._sequenceIndexes[12], 1, idx);
			_scene->_sequences.setTimingTrigger(5, 113);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_globals._sequenceIndexes[13] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[13], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 14);
			_game.syncTimers(1, _globals._sequenceIndexes[13], 1, idx);
			_scene->_sequences.setTimingTrigger(5, 113);
			break;

		default:
			_globals._sequenceIndexes[14] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[14], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 14);
			_game.syncTimers(1, _globals._sequenceIndexes[14], 1, idx);
			_scene->_sequences.setTimingTrigger(5, 113);
			break;
		}
		}
		break;

	case 113: {
		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_scene->deleteSequence(_globals._sequenceIndexes[11]);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_scene->deleteSequence(_globals._sequenceIndexes[12]);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_scene->deleteSequence(_globals._sequenceIndexes[13]);
			break;

		default:
			_scene->deleteSequence(_globals._sequenceIndexes[14]);
			break;
		}

		int newSprId = _puzzleSprites[_panelPushedNum] + 3;

		switch (_panelPushedNum) {
		case 0:
		case 4:
		case 8:
		case 12:
			if (newSprId > 12)
				newSprId = 2;
			break;

		case 1:
		case 5:
		case 9:
		case 13:
			if (newSprId > 24)
				newSprId = 14;
			break;

		case 2:
		case 6:
		case 10:
		case 14:
			if (newSprId > 36)
				newSprId = 26;
			break;

		default:
			if (newSprId > 48)
				newSprId = 38;
			break;
		}
		_puzzleSprites[_panelPushedNum] = newSprId;
		++_puzzlePictures[_panelPushedNum];
		if (_puzzlePictures[_panelPushedNum] >= 5)
			_puzzlePictures[_panelPushedNum] = 1;

		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);
			_puzzleSequences[_panelPushedNum] = _globals._sequenceIndexes[11];
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);
			_puzzleSequences[_panelPushedNum] = _globals._sequenceIndexes[12];
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_globals._sequenceIndexes[13] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[13], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 14);
			_puzzleSequences[_panelPushedNum] = _globals._sequenceIndexes[13];
			break;

		default:
			_globals._sequenceIndexes[14] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[14], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 14);
			_puzzleSequences[_panelPushedNum] = _globals._sequenceIndexes[14];
			break;
		}

		for (int i = 0; i < 16; i++) {
			if (_puzzlePictures[i] != 1)
				puzzleSolvedFl = false;
		}

		if (puzzleSolvedFl && !_trapDoorHotspotEnabled) {
			_trapDoorHotspotEnabled = true;
			_scene->deleteSequence(_globals._sequenceIndexes[6]);
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('t', 1), 80);
		}
		}
		break;

	default:
		break;
	}
}
/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
